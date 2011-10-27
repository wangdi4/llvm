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

File Name:  NEATMathTest.cpp

\*****************************************************************************/

#include <gtest/gtest.h>            // Test framework

#include "DataGenerator.h"
#include "DGHelper.h"

#include "NEATVector.h"
#include "RefALU.h"
#include "NEATALU.h"
#include "NEATValue.h"

#include "NEATALUUtils.h"
#include "ALUTest.h"
#include "llvm/System/DataTypes.h"

using namespace Validation;

template <typename T>
class NEATMathTestOneArg : public ALUTest {
public:
    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 500;
    // Vector data type length.
    static const uint32_t vectorWidth = 8;
    // Intervals with test values for common functions arguments.

    // Parameters for random data generator.
    VectorWidth currWidth;
    DataTypeVal dataTypeVal;

    typedef typename  T::Type TypeP;

    TypeP Arg1Min[NUM_TESTS*vectorWidth];
    TypeP Arg1Max[NUM_TESTS*vectorWidth];

    NEATMathTestOneArg()
    {
        currWidth = VectorWidthWrapper::ValueOf(vectorWidth);
        RefALU::SetFTZmode(T::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false

        dataTypeVal = GetDataTypeVal<typename T::Type>();

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], currWidth, NUM_TESTS);

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
        }
    }
};


template <typename T>
class NEATMathTestTwoArgs : public ALUTest {
public:
    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 500;
    // Vector data type length.
    static const uint32_t vectorWidth = 8;
    // Intervals with test values for common functions arguments.

    // Parameters for random data generator.
    VectorWidth currWidth;
    DataTypeVal dataTypeVal;

    typedef typename  T::Type TypeP;

    TypeP Arg1Min[NUM_TESTS*vectorWidth];
    TypeP Arg1Max[NUM_TESTS*vectorWidth];
    TypeP Arg2Min[NUM_TESTS*vectorWidth];
    TypeP Arg2Max[NUM_TESTS*vectorWidth];

    NEATMathTestTwoArgs()
    {
        currWidth = VectorWidthWrapper::ValueOf(vectorWidth);
        RefALU::SetFTZmode(T::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false

        dataTypeVal = GetDataTypeVal<typename T::Type>();

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Max[0], currWidth, NUM_TESTS);

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
            if (Arg2Min[i] > Arg2Max[i]) std::swap(Arg2Min[i], Arg2Max[i]);
        }
    }
};

template <typename T>
class NEATMathTestThreeArgs : public ALUTest {
public:
    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 500;
    // Vector data type length.
    static const uint32_t vectorWidth = 8;
    // Intervals with test values for common functions arguments.

    // Parameters for random data generator.
    VectorWidth currWidth;
    DataTypeVal dataTypeVal;

    typedef typename  T::Type TypeP;

    TypeP Arg1Min[NUM_TESTS*vectorWidth];
    TypeP Arg1Max[NUM_TESTS*vectorWidth];
    TypeP Arg2Min[NUM_TESTS*vectorWidth];
    TypeP Arg2Max[NUM_TESTS*vectorWidth];
    TypeP Arg3Min[NUM_TESTS*vectorWidth];
    TypeP Arg3Max[NUM_TESTS*vectorWidth];

    NEATMathTestThreeArgs()
    {
        currWidth = VectorWidthWrapper::ValueOf(vectorWidth);
        RefALU::SetFTZmode(T::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false

        dataTypeVal = GetDataTypeVal<typename T::Type>();

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Max[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg3Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg3Max[0], currWidth, NUM_TESTS);

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
            if (Arg2Min[i] > Arg2Max[i]) std::swap(Arg2Min[i], Arg2Max[i]);
            if (Arg3Min[i] > Arg3Max[i]) std::swap(Arg3Min[i], Arg3Max[i]);
        }
    }
};

// TODO: add double tests
// To enable double testing, add double to Types template arguments parameter
typedef ::testing::Types<ValueTypeContainer<float,true>,ValueTypeContainer<float,false> > FloatTypesCommon;
TYPED_TEST_CASE(NEATMathTestOneArg, FloatTypesCommon);
TYPED_TEST_CASE(NEATMathTestTwoArgs, FloatTypesCommon);
TYPED_TEST_CASE(NEATMathTestThreeArgs, FloatTypesCommon);

TYPED_TEST(NEATMathTestOneArg, Ceil)
{
    typedef typename  TypeParam::Type TypeP;

    if (SkipDoubleTest<TypeP>()){
        return;
    }
    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown(NEATALU::ceil<TypeP>));
    EXPECT_TRUE(TestUnwritten(NEATALU::ceil<TypeP>));
    EXPECT_TRUE(TestAny(NEATALU::ceil<TypeP>));

    // Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    testVal = NEATALU::ceil<TypeP>(nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::ceil<TypeP>(notNAN);
        EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }

    // Test C99 edge cases
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::ceil<TypeP>, Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::ceil<TypeP>, Utils::GetNInf<TypeP>(), Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::ceil<TypeP>, TypeP (0.0), TypeP (0.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::ceil<TypeP>, TypeP (-0.0), TypeP (-0.0)));

    // Ocl specification
    // ceil(-1 < x < 0) = -0.0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::ceil<TypeP>, TypeP (-0.1f), TypeP(-0.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::ceil<TypeP>, TypeP (-0.5f), TypeP(-0.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::ceil<TypeP>, TypeP (-0.9f), TypeP(-0.0)));

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    // Done with corner cases. Now go to testing scalar and vector functions
    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // Test on accurate values.
        NEATVector x(Width);

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
        }

        /* test for single accurate NEAT value*/
        NEATValue xVal = x[0];

        testVal = NEATALU::ceil<TypeP>(xVal);
        typedef typename superT<TypeP>::type sT;
        sT refAccValFloat;

        refAccValFloat = RefALU::ceil(sT(*xVal.GetAcc<TypeP>()));
        EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeP>() == (TypeP)refAccValFloat);

        /* test for vector of NEAT accurate */
        NEATVector testVec = NEATALU::ceil<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::ceil(sT(*x[i].GetAcc<TypeP>()));
            EXPECT_TRUE(testVec[i].IsAcc() && *testVec[i].GetAcc<TypeP>() == (TypeP)refAccValFloat);
        }

        // Test on interval values.
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        /* test for single interval NEAT value */
        sT res1, res2;

        xVal = x[0];

        testVal = NEATALU::ceil<TypeP>(xVal);

        res1 = RefALU::ceil(sT(*xVal.GetMax<TypeP>()));
        res2 = RefALU::ceil(sT(*xVal.GetMin<TypeP>()));
        if(res1 == res2)
        {
            EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeP>() == (TypeP)res1);
        } else
        {
            EXPECT_TRUE(testVal.IsUnknown());
        }

        /* test for vector of NEAT intervals */
        testVec = NEATALU::ceil<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            res1 = RefALU::ceil(sT(*x[i].GetMax<TypeP>()));
            res2 = RefALU::ceil(sT(*x[i].GetMin<TypeP>()));
            if (res1 == res2) {
                EXPECT_TRUE(testVec[i].IsAcc() && *testVec[i].GetAcc<TypeP>() == (TypeP)res1);
            } else {
                EXPECT_TRUE(testVec[i].IsUnknown());
            }
        }
    }
}


template<typename T>
bool TestAtanhInterval(T argMin, T argMax, NEATValue testVal, int ulps)
{
    bool res = true;
    typedef typename  superT<T>::type sT;
    if((argMin > 1) || (argMax < -1))
    {
        if(!testVal.IsNaN<T>())
            res = false;
    } else if ((argMin >= -1) && (argMax <= 1))
    {
        sT refMin = RefALU::atanh(sT(argMin));
        sT refMax = RefALU::atanh(sT(argMax));
        res = TestIntExpanded(refMin, refMax, testVal, ulps);
    }
    else
    {
        res = testVal.IsUnknown();
    }
    return res;
}

template<typename T>
bool TestAcoshInterval(T argMin, T argMax, NEATValue testVal, int ulps)
{
    bool res = true;
    typedef typename  superT<T>::type sT;
    if(argMax < 1)
    {
        if(!testVal.IsNaN<T>())
            res = false;
    } else if (argMin >= 1)
    {
        sT refMin = RefALU::acosh(sT(argMin));
        sT refMax = RefALU::acosh(sT(argMax));
        res = TestIntExpanded(refMin, refMax, testVal, ulps);
    }
    else
    {
        res = testVal.IsUnknown();
    }
    return res;
}

TYPED_TEST(NEATMathTestOneArg, asinh)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type sT;

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    EXPECT_TRUE(TestUnknown(NEATALU::asinh<TypeP>));
    EXPECT_TRUE(TestUnwritten(NEATALU::asinh<TypeP>));
    EXPECT_TRUE(TestAny(NEATALU::asinh<TypeP>));

    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    testVal = NEATALU::sinh<TypeP>(nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    for(uint32_t i = 0; i< this->NUM_TESTS * this->vectorWidth; i++)
    {
        /// Tests non-NaNs as input.
        NEATValue notNaN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::asinh<TypeP>(notNaN);
        EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }
    /// C99 ISO/IEC 9899 TC2
    /// asinh(+-inf) = +-inf
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::asinh<TypeP>,
        Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::asinh<TypeP>,
        Utils::GetNInf<TypeP>(), Utils::GetNInf<TypeP>()));
    /// asinh(+-0) = +- 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::asinh<TypeP>,
        (TypeP)0.0, (TypeP)0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::asinh<TypeP>,
        (TypeP)-0.0, (TypeP)-0.0));

    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; testIdx++)
    {
        NEATVector x(Width);
        // Test accurate values
        for(uint32_t i = 0; i<this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx * this->vectorWidth + i]);
        }

        // 2.a Test single accurate value
        NEATValue xVal = x[0];

        // Determine edge case inputs for which interval shouldn't be expanded
        const uint32_t NUM_EDGE_VALS = 2;
        sT edgeVals[NUM_EDGE_VALS] = { (sT)0.0, (sT)-0.0 };

        testVal = NEATALU::asinh<TypeP>(xVal);
        sT refAccValFloat;

        sT arg = sT(*xVal.GetAcc<TypeP>());
        refAccValFloat = RefALU::asinh<sT>((sT)arg);
        EXPECT_TRUE(TestNeatAcc<sT>(xVal, testVal, refAccValFloat,
            NEATALU::ASINH_ERROR, edgeVals, NUM_EDGE_VALS));

        // 2.b Test vector of accurate values
        NEATVector testVec = NEATALU::asinh<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::asinh(sT(*x[i].GetAcc<TypeP>()));
            EXPECT_TRUE(TestNeatAcc<sT>(x[i], testVec[i], refAccValFloat,
                NEATALU::ASINH_ERROR, edgeVals, NUM_EDGE_VALS));
        }

        // 3. Test interval input
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // 3.a single interval testing
        xVal = x[0];

        testVal = NEATALU::asinh<TypeP>(xVal);
        /// Arg1Min == Arg1Max has already been tested
        if(!xVal.IsAcc())
        {
            sT refMin = RefALU::asinh(sT(*xVal.GetMin<TypeP>()));
            sT refMax = RefALU::asinh(sT(*xVal.GetMax<TypeP>()));
            bool res = TestIntExpanded(refMin, refMax, testVal, NEATALU::ASINH_ERROR);
            EXPECT_TRUE(res);
        }

        // 3.b test vector of interval values
        testVec = NEATALU::asinh<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            if(!testVec[i].IsAcc())
            {
                sT refMin = RefALU::asinh(sT(*x[i].GetMin<TypeP>()));
                sT refMax = RefALU::asinh(sT(*x[i].GetMax<TypeP>()));
                bool res = TestIntExpanded(refMin, refMax, testVec[i], NEATALU::ASINH_ERROR);
                EXPECT_TRUE(res);
            }
        }
    }
}

TYPED_TEST(NEATMathTestOneArg, acosh)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type sT;

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    EXPECT_TRUE(TestUnknown(NEATALU::acosh<TypeP>));
    EXPECT_TRUE(TestUnwritten(NEATALU::acosh<TypeP>));
    EXPECT_TRUE(TestAny(NEATALU::acosh<TypeP>));

    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    testVal = NEATALU::sinh<TypeP>(nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    for(uint32_t i = 0; i< this->NUM_TESTS * this->vectorWidth; i++)
    {
        /// Tests non-NaNs as input.
        /// Output should be NaN for x < 1. Other
        NEATValue notNaN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::acosh<TypeP>(notNaN);
        if(this->Arg1Min[i] < (TypeP)1.0)
            EXPECT_TRUE(testVal.IsNaN<TypeP>());
        else
            EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }
    /// C99 ISO/IEC 9899 TC2
    /// acosh(1.0) = +0.0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::acosh<TypeP>,
        1.0, (TypeP)0.0));
    /// acosh(+inf) = +inf
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::acosh<TypeP>,
        Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));

    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; testIdx++)
    {
        NEATVector x(Width);
        // Test accurate values
        for(uint32_t i = 0; i<this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx * this->vectorWidth + i]);
        }

        // 2.a Test single accurate value
        NEATValue xVal = x[0];

        // Determine edge case inputs for which interval shouldn't be expanded
        const uint32_t NUM_EDGE_VALS = 1;
        sT edgeVals[NUM_EDGE_VALS] = { 0.0 };

        testVal = NEATALU::acosh<TypeP>(xVal);
        sT refAccValFloat;

        sT arg = sT(*xVal.GetAcc<TypeP>());
        refAccValFloat = RefALU::acosh<sT>((sT)arg);
        EXPECT_TRUE(TestNeatAcc<sT>(xVal, testVal, refAccValFloat,
            NEATALU::ACOSH_ERROR, edgeVals, NUM_EDGE_VALS));

        // 2.b Test vector of accurate values
        NEATVector testVec = NEATALU::acosh<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::acosh(sT(*x[i].GetAcc<TypeP>()));
            EXPECT_TRUE(TestNeatAcc<sT>(x[i], testVec[i], refAccValFloat,
                NEATALU::ACOSH_ERROR, edgeVals, NUM_EDGE_VALS));
        }

        // 3. Test interval input
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // 3.a single interval testing
        xVal = x[0];

        testVal = NEATALU::acosh<TypeP>(xVal);
        /// Arg1Min == Arg1Max has already been tested
        if(!xVal.IsAcc())
        {
            bool passed = TestAcoshInterval(*xVal.GetMin<TypeP>(),
                *xVal.GetMax<TypeP>(), testVal, NEATALU::ACOSH_ERROR);
            EXPECT_TRUE(passed);
        }

        // 3.b test vector of interval values
        testVec = NEATALU::acosh<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            if(!testVec[i].IsAcc())
            {
                bool passed = TestAcoshInterval(*x[i].GetMin<TypeP>(), 
                    *x[i].GetMax<TypeP>(), testVec[i], NEATALU::ACOSH_ERROR);
                EXPECT_TRUE(passed);
            }
        }
    }
}

TYPED_TEST(NEATMathTestOneArg, atanh)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type sT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const uint32_t NUM_TESTS = 500;
    const uint32_t vectorWidth = 8;
    VectorWidth Width = VectorWidthWrapper::ValueOf(vectorWidth);

    TypeP argMin[NUM_TESTS*vectorWidth];
    TypeP argMax[NUM_TESTS*vectorWidth];


    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &argMin[0], Width,
        NUM_TESTS/2);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &argMax[0], Width,
        NUM_TESTS/2);
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &argMin[NUM_TESTS * vectorWidth/2],
        Width, NUM_TESTS/2, TypeP(-1.0), TypeP(1.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &argMax[NUM_TESTS * vectorWidth/2],
        Width, NUM_TESTS/2, TypeP(-1.0), TypeP(1.0));

    // Make random data aligned with the names: Arg1Min must be <= Arg1Max
    for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
    {
        if (argMin[i] > argMax[i]) std::swap(argMin[i], argMax[i]);
    }

    EXPECT_TRUE(TestUnknown(NEATALU::atanh<TypeP>));
    EXPECT_TRUE(TestUnwritten(NEATALU::atanh<TypeP>));
    EXPECT_TRUE(TestAny(NEATALU::atanh<TypeP>));

    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    testVal = NEATALU::sinh<TypeP>(nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    for(uint32_t i = NUM_TESTS * vectorWidth / 2; i< NUM_TESTS * vectorWidth; i++)
    {
        /// Tests not-NaNs as input.
        /// Values in the interval 
        NEATValue notNaN = NEATValue(argMin[i]);
        testVal = NEATALU::atanh<TypeP>(notNaN);
        EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }
    /// C99 ISO/IEC 9899 TC2
    /// atanh(-1.0) = -inf
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::atanh<TypeP>,
        -1.0, Utils::GetNInf<TypeP>()));
    /// atanh(1.0) = +inf
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::atanh<TypeP>,
        1.0, Utils::GetPInf<TypeP>()));
    /// atanh(+-0) = +- 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::atanh<TypeP>,
        (TypeP)0.0, (TypeP)0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::atanh<TypeP>,
        (TypeP)-0.0, (TypeP)-0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::atanh<TypeP>,
        Utils::GetPInf<TypeP>(), Utils::GetNaN<TypeP>()));

    for(uint32_t testIdx = 0; testIdx < NUM_TESTS; testIdx++)
    {
        NEATVector x(Width);
        // Test accurate values
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            x[i] = NEATValue(argMin[testIdx * vectorWidth + i]);
        }

        // 2.a Test single accurate value
        NEATValue xVal = x[0];

        // Determine edge case inputs for which interval shouldn't be expanded
        const uint32_t NUM_EDGE_VALS = 4;
        sT edgeVals[NUM_EDGE_VALS] = { (sT)1.0, (sT)-1.0 , (sT)0.0, (sT)-0.0};

        testVal = NEATALU::atanh<TypeP>(xVal);
        sT refAccValFloat;

        sT arg = sT(*xVal.GetAcc<TypeP>());
        refAccValFloat = RefALU::atanh<sT>((sT)arg);
        EXPECT_TRUE(TestNeatAcc<sT>(xVal, testVal, refAccValFloat,
            NEATALU::ATANH_ERROR, edgeVals, NUM_EDGE_VALS));

        // 2.b Test vector of accurate values
        NEATVector testVec = NEATALU::atanh<TypeP>(x);
        for(uint32_t i = 0; i < vectorWidth; ++i) {
            refAccValFloat = RefALU::atanh(sT(*x[i].GetAcc<TypeP>()));
            EXPECT_TRUE(TestNeatAcc<sT>(x[i], testVec[i], refAccValFloat,
                NEATALU::ATANH_ERROR, edgeVals, NUM_EDGE_VALS));
        }

        // 3. Test interval input
        for(uint32_t i = 0; i < vectorWidth; ++i)
        {
            x[i] = NEATValue(argMin[testIdx*vectorWidth+i], argMax[testIdx*vectorWidth+i]);
        }

        // 3.a single interval testing
        xVal = x[0];

        testVal = NEATALU::atanh<TypeP>(xVal);
        /// Arg1Min == Arg1Max has already been tested
        if(!xVal.IsAcc())
        {
            bool passed = TestAtanhInterval(*xVal.GetMin<TypeP>(),
                *xVal.GetMax<TypeP>(), testVal, NEATALU::ATANH_ERROR);
            EXPECT_TRUE(passed);
        }

        // 3.b test vector of interval values
        testVec = NEATALU::atanh<TypeP>(x);
        for(uint32_t i = 0; i < vectorWidth; ++i)
        {
            if(!testVec[i].IsAcc())
            {
                bool passed = TestAtanhInterval(*x[i].GetMin<TypeP>(), 
                    *x[i].GetMax<TypeP>(), testVec[i], NEATALU::ATANH_ERROR);
                EXPECT_TRUE(passed);
            }
        }
    }
}

template<typename T>
void TestClampInterval(const NEATValue& a, const NEATValue& in_min, const NEATValue& in_max, const NEATValue& in_testVal)
{
    if(*in_min.GetMax<T>() > *in_max.GetMin<T>())
    {
        EXPECT_TRUE(in_testVal.IsAny());
    } else
    {
        static const uint32_t TESTS_COUNT = 2;
        T aStep = (*a.GetMax<T>() - *a.GetMin<T>()) / (T)TESTS_COUNT;
        T minStep = (*in_min.GetMax<T>() - *in_min.GetMin<T>()) / (T)TESTS_COUNT;
        T maxStep = (*in_max.GetMax<T>() - *in_max.GetMin<T>()) / (T)TESTS_COUNT;
        typedef typename superT<T>::type sT;
        sT refAccValFloat;
        for(T aVal = *a.GetMin<T>(); aVal < *a.GetMax<T>(); aVal += aStep)
        {
            for(T minVal = *in_min.GetMin<T>(); minVal < *in_min.GetMax<T>(); minVal += minStep)
            {
                for(T maxVal = *in_max.GetMin<T>(); maxVal < *in_max.GetMax<T>(); maxVal += maxStep)
                {
                    refAccValFloat = RefALU::clamp(sT(aVal), sT(minVal), sT(maxVal));
                    EXPECT_TRUE(in_testVal.Includes<T>((T)refAccValFloat))<<"  aVal = "<<aVal<< " ; minVal =  " << minVal<< " ; maxVal =  " << maxVal<<";  refAcc =  "
                        <<refAccValFloat<<"; min = "<<*in_testVal.GetMin<T>()<<"; max = "<<*in_testVal.GetMax<T>();
                    if(maxStep == 0.0)
                        break;
                }
                if(minStep == 0.0)
                    break;
            }
            if(aStep == 0.0)
                break;
        }
    }
}


TYPED_TEST(NEATMathTestThreeArgs, clamp)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type sT;

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeP>(NEATALU::clamp<TypeP>));
    EXPECT_TRUE(TestUnwritten<TypeP>(NEATALU::clamp<TypeP>));
    EXPECT_TRUE(TestAny<TypeP>(NEATALU::clamp<TypeP>));

    // Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue notSpecial = NEATValue(this->Arg1Min[0]);
    NEATValue testVal;

    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        NEATValue notNaN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::clamp<TypeP>(notNaN, notNaN, notNaN);
        EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }

    /// Test nan cases
    testVal = NEATALU::clamp<TypeP>(nan, notSpecial, notSpecial);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::clamp<TypeP>(notSpecial, nan, notSpecial);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::clamp<TypeP>(notSpecial, notSpecial, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::clamp<TypeP>(notSpecial, nan, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::clamp<TypeP>(nan, notSpecial, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::clamp<TypeP>(nan, nan, notSpecial);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::clamp<TypeP>(nan, nan, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    /// Now test 15 unique cases for this function
    const uint32_t CASES_COUNT = 15;// [1]  [2]  [3]  [4]  [5]  [6]  [7]  [8]  [9]  [10] [11] [12] [13] [14] [15]
    const float minLow[CASES_COUNT] = {3.0, 2.0, 2.0, 2.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    const float minUpp[CASES_COUNT] = {4.0, 4.0, 3.0, 3.0, 3.0, 4.0, 3.0, 3.0, 3.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0};
    const float maxLow[CASES_COUNT] = {5.0, 5.0, 5.0, 4.0, 4.0, 5.0, 5.0, 4.0, 4.0, 5.0, 4.0, 4.0, 3.0, 3.0, 3.0};
    const float maxUpp[CASES_COUNT] = {6.0, 6.0, 6.0, 6.0, 5.0, 6.0, 6.0, 6.0, 5.0, 6.0, 6.0, 5.0, 6.0, 5.0, 4.0};
    const float xLow[CASES_COUNT]   = {1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 5.0};
    const float xUpp[CASES_COUNT]   = {2.0, 3.0, 4.0, 5.0, 6.0, 3.0, 4.0, 5.0, 6.0, 4.0, 5.0, 6.0, 5.0, 6.0, 6.0};
    /// results were precalculated by hand
    const float clampLow[CASES_COUNT] = {minLow[0], minLow[1], minLow[2], minLow[3], minLow[4],   xLow[5], xLow[6], xLow[7],
                    xLow[8], xLow[9], xLow[10],   xLow[11], maxLow[12], maxLow[13], maxLow[14]};
    const float clampUpp[CASES_COUNT] = {minUpp[0], minUpp[1],   xUpp[2],   xUpp[3], maxUpp[4], minUpp[5], xUpp[6], xUpp[7],
                  maxUpp[8], xUpp[9], xUpp[10], maxUpp[11],   xUpp[12], maxUpp[13], maxUpp[14]};
    for(uint32_t i = 0; i<CASES_COUNT; i++)
    {
        NEATValue min = NEATValue(minLow[i], minUpp[i]);
        NEATValue max = NEATValue(maxLow[i], maxUpp[i]);
        NEATValue x = NEATValue(xLow[i], xUpp[i]);
        NEATValue res = NEATALU::clamp<TypeP>(x, min, max);
        EXPECT_TRUE(res.IsInterval());
        EXPECT_EQ(*res.GetMin<TypeP>(), clampLow[i]);
        EXPECT_EQ(*res.GetMax<TypeP>(), clampUpp[i]);
    }

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    // No C99 restrictions for
    // Done with corner cases. Now go to testing scalar and vector functions
    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // Test on accurate values.
        NEATVector minVec(Width);
        NEATVector maxVec(Width);
        NEATVector x(Width);

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            minVec[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i]);
            maxVec[i] = NEATValue(this->Arg3Min[testIdx*this->vectorWidth+i]);
        }

        /* test for single accurate NEAT value*/
        NEATValue xVal = x[0];
        NEATValue minVal = minVec[0];
        NEATValue maxVal = maxVec[0];

        testVal = NEATALU::clamp<TypeP>(xVal, minVal, maxVal);
        typedef typename superT<TypeP>::type sT;
        sT refAccValFloat;

        if(*minVal.GetAcc<TypeP>() > *maxVal.GetAcc<TypeP>())
            EXPECT_TRUE(testVal.IsAny());
        else
        {
            refAccValFloat = RefALU::clamp(sT(*xVal.GetAcc<TypeP>()), sT(*minVal.GetAcc<TypeP>()), sT(*maxVal.GetAcc<TypeP>()));
            EXPECT_TRUE(testVal.IsAcc());
            EXPECT_EQ(*testVal.GetAcc<TypeP>(), (TypeP)refAccValFloat);
        }

        /* test for vector of NEAT accurate */
        NEATVector testVec = NEATALU::clamp<TypeP>(x, minVec, maxVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            if(*minVec[i].GetAcc<TypeP>() > *maxVec[i].GetAcc<TypeP>())
                EXPECT_TRUE(testVec[i].IsAny());
            else
            {
                refAccValFloat = RefALU::clamp(sT(*x[i].GetAcc<TypeP>()), sT(*minVec[i].GetAcc<TypeP>()), sT(*maxVec[i].GetAcc<TypeP>()));
                EXPECT_TRUE(testVec[i].IsAcc());
                EXPECT_EQ((TypeP)refAccValFloat, *testVec[i].GetAcc<TypeP>());
            }
        }

        /// Test function that works with x as a vector and min/max as scalar values
        testVec = NEATALU::clamp<TypeP>(x, minVec[0], maxVec[0]);
        if(*minVec[0].GetAcc<TypeP>() > *maxVec[0].GetAcc<TypeP>())
        {
            for(uint32_t i = 0; i<this->vectorWidth; ++i)
                EXPECT_TRUE(testVec[i].IsAny());
        } else
        {
            for(uint32_t i = 0; i<this->vectorWidth; ++i)
            {
                refAccValFloat = RefALU::clamp(sT(*x[i].GetAcc<TypeP>()), sT(*minVec[0].GetAcc<TypeP>()), sT(*maxVec[0].GetAcc<TypeP>()));
                EXPECT_TRUE(testVec[i].IsAcc());
                EXPECT_EQ((TypeP)refAccValFloat, *testVec[i].GetAcc<TypeP>());
            }
        }

        // Test on interval values.
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
            minVec[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i], this->Arg2Max[testIdx*this->vectorWidth+i]);
            maxVec[i] = NEATValue(this->Arg3Min[testIdx*this->vectorWidth+i], this->Arg3Max[testIdx*this->vectorWidth+i]);
        }

        // test for single interval NEAT value
        xVal = x[0];
        minVal = minVec[0];
        maxVal = maxVec[0];

        testVal = NEATALU::clamp<TypeP>(xVal, minVal, maxVal);

        TestClampInterval<TypeP>(xVal, minVal, maxVal, testVal);

        // test for vector of NEAT intervals
        testVec = NEATALU::clamp<TypeP>(x, minVec, maxVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            TestClampInterval<TypeP>(x[i], minVec[i], maxVec[i], testVec[i]);
        }
    }
}


template <typename T>
class NEATExpTest {
public:
    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 500;
    // Vector data type length.
    static const uint32_t vectorWidth = 8;
    // Intervals with test values for common functions arguments.
    T Arg1Min[NUM_TESTS*vectorWidth];
    T Arg1Max[NUM_TESTS*vectorWidth];

    // Parameters for random data generator.
    VectorWidth currWidth;
    DataTypeVal dataTypeVal;

    typedef typename superT<T>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    NEATExpTest()
    {
        currWidth = VectorWidthWrapper::ValueOf(this->vectorWidth);
        dataTypeVal = GetDataTypeVal<T>();

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], currWidth, NUM_TESTS);

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
        }
    }

    void TestPreciseExp(NEATFuncP NEATFunc)
    // for all exp functions, except expm1
    {
        // 1.c Test edge cases described in C99 TC2 and OpenCL 1.1 specifications
        // C99 ISO/IEC 9899 TC2
        // exp(+inf) = +inf
        EXPECT_TRUE(TestPreciseRes<T>(NEATFunc, Utils::GetPInf<T>(), Utils::GetPInf<T>()));
        // exp(-inf) = +0
        EXPECT_TRUE(TestPreciseRes<T>(NEATFunc, Utils::GetNInf<T>(), T(0.0)));
        // exp(+-0) = 1
        EXPECT_TRUE(TestPreciseRes<T>(NEATFunc, T(+0.0), T(1.0)));
        EXPECT_TRUE(TestPreciseRes<T>(NEATFunc, T(-0.0), T(1.0)));
    }

    void TestExp(NEATFuncP NEATFunc, NEATFuncVecP NEATFuncVec, RefFuncP RefFunc, float ulps)
    {
        // 1.a Test special statuses
        // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
        EXPECT_TRUE(TestUnknown(NEATFunc));
        EXPECT_TRUE(TestUnwritten(NEATFunc));
        EXPECT_TRUE(TestAny(NEATFunc));

        // 1.b Test special floating point values: NaNs.
        NEATValue nan = NEATValue::NaN<T>();
        NEATValue testVal;
        // Test NaN as input
        testVal = NEATFunc(nan);
        EXPECT_TRUE(testVal.IsNaN<T>());
        for (uint32_t i = 0; i < (NUM_TESTS)*(vectorWidth); ++i)
        {
            // Test NaN value as input.
            // For binary or unary function you can include test that includes all combinations
            // of NaNs and non-NaN values
            NEATValue notNAN = NEATValue(Arg1Min[i]);
            testVal = NEATFunc(notNAN);
            EXPECT_FALSE(testVal.IsNaN<T>());
        }

        VectorWidth Width = VectorWidthWrapper::ValueOf(vectorWidth);

        // Done with corner cases. Now go to testing scalar and vector functions for randomized values
        for(uint32_t testIdx = 0; testIdx < NUM_TESTS; ++testIdx)
        {
            // 2. Test on accurate values.
            NEATVector x(Width);
            for(uint32_t i = 0; i < vectorWidth; ++i)
            {
                x[i] = NEATValue(Arg1Min[testIdx*vectorWidth+i]);
            }

            // 2.a Test single accurate value
            NEATValue xVal = x[0];

            // Determine edge case inputs for which interval shouldn't be expanded
            const uint32_t NUM_EDGE_VALS = 2;
            sT edgeVals[NUM_EDGE_VALS] = { (sT)0.0, (sT)-0.0 };

            testVal = NEATFunc(xVal);
            sT refAccValFloat;

            refAccValFloat = RefFunc(sT(*xVal.GetAcc<T>()));
            EXPECT_TRUE(TestNeatAcc<sT>(xVal, testVal, refAccValFloat, ulps, edgeVals, NUM_EDGE_VALS));

            // 2.b Test vector of accurate values
            NEATVector testVec = NEATFuncVec(x);
            for(uint32_t i = 0; i < vectorWidth; ++i) {
                refAccValFloat = RefFunc(sT(*x[i].GetAcc<T>()));
                EXPECT_TRUE(TestNeatAcc<sT>(x[i], testVec[i], refAccValFloat, ulps, edgeVals, NUM_EDGE_VALS));
            }

            // 3. Test interval input
            for(uint32_t i = 0; i < vectorWidth; ++i)
            {
                x[i] = NEATValue(Arg1Min[testIdx*vectorWidth+i], Arg1Max[testIdx*vectorWidth+i]);
            }

            // 3.a single interval testing
            sT refMin, refMax;
            xVal = x[0];

            testVal = NEATFunc(xVal);
            /// Arg1Min == Arg1Max has already been tested
            if(!xVal.IsAcc())
            {
                refMin = RefFunc(sT(*xVal.GetMax<T>()));
                refMax = RefFunc(sT(*xVal.GetMin<T>()));
                bool passed = TestIntExpanded(refMin, refMax, testVal, ulps);
                EXPECT_TRUE(passed);
            }

            // 3.b test vector of interval values

            testVec = NEATFuncVec(x);
            for(uint32_t i = 0; i < vectorWidth; ++i)
            {
                if(!testVec[i].IsAcc())
                {
                    refMax = RefFunc(sT(*x[i].GetMax<T>()));
                    refMin = RefFunc(sT(*x[i].GetMin<T>()));
                    bool passed = TestIntExpanded(refMin, refMax, testVec[i], ulps);
                    EXPECT_TRUE(passed);
                }
            }
        }
    } // void TestExp(NEATFuncP NEATFunc, NEATFuncVecP NEATFuncVec, RefFuncP RefFunc, float ulps)

};


template <typename T>
class NEATExpTestRun : public ALUTest {
};


TYPED_TEST_CASE(NEATExpTestRun, FloatTypesCommon);
TYPED_TEST(NEATExpTestRun, exp)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::exp<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::exp<TypeP>;
    RefFuncP RefFunc = &RefALU::exp<sT>;

    NEATExpTest<TypeP> expTest;

    expTest.TestPreciseExp(NEATFunc);
    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::EXP_ERROR));
}

TYPED_TEST(NEATExpTestRun, native_exp)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::native_exp<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::native_exp<TypeP>;
    RefFuncP RefFunc = &RefALU::exp<sT>;

    NEATExpTest<TypeP> expTest;

    expTest.TestPreciseExp(NEATFunc);
    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::NATIVE_EXP_ERROR));
}

TYPED_TEST(NEATExpTestRun, half_exp)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::half_exp<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::half_exp<TypeP>;
    RefFuncP RefFunc = &RefALU::exp<sT>;

    NEATExpTest<TypeP> expTest;
    expTest.TestPreciseExp(NEATFunc);
    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::HALF_EXP_ERROR));
}

TYPED_TEST(NEATExpTestRun, exp2)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::exp2<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::exp2<TypeP>;
    RefFuncP RefFunc = &RefALU::exp2<sT>;

    NEATExpTest<TypeP> expTest;
    expTest.TestPreciseExp(NEATFunc);
    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::EXP2_ERROR));
}

TYPED_TEST(NEATExpTestRun, native_exp2)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::native_exp2<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::native_exp2<TypeP>;
    RefFuncP RefFunc = &RefALU::exp2<sT>;

    NEATExpTest<TypeP> expTest;
    expTest.TestPreciseExp(NEATFunc);
    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::NATIVE_EXP2_ERROR));
}

TYPED_TEST(NEATExpTestRun, half_exp2)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::half_exp2<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::half_exp2<TypeP>;
    RefFuncP RefFunc = &RefALU::exp2<sT>;

    NEATExpTest<TypeP> expTest;
    expTest.TestPreciseExp(NEATFunc);
    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::HALF_EXP2_ERROR));
}

TYPED_TEST(NEATExpTestRun, exp10)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::exp10<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::exp10<TypeP>;
    RefFuncP RefFunc = &RefALU::exp10<sT>;

    NEATExpTest<TypeP> expTest;
    expTest.TestPreciseExp(NEATFunc);
    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::EXP10_ERROR));
}

TYPED_TEST(NEATExpTestRun, native_exp10)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::native_exp10<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::native_exp10<TypeP>;
    RefFuncP RefFunc = &RefALU::exp10<sT>;

    NEATExpTest<TypeP> expTest;
    expTest.TestPreciseExp(NEATFunc);
    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::NATIVE_EXP10_ERROR));
}

TYPED_TEST(NEATExpTestRun, half_exp10)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::half_exp10<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::half_exp10<TypeP>;
    RefFuncP RefFunc = &RefALU::exp10<sT>;

    NEATExpTest<TypeP> expTest;
    expTest.TestPreciseExp(NEATFunc);
    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::HALF_EXP10_ERROR));
}

TYPED_TEST(NEATExpTestRun, expm1)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::expm1<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::expm1<TypeP>;
    RefFuncP RefFunc = &RefALU::expm1<sT>;

    NEATExpTest<TypeP> expTest;

    // expm1 has the same edge cases for argument as other exp functions,
    // but returns different result values for these argument values

    // 1.c Test edge cases described in C99 TC2 and OpenCL 1.1 specifications
    // C99 ISO/IEC 9899 TC2
    // expm1(+inf) = +inf
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATFunc, Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));
    // expm1(-inf) = -1
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATFunc, Utils::GetNInf<TypeP>(), TypeP(-1.0)));
    // expm1(+-0) = +-0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATFunc, TypeP (+0.0), TypeP (+0.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATFunc, TypeP (-0.0), TypeP (-0.0)));

    expTest.TestExp(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::EXPM1_ERROR));
}

template <typename T>
class NEATLogTest {
public:
    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 500;
    // Vector data type length.
    static const uint32_t vectorWidth = 8;
    // Intervals with test values for common functions arguments.
    T Arg1Min[NUM_TESTS*vectorWidth];
    T Arg1Max[NUM_TESTS*vectorWidth];

    // Parameters for random data generator.
    VectorWidth currWidth;
    DataTypeVal dataTypeVal;

    NEATLogTest()
    {
        currWidth = VectorWidthWrapper::ValueOf(this->vectorWidth);
        dataTypeVal = GetDataTypeVal<T>();

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], currWidth, NUM_TESTS);

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
        }
    }

    typedef typename superT<T>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    void TestPreciseLog(NEATFuncP NEATFunc)
    // for all log functions, except log1p and logb
    {
        // C99 ISO/IEC 9899 TC2
        // log(+-0) = -inf
        EXPECT_TRUE(TestPreciseRes<T>(NEATFunc, (T)0.0, Utils::GetNInf<T>()));
        EXPECT_TRUE(TestPreciseRes<T>(NEATFunc, (T)-0.0, Utils::GetNInf<T>()));
        // log(1.0) = +0.0
        EXPECT_TRUE(TestPreciseRes<T>(NEATFunc, T(1.0), T(0.0)));
        // log(+inf) = +inf
        EXPECT_TRUE(TestPreciseRes<T>(NEATFunc, Utils::GetPInf<T>(), Utils::GetPInf<T>()));
    }

    void TestLog(NEATFuncP NEATFunc, NEATFuncVecP NEATFuncVec, RefFuncP RefFunc, float ulps, T edgeConst)
    {
        // -1.0 is for log1p, 0.0 is for the rest of logs
        EXPECT_TRUE((edgeConst == T(0.0)) || (edgeConst == T(-1.0)));

        // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
        EXPECT_TRUE(TestUnknown(NEATFunc));
        EXPECT_TRUE(TestUnwritten(NEATFunc));
        EXPECT_TRUE(TestAny(NEATFunc));

        // Test special floating point values: NaNs.
        NEATValue nan = NEATValue::NaN<T>();
        NEATValue testVal;
        testVal = NEATFunc(nan);
        EXPECT_TRUE(testVal.IsNaN<T>());
        for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
        {
            NEATValue notNAN = NEATValue(Arg1Min[i]);
            /// C99 - log returns NaN for x < 0
            /// C99 - log1p returns NaN for x < -1
            testVal = NEATFunc(notNAN);
                if(Arg1Min[i] < edgeConst)
                EXPECT_TRUE(testVal.IsNaN<T>());
            else
                EXPECT_FALSE(testVal.IsNaN<T>());
        }

        VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

        // Done with corner cases. Now go to testing scalar and vector functions
        for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
        {
            // Test on accurate values.
            NEATVector x(Width);

            for(uint32_t i = 0; i < this->vectorWidth; ++i)
            {
                x[i] = NEATValue(Arg1Min[testIdx*this->vectorWidth+i]);
            }

            /* test for single accurate NEAT value*/
            NEATValue xVal = x[0];

            const uint32_t NUM_EDGE_VALS = 3;
            sT edgeVals[NUM_EDGE_VALS] = { (sT)0.0, (sT)-0.0, (sT)1.0 };

            // if log1p function is tested, -1.0 is an edge val instead of 1.0
            if(edgeConst == T(-1.0)) {
                edgeVals[2] = (sT)(-1.0);
            }

            testVal = NEATFunc(xVal);
            sT refAccValFloat;

            refAccValFloat = RefFunc(sT(*xVal.GetAcc<T>()));
            EXPECT_TRUE(TestNeatAcc<sT>(xVal, testVal, refAccValFloat, ulps, edgeVals, NUM_EDGE_VALS));

            /* test for vector of NEAT accurate */
            NEATVector testVec = NEATFuncVec(x);
            for(uint32_t i = 0; i < this->vectorWidth; ++i) {
                refAccValFloat = RefFunc(sT(*x[i].GetAcc<T>()));
                EXPECT_TRUE(TestNeatAcc<sT>(x[i], testVec[i], refAccValFloat, ulps, edgeVals, NUM_EDGE_VALS));
            }

            // Test on interval values.
            for(uint32_t i = 0; i < this->vectorWidth; ++i)
            {
                x[i] = NEATValue(Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
            }

            // Test for single interval NEAT value */
            sT refMin, refMax;

            xVal = x[0];

            testVal = NEATFunc(xVal);

            /// Arg1Min == Arg1Max has already been tested
            if(!xVal.IsAcc())
            {
                refMax = RefFunc(sT(*xVal.GetMax<T>()));
                refMin = RefFunc(sT(*xVal.GetMin<T>()));
                if(*xVal.GetMin<T>() >= edgeConst)
                {
                    bool passed = TestIntExpanded(refMin, refMax, testVal, ulps);
                    EXPECT_TRUE(passed);
                }
            else
            {
                if(*xVal.GetMax<T>() < edgeConst)
                {
                    EXPECT_TRUE(testVal.IsAcc());
                    EXPECT_TRUE(testVal.IsNaN<T>());
                }
                else
                    EXPECT_TRUE(testVal.IsUnknown());
            }
            }

            testVec = NEATFuncVec(x);
            for(uint32_t i = 0; i < this->vectorWidth; ++i)
            {
                if(!testVec[i].IsAcc())
                {
                    refMax = RefFunc(sT(*x[i].GetMax<T>()));
                    refMin = RefFunc(sT(*x[i].GetMin<T>()));
                    if(*x[i].GetMin<T>() >= edgeConst)
                    {
                        bool passed = TestIntExpanded(refMin, refMax, testVec[i], ulps);
                        EXPECT_TRUE(passed);
                    }
                    else
                    {
                        if(*x[i].GetMax<T>() < edgeConst)
                        {
                            EXPECT_TRUE(testVec[i].IsAcc());
                            EXPECT_TRUE(testVec[i].IsNaN<T>());
                        }
                    else
                        EXPECT_TRUE(testVec[i].IsUnknown());
                    }
                }
            }
        }
    } // void TestLog(NEATFuncP NEATFunc, NEATFuncVecP NEATFuncVec, RefFuncP RefFunc, float ulps)

};

template <typename T>
class NEATLogTestRun : public ALUTest {
};
TYPED_TEST_CASE(NEATLogTestRun, FloatTypesCommon);

TYPED_TEST(NEATLogTestRun, log)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::log<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::log<TypeP>;
    RefFuncP RefFunc = &RefALU::log<sT>;

    NEATLogTest<TypeP> logTest;
    logTest.TestPreciseLog(NEATFunc);
    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::LOG_ERROR), TypeP(0.0));
}

TYPED_TEST(NEATLogTestRun, native_log)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::native_log<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::native_log<TypeP>;
    RefFuncP RefFunc = &RefALU::log<sT>;

    NEATLogTest<TypeP> logTest;
    logTest.TestPreciseLog(NEATFunc);
    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::NATIVE_LOG_ERROR), TypeP(0.0));
}

TYPED_TEST(NEATLogTestRun, half_log)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::half_log<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::half_log<TypeP>;
    RefFuncP RefFunc = &RefALU::log<sT>;

    NEATLogTest<TypeP> logTest;
    logTest.TestPreciseLog(NEATFunc);
    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::HALF_LOG_ERROR), TypeP(0.0));
}

TYPED_TEST(NEATLogTestRun, log2)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::log2<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::log2<TypeP>;
    RefFuncP RefFunc = &RefALU::log2<sT>;

    NEATLogTest<TypeP> logTest;
    logTest.TestPreciseLog(NEATFunc);
    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::LOG2_ERROR), TypeP(0.0));
}

TYPED_TEST(NEATLogTestRun, native_log2)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::native_log2<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::native_log2<TypeP>;
    RefFuncP RefFunc = &RefALU::log2<sT>;

    NEATLogTest<TypeP> logTest;
    logTest.TestPreciseLog(NEATFunc);
    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::NATIVE_LOG2_ERROR), TypeP(0.0));
}

TYPED_TEST(NEATLogTestRun, half_log2)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::half_log2<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::half_log2<TypeP>;
    RefFuncP RefFunc = &RefALU::log2<sT>;

    NEATLogTest<TypeP> logTest;
    logTest.TestPreciseLog(NEATFunc);
    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::HALF_LOG2_ERROR), TypeP(0.0));
}

TYPED_TEST(NEATLogTestRun, log10)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::log10<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::log10<TypeP>;
    RefFuncP RefFunc = &RefALU::log10<sT>;

    NEATLogTest<TypeP> logTest;
    logTest.TestPreciseLog(NEATFunc);
    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::LOG10_ERROR), TypeP(0.0));
}

TYPED_TEST(NEATLogTestRun, native_log10)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::native_log10<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::native_log10<TypeP>;
    RefFuncP RefFunc = &RefALU::log10<sT>;

    NEATLogTest<TypeP> logTest;
    logTest.TestPreciseLog(NEATFunc);
    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::NATIVE_LOG10_ERROR), TypeP(0.0));
}

TYPED_TEST(NEATLogTestRun, half_log10)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::half_log10<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::half_log10<TypeP>;
    RefFuncP RefFunc = &RefALU::log10<sT>;

    NEATLogTest<TypeP> logTest;
    logTest.TestPreciseLog(NEATFunc);
    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::HALF_LOG10_ERROR), TypeP(0.0));
}

TYPED_TEST(NEATLogTestRun, log1p)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::log1p<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::log1p<TypeP>;
    RefFuncP RefFunc = &RefALU::log1p<sT>;

    NEATLogTest<TypeP> logTest;
    // C99 ISO/IEC 9899 TC2
    // log1p(+-0) = +-0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATFunc, (TypeP)0.0, (TypeP)0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATFunc, (TypeP)-0.0, (TypeP)-0.0));
    // log1p(-1.0) = +0.0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATFunc, TypeP(-1.0), Utils::GetNInf<TypeP>()));
    // log(+inf) = +inf
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATFunc, Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));

    logTest.TestLog(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::LOG1P_ERROR), TypeP(-1.0));
}

TYPED_TEST_CASE(NEATLogbTest, FloatTypesCommon);

TYPED_TEST(NEATMathTestOneArg, logb)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    // 1.a Test special statuses
    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown(NEATALU::logb<TypeP>));
    EXPECT_TRUE(TestUnwritten(NEATALU::logb<TypeP>));
    EXPECT_TRUE(TestAny(NEATALU::logb<TypeP>));

    // 1.b Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    // Test NaN as input
    testVal = NEATALU::logb<TypeP>(nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        // Test NaN value as input.
        // For binary or unary function you can include test that includes all combinations
        // of NaNs and non-NaN values
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::logb<TypeP>(notNAN);
        EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }

    // 1.c Test edge cases described in C99 TC2 and OpenCL 1.1 specifications
    // C99 ISO/IEC 9899 TC2
    // logb(+-inf) = +inf
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::logb<TypeP>, Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::logb<TypeP>, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>()));
    // logb(+-0) = -inf
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::logb<TypeP>, TypeP (+0.0), Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::logb<TypeP>, TypeP (-0.0), Utils::GetNInf<TypeP>()));

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    // Done with corner cases. Now go to testing scalar and vector functions for randomized values
    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // 2. Test on accurate values.
        NEATVector x(Width);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
        }

        // 2.a Test single accurate value
        NEATValue xVal = x[0];

        // Determine edge case inputs for which interval shouldn't be expanded
        const uint32_t NUM_EDGE_VALS = 2;
        sT edgeVals[NUM_EDGE_VALS] = { (sT)0.0, (sT)-0.0 };

        testVal = NEATALU::logb<TypeP>(xVal);
        sT refAccValFloat;

        refAccValFloat = RefALU::logb(sT(*xVal.GetAcc<TypeP>()));
        EXPECT_TRUE(TestNeatAcc<sT>(xVal, testVal, refAccValFloat, NEATALU::LOGB_ERROR, edgeVals, NUM_EDGE_VALS));

        // 2.b Test vector of accurate values
        NEATVector testVec = NEATALU::logb<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::logb(sT(*x[i].GetAcc<TypeP>()));
            EXPECT_TRUE(TestNeatAcc<sT>(x[i], testVec[i], refAccValFloat, NEATALU::LOGB_ERROR, edgeVals, NUM_EDGE_VALS));
        }

        // 3. Test interval input
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // 3.a single interval testing
        sT refMin, refMax;
        xVal = x[0];

        testVal = NEATALU::logb<TypeP>(xVal);
        /// Arg1Min == Arg1Max has already been tested
        if(!xVal.IsAcc())
        {
            refMin = RefALU::logb(sT(*xVal.GetMax<TypeP>()));
            refMax = RefALU::logb(sT(*xVal.GetMin<TypeP>()));
            bool passed = TestIntExpanded(refMin, refMax, testVal, NEATALU::LOGB_ERROR);
            EXPECT_TRUE(passed);
        }

        // 3.b test vector of interval values

        testVec = NEATALU::logb<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            if(!testVec[i].IsAcc())
            {
                refMax = RefALU::logb(sT(*x[i].GetMax<TypeP>()));
                refMin = RefALU::logb(sT(*x[i].GetMin<TypeP>()));
                bool passed = TestIntExpanded(refMin, refMax, testVec[i], NEATALU::LOGB_ERROR);
                EXPECT_TRUE(passed);
            }
        }
    }
}

TYPED_TEST(NEATMathTestOneArg, cbrt)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    // 1.a Test special statuses
    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown(NEATALU::cbrt<TypeP>));
    EXPECT_TRUE(TestUnwritten(NEATALU::cbrt<TypeP>));
    EXPECT_TRUE(TestAny(NEATALU::cbrt<TypeP>));

    // 1.b Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    // Test NaN as input
    testVal = NEATALU::cbrt<TypeP>(nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        // Test NaN value as input.
        // For binary or unary function you can include test that includes all combinations
        // of NaNs and non-NaN values
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::cbrt<TypeP>(notNAN);
        EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // 2. Test on accurate values.
        NEATVector x(Width);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
        }

        // 2.a Test single accurate value
        NEATValue xVal = x[0];

        testVal = NEATALU::cbrt<TypeP>(xVal);
        sT refAccValFloat;

        refAccValFloat = RefALU::cbrt(sT(*xVal.GetAcc<TypeP>()));
        EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat, testVal, NEATALU::CBRT_ERROR));

        // 2.b Test vector of accurate values
        NEATVector testVec = NEATALU::cbrt<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::cbrt(sT(*x[i].GetAcc<TypeP>()));
            EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat, testVec[i], NEATALU::CBRT_ERROR));
        }

        // 3. Test interval input
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // 3.a single interval testing
        sT refMin, refMax;
        xVal = x[0];

        testVal = NEATALU::cbrt<TypeP>(xVal);
        /// Arg1Min == Arg1Max has already been tested
        if(!xVal.IsAcc())
        {
            refMin = RefALU::cbrt(sT(*xVal.GetMax<TypeP>()));
            refMax = RefALU::cbrt(sT(*xVal.GetMin<TypeP>()));
            bool passed = TestIntExpanded(refMin, refMax, testVal, NEATALU::CBRT_ERROR);
            EXPECT_TRUE(passed);
        }

        // 3.b test vector of interval values

        testVec = NEATALU::cbrt<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            if(!testVec[i].IsAcc())
            {
                refMax = RefALU::cbrt(sT(*x[i].GetMax<TypeP>()));
                refMin = RefALU::cbrt(sT(*x[i].GetMin<TypeP>()));
                bool passed = TestIntExpanded(refMin, refMax, testVec[i], NEATALU::CBRT_ERROR);
                EXPECT_TRUE(passed);
            }
        }
    }
}

template<typename T>
void TestPowInterval(const NEATValue& x, const NEATValue& y, const NEATValue& in_testVal)
{
    if((!in_testVal.IsNaN<T>()) && (!in_testVal.IsUnknown()))
    {
        static const uint32_t TESTS_COUNT = 2;
        T xStep = (*x.GetMax<T>() - *x.GetMin<T>()) / (T)TESTS_COUNT;
        T yStep = (*y.GetMax<T>() - *y.GetMin<T>()) / (T)TESTS_COUNT;
        typedef typename superT<T>::type sT;
        sT refAccVal;
        for(T xVal = *x.GetMin<T>(); xVal < *x.GetMax<T>(); xVal += xStep)
        {
            for(T yVal = *y.GetMin<T>(); yVal < *y.GetMax<T>(); yVal += yStep)
            {
                refAccVal = RefALU::flush((T)RefALU::pow(sT(xVal), sT(yVal)));
                EXPECT_TRUE(in_testVal.Includes<T>((T)refAccVal))<<"  xVal = "<<xVal<< " ; yVal =  " << yVal<<";  refAcc =  "
                        <<refAccVal<<"; min = "<<*in_testVal.GetMin<T>()<<"; max = "<<*in_testVal.GetMax<T>();
                    if(!in_testVal.Includes<T>((T)refAccVal))
                        std::cout<<";";
                if(yStep == 0.0)
                    break;
            }
            if(xStep == 0.0)
                break;
        }
    }
}

TYPED_TEST(NEATMathTestTwoArgs, pow)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;

    if (SkipDoubleTest<TypeP>())
        return;

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeP>(NEATALU::pow<TypeP>));
    EXPECT_TRUE(TestUnwritten<TypeP>(NEATALU::pow<TypeP>));
    EXPECT_TRUE(TestAny<TypeP>(NEATALU::pow<TypeP>));
    /// Test C99 precise special cases:
    // [C99] 1
    // pow(+-0, y) = +- infinity, y odd integer < 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)+0.0, TypeP(-3.0), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.0, TypeP(-3.0), Utils::GetNInf<TypeP>()));
    // [C99] 2
    // pow(+-0, y) = +infinity, y not an odd integer < 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)+0.0, TypeP(-2.0), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.0, TypeP(-2.0), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)+0.0, TypeP(-2.1), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.0, TypeP(-2.1), Utils::GetPInf<TypeP>()));
    // [C99] 3
    // pow(+-0, y) = +-0.0, y an odd integer > 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)+0.0, TypeP(3.0), TypeP(0.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.0, TypeP(3.0), TypeP(-0.0)));
    // [C99] 4
    // pow(+-0, y) = +0.0, y not an odd integer > 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)+0.0, TypeP(2.0), TypeP(0.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.0, TypeP(2.0), TypeP(0.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)+0.0, TypeP(2.1), TypeP(0.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.0, TypeP(2.1), TypeP(0.0)));
    // [C99] 5
    // pow(-1, +-inf) = 1
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-1.0, Utils::GetNInf<TypeP>(), TypeP(1.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-1.0, Utils::GetPInf<TypeP>(), TypeP(1.0)));
    // [C99] 6
    // pow(1.0, y) = 1.0 for any y, even a NaN
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)1.0, Utils::GetNaN<TypeP>(), TypeP(1.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)1.0, TypeP(2.0), TypeP(1.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)1.0, TypeP(-2.0), TypeP(1.0)));
    // [C99] 7
    // pow(x, +-0.0) = 1.0 for any x, even a NaN
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNaN<TypeP>(), (TypeP) 0.0, TypeP(1.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNaN<TypeP>(), (TypeP)-0.0, TypeP(1.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)2.0, (TypeP) 0.0, TypeP(1.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-2.0, (TypeP)-0.0, TypeP(1.0)));
    // [C99] 8
    // pow(x, y) = NaN finite (x<0) and finite non-integer y
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-2.0, (TypeP) 1.1, Utils::GetNaN<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-2.0, (TypeP)-1.1, Utils::GetNaN<TypeP>()));
    // [C99] 9
    // pow(x, -infinity) = +infinity for |x| < 1
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.5, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP) 0.5, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.0, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP) 0.0, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>()));
    // [C99] 10
    // pow(x, -infinity) = +0 for |x| > 1
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-1.5, Utils::GetNInf<TypeP>(), (TypeP)+0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP) 1.5, Utils::GetNInf<TypeP>(), (TypeP)+0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNInf<TypeP>(), Utils::GetNInf<TypeP>(), (TypeP)+0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), Utils::GetNInf<TypeP>(), (TypeP)+0.0));
    // [C99] 11
    // pow(x, +infinity) = +0 for |x| < 1
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.5, Utils::GetPInf<TypeP>(), (TypeP)+0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP) 0.5, Utils::GetPInf<TypeP>(), (TypeP)+0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-0.0, Utils::GetPInf<TypeP>(), (TypeP)+0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP) 0.0, Utils::GetPInf<TypeP>(), (TypeP)+0.0));
    // [C99] 12
    // pow(x, +infinity) = +infinity for |x| > 1
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP)-1.5, Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, (TypeP) 1.5, Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>(), Utils::GetPInf<TypeP>()));
    // [C99] 13
    // pow(-infinity, y) = -0 for y an odd integer < 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNInf<TypeP>(), (TypeP)-3.0, (TypeP)-0.0));
    // [C99] 14
    // pow(-infinity, y) = +0 for y not an odd integer < 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNInf<TypeP>(), (TypeP)-2.0, (TypeP)+0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNInf<TypeP>(), (TypeP)-1.1, (TypeP)+0.0));
    // [C99] 15
    // pow(-infinity, y) = -infinity for y an odd integer > 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNInf<TypeP>(), (TypeP)3.0, Utils::GetNInf<TypeP>()));
    // [C99] 16
    // pow(-infinity, y) = +infinity for y not an odd integer > 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNInf<TypeP>(), (TypeP)2.0, Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetNInf<TypeP>(), (TypeP)1.1, Utils::GetPInf<TypeP>()));
    // [C99] 17
    // pow(+infinity, y < 0) = +0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), (TypeP)-3.0, (TypeP)0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), (TypeP)-2.0, (TypeP)0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), (TypeP)-1.1, (TypeP)0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), Utils::GetNInf<TypeP>(), (TypeP)+0.0));
    // [C99] 18
    // pow(+infinity, y > 0) = +infinity
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), (TypeP)3.0, Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), (TypeP)2.0, Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), (TypeP)1.1, Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::pow<TypeP>, Utils::GetPInf<TypeP>(), Utils::GetNInf<TypeP>(), (TypeP)+0.0));
    // Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    // Ordinary not a special value
    NEATValue notNaN = NEATValue(TypeP(1.1));
    NEATValue testVal;
    testVal = NEATALU::pow<TypeP>(notNaN, notNaN);
    EXPECT_FALSE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::pow<TypeP>(notNaN, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::pow<TypeP>(nan, notNaN);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::pow<TypeP>(nan, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    // Use 2
    // interval with odd integer inside
    NEATValue intervalOdd(TypeP(-3.5), TypeP(-2.5));
    NEATValue intervalZero(TypeP(-0.1), TypeP(0.1));
    NEATValue intervalOne(TypeP(0.9), TypeP(1.1));
    NEATValue ans = NEATALU::pow<TypeP>(intervalZero, intervalOne);
    EXPECT_EQ(ans.GetStatus(), NEATValue::UNKNOWN);
    // interval without integers inside
    NEATValue zero(TypeP(0.0));
    NEATValue nZero(TypeP(-0.0));
    NEATValue one(TypeP(1.0));
    NEATValue nOne(TypeP(-1.0));
    NEATValue pInf(Utils::GetPInf<TypeP>());
    NEATValue nInf(Utils::GetNInf<TypeP>());
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        NEATValue x(this->Arg1Min[i], this->Arg1Max[i]);
        NEATValue y(this->Arg2Min[i], this->Arg2Max[i]);
        testVal = NEATALU::pow<TypeP>(x, y);
        TestPowInterval<TypeP>(x, y, testVal);
    }
    /// test case for exponent close to 0.9 .. 1.1 values to cover
    TypeP ExpMin[this->NUM_TESTS*this->vectorWidth];
    TypeP ExpMax[this->NUM_TESTS*this->vectorWidth];
    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &ExpMin[0], Width, this->NUM_TESTS, (TypeP)0.9, (TypeP)1.1);
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &ExpMax[0], Width, this->NUM_TESTS, (TypeP)0.9, (TypeP)1.1);
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        if(ExpMin[i] > ExpMax[i])
            std::swap(ExpMin[i], ExpMax[i]);
        NEATValue x(this->Arg1Min[i], this->Arg1Max[i]);
        NEATValue y(ExpMin[i], ExpMax[i]);
        testVal = NEATALU::pow<TypeP>(x, y);
        TestPowInterval<TypeP>(x, y, testVal);
    }

    /// Regression tests
    NEATValue xFailing((TypeP)25.144262, (TypeP)25.763393);
    NEATValue yFailing((TypeP)-31.303106, (TypeP)-30.683975);
    testVal = NEATALU::pow<TypeP>(xFailing, yFailing);
    TestPowInterval<TypeP>(xFailing, yFailing, testVal);
}

template<typename T>
void TestPowrInterval(const NEATValue& x, const NEATValue& y, const NEATValue& in_testVal)
{
    if((!in_testVal.IsNaN<T>()) && (!in_testVal.IsUnknown()))
    {
        static const uint32_t TESTS_COUNT = 2;
        T xStep = (*x.GetMax<T>() - *x.GetMin<T>()) / (T)TESTS_COUNT;
        T yStep = (*y.GetMax<T>() - *y.GetMin<T>()) / (T)TESTS_COUNT;
        typedef typename superT<T>::type sT;
        sT refAccVal;
        for(T xVal = *x.GetMin<T>(); xVal < *x.GetMax<T>(); xVal += xStep)
        {
            for(T yVal = *y.GetMin<T>(); yVal < *y.GetMax<T>(); yVal += yStep)
            {
                refAccVal = RefALU::flush((T)RefALU::powr(sT(xVal), sT(yVal)));
                EXPECT_TRUE(in_testVal.Includes<T>((T)refAccVal))<<"  xVal = "<<xVal<< " ; yVal =  " << yVal<<";  refAcc =  "
                        <<refAccVal<<"; min = "<<*in_testVal.GetMin<T>()<<"; max = "<<*in_testVal.GetMax<T>();
                    if(!in_testVal.Includes<T>((T)refAccVal))
                        std::cout<<";";
                if(yStep == 0.0)
                    break;
            }
            if(xStep == 0.0)
                break;
        }
    }
}

TYPED_TEST(NEATMathTestTwoArgs, powr)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;

    if (SkipDoubleTest<TypeP>())
        return;

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeP>(NEATALU::powr<TypeP>));
    EXPECT_TRUE(TestUnwritten<TypeP>(NEATALU::powr<TypeP>));
    EXPECT_TRUE(TestAny<TypeP>(NEATALU::powr<TypeP>));

    /// Test Additional Requirements Beyond C99 TC2:

    // powr(x, +-0.0) = 1.0 for finite x > 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::powr<TypeP>, (TypeP)2.0, (TypeP) 0.0, TypeP(1.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::powr<TypeP>, (TypeP)2.0, (TypeP) -0.0, TypeP(1.0)));
    // powr ( ±0, y ) is +Inf for finite y < 0.
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::powr<TypeP>, (TypeP)+0.0, TypeP(-3.0), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::powr<TypeP>, (TypeP)-0.0, TypeP(-3.0), Utils::GetPInf<TypeP>()));
    // powr ( ±0, -Inf) is +Inf.
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::powr<TypeP>, (TypeP)+0.0, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::powr<TypeP>, (TypeP)-0.0, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>()));
    // powr ( ±0, y ) is +0 for y > 0.
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::powr<TypeP>, (TypeP)+0.0, TypeP(3.0), (TypeP) 0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::powr<TypeP>, (TypeP)-0.0, TypeP(3.0), (TypeP) 0.0));
    // powr ( +1, y ) is 1 for finite y.
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::powr<TypeP>, (TypeP)+1.0, TypeP(3.0), (TypeP) +1.0));


    NEATValue testVal;
    NEATValue zero(TypeP(0.0));
    NEATValue nZero(TypeP(-0.0));
    NEATValue one(TypeP(1.0));
    NEATValue nOne(TypeP(-1.0));
    NEATValue pInf(Utils::GetPInf<TypeP>());
    NEATValue nInf(Utils::GetNInf<TypeP>());
    NEATValue accVal(TypeP(3.5));
    NEATValue nAccVal(TypeP(-3.5));
    // powr ( x, y ) returns NaN for x < 0.
    testVal = NEATALU::powr<TypeP>(nAccVal, accVal);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // powr ( ±0, ±0 ) returns NaN.
    testVal = NEATALU::powr<TypeP>(nZero, nZero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::powr<TypeP>(zero, zero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::powr<TypeP>(nZero, zero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::powr<TypeP>(zero, nZero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // powr ( +Inf, ±0 ) returns NaN.
    testVal = NEATALU::powr<TypeP>(pInf, zero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::powr<TypeP>(pInf, nZero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // powr ( +1, ±Inf ) returns NaN.
    testVal = NEATALU::powr<TypeP>(one, pInf);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::powr<TypeP>(one, nInf);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // powr ( x, NaN ) returns the NaN for x >= 0.
    NEATValue nan = NEATValue::NaN<TypeP>();
    testVal = NEATALU::powr<TypeP>(accVal, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // powr ( NaN, y ) returns the NaN.
    testVal = NEATALU::powr<TypeP>(nan,nOne);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::powr<TypeP>(nan,one);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    // Test special floating point values: NaNs.

    // Use 2
    // interval with odd integer inside
    NEATValue intervalOdd(TypeP(-3.5), TypeP(-2.5));
    NEATValue intervalZero(TypeP(-0.1), TypeP(0.1));
    NEATValue intervalOne(TypeP(0.9), TypeP(1.1));
    NEATValue ans = NEATALU::powr<TypeP>(intervalZero, intervalOne);
    EXPECT_EQ(ans.GetStatus(), NEATValue::UNKNOWN);
    // interval without integers inside


    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        NEATValue x(this->Arg1Min[i], this->Arg1Max[i]);
        NEATValue y(this->Arg2Min[i], this->Arg2Max[i]);
        testVal = NEATALU::powr<TypeP>(x, y);
        TestPowrInterval<TypeP>(x, y, testVal);
    }
    /// test case for exponent close to 0.9 .. 1.1 values to cover
    TypeP ExpMin[this->NUM_TESTS*this->vectorWidth];
    TypeP ExpMax[this->NUM_TESTS*this->vectorWidth];
    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &ExpMin[0], Width, this->NUM_TESTS, (TypeP)0.9, (TypeP)1.1);
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &ExpMax[0], Width, this->NUM_TESTS, (TypeP)0.9, (TypeP)1.1);
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        if(ExpMin[i] > ExpMax[i])
            std::swap(ExpMin[i], ExpMax[i]);
        NEATValue x(this->Arg1Min[i], this->Arg1Max[i]);
        NEATValue y(ExpMin[i], ExpMax[i]);
        testVal = NEATALU::powr<TypeP>(x, y);
        TestPowrInterval<TypeP>(x, y, testVal);
    }
}


TYPED_TEST(NEATMathTestTwoArgs, half_powr)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;

    if (SkipDoubleTest<TypeP>())
        return;

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeP>(NEATALU::half_powr<TypeP>));
    EXPECT_TRUE(TestUnwritten<TypeP>(NEATALU::half_powr<TypeP>));
    EXPECT_TRUE(TestAny<TypeP>(NEATALU::half_powr<TypeP>));

    /// Test Additional Requirements Beyond C99 TC2:

    // half_powr(x, +-0.0) = 1.0 for finite x > 0
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::half_powr<TypeP>, (TypeP)2.0, (TypeP) 0.0, TypeP(1.0)));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::half_powr<TypeP>, (TypeP)2.0, (TypeP) -0.0, TypeP(1.0)));
    // half_powr ( ±0, y ) is +Inf for finite y < 0.
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::half_powr<TypeP>, (TypeP)+0.0, TypeP(-3.0), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::half_powr<TypeP>, (TypeP)-0.0, TypeP(-3.0), Utils::GetPInf<TypeP>()));
    // half_powr ( ±0, -Inf) is +Inf.
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::half_powr<TypeP>, (TypeP)+0.0, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::half_powr<TypeP>, (TypeP)-0.0, Utils::GetNInf<TypeP>(), Utils::GetPInf<TypeP>()));
    // half_powr ( ±0, y ) is +0 for y > 0.
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::half_powr<TypeP>, (TypeP)+0.0, TypeP(3.0), (TypeP) 0.0));
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::half_powr<TypeP>, (TypeP)-0.0, TypeP(3.0), (TypeP) 0.0));
    // half_powr ( +1, y ) is 1 for finite y.
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::half_powr<TypeP>, (TypeP)+1.0, TypeP(3.0), (TypeP) +1.0));


    NEATValue testVal;
    NEATValue zero(TypeP(0.0));
    NEATValue nZero(TypeP(-0.0));
    NEATValue one(TypeP(1.0));
    NEATValue nOne(TypeP(-1.0));
    NEATValue pInf(Utils::GetPInf<TypeP>());
    NEATValue nInf(Utils::GetNInf<TypeP>());
    NEATValue accVal(TypeP(3.5));
    NEATValue nAccVal(TypeP(-3.5));
    // half_powr ( x, y ) returns NaN for x < 0.
    testVal = NEATALU::half_powr<TypeP>(nAccVal, accVal);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // half_powr ( ±0, ±0 ) returns NaN.
    testVal = NEATALU::half_powr<TypeP>(nZero, nZero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::half_powr<TypeP>(zero, zero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::half_powr<TypeP>(nZero, zero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::half_powr<TypeP>(zero, nZero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // half_powr ( +Inf, ±0 ) returns NaN.
    testVal = NEATALU::half_powr<TypeP>(pInf, zero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::half_powr<TypeP>(pInf, nZero);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // half_powr ( +1, ±Inf ) returns NaN.
    testVal = NEATALU::half_powr<TypeP>(one, pInf);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::half_powr<TypeP>(one, nInf);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // half_powr ( x, NaN ) returns the NaN for x >= 0.
    NEATValue nan = NEATValue::NaN<TypeP>();
    testVal = NEATALU::half_powr<TypeP>(accVal, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // half_powr ( NaN, y ) returns the NaN.
    testVal = NEATALU::half_powr<TypeP>(nan,nOne);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::half_powr<TypeP>(nan,one);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    // Test special floating point values: NaNs.

    // Use 2
    // interval with odd integer inside
    NEATValue intervalOdd(TypeP(-3.5), TypeP(-2.5));
    NEATValue intervalZero(TypeP(-0.1), TypeP(0.1));
    NEATValue intervalOne(TypeP(0.9), TypeP(1.1));
    NEATValue ans = NEATALU::half_powr<TypeP>(intervalZero, intervalOne);
    EXPECT_EQ(ans.GetStatus(), NEATValue::UNKNOWN);
    // interval without integers inside


    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        NEATValue x(this->Arg1Min[i], this->Arg1Max[i]);
        NEATValue y(this->Arg2Min[i], this->Arg2Max[i]);
        testVal = NEATALU::half_powr<TypeP>(x, y);
        TestPowrInterval<TypeP>(x, y, testVal);
    }
    /// test case for exponent close to 0.9 .. 1.1 values to cover
    TypeP ExpMin[this->NUM_TESTS*this->vectorWidth];
    TypeP ExpMax[this->NUM_TESTS*this->vectorWidth];
    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &ExpMin[0], Width, this->NUM_TESTS, (TypeP)0.9, (TypeP)1.1);
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &ExpMax[0], Width, this->NUM_TESTS, (TypeP)0.9, (TypeP)1.1);
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        if(ExpMin[i] > ExpMax[i])
            std::swap(ExpMin[i], ExpMax[i]);
        NEATValue x(this->Arg1Min[i], this->Arg1Max[i]);
        NEATValue y(ExpMin[i], ExpMax[i]);
        testVal = NEATALU::half_powr<TypeP>(x, y);
        TestPowrInterval<TypeP>(x, y, testVal);
    }
}

TYPED_TEST(NEATMathTestOneArg, fabs)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    TypeP * firstFloat = &this->Arg1Min[0];
    TypeP * secondFloat = &this->Arg1Max[0];

    TypeP firstFloatRanged[this->NUM_TESTS*this->vectorWidth];
    TypeP secondFloatRanged[this->NUM_TESTS*this->vectorWidth];

    VectorWidth curWidth = VectorWidthWrapper::ValueOf(this->vectorWidth);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
        this->NUM_TESTS,TypeP(-100.0),TypeP(+100.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
        this->NUM_TESTS,TypeP(-0.5),TypeP(0.5));

    /* test for specific values */
    // fabs ( ±0 ) returns +0
    NEATValue accVal = NEATValue(TypeP(+0.0));
    NEATValue testAccVal = NEATALU::fabs<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    accVal = NEATValue(TypeP(-0.0));
    testAccVal = NEATALU::fabs<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    // fabs ( ±INF ) returns a +INF
    accVal = NEATValue(Utils::GetPInf<TypeP>());
    testAccVal = NEATALU::fabs<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal, Utils::GetPInf<TypeP>()));

    accVal = NEATValue(Utils::GetNInf<TypeP>());
    testAccVal = NEATALU::fabs<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal, Utils::GetPInf<TypeP>()));

    VectorWidthWrapper wrap(curWidth);
    for(unsigned int testIdx = 0; testIdx < this->NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        // generate intervals with min and max correctly placed
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i],
                secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i],
                firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* generate interval from -100 to 100 */
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i]);
            if(firstFloatRanged[testIdx*wrap.GetSize()+i] < secondFloatRanged[testIdx*wrap.GetSize()+i])
                intervalRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i], secondFloatRanged[testIdx*wrap.GetSize()+i]);
            else
                intervalRanged[i] = NEATValue(secondFloatRanged[testIdx*wrap.GetSize()+i], firstFloatRanged[testIdx*wrap.GetSize()+i]);
        }

        /* test for single accurate NEAT value*/
        accVal = accurate[0];
        testAccVal = NEATALU::fabs<TypeP>(accVal);
        sT refAccValFloat;

        refAccValFloat = RefALU::fabs(sT(*accVal.GetAcc<TypeP>()));

        // for accurate check fabs return accurate
        bool passed = testAccVal.IsAcc();
        passed &= ( TypeP(refAccValFloat) == *(testAccVal.GetAcc<TypeP>()) );
        EXPECT_TRUE(passed);

        /* test for single interval NEAT value */
        sT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::fabs<TypeP>(intVal);

        refIntValMax = RefALU::fabs(sT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::fabs(sT(*intVal.GetMin<TypeP>()));

        // check fabs expands 0 ulps
        passed = TestIntExpanded<sT>(refIntValMin,refIntValMax,testIntVal,0.f);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::fabs<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::fabs(sT(*accurate[i].GetAcc<TypeP>()));
            bool passed = testAccVal.IsAcc();
            passed &= (TypeP(refAccValFloat) == *(testAccVec[i].GetAcc<TypeP>()));
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::fabs<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::fabs(sT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::fabs(sT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<sT>(refIntValMin,refIntValMax,testIntVec[i],0.f);
            EXPECT_TRUE(passed);
        }


        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::fabs<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::fabs(sT(*accurateRanged[i].GetAcc<TypeP>()));
            bool passed = testAccVal.IsAcc();
            passed &= (TypeP(refAccValFloat) == *(testAccVec[i].GetAcc<TypeP>()));
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::fabs<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::fabs(sT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::fabs(sT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<sT>(refIntValMin,refIntValMax,testIntVec[i],0.f);
            EXPECT_TRUE(passed);
        }

        /* test reference ALU produces values within NEAT intervals*/
        testIntVec = NEATALU::fabs<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            TypeP ref_in, ref_out;
            GenerateRangedVectorsAutoSeed(this->dataTypeVal, &ref_in,
                V1, 1,
                *intervalRanged[i].GetMin<TypeP>(),
                *intervalRanged[i].GetMax<TypeP>());
            ref_out = (TypeP) RefALU::fabs(sT( ref_in ));
            EXPECT_TRUE(testIntVec[i].Includes(ref_out) );
        }
    }
}

template <typename T>
class NEATRoundingTest {
public:
    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 500;
    // Vector data type length.
    static const uint32_t vectorWidth = 8;
    // Intervals with test values for common functions arguments.
    T Arg1Min[NUM_TESTS*vectorWidth];
    T Arg1Max[NUM_TESTS*vectorWidth];

    // Parameters for random data generator.
    VectorWidth curWidth;
    DataTypeVal dataTypeVal;

    NEATRoundingTest()
    {
        curWidth = VectorWidthWrapper::ValueOf(this->vectorWidth);
        dataTypeVal = GetDataTypeVal<T>();

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], curWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], curWidth, NUM_TESTS);

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
        }
    }

    typedef typename superT<T>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    void TestRoundAddReq(void) 
    {
        T Arr[vectorWidth*NUM_TESTS];
        GenerateRangedVectorsAutoSeed(dataTypeVal, &Arr[0], curWidth, NUM_TESTS, T(-0.5), T(0));
        for(unsigned int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            NEATValue accVal = NEATValue(Arr[testIdx*vectorWidth]);
            NEATValue testAccVal = NEATALU::round<T>(accVal);
            sT refVal = RefALU::round(sT(Arr[testIdx*vectorWidth]));
            EXPECT_TRUE(TestAccValue<T>(testAccVal,T(refVal)));

            NEATVector accurate(curWidth);
            for(uint32_t i = 0; i<vectorWidth; i++) {
                accurate[i] = NEATValue(Arr[testIdx*vectorWidth+i]);
            }

            NEATVector testAccVec = NEATALU::round<T>(accurate);
            for(uint32_t i = 0; i<vectorWidth; i++) {
                refVal = RefALU::round(sT(*accurate[i].GetAcc<T>()));
                bool passed = testAccVal.IsAcc();
                passed &= (T(refVal) == *(testAccVec[i].GetAcc<T>()));
                EXPECT_TRUE(passed);
            }
        }
    }

    void TestRounding(NEATFuncVecP NEATFuncVec, NEATFuncP NEATFunc, RefFuncP RefFunc)
    // for all rounding functions
    {
        // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
        EXPECT_TRUE(TestUnknown(NEATFunc));
        EXPECT_TRUE(TestUnwritten(NEATFunc));
        EXPECT_TRUE(TestAny(NEATFunc));

        /* test for specific values */
        NEATValue accVal = NEATValue(T(+0.0));
        NEATValue testAccVal = NEATFunc(accVal);
        EXPECT_TRUE(TestAccValue<T>(testAccVal,T(+0.0)));
   
        accVal = NEATValue(T(-0.0));
        testAccVal = NEATFunc(accVal);
        EXPECT_TRUE(TestAccValue<T>(testAccVal,T(-0.0)));
   
        accVal = NEATValue(Utils::GetPInf<T>());
        testAccVal = NEATFunc(accVal);
        EXPECT_TRUE(TestAccValue<T>(testAccVal, Utils::GetPInf<T>()));
   
        accVal = NEATValue(Utils::GetNInf<T>());
        testAccVal = NEATFunc(accVal);
        EXPECT_TRUE(TestAccValue<T>(testAccVal, Utils::GetNInf<T>()));

        for(unsigned int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            NEATValue accVal, intVal;
            NEATValue testAccVal, testIntVal;
            NEATValue refAccVal, refIntVal;
    
            /* load data for interval from min T to max T */
            NEATVector accurate(curWidth);
            NEATVector interval(curWidth);
    
            // generate accurate and interval values
            for(uint32_t i = 0; i<vectorWidth; i++)
            {
                accurate[i] = NEATValue(Arg1Min[testIdx*vectorWidth+i]);
                interval[i] = NEATValue(Arg1Min[testIdx*vectorWidth+i],
                                        Arg1Max[testIdx*vectorWidth+i]);
            }
    
            /* test for single accurate NEAT value*/
            accVal = accurate[0];
            testAccVal = NEATFunc(accVal);
            sT refAccValFloat;
    
            refAccValFloat = RefFunc(sT(*accVal.GetAcc<T>()));
    
            // for accurate check floor return accurate
            bool passed = testAccVal.IsAcc();
            passed &= ( T(refAccValFloat) == *(testAccVal.GetAcc<T>()) );
            EXPECT_TRUE(passed);
    
            /* test for single interval NEAT value */
            sT refIntValMin, refIntValMax;
    
            intVal = interval[0];
            testIntVal = NEATFunc(intVal);
    
            refIntValMax = RefFunc(sT(*intVal.GetMax<T>()));
            refIntValMin = RefFunc(sT(*intVal.GetMin<T>()));
            if(refIntValMin == refIntValMax)
            {
                passed = testIntVal.IsAcc() && (*testIntVal.GetAcc<T>() == refIntValMin);
            }
            else passed = testIntVal.IsUnknown();
            EXPECT_TRUE(passed);
    
            /* test for vector of NEAT accurate */
            NEATVector testAccVec = NEATFuncVec(accurate);
            for(uint32_t i = 0; i<vectorWidth; i++) {
                refAccValFloat = RefFunc(sT(*accurate[i].GetAcc<T>()));
                bool passed = testAccVal.IsAcc();
                passed &= (T(refAccValFloat) == *(testAccVec[i].GetAcc<T>()));
                EXPECT_TRUE(passed);
            }
    
            /* test for vector of NEAT intervals */
            NEATVector testIntVec = NEATFuncVec(interval);
            for(uint32_t i = 0; i<vectorWidth; i++) {
                refIntValMin = RefFunc(sT(*interval[i].GetMin<T>()));
                refIntValMax = RefFunc(sT(*interval[i].GetMax<T>()));
                if(refIntValMin == refIntValMax)
                {
                    passed = testIntVec[i].IsAcc() && (*testIntVec[i].GetAcc<T>() == refIntValMin);
                }
                else passed = testIntVec[i].IsUnknown();
                EXPECT_TRUE(passed);
            }
        }
    }
};

template <typename T>
class NEATRoundingTestRun : public ALUTest {
};
TYPED_TEST_CASE(NEATRoundingTestRun, FloatTypesCommon);

TYPED_TEST(NEATRoundingTestRun, floor)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::floor<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::floor<TypeP>;
    RefFuncP RefFunc = &RefALU::floor<sT>;

    NEATRoundingTest<TypeP> floorTest;
    floorTest.TestRounding(NEATFuncVec, NEATFunc, RefFunc);
}

TYPED_TEST(NEATRoundingTestRun, rint)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::rint<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::rint<TypeP>;
    RefFuncP RefFunc = &RefALU::rint<sT>;

    NEATRoundingTest<TypeP> rintTest;
    rintTest.TestRounding(NEATFuncVec, NEATFunc, RefFunc);
}

TYPED_TEST(NEATRoundingTestRun, round)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::round<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::round<TypeP>;
    RefFuncP RefFunc = &RefALU::round<sT>;

    NEATRoundingTest<TypeP> roundTest;
    roundTest.TestRounding(NEATFuncVec, NEATFunc, RefFunc);
    // test additional requirement for round function only
    roundTest.TestRoundAddReq();
}

TYPED_TEST(NEATRoundingTestRun, trunc)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::trunc<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::trunc<TypeP>;
    RefFuncP RefFunc = &RefALU::trunc<sT>;

    NEATRoundingTest<TypeP> truncTest;
    truncTest.TestRounding(NEATFuncVec, NEATFunc, RefFunc);
}

// hypot test
TYPED_TEST(NEATMathTestTwoArgs, hypot)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;
    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    /* test for specific values */
    NEATValue accValx, accValy;
    NEATValue testAccVal;

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeP>(NEATALU::hypot<TypeP>));
    EXPECT_TRUE(TestUnwritten<TypeP>(NEATALU::hypot<TypeP>));
    EXPECT_TRUE(TestAny<TypeP>(NEATALU::hypot<TypeP>));

    // hypot ( ±0 ) returns ±0
    const TypeP Pz = +0.0;
    const TypeP Nz = -0.0;
    const NEATValue pzero = NEATValue(Pz);
    const NEATValue nzero = NEATValue(Nz);
    testAccVal = NEATALU::hypot<TypeP>(pzero, pzero);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,Pz));
    testAccVal = NEATALU::hypot<TypeP>(pzero, nzero);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,Pz));
    testAccVal = NEATALU::hypot<TypeP>(nzero, nzero);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,Pz));

    // hypot( ±INF ) returns a +INF
    const TypeP pi = Utils::GetPInf<TypeP>();
    const TypeP ni = Utils::GetNInf<TypeP>();
    const NEATValue pinf = NEATValue(pi);
    const NEATValue ninf = NEATValue(ni);
    const NEATValue numr = NEATValue(TypeP(48.0));
    testAccVal = NEATALU::hypot<TypeP>(pinf, pinf);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,pi));
    testAccVal = NEATALU::hypot<TypeP>(pinf, ninf);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,pi));
    testAccVal = NEATALU::hypot<TypeP>(ninf, ninf);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,pi));
    testAccVal = NEATALU::hypot<TypeP>(numr, pinf);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,pi));
    testAccVal = NEATALU::hypot<TypeP>(numr, pinf);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,pi));

    // hypot( NaN, num ) returns a Nan
    NEATValue nan = NEATValue::NaN<TypeP>();
    testAccVal = NEATALU::hypot<TypeP>(NEATValue(TypeP(-48.0)), nan);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
    testAccVal = NEATALU::hypot<TypeP>(nan, NEATValue(TypeP(0.4)));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    TypeP xmin, xmax, ymin, ymax;
    NEATValue nx, ny;
    NEATValue nres;

    // all positive boundaries
    xmin = 2.0; xmax = 3.0; ymin = 45.0; ymax = 47.0;
    nx.SetIntervalVal<TypeP>(xmin, xmax);
    ny.SetIntervalVal<TypeP>(ymin, ymax);
    nres = NEATALU::hypot<TypeP>(nx, ny);
    EXPECT_TRUE(
        TestIntExpanded<SuperT>(RefALU::hypot<SuperT>(xmin, ymin),
        RefALU::hypot<SuperT>(xmax, ymax), nres, 4.0f));

    // one interval contains zero
    xmin = -2.0; xmax = 3.0; ymin = 45.0; ymax = 47.0;
    nx.SetIntervalVal<TypeP>(xmin, xmax);
    ny.SetIntervalVal<TypeP>(ymin, ymax);
    nres = NEATALU::hypot<TypeP>(nx, ny);
    EXPECT_TRUE(
        TestIntExpanded<SuperT>(RefALU::hypot<SuperT>(0.0, ymin),
        RefALU::hypot<SuperT>(xmax, ymax), nres, 4.0f));

    // both intervals contain zero
    xmin = -200.0; xmax = 3.0; ymin = -45.0; ymax = 47.0;
    nx.SetIntervalVal<TypeP>(xmin, xmax);
    ny.SetIntervalVal<TypeP>(ymin, ymax);
    nres = NEATALU::hypot<TypeP>(nx, ny);
    EXPECT_TRUE(
        TestIntExpanded<SuperT>(0.0,
        RefALU::hypot<SuperT>(xmin, ymax), nres, 4.0f));

    // all positive boundaries with positive inf
    xmin = 2.0; xmax = 3.0; ymin = 45.0; ymax = pi;
    nx.SetIntervalVal<TypeP>(xmin, xmax);
    ny.SetIntervalVal<TypeP>(ymin, ymax);
    nres = NEATALU::hypot<TypeP>(nx, ny);
    EXPECT_TRUE(
        TestIntExpanded<SuperT>(RefALU::hypot<SuperT>(xmin, ymin),
        pi, nres, 4.0f));

    // boundaries with negative inf and crossing zero
    xmin = 2.0; xmax = 3.0; ymin = ni; ymax = 45;
    nx.SetIntervalVal<TypeP>(xmin, xmax);
    ny.SetIntervalVal<TypeP>(ymin, ymax);
    nres = NEATALU::hypot<TypeP>(nx, ny);
    EXPECT_TRUE(
        TestIntExpanded<SuperT>(RefALU::hypot<SuperT>(xmin, 0.0),
        pi, nres, 4.0f));

    // regression test
    xmin = -7.0; xmax = -1.0; ymin = -3.0; ymax = 1;
    nx.SetIntervalVal<TypeP>(xmin, xmax);
    ny.SetIntervalVal<TypeP>(ymin, ymax);
    nres = NEATALU::hypot<TypeP>(nx, ny);
    EXPECT_TRUE(
        TestIntExpanded<SuperT>(
        RefALU::hypot<SuperT>(xmax, 0.0),
        RefALU::hypot<SuperT>(xmin, ymin),
        nres, 4.0f));


    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        const int32_t vw = VectorWidthWrapper(this->currWidth).GetSize();

        // Test on accurate values.
        NEATVector x(this->currWidth);
        NEATVector y(this->currWidth);
        NEATVector xv(this->currWidth);
        NEATVector yv(this->currWidth);

        for(int32_t i = 0; i < vw; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*vw+i]);
            y[i] = NEATValue(this->Arg2Min[testIdx*vw+i]);
            xv[i] = NEATValue(this->Arg1Min[testIdx*vw+i], this->Arg1Max[testIdx*vw+i]);
            yv[i] = NEATValue(this->Arg2Min[testIdx*vw+i], this->Arg2Max[testIdx*vw+i]);
        }

        /* test for single accurate NEAT value*/
        NEATValue xVal = x[0];
        NEATValue yVal = y[0];

        NEATValue testVal = NEATALU::hypot<TypeP>(xVal, yVal);
        SuperT refAccValFloat;

        refAccValFloat = RefALU::hypot(
            SuperT(*xVal.GetAcc<TypeP>()),
            SuperT(*yVal.GetAcc<TypeP>()));

        EXPECT_TRUE(TestAccExpanded<SuperT>(refAccValFloat, testVal, 4.0f) );

        /* test for vector of NEAT accurate */
        NEATVector testVec = NEATALU::hypot<TypeP>(x, y);
        for(int32_t i = 0; i < vw; ++i) {
            TypeP refValFloat = RefALU::hypot(
                SuperT(*x[i].GetAcc<TypeP>()),
                SuperT(*y[i].GetAcc<TypeP>()));
            EXPECT_TRUE(
                TestAccExpanded<SuperT>(refValFloat, testVec[i], 4.0f));
        }

        /* test reference ALU produces values within NEAT intervals*/
        testVec = NEATALU::hypot<TypeP>(xv, yv);
        for(int32_t i = 0; i < vw; ++i) {
            TypeP ref_in1, ref_in2, ref_out;
            GenerateRangedVectorsAutoSeed(this->dataTypeVal, &ref_in1,
                V1, 1,
                *xv[i].GetMin<TypeP>(),
                *xv[i].GetMax<TypeP>());
            GenerateRangedVectorsAutoSeed(this->dataTypeVal, &ref_in2,
                V1, 1,
                *yv[i].GetMin<TypeP>(),
                *yv[i].GetMax<TypeP>());

            ref_out = (TypeP) RefALU::hypot( SuperT( ref_in1 ), SuperT(ref_in2));
            EXPECT_TRUE(testVec[i].Includes<TypeP>(ref_out));
        }
    }
}


TYPED_TEST(NEATMathTestOneArg, frexp)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

// exp value is not checked, because we don't support NEAT for integers
    int testExp, refExpAcc,refExpMin, refExpMax;
    std::vector<int> expVec;

    // 1.a Test special statuses
    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    NEATValue testAccVal = NEATALU::frexp<TypeP>(NEATValue(NEATValue::ANY),&testExp);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::frexp<TypeP>(NEATValue(NEATValue::UNWRITTEN),&testExp);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::frexp<TypeP>(NEATValue(NEATValue::UNKNOWN),&testExp);
    EXPECT_TRUE(testAccVal.IsUnknown());

    // 1.b Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    // Test NaN as input
    testExp = 1;
    testVal = NEATALU::frexp<TypeP>(nan,&testExp);
    // frexp ( NaN, exp ) returns the NaN
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    // and stores 0 in exp
    EXPECT_TRUE(testExp == 0);

    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        // Test NaN value as input.
        // For binary or unary function you can include test that includes all combinations
        // of NaNs and non-NaN values
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::frexp<TypeP>(notNAN,&testExp);
        EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }


    // 1.c Test edge cases described in C99 TC2 and OpenCL 1.1 specifications
    // C99 ISO/IEC 9899 TC2
    // frexp(+-inf) = +inf and stores 0 in exp
    testExp = 1;
    testAccVal = NEATALU::frexp<TypeP>(NEATValue(Utils::GetPInf<TypeP>()),&testExp);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testExp == 0);

    testExp = 1;
    testAccVal = NEATALU::frexp<TypeP>(NEATValue(Utils::GetNInf<TypeP>()),&testExp);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testExp == 0);

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    // Done with corner cases. Now go to testing scalar and vector functions for randomized values
    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // 2. Test on accurate values.
        NEATVector x(Width);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
        }

        // 2.a Test single accurate value
        NEATValue xVal = x[0];

        // Determine edge case inputs for which interval shouldn't be expanded
        const uint32_t NUM_EDGE_VALS = 2;
        sT edgeVals[NUM_EDGE_VALS] = { (sT)0.0, (sT)-0.0 };

        testVal = NEATALU::frexp<TypeP>(xVal,&testExp);
        sT refAccValFloat;

        refAccValFloat = RefALU::frexp(sT(*xVal.GetAcc<TypeP>()),&refExpAcc);
        TestNeatAcc<sT>(xVal, testVal, refAccValFloat, NEATALU::FREXP_ERROR, edgeVals, NUM_EDGE_VALS);

        // 2.b Test vector of accurate values
        NEATVector testVec = NEATALU::frexp<TypeP>(x,expVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::frexp(sT(*x[i].GetAcc<TypeP>()),&testExp);
            TestNeatAcc<sT>(x[i], testVec[i], refAccValFloat, NEATALU::FREXP_ERROR, edgeVals, NUM_EDGE_VALS);
        }

        // 3. Test interval input
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // 3.a single interval testing
        sT refMin, refMax;
        xVal = x[0];

        testVal = NEATALU::frexp<TypeP>(xVal,&testExp);
        /// Arg1Min == Arg1Max has already been tested
        if(!xVal.IsAcc())
        {
            refMax = RefALU::frexp(sT(*xVal.GetMax<TypeP>()),&refExpMax);
            refMin = RefALU::frexp(sT(*xVal.GetMin<TypeP>()),&refExpMin);
            if(refExpMax != refExpMin)
                EXPECT_TRUE(testVal.IsUnknown());
            else
                EXPECT_TRUE(TestIntExpanded(refMin, refMax, testVal, NEATALU::FREXP_ERROR));
        }

        // 3.b test vector of interval values

        testVec = NEATALU::frexp<TypeP>(x,expVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            if(!testVec[i].IsAcc())
            {
                refMax = RefALU::frexp(sT(*x[i].GetMax<TypeP>()),&refExpMax);
                refMin = RefALU::frexp(sT(*x[i].GetMin<TypeP>()),&refExpMin);
                if(refExpMax != refExpMin)
                    EXPECT_TRUE(testVec[i].IsUnknown());
                else
                    EXPECT_TRUE(TestIntExpanded(refMin, refMax, testVec[i], NEATALU::FREXP_ERROR));
            }
        }
    }
}


TYPED_TEST(NEATMathTestOneArg, ldexp)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    int32_t Arg2[this->NUM_TESTS*this->vectorWidth];

    // Fill up argument values with random data
    GenerateRandomVectorsAutoSeed(I32, &Arg2[0], this->currWidth, this->NUM_TESTS);

    int powN = Arg2[0];

    // 1.a Test special statuses
    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    NEATValue testAccVal = NEATALU::ldexp<TypeP>(NEATValue(NEATValue::ANY),powN);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::ldexp<TypeP>(NEATValue(NEATValue::UNWRITTEN),powN);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::ldexp<TypeP>(NEATValue(NEATValue::UNKNOWN),powN);
    EXPECT_TRUE(testAccVal.IsUnknown());

    // 1.b Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    testVal = NEATALU::ldexp<TypeP>(nan,powN);
    // ldexp ( NaN, exp ) returns the NaN
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        // Test NaN value as input.
        // For binary or unary function you can include test that includes all combinations
        // of NaNs and non-NaN values
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        int powN = Arg2[i];
        testVal = NEATALU::ldexp<TypeP>(notNAN,powN);
        EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    // Done with corner cases. Now go to testing scalar and vector functions for randomized values
    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // 2. Test on accurate values.
        NEATVector x(Width);
        std::vector<int> powVec(this->vectorWidth);

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            powVec[i] = Arg2[testIdx*this->vectorWidth+i];
        }

        // 2.a Test single accurate value
        NEATValue xVal = x[0];
        powN = powVec[0];

        testVal = NEATALU::ldexp<TypeP>(xVal,powN);
        sT refAccValFloat;

        refAccValFloat = RefALU::ldexp(sT(*xVal.GetAcc<TypeP>()),powN);

        EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat, testVal, NEATALU::LDEXP_ERROR));

        // 2.b Test vector of accurate values, single value of power
        NEATVector testVec = NEATALU::ldexp<TypeP>(x,powN);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::ldexp(sT(*x[i].GetAcc<TypeP>()),powN);
            EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat, testVec[i], NEATALU::LDEXP_ERROR));
        }

        // 2.c Test vector of accurate values, vector of power values
        testVec = NEATALU::ldexp<TypeP>(x,powVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::ldexp(sT(*x[i].GetAcc<TypeP>()),powVec[i]);
            EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat, testVec[i], NEATALU::LDEXP_ERROR));
        }


        // 3. Test interval input
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // 3.a single interval testing
        sT refMin, refMax;
        xVal = x[0];

        testVal = NEATALU::ldexp<TypeP>(xVal,powN);
        /// Arg1Min == Arg1Max has already been tested
        if(!xVal.IsAcc())
        {
            refMax = RefALU::ldexp(sT(*xVal.GetMax<TypeP>()),powN);
            refMin = RefALU::ldexp(sT(*xVal.GetMin<TypeP>()),powN);
            EXPECT_TRUE(TestIntExpanded<sT>(refMin, refMax, testVal, NEATALU::LDEXP_ERROR));
        }

        // 3.b test vector of interval values, single value of power
        testVec = NEATALU::ldexp<TypeP>(x,powN);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            if(!testVec[i].IsAcc())
            {
                refMax = RefALU::ldexp(sT(*x[i].GetMax<TypeP>()),powN);
                refMin = RefALU::ldexp(sT(*x[i].GetMin<TypeP>()),powN);
                EXPECT_TRUE(TestIntExpanded<sT>(refMin, refMax, testVec[i], NEATALU::LDEXP_ERROR));
            }
        }

        // 3.b test vector of interval values, vector of power values
        testVec = NEATALU::ldexp<TypeP>(x,powVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            if(!testVec[i].IsAcc())
            {
                refMax = RefALU::ldexp(sT(*x[i].GetMax<TypeP>()),powVec[i]);
                refMin = RefALU::ldexp(sT(*x[i].GetMin<TypeP>()),powVec[i]);
                EXPECT_TRUE(TestIntExpanded<sT>(refMin, refMax, testVec[i], NEATALU::LDEXP_ERROR));
            }
        }
    }
}


template <typename T>
static void ModfTest(T refIntegrMax, T refIntegrMin, T refMax, T refMin, NEATValue xVal, NEATValue testIntegrVal, NEATValue testVal) {
    typedef typename downT<T>::type dT;

    if( refIntegrMax == refIntegrMin) {
        // we got accurate integral part, test it
        EXPECT_TRUE(TestAccExpanded<T>(refIntegrMax, testIntegrVal, NEATALU::MODF_ERROR));
        EXPECT_TRUE(TestAccExpanded<T>(refIntegrMin, testIntegrVal, NEATALU::MODF_ERROR));

        // test fractial part as interval
        EXPECT_TRUE(TestIntExpanded<T>(refMin, refMax, testVal, NEATALU::MODF_ERROR));
    } else {
        EXPECT_TRUE(TestIntExpanded<T>(refIntegrMin, refIntegrMax, testIntegrVal, NEATALU::MODF_ERROR));

        if(testVal.IsUnknown()) {
            // for example, min=1.8, max=2.2, fractial part should be greater
            // than 0.8 and less than 0.2, we are not able to make correct interval, 
            // so we set UNKNOWN here
            // in this case the difference between min and max should be < 1.0
            EXPECT_TRUE( ::fabs(*xVal.GetMax<dT>() - *xVal.GetMin<dT>()) < dT(1.0) );
        } else {

            // just to be sure: referense fractial part's min and max are inside the test interval
            EXPECT_TRUE( Utils::ge(refMin, T(*testVal.GetMin<dT>() )) && Utils::le(refMin, T(*testVal.GetMax<dT>() )));
            EXPECT_TRUE( Utils::ge(refMax, T(*testVal.GetMin<dT>() )) && Utils::le(refMax, T(*testVal.GetMax<dT>() )));

            if(*xVal.GetMin<dT>() >= dT(0.0)) {
                // all possible fractial values are less than 1.0 and greater or equal to 0.0
                EXPECT_TRUE(*testVal.GetMin<dT>() == dT(0.0));
                EXPECT_TRUE(*testVal.GetMax<dT>() == dT(1.0)-std::numeric_limits<dT>::min());
            } else if (*xVal.GetMax<dT>() <= dT(0.0)) {
                // all possible fractial values are less or equal to 0.0 and greater than -1.0
                EXPECT_TRUE(*testVal.GetMax<dT>() == dT(0.0));
                EXPECT_TRUE(*testVal.GetMin<dT>() == dT(-1.0)+std::numeric_limits<dT>::min());
            } else {
                // source interval includes 0 and difference between min and max greater than 1.0
                if (refIntegrMin == T(0.0)) {
                // example min=-0.3, max=100.1 result should be min=-0.3, max=0.(9)
                    EXPECT_TRUE(*testVal.GetMin<dT>() == refMin);
                    EXPECT_TRUE(*testVal.GetMax<dT>() == dT(1.0)-std::numeric_limits<dT>::min());
                } else if (refIntegrMax == T(0.0)) {
                // example min=-100.1, max=0.3 result should be min=-0.(9), max=0.3
                    EXPECT_TRUE(*testVal.GetMax<dT>() == refMax);
                    EXPECT_TRUE(*testVal.GetMin<dT>() == dT(-1.0)+std::numeric_limits<dT>::min());
                } else {
                // example min=-100.1, max=100.1 result should be min=-0.(9), max=0.(9)
                    EXPECT_TRUE(*testVal.GetMax<dT>() == dT(1.0)-std::numeric_limits<dT>::min());
                    EXPECT_TRUE(*testVal.GetMin<dT>() == dT(-1.0)+std::numeric_limits<dT>::min());
                }
            }
        }
    }
}

TYPED_TEST(NEATMathTestOneArg, modf)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATValue testIntegrVal;

    // 1.a Test special statuses
    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    NEATValue testAccVal = NEATALU::modf<TypeP>(NEATValue(NEATValue::ANY),&testIntegrVal);
    EXPECT_TRUE(testAccVal.IsUnknown());
    EXPECT_TRUE(testIntegrVal.IsUnknown());
    testAccVal = NEATALU::modf<TypeP>(NEATValue(NEATValue::UNWRITTEN),&testIntegrVal);
    EXPECT_TRUE(testAccVal.IsUnknown());
    EXPECT_TRUE(testIntegrVal.IsUnknown());
    testAccVal = NEATALU::modf<TypeP>(NEATValue(NEATValue::UNKNOWN),&testIntegrVal);
    EXPECT_TRUE(testAccVal.IsUnknown());
    EXPECT_TRUE(testIntegrVal.IsUnknown());

    // 1.b Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    testVal = NEATALU::modf<TypeP>(nan,&testIntegrVal);
    // modf ( NaN, exp ) returns the NaN
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    EXPECT_TRUE(testIntegrVal.IsNaN<TypeP>());

    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        // Test NaN value as input.
        // For binary or unary function you can include test that includes all combinations
        // of NaNs and non-NaN values
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::modf<TypeP>(notNAN,&testIntegrVal);
        EXPECT_FALSE(testVal.IsNaN<TypeP>());
        EXPECT_FALSE(testIntegrVal.IsNaN<TypeP>());
    }

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    // Done with corner cases. Now go to testing scalar and vector functions for randomized values
    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // 2. Test on accurate values.
        NEATVector x(Width);
        NEATVector testIntegrVec(Width);

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);            
        }

        // 2.a Test single accurate value
        NEATValue xVal = x[0];

        testVal = NEATALU::modf<TypeP>(xVal,&testIntegrVal);
        sT refAccValFloat, refAccIntegrVal;

        refAccValFloat = RefALU::modf(sT(*xVal.GetAcc<TypeP>()),&refAccIntegrVal);

        EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat, testVal, NEATALU::MODF_ERROR));
        EXPECT_TRUE(TestAccExpanded<sT>(refAccIntegrVal, testIntegrVal, NEATALU::MODF_ERROR));

        // 2.b Test vector of accurate values
        NEATVector testVec = NEATALU::modf<TypeP>(x,testIntegrVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::modf(sT(*x[i].GetAcc<TypeP>()),&refAccIntegrVal);
            EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat, testVec[i], NEATALU::MODF_ERROR));
            EXPECT_TRUE(TestAccExpanded<sT>(refAccIntegrVal, testIntegrVec[i], NEATALU::MODF_ERROR));
        }


        // 3. Test interval input
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // 3.a single interval testing
        sT refMin, refMax, refIntegrMin, refIntegrMax;
        xVal = x[0];

        testVal = NEATALU::modf<TypeP>(xVal,&testIntegrVal);
        refMax = RefALU::modf(sT(*xVal.GetMax<TypeP>()),&refIntegrMax);
        refMin = RefALU::modf(sT(*xVal.GetMin<TypeP>()),&refIntegrMin);

        ModfTest<sT>(refIntegrMax, refIntegrMin, refMax, refMin, xVal, testIntegrVal, testVal);


        // 3.b test vector of interval values
        testVec = NEATALU::modf<TypeP>(x,testIntegrVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            refMax = RefALU::modf(sT(*x[i].GetMax<TypeP>()),&refIntegrMax);
            refMin = RefALU::modf(sT(*x[i].GetMin<TypeP>()),&refIntegrMin);

            ModfTest<sT>(refIntegrMax, refIntegrMin, refMax, refMin, x[i], testIntegrVec[i], testVec[i]);
        }
    }
}


template <typename T>
static void RootnAccTest( T refAccValFloat, NEATValue xVal, NEATValue testVal, int powN) {
    typedef typename downT<T>::type dT;

    if(powN == 0)
        EXPECT_TRUE(testVal.IsNaN<dT>());
    else if( *xVal.GetAcc<dT>() < 0 && 0 == (powN&1) )
        EXPECT_TRUE(testVal.IsNaN<dT>());
    else 
        EXPECT_TRUE(TestAccExpanded<T>(refAccValFloat, testVal, NEATALU::ROOTN_ERROR));
}

template <typename T>
static void RootnIntervalTest( T refMin, T refMax, NEATValue xVal, NEATValue testVal, int powN) {
    typedef typename downT<T>::type dT;

    if(powN == 0)
        EXPECT_TRUE(testVal.IsNaN<dT>());
    else if( *xVal.GetMax<dT>() < 0 && 0 == (powN&1) )
        EXPECT_TRUE(testVal.IsNaN<dT>());
    else if( *xVal.GetMin<dT>() < 0 && 0 == (powN&1) )
        EXPECT_TRUE(testVal.IsUnknown());
    else {
        if( powN < 0) {
            if( powN&1 ) {
                // rootn ( +-0,  n ) is +-inf for odd n < 0
                if(xVal.Includes(dT(-0.0)))
                    refMin = Utils::GetNInf<T>();

                if(xVal.Includes(dT(+0.0)))
                    refMax = Utils::GetPInf<T>();
            } else {
                // rootn ( +-0,  n ) is +inf for even n < 0
                if(xVal.Includes(dT(0.0)))
                    refMax = Utils::GetPInf<T>();
            }
        }
        EXPECT_TRUE(TestIntExpanded<T>(refMin, refMax, testVal, NEATALU::ROOTN_ERROR));
    }
}

TYPED_TEST(NEATMathTestOneArg, rootn)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    int32_t Arg2[this->NUM_TESTS*this->vectorWidth];

    GenerateRandomVectorsAutoSeed(I32, &Arg2[0], this->currWidth, this->NUM_TESTS);

    int powN = Arg2[0];

    // 1.a Test special statuses
    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    NEATValue testAccVal = NEATALU::rootn<TypeP>(NEATValue(NEATValue::ANY),powN);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::rootn<TypeP>(NEATValue(NEATValue::UNWRITTEN),powN);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::rootn<TypeP>(NEATValue(NEATValue::UNKNOWN),powN);
    EXPECT_TRUE(testAccVal.IsUnknown());

    // 1.b Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    testVal = NEATALU::rootn<TypeP>(nan,powN);
    // rootn ( NaN, exp ) returns the NaN
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    testVal = NEATALU::rootn<TypeP>(nan,0);
    // rootn ( x, 0 ) returns the NaN
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    // rootn ( x, n ) returns a NaN for x < 0 and n is even
    NEATValue xVal = NEATValue(TypeP(-1.0));
    testVal = NEATALU::rootn<TypeP>(xVal,2);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    // rootn ( +-0, n ) is +-inf for odd n < 0
    xVal = NEATValue(TypeP(+0.0));
    testVal = NEATALU::rootn<TypeP>(xVal,-3);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, Utils::GetPInf<TypeP>()));
    xVal = NEATValue(TypeP(-0.0));
    testVal = NEATALU::rootn<TypeP>(xVal,-5);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, Utils::GetNInf<TypeP>()));

    // rootn ( +-0, n ) is +inf for even n < 0
    testVal = NEATALU::rootn<TypeP>(xVal,-4);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, Utils::GetPInf<TypeP>()));

    // rootn ( +-0, n ) is +0 for even n > 0
    testVal = NEATALU::rootn<TypeP>(xVal,6);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, TypeP(+0.0)));

    // rootn ( +-0, n ) is +-0 for odd n > 0
    xVal = NEATValue(TypeP(+0.0));
    testVal = NEATALU::rootn<TypeP>(xVal,3);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, TypeP(+0.0)));
    xVal = NEATValue(TypeP(-0.0));
    testVal = NEATALU::rootn<TypeP>(xVal,5);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, TypeP(-0.0)));

    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        // Test NaN value as input.
        // For binary or unary function you can include test that includes all combinations
        // of NaNs and non-NaN values
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        int powN = Arg2[i];
        testVal = NEATALU::rootn<TypeP>(notNAN,powN);

        // rootn ( x, n ) returns a NaN for x < 0 and n is even
        if( (!(powN&1)) && *notNAN.GetAcc<TypeP>() < TypeP(0.0))
            EXPECT_TRUE(testVal.IsNaN<TypeP>());
        else
            EXPECT_FALSE(testVal.IsNaN<TypeP>());
    }

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    // Done with corner cases. Now go to testing scalar and vector functions for randomized values
    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // 2. Test on accurate values.
        NEATVector x(Width);
        std::vector<int> powVec(this->vectorWidth);

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            powVec[i] = Arg2[testIdx*this->vectorWidth+i];
        }

        // 2.a Test single accurate value
        xVal = x[0];
        powN = powVec[0];

        testVal = NEATALU::rootn<TypeP>(xVal,powN);
        sT refAccValFloat;

        refAccValFloat = RefALU::rootn(sT(*xVal.GetAcc<TypeP>()),powN);
        RootnAccTest<sT>(refAccValFloat, xVal, testVal, powN);

        // 2.b Test vector of accurate values, vector of power values
        NEATVector testVec = NEATALU::rootn<TypeP>(x,powVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::rootn(sT(*x[i].GetAcc<TypeP>()),powVec[i]);
            RootnAccTest<sT>(refAccValFloat, x[i], testVec[i], powVec[i]);            
        }


        // 3. Test interval input
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // 3.a single interval testing
        sT refMin, refMax;
        xVal = x[0];

        testVal = NEATALU::rootn<TypeP>(xVal,powN);
        /// Arg1Min == Arg1Max has already been tested
        refMax = RefALU::rootn(sT(*xVal.GetMax<TypeP>()),powN);
        refMin = RefALU::rootn(sT(*xVal.GetMin<TypeP>()),powN);
        RootnIntervalTest<sT>(refMin, refMax, xVal, testVal, powN);


        // 3.b test vector of interval values, single value of power
        testVec = NEATALU::rootn<TypeP>(x,powVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            refMax = RefALU::rootn(sT(*x[i].GetMax<TypeP>()),powVec[i]);
            refMin = RefALU::rootn(sT(*x[i].GetMin<TypeP>()),powVec[i]);
            RootnIntervalTest<sT>(refMin, refMax, x[i], testVec[i], powVec[i]);
        }
    }
}

template <typename T>
static void PownAccTest( T refIn, NEATValue xVal, NEATValue testVal, int y) {
    typedef typename downT<T>::type dT;

    T ref = refIn;
    T vals[2];

    vals[0] = vals[1] = ref; // initialize values to expand interval

    if (y == 0) {
        EXPECT_TRUE(TestAccValue<dT>(testVal, dT(1.0)));
        return;
    }
    // calculate values to expand reference result interval, 
    // if input interval includes edge value
    if(xVal.IsAcc() && (Utils::eq(*xVal.GetAcc<T>(),T(+0.0)) || Utils::eq(*xVal.GetAcc<T>(), (T)-0.0)) ) {
        if(y < 0)  {
            if( y&1 ) {
                if (Utils::eq(*xVal.GetAcc<T>(),T(-0.0)))
                //pown ( +-0,  n ) is +-inf for odd n < 0
                    ref = Utils::GetNInf<T>();
                else
                    ref = Utils::GetPInf<T>();
            } else {
                //pown ( +-0,  n ) is +inf for even n < 0
                ref = Utils::GetPInf<T>();
            }
        } else {
            if( y&1 ) {
                if (Utils::eq(*xVal.GetAcc<T>(),T(-0.0)))
                //pown ( +-0,  n ) is +-0 for odd n > 0                        
                    ref = T(-0.0);
                else if (Utils::eq(*xVal.GetAcc<T>(),T(0.0)))
                //pown ( +-0,  n ) is +-0 for odd n > 0                        
                    ref = T(0.0);
            } else {
                //pown ( +-0,  n ) is +0 for even n > 0
                ref = T(+0.0);
            }
        }
    }

    EXPECT_TRUE(TestAccExpanded<T>(ref, testVal, NEATALU::POWN_ERROR));
}

template <typename T>
static void PownIntervalTest( T refMinIn, T refMaxIn, NEATValue xVal, NEATValue testVal, int y) {
    typedef typename downT<T>::type dT;

    T refMin = refMinIn;
    T refMax = refMaxIn;
    T vals[2];

    vals[0] = vals[1] = refMin; // initialize values to expand interval

    if (y == 0) {
        EXPECT_TRUE(TestAccValue<dT>(testVal, dT(1.0)));
        return;
    }

    // calculate values to expand reference result interval, 
    // if input interval includes edge values
    if( y < 0) {
        if( y&1 ) {
            // pown ( +-0,  n ) is +-inf for odd n < 0
            if(xVal.Includes(dT(-0.0)))
                vals[0] = Utils::GetNInf<T>();
            if(xVal.Includes(dT(+0.0)))
                vals[1] = Utils::GetPInf<T>();
        } else {
            // pown ( +-0,  n ) is +inf for even n < 0
            if(xVal.Includes(dT(0.0)))
                vals[0] = Utils::GetPInf<T>();
        }
    } else {
        if( y&1 ) {
            //pown ( +-0,  n ) is +-0 for odd n > 0
            if(xVal.Includes(dT(-0.0))) {
                vals[0] = T(-0.0);
            }
            if(xVal.Includes(dT(+0.0))) {
                vals[1] = T(+0.0);
            }
        } else {
            //pown ( +-0,  n ) is +0 for even n > 0
            if(xVal.Includes(dT(-0.0)) || xVal.Includes(dT(+0.0)))
            vals[0] = T(+0.0);
        }
    }

    // expand interval
    for (uint32_t i = 0; i < 2; i++)
    {
        if ( Utils::lt(vals[i], refMin) )
            refMin = vals[i];
        if ( Utils::gt(vals[i], refMax ) )
            refMax = vals[i];
    }

    EXPECT_TRUE(TestIntExpanded<T>(refMin, refMax, testVal, NEATALU::POWN_ERROR));
}

TYPED_TEST(NEATMathTestOneArg, pown)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    /// If your test doesn't currently support doubles, use SkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    int32_t Arg2[this->NUM_TESTS*this->vectorWidth];

    GenerateRandomVectorsAutoSeed(I32, &Arg2[0], this->currWidth, this->NUM_TESTS/2);
    GenerateRangedVectorsAutoSeed(I32, &Arg2[this->NUM_TESTS/2], this->currWidth, this->NUM_TESTS/2, -10, 10);

    int y = Arg2[0];

    // 1.a Test special statuses
    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    NEATValue testAccVal = NEATALU::pown<TypeP>(NEATValue(NEATValue::ANY),y);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::pown<TypeP>(NEATValue(NEATValue::UNWRITTEN),y);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::pown<TypeP>(NEATValue(NEATValue::UNKNOWN),y);
    EXPECT_TRUE(testAccVal.IsUnknown());

    // 1.b Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue testVal;
    testVal = NEATALU::pown<TypeP>(nan,y);
    // pown ( NaN, exp ) returns the NaN
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    testVal = NEATALU::pown<TypeP>(nan,0);
    // pown ( x, 0 ) returns 1
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, TypeP(1.0)));

    // pown ( +-0, n ) is +-inf for odd n < 0
    NEATValue xVal = NEATValue(TypeP(+0.0));
    testVal = NEATALU::pown<TypeP>(xVal,-3);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, Utils::GetPInf<TypeP>()));
    xVal = NEATValue(TypeP(-0.0));
    testVal = NEATALU::pown<TypeP>(xVal,-5);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, Utils::GetNInf<TypeP>()));

    // pown ( +-0, n ) is +inf for even n < 0
    testVal = NEATALU::pown<TypeP>(xVal,-4);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, Utils::GetPInf<TypeP>()));

    // pown ( +-0, n ) is +0 for even n > 0
    testVal = NEATALU::pown<TypeP>(xVal,6);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, TypeP(+0.0)));

    // pown ( +-0, n ) is +-0 for odd n > 0
    xVal = NEATValue(TypeP(+0.0));
    testVal = NEATALU::pown<TypeP>(xVal,3);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, TypeP(+0.0)));
    xVal = NEATValue(TypeP(-0.0));
    testVal = NEATALU::pown<TypeP>(xVal,5);
    EXPECT_TRUE(TestAccValue<TypeP>(testVal, TypeP(-0.0)));


    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    // Done with corner cases. Now go to testing scalar and vector functions for randomized values
    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // 2. Test on accurate values.
        NEATVector x(Width);
        std::vector<int> yVec(this->vectorWidth);

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            yVec[i] = Arg2[testIdx*this->vectorWidth+i];
        }

        // 2.a Test single accurate value
        xVal = x[0];
        y = yVec[0];

        testVal = NEATALU::pown<TypeP>(xVal,y);
        sT refAccValFloat;

        refAccValFloat = RefALU::pown(sT(*xVal.GetAcc<TypeP>()),y);
        PownAccTest<sT>(refAccValFloat, xVal, testVal, y);

        // 2.b Test vector of accurate values, vector of power values
        NEATVector testVec = NEATALU::pown<TypeP>(x,yVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::pown(sT(*x[i].GetAcc<TypeP>()),yVec[i]);
            PownAccTest<sT>(refAccValFloat, x[i], testVec[i], yVec[i]);
        }

        // 3. Test interval input
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // 3.a single interval testing
        sT refMin, refMax;
        xVal = x[0];
        y = yVec[0];

        testVal = NEATALU::pown<TypeP>(xVal,y);
        /// Arg1Min == Arg1Max has already been tested
        refMax = RefALU::pown(sT(*xVal.GetMax<TypeP>()),y);
        refMin = RefALU::pown(sT(*xVal.GetMin<TypeP>()),y);
        if(refMax < refMin)
            std::swap(refMin, refMax);
        PownIntervalTest<sT>(refMin, refMax, xVal, testVal, y);

        // 3.b test vector of interval values, vector of power values
        testVec = NEATALU::pown<TypeP>(x,yVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            refMax = RefALU::pown(sT(*x[i].GetMax<TypeP>()),yVec[i]);
            refMin = RefALU::pown(sT(*x[i].GetMin<TypeP>()),yVec[i]);
            if(refMax < refMin)
                std::swap(refMin, refMax);
            PownIntervalTest<sT>(refMin, refMax, x[i], testVec[i], yVec[i]);
        }
    }

}


template <typename T>
static bool TestNEATVal_fdim(NEATValue x, NEATValue y, NEATValue test)
{
    typedef typename superT<T>::type sT;
    bool passed = true;

    T xMax = *x.GetMax<T>();
    T yMin = *y.GetMin<T>();

    if( Utils::le<T>(xMax, yMin)) {        
        passed &= test.IsAcc();
        passed &= Utils::eq(*test.GetAcc<T>(), T(+0.0));
        return passed;
    }

    T testMin = *test.GetMin<T>();
    T testMax = *test.GetMax<T>();
    T xMin = *x.GetMin<T>();
    T yMax = *y.GetMax<T>();

    sT ref = RefALU::fdim<sT>(sT(xMin), sT(yMin));
    passed &= (Utils::ge(T(ref),testMin) && Utils::le(T(ref),testMax));
    ref = RefALU::fdim<sT>(sT(xMin), sT(yMax));
    passed &= (Utils::ge(T(ref),testMin) && Utils::le(T(ref),testMax));
    ref = RefALU::fdim<sT>(sT(xMax), sT(yMin));
    passed &= (Utils::ge(T(ref),testMin) && Utils::le(T(ref),testMax));
    ref = RefALU::fdim<sT>(sT(xMax), sT(yMax));
    passed &= (Utils::ge(T(ref),testMin) && Utils::le(T(ref),testMax));

    sT refMin, refMax;
    if( Utils::gt<T>(xMin,yMax))
        refMin = RefALU::sub<sT>(sT(xMin), sT(yMax));
    else
        refMin = 0;
    
    refMax = RefALU::sub<sT>(sT(xMax), sT(yMin));
    passed &= Utils::eq(testMin, T(refMin));
    passed &= Utils::eq(testMax, T(refMax));      

    return passed;
}

TYPED_TEST(NEATMathTestTwoArgs, fdim)
{
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;

    if (SkipDoubleTest<TypeP>())
        return;

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeP>(NEATALU::fdim<TypeP>));
    EXPECT_TRUE(TestUnwritten<TypeP>(NEATALU::fdim<TypeP>));
    EXPECT_TRUE(TestAny<TypeP>(NEATALU::fdim<TypeP>));

    // Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeP>();
    // Ordinary not a special value
    NEATValue notNaN = NEATValue(TypeP(1.1));
    NEATValue testVal;
    testVal = NEATALU::fdim<TypeP>(notNaN, notNaN);
    EXPECT_FALSE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::fdim<TypeP>(notNaN, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::fdim<TypeP>(nan, notNaN);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());
    testVal = NEATALU::fdim<TypeP>(nan, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeP>());

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // Test on accurate values
        NEATVector x(Width), y(Width);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            y[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i]);
        }

        // Test single accurate value
        NEATValue xVal = x[0];
        NEATValue yVal = y[0];

        testVal = NEATALU::fdim<TypeP>(xVal,yVal);
        EXPECT_TRUE(TestNEATVal_fdim<TypeP>(xVal,yVal,testVal));

        // Test vector of accurate values x and y
        NEATVector testVec = NEATALU::fdim<TypeP>(x,y);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            EXPECT_TRUE(TestNEATVal_fdim<TypeP>(x[i],y[i],testVec[i]));
        }

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            y[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i], this->Arg2Max[testIdx*this->vectorWidth+i]);
        }
        // Test vector of accurate values x and vector of intervals y
        testVec = NEATALU::fdim<TypeP>(x,y);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            EXPECT_TRUE(TestNEATVal_fdim<TypeP>(x[i],y[i],testVec[i]));
        }
        // Test vector of accurate values y and vector of intervals x
        testVec = NEATALU::fdim<TypeP>(y,x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            EXPECT_TRUE(TestNEATVal_fdim<TypeP>(y[i],x[i],testVec[i]));
        }        
       
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        // Test single  value
        xVal = x[0];
        yVal = y[0];
        testVal = NEATALU::fdim<TypeP>(xVal,yVal);
        EXPECT_TRUE(TestNEATVal_fdim<TypeP>(xVal,yVal,testVal));
        // Test vector of intervals values x and vector of intervals y

        testVec = NEATALU::fdim<TypeP>(x,y);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            EXPECT_TRUE(TestNEATVal_fdim<TypeP>(x[i],y[i],testVec[i]));
        }
        
    }
}

TYPED_TEST(NEATMathTestOneArg, lgamma)
{
    typedef typename TypeParam::Type TypeP;

    if (SkipDoubleTest<TypeP>())
        return;

    // The ULP values for built-in math functions lgamma and lgamma_r is currently undefined,
    // so we test special NEAT values: UNKNOWN, UNWRITTEN and ANY only

    EXPECT_TRUE(TestUnknown(NEATALU::lgamma<TypeP>));
    EXPECT_TRUE(TestUnwritten(NEATALU::lgamma<TypeP>));
    EXPECT_TRUE(TestAny(NEATALU::lgamma<TypeP>));
}

TYPED_TEST(NEATMathTestOneArg, lgamma_r)
{
    typedef typename TypeParam::Type TypeP;

    if (SkipDoubleTest<TypeP>())
        return;

    int32_t sign = 0; // int32_t is used for both floates and doubles
    int32_t * signp = &sign;
    
    // The ULP values for built-in math functions lgamma and lgamma_r is currently undefined,
    // so we test special NEAT values: UNKNOWN, UNWRITTEN and ANY only
    NEATValue testAccVal = NEATALU::lgamma_r<TypeP>(NEATValue(NEATValue::ANY),signp);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::lgamma_r<TypeP>(NEATValue(NEATValue::UNWRITTEN),signp);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::lgamma_r<TypeP>(NEATValue(NEATValue::UNKNOWN),signp);
    EXPECT_TRUE(testAccVal.IsUnknown());
}

TYPED_TEST(NEATMathTestThreeArgs, mad)
{
    typedef typename TypeParam::Type TypeP;

    if (SkipDoubleTest<TypeP>())
        return;

    // The ULP values for built-in mad is inf,
    // so we test special NEAT values: UNKNOWN, UNWRITTEN and ANY only

    EXPECT_TRUE(TestUnknown<TypeP>(NEATALU::mad<TypeP>));
    EXPECT_TRUE(TestUnwritten<TypeP>(NEATALU::mad<TypeP>));
    EXPECT_TRUE(TestAny<TypeP>(NEATALU::mad<TypeP>));
}


template <typename T>
class NEATMaxMinMagTest {
public:
    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 300;
    // Vector data type length.
    static const uint32_t vectorWidth = 8;
    // Intervals with test values for common functions arguments.
    T Arg1Min[NUM_TESTS*vectorWidth];
    T Arg1Max[NUM_TESTS*vectorWidth];
    T Arg2Min[NUM_TESTS*vectorWidth];
    T Arg2Max[NUM_TESTS*vectorWidth];

    // Parameters for random data generator.
    VectorWidth currWidth;
    DataTypeVal dataTypeVal;

    typedef NEATValue (*NEATFuncP)(const NEATValue&, const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&, const NEATVector&);

    typedef typename TestIntervalRandomly<T>::RefFuncPTwoArgs RefFuncP;

    NEATMaxMinMagTest()
    {
        currWidth = VectorWidthWrapper::ValueOf(this->vectorWidth);
        dataTypeVal = GetDataTypeVal<T>();

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Max[0], currWidth, NUM_TESTS);

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
            if (Arg2Min[i] > Arg2Max[i]) std::swap(Arg2Min[i], Arg2Max[i]);
        }
    }

    void TestSpecValues(NEATFuncP NEATFunc, RefFuncP RefFunc)
    {
        NEATValue unkn = NEATValue(NEATValue::UNKNOWN);
        NEATValue unwr = NEATValue(NEATValue::UNWRITTEN);
        NEATValue any  = NEATValue(NEATValue::ANY);
        NEATValue nan  = NEATValue::NaN<T>();
        NEATValue tmp;
    
        tmp = NEATFunc(nan, any);
        EXPECT_TRUE(tmp.IsUnknown());
        tmp = NEATFunc(any, nan);
        EXPECT_TRUE(tmp.IsUnknown());
        tmp = NEATFunc(any, any);
        EXPECT_TRUE(tmp.IsUnknown());

        EXPECT_TRUE(TestUnknown<T>(NEATFunc));
        EXPECT_TRUE(TestUnwritten<T>(NEATFunc));
        EXPECT_TRUE(TestAny<T>(NEATFunc));

        T refNaN = Utils::GetNaN<T>();
        T refPInf = Utils::GetPInf<T>();
        T refNInf = Utils::GetNInf<T>();
        T refVal = Arg1Min[0];
        NEATValue val = NEATValue(refVal);
        NEATValue pInf = NEATValue(refPInf);
        NEATValue nInf = NEATValue(refNInf);

        tmp = NEATFunc(nan, nan);
        T refRes = RefFunc(refNaN, refNaN);
        if( Utils::IsNaN<T>(refRes))
            EXPECT_TRUE(tmp.IsNaN<T>());
        else
            EXPECT_TRUE(Utils::eq<T>(refRes,*tmp.GetAcc<T>()));

        tmp = NEATFunc(pInf, pInf);
        refRes = RefFunc(refPInf, refPInf);
        EXPECT_TRUE(Utils::eq<T>(refRes,*tmp.GetAcc<T>()));
        tmp = NEATFunc(nInf, nInf);
        refRes = RefFunc(refNInf, refNInf);
        EXPECT_TRUE(Utils::eq<T>(refRes,*tmp.GetAcc<T>()));
        tmp = NEATFunc(pInf, nInf);
        refRes = RefFunc(refPInf, refNInf);
        EXPECT_TRUE(Utils::eq<T>(refRes,*tmp.GetAcc<T>()));
        tmp = NEATFunc(nInf, pInf);
        refRes = RefFunc(refNInf, refPInf);
        EXPECT_TRUE(Utils::eq<T>(refRes,*tmp.GetAcc<T>()));

        tmp = NEATFunc(val, pInf);
        refRes = RefFunc(refVal, refPInf);
        EXPECT_TRUE(Utils::eq<T>(refRes,*tmp.GetAcc<T>()));
        tmp = NEATFunc(nInf, val);
        refRes = RefFunc(refNInf, refVal);
        EXPECT_TRUE(Utils::eq<T>(refRes,*tmp.GetAcc<T>()));
        tmp = NEATFunc(val, nInf);
        refRes = RefFunc(refVal, refNInf);
        EXPECT_TRUE(Utils::eq<T>(refRes,*tmp.GetAcc<T>()));
        tmp = NEATFunc(pInf, val);
        refRes = RefFunc(refPInf, refVal);
        EXPECT_TRUE(Utils::eq<T>(refRes,*tmp.GetAcc<T>()));
    }

    void TestOtherValues(NEATFuncP NEATFunc, NEATFuncVecP NEATFuncVec, RefFuncP RefFunc) {

        VectorWidth Width = VectorWidthWrapper::ValueOf(vectorWidth);

        for(uint32_t testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            // Test result for given operation of accurate values or intervals
            NEATValue accVal, intVal;
            NEATValue testAccVal, testIntVal;
            NEATValue refAccVal, refIntVal;
       
            for(uint32_t i = 0; i<vectorWidth; i++)
            {
                uint32_t idx = testIdx * vectorWidth + i;
    
                // Test for accurate operation (input is accurate values, output - interval)
                NEATValue aAcc = NEATValue(Arg1Min[idx]);
                NEATValue bAcc = NEATValue(Arg2Min[idx]);

                NEATValue accRes = NEATFunc(aAcc,bAcc);
                T refVal = RefFunc(RefALU::flush(Arg1Min[idx]), RefALU::flush(Arg2Min[idx]));
                EXPECT_TRUE(accRes.IsAcc());
                if (accRes.IsAcc())
                    EXPECT_TRUE(refVal == *accRes.GetAcc<T>());
            }

            // Test vector
            NEATVector a(Width);
            NEATVector b(Width);
            for(uint32_t i = 0; i<vectorWidth; i++)
            {
                uint32_t idx = testIdx * vectorWidth + i;
                a[i] = NEATValue(Arg1Min[idx], Arg1Max[idx]);
                b[i] = NEATValue(Arg2Min[idx], Arg2Max[idx]);
            }
            // Call NEAT
            NEATVector res = NEATFuncVec(a,b);
            // Verify the result
            for(uint32_t i = 0; i<vectorWidth; i++) {
                uint32_t idx = testIdx * vectorWidth + i;
                if( res[i].IsAcc()) {
                    T refVal = RefFunc(RefALU::flush(Arg1Min[idx]), RefALU::flush(Arg2Min[idx]));
                    EXPECT_TRUE(refVal == *res[i].GetAcc<T>());
                }
                if( res[i].IsInterval()) {
                    TestIntervalRandomly<T> test(RefFunc,res[i],Arg1Min[idx],Arg1Max[idx],Arg2Min[idx],Arg2Max[idx],50);
                    EXPECT_TRUE(test.GetTestResult());
                }
            }
        }
    }
};


template <typename T>
class NEATMaxMinMagTestRun : public ALUTest {
};


TYPED_TEST_CASE(NEATMaxMinMagTestRun, FloatTypesCommon);
TYPED_TEST(NEATMaxMinMagTestRun, maxmag)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef NEATValue (*NEATFuncP)(const NEATValue&, const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&, const NEATVector&);
    typedef TypeP (*RefFuncP) (const TypeP&, const TypeP&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::maxmag<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::maxmag<TypeP>;
    RefFuncP RefFunc = &RefALU::maxmag<TypeP>;

    NEATMaxMinMagTest<TypeP> test;

    test.TestSpecValues(NEATFunc, RefFunc);
    test.TestOtherValues(NEATFunc, NEATFuncVec, RefFunc);
}

TYPED_TEST_CASE(NEATMaxMinMagTestRun, FloatTypesCommon);
TYPED_TEST(NEATMaxMinMagTestRun, minmag)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef NEATValue (*NEATFuncP)(const NEATValue&, const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&, const NEATVector&);
    typedef TypeP (*RefFuncP) (const TypeP&, const TypeP&);

    /// If your test doesn't currently support doubles, use DkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::minmag<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::minmag<TypeP>;
    RefFuncP RefFunc = &RefALU::minmag<TypeP>;

    NEATMaxMinMagTest<TypeP> test;

    test.TestSpecValues(NEATFunc, RefFunc);
    test.TestOtherValues(NEATFunc, NEATFuncVec, RefFunc);
}


TYPED_TEST(NEATMathTestTwoArgs, remainder)
{
    typedef typename TypeParam::Type T;
    typedef typename superT<T>::type sT;

    if (SkipDoubleTest<T>())
        return;

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<T>(NEATALU::remainder<T>));
    EXPECT_TRUE(TestUnwritten<T>(NEATALU::remainder<T>));
    EXPECT_TRUE(TestAny<T>(NEATALU::remainder<T>));

    // Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<T>();
    // Ordinary not a special value
    NEATValue notNaN = NEATValue(T(1.1));
    NEATValue testVal;
    testVal = NEATALU::remainder<T>(notNaN, notNaN);
    EXPECT_FALSE(testVal.IsNaN<T>());
    testVal = NEATALU::remainder<T>(notNaN, nan);
    EXPECT_TRUE(testVal.IsNaN<T>());
    testVal = NEATALU::remainder<T>(nan, notNaN);
    EXPECT_TRUE(testVal.IsNaN<T>());
    testVal = NEATALU::remainder<T>(nan, nan);
    EXPECT_TRUE(testVal.IsNaN<T>());

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // Test on accurate values
        NEATVector x(Width), y(Width);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            y[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i]);
        }

        // Test single accurate value
        NEATValue xVal = x[0];
        NEATValue yVal = y[0];

        testVal = NEATALU::remainder<T>(xVal,yVal);
        T refVal = RefALU::remainder<T>(RefALU::flush(*xVal.GetAcc<T>()), RefALU::flush(*yVal.GetAcc<T>()));
        if( RefALU::flush(*yVal.GetAcc<T>()) != 0)
            EXPECT_TRUE(*testVal.GetAcc<T>() == refVal);
        else
            EXPECT_TRUE(testVal.IsUnknown());
        
        // Test vector of accurate values x and y
        NEATVector testVec = NEATALU::remainder<T>(x,y);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refVal = RefALU::remainder<T>(RefALU::flush(*x[i].GetAcc<T>()), RefALU::flush(*y[i].GetAcc<T>()));
            if( RefALU::flush(*y[i].GetAcc<T>()) != 0) {
                EXPECT_TRUE(*testVec[i].GetAcc<T>() == refVal);
            }
            else
                EXPECT_TRUE(testVec[i].IsUnknown());
        }

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
            y[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i], this->Arg2Max[testIdx*this->vectorWidth+i]);
        }

        testVec = NEATALU::remainder<T>(x,y);
        // Verify the result
        for(uint32_t i = 0; i<this->vectorWidth; i++) {
            uint32_t idx = testIdx * this->vectorWidth + i;
            if( testVec[i].IsInterval()) {
                TestIntervalRandomly<T> test(RefALU::remainder,testVec[i],this->Arg1Min[idx],this->Arg1Max[idx],
                                                                          this->Arg2Min[idx],this->Arg2Max[idx],50);
                EXPECT_TRUE(test.GetTestResult());
            }
        }
    }


    {
        const int NUM_TESTS = 13;
        T data[NUM_TESTS][4] = {\
            {T(1.0), T(3.0),      T(1.0), T(3.0)},\
            {T(11.9), T(12.1),    T(1.8), T(2.2)},\
            {T(34.0), T(36.0),    T(4.9), T(5.1)},\
            {T(11.9), T(12.1),    T(1.3), T(2.2)},\
            {T(11.9), T(12.1),    T(1.6), T(2.0)},\
            {T(-1.0), T(1.0),     T(1.0), T(3.0)},\
            {T(-3.0), T(-1.0),    T(1.0), T(3.0)},\
            {T(-3.0), T(1.0),     T(1.0), T(2.0)},\
            {T(-1.0), T(3.0),     T(1.0), T(2.0)},\
            {T(-10.0), T(-1.0),   T(1.0), T(2.0)},\
            {T(-5.0), T(-2.0),    T(0.5), T(2.0)},\
            {T(1.0), T(3.0),      T(-3.0), T(-1.0)},\
            {T(5.0), T(10.0),     T(-5.0), T(-3.0)}
        };
        for(int i = 0; i<NUM_TESTS; i++)
        {
            NEATValue dividend(data[i][0], data[i][1]);
            NEATValue divisor(data[i][2], data[i][3]);
            NEATValue rem = NEATALU::remainder<T>(dividend, divisor);
            // check interval if not unknown
            bool passed = true;
            if(rem.GetStatus() != NEATValue::UNKNOWN)
            {
                static int STEPS_COUNT = 10;
                T divisorStep = (*divisor.GetMax<T>() - *divisor.GetMin<T>()) / STEPS_COUNT;
                T dividendStep = (*dividend.GetMax<T>() - *dividend.GetMin<T>()) / STEPS_COUNT;

                T min = *rem.GetMin<T>();
                T max = *rem.GetMax<T>();                

                for(T divisorIdx = *divisor.GetMin<T>(); divisorIdx < *divisor.GetMax<T>(); divisorIdx += divisorStep)
                {
                    for(T dividendIdx = *dividend.GetMin<T>(); dividendIdx<*dividend.GetMax<T>(); dividendIdx += dividendStep)
                    {
                        T res = RefALU::remainder(dividendIdx, divisorIdx);
                        if(Utils::IsInf(res) || Utils::IsNaN(res))
                            passed = false;

                        if((res > max) || (res < min))
                            passed = false;
                    }
                }
            }
            else
            {
                passed = false; // input data was formed not to have unknown cases.
            }
            EXPECT_TRUE(passed);
        }
    }
    {
        const int UNKNOWN_TESTS_COUNT = 4;
        // test unknown cases
         T data[UNKNOWN_TESTS_COUNT][4] = {\
            {T(10.9), T(11.1),    T(1.9),  T(2.1)},\
            {T(2.0),  T(7.0),     T(-1.0), T(1.0)},\
            {T(-1.0), T(1.0),     T(0.0),  T(1.0)},\
            {T(-1.0), T(1.0),     T(-0.0), T(1.0)}
        };
        for(int i = 0; i<UNKNOWN_TESTS_COUNT; i++)
        {
            NEATValue dividend(data[i][0], data[i][1]);
            NEATValue divisor(data[i][2], data[i][3]);
            NEATValue frem = NEATALU::remainder<T>(dividend, divisor);
            EXPECT_EQ(NEATValue::UNKNOWN, frem.GetStatus());
        }
    }


}

TYPED_TEST(NEATMathTestTwoArgs, remquo)
{
    typedef typename TypeParam::Type T;
    typedef typename superT<T>::type sT;

    if (SkipDoubleTest<T>())
        return;

    // The quo value (the lower seven bits of the integral quotient x/y) is the only
    // difference bitween remainder and remquo functions, but this value is not valid for interval case,
    // so test checks the accurate values only: remquo actually calls remainder function for 
    // other cases. 

    int32_t testQuo = 1, refQuo;

    // Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<T>();
    // Ordinary not a special value
    NEATValue notNaN = NEATValue(T(1.1));
    NEATValue testVal;

    // remquo (x, y, &quo) returns a NaN and 0 in quo if either argument is a NaN
    testQuo = 1;
    testVal = NEATALU::remquo<T>(notNaN, notNaN, &testQuo);
    EXPECT_FALSE(testVal.IsNaN<T>());
    EXPECT_TRUE(testQuo == 1);

    testVal = NEATALU::remquo<T>(notNaN, nan, &testQuo);
    EXPECT_TRUE(testVal.IsNaN<T>());
    EXPECT_TRUE(testQuo == 0);
    testQuo = 1;
    testVal = NEATALU::remquo<T>(nan, notNaN, &testQuo);
    EXPECT_TRUE(testVal.IsNaN<T>());
    EXPECT_TRUE(testQuo == 0);
    testQuo = 1;
    testVal = NEATALU::remquo<T>(nan, nan, &testQuo);
    EXPECT_TRUE(testVal.IsNaN<T>());
    EXPECT_TRUE(testQuo == 0);

    testQuo = 1;
    // remquo (x, y, &quo) returns a NaN and 0 in quo if x is +-inf
    testVal = NEATALU::remquo<T>(NEATValue(Utils::GetPInf<T>()),notNaN, &testQuo);
    EXPECT_TRUE(testVal.IsNaN<T>());
    EXPECT_TRUE(testQuo == 0);
    testQuo = 1;
    testVal = NEATALU::remquo<T>(NEATValue(Utils::GetNInf<T>()),notNaN, &testQuo);
    EXPECT_TRUE(testVal.IsNaN<T>());
    EXPECT_TRUE(testQuo == 0);
    // or if y is 0 and the other argument is non-NaN
    testQuo = 1;
    testVal = NEATALU::remquo<T>(notNaN, NEATValue(T(0.0)), &testQuo);
    EXPECT_TRUE(testVal.IsNaN<T>());
    EXPECT_TRUE(testQuo == 0);

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // Test on accurate values
        NEATVector x(Width), y(Width);
        std::vector<int32_t> testQuoVec;        

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            y[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i]);
        }

        // Test single accurate value
        NEATValue xVal = x[0];
        NEATValue yVal = y[0];

        testVal = NEATALU::remquo<T>(xVal,yVal, &testQuo);
        T refVal = RefALU::remquo<T>(RefALU::flush(*xVal.GetAcc<T>()), RefALU::flush(*yVal.GetAcc<T>()), &refQuo);
        if( RefALU::flush(*yVal.GetAcc<T>()) != 0) {
            EXPECT_TRUE(*testVal.GetAcc<T>() == refVal);
            EXPECT_TRUE(refQuo == testQuo);
        }
        else
            EXPECT_TRUE(testVal.IsUnknown());
        
        // Test vector of accurate values x and y
        NEATVector testVec = NEATALU::remquo<T>(x,y,testQuoVec);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refVal = RefALU::remquo<T>(RefALU::flush(*x[i].GetAcc<T>()), RefALU::flush(*y[i].GetAcc<T>()), &refQuo);
            if( RefALU::flush(*y[i].GetAcc<T>()) != 0) {
                EXPECT_TRUE(*testVec[i].GetAcc<T>() == refVal);
                EXPECT_TRUE(refQuo == testQuoVec[i]);
            }
            else
                EXPECT_TRUE(testVec[i].IsUnknown());
        }
    }
}
