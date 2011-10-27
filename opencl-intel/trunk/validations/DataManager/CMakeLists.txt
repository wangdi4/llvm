# TinyXML library is used for serialization to XML.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml)

# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

# Set useful variable for all header files to manipulate them all together.
set (DM_INCLUDE_FILES
  ${VALIDATIONS_ROOT}/Common/Exception.h
  ${VALIDATIONS_ROOT}/Common/VectorWidth.h
  ${VALIDATIONS_ROOT}/Common/DataType.h
  ${VALIDATIONS_ROOT}/Common/ImageChannelOrder.h
  ${VALIDATIONS_ROOT}/Common/ImageChannelDataType.h
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
  ${VALIDATIONS_ROOT}/NEAT/NEATValue.h
  )

# Data Manager Libarary
add_llvm_library(DataManager
  ${DM_INCLUDE_FILES}
  VectorWidth.cpp
  DataType.cpp
  TypeDesc.cpp
  ImageChannelOrder.cpp
  ImageChannelDataType.cpp
  Image.cpp
  XMLDataReadWrite.cpp
  Buffer.cpp
  BufferContainer.cpp
)

target_link_libraries(DataManager tinyxml_STL NEAT)
add_dependencies(DataManager tinyxml_STL NEAT)
