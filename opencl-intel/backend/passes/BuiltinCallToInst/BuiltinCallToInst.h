/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __SHUFFLE_CALL_TO_INST_H__
#define __SHUFFLE_CALL_TO_INST_H__

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

#include <vector>
#include <utility>

namespace intel{
    using namespace llvm;

    /// @brief BuiltinCallToInst class
    ///        In OpenCL:
    ///         - gentypen shuffle (gentypem x, ugentypen mask)
    ///         - gentypen shuffle2 (gentypem x, gentypem y, ugentypen mask)
    ///        Clang translates shuffle and shuffle2 functions calls to LLVM function calls with
    ///        specific types. Example: %call = call <8 x i32> @_Z7shuffleDv4_iDv8_j( ... )
    ///        In case the mask argument is a vector of constants, BuiltinCallToInst translates the
    ///        shuffle call to shufflevector instruction in LLVM, to gain some performance boost.
    ///        Example:
    ///        From this:
    ///         %tmp = call <8 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32> %x, <4 x i32> <i32 3, i32 2, i32 1, i32 0>)
    ///        To this:
    ///         %tmp = shufflevector <4 x i32> %x, <4 x i32> undef, <4 x i32> <i32 3, i32 2, i32 1, i32 0>
    class BuiltinCallToInst : public FunctionPass
    {
    public:
        /// @brief Pass identification, replacement for typeid
        static char ID;

        /// @brief Constructor
        BuiltinCallToInst() : FunctionPass(ID) {}

        /// @brief Provides name of pass
        virtual llvm::StringRef getPassName() const {
            return "BuiltinCallToInst";
        }

        /// @brief  LLVM Function pass entry
        /// @param  F  Function to transform
        /// @return true if changed
        virtual bool runOnFunction(Function &F);

    private:
        /// @brief built-in function type
        typedef enum {
            SHUFFLE1,
            SHUFFLE2,
            REL_IS_LESS,
            REL_IS_LESS_EQUAL,
            REL_IS_GREATER,
            REL_IS_GREATER_EQUAL,
            REL_IS_EQUAL,
            REL_IS_NOT_EQUAL,
            NOT_SUPPORTED
        } BuiltinType;

        /// @brief  Check if given called function is a supported built-in
        /// @return BuiltinType, can be NOT_SUPPORTED type.
        BuiltinType isSupportedBuiltin(CallInst* pCall);

        /// @brief Find all built-in calls in current function to handle
        void findBuiltinCallsToHandle(Function &F);

        /// @brief Handle all supported built-in calls in current function
        /// @return true if LLVM IR had changed.
        bool handleSupportedBuiltinCalls();

        /// @brief Handle all shuffle calls in current function
        /// @param shuffleCall call instruction to shuffle built-in
        /// @param shuffleType type of the called shuffle built-in
        void handleShuffleCalls(CallInst* shuffleCall, BuiltinType shuffleType);

        /// @brief Handle all relational calls in current function
        /// @param relationalCall call instruction to relational built-in
        /// @param relationalType type of the called relational built-in
        void handleRelationalCalls(CallInst* relationalCall, BuiltinType relationalType);

        /// @brief Shuffle and Shuffle2 arguments positions
        static const unsigned int SHUFFLE_VEC1_POS = 0;
        static const unsigned int SHUFFLE_MASK_POS = 1;
        static const unsigned int SHUFFLE2_VEC1_POS = 0;
        static const unsigned int SHUFFLE2_VEC2_POS = 1;
        static const unsigned int SHUFFLE2_MASK_POS = 2;

        /// @brief A vector holding all supported built-in calls with their type
        std::vector< std::pair<CallInst*, BuiltinType> > m_builtinCalls;
    };

} // namespace intel
#endif // __SHUFFLE_CALL_TO_INST_H__
