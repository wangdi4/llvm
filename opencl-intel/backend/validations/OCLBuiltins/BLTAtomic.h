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

File Name:  BLTAtomic.h

\*****************************************************************************/
#ifndef BLT_ATOMIC_H
#define BLT_ATOMIC_H

#include <vector>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Helpers.h"
#include "IBLTMapFiller.h"
#include "RefALU.h"

namespace Validation {
namespace OCLBuiltins {

    // This class adds references to the implementations of OpenCL built-in functions from 6.11.5 section.
    class AtomicMapFiller : public IBLTMapFiller
    {
    public:
        void addOpenCLBuiltins(std::map<std::string, PBLTFunc>& funcNames);
    };

    template<typename T>
    llvm::GenericValue lle_X_atomic_add(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = old + getVal<T>(arg1);

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_sub(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = old - getVal<T>(arg1);

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_xchg(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = getVal<T>(arg1);

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_inc(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = *p + 1;

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_dec(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = *p - 1;

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_cmpxchg(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        llvm::GenericValue arg2 = Args[2];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = (old == getVal<T>(arg1)) ? getVal<T>(arg2) : old;

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_min(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = std::min(old,getVal<T>(arg1));

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_max(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = std::max(old,getVal<T>(arg1));

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_and(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = old & getVal<T>(arg1);

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_or(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = old | getVal<T>(arg1);

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<typename T>
    llvm::GenericValue lle_X_atomic_xor(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        T* p = static_cast<T*>(arg0.PointerVal);
        
        T old = *p;
        *p = old ^ getVal<T>(arg1);

        getRef<T>(R) = llvm::APInt(sizeof(T)*8, old);

        return R;
    }

    template<>
    llvm::GenericValue lle_X_atomic_xchg<float>(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        float* p = static_cast<float*>(arg0.PointerVal);
        
        float old = *p;
        *p = getVal<float>(arg1);

        getRef<float>(R) = old;

        return R;
    }
	
} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_ATOMIC_H
