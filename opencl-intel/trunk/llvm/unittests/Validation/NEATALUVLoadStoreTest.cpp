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

File Name:  NEATALUVLoadStoreTest.cpp

\*****************************************************************************/

// \brief Tests for OpenCL vector load and store built-in functions (see spec. 6.11.7.) in NEATALU

#include <gtest/gtest.h>            // Test framework

#include "DataGenerator.h"
#include "DGHelper.h"

#include "NEATVector.h"
#include "RefALU.h"
#include "NEATALU.h"
#include "NEATValue.h"
#include "ALUTest.h"

#include "NEATALUUtils.h"

using namespace Validation;

template <typename T>
class NEATAluTypedVLoadStore : public ALUTest {
public:
    // Number of different random inputs to test.
    static const int NUM_TESTS = 500;
    // Vector data type length. WARNING! should be aligned with vload* function call!
    static const int vectorWidth = 8;

    // Argument values.
    size_t offset[NUM_TESTS];
    T Arg2Min[NUM_TESTS*vectorWidth];
    T Arg2Max[NUM_TESTS*vectorWidth];
    NEATValue p[NUM_TESTS*vectorWidth];

    // Parameters for random data generator.
    VectorWidth currWidth;
    DataTypeVal dataTypeVal;

    NEATAluTypedVLoadStore(){
        currWidth = VectorWidthWrapper::ValueOf(vectorWidth);
        if(sizeof(T) == sizeof(float)) {
            dataTypeVal = F32;
        } else if(sizeof(T) == sizeof(double)){
            dataTypeVal = F64;
        }
        else{
            dataTypeVal = UNSPECIFIED_DATA_TYPE;
        }

        // Fill up data
        memset(&offset[0], 0, NUM_TESTS*sizeof(size_t));

        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Max[0], currWidth, NUM_TESTS);
        for(unsigned int i = 0; i < NUM_TESTS*vectorWidth; ++i) {
            p[i] = NEATValue(std::min(Arg2Min[i], Arg2Max[i]), std::max(Arg2Min[i], Arg2Max[i]));
        }
    }
};

typedef ::testing::Types<float> FloatTypesVLoadStore;
TYPED_TEST_CASE(NEATAluTypedVLoadStore, FloatTypesVLoadStore);

TYPED_TEST(NEATAluTypedVLoadStore, vload)
{
    if (SkipDoubleTest<TypeParam>()){
        return;
    }

    for(int testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        NEATVector result = NEATALU::vload8<TypeParam>(this->offset[testIdx], &this->p[testIdx*this->vectorWidth]);
        for (int i = 0; i < NEATAluTypedVLoadStore<TypeParam>::vectorWidth; ++i) {
            EXPECT_TRUE(NEATValue::areEqual<TypeParam>(this->p[testIdx*this->vectorWidth+this->offset[testIdx]+i], result[i]));
        }
    }
}

TYPED_TEST(NEATAluTypedVLoadStore, vstore)
{
    if (SkipDoubleTest<TypeParam>()){
        return;
    }

    for(int testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // data vector to be stored
        NEATVector data(this->currWidth);
        // array to be filled by stored vector data
        NEATValue p[this->vectorWidth];

        for (int i = 0; i < NEATAluTypedVLoadStore<TypeParam>::vectorWidth; ++i) {
            data[i] = this->p[testIdx*this->vectorWidth+this->offset[testIdx]+i];
            p[i] = NEATValue(0);
        }

        NEATALU::vstore8<TypeParam>(data, this->offset[testIdx], &p[0]);

        for (int i = 0; i < NEATAluTypedVLoadStore<TypeParam>::vectorWidth; ++i) {
            EXPECT_TRUE(NEATValue::areEqual<TypeParam>(data[i], p[i]));
        }
    }
}