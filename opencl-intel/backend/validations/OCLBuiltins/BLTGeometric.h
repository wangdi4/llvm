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
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Helpers.h"
#include "IBLTMapFiller.h"
#include "RefALU.h"

namespace Validation {
namespace OCLBuiltins {

    // This class adds references to the implementations of OpenCL built-in functions from 6.11.5 section.
    class GeometricMapFiller : public IBLTMapFiller
    {
    public:
        void addOpenCLBuiltins(std::map<std::string, PBLTFunc>& funcNames);
    };

    template<typename T, int n>
    llvm::GenericValue lle_X_dot(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T dot = T(0);
        for (uint32_t i = 0; i < n; ++i)
        {
            dot += getVal<T,n>(arg0, i) * getVal<T,n>(arg1, i);
        }
        getRef<T>(R) = dot;
        return R;
    }

    template<typename T, int n>
    llvm::GenericValue lle_X_normalize(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        T len = T(0);
        for (uint32_t i = 0; i < n; ++i)
        {
            len = RefALU::add(len, RefALU::mul(getVal<T,n>(arg0, i), getVal<T,n>(arg0, i)));
        }
        T norm = RefALU::rsqrt(len);
        for (uint32_t i = 0; i < n; ++i)
        {
            getRef<T,n>(R,i) = RefALU::div(getVal<T,n>(R,i), norm);
        }
        return R;
    }

    template<typename T, int n>
    llvm::GenericValue lle_X_length(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        T len = T(0);
        for (uint32_t i = 0; i < n; ++i)
        {
            len += RefALU::add(len, RefALU::mul(getVal<T,n>(arg0, i), getVal<T,n>(arg0, i)));
        }
        getRef<T>(R) = RefALU::sqrt(len);
        return R;
    }


    template<typename T, int n>
    llvm::GenericValue lle_X_distance(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T len = T(0);
        for (uint32_t i = 0; i < n; ++i)
        {
            T d = RefALU::sub(getVal<T,n>(arg0, i), getVal<T,n>(arg1, i));
            len += RefALU::add(len, RefALU::mul(d,d));
        }
        getRef<T>(R) = RefALU::sqrt(len);
        return R;
    }


    template<typename T, uint32_t n>
    llvm::GenericValue lle_X_cross(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];

        // outVector[ 0 ] = ( vecA[ 1 ] * vecB[ 2 ] ) - ( vecA[ 2 ] * vecB[ 1 ] );
        // outVector[ 1 ] = ( vecA[ 2 ] * vecB[ 0 ] ) - ( vecA[ 0 ] * vecB[ 2 ] );
		// outVector[ 2 ] = ( vecA[ 0 ] * vecB[ 1 ] ) - ( vecA[ 1 ] * vecB[ 0 ] );

        getRef<T,n>(R,0) = ( getVal<T,n>(arg0, 1) * getVal<T,n>(arg1, 2) ) - ( getVal<T,n>(arg0, 2) * getVal<T,n>(arg1, 1) );
		getRef<T,n>(R,1) = ( getVal<T,n>(arg0, 2) * getVal<T,n>(arg1, 0) ) - ( getVal<T,n>(arg0, 0) * getVal<T,n>(arg1, 2) );
        getRef<T,n>(R,2) = ( getVal<T,n>(arg0, 0) * getVal<T,n>(arg1, 1) ) - ( getVal<T,n>(arg0, 1) * getVal<T,n>(arg1, 0) );

        if( n == 4)
            getRef<T,n>(R,2) = T(0);

        return R;
    }

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_GEOMETRIC_H
