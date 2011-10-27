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

File Name:  NEATALUCommonTest.cpp

\*****************************************************************************/

// \brief Tests for OpenCL common built-in functions (see spec. 6.11.4.) in NEATALU

#include <math.h>
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
class NEATAluTypedCommon : public ALUTest {
public:
    // Number of different random inputs to test.
    static const int NUM_TESTS = 500;
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

    NEATAluTypedCommon(){
        currWidth = VectorWidthWrapper::ValueOf(vectorWidth);
        if(sizeof(T) == sizeof(float)) {
            dataTypeVal = F32;
        } else if(sizeof(T) == sizeof(double)){
            dataTypeVal = F64;
        }
        else{
            dataTypeVal = UNSPECIFIED_DATA_TYPE;
        }

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg2Max[0], currWidth, NUM_TESTS);

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
            if (Arg2Min[i] > Arg2Max[i]) std::swap(Arg2Min[i], Arg2Max[i]);
        }
    }

};


typedef ::testing::Types<ValueTypeContainer<float,true>,ValueTypeContainer<float,false> > FloatTypesCommon2;
// TODO: add double tests
typedef ::testing::Types<float> FloatTypesCommon;
TYPED_TEST_CASE(NEATAluTypedCommon, FloatTypesCommon);

TYPED_TEST(NEATAluTypedCommon, step)
{
    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeParam>()){
        return;
    }

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeParam>(NEATALU::step<TypeParam>));
    EXPECT_TRUE(TestUnwritten<TypeParam>(NEATALU::step<TypeParam>));
    EXPECT_TRUE(TestAny<TypeParam>(NEATALU::step<TypeParam>));

    // Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeParam>();
    NEATValue notSpecial = NEATValue(this->Arg1Min[0]);
    NEATValue testVal;
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::step<TypeParam>(notNAN, notNAN);
        EXPECT_FALSE(testVal.IsNaN<TypeParam>());
    }
    testVal = NEATALU::step<TypeParam>(nan, notSpecial);
    EXPECT_TRUE(testVal.IsNaN<TypeParam>());
    testVal = NEATALU::step<TypeParam>(notSpecial, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeParam>());
    testVal = NEATALU::step<TypeParam>(nan, nan);
    EXPECT_TRUE(testVal.IsNaN<TypeParam>());

    // Test +/-INFs
    NEATValue inf = NEATValue(Utils::GetPInf<TypeParam>());
    NEATValue mInf = NEATValue(Utils::GetNInf<TypeParam>());
    testVal = NEATALU::step<TypeParam>(notSpecial, inf);
    EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)1);
    testVal = NEATALU::step<TypeParam>(inf, notSpecial);
    EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)0);
    testVal = NEATALU::step<TypeParam>(inf, inf);
    EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)1);
    testVal = NEATALU::step<TypeParam>(notSpecial, mInf);
    EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)0);
    testVal = NEATALU::step<TypeParam>(mInf, notSpecial);
    EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)1);
    testVal = NEATALU::step<TypeParam>(mInf, mInf);
    EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)1);

    for(int testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // Test on accurate values.
        NEATVector edge(this->currWidth);
        NEATVector x(this->currWidth);

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            edge[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            x[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i]);
        }

        /* test for single accurate NEAT value*/
        NEATValue edgeVal = edge[0];
        NEATValue xVal = x[0];

        testVal = NEATALU::step<TypeParam>(edgeVal, xVal);
        typedef typename superT<TypeParam>::type sT;
        sT refAccValFloat;

        refAccValFloat = RefALU::step(sT(*edgeVal.GetAcc<TypeParam>()), sT(*xVal.GetAcc<TypeParam>()));
        EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)refAccValFloat);

        /* test for vector of NEAT accurate */
        NEATVector testVec = NEATALU::step<TypeParam>(edge, x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::step(sT(*edge[i].GetAcc<TypeParam>()), sT(*x[i].GetAcc<TypeParam>()));
            EXPECT_TRUE(testVec[i].IsAcc() && *testVec[i].GetAcc<TypeParam>() == (TypeParam)refAccValFloat);
        }

        // Test on interval values.
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            edge[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
            x[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i], this->Arg2Max[testIdx*this->vectorWidth+i]);
        }

        /* test for single interval NEAT value */
        sT oneCase, zeroCase;

        edgeVal = edge[0];
        xVal = x[0];

        testVal = NEATALU::step<TypeParam>(edgeVal, xVal);

        zeroCase = RefALU::step(sT(*edgeVal.GetMin<TypeParam>()), sT(*xVal.GetMax<TypeParam>()));
        oneCase = RefALU::step(sT(*edgeVal.GetMax<TypeParam>()), sT(*xVal.GetMin<TypeParam>()));
        if (zeroCase == oneCase) {
            EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)zeroCase);
        } else {
            EXPECT_TRUE(zeroCase == sT(1) && oneCase == sT(0) && testVal.IsUnknown());
        }

        /* test for vector of NEAT intervals */
        testVec = NEATALU::step<TypeParam>(edge, x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            zeroCase = RefALU::step(sT(*edge[i].GetMin<TypeParam>()), sT(*x[i].GetMax<TypeParam>()));
            oneCase = RefALU::step(sT(*edge[i].GetMax<TypeParam>()), sT(*x[i].GetMin<TypeParam>()));
            if (zeroCase == oneCase) {
                EXPECT_TRUE(testVec[i].IsAcc() && *testVec[i].GetAcc<TypeParam>() == (TypeParam)zeroCase);
            } else {
                EXPECT_TRUE(zeroCase == sT(1) && oneCase == sT(0) && testVec[i].IsUnknown());
            }
        }
    }
}

template<typename TypeParam>
static NEATValue min_ref(const NEATValue& xVal, const NEATValue& yVal)
{
    typedef typename superT<TypeParam>::type superT;
    superT xmin=*xVal.GetMin<TypeParam>();
    superT xmax=*xVal.GetMax<TypeParam>();
    superT ymin=*yVal.GetMin<TypeParam>();
    superT ymax=*yVal.GetMax<TypeParam>();

    bool intvOverlap = (ymin <= xmax && xmin <= ymax);
    NEATValue refOut;
    if(intvOverlap)
        refOut = NEATValue(
        RefALU::fmin(*xVal.GetMin<TypeParam>(), *yVal.GetMin<TypeParam>()),
        RefALU::fmin(*xVal.GetMax<TypeParam>(), *yVal.GetMax<TypeParam>()));
    else if(xmax < ymin)
        refOut = xVal;
    else if(xmin > ymax)
        refOut = yVal;
    else
        // should not be here
        throw Exception::IllegalFunctionCall("min_ref is called with wrong arguments");
    return NEATValue(refOut);
}

// TODO: add double tests
TYPED_TEST(NEATAluTypedCommon, min)
{
    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeParam>()){
        return;
    }

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeParam>(NEATALU::min<TypeParam>));
    EXPECT_TRUE(TestUnwritten<TypeParam>(NEATALU::min<TypeParam>));
    EXPECT_TRUE(TestAny<TypeParam>(NEATALU::min<TypeParam>));

    // Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeParam>();
    NEATValue notSpecial = NEATValue(this->Arg1Min[0]);
    NEATValue testVal;
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::min<TypeParam>(notNAN, notNAN);
        EXPECT_FALSE(testVal.IsNaN<TypeParam>());
    }
    testVal = NEATALU::min<TypeParam>(nan, notSpecial);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::min<TypeParam>(notSpecial, nan);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::min<TypeParam>(nan, nan);
    EXPECT_TRUE(testVal.IsAny());

    // Test +/-INFs
    NEATValue inf = NEATValue(Utils::GetPInf<TypeParam>());
    NEATValue mInf = NEATValue(Utils::GetNInf<TypeParam>());
    testVal = NEATALU::min<TypeParam>(notSpecial, inf);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::min<TypeParam>(inf, notSpecial);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::min<TypeParam>(inf, inf);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU ::min<TypeParam>(notSpecial, mInf);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::min<TypeParam>(mInf, notSpecial);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::min<TypeParam>(mInf, mInf);
    EXPECT_TRUE(testVal.IsAny());

    // manual tests
    NEATValue nx, ny, nres;
    TypeParam xmin, xmax, ymin, ymax;

    // overlap intervals
    xmin = -1.0; xmax = 6.0; ymin = 5.0; ymax = 15.0;
    nx.SetIntervalVal<TypeParam>(xmin, xmax);
    ny.SetIntervalVal<TypeParam>(ymin, ymax);
    nres = NEATALU::min<TypeParam>(nx, ny);
    EXPECT_TRUE(nres.IsInterval() &&
        *nres.GetMin<TypeParam>() == xmin && *nres.GetMax<TypeParam>() == xmax);

    // y interval contains x interval
    xmin = -1.0; xmax = 1.0; ymin = -5.0; ymax = 15.0;
    nx.SetIntervalVal<TypeParam>(xmin, xmax);
    ny.SetIntervalVal<TypeParam>(ymin, ymax);
    nres = NEATALU::min<TypeParam>(nx, ny);
    EXPECT_TRUE(nres.IsInterval() &&
        *nres.GetMin<TypeParam>() == ymin && *nres.GetMax<TypeParam>() == xmax);

    // x interval is less than y
    xmin = -1.0; xmax = 1.0; ymin = 5.0; ymax = 15.0;
    nx.SetIntervalVal<TypeParam>(xmin, xmax);
    ny.SetIntervalVal<TypeParam>(ymin, ymax);
    nres = NEATALU::min<TypeParam>(nx, ny);
    EXPECT_TRUE(nres.IsInterval() &&
        *nres.GetMin<TypeParam>() == xmin && *nres.GetMax<TypeParam>() == xmax);

    // y interval is less than x
    xmin = -1.0; xmax = 1.0; ymin = -15.0; ymax = -13.0;
    nx.SetIntervalVal<TypeParam>(xmin, xmax);
    ny.SetIntervalVal<TypeParam>(ymin, ymax);
    nres = NEATALU::min<TypeParam>(nx, ny);
    EXPECT_TRUE(nres.IsInterval() &&
        *nres.GetMin<TypeParam>() == ymin && *nres.GetMax<TypeParam>() == ymax);


    for(int testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // Test on accurate values.
        NEATVector x(this->currWidth);
        NEATVector y(this->currWidth);

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            y[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i]);
        }

        /* test for single accurate NEAT value*/
        NEATValue xVal = x[0];
        NEATValue yVal = y[0];

        testVal = NEATALU::min<TypeParam>(xVal, yVal);
        typedef typename superT<TypeParam>::type superT;
        superT refAccValFloat;

        refAccValFloat = RefALU::min(superT(*xVal.GetAcc<TypeParam>()), superT(*yVal.GetAcc<TypeParam>()));
        EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)refAccValFloat);

        /* test for vector of NEAT accurate */
        NEATVector testVec = NEATALU::min<TypeParam>(x, y);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::min(superT(*x[i].GetAcc<TypeParam>()), superT(*y[i].GetAcc<TypeParam>()));
            EXPECT_TRUE(testVec[i].IsAcc() && *testVec[i].GetAcc<TypeParam>() == (TypeParam)refAccValFloat);
        }

        // Test on interval values.
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
            y[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i], this->Arg2Max[testIdx*this->vectorWidth+i]);        }

        /* test for single interval NEAT value */
        xVal = x[0];
        yVal = y[0];

        testVal = NEATALU::min<TypeParam>(xVal, yVal);
        NEATValue refOut = min_ref<TypeParam>(xVal, yVal);

        EXPECT_TRUE(NEATValue::areEqual<TypeParam>(refOut,testVal));

        NEATVector refVec(this->currWidth);

        /* test for vector of NEAT intervals */
        testVec = NEATALU::min<TypeParam>(x, y);

        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refVec[i] = NEATValue(min_ref<TypeParam>(x[i], y[i]));
            EXPECT_TRUE(NEATValue::areEqual<TypeParam>(refVec[i],testVec[i]));
        }

        /* test for vector of NEAT interval and scalar */
        testVec = NEATALU::min<TypeParam>(x, yVal);

        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refVec[i] = NEATValue(min_ref<TypeParam>(x[i], yVal));
            EXPECT_TRUE(NEATValue::areEqual<TypeParam>(refVec[i],testVec[i]));
        }

    }
}


template<typename TypeParam>
static NEATValue max_ref(const NEATValue& xVal, const NEATValue& yVal)
{
    typedef typename superT<TypeParam>::type superT;
    superT xmin=*xVal.GetMin<TypeParam>();
    superT xmax=*xVal.GetMax<TypeParam>();
    superT ymin=*yVal.GetMin<TypeParam>();
    superT ymax=*yVal.GetMax<TypeParam>();

    bool intvOverlap = (ymin <= xmax && xmin <= ymax);
    NEATValue refOut;
    if(intvOverlap)
        refOut = NEATValue(
        RefALU::fmax(*xVal.GetMin<TypeParam>(), *yVal.GetMin<TypeParam>()),
        RefALU::fmax(*xVal.GetMax<TypeParam>(), *yVal.GetMax<TypeParam>()));
    else if(xmin > ymax)
        refOut = xVal;
    else if(ymin > xmax)
        refOut = yVal;
    else
        // should not be here
        throw Exception::IllegalFunctionCall("max_ref is called with wrong arguments");
    return NEATValue(refOut);
}

// TODO: add double tests
typedef ::testing::Types<float> FloatTypesCommon;
TYPED_TEST_CASE(NEATAluTypedCommon, FloatTypesCommon);

TYPED_TEST(NEATAluTypedCommon, max)
{
    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeParam>()){
        return;
    }

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeParam>(NEATALU::max<TypeParam>));
    EXPECT_TRUE(TestUnwritten<TypeParam>(NEATALU::max<TypeParam>));
    EXPECT_TRUE(TestAny<TypeParam>(NEATALU::max<TypeParam>));

    // Test special floating point values: NaNs.
    NEATValue nan = NEATValue::NaN<TypeParam>();
    NEATValue notSpecial = NEATValue(this->Arg1Min[0]);
    NEATValue testVal;
    for (uint32_t i = 0; i < (this->NUM_TESTS)*(this->vectorWidth); ++i)
    {
        NEATValue notNAN = NEATValue(this->Arg1Min[i]);
        testVal = NEATALU::max<TypeParam>(notNAN, notNAN);
        EXPECT_FALSE(testVal.IsNaN<TypeParam>());
    }
    testVal = NEATALU::max<TypeParam>(nan, notSpecial);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::max<TypeParam>(notSpecial, nan);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::max<TypeParam>(nan, nan);
    EXPECT_TRUE(testVal.IsAny());

    // Test +/-INFs
    NEATValue inf = NEATValue(Utils::GetPInf<TypeParam>());
    NEATValue mInf = NEATValue(Utils::GetNInf<TypeParam>());
    testVal = NEATALU::max<TypeParam>(notSpecial, inf);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::max<TypeParam>(inf, notSpecial);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::max<TypeParam>(inf, inf);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU ::max<TypeParam>(notSpecial, mInf);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::max<TypeParam>(mInf, notSpecial);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::max<TypeParam>(mInf, mInf);
    EXPECT_TRUE(testVal.IsAny());

    // manual tests
    NEATValue nx, ny, nres;
    TypeParam xmin, xmax, ymin, ymax;

    // overlap intervals
    xmin = -1.0; xmax = 6.0; ymin = 5.0; ymax = 15.0;
    nx.SetIntervalVal<TypeParam>(xmin, xmax);
    ny.SetIntervalVal<TypeParam>(ymin, ymax);
    nres = NEATALU::max<TypeParam>(nx, ny);
    EXPECT_TRUE(nres.IsInterval() &&
        *nres.GetMin<TypeParam>() == ymin && *nres.GetMax<TypeParam>() == ymax);

    // y interval contains x interval
    xmin = -1.0; xmax = 1.0; ymin = -5.0; ymax = 15.0;
    nx.SetIntervalVal<TypeParam>(xmin, xmax);
    ny.SetIntervalVal<TypeParam>(ymin, ymax);
    nres = NEATALU::max<TypeParam>(nx, ny);
    EXPECT_TRUE(nres.IsInterval() &&
        *nres.GetMin<TypeParam>() == xmin && *nres.GetMax<TypeParam>() == ymax);

    // x interval is less than y
    xmin = -1.0; xmax = 1.0; ymin = 5.0; ymax = 15.0;
    nx.SetIntervalVal<TypeParam>(xmin, xmax);
    ny.SetIntervalVal<TypeParam>(ymin, ymax);
    nres = NEATALU::max<TypeParam>(nx, ny);
    EXPECT_TRUE(nres.IsInterval() &&
        *nres.GetMin<TypeParam>() == ymin && *nres.GetMax<TypeParam>() == ymax);

    // y interval is smaller than x
    xmin = -1.0; xmax = 1.0; ymin = -15.0; ymax = -13.0;
    nx.SetIntervalVal<TypeParam>(xmin, xmax);
    ny.SetIntervalVal<TypeParam>(ymin, ymax);
    nres = NEATALU::max<TypeParam>(nx, ny);
    EXPECT_TRUE(nres.IsInterval() &&
        *nres.GetMin<TypeParam>() == xmin && *nres.GetMax<TypeParam>() == xmax);


    for(int testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        // Test on accurate values.
        NEATVector x(this->currWidth);
        NEATVector y(this->currWidth);

        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            y[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i]);
        }

        /* test for single accurate NEAT value*/
        NEATValue xVal = x[0];
        NEATValue yVal = y[0];

        testVal = NEATALU::max<TypeParam>(xVal, yVal);
        typedef typename superT<TypeParam>::type superT;
        superT refAccValFloat;

        refAccValFloat = RefALU::max(superT(*xVal.GetAcc<TypeParam>()), superT(*yVal.GetAcc<TypeParam>()));
        EXPECT_TRUE(testVal.IsAcc() && *testVal.GetAcc<TypeParam>() == (TypeParam)refAccValFloat);

        /* test for vector of NEAT accurate */
        NEATVector testVec = NEATALU::max<TypeParam>(x, y);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::max(superT(*x[i].GetAcc<TypeParam>()), superT(*y[i].GetAcc<TypeParam>()));
            EXPECT_TRUE(testVec[i].IsAcc() && *testVec[i].GetAcc<TypeParam>() == (TypeParam)refAccValFloat);
        }

        // Test on interval values.
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
            y[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i], this->Arg2Max[testIdx*this->vectorWidth+i]);        }

        /* test for single interval NEAT value */
        xVal = x[0];
        yVal = y[0];

        testVal = NEATALU::max<TypeParam>(xVal, yVal);
        NEATValue refOut = max_ref<TypeParam>(xVal, yVal);

        EXPECT_TRUE(NEATValue::areEqual<TypeParam>(refOut,testVal));

        NEATVector refVec(this->currWidth);

        /* test for vector of NEAT intervals */
        testVec = NEATALU::max<TypeParam>(x, y);

        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refVec[i] = NEATValue(max_ref<TypeParam>(x[i], y[i]));
            EXPECT_TRUE(NEATValue::areEqual<TypeParam>(refVec[i],testVec[i]));
        }

        /* test for vector of NEAT interval and scalar */
        testVec = NEATALU::max<TypeParam>(x, yVal);

        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refVec[i] = NEATValue(max_ref<TypeParam>(x[i], yVal));
            EXPECT_TRUE(NEATValue::areEqual<TypeParam>(refVec[i],testVec[i]));
        }

    }
}

template <typename T>
class NEATDegRadTest {
public:
    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 500;
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

    typedef typename superT<T>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&);
    typedef sT (*RefFuncP) (const sT&);

    NEATDegRadTest(double low = 0.0, double high = 1.0)
    {
        currWidth = VectorWidthWrapper::ValueOf(this->vectorWidth);
        dataTypeVal = GetDataTypeVal<T>();

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], currWidth, NUM_TESTS);
        GenerateRangedVectorsAutoSeed(dataTypeVal, &Arg2Min[0], currWidth, NUM_TESTS,T(low),T(high));
        GenerateRangedVectorsAutoSeed(dataTypeVal, &Arg2Max[0], currWidth, NUM_TESTS,T(low),T(high));

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
            if (Arg2Min[i] > Arg2Max[i]) std::swap(Arg2Min[i], Arg2Max[i]);
        }
    }


    void Test(NEATFuncP NEATFunc, NEATFuncVecP NEATFuncVec, RefFuncP RefFunc, float ulps)
    {
        // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
        EXPECT_TRUE(TestUnknown(NEATFunc));
        EXPECT_TRUE(TestUnwritten(NEATFunc));
        EXPECT_TRUE(TestAny(NEATFunc));
    
        // Test special floating point values: NaNs.
        NEATValue nan = NEATValue::NaN<T>();
        NEATValue notSpecial = NEATValue(Arg1Min[0]);
        NEATValue testVal;
        for (uint32_t i = 0; i < (NUM_TESTS)*(vectorWidth); ++i)
        {
            NEATValue notNAN = NEATValue(Arg1Min[i]);
            testVal = NEATFunc(notNAN);
            EXPECT_FALSE(testVal.IsNaN<T>());
        }
        testVal = NEATFunc(nan);
        EXPECT_TRUE(testVal.IsNaN<T>());
    
    
        for(uint32_t testIdx = 0; testIdx < NUM_TESTS; ++testIdx)
        {
            // Test on accurate values.
            NEATVector acc(currWidth);
    
            for(uint32_t i = 0; i < vectorWidth; ++i)
                acc[i] = NEATValue(Arg1Min[testIdx*vectorWidth+i]);
    
            /* test for single accurate NEAT value*/
            NEATValue accVal = acc[0];
    
            testVal = NEATFunc(accVal);
            sT refAccValFloat;
            sT refIntValMin, refIntValMax;
    
            refAccValFloat = RefFunc(sT(*accVal.GetAcc<T>()));
            EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat,testVal,ulps));
    
            /* test for vector of NEAT accurate */
            NEATVector testVec = NEATFuncVec(acc);
            for(uint32_t i = 0; i < vectorWidth; ++i) {
                refAccValFloat = RefFunc(sT(*acc[i].GetAcc<T>()));
                EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat,testVec[i],ulps));
            }
    
            // Test on interval values.
            NEATVector interval(currWidth);
            for(uint32_t i = 0; i < vectorWidth; ++i)
                interval[i] = NEATValue(Arg1Min[testIdx*vectorWidth+i], Arg1Max[testIdx*vectorWidth+i]);
    
            /* test for single interval NEAT value */
    
            NEATValue intVal = interval[0];
            testVal = NEATFunc(intVal);
    
            refIntValMin = RefFunc(sT(*intVal.GetMin<T>()));
            refIntValMax = RefFunc(sT(*intVal.GetMax<T>()));
            EXPECT_TRUE(TestIntExpanded<sT>(refIntValMin,refIntValMax,testVal,ulps));
    
            /* test for vector of NEAT intervals */
            testVec = NEATFuncVec(interval);
            for(uint32_t i = 0; i < vectorWidth; ++i) {
                refIntValMin = RefFunc(sT(*interval[i].GetMin<T>()));
                refIntValMax = RefFunc(sT(*interval[i].GetMax<T>()));
                EXPECT_TRUE(TestIntExpanded<sT>(refIntValMin,refIntValMax,testVec[i],ulps));
            }
    
            /* test for vector of NEAT accurate ranged */
            for(uint32_t i = 0; i < vectorWidth; ++i)
                acc[i] = NEATValue(Arg2Min[testIdx*vectorWidth+i]);
    
            testVec = NEATFuncVec(acc);
            for(uint32_t i = 0; i < vectorWidth; ++i) {
                refAccValFloat = RefFunc(sT(*acc[i].GetAcc<T>()));
                EXPECT_TRUE(TestAccExpanded<sT>(refAccValFloat,testVec[i],ulps));
            }
    
            /* test for vector of NEAT intervals ranged */
            for(uint32_t i = 0; i < vectorWidth; ++i)
                interval[i] = NEATValue(Arg2Min[testIdx*vectorWidth+i], Arg2Max[testIdx*vectorWidth+i]);
    
            testVec = NEATFuncVec(interval);
            for(uint32_t i = 0; i < vectorWidth; ++i) {
                refIntValMin = RefFunc(sT(*interval[i].GetMin<T>()));
                refIntValMax = RefFunc(sT(*interval[i].GetMax<T>()));
                EXPECT_TRUE(TestIntExpanded<sT>(refIntValMin,refIntValMax,testVec[i],ulps));
            }
        }
    } // void TestExp(NEATFuncP NEATFunc, NEATFuncVecP NEATFuncVec, RefFuncP RefFunc, float ulps)

};


template <typename T>
class NEATDegRadTestRun : public ALUTest {
};

TYPED_TEST_CASE(NEATDegRadTestRun, FloatTypesCommon2);
TYPED_TEST(NEATDegRadTestRun, radians)
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

    NEATFuncP NEATFunc = &NEATALU::radians<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::radians<TypeP>;
    RefFuncP RefFunc = &RefALU::radians<sT>;

    NEATDegRadTest<TypeP> radiansTest(0.0,360.0); // from 0 degrees to 360 degrees

    radiansTest.Test(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::RADIANS_ERROR));
}

TYPED_TEST(NEATDegRadTestRun, degrees)
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

    NEATFuncP NEATFunc = &NEATALU::degrees<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::degrees<TypeP>;
    RefFuncP RefFunc = &RefALU::degrees<sT>;

    NEATDegRadTest<TypeP> degreesTest(0.0, (2.0*M_PIL)); // from 0 to 2*pi radians

    degreesTest.Test(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::DEGREES_ERROR));
}

template <typename T>
class NEATCommonTestOneArg : public ALUTest {
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

    NEATCommonTestOneArg()
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
class NEATCommonTestThreeArgs : public ALUTest {
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

    NEATCommonTestThreeArgs()
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

TYPED_TEST_CASE(NEATCommonTestOneArg, FloatTypesCommon2);
TYPED_TEST_CASE(NEATCommonTestThreeArgs, FloatTypesCommon2);

template <typename T>
static void testSignResult (NEATValue xVal, NEATValue testVal)
{
        // refernce functionreturns  1.0 if x > 0, -0.0 if x = -0.0, 
        // +0.0 if x = +0.0, or –1.0 if x < 0.
        T min = *xVal.GetMin<T>();
        T max = *xVal.GetMax<T>();

        if( Utils::gt(min,T(0.0)) ) {
            EXPECT_TRUE(testVal.IsAcc());
            EXPECT_TRUE(Utils::eq(T(1.0), *testVal.GetAcc<T>()));
        } else if( Utils::lt(max,T(-0.0)) ) {
            EXPECT_TRUE(testVal.IsAcc());
            EXPECT_TRUE(Utils::eq(T(-1.0), *testVal.GetAcc<T>()));
        } else if( Utils::eq(min,T(-0.0)) && Utils::eq(max,T(+0.0)) ) {
            EXPECT_TRUE(Utils::eq(T(-0.0), *testVal.GetMin<T>()));
            EXPECT_TRUE(Utils::eq(T(0.0), *testVal.GetMax<T>()));
        } else 
            EXPECT_TRUE(testVal.IsUnknown());
}

TYPED_TEST(NEATCommonTestOneArg, sign)
{
    typedef typename TypeParam::Type TypeP;
    /// If your test doesn't currently support doubles, use SkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown(NEATALU::sign<TypeP>));
    EXPECT_TRUE(TestUnwritten(NEATALU::sign<TypeP>));
    EXPECT_TRUE(TestAny(NEATALU::sign<TypeP>));

    // Returns 0.0 if input is a NaN
    EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::sign<TypeP>, Utils::GetNaN<TypeP>(), TypeP(0.0)));

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        NEATValue testVal;
        NEATVector x(Width);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
        }

        // Test accurate value
        TypeP refAccValFloat = RefALU::sign(*x[0].GetAcc<TypeP>());
        EXPECT_TRUE(TestPreciseRes<TypeP>(NEATALU::sign<TypeP>, *x[0].GetAcc<TypeP>(), refAccValFloat));

        // Test accurate vector
        NEATVector testVec = NEATALU::sign<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            refAccValFloat = RefALU::sign(*x[i].GetAcc<TypeP>());
            EXPECT_TRUE(testVec[i].IsAcc());
            if (testVec[i].IsAcc())
                EXPECT_TRUE(Utils::eq(refAccValFloat, *testVec[i].GetAcc<TypeP>()));
        }

        // Test interval value
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
        }

        testVal = NEATALU::sign<TypeP>(x[0]);
        testSignResult<TypeP>( x[0], testVal);

        // Test interval vector
        testVec = NEATALU::sign<TypeP>(x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            testSignResult<TypeP>( x[i], testVec[i]);
        }
    }
}


template <typename T>
static void testSmoothstepAccResult (NEATValue edge0Val, NEATValue edge1Val, NEATValue xVal, NEATValue testVal)
{
    if ( *edge0Val.GetAcc<T>() >= *edge1Val.GetAcc<T>() ) {
        EXPECT_TRUE(testVal.IsAny());
    } else {
        EXPECT_TRUE(testVal.IsAcc());
        if (testVal.IsAcc()) {
            // *edge0Val.GetAcc<T>() and *edge1Val.GetAcc<T>() have alredy been flushed, so flush *xVal.GetAcc<T>() only
            T refAcc = RefALU::flush(RefALU::smoothstep(*edge0Val.GetAcc<T>(), *edge1Val.GetAcc<T>(), RefALU::flush(*xVal.GetAcc<T>())));
            EXPECT_TRUE(*testVal.GetAcc<T>() == refAcc);
        }
    }
}

template <typename T>
static void testSmoothstepIntervalResult (NEATValue edge0Val, NEATValue edge1Val, NEATValue xVal, NEATValue testVal)
{
    const int num = 50;
    // the ranges are overlapped, results are undefined, because edge0 >= edge1
    if ( *edge0Val.GetMax<T>() >= *edge1Val.GetMin<T>() ) {
        EXPECT_TRUE(testVal.IsAny());
    } else {
        DataTypeVal dataTypeVal = GetDataTypeVal<T>();
        T arrX[num], arrEdge0[num], arrEdge1[num];
        
        GenerateRangedVectorsAutoSeed(dataTypeVal, &arrEdge0[0], V1, num, *edge0Val.GetMin<T>(), *edge0Val.GetMax<T>());
        GenerateRangedVectorsAutoSeed(dataTypeVal, &arrEdge1[0], V1, num, *edge1Val.GetMin<T>(), *edge1Val.GetMax<T>());
        GenerateRangedVectorsAutoSeed(dataTypeVal, &arrX[0], V1, num, *xVal.GetMin<T>(), *xVal.GetMax<T>());

        for(int i = 0;i<num;i++) {
            T refVal = RefALU::flush(RefALU::smoothstep(RefALU::flush(arrEdge0[i]), RefALU::flush(arrEdge1[i]), RefALU::flush(arrX[i])) );
            EXPECT_TRUE (refVal >= *testVal.GetMin<T>() && refVal <= *testVal.GetMax<T>());
        }
    }
}

TYPED_TEST(NEATCommonTestThreeArgs, smoothstep)
{
    typedef typename TypeParam::Type TypeP;
    /// If your test doesn't currently support doubles, use SkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.
    EXPECT_TRUE(TestUnknown<TypeP>(NEATALU::smoothstep<TypeP>));
    EXPECT_TRUE(TestUnwritten<TypeP>(NEATALU::smoothstep<TypeP>));
    EXPECT_TRUE(TestAny<TypeP>(NEATALU::smoothstep<TypeP>));

    // Returns ANY if input is a NaN
    NEATValue nan = NEATValue::NaN<TypeP>();
    NEATValue notNAN = NEATValue(this->Arg1Min[0]);

    NEATValue testVal = NEATALU::smoothstep<TypeP>(nan,notNAN,notNAN);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::smoothstep<TypeP>(notNAN,nan,notNAN);
    EXPECT_TRUE(testVal.IsAny());
    testVal = NEATALU::smoothstep<TypeP>(notNAN,notNAN,nan);
    EXPECT_TRUE(testVal.IsAny());

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
         // Test on accurate values.
        NEATVector x(Width);
        NEATVector edge0(Width);
        NEATVector edge1(Width);

        // accurate values tests
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            edge0[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i]);
            edge1[i] = NEATValue(this->Arg3Min[testIdx*this->vectorWidth+i]);
        }
        NEATValue xVal = x[0];
        NEATValue edge0Val = edge0[0];
        NEATValue edge1Val = edge1[0];

        // edges and x are scalars
        testVal = NEATALU::smoothstep<TypeP>(edge0Val,edge1Val,xVal);
        testSmoothstepAccResult<TypeP>(edge0Val,edge1Val,xVal,testVal);

        // edges are scalars, x is a vector
        NEATVector testVec = NEATALU::smoothstep<TypeP>(edge0Val,edge1Val,x);        
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            testSmoothstepAccResult<TypeP>(edge0Val,edge1Val,x[i],testVec[i]);
        }

        // edges and x are vectors
        testVec = NEATALU::smoothstep<TypeP>(edge0,edge1,x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            testSmoothstepAccResult<TypeP>(edge0[i],edge1[i],x[i],testVec[i]);
        }

        // intervals tests
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            x[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i],this->Arg1Max[testIdx*this->vectorWidth+i]);
            edge0[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i],this->Arg2Max[testIdx*this->vectorWidth+i]);
            edge1[i] = NEATValue(this->Arg3Min[testIdx*this->vectorWidth+i],this->Arg3Max[testIdx*this->vectorWidth+i]);
        }
        xVal = x[0];
        edge0Val = edge0[0];
        edge1Val = edge1[0];
        // edges and x are scalars
        testVal = NEATALU::smoothstep<TypeP>(edge0Val,edge1Val,xVal);
        testSmoothstepIntervalResult<TypeP>(edge0Val,edge1Val,xVal,testVal);

        // edges are scalars, x is a vector
        testVec = NEATALU::smoothstep<TypeP>(edge0Val,edge1Val,x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            testSmoothstepIntervalResult<TypeP>(edge0Val,edge1Val,x[i],testVec[i]);
        }

         // edges and x are vectors
        testVec = NEATALU::smoothstep<TypeP>(edge0,edge1,x);
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            testSmoothstepIntervalResult<TypeP>(edge0[i],edge1[i],x[i],testVec[i]);
        }

    }
}