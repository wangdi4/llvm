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

File Name:  TypeDesc.cpp

\*****************************************************************************/
#include "llvm/System/DataTypes.h"

#include "TypeDesc.h"
#include "dxfloat.h"
#include "NEATValue.h"
using namespace Validation;

std::map<TypeVal, TypeValWrapper::TypeMetadata> TypeValWrapper::m_metaData;

bool TypeValWrapper::m_isStaticInit = false;

void TypeValWrapper::initStatic()
{
    // TODO: not thread safe
    m_metaData.clear();

    m_metaData[THALF]    = TypeValWrapper::TypeMetadata(sizeof(CFloat16),    true, "f16");
    m_metaData[TFLOAT]   = TypeValWrapper::TypeMetadata(sizeof(float),       true, "f32");
    m_metaData[TDOUBLE]  = TypeValWrapper::TypeMetadata(sizeof(double),      true, "f64");

    m_metaData[TCHAR]    = TypeValWrapper::TypeMetadata(sizeof(int8_t),      false, "i8");
    m_metaData[TSHORT]   = TypeValWrapper::TypeMetadata(sizeof(int16_t),     false, "i16");
    m_metaData[TINT]     = TypeValWrapper::TypeMetadata(sizeof(int32_t),     false, "i32");
    m_metaData[TLONG]    = TypeValWrapper::TypeMetadata(sizeof(int64_t),     false, "i64");

    m_metaData[TUCHAR]   = TypeValWrapper::TypeMetadata(sizeof(uint8_t),     false, "u8");
    m_metaData[TUSHORT]  = TypeValWrapper::TypeMetadata(sizeof(uint16_t),    false, "u16");
    m_metaData[TUINT]    = TypeValWrapper::TypeMetadata(sizeof(uint32_t),    false, "u32");
    m_metaData[TULONG]   = TypeValWrapper::TypeMetadata(sizeof(uint64_t),    false, "u64");

    m_metaData[TBOOL]    = TypeValWrapper::TypeMetadata(sizeof(bool),        false, "bool");

    m_metaData[TVOID]    = TypeValWrapper::TypeMetadata(0,                   false, "void");

    m_metaData[TVECTOR]  = TypeValWrapper::TypeMetadata(0,                   false, "vector");
    m_metaData[TARRAY]   = TypeValWrapper::TypeMetadata(0,                   false, "array");
    m_metaData[TSTRUCT]  = TypeValWrapper::TypeMetadata(0,                   false, "struct");
    m_metaData[TPOINTER] = TypeValWrapper::TypeMetadata(0,                   false, "pointer");

    m_metaData[UNSPECIFIED_TYPE] = TypeValWrapper::TypeMetadata(0,          false, "unspecified");
    m_metaData[INVALID_TYPE] = TypeValWrapper::TypeMetadata(0,              false, "invalid");

    m_isStaticInit = true;

}

void TypeDesc::SetUpSubTypes(std::size_t in_numOfStructureSubTypes)
{
    switch(m_type.GetValue())
    {
    case THALF:
    case TFLOAT:
    case TDOUBLE:
    case TCHAR:
    case TSHORT:
    case TINT:
    case TLONG:
    case TUCHAR:
    case TUSHORT:
    case TUINT:
    case TULONG:
    case TBOOL:
    case TVOID:
    case UNSPECIFIED_TYPE:
    case INVALID_TYPE:
        m_subTypes.clear();
        break;
    case TPOINTER:
    case TVECTOR:
    case TARRAY:
        m_subTypes.resize(1);
        break;
    case TSTRUCT:
        m_subTypes.resize(in_numOfStructureSubTypes);
        break;
    }
}

bool TypeDesc::IsNEAT() const
{
    bool ret = m_isNEAT;
    for (std::size_t i = 0; i < m_subTypes.size(); ++i)
    {
        ret = ret || m_subTypes[i].IsNEAT();
    }
    return ret;
}

void TypeDesc::SetNeat(const bool inNEAT)
{
    m_isNEAT = inNEAT;
    for (std::size_t i = 0; i < m_subTypes.size(); ++i)
    {
        m_subTypes[i].SetNeat(inNEAT);
    }
    // Check the case if m_size != 0 for type description of NEAT value.
    assert(!(inNEAT == false && m_isNEAT == true && m_size != 0));
    /// set size to zero as previously calculated size is not valid anymore
    if (m_isNEAT)
        m_size = 0;
}

std::size_t TypeDesc::GetSizeInBytes() const
{
    /// works only for neat non-composite elements
    if (IsNEAT())
    {
        if (!m_type.IsComposite() && IsFloatingPoint())
            return sizeof(NEATValue);
        if (m_type.IsInteger()) // we do not store integer types in NEAT
            return 0;
    }
    if (m_size != 0)
    {
        return m_size;
    }
    if (m_type.GetSizeInBytes() == 0 || IsNEAT())
    {
        switch(m_type.GetValue())
        {
        case TVOID:
            return 1;
        case TVECTOR:
            {
                /// According to openCL spec 3-element vectors are allocated as 4-element vectors
                int numElems = (m_numElements == 3) ? 4 : m_numElements;
                return numElems * m_subTypes[0].GetSizeInBytes();
            }
        case TARRAY:
        case TPOINTER:
            // TODO: Make sure that code is correct for pointers.
            return m_numElements * m_subTypes[0].GetSizeInBytes();
        case TSTRUCT:
            {
            std::size_t typeSize = m_subTypes[0].GetSizeInBytes();
            for (uint64_t i = 1; i < m_subTypes.size(); ++i)
            {
                typeSize += m_subTypes[i].GetSizeInBytes();
            }
            return typeSize;
            }
        default:
            throw Exception::OutOfRange("Unable to calculate size of type.");
        }
    }
    return m_type.GetSizeInBytes();
}

bool TypeDesc::operator == (const TypeDesc& a) const
{
    bool ret = true;
    ret = ret && (m_type.GetValue() == a.m_type.GetValue());
    ret = ret && (m_numElements == a.m_numElements);
    ret = ret && (m_subTypes.size() == a.m_subTypes.size());
    for (uint64_t i = 0; i < m_subTypes.size(); ++i)
    {
        ret = ret && (m_subTypes[i] == a.m_subTypes[i]);
    }
    return ret;
}
