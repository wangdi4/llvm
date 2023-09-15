// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "DataType.h"
#include "dxfloat.h"
#include "llvm/Support/DataTypes.h"

using namespace Validation;

DataTypeValWrapper::DataTypeMetadataMap
    DataTypeValWrapper::m_metaData(initStaticMap());

DataTypeValWrapper::DataTypeMetadataMap DataTypeValWrapper::initStaticMap() {
  DataTypeMetadataMap metaData;

  metaData[F16] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(CFloat16), true, "f16");
  metaData[F32] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(float), true, "f32");
  metaData[F64] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(double), true, "f64");

  metaData[I8] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(int8_t), false, "i8");
  metaData[I16] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(int16_t), false, "i16");
  metaData[I32] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(int32_t), false, "i32");
  metaData[I64] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(int64_t), false, "i64");

  metaData[U8] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(uint8_t), false, "u8");
  metaData[U16] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(uint16_t), false, "u16");
  metaData[U32] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(uint32_t), false, "u32");
  metaData[U64] =
      DataTypeValWrapper::DataTypeMetadata(sizeof(uint64_t), false, "u64");

  return metaData;
}
