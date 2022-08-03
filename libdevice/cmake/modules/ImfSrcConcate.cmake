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
                               # end INTEL_CUSTOMIZATION
                               imf/imf_inline_fp32.cpp)

set(imf_fp64_fallback_src_list imf_utils/double_convert.cpp
                               # INTEL_CUSTOMIZATION
                               imf/intel/erfinv_d_la.cpp
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
