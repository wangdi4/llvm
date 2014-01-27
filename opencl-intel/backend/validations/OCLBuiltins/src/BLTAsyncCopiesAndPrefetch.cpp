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

File Name:  BLTAsyncCopiesAndPrefetch.cpp

\*****************************************************************************/

#include "BLTAsyncCopiesAndPrefetch.h"

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
llvm::GenericValue lle_X_prefetch(llvm::FunctionType *FT,
                                  const std::vector<llvm::GenericValue> &Args)
{
    return GenericValue();
}

llvm::GenericValue lle_X_wait_group_events(llvm::FunctionType *FT,
                                  const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    size_t num_events = getVal<size_t>(arg0);
    size_t* event_list = static_cast<size_t*>(arg1.PointerVal);
    for (size_t i = 0; i < num_events; ++i, ++event_list)
    {
        *event_list = 0;
    }
    return GenericValue();
}
}
}
extern "C" {
BUILTINS_API void initOCLBuiltinsAsync() {return;}
  

  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3cPKU3AS1cm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,1>(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_cPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,2>(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_cPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,3>(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_cPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,4>(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_cPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,8>(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_cPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,16>(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3hPKU3AS1hm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,1>(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_hPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,2>(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_hPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,3>(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_hPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,4>(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_hPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,8>(FT,Args);}//10
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_hPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,16>(FT,Args);}//11
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3sPKU3AS1sm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,1>(FT,Args);}//12
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_sPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,2>(FT,Args);}//13
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_sPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,3>(FT,Args);}//14
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_sPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,4>(FT,Args);}//15
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_sPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,8>(FT,Args);}//16
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_sPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,16>(FT,Args);}//17
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3tPKU3AS1tm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,1>(FT,Args);}//18
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_tPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,2>(FT,Args);}//19
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_tPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,3>(FT,Args);}//20
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_tPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,4>(FT,Args);}//21
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_tPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,8>(FT,Args);}//22
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_tPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,16>(FT,Args);}//23
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3iPKU3AS1im9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,1>(FT,Args);}//24
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_iPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,2>(FT,Args);}//25
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_iPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,3>(FT,Args);}//26
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_iPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,4>(FT,Args);}//27
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_iPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,8>(FT,Args);}//28
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_iPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,16>(FT,Args);}//29
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3jPKU3AS1jm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,1>(FT,Args);}//30
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_jPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,2>(FT,Args);}//31
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_jPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,3>(FT,Args);}//32
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_jPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,4>(FT,Args);}//33
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_jPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,8>(FT,Args);}//34
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_jPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,16>(FT,Args);}//35
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3lPKU3AS1lm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,1>(FT,Args);}//36
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_lPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,2>(FT,Args);}//37
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_lPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,3>(FT,Args);}//38
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_lPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,4>(FT,Args);}//39
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_lPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,8>(FT,Args);}//40
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_lPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,16>(FT,Args);}//41
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3mPKU3AS1mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,1>(FT,Args);}//42
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_mPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,2>(FT,Args);}//43
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_mPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,3>(FT,Args);}//44
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_mPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,4>(FT,Args);}//45
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_mPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,8>(FT,Args);}//46
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_mPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,16>(FT,Args);}//47
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3fPKU3AS1fm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,1>(FT,Args);}//48
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_fPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,2>(FT,Args);}//49
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_fPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,3>(FT,Args);}//50
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_fPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,4>(FT,Args);}//51
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_fPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,8>(FT,Args);}//52
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_fPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,16>(FT,Args);}//53
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3dPKU3AS1dm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,1>(FT,Args);}//54
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv2_dPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,2>(FT,Args);}//55
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv3_dPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,3>(FT,Args);}//56
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv4_dPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,4>(FT,Args);}//57
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv8_dPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,8>(FT,Args);}//58
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS3Dv16_dPKU3AS1S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,16>(FT,Args);}//59
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1cPKU3AS3cm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,1>(FT,Args);}//60
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_cPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,2>(FT,Args);}//61
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_cPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,3>(FT,Args);}//62
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_cPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,4>(FT,Args);}//63
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_cPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,8>(FT,Args);}//64
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_cPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int8_t,16>(FT,Args);}//65
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1hPKU3AS3hm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,1>(FT,Args);}//66
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_hPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,2>(FT,Args);}//67
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_hPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,3>(FT,Args);}//68
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_hPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,4>(FT,Args);}//69
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_hPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,8>(FT,Args);}//70
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_hPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint8_t,16>(FT,Args);}//71
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1sPKU3AS3sm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,1>(FT,Args);}//72
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_sPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,2>(FT,Args);}//73
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_sPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,3>(FT,Args);}//74
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_sPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,4>(FT,Args);}//75
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_sPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,8>(FT,Args);}//76
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_sPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int16_t,16>(FT,Args);}//77
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1tPKU3AS3tm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,1>(FT,Args);}//78
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_tPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,2>(FT,Args);}//79
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_tPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,3>(FT,Args);}//80
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_tPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,4>(FT,Args);}//81
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_tPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,8>(FT,Args);}//82
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_tPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint16_t,16>(FT,Args);}//83
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1iPKU3AS3im9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,1>(FT,Args);}//84
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_iPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,2>(FT,Args);}//85
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_iPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,3>(FT,Args);}//86
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_iPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,4>(FT,Args);}//87
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_iPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,8>(FT,Args);}//88
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_iPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int32_t,16>(FT,Args);}//89
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1jPKU3AS3jm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,1>(FT,Args);}//90
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_jPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,2>(FT,Args);}//91
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_jPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,3>(FT,Args);}//92
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_jPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,4>(FT,Args);}//93
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_jPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,8>(FT,Args);}//94
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_jPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint32_t,16>(FT,Args);}//95
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1lPKU3AS3lm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,1>(FT,Args);}//96
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_lPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,2>(FT,Args);}//97
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_lPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,3>(FT,Args);}//98
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_lPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,4>(FT,Args);}//99
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_lPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,8>(FT,Args);}//100
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_lPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<int64_t,16>(FT,Args);}//101
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1mPKU3AS3mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,1>(FT,Args);}//102
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_mPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,2>(FT,Args);}//103
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_mPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,3>(FT,Args);}//104
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_mPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,4>(FT,Args);}//105
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_mPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,8>(FT,Args);}//106
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_mPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<uint64_t,16>(FT,Args);}//107
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1fPKU3AS3fm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,1>(FT,Args);}//108
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_fPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,2>(FT,Args);}//109
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_fPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,3>(FT,Args);}//110
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_fPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,4>(FT,Args);}//111
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_fPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,8>(FT,Args);}//112
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_fPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<float,16>(FT,Args);}//113
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1dPKU3AS3dm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,1>(FT,Args);}//114
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv2_dPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,2>(FT,Args);}//115
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv3_dPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,3>(FT,Args);}//116
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv4_dPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,4>(FT,Args);}//117
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv8_dPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,8>(FT,Args);}//118
  BUILTINS_API llvm::GenericValue lle_X__Z21async_work_group_copyPU3AS1Dv16_dPKU3AS3S_m9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_copy<double,16>(FT,Args);}//119
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3cPKU3AS1cmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int8_t,1>(FT,Args);}//120
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_cPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int8_t,2>(FT,Args);}//121
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_cPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int8_t,3>(FT,Args);}//122
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_cPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int8_t,4>(FT,Args);}//123
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_cPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int8_t,8>(FT,Args);}//124
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_cPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int8_t,16>(FT,Args);}//125
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3hPKU3AS1hmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint8_t,1>(FT,Args);}//126
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_hPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint8_t,2>(FT,Args);}//127
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_hPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint8_t,3>(FT,Args);}//128
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_hPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint8_t,4>(FT,Args);}//129
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_hPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint8_t,8>(FT,Args);}//130
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_hPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint8_t,16>(FT,Args);}//131
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3sPKU3AS1smm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int16_t,1>(FT,Args);}//132
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_sPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int16_t,2>(FT,Args);}//133
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_sPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int16_t,3>(FT,Args);}//134
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_sPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int16_t,4>(FT,Args);}//135
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_sPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int16_t,8>(FT,Args);}//136
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_sPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int16_t,16>(FT,Args);}//137
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3tPKU3AS1tmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint16_t,1>(FT,Args);}//138
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_tPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint16_t,2>(FT,Args);}//139
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_tPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint16_t,3>(FT,Args);}//140
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_tPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint16_t,4>(FT,Args);}//141
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_tPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint16_t,8>(FT,Args);}//142
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_tPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint16_t,16>(FT,Args);}//143
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3iPKU3AS1imm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int32_t,1>(FT,Args);}//144
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_iPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int32_t,2>(FT,Args);}//145
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_iPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int32_t,3>(FT,Args);}//146
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_iPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int32_t,4>(FT,Args);}//147
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_iPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int32_t,8>(FT,Args);}//148
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_iPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int32_t,16>(FT,Args);}//149
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3jPKU3AS1jmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint32_t,1>(FT,Args);}//150
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_jPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint32_t,2>(FT,Args);}//151
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_jPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint32_t,3>(FT,Args);}//152
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_jPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint32_t,4>(FT,Args);}//153
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_jPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint32_t,8>(FT,Args);}//154
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_jPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint32_t,16>(FT,Args);}//155
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3lPKU3AS1lmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int64_t,1>(FT,Args);}//156
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_lPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int64_t,2>(FT,Args);}//157
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_lPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int64_t,3>(FT,Args);}//158
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_lPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int64_t,4>(FT,Args);}//159
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_lPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int64_t,8>(FT,Args);}//160
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_lPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<int64_t,16>(FT,Args);}//161
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3mPKU3AS1mmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint64_t,1>(FT,Args);}//162
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_mPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint64_t,2>(FT,Args);}//163
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_mPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint64_t,3>(FT,Args);}//164
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_mPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint64_t,4>(FT,Args);}//165
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_mPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint64_t,8>(FT,Args);}//166
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_mPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<uint64_t,16>(FT,Args);}//167
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3fPKU3AS1fmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<float,1>(FT,Args);}//168
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_fPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<float,2>(FT,Args);}//169
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_fPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<float,3>(FT,Args);}//170
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_fPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<float,4>(FT,Args);}//171
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_fPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<float,8>(FT,Args);}//172
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_fPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<float,16>(FT,Args);}//173
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3dPKU3AS1dmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<double,1>(FT,Args);}//174
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv2_dPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<double,2>(FT,Args);}//175
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv3_dPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<double,3>(FT,Args);}//176
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv4_dPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<double,4>(FT,Args);}//177
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv8_dPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<double,8>(FT,Args);}//178
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS3Dv16_dPKU3AS1S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_g2l<double,16>(FT,Args);}//179
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1cPKU3AS3cmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int8_t,1>(FT,Args);}//180
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_cPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int8_t,2>(FT,Args);}//181
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_cPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int8_t,3>(FT,Args);}//182
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_cPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int8_t,4>(FT,Args);}//183
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_cPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int8_t,8>(FT,Args);}//184
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_cPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int8_t,16>(FT,Args);}//185
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1hPKU3AS3hmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint8_t,1>(FT,Args);}//186
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_hPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint8_t,2>(FT,Args);}//187
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_hPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint8_t,3>(FT,Args);}//188
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_hPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint8_t,4>(FT,Args);}//189
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_hPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint8_t,8>(FT,Args);}//190
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_hPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint8_t,16>(FT,Args);}//191
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1sPKU3AS3smm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int16_t,1>(FT,Args);}//192
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_sPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int16_t,2>(FT,Args);}//193
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_sPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int16_t,3>(FT,Args);}//194
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_sPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int16_t,4>(FT,Args);}//195
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_sPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int16_t,8>(FT,Args);}//196
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_sPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int16_t,16>(FT,Args);}//197
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1tPKU3AS3tmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint16_t,1>(FT,Args);}//198
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_tPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint16_t,2>(FT,Args);}//199
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_tPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint16_t,3>(FT,Args);}//200
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_tPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint16_t,4>(FT,Args);}//201
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_tPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint16_t,8>(FT,Args);}//202
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_tPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint16_t,16>(FT,Args);}//203
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1iPKU3AS3imm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int32_t,1>(FT,Args);}//204
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_iPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int32_t,2>(FT,Args);}//205
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_iPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int32_t,3>(FT,Args);}//206
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_iPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int32_t,4>(FT,Args);}//207
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_iPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int32_t,8>(FT,Args);}//208
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_iPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int32_t,16>(FT,Args);}//209
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1jPKU3AS3jmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint32_t,1>(FT,Args);}//210
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_jPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint32_t,2>(FT,Args);}//211
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_jPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint32_t,3>(FT,Args);}//212
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_jPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint32_t,4>(FT,Args);}//213
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_jPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint32_t,8>(FT,Args);}//214
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_jPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint32_t,16>(FT,Args);}//215
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1lPKU3AS3lmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int64_t,1>(FT,Args);}//216
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_lPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int64_t,2>(FT,Args);}//217
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_lPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int64_t,3>(FT,Args);}//218
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_lPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int64_t,4>(FT,Args);}//219
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_lPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int64_t,8>(FT,Args);}//220
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_lPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<int64_t,16>(FT,Args);}//221
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1mPKU3AS3mmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint64_t,1>(FT,Args);}//222
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_mPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint64_t,2>(FT,Args);}//223
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_mPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint64_t,3>(FT,Args);}//224
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_mPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint64_t,4>(FT,Args);}//225
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_mPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint64_t,8>(FT,Args);}//226
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_mPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<uint64_t,16>(FT,Args);}//227
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1fPKU3AS3fmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<float,1>(FT,Args);}//228
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_fPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<float,2>(FT,Args);}//229
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_fPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<float,3>(FT,Args);}//230
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_fPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<float,4>(FT,Args);}//231
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_fPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<float,8>(FT,Args);}//232
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_fPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<float,16>(FT,Args);}//233
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1dPKU3AS3dmm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<double,1>(FT,Args);}//234
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv2_dPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<double,2>(FT,Args);}//235
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv3_dPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<double,3>(FT,Args);}//236
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv4_dPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<double,4>(FT,Args);}//237
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv8_dPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<double,8>(FT,Args);}//238
  BUILTINS_API llvm::GenericValue lle_X__Z29async_work_group_strided_copyPU3AS1Dv16_dPKU3AS3S_mm9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_async_work_group_strided_copy_l2g<double,16>(FT,Args);}//239
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1cm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//240
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_cm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//241
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_cm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//242
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_cm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//243
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_cm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//244
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_cm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//245
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1hm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//246
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_hm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//247
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_hm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//248
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_hm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//249
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_hm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//250
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_hm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//251
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1sm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//252
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_sm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//253
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_sm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//254
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_sm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//255
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_sm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//256
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_sm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//257
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1tm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//258
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_tm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//259
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_tm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//260
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_tm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//261
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_tm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//262
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_tm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//263
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1im( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//264
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_im( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//265
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_im( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//266
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_im( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//267
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_im( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//268
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_im( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//269
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1jm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//270
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_jm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//271
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_jm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//272
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_jm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//273
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_jm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//274
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_jm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//275
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1lm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//276
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_lm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//277
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_lm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//278
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_lm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//279
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_lm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//280
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_lm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//281
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1mm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//282
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_mm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//283
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_mm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//284
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_mm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//285
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_mm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//286
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_mm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//287
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1fm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//288
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_fm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//289
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_fm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//290
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_fm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//291
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_fm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//292
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_fm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//293
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1dm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//294
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv2_dm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//295
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv3_dm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//296
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv4_dm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//297
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv8_dm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//298
  BUILTINS_API llvm::GenericValue lle_X__Z8prefetchPKU3AS1Dv16_dm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_prefetch(FT,Args);}//299
  BUILTINS_API llvm::GenericValue lle_X__Z17wait_group_eventsiP9ocl_event( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_wait_group_events(FT,Args);}//300


}

  
