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

File Name:  VLoadStore.h

\*****************************************************************************/
#ifndef V_LOAD_STORE_H
#define V_LOAD_STORE_H

#include <vector>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Conformance/reference_convert.h"
#include "Helpers.h"
#include "dxfloat.h"

namespace Validation {
namespace OCLBuiltins {

    // template parameter T could be int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
    template<typename T, int n>
    llvm::GenericValue lle_X_vload(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        uint32_t offset  = arg0.IntVal.getZExtValue();
        offset *= n;
        T* p = static_cast<T*>(arg1.PointerVal);
        p += offset;
        R.AggregateVal.resize(n);
        for (uint32_t i = 0; i < n; ++i, ++p)
        {
            getRef<T, n> (R, i) = derefPointer<T>(p);
        }
        return R;
    }

    template<typename T, int n>
    llvm::GenericValue lle_X_vstore(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue arg0=Args[0];
        llvm::GenericValue arg1=Args[1];
        llvm::GenericValue arg2=Args[2];

        uint32_t offset = arg1.IntVal.getZExtValue();
        offset *= n; // (offset * n)
        T* outBuffer = static_cast<T*>(arg2.PointerVal);
        outBuffer += offset;

        for(uint32_t j = 0; j < n; ++j, ++outBuffer)
        {
            *outBuffer = getVal<T, n>(arg0, j);
        }
        return llvm::GenericValue();
    }

    template<int n, bool aligned>
    llvm::GenericValue lle_X_vload_half(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        uint32_t offset  = arg0.IntVal.getZExtValue();
        offset *= n;
        uint16_t* p = static_cast<uint16_t*>(arg1.PointerVal);
        p += offset;
        R.AggregateVal.resize(n);
        for (uint32_t i = 0; i < n; ++i, ++p)
        {
            getRef<float, n> (R, i) = float(CFloat16(*p));
        }
        return R;
    }

    template<> llvm::GenericValue lle_X_vload_half<3, true>(
        llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    // Wrapper to provide single interface to two conversion functions: double2half and float2half
    template<typename T>
    uint16_t convert2half(T f);

    template<>
    uint16_t convert2half<float>(float f);

    template<>
    uint16_t convert2half<double>(double f);

    template<typename T, int n, bool aligned>
    llvm::GenericValue lle_X_vstore_half(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        llvm::GenericValue arg2 = Args[2];
        uint32_t offset  = arg1.IntVal.getZExtValue();
        offset *= n;
        uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);
        p += offset;
        for (uint32_t i = 0; i < n; ++i, ++p)
        {
            *p = convert2half(getVal<T, n>(arg0, i));
        }
        return llvm::GenericValue();
    }

    template<> llvm::GenericValue lle_X_vstore_half<float, 3, true>(
        llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    template<> llvm::GenericValue lle_X_vstore_half<double, 3, true>(
        llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

} // namespace OCLBuiltins
} // namespace Validation

#endif // V_LOAD_STORE_H
