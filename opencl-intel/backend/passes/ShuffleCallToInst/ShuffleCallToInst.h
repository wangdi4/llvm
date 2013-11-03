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

    /// @brief ShuffleCallToInst class
    ///        In OpenCL:
    ///         - gentypen shuffle (gentypem x, ugentypen mask)
    ///         - gentypen shuffle2 (gentypem x, gentypem y, ugentypen mask)
    ///        Clang translates shuffle and shuffle2 functions calls to LLVM function calls with
    ///        specific types. Example: %call = call <8 x i32> @_Z7shuffleDv4_iDv8_j( ... )
    ///        In case the mask argument is a vector of constants, ShuffleCallToInst translates the
    ///        shuffle call to shufflevector instruction in LLVM, to gain some performance boost.
    ///        Example:
    ///        From this:
    ///         %tmp = call <8 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32> %x, <4 x i32> <i32 3, i32 2, i32 1, i32 0>)
    ///        To this:
    ///         %tmp = shufflevector <4 x i32> %x, <4 x i32> undef, <4 x i32> <i32 3, i32 2, i32 1, i32 0>
    class ShuffleCallToInst : public FunctionPass
    {
    public:
        /// @brief Pass identification, replacement for typeid
        static char ID;

        /// @brief Constructor
        ShuffleCallToInst() : FunctionPass(ID) {}

        /// @brief Provides name of pass
        virtual const char *getPassName() const {
            return "ShuffleCallToInst";
        }

        /// @brief  LLVM Function pass entry
        /// @param  F  Function to transform
        /// @return true if changed
        virtual bool runOnFunction(Function &F);

    private:
        /// @brief Shuffle function type
        typedef enum {
            SHUFFLE1,
            SHUFFLE2,
            NOT_SHUFFLE
        } ShuffleType;

        /// @brief  Check if given called function is shuffle with constant mask
        /// @return SHUFFLE1 or SHUFFLE2 in case of a shuffle function
        ///         else UNKNOWN
        ShuffleType isConstShuffle(CallInst* pCall);

        /// @brief Find all shuffle calls in current function
        void findShuffleCalls(Function &F);

        /// @brief Handle all shuffle calls in current function
        bool handleShuffleCalls();

        /// @brief Shuffle and Shuffle2 arguments positions
        static const unsigned int SHUFFLE_VEC1_POS = 0;
        static const unsigned int SHUFFLE_MASK_POS = 1;
        static const unsigned int SHUFFLE2_VEC1_POS = 0;
        static const unsigned int SHUFFLE2_VEC2_POS = 1;
        static const unsigned int SHUFFLE2_MASK_POS = 2;

        /// @brief A vector holding all shuffle calls with their type
        std::vector< std::pair<CallInst*, ShuffleType> > m_shuffleCalls;
    };

} // namespace intel
#endif // __SHUFFLE_CALL_TO_INST_H__
