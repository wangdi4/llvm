// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

// !!!IMPORTANT!!! These defines should be the same as in ImageCallbackLibrary.h
#define NONE_FALSE_NEAREST 0x10
#define CLAMP_FALSE_NEAREST 0x14
#define CLAMPTOEDGE_FALSE_NEAREST 0x12
#define REPEAT_FALSE_NEAREST 0x16
#define MIRRORED_FALSE_NEAREST 0x18

#define NONE_TRUE_NEAREST 0x11
#define CLAMP_TRUE_NEAREST 0x15
#define CLAMPTOEDGE_TRUE_NEAREST 0x13
#define REPEAT_TRUE_NEAREST 0x17
#define MIRRORED_TRUE_NEAREST 0x19

#define NONE_FALSE_LINEAR 0x20
#define CLAMP_FALSE_LINEAR 0x24
#define CLAMPTOEDGE_FALSE_LINEAR 0x22
#define REPEAT_FALSE_LINEAR 0x26
#define MIRRORED_FALSE_LINEAR 0x28

#define NONE_TRUE_LINEAR 0x21
#define CLAMP_TRUE_LINEAR 0x25
#define CLAMPTOEDGE_TRUE_LINEAR 0x23
#define REPEAT_TRUE_LINEAR 0x27
#define MIRRORED_TRUE_LINEAR 0x29

#include "cl_image_format_t.h"
#include "cl_sampler_mask.h"
#include "cl_types2.h"

#define IMG_FUNC_EXPORT
#define MAX_WORK_DIM 3
#define CBK_ARRAY_SIZE 64

// calback functions types definitions

// Coordinate callback typedefs
// coordinates callback for float input
typedef __private void *Image_F_COORD_CBK;
/// coordinates callback should return coordinates of pixel and
/// i0,j0,k0,i1,k1,j1 components for interpolation
typedef __private void *Image_FF_COORD_CBK;

// Image reading callback typedefs
// Reading from uint32_t image.
typedef __private void *Image_UI_READ_CBK;
typedef __private void *SOA4_Image_UI_READ_CBK;
typedef __private void *SOA8_Image_UI_READ_CBK;
typedef __private void *SOA16_Image_UI_READ_CBK;

// Reading from signed int image
typedef __private void *Image_I_READ_CBK;
// read callback for float images and float coordinates takes
// translated coordinates and i0,j0,k0,i1,j1,k1 components for interpolation
typedef __private void *Image_F_READ_CBK;

// read callback for float images and integer coordinates
typedef __private void *Image_FI_READ_CBK;

// Write image callback typedefs
typedef __private void *Image_UI_WRITE_CBK;
typedef __private void *Image_I_WRITE_CBK;
typedef __private void *Image_F_WRITE_CBK;

typedef __private void *SOA4_Image_UI_WRITE_CBK;
typedef __private void *SOA8_Image_UI_WRITE_CBK;
typedef __private void *SOA16_Image_UI_WRITE_CBK;

// Coordinates callback for integer input
typedef __private void *Image_I_COORD_CBK;
// Coordinates callback for SOA4 integer input
typedef __private void *SOA4_Image_I_COORD_CBK;
// Coordinates callback for SOA8 integer input
typedef __private void *SOA8_Image_I_COORD_CBK;
// Coordinates callback for SOA16 integer input
typedef __private void *SOA16_Image_I_COORD_CBK;

int4 call_Image_F_COORD_CBK(private void *cbk, __private void *image,
                            float4 coord);
float4 call_Image_FF_COORD_CBK(__private void *cbk, __private void *image,
                               float4 coord, __private int4 *square0,
                               __private int4 *square1);
uint4 call_Image_UI_READ_CBK(__private void *cbk, __private void *image,
                             int4 coord, __private void *pData);
void call_SOA4_Image_UI_READ_CBK(__private void *cbk, __private void *image,
                                 int4 coord_x, int4 coord_y,
                                 __private void *pData, __private uint4 *res_x,
                                 __private uint4 *res_y, __private uint4 *res_z,
                                 __private uint4 *res_w);
void call_SOA8_Image_UI_READ_CBK(__private void *cbk, __private void *image,
                                 int8 coord_x, int8 coord_y,
                                 __private void *pData, __private uint8 *res_x,
                                 __private uint8 *res_y, __private uint8 *res_z,
                                 __private uint8 *res_w);
void call_SOA16_Image_UI_READ_CBK(
    __private void *cbk, __private void *image, int16 coord_x, int16 coord_y,
    __private void *pData, __private uint16 *res_x, __private uint16 *res_y,
    __private uint16 *res_z, __private uint16 *res_w);
int4 call_Image_I_READ_CBK(__private void *cbk, __private void *image,
                           int4 coord, __private void *pData);
float4 call_Image_F_READ_CBK(__private void *cbk, __private void *image,
                             int4 square0, int4 square1, float4 coord,
                             __private void *pData);
float4 call_Image_FI_READ_CBK(__private void *cbk, __private void *image,
                              int4 coord, int4 dummy0, float4 dummy1,
                              __private void *pData);
void call_Image_UI_WRITE_CBK(__private void *cbk, __private void *image,
                             uint4 color);
void call_Image_I_WRITE_CBK(__private void *cbk, __private void *image,
                            int4 color);
void call_Image_F_WRITE_CBK(__private void *cbk, __private void *image,
                            float4 color);
void call_SOA4_Image_UI_WRITE_CBK(__private void *cbk, __private void *p1,
                                  __private void *p2, __private void *p3,
                                  __private void *p4, uint4 val_x, uint4 val_y,
                                  uint4 val_z, uint4 val_w);
void call_SOA8_Image_UI_WRITE_CBK(__private void *cbk, __private void *p0,
                                  __private void *p1, __private void *p2,
                                  __private void *p3, __private void *p4,
                                  __private void *p5, __private void *p6,
                                  __private void *p7, uint8 val_x, uint8 val_y,
                                  uint8 val_z, uint8 val_w);
void call_SOA16_Image_UI_WRITE_CBK(__private void *cbk, __private void *p0,
                                   __private void *p1, __private void *p2,
                                   __private void *p3, __private void *p4,
                                   __private void *p5, __private void *p6,
                                   __private void *p7, __private void *p8,
                                   __private void *p9, __private void *p10,
                                   __private void *p11, __private void *p12,
                                   __private void *p13, __private void *p14,
                                   __private void *p15, uint16 val_x,
                                   uint16 val_y, uint16 val_z, uint16 val_w);

int4 call_Image_I_COORD_CBK(__private void *cbk, __private void *image,
                            int4 coord);
void call_SOA4_Image_I_COORD_CBK(__private void *cbk, __private void *image,
                                 int4 coord_x, int4 coord_y,
                                 __private int4 *translated_coord_x,
                                 __private int4 *translated_coord_y);
void call_SOA8_Image_I_COORD_CBK(__private void *cbk, __private void *image,
                                 int8 coord_x, int8 coord_y,
                                 __private int8 *translated_coord_x,
                                 __private int8 *translated_coord_y);
void call_SOA16_Image_I_COORD_CBK(__private void *cbk, __private void *image,
                                  int16 coord_x, int16 coord_y,
                                  __private int16 *translated_coord_x,
                                  __private int16 *translated_coord_y);
Image_I_COORD_CBK const call_coord_translate_i_callback(size_t samplerIndex);
SOA4_Image_I_COORD_CBK const
call_soa4_coord_translate_i_callback(size_t samplerIndex);
SOA8_Image_I_COORD_CBK const
call_soa8_coord_translate_i_callback(size_t samplerIndex);
SOA16_Image_I_COORD_CBK const
call_soa16_coord_translate_i_callback(size_t samplerIndex);

/// Integer coordinate translation callbacks

int4 trans_coord_int_NONE_FALSE_NEAREST(__private void *image, int4 coord);
void soa4_trans_coord_int_NONE_FALSE_NEAREST(__private void *image,
                                             int4 coord_x, int4 coord_y,
                                             __private int4 *res_coord_x,
                                             __private int4 *res_coord_y);
void soa8_trans_coord_int_NONE_FALSE_NEAREST(__private void *image,
                                             int8 coord_x, int8 coord_y,
                                             __private int8 *res_coord_x,
                                             __private int8 *res_coord_y);
void soa16_trans_coord_int_NONE_FALSE_NEAREST(__private void *image,
                                              int16 coord_x, int16 coord_y,
                                              __private int16 *res_coord_x,
                                              __private int16 *res_coord_y);
int4 trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(__private void *image,
                                               int4 coord);
void soa4_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(
    __private void *image, int4 coord_x, int4 coord_y,
    __private int4 *res_coord_x, __private int4 *res_coord_y);
void soa8_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(
    __private void *image, int8 coord_x, int8 coord_y,
    __private int8 *res_coord_x, __private int8 *res_coord_y);
void soa16_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(
    __private void *image, int16 coord_x, int16 coord_y,
    __private int16 *res_coord_x, __private int16 *res_coord_y);
int4 trans_coord_int_UNDEFINED(__private void *image, int4 coord);
void soa4_trans_coord_int_UNDEFINED(__private void *image, int4 coord_x,
                                    int4 coord_y, __private int4 *res_coord_x,
                                    __private int4 *res_coord_y);
void soa8_trans_coord_int_UNDEFINED(__private void *image, int8 coord_x,
                                    int8 coord_y, __private int8 *res_coord_x,
                                    __private int8 *res_coord_y);
void soa16_trans_coord_int_UNDEFINED(__private void *image, int16 coord_x,
                                     int16 coord_y,
                                     __private int16 *res_coord_x,
                                     __private int16 *res_coord_y);

#define ALIGN16 __attribute__((aligned(16)))
// Image description. Contains all data about image and required callback
// functions
// !!!DUPLICATE!!! Has duplicate function in cl_api/cl_types.h
typedef struct _image_aux_data {
  uint dim_count; // A number of dimensions in the memory object.

  // Multi-dimensional pitch of the object, valid only for images (2D/3D).
  size_t pitch[MAX_WORK_DIM - 1];

  // Format of the memory object,valid only for images (2D/3D).
  /* cl_image_format fields:
     unsigned int image_channel_order;
     unsigned int image_channel_data_type; */
  cl_image_format_t format;

  // A pointer to the object wherein the object data is stored.
  // Could be a valid memory pointer or a handle to other object.
  __private void *pData;

  unsigned uiElementSize; // Size of image pixel element.

  // the list of float coordinate translation callback
  __private void *coord_translate_f_callback[CBK_ARRAY_SIZE];
  // the list of integer image reader & filter callbacks
  __private void *read_img_callback_int[CBK_ARRAY_SIZE];
  // the list of float   image reader & filter callbacks
  __private void *read_img_callback_float[CBK_ARRAY_SIZE];
  // the list of soa4 integer image reader & filter callbacks
  __private void *soa4_read_img_callback_int[CBK_ARRAY_SIZE];
  // the list of soa8 integer image reader & filter callbacks
  __private void *soa8_read_img_callback_int[CBK_ARRAY_SIZE];
  // the list of soa16 integer image reader & filter callbacks
  __private void *soa16_read_img_callback_int[CBK_ARRAY_SIZE];
  __private void *write_img_callback;       // the write image sampler callback
  __private void *soa4_write_img_callback;  // the write image sampler callback
  __private void *soa8_write_img_callback;  // the write image sampler callback
  __private void *soa16_write_img_callback; // the write image sampler callback

  // Image size for each dimension subtracted by one
  // Used to optimize coordinates computation not to subtract by one for each
  // read
  int dimSub1[MAX_WORK_DIM + 1] ALIGN16;
  int dim[MAX_WORK_DIM + 1] ALIGN16; // Image size for each dimension
  unsigned int offset[MAX_WORK_DIM + 1] ALIGN16; // the offset to extract pixels
  // Float image size for each dimension.
  // Used in coordinates computation to avoid int->float type conversion for
  // each read call
  float dimf[MAX_WORK_DIM + 1] ALIGN16;
  int array_size; // size of array for 1D and 2d array types, otherwise is set
                  // to -1
  int dimmask;    // Mask for dimensions in images
               // Contains ones at dim_count first bytes. Other bytes are zeros.
               // Used for coordinates clamping
} image_aux_data;
