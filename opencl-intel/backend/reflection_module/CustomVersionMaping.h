/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __CUSTOM_VERSION_MAPPING_H__
#define __CUSTOM_VERSION_MAPPING_H__

#include "FunctionDescriptor.h"
#include "utils.h"



#ifdef __APPLE__
TableRow mappings[] = {
  //TODO: geometrics, handle separately
  /*
  {{"_f_v._Z5crossDv3_fS_","_f_v.__vertical_cross3f2","_f_v.__vertical_cross3f4","_f_v.__vertical_cross3f8","_f_v.__vertical_cross3f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z5crossDv4_fS_","_f_v.__vertical_cross4f2","_f_v.__vertical_cross4f4","_f_v.__vertical_cross4f8","_f_v.__vertical_cross4f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z13fast_distanceff","_f_v.__vertical_fast_distance1f2","_f_v.__vertical_fast_distance1f4","_f_v.__vertical_fast_distance1f8","_f_v.__vertical_fast_distance1f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z13fast_distanceDv2_fS_","_f_v.__vertical_fast_distance2f2","_f_v.__vertical_fast_distance2f4","_f_v.__vertical_fast_distance2f8","_f_v.__vertical_fast_distance2f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z13fast_distanceDv3_fS_","_f_v.__vertical_fast_distance3f2","_f_v.__vertical_fast_distance3f4","_f_v.__vertical_fast_distance3f8","_f_v.__vertical_fast_distance3f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z13fast_distanceDv4_fS_","_f_v.__vertical_fast_distance4f2","_f_v.__vertical_fast_distance4f4","_f_v.__vertical_fast_distance4f8","_f_v.__vertical_fast_distance4f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z8distanceff","_f_v.__vertical_distance1f2","_f_v.__vertical_distance1f4","_f_v.__vertical_distance1f8","_f_v.__vertical_distance1f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z6lengthf","_f_v.__vertical_length1f2","_f_v.__vertical_length1f4","_f_v.__vertical_length1f8","_f_v.__vertical_length1f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z8distanceDv2_fS_","_f_v.__vertical_distance2f2","_f_v.__vertical_distance2f4","_f_v.__vertical_distance2f8","_f_v.__vertical_distance2f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z6lengthDv2_f","_f_v.__vertical_length2f2","_f_v.__vertical_length2f4","_f_v.__vertical_length2f8","_f_v.__vertical_length2f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z8distanceDv3_fS_","_f_v.__vertical_distance3f2","_f_v.__vertical_distance3f4","_f_v.__vertical_distance3f8","_f_v.__vertical_distance3f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z6lengthDv3_f","_f_v.__vertical_length3f2","_f_v.__vertical_length3f4","_f_v.__vertical_length3f8","_f_v.__vertical_length3f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z8distanceDv4_fS_","_f_v.__vertical_distance4f2","_f_v.__vertical_distance4f4","_f_v.__vertical_distance4f8","_f_v.__vertical_distance4f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z6lengthDv4_f","_f_v.__vertical_length4f2","_f_v.__vertical_length4f4","_f_v.__vertical_length4f8","_f_v.__vertical_length4f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z3dotff","_f_v.__vertical_dot1f2","_f_v.__vertical_dot1f4","_f_v.__vertical_dot1f8","_f_v.__vertical_dot1f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z3dotDv2_fS_","_f_v.__vertical_dot2f2","_f_v.__vertical_dot2f4","_f_v.__vertical_dot2f8","_f_v.__vertical_dot2f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z3dotDv3_fS_","_f_v.__vertical_dot3f2","_f_v.__vertical_dot3f4","_f_v.__vertical_dot3f8","_f_v.__vertical_dot3f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z3dotff4","_f_v.__vertical_dot4f2","_f_v.__vertical_dot4f4","_f_v.__vertical_dot4f8","_f_v.__vertical_dot4f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z11fast_lengthf","_f_v.__vertical_fast_length1f2","_f_v.__vertical_fast_length1f4","_f_v.__vertical_fast_length1f8","_f_v.__vertical_fast_length1f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z11fast_lengthDv2_f","_f_v.__vertical_fast_length2f2","_f_v.__vertical_fast_length2f4","_f_v.__vertical_fast_length2f8","_f_v.__vertical_fast_length2f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z11fast_lengthDv3_f","_f_v.__vertical_fast_length3f2","_f_v.__vertical_fast_length3f4","_f_v.__vertical_fast_length3f8","_f_v.__vertical_fast_length3f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z11fast_lengthDv4_f","_f_v.__vertical_fast_length4f2","_f_v.__vertical_fast_length4f4","_f_v.__vertical_fast_length4f8","_f_v.__vertical_fast_length4f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z14fast_normalizef","_f_v.__vertical_fast_normalize1f2","_f_v.__vertical_fast_normalize1f4","_f_v.__vertical_fast_normalize1f8","_f_v.__vertical_fast_normalize1f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z14fast_normalizeDv2_f","_f_v.__vertical_fast_normalize2f2","_f_v.__vertical_fast_normalize2f4","_f_v.__vertical_fast_normalize2f8","_f_v.__vertical_fast_normalize2f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z14fast_normalizeDv3_f","_f_v.__vertical_fast_normalize3f2","_f_v.__vertical_fast_normalize3f4","_f_v.__vertical_fast_normalize3f8","_f_v.__vertical_fast_normalize3f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z14fast_normalizef4","_f_v.__vertical_fast_normalize4f2","_f_v.__vertical_fast_normalize4f4","_f_v.__vertical_fast_normalize4f8","_f_v.__vertical_fast_normalize4f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z9normalizef","_f_v.__vertical_normalize1f2","_f_v.__vertical_normalize1f4","_f_v.__vertical_normalize1f8","_f_v.__vertical_normalize1f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z9normalizeDv2_f","_f_v.__vertical_normalize2f2","_f_v.__vertical_normalize2f4","_f_v.__vertical_normalize2f8","_f_v.__vertical_normalize2f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z9normalizeDv3_f","_f_v.__vertical_normalize3f2","_f_v.__vertical_normalize3f4","_f_v.__vertical_normalize3f8","_f_v.__vertical_normalize3f16",INVALID_ENTRY},0,1},
  {{"_f_v._Z9normalizeDv4_f","_f_v.__vertical_normalize4f2","_f_v.__vertical_normalize4f4","_f_v.__vertical_normalize4f8","_f_v.__vertical_normalize4f16",INVALID_ENTRY},0,1},
   */
  
  // Do not try to scalarize/vectorize prefetches. No, really.
  {{"_Z8prefetchPKU3AS1cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv2_dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv4_dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv8_dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },
  {{"_Z8prefetchPKU3AS1Dv16_dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, 0, 0 },

  // read / write image
  {{"_f_v._Z11read_imagefPU3AS110_image2d_tuSamplerDv2_f",INVALID_ENTRY,"_Z33__read_transposed_imagef_resamplePU3AS110_image2d_tuSamplerDv4_fS1_PS1_S2_S2_S2_",
    "_Z33__read_transposed_imagef_resamplePU3AS110_image2d_tuSamplerDv8_fS1_PS1_S2_S2_S2_",INVALID_ENTRY,INVALID_ENTRY},0,1},
  {{"_f_v._Z11read_imagefPU3AS110_image3d_tuSamplerDv4_f",INVALID_ENTRY,"_Z36__read_transposed_3d_imagef_resamplePU3AS110_image3d_tuSamplerDv4_fS1_S1_PS1_S2_S2_S2_",
    "_Z36__read_transposed_3d_imagef_resamplePU3AS110_image3d_tuSamplerDv8_fS1_S1_PS1_S2_S2_S2_",INVALID_ENTRY,INVALID_ENTRY},0,1},
  {{"_f_v._Z12write_imagefPU3AS110_image2d_tDv2_iDv4_f",INVALID_ENTRY,"_Z23write_transposed_imagefPU3AS110_image2d_tiiDv4_fS1_S1_S1_",
    "_Z23write_transposed_imagefPU3AS110_image2d_tiiDv8_fS1_S1_S1_",INVALID_ENTRY,INVALID_ENTRY},0,1},
  
  //ci_gamma
  {{"_f_v.__ci_gamma_scalar_SPI",INVALID_ENTRY,"_f_v.__ci_gamma_SPI","_f_v.__ci_gamma_SPI_8",INVALID_ENTRY,INVALID_ENTRY},0,1},

  // wrappers
  {{"allOne","allOne_v2","allOne_v4","allOne_v8","allOne_v16",INVALID_ENTRY},0,1},
  {{"allZero","allZero_v2","allZero_v4","allZero_v8","allZero_v16",INVALID_ENTRY},0,1},
  
  
};
#else
TableRow mappings[] = {
  // {{scalar_version, width2_version, ..., width16_version, width3_version}, isScalarizable, isPacketizable}
  { {"allOne","allOne_v2","allOne_v4","allOne_v8","allOne_v16",INVALID_ENTRY}, false, true},
  { {"allZero","allZero_v2","allZero_v4","allZero_v8","allZero_v16",INVALID_ENTRY}, false, true},
  { {"get_global_id", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"get_global_size", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_read_imageuii9image2d_tjDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_read_imageuii9image2d_tjDv2_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z18mask_write_imageuii9image2d_tDv2_iDv4_j", INVALID_ENTRY, "_Z23mask_soa4_write_imageuiDv4_i9image2d_tS_S_Dv4_jS0_S0_S0_", "_Z23mask_soa8_write_imageuiDv8_i9image2d_tS_S_Dv8_jS0_S0_S0_", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  { {"_Z16mask_read_imageii9image2d_tjDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imageii9image2d_tjDv2_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_write_imageii9image2d_tDv2_iDv4_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imagefi9image2d_tjDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imagefi9image2d_tjDv2_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_write_imagefi9image2d_tDv2_iDv4_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imagefi9image2d_tDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imageii9image2d_tDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_read_imageuii9image2d_tDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  // Image properties functions
  {{"_Z16get_image_height9image2d_t", INVALID_ENTRY, "_Z21soa4_get_image_height9image2d_t", "_Z21soa8_get_image_height9image2d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width9image3d_t", INVALID_ENTRY, "_Z20soa4_get_image_width9image3d_t", "_Z20soa8_get_image_width9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order9image3d_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order9image3d_t", "_Z28soa8_get_image_channel_order9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width15image2d_array_t", INVALID_ENTRY, "_Z20soa4_get_image_width15image2d_array_t", "_Z20soa8_get_image_width15image2d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width9image2d_t", INVALID_ENTRY, "_Z20soa4_get_image_width9image2d_t", "_Z20soa8_get_image_width9image2d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type15image2d_array_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type15image2d_array_t", "_Z32soa8_get_image_channel_data_type15image2d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type9image3d_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type9image3d_t", "_Z32soa8_get_image_channel_data_type9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type9image1d_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type9image1d_t", "_Z32soa8_get_image_channel_data_type9image1d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z16get_image_height9image3d_t", INVALID_ENTRY, "_Z21soa4_get_image_height9image3d_t", "_Z21soa8_get_image_height9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_depth9image3d_t", INVALID_ENTRY, "_Z20soa4_get_image_depth9image3d_t", "_Z20soa8_get_image_depth9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width9image1d_t", INVALID_ENTRY, "_Z20soa4_get_image_width9image1d_t", "_Z20soa8_get_image_width9image1d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order16image1d_buffer_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order16image1d_buffer_t", "_Z28soa8_get_image_channel_order16image1d_buffer_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type9image2d_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type9image2d_t", "_Z32soa8_get_image_channel_data_type9image2d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type15image1d_array_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type15image1d_array_t", "_Z32soa8_get_image_channel_data_type15image1d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order9image2d_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order9image2d_t", "_Z28soa8_get_image_channel_order9image2d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width15image1d_array_t", INVALID_ENTRY, "_Z20soa4_get_image_width15image1d_array_t", "_Z20soa8_get_image_width15image1d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order9image1d_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order9image1d_t", "_Z28soa8_get_image_channel_order9image1d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width16image1d_buffer_t", INVALID_ENTRY, "_Z20soa4_get_image_width16image1d_buffer_t", "_Z20soa8_get_image_width16image1d_buffer_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order15image1d_array_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order15image1d_array_t", "_Z28soa8_get_image_channel_order15image1d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z16get_image_height15image2d_array_t", INVALID_ENTRY, "_Z21soa4_get_image_height15image2d_array_t", "_Z21soa8_get_image_height15image2d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order15image2d_array_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order15image2d_array_t", "_Z28soa8_get_image_channel_order15image2d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type16image1d_buffer_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type16image1d_buffer_t", "_Z32soa8_get_image_channel_data_type16image1d_buffer_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  //// Image reading builtins
  {{"_Z12read_imageui9image2d_tjDv2_i", INVALID_ENTRY, "_Z17soa4_read_imageui9image2d_tjDv4_iS_PDv4_jS1_S1_S1_", "_Z17soa8_read_imageui9image2d_tjDv8_iS_PDv8_jS1_S1_S1_", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  //// Image writing builtins
  {{"_Z13write_imageui9image2d_tDv2_iDv4_j", INVALID_ENTRY, "_Z18soa4_write_imageui9image2d_tDv4_iS_Dv4_jS0_S0_S0_", "_Z18soa8_write_imageui9image2d_tDv8_iS_Dv8_jS0_S0_S0_", INVALID_ENTRY, INVALID_ENTRY}, false, true},
//this file is automatically gnerated by tblgen. It is strongly recmommended to use this mechanism, not to write string hardcoded
#include "CustomMappings.gen"
};
#endif
#endif//__CUSTOM_VERSION_MAPPING_H__
