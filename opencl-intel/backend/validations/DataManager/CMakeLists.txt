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

use_eh(TRUE)

include_directories(${LLVM_INCLUDE_DIRS})
# TinyXML library is used for serialization to XML.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml)
# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)
include_directories(${VALIDATIONS_ROOT}/DataVersion)

# Set useful variable for all header files to manipulate them all together.
set(DM_INCLUDE_FILES
    ${VALIDATIONS_ROOT}/Common/Exception.h
    ${VALIDATIONS_ROOT}/Common/VectorWidth.h
    ${VALIDATIONS_ROOT}/Common/DataType.h
    ${VALIDATIONS_ROOT}/Common/BufferDesc.h
    ${VALIDATIONS_ROOT}/Common/ImageDesc.h
    ${VALIDATIONS_ROOT}/Common/TypeDesc.h
    ${VALIDATIONS_ROOT}/Common/FloatOperations.h
    ${VALIDATIONS_ROOT}/Common/IMemoryObjectDesc.h
    IContainer.h
    IContainerVisitor.h
    IBufferContainer.h
    IBufferContainerList.h
    Buffer.h
    Image.h
    BufferContainer.h
    BufferContainerList.h
    IDataReader.h
    IDataWriter.h
    XMLDataReader.h
    XMLDataWriter.h
    BinaryDataReader.h
    BinaryDataWriter.h
    XMLDataReadWrite.h
    ContainerCopier.h
    ${VALIDATIONS_ROOT}/NEAT/NEATValue.h)

# Data Manager Library
add_opencl_library(
  DataManager
  STATIC
  EXCLUDE_FROM_ALL_BUILD
  ${DM_INCLUDE_FILES}
  VectorWidth.cpp
  DataType.cpp
  TypeDesc.cpp
  ImageChannelOrder.cpp
  ImageChannelDataType.cpp
  ImageType.cpp
  Image.cpp
  XMLDataReadWrite.cpp
  Buffer.cpp
  BufferContainer.cpp)
