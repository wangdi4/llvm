// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

// !!!IMPORTANT!!! These defines should be the same as in ImageCallbackLibrary.h
#define NONE_FALSE_NEAREST 0x00
#define CLAMP_FALSE_NEAREST 0x01
#define CLAMPTOEDGE_FALSE_NEAREST 0x02
#define REPEAT_FALSE_NEAREST 0x03
#define MIRRORED_FALSE_NEAREST 0x04

#define NONE_TRUE_NEAREST 0x08
#define CLAMP_TRUE_NEAREST 0x09
#define CLAMPTOEDGE_TRUE_NEAREST 0x0a
#define REPEAT_TRUE_NEAREST 0x0b
#define MIRRORED_TRUE_NEAREST 0x0c

#define NONE_FALSE_LINEAR 0x10
#define CLAMP_FALSE_LINEAR 0x11
#define CLAMPTOEDGE_FALSE_LINEAR 0x12
#define REPEAT_FALSE_LINEAR 0x13
#define MIRRORED_FALSE_LINEAR 0x14

#define NONE_TRUE_LINEAR 0x18
#define CLAMP_TRUE_LINEAR 0x19
#define CLAMPTOEDGE_TRUE_LINEAR 0x1a
#define REPEAT_TRUE_LINEAR 0x1b
#define MIRRORED_TRUE_LINEAR 0x1c

#include "cl_types2.h"

#define IMG_FUNC_EXPORT
#define MAX_WORK_DIM 3

// calback functions types definitions

// Coordinate callback typedefs
// coordinates callback for float input
typedef __constant void *Image_F_COORD_CBK;
/// coordinates callback should return coordinates of pixel and
/// i0,j0,k0,i1,k1,j1 components for interpolation
typedef __constant void *Image_FF_COORD_CBK;

// Image reading callback typedefs
// Reading from uint32_t image.
typedef __constant void *Image_UI_READ_CBK;
typedef __constant void *SOA4_Image_UI_READ_CBK;
typedef __constant void *SOA8_Image_UI_READ_CBK;


// Reading from signed int image
typedef __constant void *Image_I_READ_CBK;
// read callback for float images and float coordinates takes
// translated coordinates and i0,j0,k0,i1,j1,k1 components for interpolation
typedef __constant void *Image_F_READ_CBK;

// read callback for float images and integer coordinates
typedef __constant void *Image_FI_READ_CBK;

// Write image callback typedefs
typedef __constant void *Image_UI_WRITE_CBK;
typedef __constant void *Image_I_WRITE_CBK;
typedef __constant void *Image_F_WRITE_CBK;

typedef __constant void *SOA4_Image_UI_WRITE_CBK;
typedef __constant void *SOA8_Image_UI_WRITE_CBK;

// Coordinates callback for integer input
typedef __constant void *Image_I_COORD_CBK;
// Coordinates callback for SOA4 integer input
typedef __constant void *SOA4_Image_I_COORD_CBK;
// Coordinates callback for SOA8 integer input
typedef __constant void *SOA8_Image_I_COORD_CBK;

int4   call_Image_F_COORD_CBK      (__constant void* cbk, __private void* image, float4 coord);
float4 call_Image_FF_COORD_CBK     (__constant void* cbk, __private void* image, float4 coord, __private int4* square0, __private int4* square1);
uint4  call_Image_UI_READ_CBK      (__constant void* cbk, __private void* image, int4 coord, __private void* pData);
void   call_SOA4_Image_UI_READ_CBK (__constant void* cbk, __private void* image, int4 coord_x, int4 coord_y, __private void* pData, __private uint4* res_x, __private uint4* res_y, __private uint4* res_z, __private uint4* res_w);
void   call_SOA8_Image_UI_READ_CBK (__constant void* cbk, __private void* image, int8 coord_x, int8 coord_y, __private void* pData, __private uint8* res_x, __private uint8* res_y, __private uint8* res_z, __private uint8* res_w);
int4   call_Image_I_READ_CBK       (__constant void* cbk, __private void* image, int4 coord, __private void* pData);
float4 call_Image_F_READ_CBK       (__constant void* cbk, __private void* image, int4 square0, int4 square1, float4 coord, __private void* pData);
float4 call_Image_FI_READ_CBK      (__constant void* cbk, __private void* image, int4 coord, int4 dummy0, float4 dummy1, __private void* pData);
void   call_Image_UI_WRITE_CBK     (__constant void* cbk, __private void* image, uint4 color);
void   call_Image_I_WRITE_CBK      (__constant void* cbk, __private void* image, int4 color);
void   call_Image_F_WRITE_CBK      (__constant void* cbk, __private void* image, float4 color);
void   call_SOA4_Image_UI_WRITE_CBK(__constant void* cbk, __private void* p1, __private void* p2, __private void* p3, __private void* p4, uint4 val_x, uint4 val_y, uint4 val_z, uint4 val_w);
void   call_SOA8_Image_UI_WRITE_CBK(__constant void* cbk, __private void* p0, __private void* p1, __private void* p2, __private void* p3, __private void* p4, __private void* p5, __private void* p6, __private void* p7, uint8 val_x, uint8 val_y, uint8 val_z, uint8 val_w);

int4 call_Image_I_COORD_CBK      (__constant void* cbk, __private void* image, int4 coord);
void call_SOA4_Image_I_COORD_CBK (__constant void* cbk, __private void* image, int4 coord_x, int4 coord_y, __private int4* translated_coord_x, __private int4* translated_coord_y);
void call_SOA8_Image_I_COORD_CBK (__constant void* cbk, __private void* image, int8 coord_x, int8 coord_y, __private int8* translated_coord_x, __private int8* translated_coord_y);
Image_I_COORD_CBK const call_coord_translate_i_callback(int samplerIndex);
SOA4_Image_I_COORD_CBK const call_soa4_coord_translate_i_callback(int samplerIndex);
SOA8_Image_I_COORD_CBK const call_soa8_coord_translate_i_callback(int samplerIndex);

/// Integer coordinate translation callbacks

int4 __attribute__((overloadable)) trans_coord_int_NONE_FALSE_NEAREST(__private void* image, int4 coord);
void __attribute__((overloadable)) soa4_trans_coord_int_NONE_FALSE_NEAREST(__private void* image, int4 coord_x, int4 coord_y, __private int4* res_coord_x, __private int4* res_coord_y);
void __attribute__((overloadable)) soa8_trans_coord_int_NONE_FALSE_NEAREST(__private void* image, int8 coord_x, int8 coord_y, __private int8* res_coord_x, __private int8* res_coord_y);
int4 __attribute__((overloadable)) trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(__private void* image, int4 coord);
void __attribute__((overloadable)) soa4_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(__private void* image, int4 coord_x, int4 coord_y, __private int4* res_coord_x, __private int4* res_coord_y);
void __attribute__((overloadable)) soa8_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(__private void* image, int8 coord_x, int8 coord_y, __private int8* res_coord_x, __private int8* res_coord_y);
int4 __attribute__((overloadable)) trans_coord_int_UNDEFINED(__private void* image, int4 coord);
void __attribute__((overloadable)) soa4_trans_coord_int_UNDEFINED(__private void* image, int4 coord_x, int4 coord_y, __private int4* res_coord_x, __private int4* res_coord_y);
void __attribute__((overloadable)) soa8_trans_coord_int_UNDEFINED(__private void* image, int8 coord_x, int8 coord_y, __private int8* res_coord_x, __private int8* res_coord_y);


#define ALIGN16 __attribute__ ((aligned(16)))
// Image description. Contains all data about image and required callback functions
// !!!DUPLICATE!!! Has duplicate function in cl_api/cl_types.h
typedef struct _image_aux_data
{
  uint              dim_count;                // A number of dimensions in the memory object.
  size_t            pitch[MAX_WORK_DIM-1];    // Multi-dimensional pitch of the object, valid only for images (2D/3D).
  cl_image_format_t format;                   // Format of the memory object,valid only for images (2D/3D).
                                              /* cl_image_format fields:
                                                unsigned int image_channel_order;
                                                unsigned int image_channel_data_type; */
  __private void*             pData;                    // A pointer to the object wherein the object data is stored.
                                              // Could be a valid memory pointer or a handle to other object.
  unsigned          uiElementSize;            // Size of image pixel element.

  void*             coord_translate_f_callback[32]; //the list of float coordinate translation callback
  void*             read_img_callback_int[32];      // the list of integer image reader & filter callbacks
  void*             read_img_callback_float[32];    // the list of float   image reader & filter callbacks
  void*             soa4_read_img_callback_int[32]; // the list of soa4 integer image reader & filter callbacks
  void*             soa8_read_img_callback_int[32]; // the list of soa8 integer image reader & filter callbacks
  void*             write_img_callback;             // the write image sampler callback
  void*             soa4_write_img_callback;        // the write image sampler callback
  void*             soa8_write_img_callback;        // the write image sampler callback

  int dimSub1[MAX_WORK_DIM+1] ALIGN16;        // Image size for each dimension subtracted by one
                                              // Used to optimize coordinates computation not to subtract by one for each read
  int dim[MAX_WORK_DIM+1] ALIGN16;            // Image size for each dimension
  unsigned int offset[MAX_WORK_DIM+1] ALIGN16;// the offset to extract pixels
  float dimf[MAX_WORK_DIM+1] ALIGN16;         // Float image size for each dimension.
                                              // Used in coordinates computation to avoid
                                              // int->float type conversion for each read call
  int array_size;     // size of array for 1D and 2d array types, otherwise is set to -1
  int dimmask;        // Mask for dimensions in images
                      // Contains ones at dim_count first bytes. Other bytes are zeros.
                      // Used for coordinates clamping
} image_aux_data;

