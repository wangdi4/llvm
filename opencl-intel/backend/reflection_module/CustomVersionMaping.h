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
  {{"_f_v._Z5crossDv3_fS_","_f_v.__vertical_cross3f2","_f_v.__vertical_cross3f4","_f_v.__vertical_cross3f8","_f_v.__vertical_cross3f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z5crossDv4_fS_","_f_v.__vertical_cross4f2","_f_v.__vertical_cross4f4","_f_v.__vertical_cross4f8","_f_v.__vertical_cross4f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z13fast_distanceff","_f_v.__vertical_fast_distance1f2","_f_v.__vertical_fast_distance1f4","_f_v.__vertical_fast_distance1f8","_f_v.__vertical_fast_distance1f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z13fast_distanceDv2_fS_","_f_v.__vertical_fast_distance2f2","_f_v.__vertical_fast_distance2f4","_f_v.__vertical_fast_distance2f8","_f_v.__vertical_fast_distance2f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z13fast_distanceDv3_fS_","_f_v.__vertical_fast_distance3f2","_f_v.__vertical_fast_distance3f4","_f_v.__vertical_fast_distance3f8","_f_v.__vertical_fast_distance3f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z13fast_distanceDv4_fS_","_f_v.__vertical_fast_distance4f2","_f_v.__vertical_fast_distance4f4","_f_v.__vertical_fast_distance4f8","_f_v.__vertical_fast_distance4f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z8distanceff","_f_v.__vertical_distance1f2","_f_v.__vertical_distance1f4","_f_v.__vertical_distance1f8","_f_v.__vertical_distance1f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z6lengthf","_f_v.__vertical_length1f2","_f_v.__vertical_length1f4","_f_v.__vertical_length1f8","_f_v.__vertical_length1f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z8distanceDv2_fS_","_f_v.__vertical_distance2f2","_f_v.__vertical_distance2f4","_f_v.__vertical_distance2f8","_f_v.__vertical_distance2f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z6lengthDv2_f","_f_v.__vertical_length2f2","_f_v.__vertical_length2f4","_f_v.__vertical_length2f8","_f_v.__vertical_length2f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z8distanceDv3_fS_","_f_v.__vertical_distance3f2","_f_v.__vertical_distance3f4","_f_v.__vertical_distance3f8","_f_v.__vertical_distance3f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z6lengthDv3_f","_f_v.__vertical_length3f2","_f_v.__vertical_length3f4","_f_v.__vertical_length3f8","_f_v.__vertical_length3f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z8distanceDv4_fS_","_f_v.__vertical_distance4f2","_f_v.__vertical_distance4f4","_f_v.__vertical_distance4f8","_f_v.__vertical_distance4f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z6lengthDv4_f","_f_v.__vertical_length4f2","_f_v.__vertical_length4f4","_f_v.__vertical_length4f8","_f_v.__vertical_length4f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z3dotff","_f_v.__vertical_dot1f2","_f_v.__vertical_dot1f4","_f_v.__vertical_dot1f8","_f_v.__vertical_dot1f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z3dotDv2_fS_","_f_v.__vertical_dot2f2","_f_v.__vertical_dot2f4","_f_v.__vertical_dot2f8","_f_v.__vertical_dot2f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z3dotDv3_fS_","_f_v.__vertical_dot3f2","_f_v.__vertical_dot3f4","_f_v.__vertical_dot3f8","_f_v.__vertical_dot3f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z3dotff4","_f_v.__vertical_dot4f2","_f_v.__vertical_dot4f4","_f_v.__vertical_dot4f8","_f_v.__vertical_dot4f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z11fast_lengthf","_f_v.__vertical_fast_length1f2","_f_v.__vertical_fast_length1f4","_f_v.__vertical_fast_length1f8","_f_v.__vertical_fast_length1f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z11fast_lengthDv2_f","_f_v.__vertical_fast_length2f2","_f_v.__vertical_fast_length2f4","_f_v.__vertical_fast_length2f8","_f_v.__vertical_fast_length2f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z11fast_lengthDv3_f","_f_v.__vertical_fast_length3f2","_f_v.__vertical_fast_length3f4","_f_v.__vertical_fast_length3f8","_f_v.__vertical_fast_length3f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z11fast_lengthDv4_f","_f_v.__vertical_fast_length4f2","_f_v.__vertical_fast_length4f4","_f_v.__vertical_fast_length4f8","_f_v.__vertical_fast_length4f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z14fast_normalizef","_f_v.__vertical_fast_normalize1f2","_f_v.__vertical_fast_normalize1f4","_f_v.__vertical_fast_normalize1f8","_f_v.__vertical_fast_normalize1f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z14fast_normalizeDv2_f","_f_v.__vertical_fast_normalize2f2","_f_v.__vertical_fast_normalize2f4","_f_v.__vertical_fast_normalize2f8","_f_v.__vertical_fast_normalize2f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z14fast_normalizeDv3_f","_f_v.__vertical_fast_normalize3f2","_f_v.__vertical_fast_normalize3f4","_f_v.__vertical_fast_normalize3f8","_f_v.__vertical_fast_normalize3f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z14fast_normalizef4","_f_v.__vertical_fast_normalize4f2","_f_v.__vertical_fast_normalize4f4","_f_v.__vertical_fast_normalize4f8","_f_v.__vertical_fast_normalize4f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z9normalizef","_f_v.__vertical_normalize1f2","_f_v.__vertical_normalize1f4","_f_v.__vertical_normalize1f8","_f_v.__vertical_normalize1f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z9normalizeDv2_f","_f_v.__vertical_normalize2f2","_f_v.__vertical_normalize2f4","_f_v.__vertical_normalize2f8","_f_v.__vertical_normalize2f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z9normalizeDv3_f","_f_v.__vertical_normalize3f2","_f_v.__vertical_normalize3f4","_f_v.__vertical_normalize3f8","_f_v.__vertical_normalize3f16",INVALID_ENTRY}, false, true},
  {{"_f_v._Z9normalizeDv4_f","_f_v.__vertical_normalize4f2","_f_v.__vertical_normalize4f4","_f_v.__vertical_normalize4f8","_f_v.__vertical_normalize4f16",INVALID_ENTRY}, false, true},
   */
  
  // Do not try to scalarize/vectorize prefetches. No, really.
  {{"_Z8prefetchPKU3AS1cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv2_dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv4_dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv8_dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_cm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_hm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_sm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_tm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_im", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_jm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_lm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_mm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_fm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },
  {{"_Z8prefetchPKU3AS1Dv16_dm", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false },

  //TODO: This is rather suboptimal, but we don't have the v_ builtins, so just don't vectorize
  // We don't need this for the other relationals that have the same problem, since Apple's Clang implements them
  // as macros, so we never see them.
  {{"_Z18__cl_islessgreaterdd" , "_Z18__cl_islessgreaterDv2_fS_", "_Z18__cl_islessgreaterDv4_fS_", "_Z18__cl_islessgreaterDv8_fS_", "_Z18__cl_islessgreaterDv16_fS_", "_Z18__cl_islessgreaterDv3_fS_"}, false, false },
  {{"_Z18__cl_islessgreaterff" , "_Z18__cl_islessgreaterDv2_dS_", "_Z18__cl_islessgreaterDv4_dS_", "_Z18__cl_islessgreaterDv8_dS_", "_Z18__cl_islessgreaterDv16_dS_", "_Z18__cl_islessgreaterDv3_dS_"}, false, false },

  // fract, native_fract, modf, native_modf, sincos, native_sincos
  {{"_Z19__retbyvector_fractd" , INVALID_ENTRY, "_Z13fract_ret2ptrDv4_dPS_S0_" , "_Z13fract_ret2ptrDv8_dPS_S0_" , "_Z13fract_ret2ptrDv16_dPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z19__retbyvector_fractf" , INVALID_ENTRY, "_Z13fract_ret2ptrDv4_fPS_S0_" , "_Z13fract_ret2ptrDv8_fPS_S0_" , "_Z13fract_ret2ptrDv16_fPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z19__retbyvector_fractd" , "_Z18__retbyarray_fractDv2_d" , "_Z18__retbyarray_fractDv4_d" , "_Z18__retbyarray_fractDv8_d" , "_Z18__retbyarray_fractDv16_d" , "_Z18__retbyarray_fractDv3_d" }, true, false},
  {{"_Z19__retbyvector_fractf" , "_Z18__retbyarray_fractDv2_f" , "_Z18__retbyarray_fractDv4_f" , "_Z18__retbyarray_fractDv8_f" , "_Z18__retbyarray_fractDv16_f" , "_Z18__retbyarray_fractDv3_f" }, true, false},
  {{"_Z18__retbyvector_modfd" , INVALID_ENTRY, "_Z12modf_ret2ptrDv4_dPS_S0_" , "_Z12modf_ret2ptrDv8_dPS_S0_" , "_Z12modf_ret2ptrDv16_dPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z18__retbyvector_modff" , INVALID_ENTRY, "_Z12modf_ret2ptrDv4_fPS_S0_" , "_Z12modf_ret2ptrDv8_fPS_S0_" , "_Z12modf_ret2ptrDv16_fPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z18__retbyvector_modfd" , "_Z17__retbyarray_modfDv2_d" , "_Z17__retbyarray_modfDv4_d" , "_Z17__retbyarray_modfDv8_d" , "_Z17__retbyarray_modfDv16_d" , "_Z17__retbyarray_modfDv3_d" }, true, false},
  {{"_Z18__retbyvector_modff" , "_Z17__retbyarray_modfDv2_f" , "_Z17__retbyarray_modfDv4_f" , "_Z17__retbyarray_modfDv8_f" , "_Z17__retbyarray_modfDv16_f" , "_Z17__retbyarray_modfDv3_f" }, true, false},
  {{"_Z26__retbyvector_native_fractd" , INVALID_ENTRY, "_Z20native_fract_ret2ptrDv4_dPS_S0_" , "_Z20native_fract_ret2ptrDv8_dPS_S0_" , "_Z20native_fract_ret2ptrDv16_dPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z26__retbyvector_native_fractf" , INVALID_ENTRY, "_Z20native_fract_ret2ptrDv4_fPS_S0_" , "_Z20native_fract_ret2ptrDv8_fPS_S0_" , "_Z20native_fract_ret2ptrDv16_fPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z26__retbyvector_native_fractd" , "_Z25__retbyarray_native_fractDv2_d" , "_Z25__retbyarray_native_fractDv4_d" , "_Z25__retbyarray_native_fractDv8_d" , "_Z25__retbyarray_native_fractDv16_d" , "_Z25__retbyarray_native_fractDv3_d" }, true, false},
  {{"_Z26__retbyvector_native_fractf" , "_Z25__retbyarray_native_fractDv2_f" , "_Z25__retbyarray_native_fractDv4_f" , "_Z25__retbyarray_native_fractDv8_f" , "_Z25__retbyarray_native_fractDv16_f" , "_Z25__retbyarray_native_fractDv3_f" }, true, false},
  {{"_Z25__retbyvector_native_modfd" , INVALID_ENTRY, "_Z19native_modf_ret2ptrDv4_dPS_S0_" , "_Z19native_modf_ret2ptrDv8_dPS_S0_" , "_Z19native_modf_ret2ptrDv16_dPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z25__retbyvector_native_modff" , INVALID_ENTRY, "_Z19native_modf_ret2ptrDv4_fPS_S0_" , "_Z19native_modf_ret2ptrDv8_fPS_S0_" , "_Z19native_modf_ret2ptrDv16_fPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z25__retbyvector_native_modfd" , "_Z24__retbyarray_native_modfDv2_d" , "_Z24__retbyarray_native_modfDv4_d" , "_Z24__retbyarray_native_modfDv8_d" , "_Z24__retbyarray_native_modfDv16_d" , "_Z24__retbyarray_native_modfDv3_d" }, true, false},
  {{"_Z25__retbyvector_native_modff" , "_Z24__retbyarray_native_modfDv2_f" , "_Z24__retbyarray_native_modfDv4_f" , "_Z24__retbyarray_native_modfDv8_f" , "_Z24__retbyarray_native_modfDv16_f" , "_Z24__retbyarray_native_modfDv3_f" }, true, false},
  {{"_Z27__retbyvector_native_sincosd" , INVALID_ENTRY, "_Z21native_sincos_ret2ptrDv4_dPS_S0_" , "_Z21native_sincos_ret2ptrDv8_dPS_S0_" , "_Z21native_sincos_ret2ptrDv16_dPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z27__retbyvector_native_sincosf" , INVALID_ENTRY, "_Z21native_sincos_ret2ptrDv4_fPS_S0_" , "_Z21native_sincos_ret2ptrDv8_fPS_S0_" , "_Z21native_sincos_ret2ptrDv16_fPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z27__retbyvector_native_sincosd" , "_Z26__retbyarray_native_sincosDv2_d" , "_Z26__retbyarray_native_sincosDv4_d" , "_Z26__retbyarray_native_sincosDv8_d" , "_Z26__retbyarray_native_sincosDv16_d" , "_Z26__retbyarray_native_sincosDv3_d" }, true, false},
  {{"_Z27__retbyvector_native_sincosf" , "_Z26__retbyarray_native_sincosDv2_f" , "_Z26__retbyarray_native_sincosDv4_f" , "_Z26__retbyarray_native_sincosDv8_f" , "_Z26__retbyarray_native_sincosDv16_f" , "_Z26__retbyarray_native_sincosDv3_f" }, true, false},
  {{"_Z20__retbyvector_sincosd" , INVALID_ENTRY, "_Z14sincos_ret2ptrDv4_dPS_S0_" , "_Z14sincos_ret2ptrDv8_dPS_S0_" , "_Z14sincos_ret2ptrDv16_dPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z20__retbyvector_sincosf" , INVALID_ENTRY, "_Z14sincos_ret2ptrDv4_fPS_S0_" , "_Z14sincos_ret2ptrDv8_fPS_S0_" , "_Z14sincos_ret2ptrDv16_fPS_S0_" , INVALID_ENTRY}, false, true},
  {{"_Z20__retbyvector_sincosd" , "_Z19__retbyarray_sincosDv2_d" , "_Z19__retbyarray_sincosDv4_d" , "_Z19__retbyarray_sincosDv8_d" , "_Z19__retbyarray_sincosDv16_d" , "_Z19__retbyarray_sincosDv3_d" }, true, false},
  {{"_Z20__retbyvector_sincosf" , "_Z19__retbyarray_sincosDv2_f" , "_Z19__retbyarray_sincosDv4_f" , "_Z19__retbyarray_sincosDv8_f" , "_Z19__retbyarray_sincosDv16_f" , "_Z19__retbyarray_sincosDv3_f" }, true, false},
  
  // read / write image
  {{"_f_v._Z11read_imagefPU3AS110_image2d_tuSamplerDv2_f",INVALID_ENTRY,"_Z33__read_transposed_imagef_resamplePU3AS110_image2d_tuSamplerDv4_fS1_PS1_S2_S2_S2_",
    "_Z33__read_transposed_imagef_resamplePU3AS110_image2d_tuSamplerDv8_fS1_PS1_S2_S2_S2_",INVALID_ENTRY,INVALID_ENTRY}, false, true},
  {{"_f_v._Z11read_imagefPU3AS110_image3d_tuSamplerDv4_f",INVALID_ENTRY,"_Z36__read_transposed_3d_imagef_resamplePU3AS110_image3d_tuSamplerDv4_fS1_S1_PS1_S2_S2_S2_",
    "_Z36__read_transposed_3d_imagef_resamplePU3AS110_image3d_tuSamplerDv8_fS1_S1_PS1_S2_S2_S2_",INVALID_ENTRY,INVALID_ENTRY}, false, true},
  {{"_f_v._Z12write_imagefPU3AS110_image2d_tDv2_iDv4_f",INVALID_ENTRY,"_Z23write_transposed_imagefPU3AS110_image2d_tiiDv4_fS1_S1_S1_",
    "_Z23write_transposed_imagefPU3AS110_image2d_tiiDv8_fS1_S1_S1_",INVALID_ENTRY,INVALID_ENTRY}, false, true},
  
  //ci_gamma
  {{"_f_v.__ci_gamma_scalar_SPI",INVALID_ENTRY,"_f_v.__ci_gamma_SPI","_f_v.__ci_gamma_SPI_8",INVALID_ENTRY,INVALID_ENTRY}, false, true},

  // wrappers
  { {"__ocl_allOne","__ocl_allOne_v2","__ocl_allOne_v4","__ocl_allOne_v8","__ocl_allOne_v16",INVALID_ENTRY}, false, true},
  { {"__ocl_allZero","__ocl_allZero_v2","__ocl_allZero_v4","__ocl_allZero_v8","__ocl_allZero_v16",INVALID_ENTRY}, false, true},
  
  
};
#else
TableRow mappings[] = {
  // {{scalar_version, width2_version, ..., width16_version, width3_version}, isScalarizable, isPacketizable}
  { {"__ocl_allOne","__ocl_allOne_v2","__ocl_allOne_v4","__ocl_allOne_v8","__ocl_allOne_v16",INVALID_ENTRY}, false, true},
  { {"__ocl_allZero","__ocl_allZero_v2","__ocl_allZero_v4","__ocl_allZero_v8","__ocl_allZero_v16",INVALID_ENTRY}, false, true},
  { {"_Z13get_global_idj", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z15get_global_sizej", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
//this file is automatically gnerated by tblgen. It is strongly recmommended to use this mechanism, not to write string hardcoded
#include "CustomMappings.gen"
};
#endif
#endif//__CUSTOM_VERSION_MAPPING_H__
