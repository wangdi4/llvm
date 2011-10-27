/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  KernelUpdate.cpp

\*****************************************************************************/

// This pass is used for import of built-in functions from runtime module

#include "cl_device_api.h"
#include "cpu_dev_limits.h"
#include "TLLVMKernelInfo.h"
#include "OCLUpdatePass.h"

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/Constants.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Target/TargetData.h>
#include <llvm/System/DynamicLibrary.h>
#include <llvm/Support/raw_ostream.h>
// debug macros
#include "llvm/Support/Debug.h"


#include <map>

using namespace llvm;
using namespace std;
using namespace Intel::OpenCL::DeviceBackend;

const unsigned int BYTE_SIZE = 8;

extern "C" int getVectorizerFunctions(Pass *V, SmallVectorImpl<Function*> &Functions);

namespace Validation {

    // Update kernel assembly to match our execution environment
    class OCLReferenceKernelUpdate : public ModulePass
    {
    public:
        OCLReferenceKernelUpdate(Pass *pVect, SmallVectorImpl<Function*> &vectFunctions, 
            LLVMContext *pContext, std::vector<std::string> &UndefinedExternalFunctions) : 
        ModulePass(ID) , m_pVectorizer (pVect) , m_pVectFunctions (&vectFunctions),
            m_pLLVMContext(pContext), m_pUndefinedExternalFunctions(&UndefinedExternalFunctions) {}

        // doPassInitialization - For this pass, it removes global symbol table
        // entries for primitive types.  These are never used for linking in GCC and
        // they make the output uglier to look at, so we nuke them.
        //
        // Also, initialize instance variables.
        //
        bool runOnModule(Module &M);

    protected:
        static char ID; // Pass identification, replacement for typeid

        Module*                     m_pModule;
        llvm::NamedMDNode*          m_pOpenCLMetadata;
        Pass*                       m_pVectorizer;
        SmallVectorImpl<Function*>* m_pVectFunctions;
        LLVMContext*                m_pLLVMContext;

        std::vector<std::string>*   m_pUndefinedExternalFunctions;

        Function *RunOnKernel(Function *pFunc, NamedMDNode *localsAnchor);
        bool    IsAKernel(const Function* pFunc);
        bool    FunctionNeedsUpdate(Function *pFunc);
        bool    ParseLocalBuffers(Function* pFunc, NamedMDNode *localsAnchor,
            Argument* pLocalMem);
        void    UpdateCastToFunction(Function* func);

        void    AddWIInfoDeclarations();
        void    AddAsyncCopyDeclaration();
        void    AddPrefetchDeclaration();
        void    AddPrintfDeclaration();
        bool    m_bBarrierDecl;
        bool    m_bAsyncCopyDecl;
        bool    m_bPrefetchDecl;
        bool    m_bPrintfDecl;

        TLLVMKernelInfo	m_sInfo;
        map<const Function*, TLLVMKernelInfo>	m_mapKernelInfo;

        std::vector<Function*> m_nonInlinedFunctions;

        std::map<llvm::CallInst *, llvm::Value **> m_fixupCalls;

        // Calculate and return Global ID value for given dimension
        Value* SubstituteWIcall(llvm::CallInst *pCall,
            llvm::Argument* pWorkInfo, llvm::Argument* pWGid,
            llvm::Argument* pBaseGlbId, llvm::Argument* pLocalId);
        Value*  CalcGlobalId(llvm::CallInst *pCall, llvm::Argument* pWGinfo, llvm::Argument* pLocalId, bool bGlobal);
        Value*  m_GlbIds[MAX_WORK_DIM];

        // Barrier/AsyncCopy/Wait
        void    AddCtxAddress(llvm::CallInst* pCall, llvm::Argument* pLocalId);
        Value*  UpdatePrintf(llvm::CallInst* pCall, llvm::Argument* pLocalId);
        Value*  UpdateAsyncCopy(llvm::CallInst* pCall, llvm::Argument* pLocalId, bool strided);
        void    UpdateWaitGroup(llvm::CallInst* pCall, llvm::Argument* pLocalId);
        void    UpdatePrefetch(llvm::CallInst* pCall);
        Value*  m_pCtxPtr;

        friend  void getKernelInfoMap(ModulePass *pKUPath, map<const Function*, TLLVMKernelInfo>& infoMap);

        // Name of the built-in printf function to which calls of 'printf' in 
        // the kernel are translated
        //
        static const char* builtin_printf_name;
    };

    const char* OCLReferenceKernelUpdate::builtin_printf_name = "opencl_printf";
    char OCLReferenceKernelUpdate::ID = 0;

    ModulePass *createOCLReferenceKernelUpdatePass(Pass* pVect, SmallVectorImpl<Function*> &vectFunctions, 
        LLVMContext *context, std::vector<std::string> &UndefinedExternalFunctions) {
            return new OCLReferenceKernelUpdate(pVect, vectFunctions, context, UndefinedExternalFunctions);
    }

    BasicBlock::iterator removeInstruction(llvm::BasicBlock* pBB, llvm::BasicBlock::iterator it);

    void getKernelInfoMap(ModulePass *pKUPath, map<const Function*, TLLVMKernelInfo>& infoMap)
    {
        OCLReferenceKernelUpdate* pKU = static_cast<OCLReferenceKernelUpdate*>(pKUPath);

        infoMap.clear();
        if ( NULL != pKU )
        {
            infoMap.insert(pKU->m_mapKernelInfo.begin(), pKU->m_mapKernelInfo.end());
        }
    }

    bool OCLReferenceKernelUpdate::runOnModule(Module &M)
    {
        m_pModule = &M;

        m_bBarrierDecl = false;
        m_bAsyncCopyDecl = false;
        m_bPrefetchDecl = false;
        m_bPrintfDecl = false;

        m_mapKernelInfo.clear();

        m_nonInlinedFunctions.clear();

        m_fixupCalls.clear();

        AddWIInfoDeclarations();

        // Extract pointer to module annotations
        m_pOpenCLMetadata = m_pModule->getNamedMetadata("opencl.kernels");

        if ( NULL == m_pOpenCLMetadata )
        {
            // No metadata - no kernels. Nothing to update.
            return true;
        }

        if( NULL != m_pVectorizer )
        {
            getVectorizerFunctions(m_pVectorizer, *m_pVectFunctions);
        }

        // now we need to pass every kernel and make relevant changes to match our environment
        for (unsigned i = 0, e = m_pOpenCLMetadata->getNumOperands(); i != e; ++i) 
        {
            // Obtain kernel function from annotation
            llvm::MDNode *elt = m_pOpenCLMetadata->getOperand(i);
            Function *pFunc = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
            if ( NULL == pFunc )
            {
                continue;	// Not a function pointer
            }

            NamedMDNode *localsAnchor = m_pModule->getNamedMetadata(dyn_cast<llvm::MDString>(elt->getOperand(5))->getString());

            // Run on original kernel
#ifndef NDEBUG
            Function* pNewFunc = RunOnKernel(pFunc, localsAnchor);
            assert(pNewFunc);
#else
            RunOnKernel(pFunc, localsAnchor);
#endif
            // Run on vectorized kernel if available
            Function* pVectFunc;
            if ( !m_pVectFunctions->empty() && ( ( pVectFunc = (*m_pVectFunctions)[i]) != NULL ) )
            {
                (*m_pVectFunctions)[i] = RunOnKernel(pVectFunc, localsAnchor);
            }
        }

        for(unsigned int i = 0; i < m_nonInlinedFunctions.size(); i++)
        {
            RunOnKernel(m_nonInlinedFunctions[i], NULL);
        }

        while(!m_fixupCalls.empty())
        {
            CallInst* pCall     = m_fixupCalls.begin()->first;
            Value**   pCallArgs = m_fixupCalls.begin()->second;
            m_fixupCalls.erase(m_fixupCalls.begin());

            Function *pCallee = pCall->getCalledFunction();

            BasicBlock::iterator inst_it = pCall->getParent()->begin();
            while ( inst_it != pCall->getParent()->end() )
            {
                assert(dyn_cast<Instruction>(inst_it) && dyn_cast<Instruction>(pCall));
                if(dyn_cast<Instruction>(inst_it) == dyn_cast<Instruction>(pCall))
                {
                    break;
                }
                inst_it++;
            }

            assert(inst_it != pCall->getParent()->end());

            SmallVector<Value*, 4> params;
            // Create new call instruction with extended parameters
            params.clear();
            for(unsigned int i = 0; i < pCall->getNumArgOperands(); i++ )
            {
                params.push_back(pCall->getArgOperand(i));
            }
            params.push_back(pCallArgs[0]);
            params.push_back(pCallArgs[1]);
            params.push_back(pCallArgs[2]);
            params.push_back(pCallArgs[3]);

            CallInst *newCall = CallInst::Create(pCallee, params.begin(), params.end(), "", pCall);

            delete [] pCallArgs;

            pCall->uncheckedReplaceAllUsesWith(newCall);

            inst_it = removeInstruction(pCall->getParent(), inst_it);
        }

        return true;
    }

    Instruction* CreateInstrFromConstantExpr(ConstantExpr* pCE, Value* From, Value* To)
    {  
        Instruction *Replacement = 0;

        if (pCE->getOpcode() == Instruction::GetElementPtr)
        {
            SmallVector<Value*, 8> Indices;
            Value *Pointer = pCE->getOperand(0);
            Indices.reserve(pCE->getNumOperands()-1);
            if (Pointer == From) Pointer = To;

            for (unsigned i = 1, e = pCE->getNumOperands(); i != e; ++i) {
                Value *Val = pCE->getOperand(i);
                if (Val == From) Val = To;
                Indices.push_back(Val);
            }
            Replacement = GetElementPtrInst::Create(Pointer, Indices.begin(), Indices.end());
        }
        else if (pCE->getOpcode() == Instruction::ExtractValue)
        {
            Value *Agg = pCE->getOperand(0);
            if (Agg == From) Agg = To;

            const SmallVector<unsigned, 4> &Indices = pCE->getIndices();
            Replacement = ExtractValueInst::Create(Agg,	Indices.begin(), Indices.end());
        }
        else if (pCE->getOpcode() == Instruction::InsertValue)
        {
            Value *Agg = pCE->getOperand(0);
            Value *Val = pCE->getOperand(1);
            if (Agg == From) Agg = To;
            if (Val == From) Val = To;

            const SmallVector<unsigned, 4> &Indices = pCE->getIndices();
            Replacement = InsertValueInst::Create(Agg, Val, Indices.begin(), Indices.end());
        }
        else if (pCE->isCast())
        {
            assert(pCE->getOperand(0) == From && "Cast only has one use!");
            Replacement = CastInst::Create((Instruction::CastOps)pCE->getOpcode(), To, pCE->getType());
        }
        else if (pCE->getOpcode() == Instruction::Select)
        {
            Value *C1 = pCE->getOperand(0);
            Value *C2 = pCE->getOperand(1);
            Value *C3 = pCE->getOperand(2);
            if (C1 == From) C1 = To;
            if (C2 == From) C2 = To;
            if (C3 == From) C3 = To;
            Replacement = SelectInst::Create(C1, C2, C3);
        }
        else if (pCE->getOpcode() == Instruction::ExtractElement)
        {
            Value *C1 = pCE->getOperand(0);
            Value *C2 = pCE->getOperand(1);
            if (C1 == From) C1 = To;
            if (C2 == From) C2 = To;
            Replacement = ExtractElementInst::Create(C1, C2);
        }
        else if (pCE->getOpcode() == Instruction::InsertElement)
        {
            Value *C1 = pCE->getOperand(0);
            Value *C2 = pCE->getOperand(1);
            Value *C3 = pCE->getOperand(1);
            if (C1 == From) C1 = To;
            if (C2 == From) C2 = To;
            if (C3 == From) C3 = To;
            Replacement = InsertElementInst::Create(C1, C2, C3);
        }
        else if (pCE->getOpcode() == Instruction::ShuffleVector)
        {
            Value *C1 = pCE->getOperand(0);
            Value *C2 = pCE->getOperand(1);
            Value *C3 = pCE->getOperand(2);
            if (C1 == From) C1 = To;
            if (C2 == From) C2 = To;
            if (C3 == From) C3 = To;
            Replacement = new ShuffleVectorInst(C1, C2, C3);
        }
        else if (pCE->isCompare())
        {
            Value *C1 = pCE->getOperand(0);
            Value *C2 = pCE->getOperand(1);
            if (C1 == From) C1 = To;
            if (C2 == From) C2 = To;
            Replacement = CmpInst::Create((Instruction::OtherOps)pCE->getOpcode(), pCE->getPredicate(), C1, C2);
        }
        else
        {
            assert(0 && "Unknown ConstantExpr type!");
            return NULL;
        }

        return Replacement;
    }

    bool ChangeConstantExpression(Value* pTheValue, Value* pUser, Instruction* pBC, Instruction* Where)
    {
        // We need substitute constant expression with real instruction
        ConstantExpr* pCE = dyn_cast<ConstantExpr>(pUser);
        if ( NULL == pCE )
        {
            return false;
        }

        Instruction* pInst = CreateInstrFromConstantExpr(pCE, pTheValue, pBC);
        // Change all non-constant references
        ConstantExpr::use_iterator itCE = pCE->use_begin();
        while( itCE != pCE->use_end() )
        {
            Use &W = itCE.getUse();
            User* pLclUser = W.getUser();

            ConstantExpr* pLclCE = dyn_cast<ConstantExpr>(pLclUser);
            if ( NULL != pLclCE )
            {
                if ( ChangeConstantExpression(pUser, pLclUser, pInst, Where) )
                {
                    itCE = pCE->use_begin();        // Restart the scan
                }
                else
                {
                    ++itCE;
                }
                continue;	// continue to next usage
            }

            // Check if user is an instruction that belongs to the same function
            Instruction* pUsrInst = dyn_cast<Instruction>(pLclUser);
            if ( (NULL == pUsrInst) || (pUsrInst->getParent()->getParent() != Where->getParent()->getParent()) )
            {
                ++itCE;
                continue;
            }

            // Add instruction to the block, only the first time
            if ( pInst->use_empty() )
            {
                pInst->insertAfter(Where);
            }
            W.set(pInst);
            itCE = pCE->use_begin();		// Restart the scan
        }

        // Check if the instruction was not used
        if ( pInst->use_empty() )
        {
            delete pInst;
        }
        else if(!pInst->getParent())
        {
            pInst->insertAfter(Where);
        }

        if ( !pCE->use_empty() )
        {
            return false;
        }

        // No more references to the constant expression we can delete it
        delete pCE;
        return true;
    }

    // Substitutes a pointer to local buffer, with argument passed within kernel parameters
    bool OCLReferenceKernelUpdate::ParseLocalBuffers(Function* pFunc, NamedMDNode *localsAnchor, Argument* pLocalMem)
    {
        if(NULL == localsAnchor)
            return true;

        // Create code for local buffer
        Instruction* pFirstInst = dyn_cast<Instruction>(pFunc->getEntryBlock().begin());

        // Iterate through local buffers
        for (unsigned i = 0, e = localsAnchor->getNumOperands(); i != e; ++i) 
        {
            // Obtain local name from annotation
            MDNode *operand = localsAnchor->getOperand(i);
            GlobalValue *pLclBuff = m_pModule->getGlobalVariable(
                dyn_cast<llvm::MDString>(operand->getOperand(0))->getString(), true);

            if(NULL == pLclBuff) continue;

            // Calculate required buffer size
            llvm::TargetData TD(m_pModule);
            size_t uiArraySize = TD.getTypeSizeInBits(pLclBuff->getType()->getElementType())/8;
            assert ( 0 != uiArraySize );
            // Now retrieve to the offset of the local buffer
            GetElementPtrInst* pLocalAddr =
                GetElementPtrInst::Create(pLocalMem, ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), m_sInfo.stTotalImplSize), "", pFirstInst);
            // Now add bitcast to required/original pointer type
            CastInst* pBitCast = CastInst::Create(Instruction::BitCast, pLocalAddr, pLclBuff->getType(), "", pFirstInst);
            // Advance total implicit size
            m_sInfo.stTotalImplSize += ADJUST_SIZE_TO_MAXIMUM_ALIGN(uiArraySize);

            GlobalValue::use_iterator itVal = pLclBuff->use_begin();
            // Now we need to check all uses
            while ( itVal != pLclBuff->use_end() )
            {
                Use &U = itVal.getUse();
                if (ConstantExpr *pCE = dyn_cast<ConstantExpr>(U.getUser()))
                {
                    if ( ChangeConstantExpression(pLclBuff, pCE, pBitCast, pBitCast)  )
                    {
                        // The value was completely remove
                        // Scan from the beginning
                        itVal = pLclBuff->use_begin();
                    } else
                    {
                        // The values is still in the list
                        // Continue to next item
                        ++itVal;
                    }
                    continue;
                }

                if ( !isa<Instruction>(U.getUser()) )
                {
                    ++itVal;
                    continue;
                }

                // Is instruction
                // Now we need check use in our same function
                if (dyn_cast<Instruction>(U.getUser())->getParent()->getParent() == pFunc )
                {
                    U.set(pBitCast);
                    itVal = pLclBuff->use_begin();
                }
                else
                {
                    ++itVal;
                }
            }
        }

        return true;
    }

    Value* OCLReferenceKernelUpdate::SubstituteWIcall(llvm::CallInst *pCall,
        llvm::Argument* pWorkInfo, llvm::Argument* pWGid,
        llvm::Argument* pBaseGlbId, llvm::Argument* pLocalId)
    {
        assert(pCall && "Invalid CallInst");
        Function* pFunc = pCall->getCalledFunction();
        std::string	pFuncName = pFunc->getNameStr();
        //Module* pModule = pFunc->getParent();

        Value*	pResult = NULL;	// Object that holds the resolved value

        if ( pFuncName == "get_work_dim" )
        {
            // Now retrieve address of the DIM count
            SmallVector<Value*, 4> params;
            params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
            params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
            GetElementPtrInst* pDimCntAddr =
                GetElementPtrInst::Create(pWorkInfo, params.begin(), params.end(), "", pCall);
            // Load the Value
            pResult = new LoadInst(pDimCntAddr, "", pCall);
            return pResult;
        }

        // Calculate table index of appropriate value
        int iTableInx = 0;
        iTableInx += (pFuncName != "get_global_offset") ? 0 : 1;
        iTableInx += (pFuncName != "get_global_size") ? 0 : 2;
        iTableInx += (pFuncName != "get_local_size") ? 0 : 3;
        iTableInx += (pFuncName != "get_num_groups") ? 0 : 4;
        if ( iTableInx > 0 )
        {
            // Now retrieve address of the DIM count
            SmallVector<Value*, 4> params;
            params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
            params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), iTableInx));
            params.push_back(pCall->getArgOperand(0));
            GetElementPtrInst* pSizeAddr =
                GetElementPtrInst::Create(pWorkInfo, params.begin(), params.end(), "", pCall);
            // Load the Value
            pResult = new LoadInst(pSizeAddr, "", pCall);
            return pResult;
        }

        if ( pFuncName == "get_local_id" )
        {
            SmallVector<Value*, 4> params;
            params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
            params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
            params.push_back(pCall->getArgOperand(0));
            GetElementPtrInst* pIdAddr =
                GetElementPtrInst::Create(pLocalId, params.begin(), params.end(), "", pCall);
            // Load the Value
            pResult = new LoadInst(pIdAddr, "", pCall);
            return pResult;
        }

        if ( pFuncName == "get_group_id" )
        {
            GetElementPtrInst* pIdAddr =
                GetElementPtrInst::Create(pWGid, pCall->getArgOperand(0), "", pCall);
            // Load the Value
            pResult = new LoadInst(pIdAddr, "", pCall);
            return pResult;
        }
        if ( pFuncName == "get_global_id" )
        {
            ConstantInt* pVal = dyn_cast<ConstantInt>(pCall->getArgOperand(0));
            if ( NULL != pVal )
            {
                // We have constant in the hand, we can use pre-calculated value
                unsigned int uiDim = (unsigned int)*pVal->getValue().getRawData();
                if ( NULL == m_GlbIds[uiDim] )	// Check if we already have value for this dimension
                {
                    // Create new reference for this dimension
                    m_GlbIds[uiDim] = CalcGlobalId(pCall, pBaseGlbId, pLocalId, true);
                }
                return m_GlbIds[uiDim];
            }
            // Otherwise, calculate inplace
            return CalcGlobalId(pCall, pBaseGlbId, pLocalId, false);
        }

        return pResult;
    }

    llvm::Value* OCLReferenceKernelUpdate::CalcGlobalId(llvm::CallInst *pCall, llvm::Argument* pBaseGlbId, llvm::Argument* pLocalId, bool bGlobal)
    {
        // Load local id values
        SmallVector<Value*, 4> params;
        params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
        params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
        params.push_back(pCall->getArgOperand(0));

        llvm::BasicBlock::iterator nextInst = pCall;
        if(bGlobal)
        {
            nextInst = pCall->getParent()->getParent()->getEntryBlock().begin();
        }

        GetElementPtrInst* pLclIdAddr =
            GetElementPtrInst::Create(pLocalId, params.begin(), params.end(), "", nextInst);
        // Load the value of local id
        Value* pLocalIdVal = new LoadInst(pLclIdAddr, "", nextInst);

        // Load the value of base global index
        GetElementPtrInst* pGlbBaseAddr =
            GetElementPtrInst::Create(pBaseGlbId, pCall->getArgOperand(0), "", nextInst);
        // Load the Value
        Value* pBaseGlbIdVal = new LoadInst(pGlbBaseAddr, "", nextInst);

        // Now add these two values
        Value* pGlbId = BinaryOperator::CreateAdd(pLocalIdVal, pBaseGlbIdVal, "", nextInst);

        return pGlbId;
    }

    void OCLReferenceKernelUpdate::AddCtxAddress(llvm::CallInst* pCall, llvm::Argument* pLocalId)
    {
        // Calculate address to the Context pointer
        // Load local id values
        SmallVector<Value*, 4> params;
        params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
        params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
        // It's the next address after LocalId's
        params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), MAX_WORK_DIM));

        m_pCtxPtr = GetElementPtrInst::Create(pLocalId, params.begin(), params.end(), "", pCall->getParent()->getParent()->getEntryBlock().begin());
    }

    Value* OCLReferenceKernelUpdate::UpdatePrintf(CallInst* pCall, Argument* pLocalId)
    {
        if (NULL == m_pCtxPtr) 
        {
            AddCtxAddress(pCall, pLocalId);
        }
        assert(m_pCtxPtr && "Context pointer m_pCtxPtr created as expected");

        TargetData TD(m_pModule);

        // Find out the buffer size required to store all the arguments.
        // Note: CallInst->getNumOperands() returns the number of operands in
        // the instruction, including its destination as #0. Since this is 
        // a printf call and we're interested in all the arguments after the 
        // format string, we start with #2.
        //
        assert(pCall->getNumArgOperands() > 0 && "Expect printf to have a format string");
        unsigned total_arg_size = 0;   
        for (unsigned numarg = 1; numarg < pCall->getNumArgOperands(); ++numarg)
        {
            Value* arg = pCall->getArgOperand(numarg);
            unsigned argsize = TD.getTypeSizeInBits(arg->getType()) / 8;
            total_arg_size += argsize;
        }

        // Types used in several places
        //
        const IntegerType* int32_type = IntegerType::get(*m_pLLVMContext, 32);
        const IntegerType* int8_type = IntegerType::get(*m_pLLVMContext, 8);

        // Create the alloca instruction for allocating the buffer on the stack.
        // Also, handle the special case where printf got no vararg arguments:
        // printf("hello");
        // Since we have to pass something into the 'args' argument of 
        // opencl_printf, and 'alloca' with size 0 is undefined behavior, we
        // just allocate a dummy buffer of size 1. opencl_printf won't look at 
        // it anyway.
        //
        ArrayType* buf_arr_type;
        if (pCall->getNumArgOperands() == 1)
        {
            buf_arr_type = ArrayType::get(int8_type, 1);
        }
        else
        {
            buf_arr_type = ArrayType::get(int8_type, total_arg_size);
        }
        AllocaInst* buf_alloca_inst = new AllocaInst(buf_arr_type, "temp_arg_buf", pCall);

        // Generate instructions to store the operands into the argument buffer
        //
        unsigned buf_pointer_offset = 0;
        for (unsigned numarg = 1; numarg < pCall->getNumArgOperands(); ++numarg)
        {
            vector<Value*> index_args;
            index_args.push_back(ConstantInt::get(int32_type, 0));
            index_args.push_back(ConstantInt::get(int32_type, buf_pointer_offset));

            // getelementptr to compute the address into which this argument will 
            // be placed
            //
            GetElementPtrInst* gep_instr = GetElementPtrInst::CreateInBounds(
                buf_alloca_inst, index_args.begin(), index_args.end(), "", pCall);

            Value* arg = pCall->getArgOperand(numarg);
            const Type* argtype = arg->getType();

            // bitcast from generic i8* address to a pointer to the argument's type
            //
            BitCastInst* cast_instr = new BitCastInst(gep_instr, PointerType::getUnqual(argtype), "", pCall);

            // store argument into address. Alignment forced to 1 to make vector
            // stores safe.
            //
            (void) new StoreInst(arg, cast_instr, false, 1, pCall);

            // This argument occupied some space in the buffer. 
            // Advance the buffer pointer offset by its size to know where the next
            // argument should be placed.
            // 
            unsigned argsize = TD.getTypeSizeInBits(arg->getType()) / 8;
            buf_pointer_offset += argsize;        
        }    

        // Create a pointer to the buffer, in order to pass it to the function
        //
        vector<Value*> index_args;
        index_args.push_back(ConstantInt::get(int32_type, 0));
        index_args.push_back(ConstantInt::get(int32_type, 0));

        GetElementPtrInst* ptr_to_buf = GetElementPtrInst::CreateInBounds(
            buf_alloca_inst, index_args.begin(), index_args.end(), "", pCall);

        // Finally create the call to opencl_printf
        //
        Function* pFunc = m_pModule->getFunction(builtin_printf_name);
        assert(pFunc && "Expect builtin printf to be declared before use");

        vector<Value*> params;
        params.push_back(pCall->getArgOperand(0));
        params.push_back(ptr_to_buf);
        params.push_back(m_pCtxPtr);
        Value* res = CallInst::Create(pFunc, params.begin(), params.end(), "translated_opencl_printf_call", pCall);
        return res;
    }

    Value* OCLReferenceKernelUpdate::UpdateAsyncCopy(llvm::CallInst* pCall, llvm::Argument* pLocalId, bool strided)
    {
        if ( NULL == m_pCtxPtr )
        {
            AddCtxAddress(pCall, pLocalId);
        }
        assert(m_pCtxPtr);

        // Create new call instruction with extended parameters
        SmallVector<Value*, 4> params;
        // push original parameters
        // Need bitcast to a general pointer
        CastInst* pBCDst = CastInst::Create(Instruction::BitCast, pCall->getArgOperand(0),
            PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0), "", pCall);
        params.push_back(pBCDst);
        CastInst* pBCSrc = CastInst::Create(Instruction::BitCast, pCall->getArgOperand(1),
            PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0), "", pCall);
        params.push_back(pBCSrc);
        params.push_back(pCall->getArgOperand(2));
        params.push_back(pCall->getArgOperand(3));
        if ( strided )
            params.push_back(pCall->getArgOperand(4));
        // Distinguish operator size
        const PointerType* pPTy = dyn_cast<PointerType>(pCall->getArgOperand(0)->getType());
        assert(pPTy && "Must be a pointer");
        const Type* pPT = pPTy->getElementType();
        unsigned int uiSize = pPT->getPrimitiveSizeInBits()/8;	
        if ( 0 == uiSize )
        {
            const VectorType* pVT = dyn_cast<VectorType>(pPT);
            uiSize = pVT->getBitWidth()/8;
        }
        params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext,  sizeof(size_t) * BYTE_SIZE), uiSize));
        params.push_back(m_pCtxPtr);
        assert(pPTy && "Must by a pointer type");

        Function* pNewAsyncCopy = NULL;
        if ( strided )
        {
            pNewAsyncCopy = m_pModule->getFunction(pPTy->getAddressSpace() == 3 ? "lasync_wg_copy_strided_g2l" : "lasync_wg_copy_strided_l2g");
        }
        else
        {
            pNewAsyncCopy = m_pModule->getFunction(pPTy->getAddressSpace() == 3 ? "lasync_wg_copy_g2l" : "lasync_wg_copy_l2g");
        }

        Value* res = CallInst::Create(pNewAsyncCopy, params.begin(), params.end(), "", pCall);
        return res;
    }

    void OCLReferenceKernelUpdate::UpdateWaitGroup(llvm::CallInst* pCall, llvm::Argument* pLocalId)
    {
        if ( NULL == m_pCtxPtr )
        {
            AddCtxAddress(pCall, pLocalId);
        }
        assert(m_pCtxPtr);

        // Create new call instruction with extended parameters
        SmallVector<Value*, 4> params;
        params.push_back(pCall->getArgOperand(0));
        params.push_back(pCall->getArgOperand(1));
        params.push_back(m_pCtxPtr);
        Function* pNewWait = m_pModule->getFunction("lwait_group_events");
        CallInst::Create(pNewWait, params.begin(), params.end(), "", pCall);
    }

    void OCLReferenceKernelUpdate::UpdatePrefetch(llvm::CallInst* pCall)
    {

        unsigned int uiSizeT = m_pModule->getPointerSize()*32;

        // Create new call instruction with extended parameters
        SmallVector<Value*, 4> params;
        // push original parameters
        // Need bitcast to a general pointer
        CastInst* pBCPtr = CastInst::Create(Instruction::BitCast, pCall->getArgOperand(0),
            PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0), "", pCall);
        params.push_back(pBCPtr);
        // Put number of elements
        params.push_back(pCall->getArgOperand(1));
        // Distinguish element size
        const PointerType* pPTy = dyn_cast<PointerType>(pCall->getArgOperand(0)->getType());
        assert(pPTy && "Must be a pointer");
        const Type* pPT = pPTy->getElementType();
        unsigned int uiSize = pPT->getPrimitiveSizeInBits()/8;	
        if ( 0 == uiSize )
        {
            const VectorType* pVT = dyn_cast<VectorType>(pPT);
            uiSize = pVT->getBitWidth()/8;
        }
        params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, uiSizeT), uiSize));
        Function* pPrefetch = m_pModule->getFunction("lprefetch");
        CallInst::Create(pPrefetch, params.begin(), params.end(), "", pCall);
    }

    BasicBlock::iterator removeInstruction(BasicBlock* pBB, BasicBlock::iterator it)
    {
        BasicBlock::InstListType::iterator prev;

        if ( pBB->begin() == it)
        {
            prev = pBB->end();
        }
        else
        {
            prev = it;
            --prev;
        }

        Instruction* pInst = it;
        pInst->removeFromParent();
        delete pInst;

        if ( pBB->end() == prev)
        {
            return pBB->begin();
        }

        return ++prev;
    }

    int CalculateKernelLocalsSize(Function *pFunc)
    {
        Module *pModule = pFunc->getParent();

        int iLocalsSize = 0;

        // Extract pointer to module annotations
        NamedMDNode *OpenCLMetadata = pModule->getNamedMetadata("opencl.kernels");

        if ( NULL == OpenCLMetadata )
        {
            return -1;
        }

        // now we look for this kernel in the annotations
        llvm::MDNode *pFuncElt = NULL;
        for (unsigned i = 0, e = OpenCLMetadata->getNumOperands(); i != e; ++i) 
        {
            // Obtain kernel function from annotation
            llvm::MDNode *elt = OpenCLMetadata->getOperand(i);
            Function *pFuncVal = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
            if ( NULL == pFuncVal )
            {
                continue;	// Not a function pointer
            }

            if ( pFuncVal == pFunc )
            {
                break;
            }
        }

        if( NULL != pFuncElt )
        {
            //Function is in the annotation - it's a kernel
            // Obtain local variables array
            NamedMDNode *localsAnchor = pModule->getNamedMetadata(dyn_cast<llvm::MDString>(pFuncElt->getOperand(5))->getString());

            if( NULL != localsAnchor)
            {
                // Iterate through local buffers
                for (unsigned i = 0, e = localsAnchor->getNumOperands(); i != e; ++i) 
                {
                    // Obtain local from annotation
                    MDNode *operand = localsAnchor->getOperand(i);
                    GlobalValue *pLclBuff = pModule->getGlobalVariable(
                        dyn_cast<llvm::MDString>(operand->getOperand(0))->getString(), true);

                    if(NULL == pLclBuff) continue;

                    // Calculate required buffer size
                    const ArrayType *pArray = dyn_cast<ArrayType>(pLclBuff->getType()->getElementType());
                    unsigned int uiArraySize = pArray ? 1 : pLclBuff->getType()->getElementType()->getPrimitiveSizeInBits()/8;
                    assert ( 0 != uiArraySize );
                    while ( NULL != pArray )
                    {
                        uiArraySize *= (unsigned int)pArray->getNumElements();
                        if ( pArray->getContainedType(0)->getTypeID() != Type::ArrayTyID )
                        {
                            uiArraySize *= pArray->getContainedType(0)->getPrimitiveSizeInBits()/8;
                        }
                        pArray = dyn_cast<ArrayType>(pArray->getContainedType(0));
                    }

                    // Advance total implicit size
                    iLocalsSize += ADJUST_SIZE_TO_DCU_LINE(uiArraySize);
                }
            }
        }

        //look for calls to other kernels
        Function::BasicBlockListType::iterator bb_it = pFunc->getBasicBlockList().begin();
        while ( bb_it != pFunc->getBasicBlockList().end() )
        {
            BasicBlock::iterator inst_it = bb_it->begin();
            while ( inst_it != bb_it->end() )
            {
                CallInst* pCall = dyn_cast<CallInst>(inst_it);
                if( NULL == pCall )
                {
                    //it's not a call instruction
                    ++inst_it;
                    continue;
                }

                int iCallLocalSize = CalculateKernelLocalsSize(pCall->getCalledFunction());

                if(iCallLocalSize < 0)
                {
                    return -1;
                }

                iLocalsSize += iCallLocalSize;

                ++inst_it;
            }
            ++bb_it;
        }

        return iLocalsSize;
    }

    bool OCLReferenceKernelUpdate::FunctionNeedsUpdate(Function *pFunc)
    {
        //look for calls to other functions...
        Function::BasicBlockListType::iterator bb_it = pFunc->getBasicBlockList().begin();
        while ( bb_it != pFunc->getBasicBlockList().end() )
        {
            BasicBlock::iterator inst_it = bb_it->begin();
            while ( inst_it != bb_it->end() )
            {
                CallInst* pCall = dyn_cast<CallInst>(inst_it);
                if( NULL == pCall )
                {
                    //it's not a call instruction
                    ++inst_it;
                    continue;
                }

                Function *pCallee = pCall->getCalledFunction();

                // It's a call
                // check for builtin functions that need KernelUpdate
                if ( (("get_" == pCallee->getNameStr().substr(0,4)) &&
                    (("get_work_dim" == pCallee->getNameStr().substr(0,12)) ||
                    ("get_global_size" == pCallee->getNameStr()) ||
                    ("get_local_size" == pCallee->getNameStr().substr(0,14)) ||
                    ("get_num_groups" == pCallee->getNameStr().substr(0,14)) ||
                    ("get_local_id" == pCallee->getNameStr().substr(0,12)) ||
                    ("get_group_id" == pCallee->getNameStr().substr(0,12)) ||
                    ("get_global_id" == pCallee->getNameStr().substr(0,13))) ) ||
                    ("barrier" == pCallee->getNameStr()) ||
                    ("printf" == pCallee->getNameStr()) ||
                    ("_Z21async_work_group_copy" == pCallee->getNameStr().substr(0,25)) ||
                    ("_Z17wait_group_events" == pCallee->getNameStr().substr(0,21)) ||
                    ("_Z8prefetch" == pCallee->getNameStr().substr(0,11)) )
                {
                    return true;
                }

                std::vector<Function *>::iterator iter = m_nonInlinedFunctions.begin();
                while( (iter != m_nonInlinedFunctions.end()) && (*iter != pCallee) ) iter++;

                if(iter != m_nonInlinedFunctions.end())
                {
                    return true;
                }

                if( IsAKernel(pCallee) || FunctionNeedsUpdate(pCallee) )
                {
                    return true;
                }

                ++inst_it;
            }
            ++bb_it;
        }

        return false;
    }

    Function *OCLReferenceKernelUpdate::RunOnKernel(Function *pFunc, NamedMDNode *localsAnchor)
    {
        std::vector<const llvm::Type *> newArgsVec;

        Function::ArgumentListType::iterator argIt = pFunc->getArgumentList().begin();
        while(argIt != pFunc->getArgumentList().end())
        {
            newArgsVec.push_back(argIt->getType());
            argIt++;
        }

        unsigned int uiSizeT = m_pModule->getPointerSize()*32;

        newArgsVec.push_back(PointerType::get(m_pModule->getTypeByName("struct.WorkDim"), 0));
        newArgsVec.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
        newArgsVec.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
        newArgsVec.push_back(PointerType::get(m_pModule->getTypeByName("struct.LocalId"), 0));

        FunctionType *FTy = FunctionType::get( pFunc->getReturnType(),newArgsVec, false);

        Function *NewF = Function::Create(FTy, pFunc->getLinkage(), pFunc->getName());
        NewF->setCallingConv(CallingConv::C);

        ValueMap<const Value*, Value*> ValueMap;

        Function::arg_iterator DestI = NewF->arg_begin();
        for (Function::const_arg_iterator I = pFunc->arg_begin(), E = pFunc->arg_end(); I != E; ++I, ++DestI)
        {
            DestI->setName(I->getName());
            ValueMap[I] = DestI;
        }

        DestI->setName("pWorkDim");
        Argument *pWorkDim = DestI;
        ++DestI;
        DestI->setName("pWGId");
        Argument *pWGId = DestI;
        ++DestI;
        DestI->setName("pBaseGlbId");
        Argument *pBaseGlbId = DestI;
        ++DestI;
        DestI->setName("LocalIds");
        Argument *pLocalId = DestI;
        pLocalId->addAttr(llvm::Attribute::ByVal);

        SmallVector<ReturnInst*, 8> Returns;
        CloneFunctionInto(NewF, pFunc, ValueMap, true, Returns, "", NULL);

        // Initialize kernel related variables
        memset(&m_sInfo, 0, sizeof(TLLVMKernelInfo));
        memset(m_GlbIds, 0, sizeof(m_GlbIds));
        m_pCtxPtr = NULL;

        // Apple LLVM-IR workaround
        // 1.	Pass WI information structure as the next parameter after given function parameters
        // 2.	We don't want to use TLS for local memory.
        //		Our solution to move all internal local memory blocks to be allocated
        //		by the execution engine and passed within additional parameters to the kernel,
        //		those parameters are not exposed to the user

        std::map<Function *, int> LocalsMap;

        // Go through function blocks
        Function::BasicBlockListType::iterator bb_it = NewF->getBasicBlockList().begin();
        while ( bb_it != NewF->getBasicBlockList().end() )
        {
            BasicBlock::iterator inst_it = bb_it->begin();
            //llvm::Value* pArgVal = NULL;
            while ( inst_it != bb_it->end() )
            {
                //bool bAddWIInfo = false;

                switch (inst_it->getOpcode())
                {
                    // Call instruction
                case Instruction::Call:
                    {
                        CallInst* pCall = dyn_cast<CallInst>(inst_it);
                        std::string calledFuncName = pCall->getCalledFunction()->getNameStr();

                        // Recognize WI info functions
                        if ( "get_" == calledFuncName.substr(0, 4))
                        {
                            // Substitute extern operand with function parameter
                            Value *pNewRes = SubstituteWIcall(pCall, pWorkDim, pWGId, pBaseGlbId, pLocalId);
                            if ( NULL != pNewRes)
                            {
                                pCall->uncheckedReplaceAllUsesWith(pNewRes);
                                inst_it = removeInstruction(bb_it, inst_it);
                            } else
                            {
                                ++inst_it;
                            }
                            break;
                        }
                        // Check call for not inlined functions/ kernels
                        Function* pCallee = pCall->getCalledFunction();
                        // check for external functions, and make sure they exist
                        if (( NULL != pCallee) && ( pCallee->isDeclaration() ) && pCallee->getNameStr().compare(0, 4, "llvm"))
                        {
                            void *Ptr = llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(pCallee->getNameStr());
                            if( NULL == Ptr )
                            {
                                // report error
                                m_pUndefinedExternalFunctions->push_back(pCallee->getNameStr() + " in function " + NewF->getNameStr());
                            }
                            ++inst_it;
                            break;
                        }

                        // Check call for not inlined functions/ kernels
                        if ( NULL != pCallee && !pCallee->isDeclaration())
                        {
                            if ( !IsAKernel(pCallee) )
                            {
                                if(!FunctionNeedsUpdate(pCallee))
                                {
                                    ++inst_it;
                                    break;
                                }

                                std::vector<Function *>::iterator iter = m_nonInlinedFunctions.begin();
                                while( (iter != m_nonInlinedFunctions.end()) && (*iter != pCallee) ) iter++;

                                if(iter == m_nonInlinedFunctions.end())
                                {
                                    m_nonInlinedFunctions.push_back(pCallee);
                                }
                            }

                            int iKernelLocalSize = CalculateKernelLocalsSize(pCallee);
                            if(iKernelLocalSize < 0)
                            {
                                return false;
                            }

                            if(LocalsMap.find(pCallee) == LocalsMap.end())
                            {
                                //Kernel is not in the map yet
                                LocalsMap[pCallee] = m_sInfo.stTotalImplSize;
                                m_sInfo.stTotalImplSize += iKernelLocalSize;
                            }

                            Value **pCallArgs = new Value*[4];
                            pCallArgs[0] = pWorkDim;
                            pCallArgs[1] = pWGId;
                            pCallArgs[2] = pBaseGlbId;
                            pCallArgs[3] = pLocalId;

                            m_fixupCalls[pCall] = pCallArgs;

                            //m_sInfo.bCallKernel = true;
                        }
                        ++inst_it;
                        break;
                    }
                default:
                    ++inst_it;
                    break;
                }
            }
            ++bb_it;
        }

        m_mapKernelInfo[NewF] = m_sInfo;

        pFunc->uncheckedReplaceAllUsesWith(NewF);
        pFunc->setName("__" + pFunc->getName() + "_original");
        m_pModule->getFunctionList().push_back(NewF);

        pFunc->deleteBody();

        return NewF;
    }

    // ------------------------------------------------------------------------------
    void OCLReferenceKernelUpdate::AddWIInfoDeclarations()
    {
        // Detect size_t size
        unsigned int uiSizeT = m_pModule->getPointerSize()*32;
        /*
        struct sLocalId 
        {
        size_t	Id[MAX_WORK_DIM];
        };
        */
        // Create Work Group/Work Item info structures
        std::vector<const Type*> members;
        members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), MAX_WORK_DIM)); // Local Id's
        StructType* pLocalId = StructType::get(*m_pLLVMContext, members, true);
        m_pModule->addTypeName("struct.LocalId", pLocalId);

        /*
        struct sWorkInfo
        {
        unsigned int	uiWorkDim;
        size_t			GlobalOffset[MAX_WORK_DIM];
        size_t			GlobalSize[MAX_WORK_DIM];
        size_t			LocalSize[MAX_WORK_DIM];
        size_t			WGNumber[MAX_WORK_DIM];
        };
        */
        members.clear();
        members.push_back(IntegerType::get(*m_pLLVMContext, 32));
        members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), MAX_WORK_DIM)); // Global offset
        members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), MAX_WORK_DIM)); // Global size
        members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), MAX_WORK_DIM)); // WG size/Local size
        members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), MAX_WORK_DIM)); // Number of groups
        StructType* pWorkDimType = StructType::get(*m_pLLVMContext, members, false);
        m_pModule->addTypeName("struct.WorkDim", pWorkDimType);
    }

    void OCLReferenceKernelUpdate::AddAsyncCopyDeclaration()
    {
        if ( m_bAsyncCopyDecl )
            return;

        unsigned int uiSizeT = m_pModule->getPointerSize()*32;

        //event_t async_work_group_copy(void* pDst, void* pSrc, size_t numElem, event_t event,
        //							   size_t elemSize, LLVMExecMultipleWIWithBarrier* *ppExec);
        std::vector<const Type*> params;
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
        params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
        params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
        params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
        FunctionType* pNewType = FunctionType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), params, false);
        Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_l2g", m_pModule);
        Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_g2l", m_pModule);

        //event_t async_work_group_strided_copy(void* pDst, void* pSrc, size_t numElem, size_t stride, event_t event,
        //							   size_t elemSize, LLVMExecMultipleWIWithBarrier* *ppExec);
        params.clear();
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
        params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
        params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
        params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
        params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
        pNewType = FunctionType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), params, false);
        Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_strided_l2g", m_pModule);
        Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_strided_g2l", m_pModule);


        // void wait_group_events(int num_events, event_t event_list)
        params.clear();
        params.push_back(IntegerType::get(*m_pLLVMContext, 32));
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
        FunctionType* pWaitType = FunctionType::get(Type::getVoidTy(*m_pLLVMContext), params, false);
        Function::Create(pWaitType, (GlobalValue::LinkageTypes)0, "lwait_group_events", m_pModule);

        m_bAsyncCopyDecl = true;
    }

    void OCLReferenceKernelUpdate::AddPrintfDeclaration()
    {
        if (m_bPrintfDecl)
            return;

        // The prototype of opencl_printf is:
        // int opencl_printf(char* format, char* args, LLVMExecutable** ppExec)
        //
        vector<const Type*> params;
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, sizeof(size_t) * BYTE_SIZE), 0));

        FunctionType* pNewType = FunctionType::get(Type::getInt32Ty(*m_pLLVMContext), params, false);
        Function::Create(pNewType, Function::ExternalLinkage, builtin_printf_name, m_pModule);

        m_bPrintfDecl = true;
    }

    void OCLReferenceKernelUpdate::AddPrefetchDeclaration()
    {
        if ( m_bPrefetchDecl )
            return;

        unsigned int uiSizeT = m_pModule->getPointerSize()*32;

        std::vector<const Type*> params;
        // Source Pointer
        params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
        // Number of elements
        params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
        // Element size
        params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
        FunctionType* pNewType = FunctionType::get(Type::getVoidTy(*m_pLLVMContext), params, false);
        Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lprefetch", m_pModule);
        m_bPrefetchDecl = true;
    }

    bool OCLReferenceKernelUpdate::IsAKernel(const Function* pFunc)
    {
        for (unsigned i = 0, e = m_pOpenCLMetadata->getNumOperands(); i != e; ++i) 
        {
            // Obtain kernel function from annotation
            llvm::MDNode *elt = m_pOpenCLMetadata->getOperand(i);
            Function *pFuncVal = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
            if ( NULL == pFuncVal )
            {
                continue;	// Not a function pointer
            }

            if ( pFuncVal == pFunc )
            {
                return true;
            }
        }

        // Function not found
        return false;
    }


}
