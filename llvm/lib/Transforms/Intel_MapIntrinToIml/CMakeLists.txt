# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
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
# FIXME - We should not have to disable this warning. We need to fix the
#         violations in the source code and re-enable it.
#if (UNIX)
#  set_source_files_properties(iml_accuracy_interface.cpp PROPERTIES
#                              COMPILE_FLAGS -Wno-error=strict-aliasing)
#endif()

add_llvm_component_library(LLVMIntel_MapIntrinToIml
    MapIntrinToIml.cpp
    iml_accuracy_interface.cpp
    search.cpp

    DEPENDS
    intrinsics_gen

    LINK_COMPONENTS
    Analysis
    Core
    Scalar
    Support
    TargetParser
    TransformUtils
    )
