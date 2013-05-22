// Copyright (c) 2013 Intel Corporation
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

#include "cl_image_declaration.h"

// calback functions types definitions

// Coordinate callback typedefs
// coordinates callback for float input
typedef int4 (*__Image_F_COORD_CBK) (void*, float4);
/// coordinates callback should return coordinates of pixel and
/// i0,j0,k0,i1,k1,j1 components for interpolation
typedef float4 (*__Image_FF_COORD_CBK) (void*, float4, int4*, int4*);

// Image reading callback typedefs
// Reading from uint32_t image.
typedef uint4 (*__Image_UI_READ_CBK) (void* image, int4 coord, void* pData);
typedef void (*__SOA4_Image_UI_READ_CBK) (void* image, int4 coord_x, int4 coord_y, void* pData, uint4* res_x, uint4* res_y, uint4* res_z, uint4* res_w);
typedef void (*__SOA8_Image_UI_READ_CBK) (void* image, int8 coord_x, int8 coord_y, void* pData, uint8* res_x, uint8* res_y, uint8* res_z, uint8* res_w);


// Reading from signed int image
typedef int4 (*__Image_I_READ_CBK) (void* image, int4 coord, void* pData);
// read callback for float images and float coordinates takes
// translated coordinates and i0,j0,k0,i1,j1,k1 components for interpolation
typedef float4 (*__Image_F_READ_CBK) (void* image, int4 square0, int4 square1, float4 coord, void* pData);

// read callback for float images and integer coordinates
typedef float4 (*__Image_FI_READ_CBK) (void* image, int4 coord, int4 dummy0, float4 dummy1, void* pData);

// Write image callback typedefs
typedef void (*__Image_UI_WRITE_CBK) (void*, uint4);
typedef void (*__Image_I_WRITE_CBK) (void*, int4);
typedef void (*__Image_F_WRITE_CBK) (void*, float4);

typedef void (*__SOA4_Image_UI_WRITE_CBK) (void*, void*, void*, void*, uint4, uint4, uint4, uint4);
typedef void (*__SOA8_Image_UI_WRITE_CBK) (void*, void*, void*, void*, void*, void*, void*, void*, uint8, uint8, uint8, uint8);

// Coordinates callback for integer input
typedef int4 (*__Image_I_COORD_CBK) (void*, int4);
// Coordinates callback for SOA4 integer input
typedef void (*__SOA4_Image_I_COORD_CBK) (void*, int4, int4, int4*, int4*);
// Coordinates callback for SOA8 integer input
typedef void (*__SOA8_Image_I_COORD_CBK) (void*, int8, int8, int8*, int8*);


__Image_I_COORD_CBK const constant coord_translate_i_callback[32] = {
    trans_coord_int_NONE_FALSE_NEAREST,
    trans_coord_int_NONE_FALSE_NEAREST,
    trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED
};    //the list of integer coordinate translation callback

__SOA4_Image_I_COORD_CBK const constant soa4_coord_translate_i_callback[32] = {
    soa4_trans_coord_int_NONE_FALSE_NEAREST,
    soa4_trans_coord_int_NONE_FALSE_NEAREST,
    soa4_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED
};    //the list of soa4 integer coordinate translation callback

__SOA8_Image_I_COORD_CBK const constant soa8_coord_translate_i_callback[32] = {
    soa8_trans_coord_int_NONE_FALSE_NEAREST,
    soa8_trans_coord_int_NONE_FALSE_NEAREST,
    soa8_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED
};    //the list of soa8 integer coordinate translation callback


Image_I_COORD_CBK const constant call_coord_translate_i_callback(int samplerIndex){
return coord_translate_i_callback[samplerIndex];
}
SOA4_Image_I_COORD_CBK const constant call_soa4_coord_translate_i_callback(int samplerIndex){
return soa4_coord_translate_i_callback[samplerIndex];
}
SOA8_Image_I_COORD_CBK const constant call_soa8_coord_translate_i_callback(int samplerIndex){
return soa8_coord_translate_i_callback[samplerIndex];
}


int4   call_Image_F_COORD_CBK(void* cbk, void* image, float4 coord) {
    __Image_F_COORD_CBK m_cbk = (__Image_F_COORD_CBK)cbk;
return m_cbk(image, coord);
}

float4 call_Image_FF_COORD_CBK(void* cbk, void* image, float4 coord, int4* square0, int4* square1) {
    __Image_FF_COORD_CBK m_cbk = (__Image_FF_COORD_CBK)cbk;
return m_cbk(image, coord, square0, square1);
}

uint4  call_Image_UI_READ_CBK(void *cbk, void* image, int4 coord, void* pData) {
    __Image_UI_READ_CBK m_cbk = (__Image_UI_READ_CBK)cbk;
return m_cbk(image, coord, pData);
}

int4   call_Image_I_READ_CBK       (void *cbk, void* image, int4 coord, void* pData) {
    __Image_I_READ_CBK m_cbk = (__Image_I_READ_CBK)cbk;
return m_cbk(image, coord, pData);
}

float4 call_Image_F_READ_CBK       (void *cbk, void* image, int4 square0, int4 square1, float4 coord, void* pData) {
    __Image_F_READ_CBK m_cbk = (__Image_F_READ_CBK)cbk;
return m_cbk(image, square0, square1, coord, pData);
}

float4 call_Image_FI_READ_CBK      (void *cbk, void* image, int4 coord, int4 dummy0, float4 dummy1, void* pData){
    __Image_FI_READ_CBK m_cbk = (__Image_FI_READ_CBK)cbk;
return m_cbk(image, coord, dummy0, dummy1, pData);
}
void   call_Image_UI_WRITE_CBK     (void *cbk, void* image, uint4 color){
    __Image_UI_WRITE_CBK m_cbk = (__Image_UI_WRITE_CBK)cbk;
    m_cbk(image, color);
return;
}
void   call_Image_I_WRITE_CBK      (void *cbk, void* image, int4 color){
    __Image_I_WRITE_CBK m_cbk = (__Image_I_WRITE_CBK)cbk;
    m_cbk(image, color);
return;
}
void   call_Image_F_WRITE_CBK      (void *cbk, void* image, float4 color){
    __Image_F_WRITE_CBK m_cbk = (__Image_F_WRITE_CBK)cbk;
    m_cbk(image, color);
return;
}
void   call_SOA4_Image_UI_READ_CBK (void* cbk, void* image, int4 coord_x, int4 coord_y, void* pData, uint4* res_x, uint4* res_y, uint4* res_z, uint4* res_w){
    __SOA4_Image_UI_READ_CBK m_cbk = (__SOA4_Image_UI_READ_CBK)cbk;
    m_cbk(image, coord_x, coord_y, pData, res_x, res_y, res_z, res_w);
return;
}
void   call_SOA8_Image_UI_READ_CBK (void* cbk, void* image, int8 coord_x, int8 coord_y, void* pData, uint8* res_x, uint8* res_y, uint8* res_z, uint8* res_w){
    __SOA8_Image_UI_READ_CBK m_cbk = (__SOA8_Image_UI_READ_CBK)cbk;
    m_cbk(image, coord_x, coord_y, pData, res_x, res_y, res_z, res_w);
return;
}
void   call_SOA4_Image_UI_WRITE_CBK(void* cbk, void* p1, void* p2, void* p3, void* p4, uint4 val_x, uint4 val_y, uint4 val_z, uint4 val_w){
    __SOA4_Image_UI_WRITE_CBK m_cbk = (__SOA4_Image_UI_WRITE_CBK)cbk;
    m_cbk(p1, p2, p3, p4, val_x, val_y, val_z, val_w);
return;
}
void   call_SOA8_Image_UI_WRITE_CBK(void* cbk, void* p0, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6, void* p7, uint8 val_x, uint8 val_y, uint8 val_z, uint8 val_w){
    __SOA8_Image_UI_WRITE_CBK m_cbk = (__SOA8_Image_UI_WRITE_CBK)cbk;
    m_cbk(p0, p1, p2, p3, p4, p5, p6, p7, val_x, val_y, val_z, val_w);
return;
}
int4 call_Image_I_COORD_CBK      (void* cbk, void* image, int4 coord){
    __Image_I_COORD_CBK m_cbk = (__Image_I_COORD_CBK)cbk;
    return m_cbk(image, coord);
}
void call_SOA4_Image_I_COORD_CBK (void* cbk, void* image, int4 coord_x, int4 coord_y, int4* translated_coord_x, int4* translated_coord_y){
    __SOA4_Image_I_COORD_CBK m_cbk = (__SOA4_Image_I_COORD_CBK)cbk;
    m_cbk(image, coord_x, coord_y, translated_coord_x, translated_coord_y);
return;
}
void call_SOA8_Image_I_COORD_CBK (void* cbk, void* image, int8 coord_x, int8 coord_y, int8* translated_coord_x, int8* translated_coord_y){
    __SOA8_Image_I_COORD_CBK m_cbk = (__SOA8_Image_I_COORD_CBK)cbk;
    m_cbk(image, coord_x, coord_y, translated_coord_x, translated_coord_y);
return;
}
