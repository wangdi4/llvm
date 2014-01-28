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

File Name:  BLTImages.cpp

\*****************************************************************************/
#define DEBUG_TYPE "OpenCLReferenceRunner"


// debug macros
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include "llvm/Support/Debug.h"

#include "Helpers.h"
#include "BLTImages.h"

// !!!! HACK 
// Do not move #include "CL/cl.h" before including <math.h> since on VS2008 it generates
// Removing annoying ‘ceil’ : attributes not present on previous declaration warning C4985
#include "cl_types.h"
#include "CL/cl.h"


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
BUILTINS_API void initOCLBuiltinsImages() {return;}
  

  BUILTINS_API llvm::GenericValue lle_X__Z20get_image_array_size16ocl_image1darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_array_size(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z20get_image_array_size16ocl_image2darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_array_size(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z27get_image_channel_data_type11ocl_image1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_data_type(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z27get_image_channel_data_type16ocl_image1darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_data_type(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z27get_image_channel_data_type17ocl_image1dbuffer( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_data_type(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z27get_image_channel_data_type11ocl_image2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_data_type(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z27get_image_channel_data_type16ocl_image2darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_data_type(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z27get_image_channel_data_type11ocl_image3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_data_type(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z23get_image_channel_order11ocl_image1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_order(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z23get_image_channel_order16ocl_image1darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_order(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z23get_image_channel_order17ocl_image1dbuffer( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_order(FT,Args);}//10
  BUILTINS_API llvm::GenericValue lle_X__Z23get_image_channel_order11ocl_image2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_order(FT,Args);}//11
  BUILTINS_API llvm::GenericValue lle_X__Z23get_image_channel_order16ocl_image2darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_order(FT,Args);}//12
  BUILTINS_API llvm::GenericValue lle_X__Z23get_image_channel_order11ocl_image3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_channel_order(FT,Args);}//13
  BUILTINS_API llvm::GenericValue lle_X__Z15get_image_depth11ocl_image3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_depth(FT,Args);}//14
  BUILTINS_API llvm::GenericValue lle_X__Z13get_image_dim11ocl_image2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_dim2(FT,Args);}//15
  BUILTINS_API llvm::GenericValue lle_X__Z13get_image_dim16ocl_image2darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_dim2(FT,Args);}//16
  BUILTINS_API llvm::GenericValue lle_X__Z13get_image_dim11ocl_image3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_dim3(FT,Args);}//17
  BUILTINS_API llvm::GenericValue lle_X__Z16get_image_height11ocl_image2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_height(FT,Args);}//18
  BUILTINS_API llvm::GenericValue lle_X__Z16get_image_height16ocl_image2darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_height(FT,Args);}//19
  BUILTINS_API llvm::GenericValue lle_X__Z16get_image_height11ocl_image3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_height(FT,Args);}//20
  BUILTINS_API llvm::GenericValue lle_X__Z15get_image_width11ocl_image1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_width(FT,Args);}//21
  BUILTINS_API llvm::GenericValue lle_X__Z15get_image_width16ocl_image1darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_width(FT,Args);}//22
  BUILTINS_API llvm::GenericValue lle_X__Z15get_image_width17ocl_image1dbuffer( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_width(FT,Args);}//23
  BUILTINS_API llvm::GenericValue lle_X__Z15get_image_width11ocl_image2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_width(FT,Args);}//24
  BUILTINS_API llvm::GenericValue lle_X__Z15get_image_width16ocl_image2darray( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_width(FT,Args);}//25
  BUILTINS_API llvm::GenericValue lle_X__Z15get_image_width11ocl_image3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_image_width(FT,Args);}//26
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef11ocl_image1d11ocl_samplerf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//27
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef11ocl_image1d11ocl_sampleri( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//28
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef16ocl_image1darray11ocl_samplerDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//29
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef16ocl_image1darray11ocl_samplerDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//30
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef16ocl_image1darrayDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<float>(FT,Args);}//31
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef17ocl_image1dbufferi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<float>(FT,Args);}//32
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef11ocl_image1di( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<float>(FT,Args);}//33
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef11ocl_image2d11ocl_samplerDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//34
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef11ocl_image2d11ocl_samplerDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//35
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef16ocl_image2darray11ocl_samplerDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//36
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef16ocl_image2darray11ocl_samplerDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//37
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef21ocl_image2darraydepth11ocl_samplerDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//38
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef21ocl_image2darraydepth11ocl_samplerDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//39
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef21ocl_image2darraydepthDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<float>(FT,Args);}//40
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef16ocl_image2darrayDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<float>(FT,Args);}//41
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef16ocl_image2ddepth11ocl_samplerDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//42
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef16ocl_image2ddepth11ocl_samplerDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//43
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef16ocl_image2ddepthDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<float>(FT,Args);}//44
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef11ocl_image2dDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<float>(FT,Args);}//45
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef11ocl_image3d11ocl_samplerDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//46
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef11ocl_image3d11ocl_samplerDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<float>(FT,Args);}//47
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagef11ocl_image3dDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<float>(FT,Args);}//48
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei11ocl_image1d11ocl_samplerf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//49
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei11ocl_image1d11ocl_sampleri( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//50
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei16ocl_image1darray11ocl_samplerDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//51
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei16ocl_image1darray11ocl_samplerDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//52
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei16ocl_image1darrayDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<int32_t>(FT,Args);}//53
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei17ocl_image1dbufferi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<int32_t>(FT,Args);}//54
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei11ocl_image1di( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<int32_t>(FT,Args);}//55
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei11ocl_image2d11ocl_samplerDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//56
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei11ocl_image2d11ocl_samplerDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//57
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei16ocl_image2darray11ocl_samplerDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//58
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei16ocl_image2darray11ocl_samplerDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//59
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei16ocl_image2darrayDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<int32_t>(FT,Args);}//60
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei11ocl_image2dDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<int32_t>(FT,Args);}//61
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei11ocl_image3d11ocl_samplerDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//62
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei11ocl_image3d11ocl_samplerDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<int32_t>(FT,Args);}//63
  BUILTINS_API llvm::GenericValue lle_X__Z11read_imagei11ocl_image3dDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<int32_t>(FT,Args);}//64
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui11ocl_image1d11ocl_samplerf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//65
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui11ocl_image1d11ocl_sampleri( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//66
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui16ocl_image1darray11ocl_samplerDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//67
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui16ocl_image1darray11ocl_samplerDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//68
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui16ocl_image1darrayDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<uint32_t>(FT,Args);}//69
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui17ocl_image1dbufferi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<uint32_t>(FT,Args);}//70
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui11ocl_image1di( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<uint32_t>(FT,Args);}//71
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui11ocl_image2d11ocl_samplerDv2_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//72
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui11ocl_image2d11ocl_samplerDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//73
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui16ocl_image2darray11ocl_samplerDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//74
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui16ocl_image2darray11ocl_samplerDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//75
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui16ocl_image2darrayDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<uint32_t>(FT,Args);}//76
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui11ocl_image2dDv2_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<uint32_t>(FT,Args);}//77
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui11ocl_image3d11ocl_samplerDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//78
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui11ocl_image3d11ocl_samplerDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image<uint32_t>(FT,Args);}//79
  BUILTINS_API llvm::GenericValue lle_X__Z12read_imageui11ocl_image3dDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_read_image_samplerless<uint32_t>(FT,Args);}//80
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagef11ocl_image1diDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<float>(FT,Args);}//81
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagef16ocl_image1darrayDv2_iDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<float>(FT,Args);}//82
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagef17ocl_image1dbufferiDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<float>(FT,Args);}//83
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagef11ocl_image2dDv2_iDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<float>(FT,Args);}//84
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagef16ocl_image2darrayDv4_iDv4_f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<float>(FT,Args);}//85
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagef21ocl_image2darraydepthDv4_if( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<float>(FT,Args);}//86
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagef16ocl_image2ddepthDv2_if( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<float>(FT,Args);}//87
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagei11ocl_image1diDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<int32_t>(FT,Args);}//88
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagei16ocl_image1darrayDv2_iDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<int32_t>(FT,Args);}//89
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagei17ocl_image1dbufferiDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<int32_t>(FT,Args);}//90
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagei11ocl_image2dDv2_iDv4_i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<int32_t>(FT,Args);}//91
  BUILTINS_API llvm::GenericValue lle_X__Z12write_imagei16ocl_image2darrayDv4_iS_( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<int32_t>(FT,Args);}//92
  BUILTINS_API llvm::GenericValue lle_X__Z13write_imageui11ocl_image1diDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<uint32_t>(FT,Args);}//93
  BUILTINS_API llvm::GenericValue lle_X__Z13write_imageui16ocl_image1darrayDv2_iDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<uint32_t>(FT,Args);}//94
  BUILTINS_API llvm::GenericValue lle_X__Z13write_imageui17ocl_image1dbufferiDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<uint32_t>(FT,Args);}//95
  BUILTINS_API llvm::GenericValue lle_X__Z13write_imageui11ocl_image2dDv2_iDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<uint32_t>(FT,Args);}//96
  BUILTINS_API llvm::GenericValue lle_X__Z13write_imageui16ocl_image2darrayDv4_iDv4_j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_write_image<uint32_t>(FT,Args);}//97


}
namespace Validation {
namespace OCLBuiltins {

Conformance::image_descriptor CreateConfImageDesc(const cl_mem_obj_descriptor& in_Desc, 
                                                         cl_image_format& out_ImageFmt)
{
    Conformance::image_descriptor retDesc;
    const bool Is2D = (in_Desc.dim_count == 2);
    const bool Is3D = (in_Desc.dim_count == 3);
    
    retDesc.width = (size_t) in_Desc.dimensions.dim[0];
    retDesc.height = (size_t) ((Is2D || Is3D)? in_Desc.dimensions.dim[1] : 0);
    retDesc.depth = (size_t) (Is3D ? in_Desc.dimensions.dim[2] : 0);
    retDesc.rowPitch = (size_t) in_Desc.pitch[0];
    retDesc.slicePitch = (size_t) (Is3D ? in_Desc.pitch[1] : 0);

    retDesc.type = in_Desc.memObjType;

    retDesc.arraySize = 0;

    if(in_Desc.memObjType == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
        retDesc.height = 0;
        retDesc.arraySize = in_Desc.dimensions.dim[1];
        retDesc.slicePitch = (size_t) (in_Desc.pitch[1]);
    }

    if(in_Desc.memObjType == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
        retDesc.depth = 0;
        retDesc.arraySize = in_Desc.dimensions.dim[2];
    }
    
    out_ImageFmt.image_channel_data_type = 
        in_Desc.format.image_channel_data_type;
    out_ImageFmt.image_channel_order = 
        in_Desc.format.image_channel_order;
    retDesc.format = &out_ImageFmt;
    return retDesc;
}

Conformance::image_sampler_data CreateSamplerData(const uint32_t& in_sampler)
{
    Conformance::image_sampler_data retSampler;
    cl_addressing_mode dst_adr;
    switch(in_sampler & __ADDRESS_MASK)
    {
    case CLK_ADDRESS_NONE:
        dst_adr = CL_ADDRESS_NONE; break;
    case CLK_ADDRESS_CLAMP:
        dst_adr = CL_ADDRESS_CLAMP; break;
    case CLK_ADDRESS_CLAMP_TO_EDGE:
        dst_adr = CL_ADDRESS_CLAMP_TO_EDGE; break;
    case CLK_ADDRESS_REPEAT:
        dst_adr = CL_ADDRESS_REPEAT; break;
    case CLK_ADDRESS_MIRRORED_REPEAT:
        dst_adr = CL_ADDRESS_MIRRORED_REPEAT; break;
    default:
        throw Exception::InvalidArgument("Not supported sampler addressing mode");
        break;
    }
    
    cl_filter_mode dst_filt;
    switch(in_sampler & __FILTER_MASK)
    {
    case CLK_FILTER_NEAREST:
        dst_filt = CL_FILTER_NEAREST; break;
    case CLK_FILTER_LINEAR:
        dst_filt = CL_FILTER_LINEAR; break;
    default:
        throw Exception::InvalidArgument("Not supported sampler filter type");
    }

    retSampler.addressing_mode = dst_adr;
    retSampler.filter_mode = dst_filt;
    retSampler.normalized_coords = (in_sampler & __NORMALIZED_MASK);
    return retSampler;
}

GenericValue lle_X_get_image_dim2(FunctionType *FT,
                                  const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t width = (uint32_t)memobj->dimensions.dim[0];
    const uint32_t height = (uint32_t)memobj->dimensions.dim[1];
    
    gv.AggregateVal.resize(2);
    gv.AggregateVal[0].IntVal = APInt(32, width);
    gv.AggregateVal[1].IntVal = APInt(32, height);

    return gv;
}

GenericValue lle_X_get_image_width(FunctionType *FT,
                                   const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t width = (uint32_t)memobj->dimensions.dim[0];
    gv.IntVal = APInt(32, width);
    return gv;
}

GenericValue lle_X_get_image_height(FunctionType *FT,
                                    const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t height = (uint32_t)memobj->dimensions.dim[1];
    gv.IntVal = APInt(32, height);
    return gv;
}

GenericValue lle_X_get_image_depth(FunctionType *FT,
                                    const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t depth = (uint32_t)memobj->dimensions.dim[2];
    gv.IntVal = APInt(32, depth);
    return gv;
}

GenericValue lle_X_get_image_dim3(FunctionType *FT,
                                  const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t width = (uint32_t)memobj->dimensions.dim[0];
    const uint32_t height = (uint32_t)memobj->dimensions.dim[1];
    const uint32_t depth = (uint32_t)memobj->dimensions.dim[2];
    
    gv.AggregateVal.resize(4);
    gv.AggregateVal[0].IntVal = APInt(32, width);
    gv.AggregateVal[1].IntVal = APInt(32, height);
    gv.AggregateVal[2].IntVal = APInt(32, depth);
    gv.AggregateVal[3].IntVal = APInt(32, 0);

    return gv;
}

GenericValue lle_X_get_image_channel_data_type(FunctionType *FT,
                                  const std::vector<GenericValue> &Args) 
{
    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const int32_t image_channel_data_type = memobj->format.image_channel_data_type;
    gv.IntVal = APInt( 32, image_channel_data_type );
    return gv;
}

GenericValue lle_X_get_image_channel_order(FunctionType *FT,
                                               const std::vector<GenericValue> &Args) 
{
    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const int32_t image_channel_order = memobj->format.image_channel_order;
    gv.IntVal = APInt( 32, image_channel_order );
    return gv;
}

GenericValue lle_X_get_image_array_size(FunctionType *FT,
                                               const std::vector<GenericValue> &Args) 
{
    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    cl_mem_object_type imageType = memobj->memObjType;
    
    uint32_t image_array_size = 0;

    if ( imageType == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
        image_array_size = (uint32_t)memobj->dimensions.dim[1];
    } else if ( imageType == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
        image_array_size = (uint32_t)memobj->dimensions.dim[2];
    }
    
    gv.IntVal = APInt( 32, image_array_size );
    return gv;
}

template<>
void getColorForWriteImage(const GenericValue& PixelGV, float val[4]) {
    val[0] = PixelGV.AggregateVal[0].FloatVal;
    val[1] = PixelGV.AggregateVal[1].FloatVal;
    val[2] = PixelGV.AggregateVal[2].FloatVal;
    val[3] = PixelGV.AggregateVal[3].FloatVal;
    return;
}

template<>
void getColorForWriteImage(const GenericValue& PixelGV, int32_t val[4]) {
    val[0] = (int32_t)PixelGV.AggregateVal[0].IntVal.getSExtValue(); 
    val[1] = (int32_t)PixelGV.AggregateVal[1].IntVal.getSExtValue();
    val[2] = (int32_t)PixelGV.AggregateVal[2].IntVal.getSExtValue(); 
    val[3] = (int32_t)PixelGV.AggregateVal[3].IntVal.getSExtValue();
    return;
}

template<>
void getColorForWriteImage(const GenericValue& PixelGV, uint32_t val[4]) {
    val[0] = (uint32_t)PixelGV.AggregateVal[0].IntVal.getZExtValue();
    val[1] = (uint32_t)PixelGV.AggregateVal[1].IntVal.getZExtValue();
    val[2] = (uint32_t)PixelGV.AggregateVal[2].IntVal.getZExtValue(); 
    val[3] = (uint32_t)PixelGV.AggregateVal[3].IntVal.getZExtValue();
    return;
}

}
}
  
