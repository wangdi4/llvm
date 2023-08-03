# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2023 Intel Corporation
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

# This is a temporary solution for importing onnxruntime as a target.
# Should be replaced with find_package once 
# https://github.com/microsoft/onnxruntime/pull/8919
# is merged.

#########################

# ONNXRuntime build
# This will define the OnnxRuntime following variables:
#   onnxruntime_FOUND        -- True if the system has the onnxruntime library
#   onnxruntime_INCLUDE_DIRS -- The include directories for onnxruntime
#   onnxruntime_LIBRARIES    -- Libraries to link against
#   onnxruntime_CXX_FLAGS    -- Additional (required) compiler flags

include(FindPackageHandleStandardArgs)

if(NOT DEFINED onnxruntime_INSTALL_PREFIX)
    message(FATAL_ERROR "The onnxruntime_INSTALL_PREFIX var should point to the ONNX runtime directory")
endif()

# Copy ONNX libs/headers locally to the build to avoid leaking their
# location on a shared drive.
#
set(ONNX_DIR ${CMAKE_CURRENT_BINARY_DIR}/onnx)
file(GLOB ONNX_FILES
  "${onnxruntime_INSTALL_PREFIX}/*"
)
file(COPY ${ONNX_FILES} DESTINATION ${ONNX_DIR})

set(onnxruntime_INCLUDE_DIRS ${ONNX_DIR}/include)
set(onnxruntime_CXX_FLAGS "") # no flags needed

find_library(onnxruntime_LIBRARY onnxruntime PATHS "${ONNX_DIR}/lib")

add_library(onnxruntime SHARED IMPORTED)

get_filename_component(onnxruntime_LIBRARY_real_path "${onnxruntime_LIBRARY}" REALPATH)
set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION "${onnxruntime_LIBRARY_real_path}")
set_property(TARGET onnxruntime PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${onnxruntime_INCLUDE_DIRS}")
set_property(TARGET onnxruntime PROPERTY INTERFACE_COMPILE_OPTIONS "${onnxruntime_CXX_FLAGS}")
if(WIN32)
    set_target_properties(onnxruntime PROPERTIES
      IMPORTED_IMPLIB "${ONNX_DIR}/lib/onnxruntime.lib"
      IMPORTED_LOCATION "${ONNX_DIR}/lib/onnxruntime.dll"
    )
    # copy ONNX runtime to the build so it can be found during LIT testing
    file(COPY "${ONNX_DIR}/lib/onnxruntime.dll" DESTINATION ${CMAKE_BINARY_DIR}/bin)
else()
    file(COPY ${onnxruntime_LIBRARY}  DESTINATION ${CMAKE_BINARY_DIR}/lib FOLLOW_SYMLINK_CHAIN)
endif()

install(IMPORTED_RUNTIME_ARTIFACTS onnxruntime
    RUNTIME DESTINATION "bin" COMPONENT onnxruntime
    LIBRARY DESTINATION "lib${LLVM_LIBDIR_SUFFIX}" COMPONENT onnxruntime
)

if(WIN32)
# Some Windows distributions carry their own onnxruntime.dll in C:/Windows/system32.
# Those can clash with our onnxruntime.dll because the DLL are searched in
# the system folders before they are searched in PATH. But even higher preference is
# given to DLL that are in the same folder as the app using it. We already installed
# it to "bin", so do the same to "bin/compiler".
#
# NOTE: oddly the symbolic links while are being created not loaded correctly afterwards.
# So install a copy of onnxruntime.dll until we have a better solution.
#
#   install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
#       \"../onnxruntime.dll\"
#       \"${CMAKE_INSTALL_PREFIX}/bin/compiler/onnxruntime.dll\")"
#       COMPONENT onnxruntime)

    install(FILES "${ONNX_DIR}/lib/onnxruntime.dll"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/compiler/"
        COMPONENT onnxruntime)
endif()

find_package_handle_standard_args(onnxruntime DEFAULT_MSG onnxruntime_LIBRARY onnxruntime_INCLUDE_DIRS)

#############################

if(NOT DEFINED ONNX_MODEL_PATH)
    message(FATAL_ERROR "The ONNX_MODEL_PATH var should point to the directory containing ONNX model")
endif()

cmake_path(SET ONNX_MODEL_PATH "${ONNX_MODEL_PATH}/icx_bp_0.1.2.onnx")
message("MLPGO. Using model: ${ONNX_MODEL_PATH}")

set(BINARY_BLOB_NAME OnnxModel)
add_custom_command(OUTPUT onnx_model.inc
  COMMAND ${Python3_EXECUTABLE} ${LLVM_SOURCE_DIR}/utils/convert-to-c-array.py --input ${ONNX_MODEL_PATH} --output onnx_model.inc --name ${BINARY_BLOB_NAME}
  DEPENDS ${LLVM_SOURCE_DIR}/utils/convert-to-c-array.py ${ONNX_MODEL_PATH}
  COMMENT "Converting : ${ONNX_MODEL_PATH} to c-array"
)


# The library calls onnxruntime API which may throw an exception
# so we need exception support as well to be able to catch them.
set(LLVM_REQUIRES_EH ON)
set(LLVM_REQUIRES_RTTI ON)

add_llvm_component_library(LLVMMLPGOMLBackend
  Model.cpp
  ModelBase.cpp
  BranchPredictionModel.cpp
  onnx_model.inc

  LINK_COMPONENTS
  Support

  LINK_LIBS
  onnxruntime
)

target_compile_definitions(LLVMMLPGOMLBackend PRIVATE ONNX_MODEL=${BINARY_BLOB_NAME})
