// DMTest.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <limits>
#include <list>

#include <gtest/gtest.h>

#include "VectorWidth.h"
#include "DataType.h"
#include "BufferContainerList.h"
#include "XMLDataReader.h"
#include "XMLDataWriter.h"
#include "BinaryDataReader.h"
#include "BinaryDataWriter.h"
#include "FloatOperations.h"

using namespace Validation;
using namespace Validation::Utils;

class DataManagerDataTypes : public ::testing::Test
{
public:
    static const CFloat16 DataBufF16[];
    static const int32_t  DataBufF16Elems;
    static const int32_t  DataBufF16Size;
    static const int32_t  BufF16VectorWidth;
    static const int32_t  BufF16VectorsNum;

    static const float DataBufF32[];
    static const int32_t  DataBufF32Elems;
    static const int32_t  DataBufF32Size;
    static const int32_t  BufF32VectorWidth;
    static const int32_t  BufF32VectorsNum;

    static const double DataBufF64[];
    static const int32_t  DataBufF64Elems;
    static const int32_t  DataBufF64Size;
    static const int32_t  BufF64VectorWidth;
    static const int32_t  BufF64VectorsNum;

    static const int8_t DataBufI8[];
    static const int32_t  DataBufI8Elems;
    static const int32_t  DataBufI8Size;
    static const int32_t  BufI8VectorWidth;
    static const int32_t  BufI8VectorsNum;

    static const uint8_t DataBufU8[];
    static const int32_t  DataBufU8Elems;
    static const int32_t  DataBufU8Size;
    static const int32_t  BufU8VectorWidth;
    static const int32_t  BufU8VectorsNum;

    static const int16_t DataBufI16[];
    static const int32_t  DataBufI16Elems;
    static const int32_t  DataBufI16Size;
    static const int32_t  BufI16VectorWidth;
    static const int32_t  BufI16VectorsNum;

    static const uint16_t DataBufU16[];
    static const int32_t  DataBufU16Elems;
    static const int32_t  DataBufU16Size;
    static const int32_t  BufU16VectorWidth;
    static const int32_t  BufU16VectorsNum;

    static const int32_t DataBufI32[];
    static const int32_t  DataBufI32Elems;
    static const int32_t  DataBufI32Size;
    static const int32_t  BufI32VectorWidth;
    static const int32_t  BufI32VectorsNum;

    static const uint32_t DataBufU32[];
    static const int32_t  DataBufU32Elems;
    static const int32_t  DataBufU32Size;
    static const int32_t  BufU32VectorWidth;
    static const int32_t  BufU32VectorsNum;

    static const int64_t DataBufI64[];
    static const int32_t  DataBufI64Elems;
    static const int32_t  DataBufI64Size;
    static const int32_t  BufI64VectorWidth;
    static const int32_t  BufI64VectorsNum;

    static const uint64_t DataBufU64[];
    static const int32_t  DataBufU64Elems;
    static const int32_t  DataBufU64Size;
    static const int32_t  BufU64VectorWidth;
    static const int32_t  BufU64VectorsNum;

    BufferContainerList inputList;

protected:

    virtual void SetUp() 
    {
        ASSERT_FALSE(DataBufF16Elems % BufF16VectorWidth);
        ASSERT_FALSE(DataBufF32Elems % BufF32VectorWidth);
        ASSERT_FALSE(DataBufF64Elems % BufF64VectorWidth);
        ASSERT_FALSE(DataBufI8Elems  % BufI8VectorWidth);
        ASSERT_FALSE(DataBufU8Elems  % BufU8VectorWidth);
        ASSERT_FALSE(DataBufI16Elems % BufI16VectorWidth);
        ASSERT_FALSE(DataBufU16Elems % BufU16VectorWidth);
        ASSERT_FALSE(DataBufI32Elems % BufI32VectorWidth);
        ASSERT_FALSE(DataBufU32Elems % BufU32VectorWidth);
        ASSERT_FALSE(DataBufI64Elems % BufI64VectorWidth);
        ASSERT_FALSE(DataBufU64Elems % BufU64VectorWidth);

        IBufferContainer* args = inputList.CreateBufferContainer();

        // Buffer 0 F32
        BufferDesc arg1Desc = BufferDesc(BufF32VectorsNum, VectorWidthWrapper::ValueOf((uint32_t)BufF32VectorWidth), F32);
        IMemoryObject *buffF32 = args->CreateBuffer(arg1Desc);
        ::memcpy(buffF32->GetDataPtr(), DataBufF32, DataBufF32Size);

        // Buffer 1 F64
        BufferDesc arg2Desc = BufferDesc(BufF64VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufF64VectorWidth), F64);
        IMemoryObject *buffF64 = args->CreateBuffer(arg2Desc);
        ::memcpy(buffF64->GetDataPtr(), DataBufF64, DataBufF64Size);

        // Buffer 2 I8
        BufferDesc argI8Desc = BufferDesc(BufI8VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufI8VectorWidth), I8);
        IMemoryObject *buffI8 = args->CreateBuffer(argI8Desc);
        ::memcpy(buffI8->GetDataPtr(), DataBufI8, DataBufI8Size);

        // Buffer 3 U8
        BufferDesc argU8Desc = BufferDesc(BufU8VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufU8VectorWidth), U8);
        IMemoryObject *buffU8 = args->CreateBuffer(argU8Desc);
        ::memcpy(buffU8->GetDataPtr(), DataBufU8, DataBufU8Size);

        // Buffer 4 I16
        BufferDesc argI16Desc = BufferDesc(BufI16VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufI16VectorWidth), I16);
        IMemoryObject *buffI16 = args->CreateBuffer(argI16Desc);
        ::memcpy(buffI16->GetDataPtr(), DataBufI16, DataBufI16Size);

        // Buffer 5 U16
        BufferDesc argU16Desc = BufferDesc(BufU16VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufU16VectorWidth), U16);
        IMemoryObject *buffU16 = args->CreateBuffer(argU16Desc);
        ::memcpy(buffU16->GetDataPtr(), DataBufU16, DataBufU16Size);

        // Buffer 6 I32
        BufferDesc argI32Desc = BufferDesc(BufI32VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufI32VectorWidth), I32);
        IMemoryObject *buffI32 = args->CreateBuffer(argI32Desc);
        ::memcpy(buffI32->GetDataPtr(), DataBufI32, DataBufI32Size);

        // Buffer 7 U32
        BufferDesc argU32Desc = BufferDesc(BufU32VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufU32VectorWidth), U32);
        IMemoryObject *buffU32 = args->CreateBuffer(argU32Desc);
        ::memcpy(buffU32->GetDataPtr(), DataBufU32, DataBufU32Size);

        // Buffer 8 I64
        BufferDesc argI64Desc = BufferDesc(BufI64VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufI64VectorWidth), I64);
        IMemoryObject *buffI64 = args->CreateBuffer(argI64Desc);
        ::memcpy(buffI64->GetDataPtr(), DataBufI64, DataBufI64Size);

        // Buffer 9 U64
        BufferDesc argU64Desc = BufferDesc(BufU64VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufU64VectorWidth), U64);
        IMemoryObject *buffU64 = args->CreateBuffer(argU64Desc);
        ::memcpy(buffU64->GetDataPtr(), DataBufU64, DataBufU64Size);

        // Buffer 10 F16
        BufferDesc argF16Desc = BufferDesc(BufF16VectorsNum, VectorWidthWrapper::ValueOf((uint32_t)BufF16VectorWidth), F16);
        IMemoryObject *buffF16 = args->CreateBuffer(argF16Desc);
        ::memcpy(buffF16->GetDataPtr(), DataBufF16, DataBufF16Size);
    }

    void CheckValues(BufferContainerList& inp)
    {
        // buffer 0
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<float> baf32(*pBC->GetMemoryObject(0));

            // check that we load correct values

            EXPECT_TRUE(IsNaN(baf32.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < DataBufF32Elems; ++i )
                EXPECT_EQ ( DataBufF32[i],    baf32.GetElem(i/BufF32VectorWidth, i%BufF32VectorWidth));
        }

        // buffer 1
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<double> baF64(*pBC->GetMemoryObject(1));

            // check that we load correct values

            EXPECT_TRUE(IsNaN(baF64.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < DataBufF64Elems; ++i )
                EXPECT_EQ ( DataBufF64[i],    baF64.GetElem(i/BufF64VectorWidth, i%BufF64VectorWidth));
        }

        // buffer 2 I8
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<int8_t> baI8(*pBC->GetMemoryObject(2));

            // check that we load correct values
            for(int32_t i = 0; i < DataBufI8Elems; ++i )
                EXPECT_EQ ( DataBufI8[i],    baI8.GetElem(i/BufI8VectorWidth, i%BufI8VectorWidth));
        }

        // buffer 3 U8
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<uint8_t> baU8(*pBC->GetMemoryObject(3));

            // check that we load correct values
            for(int32_t i = 0; i < DataBufU8Elems; ++i )
                EXPECT_EQ ( DataBufU8[i],    baU8.GetElem(i/BufU8VectorWidth, i%BufU8VectorWidth));
        }

        // buffer 4 I16
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<int16_t> baI16(*pBC->GetMemoryObject(4));

            // check that we load correct values
            for(int32_t i = 0; i < DataBufI16Elems; ++i )
                EXPECT_EQ ( DataBufI16[i],    baI16.GetElem(i/BufI16VectorWidth, i%BufI16VectorWidth));
        }

        // buffer 5 U16
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<uint16_t> baU16(*pBC->GetMemoryObject(5));

            // check that we load correct values
            for(int32_t i = 0; i < DataBufU16Elems; ++i )
                EXPECT_EQ ( DataBufU16[i],    baU16.GetElem(i/BufU16VectorWidth, i%BufU16VectorWidth));
        }

        // buffer 6 I32
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<int32_t> baI32(*pBC->GetMemoryObject(6));

            // check that we load correct values
            for(int32_t i = 0; i < DataBufI32Elems; ++i )
                EXPECT_EQ ( DataBufI32[i],    baI32.GetElem(i/BufI32VectorWidth, i%BufI32VectorWidth));
        }

        // buffer 7 U32
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<uint32_t> baU32(*pBC->GetMemoryObject(7));

            // check that we load correct values
            for(int32_t i = 0; i < DataBufU32Elems; ++i )
                EXPECT_EQ ( DataBufU32[i],    baU32.GetElem(i/BufU32VectorWidth, i%BufU32VectorWidth));
        }

        // buffer 8 I64
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<int64_t> baI64(*pBC->GetMemoryObject(8));

            // check that we load correct values
            for(int32_t i = 0; i < DataBufI64Elems; ++i )
                EXPECT_EQ ( DataBufI64[i],    baI64.GetElem(i/BufI64VectorWidth, i%BufI64VectorWidth));
        }

        // buffer 9 U64
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<uint64_t> baU64(*pBC->GetMemoryObject(9));

            // check that we load correct values
            for(int32_t i = 0; i < DataBufU64Elems; ++i )
                EXPECT_EQ ( DataBufU64[i],    baU64.GetElem(i/BufU64VectorWidth, i%BufU64VectorWidth));
        }

        // buffer 10 F16
        {
            IBufferContainer * pBC = inp.GetBufferContainer(0);
            BufferAccessor<CFloat16> baF16(*pBC->GetMemoryObject(10));
            EXPECT_TRUE(IsNaN(baF16.GetElem(0,0))); // handle specially NaN
            // check that we load correct values
            for(int32_t i = 1; i < DataBufF16Elems; ++i )
                EXPECT_TRUE( DataBufF16[i] == baF16.GetElem(i/BufF16VectorWidth, i%BufF16VectorWidth) );
        }
    }

};

const CFloat16 DataManagerDataTypes::DataBufF16[] =
{
    CFloat16::GetNaN(),        +0.24f,
    CFloat16::GetPInf(),         CFloat16::GetNInf(),
    CFloat16::GetMax(),              CFloat16::GetMin(),
    324.0f,       23.0f,
    -0.f,                                           +0.0f,
    -0.5f,                                          +0.24f

};
const int32_t  DataManagerDataTypes::DataBufF16Size = sizeof(DataManagerDataTypes::DataBufF16);
const int32_t  DataManagerDataTypes::DataBufF16Elems = sizeof(DataManagerDataTypes::DataBufF16) / sizeof(CFloat16);
const int32_t  DataManagerDataTypes::BufF16VectorWidth = 2;
const int32_t  DataManagerDataTypes::BufF16VectorsNum = DataManagerDataTypes::DataBufF16Elems  / DataManagerDataTypes::BufF16VectorWidth;

const float DataManagerDataTypes::DataBufF32[] =
{
    std::numeric_limits<float>::quiet_NaN(),        +0.24f,
    std::numeric_limits<float>::infinity(),         -std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::max(),              std::numeric_limits<float>::min(),
    std::numeric_limits<float>::denorm_min(),       std::numeric_limits<float>::epsilon(),
    -0.f,                                           +0.0f,
    -0.5f,                                          +0.24f

};
const int32_t  DataManagerDataTypes::DataBufF32Size = sizeof(DataBufF32);
const int32_t  DataManagerDataTypes::DataBufF32Elems = sizeof(DataBufF32) / sizeof(float);
const int32_t  DataManagerDataTypes::BufF32VectorWidth = 2;
const int32_t  DataManagerDataTypes::BufF32VectorsNum = DataManagerDataTypes::DataBufF32Elems  / DataManagerDataTypes::BufF32VectorWidth;

const double DataManagerDataTypes::DataBufF64[] =
{
    std::numeric_limits<double>::quiet_NaN(),        +0.234,
    std::numeric_limits<double>::infinity(),         -std::numeric_limits<double>::infinity(),
    std::numeric_limits<double>::max(),              std::numeric_limits<double>::min(),
    std::numeric_limits<double>::denorm_min(),       std::numeric_limits<double>::epsilon(),
    -0.,                                           +0.0,
    -0.5,                                          +0.24,
    1234.65,                                       -4003423.452
};
const int32_t  DataManagerDataTypes::DataBufF64Size = sizeof(DataManagerDataTypes::DataBufF64);
const int32_t  DataManagerDataTypes::DataBufF64Elems = sizeof(DataManagerDataTypes::DataBufF64) / sizeof(double);
const int32_t  DataManagerDataTypes::BufF64VectorWidth = 2;
const int32_t  DataManagerDataTypes::BufF64VectorsNum = DataManagerDataTypes::DataBufF64Elems  / DataManagerDataTypes::BufF64VectorWidth;

const int8_t DataManagerDataTypes::DataBufI8[] =
{ std::numeric_limits<int8_t>::max(),std::numeric_limits<int8_t>::min(), 0, 2, 1, 4, 5, 6, -5, 88 };
const int32_t  DataManagerDataTypes::DataBufI8Size = sizeof(DataManagerDataTypes::DataBufI8);
const int32_t  DataManagerDataTypes::DataBufI8Elems = sizeof(DataManagerDataTypes::DataBufI8) / sizeof(int8_t);
const int32_t  DataManagerDataTypes::BufI8VectorWidth = 1;
const int32_t  DataManagerDataTypes::BufI8VectorsNum = DataManagerDataTypes::DataBufI8Elems  / DataManagerDataTypes::BufI8VectorWidth;

const uint8_t DataManagerDataTypes::DataBufU8[] =
{ 255, 0, 48, 1, 254, 33, 45,234, std::numeric_limits<uint8_t>::max(),std::numeric_limits<uint8_t>::min(),
   127, 32};
const int32_t  DataManagerDataTypes::DataBufU8Size = sizeof(DataManagerDataTypes::DataBufU8);
const int32_t  DataManagerDataTypes::DataBufU8Elems = sizeof(DataManagerDataTypes::DataBufU8) / sizeof(uint8_t);
const int32_t  DataManagerDataTypes::BufU8VectorWidth = 4;
const int32_t  DataManagerDataTypes::BufU8VectorsNum = DataManagerDataTypes::DataBufU8Elems  / DataManagerDataTypes::BufU8VectorWidth;

const int16_t DataManagerDataTypes::DataBufI16[] =
{ std::numeric_limits<int16_t>::max(),std::numeric_limits<int16_t>::min(), 0, 2, 1, 4, 5, 6, -5, 88,  };
const int32_t  DataManagerDataTypes::DataBufI16Size = sizeof(DataManagerDataTypes::DataBufI16);
const int32_t  DataManagerDataTypes::DataBufI16Elems = sizeof(DataManagerDataTypes::DataBufI16) / sizeof(int16_t);
const int32_t  DataManagerDataTypes::BufI16VectorWidth = 1;
const int32_t  DataManagerDataTypes::BufI16VectorsNum = DataManagerDataTypes::DataBufI16Elems  / DataManagerDataTypes::BufI16VectorWidth;

const uint16_t DataManagerDataTypes::DataBufU16[] =
{ 255, 0, 48, 1, 254, 33, std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::min() };
const int32_t  DataManagerDataTypes::DataBufU16Size = sizeof(DataManagerDataTypes::DataBufU16);
const int32_t  DataManagerDataTypes::DataBufU16Elems = sizeof(DataManagerDataTypes::DataBufU16) / sizeof(uint16_t);
const int32_t  DataManagerDataTypes::BufU16VectorWidth = 8;
const int32_t  DataManagerDataTypes::BufU16VectorsNum = DataManagerDataTypes::DataBufU16Elems  / DataManagerDataTypes::BufU16VectorWidth;

const int32_t DataManagerDataTypes::DataBufI32[] =
{ std::numeric_limits<int32_t>::max(),std::numeric_limits<int32_t>::min(), 0, 2, 1, 4, 5, 6, -5, 88  };
const int32_t  DataManagerDataTypes::DataBufI32Size = sizeof(DataManagerDataTypes::DataBufI32);
const int32_t  DataManagerDataTypes::DataBufI32Elems = sizeof(DataManagerDataTypes::DataBufI32) / sizeof(int32_t);
const int32_t  DataManagerDataTypes::BufI32VectorWidth = 1;
const int32_t  DataManagerDataTypes::BufI32VectorsNum = DataManagerDataTypes::DataBufI32Elems  / DataManagerDataTypes::BufI32VectorWidth;

const uint32_t DataManagerDataTypes::DataBufU32[] =
{ 255, 0, 48, 1, 254, 33, std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::min(), 32, 58000 };
const int32_t  DataManagerDataTypes::DataBufU32Size = sizeof(DataManagerDataTypes::DataBufU32);
const int32_t  DataManagerDataTypes::DataBufU32Elems = sizeof(DataManagerDataTypes::DataBufU32) / sizeof(uint32_t);
const int32_t  DataManagerDataTypes::BufU32VectorWidth = 2;
const int32_t  DataManagerDataTypes::BufU32VectorsNum = DataManagerDataTypes::DataBufU32Elems  / DataManagerDataTypes::BufU32VectorWidth;

const int64_t DataManagerDataTypes::DataBufI64[] =
{ std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min(), 0, 2, 1, 4, 5, 6, -5, 88  };
const int32_t  DataManagerDataTypes::DataBufI64Size = sizeof(DataManagerDataTypes::DataBufI64);
const int32_t  DataManagerDataTypes::DataBufI64Elems = sizeof(DataManagerDataTypes::DataBufI64) / sizeof(int64_t);
const int32_t  DataManagerDataTypes::BufI64VectorWidth = 1;
const int32_t  DataManagerDataTypes::BufI64VectorsNum = DataManagerDataTypes::DataBufI64Elems  / DataManagerDataTypes::BufI64VectorWidth;

const uint64_t DataManagerDataTypes::DataBufU64[] =
{ 255, 0, 48, 1, 254, 33, std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::min(), 32, 58000 };
const int32_t  DataManagerDataTypes::DataBufU64Size = sizeof(DataManagerDataTypes::DataBufU64);
const int32_t  DataManagerDataTypes::DataBufU64Elems = sizeof(DataManagerDataTypes::DataBufU64) / sizeof(uint64_t);
const int32_t  DataManagerDataTypes::BufU64VectorWidth = 2;
const int32_t  DataManagerDataTypes::BufU64VectorsNum = DataManagerDataTypes::DataBufU64Elems  / DataManagerDataTypes::BufU64VectorWidth;

/// This test checks for creation/write/load ALL supported DataTypes in Buffer:
///   integer: U8,I8,U16,I16,U32,I32,U64,I64
///   float: fp16, fp32, fp64.
///   TODO: NEAT support
TEST_F(DataManagerDataTypes, DataTypesXMLReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testDataTypesXMLReadWriteCheck.xml";
    const char OUTPUT_FILE_NAME[] = "testDataTypesXMLReadWriteCheck.xml";

    {   // Write
        XMLBufferContainerListWriter saver(OUTPUT_FILE_NAME);
        saver.Write(&inputList);
    }

    {   // Read
        XMLBufferContainerListReader loader(INPUT_FILE_NAME);
        BufferContainerList inp;
        loader.Read(&inp);
        
        CheckValues(inp);
    }
}

TEST_F(DataManagerDataTypes, DataTypesBinaryReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testDataTypesXMLReadWriteCheck.bin";
    const char OUTPUT_FILE_NAME[] = "testDataTypesXMLReadWriteCheck.bin";


    {   // Write
        BinaryContainerListWriter saver(OUTPUT_FILE_NAME);
        saver.Write(&inputList);
    }

    {   // Read
        BinaryContainerListReader loader(INPUT_FILE_NAME);
        BufferContainerList inp;
        loader.Read(&inp);
        
        CheckValues(inp);
    }
}


TEST(DataManager, DenormCheck) {
    const char INPUT_FILE_NAME[] = "testDenormCheck.xml";
    const char OUTPUT_FILE_NAME[] = "testDenormCheck.xml";

    // to suppress gcc warning: dereferencing type-punned pointer will break strict-aliasing rules
    union _cvtint2float
    {
        uint32_t dU32;
        float    dF32;
    };

    _cvtint2float d1i, d2i, d3i;

    d1i.dU32 = 01; // Smallest denormalized number
    d2i.dU32 = 020000000; // "Middle" denormalized number
    d3i.dU32 = 037777777; // Largest denormalized number
    const float d1 = d1i.dF32;
    const float d2 = d2i.dF32;
    const float d3 = d3i.dF32;
    {
        BufferContainerList inputList;
        IBufferContainer* args = inputList.CreateBufferContainer();
        BufferDesc arg1Desc = BufferDesc(2, V2, F32);
        IMemoryObject *buff = args->CreateBuffer(arg1Desc);
        float *data = (float*)buff->GetDataPtr();
        data[0] = d1; // Smallest denormalized number
        data[1] = d2; // "Middle" denormalized number
        data[2] = d3; // Largest denormalized number
        data[3] = +0.f;

        XMLBufferContainerListWriter saver(OUTPUT_FILE_NAME);
        saver.Write(&inputList);
    }

    {
        XMLBufferContainerListReader loader(INPUT_FILE_NAME);
        BufferContainerList inp;
        loader.Read(&inp);
        IBufferContainer * pBC = inp.GetBufferContainer(0);

        // check that we load correct values
        EXPECT_EQ (d1,(reinterpret_cast<float *>(pBC->GetMemoryObject(0)->GetDataPtr()))[0]);
        EXPECT_EQ (d2,(reinterpret_cast<float*>(pBC->GetMemoryObject(0)->GetDataPtr()))[1]);
        EXPECT_EQ (d3,(reinterpret_cast<float*>(pBC->GetMemoryObject(0)->GetDataPtr()))[2]);
        EXPECT_EQ (+0.f,(reinterpret_cast<float*>(pBC->GetMemoryObject(0)->GetDataPtr()))[3]);
    }
}

/// This test checks for creation/write/load multiple BufferConatiner objects in BufferContainerList object

class DataManagerContainerList : public ::testing::Test
{
public:
    static const int32_t NumOfBufferContainers = 1000;
    static const float DataBufF32[];
    static const int32_t  DataBufF32Elems;
    static const int32_t  DataBufF32Size;
    static const int32_t  BufF32VectorWidth;
    static const int32_t  BufF32VectorsNum;
    BufferContainerList inputList;

protected:

    virtual void SetUp() 
    {
        ASSERT_FALSE(DataBufF32Elems % BufF32VectorWidth);

        for(int32_t cntBC = 0; cntBC < NumOfBufferContainers; ++cntBC)
        {
            IBufferContainer* args = inputList.CreateBufferContainer();

            // Buffer 0 F32
            BufferDesc arg1Desc = BufferDesc(BufF32VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufF32VectorWidth), F32);
            IMemoryObject *buffF32 = args->CreateBuffer(arg1Desc);
            ::memcpy(buffF32->GetDataPtr(), DataBufF32, DataBufF32Size);

        }
    }

    void CheckValues(BufferContainerList& inp)
    {
        for(int32_t cntBC = 0; cntBC < NumOfBufferContainers; ++cntBC)
        {
            IBufferContainer * pBC = inp.GetBufferContainer(cntBC);
            BufferAccessor<float> baf32(*pBC->GetMemoryObject(0));

            // check that we load correct values

            EXPECT_TRUE(IsNaN(baf32.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < DataBufF32Elems; ++i )
                EXPECT_EQ ( DataBufF32[i],    baf32.GetElem(i/BufF32VectorWidth, i%BufF32VectorWidth));
        }
    }

};

const float DataManagerContainerList::DataBufF32[] =
{
    std::numeric_limits<float>::quiet_NaN(),        +0.24f,
    std::numeric_limits<float>::infinity(),         -std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::max(),              std::numeric_limits<float>::min(),
    std::numeric_limits<float>::denorm_min(),       std::numeric_limits<float>::epsilon(),
    -0.f,                                           +0.0f,
    -0.5f,                                          +0.24f

};
const int32_t  DataManagerContainerList::DataBufF32Size = sizeof(DataBufF32);
const int32_t  DataManagerContainerList::DataBufF32Elems = sizeof(DataBufF32) / sizeof(float);
const int32_t  DataManagerContainerList::BufF32VectorWidth = 2;
const int32_t  DataManagerContainerList::BufF32VectorsNum = DataBufF32Elems  / BufF32VectorWidth;


TEST_F(DataManagerContainerList, BufferContainerListXMLReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testBufferContainerListXMLReadWriteCheck.xml";
    const char OUTPUT_FILE_NAME[] = "testBufferContainerListXMLReadWriteCheck.xml";

    ASSERT_FALSE(DataBufF32Elems % BufF32VectorWidth);

    {   // Write
        XMLBufferContainerListWriter saver(OUTPUT_FILE_NAME);
        saver.Write(&inputList);
    }

    {   // Read
        XMLBufferContainerListReader loader(INPUT_FILE_NAME);
        BufferContainerList inp;
        loader.Read(&inp);

        CheckValues(inp);
    }
}

TEST_F(DataManagerContainerList, BufferContainerListBinaryReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testBufferContainerListXMLReadWriteCheck.bin";
    const char OUTPUT_FILE_NAME[] = "testBufferContainerListXMLReadWriteCheck.bin";

    ASSERT_FALSE(DataBufF32Elems % BufF32VectorWidth);

    {   // Write
        BinaryContainerListWriter saver(OUTPUT_FILE_NAME);
        saver.Write(&inputList);
    }

    {   // Read
        BinaryContainerListReader loader(INPUT_FILE_NAME);
        BufferContainerList inp;
        loader.Read(&inp);

        CheckValues(inp);
    }
}


class DataManagerContainer : public ::testing::Test
{
public:
    static float DataBufF32[];
    static int32_t  DataBufF32Elems;
    static int32_t  DataBufF32Size;
protected:

    virtual void SetUp() 
    {
        NumOfBuffers = 10;
        BufF32VectorWidth = 2;

        const int32_t  BufF32VectorsNum = DataBufF32Elems  / BufF32VectorWidth;
        ASSERT_FALSE(DataBufF32Elems % BufF32VectorWidth);

        IBufferContainer* args = inputList.CreateBufferContainer();

        for(int32_t cntB = 0; cntB < NumOfBuffers; ++cntB)
        {
            // Buffer 0 F32
            BufferDesc arg1Desc = BufferDesc(BufF32VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufF32VectorWidth), F32);
            IMemoryObject *buffF32 = args->CreateBuffer(arg1Desc);
            ::memcpy(buffF32->GetDataPtr(), DataBufF32, DataBufF32Size);
        }
    }

    void CheckValues(BufferContainerList& inp)
    {
        IBufferContainer * pBC = inp.GetBufferContainer(0);

        for(int32_t cntB = 0; cntB < NumOfBuffers; ++cntB)
        {

            BufferAccessor<float> baf32(*pBC->GetMemoryObject(cntB));

            // check that we load correct values

            EXPECT_TRUE(IsNaN(baf32.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < DataBufF32Elems; ++i )
                EXPECT_EQ ( DataBufF32[i], baf32.GetElem(i/BufF32VectorWidth, i%BufF32VectorWidth));
        }
    }

    int32_t NumOfBuffers;
    int32_t BufF32VectorWidth;
    BufferContainerList inputList;
};

float DataManagerContainer::DataBufF32[] =
{
    std::numeric_limits<float>::quiet_NaN(),        +0.24f,
    std::numeric_limits<float>::infinity(),         -std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::max(),              std::numeric_limits<float>::min(),
    std::numeric_limits<float>::denorm_min(),       std::numeric_limits<float>::epsilon(),
    -0.f,                                           +0.0f,
    -0.5f,                                          +0.24f
};

int32_t  DataManagerContainer::DataBufF32Size  = sizeof(DataManagerContainer::DataBufF32);
int32_t  DataManagerContainer::DataBufF32Elems = DataBufF32Size / sizeof(float);



/// This test checks for creation/write/load multiple Buffer objects in BufferContainer object
TEST_F(DataManagerContainer, BufferContainerXMLReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testBufferContainerXMLReadWriteCheck.xml";
    const char OUTPUT_FILE_NAME[] = "testBufferContainerXMLReadWriteCheck.xml";

    XMLBufferContainerListWriter saver(OUTPUT_FILE_NAME);
    saver.Write(&inputList);

    XMLBufferContainerListReader loader(INPUT_FILE_NAME);
    BufferContainerList inp;
    loader.Read(&inp);

    CheckValues(inp);
}

/// This test checks for creation/write/load multiple Buffer objects in BufferContainer object
TEST_F(DataManagerContainer, BufferContainerBinaryReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testBufferContainerXMLReadWriteCheck.bin";
    const char OUTPUT_FILE_NAME[] = "testBufferContainerXMLReadWriteCheck.bin";

    BinaryContainerListWriter saver(OUTPUT_FILE_NAME);
    saver.Write(&inputList);

    BinaryContainerListReader loader(INPUT_FILE_NAME);
    BufferContainerList inp;
    loader.Read(&inp);

    CheckValues(inp);
}

class DataManagerBuffer : public ::testing::Test
{
protected:
    virtual void SetUp() 
    {
        const int32_t NumOfElements = 100;

        DataBufI32 = new int32_t[NumOfElements];
        ASSERT_TRUE(DataBufI32);

        for(int32_t cntEl=0; cntEl<NumOfElements; ++cntEl)
        {
             DataBufI32[cntEl] = rand();
        }
        
        DataBufI32Elems = NumOfElements;
        BufI32VectorWidth = 2;
        BufI32VectorsNum = DataBufI32Elems  / BufI32VectorWidth;
        ASSERT_FALSE(DataBufI32Elems % BufI32VectorWidth);

        IBufferContainer* args = inputList.CreateBufferContainer();

        // Buffer I32
        BufferDesc argI32Desc = BufferDesc(BufI32VectorsNum,  VectorWidthWrapper::ValueOf((uint32_t)BufI32VectorWidth), I32);
        IMemoryObject *buffI32 = args->CreateBuffer(argI32Desc);
        ::memcpy(buffI32->GetDataPtr(), DataBufI32, DataBufI32Elems * sizeof(int32_t));
    }

    virtual void TearDown()
    {
        delete [] DataBufI32;
    }

    void CheckValues(BufferContainerList& inp)
    {
        IBufferContainer * pBC = inp.GetBufferContainer(0);
        BufferAccessor<int32_t> baI32(*pBC->GetMemoryObject(0));

        // check that we load correcst values
        for(int32_t i = 0; i < DataBufI32Elems; ++i )
            EXPECT_EQ ( DataBufI32[i],    baI32.GetElem(i/BufI32VectorWidth, i%BufI32VectorWidth));
    }

    BufferContainerList inputList;
    int32_t *DataBufI32;
    int32_t  DataBufI32Elems;
    int32_t  BufI32VectorWidth;
    int32_t  BufI32VectorsNum;
};

/// This test checks for creation/write/load multiple Vector(Elements) in Buffer object
TEST_F(DataManagerBuffer, BufferXMLReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testBufferXMLReadWriteCheck.xml";
    const char OUTPUT_FILE_NAME[] = "testBufferXMLReadWriteCheck.xml";

    XMLBufferContainerListWriter saver(OUTPUT_FILE_NAME);
    saver.Write(&inputList);

    XMLBufferContainerListReader loader(INPUT_FILE_NAME);
    BufferContainerList inp;
    loader.Read(&inp);

    CheckValues(inp);

}

TEST_F(DataManagerBuffer, BufferBinaryReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testBufferXMLReadWriteCheck.bin";
    const char OUTPUT_FILE_NAME[] = "testBufferXMLReadWriteCheck.bin";

    BinaryContainerListWriter saver(OUTPUT_FILE_NAME);
    saver.Write(&inputList);

    BinaryContainerListReader loader(INPUT_FILE_NAME);
    BufferContainerList inp;
    loader.Read(&inp);

    CheckValues(inp);

}



class DataManagerVector : public ::testing::Test
{
protected:
    virtual void SetUp() 
    { 
        const float DataBufF32[32] =
        {
            std::numeric_limits<float>::quiet_NaN(),        +0.24f,
            std::numeric_limits<float>::infinity(),         -std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::max(),              std::numeric_limits<float>::min(),
            std::numeric_limits<float>::denorm_min(),       std::numeric_limits<float>::epsilon(),
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f
        };

       // Write

        IBufferContainer* args = inputList.CreateBufferContainer();
        BufferDesc argDesc;
        IMemoryObject *buffF32;

        // Buffer 0 V1
        argDesc = BufferDesc(32, V1, F32);
        buffF32 = args->CreateBuffer(argDesc);
        ::memcpy(buffF32->GetDataPtr(), DataBufF32, sizeof(DataBufF32));


        // V2
        argDesc = BufferDesc(16, V2, F32);
        buffF32 = args->CreateBuffer(argDesc);
        ::memcpy(buffF32->GetDataPtr(), DataBufF32, sizeof(DataBufF32));

        // V3
        argDesc = BufferDesc(8, V3, F32);
        size_t sz = argDesc.GetElementDescription().GetSizeInBytes();
        buffF32 = args->CreateBuffer(argDesc);
        ::memcpy(buffF32->GetDataPtr(), DataBufF32, 8 * sz);

        // V4
        argDesc = BufferDesc(8, V4, F32);
        buffF32 = args->CreateBuffer(argDesc);
        ::memcpy(buffF32->GetDataPtr(), DataBufF32, sizeof(DataBufF32));

        // V8
        argDesc = BufferDesc(4, V8, F32);
        buffF32 = args->CreateBuffer(argDesc);
        ::memcpy(buffF32->GetDataPtr(), DataBufF32, sizeof(DataBufF32));

        // V16
        argDesc = BufferDesc(2, V16, F32);
        buffF32 = args->CreateBuffer(argDesc);
        ::memcpy(buffF32->GetDataPtr(), DataBufF32, sizeof(DataBufF32));
    }

    void CheckValues( BufferContainerList& inp)
    {
        const float DataBufF32[32] =
        {
            std::numeric_limits<float>::quiet_NaN(),        +0.24f,
            std::numeric_limits<float>::infinity(),         -std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::max(),              std::numeric_limits<float>::min(),
            std::numeric_limits<float>::denorm_min(),       std::numeric_limits<float>::epsilon(),
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f,
            -0.f,                                           +0.0f,
            -0.5f,                                          +0.24f
        };
        const int32_t  DataBufF32Elems = sizeof(DataBufF32) / sizeof(float);

        IBufferContainer * pBC = inp.GetBufferContainer(0);
        // V1
        {
            BufferAccessor<float> baf32(*pBC->GetMemoryObject(0));
            // check that we load correct values
            EXPECT_TRUE(IsNaN(baf32.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < DataBufF32Elems; ++i )
                EXPECT_EQ ( DataBufF32[i],    baf32.GetElem(i/1, i%1));
        }

        // V2
        {
            BufferAccessor<float> baf32(*pBC->GetMemoryObject(1));
            // check that we load correct values
            EXPECT_TRUE(IsNaN(baf32.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < DataBufF32Elems; ++i )
                EXPECT_EQ ( DataBufF32[i],    baf32.GetElem(i/2, i%2));
        }

        // V3
        {
            IMemoryObject * pB = pBC->GetMemoryObject(2);
            BufferAccessor<float> baf32(*pB);
            const int32_t numElems =
                (int32_t) GetBufferDescription(pB->GetMemoryObjectDesc()).NumOfElements() *
                (int32_t) GetBufferDescription(pB->GetMemoryObjectDesc()).SizeOfVector();
            // check that we load correct values
            EXPECT_TRUE(IsNaN(baf32.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < numElems; ++i )
            {
                // hack to 3 element vectors are stored as 4-element
                // ignore 4th element in store implimicitly in float3
                if(i%4 != 3)
                    EXPECT_EQ ( DataBufF32[i],    baf32.GetElem(i/4, i%4));
            }
        }

        // V4
        {
            BufferAccessor<float> baf32(*pBC->GetMemoryObject(3));
            // check that we load correct values
            EXPECT_TRUE(IsNaN(baf32.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < DataBufF32Elems; ++i )
                EXPECT_EQ ( DataBufF32[i],    baf32.GetElem(i/4, i%4));
        }

        // V8
        {
            BufferAccessor<float> baf32(*pBC->GetMemoryObject(4));
            // check that we load correct values
            EXPECT_TRUE(IsNaN(baf32.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < DataBufF32Elems; ++i )
                EXPECT_EQ ( DataBufF32[i],    baf32.GetElem(i/8, i%8));
        }

        // V16
        {
            BufferAccessor<float> baf32(*pBC->GetMemoryObject(5));
            // check that we load correct values
            EXPECT_TRUE(IsNaN(baf32.GetElem(0,0))); // handle specially NaN
            for(int32_t i = 1; i < DataBufF32Elems; ++i )
                EXPECT_EQ ( DataBufF32[i],    baf32.GetElem(i/16, i%16));
        }
    }

    BufferContainerList inputList;
};

/// This test checks for creation/write/load multiple VectorWidth types
/// V1, V2, V3, V4, V8, V16
TEST_F(DataManagerVector, VectorSizeXMLReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testVectorSizeXMLReadWriteCheck.xml";
    const char OUTPUT_FILE_NAME[] = "testVectorSizeXMLReadWriteCheck.xml";


    XMLBufferContainerListWriter saver(OUTPUT_FILE_NAME);
    saver.Write(&inputList);

    XMLBufferContainerListReader loader(INPUT_FILE_NAME);
    BufferContainerList inp;
    loader.Read(&inp);

    CheckValues(inp);
}

TEST_F(DataManagerVector, VectorSizeBinaryReadWriteCheck) {
    const char INPUT_FILE_NAME[] = "testVectorSizeXMLReadWriteCheck.bin";
    const char OUTPUT_FILE_NAME[] = "testVectorSizeXMLReadWriteCheck.bin";

    BinaryContainerListWriter saver(OUTPUT_FILE_NAME);
    saver.Write(&inputList);

    BinaryContainerListReader loader(INPUT_FILE_NAME);
    BufferContainerList inp;
    loader.Read(&inp);

    CheckValues(inp);
}



/// Test NEATValue data access functions
TEST(DataManager, NEATValue)
{
    {
        /// Test access for interval value
        NEATValue desc(0.0f, 16.0f);
        EXPECT_EQ(0.0f, *desc.GetMin<float>());
        EXPECT_EQ(16.0f, *desc.GetMax<float>());
    }
    {
        /// Test access for accurate value
        NEATValue desc(30.0f);
        EXPECT_EQ(30.0f, *desc.GetAcc<float>());
    }
    {
        //// Test access for boolean values
        NEATValue valTrue(true);
        NEATValue valFalse(false);
        NEATValue trueCpy = valTrue;
        NEATValue falseCpy = valFalse;
        EXPECT_TRUE(*valTrue.GetAcc<bool>());
        EXPECT_FALSE(*valFalse.GetAcc<bool>());
        EXPECT_TRUE(*trueCpy.GetAcc<bool>());
        EXPECT_FALSE(*falseCpy.GetAcc<bool>());
    }

}

void CheckFloatValueDesc(float acc, float minInt, float maxInt, const NEATValue& IntervalVD, const NEATValue& AccurateVD)
{
    EXPECT_EQ(acc > maxInt,false);
    EXPECT_EQ(acc < minInt, false);
    EXPECT_EQ(minInt > maxInt, false);
    EXPECT_EQ(*AccurateVD.GetAcc<float>(), acc);
    EXPECT_EQ(*IntervalVD.GetMin<float>(), minInt);
    EXPECT_EQ(*IntervalVD.GetMax<float>(), maxInt);
}

/// Test BufferContainerList storage and helper structures
TEST(DataManager, BufferContainers)
{
    // test for standard types
    {
        int data1[2][2] = {{0,3},{4,5}};
        int data2[3][2] = {{0,3},{4,5},{9,6}};
        BufferContainerList accList;
        {
            IBufferContainer* c1 = accList.CreateBufferContainer();
            EXPECT_EQ((int32_t) c1->GetMemoryObjectCount(), 0);
            BufferDesc desc(2, V2, U8);
            IMemoryObject* buf = c1->CreateBuffer(desc);
            BufferAccessor<uint8_t> acc(*buf);
            for(int i = 0; i<2; i++)
                for(int j = 0; j<2; j++)
                    acc.SetElem(i,j,data1[i][j]);
            desc.SetNumOfElements(3);
            IMemoryObject* buf1 = c1->CreateBuffer(desc);
            BufferAccessor<uint8_t> acc1(*buf1);
            for(int i = 0; i<3; i++)
                for(int j = 0; j<2; j++)
                    acc1.SetElem(i,j,data2[i][j]);
        }
                /// Now extract data
        {
            EXPECT_EQ((int32_t) accList.GetBufferContainerCount(), 1);
            IBufferContainer* bc = accList.GetBufferContainer(0);
            EXPECT_EQ((int32_t) bc->GetMemoryObjectCount(), 2);
            IMemoryObject* buf = bc->GetMemoryObject(0);
            BufferAccessor<uint8_t> acc(*buf);
            for(int i = 0; i<2; i++)
                for(int j = 0; j<2; j++)
                    EXPECT_EQ(acc.GetElem(i,j), data1[i][j]);
            buf = bc->GetMemoryObject(1);
            BufferAccessor<uint8_t> acc1(*buf);
            for(int i = 0; i<3; i++)
                for(int j = 0; j<2; j++)
                    EXPECT_EQ(acc1.GetElem(i,j), data2[i][j]);
        }
    }
    /// Test NEAT containers
    {
        /// Create NEAT data for the first BufferContainer
        float acc[2][2] = {{0,3},{4,5}};
        float min[2][2] = {{-1,0},{1,1}};
        float max[2][2] = {{1,5},{6,6}};

        BufferContainerList list;
        /// Fill data
        {
            IBufferContainer* c1 = list.CreateBufferContainer();
            BufferDesc desc(2, V2, F32, true);
            IMemoryObject* buf = c1->CreateBuffer(desc);
            IMemoryObject* buf1 = c1->CreateBuffer(desc);
            BufferAccessor<NEATValue> bAcc(*buf);
            BufferAccessor<NEATValue> bAcc1(*buf1);
            for(int i = 0; i<2; i++)
                for(int j = 0; j<2; j++)
                {
                    NEATValue vd(min[i][j], max[i][j]);
                    bAcc.SetElem(i,j,vd);
                    NEATValue vd1(acc[i][j]);
                    bAcc1.SetElem(i,j,vd1);
                }
        }
        /// Check data
        {
            EXPECT_EQ((int32_t) list.GetBufferContainerCount(), 1);
            IBufferContainer* c1 = list.GetBufferContainer(0);
            IMemoryObject* buf = c1->GetMemoryObject(0);
            BufferAccessor<NEATValue> bAcc1(*buf);
            buf = c1->GetMemoryObject(1);
            BufferAccessor<NEATValue> bAcc2(*buf);
            for(int i = 0; i<2; i++)
                for(int j = 0; j<2; j++)
                {
                    CheckFloatValueDesc(acc[i][j], min[i][j], max[i][j], bAcc1.GetElem(i,j), bAcc2.GetElem(i,j));
                }
        }
    }
}

// test of VectorWidthWrapper class
TEST(DataManager, VectorWidthWrapperTest) 
{
	VectorWidth vw;
	// iterate over possible vector widths
	for(int32_t cnt = 0; cnt < (int32_t) INVALID_WIDTH; ++cnt)
	{
		vw = (VectorWidth) cnt;
		// try to construct VectorWidthWrapper object for the vector width
		EXPECT_NO_THROW(VectorWidthWrapper vwr(vw));
	}

	// expect failure for not supported vector width
	vw = INVALID_WIDTH;
	EXPECT_THROW(VectorWidthWrapper vwr(vw), Exception::InvalidArgument);
}

// test of VectorWidthWrapper class
TEST(DataManager, DataTypeValWrapperTest)
{
	DataTypeVal dt;
	// iterate over possible vector widths
	for(int32_t cnt = 0; cnt < (int32_t) INVALID_DATA_TYPE; ++cnt)
	{
		dt = (DataTypeVal) cnt;
		// try to construct DataTypeValWrapper object for the data type
		EXPECT_NO_THROW(DataTypeValWrapper dtw(dt));
	}

	dt = INVALID_DATA_TYPE;
	EXPECT_THROW(DataTypeValWrapper dtw(dt), Exception::InvalidArgument);

}

/// test of Read and Write of NEAT buffers


class DataManagerNEAT : public ::testing::Test
{
protected:
    virtual void SetUp() 
    { 
        const int ArraySize = 2;

        /// Create NEAT data for the first BufferContainer
        float acc[ArraySize][ArraySize] = {{0.45f,3.34f}, {-4.30f,5.423f}};
        float min[ArraySize][ArraySize] = {{-1.23f,0.232f},{-31.1f,1.3344f}};
        float max[ArraySize][ArraySize] = {{1.034f,5.3423f},{6.3423f,6.6765f}};

        IBufferContainer* c1 = list.CreateBufferContainer();
        BufferDesc desc(ArraySize, V2, F32, true);
        IMemoryObject* buf = c1->CreateBuffer(desc);
        IMemoryObject* buf1 = c1->CreateBuffer(desc);
        BufferAccessor<NEATValue> bAcc(*buf);
        BufferAccessor<NEATValue> bAcc1(*buf1);
        for(int i = 0; i<ArraySize; i++)
        {
            for(int j = 0; j<ArraySize ; j++)
            {
                NEATValue vd(min[i][j], max[i][j]);
                bAcc.SetElem(i,j,vd);
                NEATValue vd1(acc[i][j]);
                bAcc1.SetElem(i,j,vd1);
            }
        }
        
        {
            BufferDesc descSV(4, V2, F64, true);
            IMemoryObject* bufSV = c1->CreateBuffer(descSV);
            BufferAccessor<NEATValue> ba(*bufSV);
            
            ba.GetElem(0,0).SetStatus(NEATValue::ANY);
            ba.GetElem(0,1).SetStatus(NEATValue::ANY);
            ba.GetElem(1,0).SetStatus(NEATValue::UNKNOWN);
            ba.GetElem(1,1).SetStatus(NEATValue::UNKNOWN);
            ba.GetElem(2,0).SetStatus(NEATValue::UNWRITTEN);
            ba.GetElem(2,1).SetStatus(NEATValue::UNWRITTEN);
            ba.GetElem(3,0).SetAccurateVal<double>(345.0);
            ba.GetElem(3,1).SetAccurateVal<double>(-345.0);
        }
    }

    void CheckValues( BufferContainerList& inp )
    {
        const int ArraySize = 2;

        /// Create NEAT data for the first BufferContainer
        float acc[ArraySize][ArraySize] = {{0.45f,3.34f}, {-4.30f,5.423f}};
        float min[ArraySize][ArraySize] = {{-1.23f,0.232f},{-31.1f,1.3344f}};
        float max[ArraySize][ArraySize] = {{1.034f,5.3423f},{6.3423f,6.6765f}};

        EXPECT_EQ((int32_t) inp.GetBufferContainerCount(), 1);
        IBufferContainer* c1 = inp.GetBufferContainer(0);
        IMemoryObject* buf = c1->GetMemoryObject(0);
        BufferAccessor<NEATValue> bAcc1(*buf);
        buf = c1->GetMemoryObject(1);
        BufferAccessor<NEATValue> bAcc2(*buf);
        for(int i = 0; i<ArraySize; i++)
        {
            for(int j = 0; j<ArraySize; j++)
            {
                CheckFloatValueDesc(acc[i][j], min[i][j], max[i][j], bAcc1.GetElem(i,j), bAcc2.GetElem(i,j));
            }
        }

        {
            IMemoryObject* bufSV = c1->GetMemoryObject(2);
            BufferAccessor<NEATValue> ba(*bufSV);

            EXPECT_EQ(ba.GetElem(0,0).GetStatus(), NEATValue::ANY);
            EXPECT_EQ(ba.GetElem(0,1).GetStatus(), NEATValue::ANY);
            EXPECT_EQ(ba.GetElem(1,0).GetStatus(), NEATValue::UNKNOWN);
            EXPECT_EQ(ba.GetElem(1,1).GetStatus(), NEATValue::UNKNOWN);
            EXPECT_EQ(ba.GetElem(2,0).GetStatus(), NEATValue::UNWRITTEN);
            EXPECT_EQ(ba.GetElem(2,1).GetStatus(), NEATValue::UNWRITTEN);
            EXPECT_EQ(ba.GetElem(3,0).GetStatus(), NEATValue::ACCURATE);
            EXPECT_EQ(*ba.GetElem(3,0).GetAcc<double>(), (345.0));
            EXPECT_EQ(ba.GetElem(3,1).GetStatus(), NEATValue::ACCURATE);
            EXPECT_EQ(*ba.GetElem(3,1).GetAcc<double>(), (-345.0));
        }
    }

    BufferContainerList list;
};

TEST_F(DataManagerNEAT, NEATXMLReadWriteTest)
{
    const char INPUT_FILE_NAME[] = "NEATXMLReadWriteTest.xml";
    const char OUTPUT_FILE_NAME[] = "NEATXMLReadWriteTest.xml";

    /// Save data to file
    XMLBufferContainerListWriter saver(OUTPUT_FILE_NAME);
    saver.Write(&list);

    // Read
    XMLBufferContainerListReader loader(INPUT_FILE_NAME);
    BufferContainerList inp;
    loader.Read(&inp);

    CheckValues(inp);
}

TEST_F(DataManagerNEAT, NEATBinaryReadWriteTest)
{
    const char INPUT_FILE_NAME[] = "NEATXMLReadWriteTest.bin";
    const char OUTPUT_FILE_NAME[] = "NEATXMLReadWriteTest.bin";

    /// Save data to file
    BinaryContainerListWriter saver(OUTPUT_FILE_NAME);
    saver.Write(&list);

    // Read
    BinaryContainerListReader loader(INPUT_FILE_NAME);
    BufferContainerList inp;
    loader.Read(&inp);

    CheckValues(inp);
}
