// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

def image2d_array: OclType<"image2d_array_t", "image2d_array_t", 1, 0, 0>;
def image1d: OclType<"image1d_t", "image1d_t", 1, 0, 0>;
def image1d_array: OclType<"image1d_array_t", "image1d_array_t", 1, 0, 0>;
def image1d_buffer: OclType<"image1d_buffer_t", "image1d_buffer_t", 1, 0, 0>;
def image3d: OclType<"image3d_t", "image3d_t", 1, 0, 0>;
def sampler_t : OclType<"sampler_t", "sampler_t", 1, 0, 0>;
def image2d_depth: OclType<"image2d_depth_t", "image2d_depth_t", 1, 0, 0>;
def image2d_array_depth: OclType<"image2d_array_depth_t", "image2d_array_depth_t", 1, 0, 0>;

// Table 6.12.14.2 Built-in Image Read Functions

let Types = [ v4f32, v4i32 ],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def read_imagef_2d  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image2d:$image, sampler_t:$sampler, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_2d  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image2d:$image, sampler_t:$sampler, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_2d  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image2d:$image, sampler_t:$sampler, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_3d  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image3d:$image, sampler_t:$sampler, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_3d  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image3d:$image, sampler_t:$sampler, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_3d  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image3d:$image, sampler_t:$sampler, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_2d_arr  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image2d_array:$image, sampler_t:$sampler, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_2d_arr  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image2d_array:$image, sampler_t:$sampler, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_2d_arr  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image2d_array:$image, sampler_t:$sampler, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_1d  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image1d:$image, sampler_t:$sampler, sgentype:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_1d  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image1d:$image, sampler_t:$sampler, sgentype:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_1d  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image1d:$image, sampler_t:$sampler, sgentype:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_1d_arr  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image1d_array:$image, sampler_t:$sampler, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_1d_arr  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image1d_array:$image, sampler_t:$sampler, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_1d_arr  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image1d_array:$image, sampler_t:$sampler, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_2d_depth  : REF_OclBuiltin<"read_imagef",  (outs sf32gentype:$ret), (ins image2d_depth:$image, sampler_t:$sampler, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagef_2d_arr_depth  : REF_OclBuiltin<"read_imagef",  (outs sf32gentype:$ret), (ins image2d_array_depth:$image, sampler_t:$sampler, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
}

// Table 6.12.14.3 Built-in Image Sampler-less Read Functions

let Types = [ v4i32 ],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {

def read_imagef_2d_samplerless  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image2d:$image, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_2d_samplerless  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image2d:$image, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_2d_samplerless  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image2d:$image, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_3d_samplerless  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image3d:$image, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_3d_samplerless  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image3d:$image, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_3d_samplerless  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image3d:$image, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_2d_arr_samplerless  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image2d_array:$image, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_2d_arr_samplerless  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image2d_array:$image, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_2d_arr_samplerless  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image2d_array:$image, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_1d_samplerless  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image1d:$image, sgentype:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_1d_samplerless  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image1d:$image, sgentype:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_1d_samplerless  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image1d:$image, sgentype:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_1d_buff_samplerless  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image1d_buffer:$image, sgentype:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_1d_buff_samplerless  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image1d_buffer:$image, sgentype:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_1d_buff_samplerless  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image1d_buffer:$image, sgentype:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_1d_arr_samplerless  : REF_OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins image1d_array:$image, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagei_1d_arr_samplerless  : REF_OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins image1d_array:$image, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def read_imageui_1d_arr_samplerless  : REF_OclBuiltin<"read_imageui",  (outs u32gentype:$ret), (ins image1d_array:$image, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def read_imagef_2d_depth_samplerless  : REF_OclBuiltin<"read_imagef",  (outs sf32gentype:$ret), (ins image2d_depth:$image, gentype2:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def read_imagef_2d_arr_depth_samplerless  : REF_OclBuiltin<"read_imagef",  (outs sf32gentype:$ret), (ins image2d_array_depth:$image, gentype4:$coord), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
}


// Table 6.12.14.4 Built-in Image Write Functions

let Types = [ v4i32 ],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def write_imagef_2d  : REF_OclBuiltin<"write_imagef",  (outs), (ins image2d:$image, gentype2:$coord, f32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def write_imagei_2d  : REF_OclBuiltin<"write_imagei",  (outs), (ins image2d:$image, gentype2:$coord, i32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def write_imageui_2d  : REF_OclBuiltin<"write_imageui",  (outs), (ins image2d:$image, gentype2:$coord, u32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def write_imagef_2d_arr  : REF_OclBuiltin<"write_imagef",  (outs), (ins image2d_array:$image, gentype4:$coord, f32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def write_imagei_2d_arr  : REF_OclBuiltin<"write_imagei",  (outs), (ins image2d_array:$image, gentype4:$coord, i32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def write_imageui_2d_arr  : REF_OclBuiltin<"write_imageui",  (outs), (ins image2d_array:$image, gentype4:$coord, u32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def write_imagef_1d  : REF_OclBuiltin<"write_imagef",  (outs), (ins image1d:$image, sgentype:$coord, f32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def write_imagei_1d  : REF_OclBuiltin<"write_imagei",  (outs), (ins image1d:$image, sgentype:$coord, i32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def write_imageui_1d  : REF_OclBuiltin<"write_imageui",  (outs), (ins image1d:$image, sgentype:$coord, u32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def write_imagef_1d_buff  : REF_OclBuiltin<"write_imagef",  (outs), (ins image1d_buffer:$image, sgentype:$coord, f32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def write_imagei_1d_buff  : REF_OclBuiltin<"write_imagei",  (outs), (ins image1d_buffer:$image, sgentype:$coord, i32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def write_imageui_1d_buff  : REF_OclBuiltin<"write_imageui",  (outs), (ins image1d_buffer:$image, sgentype:$coord, u32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def write_imagef_1d_arr  : REF_OclBuiltin<"write_imagef",  (outs), (ins image1d_array:$image, gentype2:$coord, f32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def write_imagei_1d_arr  : REF_OclBuiltin<"write_imagei",  (outs), (ins image1d_array:$image, gentype2:$coord, i32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_int32_t">] > >;
def write_imageui_1d_arr  : REF_OclBuiltin<"write_imageui",  (outs), (ins image1d_array:$image, gentype2:$coord, u32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_uint32_t">] > >;
def write_imagef_2d_depth  : REF_OclBuiltin<"write_imagef",  (outs), (ins image2d_depth:$image, gentype2:$coord, sf32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
def write_imagef_2d_arr_depth  : REF_OclBuiltin<"write_imagef",  (outs), (ins image2d_array_depth:$image, gentype4:$coord, sf32gentype:$color), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">] > >;
}

// Table 6.12.14.5 Built-in Image Query Functions

let Types = [ v1i32 ],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def get_image_width_1d  : REF_OclBuiltin<"get_image_width",  (outs gentype:$ret), (ins image1d:$image), [], [], TemplateAttr< [] > >;
def get_image_width_1d_buff  : REF_OclBuiltin<"get_image_width",  (outs gentype:$ret), (ins image1d_buffer:$image), [], [], TemplateAttr< [] > >;
def get_image_width_2d  : REF_OclBuiltin<"get_image_width",  (outs gentype:$ret), (ins image2d:$image), [], [], TemplateAttr< [] > >;
def get_image_width_3d  : REF_OclBuiltin<"get_image_width",  (outs gentype:$ret), (ins image3d:$image), [], [], TemplateAttr< [] > >;
def get_image_width_1d_arr  : REF_OclBuiltin<"get_image_width",  (outs gentype:$ret), (ins image1d_array:$image), [], [], TemplateAttr< [] > >;
def get_image_width_2d_arr  : REF_OclBuiltin<"get_image_width",  (outs gentype:$ret), (ins image2d_array:$image), [], [], TemplateAttr< [] > >;
def get_image_height_2d  : REF_OclBuiltin<"get_image_height",  (outs gentype:$ret), (ins image2d:$image), [], [], TemplateAttr< [] > >;
def get_image_height_3d  : REF_OclBuiltin<"get_image_height",  (outs gentype:$ret), (ins image3d:$image), [], [], TemplateAttr< [] > >;
def get_image_height_2d_arr  : REF_OclBuiltin<"get_image_height",  (outs gentype:$ret), (ins image2d_array:$image), [], [], TemplateAttr< [] > >;
def get_image_depth : REF_OclBuiltin<"get_image_depth",  (outs gentype:$ret), (ins image3d:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_data_type_1d : REF_OclBuiltin<"get_image_channel_data_type",  (outs gentype:$ret), (ins image1d:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_data_type_1d_buff : REF_OclBuiltin<"get_image_channel_data_type",  (outs gentype:$ret), (ins image1d_buffer:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_data_type_2d : REF_OclBuiltin<"get_image_channel_data_type",  (outs gentype:$ret), (ins image2d:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_data_type_3d : REF_OclBuiltin<"get_image_channel_data_type",  (outs gentype:$ret), (ins image3d:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_data_type_1d_arr : REF_OclBuiltin<"get_image_channel_data_type",  (outs gentype:$ret), (ins image1d_array:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_data_type_2d_arr : REF_OclBuiltin<"get_image_channel_data_type",  (outs gentype:$ret), (ins image2d_array:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_order_1d : REF_OclBuiltin<"get_image_channel_order",  (outs gentype:$ret), (ins image1d:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_order_1d_buff : REF_OclBuiltin<"get_image_channel_order",  (outs gentype:$ret), (ins image1d_buffer:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_order_2d : REF_OclBuiltin<"get_image_channel_order",  (outs gentype:$ret), (ins image2d:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_order_3d : REF_OclBuiltin<"get_image_channel_order",  (outs gentype:$ret), (ins image3d:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_order_1d_arr : REF_OclBuiltin<"get_image_channel_order",  (outs gentype:$ret), (ins image1d_array:$image), [], [], TemplateAttr< [] > >;
def get_image_channel_order_2d_arr : REF_OclBuiltin<"get_image_channel_order",  (outs gentype:$ret), (ins image2d_array:$image), [], [], TemplateAttr< [] > >;
def get_image_dim_2d : REF_OclBuiltin<"get_image_dim",  (outs gentype2:$ret), (ins image2d:$image), [], [], TemplateAttr< [] > >;
def get_image_dim_2d_arr : REF_OclBuiltin<"get_image_dim",  (outs gentype2:$ret), (ins image2d_array:$image), [], [], TemplateAttr< [] > >;
def get_image_dim_3d : REF_OclBuiltin<"get_image_dim",  (outs gentype4:$ret), (ins image3d:$image), [], [], TemplateAttr< [] > >;
def get_image_array_size_2d_arr : REF_OclBuiltin<"get_image_array_size",  (outs size_t:$ret), (ins image2d_array:$image), [], [], TemplateAttr< [] > >;
def get_image_array_size_1d_arr : REF_OclBuiltin<"get_image_array_size",  (outs size_t:$ret), (ins image1d_array:$image), [], [], TemplateAttr< [] > >;
}

class Generic {
  code Prolog = [{
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
  }];
  
  code Epilog = [{
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
    case CL_DEV_SAMPLER_ADDRESS_NONE:
        dst_adr = CL_ADDRESS_NONE; break;
    case CL_DEV_SAMPLER_ADDRESS_CLAMP:
        dst_adr = CL_ADDRESS_CLAMP; break;
    case CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE:
        dst_adr = CL_ADDRESS_CLAMP_TO_EDGE; break;
    case CL_DEV_SAMPLER_ADDRESS_REPEAT:
        dst_adr = CL_ADDRESS_REPEAT; break;
    case CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT:
        dst_adr = CL_ADDRESS_MIRRORED_REPEAT; break;
    default:
        throw Exception::InvalidArgument("Not supported sampler addressing mode");
        break;
    }
    
    cl_filter_mode dst_filt;
    switch(in_sampler & __FILTER_MASK)
    {
    case CL_DEV_SAMPLER_FILTER_NEAREST:
        dst_filt = CL_FILTER_NEAREST; break;
    case CL_DEV_SAMPLER_FILTER_LINEAR:
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
  }];
  
}
  def inc : Generic;
