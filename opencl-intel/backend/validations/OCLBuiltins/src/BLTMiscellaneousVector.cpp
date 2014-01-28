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

File Name:  BLTMiscellaneousVector.cpp

\*****************************************************************************/

#include "BLTMiscellaneousVector.h"

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

extern "C" {
BUILTINS_API void initOCLBuiltinsMisc() {return;}
  

  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_cDv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,16>(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_cDv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,16>(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_cDv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,16>(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_cDv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,16>(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_hDv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,16>(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_hDv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,16>(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_hDv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,16>(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_hS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,16>(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_sDv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,16>(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_sDv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,16>(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_sDv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,16>(FT,Args);}//10
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_sDv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,16>(FT,Args);}//11
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_tDv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,16>(FT,Args);}//12
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_tDv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,16>(FT,Args);}//13
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_tDv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,16>(FT,Args);}//14
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_tS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,16>(FT,Args);}//15
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_iDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,16>(FT,Args);}//16
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_iDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,16>(FT,Args);}//17
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_iDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,16>(FT,Args);}//18
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_iDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,16>(FT,Args);}//19
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_jDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,16>(FT,Args);}//20
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_jDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,16>(FT,Args);}//21
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_jDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,16>(FT,Args);}//22
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_jS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,16>(FT,Args);}//23
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_lDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,16>(FT,Args);}//24
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_lDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,16>(FT,Args);}//25
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_lDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,16>(FT,Args);}//26
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_lDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,16>(FT,Args);}//27
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_mDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,16>(FT,Args);}//28
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_mDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,16>(FT,Args);}//29
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_mDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,16>(FT,Args);}//30
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_mS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,16>(FT,Args);}//31
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_fDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,16>(FT,Args);}//32
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_fDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,16>(FT,Args);}//33
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_fDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,16>(FT,Args);}//34
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_fDv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,16>(FT,Args);}//35
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_dDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,16>(FT,Args);}//36
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_dDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,16>(FT,Args);}//37
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_dDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,16>(FT,Args);}//38
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_dDv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,16>(FT,Args);}//39
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_cDv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,2>(FT,Args);}//40
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_cDv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,2>(FT,Args);}//41
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_cDv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,2>(FT,Args);}//42
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_cDv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,2>(FT,Args);}//43
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_hS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,2>(FT,Args);}//44
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_hDv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,2>(FT,Args);}//45
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_hDv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,2>(FT,Args);}//46
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_hDv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,2>(FT,Args);}//47
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_sDv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,2>(FT,Args);}//48
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_sDv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,2>(FT,Args);}//49
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_sDv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,2>(FT,Args);}//50
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_sDv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,2>(FT,Args);}//51
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_tS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,2>(FT,Args);}//52
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_tDv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,2>(FT,Args);}//53
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_tDv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,2>(FT,Args);}//54
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_tDv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,2>(FT,Args);}//55
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_iDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,2>(FT,Args);}//56
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_iDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,2>(FT,Args);}//57
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_iDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,2>(FT,Args);}//58
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_iDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,2>(FT,Args);}//59
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_jS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,2>(FT,Args);}//60
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_jDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,2>(FT,Args);}//61
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_jDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,2>(FT,Args);}//62
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_jDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,2>(FT,Args);}//63
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_lDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,2>(FT,Args);}//64
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_lDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,2>(FT,Args);}//65
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_lDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,2>(FT,Args);}//66
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_lDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,2>(FT,Args);}//67
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_mS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,2>(FT,Args);}//68
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_mDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,2>(FT,Args);}//69
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_mDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,2>(FT,Args);}//70
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_mDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,2>(FT,Args);}//71
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_fDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,2>(FT,Args);}//72
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_fDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,2>(FT,Args);}//73
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_fDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,2>(FT,Args);}//74
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_fDv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,2>(FT,Args);}//75
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_dDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,2>(FT,Args);}//76
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_dDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,2>(FT,Args);}//77
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_dDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,2>(FT,Args);}//78
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_dDv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,2>(FT,Args);}//79
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_cDv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,4>(FT,Args);}//80
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_cDv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,4>(FT,Args);}//81
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_cDv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,4>(FT,Args);}//82
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_cDv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,4>(FT,Args);}//83
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_hDv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,4>(FT,Args);}//84
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_hS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,4>(FT,Args);}//85
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_hDv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,4>(FT,Args);}//86
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_hDv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,4>(FT,Args);}//87
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_sDv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,4>(FT,Args);}//88
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_sDv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,4>(FT,Args);}//89
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_sDv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,4>(FT,Args);}//90
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_sDv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,4>(FT,Args);}//91
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_tDv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,4>(FT,Args);}//92
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_tS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,4>(FT,Args);}//93
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_tDv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,4>(FT,Args);}//94
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_tDv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,4>(FT,Args);}//95
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_iDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,4>(FT,Args);}//96
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_iDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,4>(FT,Args);}//97
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_iDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,4>(FT,Args);}//98
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_iDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,4>(FT,Args);}//99
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_jDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,4>(FT,Args);}//100
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_jS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,4>(FT,Args);}//101
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_jDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,4>(FT,Args);}//102
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_jDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,4>(FT,Args);}//103
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_lDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,4>(FT,Args);}//104
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_lDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,4>(FT,Args);}//105
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_lDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,4>(FT,Args);}//106
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_lDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,4>(FT,Args);}//107
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_mDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,4>(FT,Args);}//108
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_mS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,4>(FT,Args);}//109
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_mDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,4>(FT,Args);}//110
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_mDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,4>(FT,Args);}//111
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_fDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,4>(FT,Args);}//112
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_fDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,4>(FT,Args);}//113
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_fDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,4>(FT,Args);}//114
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_fDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,4>(FT,Args);}//115
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_dDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,4>(FT,Args);}//116
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_dDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,4>(FT,Args);}//117
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_dDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,4>(FT,Args);}//118
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_dDv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,4>(FT,Args);}//119
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_cDv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,8>(FT,Args);}//120
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_cDv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,8>(FT,Args);}//121
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_cDv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,8>(FT,Args);}//122
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_cDv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int8_t,uint8_t,8>(FT,Args);}//123
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_hDv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,8>(FT,Args);}//124
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_hDv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,8>(FT,Args);}//125
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_hS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,8>(FT,Args);}//126
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_hDv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint8_t,uint8_t,8>(FT,Args);}//127
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_sDv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,8>(FT,Args);}//128
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_sDv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,8>(FT,Args);}//129
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_sDv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,8>(FT,Args);}//130
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_sDv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int16_t,uint16_t,8>(FT,Args);}//131
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_tDv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,8>(FT,Args);}//132
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_tDv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,8>(FT,Args);}//133
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_tS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,8>(FT,Args);}//134
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_tDv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint16_t,uint16_t,8>(FT,Args);}//135
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_iDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,8>(FT,Args);}//136
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_iDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,8>(FT,Args);}//137
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_iDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,8>(FT,Args);}//138
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_iDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int32_t,uint32_t,8>(FT,Args);}//139
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_jDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,8>(FT,Args);}//140
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_jDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,8>(FT,Args);}//141
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_jS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,8>(FT,Args);}//142
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_jDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint32_t,uint32_t,8>(FT,Args);}//143
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_lDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,8>(FT,Args);}//144
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_lDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,8>(FT,Args);}//145
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_lDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,8>(FT,Args);}//146
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_lDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<int64_t,uint64_t,8>(FT,Args);}//147
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_mDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,8>(FT,Args);}//148
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_mDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,8>(FT,Args);}//149
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_mS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,8>(FT,Args);}//150
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_mDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<uint64_t,uint64_t,8>(FT,Args);}//151
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_fDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,8>(FT,Args);}//152
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_fDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,8>(FT,Args);}//153
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_fDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,8>(FT,Args);}//154
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_fDv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<float,uint32_t,8>(FT,Args);}//155
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv2_dDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,8>(FT,Args);}//156
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv4_dDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,8>(FT,Args);}//157
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv8_dDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,8>(FT,Args);}//158
  BUILTINS_API llvm::GenericValue lle_X__Z7shuffleDv16_dDv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle<double,uint64_t,8>(FT,Args);}//159
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_cS_Dv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,16>(FT,Args);}//160
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_cS_Dv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,16>(FT,Args);}//161
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_cS_Dv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,16>(FT,Args);}//162
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_cS_Dv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,16>(FT,Args);}//163
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_hS_Dv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,16>(FT,Args);}//164
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_hS_Dv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,16>(FT,Args);}//165
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_hS_Dv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,16>(FT,Args);}//166
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,16>(FT,Args);}//167
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_sS_Dv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,16>(FT,Args);}//168
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_sS_Dv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,16>(FT,Args);}//169
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_sS_Dv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,16>(FT,Args);}//170
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_sS_Dv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,16>(FT,Args);}//171
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_tS_Dv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,16>(FT,Args);}//172
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_tS_Dv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,16>(FT,Args);}//173
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_tS_Dv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,16>(FT,Args);}//174
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,16>(FT,Args);}//175
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_iS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,16>(FT,Args);}//176
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_iS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,16>(FT,Args);}//177
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_iS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,16>(FT,Args);}//178
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_iS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,16>(FT,Args);}//179
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_jS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,16>(FT,Args);}//180
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_jS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,16>(FT,Args);}//181
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_jS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,16>(FT,Args);}//182
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,16>(FT,Args);}//183
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_lS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,16>(FT,Args);}//184
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_lS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,16>(FT,Args);}//185
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_lS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,16>(FT,Args);}//186
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_lS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,16>(FT,Args);}//187
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_mS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,16>(FT,Args);}//188
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_mS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,16>(FT,Args);}//189
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_mS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,16>(FT,Args);}//190
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,16>(FT,Args);}//191
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_fS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,16>(FT,Args);}//192
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_fS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,16>(FT,Args);}//193
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_fS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,16>(FT,Args);}//194
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_fS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,16>(FT,Args);}//195
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_dS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,16>(FT,Args);}//196
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_dS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,16>(FT,Args);}//197
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_dS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,16>(FT,Args);}//198
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_dS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,16>(FT,Args);}//199
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_cS_Dv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,2>(FT,Args);}//200
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_cS_Dv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,2>(FT,Args);}//201
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_cS_Dv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,2>(FT,Args);}//202
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_cS_Dv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,2>(FT,Args);}//203
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,2>(FT,Args);}//204
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_hS_Dv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,2>(FT,Args);}//205
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_hS_Dv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,2>(FT,Args);}//206
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_hS_Dv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,2>(FT,Args);}//207
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_sS_Dv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,2>(FT,Args);}//208
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_sS_Dv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,2>(FT,Args);}//209
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_sS_Dv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,2>(FT,Args);}//210
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_sS_Dv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,2>(FT,Args);}//211
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,2>(FT,Args);}//212
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_tS_Dv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,2>(FT,Args);}//213
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_tS_Dv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,2>(FT,Args);}//214
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_tS_Dv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,2>(FT,Args);}//215
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_iS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,2>(FT,Args);}//216
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_iS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,2>(FT,Args);}//217
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_iS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,2>(FT,Args);}//218
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_iS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,2>(FT,Args);}//219
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,2>(FT,Args);}//220
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_jS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,2>(FT,Args);}//221
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_jS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,2>(FT,Args);}//222
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_jS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,2>(FT,Args);}//223
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_lS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,2>(FT,Args);}//224
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_lS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,2>(FT,Args);}//225
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_lS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,2>(FT,Args);}//226
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_lS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,2>(FT,Args);}//227
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,2>(FT,Args);}//228
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_mS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,2>(FT,Args);}//229
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_mS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,2>(FT,Args);}//230
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_mS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,2>(FT,Args);}//231
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_fS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,2>(FT,Args);}//232
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_fS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,2>(FT,Args);}//233
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_fS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,2>(FT,Args);}//234
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_fS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,2>(FT,Args);}//235
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_dS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,2>(FT,Args);}//236
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_dS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,2>(FT,Args);}//237
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_dS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,2>(FT,Args);}//238
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_dS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,2>(FT,Args);}//239
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_cS_Dv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,4>(FT,Args);}//240
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_cS_Dv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,4>(FT,Args);}//241
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_cS_Dv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,4>(FT,Args);}//242
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_cS_Dv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,4>(FT,Args);}//243
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_hS_Dv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,4>(FT,Args);}//244
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,4>(FT,Args);}//245
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_hS_Dv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,4>(FT,Args);}//246
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_hS_Dv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,4>(FT,Args);}//247
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_sS_Dv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,4>(FT,Args);}//248
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_sS_Dv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,4>(FT,Args);}//249
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_sS_Dv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,4>(FT,Args);}//250
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_sS_Dv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,4>(FT,Args);}//251
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_tS_Dv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,4>(FT,Args);}//252
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,4>(FT,Args);}//253
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_tS_Dv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,4>(FT,Args);}//254
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_tS_Dv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,4>(FT,Args);}//255
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_iS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,4>(FT,Args);}//256
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_iS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,4>(FT,Args);}//257
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_iS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,4>(FT,Args);}//258
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_iS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,4>(FT,Args);}//259
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_jS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,4>(FT,Args);}//260
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,4>(FT,Args);}//261
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_jS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,4>(FT,Args);}//262
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_jS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,4>(FT,Args);}//263
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_lS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,4>(FT,Args);}//264
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_lS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,4>(FT,Args);}//265
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_lS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,4>(FT,Args);}//266
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_lS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,4>(FT,Args);}//267
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_mS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,4>(FT,Args);}//268
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,4>(FT,Args);}//269
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_mS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,4>(FT,Args);}//270
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_mS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,4>(FT,Args);}//271
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_fS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,4>(FT,Args);}//272
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_fS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,4>(FT,Args);}//273
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_fS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,4>(FT,Args);}//274
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_fS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,4>(FT,Args);}//275
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_dS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,4>(FT,Args);}//276
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_dS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,4>(FT,Args);}//277
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_dS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,4>(FT,Args);}//278
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_dS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,4>(FT,Args);}//279
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_cS_Dv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,8>(FT,Args);}//280
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_cS_Dv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,8>(FT,Args);}//281
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_cS_Dv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,8>(FT,Args);}//282
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_cS_Dv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int8_t,uint8_t,8>(FT,Args);}//283
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_hS_Dv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,8>(FT,Args);}//284
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_hS_Dv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,8>(FT,Args);}//285
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,8>(FT,Args);}//286
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_hS_Dv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint8_t,uint8_t,8>(FT,Args);}//287
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_sS_Dv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,8>(FT,Args);}//288
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_sS_Dv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,8>(FT,Args);}//289
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_sS_Dv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,8>(FT,Args);}//290
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_sS_Dv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int16_t,uint16_t,8>(FT,Args);}//291
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_tS_Dv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,8>(FT,Args);}//292
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_tS_Dv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,8>(FT,Args);}//293
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,8>(FT,Args);}//294
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_tS_Dv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint16_t,uint16_t,8>(FT,Args);}//295
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_iS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,8>(FT,Args);}//296
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_iS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,8>(FT,Args);}//297
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_iS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,8>(FT,Args);}//298
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_iS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int32_t,uint32_t,8>(FT,Args);}//299
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_jS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,8>(FT,Args);}//300
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_jS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,8>(FT,Args);}//301
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,8>(FT,Args);}//302
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_jS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint32_t,uint32_t,8>(FT,Args);}//303
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_lS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,8>(FT,Args);}//304
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_lS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,8>(FT,Args);}//305
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_lS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,8>(FT,Args);}//306
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_lS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<int64_t,uint64_t,8>(FT,Args);}//307
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_mS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,8>(FT,Args);}//308
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_mS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,8>(FT,Args);}//309
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,8>(FT,Args);}//310
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_mS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<uint64_t,uint64_t,8>(FT,Args);}//311
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_fS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,8>(FT,Args);}//312
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_fS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,8>(FT,Args);}//313
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_fS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,8>(FT,Args);}//314
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_fS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<float,uint32_t,8>(FT,Args);}//315
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv2_dS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,8>(FT,Args);}//316
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv4_dS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,8>(FT,Args);}//317
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv8_dS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,8>(FT,Args);}//318
  BUILTINS_API llvm::GenericValue lle_X__Z8shuffle2Dv16_dS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_shuffle2<double,uint64_t,8>(FT,Args);}//319


}

  
