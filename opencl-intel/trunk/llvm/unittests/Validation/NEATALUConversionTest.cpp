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

File Name:  NEATALUConversionTest.cpp

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
class NEATAluTypedConversion : public ALUTest {
public:
    // Number of different random inputs to test.
    static const int NUM_TESTS = 500;
    // Vector data type length. WARNING! should be aligned with convert_float* function call!
    static const int vectorWidth = 4;

    // Random values for the first argument.
    T src[NUM_TESTS*vectorWidth];

    // Parameters for random data generator.
    VectorWidth currWidth;
    DataTypeVal dataTypeVal;

    NEATAluTypedConversion(){
        currWidth = VectorWidthWrapper::ValueOf(vectorWidth);
        switch (sizeof(T))
        {
        case 1:
            dataTypeVal = I8;
            break;
        case 2:
            dataTypeVal = I16;
            break;
        case 4:
            dataTypeVal = I32;
            break;
        case 8:
            dataTypeVal = I64;
            break;
        default:
            dataTypeVal = UNSPECIFIED_DATA_TYPE;
        }

        // Fill up data
        this->seed = GenerateRandomVectors(dataTypeVal, &src[0], currWidth, NUM_TESTS);
    }
};

typedef ::testing::Types<char,short,int,long,long long> IntTypesConversion;
TYPED_TEST_CASE(NEATAluTypedConversion, IntTypesConversion);

TYPED_TEST(NEATAluTypedConversion, convert_float)
{
    // Scalar test
    for(int testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        NEATValue result = NEATALU::convert_float<TypeParam>(&this->src[testIdx]);
        EXPECT_TRUE(TestAccValue<float>(result, float(this->src[testIdx])));
    }

    // Vector test
    for(int testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        NEATVector result = NEATALU::convert_float<TypeParam, 4>(this->src+(this->vectorWidth)*testIdx);
        for (int i = 0; i < this->vectorWidth; ++i) {
            EXPECT_TRUE(TestAccValue<float>(result[i], float(this->src[this->vectorWidth*testIdx+i])));
        }
    }
}

