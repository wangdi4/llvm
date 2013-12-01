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

File Name:  BLTMiscellaneousVector.h

\*****************************************************************************/
#ifndef BLT_MISCELLANEOUSVECTOR_H
#define BLT_MISCELLANEOUSVECTOR_H

#include <vector>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Helpers.h"
#include "RefALU.h"
#include "Utils.h"

namespace Validation {
namespace OCLBuiltins {

    template<typename T1, typename T2, int n>
    llvm::GenericValue lle_X_shuffle(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;        
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];

        R.AggregateVal.resize(n);

        int m = (int)arg0.AggregateVal.size();
        // For shuffle, only the ilogb(2m-1) least significant bits of each mask 
        // element are considered. Other bits in the mask shall be ignored.

        unsigned mask = (1 << shuffleGetNumMaskBits(m)) - 1;

        for( uint32_t i=0; i<n; i++) {
            unsigned j = (unsigned)getVal<T2,n>(arg1, i) & mask;
            getRef<T1,n>(R, i) = getRef<T1,n>(arg0, j);
        }

        return R;
    }

    template<typename T1, typename T2, int n>
    llvm::GenericValue lle_X_shuffle2(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        llvm::GenericValue arg2 = Args[2];

        R.AggregateVal.resize(n);

        int m = (int)arg0.AggregateVal.size();
        assert(m == (int)arg1.AggregateVal.size() && "both arg0 and arg1 vectors should have the same size");
        // For shuffle2, only the ilogb(2m-1)+1 least significant bits of each mask 
        // element are considered. Other bits in the mask shall be ignored.

        unsigned mask = (1 << (shuffleGetNumMaskBits(m) + 1)) - 1;

        for( uint32_t i=0; i<n; i++) {
            unsigned j = (unsigned)getVal<T2,n>(arg2, i) & mask;
            if(j < arg0.AggregateVal.size()) {
                getRef<T1,n>(R, i) = getRef<T1,n>(arg0, j);
            } else {
                getRef<T1,n>(R, i) = getRef<T1,n>(arg1,j-arg0.AggregateVal.size());
            }
        }

        return R;
    }

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_MISCELLANEOUSVECTOR_H
