/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Reference OpenCL Builtins                                                   *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTAtomic.cpp

\*****************************************************************************/

#include "BLTAtomic.h"

using namespace llvm;
using std::string;
using std::vector;
using namespace Validation::OCLBuiltins;

#ifndef BUILTINS_API
   #if defined(_WIN32)
      #define BUILTINS_API __declspec(dllexport)
   #else
      #define BUILTINS_API
   #endif
#endif

namespace Validation {
namespace OCLBuiltins {
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
    }
    }
extern "C" {
BUILTINS_API void initOCLBuiltinsAtomic() {return;}
  

  BUILTINS_API llvm::GenericValue lle_X__Z8atom_addPU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<int32_t>(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_addPU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<uint32_t>(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_addPU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<int32_t>(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_addPU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<uint32_t>(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_addPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<int32_t>(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_addPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<uint32_t>(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_addPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<int32_t>(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_addPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<uint32_t>(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_andPU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<int32_t>(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_andPU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<uint32_t>(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_andPU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<int32_t>(FT,Args);}//10
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_andPU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<uint32_t>(FT,Args);}//11
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_andPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<int32_t>(FT,Args);}//12
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_andPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<uint32_t>(FT,Args);}//13
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_andPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<int32_t>(FT,Args);}//14
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_andPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<uint32_t>(FT,Args);}//15
  BUILTINS_API llvm::GenericValue lle_X__Z12atom_cmpxchgPU3AS1iii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<int32_t>(FT,Args);}//16
  BUILTINS_API llvm::GenericValue lle_X__Z12atom_cmpxchgPU3AS1jjj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<uint32_t>(FT,Args);}//17
  BUILTINS_API llvm::GenericValue lle_X__Z12atom_cmpxchgPU3AS3iii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<int32_t>(FT,Args);}//18
  BUILTINS_API llvm::GenericValue lle_X__Z12atom_cmpxchgPU3AS3jjj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<uint32_t>(FT,Args);}//19
  BUILTINS_API llvm::GenericValue lle_X__Z12atom_cmpxchgPVU3AS1iii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<int32_t>(FT,Args);}//20
  BUILTINS_API llvm::GenericValue lle_X__Z12atom_cmpxchgPVU3AS1jjj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<uint32_t>(FT,Args);}//21
  BUILTINS_API llvm::GenericValue lle_X__Z12atom_cmpxchgPVU3AS3iii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<int32_t>(FT,Args);}//22
  BUILTINS_API llvm::GenericValue lle_X__Z12atom_cmpxchgPVU3AS3jjj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<uint32_t>(FT,Args);}//23
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_decPU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<int32_t>(FT,Args);}//24
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_decPU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<uint32_t>(FT,Args);}//25
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_decPU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<int32_t>(FT,Args);}//26
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_decPU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<uint32_t>(FT,Args);}//27
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_decPVU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<int32_t>(FT,Args);}//28
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_decPVU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<uint32_t>(FT,Args);}//29
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_decPVU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<int32_t>(FT,Args);}//30
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_decPVU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<uint32_t>(FT,Args);}//31
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_incPU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<int32_t>(FT,Args);}//32
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_incPU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<uint32_t>(FT,Args);}//33
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_incPU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<int32_t>(FT,Args);}//34
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_incPU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<uint32_t>(FT,Args);}//35
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_incPVU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<int32_t>(FT,Args);}//36
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_incPVU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<uint32_t>(FT,Args);}//37
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_incPVU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<int32_t>(FT,Args);}//38
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_incPVU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<uint32_t>(FT,Args);}//39
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_maxPU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<int32_t>(FT,Args);}//40
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_maxPU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<uint32_t>(FT,Args);}//41
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_maxPU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<int32_t>(FT,Args);}//42
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_maxPU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<uint32_t>(FT,Args);}//43
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_maxPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<int32_t>(FT,Args);}//44
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_maxPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<uint32_t>(FT,Args);}//45
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_maxPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<int32_t>(FT,Args);}//46
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_maxPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<uint32_t>(FT,Args);}//47
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_minPU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<int32_t>(FT,Args);}//48
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_minPU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<uint32_t>(FT,Args);}//49
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_minPU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<int32_t>(FT,Args);}//50
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_minPU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<uint32_t>(FT,Args);}//51
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_minPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<int32_t>(FT,Args);}//52
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_minPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<uint32_t>(FT,Args);}//53
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_minPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<int32_t>(FT,Args);}//54
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_minPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<uint32_t>(FT,Args);}//55
  BUILTINS_API llvm::GenericValue lle_X__Z7atom_orPU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<int32_t>(FT,Args);}//56
  BUILTINS_API llvm::GenericValue lle_X__Z7atom_orPU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<uint32_t>(FT,Args);}//57
  BUILTINS_API llvm::GenericValue lle_X__Z7atom_orPU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<int32_t>(FT,Args);}//58
  BUILTINS_API llvm::GenericValue lle_X__Z7atom_orPU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<uint32_t>(FT,Args);}//59
  BUILTINS_API llvm::GenericValue lle_X__Z7atom_orPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<int32_t>(FT,Args);}//60
  BUILTINS_API llvm::GenericValue lle_X__Z7atom_orPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<uint32_t>(FT,Args);}//61
  BUILTINS_API llvm::GenericValue lle_X__Z7atom_orPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<int32_t>(FT,Args);}//62
  BUILTINS_API llvm::GenericValue lle_X__Z7atom_orPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<uint32_t>(FT,Args);}//63
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_subPU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<int32_t>(FT,Args);}//64
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_subPU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<uint32_t>(FT,Args);}//65
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_subPU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<int32_t>(FT,Args);}//66
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_subPU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<uint32_t>(FT,Args);}//67
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_subPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<int32_t>(FT,Args);}//68
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_subPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<uint32_t>(FT,Args);}//69
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_subPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<int32_t>(FT,Args);}//70
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_subPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<uint32_t>(FT,Args);}//71
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<int32_t>(FT,Args);}//72
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<uint32_t>(FT,Args);}//73
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPU3AS1ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<float>(FT,Args);}//74
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<int32_t>(FT,Args);}//75
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<uint32_t>(FT,Args);}//76
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPU3AS3ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<float>(FT,Args);}//77
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<int32_t>(FT,Args);}//78
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<uint32_t>(FT,Args);}//79
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPVU3AS1ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<float>(FT,Args);}//80
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<int32_t>(FT,Args);}//81
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<uint32_t>(FT,Args);}//82
  BUILTINS_API llvm::GenericValue lle_X__Z9atom_xchgPVU3AS3ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<float>(FT,Args);}//83
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_xorPU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<int32_t>(FT,Args);}//84
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_xorPU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<uint32_t>(FT,Args);}//85
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_xorPU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<int32_t>(FT,Args);}//86
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_xorPU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<uint32_t>(FT,Args);}//87
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_xorPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<int32_t>(FT,Args);}//88
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_xorPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<uint32_t>(FT,Args);}//89
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_xorPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<int32_t>(FT,Args);}//90
  BUILTINS_API llvm::GenericValue lle_X__Z8atom_xorPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<uint32_t>(FT,Args);}//91
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_addPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<int32_t>(FT,Args);}//92
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_addPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<uint32_t>(FT,Args);}//93
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_addPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<int32_t>(FT,Args);}//94
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_addPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_add<uint32_t>(FT,Args);}//95
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_andPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<int32_t>(FT,Args);}//96
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_andPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<uint32_t>(FT,Args);}//97
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_andPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<int32_t>(FT,Args);}//98
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_andPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_and<uint32_t>(FT,Args);}//99
  BUILTINS_API llvm::GenericValue lle_X__Z14atomic_cmpxchgPVU3AS1iii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<int32_t>(FT,Args);}//100
  BUILTINS_API llvm::GenericValue lle_X__Z14atomic_cmpxchgPVU3AS1jjj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<uint32_t>(FT,Args);}//101
  BUILTINS_API llvm::GenericValue lle_X__Z14atomic_cmpxchgPVU3AS3iii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<int32_t>(FT,Args);}//102
  BUILTINS_API llvm::GenericValue lle_X__Z14atomic_cmpxchgPVU3AS3jjj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_cmpxchg<uint32_t>(FT,Args);}//103
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_decPVU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<int32_t>(FT,Args);}//104
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_decPVU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<uint32_t>(FT,Args);}//105
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_decPVU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<int32_t>(FT,Args);}//106
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_decPVU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_dec<uint32_t>(FT,Args);}//107
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_incPVU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<int32_t>(FT,Args);}//108
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_incPVU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<uint32_t>(FT,Args);}//109
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_incPVU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<int32_t>(FT,Args);}//110
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_incPVU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_inc<uint32_t>(FT,Args);}//111
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_maxPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<int32_t>(FT,Args);}//112
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_maxPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<uint32_t>(FT,Args);}//113
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_maxPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<int32_t>(FT,Args);}//114
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_maxPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_max<uint32_t>(FT,Args);}//115
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_minPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<int32_t>(FT,Args);}//116
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_minPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<uint32_t>(FT,Args);}//117
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_minPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<int32_t>(FT,Args);}//118
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_minPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_min<uint32_t>(FT,Args);}//119
  BUILTINS_API llvm::GenericValue lle_X__Z9atomic_orPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<int32_t>(FT,Args);}//120
  BUILTINS_API llvm::GenericValue lle_X__Z9atomic_orPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<uint32_t>(FT,Args);}//121
  BUILTINS_API llvm::GenericValue lle_X__Z9atomic_orPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<int32_t>(FT,Args);}//122
  BUILTINS_API llvm::GenericValue lle_X__Z9atomic_orPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_or<uint32_t>(FT,Args);}//123
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_subPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<int32_t>(FT,Args);}//124
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_subPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<uint32_t>(FT,Args);}//125
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_subPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<int32_t>(FT,Args);}//126
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_subPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_sub<uint32_t>(FT,Args);}//127
  BUILTINS_API llvm::GenericValue lle_X__Z11atomic_xchgPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<int32_t>(FT,Args);}//128
  BUILTINS_API llvm::GenericValue lle_X__Z11atomic_xchgPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<uint32_t>(FT,Args);}//129
  BUILTINS_API llvm::GenericValue lle_X__Z11atomic_xchgPVU3AS1ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<float>(FT,Args);}//130
  BUILTINS_API llvm::GenericValue lle_X__Z11atomic_xchgPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<int32_t>(FT,Args);}//131
  BUILTINS_API llvm::GenericValue lle_X__Z11atomic_xchgPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<uint32_t>(FT,Args);}//132
  BUILTINS_API llvm::GenericValue lle_X__Z11atomic_xchgPVU3AS3ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xchg<float>(FT,Args);}//133
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_xorPVU3AS1ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<int32_t>(FT,Args);}//134
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_xorPVU3AS1jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<uint32_t>(FT,Args);}//135
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_xorPVU3AS3ii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<int32_t>(FT,Args);}//136
  BUILTINS_API llvm::GenericValue lle_X__Z10atomic_xorPVU3AS3jj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_atomic_xor<uint32_t>(FT,Args);}//137


}

  
