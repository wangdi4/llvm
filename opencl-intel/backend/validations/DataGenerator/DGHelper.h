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

File Name:  DGHelper.h

\*****************************************************************************/
#ifndef __DGHELPER_H__
#define __DGHELPER_H__

#include "llvm/Support/raw_ostream.h"
#include "DataGenerator.h"
#include <BufferContainerList.h>

// this file contains helper function for DataGenerator

namespace Validation
{

uint64_t SetSeed(uint64_t seed);
uint64_t GetCurrentSeed(void);
void UpdateCurrentSeed(void);

#define SET_BUFFER_CONTAINER\
    size_t size = sizeof(T);\
    switch(dataTypeVal) {\
        case U8:\
        case I8:\
            if ( size != sizeof(uint8_t)) {\
                throw Exception::InvalidArgument(\
                "[DataGenerator::GenerateRandomVectors] wrong data type\n");\
            }\
            break;\
        case U16:\
        case I16:\
        case F16:\
            if ( size != sizeof(uint16_t)) {\
                throw Exception::InvalidArgument(\
                "[DataGenerator::GenerateRandomVectors] wrong data type\n");\
            }\
            break;\
        case U32:\
        case I32:\
        case F32:\
            if ( size != sizeof(uint32_t)) {\
                throw Exception::InvalidArgument(\
                "[DataGenerator::GenerateRandomVectors] wrong data type\n");\
            }\
            break;\
        case I64:\
        case U64:\
        case F64:\
            if ( size != sizeof(uint64_t)) {\
                throw Exception::InvalidArgument(\
                "[DataGenerator::GenerateRandomVectors] wrong data type\n");\
            }\
            break;\
        default:\
            throw Exception::InvalidArgument(\
            "[DataGenerator::GenerateRandomVectors] Unsupported data format");\
            break;\
    }\
    DataGenerator::BufferContainerFillMethod bcfm;\
    DataGenerator::BufferContainerListFillMethod bclfm;\
    BufferContainerList list;\
    DataTypeValWrapper dataType;\
    VectorWidthWrapper vecWidth;\
    vecWidth.SetValue(VectorWidth(vecW));\
    dataType.SetValue(dataTypeVal);

#define GET_BUFFER_CONTAINER\
    DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);\
    DataGenerator dg(bclfm);\
    dg.Read(&list);\
    seed = dg.GetSeed();\
    T* data = (T*)list.GetBufferContainer(0)->GetMemoryObject(0)->GetDataPtr();\
    T *arrOfVec = (T*)arr;\
    for(uint32_t i = 0; i<n; i++) {\
         for(uint32_t j = 0; j<vecWidth.GetSize(); j++) {\
             arrOfVec[j] = data[j]; }\
         arrOfVec += vecWidth.GetSize();\
         data += vecWidth.GetSize();\
    }

#define GET_BUFFER_CONTAINER_SEED(seed)\
    DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);\
    DataGenerator dg(bclfm,seed);\
    dg.Read(&list);\
    T* data = (T*)list.GetBufferContainer(0)->GetMemoryObject(0)->GetDataPtr();\
    T *arrOfVec = (T*)arr;\
    for(uint32_t i = 0; i<n; i++) {\
         for(uint32_t j = 0; j<vecWidth.GetSize(); j++) {\
             arrOfVec[j] = data[j]; }\
         arrOfVec += vecWidth.GetSize();\
         data += vecWidth.GetSize();\
    }

/// @brief Static function to generate a number of random vectors and
/// returns the internally calculated seed used for data generation
/// @param [in] dataTypeVal. Data type of data to be generated, see
/// Validation::DataTypeVal for allowed data types
/// @param [inout] *arr. Pointer to thirst vector to be generated.
/// data type T should match dataTypeVal value, for example
/// dataTypeVal==U8 corresponds to uint8_t *arr
/// @param [in] vecW. vector width, see Validation::VectorWidth
/// @param [in] n. number of vectors to be generated.
/// so, functions fills array *arr by random generated n vectors of
/// vecW width and dataTypeVal type. usage example:
/// uint32_t arr[16*8];
/// uint64_t seed = GenerateRandomVectors(U32,&arr[0],V8,16);
/// array arr is filled by unsigned 32-bit data, 16 vectors of 8 element

template <typename T>
static uint64_t GenerateRandomVectors(DataTypeVal dataTypeVal, const T *arr,
                                      VectorWidth vecW, const uint32_t n)
{
    uint64_t seed = 0;
    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateRandomVectors] zero data pointer\n");

    SET_BUFFER_CONTAINER

    DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,n);

    GET_BUFFER_CONTAINER

    return seed;
}

/// @brief Static function to generate a number of random vectors and
/// returns the internally calculated seed used for data generation
/// generated data is between low and high limits
/// @param [in] dataTypeVal. Data type of data to be generated, see
/// Validation::DataTypeVal for allowed data types
/// @param [inout] *arr. Pointer to thirst vector to be generated.
/// data type T should match dataTypeVal value, for example
/// dataTypeVal==U8 corresponds to uint8_t *arr
/// @param [in] vecW. vector width, see Validation::VectorWidth
/// @param [in] n. number of vectors to be generated.
/// @param [in] low. low limit of range
/// @param [in] high. high limit of range
/// so, functions fills array *arr by random generated n vectors of
/// vecW width and dataTypeVal type. usage example:
/// uint32_t arr[16*8];
/// uint64_t seed = GenerateRandomVectors(U32,&arr[0],V8,16,12,1000);
/// array arr is filled by unsigned 32-bit data, 16 vectors of 8 element
/// generated data is higher 12 and lower 1000
template <typename T>
static uint64_t GenerateRangedVectors(DataTypeVal dataTypeVal, const T *arr,
                                      VectorWidth vecW,
                                      const uint32_t n, T low, T high)
{
    uint64_t seed = 0;
    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateRangedVectors] zero data pointer");

    SET_BUFFER_CONTAINER

    DataGenerator::SetRandomFromRangeMethod(bcfm,dataType,vecWidth,n,low,high);

    GET_BUFFER_CONTAINER

    return seed;
}
/// @brief Static function to generate a number of random vectors
/// of special values: NaNs, INFs and
/// returns the internally calculated seed used for data generation
/// @param [in] dataTypeVal. Data type of data to be generated, see
/// Validation::DataTypeVal for allowed data types
/// @param [inout] *arr. Pointer to thirst vector to be generated.
/// data type T should match dataTypeVal value, for example
/// dataTypeVal==U8 corresponds to uint8_t *arr
/// @param [in] vecW. vector width, see Validation::VectorWidth
/// @param [in] n. number of vectors to be generated.
/// @param [in] prob. probability of special values in vector
/// if prob==0.5, the half of generated values are special values
/// so, functions fills array *arr by random generated n vectors of
/// vecW width and dataTypeVal type. usage example:
/// float arr[16*8];
/// uint64_t seed = GenerateRandomVectors(F32,&arr[0],V8,16,0.5);
/// array arr is filled by float 32-bit data, 16 vectors of 8 element
/// about 64 values (16*8*0.5) are special values
template <typename T>
static uint64_t GenerateSpecialVectors(DataTypeVal dataTypeVal, const T *arr,
                                       VectorWidth vecW,
                                       const uint32_t n, float prob)
{
    uint64_t seed = 0;
    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateSpecialVectors] zero data pointer");

    SET_BUFFER_CONTAINER

    DataGenerator::SetSpecialValuesMethod(bcfm,dataType,vecWidth,n,prob);

    GET_BUFFER_CONTAINER

    return seed;
}
/// @brief Static function to generate a number of random vectors with
/// predefined  seed
/// @param [in] dataTypeVal. Data type of data to be generated, see
/// Validation::DataTypeVal for allowed data types
/// @param [inout] *arr. Pointer to thirst vector to be generated.
/// data type T should match dataTypeVal value, for example
/// dataTypeVal==U8 corresponds to uint8_t *arr
/// @param [in] vecW. vector width, see Validation::VectorWidth
/// @param [in] n. number of vectors to be generated.
/// @param [in] seed. Seed used to generate data
/// so, functions fills array *arr by random generated n vectors of
/// vecW width and dataTypeVal type. usage example:
/// uint32_t arr[16*8];
/// GenerateRandomVectorsSeed(U32,&arr[0],V8,16, 1234567890);
/// array arr is filled by unsigned 32-bit data, 16 vectors of 8 element
template <typename T>
static void GenerateRandomVectorsSeed(DataTypeVal dataTypeVal, const T *arr,
                                      VectorWidth vecW,
                                      const uint32_t n, uint64_t seed)
{
    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateRandomVectors] zero data pointer");

    SET_BUFFER_CONTAINER

    DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,n);

    GET_BUFFER_CONTAINER_SEED(seed)
}
/// @brief Static function to generate a number of random vectors with
/// predefined  seed
/// generated data is between low and high limits
/// @param [in] dataTypeVal. Data type of data to be generated, see
/// Validation::DataTypeVal for allowed data types
/// @param [inout] *arr. Pointer to thirst vector to be generated.
/// data type T should match dataTypeVal value, for example
/// dataTypeVal==U8 corresponds to uint8_t *arr
/// @param [in] vecW. vector width, see Validation::VectorWidth
/// @param [in] n. number of vectors to be generated.
/// @param [in] low. low limit of range
/// @param [in] high. high limit of range
/// @param [in] seed. Seed used to generate data
/// so, functions fills array *arr by random generated n vectors of
/// vecW width and dataTypeVal type. usage example:
/// uint32_t arr[16*8];
/// GenerateRandomVectors(U32,&arr[0],V8,16,12,1000,123456);
/// array arr is filled by unsigned 32-bit data, 16 vectors of 8 element
/// generated data is higher 12 and lower 1000

template <typename T>
static void GenerateRangedVectorsSeed(DataTypeVal dataTypeVal, const T *arr,
                                      VectorWidth vecW,
                                      const uint32_t n, T low, T high,
                                      uint64_t seed)
{
    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateRandomVectors] zero data pointer");

    SET_BUFFER_CONTAINER

    DataGenerator::SetRandomFromRangeMethod(bcfm,dataType,vecWidth,n,low,high);

    GET_BUFFER_CONTAINER_SEED(seed)
}
/// @brief Static function to generate a number of random vectors with
/// predefined  seed
/// returns the internally calculated seed used for data generation
/// @param [in] dataTypeVal. Data type of data to be generated, see
/// Validation::DataTypeVal for allowed data types
/// @param [inout] *arr. Pointer to thirst vector to be generated.
/// data type T should match dataTypeVal value, for example
/// dataTypeVal==U8 corresponds to uint8_t *arr
/// @param [in] vecW. vector width, see Validation::VectorWidth
/// @param [in] n. number of vectors to be generated.
/// @param [in] prob. probability of special values in vector
/// if prob==0.5, the half of generated values are special values
/// @param [in] seed. Seed used to generate data
/// so, functions fills array *arr by random generated n vectors of
/// vecW width and dataTypeVal type. usage example:
/// float arr[16*8];
/// GenerateRandomVectors(F32,&arr[0],V8,16,0.5,1234567);
/// array arr is filled by float 32-bit data, 16 vectors of 8 element
/// about 64 values (16*8*0.5) are special values

template <typename T>
static void GenerateSpecialVectorsSeed(DataTypeVal dataTypeVal, const T *arr,
                                           VectorWidth vecW,
                                           const uint32_t n, float prob,
                                           uint64_t seed)
{
    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateSpecialVectors] zero data pointer");

    SET_BUFFER_CONTAINER

    DataGenerator::SetSpecialValuesMethod(bcfm,dataType,vecWidth,n,prob);

    GET_BUFFER_CONTAINER_SEED(seed)
}

// for testing
uint64_t GetUpdateConst(void);


bool GetSeedFlag(void);

// this function performs as like as GenerateRangedVectors, but 
// after every call this funcion updates currentSeed value by a predefined constant
template <typename T>
static void GenerateRangedVectorsAutoSeed(DataTypeVal dataTypeVal, const T *arr,
                                          VectorWidth vecW,
                                          const uint32_t n, T low, T high)
{
    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateRandomVectors] zero data pointer");

    if(!GetSeedFlag())
        llvm::errs()<<"[GenerateRangedVectorsAutoSeed WARNING] seed is not set. Default value is used.\n";

    SET_BUFFER_CONTAINER

    DataGenerator::SetRandomFromRangeMethod(bcfm,dataType,vecWidth,n,low,high);

    GET_BUFFER_CONTAINER_SEED(GetCurrentSeed())

    UpdateCurrentSeed();
}

// this function performs as like as GenerateRandomVectors, but produces float or double values with
// separately generated exponent and mantissa. Comparing with GenerateRandomVectors, it makes
// the more representable sampling of values whith small mantissa,  also
// after every call this funcion updates currentSeed value by a predefined constant
void GenerateRandomVectorsAutoSeed(DataTypeVal dataTypeVal, const double *arr,
                                   VectorWidth vecW, const uint32_t n);
void GenerateRandomVectorsAutoSeed(DataTypeVal dataTypeVal, const float *arr,
                                   VectorWidth vecW, const uint32_t n);

template <typename T>
void GenerateRandomVectorsAutoSeed(DataTypeVal dataTypeVal, const T *arr,
                                   VectorWidth vecW, const uint32_t n)
{    if(arr == NULL)
        throw Exception::InvalidArgument(
        "[DataGenerator::GenerateRandomVectors] zero data pointer\n");

    if(!GetSeedFlag())
        llvm::errs()<<"[GenerateRandomVectorsAutoSeed WARNING] seed is not set. Default value is used.\n";

    SET_BUFFER_CONTAINER

    DataGenerator::SetRandomMethod(bcfm,dataType,vecWidth,n);

    GET_BUFFER_CONTAINER_SEED(GetCurrentSeed())

    UpdateCurrentSeed();
}



#undef SET_BUFFER_CONTAINER
#undef GET_BUFFER_CONTAINER
#undef GET_BUFFER_CONTAINER_SEED

}
#endif // __DGHELPER_H__

