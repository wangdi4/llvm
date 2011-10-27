/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  NEATALUTest.cpp

\*****************************************************************************/

#include "BufferContainerList.h"
#include "DataGenerator.h"
#include "DGHelper.h"

#include "FloatOperations.h"
#include "NEATVector.h"
#include "RefALU.h"
#include "NEATALU.h"
#include <gtest/gtest.h>
#include "NEATValue.h"
#include <fstream>

#include "NEATALUUtils.h"
#include "ALUTest.h"

using namespace Validation;

/// Use google type tests feature
template <typename T>
class NEATAluTyped : public ::testing::Test {};
// TODO: add double tests support
typedef ::testing::Types<float> FloatTypes;
TYPED_TEST_CASE(NEATAluTyped, FloatTypes);

typedef NEATVector (*NEATVectorBinaryOp)(const NEATVector&, const NEATVector&);

template<class T>
class sT{
};

template<>
class sT<float>{
public:
       typedef double type;
};

template<>
class sT<double>{
public:
       typedef long double type ;
};

template<class T>
class dT{
};

template<>
class dT<double>{
public:
       typedef float type;
};

template<>
class dT<long double>{
public:
       typedef double type ;
};


TEST(NEATAlu, NEATRef)
{
    EXPECT_EQ(0.6f, RefALU::add<float>(0.2f, 0.4f));
    EXPECT_EQ(8.0f, RefALU::mul<float>(2.0f, 4.0f));
    EXPECT_EQ(-0.2f, RefALU::sub<float>(0.2f, 0.4f));
}

/// Tests given ALU operation. Operation should has range and domain being equal to
/// whole set of real numbers.
template<typename TypeParam>
void BasicTest(NEATScalarBinaryOp ScalarOp,NEATVectorBinaryOp VectorOp,
               TypeParam (RefFunc)(const TypeParam&, const TypeParam&), int ulpsCount)
{
    typedef typename sT<TypeParam>::type SuperT;

    // Test corner cases first
    NEATValue unkn = NEATValue(NEATValue::UNKNOWN);
    NEATValue unwr = NEATValue(NEATValue::UNWRITTEN);
    NEATValue any  = NEATValue(NEATValue::ANY);
    NEATValue nan  = NEATValue::NaN<TypeParam>();
    NEATValue tmp;

    tmp = ScalarOp(unkn, unkn);
    EXPECT_TRUE(tmp.IsUnknown());
    tmp = ScalarOp(unkn, unwr);
    EXPECT_TRUE(tmp.IsUnknown());
    tmp = ScalarOp(unwr, unwr);
    EXPECT_TRUE(tmp.IsUnknown());
    tmp = ScalarOp(nan, any);
    EXPECT_TRUE(tmp.IsNaN<TypeParam>());
    tmp = ScalarOp(any, any);
    EXPECT_TRUE(tmp.IsAny());
    tmp = ScalarOp(nan, nan);
    EXPECT_TRUE(tmp.IsNaN<TypeParam>());

    // Done with corner cases. Now go to testing scalar and vector functions
    const int NUM_TESTS = 500;
    const VectorWidth VECTOR_WIDTH = V8;
    const uint32_t VECTOR_SIZE = 8;
    VectorWidthWrapper wrap(VECTOR_WIDTH);

    DataTypeVal dataTypeVal;
    if(sizeof(TypeParam) == sizeof(float))
    {
        dataTypeVal = F32;
    } else if(sizeof(TypeParam) == sizeof(double))
    {
        dataTypeVal = F64;
    } else
    {
        GTEST_FAIL();
    }

    TypeParam aMin[NUM_TESTS*8];
    TypeParam aMax[NUM_TESTS*8];

    TypeParam bMin[NUM_TESTS*8];
    TypeParam bMax[NUM_TESTS*8];

    uint64_t bSeed, aSeed;

    aSeed = GenerateRandomVectors(dataTypeVal, &aMin[0], VECTOR_WIDTH, NUM_TESTS);
    GenerateRandomVectorsSeed(dataTypeVal, &aMax[0], VECTOR_WIDTH, NUM_TESTS,
                              (aSeed+18426)); // add some fixed arbitrary number

    bSeed = GenerateRandomVectors(dataTypeVal, &bMin[0], VECTOR_WIDTH, NUM_TESTS);
    GenerateRandomVectorsSeed(dataTypeVal, &bMax[0], VECTOR_WIDTH, NUM_TESTS,
                              (bSeed+18426)); // add some fixed arbitrary number

    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        // Test result for given operation of accurate values or intervals
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        SuperT minRef[VECTOR_SIZE];
        SuperT maxRef[VECTOR_SIZE];

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            // Test for accurate operation
            uint32_t idx = testIdx * wrap.GetSize() + i;

            // First test addition of accurate values
            NEATValue aAcc = NEATValue((TypeParam)aMin[idx]);
            NEATValue bAcc = NEATValue((TypeParam)bMin[idx]);

            NEATValue accRes = ScalarOp(aAcc,bAcc);
            SuperT refRes = RefFunc(aMin[idx], bMin[idx]);
            // if operation is accurate
            if(ulpsCount == 0)
            {
                EXPECT_EQ(accRes.GetStatus(), NEATValue::ACCURATE)<<"aSeed = "<<aSeed<<" bSeed = "<<bSeed;
                EXPECT_EQ(*accRes.GetAcc<TypeParam>(), refRes)<<"aSeed = "<<aSeed<<" bSeed = "<<bSeed;
            }
            else
            // Operation has some inaccuracy
            {
                EXPECT_EQ(accRes.GetStatus(), NEATValue::INTERVAL)<<"aSeed = "<<aSeed<<"bSeed = "<<bSeed;
                TypeParam aluMin = *accRes.GetMin<TypeParam>();
                TypeParam aluMax = *accRes.GetMax<TypeParam>();
                float diff1 = Utils::ulpsDiff(refRes, aluMin);
                float diff2 = Utils::ulpsDiff(refRes, aluMax);
                // Assume that aluMin can probably become equal to accRes if
                // We expand by one ulp
                EXPECT_LE(aluMin, refRes);
                EXPECT_GE(aluMax, refRes);
                EXPECT_LE(diff1, ulpsCount);
                EXPECT_LE(diff2, ulpsCount);
                EXPECT_GT(diff1, ulpsCount - 1);
                EXPECT_GT(diff2, ulpsCount - 1);
            }

            // Ensure that minimums won't exceed maximums
            if(aMin[idx] > aMax[idx])
                std::swap(aMin[idx], aMax[idx]);
            if(bMin[idx] > bMax[idx])
                std::swap(bMin[idx], bMax[idx]);
            // Test interval
            NEATValue a = NEATValue(aMin[idx], aMax[idx]);
            NEATValue b = NEATValue(bMin[idx], bMax[idx]);
            // Calc reference interval result
            const uint32_t RES_COUNT = 4;
            SuperT allResults[RES_COUNT] = {RefFunc((SuperT)aMin[idx], (SuperT)bMin[idx]),
                                            RefFunc((SuperT)aMax[idx], (SuperT)bMin[idx]),
                                            RefFunc((SuperT)aMin[idx], (SuperT)bMax[idx]),
                                            RefFunc((SuperT)aMax[idx], (SuperT)bMax[idx])};
            minRef[i] = FindMin(allResults, RES_COUNT);
            maxRef[i] = FindMax(allResults, RES_COUNT);

            NEATValue res = ScalarOp(a,b);
            bool passed = false;
            switch(res.GetStatus())
            {
            case NEATValue::INTERVAL:
                passed = TestIntExpanded<SuperT>(minRef[i], maxRef[i], res, NEATALU::ADD_ERROR);
                break;
            case NEATValue::ACCURATE:
                if(minRef[i] != maxRef[i])
                    passed = false;
                else
                {
                    TypeParam acc = *res.GetAcc<TypeParam>();
                    passed = (minRef[i] == maxRef[i]) && (minRef[i] == acc);
                }
                break;
            default:
                passed = false;
                break;
            }
            EXPECT_TRUE(passed)<<"aSeed = "<<aSeed<<" bSeed = "<<bSeed;
        }
        // Test vector
        NEATVector a(VECTOR_WIDTH);
        NEATVector b(VECTOR_WIDTH);
        for(uint32_t i = 0; i<VECTOR_SIZE; i++)
        {
            uint32_t idx = testIdx * VECTOR_SIZE + i;
            a[i] = NEATValue(aMin[idx], aMax[idx]);
            b[i] = NEATValue(bMin[idx], bMax[idx]);
        }
        // Call NEAT
        NEATVector res = VectorOp(a,b);
        // Verify the result
        for(uint32_t i = 0; i<VECTOR_SIZE; i++)
        {
            bool passed = TestIntExpanded<SuperT>(minRef[i], maxRef[i], res[i], ulpsCount);
            EXPECT_TRUE(passed)<<"aSeed = "<<aSeed<<" bSeed = "<<bSeed;
        }
    }
}

/// Tests NEAT addition including vector function and corner values
/// Two ranges tested. First for the whole interval, second from -10 to 10
TYPED_TEST(NEATAluTyped, AddSubMul)
{
    bool typeSupported = true;
    if(sizeof(TypeParam) == sizeof(double)) {
        typeSupported = sizeof(double) < sizeof(long double);
        printf("WARNING: test size of long double is equal to size of double\n");
    }

    if(typeSupported)
    {
        BasicTest<TypeParam>(NEATALU::add<TypeParam>, NEATALU::add<TypeParam>, RefALU::add<TypeParam>, NEATALU::ADD_ERROR);
        BasicTest<TypeParam>(NEATALU::mul<TypeParam>, NEATALU::mul<TypeParam>, RefALU::mul<TypeParam>, NEATALU::MUL_ERROR);
        BasicTest<TypeParam>(NEATALU::sub<TypeParam>, NEATALU::sub<TypeParam>, RefALU::sub<TypeParam>, NEATALU::SUB_ERROR);

        // Separately check NaN cases for mul:
        NEATValue zero((TypeParam)0);
        NEATValue zeroNeg((TypeParam)-0);
        NEATValue pInf(Utils::GetPInf<TypeParam>());
        NEATValue nInf(Utils::GetNInf<TypeParam>());
        NEATValue tmp;
        // Test NaNs
        tmp = NEATALU::mul<TypeParam>(zero, pInf);
        EXPECT_TRUE(tmp.IsNaN<TypeParam>());
        tmp = NEATALU::mul<TypeParam>(zero, nInf);
        EXPECT_TRUE(tmp.IsNaN<TypeParam>());
        tmp = NEATALU::mul<TypeParam>(zeroNeg, pInf);
        EXPECT_TRUE(tmp.IsNaN<TypeParam>());
        tmp = NEATALU::mul<TypeParam>(zeroNeg, nInf);
        EXPECT_TRUE(tmp.IsNaN<TypeParam>());
    }
}


TYPED_TEST(NEATAluTyped, fmod)
{
    const int NUM_TESTS = 13;
    {
        TypeParam data[NUM_TESTS][4] = {\
            {1.0f, 3.0f,      1.0f, 3.0f},\
            {11.9f, 12.1f,    1.8f, 2.2f},\
            {10.9f, 11.1f,    1.9f, 2.1f},\
            {11.9f, 12.1f,    1.3f, 2.2f},\
            {11.9f, 12.1f,    1.6f, 2.0f},\
            {-1.0f, 1.0f,     1.0f, 3.0f},\
            {-3.0f, -1.0f,    1.0f, 3.0f},\
            {-3.0f, 1.0f,     1.0f, 2.0f},\
            {-1.0f, 3.0f,     1.0f, 2.0f},\
            {-10.0f, -1.0f,   1.0f, 2.0f},\
            {-5.0f, -2.0f,    0.5f, 2.0f},\
            {1.0f, 3.0f,      -3.0f, -1.0f},\
            {5.0f, 10.0f,     -5.0f, -3.0f}
        };
        for(int i = 0; i<NUM_TESTS; i++)
        {
            NEATValue dividend(data[i][0], data[i][1]);
            NEATValue divisor(data[i][2], data[i][3]);
            NEATValue frem = NEATALU::fmod<TypeParam>(dividend, divisor);
            // check interval if not unknown
            bool passed = true;
            if(frem.GetStatus() != NEATValue::UNKNOWN)
            {
                static int STEPS_COUNT = 10;
                TypeParam divisorStep = (*divisor.GetMax<TypeParam>() - *divisor.GetMin<TypeParam>()) / STEPS_COUNT;
                TypeParam dividendStep = (*dividend.GetMax<TypeParam>() - *dividend.GetMin<TypeParam>()) / STEPS_COUNT;
                for(TypeParam divisorIdx = *divisor.GetMin<TypeParam>(); divisorIdx < *divisor.GetMax<TypeParam>(); divisorIdx += divisorStep)
                {
                    for(TypeParam dividendIdx = *dividend.GetMin<TypeParam>(); dividendIdx<*dividend.GetMax<TypeParam>(); dividendIdx += dividendStep)
                    {
                        TypeParam res = fmod(dividendIdx, divisorIdx);
                        if(Utils::IsInf(res) || Utils::IsNaN(res))
                            passed = false;
                        TypeParam min = *frem.GetMin<TypeParam>();
                        TypeParam max = *frem.GetMax<TypeParam>();
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
        const int UNKNOWN_TESTS_COUNT = 8;
        // test unknown cases
         TypeParam data[UNKNOWN_TESTS_COUNT][4] = {\
            {34.0f, 36.0f,      4.9f, 5.1f},\
            {34.0f, 36.0f,      5.0f, 5.0f},\
            {11.9f, 12.1f,      2.0f, 2.0f},\
            {11.9f, 12.1f,      2.0f, 2.1f},\
            {11.9f, 12.1f,      1.9f, 2.0f},\
            {2.0f, 7.0f,      -1.0f, 1.0f},\
            {-1.0f , 1.0f,      0.0f, 1.0f},\
            {-1.0f, 1.0f,       -0.0f, 1.0f}
        };
        for(int i = 0; i<UNKNOWN_TESTS_COUNT; i++)
        {
            NEATValue dividend(data[i][0], data[i][1]);
            NEATValue divisor(data[i][2], data[i][3]);
            NEATValue frem = NEATALU::fmod<TypeParam>(dividend, divisor);
            EXPECT_EQ(NEATValue::UNKNOWN, frem.GetStatus());
        }
    }
}

/// Tests NEATAlu::fcmp scalar and vector functionality with the following corner cases:
/// 1. Positive and negative results for each type of comparison, including ordered and unordered - without NaNs
/// 2. Test NaNs for each type of comparison - ordered and unordered
/// 3. UNKNOWN answers for each type of comparison
TYPED_TEST(NEATAluTyped, fcmp)
{
    const int NUM_TESTS = 16;
    // We have five completely different operations that involves comparison: ==, <, <=, >, >=, !=
    // Therefore we have only 6 operations to implement directly
    const int UNIQUE_CMP_TYPES = 6;

    TypeParam less[2] = {(TypeParam)0.1, (TypeParam) 0.2};
    TypeParam middle[2] = {(TypeParam)0.3, (TypeParam) 0.4};
    TypeParam greater[2] = {(TypeParam)0.5, (TypeParam) 0.6};

    CmpType cmpType[NUM_TESTS] = {CMP_OEQ, CMP_OGT, CMP_OGE, CMP_OLT, CMP_OLE, CMP_ONE, CMP_ORD,
                                  CMP_UEQ, CMP_UGT, CMP_UGE, CMP_ULT, CMP_ULE, CMP_UNE, CMP_UNO, CMP_FALSE, CMP_TRUE};
    // For each type of comparison true and false return values should be tested
    // Ordered/unordered doesn't matter in this case
    // Test the following comparison types:
    TypeParam cmpWith[UNIQUE_CMP_TYPES][2] =       {{1.0, 1.0}, // ==
                                                   {middle[0], middle[1]}, {middle[0], middle[1]}, // >   >=
                                                   {middle[0], middle[1]}, {middle[0], middle[1]},// < <=
                                                   {1.0,1.0}}; // !=

    TypeParam negative[UNIQUE_CMP_TYPES][2] =      {{2.0, 2.0}, // ==
                                                   {greater[0], greater[1]}, {greater[0], greater[1]}, // > >=
                                                   {less[0], less[1]}, {less[0], less[1]},  // < <=
                                                   {1.0, 1.0}}; // !=

    // For each type of comparison true and false return values should be tested

    TypeParam positive[UNIQUE_CMP_TYPES][2] =      {{1.0, 1.0}, // ==
                                                   {less[0], less[1]}, {less[0], less[1]}, // > >=
                                                   {greater[0], greater[1]}, {greater[0], greater[1]},  // < <=
                                                   {2.0, 2.0}}; // !=

    for(int i = 0; i<UNIQUE_CMP_TYPES; i++)
    {
        {
            // Tests returning true for ordinary comparison functions
            NEATValue   with(cmpWith[i][0], cmpWith[i][1]);
            NEATValue   pos(positive[i][0], positive[i][1]);
            NEATValue   neg(negative[i][0], negative[i][1]);
            NEATValue trueAns1  = NEATALU::fcmp<TypeParam>(with, pos, cmpType[i]);
            NEATValue trueAns2  = NEATALU::fcmp<TypeParam>(with, pos, cmpType[i + UNIQUE_CMP_TYPES + 1]); // Add 1 here for pure ordered/unordered comparison test
            NEATValue falseAns1 = NEATALU::fcmp<TypeParam>(with, neg, cmpType[i]);
            NEATValue falseAns2 = NEATALU::fcmp<TypeParam>(with, neg, cmpType[i + UNIQUE_CMP_TYPES + 1]); // Add 1 here for pure ordered/unordered comparison test
            EXPECT_EQ(NEATValue::ACCURATE, trueAns1.GetStatus());
            EXPECT_EQ(NEATValue::ACCURATE, trueAns2.GetStatus());
            EXPECT_EQ(NEATValue::ACCURATE, falseAns1.GetStatus());
            EXPECT_EQ(NEATValue::ACCURATE, falseAns2.GetStatus());
            if(trueAns1.GetStatus() == NEATValue::ACCURATE)
                EXPECT_TRUE(*trueAns1.GetAcc<bool>());
            if(trueAns2.GetStatus() == NEATValue::ACCURATE)
                EXPECT_TRUE(*trueAns2.GetAcc<bool>());
            if(falseAns1.GetStatus() == NEATValue::ACCURATE)
                EXPECT_FALSE(*falseAns1.GetAcc<bool>());
            if(falseAns2.GetStatus() == NEATValue::ACCURATE)
                EXPECT_FALSE(*falseAns2.GetAcc<bool>());
        }
    }

    // True/false comparison functions:
    NEATValue tmp1((TypeParam)0.0);
    NEATValue tmp2((TypeParam)0.0);
    NEATValue trueRes  = NEATALU::fcmp<TypeParam>(tmp1, tmp2, CMP_TRUE);
    NEATValue falseRes = NEATALU::fcmp<TypeParam>(tmp1, tmp2, CMP_FALSE);
    EXPECT_TRUE(*trueRes.GetAcc<bool>());
    EXPECT_FALSE(*falseRes.GetAcc<bool>());

    // Ordered/unordered comparison functions with NaNs:
    NEATValue nanVal = NEATValue::NaN<TypeParam>();
    for(int i = 0; i<UNIQUE_CMP_TYPES + 1; i++)
    {
        // ordered comparison
        NEATValue ord1 = NEATALU::fcmp<TypeParam>(nanVal, tmp1, cmpType[i]);
        NEATValue ord2 = NEATALU::fcmp<TypeParam>(tmp1, nanVal, cmpType[i]);
        NEATValue ord3 = NEATALU::fcmp<TypeParam>(nanVal, nanVal, cmpType[i]);
        // unordered comparison
        NEATValue uno1 = NEATALU::fcmp<TypeParam>(nanVal, tmp1, cmpType[i + UNIQUE_CMP_TYPES + 1]);
        NEATValue uno2 = NEATALU::fcmp<TypeParam>(tmp1, nanVal, cmpType[i + UNIQUE_CMP_TYPES + 1]);
        NEATValue uno3 = NEATALU::fcmp<TypeParam>(nanVal, nanVal, cmpType[i + UNIQUE_CMP_TYPES + 1]);

        EXPECT_FALSE(*ord1.GetAcc<bool>());
        EXPECT_FALSE(*ord2.GetAcc<bool>());
        EXPECT_FALSE(*ord3.GetAcc<bool>());
        EXPECT_TRUE(*uno1.GetAcc<bool>());
        EXPECT_TRUE(*uno2.GetAcc<bool>());
        EXPECT_TRUE(*uno3.GetAcc<bool>());
    }
    // UNKNOWN cases for each type of comparison

    // For each type of comparison true and false return values should be tested
    // Ordered/unordered doesn't matter in this case
    // Test the following comparison types:
    for(int i = 0; i<UNIQUE_CMP_TYPES; i++)
    {
        {
            // Tests returning true for ordinary comparison functions
            NEATValue   cmpWith(middle[0], middle[1]);
            NEATValue unknown1 = NEATALU::fcmp<TypeParam>(cmpWith, cmpWith, cmpType[i]);
            NEATValue unknown2  = NEATALU::fcmp<TypeParam>(cmpWith, cmpWith, cmpType[i + UNIQUE_CMP_TYPES + 1]); // Add 1 here for pure ordered/unordered comparison test
            EXPECT_EQ(NEATValue::UNKNOWN, unknown1.GetStatus());
            EXPECT_EQ(NEATValue::UNKNOWN, unknown2.GetStatus());
        }
    }

    TypeParam tp = 1;
    tp++;
    NEATValue       val1((TypeParam)0.1, (TypeParam)0.2);
    NEATValue       val2((TypeParam)0.1, (TypeParam)0.2);
    NEATValue ans = NEATALU::fcmp<TypeParam>(val1, val2, CMP_OEQ);
    EXPECT_EQ(ans.GetStatus(), NEATValue::UNKNOWN);
}



/* test for extractelement instruction */
TYPED_TEST(NEATAluTyped, extractelement)
{
    const int WIDTHS_COUNT = 6;
    const int NUM_TESTS = 100;
    VectorWidth widths[] = {V1,V2,V3,V4,V8,V16};

    TypeParam firstFloat[NUM_TESTS][16];
    TypeParam secondFloat[NUM_TESTS][16];

    if(sizeof(TypeParam) == sizeof(float)) {
        GenerateRandomVectors(F32, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F32, &secondFloat[0][0], V16, NUM_TESTS);
    } else if(sizeof(TypeParam) == sizeof(double)){
        GenerateRandomVectors(F64, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F64, &secondFloat[0][0], V16, NUM_TESTS);
    } else {
        GTEST_FAIL();
    }


    for(int widthIdx = 0; widthIdx < WIDTHS_COUNT; widthIdx ++)
    {
        VectorWidth curWidth = widths[widthIdx];
        VectorWidthWrapper wrap(curWidth);
        for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            NEATVector accurate(curWidth);
            NEATVector interval(curWidth);

            for(uint32_t i = 0; i<wrap.GetSize(); i++)
            {
                accurate[i] = NEATValue(firstFloat[testIdx][i]);
                if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                    interval[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                else
                    interval[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
            }

            bool passed = true;
            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue val = NEATALU::extractelement(accurate, i);
                passed &= NEATValue::areEqual<TypeParam>(accurate[i], val);

                val = NEATALU::extractelement(interval, i);
                passed &= NEATValue::areEqual<TypeParam>(interval[i], val);
            }
            EXPECT_TRUE(passed);
        }
    }
}

/* test for insertelement instruction */
TYPED_TEST(NEATAluTyped, insertelement)
{
    const int WIDTHS_COUNT = 6;
    const int NUM_TESTS = 50;
    VectorWidth widths[] = {V1,V2,V3,V4,V8,V16};

    TypeParam firstFloat[NUM_TESTS][16];
    TypeParam secondFloat[NUM_TESTS][16];

    if(sizeof(TypeParam) == sizeof(float)) {
        GenerateRandomVectors(F32, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F32, &secondFloat[0][0], V16, NUM_TESTS);
    } else if(sizeof(TypeParam) == sizeof(double)){
        GenerateRandomVectors(F64, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F64, &secondFloat[0][0], V16, NUM_TESTS);
    } else {
        GTEST_FAIL();
    }

    for(int widthIdx = 0; widthIdx < WIDTHS_COUNT; widthIdx ++)
    {
        VectorWidth curWidth = widths[widthIdx];
        VectorWidthWrapper wrap(curWidth);
        for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            NEATVector accurate(curWidth);
            NEATVector interval(curWidth);

            for(uint32_t i = 0; i<wrap.GetSize(); i++)
            {
                accurate[i] = NEATValue(firstFloat[testIdx][i]);
                if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                    interval[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                else
                    interval[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
            }

            bool passed = true;

            NEATVector testVec(curWidth);
            NEATVector testVec2(curWidth);

            for(unsigned int i = 0, j = 0; i<wrap.GetSize(); i+=2, j++)
            {
                testVec = NEATALU::insertelement( testVec, accurate[j], i );
                testVec2 = NEATALU::insertelement( testVec2, interval[j], i );
            }

            for(unsigned int i = 0, j = 0; i<wrap.GetSize(); i+=2, j++)
            {
                passed &= NEATValue::areEqual<TypeParam>(accurate[j], testVec[i]);
                passed &= NEATValue::areEqual<TypeParam>(interval[j], testVec2[i]);
            }

            EXPECT_TRUE(passed);
        }
    }
}

/* test for shufflevector instruction */
TYPED_TEST(NEATAluTyped, shufflevector)
{
    const int NUM_TESTS = 4;

    TypeParam firstFloat[NUM_TESTS][16];
    TypeParam secondFloat[NUM_TESTS][16];

    TypeParam add[NUM_TESTS][16];
    TypeParam mul[NUM_TESTS][16];

    if(sizeof(TypeParam) == sizeof(float)) {
        GenerateRandomVectors(F32, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F32, &secondFloat[0][0], V16, NUM_TESTS);
    } else if(sizeof(TypeParam) == sizeof(double)){
        GenerateRandomVectors(F64, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F64, &secondFloat[0][0], V16, NUM_TESTS);
    } else {
        GTEST_FAIL();
    }

        for(int i = 0; i<NUM_TESTS; i++)
        {
            for(int j = 0; j<16; j++)
            {
                add[i][j] = firstFloat[i][j] + secondFloat[i][j];
                mul[i][j] = firstFloat[i][j] * secondFloat[i][j];
            }
        }

        {
            // test to make dst vector from two src vectors
            // mask indexes can't be grater than the sum of widths of src vectors
            const int WIDTHS_COUNT = 6;
            VectorWidth widths[]  = {V1,V2,V3,V4,V8,V16};

            for(int widthSrc1Idx = 0; widthSrc1Idx < WIDTHS_COUNT; widthSrc1Idx ++)
                for(int widthSrc2Idx = 0; widthSrc2Idx < WIDTHS_COUNT; widthSrc2Idx ++)
                    for(int widthDstIdx = 0; widthDstIdx < WIDTHS_COUNT; widthDstIdx ++)
            {
                VectorWidth width1 = widths[widthSrc1Idx];
                VectorWidth width2 = widths[widthSrc2Idx];
                VectorWidth widthDst = widths[widthDstIdx];
                VectorWidthWrapper wrap1(width1);
                VectorWidthWrapper wrap2(width2);
                VectorWidthWrapper wrapDst(widthDst);

                for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
                {
                    NEATVector vec1(width1); //accurate
                    NEATVector vec2(width2); // interval

                    for(uint32_t i = 0; i<wrap1.GetSize(); i++)
                    {
                        vec1[i] = NEATValue(firstFloat[testIdx][i]);
                    }
                    for(uint32_t i = 0; i<wrap2.GetSize(); i++)
                    {
                        if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                            vec2[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                        else
                            vec2[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
                    }
                    DataGenerator::BufferContainerFillMethod bcfm;
                    DataGenerator::BufferContainerListFillMethod bclfm;
                    BufferContainerList list;
                    DataTypeValWrapper dataType;

                    dataType.SetValue(U32);
                    uint32_t maxSrcIndex = uint32_t(wrap1.GetSize()+wrap2.GetSize());
                    DataGenerator::SetBufferContainerFillMethod<uint32_t>(bcfm,
                        dataType,wrapDst,1,DataGenerator::FILL_RANDOM_FROM_RANGE,
                        0,0,maxSrcIndex,0);
                    DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);
                    DataGenerator dg(bclfm);
                    dg.Read(&list);

                    uint32_t* data = (uint32_t*)list.GetBufferContainer(0)->GetMemoryObject(0)->GetDataPtr();

                    std::vector<unsigned int> mask(wrapDst.GetSize());

                    for(unsigned int i = 0; i<wrapDst.GetSize(); i++) {
                        mask[i] = data[i];
                    }

                    NEATVector testVec = NEATALU::shufflevector( vec1, vec2, mask );

                    bool passed = true;

                    if(testVec.GetSize() != wrapDst.GetSize())
                        passed = false;
                    EXPECT_TRUE(passed);

                    // reference shuffle
                    NEATVector refVec(widthDst);

                    for(unsigned int i = 0; i<wrapDst.GetSize(); i++)
                    {
                        unsigned j = mask[i];
                        if(j < vec1.GetSize())
                            refVec[i] = vec1[j];
                        else {
                            refVec[i] = vec2[j-vec1.GetSize()];
                        }
                    }

                    // compasion of results
                    for(unsigned int i = 0; i<wrapDst.GetSize(); i++)
                    {
                        passed &= NEATValue::areEqual<TypeParam>(refVec[i], testVec[i]);
                    }

                    EXPECT_TRUE(passed);
                }
            }
        }

    {
        // test to make dst vector from one src vector - second vector can have zero width,
        // mask indexes can't be grater than the width of first vector
        const int WIDTHS_COUNT = 6;
        VectorWidth widths[]  = {V1,V2,V3,V4,V8,V16};

        for(int widthSrc1Idx = 0; widthSrc1Idx < WIDTHS_COUNT; widthSrc1Idx ++)
            for(int widthSrc2Idx = 0; widthSrc2Idx < WIDTHS_COUNT; widthSrc2Idx ++)
                for(int widthDstIdx = 0; widthDstIdx < WIDTHS_COUNT; widthDstIdx ++)
        {
            VectorWidth width1 = widths[widthSrc1Idx];
            VectorWidth width2 = widths[widthSrc2Idx];
            VectorWidth widthDst = widths[widthDstIdx];
            VectorWidthWrapper wrap1(width1);
            VectorWidthWrapper wrap2(width2);
            VectorWidthWrapper wrapDst(widthDst);

            for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
            {
                NEATVector vec1(width1); //accurate
                NEATVector vec2(width2); // interval

                for(uint32_t i = 0; i<wrap1.GetSize(); i++)
                {
                    vec1[i] = NEATValue(firstFloat[testIdx][i]);
                }
                for(uint32_t i = 0; i<wrap2.GetSize(); i++)
                {
                    if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                        vec2[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                    else
                        vec2[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
                }

                DataGenerator::BufferContainerFillMethod bcfm;
                DataGenerator::BufferContainerListFillMethod bclfm;
                BufferContainerList list;
                DataTypeValWrapper dataType;

                dataType.SetValue(U32);
                // mask indexes can't be grater than the width of first vector
                uint32_t maxSrcIndex = uint32_t(wrap1.GetSize());

                /* generate random index for shufflevector */
                DataGenerator::SetBufferContainerFillMethod<uint32_t>(bcfm,
                    dataType,wrapDst,1,DataGenerator::FILL_RANDOM_FROM_RANGE,
                    0,0,maxSrcIndex,0);
                DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);
                DataGenerator dg(bclfm);
                dg.Read(&list);

                uint32_t* data = (uint32_t*)list.GetBufferContainer(0)->GetMemoryObject(0)->GetDataPtr();

                std::vector<unsigned int> mask(wrapDst.GetSize());

                for(unsigned int i = 0; i<wrapDst.GetSize(); i++) {
                    mask[i] = data[i];
                }

                NEATVector testVec = NEATALU::shufflevector( vec1, vec2, mask );

                bool passed = true;

                if(testVec.GetSize() != wrapDst.GetSize())
                    passed = false;
                EXPECT_TRUE(passed);

                // reference shuffle
                NEATVector refVec(widthDst);

                for(unsigned int i = 0; i<wrapDst.GetSize(); i++)
                {
                    unsigned j = mask[i];
                    if(j < vec1.GetSize())
                        refVec[i] = vec1[j];
                }

                // compasion of results
                for(unsigned int i = 0; i<wrapDst.GetSize(); i++)
                {
                    passed &= NEATValue::areEqual<TypeParam>(refVec[i], testVec[i]);
                }

                EXPECT_TRUE(passed);
            }
        }
    }
}

/* aux functions for testing */
static bool areEqualFloatDouble (const NEATValue& val_1/*float*/, const NEATValue& val_2/*double*/)
    // the first operand must correspond to float data, the second operand must correspond to double data
{
    NEATValue val1 = val_1;
    NEATValue val2 = val_2;

    if( val1.IsNaN<float>() && val2.IsNaN<double>())
        return true;

    NEATValue::Status status = val1.GetStatus();

    switch (status) {
        case NEATValue::ACCURATE:
            if (double(*val1.GetAcc<float>()) == *val2.GetAcc<double>())
                return true;
            else
                return false;
            break;
        case NEATValue::UNKNOWN:
        case NEATValue::UNWRITTEN:
        case NEATValue::ANY:
            return true;
            break;
        case NEATValue::INTERVAL:
            {
                bool res = true;
                float minF = *val1.GetMin<float>();
                float maxF = *val1.GetMax<float>();
                double minD = *val2.GetMin<double>();
                double maxD = *val2.GetMax<double>();

                if( Utils::IsNaN(minF)) {
                    res &= (Utils::IsNaN(minF) && Utils::IsNaN(minD));
                } else {
                    res &= (double(minF) == minD);
                }

                if (Utils::IsNaN(maxF)) {
                    res &= (Utils::IsNaN(maxF) && Utils::IsNaN(maxD));
                } else {
                    res &= (double(maxF) == maxD);
                }
                return res;
            }
            break;
        default:
            return false;
            break;
    }

}

static bool areEqualDoubleFloat (const NEATValue& val_1/*double*/, const NEATValue& val_2/*float*/)
    // the first operand must correspond to float data, the second operand must correspond to double data
{
    NEATValue val1 = val_1;
    NEATValue val2 = val_2;

    if( val1.IsNaN<double>() && val2.IsNaN<float>())
        return true;

    NEATValue::Status status = val1.GetStatus();

    switch (status) {
        case NEATValue::ACCURATE:
            if (float(*val1.GetAcc<double>()) == *val2.GetAcc<float>())
                return true;
            else
                return false;
            break;
        case NEATValue::UNKNOWN:
        case NEATValue::UNWRITTEN:
        case NEATValue::ANY:
            return true;
            break;
        case NEATValue::INTERVAL:
        {
            bool res = true;
            double minD = *val1.GetMin<double>();
            double maxD = *val1.GetMax<double>();
            float minF = *val2.GetMin<float>();
            float maxF = *val2.GetMax<float>();

            if( Utils::IsNaN(minD)) {
                res &= (Utils::IsNaN(minD) && Utils::IsNaN(minF));
            } else {
                res &= (float(minD) == minF);
            }

            if (Utils::IsNaN(maxD)) {
                res &= (Utils::IsNaN(maxD) && Utils::IsNaN(maxF));
            } else {
                res &= (float(maxD) == maxF);
            }
            return res;
         }
            break;
        default:
            return false;
            break;
    }

}

TEST(NEATAlu, fpext)
{
    const int NUM_TESTS = 100;
    // fpext float to double is the only valid fpext so far
    {
        VectorWidthWrapper curWidth;
        curWidth.SetValue(VectorWidth(V16));
        VectorWidthWrapper wrap(curWidth);

        float firstFloat[NUM_TESTS][16];
        float secondFloat[NUM_TESTS][16];

        GenerateRandomVectors(F32, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F32, &secondFloat[0][0], V16, NUM_TESTS);

        for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            NEATVector accurate(curWidth.GetValue());
            NEATVector interval(curWidth.GetValue());

            for(uint32_t i = 0; i<wrap.GetSize(); i++)
            {
                accurate[i] = NEATValue(firstFloat[testIdx][i]);
                if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                    interval[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                else
                    interval[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
            }
            bool passed = true;

            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue ext = NEATALU::fpext<float,double>(accurate[i]);
                passed &= areEqualFloatDouble(accurate[i], ext);
            }

            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue ext = NEATALU::fpext<float,double>(interval[i]);
                passed &= areEqualFloatDouble(interval[i], ext);
            }

            EXPECT_TRUE(passed);
        }
    }

    {
        // test for special values : INf, NaN
        DataGenerator::BufferContainerFillMethod bcfm;
        DataGenerator::BufferContainerListFillMethod bclfm;
        BufferContainerList list;
        DataTypeValWrapper dataType;
        VectorWidthWrapper curWidth;

        dataType.SetValue(F32);
        curWidth.SetValue(VectorWidth(V16));

        for(int i = 0; i<NUM_TESTS*2; i++)
        {
            DataGenerator::SetBufferContainerFillMethod<float>(bcfm,dataType,
                curWidth,1,DataGenerator::FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES,0,0,0,1.0);
        }

        DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);
        DataGenerator dg(bclfm);
        dg.Read(&list);

        VectorWidthWrapper wrap(curWidth);
        for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            float* firstFloat = (float*)list.GetBufferContainer(0)->GetMemoryObject(testIdx)->GetDataPtr();
            float* secondFloat = (float*)list.GetBufferContainer(0)->GetMemoryObject(testIdx+NUM_TESTS)->GetDataPtr();

            NEATVector accurate(curWidth.GetValue());
            NEATVector interval(curWidth.GetValue());

            for(uint32_t i = 0; i<wrap.GetSize(); i++)
            {
                accurate[i] = NEATValue(firstFloat[i]);
                if(Utils::lt(firstFloat[i],secondFloat[i]))
                    interval[i] = NEATValue(firstFloat[i], secondFloat[i]);
                else
                    interval[i] = NEATValue(secondFloat[i], firstFloat[i]);
            }

            bool passed = true;

            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue ext = NEATALU::fpext<float,double>(accurate[i]);
                passed &= areEqualFloatDouble(accurate[i], ext);
            }

            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue ext = NEATALU::fpext<float,double>(interval[i]);
                passed &= areEqualFloatDouble(interval[i], ext);
            }

            NEATVector specVec(wrap.GetValue());
            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                if(Utils::lt(firstFloat[i], secondFloat[i]))
                    specVec[i] = NEATValue(firstFloat[i],secondFloat[i]);
                else
                    specVec[i] = NEATValue(secondFloat[i],firstFloat[i]);
            }
            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue ext = NEATALU::fpext<float,double>(specVec[i]);
                passed &= areEqualFloatDouble(specVec[i], ext);
            }

            EXPECT_TRUE(passed);

        }
    }
}

TEST(NEATAlu, fptrunc)
{
    const int NUM_TESTS = 100;
    // fptrunc double to float is the only valid fptrunc so far

    {
        VectorWidthWrapper curWidth;
        curWidth.SetValue(VectorWidth(V16));
        VectorWidthWrapper wrap(curWidth);

        double firstDouble[NUM_TESTS][16];
        double secondDouble[NUM_TESTS][16];

        GenerateRandomVectors(F64, &firstDouble[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F64, &secondDouble[0][0], V16, NUM_TESTS);

        for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            NEATVector accurate(curWidth.GetValue());
            NEATVector interval(curWidth.GetValue());

            for(uint32_t i = 0; i<wrap.GetSize(); i++)
            {
                accurate[i] = NEATValue(firstDouble[testIdx][i]);
                if(firstDouble[testIdx][i] < secondDouble[testIdx][i])
                    interval[i] = NEATValue(firstDouble[testIdx][i], secondDouble[testIdx][i]);
                else
                    interval[i] = NEATValue(secondDouble[testIdx][i], firstDouble[testIdx][i]);
            }

            bool passed = true;

            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue ext = NEATALU::fptrunc<double,float>(accurate[i]);
                passed &= areEqualDoubleFloat(accurate[i], ext);
            }
            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue ext = NEATALU::fptrunc<double,float>(interval[i]);
                passed &= areEqualDoubleFloat(interval[i], ext);
            }

            EXPECT_TRUE(passed);
        }
    }

    {
        // test for special values : INf, NaN
        DataGenerator::BufferContainerFillMethod bcfm;
        DataGenerator::BufferContainerListFillMethod bclfm;
        BufferContainerList list;
        DataTypeValWrapper dataType;
        VectorWidthWrapper curWidth;

        dataType.SetValue(F64);
        curWidth.SetValue(VectorWidth(V16));

        for(int i = 0; i<NUM_TESTS*2; i++)
        {
            DataGenerator::SetBufferContainerFillMethod<double>(bcfm,dataType,
                curWidth,1,DataGenerator::FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES,0,0,0,1.0);
        }

        DataGenerator::SetBufferContainerListFillMethod(bclfm, bcfm);
        DataGenerator dg(bclfm);
        dg.Read(&list);

        VectorWidthWrapper wrap(curWidth);
        for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            double* firstDouble = (double*)list.GetBufferContainer(0)->GetMemoryObject(testIdx)->GetDataPtr();
            double* secondDouble = (double*)list.GetBufferContainer(0)->GetMemoryObject(testIdx+NUM_TESTS)->GetDataPtr();

            NEATVector accurate(curWidth.GetValue());
            NEATVector interval(curWidth.GetValue());

            for(uint32_t i = 0; i<wrap.GetSize(); i++)
            {
                accurate[i] = NEATValue(firstDouble[i]);
                if(Utils::lt(firstDouble[i], secondDouble[i]))
                    interval[i] = NEATValue(firstDouble[i], secondDouble[i]);
                else
                    interval[i] = NEATValue(secondDouble[i], firstDouble[i]);
            }

            bool passed = true;

            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue ext = NEATALU::fptrunc<double,float>(accurate[i]);
                passed &= areEqualDoubleFloat(accurate[i], ext);
            }
            for(unsigned int i = 0; i<wrap.GetSize(); i++)
            {
                NEATValue ext = NEATALU::fptrunc<double,float>(interval[i]);
                passed &= areEqualDoubleFloat(interval[i], ext);
            }

            EXPECT_TRUE(passed);
        }
    }
}

TEST(NEATAlu, bitcast)
{
    // currently valid bitcast operations for NEATALU:
    // floats to doubles, double to floats
    const int NUM_TESTS = 100;

    {
        // test for float
        float firstFloat[NUM_TESTS][16];
        float secondFloat[NUM_TESTS][16];

        GenerateRandomVectors(F32, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F32, &secondFloat[0][0], V16, NUM_TESTS);

        {
            NEATVector accurate(V2);
            NEATVector interval(V2);
            // test to make dst double value src vector of two floats
            bool passed = true;
            for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
            {

                for(uint32_t i = 0; i<2; i++)
                {
                    accurate[i] = NEATValue(firstFloat[testIdx][i]);
                    if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                        interval[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                    else
                        interval[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
                }

                try {
                    NEATValue testVal = NEATALU::bitcast<float,double,NEATValue>(accurate);
                    NEATValue ref = NEATValue(NEATValue::UNKNOWN);
                    if(!(NEATValue::areEqual<double>(testVal,ref)))
                        passed &= false;
                }
                catch (Exception::InvalidArgument){
                     passed &= false;
                }

                for(uint32_t i = 0; i<2; i++)
                {
                    interval[i] = NEATValue(firstFloat[testIdx][i]);
                    if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                        interval[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                    else
                        interval[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
                }

                try {
                    NEATValue testVal = NEATALU::bitcast<float,double,NEATValue>(interval);
                    NEATValue ref = NEATValue(NEATValue::UNKNOWN);
                    if(!(NEATValue::areEqual<double>(testVal,ref)))
                        passed &= false;
                }
                catch (Exception::InvalidArgument){
                     passed &= false;
                }
            }

            EXPECT_TRUE(passed);
        }

        {
            // test to make dst vector from src vectors
            const int WIDTHS_COUNT = 6;
            VectorWidth widths[]  = {V1,V2,V3,V4,V8,V16};
            bool passed = true;

            for(int widthSrcIdx = 0; widthSrcIdx < WIDTHS_COUNT; widthSrcIdx ++)
            {
                VectorWidth width = widths[widthSrcIdx];
                VectorWidthWrapper wrap(width);

                for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
                {
                    NEATVector accurate(width);
                    NEATVector interval(width);

                    for(uint32_t i = 0; i<wrap.GetSize(); i++)
                    {
                        accurate[i] = NEATValue(firstFloat[testIdx][i]);
                        if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                            interval[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                        else
                            interval[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
                    }


                    int testSize = 0;
                    try {
                        NEATVector testVec = NEATALU::bitcast<float,double>(accurate);
                        testSize = testVec.GetSize();
                        NEATValue ref = NEATValue(NEATValue::UNKNOWN);
                        for(int i = 0; i<testSize; i++)
                        {
                            if(!(NEATValue::areEqual<double>(testVec[i],ref)))
                                passed &= false;
                        }
                    }
                    catch (Exception::InvalidArgument){
                        testSize = -1;
                    }

                    switch(wrap.GetSize()) {
                        case 1:
                        case 3:
                            if(testSize != -1)
                                passed &= false;
                            break;
                        case 2:
                            if(testSize != 1)
                                passed &= false;
                            break;
                        case 4:
                            if(testSize != 2)
                                passed &= false;
                            break;
                        case 8:
                            if(testSize != 4)
                                passed &= false;
                            break;
                        case 16:
                            if(testSize != 8)
                                passed &= false;
                            break;
                        default:
                            break;
                    }

                    testSize = 0;
                    try {
                        NEATVector testVec = NEATALU::bitcast<float,double>(interval);
                        testSize = testVec.GetSize();
                        NEATValue ref = NEATValue(NEATValue::UNKNOWN);
                        for(int i = 0; i<testSize; i++)
                        {
                            if(!(NEATValue::areEqual<double>(testVec[i],ref)))
                                passed &= false;
                        }
                    }
                    catch (Exception::InvalidArgument){
                        testSize = -1;
                    }
                    switch(wrap.GetSize()) {
                        case 1:
                        case 3:
                            if(testSize != -1)
                                passed &= false;
                            break;
                        case 2:
                            if(testSize != 1)
                                passed &= false;
                            break;
                        case 4:
                            if(testSize != 2)
                                passed &= false;
                            break;
                        case 8:
                            if(testSize != 4)
                                passed &= false;
                            break;
                        case 16:
                            if(testSize != 8)
                                passed &= false;
                            break;
                        default:
                            break;
                    }
                }
            }
            EXPECT_TRUE(passed);
        }
    }

    {
        // test for double
        double firstFloat[NUM_TESTS][16];
        double secondFloat[NUM_TESTS][16];

        GenerateRandomVectors(F64, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F64, &secondFloat[0][0], V16, NUM_TESTS);

        {
            // test to make dst vector of two floats from src double value
            bool passed = true;
            for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
            {
                NEATValue accurate = NEATValue(firstFloat[testIdx][0]);
                NEATValue interval;

                if(firstFloat[testIdx][0] < secondFloat[testIdx][0])
                    interval = NEATValue(firstFloat[testIdx][0], secondFloat[testIdx][0]);
                else
                    interval = NEATValue(secondFloat[testIdx][0], firstFloat[testIdx][0]);

                int testSize = 0;
                try {
                    NEATVector testVec = NEATALU::bitcast<double,float>(accurate);
                    NEATValue ref = NEATValue(NEATValue::UNKNOWN);
                    testSize = testVec.GetSize();
                    if(testSize != 2)
                        passed &= false;
                    for(int i=0;i<2;i++) {
                        if(!(NEATValue::areEqual<double>(testVec[i],ref)))
                            passed &= false;
                    }
                }
                catch (Exception::InvalidArgument){
                     passed &= false;
                }

                testSize = 0;
                try {
                    NEATVector testVec = NEATALU::bitcast<double,float>(interval);
                    NEATValue ref = NEATValue(NEATValue::UNKNOWN);
                    testSize = testVec.GetSize();
                    if(testSize != 2)
                        passed &= false;
                    for(int i=0;i<2;i++) {
                        if(!(NEATValue::areEqual<double>(testVec[i],ref)))
                            passed &= false;
                    }
                }
                catch (Exception::InvalidArgument){
                     passed &= false;
                }
            }
            EXPECT_TRUE(passed);
        }

        {
            // test to make dst vector src vectors
            const int WIDTHS_COUNT = 6;
            VectorWidth widths[]  = {V1,V2,V3,V4,V8,V16};
            bool passed = true;

            for(int widthSrcIdx = 0; widthSrcIdx < WIDTHS_COUNT; widthSrcIdx ++)
            {
                VectorWidth width = widths[widthSrcIdx];
                VectorWidthWrapper wrap(width);

                for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
                {
                    NEATVector accurate(width);
                    NEATVector interval(width);

                    for(uint32_t i = 0; i<wrap.GetSize(); i++)
                    {
                        accurate[i] = NEATValue(firstFloat[testIdx][i]);
                        if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                            interval[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                        else
                            interval[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
                    }

                    int testSize = 0;
                    try {
                        NEATVector testVec = NEATALU::bitcast<double,float>(accurate);
                        testSize = testVec.GetSize();
                        NEATValue ref = NEATValue(NEATValue::UNKNOWN);
                        for(int i = 0; i<testSize; i++)
                        {
                            if(!(NEATValue::areEqual<double>(testVec[i],ref)))
                                passed &= false;
                        }
                    }
                    catch (Exception::InvalidArgument){
                        testSize = -1;
                    }

                    switch(wrap.GetSize()) {
                        case 3:
                        case 16:
                            if(testSize != -1)
                                passed &= false;
                            break;
                        case 1:
                            if(testSize != 2)
                                passed &= false;
                            break;
                        case 2:
                            if(testSize != 4)
                                passed &= false;
                            break;
                        case 4:
                            if(testSize != 8)
                                passed &= false;
                            break;
                        case 8:
                            if(testSize != 16)
                                passed &= false;
                            break;
                        default:
                            break;
                    }

                    testSize = 0;
                    try {
                        NEATVector testVec = NEATALU::bitcast<double,float>(interval);
                        testSize = testVec.GetSize();
                        NEATValue ref = NEATValue(NEATValue::UNKNOWN);
                        for(int i = 0; i<testSize; i++)
                        {
                            if(!(NEATValue::areEqual<double>(testVec[i],ref)))
                                passed &= false;
                        }
                    }
                    catch (Exception::InvalidArgument){
                        testSize = -1;
                    }

                    switch(wrap.GetSize()) {
                        case 3:
                        case 16:
                            if(testSize != -1)
                                passed &= false;
                            break;
                        case 1:
                            if(testSize != 2)
                                passed &= false;
                            break;
                        case 2:
                            if(testSize != 4)
                                passed &= false;
                            break;
                        case 4:
                            if(testSize != 8)
                                passed &= false;
                            break;
                        case 8:
                            if(testSize != 16)
                                passed &= false;
                            break;
                        default:
                            break;
                    }
                }
            }
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTyped, select_llvm)
{
    const int NUM_TESTS = 100;
    TypeParam firstFloat[NUM_TESTS][16];
    TypeParam secondFloat[NUM_TESTS][16];

    if(sizeof(TypeParam) == sizeof(float)) {
        GenerateRandomVectors(F32, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F32, &secondFloat[0][0], V16, NUM_TESTS);
    } else if(sizeof(TypeParam) == sizeof(double)){
        GenerateRandomVectors(F64, &firstFloat[0][0], V16, NUM_TESTS);
        GenerateRandomVectors(F64, &secondFloat[0][0], V16, NUM_TESTS);
    } else {
        GTEST_FAIL();
    }

    {
        // test to make dst vector from src vectors
        const int WIDTHS_COUNT = 6;
        VectorWidth widths[]  = {V1,V2,V3,V4,V8,V16};
        bool passed = true;

        for(int widthSrcIdx = 0; widthSrcIdx < WIDTHS_COUNT; widthSrcIdx ++)
        {
            VectorWidth width = widths[widthSrcIdx];
            VectorWidthWrapper wrap(width);

            for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
            {
                NEATVector accurate(width);
                NEATVector interval(width);

                for(uint32_t i = 0; i<wrap.GetSize(); i++)
                {
                    accurate[i] = NEATValue(firstFloat[testIdx][i]);
                    if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                        interval[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                    else
                        interval[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
                }

                for(unsigned i=0;i<wrap.GetSize();i++) {
                    bool cond = true;
                    NEATValue tstVal1 = NEATALU::select(cond, interval[i], accurate[i]);
                    cond = false;
                    NEATValue tstVal2 = NEATALU::select(cond, interval[i], accurate[i]);

                    passed &= NEATValue::areEqual<TypeParam>(tstVal1,interval[i]);
                    passed &= NEATValue::areEqual<TypeParam>(tstVal2,accurate[i]);
                }

                std::vector<bool> condVec(wrap.GetSize());
                unsigned k=0;
                for(unsigned i=0;i<wrap.GetSize();i++)
                    if((firstFloat[testIdx][i] + secondFloat[testIdx][i]) > 0 )
                    {
                        condVec[i] = true;
                        k++;
                    }
                    else
                        condVec[i] = false;
                //printf(" k = %d from %d\n ",k,wrap.GetSize());

                NEATVector testVec = NEATALU::select(condVec,interval,accurate);
                for(unsigned int i = 0; i<testVec.GetSize(); i++)
                {
                    if(condVec[i])
                        passed &= NEATValue::areEqual<TypeParam>(testVec[i],interval[i]);
                    else
                        passed &= NEATValue::areEqual<TypeParam>(testVec[i],accurate[i]);
                }

                for(int widthSrcIdx2 = 0; widthSrcIdx2 < WIDTHS_COUNT; widthSrcIdx2 ++)
                {
                    VectorWidth width = widths[widthSrcIdx2];
                    VectorWidthWrapper wrap(width);

                    NEATVector accurate(width);
                    NEATVector interval(width);

                    for(uint32_t i = 0; i<wrap.GetSize(); i++)
                    {
                        accurate[i] = NEATValue(firstFloat[testIdx][i]);
                        if(firstFloat[testIdx][i] < secondFloat[testIdx][i])
                            interval[i] = NEATValue(firstFloat[testIdx][i], secondFloat[testIdx][i]);
                        else
                            interval[i] = NEATValue(secondFloat[testIdx][i], firstFloat[testIdx][i]);
                    }

                    bool cond = true;
                    NEATVector tstVec1 = NEATALU::select(cond, interval, accurate);
                    cond = false;
                    NEATVector tstVec2 = NEATALU::select(cond, interval, accurate);

                    if(tstVec1.GetSize() == interval.GetSize()) {
                        for(unsigned int i=0;i<tstVec1.GetSize();i++) {
                            passed &= NEATValue::areEqual<TypeParam>(tstVec1[i],interval[i]);
                        }
                    } else
                        passed = false;

                    if(tstVec2.GetSize() == accurate.GetSize()) {
                        for(unsigned int i=0;i<tstVec2.GetSize();i++) {
                            passed &= NEATValue::areEqual<TypeParam>(tstVec2[i],accurate[i]);
                        }
                    } else
                        passed = false;
                }
            }
            EXPECT_TRUE(passed);
        }
    }
}


template <typename T>
class NEATAluTypedMath : public ALUTest {
public:
    DataTypeVal dataTypeVal;

    NEATAluTypedMath(){

        RefALU::SetFTZmode(T::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false

        dataTypeVal = GetDataTypeVal<typename T::Type>();
    }
};

typedef ::testing::Types<ValueTypeContainer<float,true>,ValueTypeContainer<float,false> > FloatTypesMathFTZ;
TYPED_TEST_CASE(NEATAluTypedMath, FloatTypesMathFTZ);

template <typename T>
class NEATDivTest {
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
    typedef NEATValue (*NEATFuncP)(const NEATValue&, const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&, const NEATVector&);
    typedef sT (*RefFuncP) (const sT&, const sT&);

    NEATDivTest()
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

    void TestDiv(NEATFuncP NEATFunc, NEATFuncVecP NEATFuncVec, RefFuncP RefFunc, float ulps)
    {
        // Test corner cases first
        NEATValue unkn = NEATValue(NEATValue::UNKNOWN);
        NEATValue unwr = NEATValue(NEATValue::UNWRITTEN);
        NEATValue any  = NEATValue(NEATValue::ANY);
        NEATValue nan  = NEATValue::NaN<T>();
        NEATValue zero = NEATValue((T)0.0f);
        NEATValue zeroNeg = NEATValue((T)-0.0f);
        NEATValue pInf(Utils::GetPInf<T>());
        NEATValue nInf(Utils::GetNInf<T>());
        NEATValue one((T)1.0f);
        NEATValue nOne((T)-1.0f);
        NEATValue tmp;
    
        tmp = NEATFunc(unkn, unkn);
        EXPECT_TRUE(tmp.IsUnknown());
        tmp = NEATFunc(unkn, unwr);
        EXPECT_TRUE(tmp.IsUnknown());
        tmp = NEATFunc(unwr, unwr);
        EXPECT_TRUE(tmp.IsUnknown());
        tmp = NEATFunc(unwr, unkn); 
        EXPECT_TRUE(tmp.IsUnknown());
        tmp = NEATFunc(nan, any);
        EXPECT_TRUE(tmp.IsNaN<T>());
        tmp = NEATFunc(any, any);
        EXPECT_TRUE(tmp.IsUnknown());
        tmp = NEATFunc(nan, nan);
        EXPECT_TRUE(tmp.IsNaN<T>());
        // Division specific cases with zeros
        // 0/0 NaN
        tmp = NEATFunc(zero, zero);
        EXPECT_TRUE(tmp.IsNaN<T>());
        // +-Inf/+-Inf = NaN
        tmp = NEATFunc(pInf, pInf);
        EXPECT_TRUE(tmp.IsNaN<T>());
        tmp = NEATFunc(pInf, nInf);
        EXPECT_TRUE(tmp.IsNaN<T>());
        tmp = NEATFunc(nInf, pInf);
        EXPECT_TRUE(tmp.IsNaN<T>());
        tmp = NEATFunc(nInf, nInf);
        EXPECT_TRUE(tmp.IsNaN<T>());
        // (-+)1/(+-)0 = (+-)Inf
        tmp = NEATFunc(one, zero);
        EXPECT_EQ(*tmp.GetAcc<T>(), Utils::GetPInf<T>());
        tmp = NEATFunc(nOne, zero);
        EXPECT_EQ(*tmp.GetAcc<T>(), Utils::GetNInf<T>());
        tmp = NEATFunc(one, zeroNeg);
        EXPECT_EQ(*tmp.GetAcc<T>(), Utils::GetNInf<T>());
        tmp = NEATFunc(nOne, zeroNeg);
        EXPECT_EQ(*tmp.GetAcc<T>(), Utils::GetPInf<T>());
 
        VectorWidth Width = VectorWidthWrapper::ValueOf(vectorWidth);

        for(uint32_t testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
        {
            // Test result for given operation of accurate values or intervals
            NEATValue accVal, intVal;
            NEATValue testAccVal, testIntVal;
            NEATValue refAccVal, refIntVal;
    
            T minRef[vectorWidth];
            T maxRef[vectorWidth];
    
            for(uint32_t i = 0; i<vectorWidth; i++)
            {
                uint32_t idx = testIdx * vectorWidth + i;
    
                // Test for accurate operation (input is accurate values, output - interval)
                NEATValue aAcc = NEATValue(Arg1Min[idx]);
                NEATValue bAcc = NEATValue(Arg2Min[idx]);
                // NEAT division for two accurate values
                NEATValue accRes = NEATFunc(aAcc,bAcc);
                sT refRes = RefFunc(sT(Arg1Min[idx]), sT(Arg2Min[idx]));
                EXPECT_TRUE(TestAccExpanded<sT>(refRes, accRes, ulps));

                // Now test intervals as input
                // Calc expected result
                const uint32_t RES_COUNT = 8;
                sT allResults[RES_COUNT] = { RefFunc(sT(Arg1Min[idx]), sT(Arg2Min[idx])), RefFunc(sT(Arg1Max[idx]), sT(Arg2Min[idx])),
                RefFunc(sT(Arg1Min[idx]), sT(Arg2Max[idx])), RefFunc(sT(Arg1Max[idx]), sT(Arg2Max[idx])),
                RefALU::mul(sT(Arg1Min[idx]), RefFunc((sT)1.0,(sT(Arg2Min[idx])) )), //Arg1Min * (1/Arg2Min)
                RefALU::mul(sT(Arg1Max[idx]), RefFunc((sT)1.0,(sT(Arg2Min[idx])) )), //Arg1Max * (1/Arg2Min)
                RefALU::mul(sT(Arg1Min[idx]), RefFunc((sT)1.0,(sT(Arg2Max[idx])) )), //Arg1Min * (1/Arg2Max)
                RefALU::mul(sT(Arg1Max[idx]), RefFunc((sT)1.0,(sT(Arg2Max[idx])) )) };//Arg1Max * (1/Arg2Max)
                minRef[i] = FindMin(allResults, RES_COUNT);
                maxRef[i] = FindMax(allResults, RES_COUNT);
    
                // Reference interval borders calulated
    
                // Test interval
                NEATValue a = NEATValue(Arg1Min[idx], Arg1Max[idx]);
                NEATValue b = NEATValue(Arg2Min[idx], Arg2Max[idx]);
                NEATValue res = NEATFunc(a,b);
                bool passed = false;
                if(res.GetStatus() != NEATValue::UNKNOWN)
                    passed = TestIntExpanded<sT>(minRef[i], maxRef[i], res, ulps);
                else
                    // result unknown. Divisor should contain zero
                    passed = (b.Includes<T>(0.0) || (b.Includes<T>(-0.0)));
                // Probably test for non-interval value
                EXPECT_TRUE(passed);
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
            for(uint32_t i = 0; i<vectorWidth; i++)
            {
                bool passed;
                if(res[i].GetStatus() != NEATValue::UNKNOWN)
                    passed = TestIntExpanded<sT>(minRef[i], maxRef[i], res[i], ulps);
                else
                    passed = (b[i].Includes<T>(0.0) || b[i].Includes<T>(-0.0));
                EXPECT_TRUE(passed);
            }
        }
    } // void TestDiv(NEATFuncP NEATFunc, NEATFuncVecP NEATFuncVec, RefFuncP RefFunc, float ulps)

};


template <typename T>
class NEATDivTestRun : public ALUTest {
};


TYPED_TEST_CASE(NEATDivTestRun, FloatTypesMathFTZ);
TYPED_TEST(NEATDivTestRun, div)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&, const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&, const NEATVector&);
    typedef sT (*RefFuncP) (const sT&, const sT&);

    /// If your test doesn't currently support doubles, use SkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::div<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::div<TypeP>;
    RefFuncP RefFunc = &RefALU::div<sT>;

    NEATDivTest<TypeP> divTest;

    divTest.TestDiv(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::DIV_ERROR));
}

TYPED_TEST(NEATDivTestRun, half_divide)
{
    RefALU::SetFTZmode(TypeParam::mode); // we use ValueTypeContainer type here, T::mode is FTZ mode, can be true or false
    typedef typename TypeParam::Type TypeP;
    typedef typename superT<TypeP>::type sT;
    typedef NEATValue (*NEATFuncP)(const NEATValue&, const NEATValue&);
    typedef NEATVector (*NEATFuncVecP)(const NEATVector&, const NEATVector&);
    typedef sT (*RefFuncP) (const sT&, const sT&);

    /// If your test doesn't currently support doubles, use SkipDoubleTest
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    NEATFuncP NEATFunc = &NEATALU::half_divide<TypeP>;
    NEATFuncVecP NEATFuncVec = &NEATALU::half_divide<TypeP>;
    RefFuncP RefFunc = &RefALU::div<sT>;

    NEATDivTest<TypeP> divTest;

    divTest.TestDiv(NEATFunc, NEATFuncVec, RefFunc, float(NEATALU::HALF_DIVIDE_ERROR));
}


// seed taken from command line
extern uint64_t seedForValidation;

template <typename T>
static bool TestExpandInterval(T firstIn, T secondIn, float ulps) {

               typedef typename  sT<T>::type SuperT;

               SuperT firstRef = (SuperT)firstIn;
               SuperT secondRef = (SuperT)secondIn;

               SuperT first = firstRef;
               SuperT second = secondRef;

               NEATALU::ExpandFloatInterval(&first, &second, ulps);
               T min = NEATALU::castDown(first);
               T max = NEATALU::castDown(second);

               float diffH_1 =   Utils::ulpsDiffSamePrecision(firstRef, first);
               float diffH_2 =   Utils::ulpsDiffSamePrecision(secondRef, second);

               float diffL_1 = Utils::ulpsDiff(firstRef,min);
               float diffL_2 = Utils::ulpsDiff(secondRef,max);

               bool res = true;

               if(!(Utils::IsInf(first) || Utils::IsInf(firstRef)))
                   res &= (fabs(diffH_1)<=ulps) && (fabs(diffH_1)>=(ulps-1.f));
               if(!(Utils::IsInf(second) || Utils::IsInf(secondRef)))
                   res &= (fabs(diffH_2)<=ulps) && (fabs(diffH_2)>=(ulps-1.f));

               if(!(Utils::IsInf(min) || Utils::IsInf(firstRef)))
                   res &= (fabs(diffL_1)<=ulps) && (fabs(diffL_1)>=(ulps-diffLimit));
               if(!(Utils::IsInf(max) || Utils::IsInf(secondRef)))
                   res &= (fabs(diffL_2)<=ulps) && (fabs(diffL_2)>=(ulps-diffLimit));

               return res;
}

// TODO: long double data generation is not supported so far,
// currently long doubles are produced from doubles
TYPED_TEST(NEATAluTypedMath, ExpandFloatInterval)
{
    typedef typename  TypeParam::Type TypeP;
    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 1000;

    double firstFloat[NUM_TESTS*8];
    double secondFloat[NUM_TESTS*8];

    double firstFloatRanged[NUM_TESTS*8];
    double secondFloatRanged[NUM_TESTS*8];

    float ulps[NUM_TESTS];

    VectorWidth curWidth = V8;
    uint64_t seedRnd = SetSeed(seedForValidation);

    this->dataTypeVal = GetDataTypeVal<TypeP>();

    if(this->dataTypeVal == F32)
    {
        double highLimit = std::numeric_limits<float>::max();
        double lowLimit = -std::numeric_limits<float>::max();

        GenerateRangedVectorsAutoSeed(F64, &firstFloat[0], curWidth, NUM_TESTS,
                                      lowLimit, highLimit);
        GenerateRangedVectorsAutoSeed(F64, &secondFloat[0], curWidth, NUM_TESTS,
                                      lowLimit, highLimit);

    } else if(this->dataTypeVal == F64){
        GenerateRandomVectorsAutoSeed(F64, &firstFloat[0], curWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(F64, &secondFloat[0], curWidth, NUM_TESTS);
    } else {
        GTEST_FAIL();
    }


    GenerateRangedVectorsAutoSeed(F64, &firstFloatRanged[0], curWidth,
                                  NUM_TESTS,-1000.0,1000.0);

    GenerateRangedVectorsAutoSeed(F64, &secondFloatRanged[0], curWidth,
                                  NUM_TESTS,-1000.0,1000.0);

    GenerateRangedVectorsAutoSeed(F32, &ulps[0], V1, NUM_TESTS,diffLimit,16.0f);

    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        ulps[testIdx] = floor(ulps[testIdx]);
    }

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            {
               TypeP acc = firstFloat[testIdx*wrap.GetSize()+i];
               bool res = TestExpandInterval(acc,acc,ulps[i]);
               EXPECT_TRUE(res)<<" seedRnd "<<seedRnd<<"\n";
            }

            {
               TypeP first, second;

               if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
               {
                   first = firstFloat[testIdx*wrap.GetSize()+i];
                   second = secondFloat[testIdx*wrap.GetSize()+i];
               } else {
                   second = firstFloat[testIdx*wrap.GetSize()+i];
                   first = secondFloat[testIdx*wrap.GetSize()+i];
               }
               bool res = TestExpandInterval(first,second,ulps[i]);
               EXPECT_TRUE(res)<<" seedRnd "<<seedRnd<<"\n";
            }
        }


        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            {
               TypeP acc = firstFloatRanged[testIdx*wrap.GetSize()+i];
               EXPECT_TRUE(TestExpandInterval(acc,acc,ulps[i]))<<" seedRnd "<<seedRnd<<"\n";
            }

            {
               TypeP first, second;

               if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloatRanged[testIdx*wrap.GetSize()+i])
               {
                   first = firstFloatRanged[testIdx*wrap.GetSize()+i];
                   second = secondFloatRanged[testIdx*wrap.GetSize()+i];
               } else {
                   second = firstFloatRanged[testIdx*wrap.GetSize()+i];
                   first = secondFloatRanged[testIdx*wrap.GetSize()+i];
               }
               bool res = TestExpandInterval(first,second,ulps[i]);
               EXPECT_TRUE(res)<<" seedRnd "<<seedRnd<<"\n";
            }
        }
     }
}


TYPED_TEST(NEATAluTypedMath, sin)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;
    VectorWidth curWidth = V8;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];


    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                    NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0],
                                      curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0],
                                      curWidth, NUM_TESTS, TypeP(-NEATALU::pi_2+0.01),
                                      TypeP(NEATALU::pi_2-0.01));

    // +- 0.01 to avoid including PI/2 to interval max amd min points are tested
    // on not ranged vectors (firstFloat, secondFloat)
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-NEATALU::pi_2+0.01),TypeP(NEATALU::pi_2-0.01));

    /* test for specific values */
    NEATValue testAccVal = NEATALU::sin<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sin<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sin<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sin<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::sin<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::sin<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::sin<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::sin<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        SuperT refAccValFloat;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for monotonic interval from -pi/2 to pi/2 */
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

        testAccVal = NEATALU::sin<TypeP>(accVal);
        refAccValFloat = RefALU::sin(SuperT(*accVal.GetAcc<TypeP>()));
        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
            // sin(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
            // sin(-0) == -0
        } else {
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,NEATALU::SIN_ERROR);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::sin<TypeP>(intVal);

        bool passed = true;

        {
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*intVal.GetMax<TypeP>(),
                         RefALU::neg(*intVal.GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*intVal.GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0/2.0*NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(7.0/2.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi_2) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(5.0/2.0*NEATALU::pi) );
            }

            refIntValMax = RefALU::sin(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::sin(SuperT(*intVal.GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::SIN_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::SIN_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVal.GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVal.GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVal.GetMin<TypeP>())) > NEATALU::SIN_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if(*testIntVal.GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVal.GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVal.GetMax<TypeP>())) > NEATALU::SIN_ERROR)
                        passed = false;
        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT intervals */
        passed = true;
        NEATVector testIntVec1 = NEATALU::sin<TypeP>(interval);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {

            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*interval[i].GetMax<TypeP>(),
                         RefALU::neg(*interval[i].GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*interval[i].GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);

                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0/2.0*NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(7.0/2.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi_2) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(5.0/2.0*NEATALU::pi) );
            }

            refIntValMax = RefALU::sin(SuperT(*interval[i].GetMax<TypeP>()));
            refIntValMin = RefALU::sin(SuperT(*interval[i].GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                SuperT a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::SIN_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::SIN_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVec1[i].GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVec1[i].GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVec1[i].GetMin<TypeP>())) > NEATALU::SIN_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if( *testIntVec1[i].GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVec1[i].GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVec1[i].GetMax<TypeP>())) > NEATALU::SIN_ERROR)
                        passed = false;
        }
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate in range */
        NEATVector testAccVec = NEATALU::sin<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sin(+0) == +0
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sin(-0) == -0
            } else {
                refAccValFloat = RefALU::sin(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::SIN_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        NEATVector testIntVec = NEATALU::sin<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::sin(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::sin(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::SIN_ERROR);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, native_sin)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;
    VectorWidth curWidth = V8;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];


    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                    NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0],
                                      curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0],
                                      curWidth, NUM_TESTS, TypeP(-NEATALU::pi_2+0.01),
                                      TypeP(NEATALU::pi_2-0.01));

    // +- 0.01 to avoid including PI/2 to interval max amd min points are tested
    // on not ranged vectors (firstFloat, secondFloat)
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-NEATALU::pi_2+0.01),TypeP(NEATALU::pi_2-0.01));

    /* test for specific values */
    NEATValue testAccVal = NEATALU::native_sin<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_sin<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_sin<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_sin<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::native_sin<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::native_sin<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::native_sin<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::native_sin<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        SuperT refAccValFloat;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for monotonic interval from -pi/2 to pi/2 */
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

        testAccVal = NEATALU::native_sin<TypeP>(accVal);
        refAccValFloat = RefALU::sin(SuperT(*accVal.GetAcc<TypeP>()));
        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
            // sin(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
            // sin(-0) == -0
        } else {
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,NEATALU::NATIVE_SIN_ERROR);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::native_sin<TypeP>(intVal);

        bool passed = true;

        {
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*intVal.GetMax<TypeP>(),
                         RefALU::neg(*intVal.GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*intVal.GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0/2.0*NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(7.0/2.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi_2) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(5.0/2.0*NEATALU::pi) );
            }

            refIntValMax = RefALU::sin(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::sin(SuperT(*intVal.GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::NATIVE_SIN_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::NATIVE_SIN_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVal.GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVal.GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVal.GetMin<TypeP>())) > NEATALU::NATIVE_SIN_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if(*testIntVal.GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVal.GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVal.GetMax<TypeP>())) > NEATALU::NATIVE_SIN_ERROR)
                        passed = false;
        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT intervals */
        passed = true;
        NEATVector testIntVec1 = NEATALU::native_sin<TypeP>(interval);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {

            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*interval[i].GetMax<TypeP>(),
                         RefALU::neg(*interval[i].GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*interval[i].GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);

                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0/2.0*NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(7.0/2.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi_2) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(5.0/2.0*NEATALU::pi) );
            }

            refIntValMax = RefALU::sin(SuperT(*interval[i].GetMax<TypeP>()));
            refIntValMin = RefALU::sin(SuperT(*interval[i].GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                SuperT a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::NATIVE_SIN_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::NATIVE_SIN_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVec1[i].GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVec1[i].GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVec1[i].GetMin<TypeP>())) > NEATALU::NATIVE_SIN_ERROR)
                        passed = false;


            if(hasMaxPoint) {
                if( *testIntVec1[i].GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVec1[i].GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVec1[i].GetMax<TypeP>())) > NEATALU::NATIVE_SIN_ERROR)
                        passed = false;

        }
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate in range */
        NEATVector testAccVec = NEATALU::native_sin<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sin(+0) == +0
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sin(-0) == -0
            } else {
                refAccValFloat = RefALU::sin(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::NATIVE_SIN_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        NEATVector testIntVec = NEATALU::native_sin<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::sin(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::sin(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::NATIVE_SIN_ERROR);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, half_sin)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;
    VectorWidth curWidth = V8;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];


    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                    NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0],
                                      curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0],
                                      curWidth, NUM_TESTS, TypeP(-NEATALU::pi_2+0.01),
                                      TypeP(NEATALU::pi_2-0.01));

    // +- 0.01 to avoid including PI/2 to interval max amd min points are tested
    // on not ranged vectors (firstFloat, secondFloat)
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-NEATALU::pi_2+0.01),TypeP(NEATALU::pi_2-0.01));

    /* test for specific values */
    NEATValue testAccVal = NEATALU::half_sin<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_sin<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_sin<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_sin<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::half_sin<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::half_sin<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::half_sin<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::half_sin<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        SuperT refAccValFloat;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for monotonic interval from -pi/2 to pi/2 */
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

        testAccVal = NEATALU::half_sin<TypeP>(accVal);
        refAccValFloat = RefALU::sin(SuperT(*accVal.GetAcc<TypeP>()));
        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
            // sin(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
            // sin(-0) == -0
        } else {
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,NEATALU::HALF_SIN_ERROR);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::half_sin<TypeP>(intVal);

        bool passed = true;

        {
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*intVal.GetMax<TypeP>(),
                         RefALU::neg(*intVal.GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*intVal.GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0/2.0*NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(7.0/2.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi_2) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(5.0/2.0*NEATALU::pi) );
            }

            refIntValMax = RefALU::sin(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::sin(SuperT(*intVal.GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::HALF_SIN_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::HALF_SIN_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVal.GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVal.GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVal.GetMin<TypeP>())) > NEATALU::HALF_SIN_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if(*testIntVal.GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVal.GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVal.GetMax<TypeP>())) > NEATALU::HALF_SIN_ERROR)
                        passed = false;
        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT intervals */
        passed = true;
        NEATVector testIntVec1 = NEATALU::half_sin<TypeP>(interval);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {

            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*interval[i].GetMax<TypeP>(),
                         RefALU::neg(*interval[i].GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*interval[i].GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);

                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0/2.0*NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(7.0/2.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi_2) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(5.0/2.0*NEATALU::pi) );
            }

            refIntValMax = RefALU::sin(SuperT(*interval[i].GetMax<TypeP>()));
            refIntValMin = RefALU::sin(SuperT(*interval[i].GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                SuperT a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::HALF_SIN_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::HALF_SIN_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVec1[i].GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVec1[i].GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVec1[i].GetMin<TypeP>())) > NEATALU::HALF_SIN_ERROR)
                        passed = false;


            if(hasMaxPoint) {
                if( *testIntVec1[i].GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVec1[i].GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVec1[i].GetMax<TypeP>())) > NEATALU::HALF_SIN_ERROR)
                        passed = false;

        }
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate in range */
        NEATVector testAccVec = NEATALU::half_sin<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sin(+0) == +0
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sin(-0) == -0
            } else {
                refAccValFloat = RefALU::sin(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::HALF_SIN_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        NEATVector testIntVec = NEATALU::half_sin<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::sin(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::sin(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::HALF_SIN_ERROR);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, cos)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;


    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                        NUM_TESTS,TypeP(-NEATALU::pi+0.01),TypeP(0-0.01));
    // +- 0.01 to avoid including PI to interval max amd min points are tested
    // on not ranged vectors (firstFloat, secondFloat)
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-NEATALU::pi+0.01),TypeP(0-0.01));

    /* test for specific values */
    NEATValue testAccVal = NEATALU::cos<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::cos<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::cos<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::cos<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::cos<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    testAccVal = NEATALU::cos<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    testAccVal = NEATALU::cos<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::cos<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for monotonic interval from 0 to pi */
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i]);
            if(firstFloatRanged[testIdx*wrap.GetSize()+i] < secondFloatRanged[testIdx*wrap.GetSize()+i])
                intervalRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i], secondFloatRanged[testIdx*wrap.GetSize()+i]);
            else
                intervalRanged[i] = NEATValue(secondFloatRanged[testIdx*wrap.GetSize()+i], firstFloatRanged[testIdx*wrap.GetSize()+i]);
        }

        /* test for single accurate NEAT value*/
        SuperT refAccValFloat;
        accVal = accurate[0];

        testAccVal = NEATALU::cos<TypeP>(accVal);

        if ((Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0))) ||
            (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0))))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));
                // cos(+-0) == 1
        } else {
            refAccValFloat = RefALU::cos(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,NEATALU::COS_ERROR);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::cos<TypeP>(intVal);

        bool passed = true;

        {
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*intVal.GetMax<TypeP>(),
                         RefALU::neg(*intVal.GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*intVal.GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::two_pi) );
            }

            refIntValMax = RefALU::cos(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::cos(SuperT(*intVal.GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::COS_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::COS_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVal.GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVal.GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVal.GetMin<TypeP>())) > NEATALU::COS_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if(*testIntVal.GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVal.GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVal.GetMax<TypeP>())) > NEATALU::COS_ERROR)
                        passed = false;

            EXPECT_TRUE(passed);
        }


        /* test for vector of NEAT intervals */
        passed = true;
        NEATVector testIntVec1 = NEATALU::cos<TypeP>(interval);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {

            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*interval[i].GetMax<TypeP>(),
                         RefALU::neg(*interval[i].GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*interval[i].GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::two_pi) );
            }

            refIntValMax = RefALU::cos(SuperT(*interval[i].GetMax<TypeP>()));
            refIntValMin = RefALU::cos(SuperT(*interval[i].GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::COS_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::COS_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVec1[i].GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVec1[i].GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVec1[i].GetMin<TypeP>())) > NEATALU::COS_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if( *testIntVec1[i].GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVec1[i].GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVec1[i].GetMax<TypeP>())) > NEATALU::COS_ERROR)
                        passed = false;
        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate in range */
        NEATVector testAccVec = NEATALU::cos<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if ((Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0))) ||
                (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0))))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(1.0)));
                // cos(+-0) == 1
            } else {
                refAccValFloat = RefALU::cos(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::COS_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        NEATVector testIntVec = NEATALU::cos<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::cos(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::cos(SuperT(*intervalRanged[i].GetMax<TypeP>()));
            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::COS_ERROR);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, native_cos)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;


    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                        NUM_TESTS,TypeP(-NEATALU::pi+0.01),TypeP(0-0.01));
    // +- 0.01 to avoid including PI to interval max amd min points are tested
    // on not ranged vectors (firstFloat, secondFloat)
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-NEATALU::pi+0.01),TypeP(0-0.01));

    /* test for specific values */
    NEATValue testAccVal = NEATALU::native_cos<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_cos<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_cos<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_cos<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::native_cos<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    testAccVal = NEATALU::native_cos<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    testAccVal = NEATALU::native_cos<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::native_cos<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for monotonic interval from 0 to pi */
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i]);
            if(firstFloatRanged[testIdx*wrap.GetSize()+i] < secondFloatRanged[testIdx*wrap.GetSize()+i])
                intervalRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i], secondFloatRanged[testIdx*wrap.GetSize()+i]);
            else
                intervalRanged[i] = NEATValue(secondFloatRanged[testIdx*wrap.GetSize()+i], firstFloatRanged[testIdx*wrap.GetSize()+i]);
        }

        /* test for single accurate NEAT value*/
        SuperT refAccValFloat;
        accVal = accurate[0];

        testAccVal = NEATALU::native_cos<TypeP>(accVal);

        if ((Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0))) ||
            (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0))))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));
                // cos(+-0) == 1
        } else {
            refAccValFloat = RefALU::cos(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,NEATALU::NATIVE_COS_ERROR);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::native_cos<TypeP>(intVal);

        bool passed = true;

        {
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*intVal.GetMax<TypeP>(),
                         RefALU::neg(*intVal.GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*intVal.GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::two_pi) );
            }

            refIntValMax = RefALU::cos(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::cos(SuperT(*intVal.GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::NATIVE_COS_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::NATIVE_COS_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVal.GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVal.GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVal.GetMin<TypeP>())) > NEATALU::NATIVE_COS_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if(*testIntVal.GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVal.GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVal.GetMax<TypeP>())) > NEATALU::NATIVE_COS_ERROR)
                        passed = false;

            EXPECT_TRUE(passed);
        }


        /* test for vector of NEAT intervals */
        passed = true;
        NEATVector testIntVec1 = NEATALU::native_cos<TypeP>(interval);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {

            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*interval[i].GetMax<TypeP>(),
                         RefALU::neg(*interval[i].GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*interval[i].GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::two_pi) );
            }

            refIntValMax = RefALU::cos(SuperT(*interval[i].GetMax<TypeP>()));
            refIntValMin = RefALU::cos(SuperT(*interval[i].GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::NATIVE_COS_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::NATIVE_COS_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVec1[i].GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVec1[i].GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVec1[i].GetMin<TypeP>())) > NEATALU::NATIVE_COS_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if( *testIntVec1[i].GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVec1[i].GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVec1[i].GetMax<TypeP>())) > NEATALU::NATIVE_COS_ERROR)
                        passed = false;
        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate in range */
        NEATVector testAccVec = NEATALU::native_cos<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if ((Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0))) ||
                (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0))))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(1.0)));
                // cos(+-0) == 1
            } else {
                refAccValFloat = RefALU::cos(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::NATIVE_COS_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        NEATVector testIntVec = NEATALU::native_cos<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::cos(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::cos(SuperT(*intervalRanged[i].GetMax<TypeP>()));
            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::NATIVE_COS_ERROR);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, half_cos)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;


    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                        NUM_TESTS,TypeP(-NEATALU::pi+0.01),TypeP(0-0.01));
    // +- 0.01 to avoid including PI to interval max amd min points are tested
    // on not ranged vectors (firstFloat, secondFloat)
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-NEATALU::pi+0.01),TypeP(0-0.01));

    /* test for specific values */
    NEATValue testAccVal = NEATALU::half_cos<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_cos<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_cos<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_cos<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::half_cos<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    testAccVal = NEATALU::half_cos<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    testAccVal = NEATALU::half_cos<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::half_cos<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for monotonic interval from 0 to pi */
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i]);
            if(firstFloatRanged[testIdx*wrap.GetSize()+i] < secondFloatRanged[testIdx*wrap.GetSize()+i])
                intervalRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i], secondFloatRanged[testIdx*wrap.GetSize()+i]);
            else
                intervalRanged[i] = NEATValue(secondFloatRanged[testIdx*wrap.GetSize()+i], firstFloatRanged[testIdx*wrap.GetSize()+i]);
        }

        /* test for single accurate NEAT value*/
        SuperT refAccValFloat;
        accVal = accurate[0];

        testAccVal = NEATALU::half_cos<TypeP>(accVal);

        if ((Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0))) ||
            (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0))))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));
                // cos(+-0) == 1
        } else {
            refAccValFloat = RefALU::cos(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,NEATALU::HALF_COS_ERROR);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::half_cos<TypeP>(intVal);

        bool passed = true;

        {
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*intVal.GetMax<TypeP>(),
                         RefALU::neg(*intVal.GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*intVal.GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::two_pi) );
            }

            refIntValMax = RefALU::cos(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::cos(SuperT(*intVal.GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::HALF_COS_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::HALF_COS_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVal.GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVal.GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVal.GetMin<TypeP>())) > NEATALU::HALF_COS_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if(*testIntVal.GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVal.GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVal.GetMax<TypeP>())) > NEATALU::HALF_COS_ERROR)
                        passed = false;

            EXPECT_TRUE(passed);
        }


        /* test for vector of NEAT intervals */
        passed = true;
        NEATVector testIntVec1 = NEATALU::half_cos<TypeP>(interval);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {

            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*interval[i].GetMax<TypeP>(),
                         RefALU::neg(*interval[i].GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)NEATALU::two_pi))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*interval[i].GetMin<TypeP>(),
                                 (TypeP)NEATALU::two_pi ) ), (TypeP)NEATALU::two_pi );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::pi) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0*NEATALU::pi) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(NEATALU::two_pi) );
            }

            refIntValMax = RefALU::cos(SuperT(*interval[i].GetMax<TypeP>()));
            refIntValMin = RefALU::cos(SuperT(*interval[i].GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            TypeP dMin = NEATALU::castDown(refIntValMin-NEATALU::HALF_COS_ERROR*NEATALU::ComputeUlp(refIntValMin));
            TypeP dMax = NEATALU::castDown(refIntValMax+NEATALU::HALF_COS_ERROR*NEATALU::ComputeUlp(refIntValMax));

            if(hasMinPoint) {
                if( *testIntVec1[i].GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMin)) && *testIntVec1[i].GetMin<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVec1[i].GetMin<TypeP>())) > NEATALU::HALF_COS_ERROR)
                        passed = false;

            if(hasMaxPoint) {
                if( *testIntVec1[i].GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if (!(Utils::IsDenorm<TypeP>(dMax)) && *testIntVec1[i].GetMax<TypeP>()==0)
                    if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVec1[i].GetMax<TypeP>())) > NEATALU::HALF_COS_ERROR)
                        passed = false;
        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate in range */
        NEATVector testAccVec = NEATALU::half_cos<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if ((Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0))) ||
                (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0))))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(1.0)));
                // cos(+-0) == 1
            } else {
                refAccValFloat = RefALU::cos(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::HALF_COS_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        NEATVector testIntVec = NEATALU::half_cos<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::cos(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::cos(SuperT(*intervalRanged[i].GetMax<TypeP>()));
            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::HALF_COS_ERROR);
            EXPECT_TRUE(passed);
        }
    }
}

//TODO: it is not clear what should be output range if input range includes
// n*pi/2, because tan(pi/2) == INF
TYPED_TEST(NEATAluTypedMath, tan)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
        NUM_TESTS,TypeP(-NEATALU::pi_2),TypeP(NEATALU::pi_2));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-NEATALU::pi_2),TypeP(NEATALU::pi_2));


    /* test for specific values */
    NEATValue testAccVal = NEATALU::tan<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::tan<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::tan<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::tan<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::tan<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::tan<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::tan<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::tan<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -pi/2 to pi/2 */
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

        testAccVal = NEATALU::tan<TypeP>(accVal);
        SuperT refAccValFloat;
        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
            // tan(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
            // tan(-0) == -0
        } else {
            refAccValFloat = RefALU::tan(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,NEATALU::TAN_ERROR);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::tan<TypeP>(intVal);

        refIntValMax = RefALU::tan(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::tan(SuperT(*intVal.GetMin<TypeP>()));

        bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,NEATALU::TAN_ERROR);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::tan<TypeP>(accurate);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sin(+0) == +0
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sin(-0) == -0
            } else {
                refAccValFloat = RefALU::tan(SuperT(*accurate[i].GetAcc<TypeP>()));
                passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::TAN_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::tan<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::tan(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::tan(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::TAN_ERROR);
            EXPECT_TRUE(passed);
        }


        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::tan<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sin(+0) == +0
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sin(-0) == -0
            } else {
                refAccValFloat = RefALU::tan(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::TAN_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::tan<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::tan(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::tan(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::TAN_ERROR);
            EXPECT_TRUE(passed);
        }
    }
}

//TODO: it is not clear what should be output range if input range includes
// n*pi/2, because tan(pi/2) == INF
TYPED_TEST(NEATAluTypedMath, half_tan)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
        NUM_TESTS,TypeP(-NEATALU::pi_2),TypeP(NEATALU::pi_2));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-NEATALU::pi_2),TypeP(NEATALU::pi_2));


    /* test for specific values */
    NEATValue testAccVal = NEATALU::half_tan<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_tan<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_tan<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_tan<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::half_tan<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::half_tan<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::half_tan<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::half_tan<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -pi/2 to pi/2 */
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

        testAccVal = NEATALU::half_tan<TypeP>(accVal);
        SuperT refAccValFloat;
        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
            // half_tan(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
            // half_tan(-0) == -0
        } else {
            refAccValFloat = RefALU::tan(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,NEATALU::HALF_TAN_ERROR);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::half_tan<TypeP>(intVal);

        refIntValMax = RefALU::tan(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::tan(SuperT(*intVal.GetMin<TypeP>()));

        bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,NEATALU::HALF_TAN_ERROR);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::half_tan<TypeP>(accurate);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sin(+0) == +0
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sin(-0) == -0
            } else {
                refAccValFloat = RefALU::tan(SuperT(*accurate[i].GetAcc<TypeP>()));
                passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::HALF_TAN_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::half_tan<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::tan(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::tan(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::HALF_TAN_ERROR);
            EXPECT_TRUE(passed);
        }


        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::half_tan<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sin(+0) == +0
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sin(-0) == -0
            } else {
                refAccValFloat = RefALU::tan(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::HALF_TAN_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::half_tan<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::tan(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::tan(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::HALF_TAN_ERROR);
            EXPECT_TRUE(passed);
        }
    }
}

//TODO: it is not clear what should be output range if input range includes
// n*pi/2, because tan(pi/2) == INF
TYPED_TEST(NEATAluTypedMath, native_tan)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
        NUM_TESTS,TypeP(-NEATALU::pi_2),TypeP(NEATALU::pi_2));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-NEATALU::pi_2),TypeP(NEATALU::pi_2));


    /* test for specific values */
    NEATValue testAccVal = NEATALU::native_tan<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_tan<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_tan<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_tan<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::native_tan<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::native_tan<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::native_tan<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::native_tan<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -pi/2 to pi/2 */
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

        testAccVal = NEATALU::native_tan<TypeP>(accVal);
        SuperT refAccValFloat;
        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
            // native_tan(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
            // native_tan(-0) == -0
        } else {
            refAccValFloat = RefALU::tan(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,NEATALU::NATIVE_TAN_ERROR);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::native_tan<TypeP>(intVal);

        refIntValMax = RefALU::tan(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::tan(SuperT(*intVal.GetMin<TypeP>()));

        bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,NEATALU::NATIVE_TAN_ERROR);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::native_tan<TypeP>(accurate);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sin(+0) == +0
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sin(-0) == -0
            } else {
                refAccValFloat = RefALU::tan(SuperT(*accurate[i].GetAcc<TypeP>()));
                passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::NATIVE_TAN_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::native_tan<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::tan(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::tan(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::NATIVE_TAN_ERROR);
            EXPECT_TRUE(passed);
        }


        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::native_tan<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sin(+0) == +0
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sin(-0) == -0
            } else {
                refAccValFloat = RefALU::tan(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],NEATALU::NATIVE_TAN_ERROR);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::native_tan<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::tan(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::tan(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],NEATALU::NATIVE_TAN_ERROR);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, tanpi)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                             NUM_TESTS,TypeP(-0.5),TypeP(0.5));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-0.5),TypeP(0.5));


    /* test for specific values */
    // tanpi ( ±0 ) returns ±0
    NEATValue accVal = NEATValue(TypeP(+0.0));
    NEATValue testAccVal = NEATALU::tanpi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    accVal = NEATValue(TypeP(-0.0));
    testAccVal = NEATALU::tanpi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    // tanpi ( ±INF ) returns a NaN
    accVal = NEATValue(Utils::GetPInf<TypeP>());
    testAccVal = NEATALU::tanpi<TypeP>(accVal);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
    accVal = NEATValue(Utils::GetNInf<TypeP>());
    testAccVal = NEATALU::tanpi<TypeP>(accVal);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    // tanpi (n+0.5) for even integer n is +INF where n+0.5 is representable
    accVal = NEATValue(TypeP(2.5));
    testAccVal = NEATALU::tanpi<TypeP>(accVal);
    EXPECT_TRUE(Utils::IsPInf(*testAccVal.GetAcc<TypeP>()));
    // tanpi (n+0.5) for odd integer n is -INF where n+0.5 is representable
    accVal = NEATValue(TypeP(3.5));
    testAccVal = NEATALU::tanpi<TypeP>(accVal);
    EXPECT_TRUE(Utils::IsNInf(*testAccVal.GetAcc<TypeP>()));

//TODO:
    // tanpi ( n ) is RefCopysign( 0.0, n) for even integers n.
    // tanpi ( n ) is RefCopysign( 0.0, - n) for odd integers n.

    testAccVal = NEATALU::tanpi<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::tanpi<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::tanpi<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::tanpi<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -pi/2 to pi/2 */
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

        testAccVal = NEATALU::tanpi<TypeP>(accVal);
        SuperT refAccValFloat;

        refAccValFloat = RefALU::tanpi(SuperT(*accVal.GetAcc<TypeP>()));

        bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,6.f);
        EXPECT_TRUE(passed);


        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::tanpi<TypeP>(intVal);

        refIntValMax = RefALU::tanpi(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::tanpi(SuperT(*intVal.GetMin<TypeP>()));

        passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,6.f);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::tanpi<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::tanpi(SuperT(*accurate[i].GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],6.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::tanpi<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::tanpi(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::tanpi(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],6.f);
            EXPECT_TRUE(passed);
        }


        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::tanpi<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::tanpi(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
            passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],6.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::tanpi<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::tanpi(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::tanpi(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],6.f);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, asin)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                                      NUM_TESTS,TypeP(-1.0+0.001),TypeP(1.0-0.001));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-1.0+0.001),TypeP(1.0-0.001));


    /* test for specific values */
    NEATValue testAccVal = NEATALU::asin<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::asin<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::asin<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::asin<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    // asin(+0)==+0
    testAccVal = NEATALU::asin<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
    // asin(-0)==-0
    testAccVal = NEATALU::asin<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -1.0 to 1.0 */
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

        testAccVal = NEATALU::asin<TypeP>(accVal);
        SuperT  refAccValFloat;

        if((*accVal.GetAcc<TypeP>() > TypeP(1.0)) ||
           (*accVal.GetAcc<TypeP>() < TypeP(-1.0) ))
        {
            /* if accurate src < -1.0 or src > 1.0 asin returns NaN */
            EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0))); // asin(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0))); // asin(-0) == -0
        } else {
            refAccValFloat = RefALU::asin(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,4.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::asin<TypeP>(intVal);

        if( *intVal.GetMax<TypeP>() < TypeP(-1.0) ||
            *intVal.GetMin<TypeP>() > TypeP(1.0))
            EXPECT_TRUE(testIntVal.IsNaN<TypeP>());
        else if( *intVal.GetMin<TypeP>() < TypeP(-1.0) ||
                 *intVal.GetMax<TypeP>() > TypeP(1.0))
                 EXPECT_TRUE(testIntVal.IsUnknown());
        else {
            refIntValMax = RefALU::asin(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::asin(SuperT(*intVal.GetMin<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,4.f);
            EXPECT_TRUE(passed);
         }

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::asin<TypeP>(accurate);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        if((*accurate[i].GetAcc<TypeP>() > TypeP(1.0)) ||
           (*accurate[i].GetAcc<TypeP>() < TypeP(-1.0) ))
            {
                /* if accurate src < -1.0 or src > 1.0 asin returns NaN */
                EXPECT_TRUE(testAccVec[i].IsNaN<TypeP>());
            }  else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0))); // asin(+0) == +0
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0))); // asin(-0) == -0
            } else {
                refAccValFloat = RefALU::asin(SuperT(*accurate[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],4.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::asin<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if((*interval[i].GetMax<TypeP>() < TypeP(-1.0)) ||
               (*interval[i].GetMin<TypeP>() > TypeP(1.0) )) {
                /* if the whole interval is upper or lower (-1;1) asin returns NaN */
                EXPECT_TRUE(testIntVec[i].IsNaN<TypeP>());
            }
            else if((*interval[i].GetMax<TypeP>() > TypeP(1.0)) ||
               (*interval[i].GetMin<TypeP>() < TypeP(-1.0) )) {
                /* if random src < -1.0 or src > 1.0 asin returns UNKNOWN */
                EXPECT_TRUE(testIntVec[i].IsUnknown());
            } else {
                refIntValMin = RefALU::asin(SuperT(*interval[i].GetMin<TypeP>()));
                refIntValMax = RefALU::asin(SuperT(*interval[i].GetMax<TypeP>()));

                bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],4.f);
                EXPECT_TRUE(passed);
            }
        }


        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::asin<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // asin(+0) == +0
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // asin(-0) == -0
            } else {
                refAccValFloat = RefALU::asin(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],4.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::asin<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::asin(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::asin(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],4.f);
            EXPECT_TRUE(passed);
        }
    }
}


TYPED_TEST(NEATAluTypedMath, acos)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
        NUM_TESTS,TypeP(-1.0+0.001),TypeP(1.0-0.001));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-1.0+0.001),TypeP(1.0-0.001));

    /* test for specific values */

    NEATValue testAccVal = NEATALU::acos<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::acos<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::acos<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::acos<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::acos<TypeP>(NEATValue(TypeP(1.0),TypeP(1.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.0)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -1.0 to 1.0 */
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

        testAccVal = NEATALU::acos<TypeP>(accVal);
        SuperT refAccValFloat;

        if((*accVal.GetAcc<TypeP>() > TypeP(1.0)) ||
           (*accVal.GetAcc<TypeP>() < TypeP(-1.0) ))
        {
            /* if accurate src < -1.0 or src > 1.0 acos returns NaNa */
            EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
        } else if (*accVal.GetAcc<TypeP>() == TypeP(1.0))
        {
            // special accurate value acos(1)==0
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.0)));
        } else {
            refAccValFloat = RefALU::acos(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,4.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::acos<TypeP>(intVal);

        if( *intVal.GetMax<TypeP>() < TypeP(-1.0) ||
            *intVal.GetMin<TypeP>() > TypeP(1.0))
            EXPECT_TRUE(testIntVal.IsNaN<TypeP>());
        else if( *intVal.GetMin<TypeP>() < TypeP(-1.0) ||
            *intVal.GetMax<TypeP>() > TypeP(1.0))
        {
            EXPECT_TRUE(testIntVal.IsUnknown());
        }
        else {
            refIntValMax = RefALU::acos(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::acos(SuperT(*intVal.GetMin<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,4.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::acos<TypeP>(accurate);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        if((*accurate[i].GetAcc<TypeP>() > TypeP(1.0)) ||
           (*accurate[i].GetAcc<TypeP>() < TypeP(-1.0) ))
            {
                /* if accurate src < -1.0 or src > 1.0 acos returns NaN */
                EXPECT_TRUE(testAccVec[i].IsNaN<TypeP>());
            }
            else if (*accurate[i].GetAcc<TypeP>() == TypeP(1.0))
            {
                // special accurate value acos(1)==0
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
            } else {
                refAccValFloat = RefALU::acos(SuperT(*accurate[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],4.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::acos<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if((*interval[i].GetMax<TypeP>() < TypeP(-1.0)) ||
               (*interval[i].GetMin<TypeP>() > TypeP(1.0) )) {
                /* if the whole interval is upper or lower (-1;1) acos returns NaN */
                EXPECT_TRUE(testIntVec[i].IsNaN<TypeP>());
            }
            else if((*interval[i].GetMax<TypeP>() > TypeP(1.0)) ||
               (*interval[i].GetMin<TypeP>() < TypeP(-1.0) )) {
                /* if random src < -1.0 or src > 1.0 acos returns UNKNOWN */
                EXPECT_TRUE(testIntVec[i].IsUnknown());
            } else {
                refIntValMin = RefALU::acos(SuperT(*interval[i].GetMin<TypeP>()));
                refIntValMax = RefALU::acos(SuperT(*interval[i].GetMax<TypeP>()));

                bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],4.f);
                EXPECT_TRUE(passed);
            }
        }


        /* test for vector of NEAT accurate in range */
        // accurateRanged[i]==1 is not tested, because generated interval not includes it
        testAccVec = NEATALU::acos<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::acos(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],4.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::acos<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::acos(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::acos(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],4.f);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, atan)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    /* just to reduce the interval from the full range of current data type */
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                                      NUM_TESTS,TypeP(-100.0),TypeP(100.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-100.0),TypeP(100.0));


    /* test for specific values */
    NEATValue testAccVal = NEATALU::atan<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atan<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atan<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atan<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::atan<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::atan<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::atan<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(NEATALU::pi_2)));

    testAccVal = NEATALU::atan<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-NEATALU::pi_2)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -100 to 100 */
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

        testAccVal = NEATALU::atan<TypeP>(accVal);
        SuperT refAccValFloat;

        refAccValFloat = RefALU::atan(SuperT(*accVal.GetAcc<TypeP>()));
        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
            // atan(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
            // atan(-0) == -0
        } else {
            EXPECT_TRUE(TestAccExpanded<SuperT>(refAccValFloat,testAccVal,5.f));
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::atan<TypeP>(intVal);

        refIntValMax = RefALU::atan(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::atan(SuperT(*intVal.GetMin<TypeP>()));

        bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,5.f);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::atan<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // atan(+0) == +0
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // atan(-0) == -0
            } else {
                refAccValFloat = RefALU::atan(SuperT(*accurate[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::atan<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::atan(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::atan(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],5.f);
            EXPECT_TRUE(passed);
        }


        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::atan<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // atan(+0) == +0
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // atan(-0) == -0
            } else {
                refAccValFloat = RefALU::atan(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::atan<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::atan(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::atan(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],5.f);
            EXPECT_TRUE(passed);
        }
    }
}


TYPED_TEST(NEATAluTypedMath, sinh)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    // result of sinh(x) with |x| > 10 is close to INF, so test for ranged argument is enough
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                      NUM_TESTS,TypeP(-10.0),TypeP(10.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth,
                              NUM_TESTS,TypeP(-10.0),TypeP(10.0));


    /* test for specific values */
    NEATValue testAccVal = NEATALU::sinh<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sinh<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sinh<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sinh<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::sinh<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::sinh<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::sinh<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(Utils::IsPInf(*testAccVal.GetAcc<TypeP>()));

    testAccVal = NEATALU::sinh<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(Utils::IsNInf(*testAccVal.GetAcc<TypeP>()));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

       /* test for single accurate NEAT value*/
        accVal = accurate[0];

        testAccVal = NEATALU::sinh<TypeP>(accVal);
        SuperT refAccValFloat;

        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
            // sinh(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
            // sinh(-0) == -0
        } else {
            refAccValFloat = RefALU::sinh(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,4.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::sinh<TypeP>(intVal);

        refIntValMax = RefALU::sinh(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::sinh(SuperT(*intVal.GetMin<TypeP>()));

        bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,4.f);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::sinh<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // sinh(+0) == +0
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // sinh(-0) == -0
            } else {
                refAccValFloat = RefALU::sinh(SuperT(*accurate[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],4.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::sinh<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::sinh(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::sinh(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],4.f);
            EXPECT_TRUE(passed);
        }
    }
}


TYPED_TEST(NEATAluTypedMath, cosh)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    VectorWidth curWidth = V8;

   // we use data range from -10 to 10 because with greater data sinh result
   // reaches infinity (at least for float data)

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                      NUM_TESTS,TypeP(-10.0),TypeP(10.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth,
                              NUM_TESTS,TypeP(-10.0),TypeP(10.0));

    /* test for specific values */
    NEATValue testAccVal = NEATALU::cosh<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::cosh<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::cosh<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::cosh<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::cosh<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    testAccVal = NEATALU::cosh<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    testAccVal = NEATALU::cosh<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(Utils::IsPInf(*testAccVal.GetAcc<TypeP>()));

    testAccVal = NEATALU::cosh<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(Utils::IsNInf(*testAccVal.GetAcc<TypeP>()));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }


       /* test for single accurate NEAT value*/
        accVal = accurate[0];

        testAccVal = NEATALU::cosh<TypeP>(accVal);
        SuperT refAccValFloat;

        if ((Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0))) ||
            (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0))))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));
            // cosh(+-0) == 1
        } else {
            refAccValFloat = RefALU::cosh(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,4.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;
        intVal = interval[0];

        testIntVal = NEATALU::cosh<TypeP>(intVal);

        refIntValMax = RefALU::cosh(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::cosh(SuperT(*intVal.GetMin<TypeP>()));

        bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,4.f);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::cosh<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if ((Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0))) ||
                (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0))))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(1.0)));
                // cosh(+-0) == 1
            } else {
                refAccValFloat = RefALU::cosh(SuperT(*accurate[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],4.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::cosh<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::cosh(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::cosh(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],4.f);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, tanh)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    VectorWidth curWidth = V8;


   // we use data range from -7 to 7 because with greater data tanh result reaches its limit = 1.0 (or -1.0)
   // (at least for float data)

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                      NUM_TESTS,TypeP(-7.0),TypeP(7.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth,
                              NUM_TESTS,TypeP(-7.0),TypeP(7.0));


    /* test for specific values */
    NEATValue testAccVal = NEATALU::tanh<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::tanh<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::tanh<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::tanh<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::tanh<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::tanh<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::tanh<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+1.0)));

    testAccVal = NEATALU::tanh<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-1.0)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }


       /* test for single accurate NEAT value*/
        accVal = accurate[0];
        testAccVal = NEATALU::tanh<TypeP>(accVal);
        SuperT refAccValFloat;

        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
            // tanh(+0) == +0
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
            // tanh(-0) == -0
        } else {
            refAccValFloat = RefALU::tanh(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,5.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;
        intVal = interval[0];

        testIntVal = NEATALU::tanh<TypeP>(intVal);

        refIntValMax = RefALU::tanh(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::tanh(SuperT(*intVal.GetMin<TypeP>()));

        bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,5.f);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::tanh<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
                // tanh(+0) == +0
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
                // tanh(-0) == -0
            } else {
                refAccValFloat = RefALU::tanh(SuperT(*accurate[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::tanh<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::tanh(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::tanh(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],5.f);
            EXPECT_TRUE(passed);
        }
    }
}

/* sin and cos are tested by own tests, so here we just have a kind of sanity
   check that sin is sin and cos is cos */
TYPED_TEST(NEATAluTypedMath, sincos)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 100;

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    // choosen interval not includes max and min for both functions
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                                  NUM_TESTS,TypeP(0.1),TypeP(NEATALU::pi_2-0.1));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                                  NUM_TESTS,TypeP(0.1),TypeP(NEATALU::pi_2-0.1));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccValSin, testIntValSin, testAccValCos, testIntValCos;
        NEATValue refAccValSin, refIntValSin, refAccValCos, refIntValCos;

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        /* load data for interval from 0.1 to pi/2-0.1 */
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i]);
            if(firstFloatRanged[testIdx*wrap.GetSize()+i] < secondFloatRanged[testIdx*wrap.GetSize()+i])
                intervalRanged[i] = NEATValue(firstFloatRanged[testIdx*wrap.GetSize()+i], secondFloatRanged[testIdx*wrap.GetSize()+i]);
            else
                intervalRanged[i] = NEATValue(secondFloatRanged[testIdx*wrap.GetSize()+i], firstFloatRanged[testIdx*wrap.GetSize()+i]);
        }

       /* test for single accurate NEAT value*/
        accVal = accurateRanged[0];
        // init is not needed, testAccValCos to be changed by sincos result
        testAccValCos = accurateRanged[1];

        testAccValSin = NEATALU::sincos<TypeP>(accVal,&testAccValCos);

        SuperT refAccValFloatSin, refAccValFloatCos;

        refAccValFloatSin = RefALU::sin(SuperT(*accVal.GetAcc<TypeP>()));

        refAccValFloatCos = RefALU::cos(SuperT(*accVal.GetAcc<TypeP>()));
        bool passed = TestAccExpanded<SuperT>(refAccValFloatSin,testAccValSin,4.f);
        EXPECT_TRUE(passed);

        passed = TestAccExpanded<SuperT>(refAccValFloatCos,testAccValCos,4.f);
        EXPECT_TRUE(passed);

        /* test for single interval NEAT value */
        SuperT refIntValSinMin, refIntValSinMax, refIntValCosMin, refIntValCosMax;

        intVal = intervalRanged[0];

        // actually init is not needed here
        testIntValCos = intervalRanged[1];

        refIntValSinMax = RefALU::sin(SuperT(*intVal.GetMax<TypeP>()));
        refIntValSinMin = RefALU::sin(SuperT(*intVal.GetMin<TypeP>()));
        refIntValCosMax = RefALU::cos(SuperT(*intVal.GetMax<TypeP>()));
        refIntValCosMin = RefALU::cos(SuperT(*intVal.GetMin<TypeP>()));

        testIntValSin = NEATALU::sincos<TypeP>(intVal,&testIntValCos);

        passed = TestIntExpanded<SuperT>(refIntValSinMin,refIntValSinMax,testIntValSin,4.f);
        EXPECT_TRUE(passed);

        passed = TestIntExpanded<SuperT>(refIntValCosMin,refIntValCosMax,testIntValCos,4.f);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVecCos = accurateRanged;
        NEATVector testAccVecSin = NEATALU::sincos<TypeP>(accurateRanged,testAccVecCos);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloatSin = RefALU::sin(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
            refAccValFloatCos = RefALU::cos(SuperT(*accurateRanged[i].GetAcc<TypeP>()));

            bool passed = TestAccExpanded<SuperT>(refAccValFloatSin,testAccVecSin[i],4.f);
            EXPECT_TRUE(passed);

            passed = TestAccExpanded<SuperT>(refAccValFloatCos,testAccVecCos[i],4.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVecCos = intervalRanged;
        NEATVector testIntVecSin = NEATALU::sincos<TypeP>(intervalRanged,testIntVecCos);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValSinMin = RefALU::sin(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValSinMax = RefALU::sin(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            refIntValCosMin = RefALU::cos(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValCosMax = RefALU::cos(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValSinMin,refIntValSinMax,testIntVecSin[i],4.f);
            EXPECT_TRUE(passed);

            passed = TestIntExpanded<SuperT>(refIntValCosMin,refIntValCosMax,testIntVecCos[i],4.f);
            EXPECT_TRUE(passed);
        }
    }
}


TYPED_TEST(NEATAluTypedMath, sinpi)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0],
                                  curWidth, NUM_TESTS, TypeP(-0.49),
                                  TypeP(0.49));

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-0.49),TypeP(0.49));


    /* test for specific values */
    // sinpi (+-0 ) returns +-0
    NEATValue accVal = NEATValue(TypeP(+0.0),TypeP(+0.0));
    NEATValue testAccVal = NEATALU::sinpi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    accVal = NEATValue(TypeP(-0.0),TypeP(-0.0));
    testAccVal = NEATALU::sinpi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    //sinpi ( ±INF ) returns a NaN
    accVal = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    testAccVal = NEATALU::sinpi<TypeP>(accVal);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
    accVal = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    testAccVal = NEATALU::sinpi<TypeP>(accVal);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::sinpi<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sinpi<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sinpi<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sinpi<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    // sinpi ( -n ) returns -0 for negative integers n
    accVal = NEATValue(TypeP(-2.0));
    testAccVal = NEATALU::sinpi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    accVal = NEATValue(TypeP(-6.0));
    testAccVal = NEATALU::sinpi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    // sinpi ( +n ) returns +0 for positive integers n
    accVal = NEATValue(TypeP(3.0));
    testAccVal = NEATALU::sinpi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    accVal = NEATValue(TypeP(4.0));
    testAccVal = NEATALU::sinpi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;
        SuperT refAccValFloat;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for monotonic interval from -pi/2 to pi/2,i.e. from -1/2 to 1/2 for sinpi */
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

        testAccVal = NEATALU::sinpi<TypeP>(accVal);

        TypeP data = *accVal.GetAcc<TypeP>();
        double intpartF;
        TypeP fractpart = modf(double(data), &intpartF);

        if( Utils::eq<TypeP>(*accVal.GetAcc<TypeP>(), -0.0f) ||
           (fractpart == 0 && *accVal.GetAcc<TypeP>() < 0 ))
        {
            // sinpi ( -0 ) returns -0
            // sinpi ( -n ) returns -0 for negative integers n
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

        } else if( Utils::eq<TypeP>(*accVal.GetAcc<TypeP>(), +0.0f) ||
           (fractpart == 0 && *accVal.GetAcc<TypeP>() >= 0 ))
        {
            // sinpi ( +0 ) returns +0
            // sinpi ( +n ) returns +0 for positive integers n
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
        } else {
            refAccValFloat = RefALU::sinpi(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,4.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::sinpi<TypeP>(intVal);

        bool passed = true;

        {
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*intVal.GetMax<TypeP>(),
                         RefALU::neg(*intVal.GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)2.0))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*intVal.GetMin<TypeP>(),
                                 (TypeP)2.0 ) ), (TypeP)2.0 );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(1.5) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.5) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.5) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(2.5) );
            }

            refIntValMax = RefALU::sinpi(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::sinpi(SuperT(*intVal.GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            if(hasMinPoint) {
                if( *testIntVal.GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVal.GetMin<TypeP>())) > 4.f)
                    passed = false;

            if(hasMaxPoint) {
                if( *testIntVal.GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVal.GetMax<TypeP>())) > 4.f)
                    passed = false;
        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT intervals */
        passed = true;
        NEATVector testIntVec1 = NEATALU::sinpi<TypeP>(interval);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {

            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*interval[i].GetMax<TypeP>(),
                         RefALU::neg(*interval[i].GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)2.0))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*interval[i].GetMin<TypeP>(),
                                 (TypeP)2.0 ) ), (TypeP)2.0 );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(1.5) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.5) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.5) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(2.5) );
            }

            refIntValMax = RefALU::sinpi(SuperT(*interval[i].GetMax<TypeP>()));
            refIntValMin = RefALU::sinpi(SuperT(*interval[i].GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            if(hasMinPoint) {
                if( *testIntVec1[i].GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVec1[i].GetMin<TypeP>())) > 4.f)
                    passed = false;

            if(hasMaxPoint) {
                if( *testIntVec1[i].GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVec1[i].GetMax<TypeP>())) > 4.f)
                    passed = false;

        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate in range */
        passed = true;

        NEATVector testAccVec = NEATALU::sinpi<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {

            TypeP data = *accurateRanged[i].GetAcc<TypeP>();
            double intpartF;
            TypeP fractpart = modf(double(data), &intpartF);

            if( Utils::eq<TypeP>(*accurateRanged[i].GetAcc<TypeP>(), -0.0f) ||
               (fractpart == 0 && *accurateRanged[i].GetAcc<TypeP>() < 0 ))
            {
                // sinpi ( -0 ) returns -0
                // sinpi ( -n ) returns -0 for negative integers n
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));

            } else if( Utils::eq<TypeP>(*accurateRanged[i].GetAcc<TypeP>(), +0.0f) ||
               (fractpart == 0 && *accurateRanged[i].GetAcc<TypeP>() >= 0 ))
            {
                // sinpi ( +0 ) returns +0
                // sinpi ( +n ) returns +0 for positive integers n
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
            } else {
                refAccValFloat = RefALU::sinpi(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],4.f);
                EXPECT_TRUE(passed);
            }


        }

        /* test for vector of NEAT intervals in range */
        NEATVector testIntVec = NEATALU::sinpi<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::sinpi(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::sinpi(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],4.f);
            EXPECT_TRUE(passed);
        }
    }
}


TYPED_TEST(NEATAluTypedMath, cospi)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                                      NUM_TESTS,TypeP(0.1),TypeP(0.9));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(0.1),TypeP(0.9));


    /* test for specific values */
    // cospi ( ±0 ) returns 1
    NEATValue accVal = NEATValue(TypeP(+0.0),TypeP(+0.0));
    NEATValue testAccVal = NEATALU::cospi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    accVal = NEATValue(TypeP(-0.0),TypeP(-0.0));
    testAccVal = NEATALU::cospi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    //cospi ( ±INF ) returns a NaN
    accVal = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    testAccVal = NEATALU::cospi<TypeP>(accVal);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
    accVal = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    testAccVal = NEATALU::cospi<TypeP>(accVal);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::cospi<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::cospi<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::cospi<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::cospi<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    // cospi ( n + 0.5 ) is +0 for any integer n where n + 0.5 is representable
    accVal = NEATValue(TypeP(-2.5));
    testAccVal = NEATALU::cospi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    accVal = NEATValue(TypeP(3.5));
    testAccVal = NEATALU::cospi<TypeP>(accVal);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for monotonic interval from 0 to pi, i.e. from 0.0 to 1.0 */
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

        testAccVal = NEATALU::cospi<TypeP>(accVal);
        SuperT refAccValFloat;

        TypeP data = fabs(*accVal.GetAcc<TypeP>());

        if(data == 0)
            // cospi(+-0)==1
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));
        else {
            double intpartF;
            TypeP fractpart = modf(double(data), &intpartF);;
            if(fractpart == TypeP(0.5))
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.0)));
            else {
                // TODO: for TypeP==double it should be ldexp( (double)0x1LL, 54)
                SuperT limit = ldexp( (double)0x1LL, 52);
                if( (SuperT)data >= limit)
                {
                    //TODO: check result for EQ 1.0
                    //bool passed = TestAccExpanded<SuperT>(SuperT(1.0),testAccVal,4.f);
                    //EXPECT_TRUE(passed);
                }
                else {
                    refAccValFloat = RefALU::cospi(SuperT(*accVal.GetAcc<TypeP>()));
                    bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,4.f);
                    EXPECT_TRUE(passed);
                }
            }
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::cospi<TypeP>(intVal);

        bool passed = true;

        {
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*intVal.GetMax<TypeP>(),
                         RefALU::neg(*intVal.GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)2.0))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*intVal.GetMin<TypeP>(),
                                 (TypeP)2.0 ) ), (TypeP)2.0 );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(1.0) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(2.0) );
            }

            refIntValMax = RefALU::cospi(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::cospi(SuperT(*intVal.GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            if(hasMinPoint) {
                if( *testIntVal.GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVal.GetMin<TypeP>())) > 4.f)
                    passed = false;

            if(hasMaxPoint) {
                if( *testIntVal.GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVal.GetMax<TypeP>())) > 4.f)
                    passed = false;
        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT intervals */
        passed = true;
        NEATVector testIntVec1 = NEATALU::cospi<TypeP>(interval);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {

            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            TypeP diff = RefALU::add(*interval[i].GetMax<TypeP>(),
                         RefALU::neg(*interval[i].GetMin<TypeP>()) );

            if(Utils::ge(diff, (TypeP)2.0))
            {
                hasMinPoint = true;
                hasMaxPoint = true;
            } else {
                TypeP offStart = RefALU::mul<TypeP>( RefALU::frc<TypeP>(
                                 RefALU::div<TypeP>(*interval[i].GetMin<TypeP>(),
                                 (TypeP)2.0 ) ), (TypeP)2.0 );

                TypeP offEnd= RefALU::add<TypeP>( offStart, diff );

                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(1.0) );
                hasMinPoint |= range.Includes<TypeP>( (TypeP)(3.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(0.0) );
                hasMaxPoint |= range.Includes<TypeP>( (TypeP)(2.0) );
            }

            refIntValMax = RefALU::cospi(SuperT(*interval[i].GetMax<TypeP>()));
            refIntValMin = RefALU::cospi(SuperT(*interval[i].GetMin<TypeP>()));

            if(refIntValMin > refIntValMax) {
                typename  superT<TypeP>::type a = refIntValMax;
                refIntValMax = refIntValMin;
                refIntValMin = a;
            }

            if(hasMinPoint) {
                if( *testIntVec1[i].GetMin<TypeP>() != TypeP(-1.0))
                    passed = false;
            } else
                if(fabs( Utils::ulpsDiff(refIntValMin,*testIntVec1[i].GetMin<TypeP>())) > 4.f)
                    passed = false;

            if(hasMaxPoint) {
                if( *testIntVec1[i].GetMax<TypeP>() != TypeP(1.0))
                    passed = false;
            } else
                if(fabs( Utils::ulpsDiff(refIntValMax,*testIntVec1[i].GetMax<TypeP>())) > 4.f)
                    passed = false;
        }

        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate in range */
        NEATVector testAccVec = NEATALU::cospi<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
        TypeP data = fabs(*accurateRanged[i].GetAcc<TypeP>());
            if(data == 0)
                // cospi(+-0)==1
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(1.0)));
            else {
                double intpartF;
                TypeP fractpart = modf(double(data), &intpartF);
                if(fractpart == TypeP(0.5))
                     EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(0.0)));
                else {
                     refAccValFloat = RefALU::cospi(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                     bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],4.f);
                     EXPECT_TRUE(passed);
                }
            }
        }

        /* test for vector of NEAT intervals in range */
        NEATVector testIntVec = NEATALU::cospi<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::cospi(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::cospi(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],4.f);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, asinpi)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                                      NUM_TESTS,TypeP(-0.999),TypeP(0.999));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                              NUM_TESTS,TypeP(-0.999),TypeP(0.999));


    /* test for specific values */
    NEATValue testAccVal = NEATALU::asinpi<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::asinpi<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::asinpi<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::asinpi<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    //asinpi ( ±0 ) = ±0
    testAccVal = NEATALU::asinpi<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::asinpi<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -1.0 to 1.0 */
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

        testAccVal = NEATALU::asinpi<TypeP>(accVal);
        SuperT refAccValFloat;

        if((*accVal.GetAcc<TypeP>() > TypeP(1.0)) ||
           (*accVal.GetAcc<TypeP>() < TypeP(-1.0) ))
        {
            /* if random src < -1.0 or src > 1.0 asinpi returns NaN */
            EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
        } else {
            refAccValFloat = RefALU::asinpi(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,5.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::asinpi<TypeP>(intVal);

        if( *intVal.GetMin<TypeP>() < TypeP(-1.0) ||
            *intVal.GetMax<TypeP>() > TypeP(1.0))
            EXPECT_TRUE(testIntVal.IsNaN<TypeP>());
        else {
            refIntValMax = RefALU::asinpi(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::asinpi(SuperT(*intVal.GetMin<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,5.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::asinpi<TypeP>(accurate);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if((*accurate[i].GetAcc<TypeP>() > TypeP(1.0)) ||
            (*accurate[i].GetAcc<TypeP>() < TypeP(-1.0) ))
            {
                /* if random src < -1.0 or src > 1.0 asinpi returns NaN */
                EXPECT_TRUE(testAccVec[i].IsNaN<TypeP>());
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
            } else
            {
                refAccValFloat = RefALU::asinpi(SuperT(*accurate[i].GetAcc<TypeP>()));
                NEATValue testAccVal = NEATALU::asinpi<TypeP>(accurate[i]);
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::asinpi<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if((*interval[i].GetMax<TypeP>() > TypeP(1.0)) ||
               (*interval[i].GetMin<TypeP>() < TypeP(-1.0)) ||
               (*interval[i].GetMax<TypeP>() < TypeP(-1.0)) ||
               (*interval[i].GetMin<TypeP>() > TypeP(1.0)))
            {
                /* if random src < -1.0 or src > 1.0 asinpi returns NaN */
                EXPECT_TRUE(testIntVec[i].IsNaN<TypeP>());
            } else {
                refIntValMin = RefALU::asinpi(SuperT(*interval[i].GetMin<TypeP>()));
                refIntValMax = RefALU::asinpi(SuperT(*interval[i].GetMax<TypeP>()));

                bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }


        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::asinpi<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
            } else
            {
                refAccValFloat = RefALU::asinpi(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::asinpi<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::asinpi(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::asinpi(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],5.f);
            EXPECT_TRUE(passed);
        }
    }
}



TYPED_TEST(NEATAluTypedMath, acospi)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                                  NUM_TESTS,TypeP(-0.999),TypeP(0.999));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                                  NUM_TESTS,TypeP(-0.999),TypeP(0.999));

    /* test for specific values */
    NEATValue testAccVal = NEATALU::acospi<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::acospi<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::acospi<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::acospi<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    //acospi ( 1 ) = +0
    testAccVal = NEATALU::acospi<TypeP>(NEATValue(TypeP(1.0),TypeP(1.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal, TypeP(0.0)));


    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -1.0 to 1.0 */
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

        testAccVal = NEATALU::acospi<TypeP>(accVal);
        SuperT refAccValFloat;

        if((*accVal.GetAcc<TypeP>() > TypeP(1.0)) ||
           (*accVal.GetAcc<TypeP>() < TypeP(-1.0) ))
        {
            /* if random src < -1.0 or src > 1.0 acospi returns NaN */
            EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
        } else if (*accVal.GetAcc<TypeP>() == TypeP(1.0)) // special accurate value
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal, TypeP(0.0)));
        } else {
            refAccValFloat = RefALU::acospi(SuperT(*accVal.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,5.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::acospi<TypeP>(intVal);

        if( *intVal.GetMin<TypeP>() < TypeP(-1.0) ||
            *intVal.GetMax<TypeP>() > TypeP(1.0))
            EXPECT_TRUE(testIntVal.IsNaN<TypeP>());
        else {
            refIntValMax = RefALU::acospi(SuperT(*intVal.GetMax<TypeP>()));
            refIntValMin = RefALU::acospi(SuperT(*intVal.GetMin<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,5.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::acospi<TypeP>(accurate);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        if (*accVal.GetAcc<TypeP>() == TypeP(1.0)) // special accurate value
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal, TypeP(0.0)));
        } else
        if((*accurate[i].GetAcc<TypeP>() > TypeP(1.0)) ||
           (*accurate[i].GetAcc<TypeP>() < TypeP(-1.0) ))
            {
                /* if random src < -1.0 or src > 1.0 acospi returns NaN */
                EXPECT_TRUE(testAccVec[i].IsNaN<TypeP>());
            } else {
                refAccValFloat = RefALU::acospi(SuperT(*accurate[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::acospi<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if((*interval[i].GetMax<TypeP>() > TypeP(1.0)) ||
               (*interval[i].GetMin<TypeP>() < TypeP(-1.0)) ||
               (*interval[i].GetMax<TypeP>() < TypeP(-1.0)) ||
               (*interval[i].GetMin<TypeP>() > TypeP(1.0)))
            {
                /* if random src < -1.0 or src > 1.0 acospi returns NaN */
                EXPECT_TRUE(testIntVec[i].IsNaN<TypeP>());
            } else {
                refIntValMin = RefALU::acospi(SuperT(*interval[i].GetMin<TypeP>()));
                refIntValMax = RefALU::acospi(SuperT(*interval[i].GetMax<TypeP>()));

                bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }


        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::acospi<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if((*accurateRanged[i].GetAcc<TypeP>() > TypeP(1.0)) ||
               (*accurateRanged[i].GetAcc<TypeP>() < TypeP(-1.0) ))
            {
                /* if random src < -1.0 or src > 1.0 acospi returns NaN */
                EXPECT_TRUE(testAccVec[i].IsNaN<TypeP>());
            } else if (*accurateRanged[i].GetAcc<TypeP>() == TypeP(1.0)) // special accurate value
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i], TypeP(0.0)));
            } else {
                refAccValFloat = RefALU::acospi(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::acospi<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::acospi(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::acospi(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],5.f);
            EXPECT_TRUE(passed);
        }
    }
}


TYPED_TEST(NEATAluTypedMath, atanpi)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    TypeP firstFloatRanged[NUM_TESTS*8];
    TypeP secondFloatRanged[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);

    /* just to reduce the interval from the full range of current data type */
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged[0], curWidth,
                                  NUM_TESTS,TypeP(-100.0),TypeP(100.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged[0], curWidth,
                                  NUM_TESTS,TypeP(-100.0),TypeP(100.0));


    /* test for specific values */
    //atanpi ( ±0 ) = ±0.
    NEATValue testAccVal = NEATALU::atanpi<TypeP>(NEATValue(TypeP(+0.0),TypeP(+0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    testAccVal = NEATALU::atanpi<TypeP>(NEATValue(TypeP(-0.0),TypeP(-0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));

    testAccVal = NEATALU::atanpi<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atanpi<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atanpi<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atanpi<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    //atanpi ( ±INF ) = ±0.5
    testAccVal = NEATALU::atanpi<TypeP>(NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>()));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.5)));

    testAccVal = NEATALU::atanpi<TypeP>(NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>()));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.5)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        NEATVector accurateRanged(curWidth);
        NEATVector intervalRanged(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -100 to 100 */
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

        testAccVal = NEATALU::atanpi<TypeP>(accVal);
        SuperT refAccValFloat;

        if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(+0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));
        } else if (Utils::eq(*accVal.GetAcc<TypeP>(),TypeP(-0.0)))
        {
            EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));
        } else
        {
            refAccValFloat = RefALU::atanpi(SuperT(*accVal.GetAcc<TypeP>()));
            if(refAccValFloat == SuperT(0.5))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.5)));
            } else if (refAccValFloat == SuperT(-0.5))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.5)));
            } else
            {
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,5.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::atanpi<TypeP>(intVal);

        refIntValMax = RefALU::atanpi(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::atanpi(SuperT(*intVal.GetMin<TypeP>()));

        if(refIntValMin == (SuperT)0.5)
            EXPECT_TRUE((*testIntVal.GetMin<TypeP>() == TypeP(0.5)));
        if(refIntValMax == (SuperT)0.5)
            EXPECT_TRUE((*testIntVal.GetMax<TypeP>() == TypeP(0.5)));
        if(refIntValMin == (SuperT)-0.5)
            EXPECT_TRUE((*testIntVal.GetMin<TypeP>() == TypeP(-0.5)));
        if(refIntValMax == (SuperT)-0.5)
            EXPECT_TRUE((*testIntVal.GetMax<TypeP>() == TypeP(-0.5)));
        if((fabs(refIntValMin) != (SuperT)0.5) &&
           (fabs(refIntValMax) != (SuperT)0.5))
        {
            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,5.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::atanpi<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
            } else if (Utils::eq(*accurate[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
            } else
            {
                refAccValFloat = RefALU::atanpi(SuperT(*accurate[i].GetAcc<TypeP>()));
                if(refAccValFloat == (SuperT)0.5)
                    EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(0.5)));
                else if (refAccValFloat == (SuperT)(-0.5))
                    EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.5)));
                else {
                    bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],5.f);
                    EXPECT_TRUE(passed);
                }
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::atanpi<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::atanpi(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::atanpi(SuperT(*interval[i].GetMax<TypeP>()));

            if(refIntValMin == (SuperT)0.5)
                EXPECT_TRUE((*testIntVec[i].GetMin<TypeP>() == TypeP(0.5)));
            if(refIntValMax == (SuperT)0.5)
                EXPECT_TRUE((*testIntVec[i].GetMax<TypeP>() == TypeP(0.5)));
            if(refIntValMin == (SuperT)-0.5)
                EXPECT_TRUE((*testIntVec[i].GetMin<TypeP>() == TypeP(-0.5)));
            if(refIntValMax == (SuperT)-0.5)
                EXPECT_TRUE((*testIntVec[i].GetMax<TypeP>() == TypeP(-0.5)));
            if((fabs(refIntValMin) != (SuperT)0.5) &&
               (fabs(refIntValMax) != (SuperT)0.5))
            {
                bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::atanpi<TypeP>(accurateRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(+0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(+0.0)));
            } else if (Utils::eq(*accurateRanged[i].GetAcc<TypeP>(),TypeP(-0.0)))
            {
                EXPECT_TRUE(TestAccValue<TypeP>(testAccVec[i],TypeP(-0.0)));
            } else
            {
                refAccValFloat = RefALU::atanpi(SuperT(*accurateRanged[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],5.f);
                EXPECT_TRUE(passed);
            }
        }


        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::atanpi<TypeP>(intervalRanged);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::atanpi(SuperT(*intervalRanged[i].GetMin<TypeP>()));
            refIntValMax = RefALU::atanpi(SuperT(*intervalRanged[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],5.f);
            EXPECT_TRUE(passed);
        }
    }
}


TYPED_TEST(NEATAluTypedMath, atan2)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }
    const int NUM_TESTS = 500;

    TypeP firstFloatX[NUM_TESTS*8];
    TypeP secondFloatX[NUM_TESTS*8];

    TypeP firstFloatRangedX[NUM_TESTS*8];
    TypeP secondFloatRangedX[NUM_TESTS*8];

    TypeP firstFloatY[NUM_TESTS*8];
    TypeP secondFloatY[NUM_TESTS*8];

    TypeP firstFloatRangedY[NUM_TESTS*8];
    TypeP secondFloatRangedY[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloatX[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloatX[0], curWidth, NUM_TESTS);

    /* just to reduce the interval from the full range of current data type */
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRangedX[0], curWidth,
                                  NUM_TESTS,TypeP(-100.0),TypeP(100.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRangedX[0], curWidth,
                                  NUM_TESTS,TypeP(-100.0),TypeP(100.0));

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloatY[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloatY[0], curWidth, NUM_TESTS);
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRangedY[0], curWidth,
                                  NUM_TESTS,TypeP(-100.0),TypeP(100.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRangedY[0], curWidth,
                                  NUM_TESTS,TypeP(-100.0),TypeP(100.0));

    /* test for specific values */
    NEATValue accValX = NEATValue(TypeP(0.0),TypeP(0.0));
    NEATValue accValY = NEATValue(firstFloatY[0],firstFloatY[0]);
    NEATValue testAccVal = NEATALU::atan2<TypeP>(accValY,accValX);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    accValX = NEATValue(firstFloatX[1],firstFloatX[1]);
    testAccVal = NEATALU::atan2<TypeP>(NEATValue(NEATValue::ANY),accValX);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::atan2<TypeP>(accValY,NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atan2<TypeP>(NEATValue(NEATValue::UNKNOWN),accValX);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::atan2<TypeP>(accValY,NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atan2<TypeP>(NEATValue(NEATValue::UNWRITTEN),accValX);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::atan2<TypeP>(accValY,NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());


    testAccVal = NEATALU::atan2<TypeP>(NEATValue::NaN<TypeP>(),accValX);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
    testAccVal = NEATALU::atan2<TypeP>(accValY,NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());


    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accValY, accValX, intValY, intValX;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurateX(curWidth);
        NEATVector intervalX(curWidth);

        NEATVector accurateRangedX(curWidth);
        NEATVector intervalRangedX(curWidth);

        NEATVector accurateY(curWidth);
        NEATVector intervalY(curWidth);

        NEATVector accurateRangedY(curWidth);
        NEATVector intervalRangedY(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateX[i] = NEATValue(firstFloatX[testIdx*wrap.GetSize()+i]);
            if(firstFloatX[testIdx*wrap.GetSize()+i] < secondFloatX[testIdx*wrap.GetSize()+i])
                intervalX[i] = NEATValue(firstFloatX[testIdx*wrap.GetSize()+i], secondFloatX[testIdx*wrap.GetSize()+i]);
            else
                intervalX[i] = NEATValue(secondFloatX[testIdx*wrap.GetSize()+i], firstFloatX[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -100 to 100 */
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRangedX[i] = NEATValue(firstFloatRangedX[testIdx*wrap.GetSize()+i]);
            if(firstFloatRangedX[testIdx*wrap.GetSize()+i] < secondFloatRangedX[testIdx*wrap.GetSize()+i])
                intervalRangedX[i] = NEATValue(firstFloatRangedX[testIdx*wrap.GetSize()+i], secondFloatRangedX[testIdx*wrap.GetSize()+i]);
            else
                intervalRangedX[i] = NEATValue(secondFloatRangedX[testIdx*wrap.GetSize()+i], firstFloatRangedX[testIdx*wrap.GetSize()+i]);
        }

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateY[i] = NEATValue(firstFloatY[testIdx*wrap.GetSize()+i]);
            if(firstFloatY[testIdx*wrap.GetSize()+i] < secondFloatY[testIdx*wrap.GetSize()+i])
                intervalY[i] = NEATValue(firstFloatY[testIdx*wrap.GetSize()+i], secondFloatY[testIdx*wrap.GetSize()+i]);
            else
                intervalY[i] = NEATValue(secondFloatY[testIdx*wrap.GetSize()+i], firstFloatY[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -100 to 100 */
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRangedY[i] = NEATValue(firstFloatRangedY[testIdx*wrap.GetSize()+i]);
            if(firstFloatRangedY[testIdx*wrap.GetSize()+i] < secondFloatRangedY[testIdx*wrap.GetSize()+i])
                intervalRangedY[i] = NEATValue(firstFloatRangedY[testIdx*wrap.GetSize()+i], secondFloatRangedY[testIdx*wrap.GetSize()+i]);
            else
                intervalRangedY[i] = NEATValue(secondFloatRangedY[testIdx*wrap.GetSize()+i], firstFloatRangedY[testIdx*wrap.GetSize()+i]);
        }


       /* test for single accurate NEAT value*/
        accValY = accurateY[0];
        accValX = accurateX[0];

        testAccVal = NEATALU::atan2<TypeP>(accValY,accValX);
        SuperT refAccValFloat;

        if(Utils::eq<TypeP>(*accValX.GetAcc<TypeP>(),TypeP(0.0)))
        {
            EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
        } else {
            refAccValFloat = RefALU::atan2(SuperT(*accValY.GetAcc<TypeP>()),
                                           SuperT(*accValX.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,6.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intValY = intervalY[0];
        intValX = intervalX[0];

        testIntVal = NEATALU::atan2<TypeP>(intValY,intValX);

        if(Utils::eq<TypeP>(*intValX.GetMin<TypeP>(),TypeP(0.0)) ||
           Utils::eq<TypeP>(*intValX.GetMax<TypeP>(),TypeP(0.0)))
        {
            EXPECT_TRUE(testIntVal.IsNaN<TypeP>());
        } else {
            refIntValMin = RefALU::atan2(SuperT(*intValY.GetMin<TypeP>()),
                                         SuperT(*intValX.GetMin<TypeP>()));
            refIntValMax = RefALU::atan2(SuperT(*intValY.GetMax<TypeP>()),
                                         SuperT(*intValX.GetMax<TypeP>()));
            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,6.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::atan2<TypeP>(accurateY,accurateX);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if(Utils::eq<TypeP>(*accurateX[i].GetAcc<TypeP>(),TypeP(0.0)))
            {
                EXPECT_TRUE(testAccVec[i].IsNaN<TypeP>());
            } else {
                refAccValFloat = RefALU::atan2(SuperT(*accurateY[i].GetAcc<TypeP>()),
                                               SuperT(*accurateX[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],6.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::atan2<TypeP>(intervalY,intervalX);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if(Utils::eq<TypeP>(*intervalX[i].GetMin<TypeP>(),TypeP(0.0)) ||
               Utils::eq<TypeP>(*intervalX[i].GetMax<TypeP>(),TypeP(0.0)))
            {
                EXPECT_TRUE(testIntVec[i].IsNaN<TypeP>());
            } else {
                refIntValMin = RefALU::atan2(SuperT(*intervalY[i].GetMin<TypeP>()),
                                             SuperT(*intervalX[i].GetMin<TypeP>()));
                refIntValMax = RefALU::atan2(SuperT(*intervalY[i].GetMax<TypeP>()),
                                             SuperT(*intervalX[i].GetMax<TypeP>()));
                bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],6.f);
                EXPECT_TRUE(passed);
             }
        }

        /* test for vector of NEAT accurate in range */
        testAccVec = NEATALU::atan2<TypeP>(accurateRangedY,accurateRangedX);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if(Utils::eq<TypeP>(*accurateRangedX[i].GetAcc<TypeP>(),TypeP(0.0)))
            {
                EXPECT_TRUE(testAccVec[i].IsNaN<TypeP>());
            } else {
                refAccValFloat = RefALU::atan2(SuperT(*accurateRangedY[i].GetAcc<TypeP>()),
                                               SuperT(*accurateRangedX[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],6.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::atan2<TypeP>(intervalRangedY,intervalRangedX);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if(Utils::eq<TypeP>(*intervalRangedX[i].GetMin<TypeP>(),TypeP(0.0)) ||
               Utils::eq<TypeP>(*intervalRangedX[i].GetMax<TypeP>(),TypeP(0.0)))
            {
                EXPECT_TRUE(testIntVec[i].IsNaN<TypeP>());
            } else {
                refIntValMin = RefALU::atan2(SuperT(*intervalRangedY[i].GetMin<TypeP>()),
                                             SuperT(*intervalRangedX[i].GetMin<TypeP>()));
                refIntValMax = RefALU::atan2(SuperT(*intervalRangedY[i].GetMax<TypeP>()),
                                             SuperT(*intervalRangedX[i].GetMax<TypeP>()));
                bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],6.f);
                EXPECT_TRUE(passed);
            }
        }
    }
}

TYPED_TEST(NEATAluTypedMath, atan2pi)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloatX[NUM_TESTS*8];
    TypeP secondFloatX[NUM_TESTS*8];

    TypeP firstFloatRangedX[NUM_TESTS*8];
    TypeP secondFloatRangedX[NUM_TESTS*8];

    TypeP firstFloatY[NUM_TESTS*8];
    TypeP secondFloatY[NUM_TESTS*8];

    TypeP firstFloatRangedY[NUM_TESTS*8];
    TypeP secondFloatRangedY[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloatX[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloatX[0], curWidth, NUM_TESTS);

    /* just to reduce the interval from the full range of current data type */
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRangedX[0], curWidth,
                                  NUM_TESTS,TypeP(-100.0),TypeP(100.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRangedX[0], curWidth,
                                  NUM_TESTS,TypeP(-100.0),TypeP(100.0));

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloatY[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloatY[0], curWidth, NUM_TESTS);
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRangedY[0], curWidth,
                              NUM_TESTS,TypeP(-100.0),TypeP(100.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRangedY[0], curWidth,
                              NUM_TESTS,TypeP(-100.0),TypeP(100.0));

    /* test for specific values */
    NEATValue accValX = NEATValue(TypeP(0.0),TypeP(0.0));
    NEATValue accValY = NEATValue(firstFloatY[0],firstFloatY[0]);
    NEATValue testAccVal;

    accValX = NEATValue(firstFloatX[1],firstFloatX[1]);
    testAccVal = NEATALU::atan2pi<TypeP>(NEATValue(NEATValue::ANY),accValX);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atan2pi<TypeP>(NEATValue(NEATValue::UNKNOWN),accValX);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atan2pi<TypeP>(NEATValue(NEATValue::UNWRITTEN),accValX);
    EXPECT_TRUE(testAccVal.IsUnknown());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::atan2pi<TypeP>(NEATValue::NaN<TypeP>(),accValX);
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    // atan2pi ( -0, -0 ) = -1
    accValX = NEATValue(TypeP(-0.0),TypeP(-0.0));
    accValY = NEATValue(TypeP(-0.0),TypeP(-0.0));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-1.0)));

    // atan2pi ( +0, -0 ) = +1
    accValX = NEATValue(TypeP(-0.0),TypeP(-0.0));
    accValY = NEATValue(TypeP(+0.0),TypeP(+0.0));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));


    // atan2pi ( +0, +0 ) = +0
    accValX = NEATValue(TypeP(+0.0),TypeP(+0.0));
    accValY = NEATValue(TypeP(+0.0),TypeP(+0.0));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));


    // atan2pi ( -0, +0 ) = -0
    accValX = NEATValue(TypeP(+0.0),TypeP(+0.0));
    accValY = NEATValue(TypeP(-0.0),TypeP(-0.0));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));


    // atan2pi ( +0, x ) returns +1 for x < 0
    accValX = NEATValue(TypeP(-0.5),TypeP(-0.5));
    accValY = NEATValue(TypeP(+0.0),TypeP(+0.0));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+1.0)));

    // atan2pi ( -0, x ) returns -1 for x < 0
    accValX = NEATValue(TypeP(-0.5),TypeP(-0.5));
    accValY = NEATValue(TypeP(-0.0),TypeP(-0.0));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-1.0)));


    // atan2pi ( +0, x ) returns +0 for x > 0
    accValX = NEATValue(TypeP(0.5),TypeP(0.5));
    accValY = NEATValue(TypeP(+0.0),TypeP(+0.0));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));


    // atan2pi ( -0, x ) returns -0 for x > 0
    accValX = NEATValue(TypeP(0.5),TypeP(0.5));
    accValY = NEATValue(TypeP(-0.0),TypeP(-0.0));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));


    // atan2pi ( y, ±0 ) returns -0.5 for y < 0
    accValX = NEATValue(TypeP(+0.0),TypeP(+0.0));
    accValY = NEATValue(TypeP(-0.9),TypeP(-0.9));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.5)));


    accValX = NEATValue(TypeP(-0.0),TypeP(-0.0));
    accValY = NEATValue(TypeP(-0.9),TypeP(-0.9));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.5)));


    // atan2pi ( y, ±0 ) returns 0.5 for y > 0
    accValX = NEATValue(TypeP(+0.0),TypeP(+0.0));
    accValY = NEATValue(TypeP(0.9),TypeP(0.9));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.5)));


    accValX = NEATValue(TypeP(-0.0),TypeP(-0.0));
    accValY = NEATValue(TypeP(0.9),TypeP(0.9));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.5)));

    // atan2pi ( +INF, x ) returns +0.5 for finite x
    accValX = NEATValue(TypeP(0.5),TypeP(0.5));
    accValY = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.5)));

    // atan2pi ( -INF, x ) returns -0.5 for finite x
    accValX = NEATValue(TypeP(0.5),TypeP(0.5));
    accValY = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.5)));


    // atan2pi (+INF, -INF ) returns +0.75
    accValX = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    accValY = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.75)));

    // atan2pi (-INF, -INF ) returns -0.75
    accValX = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    accValY = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.75)));


    // atan2pi (+INF, +INF ) returns +0.25
    accValX = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    accValY = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.25)));

    // atan2pi (-INF, +INF ) returns -0.25
    accValX = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    accValY = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.25)));


    // atan2pi ( +y, -INF ) returns +1 for finite y > 0
    accValX = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    accValY = NEATValue(TypeP(0.33),TypeP(0.33));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(1.0)));

    // atan2pi ( -y, -INF ) returns -1 for finite y > 0
    accValX = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    accValY = NEATValue(TypeP(-0.33),TypeP(-0.33));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-1.0)));

    // atan2pi ( +y, +INF ) returns +0 for finite y > 0
    accValX = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    accValY = NEATValue(TypeP(0.33),TypeP(0.33));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(+0.0)));

    // atan2pi ( -y, +INF ) returns -0 for finite y > 0
    accValX = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    accValY = NEATValue(TypeP(-0.33),TypeP(-0.33));
    testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(-0.0)));


    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accValY, accValX, intValY, intValX;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurateX(curWidth);
        NEATVector intervalX(curWidth);

        NEATVector accurateRangedX(curWidth);
        NEATVector intervalRangedX(curWidth);

        NEATVector accurateY(curWidth);
        NEATVector intervalY(curWidth);

        NEATVector accurateRangedY(curWidth);
        NEATVector intervalRangedY(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateX[i] = NEATValue(firstFloatX[testIdx*wrap.GetSize()+i]);
            if(firstFloatX[testIdx*wrap.GetSize()+i] < secondFloatX[testIdx*wrap.GetSize()+i])
                intervalX[i] = NEATValue(firstFloatX[testIdx*wrap.GetSize()+i], secondFloatX[testIdx*wrap.GetSize()+i]);
            else
                intervalX[i] = NEATValue(secondFloatX[testIdx*wrap.GetSize()+i], firstFloatX[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -100 to 100 */
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRangedX[i] = NEATValue(firstFloatRangedX[testIdx*wrap.GetSize()+i]);
            if(firstFloatRangedX[testIdx*wrap.GetSize()+i] < secondFloatRangedX[testIdx*wrap.GetSize()+i])
                intervalRangedX[i] = NEATValue(firstFloatRangedX[testIdx*wrap.GetSize()+i], secondFloatRangedX[testIdx*wrap.GetSize()+i]);
            else
                intervalRangedX[i] = NEATValue(secondFloatRangedX[testIdx*wrap.GetSize()+i], firstFloatRangedX[testIdx*wrap.GetSize()+i]);
        }

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateY[i] = NEATValue(firstFloatY[testIdx*wrap.GetSize()+i]);
            if(firstFloatY[testIdx*wrap.GetSize()+i] < secondFloatY[testIdx*wrap.GetSize()+i])
                intervalY[i] = NEATValue(firstFloatY[testIdx*wrap.GetSize()+i], secondFloatY[testIdx*wrap.GetSize()+i]);
            else
                intervalY[i] = NEATValue(secondFloatY[testIdx*wrap.GetSize()+i], firstFloatY[testIdx*wrap.GetSize()+i]);
        }

        /* load data for interval from -100 to 100 */
        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRangedY[i] = NEATValue(firstFloatRangedY[testIdx*wrap.GetSize()+i]);
            if(firstFloatRangedY[testIdx*wrap.GetSize()+i] < secondFloatRangedY[testIdx*wrap.GetSize()+i])
                intervalRangedY[i] = NEATValue(firstFloatRangedY[testIdx*wrap.GetSize()+i], secondFloatRangedY[testIdx*wrap.GetSize()+i]);
            else
                intervalRangedY[i] = NEATValue(secondFloatRangedY[testIdx*wrap.GetSize()+i], firstFloatRangedY[testIdx*wrap.GetSize()+i]);
        }


       /* test for single accurate NEAT value*/
        accValY = accurateY[0];
        accValX = accurateX[0];

        testAccVal = NEATALU::atan2pi<TypeP>(accValY,accValX);
        SuperT refAccValFloat;

        if(Utils::eq<TypeP>(*accValX.GetAcc<TypeP>(),TypeP(0.0)))
        {
            EXPECT_TRUE(testAccVal.IsNaN<TypeP>());
        } else {
            refAccValFloat = RefALU::atan2pi(SuperT(*accValY.GetAcc<TypeP>()),
                                             SuperT(*accValX.GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,6.f);
            EXPECT_TRUE(passed);
        }

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intValY = intervalY[0];
        intValX = intervalX[0];

        testIntVal = NEATALU::atan2pi<TypeP>(intValY,intValX);

        if(Utils::eq<TypeP>(*intValX.GetMin<TypeP>(),TypeP(0.0)) ||
           Utils::eq<TypeP>(*intValX.GetMax<TypeP>(),TypeP(0.0)))
        {
            EXPECT_TRUE(testIntVal.IsNaN<TypeP>());
        } else {
            refIntValMax = RefALU::atan2pi(SuperT(*intValY.GetMax<TypeP>()),
                                           SuperT(*intValX.GetMax<TypeP>()));
            refIntValMin = RefALU::atan2pi(SuperT(*intValY.GetMin<TypeP>()),
                                           SuperT(*intValX.GetMin<TypeP>()));
            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,6.f);
            EXPECT_TRUE(passed);
        }

        /* test for vector of NEAT accurate */

        NEATVector testAccVec = NEATALU::atan2pi<TypeP>(accurateY,accurateX);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if(Utils::eq<TypeP>(*accurateX[i].GetAcc<TypeP>(),TypeP(0.0)))
            {
                EXPECT_TRUE(testAccVec[i].IsNaN<TypeP>());
            } else {
                refAccValFloat = RefALU::atan2pi(SuperT(*accurateY[i].GetAcc<TypeP>()),
                                                 SuperT(*accurateX[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],6.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::atan2pi<TypeP>(intervalY,intervalX);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if(Utils::eq<TypeP>(*intervalX[i].GetMin<TypeP>(),TypeP(0.0)) ||
               Utils::eq<TypeP>(*intervalX[i].GetMax<TypeP>(),TypeP(0.0)))
            {
                EXPECT_TRUE(testIntVec[i].IsNaN<TypeP>());
            } else {
                refIntValMax = RefALU::atan2pi(SuperT(*intervalY[i].GetMax<TypeP>()),
                                               SuperT(*intervalX[i].GetMax<TypeP>()));
                refIntValMin = RefALU::atan2pi(SuperT(*intervalY[i].GetMin<TypeP>()),
                                               SuperT(*intervalX[i].GetMin<TypeP>()));

                bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],6.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT accurate in range */

        testAccVec = NEATALU::atan2pi<TypeP>(accurateRangedY,accurateRangedX);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if(Utils::eq<TypeP>(*accurateRangedX[i].GetAcc<TypeP>(),TypeP(0.0)))
            {
                EXPECT_TRUE(testAccVec[i].IsNaN<TypeP>());
            } else {
                refAccValFloat = RefALU::atan2pi(SuperT(*accurateRangedY[i].GetAcc<TypeP>()),
                                                 SuperT(*accurateRangedX[i].GetAcc<TypeP>()));
                bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],6.f);
                EXPECT_TRUE(passed);
            }
        }

        /* test for vector of NEAT intervals in range */
        testIntVec = NEATALU::atan2pi<TypeP>(intervalRangedY,intervalRangedX);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            if(Utils::eq<TypeP>(*intervalRangedX[i].GetMin<TypeP>(),TypeP(0.0)) ||
               Utils::eq<TypeP>(*intervalRangedX[i].GetMax<TypeP>(),TypeP(0.0)))
            {
                EXPECT_TRUE(testIntVec[i].IsNaN<TypeP>());
            } else {
                refIntValMax = RefALU::atan2pi(SuperT(*intervalRangedY[i].GetMax<TypeP>()),
                                               SuperT(*intervalRangedX[i].GetMax<TypeP>()));
                refIntValMin = RefALU::atan2pi(SuperT(*intervalRangedY[i].GetMin<TypeP>()),
                                               SuperT(*intervalRangedX[i].GetMin<TypeP>()));

                bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],6.f);
                EXPECT_TRUE(passed);
            }
        }
    }
}


TYPED_TEST(NEATAluTypedMath, sqrt)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    float ulpErr = float(NEATALU::SQRT_ERROR);

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                  NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth,
                              NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());


    /* test for specific values */
    NEATValue testAccVal = NEATALU::sqrt<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sqrt<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sqrt<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::sqrt<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::sqrt<TypeP>(NEATValue(TypeP(0.0),TypeP(0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.0)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

       /* test for single accurate NEAT value*/
        accVal = accurate[0];

        testAccVal = NEATALU::sqrt<TypeP>(accVal);
        SuperT refAccValFloat;

        refAccValFloat = RefALU::sqrt(SuperT(*accVal.GetAcc<TypeP>()));
        bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::sqrt<TypeP>(intVal);

        refIntValMax = RefALU::sqrt(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::sqrt(SuperT(*intVal.GetMin<TypeP>()));

        passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::sqrt<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::sqrt(SuperT(*accurate[i].GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],ulpErr);
            EXPECT_TRUE(passed);

        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::sqrt<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::sqrt(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::sqrt(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],ulpErr);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, half_sqrt)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    float ulpErr = float(NEATALU::HALF_SQRT_ERROR);

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                  NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth,
                              NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());


    /* test for specific values */
    NEATValue testAccVal = NEATALU::half_sqrt<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_sqrt<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_sqrt<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_sqrt<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::half_sqrt<TypeP>(NEATValue(TypeP(0.0),TypeP(0.0)));
    EXPECT_TRUE(TestAccValue<TypeP>(testAccVal,TypeP(0.0)));

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

       /* test for single accurate NEAT value*/
        accVal = accurate[0];

        testAccVal = NEATALU::half_sqrt<TypeP>(accVal);
        SuperT refAccValFloat;

        refAccValFloat = RefALU::sqrt(SuperT(*accVal.GetAcc<TypeP>()));
        bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::half_sqrt<TypeP>(intVal);

        refIntValMax = RefALU::sqrt(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::sqrt(SuperT(*intVal.GetMin<TypeP>()));

        passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::half_sqrt<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::sqrt(SuperT(*accurate[i].GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],ulpErr);
            EXPECT_TRUE(passed);

        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::half_sqrt<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::sqrt(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::sqrt(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],ulpErr);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, rsqrt)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    float ulpErr = float(NEATALU::RSQRT_ERROR);

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                  NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth,
                              NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());


    /* test for specific values */
    NEATValue testAccVal = NEATALU::rsqrt<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::rsqrt<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::rsqrt<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::rsqrt<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::rsqrt<TypeP>(NEATValue(TypeP(0.0),TypeP(0.0)));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

       /* test for single accurate NEAT value*/
        accVal = accurate[0];

        testAccVal = NEATALU::rsqrt<TypeP>(accVal);
        SuperT refAccValFloat;

        refAccValFloat = RefALU::rsqrt(SuperT(*accVal.GetAcc<TypeP>()));
        bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::rsqrt<TypeP>(intVal);

        refIntValMax = RefALU::rsqrt(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::rsqrt(SuperT(*intVal.GetMin<TypeP>()));

        passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::rsqrt<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::rsqrt(SuperT(*accurate[i].GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],ulpErr);
            EXPECT_TRUE(passed);

        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::rsqrt<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::rsqrt(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::rsqrt(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],ulpErr);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, native_rsqrt)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    float ulpErr = float(NEATALU::NATIVE_RSQRT_ERROR);

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                  NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth,
                              NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());


    /* test for specific values */
    NEATValue testAccVal = NEATALU::native_rsqrt<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_rsqrt<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_rsqrt<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::native_rsqrt<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::native_rsqrt<TypeP>(NEATValue(TypeP(0.0),TypeP(0.0)));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

       /* test for single accurate NEAT value*/
        accVal = accurate[0];

        testAccVal = NEATALU::native_rsqrt<TypeP>(accVal);
        SuperT refAccValFloat;

        refAccValFloat = RefALU::rsqrt(SuperT(*accVal.GetAcc<TypeP>()));
        bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::native_rsqrt<TypeP>(intVal);

        refIntValMax = RefALU::rsqrt(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::rsqrt(SuperT(*intVal.GetMin<TypeP>()));

        passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::native_rsqrt<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::rsqrt(SuperT(*accurate[i].GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],ulpErr);
            EXPECT_TRUE(passed);

        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::native_rsqrt<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::rsqrt(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::rsqrt(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],ulpErr);
            EXPECT_TRUE(passed);
        }
    }
}

TYPED_TEST(NEATAluTypedMath, half_rsqrt)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    float ulpErr = float(NEATALU::HALF_RSQRT_ERROR);

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*8];
    TypeP secondFloat[NUM_TESTS*8];

    VectorWidth curWidth = V8;

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth,
                                  NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth,
                              NUM_TESTS,TypeP(0.01),std::numeric_limits<TypeP>::max());


    /* test for specific values */
    NEATValue testAccVal = NEATALU::half_rsqrt<TypeP>(NEATValue(NEATValue::ANY));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_rsqrt<TypeP>(NEATValue(NEATValue::UNKNOWN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_rsqrt<TypeP>(NEATValue(NEATValue::UNWRITTEN));
    EXPECT_TRUE(testAccVal.IsUnknown());

    testAccVal = NEATALU::half_rsqrt<TypeP>(NEATValue::NaN<TypeP>());
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    testAccVal = NEATALU::half_rsqrt<TypeP>(NEATValue(TypeP(0.0),TypeP(0.0)));
    EXPECT_TRUE(testAccVal.IsNaN<TypeP>());

    VectorWidthWrapper wrap(curWidth);
    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        NEATValue accVal, intVal;
        NEATValue testAccVal, testIntVal;
        NEATValue refAccVal, refIntVal;

        /* load data for interval from min TypeP to max TypeP */
        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }

       /* test for single accurate NEAT value*/
        accVal = accurate[0];

        testAccVal = NEATALU::half_rsqrt<TypeP>(accVal);
        SuperT refAccValFloat;

        refAccValFloat = RefALU::rsqrt(SuperT(*accVal.GetAcc<TypeP>()));
        bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for single interval NEAT value */
        SuperT refIntValMin, refIntValMax;

        intVal = interval[0];
        testIntVal = NEATALU::half_rsqrt<TypeP>(intVal);

        refIntValMax = RefALU::rsqrt(SuperT(*intVal.GetMax<TypeP>()));
        refIntValMin = RefALU::rsqrt(SuperT(*intVal.GetMin<TypeP>()));

        passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVal,ulpErr);
        EXPECT_TRUE(passed);

        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::half_rsqrt<TypeP>(accurate);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::rsqrt(SuperT(*accurate[i].GetAcc<TypeP>()));
            bool passed = TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],ulpErr);
            EXPECT_TRUE(passed);

        }

        /* test for vector of NEAT intervals */
        NEATVector testIntVec = NEATALU::half_rsqrt<TypeP>(interval);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refIntValMin = RefALU::rsqrt(SuperT(*interval[i].GetMin<TypeP>()));
            refIntValMax = RefALU::rsqrt(SuperT(*interval[i].GetMax<TypeP>()));

            bool passed = TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],ulpErr);
            EXPECT_TRUE(passed);
        }
    }
}

template<typename T>
T GetMaxValue( T vec[], size_t vecSize )
{
    T max = vec[0];
    for( size_t i = 1; i < vecSize; i++ )
        if(vec[i] > max)
            max = vec[i];

    return max;
}
template<typename T>
T GetMinValue( T vec[], size_t vecSize )
{
    T min = vec[0];
    for( size_t i = 1; i < vecSize; i++ )
        if(vec[i] < min)
            min = vec[i];

    return min;
}



template<typename T>
T GetMaxAbsAcc(NEATValue val0, NEATValue val1)
{
    T aaa;
    if( fabs(*val0.GetAcc<T>()) > fabs(*val1.GetAcc<T>()))
            aaa = *val0.GetAcc<T>();
        else
            aaa = *val1.GetAcc<T>();
    return aaa;
}

template<typename T>
T GetMaxAbsAccVec(uint32_t size, NEATVector arr0, NEATVector arr1)
{
    T aaa;
    if( fabs(*arr0[0].GetAcc<T>()) > fabs(*arr1[0].GetAcc<T>()))
            aaa = *arr0[0].GetAcc<T>();
        else
            aaa = *arr1[0].GetAcc<T>();
        for(uint32_t i = 1; i<size; i++) {
            if( fabs(*arr0[i].GetAcc<T>()) > fabs(aaa))
                aaa = *arr0[i].GetAcc<T>();
            if( fabs(*arr1[i].GetAcc<T>()) > fabs(aaa))
                aaa = *arr1[i].GetAcc<T>();
        }
    return aaa;
}


template<typename T>
T GetMaxAbsInt(NEATValue val0, NEATValue val1)
{
    T maxAbs = 0;

    T a0 = fabs(*val0.GetMin<T>());
    T a1 = fabs(*val1.GetMin<T>());
    T a2 = fabs(*val0.GetMax<T>());
    T a3 = fabs(*val1.GetMax<T>());

    a0 = (a0 > a1 ? a0 : a1);
    a2 = (a2 > a3 ? a2 : a3);
    maxAbs = (a0 > a2 ? a0 : a2);

    return maxAbs;
}

template<typename T>
T GetMaxAbsIntVec(uint32_t size, NEATVector arr0, NEATVector arr1)
{
    T maxAbs = 0;

    for(uint32_t i = 0; i<size; i++) {
        T a0 = fabs(*arr0[i].GetMin<T>());
        T a1 = fabs(*arr1[i].GetMin<T>());
        T a2 = fabs(*arr0[i].GetMax<T>());
        T a3 = fabs(*arr1[i].GetMax<T>());

        a0 = (a0 > a1 ? a0 : a1);
        a2 = (a2 > a3 ? a2 : a3);
        a0 = (a0 > a2 ? a0 : a2);

        if( a0 > maxAbs)
            maxAbs = a0;
    }

    return maxAbs;
}

TYPED_TEST(NEATAluTypedMath, dot)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat0[NUM_TESTS*4];
    TypeP secondFloat0[NUM_TESTS*4];
    TypeP firstFloat1[NUM_TESTS*4];
    TypeP secondFloat1[NUM_TESTS*4];

    TypeP firstFloatRanged0[NUM_TESTS*4];
    TypeP secondFloatRanged0[NUM_TESTS*4];
    TypeP firstFloatRanged1[NUM_TESTS*4];
    TypeP secondFloatRanged1[NUM_TESTS*4];

    uint32_t indexArr[4];

    VectorWidth curWidth = V4;
    // geometric functions' arguments are float, float2, float3, or float4

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat0[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat0[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat1[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat1[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged0[0], curWidth,
                                  NUM_TESTS,TypeP(-1000.0),TypeP(+1000.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged0[0], curWidth,
                              NUM_TESTS,TypeP(-1000.0),TypeP(+1000.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &firstFloatRanged1[0], curWidth,
                                  NUM_TESTS,TypeP(-1000.0),TypeP(+1000.0));
    GenerateRangedVectorsAutoSeed(this->dataTypeVal, &secondFloatRanged1[0], curWidth,
                              NUM_TESTS,TypeP(-1000.0),TypeP(+1000.0));

    GenerateRangedVectorsAutoSeed<uint32_t>(U32, &indexArr[0], curWidth, 1, 0, 4);

    /* test for specific values */
    VectorWidthWrapper wrap(curWidth);

    float ulpErr = 2.f*float(wrap.GetSize()) - 1.f;

    NEATVector a0(curWidth);
    NEATVector a1(curWidth);

    for(uint32_t i = 0; i<wrap.GetSize(); i++)
    {
        a0[i] = NEATValue(firstFloat0[i]);
        a1[i] = NEATValue(firstFloat1[i]);
    }

    NEATVector b0 = a0;
    NEATVector b1 = a1;

    b0[indexArr[0]] = NEATValue(NEATValue::NaN<TypeP>());
    NEATValue testAcc = NEATALU::dot<TypeP>(b0,b1);
    EXPECT_TRUE(testAcc.IsNaN<TypeP>());

    b0[indexArr[0]] = a0[indexArr[0]]; // restore b0
    b1[indexArr[1]] = NEATValue(NEATValue::UNKNOWN);
    testAcc = NEATALU::dot<TypeP>(b0,b1);
    EXPECT_TRUE(testAcc.IsUnknown());

    b1[indexArr[1]] = a1[indexArr[1]]; // restore b1
    b0[indexArr[2]] = NEATValue(NEATValue::UNWRITTEN);
    testAcc = NEATALU::dot<TypeP>(b0,b1);
    EXPECT_TRUE(testAcc.IsUnknown());

    b0[indexArr[1]] = a0[indexArr[1]]; // restore b0
    b1[indexArr[2]] = NEATValue(NEATValue::ANY);
    testAcc = NEATALU::dot<TypeP>(b0,b1);
    EXPECT_TRUE(testAcc.IsUnknown());


    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        SuperT refAccValFloat, refIntValMin, refIntValMax;
        SuperT refForUlps;

        NEATVector accurate0(curWidth);
        NEATVector interval0(curWidth);
        NEATVector accurate1(curWidth);
        NEATVector interval1(curWidth);

        NEATVector accurateRanged0(curWidth);
        NEATVector intervalRanged0(curWidth);
        NEATVector accurateRanged1(curWidth);
        NEATVector intervalRanged1(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate0[i] = NEATValue(firstFloat0[testIdx*wrap.GetSize()+i]);
            if(firstFloat0[testIdx*wrap.GetSize()+i] < secondFloat0[testIdx*wrap.GetSize()+i])
                interval0[i] = NEATValue(firstFloat0[testIdx*wrap.GetSize()+i], secondFloat0[testIdx*wrap.GetSize()+i]);
            else
                interval0[i] = NEATValue(secondFloat0[testIdx*wrap.GetSize()+i], firstFloat0[testIdx*wrap.GetSize()+i]);
        }

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate1[i] = NEATValue(firstFloat1[testIdx*wrap.GetSize()+i]);
            if(firstFloat1[testIdx*wrap.GetSize()+i] < secondFloat1[testIdx*wrap.GetSize()+i])
                interval1[i] = NEATValue(firstFloat1[testIdx*wrap.GetSize()+i], secondFloat1[testIdx*wrap.GetSize()+i]);
            else
                interval1[i] = NEATValue(secondFloat1[testIdx*wrap.GetSize()+i], firstFloat1[testIdx*wrap.GetSize()+i]);
        }

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRanged0[i] = NEATValue(firstFloatRanged0[testIdx*wrap.GetSize()+i]);
            if(firstFloatRanged0[testIdx*wrap.GetSize()+i] < secondFloatRanged0[testIdx*wrap.GetSize()+i])
                intervalRanged0[i] = NEATValue(firstFloatRanged0[testIdx*wrap.GetSize()+i], secondFloatRanged0[testIdx*wrap.GetSize()+i]);
            else
                intervalRanged0[i] = NEATValue(secondFloatRanged0[testIdx*wrap.GetSize()+i], firstFloatRanged0[testIdx*wrap.GetSize()+i]);
        }

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurateRanged1[i] = NEATValue(firstFloatRanged1[testIdx*wrap.GetSize()+i]);
            if(firstFloatRanged1[testIdx*wrap.GetSize()+i] < secondFloatRanged1[testIdx*wrap.GetSize()+i])
                intervalRanged1[i] = NEATValue(firstFloatRanged1[testIdx*wrap.GetSize()+i], secondFloatRanged1[testIdx*wrap.GetSize()+i]);
            else
                intervalRanged1[i] = NEATValue(secondFloatRanged1[testIdx*wrap.GetSize()+i], firstFloatRanged1[testIdx*wrap.GetSize()+i]);
        }

        /* test for NEAT value accurate */
        NEATValue testAcc = NEATALU::dot<TypeP>(accurate0[0],accurate1[0]);
        refAccValFloat = RefALU::mul(SuperT(*accurate0[0].GetAcc<TypeP>()), SuperT(*accurate1[0].GetAcc<TypeP>()));
        EXPECT_TRUE(TestAccExpanded<SuperT>(refAccValFloat,testAcc,1.0f)); // ulpErr = 1.0f here, becasue it is vector of 1 element

        /* test for vector of NEAT accurate */
        testAcc = NEATALU::dot<TypeP>(accurate0,accurate1);

        refForUlps = (SuperT)GetMaxAbsAccVec<TypeP>(wrap.GetSize(), accurate0, accurate1);
        refForUlps = refForUlps*refForUlps;

        refAccValFloat = 0;
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            SuperT r = RefALU::mul(SuperT(*accurate0[i].GetAcc<TypeP>()),
                                   SuperT(*accurate1[i].GetAcc<TypeP>()));
            refAccValFloat = RefALU::add(refAccValFloat, r);
        }
        EXPECT_TRUE(TestAccExpandedDotMix<SuperT>(refAccValFloat,testAcc,refForUlps,ulpErr));

        /* test for NEAT value interval */
        NEATValue testInt = NEATALU::dot<TypeP>(interval0[0], interval1[0]);
        {
            SuperT r[4];
            r[0] = RefALU::mul(SuperT(*interval0[0].GetMin<TypeP>()), SuperT(*interval1[0].GetMin<TypeP>()));
            r[1] = RefALU::mul(SuperT(*interval0[0].GetMax<TypeP>()), SuperT(*interval1[0].GetMax<TypeP>()));
            r[2] = RefALU::mul(SuperT(*interval0[0].GetMin<TypeP>()), SuperT(*interval1[0].GetMax<TypeP>()));
            r[3] = RefALU::mul(SuperT(*interval0[0].GetMax<TypeP>()), SuperT(*interval1[0].GetMin<TypeP>()));

            refIntValMax = GetMaxValue<SuperT>(r,4);
            refIntValMin = GetMinValue<SuperT>(r,4);
        }
        EXPECT_TRUE(TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testInt,1.0f)); // ulpErr = 1.0f here, becasue it is vector of 1 element

        /* test for vector of NEAT intervals */
        refIntValMin = 0;
        refIntValMax = 0;
        testInt = NEATALU::dot<TypeP>(interval0, interval1);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            SuperT r[4];
            r[0] = RefALU::mul(SuperT(*interval0[i].GetMin<TypeP>()), SuperT(*interval1[i].GetMin<TypeP>()));
            r[1] = RefALU::mul(SuperT(*interval0[i].GetMax<TypeP>()), SuperT(*interval1[i].GetMax<TypeP>()));
            r[2] = RefALU::mul(SuperT(*interval0[i].GetMin<TypeP>()), SuperT(*interval1[i].GetMax<TypeP>()));
            r[3] = RefALU::mul(SuperT(*interval0[i].GetMax<TypeP>()), SuperT(*interval1[i].GetMin<TypeP>()));

            refIntValMax = RefALU::add(refIntValMax,GetMaxValue<SuperT>(r,4));
            refIntValMin = RefALU::add(refIntValMin,GetMinValue<SuperT>(r,4));
        }
        refForUlps = (SuperT)GetMaxAbsIntVec<TypeP>(wrap.GetSize(), interval0, interval1);
        refForUlps = refForUlps*refForUlps;
        EXPECT_TRUE(TestIntExpandedDotMix<SuperT>(refIntValMin,refIntValMax,testInt,refForUlps,ulpErr));

        /* test for NEAT value accurate ranged*/
        testAcc = NEATALU::dot<TypeP>(accurateRanged0[0],accurateRanged1[0]);
        refAccValFloat = RefALU::mul(SuperT(*accurateRanged0[0].GetAcc<TypeP>()),
                                     SuperT(*accurateRanged1[0].GetAcc<TypeP>()));
        EXPECT_TRUE(TestAccExpanded<SuperT>(refAccValFloat,testAcc,1.0f));

        /* test for vector of NEAT accurate ranged*/
        testAcc = NEATALU::dot<TypeP>(accurateRanged0,accurateRanged1);

        refForUlps = (SuperT)GetMaxAbsAccVec<TypeP>(wrap.GetSize(), accurateRanged0, accurateRanged1);
        refForUlps = refForUlps*refForUlps;

        refAccValFloat = 0;
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            SuperT r = RefALU::mul(SuperT(*accurateRanged0[i].GetAcc<TypeP>()),
                                   SuperT(*accurateRanged1[i].GetAcc<TypeP>()));
            refAccValFloat = RefALU::add(refAccValFloat, r);
        }
        EXPECT_TRUE(TestAccExpandedDotMix<SuperT>(refAccValFloat,testAcc,refForUlps,ulpErr));


        /* test for NEAT value intervals ranged*/
        testInt = NEATALU::dot<TypeP>(intervalRanged0[0], intervalRanged1[0]);
        {
            SuperT r[4];
            r[0] = RefALU::mul(SuperT(*intervalRanged0[0].GetMin<TypeP>()),
                               SuperT(*intervalRanged1[0].GetMin<TypeP>()));
            r[1] = RefALU::mul(SuperT(*intervalRanged0[0].GetMax<TypeP>()),
                               SuperT(*intervalRanged1[0].GetMax<TypeP>()));
            r[2] = RefALU::mul(SuperT(*intervalRanged0[0].GetMin<TypeP>()),
                               SuperT(*intervalRanged1[0].GetMax<TypeP>()));
            r[3] = RefALU::mul(SuperT(*intervalRanged0[0].GetMax<TypeP>()),
                               SuperT(*intervalRanged1[0].GetMin<TypeP>()));

            refIntValMax = GetMaxValue<SuperT>(r,4);
            refIntValMin = GetMinValue<SuperT>(r,4);
        }
        // mul is the only operation here
        EXPECT_TRUE(TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testInt,0.5f));

        /* test for vector of NEAT intervals ranged*/
        refIntValMin = 0;
        refIntValMax = 0;
        testInt = NEATALU::dot<TypeP>(intervalRanged0, intervalRanged1);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            SuperT r[4];
            r[0] = RefALU::mul(SuperT(*intervalRanged0[i].GetMin<TypeP>()),
                               SuperT(*intervalRanged1[i].GetMin<TypeP>()));
            r[1] = RefALU::mul(SuperT(*intervalRanged0[i].GetMax<TypeP>()),
                               SuperT(*intervalRanged1[i].GetMax<TypeP>()));
            r[2] = RefALU::mul(SuperT(*intervalRanged0[i].GetMin<TypeP>()),
                               SuperT(*intervalRanged1[i].GetMax<TypeP>()));
            r[3] = RefALU::mul(SuperT(*intervalRanged0[i].GetMax<TypeP>()),
                               SuperT(*intervalRanged1[i].GetMin<TypeP>()));

            refIntValMax = RefALU::add(refIntValMax,GetMaxValue<SuperT>(r,4));
            refIntValMin = RefALU::add(refIntValMin,GetMinValue<SuperT>(r,4));
        }
        refForUlps = (SuperT)GetMaxAbsIntVec<TypeP>(wrap.GetSize(), intervalRanged0, intervalRanged1);
        refForUlps = refForUlps*refForUlps;
        EXPECT_TRUE(TestIntExpandedDotMix<SuperT>(refIntValMin,refIntValMax,testInt,refForUlps,ulpErr));
    }
}

TYPED_TEST(NEATAluTypedMath, mix)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    float ulpErr = 1.5f; // the rounding error of sub, mul and add

    const int NUM_TESTS = 500;

    TypeP firstFloat0[NUM_TESTS*4];
    TypeP secondFloat0[NUM_TESTS*4];
    TypeP firstFloat1[NUM_TESTS*4];
    TypeP secondFloat1[NUM_TESTS*4];

    TypeP aArrFirst[NUM_TESTS*4];
    TypeP aArrSecond[NUM_TESTS*4];
    uint32_t indexArr[4];

    VectorWidth curWidth = V4;
    // geometric functions' arguments are float, float2, float3, or float4

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat0[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat0[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat1[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat1[0], curWidth, NUM_TESTS);

    GenerateRangedVectorsAutoSeed<float>(F32, &aArrFirst[0], curWidth,
                              NUM_TESTS,0.f,1.f);
    GenerateRangedVectorsAutoSeed<float>(F32, &aArrSecond[0], curWidth,
                              NUM_TESTS,0.f,1.f);

    GenerateRangedVectorsAutoSeed<uint32_t>(U32, &indexArr[0], curWidth, 1, 0, 4);

    /* test for specific values */
    VectorWidthWrapper wrap(curWidth);

    NEATVector a0(curWidth);
    NEATVector a1(curWidth);
    NEATVector aArr(curWidth);

    for(uint32_t i = 0; i<wrap.GetSize(); i++)
    {
        a0[i] = NEATValue(firstFloat0[i]);
        a1[i] = NEATValue(firstFloat1[i]);
        if(aArrFirst[i] < aArrSecond[i])
            aArr[i] = NEATValue(aArrFirst[i],aArrSecond[i]);
        else
            aArr[i] = NEATValue(aArrSecond[i],aArrFirst[i]);
    }

    NEATVector b0 = a0;
    NEATVector b1 = a1;

    // Test for NaN
    b0[indexArr[0]] = NEATValue(NEATValue::NaN<TypeP>());
    NEATVector testAcc = NEATALU::mix<TypeP>(b0,b1,aArr);
    EXPECT_TRUE(testAcc[indexArr[0]].IsNaN<TypeP>());
    b0[indexArr[0]] = a0[indexArr[0]]; // restore b0
    b1[indexArr[0]] = NEATValue(NEATValue::NaN<TypeP>());
    testAcc = NEATALU::mix<TypeP>(b0,b1,aArr);
    EXPECT_TRUE(testAcc[indexArr[0]].IsNaN<TypeP>());
    b1[indexArr[0]] = a1[indexArr[0]]; // restore b1

    // Test for UNKNOWN,UNWRITTEN,ANY
    b1[indexArr[1]] = NEATValue(NEATValue::UNKNOWN);
    testAcc = NEATALU::mix<TypeP>(b0,b1,aArr);
    EXPECT_TRUE(testAcc[indexArr[1]].IsUnknown());
    b1[indexArr[1]] = a1[indexArr[1]]; // restore b1

    b0[indexArr[2]] = NEATValue(NEATValue::UNWRITTEN);
    testAcc = NEATALU::mix<TypeP>(b0,b1,aArr);
    EXPECT_TRUE(testAcc[indexArr[2]].IsUnknown());
    b0[indexArr[1]] = a0[indexArr[1]]; // restore b0

    b1[indexArr[2]] = NEATValue(NEATValue::ANY);
    testAcc = NEATALU::mix<TypeP>(b0,b1,aArr);
    EXPECT_TRUE(testAcc[indexArr[2]].IsUnknown());
    b1[indexArr[2]] = a1[indexArr[2]]; // restore b1

    // Test for range (-1;0) for an array
    aArr[indexArr[0]] = NEATValue(TypeP(-2.0),TypeP(0.2));
    testAcc = NEATALU::mix<TypeP>(b0,b1,aArr);
    EXPECT_TRUE(testAcc[indexArr[0]].IsUnknown());

    aArr[indexArr[0]] = NEATValue(TypeP(0.2),TypeP(3.2));
    testAcc = NEATALU::mix<TypeP>(b0,b1,aArr);
    EXPECT_TRUE(testAcc[indexArr[0]].IsUnknown());

    aArr[indexArr[0]] = NEATValue(TypeP(-10.2),TypeP(-3.2));
    testAcc = NEATALU::mix<TypeP>(b0,b1,aArr);
    EXPECT_TRUE(testAcc[indexArr[0]].IsAny());

    aArr[indexArr[0]] = NEATValue(TypeP(3.2),TypeP(10.2));
    testAcc = NEATALU::mix<TypeP>(b0,b1,aArr);
    EXPECT_TRUE(testAcc[indexArr[0]].IsAny());


    // Test for range (-1;0) for a single value
    NEATValue aVal = NEATValue(TypeP(-2.0),TypeP(0.2));
    testAcc = NEATALU::mix<TypeP>(b0,b1,aVal);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        EXPECT_TRUE(testAcc[i].IsUnknown());
    }

    aVal = NEATValue(TypeP(0.2),TypeP(3.2));
    testAcc = NEATALU::mix<TypeP>(b0,b1,aVal);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        EXPECT_TRUE(testAcc[i].IsUnknown());
    }

    aVal = NEATValue(TypeP(-10.2),TypeP(-3.2));
    testAcc = NEATALU::mix<TypeP>(b0,b1,aVal);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        EXPECT_TRUE(testAcc[i].IsAny());
    }

    aVal = NEATValue(TypeP(3.2),TypeP(10.2));
    testAcc = NEATALU::mix<TypeP>(b0,b1,aVal);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        EXPECT_TRUE(testAcc[i].IsAny());
    }


    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        SuperT refAccValFloat;
        SuperT refIntValMin, refIntValMax;

        NEATVector accurate0(curWidth);
        NEATVector interval0(curWidth);
        NEATVector accurate1(curWidth);
        NEATVector interval1(curWidth);
        NEATVector accA(curWidth);
        NEATVector intA(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate0[i] = NEATValue(firstFloat0[testIdx*wrap.GetSize()+i]);
            if(firstFloat0[testIdx*wrap.GetSize()+i] < secondFloat0[testIdx*wrap.GetSize()+i])
                interval0[i] = NEATValue(firstFloat0[testIdx*wrap.GetSize()+i], secondFloat0[testIdx*wrap.GetSize()+i]);
            else
                interval0[i] = NEATValue(secondFloat0[testIdx*wrap.GetSize()+i], firstFloat0[testIdx*wrap.GetSize()+i]);
        }

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate1[i] = NEATValue(firstFloat1[testIdx*wrap.GetSize()+i]);
            if(firstFloat1[testIdx*wrap.GetSize()+i] < secondFloat1[testIdx*wrap.GetSize()+i])
                interval1[i] = NEATValue(firstFloat1[testIdx*wrap.GetSize()+i], secondFloat1[testIdx*wrap.GetSize()+i]);
            else
                interval1[i] = NEATValue(secondFloat1[testIdx*wrap.GetSize()+i], firstFloat1[testIdx*wrap.GetSize()+i]);
        }

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accA[i] = NEATValue(aArrFirst[testIdx*wrap.GetSize()+i]);
            if(aArrFirst[testIdx*wrap.GetSize()+i] < aArrSecond[testIdx*wrap.GetSize()+i])
                intA[i] = NEATValue(aArrFirst[testIdx*wrap.GetSize()+i], aArrSecond[testIdx*wrap.GetSize()+i]);
            else
                intA[i] = NEATValue(aArrSecond[testIdx*wrap.GetSize()+i], aArrFirst[testIdx*wrap.GetSize()+i]);
        }

        /* test for NEAT value accurate */

        SuperT refForUlps = (SuperT)GetMaxAbsAccVec<TypeP>(1,accurate0,accurate1);

        NEATValue testAccVal = NEATALU::mix<TypeP>(accurate0[0],accurate1[0],accA[0]);
        refAccValFloat = RefALU::sub(SuperT(*accurate1[0].GetAcc<TypeP>()), SuperT(*accurate0[0].GetAcc<TypeP>()));
        refAccValFloat = RefALU::mul(refAccValFloat,SuperT(*accA[0].GetAcc<TypeP>()));
        refAccValFloat = RefALU::add(refAccValFloat,SuperT(*accurate0[0].GetAcc<TypeP>()));

        EXPECT_TRUE(TestAccExpandedDotMix<SuperT>(refAccValFloat,testAccVal,refForUlps,ulpErr));

        /* test for NEAT value interval */
        NEATValue testIntVal = NEATALU::mix<TypeP>(interval0[0], interval1[0], intA[0]);
        {
            refForUlps = (SuperT)GetMaxAbsInt<TypeP>(interval0[0], interval1[0]);

            SuperT r[4];
            r[0] = RefALU::sub(SuperT(*interval1[0].GetMin<TypeP>()), SuperT(*interval0[0].GetMin<TypeP>()));
            r[1] = RefALU::sub(SuperT(*interval1[0].GetMax<TypeP>()), SuperT(*interval0[0].GetMax<TypeP>()));

            refIntValMax = GetMaxValue<SuperT>(r,2);
            refIntValMin = GetMinValue<SuperT>(r,2);

            r[0] = RefALU::mul(refIntValMin, SuperT(*intA[0].GetMin<TypeP>()));
            r[1] = RefALU::mul(refIntValMin, SuperT(*intA[0].GetMax<TypeP>()));
            r[2] = RefALU::mul(refIntValMax, SuperT(*intA[0].GetMax<TypeP>()));
            r[3] = RefALU::mul(refIntValMax, SuperT(*intA[0].GetMin<TypeP>()));

            refIntValMax = GetMaxValue<SuperT>(r,4);
            refIntValMin = GetMinValue<SuperT>(r,4);

            refIntValMax = RefALU::add(refIntValMax,SuperT(*interval0[0].GetMax<TypeP>()));
            refIntValMin = RefALU::add(refIntValMin,SuperT(*interval0[0].GetMin<TypeP>()));

            EXPECT_TRUE(TestIntExpandedDotMix<SuperT>(refIntValMin,refIntValMax,testIntVal,refForUlps,ulpErr));
        }

        /* test for vector of NEAT accurate, third arg is value */
        NEATVector testAccVec = NEATALU::mix<TypeP>(accurate0,accurate1,accA[0]);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refForUlps = (SuperT)GetMaxAbsAcc<TypeP>(accurate0[i],accurate1[i]);

            refAccValFloat = RefALU::sub(SuperT(*accurate1[i].GetAcc<TypeP>()), SuperT(*accurate0[i].GetAcc<TypeP>()));
            refAccValFloat = RefALU::mul(refAccValFloat,SuperT(*accA[0].GetAcc<TypeP>()));
            refAccValFloat = RefALU::add(refAccValFloat,SuperT(*accurate0[i].GetAcc<TypeP>()));

            EXPECT_TRUE(TestAccExpandedDotMix<SuperT>(refAccValFloat,testAccVec[i],refForUlps,ulpErr));
        }
        /* test for vector of NEAT interval, third arg is value */
        NEATVector testIntVec = NEATALU::mix<TypeP>(interval0, interval1, intA[0]);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refForUlps = (SuperT)GetMaxAbsInt<TypeP>(interval0[i], interval1[i]);

            SuperT r[4];
            r[0] = RefALU::sub(SuperT(*interval1[i].GetMin<TypeP>()), SuperT(*interval0[i].GetMin<TypeP>()));
            r[1] = RefALU::sub(SuperT(*interval1[i].GetMax<TypeP>()), SuperT(*interval0[i].GetMax<TypeP>()));

            refIntValMax = GetMaxValue<SuperT>(r,2);
            refIntValMin = GetMinValue<SuperT>(r,2);

            r[0] = RefALU::mul(refIntValMin, SuperT(*intA[0].GetMin<TypeP>()));
            r[1] = RefALU::mul(refIntValMin, SuperT(*intA[0].GetMax<TypeP>()));
            r[2] = RefALU::mul(refIntValMax, SuperT(*intA[0].GetMax<TypeP>()));
            r[3] = RefALU::mul(refIntValMax, SuperT(*intA[0].GetMin<TypeP>()));

            refIntValMax = GetMaxValue<SuperT>(r,4);
            refIntValMin = GetMinValue<SuperT>(r,4);

            refIntValMax = RefALU::add(refIntValMax,SuperT(*interval0[i].GetMax<TypeP>()));
            refIntValMin = RefALU::add(refIntValMin,SuperT(*interval0[i].GetMin<TypeP>()));

            EXPECT_TRUE(TestIntExpandedDotMix<SuperT>(refIntValMin,refIntValMax,testIntVec[i],refForUlps,ulpErr));
        }

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accA[i] = NEATValue(aArrFirst[testIdx*wrap.GetSize()+i]);
            if(aArrFirst[testIdx*wrap.GetSize()+i] < aArrSecond[testIdx*wrap.GetSize()+i])
                intA[i] = NEATValue(aArrFirst[testIdx*wrap.GetSize()+i], aArrSecond[testIdx*wrap.GetSize()+i]);
            else
                intA[i] = NEATValue(aArrSecond[testIdx*wrap.GetSize()+i], aArrFirst[testIdx*wrap.GetSize()+i]);
        }

        /* test for vector of NEAT accurate, third arg is vector */
        testAccVec = NEATALU::mix<TypeP>(accurate0,accurate1,accA);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refForUlps = (SuperT)GetMaxAbsAcc<TypeP>(accurate0[i],accurate1[i]);
            refAccValFloat = RefALU::sub(SuperT(*accurate1[i].GetAcc<TypeP>()), SuperT(*accurate0[i].GetAcc<TypeP>()));
            refAccValFloat = RefALU::mul(refAccValFloat,SuperT(*accA[i].GetAcc<TypeP>()));
            refAccValFloat = RefALU::add(refAccValFloat,SuperT(*accurate0[i].GetAcc<TypeP>()));
            EXPECT_TRUE(TestAccExpandedDotMix<SuperT>(refAccValFloat,testAccVec[i],refForUlps,ulpErr));
        }

        /* test for vector of NEAT interval, third arg is vector */
        testIntVec = NEATALU::mix<TypeP>(interval0, interval1, intA);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refForUlps = (SuperT)GetMaxAbsInt<TypeP>(interval0[i], interval1[i]);
            SuperT r[4];
            r[0] = RefALU::sub(SuperT(*interval1[i].GetMin<TypeP>()), SuperT(*interval0[i].GetMin<TypeP>()));
            r[1] = RefALU::sub(SuperT(*interval1[i].GetMax<TypeP>()), SuperT(*interval0[i].GetMax<TypeP>()));

            refIntValMax = GetMaxValue<SuperT>(r,2);
            refIntValMin = GetMinValue<SuperT>(r,2);

            r[0] = RefALU::mul(refIntValMin, SuperT(*intA[i].GetMin<TypeP>()));
            r[1] = RefALU::mul(refIntValMin, SuperT(*intA[i].GetMax<TypeP>()));
            r[2] = RefALU::mul(refIntValMax, SuperT(*intA[i].GetMax<TypeP>()));
            r[3] = RefALU::mul(refIntValMax, SuperT(*intA[i].GetMin<TypeP>()));

            refIntValMax = GetMaxValue<SuperT>(r,4);
            refIntValMin = GetMinValue<SuperT>(r,4);

            refIntValMax = RefALU::add(refIntValMax,SuperT(*interval0[i].GetMax<TypeP>()));
            refIntValMin = RefALU::add(refIntValMin,SuperT(*interval0[i].GetMin<TypeP>()));

            EXPECT_TRUE(TestIntExpandedDotMix<SuperT>(refIntValMin,refIntValMax,testIntVec[i],refForUlps,ulpErr));
        }
    }
}


TYPED_TEST(NEATAluTypedMath, normalize)
{
    typedef typename  TypeParam::Type TypeP;
    typedef typename  superT<TypeP>::type SuperT;

    // Check if we are able to test double built-in function.
    if (SkipDoubleTest<TypeP>()){
        return;
    }

    const int NUM_TESTS = 500;

    TypeP firstFloat[NUM_TESTS*4];
    TypeP secondFloat[NUM_TESTS*4];
    uint32_t indexArr[4];

    VectorWidth curWidth = V4;
    // geometric functions' arguments are float, float2, float3, or float4

    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &firstFloat[0], curWidth, NUM_TESTS);
    GenerateRandomVectorsAutoSeed(this->dataTypeVal, &secondFloat[0], curWidth, NUM_TESTS);
    GenerateRangedVectorsAutoSeed<uint32_t>(U32, &indexArr[0], curWidth, 1, 0, 4);

    /* test for specific values */
    VectorWidthWrapper wrap(curWidth);

    float ulpErr = 2.5f +                 // error in rsqrt + error in multiply
    ( 0.5f * (float) wrap.GetSize() +      // cumulative error for multiplications
     0.5f * (float) (wrap.GetSize()-1));   // cumulative error for additions


    NEATVector a0(curWidth);

    for(uint32_t i = 0; i<wrap.GetSize(); i++)
        a0[i] = NEATValue(firstFloat[i]);

    NEATVector b0 = a0;

   // normalize (v) returns a vector full of NaNs if any element is a NaN
    b0[indexArr[0]] = NEATValue(NEATValue::NaN<TypeP>());
    NEATVector testAcc = NEATALU::normalize<TypeP>(b0);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        EXPECT_TRUE(testAcc[i].IsNaN<TypeP>());
    }

    // normalize (v) returns a vector UNKNOWN if any element is an UNKNOWN
    b0 = a0; // restore b0
    b0[indexArr[1]] = NEATValue(NEATValue::UNKNOWN);
    testAcc = NEATALU::normalize<TypeP>(b0);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        EXPECT_TRUE(testAcc[i].IsUnknown());
    }

    // normalize (v) returns a vector UNKNOWN if any element is an UNWRITTEN
    b0 = a0; // restore b0
    b0[indexArr[2]] = NEATValue(NEATValue::UNWRITTEN);
    testAcc = NEATALU::normalize<TypeP>(b0);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        EXPECT_TRUE(testAcc[i].IsUnknown());
    }

    // normalize (v) returns a vector UNKNOWN if any element is an ANY
    b0 = a0; // restore b0
    b0[indexArr[3]] = NEATValue(NEATValue::ANY);
    testAcc = NEATALU::normalize<TypeP>(b0);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        EXPECT_TRUE(testAcc[i].IsUnknown());
    }

    // normalize ( v ) for which any element in v is infinite shall proceed as if
    // the elements in v were replaced as follows:
    // for( i = 0; i < sizeof(v) / sizeof(v[0] ); i++ )
    // v[i] = isinf(v[i] ) ? copysign(1.0, v[i]) : 0.0 * v [i];
    b0 = a0; // restore b0
    b0[indexArr[0]] = NEATValue(Utils::GetNInf<TypeP>(),Utils::GetNInf<TypeP>());
    testAcc = NEATALU::normalize<TypeP>(b0);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        if(i == indexArr[0]) {
            EXPECT_TRUE(testAcc[i].IsUnknown());
        }
        else {
            TypeP accVal = *testAcc[i].GetAcc<TypeP>();
            if (*b0[i].GetAcc<TypeP>() < 0)
                EXPECT_TRUE(Utils::eq(accVal,TypeP(-0.0)));
            else
                EXPECT_TRUE(Utils::eq(accVal,TypeP(0.0)));
        }
    }

    b0 = a0; // restore b0
    b0[indexArr[1]] = NEATValue(Utils::GetPInf<TypeP>(),Utils::GetPInf<TypeP>());
    testAcc = NEATALU::normalize<TypeP>(b0);
    for(uint32_t i = 0; i<wrap.GetSize(); i++) {
        if(i == indexArr[1])
            EXPECT_TRUE(testAcc[i].IsUnknown());
        else {
            TypeP accVal = *testAcc[i].GetAcc<TypeP>();
            if (*b0[i].GetAcc<TypeP>() < 0)
                EXPECT_TRUE(Utils::eq(accVal,TypeP(-0.0)));
            else
                EXPECT_TRUE(Utils::eq(accVal,TypeP(0.0)));
        }
    }

    for(int testIdx = 0; testIdx < NUM_TESTS; testIdx ++)
    {
        SuperT refAccValFloat, refTotalAcc;
        SuperT refIntValMin, refIntValMax, refTotalMin, refTotalMax;

        NEATVector accurate(curWidth);
        NEATVector interval(curWidth);

        for(uint32_t i = 0; i<wrap.GetSize(); i++)
        {
            accurate[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i]);
            if(firstFloat[testIdx*wrap.GetSize()+i] < secondFloat[testIdx*wrap.GetSize()+i])
                interval[i] = NEATValue(firstFloat[testIdx*wrap.GetSize()+i], secondFloat[testIdx*wrap.GetSize()+i]);
            else
                interval[i] = NEATValue(secondFloat[testIdx*wrap.GetSize()+i], firstFloat[testIdx*wrap.GetSize()+i]);
        }


        /* test for vector of NEAT accurate */
        NEATVector testAccVec = NEATALU::normalize<TypeP>(accurate);

        refTotalAcc = SuperT(0.0);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            SuperT localRes = RefALU::mul(SuperT(*accurate[i].GetAcc<TypeP>()),SuperT(*accurate[i].GetAcc<TypeP>()));
            refTotalAcc = RefALU::add(refTotalAcc,localRes);
        }
        refTotalAcc = RefALU::rsqrt(refTotalAcc);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            refAccValFloat = RefALU::mul(SuperT(*accurate[i].GetAcc<TypeP>()),refTotalAcc);
            EXPECT_TRUE(TestAccExpanded<SuperT>(refAccValFloat,testAccVec[i],ulpErr));
        }

        /* test for vector of NEAT interval */
        NEATVector testIntVec = NEATALU::normalize<TypeP>(interval);

        refTotalMin = SuperT(0.0);
        refTotalMax = SuperT(0.0);
        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            SuperT r[3];

            r[0] = RefALU::mul(SuperT(*interval[i].GetMin<TypeP>()), SuperT(*interval[i].GetMin<TypeP>()));
            r[1] = RefALU::mul(SuperT(*interval[i].GetMax<TypeP>()), SuperT(*interval[i].GetMax<TypeP>()));

            if(*interval[i].GetMin<TypeP>() <= TypeP(0) && *interval[i].GetMax<TypeP>() >= TypeP(0))
            {
                r[2] = SuperT(0);
                refIntValMax = GetMaxValue<SuperT>(r,3);
                refIntValMin = GetMinValue<SuperT>(r,3);
            } else {
                refIntValMax = GetMaxValue<SuperT>(r,2);
                refIntValMin = GetMinValue<SuperT>(r,2);
            }

            refTotalMin = RefALU::add(refTotalMin,refIntValMin);
            refTotalMax = RefALU::add(refTotalMax,refIntValMax);
        }
        refTotalMin = RefALU::rsqrt(refTotalMin);
        refTotalMax = RefALU::rsqrt(refTotalMax);

        for(uint32_t i = 0; i<wrap.GetSize(); i++) {
            SuperT r[4];
            r[0] = RefALU::mul(SuperT(*interval[i].GetMax<TypeP>()),refTotalMax);
            r[1] = RefALU::mul(SuperT(*interval[i].GetMin<TypeP>()),refTotalMax);
            r[2] = RefALU::mul(SuperT(*interval[i].GetMax<TypeP>()),refTotalMin);
            r[3] = RefALU::mul(SuperT(*interval[i].GetMin<TypeP>()),refTotalMin);

            refIntValMax = GetMaxValue<SuperT>(r,4);
            refIntValMin = GetMinValue<SuperT>(r,4);

            EXPECT_TRUE(TestIntExpanded<SuperT>(refIntValMin,refIntValMax,testIntVec[i],ulpErr));
        }


        float ulpsErrVal = 3.0f; // 3 ulps = error in rsqrt + error in multiply + cumulative error for multiplications
        /* test for NEAT value accurate */
        NEATValue testAccVal = NEATALU::normalize<TypeP>(accurate[0]);
        refTotalAcc = RefALU::mul(SuperT(*accurate[0].GetAcc<TypeP>()),SuperT(*accurate[0].GetAcc<TypeP>()));
        refTotalAcc = RefALU::rsqrt(refTotalAcc);
        refAccValFloat = RefALU::mul(SuperT(*accurate[0].GetAcc<TypeP>()),refTotalAcc);
        EXPECT_TRUE(TestAccExpanded<SuperT>(refAccValFloat,testAccVal,ulpsErrVal));

        /* test for NEAT value interval */
        NEATValue testIntVal = NEATALU::normalize<TypeP>(interval[0]);
        {
            TypeP min = *interval[0].GetMin<TypeP>();
            TypeP max = *interval[0].GetMax<TypeP>();
            if(min > 0)
                EXPECT_TRUE(TestAccExpanded<SuperT>(SuperT(1.0),testIntVal,ulpsErrVal));
            else if (max < 0)
                EXPECT_TRUE(TestAccExpanded<SuperT>(SuperT(-1.0),testIntVal,ulpsErrVal));
            else
                EXPECT_TRUE(testIntVal.IsUnknown());
        }
    }
}

TYPED_TEST(NEATAluTypedMath, copysign)
{
    typedef typename  TypeParam::Type T;

    /// If your test doesn't currently support doubles, use SkipDoubleTest
    if (SkipDoubleTest<T>()){
        return;
    }

    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 100;
    // Vector data type length.
    static const uint32_t vectorWidth = 8;
    // Intervals with test values for common functions arguments.

    // Parameters for random data generator.
    DataTypeVal dataTypeVal = GetDataTypeVal<T>();
    VectorWidth currWidth = VectorWidthWrapper::ValueOf(vectorWidth);

    T Arg1Min[NUM_TESTS*vectorWidth];
    T Arg1Max[NUM_TESTS*vectorWidth];
    T Arg2Min[NUM_TESTS*vectorWidth];
    T Arg2Max[NUM_TESTS*vectorWidth];

    // positive data
    GenerateRangedVectorsAutoSeed<T>(dataTypeVal, &Arg1Min[0], currWidth, NUM_TESTS,0,1000);
    GenerateRangedVectorsAutoSeed<T>(dataTypeVal, &Arg1Max[0], currWidth, NUM_TESTS,0,1000);
    GenerateRangedVectorsAutoSeed<T>(dataTypeVal, &Arg2Min[0], currWidth, NUM_TESTS,0,1000);
    GenerateRangedVectorsAutoSeed<T>(dataTypeVal, &Arg2Max[0], currWidth, NUM_TESTS,0,1000);

    // Make random data aligned with the names: ArgMin must be <= ArgMax
    for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
    {
        if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
        if (Arg2Min[i] > Arg2Max[i]) std::swap(Arg2Min[i], Arg2Max[i]);
    }

    NEATValue testVal;
    NEATValue notNan = NEATValue(10.5);
    // Test special NEAT values: UNKNOWN, UNWRITTEN and ANY.    
    EXPECT_TRUE(TestUnwritten<T>(NEATALU::copysign<T>));
    EXPECT_TRUE(TestAny<T>(NEATALU::copysign<T>));
    EXPECT_TRUE(TestUnknown<T>(NEATALU::copysign<T>));


    NEATValue nan = NEATValue::NaN<T>();
    T refVal, refMin, refMax;

    testVal = NEATALU::copysign<T>(nan, notNan);
    EXPECT_TRUE(testVal.IsNaN<T>());
    testVal = NEATALU::copysign<T>(notNan, nan);
    EXPECT_TRUE(testVal.IsNaN<T>());
    testVal = NEATALU::copysign<T>(nan, nan);
    EXPECT_TRUE(testVal.IsNaN<T>());

    for(uint32_t testIdx = 0; testIdx < NUM_TESTS; ++testIdx)
    {
        // 1. test for x is accurate and y is accurate
        // 1.1 x0, y0
        T x0 = Arg1Min[testIdx];
        T y0 = Arg2Min[testIdx];
        testVal = NEATALU::copysign<T>(NEATValue(x0), NEATValue(y0));
        refVal = RefALU::copysign<T>(x0, y0);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetAcc<T>(),refVal));

        T xVec0[vectorWidth], yVec0[vectorWidth];
        NEATVector xNEATVec(currWidth),yNEATVec(currWidth),testVec(currWidth);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec0[i] = Arg1Min[testIdx * vectorWidth + i];
            yVec0[i] = Arg2Min[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec0[i]);
            yNEATVec[i] = NEATValue(yVec0[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refVal = RefALU::copysign<T>(xVec0[i], yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetAcc<T>(),refVal));
        }

        // 1.2 -x0, y0
        testVal = NEATALU::copysign<T>(NEATValue(-x0),  NEATValue(y0));
        refVal = RefALU::copysign<T>(-x0, y0);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetAcc<T>(),refVal));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xNEATVec[i] = NEATValue(-xVec0[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refVal = RefALU::copysign<T>(-xVec0[i], yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetAcc<T>(),refVal));
        }
        // 1.3 x0, -y0
        testVal = NEATALU::copysign<T>(NEATValue(x0), NEATValue(-y0));
        refVal = RefALU::copysign<T>(x0, -y0);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetAcc<T>(),refVal));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xNEATVec[i] = NEATValue(xVec0[i]);
            yNEATVec[i] = NEATValue(-yVec0[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refVal = RefALU::copysign<T>(xVec0[i], -yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetAcc<T>(),refVal));
        }
        // 1.4 -x0, -y0
        testVal = NEATALU::copysign<T>(NEATValue(-x0), NEATValue(-y0));
        refVal = RefALU::copysign<T>(-x0, -y0);
        EXPECT_TRUE(testVal.IsAcc());
        EXPECT_TRUE(Utils::eq<T>(*testVal.GetAcc<T>(),refVal));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xNEATVec[i] = NEATValue(-xVec0[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refVal = RefALU::copysign<T>(-xVec0[i], -yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetAcc<T>(),refVal));
        }
    
        // 2. test for x is interval and y is accurate
        T x1 = Arg1Min[testIdx];
        T x2 = Arg1Max[testIdx];
        // 2.1 (x1,x2), y0
        testVal = NEATALU::copysign<T>(NEATValue(x1,x2), NEATValue(y0));
        refMin = RefALU::copysign<T>(x1, y0);
        refMax = RefALU::copysign<T>(x2, y0);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));

        T xVec1[vectorWidth], xVec2[vectorWidth];
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = Arg1Min[testIdx * vectorWidth + i];
            xVec2[i] = Arg1Max[testIdx * vectorWidth + i];
            yVec0[i] = Arg2Min[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec1[i],xVec2[i]);
            yNEATVec[i] = NEATValue(yVec0[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec1[i], yVec0[i]);
            refMax = RefALU::copysign<T>(xVec2[i], yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }

        // 2.2 (-x1,x2), y0
        testVal = NEATALU::copysign<T>(NEATValue(-x1,x2), NEATValue(y0));    
        // numbers from -x1 to 0 became positive, so result interval is (0,x2)
        refMin = 0; 
        refMax = RefALU::copysign<T>(x2, y0);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = -Arg1Min[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec1[i],xVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMax = RefALU::copysign<T>(xVec2[i], yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 2.3 (-x2,-x1), y0
        testVal = NEATALU::copysign<T>(NEATValue(-x2,-x1), NEATValue(y0));    
        refMax = RefALU::copysign<T>(-x2, y0); // abs(x2) > abs(x1), so it is max
        refMin = RefALU::copysign<T>(-x1, y0);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = -Arg1Max[testIdx * vectorWidth + i];
            xVec2[i] = -Arg1Min[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec1[i],xVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec2[i], yVec0[i]);
            refMax = RefALU::copysign<T>(xVec1[i], yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 2.4 (x1,x2), -y0
        testVal = NEATALU::copysign<T>(NEATValue(x1,x2), NEATValue(-y0));
        refMax = RefALU::copysign<T>(x1, -y0);
        refMin = RefALU::copysign<T>(x2, -y0);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = Arg1Min[testIdx * vectorWidth + i];
            xVec2[i] = Arg1Max[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec1[i],xVec2[i]);
            yVec0[i] = -Arg2Min[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec0[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMax = RefALU::copysign<T>(xVec1[i], yVec0[i]);
            refMin = RefALU::copysign<T>(xVec2[i], yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 2.5 (-x1,x2), -y0
        testVal = NEATALU::copysign<T>(NEATValue(-x1,x2), NEATValue(-y0));    
        // numbers from 0 to x2 became negative, so result interval is (-x2,0)
        refMax = 0; 
        refMin = RefALU::copysign<T>(x2, -y0);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = -Arg1Min[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec1[i],xVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {            
            refMin = RefALU::copysign<T>(xVec2[i], yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 2.6 (-x2,-x1), y0
        testVal = NEATALU::copysign<T>(NEATValue(-x2,-x1), NEATValue(y0));
        refMax = RefALU::copysign<T>(-x2, y0);  // abs(x2) > abs(x1), so it is max
        refMin = RefALU::copysign<T>(-x1, y0);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = -Arg1Max[testIdx * vectorWidth + i];
            xVec2[i] = -Arg1Min[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec1[i],xVec2[i]);
            yVec0[i] = Arg2Min[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec0[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMax = RefALU::copysign<T>(xVec1[i], yVec0[i]);
            refMin = RefALU::copysign<T>(xVec2[i], yVec0[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 3. test for x is accurate and y is interval
        // 3.1 x0, (y1,y2)
        T yVec1[vectorWidth], yVec2[vectorWidth];
        T y1 = Arg2Min[testIdx];
        T y2 = Arg2Max[testIdx];
        testVal = NEATALU::copysign<T>(NEATValue(x0), NEATValue(y1,y2));
        refMin = RefALU::copysign<T>(x0, y1);
        refMax = RefALU::copysign<T>(x0, y2);
        // result shell be accurate
        EXPECT_TRUE(testVal.IsAcc());
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec0[i] = Arg1Max[testIdx * vectorWidth + i];
            yVec1[i] = Arg2Min[testIdx * vectorWidth + i];
            yVec2[i] = Arg2Max[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec0[i]);
            yNEATVec[i] = NEATValue(yVec1[i], yVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec0[i], yVec1[i]);
            refMax = RefALU::copysign<T>(xVec0[i], yVec2[i]);
            EXPECT_TRUE(testVec[i].IsAcc());
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 3.2 x0, (-y1,y2)
        testVal = NEATALU::copysign<T>(NEATValue(x0), NEATValue(-y1,y2));    
        // result values are -x0 and x0 exactly with no allowed interval 
        // between these values, so result is UNKNOWN
        EXPECT_TRUE( testVal.IsUnknown());
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            yVec1[i] = -Arg2Min[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec1[i], yVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            EXPECT_TRUE(testVec[i].IsUnknown());
        }
        // 3.3 x0, (-y2,-y1)
        testVal = NEATALU::copysign<T>(NEATValue(x0), NEATValue(-y2,-y1));
        refMax = RefALU::copysign<T>(x0, -y2);
        refMin = RefALU::copysign<T>(x0, -y1);
        // result shell be accurate
        EXPECT_TRUE(testVal.IsAcc());
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetAcc<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetAcc<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            yVec1[i] = -Arg2Max[testIdx * vectorWidth + i];
            yVec2[i] = -Arg2Min[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec1[i], yVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec0[i], yVec1[i]);
            refMax = RefALU::copysign<T>(xVec0[i], yVec2[i]);
            EXPECT_TRUE(testVec[i].IsAcc());
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 3.4 -x0, (y1,y2)
        y1 = Arg2Min[testIdx]; y2 = Arg2Max[testIdx];
        testVal = NEATALU::copysign<T>(NEATValue(-x0), NEATValue(y1,y2));
        refMin = RefALU::copysign<T>(-x0, y1);
        refMax = RefALU::copysign<T>(-x0, y2);
        // result shell be accurate
        EXPECT_TRUE(testVal.IsAcc());
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetAcc<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetAcc<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec0[i] = -Arg1Max[testIdx * vectorWidth + i];
            yVec1[i] = Arg2Min[testIdx * vectorWidth + i];
            yVec2[i] = Arg2Max[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec0[i]);
            yNEATVec[i] = NEATValue(yVec1[i], yVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec0[i], yVec1[i]);
            refMax = RefALU::copysign<T>(xVec0[i], yVec2[i]);
            EXPECT_TRUE(testVec[i].IsAcc());
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 3.5 -x0, (-y1,y2)
        testVal = NEATALU::copysign<T>(NEATValue(-x0), NEATValue(-y1,y2));    
        // result values are -x0 and x0 exactly with no allowed interval 
        // between these values, so result is UNKNOWN
        EXPECT_TRUE( testVal.IsUnknown());
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            yVec1[i] = -Arg2Min[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec1[i], yVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            EXPECT_TRUE(testVec[i].IsUnknown());
        }
        // 3.6 -x0, (-y2,-y1)
        testVal = NEATALU::copysign<T>(NEATValue(-x0), NEATValue(-y2,-y1));
        refMax = RefALU::copysign<T>(-x0, -y2);
        refMin = RefALU::copysign<T>(-x0, -y1);
        // result shell be accurate
        EXPECT_TRUE(testVal.IsAcc());
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));

        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            yVec1[i] = -Arg2Max[testIdx * vectorWidth + i];
            yVec2[i] = -Arg2Min[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec1[i], yVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec0[i], yVec1[i]);
            refMax = RefALU::copysign<T>(xVec0[i], yVec2[i]);
            EXPECT_TRUE(testVec[i].IsAcc());
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }

        // 4. test for x is interval and y is interval
        // 4.1 (x1,x2) (y1,y2)
        testVal = NEATALU::copysign<T>(NEATValue(x1,x2), NEATValue(y1,y2));
        // copysign(x1,y1) == copysign(x1,y2) and 
        // copysign(x2,y1) == copysign(x2,y2) because y1>0 and y2>0
        refMin = RefALU::copysign<T>(x1, y1);
        refMax = RefALU::copysign<T>(x2, y2);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = Arg1Min[testIdx * vectorWidth + i];
            xVec2[i] = Arg1Max[testIdx * vectorWidth + i];
            yVec1[i] = Arg2Min[testIdx * vectorWidth + i];
            yVec2[i] = Arg2Max[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec1[i], yVec2[i]);
            xNEATVec[i] = NEATValue(xVec1[i], xVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec1[i], yVec1[i]);
            refMax = RefALU::copysign<T>(xVec2[i], yVec2[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 4.2 (-x1,x2) (y1,y2)
        testVal = NEATALU::copysign<T>(NEATValue(-x1,x2), NEATValue(y1,y2));
        // numbers from -x1 to 0 became positive, so result interval is (0,x2)
        refMin = 0;
        refMax = RefALU::copysign<T>(x2, y2);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = -Arg1Min[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec1[i], xVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMax = RefALU::copysign<T>(xVec2[i], yVec2[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 4.3 (-x2,-x1) (y1,y2)
        testVal = NEATALU::copysign<T>(NEATValue(-x2,-x1), NEATValue(y1,y2));    
        refMin = RefALU::copysign<T>(-x1, y1);
        refMax = RefALU::copysign<T>(-x2, y2); // abs(x2) > abs(x1), so it is max
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = -Arg1Min[testIdx * vectorWidth + i];
            xVec2[i] = -Arg1Max[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec2[i], xVec1[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec1[i], yVec1[i]);
            refMax = RefALU::copysign<T>(xVec2[i], yVec2[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 4.4 (x1,x2) (-y2,-y1)
        testVal = NEATALU::copysign<T>(NEATValue(x1,x2), NEATValue(-y2,-y1));
        // copysign(x1,-y1) == copysign(x1,-y2) and 
        // copysign(x2,-y1) == copysign(x2,-y2) because -y1<0 and -y2<0
        refMax = RefALU::copysign<T>(x1, -y1);
        refMin = RefALU::copysign<T>(x2, -y2);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = Arg1Min[testIdx * vectorWidth + i];
            xVec2[i] = Arg1Max[testIdx * vectorWidth + i];
            yVec1[i] = -Arg2Min[testIdx * vectorWidth + i];
            yVec2[i] = -Arg2Max[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec2[i], yVec1[i]);
            xNEATVec[i] = NEATValue(xVec1[i], xVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMax = RefALU::copysign<T>(xVec1[i], yVec1[i]);
            refMin = RefALU::copysign<T>(xVec2[i], yVec2[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 4.5 (-x1,x2) (-y2,-y1)
        testVal = NEATALU::copysign<T>(NEATValue(-x1,x2), NEATValue(-y2,-y1));
        // numbers from 0 to x2 became negative, so result interval is (-x2,0)
        refMax = 0;
        refMin = RefALU::copysign<T>(x2, -y2);
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = -Arg1Min[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec1[i], xVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec2[i], yVec2[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }
        // 4.6 (-x2,-x1) (-y2,-y1)
        testVal = NEATALU::copysign<T>(NEATValue(-x2,-x1), NEATValue(-y2,-y1));    
        refMax = RefALU::copysign<T>(-x1, -y1);
        refMin = RefALU::copysign<T>(-x2, -y2); // abs(x2) > abs(x1), so it is min
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = -Arg1Min[testIdx * vectorWidth + i];
            xVec2[i] = -Arg1Max[testIdx * vectorWidth + i];
            yVec1[i] = -Arg2Min[testIdx * vectorWidth + i];
            yVec2[i] = -Arg2Max[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec2[i], yVec1[i]);
            xNEATVec[i] = NEATValue(xVec2[i], xVec1[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMax = RefALU::copysign<T>(xVec1[i], yVec1[i]);
            refMin = RefALU::copysign<T>(xVec2[i], yVec2[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        }    
        // 4.7 (x1,x2) (-y1,y2)
        testVal = NEATALU::copysign<T>(NEATValue(x1,x2), NEATValue(-y1,y2));
        // abs(x2) > abs(x1)
        refMin = RefALU::copysign<T>(x2, -y1); // so it is min
        refMax = RefALU::copysign<T>(x2, y2);  // and it is max
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        // 4.8 (-x1,x2) (-y1,y2)
        testVal = NEATALU::copysign<T>(NEATValue(-x1,x2), NEATValue(-y1,y2));
        // abs(x2) > abs(x1)
        refMin = RefALU::copysign<T>(x2, -y1); // so it is min
        refMax = RefALU::copysign<T>(x2, y2);  // and it is max
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = Arg1Min[testIdx * vectorWidth + i];
            xVec2[i] = Arg1Max[testIdx * vectorWidth + i];
            yVec1[i] = -Arg2Min[testIdx * vectorWidth + i];
            yVec2[i] = Arg2Max[testIdx * vectorWidth + i];
            yNEATVec[i] = NEATValue(yVec1[i], yVec2[i]);
            xNEATVec[i] = NEATValue(xVec1[i], xVec2[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec2[i], yVec1[i]);
            refMax = RefALU::copysign<T>(xVec2[i], yVec2[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        } 
        // 4.9 (-x2,-x1) (-y1,y2)
        testVal = NEATALU::copysign<T>(NEATValue(-x2,-x1), NEATValue(-y1,y2));
        // abs(x2) > abs(x1)
        refMin = RefALU::copysign<T>(-x2, -y1); // so it is min
        refMax = RefALU::copysign<T>(-x2, y2);  // and it is max
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMin<T>(),refMin));
        EXPECT_TRUE( Utils::eq<T>(*testVal.GetMax<T>(),refMax));
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            xVec1[i] = -Arg1Min[testIdx * vectorWidth + i];
            xVec2[i] = -Arg1Max[testIdx * vectorWidth + i];
            xNEATVec[i] = NEATValue(xVec2[i], xVec1[i]);
        }
        testVec = NEATALU::copysign<T>(xNEATVec, yNEATVec);
        for(uint32_t i = 0; i<vectorWidth; ++i)
        {
            refMin = RefALU::copysign<T>(xVec2[i], yVec1[i]);
            refMax = RefALU::copysign<T>(xVec2[i], yVec2[i]);
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMin<T>(),refMin));
            EXPECT_TRUE( Utils::eq<T>(*testVec[i].GetMax<T>(),refMax));
        } 
    }
}


template <typename T, typename I>
class SelectTypeContainer {
    public:
        typedef T ABType;
        typedef I CType;
};

template <typename T>
class NEATRelationalTestSelect : public ALUTest {
public:
    // Number of different random inputs to test.
    static const uint32_t NUM_TESTS = 200;
    // Vector data type length.
    static const uint32_t vectorWidth = 8;
    // Intervals with test values for common functions arguments.

    // Parameters for random data generator.
    VectorWidth currWidth;
    DataTypeVal dataTypeValAB, dataTypeValC;

    typedef typename  T::ABType TypeAB;
    typedef typename  T::CType TypeC;

    TypeAB Arg1Min[NUM_TESTS*vectorWidth];
    TypeAB Arg1Max[NUM_TESTS*vectorWidth];
    TypeAB Arg2Min[NUM_TESTS*vectorWidth];
    TypeAB Arg2Max[NUM_TESTS*vectorWidth];
    TypeC Arg3[NUM_TESTS*vectorWidth];

    NEATRelationalTestSelect()
    {
        currWidth = VectorWidthWrapper::ValueOf(vectorWidth);

        dataTypeValAB = GetDataTypeVal<TypeAB>();
        dataTypeValC = GetDataTypeVal<TypeC>();

        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeValAB, &Arg1Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeValAB, &Arg1Max[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeValAB, &Arg2Min[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeValAB, &Arg2Max[0], currWidth, NUM_TESTS);
        GenerateRandomVectorsAutoSeed(dataTypeValC, &Arg3[0], currWidth, NUM_TESTS);

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (unsigned int i = 0; i < NUM_TESTS * vectorWidth; ++i)
        {
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);
            if (Arg2Min[i] > Arg2Max[i]) std::swap(Arg2Min[i], Arg2Max[i]);
        }
    }
};


typedef ::testing::Types<SelectTypeContainer<float,int8_t>,SelectTypeContainer<float,uint8_t>,
                         SelectTypeContainer<float,int16_t>,SelectTypeContainer<float,uint16_t>,
                         SelectTypeContainer<float,int32_t>,SelectTypeContainer<float,uint32_t>,
                         SelectTypeContainer<float,int64_t>,SelectTypeContainer<float,uint64_t> > SelectTypes;

TYPED_TEST_CASE(NEATRelationalTestSelect, SelectTypes);

TYPED_TEST(NEATRelationalTestSelect, select)
{
    typedef typename  TypeParam::ABType TypeAB;
    typedef typename  TypeParam::CType TypeC;

    if (SkipDoubleTest<TypeAB>()){
        return;
    }

    VectorWidth Width = VectorWidthWrapper::ValueOf(this->vectorWidth);

    // Done with corner cases. Now go to testing scalar and vector functions
    for(uint32_t testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx)
    {
        NEATVector a(Width),b(Width),res(Width);
        std::vector<int64_t> cond;

        // Test on accurate values.
        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            a[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i]);
            b[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i]);
        }

        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            int64_t c = 0;
            if (this->Arg3[testIdx*this->vectorWidth+i] & getMSB<TypeC>())
                c = int64_t(INT64_MIN); // set MSB
            
            cond.push_back(c);
        }

        // Test scalar result = c ? b : a
        NEATValue tstVal = NEATALU::select<TypeAB>(a[0], b[0], cond[0]);

        bool passed = false;
        if ( this->Arg3[testIdx*this->vectorWidth] )
            passed = NEATValue::areEqual<TypeAB>(tstVal,b[0]);
        else
            passed = NEATValue::areEqual<TypeAB>(tstVal,a[0]);
        EXPECT_TRUE(passed);
        
        // Test vector result[i] = if MSB of c[i] is set ? b[i] : a[i]
        NEATVector tstVec = NEATALU::select<TypeAB>(a, b, cond);

        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            TypeC localCond = this->Arg3[testIdx*this->vectorWidth+i] & getMSB<TypeC>();
            passed = false;
            if ( localCond )
                passed = NEATValue::areEqual<TypeAB>(tstVec[i],b[i]);
            else
                passed = NEATValue::areEqual<TypeAB>(tstVec[i],a[i]);
            EXPECT_TRUE(passed);
        }

        // Test on interval values.
        for(uint32_t i = 0; i < this->vectorWidth; ++i)
        {
            a[i] = NEATValue(this->Arg1Min[testIdx*this->vectorWidth+i], this->Arg1Max[testIdx*this->vectorWidth+i]);
            b[i] = NEATValue(this->Arg2Min[testIdx*this->vectorWidth+i], this->Arg2Max[testIdx*this->vectorWidth+i]);
        }

        // Test scalar result = c ? b : a
        tstVal = NEATALU::select<TypeAB>(a[0], b[0],  this->Arg3[testIdx*this->vectorWidth] );

        passed = false;
        if ( this->Arg3[testIdx*this->vectorWidth] )
            passed = NEATValue::areEqual<TypeAB>(tstVal,b[0]);
        else
            passed = NEATValue::areEqual<TypeAB>(tstVal,a[0]);
        EXPECT_TRUE(passed);

        // Test vector result[i] = if MSB of c[i] is set ? b[i] : a[i]
        tstVec = NEATALU::select<TypeAB>(a, b, cond);

        for(uint32_t i = 0; i < this->vectorWidth; ++i) {
            TypeC localCond = this->Arg3[testIdx*this->vectorWidth+i] & getMSB<TypeC>();
            passed = false;
            if ( localCond )
                passed = NEATValue::areEqual<TypeAB>(tstVec[i],b[i]);
            else
                passed = NEATValue::areEqual<TypeAB>(tstVec[i],a[i]);
            EXPECT_TRUE(passed);
        }

    }
}