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

File Name:  BLTGeometric.cpp

\*****************************************************************************/

#include "BLTGeometric.h"

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
BUILTINS_API void initOCLBuiltinsGeometric() {return;}
  

  BUILTINS_API llvm::GenericValue lle_X__Z5crossDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_cross<float,3>(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z5crossDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_cross<float,4>(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z5crossDv3_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_cross<double,3>(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z5crossDv4_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_cross<double,4>(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z8distanceff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<float,1>(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z8distanceDv2_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<float,2>(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z8distanceDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<float,3>(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z8distanceDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<float,4>(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z8distancedd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<double,1>(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z8distanceDv2_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<double,2>(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z8distanceDv3_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<double,3>(FT,Args);}//10
  BUILTINS_API llvm::GenericValue lle_X__Z8distanceDv4_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<double,4>(FT,Args);}//11
  BUILTINS_API llvm::GenericValue lle_X__Z3dotff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_dot<float,1>(FT,Args);}//12
  BUILTINS_API llvm::GenericValue lle_X__Z3dotDv2_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_dot<float,2>(FT,Args);}//13
  BUILTINS_API llvm::GenericValue lle_X__Z3dotDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_dot<float,3>(FT,Args);}//14
  BUILTINS_API llvm::GenericValue lle_X__Z3dotDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_dot<float,4>(FT,Args);}//15
  BUILTINS_API llvm::GenericValue lle_X__Z3dotdd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_dot<double,1>(FT,Args);}//16
  BUILTINS_API llvm::GenericValue lle_X__Z3dotDv2_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_dot<double,2>(FT,Args);}//17
  BUILTINS_API llvm::GenericValue lle_X__Z3dotDv3_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_dot<double,3>(FT,Args);}//18
  BUILTINS_API llvm::GenericValue lle_X__Z3dotDv4_dS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_dot<double,4>(FT,Args);}//19
  BUILTINS_API llvm::GenericValue lle_X__Z13fast_distanceff( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<float,1>(FT,Args);}//20
  BUILTINS_API llvm::GenericValue lle_X__Z13fast_distanceDv2_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<float,2>(FT,Args);}//21
  BUILTINS_API llvm::GenericValue lle_X__Z13fast_distanceDv3_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<float,3>(FT,Args);}//22
  BUILTINS_API llvm::GenericValue lle_X__Z13fast_distanceDv4_fS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_distance<float,4>(FT,Args);}//23
  BUILTINS_API llvm::GenericValue lle_X__Z11fast_lengthf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<float,1>(FT,Args);}//24
  BUILTINS_API llvm::GenericValue lle_X__Z11fast_lengthDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<float,2>(FT,Args);}//25
  BUILTINS_API llvm::GenericValue lle_X__Z11fast_lengthDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<float,3>(FT,Args);}//26
  BUILTINS_API llvm::GenericValue lle_X__Z11fast_lengthDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<float,4>(FT,Args);}//27
  BUILTINS_API llvm::GenericValue lle_X__Z14fast_normalizef( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<float,1>(FT,Args);}//28
  BUILTINS_API llvm::GenericValue lle_X__Z14fast_normalizeDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<float,2>(FT,Args);}//29
  BUILTINS_API llvm::GenericValue lle_X__Z14fast_normalizeDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<float,3>(FT,Args);}//30
  BUILTINS_API llvm::GenericValue lle_X__Z14fast_normalizeDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<float,4>(FT,Args);}//31
  BUILTINS_API llvm::GenericValue lle_X__Z6lengthf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<float,1>(FT,Args);}//32
  BUILTINS_API llvm::GenericValue lle_X__Z6lengthDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<float,2>(FT,Args);}//33
  BUILTINS_API llvm::GenericValue lle_X__Z6lengthDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<float,3>(FT,Args);}//34
  BUILTINS_API llvm::GenericValue lle_X__Z6lengthDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<float,4>(FT,Args);}//35
  BUILTINS_API llvm::GenericValue lle_X__Z6lengthd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<double,1>(FT,Args);}//36
  BUILTINS_API llvm::GenericValue lle_X__Z6lengthDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<double,2>(FT,Args);}//37
  BUILTINS_API llvm::GenericValue lle_X__Z6lengthDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<double,3>(FT,Args);}//38
  BUILTINS_API llvm::GenericValue lle_X__Z6lengthDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_length<double,4>(FT,Args);}//39
  BUILTINS_API llvm::GenericValue lle_X__Z9normalizef( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<float,1>(FT,Args);}//40
  BUILTINS_API llvm::GenericValue lle_X__Z9normalizeDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<float,2>(FT,Args);}//41
  BUILTINS_API llvm::GenericValue lle_X__Z9normalizeDv3_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<float,3>(FT,Args);}//42
  BUILTINS_API llvm::GenericValue lle_X__Z9normalizeDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<float,4>(FT,Args);}//43
  BUILTINS_API llvm::GenericValue lle_X__Z9normalized( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<double,1>(FT,Args);}//44
  BUILTINS_API llvm::GenericValue lle_X__Z9normalizeDv2_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<double,2>(FT,Args);}//45
  BUILTINS_API llvm::GenericValue lle_X__Z9normalizeDv3_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<double,3>(FT,Args);}//46
  BUILTINS_API llvm::GenericValue lle_X__Z9normalizeDv4_d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_normalize<double,4>(FT,Args);}//47


}

  
