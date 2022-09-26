# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION

set(imf_fp32_fallback_src_list imf_utils/integer_misc.cpp
                               imf_utils/half_convert.cpp
                               imf_utils/float_convert.cpp
                               imf_utils/simd_emulate.cpp
                               # INTEL_CUSTOMIZATION
                               imf/intel/cos_h_la.cpp
                               imf/intel/exp10_h_la.cpp
                               imf/intel/exp2_h_la.cpp
                               imf/intel/exp_h_la.cpp
                               imf/intel/ln_h_la.cpp
                               imf/intel/log10_h_la.cpp
                               imf/intel/log2_h_la.cpp
                               imf/intel/sin_h_la.cpp
                               imf/intel/erfinv_s_ha.cpp
                               imf/intel/cos_s_ha.cpp
                               imf/intel/cbrt_s_la.cpp
                               imf/intel/atan_s_ha.cpp
                               imf/intel/atanh_s_la.cpp
                               imf/intel/atan2_s_la.cpp
                               imf/intel/asin_s_la.cpp
                               imf/intel/asinh_s_la.cpp
                               imf/intel/acos_s_ha.cpp
                               imf/intel/acosh_s_la.cpp
                               imf/intel/erf_s_ha.cpp
                               imf/intel/cospi_s_ha.cpp
                               imf/intel/cosh_s_la.cpp
                               imf/intel/frexp_s_xa.cpp
                               imf/intel/fmod_s_xa.cpp
                               imf/intel/fdim_s_ha.cpp
                               imf/intel/exp_s_la.cpp
                               imf/intel/expm1_s_ha.cpp
                               imf/intel/exp2_s_la.cpp
                               imf/intel/exp10_s_la.cpp
                               imf/intel/erfc_s_la.cpp
                               imf/intel/erfcinv_s_la.cpp
                               imf/intel/norm3d_s_la.cpp
                               imf/intel/nextafter_s_xa.cpp
                               imf/intel/modf_s_xa.cpp
                               imf/intel/logb_s_xa.cpp
                               imf/intel/log2_s_ha.cpp
                               imf/intel/log1p_s_ha.cpp
                               imf/intel/log10_s_la.cpp
                               imf/intel/ln_s_ha.cpp
                               imf/intel/lgamma_s_ep.cpp
                               imf/intel/ldexp_s_xa.cpp
                               imf/intel/invcbrt_s_ha.cpp
                               imf/intel/ilogb_s_xa.cpp
                               imf/intel/tan_s_la.cpp
                               imf/intel/sin_s_ha.cpp
                               imf/intel/sinpi_s_ha.cpp
                               imf/intel/sinh_s_la.cpp
                               imf/intel/sincos_s_ha.cpp
                               imf/intel/round_s_xa.cpp
                               imf/intel/rnorm4d_s_la.cpp
                               imf/intel/rnorm3d_s_la.cpp
                               imf/intel/rhypot_s_la.cpp
                               imf/intel/remquo_s_xa.cpp
                               imf/intel/remainder_s_xa.cpp
                               imf/intel/pow_s_la.cpp
                               imf/intel/norm4d_s_la.cpp
                               imf/intel/tgamma_s_la.cpp
                               imf/intel/tanh_s_ha.cpp
                               imf/intel/cdfnorm_s_la.cpp
                               imf/intel/erfcx_s_la.cpp
                               imf/intel/hypot_s_la.cpp
                               imf/intel/isfinite_s_xa.cpp
                               imf/intel/isinf_s_xa.cpp
                               imf/intel/isnan_s_xa.cpp
                               imf/intel/llrint_s_xa.cpp
                               imf/intel/llround_s_xa.cpp
                               imf/intel/lrint_s_xa.cpp
                               imf/intel/lround_s_xa.cpp
                               imf/intel/nan_s_xa.cpp
                               imf/intel/scalbn_s_xa.cpp
                               imf/intel/signbit_s_xa.cpp
                               # end INTEL_CUSTOMIZATION
                               imf/imf_inline_fp32.cpp)

set(imf_fp64_fallback_src_list imf_utils/double_convert.cpp
                               # INTEL_CUSTOMIZATION
                               imf/intel/erfinv_d_la.cpp
                               imf/intel/acos_d_ha.cpp
                               imf/intel/acosh_d_la.cpp
                               imf/intel/asin_d_ha.cpp
                               imf/intel/asinh_d_la.cpp
                               imf/intel/atan2_d_ha.cpp
                               imf/intel/atan_d_ha.cpp
                               imf/intel/atanh_d_ha.cpp
                               imf/intel/cbrt_d_ha.cpp
                               imf/intel/cdfnorm_d_la.cpp
                               imf/intel/cdfnorminv_d_la.cpp
                               imf/intel/cos_d_ha.cpp
                               imf/intel/cosh_d_ha.cpp
                               imf/intel/cospi_d_ha.cpp
                               imf/intel/erfc_d_la.cpp
                               imf/intel/erfcinv_d_la.cpp
                               imf/intel/erfcx_d_la.cpp
                               imf/intel/erf_d_ha.cpp
                               imf/intel/exp10_d_la.cpp
                               imf/intel/exp2_d_ha.cpp
                               imf/intel/exp_d_ha.cpp
                               imf/intel/expm1_d_ha.cpp
                               imf/intel/fdim_d_ha.cpp
                               imf/intel/fmod_d_xa.cpp
                               imf/intel/frexp_d_xa.cpp
                               imf/intel/hypot_d_la.cpp
                               imf/intel/ilogb_d_xa.cpp
                               imf/intel/invcbrt_d_ha.cpp
                               imf/intel/isfinite_d_xa.cpp
                               imf/intel/isinf_d_xa.cpp
                               imf/intel/isnan_d_xa.cpp
                               imf/intel/ldexp_d_xa.cpp
                               imf/intel/llrint_d_xa.cpp
                               imf/intel/llround_d_xa.cpp
                               imf/intel/ln_d_ha.cpp
                               imf/intel/log10_d_ha.cpp
                               imf/intel/log1p_d_ha.cpp
                               imf/intel/log2_d_ha.cpp
                               imf/intel/logb_d_xa.cpp
                               imf/intel/lrint_d_xa.cpp
                               imf/intel/lround_d_xa.cpp
                               imf/intel/modf_d_xa.cpp
                               imf/intel/nan_d_xa.cpp
                               imf/intel/nextafter_d_xa.cpp
                               imf/intel/norm3d_d_la.cpp
                               imf/intel/norm4d_d_la.cpp
                               imf/intel/pow_d_la.cpp
                               imf/intel/remainder_d_xa.cpp
                               imf/intel/remquo_d_xa.cpp
                               imf/intel/rhypot_d_la.cpp
                               imf/intel/rnorm3d_d_la.cpp
                               imf/intel/rnorm4d_d_la.cpp
                               imf/intel/round_d_xa.cpp
                               imf/intel/scalbn_d_xa.cpp
                               imf/intel/signbit_d_xa.cpp
                               imf/intel/sincos_d_la.cpp
                               imf/intel/sincospi_d_la.cpp
                               imf/intel/sin_d_ha.cpp
                               imf/intel/sinh_d_la.cpp
                               imf/intel/sinpi_d_ha.cpp
                               imf/intel/tan_d_ha.cpp
                               imf/intel/tanh_d_ha.cpp
                               # end INTEL_CUSTOMIZATION
                               imf/imf_inline_fp64.cpp)

if (FP64 STREQUAL 0)
  set(imf_fallback_src_list ${imf_fp32_fallback_src_list})
  set(imf_fallback_dest ${DEST_DIR}/imf_fp32_fallback.cpp)
else()
  set(imf_fallback_src_list ${imf_fp64_fallback_src_list})
  set(imf_fallback_dest ${DEST_DIR}/imf_fp64_fallback.cpp)
endif()

set(flag 0)
foreach(src ${imf_fallback_src_list})
  file(READ ${SRC_DIR}/${src} src_contents)
  if(flag STREQUAL 0)
    file(WRITE ${imf_fallback_dest} "${src_contents}")
    set(flag 1)
  else()
    file(APPEND ${imf_fallback_dest} "${src_contents}")
  endif()
endforeach()
