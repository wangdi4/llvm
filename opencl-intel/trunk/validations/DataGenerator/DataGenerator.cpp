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

File Name:  DataGenerator.cpp

\*****************************************************************************/
#include "DataGenerator.h"
#include <BufferContainer.h>
#include "IMemoryObject.h"
#include "Buffer.h"

using namespace Validation;

void DataGenerator::InitGenerator(void)
{
    const uint32_t INIT_VALUE_1 = 6334;
    const uint32_t INIT_VALUE_2 = 18467;
    const uint32_t INIT_VALUE_3 = 41;

    //set init data for generator
    m_v11 = INIT_VALUE_1;
    m_v12 = INIT_VALUE_2;
    m_v13 = INIT_VALUE_3;
    m_v2 = (uint32_t)m_Seed;
    m_flag = 1;

    //warm up the generator
    for(int32_t i = 0; i < 10; i++)
    {
        Generator();
    }
}

double DataGenerator::Generator(void)
{
    const int32_t MPY_INT32 = 69069;
    const int32_t ADD_INT32 = 1013904243;
    const double MPY_DBL = 0.4656613e-9;

    int32_t local_1 = m_v12 - (m_v13 + m_flag);

    if( local_1 > 0)
    {
        m_flag = 0;
    }
    else
    {
        local_1 -= 18 ;
        m_flag = 1;
    }

    m_v13 = m_v12;
    m_v12 = m_v11;
    m_v11 = local_1;
/// casting to int32_t is important
    int32_t local_2 = MPY_INT32 * m_v2 + ADD_INT32;

    m_v2 = local_2;
    return (MPY_DBL * ( local_1 + local_2 ));
}

void DataGenerator::FillRandom( CFloat16* arr, int32_t len,
                                CFloat16 max, CFloat16 min )
{
    float local_max = float(max);
    float local_min = float(min);

    if (MyIsnan(local_max,local_max) || MyIsnan(local_min,local_min))
    {
        throw Exception::InvalidArgument(
        "[DataGenerator::FillRandom] bad data range limits");
    }
    if(local_max <= local_min)
    {
        throw Exception::InvalidArgument(
        "[DataGenerator::FillRandom] highValue is lower or equal to lowValue");
    }

    double add = (double)(local_max+local_min)/2.0;
    double mpy = (double)(local_max-local_min)/2.0;
    for( int32_t i = 0; i < len; i++ )
    {
        float a = float(mpy*Generator()+add);
        // to avoid issue like local_max= -17019 vs a=-17018 due to rounding error form double precision Generator()
        if(a > local_max) a = local_max;
        if(a < local_min) a = local_min;
        arr[i] = CFloat16(a);
    }
}

void DataGenerator::FillRandomSpecial( CFloat16* arr, int32_t len, float specialValuesProb )
{
    const int32_t specValLen = 6;
    CFloat16 specVal[specValLen];
    float prob;

    specVal[0] = CFloat16::GetNaN();
    specVal[1] = CFloat16::GetPInf();
    specVal[2] = CFloat16::GetNInf();
    specVal[3] = CFloat16(+0.f);
    specVal[4] = CFloat16(-0.f);
    MakeDenormValue(&specVal[5]);

    CFloat16 max = CFloat16::GetMax();
    CFloat16 min = CFloat16(0.f-float(max));

    FillRandom(arr,len,max,min);
    if( specialValuesProb > 0.0f)
    {
        for(int32_t j=0; j<len; j++)
        {
            prob = (float)(0.5*Generator()+0.5); // to make (0;1] from (-1;1]
            if( prob <= specialValuesProb)
            {
                uint32_t i = (uint32_t)((((double)specValLen)/2.0)*Generator()+((double)specValLen)/2.0);
                arr[j] = specVal[i%specValLen]; // just to be sure i less than specValLen
            }
        }
    }
}

/// internal function to make 16bit denormal value with ranodmly generated mantissa
void DataGenerator::MakeDenormValue(CFloat16* a)
{
    static const uint16_t FLOAT16_MANT_MASK = 0x03FF;
    double mpy, add;
    uint16_t u;

    mpy = add = ((double)FLOAT16_MANT_MASK-1.0)/2.0;
    u = (uint16_t)(mpy*Generator()+add);

    *a = *(CFloat16*)&u;
}
/// internal function to make 32bit denormal value with ranodmly generated mantissa
void DataGenerator::MakeDenormValue(float* a)
{
    static const uint32_t FLOAT_MANT_MASK = 0x007FFFFF;
    double mpy, add;
    FillDataType u;

    mpy = add = ((double)FLOAT_MANT_MASK-1.0)/2.0;
    u.dU32 = (uint32_t)(mpy*Generator()+add);

    *a = u.dF32;
}
/// internal function to make 64bit denormal value with ranodmly generated mantissa
void DataGenerator::MakeDenormValue(double* a)
{
    static const uint64_t DOUBLE_MANT_MASK = 0x000FFFFFFFFFFFFF;
    double mpy, add;
    FillDataType u;

    mpy = add = ((double)DOUBLE_MANT_MASK-1.0)/2.0;
    u.dU64 = (uint64_t)(mpy*Generator()+add);

    *a = u.dF64;
}


void DataGenerator::Read(IContainer* p)
{
    if(p == NULL)
    {
        throw Exception::InvalidArgument("[DataGenerator::Read] Input object pointer is NULL");
    }

    if(m_BufferContainerListFillMethod.empty())
    {
        throw Exception::InvalidArgument("[DataGenerator::Read] Buffer container list is empty");
    }

    // check output object supports IBufferContainerList interface
    IBufferContainerList* pBCL = dynamic_cast<IBufferContainerList*>(p);

    for(uint32_t j = 0; j < m_BufferContainerListFillMethod.size(); j++)
    {
        BufferContainerFillMethod fillMethod = m_BufferContainerListFillMethod.at(j);
        // create BufferContainer
        IBufferContainer* buff = pBCL->CreateBufferContainer();

        for(uint32_t i = 0; i < fillMethod.size(); i++)
        {
            BufferDescAndFillMethodPair pair = fillMethod.at(i);
            IMemoryObject* pBuffer = buff->CreateBuffer(pair.first);

            FillBuffer(pair.second, pBuffer);
        }
    }
}


void DataGenerator::FillBuffer(const BufferFillMethodDesc& bfmd, IMemoryObject* buff)
{
    if(buff == NULL)
    {
        throw Exception::InvalidArgument("[DataGenerator::FillBuffer] Input object pointer is NULL");
    }

    BufferDesc buffDesc = GetBufferDescription(buff->GetMemoryObjectDesc());
    if (!(buffDesc.GetElementDescription().GetType() == TVECTOR || 
        buffDesc.GetElementDescription().IsInteger() ||
        buffDesc.GetElementDescription().GetType() == TFLOAT ||
        buffDesc.GetElementDescription().GetType() == TDOUBLE ||
        buffDesc.GetElementDescription().GetType() == THALF))
        throw Exception::InvalidArgument("FillBuffer method supports only buffer of vectors, integers or scalar floating point values.");
    void* dataPtr = buff->GetDataPtr();

    // Compute number of scalar elements to generate.
    int32_t len = buffDesc.NumOfElements();
    // If buffer element is vector, than we need to generate number of buffer elements (vectors) multiplied by the vector width.
    if (buffDesc.GetElementDescription().GetType() == TVECTOR)
    {
        len *= buffDesc.GetElementDescription().GetNumberOfElements();
    }
    // Get the type value of scalar elements.
    TypeVal elemType = (buffDesc.GetElementDescription().GetType() == TVECTOR) ?
        buffDesc.GetElementDescription().GetSubTypeDesc(0).GetType() :
        buffDesc.GetElementDescription().GetType();

    switch(bfmd.fillMethod)
    {
    case FILL_VALUE:
        switch(elemType)
        {
            case TCHAR:
                FillConst<int8_t>((int8_t*)dataPtr,len, bfmd.fillValue.dI8);
                break;
            case TUCHAR:
                FillConst<uint8_t>((uint8_t*)dataPtr,len, bfmd.fillValue.dU8);
                break;
            case TSHORT:
                FillConst<int16_t>((int16_t*)dataPtr,len, bfmd.fillValue.dI16);
                break;
            case TUSHORT:
                FillConst<uint16_t>((uint16_t*)dataPtr,len, bfmd.fillValue.dU16);
                break;
            case TINT:
                FillConst<int32_t>((int32_t*)dataPtr,len, bfmd.fillValue.dI32);
                break;
            case TUINT:
                FillConst<uint32_t>((uint32_t*)dataPtr,len, bfmd.fillValue.dU32);
                break;
            case TLONG:
                FillConst<int64_t>((int64_t*)dataPtr,len, bfmd.fillValue.dI64);
                break;
            case TULONG:
                FillConst<uint64_t>((uint64_t*)dataPtr,len, bfmd.fillValue.dU64);
                break;
            case THALF: // CFloat16 represents half floats, but uint16_t is used for storing.
                FillConst<uint16_t>((uint16_t*)dataPtr,len, bfmd.fillValue.dF16);
                break;
            case TFLOAT:
                FillConst<float>((float*)dataPtr,len, bfmd.fillValue.dF32);
                break;
            case TDOUBLE:
                FillConst<double>((double*)dataPtr,len,bfmd.fillValue.dF64);
                break;
            default:
                throw Exception::InvalidArgument("[DataGenerator::FillBuffer] Unsupported data format");
                break;
        }
        break;
    case FILL_RANDOM:
        switch(elemType)
        {
            case TCHAR:
                FillRandom<int8_t>((int8_t*)dataPtr,len, std::numeric_limits<int8_t>::max(), (0-std::numeric_limits<int8_t>::max()));
                break;
            case TUCHAR:
                FillRandom<uint8_t>((uint8_t*)dataPtr,len, std::numeric_limits<uint8_t>::max(), std::numeric_limits<uint8_t>::min());
                break;
            case TSHORT:
                FillRandom<int16_t>((int16_t*)dataPtr,len, std::numeric_limits<int16_t>::max(), (0-std::numeric_limits<int16_t>::max()));
                break;
            case TUSHORT:
                FillRandom<uint16_t>((uint16_t*)dataPtr,len, std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::min());
                break;
            case TINT:
                FillRandom<int32_t>((int32_t*)dataPtr,len, std::numeric_limits<int32_t>::max(), (0-std::numeric_limits<int32_t>::max()));
                break;
            case TUINT:
                FillRandom<uint32_t>((uint32_t*)dataPtr,len, std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::min());
                break;
            case TLONG:
                FillRandom<int64_t>((int64_t*)dataPtr,len, std::numeric_limits<int64_t>::max(), (0-std::numeric_limits<int64_t>::max()));
                break;
            case TULONG:
                FillRandom<uint64_t>((uint64_t*)dataPtr,len, std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::min());
                break;
            case THALF:
                {
                    CFloat16 max = CFloat16::GetMax();
                    FillRandom((CFloat16*)dataPtr,len, max, CFloat16(0.f-float(max)));
                }
                break;
            case TFLOAT:
                FillRandom<float>((float*)dataPtr,len, std::numeric_limits<float>::max(), (0-std::numeric_limits<float>::max()));
                break;
            case TDOUBLE:
                FillRandom<double>((double*)dataPtr,len, std::numeric_limits<double>::max(), (0-std::numeric_limits<double>::max()));
                break;
            default:
                throw Exception::InvalidArgument("[DataGenerator::FillBuffer] Unsupported data format");
                break;
        }
        break;
    case FILL_RANDOM_FROM_RANGE:

        switch(elemType)
        {
            case TCHAR:
                FillRandom<int8_t>((int8_t*)dataPtr,len, bfmd.highValue.dI8, bfmd.lowValue.dI8);
                break;
            case TUCHAR:
                FillRandom<uint8_t>((uint8_t*)dataPtr,len, bfmd.highValue.dU8, bfmd.lowValue.dU8);
                break;
            case TSHORT:
                FillRandom<int16_t>((int16_t*)dataPtr,len, bfmd.highValue.dI16, bfmd.lowValue.dI16);
                break;
            case TUSHORT:
                FillRandom<uint16_t>((uint16_t*)dataPtr,len, bfmd.highValue.dU16,bfmd.lowValue.dU16);
                break;
            case TINT:
                FillRandom<int32_t>((int32_t*)dataPtr,len, bfmd.highValue.dI32, bfmd.lowValue.dI32);
                break;
            case TUINT:
                FillRandom<uint32_t>((uint32_t*)dataPtr,len, bfmd.highValue.dU32, bfmd.lowValue.dU32);
                break;
            case TLONG:
                FillRandom<int64_t>((int64_t*)dataPtr,len, bfmd.highValue.dI64, bfmd.lowValue.dI64);
                break;
            case TULONG:
                FillRandom<uint64_t>((uint64_t*)dataPtr,len, bfmd.highValue.dU64, bfmd.lowValue.dU64);
                break;
            case THALF:
                {
                    CFloat16 low = CFloat16(bfmd.lowValue.dF16);
                    CFloat16 high = CFloat16(bfmd.highValue.dF16);
                    FillRandom((CFloat16*)dataPtr,len, high, low);
                }
                break;
            case TFLOAT:
                FillRandom<float>((float*)dataPtr,len, bfmd.highValue.dF32, bfmd.lowValue.dF32);
                break;
            case TDOUBLE:
                FillRandom<double>((double*)dataPtr,len, bfmd.highValue.dF64, bfmd.lowValue.dF64);
                break;
            default:
                throw Exception::InvalidArgument("[DataGenerator::FillBuffer] Unsupported data format");
                break;
        }
        break;
    case FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES:
        if (bfmd.specialValuesProb > 1.f || bfmd.specialValuesProb < 0.f)
        {
            throw Exception::InvalidArgument("[DataGenerator::FillBuffer] The special value probability is out of range (0.f;1.f)");
        }
        switch(elemType)
        {
            case THALF:
                FillRandomSpecial((CFloat16*)dataPtr, len, bfmd.specialValuesProb);
            break;
            case TFLOAT:
                FillRandomSpecial<float>((float*)dataPtr, len, bfmd.specialValuesProb);
            break;
            case TDOUBLE:
                FillRandomSpecial<double>((double*)dataPtr, len, bfmd.specialValuesProb);
            break;
            default:
                throw Exception::InvalidArgument("[DataGenerator::FillBuffer] Unsupported data format");
            break;
        }
        break;
    case INVALID_FILL_METHOD:
    default:
        throw Exception::InvalidArgument("[DataGenerator::FillBuffer] Invalid fill method");
        break;
    }
}

void DataGenerator::SetSpecialValuesMethod(BufferContainerFillMethod& bcfm,
                                           DataTypeValWrapper dataType,
                                           VectorWidthWrapper vecWidth,
                                           const uint32_t buffLen,
                                           float specialValuesProb)
 {
     if( dataType.GetValue() == F16)
         SetBufferContainerFillMethod<CFloat16>(bcfm, dataType, vecWidth,
                                         buffLen,
                                         FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES,
                                         CFloat16(0.0f), CFloat16(0.0f),
                                         CFloat16(0.0f), specialValuesProb);
     else if( dataType.GetValue() == F32)
         SetBufferContainerFillMethod<float>(bcfm, dataType, vecWidth,
                                         buffLen,
                                         FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES,
                                         0.0f, 0.0f, 0.0f, specialValuesProb);
     else if( dataType.GetValue() == F64)
         SetBufferContainerFillMethod<double>(bcfm,
                                         dataType, vecWidth, buffLen,
                                         FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES,
                                         0.0, 0.0, 0.0, specialValuesProb);
     else
         throw Exception::InvalidArgument(
         "[DataGenerator::SetSpecialValuesMethod] Unsupported data format");
 }

void DataGenerator::SetRandomMethod(BufferContainerFillMethod& bcfm,
                                    DataTypeValWrapper dataType,
                                    VectorWidthWrapper vecWidth,
                                    const uint32_t buffLen)
{
    switch(dataType.GetValue())
    {
        case I8:
            SetBufferContainerFillMethod<int8_t>(bcfm, dataType, vecWidth,
                                                 buffLen, FILL_RANDOM,
                                                 0, 0, 0, 0.0f);
            break;
        case U8:
            SetBufferContainerFillMethod<uint8_t>(bcfm, dataType, vecWidth,
                                                  buffLen, FILL_RANDOM,
                                                  0, 0, 0, 0.0f);
            break;
        case I16:
            SetBufferContainerFillMethod<int16_t>(bcfm, dataType, vecWidth,
                                                  buffLen, FILL_RANDOM,
                                                  0, 0, 0, 0.0f);
            break;
        case U16:
            SetBufferContainerFillMethod<uint16_t>(bcfm, dataType, vecWidth,
                                                   buffLen, FILL_RANDOM,
                                                   0, 0, 0, 0.0f);
            break;
        case I32:
            SetBufferContainerFillMethod<int32_t>(bcfm, dataType, vecWidth,
                                                  buffLen, FILL_RANDOM,
                                                  0, 0, 0, 0.0f);
            break;
        case U32:
            SetBufferContainerFillMethod<uint32_t>(bcfm, dataType, vecWidth,
                                                   buffLen, FILL_RANDOM,
                                                   0, 0, 0, 0.0f);
            break;
        case I64:
            SetBufferContainerFillMethod<int64_t>(bcfm, dataType, vecWidth,
                                                  buffLen, FILL_RANDOM,
                                                  0, 0, 0, 0.0f);
            break;
        case U64:
            SetBufferContainerFillMethod<uint64_t>(bcfm, dataType, vecWidth,
                                                   buffLen, FILL_RANDOM,
                                                   0, 0, 0, 0.0f);
            break;
        case F16:
            SetBufferContainerFillMethod<CFloat16>(bcfm, dataType, vecWidth,
                                                   buffLen, FILL_RANDOM,
                                                   0.0f, 0.0f, 0.0f, 0.0f);
            break;
        case F32:
            SetBufferContainerFillMethod<float>(bcfm, dataType, vecWidth,
                                                buffLen, FILL_RANDOM,
                                                0, 0, 0, 0.0f);
            break;
        case F64:
            SetBufferContainerFillMethod<double>(bcfm, dataType, vecWidth,
                                                 buffLen, FILL_RANDOM,
                                                 0, 0, 0, 0.0f);
            break;
        default:
            throw Exception::InvalidArgument("[DataGenerator::SetRandomMethod] Unsupported data format");
            break;
    }
}

