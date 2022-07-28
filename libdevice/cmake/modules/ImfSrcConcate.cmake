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
                               imf/imf_inline_fp32.cpp
                               imf/intel/erfinv_s_ha.cpp)
                               # end INTEL_CUSTOMIZATION

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
