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

File Name:  BLTCommon.h

\*****************************************************************************/
#ifndef BLT_COMMON_H
#define BLT_COMMON_H

#include <vector>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Helpers.h"
#include "IBLTMapFiller.h"
#include <RefALU.h>

namespace Validation {
namespace OCLBuiltins {

// This class adds references to the implementations of OpenCL built-in functions from 6.11.4 section.
class CommonMapFiller : public IBLTMapFiller
{
public:
    void addOpenCLBuiltins(std::map<std::string, PBLTFunc>& funcNames);
};

template<typename T, int n, int s>
llvm::GenericValue lle_X_max(const llvm::FunctionType *FT,
                       const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    R.AggregateVal.resize(n);
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    for (uint32_t i = 0; i < n; ++i)
    {
        getRef<T,n>(R, i) = RefALU::max(getVal<T,n>(arg0, i), getVal<T,s>(arg1, i));
    }
    return R;
}

template<typename T, int n, int s>
llvm::GenericValue lle_X_min(const llvm::FunctionType *FT,
                       const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    R.AggregateVal.resize(n);
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    for (uint32_t i = 0; i < n; ++i)
    {
        getRef<T,n>(R, i) = RefALU::min(getRef<T,n>(arg0, i), getRef<T,s>(arg1, i));
    }
    return R;
}

template<typename T, int n, int s>
llvm::GenericValue lle_X_mix(const llvm::FunctionType *FT,
                             const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    R.AggregateVal.resize(n);
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    for (uint32_t i = 0; i < n; ++i)
    {
        getRef<T,n>(R, i) = getVal<T,n>(arg0, i) + (getVal<T,n>(arg1, i) - getVal<T,n>(arg0, i)) * getVal<T,s>(arg2, i);
    }
    return R;
}

template<typename T, int s, int n>
llvm::GenericValue lle_X_step(const llvm::FunctionType *FT,
                             const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    R.AggregateVal.resize(n);
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    for (uint32_t i = 0; i < n; ++i)
    {
        getRef<T,n>(R, i) = RefALU::step(getRef<T,s>(arg0, i), getRef<T,n>(arg1, i));
    }
    return R;
}

template<typename T, int s, int n>
llvm::GenericValue lle_X_smoothstep(const llvm::FunctionType *FT,
                             const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    R.AggregateVal.resize(n);
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    for (uint32_t i = 0; i < n; ++i)
    {
        getRef<T,n>(R, i) = RefALU::smoothstep(getRef<T,s>(arg0, i), getRef<T,s>(arg1, i), getRef<T,n>(arg2, i));
    }
    return R;
}

template<typename T, int n, int s>
llvm::GenericValue lle_X_clamp(const llvm::FunctionType *FT,
                             const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    R.AggregateVal.resize(n);
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    for (uint32_t i = 0; i < n; ++i)
    {
        getRef<T,n>(R, i) = RefALU::min(RefALU::max(getVal<T,n>(arg0, i), getVal<T,s>(arg1, i)), getVal<T,s>(arg2, i));
    }
    return R;
}

DEFINE_BLT_ONE_ARG(radians)
DEFINE_BLT_ONE_ARG(degrees)
DEFINE_BLT_ONE_ARG(sign)
} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_COMMON_H
