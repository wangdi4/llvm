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

File Name:  BLTRelational.h

\*****************************************************************************/
#ifndef BLT_RELATIONAL_H
#define BLT_RELATIONAL_H

#include <vector>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Helpers.h"
#include "IBLTMapFiller.h"
#include "RefALU.h"

namespace Validation {
namespace OCLBuiltins {

    // This class adds references to the implementations of OpenCL built-in functions from 6.2 section.
    class RelationalMapFiller : public IBLTMapFiller
    {
    public:
        void addOpenCLBuiltins(std::map<std::string, PBLTFunc>& funcNames);
    };

    template<typename T>
    llvm::GenericValue lle_X_isinf(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        getRef<int32_t>(R) = llvm::APInt(32, RefALU::isInf<T>(getVal<T>(arg0)), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isinf(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, RefALU::isInf<T>(getVal<T,n>(arg0, i)), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isnormal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        getRef<int32_t>(R) = llvm::APInt(32, RefALU::isNormal<T>(getVal<T>(arg0)), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isnormal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, RefALU::isNormal<T>(getVal<T,n>(arg0, i)), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isnan(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        getRef<int32_t>(R) = llvm::APInt(32, RefALU::isNan<T>(getVal<T>(arg0)), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isnan(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, RefALU::isNan<T>(getVal<T,n>(arg0, i)), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isgreater(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        getRef<int32_t>(R) = llvm::APInt(32, getVal<T>(arg0) > getVal<T>(arg1), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isgreater(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];

        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, getVal<T,n>(arg0, i) > getVal<T,n>(arg1,i), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isequal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        getRef<int32_t>(R) = llvm::APInt(32, getVal<T>(arg0) == getVal<T>(arg1), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isequal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, getVal<T,n>(arg0, i) == getVal<T,n>(arg1,i), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isgreaterequal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        getRef<int32_t>(R) = llvm::APInt(32, getVal<T>(arg0) >= getVal<T>(arg1), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isgreaterequal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, getVal<T,n>(arg0, i) >= getVal<T,n>(arg1,i), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isless(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        getRef<int32_t>(R) = llvm::APInt(32, getVal<T>(arg0) < getVal<T>(arg1), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isless(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, getVal<T,n>(arg0, i) < getVal<T,n>(arg1,i), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_islessequal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        getRef<int32_t>(R) = llvm::APInt(32, getVal<T>(arg0) <= getVal<T>(arg1), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_islessequal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, getVal<T,n>(arg0, i) <= getVal<T,n>(arg1,i), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_islessgreater(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        getRef<int32_t>(R) = llvm::APInt(32, ((getVal<T>(arg0) < getVal<T>(arg1)) || (getVal<T>(arg0) > getVal<T>(arg1))), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_islessgreater(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) =  -llvm::APInt(32, ((getVal<T,n>(arg0, i) < getVal<T,n>(arg1,i)) || (getVal<T,n>(arg0, i) > getVal<T,n>(arg1,i))), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isnotequal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        getRef<int32_t>(R) = llvm::APInt(32, getVal<T>(arg0) != getVal<T>(arg1), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isnotequal(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        for (int32_t i = 0; i < n; ++i)
        {
           getRef<int32_t,n>(R,i) = -llvm::APInt(32, getVal<T,n>(arg0, i) != getVal<T,n>(arg1,i), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isunordered(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        getRef<int32_t>(R) = llvm::APInt(32, (RefALU::isNan<T>(getVal<T>(arg0)) || RefALU::isNan<T>(getVal<T>(arg1))), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isunordered(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, (RefALU::isNan<T>(getVal<T,n>(arg0, i)) || RefALU::isNan<T>(getVal<T,n>(arg1, i))), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isordered(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        getRef<int32_t>(R) = llvm::APInt(32, (getVal<T>(arg0) == getVal<T>(arg0) && getVal<T>(arg1) == getVal<T>(arg1)), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isordered(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = -llvm::APInt(32, (getVal<T,n>(arg0, i) == getVal<T,n>(arg0,i) && getVal<T,n>(arg1, i) == getVal<T,n>(arg1,i)), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_isfinite(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        getRef<int32_t>(R) = llvm::APInt(32, (!RefALU::isInf<T>(getVal<T>(arg0)) && !RefALU::isNan<T>(getVal<T>(arg0))), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_isfinite(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = llvm::APInt(32, -(!RefALU::isInf<T>(getVal<T,n>(arg0, i)) && !RefALU::isNan<T>(getVal<T,n>(arg0, i))), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_signbit(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        getRef<int32_t>(R) = llvm::APInt(32, RefALU::signbit<T>(getVal<T>(arg0)), true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_signbit(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        for (int32_t i = 0; i < n; ++i)
        {
            getRef<int32_t,n>(R,i) = llvm::APInt(32, (-RefALU::signbit<T>(getVal<T,n>(arg0, i))), true);
        }
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_any(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        T sum = 0;
        sum |= getVal<T>(arg0); 
        sum = (sum != 0) ? 1 : 0;
        getRef<int32_t>(R) = llvm::APInt(32, sum != 0, true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_any(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        typedef typename signedT<T>::type sT;
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        T sum = 0;
        T mask = intMin<sT>(); // we need 1 in highest bit here
        for (int32_t i = 0; i < n; ++i)
        {
            sum |= getVal<T,n>(arg0, i) & mask;
        }

        getRef<int32_t>(R) = llvm::APInt(32, (sum != 0) ? 1 : 0, true);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_all(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        typedef typename signedT<T>::type sT;
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        T mask = intMin<sT>();
        T sum = mask; // we need 1 in highest bit here
        sum &= getVal<T>(arg0); 
        sum = (sum != 0) ? 1 : 0;
        getRef<int32_t>(R) = llvm::APInt(32, sum != 0, true);
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_all(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        typedef typename signedT<T>::type sT;
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        T mask = intMin<sT>();  // we need 1 in highest bit here
        T sum = mask;
        for (int32_t i = 0; i < n; ++i)
        {
            sum &= getVal<T,n>(arg0, i) & mask;
        }

        getRef<int32_t>(R) = llvm::APInt(32, (sum != 0) ? 1 : 0, true);

        return R;
    }

    template<typename T>
    llvm::GenericValue localBitselect( T inA, T inB, T inC )
    {
        llvm::GenericValue R;
        T out = ( inA & ~inC ) | ( inB & inC );
        getRef<T>(R) = llvm::APInt(sizeof(T)*8, out, isSignedType<T>());
        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_bitselect(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        llvm::GenericValue arg2 = Args[2];

        R = localBitselect<T>(  getVal<T>(arg0),  getVal<T>(arg1),  getVal<T>(arg2) );
        return R;
    }
    template<typename T, int32_t n>
    llvm::GenericValue lle_X_bitselect(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        llvm::GenericValue arg2 = Args[2];

        for (int32_t i = 0; i < n; ++i)
        {
            R.AggregateVal[i] = localBitselect<T>(  getVal<T,n>(arg0, i),  getVal<T,n>(arg1, i),  getVal<T,n>(arg2, i) );
        }

        return R;
    }


    template<typename T>
    llvm::GenericValue selectResult( T inC )
    {
        llvm::GenericValue R;
        getRef<T>(R) = llvm::APInt(sizeof(T)*8, inC, isSignedType<T>());
        return R;
    }
    template<typename T, typename C>
    llvm::GenericValue lle_X_select(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        llvm::GenericValue arg2 = Args[2];

        T out = getVal<C>(arg2) ? getVal<T>(arg1) : getVal<T>(arg0); // For a scalar type, result = c ? b : a.

        R = selectResult<T>(out);
        return R;
    }
    template<typename T, typename C, int32_t n>
    llvm::GenericValue lle_X_select(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        typedef typename signedT<C>::type sC;
        llvm::GenericValue R;
        R.AggregateVal.resize(n);
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        llvm::GenericValue arg2 = Args[2];

        // For each component of a vector type,
        // result[i] = if MSB of c[i] is set ? b[i] : a[i].
        for (int32_t i = 0; i < n; ++i)
        {
            bool cond =  (getVal<C,n>(arg2, i) & intMin<sC>());
            T out = cond ? getVal<T,n>(arg1, i) : getVal<T,n>(arg0, i);
            R.AggregateVal[i] = selectResult<T>(out);
        }
        return R;
    }

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_RELATIONAL_H
