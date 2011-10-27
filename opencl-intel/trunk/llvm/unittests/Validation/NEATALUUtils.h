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

File Name:  NEATALUUtils.h

\*****************************************************************************/
#ifndef __NEATALUUTILS_H__
#define __NEATALUUTILS_H__

// \brief Useful functions to share between NEATALU tests.

#include "NEATValue.h"
#include <stdio.h>
#include "NEATALU.h"

#define isnan( x )   ((x) != (x))

#ifndef LOCAL_MAX
#define LOCAL_MAX( _a, _b )       ((_a) > (_b) ? (_a) : (_b))
#endif

#ifndef FLT_MANT_DIG
#define FLT_MANT_DIG    24                      /* # of bits in mantissa */
#endif
#ifndef FLT_MIN_EXP
#define FLT_MIN_EXP     (-125)                  /* min binary exponent */
#endif
#ifndef DBL_MANT_DIG
#define DBL_MANT_DIG    53                      /* # of bits in mantissa */
#endif
#ifndef DBL_MIN_EXP
#define DBL_MIN_EXP     (-1021)                 /* min binary exponent */
#endif

namespace Validation {

    template <typename T> T getMSB();

    // class to combine typed tests and value-parameterized tests in googletest framework
    template <typename T, bool inMode>
    class ValueTypeContainer {
        public:
            typedef T Type;
            static const bool mode = inMode;
    };


    template <typename T> DataTypeVal GetDataTypeVal();

    // the limit of difference between given ulps and calculated ulps
    static const float diffLimit = 2.0f;

    bool    RefIsinf(double _x);
    double  RefCopysign( double x, double y );

    int MyIlogb (double x);
    int MyIlogbl (long double x);

    template<typename T>
    T FindMax(T* arr, uint32_t count)
    {
        T res = arr[0];
        for(uint32_t i = 1; i<count; i++)
            res = std::max(res, arr[i]);
        return res;
    }

    template<typename T>
    T FindMin(const T* const arr, uint32_t count)
    {
        T res = arr[0];
        for(uint32_t i = 1; i<count; i++)
            res = std::min(res, arr[i]);
        return res;
    }

    template <typename T>
    static bool TestAccValue(NEATValue testAccVal, T refVal)
    {
        bool passed = testAccVal.IsAcc();
        T AccVal;
        memcpy(&AccVal, testAccVal.GetAcc<T>(), sizeof(T));
        if (passed) {
            passed = Utils::eq(AccVal, (T)refVal);
        }
        return passed;
    }

    template <typename T>
    static bool TestAccExpanded(T ref, NEATValue test, float ulps)
    {
        bool res = true;
        typedef typename  downT<T>::type dT;

        dT min = *test.GetMin<dT>();
        dT max = *test.GetMax<dT>();

        min = RefALU::flush<dT>(min);
        max = RefALU::flush<dT>(max);

        if(Utils::IsNInf(ref)) {
            if(!test.IsAcc())
                return false;
            if(! Utils::IsNInf(*test.GetAcc<dT>()))
                return false;
        } else if (Utils::IsPInf(ref)) {
            if(!test.IsAcc())
                return false;
            if(! Utils::IsPInf(*test.GetAcc<dT>()))
                return false;
        } else if(ref == 0 && RefALU::GetFTZmode()) {
            // one ulp for zero is denormal in float point precision
            // so, flushed min and max should be zero
            if( min != 0 || max != 0)
                return false;
        } else if(Utils::IsNaN(ref))
        {
            if(!test.IsNaN<dT>())
                return false;
        }
        else {
            dT refMin = NEATALU::castDown(ref-ulps*NEATALU::ComputeUlp(ref));
            dT refMax = NEATALU::castDown(ref+ulps*NEATALU::ComputeUlp(ref));

            float diff1 = Utils::ulpsDiff(ref,min);
            float diff2 = Utils::ulpsDiff(ref,max);

            res = true;
            // if refMin is INF, min should be INF
            if(Utils::IsInf<dT>(refMin)) {
                res &= Utils::IsInf<dT>(min);
            } else
            // if min is denormal, it should be flashed to zero,
            // otherwise sub ulps
            if (refMin==0 || Utils::IsDenorm<dT>(refMin)) {
                if (RefALU::GetFTZmode())
                    res &= (min==0);
                else {
                    float diffDenorm = Utils::ulpsDiffDenormal(ref,min);
                    res &= (fabs(diffDenorm) <= ulps);
                    res &= (fabs(diffDenorm) >= (ulps-diffLimit));
                }
            } else {
                res &= (fabs(diff1) <= ulps);
                res &= (fabs(diff1) >= (ulps-diffLimit));
            }

            // if refMax is INF, max should be INF
            if(Utils::IsInf<dT>(refMax)) {
                res &= Utils::IsInf<dT>(max);
            } else
            // if max is denormal, it should be flashed to zero,
            // otherwise add ulps
            if (refMax==0 || Utils::IsDenorm<dT>(refMax)) {
                if (RefALU::GetFTZmode())
                    res &= (max==0);
                else {
                    float diffDenorm = Utils::ulpsDiffDenormal(ref,max);
                    res &= (fabs(diffDenorm) <= ulps);
                    res &= (fabs(diffDenorm) >= (ulps-diffLimit));
                }
            } else {
                res &= (fabs(diff2) <= ulps);
                res &= (fabs(diff2) >= (ulps-diffLimit));
            }
        }
        return res;
    }

    /// Tests NEAT for accurate NEATValue input.
    /// It takes NEAT input and output values, refernce output, relative error and corner cases for this function
    /// It ensures that in edge case result is equal to reference and accurate.
    /// If input is not an edge case it checks that output interval is correct according to fiven accuracy.
    /// @param neatIn   Input value for testing NEAT function
    /// @param neatOut  Ouput of testing NEAT function
    /// @param ref      Reference output to compare with
    /// @param ulpErr   Function accuracy
    /// @param cor      Pointer to buffer with corner case inputs
    /// @param corSize  Size of buffer with corner cases
    template <typename T>
    static bool TestNeatAcc(NEATValue neatIn, NEATValue neatOut, T ref, float ulpErr, T* cor, uint32_t corSize)
    {
        typedef typename  downT<T>::type dT;
        bool isCorner = false;
        bool passed = true;
        if(!neatIn.IsAcc())
            return false;
        for(uint32_t i = 0;i<corSize; i++)
        {
            if(Utils::eq((dT)cor[i], *neatIn.GetAcc<dT>()))
                isCorner = true;
        }
        if(isCorner)
        {
            passed &= neatOut.IsAcc();
            passed &= (*neatOut.GetAcc<dT>() == ref);
        }
        else
        {
            passed &= TestAccExpanded<T>(ref, neatOut, ulpErr);
        }
        return passed;
    }

    template <typename T>
    static bool TestIntExpanded(T refMinIn, T refMaxIn, NEATValue test, float ulps)
    {
        typedef typename downT<T>::type dT;

        bool res = true;
        T refMin, refMax;

        if( refMinIn > refMaxIn)
        {
            refMin = refMaxIn;
            refMax = refMinIn;
        } else
        {
            refMin = refMinIn;
            refMax = refMaxIn;
        }

        dT min = *test.GetMin<dT>();
        dT max = *test.GetMax<dT>();

        min = RefALU::flush<dT>(min);
        max = RefALU::flush<dT>(max);

        if(Utils::IsNInf(refMin)) {
            if(! Utils::IsNInf(min))
                return false;
        } else if (Utils::IsPInf(refMin)) {
            if(! Utils::IsPInf(min))
                return false;
        } 
        else if((refMin == 0) && RefALU::GetFTZmode()) {
            // one ulp for zero is denormal in float point precision
            // so, flushed min and max should be zero
            if( min != 0)
                return false;
        } else {
            dT refMinUlps = NEATALU::castDown(refMin-ulps*NEATALU::ComputeUlp(refMin));

            res = true;
            // if refMinUlps is INF, min should be INF
            if(Utils::IsInf<dT>(refMinUlps)) {
                res &= Utils::IsInf<dT>(min);
            } else if (refMinUlps==0 || Utils::IsDenorm<dT>(refMinUlps))
            {
                if (RefALU::GetFTZmode())
                    res &= (min==0);
                else {
                    float diff = Utils::ulpsDiffDenormal(refMin,min);
                    res &= (fabs(diff) <= ulps);
                    res &= (fabs(diff) >= (ulps-diffLimit));
                }
            } else {
                float diff = Utils::ulpsDiff(refMin,min);
                res &= (fabs(diff) <= ulps);
                res &= (fabs(diff) >= (ulps-diffLimit));
            }
        }

        if(Utils::IsNInf(refMax)) {
            if(! Utils::IsNInf(max))
                return false;
        } else if (Utils::IsPInf(refMax)) {
            if(! Utils::IsPInf(max))
                return false;
        } else if((refMax == 0) && RefALU::GetFTZmode()) {
            // one ulp for zero is denormal in float point precision
            // so, flushed max and max should be zero
            if( max != 0)
                return false;
        } else {
            dT refMaxUlps = NEATALU::castDown(refMax+ulps*NEATALU::ComputeUlp(refMax));

            res = true;
            // if refMaxUlps is INF, max should be INF
            if(Utils::IsInf<dT>(refMaxUlps)) {
                res &= Utils::IsInf<dT>(max);
            } else if (refMaxUlps==0 || Utils::IsDenorm<dT>(refMaxUlps))
            {
                if (RefALU::GetFTZmode())
                    res &= (max==0);
                else {
                    float diff = Utils::ulpsDiffDenormal(refMax,max);
                    res &= (fabs(diff) <= ulps);
                    res &= (fabs(diff) >= (ulps-diffLimit));
                }
            } else {
                float diff = Utils::ulpsDiff(refMax,max);
                res &= (fabs(diff) <= ulps);
                res &= (fabs(diff) >= (ulps-diffLimit));
            }

        }
        return res;
    }
    // Tests special NEATValue for function with three arguments.
    // Result must be NEATValue with UNKNOWN status
    template <typename T, NEATValue::Status st>
    bool TestSpecialNEATValue(NEATScalarTernaryOp f){
        bool result = true;
        NEATValue goodValue = NEATValue((T)0.1);
        NEATValue stVal = NEATValue(st);
        NEATValue res = f(stVal, goodValue, goodValue);
        result &= res.IsUnknown();
        res = f(goodValue, stVal, goodValue);
        result &= res.IsUnknown();
        res = f(goodValue, goodValue, stVal);
        result &= res.IsUnknown();
        return result;
    }


    template <typename T>
    static bool TestAccExpandedDotMix(T ref, NEATValue test, T ref4ulps, float ulps)
    {
        bool res = true;
        typedef typename  downT<T>::type dT;

        dT min = *test.GetMin<dT>();
        dT max = *test.GetMax<dT>();

        min = RefALU::flush<dT>(min);
        max = RefALU::flush<dT>(max);

        if(Utils::IsNInf(ref)) {
            if(!test.IsAcc())
                return false;
            if(! Utils::IsNInf(*test.GetAcc<dT>()))
                return false;
        } else if (Utils::IsPInf(ref)) {
            if(!test.IsAcc())
                return false;
            if(! Utils::IsPInf(*test.GetAcc<dT>()))
                return false;
        } else if((ref == 0) && RefALU::GetFTZmode()) {
            // one ulp for zero is denormal in float point precision
            // so, flushed min and max should be zero
            if( min != 0 || max != 0)
                return false;
        } else {
            T oneUlp = NEATALU::ComputeUlp(ref4ulps);

            T  refMinHigh = ref-T(ulps)*T(oneUlp);
            T  refMaxHigh = ref+T(ulps)*T(oneUlp);
            dT refMin = NEATALU::castDown(refMinHigh);
            dT refMax = NEATALU::castDown(refMaxHigh);

            res = true;
            // if refMin is INF, min should be INF
            if(Utils::IsInf<dT>(refMin)) {
                res &= Utils::IsInf<dT>(min);
            } else {
            // if refMin is denormal and FTZ mode on, result should be zero
            // if refMin is zero and FTZ mode on, result should be zero
            if( (Utils::IsDenorm<dT>(refMin) && RefALU::GetFTZmode()) || (refMin==0 && RefALU::GetFTZmode()))
                res &= (min==0);
            // otherwise, refMin should be equal to result
            else
                res &= (T(fabs(refMin - min)) <= T(ulps)*oneUlp);
            }

            // if refMax is INF, max should be INF
            if(Utils::IsInf<dT>(refMax)) {
                res &= Utils::IsInf<dT>(max);
            } else {
            // if refMax is denormal and FTZ mode on, result should be zero
            // if refMax is zero and FTZ mode on, result should be zero
            if( (Utils::IsDenorm<dT>(refMax) && RefALU::GetFTZmode()) || (refMax==0 && RefALU::GetFTZmode()))
                res &= (max==0);
            else
            // otherwise, refMax should be equal to result
                res &= (T(fabs(refMax - max)) <= T(ulps)*oneUlp);
            }

        }
        return res;
    }


    template <typename T>
    static bool TestIntExpandedDotMix(T refMinIn, T refMaxIn, NEATValue test, T ref4ulps, float ulps)
    {
        typedef typename downT<T>::type dT;

        bool res = true;
        T refMin, refMax;

        if( refMinIn > refMaxIn)
        {
            refMin = refMaxIn;
            refMax = refMinIn;
        } else
        {
            refMin = refMinIn;
            refMax = refMaxIn;
        }

        dT min = *test.GetMin<dT>();
        dT max = *test.GetMax<dT>();

        min = RefALU::flush<dT>(min);
        max = RefALU::flush<dT>(max);

        if(Utils::IsNInf(refMin)) {
            if(! Utils::IsNInf(min))
                return false;
        } else if (Utils::IsPInf(refMin)) {
            if(! Utils::IsPInf(min))
                return false;
        } else if((refMin == 0) && RefALU::GetFTZmode() ) {
            // one ulp for zero is denormal in float point precision
            // so, flushed min and max should be zero
            if( min != 0)
                return false;
        } else {
            T oneUlp = NEATALU::ComputeUlp(ref4ulps);
            T  refMinHigh = refMin-T(ulps*oneUlp);
            dT refMinUlps = NEATALU::castDown(refMinHigh);

            res = true;
            // if refMinUlps is INF, min should be INF
            if(Utils::IsInf<dT>(refMinUlps)) {
                res &= Utils::IsInf<dT>(min);
            } else if((Utils::IsDenorm<dT>(refMinUlps) && RefALU::GetFTZmode()) || (refMinUlps==0 && RefALU::GetFTZmode()))
            // if refMin is denormal and FTZ mode on, result should be zero
            // if refMin is zero and FTZ mode on, result should be zero
            {
                res &= (min==0);
            } else {
            // otherwise, refMinUlps should be equal to result
                res &= (T(fabs(refMinUlps - min)) <= T(ulps)*oneUlp);
            }
        }

        if(Utils::IsNInf(refMax)) {
            if(! Utils::IsNInf(max))
                return false;
        } else if (Utils::IsPInf(refMax)) {
            if(! Utils::IsPInf(max))
                return false;
        } else if((refMax == 0) && RefALU::GetFTZmode()) {
            // one ulp for zero is denormal in float point precision
            // so, flushed max and max should be zero
            if( max != 0)
                return false;
        } else {
            T oneUlp = NEATALU::ComputeUlp(ref4ulps);
            T  refMaxHigh = refMax+T(ulps*oneUlp);
            dT refMaxUlps = NEATALU::castDown(refMaxHigh);

            res = true;
            // if refMaxUlps is INF, max should be INF
            if(Utils::IsInf<dT>(refMaxUlps)) {
                res &= Utils::IsInf<dT>(max);
            } else if((Utils::IsDenorm<dT>(refMaxUlps) && RefALU::GetFTZmode()) || (refMaxUlps==0 && RefALU::GetFTZmode()))
            // if refMax is denormal and FTZ mode on, result should be zero
            // if refMax is zero and FTZ mode on, result should be zero
            {
                res &= (max==0);
            } else {
            // otherwise, refMaxulps should be equal to result
                res &= (T(fabs(refMaxUlps - max)) <= T(ulps)*oneUlp);
            }
        }
        return res;
    }


    // generate num (from 1 to numMax=500) random values for a, a and b or a,b and c 
    // in the ranges [aMin, aMax], [bMin, bMax] and [cMix,cMax] correspondly. 
    // All result values, calculated by reference function, should be 
    // between low and high limits of interval of testVal
    template <typename T>
    class TestIntervalRandomly {
    private:
           DataTypeVal dataTypeVal;
           bool test;
           static const uint32_t numMax = 500;
           T arrA[numMax], arrB[numMax], arrC[numMax];
    public:
           typedef T (*RefFuncPOneArg)(const T&);
           typedef T (*RefFuncPTwoArgs)(const T&, const T&);
           typedef T (*RefFuncPThreeArgs)(const T&, const T&, const T&);

           TestIntervalRandomly(RefFuncPOneArg RefFunc, NEATValue testVal, T aMin, T aMax, const uint32_t num) {
               assert(num <= numMax);
               dataTypeVal = GetDataTypeVal<T>();
               GenerateRangedVectorsAutoSeed(dataTypeVal, &arrA[0], V1, num, aMin, aMax);
               test = true;
               for(uint32_t j = 0;j<num;j++) {
                   T refVal = RefFunc( RefALU::flush(arrA[j]) ); 
                   test &= (refVal >= *testVal.GetMin<T>() && refVal <= *testVal.GetMax<T>());
               }
           }
           TestIntervalRandomly(RefFuncPTwoArgs RefFunc,NEATValue testVal, T aMin, T aMax, T bMin, T bMax, const uint32_t num) {
               assert(num <= numMax);
               dataTypeVal = GetDataTypeVal<T>();
               GenerateRangedVectorsAutoSeed(dataTypeVal, &arrA[0], V1, num, aMin, aMax);
               GenerateRangedVectorsAutoSeed(dataTypeVal, &arrB[0], V1, num, bMin, bMax);
               test = true;
               for(uint32_t j = 0;j<num;j++) {
                   T refVal = RefFunc( RefALU::flush(arrA[j]), RefALU::flush(arrB[j]) ); 
                   test &= (refVal >= *testVal.GetMin<T>() && refVal <= *testVal.GetMax<T>());
               }
           }
           TestIntervalRandomly(RefFuncPThreeArgs RefFunc,NEATValue testVal, T aMin, T aMax, T bMin, T bMax, T cMin, T cMax, const uint32_t num) {
               assert(num <= numMax);
               dataTypeVal = GetDataTypeVal<T>();                  
               GenerateRangedVectorsAutoSeed(dataTypeVal, &arrA[0], V1, num, aMin, aMax);
               GenerateRangedVectorsAutoSeed(dataTypeVal, &arrB[0], V1, num, bMin, bMax);
               GenerateRangedVectorsAutoSeed(dataTypeVal, &arrC[0], V1, num, cMin, cMax);
               test = true;
               for(uint32_t j = 0;j<num;j++) {
                   T refVal = RefFunc( RefALU::flush(arrA[j]), RefALU::flush(arrB[j]), RefALU::flush(arrC[j]) ); 
                   test &= (refVal >= *testVal.GetMin<T>() && refVal <= *testVal.GetMax<T>());
               }
           }

           bool GetTestResult() {
               return test;
           }

    };


    // Tests special NEATValue for function with two arguments.
    // Result must be NEATValue with UNKNOWN status.
    template <typename T, NEATValue::Status st>
    bool TestSpecialNEATValue(NEATScalarBinaryOp f){
        bool result = true;
        NEATValue goodValue = NEATValue((T)0.1);
        NEATValue res = f(NEATValue(st), goodValue);
        result = result && res.IsUnknown();
        res = f(goodValue, NEATValue(st));
        result = result && res.IsUnknown();
        res = f(NEATValue(st), NEATValue(st));
        return result && res.IsUnknown();
    }

    // Tests special NEATValue for function with single argument.
    // Result must be NEATValue with UNKNOWN status.
    template <NEATValue::Status st>
    bool TestSpecialNEATValue(NEATScalarUnaryOp f){
        NEATValue res = f(NEATValue(st));
        return res.IsUnknown();
    }

    /// Tests that for binary function f and arguments arg1 and arg2
    /// NEAT returns accurate value that is equal to refOut
    template <typename T>
    bool TestPreciseRes(NEATScalarBinaryOp f, T arg1, T arg2, T refOut){
        bool passed = true;
        NEATValue val1 = NEATValue(arg1);
        NEATValue val2 = NEATValue(arg2);
        NEATValue res = f(val1, val2);
        if(Utils::IsNaN<T>(refOut) && (res.IsNaN<T>()))
            return true;
        passed &= res.IsAcc();
        passed &= Utils::eq(*res.GetAcc<T>(), refOut);
        return passed;
    }

    /// Tests that for unary function f and argument arg
    /// NEAT returns accurate value that is equal to refOut
    template <typename T>
    bool TestPreciseRes(NEATScalarUnaryOp f, T arg, T refOut){
        bool passed = true;
        NEATValue val = NEATValue(arg);
        NEATValue res = f(val);
        if(Utils::IsNaN<T>(refOut) && (res.IsNaN<T>()))
            return true;
        passed &= res.IsAcc();
        passed &= Utils::eq(*res.GetAcc<T>(), refOut);
        return passed;
    }

    template <typename T>
    bool TestUnknown(NEATScalarTernaryOp f){
        return TestSpecialNEATValue<T, NEATValue::UNKNOWN>(f);
    }

    template <typename T>
    bool TestUnknown(NEATScalarBinaryOp f){
        return TestSpecialNEATValue<T, NEATValue::UNKNOWN>(f);
    }
    
    bool TestUnknown(NEATScalarUnaryOp f);

    template <typename T>
    bool TestUnwritten(NEATScalarTernaryOp f){
        return TestSpecialNEATValue<T, NEATValue::UNWRITTEN>(f);
    }

    template <typename T>
    bool TestUnwritten(NEATScalarBinaryOp f){
        return TestSpecialNEATValue<T, NEATValue::UNWRITTEN>(f);
    }

    bool TestUnwritten(NEATScalarUnaryOp f);

    template <typename T>
    bool TestAny(NEATScalarTernaryOp f){
        return TestSpecialNEATValue<T, NEATValue::ANY>(f);
    }


    template <typename T>
    bool TestAny(NEATScalarBinaryOp f){
        return TestSpecialNEATValue<T, NEATValue::ANY>(f);
    }

    bool TestAny(NEATScalarUnaryOp f);

    // This method checks if 'long double' data type is larger in memory then 'double'.
    // If their sizes are equal, we can't to test 'double' version of built-in function because
    // reference value must be computed in higher precision.
    template <typename T>
    bool SkipDoubleTest(){
        bool LongDoubleSizeEqualsDoubleSize = false;
        if(sizeof(T) == sizeof(double)) {
            LongDoubleSizeEqualsDoubleSize = sizeof(double) == sizeof(long double);
        }

        if(LongDoubleSizeEqualsDoubleSize) {
            printf("WARNING: size of long double is equal to size of double\n");
            printf("WARNING: test for double will be skipped.\n");
        }
        return LongDoubleSizeEqualsDoubleSize;
    }

} // namespace Validation

#endif // #ifndef __NEATALUUTILS_H__
