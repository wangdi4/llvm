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

File Name:  BLTRelational.cpp

\*****************************************************************************/

#include "BLTRelational.h"

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
BUILTINS_API void initOCLBuiltinsRelational() {return;}
  

  BUILTINS_API llvm::GenericValue lle_X__Z3allc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int8_t>(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z3alls( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int16_t>(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z3alli( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int32_t>(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z3alll( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int64_t>(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv2_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int8_t,2>(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv3_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int8_t,3>(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv4_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int8_t,4>(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv8_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int8_t,8>(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv16_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int8_t,16>(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv2_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int16_t,2>(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv3_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int16_t,3>(FT,Args);}//10
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv4_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int16_t,4>(FT,Args);}//11
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv8_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int16_t,8>(FT,Args);}//12
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv16_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int16_t,16>(FT,Args);}//13
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int32_t,2>(FT,Args);}//14
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv3_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int32_t,3>(FT,Args);}//15
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int32_t,4>(FT,Args);}//16
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv8_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int32_t,8>(FT,Args);}//17
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv16_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int32_t,16>(FT,Args);}//18
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv2_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int64_t,2>(FT,Args);}//19
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv3_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int64_t,3>(FT,Args);}//20
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv4_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int64_t,4>(FT,Args);}//21
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv8_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int64_t,8>(FT,Args);}//22
  BUILTINS_API llvm::GenericValue lle_X__Z3allDv16_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_all<int64_t,16>(FT,Args);}//23
  BUILTINS_API llvm::GenericValue lle_X__Z3anyc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int8_t>(FT,Args);}//24
  BUILTINS_API llvm::GenericValue lle_X__Z3anys( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int16_t>(FT,Args);}//25
  BUILTINS_API llvm::GenericValue lle_X__Z3anyi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int32_t>(FT,Args);}//26
  BUILTINS_API llvm::GenericValue lle_X__Z3anyl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int64_t>(FT,Args);}//27
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv2_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int8_t,2>(FT,Args);}//28
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv3_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int8_t,3>(FT,Args);}//29
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv4_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int8_t,4>(FT,Args);}//30
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv8_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int8_t,8>(FT,Args);}//31
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv16_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int8_t,16>(FT,Args);}//32
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv2_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int16_t,2>(FT,Args);}//33
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv3_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int16_t,3>(FT,Args);}//34
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv4_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int16_t,4>(FT,Args);}//35
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv8_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int16_t,8>(FT,Args);}//36
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv16_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int16_t,16>(FT,Args);}//37
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int32_t,2>(FT,Args);}//38
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv3_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int32_t,3>(FT,Args);}//39
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int32_t,4>(FT,Args);}//40
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv8_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int32_t,8>(FT,Args);}//41
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv16_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int32_t,16>(FT,Args);}//42
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv2_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int64_t,2>(FT,Args);}//43
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv3_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int64_t,3>(FT,Args);}//44
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv4_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int64_t,4>(FT,Args);}//45
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv8_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int64_t,8>(FT,Args);}//46
  BUILTINS_API llvm::GenericValue lle_X__Z3anyDv16_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_any<int64_t,16>(FT,Args);}//47
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectccc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int8_t>(FT,Args);}//48
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselecthhh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint8_t>(FT,Args);}//49
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectsss( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int16_t>(FT,Args);}//50
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectttt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint16_t>(FT,Args);}//51
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectiii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int32_t>(FT,Args);}//52
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectjjj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint32_t>(FT,Args);}//53
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectlll( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int64_t>(FT,Args);}//54
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectmmm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint64_t>(FT,Args);}//55
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectfff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<float>(FT,Args);}//56
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<double>(FT,Args);}//57
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int8_t,2>(FT,Args);}//58
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int8_t,3>(FT,Args);}//59
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int8_t,4>(FT,Args);}//60
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int8_t,8>(FT,Args);}//61
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int8_t,16>(FT,Args);}//62
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint8_t,2>(FT,Args);}//63
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint8_t,3>(FT,Args);}//64
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint8_t,4>(FT,Args);}//65
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint8_t,8>(FT,Args);}//66
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint8_t,16>(FT,Args);}//67
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int16_t,2>(FT,Args);}//68
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int16_t,3>(FT,Args);}//69
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int16_t,4>(FT,Args);}//70
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int16_t,8>(FT,Args);}//71
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int16_t,16>(FT,Args);}//72
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint16_t,2>(FT,Args);}//73
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint16_t,3>(FT,Args);}//74
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint16_t,4>(FT,Args);}//75
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint16_t,8>(FT,Args);}//76
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint16_t,16>(FT,Args);}//77
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int32_t,2>(FT,Args);}//78
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int32_t,3>(FT,Args);}//79
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int32_t,4>(FT,Args);}//80
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int32_t,8>(FT,Args);}//81
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int32_t,16>(FT,Args);}//82
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint32_t,2>(FT,Args);}//83
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint32_t,3>(FT,Args);}//84
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint32_t,4>(FT,Args);}//85
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint32_t,8>(FT,Args);}//86
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint32_t,16>(FT,Args);}//87
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int64_t,2>(FT,Args);}//88
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int64_t,3>(FT,Args);}//89
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int64_t,4>(FT,Args);}//90
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int64_t,8>(FT,Args);}//91
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<int64_t,16>(FT,Args);}//92
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint64_t,2>(FT,Args);}//93
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint64_t,3>(FT,Args);}//94
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint64_t,4>(FT,Args);}//95
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint64_t,8>(FT,Args);}//96
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<uint64_t,16>(FT,Args);}//97
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<float,2>(FT,Args);}//98
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<float,3>(FT,Args);}//99
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<float,4>(FT,Args);}//100
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<float,8>(FT,Args);}//101
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<float,16>(FT,Args);}//102
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv2_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<double,2>(FT,Args);}//103
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv3_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<double,3>(FT,Args);}//104
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv4_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<double,4>(FT,Args);}//105
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv8_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<double,8>(FT,Args);}//106
  BUILTINS_API llvm::GenericValue lle_X__Z9bitselectDv16_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_bitselect<double,16>(FT,Args);}//107
  BUILTINS_API llvm::GenericValue lle_X__Z8isfinitef( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<float>(FT,Args);}//108
  BUILTINS_API llvm::GenericValue lle_X__Z8isfinited( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<double>(FT,Args);}//109
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<float,2>(FT,Args);}//110
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<float,3>(FT,Args);}//111
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<float,4>(FT,Args);}//112
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<float,8>(FT,Args);}//113
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<float,16>(FT,Args);}//114
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<double,2>(FT,Args);}//115
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<double,3>(FT,Args);}//116
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<double,4>(FT,Args);}//117
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<double,8>(FT,Args);}//118
  BUILTINS_API llvm::GenericValue lle_X__Z8isfiniteDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isfinite<double,16>(FT,Args);}//119
  BUILTINS_API llvm::GenericValue lle_X__Z5isinff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<float>(FT,Args);}//120
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<double>(FT,Args);}//121
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<float,2>(FT,Args);}//122
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<float,3>(FT,Args);}//123
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<float,4>(FT,Args);}//124
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<float,8>(FT,Args);}//125
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<float,16>(FT,Args);}//126
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<double,2>(FT,Args);}//127
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<double,3>(FT,Args);}//128
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<double,4>(FT,Args);}//129
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<double,8>(FT,Args);}//130
  BUILTINS_API llvm::GenericValue lle_X__Z5isinfDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isinf<double,16>(FT,Args);}//131
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<float>(FT,Args);}//132
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterdd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<double>(FT,Args);}//133
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv2_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<float,2>(FT,Args);}//134
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<float,3>(FT,Args);}//135
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<float,4>(FT,Args);}//136
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv8_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<float,8>(FT,Args);}//137
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv16_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<float,16>(FT,Args);}//138
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv2_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<double,2>(FT,Args);}//139
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv3_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<double,3>(FT,Args);}//140
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv4_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<double,4>(FT,Args);}//141
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv8_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<double,8>(FT,Args);}//142
  BUILTINS_API llvm::GenericValue lle_X__Z13islessgreaterDv16_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_islessgreater<double,16>(FT,Args);}//143
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<float>(FT,Args);}//144
  BUILTINS_API llvm::GenericValue lle_X__Z5isnand( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<double>(FT,Args);}//145
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<float,2>(FT,Args);}//146
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<float,3>(FT,Args);}//147
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<float,4>(FT,Args);}//148
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<float,8>(FT,Args);}//149
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<float,16>(FT,Args);}//150
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<double,2>(FT,Args);}//151
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<double,3>(FT,Args);}//152
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<double,4>(FT,Args);}//153
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<double,8>(FT,Args);}//154
  BUILTINS_API llvm::GenericValue lle_X__Z5isnanDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnan<double,16>(FT,Args);}//155
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<float>(FT,Args);}//156
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormald( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<double>(FT,Args);}//157
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<float,2>(FT,Args);}//158
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<float,3>(FT,Args);}//159
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<float,4>(FT,Args);}//160
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<float,8>(FT,Args);}//161
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<float,16>(FT,Args);}//162
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<double,2>(FT,Args);}//163
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<double,3>(FT,Args);}//164
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<double,4>(FT,Args);}//165
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<double,8>(FT,Args);}//166
  BUILTINS_API llvm::GenericValue lle_X__Z8isnormalDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isnormal<double,16>(FT,Args);}//167
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<float>(FT,Args);}//168
  BUILTINS_API llvm::GenericValue lle_X__Z9isordereddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<double>(FT,Args);}//169
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv2_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<float,2>(FT,Args);}//170
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<float,3>(FT,Args);}//171
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<float,4>(FT,Args);}//172
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv8_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<float,8>(FT,Args);}//173
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv16_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<float,16>(FT,Args);}//174
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv2_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<double,2>(FT,Args);}//175
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv3_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<double,3>(FT,Args);}//176
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv4_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<double,4>(FT,Args);}//177
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv8_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<double,8>(FT,Args);}//178
  BUILTINS_API llvm::GenericValue lle_X__Z9isorderedDv16_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isordered<double,16>(FT,Args);}//179
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<float>(FT,Args);}//180
  BUILTINS_API llvm::GenericValue lle_X__Z11isunordereddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<double>(FT,Args);}//181
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv2_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<float,2>(FT,Args);}//182
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<float,3>(FT,Args);}//183
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<float,4>(FT,Args);}//184
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv8_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<float,8>(FT,Args);}//185
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv16_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<float,16>(FT,Args);}//186
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv2_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<double,2>(FT,Args);}//187
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv3_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<double,3>(FT,Args);}//188
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv4_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<double,4>(FT,Args);}//189
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv8_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<double,8>(FT,Args);}//190
  BUILTINS_API llvm::GenericValue lle_X__Z11isunorderedDv16_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_isunordered<double,16>(FT,Args);}//191
  BUILTINS_API llvm::GenericValue lle_X__Z6selectccc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,int8_t>(FT,Args);}//192
  BUILTINS_API llvm::GenericValue lle_X__Z6selecthhc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,int8_t>(FT,Args);}//193
  BUILTINS_API llvm::GenericValue lle_X__Z6selectsss( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,int16_t>(FT,Args);}//194
  BUILTINS_API llvm::GenericValue lle_X__Z6selecttts( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,int16_t>(FT,Args);}//195
  BUILTINS_API llvm::GenericValue lle_X__Z6selectiii( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,int32_t>(FT,Args);}//196
  BUILTINS_API llvm::GenericValue lle_X__Z6selectjji( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,int32_t>(FT,Args);}//197
  BUILTINS_API llvm::GenericValue lle_X__Z6selectlll( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,int64_t>(FT,Args);}//198
  BUILTINS_API llvm::GenericValue lle_X__Z6selectmml( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,int64_t>(FT,Args);}//199
  BUILTINS_API llvm::GenericValue lle_X__Z6selectffi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,int32_t>(FT,Args);}//200
  BUILTINS_API llvm::GenericValue lle_X__Z6selectddl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,int64_t>(FT,Args);}//201
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,int8_t,2>(FT,Args);}//202
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,int8_t,3>(FT,Args);}//203
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,int8_t,4>(FT,Args);}//204
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,int8_t,8>(FT,Args);}//205
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_cS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,int8_t,16>(FT,Args);}//206
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_hS_Dv2_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,int8_t,2>(FT,Args);}//207
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_hS_Dv3_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,int8_t,3>(FT,Args);}//208
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_hS_Dv4_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,int8_t,4>(FT,Args);}//209
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_hS_Dv8_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,int8_t,8>(FT,Args);}//210
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_hS_Dv16_c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,int8_t,16>(FT,Args);}//211
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,int16_t,2>(FT,Args);}//212
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,int16_t,3>(FT,Args);}//213
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,int16_t,4>(FT,Args);}//214
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,int16_t,8>(FT,Args);}//215
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_sS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,int16_t,16>(FT,Args);}//216
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_tS_Dv2_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,int16_t,2>(FT,Args);}//217
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_tS_Dv3_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,int16_t,3>(FT,Args);}//218
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_tS_Dv4_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,int16_t,4>(FT,Args);}//219
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_tS_Dv8_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,int16_t,8>(FT,Args);}//220
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_tS_Dv16_s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,int16_t,16>(FT,Args);}//221
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,int32_t,2>(FT,Args);}//222
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,int32_t,3>(FT,Args);}//223
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,int32_t,4>(FT,Args);}//224
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,int32_t,8>(FT,Args);}//225
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_iS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,int32_t,16>(FT,Args);}//226
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_jS_Dv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,int32_t,2>(FT,Args);}//227
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_jS_Dv3_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,int32_t,3>(FT,Args);}//228
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_jS_Dv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,int32_t,4>(FT,Args);}//229
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_jS_Dv8_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,int32_t,8>(FT,Args);}//230
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_jS_Dv16_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,int32_t,16>(FT,Args);}//231
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,int64_t,2>(FT,Args);}//232
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,int64_t,3>(FT,Args);}//233
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,int64_t,4>(FT,Args);}//234
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,int64_t,8>(FT,Args);}//235
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_lS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,int64_t,16>(FT,Args);}//236
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_mS_Dv2_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,int64_t,2>(FT,Args);}//237
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_mS_Dv3_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,int64_t,3>(FT,Args);}//238
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_mS_Dv4_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,int64_t,4>(FT,Args);}//239
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_mS_Dv8_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,int64_t,8>(FT,Args);}//240
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_mS_Dv16_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,int64_t,16>(FT,Args);}//241
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_fS_Dv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,int32_t,2>(FT,Args);}//242
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_fS_Dv3_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,int32_t,3>(FT,Args);}//243
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_fS_Dv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,int32_t,4>(FT,Args);}//244
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_fS_Dv8_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,int32_t,8>(FT,Args);}//245
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_fS_Dv16_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,int32_t,16>(FT,Args);}//246
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_dS_Dv2_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,int64_t,2>(FT,Args);}//247
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_dS_Dv3_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,int64_t,3>(FT,Args);}//248
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_dS_Dv4_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,int64_t,4>(FT,Args);}//249
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_dS_Dv8_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,int64_t,8>(FT,Args);}//250
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_dS_Dv16_l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,int64_t,16>(FT,Args);}//251
  BUILTINS_API llvm::GenericValue lle_X__Z6selectcch( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,uint8_t>(FT,Args);}//252
  BUILTINS_API llvm::GenericValue lle_X__Z6selecthhh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,uint8_t>(FT,Args);}//253
  BUILTINS_API llvm::GenericValue lle_X__Z6selectsst( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,uint16_t>(FT,Args);}//254
  BUILTINS_API llvm::GenericValue lle_X__Z6selectttt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,uint16_t>(FT,Args);}//255
  BUILTINS_API llvm::GenericValue lle_X__Z6selectiij( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,uint32_t>(FT,Args);}//256
  BUILTINS_API llvm::GenericValue lle_X__Z6selectjjj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,uint32_t>(FT,Args);}//257
  BUILTINS_API llvm::GenericValue lle_X__Z6selectllm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,uint64_t>(FT,Args);}//258
  BUILTINS_API llvm::GenericValue lle_X__Z6selectmmm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,uint64_t>(FT,Args);}//259
  BUILTINS_API llvm::GenericValue lle_X__Z6selectffj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,uint32_t>(FT,Args);}//260
  BUILTINS_API llvm::GenericValue lle_X__Z6selectddm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,uint64_t>(FT,Args);}//261
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_cS_Dv2_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,uint8_t,2>(FT,Args);}//262
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_cS_Dv3_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,uint8_t,3>(FT,Args);}//263
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_cS_Dv4_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,uint8_t,4>(FT,Args);}//264
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_cS_Dv8_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,uint8_t,8>(FT,Args);}//265
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_cS_Dv16_h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int8_t,uint8_t,16>(FT,Args);}//266
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,uint8_t,2>(FT,Args);}//267
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,uint8_t,3>(FT,Args);}//268
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,uint8_t,4>(FT,Args);}//269
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,uint8_t,8>(FT,Args);}//270
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_hS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint8_t,uint8_t,16>(FT,Args);}//271
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_sS_Dv2_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,uint16_t,2>(FT,Args);}//272
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_sS_Dv3_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,uint16_t,3>(FT,Args);}//273
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_sS_Dv4_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,uint16_t,4>(FT,Args);}//274
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_sS_Dv8_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,uint16_t,8>(FT,Args);}//275
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_sS_Dv16_t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int16_t,uint16_t,16>(FT,Args);}//276
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,uint16_t,2>(FT,Args);}//277
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,uint16_t,3>(FT,Args);}//278
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,uint16_t,4>(FT,Args);}//279
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,uint16_t,8>(FT,Args);}//280
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_tS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint16_t,uint16_t,16>(FT,Args);}//281
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_iS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,uint32_t,2>(FT,Args);}//282
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_iS_Dv3_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,uint32_t,3>(FT,Args);}//283
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_iS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,uint32_t,4>(FT,Args);}//284
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_iS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,uint32_t,8>(FT,Args);}//285
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_iS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int32_t,uint32_t,16>(FT,Args);}//286
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,uint32_t,2>(FT,Args);}//287
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,uint32_t,3>(FT,Args);}//288
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,uint32_t,4>(FT,Args);}//289
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,uint32_t,8>(FT,Args);}//290
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_jS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint32_t,uint32_t,16>(FT,Args);}//291
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_lS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,uint64_t,2>(FT,Args);}//292
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_lS_Dv3_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,uint64_t,3>(FT,Args);}//293
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_lS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,uint64_t,4>(FT,Args);}//294
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_lS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,uint64_t,8>(FT,Args);}//295
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_lS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<int64_t,uint64_t,16>(FT,Args);}//296
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,uint64_t,2>(FT,Args);}//297
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,uint64_t,3>(FT,Args);}//298
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,uint64_t,4>(FT,Args);}//299
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,uint64_t,8>(FT,Args);}//300
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_mS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<uint64_t,uint64_t,16>(FT,Args);}//301
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_fS_Dv2_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,uint32_t,2>(FT,Args);}//302
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_fS_Dv3_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,uint32_t,3>(FT,Args);}//303
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_fS_Dv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,uint32_t,4>(FT,Args);}//304
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_fS_Dv8_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,uint32_t,8>(FT,Args);}//305
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_fS_Dv16_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<float,uint32_t,16>(FT,Args);}//306
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv2_dS_Dv2_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,uint64_t,2>(FT,Args);}//307
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv3_dS_Dv3_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,uint64_t,3>(FT,Args);}//308
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv4_dS_Dv4_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,uint64_t,4>(FT,Args);}//309
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv8_dS_Dv8_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,uint64_t,8>(FT,Args);}//310
  BUILTINS_API llvm::GenericValue lle_X__Z6selectDv16_dS_Dv16_m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_select<double,uint64_t,16>(FT,Args);}//311
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<float>(FT,Args);}//312
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<double>(FT,Args);}//313
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<float,2>(FT,Args);}//314
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<float,3>(FT,Args);}//315
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<float,4>(FT,Args);}//316
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<float,8>(FT,Args);}//317
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<float,16>(FT,Args);}//318
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<double,2>(FT,Args);}//319
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<double,3>(FT,Args);}//320
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<double,4>(FT,Args);}//321
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<double,8>(FT,Args);}//322
  BUILTINS_API llvm::GenericValue lle_X__Z7signbitDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_signbit<double,16>(FT,Args);}//323


}

namespace Validation {
namespace OCLBuiltins {
template<>
    llvm::GenericValue localBitselect( float inA, float inB, float inC )
    {
        llvm::GenericValue R;
        union {uint32_t u; float f;} a, b, c, out;
        a.f = inA;
        b.f = inB;
        c.f = inC;
        out.u = ( a.u & ~c.u ) | ( b.u & c.u );
        getRef<float>(R) = out.f;
        return R;
    }
    template<>
    llvm::GenericValue localBitselect( double inA, double inB, double inC )
    {
        llvm::GenericValue R;
        union {uint64_t u; double f;} a, b, c, out;
        a.f = inA;
        b.f = inB;
        c.f = inC;
        out.u = ( a.u & c.u ) | ( b.u & c.u );
        getRef<double>(R) = out.f;
        return R;
    }

    template<>
    llvm::GenericValue selectResult( float inC )
    {
        llvm::GenericValue R;
        getRef<float>(R) = inC;
        return R;
    }
    template<>
    llvm::GenericValue selectResult( double inC )
    {
        llvm::GenericValue R;
        getRef<double>(R) = inC;
        return R;
    }

}
}  
  
