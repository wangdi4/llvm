// DMTest.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include <time.h>

#include "BufferContainerList.h"
#include "DataGenerator.h"
#include "DGHelper.h"
#include "IMemoryObject.h"
#include "Buffer.h"

#include <iostream>
#include <limits>
#include <list>

#include <gtest/gtest.h>

using namespace Validation;

// x and y should be the same variable: my_isnan(a,a);
// It is done to avoid code like "return x != x" being optimized to
// "return false" by some "smart" compiler.
// Functions similar _isnan() are not supported by some compilers.
template<typename T> bool my_isnan(T x,T y)
 {
   return x != y;
 }

template<typename T> DataGenerator::FillDataType
                     SetFillDataType(DataTypeVal dataType,T value)
{
    DataGenerator::FillDataType result;
    switch(dataType)
    {
        case I8:
            result.dI8 = (int8_t)value;
        break;
        case U8:
            result.dU8 = (uint8_t)value;
            break;
        case I16:
            result.dI16 = (int16_t)value;
            break;
        case U16:
            result.dU16 = (uint16_t)value;
            break;
        case I32:
            result.dI32 = (int32_t)value;
            break;
        case U32:
            result.dU32 = (uint32_t)value;
            break;
        case I64:
            result.dI64 = (int64_t)value;
            break;
        case U64:
            result.dU64 = (uint64_t)value;
            break;
        case F16:
    // internal representation is 16bit value so uint16_t is used for F16 type.
            if(sizeof(value) == sizeof(uint16_t))
            {
                uint16_t * a = (uint16_t*)&value;
                result.dF16 = *a;
            } else {
                std::cerr << "[SetFillDataType] the size of F16 is not valid\n";
            }
            break;
        case F32:
            result.dF32 = (float)value;
            break;
        case F64:
            result.dF64 = (double)value;
            break;
        default:
            std::cerr << "[SetFillDataType] Unsupported data format\n";
            throw;
            break;
    }
    return result;
}


// function to compare results of using FILL_VALUE method for data generation
// Each element of input buffer "buf" should be equal to "value"
void CompareFillValue(DataTypeValWrapper dataType,
                      DataGenerator::FillDataType value, IMemoryObject* buf)
{
    BufferDesc desc = GetBufferDescription(buf->GetMemoryObjectDesc());

    switch(dataType.GetValue())
    {
        case I8:
            {
                int8_t* data = (int8_t*)buf->GetDataPtr();
                for(uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dI8,data[j]);
                }
            }
        break;
        case U8:
            {
                uint8_t* data = (uint8_t*)buf->GetDataPtr();
                for(uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dU8,data[j]);
                }
            }
            break;
        case I16:
            {
                int16_t* data = (int16_t*)buf->GetDataPtr();
                for(uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dI16,data[j]);
                }
            }
            break;
        case U16:
            {
                uint16_t* data = (uint16_t*)buf->GetDataPtr();
                for( uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dU16,data[j]);
                }
            }
            break;
        case I32:
             {
                int32_t* data = (int32_t*)buf->GetDataPtr();
                for( uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dI32,data[j]);
                }
            }
            break;
        case U32:
            {
                uint32_t* data = (uint32_t*)buf->GetDataPtr();
                for( uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dU32,data[j]);
                }
            }
            break;
        case I64:
            {
                int64_t* data = (int64_t*)buf->GetDataPtr();
                for( uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dI64,data[j]);
                }
            }
            break;
        case U64:
            {
                uint64_t* data = (uint64_t*)buf->GetDataPtr();
                for( uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dU64,data[j]);
                }
            }
            break;
        case F16:
// internal representation is 16bit value so uint16_t is used for F16 type.
            {
                uint16_t* data = (uint16_t*)buf->GetDataPtr();
                for( uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dF16,data[j]);
                }
            }
            break;
        case F32:
            {
                float* data = (float*)buf->GetDataPtr();
                for( uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dF32,data[j]);
                }
            }
            break;
        case F64:
            {
                double* data = (double*)buf->GetDataPtr();
                for( uint32_t j=0;j<desc.NumOfElements() * desc.SizeOfVector();j++)
                {
                    EXPECT_EQ(value.dF64,data[j]);
                }
            }
            break;
        default:
            std::cerr << "[CompareFillValue] Unsupported data format\n";
            throw;
            break;
    }
}


TEST(DataGenerator, FillValueTest) {

typedef std::pair<DataTypeValWrapper, DataGenerator::FillDataType> FillValueTestPair;
typedef std::vector<FillValueTestPair> FillValueTestVector;

    srand ( time(NULL) );
    const uint32_t NUM_TESTS  = 50;

    for(uint32_t buffLen=1; buffLen <= NUM_TESTS; buffLen++)
    {
        for(int32_t vecSize=int32_t(V1); vecSize<int32_t(INVALID_WIDTH); vecSize++)
        {
            FillValueTestPair testPair;
            FillValueTestVector testVector;

            VectorWidthWrapper vecWidth;
            vecWidth.SetValue(VectorWidth(vecSize));

            DataGenerator::BufferContainerFillMethod bcfm;
            DataGenerator::BufferContainerListFillMethod bclfm;
            BufferContainerList list;
            int32_t randVal;
 // I8
            randVal = rand();
            testPair.first.SetValue(I8);
            DataGenerator::SetValueMethod<int8_t>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<int8_t>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);
 // I16
            randVal = rand();
            testPair.first.SetValue(I16);
            DataGenerator::SetValueMethod<int16_t>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<int16_t>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);
 // I32
            randVal = rand();
            testPair.first.SetValue(I32);
            DataGenerator::SetValueMethod<int32_t>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<int32_t>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);
 // I64
            randVal = rand();
            testPair.first.SetValue(I64);
            DataGenerator::SetValueMethod<int64_t>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<int64_t>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);
 // U8
            randVal = rand();
            testPair.first.SetValue(U8);
            DataGenerator::SetValueMethod<uint8_t>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<uint8_t>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);
 // U16
            randVal = rand();
            testPair.first.SetValue(U16);
            DataGenerator::SetValueMethod<uint16_t>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<uint16_t>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);
 // U32
            randVal = rand();
            testPair.first.SetValue(U32);
            DataGenerator::SetValueMethod<uint32_t>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<uint32_t>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);
 // U64
            randVal = rand();
            testPair.first.SetValue(U64);
            DataGenerator::SetValueMethod<uint64_t>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<uint64_t>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);
 // F16
            // a bit different from other cases
            {
                randVal = rand();
                CFloat16 maxF16_float16 = CFloat16::GetMax();
                int32_t maxF16 = int32_t(float(maxF16_float16));

                CFloat16 valueFloat16 = CFloat16(float(randVal%maxF16));
                testPair.first.SetValue(F16);
                DataGenerator::SetValueMethod<CFloat16>(bcfm,testPair.first,vecWidth,buffLen,valueFloat16);
                //DataGenerator::SetBufferContainerFillMethod<CFloat16>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,valueFloat16,0.f,0.f,0);
                testPair.second = SetFillDataType(testPair.first.GetValue(),valueFloat16);
                testVector.push_back(testPair);
            }
 // F32
            randVal = rand();
            testPair.first.SetValue(F32);
            DataGenerator::SetValueMethod<float>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<float>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);
 // F64
            randVal = rand();
            testPair.first.SetValue(F64);
            DataGenerator::SetValueMethod<double>(bcfm,testPair.first,vecWidth,buffLen,randVal);
            //DataGenerator::SetBufferContainerFillMethod<double>(bcfm,testPair.first,vecWidth,buffLen,DataGenerator::FILL_VALUE,randVal,0,0,0);
            testPair.second = SetFillDataType(testPair.first.GetValue(),randVal);
            testVector.push_back(testPair);

            DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);

            DataGenerator dg(bclfm,1234);

            dg.Read(&list);

            for( uint32_t i=0;i<testVector.size();i++)
            {
                FillValueTestPair testPair_n = testVector.at(i);
                CompareFillValue(testPair_n.first,testPair_n.second,list.GetBufferContainer(0)->GetMemoryObject(i));
            }
        }
    }
}

// function to compare results of using FILL_RANDOM method for data generation
// Each element of buffer "bufRef" should be equal to the appropriate element of "bufSame"
// and buffer "bufDiff" should not be the same with "bufRef"
void CompareSeed(DataTypeValWrapper dataType, IMemoryObject* bufRef, IMemoryObject* bufSame, IMemoryObject* bufDiff, uint64_t seed)
{
    BufferDesc descRef = GetBufferDescription(bufRef->GetMemoryObjectDesc());
    BufferDesc descSame = GetBufferDescription(bufSame->GetMemoryObjectDesc());
    BufferDesc descDiff = GetBufferDescription(bufDiff->GetMemoryObjectDesc());
    int32_t diffCnt = 0;

    if((descRef.NumOfElements() * descRef.SizeOfVector() != descSame.NumOfElements() * descSame.SizeOfVector()) ||
       (descRef.NumOfElements() * descRef.SizeOfVector() != descDiff.NumOfElements() * descDiff.SizeOfVector()))
    {
        std::cerr << "[CompareSeed] wrong data buffer size\n";
        throw;
    }

    switch(dataType.GetValue())
    {
        case I8:
            {
                int8_t* dataRef = (int8_t*)bufRef->GetDataPtr();
                int8_t* dataSame = (int8_t*)bufSame->GetDataPtr();
                int8_t* dataDiff = (int8_t*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j]);
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "<<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
        break;
        case U8:
            {
                uint8_t* dataRef = (uint8_t*)bufRef->GetDataPtr();
                uint8_t* dataSame = (uint8_t*)bufSame->GetDataPtr();
                uint8_t* dataDiff = (uint8_t*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j]);
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "<<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
            break;
        case I16:
            {
                int16_t* dataRef = (int16_t*)bufRef->GetDataPtr();
                int16_t* dataSame = (int16_t*)bufSame->GetDataPtr();
                int16_t* dataDiff = (int16_t*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j]);
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "<<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
             }
            break;
        case U16:
            {
                uint16_t* dataRef = (uint16_t*)bufRef->GetDataPtr();
                uint16_t* dataSame = (uint16_t*)bufSame->GetDataPtr();
                uint16_t* dataDiff = (uint16_t*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j]);
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "<<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
            break;
        case I32:
            {
                int32_t* dataRef = (int32_t*)bufRef->GetDataPtr();
                int32_t* dataSame = (int32_t*)bufSame->GetDataPtr();
                int32_t* dataDiff = (int32_t*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j]);
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "
                    <<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
            break;
        case U32:
            {
                uint32_t* dataRef = (uint32_t*)bufRef->GetDataPtr();
                uint32_t* dataSame = (uint32_t*)bufSame->GetDataPtr();
                uint32_t* dataDiff = (uint32_t*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j]);
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                 }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "
                    <<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
            break;
        case I64:
            {
                int64_t* dataRef = (int64_t*)bufRef->GetDataPtr();
                int64_t* dataSame = (int64_t*)bufSame->GetDataPtr();
                int64_t* dataDiff = (int64_t*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j])<<"[CompareSeed] seed = "<<seed<<"\n";
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "
                    <<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
            break;
        case U64:
            {
                uint64_t* dataRef = (uint64_t*)bufRef->GetDataPtr();
                uint64_t* dataSame = (uint64_t*)bufSame->GetDataPtr();
                uint64_t* dataDiff = (uint64_t*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j])<<"[CompareSeed] seed = "<<seed<<"\n";
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "
                    <<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
            break;
        case F16:
            {
                CFloat16* dataRef = (CFloat16*)bufRef->GetDataPtr();
                CFloat16* dataSame = (CFloat16*)bufSame->GetDataPtr();
                CFloat16* dataDiff = (CFloat16*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    float dataRefFloat = float(dataRef[j]);
                    float dataSameFloat = float(dataSame[j]);
                    float dataDiffFloat = float(dataDiff[j]);
                    EXPECT_EQ(dataRefFloat,dataSameFloat)<<"[CompareSeed] seed = "<<seed<<"\n";
                    if(dataRefFloat != dataDiffFloat)
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "
                    <<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
            break;
        case F32:
            {
                float* dataRef = (float*)bufRef->GetDataPtr();
                float* dataSame = (float*)bufSame->GetDataPtr();
                float* dataDiff = (float*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j])<<"[CompareSeed] seed = "<<seed<<"\n";
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "
                    <<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
            break;
        case F64:
            {
                double* dataRef = (double*)bufRef->GetDataPtr();
                double* dataSame = (double*)bufSame->GetDataPtr();
                double* dataDiff = (double*)bufDiff->GetDataPtr();
                diffCnt = 0;
                for( uint32_t j=0; j<descRef.NumOfElements() * descRef.SizeOfVector(); j++)
                {
                    EXPECT_EQ(dataRef[j],dataSame[j])<<"[CompareSeed] seed = "<<seed<<"\n";
                    if(dataRef[j] != dataDiff[j])
                    {
                        diffCnt++;
                    }
                }
                if(diffCnt == 0)
                    llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "
                    <<(descRef.NumOfElements() * descRef.SizeOfVector())<<" elements\n";
            }
            break;
        default:
            std::cerr << "[CompareSeed] Unsupported data format\n";
            throw;
            break;
    }
}

TEST(DataGenerator, SeedTest) {

    uint64_t seed = time(NULL);
    const uint32_t NUM_TESTS  = 50;
    for(uint32_t buffLen=1; buffLen<=NUM_TESTS; buffLen++)
    {
        for(int32_t vecSize = (int32_t)V1; vecSize<(int32_t)INVALID_WIDTH; vecSize++)
        {
            DataGenerator::BufferContainerFillMethod bcfm;
            DataGenerator::BufferContainerListFillMethod bclfm;
            BufferContainerList listRef, listSame, listDiff;

            VectorWidthWrapper vecWidth;
            vecWidth.SetValue(VectorWidth(vecSize));

            DataTypeValWrapper dataType;
            std::vector<DataTypeValWrapper> testVector;

            dataType.SetValue(U8);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<uint8_t>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            dataType.SetValue(U16);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<uint16_t>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            dataType.SetValue(U32);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<uint32_t>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            dataType.SetValue(U64);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<uint64_t>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            dataType.SetValue(I8);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<int8_t>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            dataType.SetValue(I16);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<int16_t>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            dataType.SetValue(I32);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<int32_t>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            dataType.SetValue(I64);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<int64_t>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            dataType.SetValue(F16);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<CFloat16>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0.f,0.f,0.f,0);
            testVector.push_back(dataType);

            dataType.SetValue(F32);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<float>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            dataType.SetValue(F64);
            DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,buffLen);
            //DataGenerator::SetBufferContainerFillMethod<double>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM,0,0,0,0);
            testVector.push_back(dataType);

            DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);

            DataGenerator dgRef(bclfm, seed);
            dgRef.Read(&listRef);

            DataGenerator dgDiff(bclfm, seed+1234);
            dgDiff.Read(&listDiff);

            DataGenerator dgSame(bclfm, seed);
            dgSame.Read(&listSame);

            for( uint32_t i=0;i<testVector.size();i++)
            {
                CompareSeed(testVector.at(i),
                            listRef.GetBufferContainer(0)->GetMemoryObject(i),
                            listSame.GetBufferContainer(0)->GetMemoryObject(i),
                            listDiff.GetBufferContainer(0)->GetMemoryObject(i),seed);
            }

        }
    }
}


// function to compare results of using FILL_RANDOM_FROM_RANGE method for data
// generation.
// each element of fuffer "buf" should match the range (low,high)
void CompareRandomRange(DataTypeValWrapper dataType,
                        DataGenerator::FillDataType low,
                        DataGenerator::FillDataType high,
                        IMemoryObject* buf, uint64_t seed)
{
    BufferDesc desc = GetBufferDescription(buf->GetMemoryObjectDesc());

    switch(dataType.GetValue())
    {
        case I8:
            {
                int8_t* data = (int8_t*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dI8,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dI8,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
        break;
        case U8:
            {
                uint8_t* data = (uint8_t*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dU8,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dU8,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        case I16:
            {
                int16_t* data = (int16_t*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dI16,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dI16,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        case U16:
            {
                uint16_t* data = (uint16_t*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dU16,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dU16,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        case I32:
             {
                int32_t* data = (int32_t*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dI32,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dI32,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        case U32:
            {
                uint32_t* data = (uint32_t*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dU32,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dU32,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        case I64:
            {
                int64_t* data = (int64_t*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dI64,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dI64,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        case U64:
            {
                uint64_t* data = (uint64_t*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dU64,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dU64,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        case F16:
            {
                uint16_t* data = (uint16_t*)buf->GetDataPtr();
                CFloat16 high_value = CFloat16(high.dF16);
                CFloat16 low_value = CFloat16(low.dF16);
                float highFloat = float(high_value);
                float lowFloat = float(low_value);
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    CFloat16 data_j = CFloat16(data[j]);
                    float dataFloat = float(data_j);
                    EXPECT_GE(highFloat,dataFloat)<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(lowFloat,dataFloat)<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        case F32:
            {
                float* data = (float*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dF32,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dF32,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        case F64:
            {
                double* data = (double*)buf->GetDataPtr();
                for( uint32_t j=0; j<desc.NumOfElements() * desc.SizeOfVector(); j++)
                {
                    EXPECT_GE(high.dF64,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                    EXPECT_LE(low.dF64,data[j])<<"[RandomRangeTest] seed = "<<seed<<"\n";
                }
            }
            break;
        default:
            std::cerr << "[CompareRandomRange] Unsupported data format\n";
            throw;
            break;
    }
}


struct RandomRangeTestStruct
{
    DataTypeValWrapper dataType;
    DataGenerator::FillDataType low;
    DataGenerator::FillDataType high;
};

TEST(DataGenerator, RandomRangeTest)
{
    typedef std::vector<RandomRangeTestStruct> RandomRangeTestVector;
    const uint32_t NUM_TESTS  = 50;

    srand ( time(NULL) );

    for(uint32_t buffLen=1; buffLen<=NUM_TESTS; buffLen++)
    {
        for(int32_t vecSize = (int32_t)V1; vecSize<(int32_t)INVALID_WIDTH; vecSize++)
        {
            DataGenerator::BufferContainerFillMethod bcfm;
            DataGenerator::BufferContainerListFillMethod bclfm;
            BufferContainerList list;

            VectorWidthWrapper vecWidth;
            vecWidth.SetValue(VectorWidth(vecSize));

            RandomRangeTestStruct testData;
            RandomRangeTestVector testVector;

// unsigned integer data types
// U8
            {
                uint8_t highValue;
                uint8_t lowValue;
                do {
                    highValue = (uint8_t)(rand() % std::numeric_limits<uint8_t>::max());
                    lowValue = (uint8_t)(rand() % std::numeric_limits<uint8_t>::max());
                } while (highValue <= lowValue); // in order to avoid (lowValue == highValue)
                testData.low.dU8 = lowValue;
                testData.high.dU8 = highValue;
                testData.dataType.SetValue(U8);

                DataGenerator::SetRandomFromRangeMethod<uint8_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<uint8_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// U16
            {
                uint16_t highValue;
                uint16_t lowValue;
                do {
                    highValue = (uint16_t)(rand() % std::numeric_limits<uint16_t>::max());
                    lowValue = (uint16_t)(rand() % std::numeric_limits<uint16_t>::max());
                } while (highValue <= lowValue);
                testData.low.dU16 = lowValue;
                testData.high.dU16 = highValue;
                testData.dataType.SetValue(U16);

                DataGenerator::SetRandomFromRangeMethod<uint16_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<uint16_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// U32
            {
                uint32_t highValue;
                uint32_t lowValue;
                do {
                    highValue = (uint32_t)(rand() % std::numeric_limits<uint32_t>::max());
                    lowValue = (uint32_t)(rand() % std::numeric_limits<uint32_t>::max());
                } while (highValue <= lowValue);
                testData.low.dU32 = lowValue;
                testData.high.dU32 = highValue;
                testData.dataType.SetValue(U32);

                DataGenerator::SetRandomFromRangeMethod<uint32_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<uint32_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// U64
            {
                uint64_t highValue;
                uint64_t lowValue;
                do {
                    highValue = std::numeric_limits<uint64_t>::max() -
                        (uint64_t)(rand() % std::numeric_limits<uint32_t>::max());
                    //in order to make highValue higher than top of uint32_t max value
                    lowValue = highValue - (uint64_t)rand();
                } while (highValue <= lowValue);
                testData.low.dU64 = lowValue;
                testData.high.dU64 = highValue;
                testData.dataType.SetValue(U64);

                DataGenerator::SetRandomFromRangeMethod<uint64_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<uint64_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// signed integer data types
        // highValue is a positive number
// I8
            {
                int8_t highValue;
                int8_t lowValue;
                do {
                    highValue = (int8_t)(rand() % std::numeric_limits<int8_t>::max() + 1);
                    lowValue = (int8_t)(highValue-
                               (rand() % (std::numeric_limits<int8_t>::max()+1)));
                } while (highValue <= lowValue);
                testData.low.dI8 = lowValue;
                testData.high.dI8 = highValue;
                testData.dataType.SetValue(I8);

                DataGenerator::SetRandomFromRangeMethod<int8_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<int8_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// I16
            {
                int16_t highValue;
                int16_t lowValue;
                do {
                    highValue = (int16_t)(rand() % std::numeric_limits<int16_t>::max() + 1);
                    lowValue = (int16_t)(highValue - (rand() % (std::numeric_limits<int16_t>::max()+1)));
                } while (highValue <= lowValue);
                testData.low.dI16 = lowValue;
                testData.high.dI16 = highValue;
                testData.dataType.SetValue(I16);

                DataGenerator::SetRandomFromRangeMethod<int16_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<int16_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// I32
            {
                int32_t highValue;
                int32_t lowValue;
                do {
                    highValue = (int32_t)(rand() % std::numeric_limits<int32_t>::max() + 1);
                    lowValue = (int32_t)(highValue - (rand() % (std::numeric_limits<int32_t>::max()+1)));
                } while (highValue <= lowValue);
                testData.low.dI32 = lowValue;
                testData.high.dI32 = highValue;
                testData.dataType.SetValue(I32);

                DataGenerator::SetRandomFromRangeMethod<int32_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<int32_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// I64
            {
                int64_t highValue;
                int64_t lowValue;
                do {
                    highValue = std::numeric_limits<int64_t>::max() - (int64_t)rand();
                    lowValue = highValue-(int64_t)rand();
                } while (highValue <= lowValue);
                testData.low.dI64 = lowValue;
                testData.high.dI64 = highValue;
                testData.dataType.SetValue(I64);

                DataGenerator::SetRandomFromRangeMethod<int64_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<int64_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// highValue is negative number
// I8
            {
                int8_t highValue;
                int8_t lowValue;
                do {
                    highValue = (int8_t)(-(rand() % std::numeric_limits<int8_t>::max() + 1));
                    lowValue = (int8_t)(highValue-(rand() % (std::numeric_limits<int8_t>::max()+1)));
                } while (highValue <= lowValue);
                testData.low.dI8 = lowValue;
                testData.high.dI8 = highValue;
                testData.dataType.SetValue(I8);

                DataGenerator::SetRandomFromRangeMethod<int8_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<int8_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// I16
            {
                int16_t highValue;
                int16_t lowValue;
                do {
                    highValue = (int16_t)(-(rand() % std::numeric_limits<int16_t>::max() + 1));
                    lowValue = (int16_t)(highValue-(rand() % (std::numeric_limits<int16_t>::max()+1)));
                } while (highValue <= lowValue);
                testData.low.dI16 = lowValue;
                testData.high.dI16 = highValue;
                testData.dataType.SetValue(I16);

                DataGenerator::SetRandomFromRangeMethod<int16_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<int16_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// I32
            {
                int32_t highValue;
                int32_t lowValue;
                do {
                    highValue = (int32_t)(-(rand() % std::numeric_limits<int32_t>::max() + 1));
                    lowValue = (int32_t)(highValue-(rand() % (std::numeric_limits<int32_t>::max()+1)));
                } while (highValue <= lowValue);
                testData.low.dI32 = lowValue;
                testData.high.dI32 = highValue;
                testData.dataType.SetValue(I32);

                DataGenerator::SetRandomFromRangeMethod<int32_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<int32_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// I64
            {
                int64_t highValue;
                int64_t lowValue;
                do {
                    lowValue = (0 - std::numeric_limits<int64_t>::max()) + (int64_t)rand();
                    highValue = lowValue+(int64_t)rand();
                } while (highValue <= lowValue);
                testData.low.dI64 = lowValue;
                testData.high.dI64 = highValue;
                testData.dataType.SetValue(I64);

                DataGenerator::SetRandomFromRangeMethod<int64_t>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<int64_t>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// float point data types
            // highValue is positive number
// F16
            {
                float highValueFloat;
                float lowValueFloat;
                CFloat16 highValue;
                CFloat16 lowValue;
                CFloat16 max16 = CFloat16::GetMax();
                uint16_t a = *(uint16_t*)&max16;
                int32_t b = (int32_t)a;

                do {
                    do {
                        highValueFloat = float(rand()%b);
                        lowValueFloat = highValueFloat-float(rand()%b);
                    } while (highValueFloat <= lowValueFloat);
                    highValue = CFloat16(highValueFloat);
                    lowValue = CFloat16(lowValueFloat);
                } while (float(highValue) <= float(lowValue));

                // the internal representation of CFloat16 is uint16_t
                testData.low.dF16 = *(uint16_t*)&lowValue;
                testData.high.dF16 = *(uint16_t*)&highValue;
                testData.dataType.SetValue(F16);

                DataGenerator::SetRandomFromRangeMethod<CFloat16>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<CFloat16>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0.f,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// F32
            {
                float highValue;
                float lowValue;
                do {
                    highValue = (float)(rand());
                    lowValue = highValue-(float)rand();
                } while (highValue <= lowValue);
                testData.low.dF32 = lowValue;
                testData.high.dF32 = highValue;
                testData.dataType.SetValue(F32);

                DataGenerator::SetRandomFromRangeMethod<float>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<float>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// F64
            {
                double highValue;
                double lowValue;
                do {
                    highValue = (double)(rand());
                    lowValue = highValue-(double)rand();
                } while (highValue <= lowValue);
                testData.low.dF64 = lowValue;
                testData.high.dF64 = highValue;
                testData.dataType.SetValue(F64);

                DataGenerator::SetRandomFromRangeMethod<double>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<double>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// F16
           // highValue is negative number
            {
                float highValueFloat;
                float lowValueFloat;
                CFloat16 highValue;
                CFloat16 lowValue;
                CFloat16 max16 = CFloat16::GetMax();
                uint16_t a = *(uint16_t*)&max16;
                int32_t b = (int32_t)a;

                do {
                    do {
                        highValueFloat = float(-rand()%b);
                        lowValueFloat = highValueFloat-float(rand()%b);
                    } while (highValueFloat <= lowValueFloat);
                    highValue = CFloat16(highValueFloat);
                    lowValue = CFloat16(lowValueFloat);
                } while (float(highValue) <= float(lowValue));

                testData.low.dF16 = *(uint16_t*)&lowValue;
                testData.high.dF16 = *(uint16_t*)&highValue;
                testData.dataType.SetValue(F16);

                DataGenerator::SetRandomFromRangeMethod<CFloat16>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<CFloat16>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0.f,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// F32
            {
                float highValue;
                float lowValue;
                do {
                    highValue = (float)(-rand());
                    lowValue = highValue-(float)rand();
                 } while (highValue <= lowValue);
                testData.low.dF32 = lowValue;
                testData.high.dF32 = highValue;
                testData.dataType.SetValue(F32);

                DataGenerator::SetRandomFromRangeMethod<float>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<float>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }
// F64
            {
                double highValue;
                double lowValue;
                do {
                    highValue = (double)(-rand());
                    lowValue = highValue-(double)rand();
                } while (highValue <= lowValue);
                testData.low.dF64 = lowValue;
                testData.high.dF64 = highValue;
                testData.dataType.SetValue(F64);

                DataGenerator::SetRandomFromRangeMethod<double>(bcfm,
                               testData.dataType,vecWidth,buffLen,
                               lowValue,highValue);
                //DataGenerator::SetBufferContainerFillMethod<double>(bcfm,testData.dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FROM_RANGE,0,lowValue,highValue,0);
                testVector.push_back(testData);
            }

            DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);

            DataGenerator dg(bclfm);
            uint64_t seed = dg.GetSeed();

            dg.Read(&list);

            for( uint32_t i=0;i<testVector.size();i++)
            {
                RandomRangeTestStruct testData_n = testVector.at(i);
                CompareRandomRange(testData_n.dataType,
                                   testData_n.low,testData_n.high,
                                list.GetBufferContainer(0)->GetMemoryObject(i),seed);
            }
        }
    }
}

bool IsDenormValue( float a )
{
    static const uint32_t FLOAT_EXP_MASK = 0x7F800000;
    static const uint32_t FLOAT_MANT_MASK = 0x007FFFFF;
    DataGenerator::FillDataType u;

    u.dF32 = a;
    return ((u.dU32 & FLOAT_EXP_MASK) == 0) && ((u.dU32 & FLOAT_MANT_MASK) != 0);
}
bool IsDenormValue( double a )
{
    static const uint64_t DOUBLE_EXP_MASK = 0x7FF0000000000000;
    static const uint64_t DOUBLE_MANT_MASK = 0x000FFFFFFFFFFFFF;
    DataGenerator::FillDataType l;

    l.dF64 = a;
    return ((l.dU64 & DOUBLE_EXP_MASK) == 0) && ((l.dU64 & DOUBLE_MANT_MASK) != 0);
}
//DataGenerator::FillDataType

void CompareSpecialValue_CFloat16(float specialValuesProb, IMemoryObject* buf, uint64_t seed)
{
    BufferDesc desc = GetBufferDescription(buf->GetMemoryObjectDesc());
    CFloat16* data = (CFloat16*)buf->GetDataPtr();

    int32_t cntNaN = 0;
    int32_t cntPosZero = 0;
    int32_t cntNegZero = 0;
    int32_t cntPosInf = 0;
    int32_t cntNegInf = 0;
    int32_t cntDenorm = 0;

    int32_t dataNumLen = (int32_t)(desc.NumOfElements() * desc.SizeOfVector());

    for( int32_t i=0; i<dataNumLen; i++)
    {
        if(data[i].IsPInf())
            cntPosInf++;
        if(data[i].IsNInf())
            cntNegInf++;
        if(data[i] == CFloat16(0.f))
            cntPosZero++;
        if(data[i] == CFloat16(-0.f))
            cntNegZero++;
        if(data[i].IsNaN())
            cntNaN++;
        if(data[i].IsDenorm())
            cntDenorm++;
    }

    int32_t cntSum = cntNaN+cntPosZero+cntNegZero+cntPosInf+cntNegInf+cntDenorm;
    int32_t numExpected = (int32_t)((float)dataNumLen*specialValuesProb);
#if 0
    std::cout<<"NaN "<<cntNaN<<" posZero "<<cntPosZero<<" negZero "<<cntNegZero<<" posINF "<<cntPosInf<<" negINF "<<cntNegInf<<" cntDenorm = "<<cntDenorm<<std::endl;
    std::cout<<" numExpected = "<<numExpected<<" cntSum = "<<cntSum;
    if(numExpected > cntSum) std::cout<<" cntSum is "<<((((float)(numExpected-cntSum))/((float)numExpected))*100)<<" % less";
    std::cout<<std::endl;
#endif

    if (specialValuesProb == 0.0f)
    {
        EXPECT_EQ(0,cntSum)<<"[CompareSpecialValue] seed = "<<seed<<"\n";
    } else if (specialValuesProb == 1.0f)
    {
        EXPECT_EQ(dataNumLen,cntSum)<<"[CompareSpecialValue] seed = "<<seed<<"\n";
    } else if(numExpected >= dataNumLen/4)
    {
        int32_t numCorrected = (int32_t)((float)numExpected*1.2f);
        EXPECT_LE(numExpected,numCorrected)<<"[CompareSpecialValue] seed = "<<seed<<"\n";
    }

}
// function to compare results of using FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES method for data generation
template<typename T> void CompareSpecialValue(float specialValuesProb, IMemoryObject* buf, uint64_t seed)
{
    BufferDesc desc = GetBufferDescription(buf->GetMemoryObjectDesc());
    T* data = (T*)buf->GetDataPtr();

    int32_t cntNaN = 0;
    int32_t cntZero = 0;
    int32_t cntPosInf = 0;
    int32_t cntNegInf = 0;
    int32_t cntDenorm = 0;

    T posInf = std::numeric_limits<T>::infinity();
    T negInf = -std::numeric_limits<T>::infinity();

    int32_t dataNumLen = (int32_t)(desc.NumOfElements() * desc.SizeOfVector());

    for( int32_t i=0; i<dataNumLen; i++)
    {
        if(data[i] == posInf)
            cntPosInf++;
        if(data[i] == negInf)
            cntNegInf++;
        if(data[i] == 0)
            cntZero++;
        if(my_isnan<T>(data[i],data[i]))
            cntNaN++;
        if(IsDenormValue(data[i]))
            cntDenorm++;
    }

    int32_t cntSum = cntNaN+cntZero+cntPosInf+cntNegInf+cntDenorm;
    int32_t numExpected = (int32_t)((float)dataNumLen*specialValuesProb);
#if 0
    std::cout<<"NaN "<<cntNaN<<" zero "<<cntZero<<" posINF "<<cntPosInf<<" negINF "<<cntNegInf<<" cntDenorm = "<<cntDenorm<<std::endl;
    std::cout<<" numExpected = "<<numExpected<<" cntSum = "<<cntSum;
    if(numExpected > cntSum) std::cout<<" cntSum is "<<((((float)(numExpected-cntSum))/((float)numExpected))*100)<<" % less";
    std::cout<<std::endl;
#endif
    if (specialValuesProb == 0.0f)
    {
        EXPECT_EQ(0,cntSum)<<"[CompareSpecialValue] seed = "<<seed;
    } else if (specialValuesProb == 1.0f)
    {
        EXPECT_EQ(dataNumLen,cntSum)<<"[CompareSpecialValue_CFloat16] seed = "<<seed;
    } else if(numExpected >= dataNumLen/4)
    {
        int32_t numCorrected = (int32_t)((float)numExpected*1.2f);
        EXPECT_LE(numExpected,numCorrected)<<"[CompareSpecialValue_CFloat16] seed = "<<seed;
    }
}


TEST(DataGenerator, SpecialValueTest) {

    const uint32_t NUM_TESTS  = 50;
    const int32_t probNum  = 7;
    float prob[probNum] = {0.0f, 0.1f, 0.3f, 0.5f, 0.7f, 0.9f, 1.0f};

    for(uint32_t buffLen=1; buffLen<= NUM_TESTS; buffLen++)
    {
        for(int32_t i=0; i<probNum; i++)
        {
            for(int32_t vecSize=(int32_t)V1; vecSize<(int32_t)INVALID_WIDTH; vecSize++)
            {
                VectorWidthWrapper vecWidth;
                vecWidth.SetValue(VectorWidth(vecSize));
                DataTypeValWrapper dataType;

                DataGenerator::BufferContainerFillMethod bcfm;
                DataGenerator::BufferContainerListFillMethod bclfm;
                BufferContainerList list;

                dataType.SetValue(F16);
                DataGenerator::SetSpecialValuesMethod(bcfm,dataType,vecWidth,buffLen,prob[i]);
                //DataGenerator::SetBufferContainerFillMethod<CFloat16>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES,0.f,0.f,0.f,prob[i]);
                dataType.SetValue(F32);
                DataGenerator::SetSpecialValuesMethod(bcfm,dataType,vecWidth,buffLen,prob[i]);
                //DataGenerator::SetBufferContainerFillMethod<float>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES,0,0,0,prob[i]);
                dataType.SetValue(F64);
                DataGenerator::SetSpecialValuesMethod(bcfm,dataType,vecWidth,buffLen,prob[i]);
                //DataGenerator::SetBufferContainerFillMethod<double>(bcfm,dataType,vecWidth,buffLen,DataGenerator::FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES,0,0,0,prob[i]);

                DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);

                DataGenerator dg(bclfm);

                dg.Read(&list);

                uint64_t seed = dg.GetSeed();
                CompareSpecialValue_CFloat16(prob[i],list.GetBufferContainer(0)->GetMemoryObject(0),seed);
                CompareSpecialValue<float>(prob[i],list.GetBufferContainer(0)->GetMemoryObject(1),seed);
                CompareSpecialValue<double>(prob[i],list.GetBufferContainer(0)->GetMemoryObject(2),seed);

            }
        }
    }
}

TEST(DataGenerator, MultipleBufferContainersTest) {
    const uint32_t NUM_TESTS  = 50;

    for(uint32_t testId = 0; testId < NUM_TESTS; ++testId)
    {
        for(int32_t vecSize=(int32_t)V1; vecSize<(int32_t)INVALID_WIDTH; vecSize++)
        {
            uint32_t buffLen = testId;
            VectorWidthWrapper vecWidth;
            vecWidth.SetValue(VectorWidth(vecSize));
            DataTypeValWrapper dataType;

            BufferContainerList BCList;
            DataGenerator::BufferContainerListFillMethod bclfm;

            {
                DataGenerator::BufferContainerFillMethod bcfm;
                dataType.SetValue(I64);
                DataGenerator::SetRandomMethod(bcfm, dataType, vecWidth, buffLen);
                dataType.SetValue(U64);
                DataGenerator::SetRandomMethod(bcfm, dataType, vecWidth, buffLen);
                DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);
            }

            {
                DataGenerator::BufferContainerFillMethod bcfm;
                dataType.SetValue(F16);
                DataGenerator::SetRandomMethod(bcfm, dataType, vecWidth, buffLen);
                dataType.SetValue(F32);
                DataGenerator::SetRandomMethod(bcfm, dataType, vecWidth, buffLen);
                dataType.SetValue(F64);
                DataGenerator::SetRandomMethod(bcfm, dataType, vecWidth, buffLen);
                DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);
            }

            DataGenerator dg(bclfm);
            dg.Read(&BCList);

            EXPECT_EQ(BCList.GetBufferContainerCount(), (std::size_t)2);
            EXPECT_EQ(BCList.GetBufferContainer(0)->GetMemoryObjectCount(), (std::size_t)2);
            EXPECT_EQ(BCList.GetBufferContainer(1)->GetMemoryObjectCount(), (std::size_t)3);
        }
    }
}

struct ArbitraryTestStruct
{
    DataTypeValWrapper dataType;
    DataGenerator::FillDataType value;
    DataGenerator::FillDataType low;
    DataGenerator::FillDataType high;
    DataGenerator::FillMethod fillMethod;
    float prob;
};

TEST(DataGenerator, ArbitraryTest)
{
    ArbitraryTestStruct tS;
    std::vector<ArbitraryTestStruct> testVector;

    VectorWidthWrapper vectorWidth;

    DataGenerator::BufferContainerFillMethod bcfm;
    DataGenerator::BufferContainerListFillMethod bclfm;
    BufferContainerList list;
// to fill buffer of 319 vectors of 8 elements of int32_t type by
// random data in range (-2090,5762)
    tS.dataType.SetValue(I32);
    tS.value.dI32 = 999; // doesn't matter if FILL_RANDOM_FROM_RANGE
    tS.low.dI32 = -2090;
    tS.high.dI32 = 5762;
    vectorWidth.SetValue(V8);
    tS.fillMethod = DataGenerator::FILL_RANDOM_FROM_RANGE;
    tS.prob = 0.6f; // doesn't matter if FILL_RANDOM_FROM_RANGE
    DataGenerator::SetBufferContainerFillMethod<int32_t>(bcfm, tS.dataType,
                   vectorWidth, 319, tS.fillMethod, tS.value.dI32, tS.low.dI32,
                   tS.high.dI32, tS.prob);
    testVector.push_back(tS);
// to fill buffer of 319 vectors of 8 elements of int32_t type by const value 999
    tS.dataType.SetValue(I32);
    tS.value.dI32 = 999;
    tS.low.dI32 = -2090; // doesn't matter if FILL_VALUE
    tS.high.dI32 = 5762; // doesn't matter if FILL_VALUE
    vectorWidth.SetValue(V8);
    tS.fillMethod = DataGenerator::FILL_VALUE;
    tS.prob = 0.6f; // doesn't matter if FILL_VALUE
    DataGenerator::SetBufferContainerFillMethod<int32_t>(bcfm, tS.dataType,
                            vectorWidth, 319, tS.fillMethod, tS.value.dI32,
                            tS.low.dI32, tS.high.dI32, tS.prob);
    testVector.push_back(tS);
// to fill buffer of 319 vectors of 8 elements of float by random data with
// 0.6 probability of special values
// i.e. about 319*8*0.6=1531 special values like NaN,+INF,-INF,+0,-0,DeNORM
    tS.dataType.SetValue(F32);
    tS.value.dF32 = 999; // doesn't matter if FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES
    tS.low.dF32 = -2090; // doesn't matter if FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES
    tS.high.dF32 = 5762; // doesn't matter if FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES
    vectorWidth.SetValue(V4);
    tS.fillMethod = DataGenerator::FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES;
    tS.prob = 0.6f;
    DataGenerator::SetBufferContainerFillMethod<float>(bcfm, tS.dataType,
                          vectorWidth, 319, tS.fillMethod, tS.value.dF32,
                          tS.low.dF32, tS.high.dF32, tS.prob);
    testVector.push_back(tS);
//////////////////////////
//  next test case
/////////////////////////

    DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);

    DataGenerator dg(bclfm);
    uint64_t seed = dg.GetSeed();

    dg.Read(&list);


    for( uint32_t i=0;i<testVector.size();i++)
    {
         ArbitraryTestStruct t = testVector.at(i);
         switch(t.fillMethod)
         {
         case DataGenerator::FILL_VALUE:
             CompareFillValue(t.dataType,t.value,
                              list.GetBufferContainer(0)->GetMemoryObject(i));
             break;
         case DataGenerator::FILL_RANDOM:
             // nothing to compare
             break;
         case DataGenerator::FILL_RANDOM_FROM_RANGE:
             CompareRandomRange(t.dataType,t.low,t.high,
                                list.GetBufferContainer(0)->GetMemoryObject(i),seed);
             break;
         case DataGenerator::FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES:
             if(t.dataType.GetValue()==F16)
             {
                 CompareSpecialValue_CFloat16(t.prob,
                                      list.GetBufferContainer(0)->GetMemoryObject(i),seed);
             }
             else if (t.dataType.GetValue()==F32)
             {
                 CompareSpecialValue<float>(t.prob,
                                     list.GetBufferContainer(0)->GetMemoryObject(i),seed);
             }
             else if (t.dataType.GetValue()==F64)
             {
                 CompareSpecialValue<double>(t.prob,
                                     list.GetBufferContainer(0)->GetMemoryObject(i),seed);
             }
             else
             {
                 std::cerr << "[ArbitraryTest] Unsupported data format\n";
             }
             break;
         default:
             std::cerr << "[ArbitraryTest] Unsupported data format\n";
             throw;
             break;
         }
    }

}

/// Tests for DataGenerator helper's functions GenerateRandomVectors[Seed],
/// GenerateRangedVectors[Seed], GeneratespecialVectors[Seed]

//// Use google type tests feature
template <typename T>
class DataGeneratorTypedUnsigned : public ::testing::Test {};
template <typename T>
class DataGeneratorTypedSigned : public ::testing::Test {};
template <typename T>
class DataGeneratorTypedFloat : public ::testing::Test {};

typedef ::testing::Types< uint8_t, uint16_t, uint32_t, uint64_t > UnsignedDGTypes;
typedef ::testing::Types< int8_t, int16_t, int32_t, int64_t > SignedDGTypes;
typedef ::testing::Types< CFloat16, float, double > FloatDGTypes;

TYPED_TEST_CASE(DataGeneratorTypedUnsigned, UnsignedDGTypes);
TYPED_TEST_CASE(DataGeneratorTypedSigned, SignedDGTypes);
TYPED_TEST_CASE(DataGeneratorTypedFloat, FloatDGTypes);

template <typename T> void TestGenerateRandomVectors(DataTypeVal dataTypeVal)
{
    const uint32_t NUM_TESTS = 10;
    T data1[NUM_TESTS*16], data2[NUM_TESTS*16];
    uint64_t seed1, seed2;

    for(uint32_t buffLen = 1; buffLen <= NUM_TESTS; buffLen++)
    {
        for(uint32_t vecSize = uint32_t(V2); vecSize<uint32_t(INVALID_WIDTH); vecSize++)
        {
            VectorWidthWrapper vecWidth;
            vecWidth.SetValue(VectorWidth(vecSize));
            /// generates buffLen number of random vectors of vecSize width of dataTypeVal data type
            seed1 = GenerateRandomVectors(dataTypeVal, &data1[0], VectorWidth(vecSize), buffLen);
            seed2 = seed1;
            GenerateRandomVectorsSeed(dataTypeVal, &data2[0], VectorWidth(vecSize), buffLen, seed2);

            for(uint32_t i=0;i<buffLen;i++) {
                for(uint32_t j=0;j<vecWidth.GetSize();j++) {
                     EXPECT_EQ(data1[i*vecWidth.GetSize()+j],data2[i*vecWidth.GetSize()+j])
                         <<"[TestGenerateRandomVectors] seed = "<<seed2<<"\n";
                }
            }
        }
    }
}

TYPED_TEST(DataGeneratorTypedUnsigned, GenerateRandomVectors)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);

    if( typeSize == sizeof(uint8_t))
        dataTypeVal = U8;
    else if ( typeSize == sizeof(uint16_t))
        dataTypeVal = U16;
    else if ( typeSize == sizeof(uint32_t))
        dataTypeVal = U32;
    else if ( typeSize == sizeof(uint64_t))
        dataTypeVal = U64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateRandomVectors<TypeParam>(dataTypeVal);
}

TYPED_TEST(DataGeneratorTypedSigned, GenerateRandomVectors)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);

    if( typeSize == sizeof(int8_t))
        dataTypeVal = I8;
    else if ( typeSize == sizeof(int16_t))
        dataTypeVal = I16;
    else if ( typeSize == sizeof(int32_t))
        dataTypeVal = I32;
    else if ( typeSize == sizeof(int64_t))
        dataTypeVal = I64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateRandomVectors<TypeParam>(dataTypeVal);
}

TYPED_TEST(DataGeneratorTypedFloat, GenerateRandomVectors)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);

    if ( typeSize == sizeof(CFloat16))
        dataTypeVal = F16;
    else if ( typeSize == sizeof(float))
        dataTypeVal = F32;
    else if ( typeSize == sizeof(double))
        dataTypeVal = F64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateRandomVectors<TypeParam>(dataTypeVal);
}


template <typename T> void TestGenerateRangedVectors(DataTypeVal dataTypeVal, T low, T high)
{
    const uint32_t NUM_TESTS = 10;
    T data1[NUM_TESTS*16], data2[NUM_TESTS*16];
    uint64_t seed1, seed2;

    for(uint32_t buffLen=1; buffLen<=NUM_TESTS; buffLen++)
    {
        for(uint32_t vecSize = uint32_t(V1); vecSize<uint32_t(INVALID_WIDTH); vecSize++)
        {
            seed1 = GenerateRangedVectors(dataTypeVal, &data1[0], VectorWidth(vecSize), buffLen, low, high);
            seed2 = seed1;
            GenerateRangedVectorsSeed(dataTypeVal, &data2[0], VectorWidth(vecSize), buffLen, low, high, seed2);

            for(uint32_t i=0;i<buffLen;i++)
                for(uint32_t j=0;j<vecSize;j++) {
                     EXPECT_EQ(data1[i*vecSize+j],data2[i*vecSize+j])
                     <<"[TestGenerateRangedVectors] seed = "<<seed2<<"\n";
                }
        }
    }
}

TYPED_TEST(DataGeneratorTypedUnsigned, GenerateRangedVectors)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);
    TypeParam highValue;
    TypeParam lowValue;

    do {
        highValue = (TypeParam)(rand() % std::numeric_limits<TypeParam>::max());
        lowValue = (TypeParam)(rand() % std::numeric_limits<TypeParam>::max());
    } while (highValue <= lowValue);

    if( typeSize == sizeof(uint8_t))
        dataTypeVal = U8;
    else if ( typeSize == sizeof(uint16_t))
        dataTypeVal = U16;
    else if ( typeSize == sizeof(uint32_t))
        dataTypeVal = U32;
    else if ( typeSize == sizeof(uint64_t))
        dataTypeVal = U64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateRangedVectors<TypeParam>(dataTypeVal, lowValue, highValue);
}

TYPED_TEST(DataGeneratorTypedSigned, GenerateRangedVectors)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);
    TypeParam highValue;
    TypeParam lowValue;

    do {
        highValue = (TypeParam)(rand() % std::numeric_limits<TypeParam>::max() + 1);
        lowValue = (TypeParam)(highValue-(rand() % (std::numeric_limits<TypeParam>::max()+1)));
    } while (highValue <= lowValue);

    if( typeSize == sizeof(int8_t))
        dataTypeVal = I8;
    else if ( typeSize == sizeof(int16_t))
        dataTypeVal = I16;
    else if ( typeSize == sizeof(int32_t))
        dataTypeVal = I32;
    else if ( typeSize == sizeof(int64_t))
        dataTypeVal = I64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateRangedVectors<TypeParam>(dataTypeVal, lowValue, highValue);
}

TYPED_TEST(DataGeneratorTypedFloat, GenerateRangedVectors)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);

    TypeParam highValue;
    TypeParam lowValue;

    // special case for CFloat16, because this type is used to store data only,
    // operations on CFloat16 are made after data casted to 32bit float type
    if ( typeSize == sizeof(CFloat16)) {
        float highValueFloat;
        float lowValueFloat;

        CFloat16 max16 = CFloat16::GetMax();
        uint16_t a = *(uint16_t*)&max16;
        int32_t b = (int32_t)a;

        do {
            do {
                highValueFloat = float(rand()%b);
                lowValueFloat = highValueFloat-float(rand()%b);
            } while (highValueFloat <= lowValueFloat);
            highValue = CFloat16(highValueFloat);
            lowValue = CFloat16(lowValueFloat);
        } while (float(highValue) <= float(lowValue));

    } else {

        do {
            highValue = TypeParam(float(rand()));
            lowValue = highValue-(TypeParam)((float)rand());
        } while (highValue <= lowValue);

    }

    if ( typeSize == sizeof(CFloat16))
        dataTypeVal = F16;
    else if ( typeSize == sizeof(float))
        dataTypeVal = F32;
    else if ( typeSize == sizeof(double))
        dataTypeVal = F64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateRangedVectors<TypeParam>(dataTypeVal, lowValue, highValue);
}


template <typename T> void TestGenerateSpecialVectors(DataTypeVal dataTypeVal, float prob)
{
    const uint32_t NUM_TESTS = 10;
    T data1[NUM_TESTS*16], data2[NUM_TESTS*16];
    uint64_t seed1, seed2;

    for(uint32_t buffLen=1; buffLen<=NUM_TESTS; buffLen++)
    {
        for(uint32_t vecSize = uint32_t(V1); vecSize<uint32_t(INVALID_WIDTH); vecSize++)
        {
            seed1 = GenerateSpecialVectors(dataTypeVal, &data1[0], VectorWidth(vecSize),
                                           buffLen, prob);
            seed2 = seed1;
            GenerateSpecialVectorsSeed(dataTypeVal, &data2[0], VectorWidth(vecSize),
                                       buffLen, prob, seed2);
            for(uint32_t i=0;i<buffLen;i++)
                for(uint32_t j=0;j<vecSize;j++) {
                    bool b1 = my_isnan(data1[i*vecSize+j],data1[i*vecSize+j]);

                    if(b1) {
                        bool b2 = my_isnan(data2[i*vecSize+j],data2[i*vecSize+j]);
                        EXPECT_EQ(b1,b2)<<"[TestGenerateRangedVectors] seed = "<<seed2<<"\n";
                    }
                    else
                        EXPECT_EQ(data1[i*vecSize+j],data2[i*vecSize+j])<<
                        "[TestGenerateRangedVectors] seed = "<<seed2<<"\n";
                }
        }
    }
}

TYPED_TEST(DataGeneratorTypedFloat, GenerateSpecialVectors)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);

    float prob = 1.0f;

    if ( typeSize == sizeof(CFloat16))
        dataTypeVal = F16;
    else if ( typeSize == sizeof(float))
        dataTypeVal = F32;
    else if ( typeSize == sizeof(double))
        dataTypeVal = F64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateSpecialVectors<TypeParam>(dataTypeVal, prob);
}

template <typename T> void TestGenerateRandomVectorsAutoSeed(DataTypeVal dataTypeVal)
{
    const uint32_t NUM_TESTS = 10;
    T data1[NUM_TESTS*16], data2[NUM_TESTS*16];
    uint64_t seed1, seed2;

    for(uint32_t vecSize = uint32_t(V2); vecSize<uint32_t(INVALID_WIDTH); vecSize++)
    {
        for(uint32_t i=0;i<NUM_TESTS*16;i++) {
            data1[i] = T(0.f);
            data2[i] = T(0.f);
        }
        SetSeed(0); //set initial seed, random

        for(uint32_t buffLen = 1; buffLen <= NUM_TESTS; buffLen++)
        {
            VectorWidthWrapper vecWidth;
            vecWidth.SetValue(VectorWidth(vecSize));
            /// generates buffLen number of random vectors of vecSize width of dataTypeVal data type
            GenerateRandomVectorsAutoSeed(dataTypeVal, &data1[0], VectorWidth(vecSize), buffLen);
            seed1 = GetCurrentSeed();
            GenerateRandomVectorsAutoSeed(dataTypeVal, &data2[0], VectorWidth(vecSize), buffLen);
            seed2 = GetCurrentSeed();
            EXPECT_EQ(seed2,(seed1+GetUpdateConst())); // new seed should be equal to previous seed plus constant

            uint32_t diffCnt = 0;
            for(uint32_t i=0;i<buffLen;i++) {
                for(uint32_t j=0;j<vecWidth.GetSize();j++) {
                    if(data1[i*vecWidth.GetSize()+j] != data2[i*vecWidth.GetSize()+j])
                        diffCnt++;
                }
            }
            if(diffCnt == 0)
                llvm::errs()<<"[DATA GENERATOR WARNING] zero differences from "<<buffLen*vecWidth.GetSize()<<" elements\n";
        }
    }
}

TYPED_TEST(DataGeneratorTypedUnsigned, GenerateRandomVectorsAutoSeed)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);

    if( typeSize == sizeof(uint8_t))
        dataTypeVal = U8;
    else if ( typeSize == sizeof(uint16_t))
        dataTypeVal = U16;
    else if ( typeSize == sizeof(uint32_t))
        dataTypeVal = U32;
    else if ( typeSize == sizeof(uint64_t))
        dataTypeVal = U64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateRandomVectorsAutoSeed<TypeParam>(dataTypeVal);
}

TYPED_TEST(DataGeneratorTypedSigned, GenerateRandomVectorsAutoSeed)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);

    if( typeSize == sizeof(int8_t))
        dataTypeVal = I8;
    else if ( typeSize == sizeof(int16_t))
        dataTypeVal = I16;
    else if ( typeSize == sizeof(int32_t))
        dataTypeVal = I32;
    else if ( typeSize == sizeof(int64_t))
        dataTypeVal = I64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateRandomVectorsAutoSeed<TypeParam>(dataTypeVal);
}

TYPED_TEST(DataGeneratorTypedFloat, GenerateRandomVectorsAutoSeed)
{
    DataTypeVal dataTypeVal;
    size_t typeSize = sizeof(TypeParam);

    if ( typeSize == sizeof(CFloat16))
        dataTypeVal = F16;
    else if ( typeSize == sizeof(float))
        dataTypeVal = F32;
    else if ( typeSize == sizeof(double))
        dataTypeVal = F64;
    else
        dataTypeVal = INVALID_DATA_TYPE;

    EXPECT_NE(dataTypeVal, INVALID_DATA_TYPE);

    TestGenerateRandomVectorsAutoSeed<TypeParam>(dataTypeVal);
}
