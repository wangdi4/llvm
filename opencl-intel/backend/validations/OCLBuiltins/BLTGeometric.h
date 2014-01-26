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

File Name:  BLTGeometric.h

\*****************************************************************************/
#ifndef BLT_GEOMETRIC_H
#define BLT_GEOMETRIC_H

#include <vector>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Helpers.h"
#include "RefALU.h"

namespace Validation {
namespace OCLBuiltins {

    template<typename T, int n>
    llvm::GenericValue lle_X_dot(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];

        T a[n], b[n];
        for (uint32_t i = 0; i < n; ++i)
        {
            a[i] = getVal<T,n>(arg0, i);
            b[i] = getVal<T,n>(arg1, i);
        }

        T dot = RefALU::dot<T>(a, b, n);
        
        getRef<T>(R) = dot;
        return R;
    }

    template<typename T, int n>
    llvm::GenericValue lle_X_normalize(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        T a[n], out[n];
        for (uint32_t i = 0; i < n; ++i)
            a[i] = getVal<T,n>(arg0, i);

        RefALU::normalize(a, out, n);
        
        for (uint32_t i = 0; i < n; ++i)
            getRef<T,n>(R, i) = out[i];

        return R;
    }

    template<typename T, int n>
    llvm::GenericValue lle_X_length(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        T a[n];
        for (uint32_t i = 0; i < n; ++i)
            a[i] = getVal<T,n>(arg0, i);

        getRef<T>(R) = RefALU::length(a, n);
        return R;
    }


    template<typename T, int n>
    llvm::GenericValue lle_X_distance(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T a[n], b[n];
        for (uint32_t i = 0; i < n; ++i)
        {
            a[i] = getVal<T,n>(arg0, i);
            b[i] = getVal<T,n>(arg1, i);
        }
        getRef<T>(R) = RefALU::distance(a, b, n);
        return R;
    }


    template<typename T, uint32_t n>
    llvm::GenericValue lle_X_cross(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];

        // outVector[ 0 ] = ( vecA[ 1 ] * vecB[ 2 ] ) - ( vecA[ 2 ] * vecB[ 1 ] );
        // outVector[ 1 ] = ( vecA[ 2 ] * vecB[ 0 ] ) - ( vecA[ 0 ] * vecB[ 2 ] );
		// outVector[ 2 ] = ( vecA[ 0 ] * vecB[ 1 ] ) - ( vecA[ 1 ] * vecB[ 0 ] );
        T a[n], b[n], out[n];

        for (uint32_t i = 0; i < n; ++i)
        {
            a[i] = getVal<T,n>(arg0, i);
            b[i] = getVal<T,n>(arg1, i);
        }
        
        RefALU::cross(a, b, out, n);

        for (uint32_t i = 0; i < n; ++i)
        {
            getRef<T,n>(R,i) = out[i];
        }

        return R;
    }

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_GEOMETRIC_H
