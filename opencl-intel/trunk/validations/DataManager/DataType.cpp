/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  DataType.cpp

\*****************************************************************************/
#include "llvm/System/DataTypes.h"

#include "DataType.h"
#include "dxfloat.h"
using namespace Validation;

std::map<DataTypeVal, DataTypeValWrapper::DataTypeMetadata> DataTypeValWrapper::m_metaData;

bool DataTypeValWrapper::m_isStaticInit = false;

void DataTypeValWrapper::initStatic()
{
	// TODO: not thread safe
	m_metaData.clear();

	m_metaData[F16] = DataTypeValWrapper::DataTypeMetadata(sizeof(CFloat16), true, "f16");
	m_metaData[F32] = DataTypeValWrapper::DataTypeMetadata(sizeof(float), true, "f32");
	m_metaData[F64] = DataTypeValWrapper::DataTypeMetadata(sizeof(double), true, "f64");

	m_metaData[I8] =	DataTypeValWrapper::DataTypeMetadata(sizeof(int8_t), false, "i8");
	m_metaData[I16] =	DataTypeValWrapper::DataTypeMetadata(sizeof(int16_t), false, "i16");
	m_metaData[I32] =	DataTypeValWrapper::DataTypeMetadata(sizeof(int32_t), false, "i32");
	m_metaData[I64] =	DataTypeValWrapper::DataTypeMetadata(sizeof(int64_t), false, "i64");

	m_metaData[U8] =	DataTypeValWrapper::DataTypeMetadata(sizeof(uint8_t), false, "u8");
	m_metaData[U16] =	DataTypeValWrapper::DataTypeMetadata(sizeof(uint16_t), false, "u16");
	m_metaData[U32] =	DataTypeValWrapper::DataTypeMetadata(sizeof(uint32_t), false, "u32");
	m_metaData[U64] =	DataTypeValWrapper::DataTypeMetadata(sizeof(uint64_t), false, "u64");

	m_isStaticInit = true;

}
