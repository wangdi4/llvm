/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ShuffleCallToInst.cpp

\*****************************************************************************/

#include "ShuffleCallToInst.h"

#include "llvm/Support/InstIterator.h"
#include "llvm/Constants.h"

#include "NameMangleAPI.h"

extern "C" {
    /// @brief Creates new ShuffleCallToInst pass
    void* createShuffleCallToInstPass() {
        return new Intel::OpenCL::DeviceBackend::ShuffleCallToInst();
    }
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {
    using namespace llvm;

    /// @brief Pass identification, replacement for typeid
    char ShuffleCallToInst::ID = 0;


    /// @brief  LLVM Function pass entry
    /// @param  F  Function to transform
    /// @return true if changed
    bool ShuffleCallToInst::runOnFunction(Function &F) {
        findShuffleCalls(F);
        return handleShuffleCalls();
    }


    /// @brief Find all shuffle calls in current function
    void ShuffleCallToInst::findShuffleCalls(Function &F) {
        m_shuffleCalls.clear();

        // Run over all instructions in current function
        for (inst_iterator it = inst_begin(F), end = inst_end(F); it != end; ++it) {
            // Check if current instruction is a call instruction
            CallInst* inst = dyn_cast<CallInst>(&*it);
            if (!inst) continue;

            // Check if called function is a shuffle function and the mask is constant
            ShuffleType shuffleType = isConstShuffle(inst);
            std::pair<CallInst*, ShuffleType> shuffleCallPair;
            switch (shuffleType) {
            case SHUFFLE1:
            case SHUFFLE2:
                // Add this instruction for handling
                shuffleCallPair = std::pair<CallInst*, ShuffleType>(inst, shuffleType);
                m_shuffleCalls.push_back(shuffleCallPair);
                break;
            default:
                continue;
            }
        }
    }


    /// @brief Handle all shuffle calls in current function
    bool ShuffleCallToInst::handleShuffleCalls() {
        // Check if there are shuffle calls
        if (m_shuffleCalls.empty()) return false;

        // Run over all shuffle calls in function
        for (unsigned i = 0; i < m_shuffleCalls.size(); ++i) {
            CallInst* shuffleCall = m_shuffleCalls[i].first;
            ShuffleType shuffleType = m_shuffleCalls[i].second;

            // Get function operands, depending on shuffle type
            Value *firstVec = NULL;
            Value *secondVec = NULL;
            Value *mask = NULL;
            switch (shuffleType) {
            case SHUFFLE2:
                // First vector operand
                firstVec = shuffleCall->getArgOperand(SHUFFLE2_VEC1_POS);
                // Second vector operand
                secondVec = shuffleCall->getArgOperand(SHUFFLE2_VEC2_POS);
                // Mask operand
                mask = shuffleCall->getArgOperand(SHUFFLE2_MASK_POS);
                break;

            case SHUFFLE1:
                // First vector operand
                firstVec = shuffleCall->getArgOperand(SHUFFLE_VEC1_POS);
                // Second vector operand: undef vector with type of firstVec
                secondVec = UndefValue::get(firstVec->getType());
                // Mask operand
                mask = shuffleCall->getArgOperand(SHUFFLE_MASK_POS);
                break;

            default:
                assert(0 && "Shuffle function of unknown type.");
            }

            // Convert mask type to vector of i32, shuffflevector mask scalar size is always 32
            Constant* newMask = NULL;
            unsigned int maskVecSize = mask->getType()->getNumElements();
            Type *maskType = VectorType::get(Type::getInt32Ty(shuffleCall->getContext()), maskVecSize);

            // We previously searched for shuffle calls with constant mask only
            // So we can assume mask is constant here
            // If mask scalar size is not 32 then Zext or Trunc the mask to get to 32
            if (mask->getType()->getScalarSizeInBits() < maskType->getScalarSizeInBits()) {
                newMask = ConstantExpr::getZExt(cast<Constant>(mask), maskType);
            }
            else if (mask->getType()->getScalarSizeInBits() > maskType->getScalarSizeInBits()) {
                newMask = ConstantExpr::getTrunc(cast<Constant>(mask), maskType);
            }
            else {
                newMask = cast<Constant>(mask);
            }

            // Create the new shufflevector instruction
            ShuffleVectorInst* newShuffleInst = new ShuffleVectorInst(firstVec, secondVec, newMask, "newShuffle", shuffleCall);
            shuffleCall->replaceAllUsesWith(newShuffleInst);
        }
        return true;
    }


    /// @brief Check if a given called function is shuffle with constant mask
    /// @return SHUFFLE1 or SHUFFLE2 in case of a shuffle function
    ///         else NOT_SHUFFLE
    ShuffleCallToInst::ShuffleType ShuffleCallToInst::isConstShuffle(CallInst* pCall) {
        // Get function name
        std::string calledFuncName = pCall->getCalledFunction()->getNameStr();
        std::string strippedName;
        try {
            // stripName is one of the NameMangler APIs which returns the demangled name of a function
            strippedName = stripName(calledFuncName.c_str());
        } catch (const char *) {
            // stripName throws an exception when the function name is not a mangled name
            return NOT_SHUFFLE;
        }

        // Check if its shuffle function and mask is constant
        if (strippedName == "shuffle" && isa<Constant>(pCall->getArgOperand(SHUFFLE_MASK_POS)) ) {
            return SHUFFLE1;
        }
        if (strippedName == "shuffle2" && isa<Constant>(pCall->getArgOperand(SHUFFLE2_MASK_POS)) ) {
            return SHUFFLE2;
        }
        return NOT_SHUFFLE;
    }

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {


/// Register pass for opt
static llvm::RegisterPass<Intel::OpenCL::DeviceBackend::ShuffleCallToInst> ShuffleCallToInstPass("shuffle-call-to-inst", "Replace calls to shuffle functions that has const mask with LLVM shuffle instruction.");
