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

File Name:  DGHelper.cpp

\*****************************************************************************/

#include "DGHelper.h"

namespace Validation {

static const uint64_t updateConst = 27345;

static uint64_t currentSeed = 0;
// some constant to start if seed is not set externally or randomly, i.e.
// if function SetSeed hasn't been called
static uint64_t startSeed = 1298562914; 
static bool setSeedFlag = false;

// for testing
uint64_t GetUpdateConst(void) {
    return updateConst;
}

// if we have externally set seed, use it,
// otherwise generate random seed
uint64_t SetSeed(uint64_t seed) {
    if(seed)
        startSeed = seed;
    else {
        startSeed = time(NULL)+rand();
    }
    currentSeed = startSeed;
    setSeedFlag = true;
    return startSeed; 
}

uint64_t GetCurrentSeed(void) {
    return currentSeed;
}

bool GetSeedFlag(void) {
    return setSeedFlag;
}

void UpdateCurrentSeed(void) {
        setSeedFlag = true;
        currentSeed += updateConst;
}

void GenerateRandomVectorsAutoSeed(DataTypeVal dataTypeVal, const float *arr,
                                      VectorWidth vecW, const uint32_t n)
{
    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateRandomVectors] zero data pointer");

    if(!GetSeedFlag())
        llvm::errs()<<"[GenerateRandomVectorsAutoSeed WARNING] seed is not set. Default value is used.\n";

    union {uint32_t u; float f;} y;

    DataGenerator::BufferContainerFillMethod bcfm;
    DataGenerator::BufferContainerListFillMethod bclfm;
    BufferContainerList list;
    DataTypeValWrapper dataTypeExp,dataTypeMant;
    VectorWidthWrapper vecWidth;
    vecWidth.SetValue(VectorWidth(vecW));
    
    uint8_t low8 = 0x01; // min exponent value for normalized float 8 bits
    uint8_t high8 = 0xfe; // max exponent value for normalized float 8 bits
    dataTypeExp.SetValue(U8);
    DataGenerator::SetRandomFromRangeMethod(bcfm,dataTypeExp,vecWidth,n,low8,high8);

    int32_t low23 = -8388608; // min mantissa for normalized float - 23 bits
    int32_t high23 = 8388607; // max mantissa for normalized float - 23 bits (0x7fffff)
    dataTypeMant.SetValue(I32);
    DataGenerator::SetRandomFromRangeMethod(bcfm,dataTypeMant,vecWidth,n,low23,high23);

    DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);
    DataGenerator dg(bclfm,GetCurrentSeed());
    dg.Read(&list);
    uint8_t* dataExp = (uint8_t*)list.GetBufferContainer(0)->GetMemoryObject(0)->GetDataPtr();
    int32_t* dataMant = (int32_t*)list.GetBufferContainer(0)->GetMemoryObject(1)->GetDataPtr();

    float *arrOfVec = (float*)arr;
    for(uint32_t i = 0; i<n; i++) {       
         for(uint32_t j = 0; j<vecWidth.GetSize(); j++)
         {
             // float number : 1 bit sign, 8 bits exponent, 23 bits mantissa
             uint32_t expVal = (uint32_t)dataExp[j];
             expVal <<= 23; // move exponent higher than mantissa
             y.u =  (uint32_t)dataMant[j]; // set mantissa value            
             y.u &= 0x807fffff; // mask 8 bits of exponent
             y.u |= expVal; // add exponent
             arrOfVec[j] = y.f;
         }
         arrOfVec += vecWidth.GetSize();
         dataExp += vecWidth.GetSize();
         dataMant += vecWidth.GetSize();
    }

    UpdateCurrentSeed();
}


void GenerateRandomVectorsAutoSeed(DataTypeVal dataTypeVal, const double *arr,
                                      VectorWidth vecW, const uint32_t n)
{
    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateRandomVectors] zero data pointer");

    if(!GetSeedFlag())
        llvm::errs()<<"[GenerateRandomVectorsAutoSeed WARNING] seed is not set. Default value is used.\n";

    union {uint64_t u; double f;} y;

    DataGenerator::BufferContainerFillMethod bcfm;
    DataGenerator::BufferContainerListFillMethod bclfm;
    BufferContainerList list;
    DataTypeValWrapper dataTypeExp,dataTypeMant;
    VectorWidthWrapper vecWidth;
    vecWidth.SetValue(VectorWidth(vecW));
    
    uint16_t low11 = 0x0001; // min exponent value for normalized double 11 bits
    uint16_t high11 = 0x07ff; // max exponent value for normalized double 11 bits
    dataTypeExp.SetValue(U16);
    DataGenerator::SetRandomFromRangeMethod(bcfm,dataTypeExp,vecWidth,n,low11,high11);

    int64_t low52 = -4503599627370496; // min mantissa for normalized double 52 bits
    int64_t high52 = 4503599627370495; // max mantissa for normalized double 52 bits (0x0FFFFFFFFFFFFF)
    dataTypeMant.SetValue(I64);
    DataGenerator::SetRandomFromRangeMethod(bcfm,dataTypeMant,vecWidth,n,low52,high52);

    DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);
    DataGenerator dg(bclfm,GetCurrentSeed());
    dg.Read(&list);
    uint16_t* dataExp = (uint16_t*)list.GetBufferContainer(0)->GetMemoryObject(0)->GetDataPtr();
    int64_t* dataMant = (int64_t*)list.GetBufferContainer(0)->GetMemoryObject(1)->GetDataPtr();

    double *arrOfVec = (double*)arr;
    for(uint32_t i = 0; i<n; i++) {       
         for(uint32_t j = 0; j<vecWidth.GetSize(); j++)
         {
             // double number : 1 bit sign, 11 bits exponent, 52 bits mantissa
             uint64_t expVal = (uint64_t)dataExp[j];
             expVal <<= 52; // move exponent higher than mantissa
             y.u =  (uint64_t)dataMant[j]; // set mantissa value            
             y.u &= 0x800fffffffffffff; // mask 11 bits of exponent
             y.u |= expVal; // add exponent
             arrOfVec[j] = y.f;
         }
         arrOfVec += vecWidth.GetSize();
         dataExp += vecWidth.GetSize();
         dataMant += vecWidth.GetSize();
    }

    UpdateCurrentSeed();
}



}