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

File Name:  BLTAsyncCopiesAndPrefetch.h

\*****************************************************************************/
#ifndef BLT_ASYNC_COPIES_AND_PREFETCH_H
#define BLT_ASYNC_COPIES_AND_PREFETCH_H

#include <vector>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Helpers.h"
#include "IBLTMapFiller.h"
#include <RefALU.h>

namespace Validation {
namespace OCLBuiltins {

// This class adds references to the implementations of OpenCL built-in functions from 6.11.10 section.
class AsyncCopiesAndPrefetchMapFiller : public IBLTMapFiller
{
public:
    void addOpenCLBuiltins(std::map<std::string, PBLTFunc>& funcNames);
};

llvm::GenericValue lle_X_prefetch(const llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args);
llvm::GenericValue lle_X_wait_group_events(const llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args);

template <typename T, int n>
llvm::GenericValue lle_X_async_work_group_copy(const llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    llvm::GenericValue arg3 = Args[3];
    size_t num_gentypes = getVal<size_t>(arg2);
    size_t eventValue = getVal<size_t>(arg3);
    T* dst = static_cast<T*>(arg0.PointerVal);
    T* src = static_cast<T*>(arg1.PointerVal);
    std::copy(src, src+ n * num_gentypes, dst);
    if (eventValue)
        R.IntVal = llvm::APInt(sizeof(size_t)*8, eventValue);
    else
        R.IntVal = llvm::APInt(sizeof(size_t)*8, 1);
    return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_async_work_group_strided_copy_l2g(const llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    llvm::GenericValue arg3 = Args[3];
    llvm::GenericValue arg4 = Args[4];
    size_t num_gentypes = getVal<size_t>(arg2);
    size_t dst_stride = getVal<size_t>(arg3);
    size_t eventValue = getVal<size_t>(arg4);
    T* dst = static_cast<T*>(arg0.PointerVal);
    T* src = static_cast<T*>(arg1.PointerVal);
    for (size_t i = 0; i < num_gentypes; ++i, dst += n * dst_stride, src += n)
    {
        std::copy(src, src+n, dst);
    }
    if (eventValue)
        R.IntVal = llvm::APInt(sizeof(size_t)*8, eventValue);
    else
        R.IntVal = llvm::APInt(sizeof(size_t)*8, 1);
    return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_async_work_group_strided_copy_g2l(const llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    llvm::GenericValue arg3 = Args[3];
    llvm::GenericValue arg4 = Args[4];
    size_t num_gentypes = getVal<size_t>(arg2);
    size_t src_stride = getVal<size_t>(arg3);
    size_t eventValue = getVal<size_t>(arg4);
    T* dst = static_cast<T*>(arg0.PointerVal);
    T* src = static_cast<T*>(arg1.PointerVal);
    for (size_t i = 0; i < num_gentypes; ++i, src += n * src_stride, dst += n)
    {
        std::copy(src, src+n, dst);
    }
    if (eventValue)
        R.IntVal = llvm::APInt(sizeof(size_t)*8, eventValue);
    else
        R.IntVal = llvm::APInt(sizeof(size_t)*8, 1);
    return R;
}

} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_ASYNC_COPIES_AND_PREFETCH_H
