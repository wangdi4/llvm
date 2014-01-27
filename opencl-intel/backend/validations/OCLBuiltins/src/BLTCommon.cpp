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

File Name:  BLTCommon.cpp

\*****************************************************************************/

#include "BLTCommon.h"

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
BUILTINS_API void initOCLBuiltinsCommon() {return;}
  

  BUILTINS_API llvm::GenericValue lle_X__Z5clampfff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,1,1>(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv2_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,2,2>(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv3_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,3,3>(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv4_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,4,4>(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv8_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,8,8>(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv16_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,16,16>(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z5clampddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,1,1>(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv2_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,2,2>(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv3_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,3,3>(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv4_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,4,4>(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv8_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,8,8>(FT,Args);}//10
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv16_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,16,16>(FT,Args);}//11
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv2_fff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,2,1>(FT,Args);}//12
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv3_fff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,3,1>(FT,Args);}//13
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv4_fff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,4,1>(FT,Args);}//14
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv8_fff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,8,1>(FT,Args);}//15
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv16_fff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<float,16,1>(FT,Args);}//16
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv2_ddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,2,1>(FT,Args);}//17
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv3_ddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,3,1>(FT,Args);}//18
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv4_ddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,4,1>(FT,Args);}//19
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv8_ddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,8,1>(FT,Args);}//20
  BUILTINS_API llvm::GenericValue lle_X__Z5clampDv16_ddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_clamp<double,16,1>(FT,Args);}//21
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<float,1>(FT,Args);}//22
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<float,2>(FT,Args);}//23
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<float,3>(FT,Args);}//24
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<float,4>(FT,Args);}//25
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<float,8>(FT,Args);}//26
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<float,16>(FT,Args);}//27
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<double,1>(FT,Args);}//28
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<double,2>(FT,Args);}//29
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<double,3>(FT,Args);}//30
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<double,4>(FT,Args);}//31
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<double,8>(FT,Args);}//32
  BUILTINS_API llvm::GenericValue lle_X__Z7degreesDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_degrees<double,16>(FT,Args);}//33
  BUILTINS_API llvm::GenericValue lle_X__Z3maxff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,1,1>(FT,Args);}//34
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv2_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,2,2>(FT,Args);}//35
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,3,3>(FT,Args);}//36
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,4,4>(FT,Args);}//37
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv8_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,8,8>(FT,Args);}//38
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv16_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,16,16>(FT,Args);}//39
  BUILTINS_API llvm::GenericValue lle_X__Z3maxdd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,1,1>(FT,Args);}//40
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv2_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,2,2>(FT,Args);}//41
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv3_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,3,3>(FT,Args);}//42
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv4_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,4,4>(FT,Args);}//43
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv8_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,8,8>(FT,Args);}//44
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv16_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,16,16>(FT,Args);}//45
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv2_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,2,1>(FT,Args);}//46
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv3_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,3,1>(FT,Args);}//47
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv4_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,4,1>(FT,Args);}//48
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv8_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,8,1>(FT,Args);}//49
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv16_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<float,16,1>(FT,Args);}//50
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv2_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,2,1>(FT,Args);}//51
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv3_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,3,1>(FT,Args);}//52
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv4_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,4,1>(FT,Args);}//53
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv8_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,8,1>(FT,Args);}//54
  BUILTINS_API llvm::GenericValue lle_X__Z3maxDv16_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_max<double,16,1>(FT,Args);}//55
  BUILTINS_API llvm::GenericValue lle_X__Z3minff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,1,1>(FT,Args);}//56
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv2_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,2,2>(FT,Args);}//57
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,3,3>(FT,Args);}//58
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,4,4>(FT,Args);}//59
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv8_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,8,8>(FT,Args);}//60
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv16_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,16,16>(FT,Args);}//61
  BUILTINS_API llvm::GenericValue lle_X__Z3mindd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,1,1>(FT,Args);}//62
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv2_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,2,2>(FT,Args);}//63
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv3_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,3,3>(FT,Args);}//64
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv4_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,4,4>(FT,Args);}//65
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv8_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,8,8>(FT,Args);}//66
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv16_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,16,16>(FT,Args);}//67
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv2_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,2,1>(FT,Args);}//68
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv3_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,3,1>(FT,Args);}//69
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv4_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,4,1>(FT,Args);}//70
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv8_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,8,1>(FT,Args);}//71
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv16_ff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<float,16,1>(FT,Args);}//72
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv2_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,2,1>(FT,Args);}//73
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv3_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,3,1>(FT,Args);}//74
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv4_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,4,1>(FT,Args);}//75
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv8_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,8,1>(FT,Args);}//76
  BUILTINS_API llvm::GenericValue lle_X__Z3minDv16_dd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_min<double,16,1>(FT,Args);}//77
  BUILTINS_API llvm::GenericValue lle_X__Z3mixfff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,1,1>(FT,Args);}//78
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv2_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,2,2>(FT,Args);}//79
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv3_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,3,3>(FT,Args);}//80
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv4_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,4,4>(FT,Args);}//81
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv8_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,8,8>(FT,Args);}//82
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv16_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,16,16>(FT,Args);}//83
  BUILTINS_API llvm::GenericValue lle_X__Z3mixddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,1,1>(FT,Args);}//84
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv2_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,2,2>(FT,Args);}//85
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv3_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,3,3>(FT,Args);}//86
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv4_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,4,4>(FT,Args);}//87
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv8_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,8,8>(FT,Args);}//88
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv16_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,16,16>(FT,Args);}//89
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv2_fS_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,2,1>(FT,Args);}//90
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv3_fS_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,3,1>(FT,Args);}//91
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv4_fS_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,4,1>(FT,Args);}//92
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv8_fS_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,8,1>(FT,Args);}//93
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv16_fS_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<float,16,1>(FT,Args);}//94
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv2_dS_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,2,1>(FT,Args);}//95
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv3_dS_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,3,1>(FT,Args);}//96
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv4_dS_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,4,1>(FT,Args);}//97
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv8_dS_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,8,1>(FT,Args);}//98
  BUILTINS_API llvm::GenericValue lle_X__Z3mixDv16_dS_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_mix<double,16,1>(FT,Args);}//99
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<float,1>(FT,Args);}//100
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<float,2>(FT,Args);}//101
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<float,3>(FT,Args);}//102
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<float,4>(FT,Args);}//103
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<float,8>(FT,Args);}//104
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<float,16>(FT,Args);}//105
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<double,1>(FT,Args);}//106
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<double,2>(FT,Args);}//107
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<double,3>(FT,Args);}//108
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<double,4>(FT,Args);}//109
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<double,8>(FT,Args);}//110
  BUILTINS_API llvm::GenericValue lle_X__Z7radiansDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_radians<double,16>(FT,Args);}//111
  BUILTINS_API llvm::GenericValue lle_X__Z4signf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<float,1>(FT,Args);}//112
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<float,2>(FT,Args);}//113
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<float,3>(FT,Args);}//114
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<float,4>(FT,Args);}//115
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<float,8>(FT,Args);}//116
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<float,16>(FT,Args);}//117
  BUILTINS_API llvm::GenericValue lle_X__Z4signd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<double,1>(FT,Args);}//118
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<double,2>(FT,Args);}//119
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<double,3>(FT,Args);}//120
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<double,4>(FT,Args);}//121
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<double,8>(FT,Args);}//122
  BUILTINS_API llvm::GenericValue lle_X__Z4signDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_sign<double,16>(FT,Args);}//123
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepfff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,1,1>(FT,Args);}//124
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv2_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,2,2>(FT,Args);}//125
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv3_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,3,3>(FT,Args);}//126
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv4_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,4,4>(FT,Args);}//127
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv8_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,8,8>(FT,Args);}//128
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv16_fS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,16,16>(FT,Args);}//129
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepddd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,1,1>(FT,Args);}//130
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv2_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,2,2>(FT,Args);}//131
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv3_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,3,3>(FT,Args);}//132
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv4_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,4,4>(FT,Args);}//133
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv8_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,8,8>(FT,Args);}//134
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepDv16_dS_S_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,16,16>(FT,Args);}//135
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepffDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,1,2>(FT,Args);}//136
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepffDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,1,3>(FT,Args);}//137
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepffDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,1,4>(FT,Args);}//138
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepffDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,1,8>(FT,Args);}//139
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepffDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<float,1,16>(FT,Args);}//140
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepddDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,1,2>(FT,Args);}//141
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepddDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,1,3>(FT,Args);}//142
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepddDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,1,4>(FT,Args);}//143
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepddDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,1,8>(FT,Args);}//144
  BUILTINS_API llvm::GenericValue lle_X__Z10smoothstepddDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_smoothstep<double,1,16>(FT,Args);}//145
  BUILTINS_API llvm::GenericValue lle_X__Z4stepff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,1,1>(FT,Args);}//146
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv2_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,2,2>(FT,Args);}//147
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,3,3>(FT,Args);}//148
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,4,4>(FT,Args);}//149
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv8_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,8,8>(FT,Args);}//150
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv16_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,16,16>(FT,Args);}//151
  BUILTINS_API llvm::GenericValue lle_X__Z4stepdd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,1,1>(FT,Args);}//152
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv2_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,2,2>(FT,Args);}//153
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv3_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,3,3>(FT,Args);}//154
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv4_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,4,4>(FT,Args);}//155
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv8_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,8,8>(FT,Args);}//156
  BUILTINS_API llvm::GenericValue lle_X__Z4stepDv16_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,16,16>(FT,Args);}//157
  BUILTINS_API llvm::GenericValue lle_X__Z4stepfDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,1,2>(FT,Args);}//158
  BUILTINS_API llvm::GenericValue lle_X__Z4stepfDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,1,3>(FT,Args);}//159
  BUILTINS_API llvm::GenericValue lle_X__Z4stepfDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,1,4>(FT,Args);}//160
  BUILTINS_API llvm::GenericValue lle_X__Z4stepfDv8_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,1,8>(FT,Args);}//161
  BUILTINS_API llvm::GenericValue lle_X__Z4stepfDv16_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<float,1,16>(FT,Args);}//162
  BUILTINS_API llvm::GenericValue lle_X__Z4stepdDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,1,2>(FT,Args);}//163
  BUILTINS_API llvm::GenericValue lle_X__Z4stepdDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,1,3>(FT,Args);}//164
  BUILTINS_API llvm::GenericValue lle_X__Z4stepdDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,1,4>(FT,Args);}//165
  BUILTINS_API llvm::GenericValue lle_X__Z4stepdDv8_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,1,8>(FT,Args);}//166
  BUILTINS_API llvm::GenericValue lle_X__Z4stepdDv16_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_step<double,1,16>(FT,Args);}//167


}

  
