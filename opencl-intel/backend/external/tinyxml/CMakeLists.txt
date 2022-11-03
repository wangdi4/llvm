#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.
#

set(TINYXML_HEADERS tinystr.h tinyxml.h)

add_definitions(-DTIXML_USE_STL)

# disable "implicit-fallthrough" warning in TiXmlBase::ConvertUTF32ToUTF8()
if(NOT WIN32)
  add_compile_options(-Wno-implicit-fallthrough -Wno-suggest-override)
endif(NOT WIN32)

add_opencl_library(tinyxml_STL ${TINYXML_HEADERS} tinystr.cpp tinyxml.cpp
                   tinyxmlerror.cpp tinyxmlparser.cpp)
