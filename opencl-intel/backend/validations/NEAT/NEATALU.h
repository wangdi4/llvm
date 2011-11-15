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

File Name:  NEATALU.h


\*****************************************************************************/
#ifndef __NEATALU_H__
#define __NEATALU_H__

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "NEATValue.h"
#include "RefALU.h"
#include "NEATVector.h"
#include "FloatOperations.h"
#include "ImagesALU.h"
#include <math.h>
#include <vector>

namespace Validation
{
    enum CmpType {CMP_FALSE, CMP_OEQ, CMP_OGT, CMP_OGE, CMP_OLT, CMP_OLE, CMP_ONE, CMP_ORD,
                              CMP_UEQ, CMP_UGT, CMP_UGE, CMP_ULT, CMP_ULE, CMP_UNE, CMP_UNO, CMP_TRUE};

    template<typename T>
    class superT;

    template<>
    class superT<float>{
    public:
           typedef double type;
    };

    template<>
    class superT<double>{
    public:
           typedef long double type ;
    };


    template<typename T>
    class downT;

    template<>
    class downT<double>{
    public:
           typedef float type;
    };

    template<>
    class downT<long double>{
    public:
           typedef double type ;
    };

    // Wrapper class to specialize that float and double values are represented in NEAT by NEATValue.
    // The rest of the types are treated as they are.
    template<class T>
    class NEATType{
    public:
        typedef T type;
    };

    template<> class NEATType<float> {
    public:
        typedef NEATValue type;
    };

    template<> class NEATType<double>{
    public:
        typedef NEATValue type;
    };


    typedef NEATValue (*NEATScalarTernaryOp)(const NEATValue&, const NEATValue&, const NEATValue&);
    typedef NEATValue (*NEATScalarBinaryOp)(const NEATValue&, const NEATValue&);
    typedef NEATValue (*NEATScalarUnaryOp)(const NEATValue&);

    /// @brief Makes all the arithmetic operations for NEAT
    /// This class is similar to the ref ALU (refalu.cpp), only it works on NEATValues and NEATVectors
    /// The results of its operations can store either the range of allowed values or
    /// vector of value ranges.
    class NEATALU
    {
    public:

        /// @brief the maximal error (rel or abs) in various inaccurate operations
        static const int ADD_ERROR = 0;
        static const int SUB_ERROR = 0;
        static const int MUL_ERROR = 0;
        static const int FMOD_ERROR = 0;
        static const int SIN_ERROR = 4;
        static const int COS_ERROR = 4;
        static const int TAN_ERROR = 5;
        static const int EXP_ERROR = 3;
        static const int EXP2_ERROR = 3;
        static const int EXP10_ERROR = 3;
        static const int EXPM1_ERROR = 3;
        static const int LOG_ERROR = 3;
        static const int LOG2_ERROR = 3;
        static const int LOG10_ERROR = 3;
        static const int LOG1P_ERROR = 2;
        static const int LOGB_ERROR = 0;
        static const int FREXP_ERROR = 0;
        static const int LDEXP_ERROR = 0;
        static const int MODF_ERROR = 0;
        static const int ROOTN_ERROR = 16;
        static const int POW_ERROR = 16;
        static const int POWR_ERROR = 16;
        static const int POWN_ERROR = 16;
        static const int RSQRT_ERROR = 2;
        static const double DIV_ERROR; // = 2.5f
        static const double NORMALIZE_ERROR; // 2f + 0.5f  error in rsqrt + error in multiply
        static const int FAST_NORMALIZE_ERROR = 8192;
        static const double MIX_ERROR; // 1.5f
        static const int FABS_ERROR = 0;
        static const int FLOOR_ERROR = 0;
        static const int HYPOT_ERROR = 4;
        static const int RADIANS_ERROR = 2; //as like as in conformance
        static const int DEGREES_ERROR = 2; //as like as in conformance
        static const int ASINH_ERROR = 4;
        static const int ACOSH_ERROR = 4;
        static const int ATANH_ERROR = 5;
        static const int NEXTAFTER_ERROR = 0;
        static const int FMA_ERROR = 0;
        static const int FRACT_ERROR = 0;
        static const int FMAX_ERROR = 0;
        static const int SQRT_ERROR = 3;
        static const int CBRT_ERROR = 2;
        static const int COPYSIGN_ERROR = 0;
        static const int FDIM_ERROR = 0;
        static const int CROSS_ERROR = 3;

        // These values are provided by SVML team and used by back-end compiler.
        static const int NATIVE_RECIP_ERROR = 4990;
        static const int NATIVE_DIVIDE_ERROR = 16800000;
        static const int NATIVE_SIN_ERROR = 2230;
        static const int NATIVE_COS_ERROR = 2230;
        static const double NATIVE_TAN_ERROR; // = 2.16;
        static const int NATIVE_SQRT_ERROR = 3590;
        static const int NATIVE_RSQRT_ERROR = 3590;
        static const int NATIVE_EXP_ERROR = 1260;
        static const int NATIVE_EXP2_ERROR = 1230;
        static const int NATIVE_EXP10_ERROR = 1280;
        static const int NATIVE_LOG_ERROR = 7200;
        static const int NATIVE_LOG2_ERROR = 651;
        static const int NATIVE_LOG10_ERROR = 651;
        static const int NATIVE_POWR_ERROR = 1310;

        static const int HALF_EXP_ERROR = 8192;
        static const int HALF_EXP2_ERROR = 8192;
        static const int HALF_EXP10_ERROR = 8192;
        static const int HALF_LOG_ERROR = 8192;
        static const int HALF_LOG2_ERROR = 8192;
        static const int HALF_LOG10_ERROR = 8192;
        static const int HALF_RSQRT_ERROR = 8192;
        static const int HALF_SQRT_ERROR = 8192;
        static const int HALF_COS_ERROR = 8192;
        static const int HALF_SIN_ERROR = 8192;
        static const int HALF_TAN_ERROR = 8192;
        static const int HALF_POWR_ERROR = 8192;
        static const int HALF_DIVIDE_ERROR = 8192;
        static const int HALF_RECIP_ERROR = 8192;

        static const long double pi;
        static const long double two_pi;
        static const long double pi_2;
        static const long double div180by_pi;

        /// @brief default ctor
        NEATALU();

    private:
        /// @brief Goes through input vector vec and for each
        /// element of vector it calls NEATValueFunction f, that works with pair of NEAT values.
        /// @param [in]     val    NEATValue to take as first parameter of NEATValueFunction
        /// @param [in]     vec    Vector of elements to take as second parameter of NEATValueFunction
        /// @param [in]     NEATScalarBinaryOp  Binary operation function pointer that takes two NEAT values as an input
        /// @return         Resulting vector with the same width as for vec
        /// @throws InvalidArgument
        static NEATVector processVector(const NEATValue& val, const NEATVector& vec, NEATScalarBinaryOp f);

        /// @brief Goes through input vectors vec1, vec2 and vec3 for each
        /// three elements it calls NEATValueFunction f, that works with three NEAT values.
        /// @param [in]     vec1    Vector of elements to take as first parameter of NEATValueFunction
        /// @param [in]     vec2    Vector of elements to take as second parameter of NEATValueFunction
        /// @param [in]     vec3    Vector of elements to take as third parameter of NEATValueFunction
        /// @param [in]     NEATScalarTernaryOp  Ternary operation function pointer that takes three NEAT values as an input
        /// @return         Resulting vector with the same width as for vec1, vec2 and vec3
        /// @throws InvalidArgument
        static NEATVector processVector(const NEATVector& vec1, const NEATVector& vec2, const NEATVector& vec3, NEATScalarTernaryOp f);

        /// @brief Goes through input vector vec1 and vec2 and for each
        /// pair of elements it calls NEATValueFunction f, that works with pair of NEAT values.
        /// @param [in]     vec1    Vector of elements to take as first parameter of NEATValueFunction
        /// @param [in]     vec2    Vector of elements to take as second parameter of NEATValueFunction
        /// @param [in]     val     NEATValue to take as third parameter of NEATValueFunction
        /// @param [in]     NEATScalarTernaryOp  Ternary operation function pointer that takes three NEAT values as an input
        /// @return         Resulting vector with the same width as for vec1 and vec2
        /// @throws InvalidArgument
        static NEATVector processVector(const NEATVector& vec1, const NEATVector& vec2, const NEATValue& val, NEATScalarTernaryOp f);

        /// @brief Goes through input vector vec1 for each element
        /// of vec1 it calls NEATValueFunction f,
        /// with vec1[i], val2 and val3 arguments.
        /// @param [in]     vec1    Vector of elements to take as first parameter of NEATValueFunction
        /// @param [in]     val2    Value to take as second parameter of NEATValueFunction
        /// @param [in]     val3    Value to take as third parameter of NEATValueFunction
        /// @param [in]     NEATScalarTernaryOp  Ternary operation function pointer that takes two NEAT values as an input
        /// @return         Resulting vector with the same width as for vec1
        /// @throws InvalidArgument
        static NEATVector processVector(const NEATVector& vec1, const NEATValue& val2, const NEATValue& val3, NEATScalarTernaryOp f);

        /// @brief Goes through input vector vec and for each
        /// element of vector it calls NEATValueFunction f, that works with pair of NEAT values.
        /// @param [in]     vec    Vector of elements to take as second parameter of NEATValueFunction
        /// @param [in]     val    NEATValue to take as first parameter of NEATValueFunction
        /// @param [in]     NEATScalarBinaryOp  Binary operation function pointer that takes two NEAT values as an input
        /// @return         Resulting vector with the same width as for vec
        /// @throws InvalidArgument
        static NEATVector processVector(const NEATVector& vec, const NEATValue& val, NEATScalarBinaryOp f);

        /// @brief Goes through input vector vec1 and vec2 and for each
        /// pair of elements it calls NEATValueFunction f, that works with pair of NEAT values.
        /// @param [in]     vec1    Vector of elements to take as first parameter of NEATValueFunction
        /// @param [in]     vec2    Vector of elements to take as second parameter of NEATValueFunction
        /// @param [in]     NEATScalarBinaryOp  Binary operation function pointer that takes two NEAT values as an input
        /// @return         Resulting vector with the same width as for vec1 and vec2
        /// @throws InvalidArgument
        static NEATVector processVector(const NEATVector& vec1, const NEATVector& vec2, NEATScalarBinaryOp f);

        /// @brief Goes through input vector vec and calls for each
        /// element NEATValueFunction f, that works with single NEAT value.
        /// @param [in]     vec    Vector of elements to take as parameter of NEATValueFunction
        /// @param [in]     NEATScalarUnaryOp  Unary operation function pointer that takes
        /// single NEAT value as an input
        /// @return         Resulting vector with the same width as for vec
        static NEATVector processVector(const NEATVector& vec, NEATScalarUnaryOp f);

        /// @brief Combines number of values to one interval.
        /// Handles corner cases and choses maximal and minimal values in an array.
        /// Output interval is constructed as minimal to maximal range of values.
        /// @param [in]     vals    Contains values that should be combined to one interval
        /// @param [in]     num     Number of values in val
        /// @param [inout]  minOut  Pointer to write output minimum
        /// @param [inout]  maxOut  Pointer to write output maximum
        template<typename T>
        static NEATValue::Status Combine(T vals[], size_t num, T& minOut, T& maxOut)
        {
            // Count edge cases
            uint32_t numPInf = 0;
            uint32_t numNInf = 0;
            uint32_t numNaN = 0;
            for (uint32_t i = 0; i < num; i ++)
            {
                numPInf += Utils::IsPInf(vals[i]);
                numNInf += Utils::IsNInf(vals[i]);
                numNaN  += Utils::IsNaN(vals[i]);
            }

            // All values are possible.
            if((numNaN > 0) && (numPInf > 0) && (numNInf > 0))
            {
                return NEATValue::ANY;
            }

            // all values are equal (pInf, mInf or NaN)
            if (numNaN == num || numNInf == num || numPInf == num)
            {
                minOut = vals[0];
                maxOut = vals[0];
                return NEATValue::ACCURATE;
            }

            // We cannot represent this values by single interval
            if(numNaN>0)
            {
                return NEATValue::UNKNOWN;
            }

            // Now we need to find the smallest value and set it as min
            // and take the biggest value and set it as max
            // we want -0.0 to be smaller than +0.0, so we use our internal methods (and not the ALU's methods)
            T min = vals[0];
            T max = vals[0];

            for (uint32_t i = 1; i < num; i++)
            {
                if ( Utils::lt(vals[i], min) )
                    min = vals[i];
                if ( Utils::lt(max, vals[i]) )
                    max = vals[i];
            }

            minOut = min;
            maxOut = max;
            return NEATValue::INTERVAL;
        }


        static double roundingUlps(double ulps) {
            return (ulps - 0.5);
        }

        static bool IsCmpOrdered(CmpType type)
        {
            if((type == CMP_OEQ) || (type == CMP_OGE) || (type == CMP_OGT) || (type == CMP_OLE) || (type == CMP_OLT) || (type == CMP_ONE) || (type == CMP_ORD))
                return true;
            else
                return false;
        }
        template<typename T>
        static NEATValue lt(const NEATValue& a, const NEATValue& b)
        {
            bool res = false;
            // We can return answer in two cases:
            // a<b(res = true):     ---------|---------------|-------|--------------|----
            //                             a.min            a.max   b.min          b.max
            //
            // a<=b(res = false):    ---------|---------------|-------|--------------|----
            //                             b.min            b.max   a.min          a.max
            //
            if(RefALU::lt(*a.GetMax<T>(), *b.GetMin<T>()))
                res = true;
            else if(RefALU::le(*b.GetMax<T>(), *a.GetMin<T>()))
                res = false;
            else
                return NEATValue(NEATValue::UNKNOWN);
            return NEATValue(res);
        }

        template<typename T>
        static NEATValue eq(const NEATValue& a, const NEATValue& b)
        {
            //We can return answer in two cases:
            // a<b(res = true):     ---------|---------------|-------|--------------|----
            //                             a.min            a.max   b.min          b.max
            //
            // a>b(res = false):    ---------|---------------|-------|--------------|----
            //                             b.min            b.max   a.min          a.max
            //

            if((a.GetStatus() == NEATValue::ACCURATE) && (b.GetStatus() == NEATValue::ACCURATE) &&
                (*a.GetAcc<T>() == *b.GetAcc<T>()))
            {
                return NEATValue(true);
            }
            else if(RefALU::lt(*a.GetMax<T>(), *b.GetMin<T>()) || RefALU::lt(*b.GetMax<T>(), *a.GetMin<T>()))
            {
               return  NEATValue(false);
            }
            else
            {
                return NEATValue(NEATValue::UNKNOWN);
            }
        }

        template<typename T>
        static NEATValue ne(const NEATValue& a, const NEATValue& b)
        {
            NEATValue eqVal = eq<T>(a,b);
            if(eqVal.GetStatus() == NEATValue::UNKNOWN)
                return NEATValue(NEATValue::UNKNOWN);
            else
            {
                bool eqBool = *eqVal.GetAcc<bool>();
                return NEATValue(!eqBool);
            }
        }

        template<typename T>
        static NEATValue gt(const NEATValue& a, const NEATValue& b)
        {
            return lt<T>(b, a);
        }

        template<typename T>
        static NEATValue le(const NEATValue& a, const NEATValue& b)
        {
            bool res;
            //We can return answer in two cases:
            // a<=b(res = true):   ---------|---------------|-------|--------------|----
            //                             a.min            a.max   b.min          b.max
            //
            // a<b(res = false):    ---------|---------------|-------|--------------|----
            //                             b.min            b.max   a.min          a.max
            //
            if(RefALU::le(*a.GetMax<T>(), *b.GetMin<T>()))
                res = true;
            else if(RefALU::lt(*b.GetMax<T>(), *a.GetMin<T>()))
                res = false;
            else
                return NEATValue(NEATValue::UNKNOWN);
            return NEATValue(res);
        }

        template<typename T>
        static NEATValue ge(const NEATValue& a, const NEATValue& b)
        {
            return le<T>(b,a);
        }

        template<typename T>
        static bool PowEdgePowerAcc(const T& xMin, const T& xMax, const T& y, NEATValue& res)
        {
            if(Utils::IsNInf(y))
            {
                /// [C99] 9
                // pow(x, -inf) = +inf for |x| < 1
                if((xMin > -1.0) && (xMax < 1.0))
                    res = NEATValue(Utils::GetPInf<T>());
                /// [C99] 10
                // pow(x, -inf) = + 0.0 for |x| > 1
                else if((xMin > 1.0) || (xMax < 1.0))
                    res = NEATValue((T) +0.0);
                else
                    res = NEATValue(NEATValue::UNKNOWN);
                return true;
            }
            if(Utils::IsPInf(y))
            {
                /// [C99] 11
                // pow(x, +inf) = +0.0 for |x| < 1
                if((xMin > -1.0) && (xMax < 1.0))
                    res = NEATValue((T) +0.0);
                /// [C99] 12
                // pow(x, -inf) = + inf for |x| > 1
                else if((xMin > 1.0) || (xMax < 1.0))
                    res = NEATValue((T) Utils::GetPInf<T>());
                else
                    res = NEATValue(NEATValue::UNKNOWN);
                return true;
            }
            return false;
        }

        template<typename T>
        static bool PowEdgeBaseAcc(const T& x, const T& yMin, const T& yMax, NEATValue& res)
        {
            /// C99
            if(Utils::eq(yMin, yMax))
            {
                T y = yMin;
                /// pow(+-0, y)
                if(Utils::eq<T>(x, 0.0) || Utils::eq<T>(x, -0.0))
                {
                    T sign = 1.0;
                    if(Utils::lt<T>(x, 0.0))
                        sign = -1.0;
                    /// integer cases
                    if(y < 0.0)
                    {
                        /// [C99] 1
                        if(Utils::IsOdd(y))
                        {
                            res = NEATValue(Utils::GetPInf<T>() * sign);
                            return true;
                        }else
                        {
                            res = NEATValue(Utils::GetPInf<T>());/// y is not an odd integer
                            return true;
                        }
                    }
                    if(Utils::gt<T>(y, 0))
                    {
                        /// [C99] 3
                        if(Utils::IsOdd(y))
                        {
                            if(sign > 0)
                                res = NEATValue(T(0.0));
                            else
                                res = NEATValue(T(-0.0));
                            return true;
                        }
                        else /// [C99] 4
                        {
                            res = NEATValue(T(0.0));
                            return true;
                        }
                    }
                } // pow(+-0, y)

                /// [C99] 5
                // pow(-1, +-inf) = 1
                if(( x == -1.0) && Utils::IsInf(y))
                {
                    res = NEATValue((T)1.0);
                    return true;
                }

                // pow(- inf, y)
                if(Utils::IsNInf<T>(x))
                {
                    if(Utils::lt<T>(y, 0))
                    {
                        /// [C99] 13
                        if(Utils::IsOdd(y))
                        {
                            res = NEATValue((T)-0.0);
                            return true;
                        }
                        else/// [C99] 14
                        {
                            res = NEATValue((T)+0.0);
                            return true;
                        }
                    }
                    if(Utils::gt<T>(y, 0))
                    {
                        /// [C99] 15
                        if(Utils::IsOdd(y))
                        {
                            res = NEATValue(Utils::GetNInf<T>());
                            return true;
                        }
                        else/// [C99] 16
                        {
                            res = NEATValue(Utils::GetPInf<T>());
                            return true;
                        }
                    }
                }
            } /// if y is accurate

            // pow(+inf, y)
            if(Utils::IsPInf<T>(x))
            {
                if(Utils::lt<T>(yMax, 0.0)) /// [C99] 17
                {
                    res = NEATValue((T)+0.0);
                    return true;
                }else if(Utils::gt<T>(yMin, 0.0))/// [C99] 18
                {
                    res = NEATValue((T)Utils::GetPInf<T>());
                    return true;
                }
                else
                {
                    res = NEATValue(NEATValue::UNKNOWN);
                    return true;
                }
            }
            return false;
        }

        /// used in power function
        /// Checks if input for power function belongs to one of edge cases
        template<typename T>
        static bool PowEdgeCases(const T& xMin, const T& xMax, const T& yMin, const T& yMax, NEATValue& res)
        {
            /// x is accurate
            if(Utils::eq(xMin, xMax))
            {
                T xAcc = xMin;
                if(PowEdgeBaseAcc(xAcc, yMin, yMax, res))
                    return true;
            } // x is accurate

            /// y is accurate
            if(Utils::eq(yMin, yMax))
            {
                T yAcc = yMin;
                if(PowEdgePowerAcc(xMin, xMax, yAcc, res))
                    return true;
            } // y is accurate

            /// [C99] 8
            /// pow(x<0, finite non-integer y) = NaN
            if(Utils::lt<T>(xMax, 0))
            {
                /// for finite y values
                if((!Utils::IsNInf(yMin)) && (!Utils::IsPInf(yMax)))
                {
                    // if interval contains integer
                    if(ContainsInteger(yMin, yMax))
                    {
                        // if integer contains integers and non-integer values
                        if(!Utils::eq(yMin, yMax))
                        {
                            // result is unknown (both NaN and non-NaN results are possible)
                            res = NEATValue(NEATValue::UNKNOWN);
                            return true;
                        }
                    }
                    else
                    {
                        // if there is no integers in the interval, then answer is NaN
                        res = NEATValue::NaN<T>();
                        return true;
                    }
                }
            } // x < 0
            return false;
        }
public:

    static double castDown(long double a)
    {
        return (double)(a);
    }
    static float castDown(double a)
    {
        return (float)(a);
    }
    static CFloat16 castDown(float a)
    {
        return (CFloat16)(a);
    }


    /// @brief Converts integer value of type IntT to NEATValue that contains converted
    /// accurate value of type FloatT.
    /// @param  [in]    intVal  Integer that should be converted to floating point value.
    /// @return         NEATValue that contains converted floating point value.
    template<typename FloatT, typename IntT>
    static NEATValue ToFloat(IntT intVal)
    {
        IsFloatType<FloatT> _notUsed;
        UNUSED_ARGUMENT(_notUsed);
        IsIntegerType<IntT> __notUsed;
        UNUSED_ARGUMENT(__notUsed);
        NEATValue toReturn((FloatT)intVal);
        return toReturn;
    }

    template<typename FloatT, typename IntT>
    static NEATVector ToFloat(std::vector<IntT> intVals)
    {
        IsFloatType<FloatT> _notUsed;
        UNUSED_ARGUMENT(_notUsed);
        IsIntegerType<IntT> __notUsed;
        UNUSED_ARGUMENT(__notUsed);
        const VectorWidth vWidths[MAX_VECTOR_WIDTH + 1] = {INVALID_WIDTH,V1,V2,V3,V4,INVALID_WIDTH,INVALID_WIDTH,INVALID_WIDTH,
                                   V8,INVALID_WIDTH,INVALID_WIDTH,INVALID_WIDTH,INVALID_WIDTH,INVALID_WIDTH,INVALID_WIDTH,INVALID_WIDTH,V16};
        VectorWidth width = vWidths[intVals.size()];
        if(width == INVALID_WIDTH)
        {
            throw Exception::InvalidArgument("Specified vector size is invalid.");
        }
        NEATVector res(width);
        for(size_t i = 0; i < intVals.size(); i++)
        {
            NEATValue val = ToFloat<FloatT, IntT>(intVals[i]);
            res[i] = val;
        }
        return res;
    }

    static double ComputeUlp(double ref) {
        double oneUlp = 0.;

        if(Utils::lt<double>(::fabs(ref), ::ldexp(1.0,-126))) // ref < 2**(-126)
        {
            oneUlp = ::ldexp(1.0,-149); // oneUlp = 2**(-126-23)
        } else {
            double c =
                ::floor(::log10(::fabs(ref))/::log10(2.0))
                - 23.0;
            oneUlp = ::ldexp(1.0,int(c));
        }
        return oneUlp;
    }

    static long double ComputeUlp(long double x) {

        if( sizeof( double) >= sizeof( long double ) )
        {
            printf( "This architecture needs something higher precision than double\
                    to check the precision of double.\nTest FAILED.\n" );
            abort();
        }

        long double oneUlp = 0.0L;

        if(Utils::lt<double>(::fabs(x), ::ldexp(1.0L,-1022))) // x < 2**(-1022)
        {
            oneUlp = ::ldexp(1.0L,-1074); // oneUlp = 2**(-1022-52)
        } else {
            // oneUlp = 2**(floor(log2(abs(x)))-52)
            double c = ::floor(::log10(::fabs(x))/::log10(2.0L)) - 52.0L;
            //oneUlp = pow(2, c);
            oneUlp = ::ldexp(1.0L,int(c));
        }
        return oneUlp;
    }

    template<typename T>
    static void ExpandFloatInterval(T * minInOut, T * maxInOut, double ulps)
    {
        if (ulps <= 0.)
            return;

        typedef typename downT<T>::type DownT;
        int n;

        T refMax = *maxInOut;
        T refMin = *minInOut;
        T ulpMax = ComputeUlp(refMax); // calc ulp for ref value
        T ulpMin = ComputeUlp(refMin); // calc ulp for ref value

        // high limit expand
        T result = ::frexp (refMax , &n);

        // check if refMax on the boundary of higher exponent
        if(::fabs(result) == 0.5)
        {
            //ulpMax = ComputeUlp(refMax+ulps*ulpMax); // calc ulp for ref value
            ulpMax /= 2.;
        }
        
        T resMax = refMax+ulpMax*((T)ulps); // add ulps to ref
        
        DownT maxD = (DownT)resMax; // downcast to lower precision
        
        T maxS = T(maxD); // conversion to higher precision
        
        if(maxS > resMax)
        {
            T lowUlp = ComputeUlp(maxS);
            maxS -= lowUlp;
        }
        *maxInOut = maxS;
       
        // low limit expand
        result = ::frexp (refMin , &n);

        // check if refMin on the boundary of higher exponent
        if(::fabs(result) == 0.5)
        {
             //ulpMin = ComputeUlp(refMin-ulps*ulpMin); // calc ulp for ref value
             ulpMin /= 2.;
        }
        
        T resMin = refMin-ulpMin*((T)(ulps)); // add ulps to ref
        DownT minD = (DownT)resMin; // downcast to lower precision
        
        T minS = T(minD); // conversion to higher precision
        
        if(minS < resMin)
        {
            // if we are, reduce result by one ulp, calculated for
            // downcasted value
            T lowUlp = ComputeUlp(minS);
            minS += lowUlp;        
        }
        *minInOut = minS;
    }


    // this function uses refMax and refMin different from maxInOut and minInOut
    // examples of using: dot, mix
    template<typename T>
    static void ExpandFloatInterval(T * minInOut, T * maxInOut, T ref4Ulps, double ulps)
    {
        if (ulps <= 0.)
            return;

        typedef typename downT<T>::type DownT;
        int n;

        T refMax = *maxInOut;
        T refMin = *minInOut;

        T ulpMax = ComputeUlp(ref4Ulps); // calc ulp for ref value
        T ulpMin = ComputeUlp(ref4Ulps); // calc ulp for ref value

        // high limit expand
        T result = ::frexp (refMax , &n);

        // check if refMax on the boundary of higher exponent
        if(::fabs(result) == 0.5)
        {
            //ulpMax = ComputeUlp(refMax+ulps*ulpMax); // calc ulp for ref value
            ulpMax /= 2.;
        }
        
        T resMax = refMax+ulpMax*((T)ulps); // add ulps to ref
        
        DownT maxD = (DownT)resMax; // downcast to lower precision
        
        T maxS = T(maxD); // conversion to higher precision
        
        if(maxS > resMax)
        {
            T lowUlp = ComputeUlp(maxS);
            maxS -= lowUlp;
        }

        *maxInOut = maxS;
       
        // low limit expand
        result = ::frexp (refMin , &n);

        // check if refMin on the boundary of higher exponent
        if(::fabs(result) == 0.5)
        {
             //ulpMin = ComputeUlp(refMin-ulps*ulpMin); // calc ulp for ref value
             ulpMin /= 2.;
        }
        
        T resMin = refMin-ulpMin*((T)(ulps)); // add ulps to ref
        DownT minD = (DownT)resMin; // downcast to lower precision
        
        T minS = T(minD); // conversion to higher precision
        
        if(minS < resMin)
        {
            // if we are, reduce result by one ulp, calculated for
            // downcasted value     
            T lowUlp = ComputeUlp(minS);
            minS += lowUlp;   
        }
        *minInOut = minS;
    }

    template<typename T>
    static NEATValue ComputeResult(T vals[], const int valsCount, double ulps)
    {
        typedef typename  downT<T>::type DownT;
        T min=T(0), max=T(0);
        NEATValue res;

        res.SetStatus(Combine(vals, valsCount, min, max));

        if(res.GetStatus() == NEATValue::INTERVAL ||
           res.GetStatus() == NEATValue::ACCURATE)
        {
            ExpandFloatInterval<T>(&min, &max, ulps);
            DownT minD = castDown(min);
            DownT maxD = castDown(max);

            minD = RefALU::flush<DownT>(minD);
            maxD = RefALU::flush<DownT>(maxD);

            res = NEATValue(minD, maxD);
        }

        return res;
    }

    // check if "a" is ANY or UNWRITTEN or UNKNOWN
    static bool CheckAUU(const NEATValue& a)
    {
        if(a.IsAny()) {
            return true;
        }
        if(a.IsUnwritten()) {
            return true;
        }
        if(a.IsUnknown()) {
            return true;
        }
        return false;
    }

    template <typename T>
    static NEATValue add ( const NEATValue& a, const NEATValue& b )
    {
        typedef typename superT<T>::type SuperT;
        // Check if both arguments may have any value
        if(a.IsAny() && b.IsAny())
            // Then result is any also
            return NEATValue(NEATValue::ANY);

        if (a.IsNaN<T>() || b.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Case when both could be any was tested. Other special statuses lead to unknown result
        if(CheckAUU(a) || (CheckAUU(b)))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        // There is no additions to C99 in OpenCL specification concerning add
        // Therefore we can suppose that C++ add handles infinities and NaNs correctly
        // No need in high precision also as add result is correctly rounded
        SuperT min = (SuperT)RefALU::add( *a.GetMin<T>(), *b.GetMin<T>() );
        SuperT max = (SuperT)RefALU::add( *a.GetMax<T>(), *b.GetMax<T>() );

        NEATValue toReturn;
        ExpandFloatInterval( &min, &max, ADD_ERROR);
        toReturn = NEATValue((T)min, (T)max);

        return toReturn;
    }

    template <typename T>
    static NEATValue mul ( const NEATValue& a, const NEATValue& b )
    {
        typedef typename superT<T>::type SuperT;
        // Check if both arguments may have any value
        if(a.IsAny() && b.IsAny())
            // Then result is any also
            return NEATValue(NEATValue::ANY);

        if (a.IsNaN<T>() || b.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Case when both could be any was tested. Other special statuses lead to unknown result
        if(CheckAUU(a) || (CheckAUU(b)))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        // Assume that compiler is C99 compliant and edge cases will be handled correctly

        SuperT vals[4];

        // calculating all the possible combinations
        vals[0] = RefALU::mul((SuperT)*a.GetMin<T>(), (SuperT)*b.GetMax<T>());
        vals[1] = RefALU::mul((SuperT)*a.GetMin<T>(), (SuperT)*b.GetMin<T>());
        vals[2] = RefALU::mul((SuperT)*a.GetMax<T>(), (SuperT)*b.GetMin<T>());
        vals[3] = RefALU::mul((SuperT)*a.GetMax<T>(), (SuperT)*b.GetMax<T>());

        return ComputeResult(vals, 4, NEATALU::ADD_ERROR);
    }

    /// @brief Divides two neat values
    /// @param [in] dividend
    /// @param [in] divisor
    template <typename T>
    static NEATValue div ( const NEATValue& a, const NEATValue& b )
    {
        return InternalDiv<T>(a, b, NEATALU::DIV_ERROR);
    }

    /// @brief Divides two neat values
    /// @param [in] dividend
    /// @param [in] divisor
    template <typename T>
    static NEATValue native_divide ( const NEATValue& a, const NEATValue& b )
    {
        return InternalDiv<T>(a, b, double(NEATALU::NATIVE_DIVIDE_ERROR));
    }

    /// @brief Divides two neat values
    /// @param [in] dividend
    /// @param [in] divisor
    template<typename T>
    static NEATVector native_divide(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVector(vec1, vec2, native_divide<T>);
    }

    template <typename T>
    static NEATValue half_divide ( const NEATValue& a, const NEATValue& b )
    {
        return InternalDiv<T>(a, b, double(NEATALU::HALF_DIVIDE_ERROR));
    }

    template<typename T>
    static NEATVector half_divide(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVector(vec1, vec2, half_divide<T>);
    }

    template <typename T>
    static NEATValue InternalRecip ( const NEATValue& a, const double& ulps)
    {
       typedef typename superT<T>::type SuperT;
        // Case when both could be any was tested. Other special statuses lead to unknown result
        if(CheckAUU(a))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // if a is not finite (i.e. NaN or Inf), the result is NaN
        if (!a.IsAcc() && (a.Includes((T)0.0) || a.Includes((T)(-0.0))) )
        {
            // if a (in 1/a) has more than one allowed value
            // and one of the values allowed is +0.0 or -0.0,
            // then we cannot predict the result (it may be finite and may be inf)
            return NEATValue(NEATValue::UNKNOWN);
        }

        const int RES_COUNT = 2;
        SuperT val[RES_COUNT];

        val[0] = RefALU::div((SuperT)1.0,(SuperT)*a.GetMin<T>());
        val[1] = RefALU::div((SuperT)1.0,(SuperT)*a.GetMax<T>());

        return ComputeResult(val, RES_COUNT, ulps);
    }

    template <typename T>
    static NEATValue native_recip ( const NEATValue& a )
    {
        return InternalRecip<T>(a, double(NEATALU::NATIVE_RECIP_ERROR)); 
    }

    template<typename T>
    static NEATVector native_recip(const NEATVector& vec)
    {
        return processVector(vec,native_recip<T>);
    }

    template <typename T>
    static NEATValue half_recip ( const NEATValue& a )
    {
        return InternalRecip<T>(a, double(NEATALU::HALF_RECIP_ERROR)); 
    }

    template<typename T>
    static NEATVector half_recip(const NEATVector& vec)
    {
        return processVector(vec,half_recip<T>);
    }

    template <typename T>
    static NEATValue neg (const NEATValue& a)
    {
        // Check argument may have any value
        if(a.IsAny())
            // Then result is any also
            return NEATValue(NEATValue::ANY);

        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Case when both could be any was tested. Other special statuses lead to unknown result
        if(CheckAUU(a))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        T min = RefALU::neg(*a.GetMax<T>());
        T max = RefALU::neg(*a.GetMin<T>());

        return NEATValue(min, max);
    }

    /// @brief calculates remainder of two number
    template <typename T>
    static NEATValue fmod( const NEATValue& dividend, const NEATValue& divisor)
    {
        typedef typename superT<T>::type SuperT;
        if(dividend.IsAny() && divisor.IsAny())
            return NEATValue(NEATValue::ANY);
        // if a or b is NaN, res is NaN
        if (dividend.IsNaN<T>() || divisor.IsNaN<T>())
            return NEATValue::NaN<T>();

        if(CheckAUU(dividend) || CheckAUU(divisor))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }
        // if a and b are not finite (i.e. NaN or Inf), the result is NaN
        if (!dividend.IsFinite<T>() && !divisor.IsFinite<T>())
            return NEATValue::NaN<T>();

        if (divisor.Includes((T)(0.0)) || divisor.Includes((T)(-0.0)) )
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        if(dividend.Includes((T)-0.0))
        {
            NEATValue positiveDividend((T)+0.0, *dividend.GetMax<T>());
            NEATValue negativeDividend((T)+0.0, -*dividend.GetMin<T>());
            NEATValue positiveRes = fmod<T>(positiveDividend, divisor);
            NEATValue negativeRes = fmod<T>(negativeDividend, divisor);
            T bounds[4] = {*positiveRes.GetMin<T>(), *positiveRes.GetMax<T>(), -*negativeRes.GetMin<T>(), -*negativeRes.GetMax<T>()};
            NEATValue preResult;
            T ansMin = (T)0.0, ansMax = (T)0.0;
            preResult.SetStatus(Combine(bounds, 4, ansMin, ansMax));
            if(preResult.GetStatus() == NEATValue::ACCURATE)
                preResult.SetAccurateVal(ansMin); // Doesn't really matter use min or max
            else if(preResult.GetStatus() == NEATValue::INTERVAL)
            {
                preResult.SetIntervalVal(ansMin, ansMax);
            }
            return preResult;
        }

        // Resulting value sign is determined by dividend sign
        // divisor sign can be ignored

        int sign = 1;
        T minDividend = (*dividend.GetMin<T>());
        T maxDividend = (*dividend.GetMax<T>());
        if(minDividend < 0)
        {
            T tmp = maxDividend;
            maxDividend = -minDividend;
            minDividend = -tmp;
            sign = -1;
        }
        T minDivisor = *divisor.GetMin<T>();
        T maxDivisor = *divisor.GetMax<T>();
        // check if divisor interval is negative. Different signs of interval boundaries were handled earlier
        if((minDivisor < 0) && (maxDivisor < 0))
        {
            T tmp = maxDivisor;
            maxDivisor = -minDivisor;
            minDivisor = -tmp;
        }

        // If length of dividend interval exceeds max divisor value then result is widest interval -
        // from zero to max divisor.
        if(maxDividend - minDividend >= maxDivisor)
        {
            if(sign == -1)
                return NEATALU::neg<T>(NEATValue((T)0.0, maxDivisor));
            else
                return NEATValue((T)0.0, maxDivisor);
        }

        // Now set Divisor to be maximal and calc boundary remainder values
        T minRem1 = RefALU::fmod(minDividend, maxDivisor);
        T maxRem1 = RefALU::fmod(maxDividend, maxDivisor);
        // Set Divisor to be minimal and calc boundary remainder values
        //T minRem2 = std::fmod(minDividend, minDivisor);
        T maxRem2 = RefALU::fmod(maxDividend, minDivisor);

        T maxDiv1 = ::floor(maxDividend / maxDivisor);
        T maxDiv2 = ::floor(maxDividend / minDivisor);

        NEATValue toReturn;
        toReturn.SetStatus(NEATValue::UNKNOWN);

        // if remainder diagram is a solid line
        if((minRem1 < maxRem1) || (minRem1 == maxRem1))
        {
            // Three cases are possible:
            // 1. "reminder loop" - answer is one interval from 0.0 to max value (not equal to maxDivisor)
            // 2. single interval - reminder value should fit an interval from minDiv1 to maxDiv2
            // 3. two intervals - for smaller minDivisor we have smaller reminder, but not greater than minDiv1.
            // Answer is unknown in this case.

            if( Utils::IsInf<T>(maxDiv1) && Utils::IsInf<T>(maxDiv2))
                toReturn = NEATValue(minRem1,  maxRem2);
            else if((maxDiv2 >= maxDiv1 + 2) ||
                ((maxDiv2 == maxDiv1 + 1) && (maxRem2 >= minRem1)))
            {
                if(maxDivisor>maxDividend)
                    toReturn = NEATValue((T)0.0, maxDividend);
                else
                // case 1
                    toReturn = NEATValue((T)0.0, maxDividend / maxDiv1);
            } else if((maxRem2 >= minRem1) || (maxRem2 == 0))
            {
                // case 2
                toReturn = NEATValue(minRem1,  maxRem2);
            }
        }
        // in this case remainder diagram has a break
        else
        {
            // Three cases are possible:
            // 1. "reminder loop" answer is one interval from 0.0 to maxDivisor
            // 2. two intervals - unknown value - maxDiv2 < minDiv1 and
            if((maxDiv2 >= maxDiv1 + 1) ||
                ((maxDiv1 == maxDiv2) && ((maxRem2 > minRem1) || (maxRem2 == 0))))
            {
                toReturn = NEATValue((T)0.0, maxDivisor);
            }
        }

        if(sign == -1)
            toReturn = NEATALU::neg<T>(toReturn);

        // two intervals case
        return toReturn;
    }


    template <typename T>
    static T round2nearestEven( const T& x)
    {
        if (x < T(0.0))
            return -round2nearestEven <T> ( -x );

        T sign = T(0.0);
        if (x < T(0.0))
            sign = T(-1.0);
        if (x > T(0.0))
            sign = T(1.0);

        T absRes = ::fabs(x);
        T floorRes  = ::floor(absRes);
        if (absRes - floorRes != 0.5) {
            // Return the integral value nearest to x rounding halfway cases away from zero
            return ::floor(absRes + T(0.5)) * sign;
        }

        if (::fmod(floorRes, T(2.0)) == 0)
        // if even
            return floorRes * sign;
        // if odd
        return ::ceil(absRes) * sign;

    }

    /// @brief calculates remainder of two number
    template <typename T>
    static NEATValue remainder( const NEATValue& x, const NEATValue& y)
    {
        typedef typename superT<T>::type SuperT;

        if(CheckAUU(x) || CheckAUU(y))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        if ( x.IsNaN<T>() || y.IsNaN<T>() || 
            (y.IsAcc() && (Utils::eq<T>(*y.GetAcc<T>(),T(0.0)) || Utils::eq<T>(*y.GetAcc<T>(),T(-0.0))) )
           )
            return NEATValue::NaN<T>();

        NEATValue dividend = flush<T>(x);
        NEATValue divisor = flush<T>(y);

        // if a and b are not finite (i.e. NaN or Inf), the result is NaN
        if (!dividend.IsFinite<T>() && !divisor.IsFinite<T>())
            return NEATValue::NaN<T>();

        if (divisor.Includes((T)(0.0)) || divisor.Includes((T)(-0.0)) )
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        if (divisor.IsAcc() && dividend.IsAcc())
            return NEATValue(RefALU::remainder<T>(*dividend.GetAcc<T>(), *divisor.GetAcc<T>()));

        if(dividend.Includes((T)-0.0))
        {
            NEATValue positiveDividend((T)+0.0, *dividend.GetMax<T>());
            NEATValue negativeDividend((T)+0.0, -*dividend.GetMin<T>());
            NEATValue positiveRes = remainder<T>(positiveDividend, divisor);
            NEATValue negativeRes = remainder<T>(negativeDividend, divisor);

            if( positiveRes.IsUnknown() || negativeRes.IsUnknown() ) {
                return NEATValue(NEATValue::UNKNOWN);
            } else {
                T bounds[4] = {*positiveRes.GetMin<T>(), *positiveRes.GetMax<T>(), -*negativeRes.GetMin<T>(), -*negativeRes.GetMax<T>()};
                NEATValue preResult;
                T ansMin = (T)0.0, ansMax = (T)0.0;
                preResult.SetStatus(Combine(bounds, 4, ansMin, ansMax));
                if(preResult.GetStatus() == NEATValue::ACCURATE)
                    preResult.SetAccurateVal(ansMin); // Doesn't really matter use min or max
                else if(preResult.GetStatus() == NEATValue::INTERVAL)
                {
                    preResult.SetIntervalVal(ansMin, ansMax);
                }
                return preResult;
            }
        }

        int sign = 1;
        T minDividend = (*dividend.GetMin<T>());
        T maxDividend = (*dividend.GetMax<T>());
        if(minDividend < 0)
        {
            T tmp = maxDividend;
            maxDividend = -minDividend;
            minDividend = -tmp;
            sign = -1;
        }

        T minDivisor = *divisor.GetMin<T>();
        T maxDivisor = *divisor.GetMax<T>();
        if((minDivisor < 0) && (maxDivisor < 0))
        {
            T tmp = maxDivisor;
            maxDivisor = -minDivisor;
            minDivisor = -tmp;
        }

        // If length of dividend interval exceeds max divisor value then result is widest interval -
        // from zero to max divisor.
        if(maxDividend - minDividend >= maxDivisor)
        {
            return NEATValue(-RefALU::fabs(maxDivisor/T(2.0)), RefALU::fabs(maxDivisor/T(2.0)));
        }

        // Now set Divisor to be maximal and calc boundary remainder values
        T minRem1 = RefALU::remainder(minDividend, maxDivisor);
        T maxRem1 = RefALU::remainder(maxDividend, maxDivisor);
        // Set Divisor to be minimal and calc boundary remainder values
        T maxRem2 = RefALU::remainder(maxDividend, minDivisor);

        T maxDiv1 = round2nearestEven(maxDividend / maxDivisor);
        T maxDiv2 = round2nearestEven(maxDividend / minDivisor);

        NEATValue toReturn;
        toReturn.SetStatus(NEATValue::UNKNOWN);

        // if remainder diagram is a solid line
        if((minRem1 < maxRem1) || (minRem1 == maxRem1))
        {
            // Three cases are possible:
            // 1. "reminder loop" - answer is one interval from 0.0 to max value (not equal to maxDivisor)
            // 2. single interval - reminder value should fit an interval from minDiv1 to maxDiv2
            // 3. two intervals - for smaller minDivisor we have smaller reminder, but not greater than minDiv1.
            // Answer is unknown in this case.

            if( Utils::IsInf<T>(maxDiv1) && Utils::IsInf<T>(maxDiv2))
                toReturn = NEATValue(minRem1,  maxRem2);
            else if((maxDiv2 >= maxDiv1 + 2) ||
                ((maxDiv2 == maxDiv1 + 1) && (maxRem2 >= minRem1)))
            {
                if(maxDivisor>maxDividend)
                    toReturn = NEATValue(-RefALU::fabs(maxDividend), RefALU::fabs(maxDividend));
                else 
                    // case 1
                    toReturn = NEATValue(-RefALU::fabs( (maxDividend / maxDiv1)/T(2.0)), RefALU::fabs( (maxDividend / maxDiv1)/T(2.0)));
            } else if((maxRem2 >= minRem1) || (maxRem2 == 0))
            {
                // case 2
                toReturn = NEATValue(minRem1,  maxRem2);
            }
        }
        // in this case remainder diagram has a break
        else
        {
            // Three cases are possible:
            // 1. "reminder loop" answer is one interval from 0.0 to maxDivisor
            // 2. two intervals - unknown value - maxDiv2 < minDiv1 and
            if((maxDiv2 >= maxDiv1 + 1) ||
                ((maxDiv1 == maxDiv2) && ((maxRem2 > minRem1) || (maxRem2 == 0))))
            {
                toReturn = NEATValue(-RefALU::fabs(maxDivisor/T(2.0)), RefALU::fabs(maxDivisor/T(2.0)));
            }
        }

        if(sign == -1)
            toReturn = NEATALU::neg<T>(toReturn);

        return toReturn;
    }

    template<typename T>
    static NEATValue remquo(const NEATValue& x, const NEATValue& y, int32_t *quo)
    {
        if(CheckAUU(x) || CheckAUU(y))
            return NEATValue(NEATValue::UNKNOWN);

        // according to the OpenCL Specification, Version: 1.1
        // 7.5.1 Additional Requirements Beyond C99 TC2
        if ( (x.IsAcc() && Utils::IsInf<T>(*x.GetAcc<T>()) ) ||
             (y.IsAcc() && (Utils::eq<T>(*y.GetAcc<T>(),T(0.0)) || Utils::eq<T>(*y.GetAcc<T>(),T(-0.0))) ) ||
             (x.IsNaN<T>() && y.IsNaN<T>())  ) 
        {
            *quo = 0;
            return NEATValue::NaN<T>();
        }

        if (x.IsAcc() && y.IsAcc()) {
            T res = RefALU::remquo<T>(*x.GetAcc<T>(), *y.GetAcc<T>(), quo);
            return NEATValue(res);
        } else {
            // quo is the lower seven bits of the integral quotient x/y,
            // we are not able to calculate this value for interval, so set it to 0
            *quo = 0;
            return remainder<T>(x, y);
        }
    }

    template<typename T>
    static NEATVector remquo(const NEATVector& vecX, const NEATVector& vecY, std::vector<int32_t>& quo)
    {
      NEATVector toReturn(vecX.GetWidth());
      size_t size = vecX.GetSize();
      quo.resize(size);
      for(size_t i = 0; i < size; ++i)
      {
          toReturn[i] = remquo<T>(vecX[i],vecY[i],(int32_t*)(&quo[i]));
      }
      return toReturn;
    }

    template<typename T>
    static NEATValue fcmp ( const NEATValue& a, const NEATValue& b, CmpType comparison)
    {
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a) || CheckAUU(b)) return NEATValue(NEATValue::UNKNOWN);

        bool someNaNs = a.IsNaN<T>() || b.IsNaN<T>();
        if(IsCmpOrdered(comparison))
        {
            if(someNaNs)
                return NEATValue(false);
        } else
        {
            if(someNaNs)
                return NEATValue(true);
        }
        NEATValue res = NEATValue(NEATValue::UNKNOWN);
        switch(comparison)
        {
        case CMP_FALSE:
            res = NEATValue(false);
            break;
        case CMP_TRUE:
            res = NEATValue(true);
            break;
        case CMP_OEQ: case CMP_UEQ:
            res = eq<T>(a,b);
            break;
        case CMP_OGT: case CMP_UGT:
            res = gt<T>(a,b);
            break;
        case CMP_OGE: case CMP_UGE:
            res = ge<T>(a,b);
            break;
        case CMP_OLE: case CMP_ULE:
            res = le<T>(a,b);
            break;
        case CMP_OLT: case CMP_ULT:
            res = lt<T>(a,b);
            break;
        case CMP_ONE: case CMP_UNE:
            res = ne<T>(a,b);
            break;
        case CMP_ORD:
            res = NEATValue(false);
            break;
        case CMP_UNO:
            res = NEATValue(true);
            break;
        }
        return res;
    }

    // names like IS#name# were choosen to avoid conflict with defines "isnan" and "isinf" included from conformance
    // and to keep names of relational functions looks similar
    template<typename T>
    static NEATValue ISequal ( const NEATValue& a, const NEATValue& b) {
        return fcmp<T>(a, b, CMP_OEQ);
    }
    template<typename T>
    static NEATValue ISnotequal ( const NEATValue& a, const NEATValue& b) {
        return fcmp<T>(a, b, CMP_ONE);
    }
    template<typename T>
    static NEATValue ISgreater ( const NEATValue& a, const NEATValue& b) {
        return fcmp<T>(a, b, CMP_OGT);
    }
    template<typename T>
    static NEATValue ISgreaterequal ( const NEATValue& a, const NEATValue& b) {
        return fcmp<T>(a, b, CMP_OGE);
    }
    template<typename T>
    static NEATValue ISless ( const NEATValue& a, const NEATValue& b) {
        return fcmp<T>(a, b, CMP_OLT);
    }
    template<typename T>
    static NEATValue ISlessequal ( const NEATValue& a, const NEATValue& b) {
        return fcmp<T>(a, b, CMP_OLE);
    }
    template<typename T>
    static NEATValue ISlessgreater ( const NEATValue& a, const NEATValue& b) {
        NEATValue res1 = fcmp<T>(a, b, CMP_OLT);
        NEATValue res2 = fcmp<T>(a, b, CMP_OGT);
        if(CheckAUU(res1) || CheckAUU(res1)) 
            return NEATValue(NEATValue::UNKNOWN);
        else {
            // it should not be interval here
            if( res1.IsInterval() || res2.IsInterval())
                throw Exception::NEATTrackFailure("[NEATALU::ISlessgreater] Only ACCURATE NEAT value is supported\n");
            else {
                return NEATValue(*res1.GetAcc<bool>() || *res2.GetAcc<bool>());
            }
        }
    }
    template<typename T>
    static NEATValue ISfinite ( const NEATValue& a) {
        if(CheckAUU(a)) 
            return NEATValue(NEATValue::UNKNOWN);
        if (a.IsFinite<T>()) 
            return NEATValue(true);
        else
            return NEATValue(false);
    }
    template<typename T>
    static NEATValue ISinf ( const NEATValue& a) {
        if(CheckAUU(a)) 
            return NEATValue(NEATValue::UNKNOWN);
        if(a.IsAcc()) {
            T tmp;
            memcpy(&tmp, a.GetAcc<T>(), sizeof(T));
            if(Utils::IsInf<T>(tmp))
                return NEATValue(true);
        }

        T min = *a.GetMin<T>();
        T max = *a.GetMax<T>();
        if(Utils::IsPInf(min) || Utils::IsPInf(max) || Utils::IsNInf(min) || Utils::IsNInf(max))
            return NEATValue(NEATValue::UNKNOWN);
        else
            return NEATValue(false);
    }
    template<typename T>
    static NEATValue ISnan ( const NEATValue& a) {
        if(CheckAUU(a)) 
            return NEATValue(NEATValue::UNKNOWN);
        if (a.IsNaN<T>()) 
            return NEATValue(true);
        else
            return NEATValue(false);
    }
    template<typename T>
    static NEATValue ISnormal ( const NEATValue& a) {
        if(CheckAUU(a)) 
            return NEATValue(NEATValue::UNKNOWN);
        T min = *a.GetMin<T>();
        T max = *a.GetMax<T>();
        if(RefALU::isNormal<T>(min) && RefALU::isNormal<T>(max) && RefALU::isNormal<T>(min) && RefALU::isNormal<T>(max))
            return NEATValue(true);
        else 
            return NEATValue(false);
    }
    template<typename T>
    static NEATValue ISordered ( const NEATValue& a, const NEATValue& b) {
        if(CheckAUU(a) || CheckAUU(b)) 
            return NEATValue(NEATValue::UNKNOWN);
        NEATValue res1 =  fcmp<T>(a, a, CMP_OEQ);
        NEATValue res2 =  fcmp<T>(b, b, CMP_OEQ);
        // it should not be interval here
        if( res1.IsInterval() || res2.IsInterval())
            throw Exception::NEATTrackFailure("[NEATALU::ISordered] Only ACCURATE NEAT value is supported\n");
        else {
            return NEATValue(*res1.GetAcc<bool>() || *res2.GetAcc<bool>());
        }
    }
    template<typename T>
    static NEATValue ISunordered ( const NEATValue& a, const NEATValue& b) {
        if(CheckAUU(a) || CheckAUU(b)) 
            return NEATValue(NEATValue::UNKNOWN);
        if (a.IsNaN<T>() || b.IsNaN<T>()) 
            return NEATValue(true);
        else
            return NEATValue(false);
    }
    template<typename T>
    static NEATValue signbit ( const NEATValue& a) {
        if(CheckAUU(a)) 
            return NEATValue(NEATValue::UNKNOWN);
        if( Utils::ge(*a.GetMin<T>(), T(0.0)))
            return NEATValue(false);
        else if ( Utils::le(*a.GetMax<T>(), T(-0.0)))
            return NEATValue(true);
        else
            return NEATValue(NEATValue::UNKNOWN);
    }


    static float localBitselect( float inA, float inB, float inC )
    {
        union {uint32_t u; float f;} a, b, c, out;
        a.f = inA;
        b.f = inB;
        c.f = inC;
        out.u = ( a.u & ~c.u ) | ( b.u & c.u );
        return out.f;
    }
    static double localBitselect( double inA, double inB, double inC )
    {
        union {uint64_t u; double f;} a, b, c, out;
        a.f = inA;
        b.f = inB;
        c.f = inC;
        out.u = ( a.u & ~c.u ) | ( b.u & c.u );
        return out.f;
    }
    template<typename T>
    static NEATValue bitselect ( const NEATValue& a, const NEATValue& b, const NEATValue& c) {
        if(CheckAUU(a) || CheckAUU(b) || CheckAUU(c)) 
            return NEATValue(NEATValue::UNKNOWN);
        if( a.IsAcc() && b.IsAcc() && c.IsAcc() )
            return NEATValue(localBitselect(*a.GetAcc<T>(),*b.GetAcc<T>(),*c.GetAcc<T>()));
        else
            return NEATValue(NEATValue::UNKNOWN);
    }
    template<typename T>
    static NEATVector bitselect(const NEATVector& vec0, const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVector(vec0,vec1,vec2,bitselect<T>);
    }

    template<typename T>
    static NEATVector processVectorRelational(const NEATVector& vec1, const NEATVector& vec2, NEATScalarBinaryOp f)
    {
        NEATVector toReturn(vec1.GetWidth());
        if(vec1.GetSize() != vec2.GetSize())
            throw Exception::InvalidArgument("Vectors with different sizes were passed to function");
        for(uint32_t i = 0; i<vec1.GetSize(); i++)
        {
            NEATValue res = f(vec1[i], vec2[i]);
            // These functions shall return a 0 if the specified relation is false and a 1 if the specified relation is true for scalar
            // argument types and a 0 if the specified relation is false and a –1 (i.e. all
            // bits set) if the specified relation is true for vector argument types
            if(res.IsAcc())
            {
                // TODO: remove that workaround for "dereferencing pointer in static member" warning
                T tmp;
                memcpy(&tmp, res.GetAcc<T>(), sizeof(T));
                toReturn[i] = NEATValue(T(0.0) - tmp);
            }
            else if (res.IsUnknown())
                toReturn[i] = res;
            else 
                throw Exception::InvalidArgument("Only ACCURATE and UNKNOWN NEAT values are supported");
        }
        return toReturn;
    }

    template<typename T>
    static NEATVector processVectorRelational(const NEATVector& vec, NEATScalarUnaryOp f)
    {
        NEATVector toReturn(vec.GetWidth());
        for(uint32_t i = 0; i<vec.GetSize(); i++)
        {
            NEATValue res = f(vec[i]);
            // These functions shall return a 0 if the specified relation is false and a 1 if the specified relation is true for scalar
            // argument types and a 0 if the specified relation is false and a –1 (i.e. all
            // bits set) if the specified relation is true for vector argument types
            if(res.IsAcc())
            {
                // TODO: remove that workaround for "dereferencing pointer in static member" warning
                T tmp;
                memcpy(&tmp, res.GetAcc<T>(), sizeof(T));
                toReturn[i] = NEATValue(T(0.0) - tmp);
            }
            else if (res.IsUnknown())
                toReturn[i] = res;
            else 
                throw Exception::InvalidArgument("Only ACCURATE and UNKNOWN NEAT values are supported");
        }
        return toReturn;
    }

    template<typename T>
    static NEATVector ISequal(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVectorRelational<T>(vec1,vec2,ISequal<T>);
    }
    template<typename T>
    static NEATVector ISnotequal(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVectorRelational<T>(vec1,vec2,ISnotequal<T>);
    }
    template<typename T>
    static NEATVector ISgreater(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVectorRelational<T>(vec1,vec2,ISgreater<T>);
    }
    template<typename T>
    static NEATVector ISgreaterequal(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVectorRelational<T>(vec1,vec2,ISgreaterequal<T>);
    }
    template<typename T>
    static NEATVector ISless(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVectorRelational<T>(vec1,vec2,ISless<T>);
    }
    template<typename T>
    static NEATVector ISlessequal(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVectorRelational<T>(vec1,vec2,ISlessequal<T>);
    }
    template<typename T>
    static NEATVector ISlessgreater(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVectorRelational<T>(vec1,vec2,ISlessgreater<T>);
    }
    template<typename T>
    static NEATVector ISfinite(const NEATVector& vec)
    {
        return processVectorRelational<T>(vec,ISfinite<T>);
    }
    template<typename T>
    static NEATVector ISinf(const NEATVector& vec)
    {
        return processVectorRelational<T>(vec,ISinf<T>);
    }
    template<typename T>
    static NEATVector ISnan(const NEATVector& vec)
    {
        return processVectorRelational<T>(vec,ISnan<T>);
    }
    template<typename T>
    static NEATVector ISnormal(const NEATVector& vec)
    {
        return processVectorRelational<T>(vec,ISnormal<T>);
    }
    template<typename T>
    static NEATVector ISordered(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVectorRelational<T>(vec1,vec2,ISordered<T>);
    }
    template<typename T>
    static NEATVector ISunordered(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVectorRelational<T>(vec1,vec2,ISunordered<T>);
    }
    template<typename T>
    static NEATVector signbit(const NEATVector& vec)
    {
        return processVectorRelational<T>(vec,signbit<T>);
    }

    template <typename T>
    static NEATValue sub ( const NEATValue& a, const NEATValue& b )
    {
        NEATValue bNeg = neg<T>(b);
        return add<T>(a,bNeg);
    }

    // flushes denormals to zero
    template <typename T>
    static NEATValue flush(const NEATValue& a)
    {
        if (a.IsUnknown())
            return NEATValue(NEATValue::UNKNOWN);
        if (a.IsAny())
            return NEATValue(NEATValue::ANY);
        if( a.IsNaN<T>())
            return NEATValue(NEATValue::NaN<T>());

        T min, max;

        min = RefALU::flush<T>(*a.GetMin<T>());
        max = RefALU::flush<T>(*a.GetMax<T>());

        return NEATValue(min, max);
    }

    template<typename T>
    static NEATValue sin(const NEATValue& a)
    {
        double ulps = double(SIN_ERROR);
        return InternalSin<T>(a, ulps);
    }

    template<typename T>
    static NEATVector sin(const NEATVector& vec)
    {
        return processVector(vec,sin<T>);
    }

    template<typename T>
    static NEATValue native_sin(const NEATValue& a)
    {
        double ulps = double(NATIVE_SIN_ERROR);
        return InternalSin<T>(a, ulps);
    }

    template<typename T>
    static NEATVector native_sin(const NEATVector& vec)
    {
        return processVector(vec,native_sin<T>);
    }

    template<typename T>
    static NEATValue half_sin(const NEATValue& a)
    {
        double ulps = double(HALF_SIN_ERROR);
        return InternalSin<T>(a, ulps);
    }

    template<typename T>
    static NEATVector half_sin(const NEATVector& vec)
    {
        return processVector(vec,half_sin<T>);
    }

    template<typename T>
    static NEATValue cos(const NEATValue& a)
    {
        double ulps = double(COS_ERROR);
        return InternalCos<T>(a, ulps);
    }

    template<typename T>
    static NEATVector cos(const NEATVector& vec)
    {
        return processVector(vec,cos<T>);
    }

    template<typename T>
    static NEATValue native_cos(const NEATValue& a)
    {
        double ulps = double(NATIVE_COS_ERROR);
        return InternalCos<T>(a, ulps);
    }

    template<typename T>
    static NEATVector native_cos(const NEATVector& vec)
    {
        return processVector(vec,native_cos<T>);
    }

    template<typename T>
    static NEATValue half_cos(const NEATValue& a)
    {
        double ulps = double(HALF_COS_ERROR);
        return InternalCos<T>(a, ulps);
    }

    template<typename T>
    static NEATVector half_cos(const NEATVector& vec)
    {
        return processVector(vec,half_cos<T>);
    }

    template<typename T>
    static NEATValue sincos(const NEATValue& a, NEATValue * y)
    {
        *y = NEATALU::cos<T>(a);
        return NEATALU::sin<T>(a);
    }

    template<typename T>
    static NEATVector sincos(const NEATVector& vec1, NEATVector& vec2)
    {
        vec2 = cos<T>(vec1);
        return sin<T>(vec1);
    }

    template<typename T>
    static NEATValue tan(const NEATValue& a)
    {
        return InternalTan<T>(a, TAN_ERROR);
    }

    template<typename T>
    static NEATVector tan(const NEATVector& vec)
    {
        return processVector(vec,tan<T>);
    }

    template<typename T>
    static NEATValue native_tan(const NEATValue& a)
    {
        return InternalTan<T>(a, NATIVE_TAN_ERROR);
    }

    template<typename T>
    static NEATVector native_tan(const NEATVector& vec)
    {
        return processVector(vec, native_tan<T>);
    }

    template<typename T>
    static NEATValue half_tan(const NEATValue& a)
    {
        return InternalTan<T>(a, HALF_TAN_ERROR);
    }

    template<typename T>
    static NEATVector half_tan(const NEATVector& vec)
    {
        return processVector(vec, half_tan<T>);
    }

    template<typename T>
    static NEATValue asin(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        if (!flushed.IsFinite<T>())
            return NEATValue::NaN<T>();

        if(flushed.IsAcc()) {
            //C 99 ISO/IEC 9899:TC2
            // F.9.1.2 asin(x) returns a NaN for | x | > 1
            if(Utils::lt(*flushed.GetAcc<T>(), (T)(-1.0)) ||
               Utils::gt(*flushed.GetAcc<T>(), (T)(1.0)))
            {
                return NEATValue(NEATValue::NaN<T>());
            }
            //C 99 ISO/IEC 9899:TC2
            // F.9.1.2 asin(+-0) returns +-0
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0)))
            {
                return NEATValue((T)(+0.0),(T)(+0.0));
            }
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0)))
            {
                return NEATValue((T)(-0.0),(T)(-0.0));
            }
        }

        // the whole interval is upper or lower (-1;1)
        // including +Inf,-Inf
        if(Utils::lt(*flushed.GetMax<T>(), (T)(-1.0)) ||
           Utils::gt(*flushed.GetMin<T>(), (T)(1.0)))
        {
             return NEATValue(NEATValue::NaN<T>());
        }

        // ASIN argument must be in range (-1 .. 1) interval
        if(Utils::lt(*flushed.GetMin<T>(), (T)(-1.0)) ||
           Utils::gt(*flushed.GetMax<T>(), (T)(1.0)))
        {
             return NEATValue(NEATValue::UNKNOWN);
        }

        // calculate reference result with higher precision
        SuperT resSuperT[2];
        resSuperT[0] = RefALU::asin(SuperT(*flushed.GetMin<T>()));
        resSuperT[1] = RefALU::asin(SuperT(*flushed.GetMax<T>()));

        // combine results, expand interval and cast down
        return ComputeResult(resSuperT, 2, 4.f);
    }

    template<typename T>
    static NEATVector asin(const NEATVector& vec)
    {
        return processVector(vec,asin<T>);
    }


    template<typename T>
    static NEATValue acos(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        //TODO: handle different flush for doubles
        NEATValue flushed = flush<T>(a);

        if(flushed.IsNaN<T>())
            return NEATValue(NEATValue::NaN<T>());


        if(flushed.IsAcc()) {
            //C 99 ISO/IEC 9899:TC2
            // F.9.1.1 acos(x) returns a NaN for | x | > 1
            if(Utils::lt(*flushed.GetAcc<T>(), (T)(-1.0)) ||
               Utils::gt(*flushed.GetAcc<T>(), (T)(1.0)))
            {
                return NEATValue(NEATValue::NaN<T>());
            }
            //C 99 ISO/IEC 9899:TC2
            // F.9.1.1 acos(1) returns +0.
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(1.0)))
            {
                return NEATValue((T)(+0.0),(T)(+0.0));
            }
        }


        // the whole interval is upper or lower (-1;1)
        // including +Inf,-Inf
        if(Utils::lt(*flushed.GetMax<T>(), (T)(-1.0)) ||
           Utils::gt(*flushed.GetMin<T>(), (T)(1.0)))
        {
             return NEATValue(NEATValue::NaN<T>());
        }

        // ACOS argument must be in range (-1 .. 1) interval
        if(Utils::lt(*flushed.GetMin<T>(), (T)(-1.0)) ||
           Utils::gt(*flushed.GetMax<T>(), (T)(1.0)))
        {
             return NEATValue(NEATValue::UNKNOWN);
        }

        // calculate reference result with higher precision
        SuperT resSuperT[2];
        resSuperT[0] = RefALU::acos(SuperT(*flushed.GetMin<T>()));
        resSuperT[1] = RefALU::acos(SuperT(*flushed.GetMax<T>()));

        // combine results, expand interval and cast down
        return ComputeResult(resSuperT, 2, 4.f);
    }

    template<typename T>
    static NEATVector acos(const NEATVector& vec)
    {
        return processVector(vec,acos<T>);
    }

    template<typename T>
    static NEATValue atan(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        // atan of NaN is NaN
        if (flushed.IsNaN<T>())
        {
            return NEATValue::NaN<T>();
        }

        if(flushed.IsAcc()) {
            //C 99 ISO/IEC 9899:TC2
            // F.9.1.3 atan(+-0) returns +-0
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0)))
            {
                return NEATValue(T(-0.0),T(-0.0));
            }
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0)))
            {
                  return NEATValue(T(+0.0),T(+0.0));
            }
            //C 99 ISO/IEC 9899:TC2
            // F.9.1.3 atan(+-INF) returns +-(pi/2)
            if(Utils::IsPInf(*flushed.GetAcc<T>())) {
                return NEATValue(T(pi_2),T(pi_2));
            }
            if(Utils::IsNInf(*flushed.GetAcc<T>())) {
                return NEATValue(T(-pi_2),T(-pi_2));
            }
        }

        // calculate reference result with higher precision
        SuperT resSuperT[2];
        resSuperT[0] = RefALU::atan(SuperT(*flushed.GetMin<T>()));
        resSuperT[1] = RefALU::atan(SuperT(*flushed.GetMax<T>()));

        // combine results, expand interval and cast down
        return ComputeResult(resSuperT, 2, 5.f);
    }

    template<typename T>
    static NEATVector atan(const NEATVector& vec)
    {
        return processVector(vec,atan<T>);
    }


    template<typename T>
    static NEATValue sinh(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        // if NaN, return NaN
        if (flushed.IsNaN<T>())
            return NEATValue::NaN<T>();

        if (flushed.IsAcc())
        {
            //C 99 ISO/IEC 9899:TC2
            // F.9.2.5 sinh(+-0) returns +-0
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0)))
            {
                return NEATValue(T(-0.0),T(-0.0));
            }
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0)))
            {
                return NEATValue(T(+0.0),T(+0.0));
            }
            //C 99 ISO/IEC 9899:TC2
            // F.9.2.5 sinh(+-INF) returns +-INF
            if(Utils::IsPInf(*flushed.GetAcc<T>()))
            {
                return NEATValue(Utils::GetPInf<T>());
            }
            if(Utils::IsNInf(*flushed.GetAcc<T>()))
            {
                return NEATValue(Utils::GetNInf<T>());
            }
        }


        // calculate reference result with higher precision
        SuperT resSuperT[2];
        resSuperT[0] = RefALU::sinh(SuperT(*flushed.GetMin<T>()));
        resSuperT[1] = RefALU::sinh(SuperT(*flushed.GetMax<T>()));

        // combine results, expand interval and cast down
        return ComputeResult(resSuperT, 2, 4.f);
    }

    template<typename T>
    static NEATVector sinh(const NEATVector& vec)
    {
        return processVector(vec,sinh<T>);
    }

    template<typename T>
    static NEATValue cosh(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        // if NaN, return NaN
        if (flushed.IsNaN<T>())
            return NEATValue::NaN<T>();

        if (flushed.IsAcc())
        {
            //C 99 ISO/IEC 9899:TC2
            // F.9.2.4 cosh(+-0) returns 1
            if((Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0))) ||
               (Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0))))
            {
                return NEATValue(T(1.0),T(1.0));
            }
            //C 99 ISO/IEC 9899:TC2
            // F.9.2.4 cosh(+-INF) returns +-INF
            if(Utils::IsPInf(*flushed.GetAcc<T>()))
            {
                return NEATValue(Utils::GetPInf<T>());
            }
            if(Utils::IsNInf(*flushed.GetAcc<T>()))
            {
                return NEATValue(Utils::GetNInf<T>());
            }
        }

        // calculate reference result with higher precision
        SuperT resSuperT[2];
        resSuperT[0] = RefALU::cosh(SuperT(*flushed.GetMin<T>()));
        resSuperT[1] = RefALU::cosh(SuperT(*flushed.GetMax<T>()));

        // combine results, expand interval and cast down
        return ComputeResult(resSuperT, 2, 4.f);
    }

    template<typename T>
    static NEATVector cosh(const NEATVector& vec)
    {
        return processVector(vec,cosh<T>);
    }

    template<typename T>
    static NEATValue tanh(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        // if NaN, return NaN
        if (flushed.IsNaN<T>())
            return NEATValue::NaN<T>();

        if (flushed.IsAcc())
        {
            //C 99 ISO/IEC 9899:TC2
            // F.9.2.6 tanh(+-0) returns +-0
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0)))
            {
                return NEATValue(T(-0.0),T(-0.0));
            }
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0)))
            {
                return NEATValue(T(+0.0),T(+0.0));
            }
            //C 99 ISO/IEC 9899:TC2
            // F.9.2.6 tanh(+-INF) returns +-1
            if(Utils::IsPInf(*flushed.GetAcc<T>()))
            {
                return NEATValue(T(+1.0),T(+1.0));
            }
            if(Utils::IsNInf(*flushed.GetAcc<T>()))
            {
                return NEATValue(T(-1.0),T(-1.0));
            }
        }

        // calculate reference result with higher precision
        SuperT resSuperT[2];
        resSuperT[0] = RefALU::tanh(SuperT(*flushed.GetMin<T>()));
        resSuperT[1] = RefALU::tanh(SuperT(*flushed.GetMax<T>()));

        // combine results, expand interval and cast down
        return ComputeResult(resSuperT, 2, 5.f);
    }

    template<typename T>
    static NEATVector tanh(const NEATVector& vec)
    {
        return processVector(vec,tanh<T>);
    }


    template<typename T>
    static NEATValue asinpi(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        if (!flushed.IsFinite<T>())
            return NEATValue::NaN<T>();

        if(flushed.IsAcc() && Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0)))
        {
            return NEATValue(T(+0.0),T(+0.0));
        }
        if(flushed.IsAcc() && Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0)))
        {
            return NEATValue(T(-0.0),T(-0.0));
        }

        // ACOS argument must be in range (-1 .. 1)
        if(Utils::gt(*flushed.GetMin<T>(), (T)(1.0)) ||
           Utils::lt(*flushed.GetMax<T>(), (T)(-1.0)))
        {
                return NEATValue::NaN<T>();
        }
        if(Utils::lt(*flushed.GetMin<T>(), (T)(-1.0)) ||
           Utils::gt(*flushed.GetMax<T>(), (T)(1.0)))
        {
            // according to The OpenCL Specification, Version: 1.1,
            // Document Revision: 36
            // 7.5.1 Additional Requirements Beyond C99 TC2
             return NEATValue::NaN<T>();
        }

        SuperT resSuperT[2];
        // calculate reference result with higher precision
        resSuperT[0] = RefALU::asinpi(SuperT(*flushed.GetMin<T>()));
        resSuperT[1] = RefALU::asinpi(SuperT(*flushed.GetMax<T>()));

        // combine results, expand interval and cast down
        return ComputeResult(resSuperT, 2, 5.f);
    }

    template<typename T>
    static NEATVector asinpi(const NEATVector& vec)
    {
        return processVector(vec,asinpi<T>);
    }

    template<typename T>
    static NEATValue acospi(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        if (!flushed.IsFinite<T>())
            return NEATValue::NaN<T>();


        if(flushed.IsAcc() && Utils::eq(*flushed.GetAcc<T>(), (T)(1.0)))
        {
            return NEATValue(T(+0.0),T(+0.0));
        }

        // ACOS argument must be in range (-1 .. 1)
        if(Utils::gt(*flushed.GetMin<T>(), (T)(1.0)) ||
           Utils::lt(*flushed.GetMax<T>(), (T)(-1.0)))
        {
                return NEATValue::NaN<T>();
        }
        if(Utils::lt(*flushed.GetMin<T>(), (T)(-1.0)) ||
           Utils::gt(*flushed.GetMax<T>(), (T)(1.0)))
        {
            // according to The OpenCL Specification, Version: 1.1,
            // Document Revision: 36
            // 7.5.1 Additional Requirements Beyond C99 TC2
             return NEATValue::NaN<T>();
        }

        SuperT resSuperT[2];

        // calculate reference result with higher precision
        resSuperT[0] = RefALU::acospi(SuperT(*flushed.GetMin<T>()));
        resSuperT[1] = RefALU::acospi(SuperT(*flushed.GetMax<T>()));

        // combine results, expand interval and cast down
        return ComputeResult(resSuperT, 2, 5.f);
    }

    template<typename T>
    static NEATVector acospi(const NEATVector& vec)
    {
        return processVector(vec,acospi<T>);
    }

    template<typename T>
    static NEATValue atanpi(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        // atanpi of NaN is NaN
        if (flushed.IsNaN<T>())
        {
            return NEATValue::NaN<T>();
        }

        // according to The OpenCL Specification, Version: 1.1,
        // Document Revision: 36
        // 7.5.1 Additional Requirements Beyond C99 TC2
        if (flushed.IsAcc())
        {
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0)))
                return NEATValue(T(-0.0),T(-0.0));
            if(Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0)))
                return NEATValue(T(+0.0),T(+0.0));
            if (Utils::IsPInf(*flushed.GetAcc<T>()))
                return NEATValue(T(+0.5),T(+0.5));
            if (Utils::IsNInf(*flushed.GetAcc<T>()))
                return NEATValue(T(-0.5),T(-0.5));
        }

        SuperT resSuperT[2];
        // calculate reference result with higher precision
        resSuperT[0] = RefALU::atanpi(SuperT(*flushed.GetMin<T>()));
        resSuperT[1] = RefALU::atanpi(SuperT(*flushed.GetMax<T>()));

        // if |min| == SOME_BIG_NUMBER and |max| == OTHER_BIG_NUMBER, so the result
        // of atanpi is limit, i.e 0.5 or -0.5 we produce accurate value
        if(resSuperT[0] == resSuperT[1])
        {
            if( resSuperT[0] == (SuperT)0.5)
                return NEATValue(T(0.5),T(0.5));
            if( resSuperT[0] == (SuperT)-0.5)
                return NEATValue(T(-0.5),T(-0.5));
        }

        // combine results, expand interval and cast down
        NEATValue res = ComputeResult(resSuperT, 2, 5.f);

        bool hasMinLimit = (resSuperT[0]==(SuperT)(-0.5));
        bool hasMaxLimit = (resSuperT[1]==(SuperT)(0.5));

        return NEATValue(hasMinLimit ? (T)-0.5 : *res.GetMin<T>(), hasMaxLimit ? (T)0.5 : *res.GetMax<T>() );
    }

    template<typename T>
    static NEATVector atanpi(const NEATVector& vec)
    {
        return processVector(vec,atanpi<T>);
    }

    template<typename T>
    static NEATValue sinpi(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        static const T MAX_POINTS[] = { (T)(0.5), (T)(2.5) };
        static const T MIN_POINTS[] = { (T)(1.5), (T)(3.5) };

        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        if (!flushed.IsFinite<T>())
            return NEATValue::NaN<T>();

        // 7.5.1 Additional Requirements Beyond C99 TC2
        if (flushed.IsAcc())
        {
            T data = *flushed.GetAcc<T>();
            double intpartF;
            T fractpart = ::modf(double(data), &intpartF);

            if( Utils::eq<T>(*flushed.GetAcc<T>(), -0.0f) ||
               (fractpart == 0 && *flushed.GetAcc<T>() < 0 ))
            {
                // sinpi ( -0 ) returns -0
                // sinpi ( -n ) returns -0 for negative integers n
                return NEATValue(T(-0.0),T(-0.0));
            }

            if( Utils::eq<T>(*flushed.GetAcc<T>(), +0.0f) ||
               (fractpart == 0 && *flushed.GetAcc<T>() >= 0 ))
            {
                // sinpi ( +0 ) returns +0
                // sinpi ( +n ) returns +0 for positive integers n
                return NEATValue(T(+0.0),T(+0.0));
            }
        }


        // we now want to search if there is a min/max point between min and max
        bool hasMinPoint = false;
        bool hasMaxPoint = false;

        if (!flushed.IsAcc()) {

            // diff = max - min
            T diff = RefALU::add( *flushed.GetMax<T>(), RefALU::neg(*flushed.GetMin<T>()) );

            if ( Utils::ge(diff, (T)2.0) )
            {
                // surely there will be a max point and a min point in a 2*pi area
                hasMinPoint = true;
                hasMaxPoint = true;
            }
            else
            {
                // we normalize the range to [0..4*pi]
                // offStart is:  min % (2*pi)
                T offStart = RefALU::mul<T>( RefALU::frc<T>( RefALU::div<T>( *flushed.GetMin<T>(), (T)2.0  ) ), (T)2.0 );
                T offEnd   = RefALU::add<T>( offStart, diff );

                // we check if the range includes min or max points
                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<T>( MIN_POINTS[0] );
                hasMinPoint |= range.Includes<T>( MIN_POINTS[1] );
                hasMaxPoint |= range.Includes<T>( MAX_POINTS[0] );
                hasMaxPoint |= range.Includes<T>( MAX_POINTS[1] );

            }
        }

        SuperT val[2];

        val[0] = RefALU::sinpi(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::sinpi(SuperT(*flushed.GetMax<T>()));

        NEATValue res = ComputeResult(val, 2, 4.f);

        return NEATValue( hasMinPoint ? (T)-1.0 : *res.GetMin<T>(), hasMaxPoint ? (T)+1.0 : *res.GetMax<T>() );

    }

    template<typename T>
    static NEATVector sinpi(const NEATVector& vec)
    {
        return processVector(vec,sinpi<T>);
    }

    template<typename T>
    static NEATValue cospi(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        static const T MAX_POINTS[] = { (T)(0.0), (T)(2.0) };
        static const T MIN_POINTS[] = { (T)(1.0), (T)(3.0) };

        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        if (!flushed.IsFinite<T>())
            return NEATValue::NaN<T>();

        // 7.5.1 Additional Requirements Beyond C99 TC2
       // for 0.0 we must return exactly 1.0
        if (flushed.IsAcc())
        {
            T data = ::fabs(*flushed.GetAcc<T>());

            if(data == 0)
            {
                return NEATValue(T(1.0),T(1.0));
            }

            double intpartF;
            T fractpart = ::modf(double(data), &intpartF);

            if(fractpart == T(0.5))
            {
                //cospi ( n + 0.5 ) is +0 for any integer n where n + 0.5 is representable
                return NEATValue(T(+0.0),T(+0.0));
            }
        }

        // we now want to search if there is a min/max point between min and max
        bool hasMinPoint = false;
        bool hasMaxPoint = false;

        T min = *flushed.GetMin<T>();
        T max = *flushed.GetMax<T>();

        if (!flushed.IsAcc()) {
            // diff = max - min

            T diff = RefALU::add( max, RefALU::neg(min) );

            if ( Utils::ge(diff, (T)2.0) )
            {
                // surely there will be a max point and a min point in a 2*pi area
                hasMinPoint = true;
                hasMaxPoint = true;
            }
            else
            {
                // we normalize the range to [0..4*pi]

                // offStart is:  min % (2*pi)
                T offStart = RefALU::mul<T>( RefALU::frc<T>( RefALU::div<T>( min, (T)2.0 ) ), (T)2.0 );
                T offEnd   = RefALU::add<T>( offStart, diff );

                // we check if the range includes min or max points
                NEATValue range(offStart, offEnd);
                hasMinPoint |= range.Includes<T>( MIN_POINTS[0] );
                hasMinPoint |= range.Includes<T>( MIN_POINTS[1] );
                hasMaxPoint |= range.Includes<T>( MAX_POINTS[0] );
                hasMaxPoint |= range.Includes<T>( MAX_POINTS[1] );

            }
        }

        SuperT val[2];

        val[0] = RefALU::cospi(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::cospi(SuperT(*flushed.GetMax<T>()));

        NEATValue res = ComputeResult(val, 2, 4.f);

        return NEATValue( hasMinPoint ? (T)-1.0 : *res.GetMin<T>(), hasMaxPoint ? (T)+1.0 : *res.GetMax<T>() );

    }

    template<typename T>
    static NEATVector cospi(const NEATVector& vec)
    {
        return processVector(vec,cospi<T>);
    }

    //TODO: it is not clear what should be output range if input range includes
    // n*pi/2, because tan(pi/2) == INF, so current implementation is correct for
    // range [-pi/2;pi/2] only
    template<typename T>
    static NEATValue tanpi(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        // if not a finite number, return NaN
        if (!flushed.IsFinite<T>())
            return NEATValue::NaN<T>();

        // for 0.0 we must return exactly 0
        if (flushed.IsAcc() && Utils::eq<T>(*flushed.GetAcc<T>(), -0.0))
        {
             return NEATValue(T(-0.0), T(-0.0));
        }
        if (flushed.IsAcc() && Utils::eq<T>(*flushed.GetAcc<T>(), +0.0))
        {
             return NEATValue(T(+.0), T(+0.0));
        }

        // TODO: implement this:
        // tanpi (n) is copysign(0.0, n) for even integers n.
        // tanpi (n) is copysign(0.0, -n) for odd integers n.

        if (flushed.IsAcc()) {
            T data = ::fabs(*flushed.GetAcc<T>());
            double intpartF;
            T fractpart = ::modf(double(data), &intpartF);
            if( Utils::eq<T>(fractpart, T(0.0)))
            {
                int64_t intpartI = int64_t(intpartF);
                int64_t intpart2 = intpartI >> 1;
                if((intpart2 << 1) == intpartI)
                {
                    // even
                    //return copysign(0.0, n)
                } else {
                    // odd
                    //return copysign(0.0, -n)
                }
            }
        }


        // tanpi (n+0.5) for even integer n is +INF where n+0.5 is representable
        // tanpi (n+0.5) for odd integer n is -INF where n+0.5 is representable
        if (flushed.IsAcc()) {
            T data = *flushed.GetAcc<T>();
            double intpartF;
            T fractpart = ::modf(double(data), &intpartF);
            if( Utils::eq<T>(fractpart, T(0.5)))
            {
                int64_t intpartI = int64_t(intpartF);
                int64_t intpart2 = intpartI >> 1;
                if((intpart2 << 1) == intpartI)
                {
                    // even
                    return NEATValue(Utils::GetPInf<T>());
                } else {
                    // odd
                    return NEATValue(Utils::GetNInf<T>());
                }
            }
        }

        SuperT val[2];
        val[0] = RefALU::tanpi(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::tanpi(SuperT(*flushed.GetMax<T>()));

        return ComputeResult(val, 2, 6.f);
    }

    template<typename T>
    static NEATVector tanpi(const NEATVector& vec)
    {
        return processVector(vec,tanpi<T>);
    }

    template<typename T>
    static NEATValue atan2(const NEATValue& y, const NEATValue& x)
    {
        typedef typename superT<T>::type SuperT;
        if(CheckAUU(x) || CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushedX = flush<T>(x);
        NEATValue flushedY = flush<T>(y);

        if(flushedX.IsAcc() && Utils::eq<T>(*flushedX.GetAcc<T>(), T(0.0)))
            return NEATValue::NaN<T>();

        if (flushedX.IsNaN<T>() || flushedY.IsNaN<T>())
        {
            return NEATValue::NaN<T>();
            // or UNKNOWN ???
        }

        if(Utils::eq<T>(*flushedX.GetMin<T>(), T(0.0)) ||
           Utils::eq<T>(*flushedX.GetMax<T>(), T(0.0)))
            return NEATValue::NaN<T>();

        T minX = *flushedX.GetMin<T>();
        T maxX = *flushedX.GetMax<T>();
        T minY = *flushedY.GetMin<T>();
        T maxY = *flushedY.GetMax<T>();

        SuperT val[2];
        val[0] = RefALU::atan2(SuperT(minY),
                               SuperT(minX));
        val[1] = RefALU::atan2(SuperT(maxY),
                               SuperT(maxX));

        return ComputeResult(val, 2, 6.f);
    }

    template<typename T>
    static NEATVector atan2(const NEATVector& vecx, const NEATVector& vecy)
    {
        return processVector(vecx,vecy,atan2<T>);
    }

    template<typename T>
    static NEATValue atan2pi(const NEATValue& y, const NEATValue& x)
    {
        typedef typename superT<T>::type SuperT;
        if(CheckAUU(x) || CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushedX = flush<T>(x);
        NEATValue flushedY = flush<T>(y);

        if (flushedX.IsNaN<T>() || flushedY.IsNaN<T>())
        {
            return NEATValue::NaN<T>();
            // TODO: fix
            // If one is NaN - > UNKNOWN
            // if both - NaN
        }

        // 7.5.1 Additional Requirements Beyond C99 TC2
        // atan2pi ( ±0, -0 ) = ±1
        if (flushedX.IsAcc() && Utils::eq<T>(*flushedX.GetAcc<T>(), -0.0)) {
            if (flushedY.IsAcc() && Utils::eq<T>(*flushedY.GetAcc<T>(), +0.0))
                return (NEATValue((T)(+1.0),(T)(+1.0)));
            else if (flushedY.IsAcc() && Utils::eq<T>(*flushedY.GetAcc<T>(), -0.0))
                return (NEATValue((T)(-1.0),(T)(-1.0)));
        }

        // atan2pi ( ±0, +0 ) = ± 0
        if (flushedX.IsAcc() && Utils::eq<T>(*flushedX.GetAcc<T>(), +0.0)) {
            if (flushedY.IsAcc() && Utils::eq<T>(*flushedY.GetAcc<T>(), +0.0))
                return (NEATValue((T)(+0.0),(T)(+0.0)));
            else if (flushedY.IsAcc() && Utils::eq<T>(*flushedY.GetAcc<T>(), -0.0))
                return (NEATValue((T)(-0.0),(T)(-0.0)));
        }

        // atan2pi ( ±0, x ) returns ± 1 for x < 0
        if (flushedX.IsAcc() && Utils::lt<T>(*flushedX.GetAcc<T>(), 0.0)) {
            if (flushedY.IsAcc() && Utils::eq<T>(*flushedY.GetAcc<T>(), +0.0))
                return (NEATValue((T)(+1.0),(T)(+1.0)));
            else if (flushedY.IsAcc() && Utils::eq<T>(*flushedY.GetAcc<T>(), -0.0))
                return (NEATValue((T)(-1.0),(T)(-1.0)));
        }

        // atan2pi ( ±0, x ) returns ± 0 for x > 0
        if (flushedX.IsAcc() && Utils::gt<T>(*flushedX.GetAcc<T>(), 0.0)) {
            if (flushedY.IsAcc() && Utils::eq<T>(*flushedY.GetAcc<T>(), +0.0))
                return (NEATValue((T)(+0.0),(T)(+0.0)));
            else if (flushedY.IsAcc() && Utils::eq<T>(*flushedY.GetAcc<T>(), -0.0))
                return (NEATValue((T)(-0.0),(T)(-0.0)));
        }

        // atan2pi ( y, ±0 ) returns -0.5 for y < 0
        // atan2pi ( y, ±0 ) returns 0.5 for y > 0
        if ((flushedX.IsAcc() && Utils::eq<T>(*flushedX.GetAcc<T>(), -0.0)) ||
            (flushedX.IsAcc() && Utils::eq<T>(*flushedX.GetAcc<T>(), +0.0)))
        {
            if (flushedY.IsAcc() && Utils::lt<T>(*flushedY.GetAcc<T>(), 0.0))
                return (NEATValue((T)(-0.5),(T)(-0.5)));
            else if (flushedY.IsAcc() && Utils::gt<T>(*flushedY.GetAcc<T>(), 0.0))
                return (NEATValue((T)(0.5),(T)(0.5)));
        }

        // atan2pi ( ±INF, x ) returns ± 0.5 for finite x
        // atan2pi (±INF, -INF ) returns ±0.75
        // atan2pi (±INF, +INF ) returns ±0.25
        if (flushedY.IsAcc() && Utils::IsPInf(*flushedY.GetAcc<T>())) {
            if(flushedX.IsFinite<T>())
                return (NEATValue((T)(0.5),(T)(0.5)));
            else if(flushedX.IsAcc() && Utils::IsPInf(*flushedX.GetAcc<T>()))
                return (NEATValue((T)(0.25),(T)(0.25)));
            else if(flushedX.IsAcc() && Utils::IsNInf(*flushedX.GetAcc<T>()))
                return (NEATValue((T)(0.75),(T)(0.75)));
        }
        if (flushedY.IsAcc() && Utils::IsNInf(*flushedY.GetAcc<T>())) {
            if(flushedX.IsFinite<T>())
                return (NEATValue((T)(-0.5),(T)(-0.5)));
            else if(flushedX.IsAcc() && Utils::IsPInf(*flushedX.GetAcc<T>()))
                return (NEATValue((T)(-0.25),(T)(-0.25)));
            else if(flushedX.IsAcc() && Utils::IsNInf(*flushedX.GetAcc<T>()))
                return (NEATValue((T)(-0.75),(T)(-0.75)));
        }

        // atan2pi ( ±y, -INF ) returns ± 1 for finite y > 0
        if (flushedX.IsAcc() && Utils::IsNInf(*flushedX.GetAcc<T>())) {
            if(flushedY.IsFinite<T>()) {
                if(flushedY.IsAcc() && Utils::lt<T>(*flushedY.GetAcc<T>(), 0.0))
                    return (NEATValue((T)(-1.0),(T)(-1.0)));
                if(flushedY.IsAcc() && Utils::gt<T>(*flushedY.GetAcc<T>(), 0.0))
                    return (NEATValue((T)(1.0),(T)(1.0)));
            }
        }

        // atan2pi ( ±y, +INF ) returns ± 0 for finite y > 0
        if (flushedX.IsAcc() && Utils::IsPInf(*flushedX.GetAcc<T>())) {
            if(flushedY.IsFinite<T>()) {
                if(flushedY.IsAcc() && Utils::lt<T>(*flushedY.GetAcc<T>(), 0.0))
                    return (NEATValue((T)(-0.0),(T)(-0.0)));
                if(flushedY.IsAcc() && Utils::gt<T>(*flushedY.GetAcc<T>(), 0.0))
                    return (NEATValue((T)(+0.0),(T)(+0.0)));
            }
        }

        SuperT minX, minY, maxX, maxY;
        minX = SuperT(*flushedX.GetMin<T>());
        minY = SuperT(*flushedY.GetMin<T>());
        maxX = SuperT(*flushedX.GetMax<T>());
        maxY = SuperT(*flushedY.GetMax<T>());

        SuperT val[2];

        val[0] = RefALU::atan2pi(minY,minX);
        val[1] = RefALU::atan2pi(maxY,maxX);

        return ComputeResult(val, 2, 6.f);
    }

    template<typename T>
    static NEATVector atan2pi(const NEATVector& vecx, const NEATVector& vecy)
    {
        return processVector(vecx,vecy,atan2pi<T>);
    }

    template<typename T>
    static NEATValue asinh(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        if (flushed.IsNaN<T>())
        {
            return NEATValue::NaN<T>();
        }

        if(flushed.IsAcc())
        {
            /// C99
            ///.asinh(+-0) == +-0
            if(Utils::eq(*flushed.GetAcc<T>(), (T)0.0) || Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0)))
                return flushed;
            /// asinh(+-inf) = +-inf
            if(Utils::IsInf(*flushed.GetAcc<T>()))
            {
                return flushed;
            }
        }

        SuperT val[2];
        val[0] = RefALU::asinh(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::asinh(SuperT(*flushed.GetMax<T>()));

        return ComputeResult(val, 2, ASINH_ERROR);
    }

    template<typename T>
    static NEATValue acosh(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        if (flushed.IsNaN<T>())
        {
            return NEATValue::NaN<T>();
        }

        if(flushed.IsAcc())
        {
            /// C99
            ///.acosh(1) == +0
            if(Utils::eq(*flushed.GetAcc<T>(), (T)1.0))
                return NEATValue((T)+0.0);
            /// acosh(+inf) = +inf;
            if(Utils::IsPInf(*flushed.GetAcc<T>()))
                return flushed;
        }

        /// C99
        /// acosh(x < 1) = NaN
        if(*flushed.GetMin<T>() < 1.0)
        {
            if(*flushed.GetMax<T>() < 1.0)
                return NEATValue::NaN<T>();
            else
                return NEATValue(NEATValue::UNKNOWN);
        }

        SuperT val[2];
        val[0] = RefALU::acosh(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::acosh(SuperT(*flushed.GetMax<T>()));

        return ComputeResult(val, 2, ACOSH_ERROR);
    }

    template<typename T>
    static NEATValue atanh(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        if (flushed.IsNaN<T>())
        {
            return NEATValue::NaN<T>();
        }

        if(flushed.IsAcc())
        {
            /// C99
            ///.atanh(+-0) == +0
            if(Utils::eq(*flushed.GetAcc<T>(), (T)0.0))
                return NEATValue((T)+0.0);
            if(Utils::eq(*flushed.GetAcc<T>(), (T)-0.0))
                return NEATValue((T)-0.0);
            /// atanh(+-1) == +-inf
            if(Utils::eq(*flushed.GetAcc<T>(), (T)1.0))
                return NEATValue(Utils::GetPInf<T>());
            if(Utils::eq(*flushed.GetAcc<T>(), (T)-1.0))
                return NEATValue(Utils::GetNInf<T>());
        }

        /// C99
        /// atanh(|x| > 1) = NaN
        /// 1. min < -1.0
        if(*flushed.GetMin<T>() < -1.0)
        {
            /// a) whole interval < -1.0
            if(*flushed.GetMax<T>() < -1.0)
                return NEATValue::NaN<T>();
            else
                /// there are values > -1.0
                return NEATValue(NEATValue::UNKNOWN);
        }
        /// 2. -1 <= min <= 1
        else if((*flushed.GetMin<T>() >= - 1.0) && (*flushed.GetMin<T>() <= 1.0))
        {
            if(*flushed.GetMax<T>() > 1)
                return NEATValue(NEATValue::UNKNOWN);
        }
        /// 3. min > 1
        else
            return NEATValue::NaN<T>();

        SuperT val[2];
        val[0] = RefALU::atanh(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::atanh(SuperT(*flushed.GetMax<T>()));

        return ComputeResult(val, 2, ATANH_ERROR);
    }

    template<typename T>
    static NEATVector atanh(const NEATVector& vecx)
    {
        return processVector(vecx,atanh<T>);
    }

    template<typename T>
    static NEATVector acosh(const NEATVector& vecx)
    {
        return processVector(vecx,acosh<T>);
    }

    template<typename T>
    static NEATVector asinh(const NEATVector& vecx)
    {
        return processVector(vecx,asinh<T>);
    }

    template<typename T>
    static NEATValue native_sqrt(const NEATValue& a)
    {
        return InternalSqrt<T>(a, double(NEATALU::NATIVE_SQRT_ERROR));
    }

    template<typename T>
    static NEATVector native_sqrt(const NEATVector& vec)
    {
        return processVector(vec,native_sqrt<T>);
    }

    template<typename T>
    static NEATValue sqrt(const NEATValue& a)
    {
        return InternalSqrt<T>(a, double(NEATALU::SQRT_ERROR));
    }

    template<typename T>
    static NEATVector sqrt(const NEATVector& vec)
    {
        return processVector(vec,sqrt<T>);
    }

    template<typename T>
    static NEATValue half_sqrt(const NEATValue& a)
    {
        return InternalSqrt<T>(a, double(NEATALU::HALF_SQRT_ERROR));
    }

    template<typename T>
    static NEATVector half_sqrt(const NEATVector& vec)
    {
        return processVector(vec,half_sqrt<T>);
    }

    template<typename T>
    static NEATValue rsqrt(const NEATValue& a)
    {
        return InternalRsqrt<T>(a, RSQRT_ERROR);
    }

    template<typename T>
    static NEATVector rsqrt(const NEATVector& vec)
    {
        return processVector(vec,rsqrt<T>);
    }

    template<typename T>
    static NEATValue native_rsqrt(const NEATValue& a)
    {
        return InternalRsqrt<T>(a, NATIVE_RSQRT_ERROR);
    }

    template<typename T>
    static NEATVector native_rsqrt(const NEATVector& vec)
    {
        return processVector(vec,native_rsqrt<T>);
    }

    template<typename T>
    static NEATValue half_rsqrt(const NEATValue& a)
    {
        return InternalRsqrt<T>(a, HALF_RSQRT_ERROR);
    }

    template<typename T>
    static NEATVector half_rsqrt(const NEATVector& vec)
    {
        return processVector(vec,half_rsqrt<T>);
    }

    template<typename T>
    static NEATVector add(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVector(vec1, vec2, add<T>);
    }

    template<typename T>
    static NEATVector sub(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVector(vec1, vec2, sub<T>);
    }

    template<typename T>
    static NEATVector mul(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVector(vec1, vec2, mul<T>);
    }

    template<typename T>
    static NEATVector div(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVector(vec1, vec2, div<T>);
    }

    template<typename T>
    static NEATVector fmod(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVector(vec1, vec2, fmod<T>);
    }

    template<typename T>
    static NEATVector remainder(const NEATVector& vec1, const NEATVector& vec2)
    {
        return processVector(vec1, vec2, remainder<T>);
    }

    template<typename T>
    static NEATValue lgamma(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        // we don't support +-0 and negative arguments so far, because 
        // logG(0)=logG(-1)=logG(-3)= +inf, and function is not defined for -1<x<0, -3<x<-2, -5<x<-4 and so on, 
        // but it is defined for -2<x<-1, -4<x<-3, -6<x<-5 and so on
        if(flushed.Includes<T>(T(0)) || *flushed.GetMax<T>() < T(0.0) || *flushed.GetMin<T>() < T(0.0) )
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        // The ULP values for built-in math functions lgamma and lgamma_r is currently undefined,
        // so we return range from -inf to +inf,as it is done in conformance, instead of:
/*
        // lgamma plot can be found here http://mathworld.wolfram.com/LogGammaFunction.html

        // lgamma has a min at this point (if x >0), as like as gamma function http://en.wikipedia.org/wiki/Gamma_function
        SuperT xMinPosVal = SuperT(1.46163);

        SuperT val[3];
        val[0] = RefALU::lgamma(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::lgamma(SuperT(*flushed.GetMax<T>()));
        if(flushed.Includes<T>(T(xMinPosVal))
            val[2] = RefALU::lgamma(xMinPosVal);
        else
            val[2] = val[0];        
        return ComputeResult(val, 3, LGAMMA_ERROR);
*/

        return NEATValue((T)Utils::GetNInf<T>(),(T)Utils::GetPInf<T>());
    }

    template<typename T>
    static NEATVector lgamma(const NEATVector& vec)
    {
        return processVector(vec, lgamma<T>);
    }


    template<typename T>
    static NEATValue lgamma_r(const NEATValue& a, int32_t *signp)
    {
        typedef typename superT<T>::type SuperT;
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);
        
        // currently not used, so produce result as like as conformance
        *signp = 0;

        // we don't support +-0 and negative arguments so far, because 
        // logG(0)=logG(-1)=logG(-3)= +inf, and function is not defined for -1<x<0, -3<x<-2, -5<x<-4 and so on, 
        // but it is defined for -2<x<-1, -4<x<-3, -6<x<-5 and so on
        if(flushed.Includes<T>(T(0)) || *flushed.GetMax<T>() < T(0.0) || *flushed.GetMin<T>() < T(0.0) )
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        // The ULP values for built-in math functions lgamma and lgamma_r is currently undefined,
        // so we return diapasone form -inf to +inf,as it is done in conformance, instead of:
/*
        // lgamma plot can be found here http://mathworld.wolfram.com/LogGammaFunction.html

        // lgamma has a min at this point (if x >0), as like as gamma function http://en.wikipedia.org/wiki/Gamma_function
        SuperT xMinPosVal = SuperT(1.46163);

        SuperT val[3];
        val[0] = RefALU::lgamma(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::lgamma(SuperT(*flushed.GetMax<T>()));
        if(flushed.Includes<T>(T(xMinPosVal))
            val[2] = RefALU::lgamma(xMinPosVal);
        else
            val[2] = val[0];        
        return ComputeResult(val, 3, LGAMMA_ERROR);
*/

        return NEATValue((T)Utils::GetNInf<T>(),(T)Utils::GetPInf<T>());
    }

    template<typename T>
    static NEATVector lgamma_r(const NEATVector& vec, std::vector<int32_t>& signp)
    {
      NEATVector toReturn(vec.GetWidth());
      size_t size = vec.GetSize();
      signp.resize(size);
      for(uint32_t i = 0; i < uint32_t(size); ++i)
      {
          toReturn[i] = lgamma_r<T>(vec[i],(int32_t*)(&signp[i]));
      }
      return toReturn;
    }

    template<typename T>
    static NEATValue mad(const NEATValue& a, const NEATValue& b, const NEATValue& c)
    {
        typedef typename superT<T>::type SuperT;
        if(CheckAUU(a) || CheckAUU(b) || CheckAUU(c))
            return NEATValue(NEATValue::UNKNOWN);

        // For mad function any value allowed (i.e. infinite ulp) so far
        return NEATValue((T)Utils::GetNInf<T>(),(T)Utils::GetPInf<T>());
    }

    template<typename T>
    static NEATVector mad(const NEATVector& vec1, const NEATVector& vec2, const NEATVector& vec3)
    {
        return processVector(vec1,vec2,vec3, mad<T>);
    }

    /// As signature of this function differs from others,
    /// Go through whole vector manually without processVector call.
    template<typename T>
    static NEATVector fcmp(const NEATVector& src1, const NEATVector& src2, CmpType comparison)
    {
        NEATVector vec1 = src1;
        NEATVector vec2 = src2;

        NEATVector toReturn(vec1.GetWidth());
        if(vec1.GetSize() != vec2.GetSize())
            throw Exception::InvalidArgument("Vectors with different sizes were passed to function");
        for(uint32_t i = 0; i<vec1.GetSize(); i++)
        {
            toReturn[i] = fcmp<T>(vec1[i], vec2[i], comparison);
        }
        return toReturn;
    }

    template<typename T>
    static T GetSinCosUpperLimit();

    template<typename T>
    static T GetSinCosLowerLimit();


    static NEATValue extractelement ( const NEATVector& vec, const uint32_t& idx );
    static NEATVector insertelement ( NEATVector vec, const NEATValue elt, const uint32_t& idx );
    static NEATVector shufflevector ( const NEATVector& vec1, const NEATVector& vec2,
                                      const std::vector<uint32_t>& mask);


    template<typename S, typename D>
    // fpext float to double is the only valid fpext so far
    static NEATValue fpext(const NEATValue& a)
    {
        NEATValue src = a;
        NEATValue val = src;
        // set new interval equal to current interval

        if(src.IsNaN<S>()) {
            val.SetAccurateVal<D>(Utils::GetNaN<D>());
        } else if(src.IsInterval()) {
            S minSrc = *src.GetMin<S>();
            S maxSrc = *src.GetMax<S>();
            D minDst = D(minSrc);
            D maxDst = D(maxSrc);
/*
            // TODO: test is it needed ( because D minDst = D(minSrc) could make it)
            if(Utils::IsPInf(minSrc))
            {
                minDst = Utils::GetPInf<D>();
            }
            if(Utils::IsNInf(minSrc)) {
                minDst = Utils::GetNInf<D>();
            }

            if(Utils::IsPInf(maxSrc)) {
                maxDst = Utils::GetPInf<D>();
            }
            if(Utils::IsNInf(maxSrc)) {
                maxDst = Utils::GetNInf<D>();
            }

            //TODO: check if it is needed to support denorm here
*/
            val.SetIntervalVal<D>(minDst,maxDst);
        } else if(src.IsAcc()) {

            D accValD = D(*src.GetAcc<S>()); // PInf and NInf are supported here
            val.SetAccurateVal<D>(accValD);

        }
        return val;
    }

    // only two bitcast with floating point value are valid so far:
    // double to the vector of two floats
    // and vector of two floats to double
    template<typename SRC, typename DEST>
    static NEATVector bitcast (const NEATValue& a) {
      // double to the vector of two floats
      NEATVector dst(V2);
      if( sizeof(SRC) == sizeof(double) && sizeof(DEST) == sizeof(float)) {
        dst[0] = NEATValue(NEATValue::UNKNOWN);
        dst[1] = NEATValue(NEATValue::UNKNOWN);
      } else {
        throw Exception::InvalidArgument(
          "[NEATALU::bitcast] wrong data type\n");
      }
      return dst;
    }


    template<typename SRC, typename ELEM_TYPE>
    static NEATVector bitcast (const NEATVector& vec) {
      NEATVector src = vec;
      VectorWidth dstWidth;
      unsigned int ratio;
      if( sizeof(SRC) > sizeof(ELEM_TYPE)) {
        // double to float bitcast
        ratio = sizeof(SRC) / sizeof(ELEM_TYPE);
        if(ratio != 2)
          throw Exception::InvalidArgument(
          "[NEATALU::bitcast] wrong data type\n");
        switch(src.GetWidth()) {
                    case V1:
                      dstWidth = V2;
                      break;
                    case V2:
                      dstWidth = V4;
                      break;
                    case V4:
                      dstWidth = V8;
                      break;
                    case V8:
                      dstWidth = V16;
                      break;
                    case V3:
                    case V16:
                    default:
                      throw Exception::InvalidArgument(
                        "[NEATALU::bitcast] wrong vector size\n");
                      break;
        }
      } else if ( sizeof(SRC) < sizeof(ELEM_TYPE)) {
        ratio = sizeof(ELEM_TYPE) / sizeof(SRC);
        if(ratio != 2)
          throw Exception::InvalidArgument(
          "[NEATALU::bitcast] wrong data type\n");
        // float to double bitcast
        switch(src.GetWidth()) {
                    case V2:
                      dstWidth = V1;
                      break;
                    case V4:
                      dstWidth = V2;
                      break;
                    case V8:
                      dstWidth = V4;
                      break;
                    case V16:
                      dstWidth = V8;
                      break;
                    case V1:
                    case V3:
                    default:
                      throw Exception::InvalidArgument(
                        "[NEATALU::bitcast] wrong vector size\n");
                      break;
        }
      } else {
        throw Exception::InvalidArgument(
          "[NEATALU::bitcast] wrong data type\n");
        // the only valid ratio is 2 so far
      }

      NEATVector dst(dstWidth);
      for(unsigned int i = 0; i< dst.GetSize(); i++) {
        dst[i] = NEATValue(NEATValue::UNKNOWN);
      }

      return dst;
    }

    template<typename S, typename D, typename R>
    static R bitcast (const NEATVector& vec) {
        NEATVector src = vec;
        unsigned int ratio = sizeof(D) / sizeof(S);

        if(ratio != 2) {
            throw Exception::InvalidArgument(
            "[NEATALU::bitcast] wrong data type\n");
        }
        if(src.GetSize() != 2) {
            throw Exception::InvalidArgument(
                "[NEATALU::bitcast] wrong src vector size\n");
        }

        return NEATValue(NEATValue::UNKNOWN);
    }

    template<typename S, typename D>
    // trunc double to float is the only valid trunc so far, so
    // S should be double and D should be float
    static NEATValue fptrunc (const NEATValue& a) {
        NEATValue src = a;
        NEATValue dst = src;
        // set new interval equal to current interval

        if(src.IsNaN<S>()) {
            dst.SetAccurateVal<D>(Utils::GetNaN<D>());
        } else if(src.IsAcc()) {

            D vFloat = D(*src.GetAcc<S>());
            dst.SetAccurateVal<D>(vFloat);

        } else if(src.IsInterval()) {
            // TODO: check for INF here
            D vMinF = D(*src.GetMin<S>());
            D vMaxF = D(*src.GetMax<S>());

            dst.SetIntervalVal<D>(vMinF,vMaxF);
        }
        return dst;
    }

    template<typename T>
    static NEATValue ceil(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;

        /// Unknown/NaN checks
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);
        if(a.IsNaN<T>())
            return NEATValue::NaN<T>();
        NEATValue flushed = flush<T>(a);
        // Process in corner cases processing as ceil is precise
        // and won't produce an error

        /// OpenCL 1.1 specification ceil(-1 < x < 0) returns -0
        if((*flushed.GetMin<T>() > (T)-1.0) && (*flushed.GetMax<T>() < 0))
            return NEATValue(T(-0.0));
        if(flushed.IsAcc())
        {
            /// C99 specification
            T accVal = *flushed.GetAcc<T>();
            /// ceil(-0) = -0
            if(Utils::eq(accVal, (T)-0.0))
                return NEATValue((T)-0.0);
            /// ceil(0) = 0
            else if(Utils::eq(accVal, (T)0.0))
                return NEATValue((T)0.0);
            /// ceil(+inf) = +inf
            else if(Utils::IsPInf(accVal))
                return NEATValue(Utils::GetPInf<T>());
            /// ceil(-inf) = -inf
            else if(Utils::IsNInf(accVal))
                return NEATValue(Utils::GetNInf<T>());
        }

        SuperT min = RefALU::ceil((SuperT)*flushed.GetMin<T>());
        SuperT max = RefALU::ceil((SuperT)*flushed.GetMax<T>());

        /// Ceil is step function.Thus when minimum is not equal to maximum
        /// answer is a set of discrete integers that cannot be represented by
        /// the single interval

        if(min != max)
            return NEATValue(NEATValue::UNKNOWN);
        else
            return NEATValue((T)min); // doesn't really matter what to return. min and max are equal
    }

    template<typename T>
    static NEATVector ceil(const NEATVector& a)
    {
        return processVector(a, ceil<T>);
    }

    /// Clamp function according to openCL specification is implemented as
    /// clamp(x, in_min, in_max) = min(max(x, in_min) in_max)
    template<typename T>
    static NEATValue clamp(const NEATValue& a, const NEATValue& in_min, const NEATValue& in_max)
    {
        typedef typename superT<T>::type SuperT;

        // If all values are NaNs then answer is NaN
        if(a.IsNaN<T>() || in_min.IsNaN<T>() || in_max.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Check unknown cases
        // Unknown values can be determined by status
        if(CheckAUU(in_min) || CheckAUU(in_max) || CheckAUU(a))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        NEATValue aFlushed = flush<T>(a);
        NEATValue minFlushed = flush<T>(in_min);
        NEATValue maxFlushed = flush<T>(in_max);

        /// Results are undefined if min > max
        /// according to ocl specification
        if(*minFlushed.GetMax<T>()>*maxFlushed.GetMin<T>())
            return NEATValue(NEATValue::ANY);

        // Done with corner cases

        T min, max;

        // aFlushed.max < minFlushed.max
        if(*aFlushed.GetMax<T>() < *minFlushed.GetMax<T>())
        {
            min = std::max(*minFlushed.GetMin<T>(), *aFlushed.GetMin<T>());
            max = *minFlushed.GetMax<T>();
        } else //  minFlushed.max <= aFlushed.max < maxFlushed.max
        if(*aFlushed.GetMax<T>() < *maxFlushed.GetMax<T>())
        {
            min = std::max(*minFlushed.GetMin<T>(), *aFlushed.GetMin<T>());
            min = std::min(min, *maxFlushed.GetMin<T>());
            max = *aFlushed.GetMax<T>();
        } else // aFlushed.max > maxFlushed.max
        {
            min = std::min(*aFlushed.GetMin<T>(), *maxFlushed.GetMin<T>());
            min = std::max(min, *minFlushed.GetMin<T>());
            max = *maxFlushed.GetMax<T>();
        }

        return NEATValue(min, max);
    }

    template<typename T>
    static NEATVector clamp(const NEATVector& a, const NEATValue& in_min, const NEATValue& in_max)
    {
        return processVector(a, in_min, in_max, clamp<T>);
    }

    template<typename T>
    static NEATVector clamp(const NEATVector& a, const NEATVector& in_min, const NEATVector& in_max)
    {
        return processVector(a, in_min, in_max, clamp<T>);
    }


    // TODO: do we need to keep 'exp' here, maybe to change
    // static NEATValue frexp(const NEATValue& a, int * exp) to
    // static NEATValue frexp(const NEATValue& a) - with no exp
    template<typename T>
    static NEATValue frexp(const NEATValue& a, int32_t * exp)
    {
        typedef typename superT<T>::type SuperT;

        /// Unknown/NaN checks
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        if(a.IsNaN<T>()) {
            // openCL spec Version: 1.1
            // 7.5.1 Additional Requirements Beyond C99 TC2
            *exp = 0;
            return NEATValue::NaN<T>();
        }

        NEATValue flushed = flush<T>(a);

        if(flushed.IsAcc())
        {
            // openCL spec Version: 1.1
            // 7.5.1 Additional Requirements Beyond C99 TC2
            T val;
            memcpy(&val, flushed.GetAcc<T>(), sizeof(T));
            if(Utils::IsPInf<T>(val)) {
                *exp = 0;
                return NEATValue(Utils::GetPInf<T>());
            }
            if(Utils::IsNInf<T>(val)) {
                *exp = 0;
                return NEATValue(Utils::GetNInf<T>());
            }
        }

        // we don't use integer results so far, but may be they to be used in future
        int32_t exp0, exp1;

        SuperT val[2];
        val[0] = RefALU::frexp(SuperT(*flushed.GetMin<T>()),&exp0);
        val[1] = RefALU::frexp(SuperT(*flushed.GetMax<T>()),&exp1);

        if( exp0 == exp1)
            *exp = exp0;
        else {
        // TODO: return some meaningful value here (if it is needed)
            *exp = -1;
            return NEATValue(NEATValue::UNKNOWN);
        }

        return ComputeResult(val, 2, FREXP_ERROR);
    }

    template<typename T>
    static NEATVector frexp(const NEATVector& vec, std::vector<int>& exp)
    {
      NEATVector toReturn(vec.GetWidth());
      size_t size = vec.GetSize();
      exp.resize(size);
      for(uint32_t i = 0; i < uint32_t(size); ++i)
      {
          toReturn[i] = frexp<T>(vec[i],(int*)(&exp[i]));
      }
      return toReturn;
    }


    template<typename T>
    static NEATValue ldexp(const NEATValue& a, const int& n)
    {
        typedef typename superT<T>::type SuperT;

        /// Unknown/NaN checks
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        if(a.IsNaN<T>()) {
            return NEATValue::NaN<T>();
        }

        NEATValue flushed = flush<T>(a);

        SuperT val[2];
        val[0] = RefALU::ldexp(SuperT(*flushed.GetMin<T>()),n);
        val[1] = RefALU::ldexp(SuperT(*flushed.GetMax<T>()),n);

        return ComputeResult(val, 2, LDEXP_ERROR);
    }


    template<typename T>
    static NEATVector ldexp(const NEATVector& vec, const int& n)
    {
        NEATVector toReturn(vec.GetWidth());
        size_t size = vec.GetSize();
        for(uint32_t i = 0; i < uint32_t(size); ++i)
        {
            toReturn[i] = ldexp<T>(vec[i],n);
        }
        return toReturn;
    }

    template<typename T>
    static NEATVector ldexp(const NEATVector& vec, std::vector<int>& n)
    {
        NEATVector toReturn(vec.GetWidth());
        size_t size = vec.GetSize();
        for(uint32_t i = 0; i < uint32_t(size); ++i)
        {
            toReturn[i] = ldexp<T>(vec[i],n[i]);
        }
        return toReturn;
    }

    template<typename T>
    static NEATValue modf(const NEATValue& a, NEATValue * iptr)
    {
        typedef typename superT<T>::type SuperT;

        if(CheckAUU(a)) {
            *iptr = NEATValue(NEATValue::UNKNOWN);
            return NEATValue(NEATValue::UNKNOWN);
        }

        if(a.IsNaN<T>()) {
            /// C99 ISO/IEC 9899 TC3
            /// modf(NaN, iptr) stores a NaN in the object pointed
            /// to by iptr (and returns a NaN)
            *iptr = NEATValue::NaN<T>();
            return NEATValue::NaN<T>();
        }

        NEATValue flushed = flush<T>(a);

        if(flushed.IsAcc())
        {
            // openCL spec Version: 1.1
            // 7.5.2 Changes to C99 TC2 Behavior
            // gentype modf ( gentype value, gentype *iptr )
            // {
            //     *iptr = trunc( value );
            //      return copysign( isinf( value ) ? 0.0 : value – *iptr, value );
            // }
            SuperT val = SuperT(*flushed.GetAcc<T>());
            SuperT integr = RefALU::trunc(val);
            SuperT res;

            *iptr = NEATValue(T(integr));

            if(Utils::IsInf<SuperT>(val))
            {
                res = SuperT(0.0);
            } else {
                res = val - integr;
            }

            res = RefALU::copysign(res, val);
            return NEATValue(T(res));
        }

        SuperT valF[2]; // fractional parts
        SuperT valI[2]; // integral parts

        SuperT min = *flushed.GetMin<T>();
        SuperT max = *flushed.GetMax<T>();

        valF[0] = RefALU::modf(min, &valI[0]);
        valF[1] = RefALU::modf(max, &valI[1]);

        NEATValue resI = ComputeResult(valI, 2, MODF_ERROR);
        *iptr = resI; // set the integral part

        if(*resI.GetMin<T>() == *resI.GetMax<T>()) {
            // if integral parts are the same, fractional parts are the min and max
            return ComputeResult(valF, 2, MODF_ERROR);
        } else {
            SuperT diff = RefALU::abs(max - min);
            if( diff < SuperT(1.0) ) {
                // for example, min=1.8, max=2.2, fractial part should be greater
                // than 0.8 and less than 0.2, we are not able to make correct interval,
                // so we set UNKNOWN here
                return NEATValue(NEATValue::UNKNOWN);
            } else {
                if(Utils::ge(min, SuperT(0.0))) {
                    // all possible fractial values are
                    // less than 1.0 and greater or equal to 0.0
                    return NEATValue( T(0.0), T(1.0)-std::numeric_limits<T>::min());
                } else if(Utils::le(max, SuperT(0.0))) {
                    // all possible fractial values are
                    // less or equal to 0.0 and greater than -1.0
                    return NEATValue( T(-1.0)+std::numeric_limits<T>::min(), T(0.0));
                } else {
                    // source interval includes 0 and difference between min and max greater than 1.0
                    if(valI[0] == SuperT(0.0)) {
                        // example min=-0.3, max=100.1 result should be
                        // min=-0.3, max=0.(9)
                        return NEATValue( T(valF[0]), T(1.0)-std::numeric_limits<T>::min());
                    } else if (valI[1] == SuperT(0.0)) {
                        // example min=-100.1, max=0.3 result should be
                        // min=-0.(9), max=0.3
                        return NEATValue( T(-1.0)+std::numeric_limits<T>::min(), T(valF[1]));
                    } else
                        // example min=-100.1, max=100.1 result should be
                        // min=-0.(9), max=0.(9)
                        return NEATValue( T(-1.0)+std::numeric_limits<T>::min(),
                                          T(1.0)-std::numeric_limits<T>::min());
                }
            }
        }
    }

    template<typename T>
    static NEATVector modf(const NEATVector& vec1, NEATVector& vec2)
    {
        NEATVector toReturn(vec1.GetWidth());
        size_t size = vec1.GetSize();
        for(uint32_t i = 0; i < uint32_t(size); ++i)
        {
            toReturn[i] = modf<T>(vec1[i],&vec2[i]);
        }
        return toReturn;
    }

    template<typename T>
    static NEATValue rootn(const NEATValue& a, const int& n)
    {
        typedef typename superT<T>::type SuperT;

        if(CheckAUU(a)) {
            return NEATValue(NEATValue::UNKNOWN);
        }

        if(a.IsNaN<T>()) {
            return NEATValue::NaN<T>();
        }

        // openCL spec Version: 1.1
        // 7.5.1 Additional Requirements Beyond C99 TC2
        // rootn (x, 0) returns NaN
        if(n == 0) {
            return NEATValue::NaN<T>();
        }

        NEATValue flushed = flush<T>(a);
        if(flushed.IsAcc())
        {
            // openCL spec Version: 1.1
            // 7.5.1 Additional Requirements Beyond C99 TC2
            //rootn (x, n) returns flushed NaN for x < 0 and n is even.
            T accVal;
            memcpy(&accVal, flushed.GetAcc<T>(), sizeof(T));
            if( accVal < 0 && 0 == (n&1) ) {
                return NEATValue::NaN<T>();
            }

            if( accVal == T(0.0) )
            {
                if(n < 0)  {
                    if( n&1 ) {
                        //rootn ( +-0,  n ) is +-inf for odd n < 0
                        if(Utils::eq(accVal,(T)(-0.0)))
                            return NEATValue(Utils::GetNInf<T>());
                        else
                            return NEATValue(Utils::GetPInf<T>());
                    } else {
                        //rootn ( +-0,  n ) is +inf for even n < 0
                        return NEATValue(Utils::GetPInf<T>());
                    }
                } else {
                    if( n&1 ) {
                        //rootn ( +-0,  n ) is +-0 for odd n > 0
                        return flushed;
                    } else {
                        //rootn ( +-0,  n ) is +0 for even n > 0
                        return NEATValue(T(0.0));
                    }
                }
            }
        }
        // flushed is interval

        // n is even, the whole interval is < 0, so result is NaN
        if( Utils::lt(*flushed.GetMax<T>(),T(0.0)) && (!(n&1))) {
            return NEATValue::NaN<T>();
        }

        // n is even, the interval is partly < 0, so result is UNKNOWN
        if( Utils::lt(*flushed.GetMin<T>(),T(0.0)) && (!(n&1))) {
            return NEATValue(NEATValue::UNKNOWN);
        }

        SuperT val[4];
        val[0] = RefALU::rootn(SuperT(*flushed.GetMin<T>()),n);
        val[1] = RefALU::rootn(SuperT(*flushed.GetMax<T>()),n);

        if( n < 0) {
            if( n&1 ) {
                // rootn ( +-0,  n ) is +-inf for odd n < 0
                if(flushed.Includes(T(-0.0)))
                    val[0] = Utils::GetNInf<T>();

                if(flushed.Includes(T(+0.0)))
                    val[1] = Utils::GetPInf<T>();
            } else {
                // rootn ( +-0,  n ) is +inf for even n < 0
                if(flushed.Includes(T(0.0)))
                    val[1] = Utils::GetPInf<T>();
            }
        }
        return ComputeResult(val, 2, ROOTN_ERROR);
    }

    template<typename T>
    static NEATVector rootn(const NEATVector& vec, const std::vector<int>& n)
    {
        NEATVector toReturn(vec.GetWidth());
        size_t size = vec.GetSize();
        for(uint32_t i = 0; i < uint32_t(size); ++i)
        {
            toReturn[i] = rootn<T>(vec[i],n[i]);
        }
        return toReturn;
    }

    template<typename T>
    static NEATValue exp(const NEATValue& a)
    {
        double ulps = double(EXP_ERROR);
        return InternalExp<T>(a, ulps);
    }

    template<typename T>
    static NEATVector exp(const NEATVector& a)
    {
        return processVector(a, exp<T>);
    }

    template<typename T>
    static NEATValue native_exp(const NEATValue& a)
    {
        double ulps = double(NATIVE_EXP_ERROR);
        return InternalExp<T>(a, ulps);
    }

    template<typename T>
    static NEATVector native_exp(const NEATVector& a)
    {
        return processVector(a, native_exp<T>);
    }

    template<typename T>
    static NEATValue half_exp(const NEATValue& a)
    {
        double ulps = double(HALF_EXP_ERROR);
        return InternalExp<T>(a, ulps);
    }

    template<typename T>
    static NEATVector half_exp(const NEATVector& a)
    {
        return processVector(a, half_exp<T>);
    }

    template<typename T>
    static NEATValue exp2(const NEATValue& a)
    {
        double ulps = double(EXP2_ERROR);
        return InternalExp2<T>(a, ulps);
    }

    template<typename T>
    static NEATVector exp2(const NEATVector& a)
    {
        return processVector(a, exp2<T>);
    }

    template<typename T>
    static NEATValue native_exp2(const NEATValue& a)
    {
        double ulps = double(NATIVE_EXP2_ERROR);
        return InternalExp2<T>(a, ulps);
    }

    template<typename T>
    static NEATVector native_exp2(const NEATVector& a)
    {
        return processVector(a, native_exp2<T>);
    }

    template<typename T>
    static NEATValue half_exp2(const NEATValue& a)
    {
        double ulps = double(HALF_EXP2_ERROR);
        return InternalExp2<T>(a, ulps);
    }

    template<typename T>
    static NEATVector half_exp2(const NEATVector& a)
    {
        return processVector(a, half_exp2<T>);
    }

    template<typename T>
    static NEATValue exp10(const NEATValue& a)
    {
        double ulps = double(EXP10_ERROR);
        return InternalExp10<T>(a, ulps);
    }

    template<typename T>
    static NEATVector exp10(const NEATVector& a)
    {
        return processVector(a, exp10<T>);
    }

    template<typename T>
    static NEATValue native_exp10(const NEATValue& a)
    {
        double ulps = double(NATIVE_EXP10_ERROR);
        return InternalExp10<T>(a, ulps);
    }

    template<typename T>
    static NEATVector native_exp10(const NEATVector& a)
    {
        return processVector(a, native_exp10<T>);
    }

    template<typename T>
    static NEATValue half_exp10(const NEATValue& a)
    {
        double ulps = double(HALF_EXP10_ERROR);
        return InternalExp10<T>(a, ulps);
    }

    template<typename T>
    static NEATVector half_exp10(const NEATVector& a)
    {
        return processVector(a, half_exp10<T>);
    }

    template<typename T>
    static NEATValue expm1(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        /// Check for ANY, UNWRITTEN, UNKNOWN
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(a);

        if(a.IsNaN<T>())
            return NEATValue::NaN<T>();

        if(flushed.IsAcc())
        {
            // openCL spec Version: 1.1
            // 7.5.1 Additional Requirements Beyond C99 TC2
            /// expm1(+inf) = +inf
            if(Utils::IsPInf<T>(*flushed.GetAcc<T>()))
                return NEATValue(Utils::GetPInf<T>());
            /// expm1(-inf) = -1
            if(Utils::IsNInf<T>(*flushed.GetAcc<T>()))
                return NEATValue((T)-1.0);
            /// expm1(+-0) = +-0
            if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)-0.0))
                return NEATValue((T)-0.0);
            if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)+0.0))
                return NEATValue((T)+0.0);
        }

        SuperT val[2];

        val[0] = RefALU::expm1(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::expm1(SuperT(*flushed.GetMax<T>()));

        return ComputeResult(val, 2, EXPM1_ERROR);
    }

    template<typename T>
    static NEATVector expm1(const NEATVector& a)
    {
        return processVector(a, expm1<T>);
    }

    template<typename T>
    static NEATValue log2(const NEATValue& a)
    {
        double ulps = double(LOG2_ERROR);
        return InternalLog2<T>(a, ulps);
    }

    template<typename T>
    static NEATVector log2(const NEATVector& a)
    {
        return processVector(a, log2<T>);
    }

    template<typename T>
    static NEATValue native_log2(const NEATValue& a)
    {
        double ulps = double(NATIVE_LOG2_ERROR);
        return InternalLog2<T>(a, ulps);
    }

    template<typename T>
    static NEATVector native_log2(const NEATVector& a)
    {
        return processVector(a, native_log2<T>);
    }

    template<typename T>
    static NEATValue half_log2(const NEATValue& a)
    {
        double ulps = double(HALF_LOG2_ERROR);
        return InternalLog2<T>(a, ulps);
    }

    template<typename T>
    static NEATVector half_log2(const NEATVector& a)
    {
        return processVector(a, half_log2<T>);
    }

    template<typename T>
    static NEATValue log(const NEATValue& a)
    {
        double ulps = double(LOG_ERROR);
        return InternalLog<T>(a, ulps);
    }

    template<typename T>
    static NEATVector log(const NEATVector& a)
    {
        return processVector(a, log<T>);
    }

    template<typename T>
    static NEATValue native_log(const NEATValue& a)
    {
        double ulps = double(NATIVE_LOG_ERROR);
        return InternalLog<T>(a, ulps);
    }

    template<typename T>
    static NEATVector native_log(const NEATVector& a)
    {
        return processVector(a, native_log<T>);
    }

    template<typename T>
    static NEATValue half_log(const NEATValue& a)
    {
        double ulps = double(HALF_LOG_ERROR);
        return InternalLog<T>(a, ulps);
    }

    template<typename T>
    static NEATVector half_log(const NEATVector& a)
    {
        return processVector(a, half_log<T>);
    }

    template<typename T>
    static NEATValue log10(const NEATValue& a)
    {
        double ulps = double(LOG10_ERROR);
        return InternalLog10<T>(a, ulps);
    }

    template<typename T>
    static NEATVector log10(const NEATVector& a)
    {
        return processVector(a, log10<T>);
    }

    template<typename T>
    static NEATValue native_log10(const NEATValue& a)
    {
        double ulps = double(NATIVE_LOG10_ERROR);
        return InternalLog10<T>(a, ulps);
    }

    template<typename T>
    static NEATVector native_log10(const NEATVector& a)
    {
        return processVector(a, native_log10<T>);
    }
    template<typename T>
    static NEATValue half_log10(const NEATValue& a)
    {
        double ulps = double(HALF_LOG10_ERROR);
        return InternalLog10<T>(a, ulps);
    }

    template<typename T>
    static NEATVector half_log10(const NEATVector& a)
    {
        return processVector(a, half_log10<T>);
    }

    template<typename T>
    static NEATValue log1p(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // Check for ANY, UNWRITTEN, UNKNOWN
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        if(a.IsNaN<T>())
            return NEATValue::NaN<T>();

        NEATValue flushed = flush<T>(a);

        if(flushed.IsAcc())
        {
            /// log1p(+0) = +0
            if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)+0.0))
                return NEATValue((T)+0.0);
            /// log1p(+0) = -0
            if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)-0.0))
                return NEATValue((T)-0.0);
            /// log1p(-1) = -inf
            if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)-1.0))
                return NEATValue(Utils::GetNInf<T>());
            /// log1p(x < -1) = NaN
            if(Utils::lt<T>(*flushed.GetAcc<T>(), (T)-1.0))
                return NEATValue::NaN<T>();
            /// log1p(+inf) = +inf
            if(Utils::IsPInf<T>(*flushed.GetAcc<T>()))
                return NEATValue(Utils::GetPInf<T>());
        }

        SuperT val[2];

        val[0] = RefALU::log1p(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::log1p(SuperT(*flushed.GetMax<T>()));

        return ComputeResult(val, 2, LOG1P_ERROR);
    }

    template<typename T>
    static NEATVector log1p(const NEATVector& a)
    {
        return processVector(a, log1p<T>);
    }

    template<typename T>
    static NEATValue logb(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // Check for ANY, UNWRITTEN, UNKNOWN
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        if(a.IsNaN<T>())
            return NEATValue::NaN<T>();

        NEATValue flushed = flush<T>(a);

        if(flushed.IsAcc())
        {
            /// logb(+-0) = -inf
            if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)+0.0) || Utils::eq<T>(*flushed.GetAcc<T>(), (T)-0.0))
                return NEATValue(Utils::GetNInf<T>());
            /// logb(+-inf) = +inf
            if(Utils::IsPInf<T>(*flushed.GetAcc<T>()) || Utils::IsNInf<T>(*flushed.GetAcc<T>()))
                return NEATValue(Utils::GetPInf<T>());
        }

        SuperT val[2];

        val[0] = RefALU::logb(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::logb(SuperT(*flushed.GetMax<T>()));

        return ComputeResult(val, 2, LOGB_ERROR);
    }

    template<typename T>
    static NEATVector logb(const NEATVector& a)
    {
        return processVector(a, logb<T>);
    }

    template<typename T>
    static NEATValue cbrt(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // Check for ANY, UNWRITTEN, UNKNOWN
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        if(a.IsNaN<T>())
            return NEATValue::NaN<T>();

        NEATValue flushed = flush<T>(a);

        SuperT val[2];

        val[0] = RefALU::cbrt(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::cbrt(SuperT(*flushed.GetMax<T>()));

        return ComputeResult(val, 2, CBRT_ERROR);
    }

    template<typename T>
    static NEATVector cbrt(const NEATVector& a)
    {
        return processVector(a, cbrt<T>);
    }

    template<typename T>
    static NEATValue copysign(const NEATValue& x, const NEATValue& y)
    {
        // Check for ANY, UNWRITTEN, UNKNOWN
        if(CheckAUU(x) || CheckAUU(y))
            return NEATValue(NEATValue::UNKNOWN);

        if(x.IsNaN<T>() || y.IsNaN<T>())
            return NEATValue::NaN<T>();

        NEATValue flushedX = flush<T>(x);
        NEATValue flushedY = flush<T>(y);

        // if x is accurate and y is interval with yMin <0 and yMax >0
        // result values are -x and x exactly with no allowed interval 
        // between these values, so result is UNKNOWN
        if ( flushedX.IsAcc() && 
            Utils::lt<T>(*flushedY.GetMin<T>(),T(0.0)) && 
            Utils::gt<T>(*flushedY.GetMax<T>(),T(0.0)) )
            return NEATValue(NEATValue::UNKNOWN);

        T val[5];

        val[0] = RefALU::copysign(*flushedX.GetMin<T>(),*flushedY.GetMin<T>());
        val[1] = RefALU::copysign(*flushedX.GetMin<T>(),*flushedY.GetMax<T>());
        val[2] = RefALU::copysign(*flushedX.GetMax<T>(),*flushedY.GetMin<T>());
        val[3] = RefALU::copysign(*flushedX.GetMax<T>(),*flushedY.GetMax<T>());
        val[4] = val[0];

        if (flushedX.Includes<T>(T(0.0)))
            val[4] = T(0.0);

        T min=T(0), max=T(0);
        NEATValue res;

        res.SetStatus(Combine(val, 5, min, max));

        if(res.GetStatus() == NEATValue::INTERVAL ||
           res.GetStatus() == NEATValue::ACCURATE)
        {
            res = NEATValue(min, max);
        }

        return res;
    }

    template<typename T>
    static NEATVector copysign(const NEATVector& vecx, const NEATVector& vecy)
    {
        return processVector(vecx, vecy, copysign<T>);
    }

    template<typename T>
    static NEATValue fdim(const NEATValue& x, const NEATValue& y)
    {
        typedef typename superT<T>::type SuperT;
        // Check for ANY, UNWRITTEN, UNKNOWN
        if(CheckAUU(x) || CheckAUU(y))
            return NEATValue(NEATValue::UNKNOWN);

        if(x.IsNaN<T>() || y.IsNaN<T>())
            return NEATValue::NaN<T>();

        NEATValue flushedX = flush<T>(x);
        NEATValue flushedY = flush<T>(y);

        if( Utils::gt<T>(*flushedX.GetMin<T>(),*flushedY.GetMax<T>())) {
            // the whole x interval is higher than whole y interval
            // return x - y if x > y
            return NEATALU::sub<T>(flushedX, flushedY);
        }
        if ( Utils::le<T>(*flushedX.GetMax<T>(),*flushedY.GetMin<T>())) {
            // the whole x interval is lower than whole y interval
            // return +0 if x is less than or equal to y
            return NEATValue(T(+0.0));
        }

        T xMax = *flushedX.GetMax<T>();
        T yMin = *flushedY.GetMin<T>();

        // the low interval limit is +0.0, i.e. result is 0 if x is less than or equal to y
        // the upper interval limit is max(x)-min(y), because result is x-y if x>y
        SuperT maxRes = RefALU::sub(SuperT(xMax), SuperT(yMin));
        return NEATValue(T(0.0),castDown(maxRes));
    }

    template<typename T>
    static NEATVector fdim(const NEATVector& vecx, const NEATVector& vecy)
    {
        return processVector(vecx, vecy, fdim<T>);
    }

// FP_ILOGB0 and FP_ILOGBNAN values are taken from conformance
    #ifndef FP_ILOGB0
        #define FP_ILOGB0   INT_MIN
    #endif

    #ifndef FP_ILOGBNAN
        #define FP_ILOGBNAN   INT_MAX
    #endif

    template<typename T>
    static NEATValue ilogb(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // Check for ANY, UNWRITTEN, UNKNOWN
        if(CheckAUU(a))
            return NEATValue(NEATValue::UNKNOWN);

        // ilogb(x) returns FP_ILOGBNAN if x is NaN
        if(a.IsNaN<T>())
            return NEATValue(T(FP_ILOGBNAN));

        NEATValue flushed = flush<T>(a);

        if(flushed.IsAcc())
        {
            // ilogb(x) returns FP_ILOGB0 if x is zero
            T tmp;
            memcpy(&tmp, flushed.GetAcc<T>(), sizeof(T));
            if(Utils::eq<T>(tmp, (T)+0.0) || Utils::eq<T>(tmp, (T)-0.0))
                return NEATValue(T(FP_ILOGB0));
        }

        SuperT min = RefALU::ilogb(SuperT(*flushed.GetMin<T>()));
        SuperT max = RefALU::ilogb(SuperT(*flushed.GetMax<T>()));
        if (min != max)
            return NEATValue(NEATValue::UNKNOWN);
        else
           return NEATValue(T(min));        
    }

    template<typename T>
    static NEATVector ilogb(const NEATVector& a)
    {
        return processVector(a, ilogb<T>);
    }

    /// Returns true if there is integer value in [min; max] interval or false otherwise
    template<typename T>
    static bool ContainsInteger(T min, T max)
    {
        T minFloor = ::floor(min);
        T maxFloor = ::floor(max);
        if(maxFloor == minFloor)
        {
            if((minFloor != min) && (maxFloor != max))
            {
                return false;
            }
        }
        return true;
    }

    /// returns true if given interval contains any odd value
    template<typename T>
    static bool ContainsOdd(T min, T max)
    {
        T minFloor = ::floor((min - (T)1) * (T)0.5);
        T maxFloor = ::floor((max - (T)1) * (T)0.5);
        return (minFloor != maxFloor);
    }

    template<typename T>
    static NEATValue pow(const NEATValue& x, const NEATValue& y)
    {
        return InternalPow<T>(x, y, double(POW_ERROR));
    }

    template<typename T>
    static NEATVector pow(const NEATVector& base, const NEATVector& power)
    {
        return processVector(base, power, pow<T>);
    }

    template<typename T>
    static NEATValue powr(const NEATValue& x, const NEATValue& y)
    {
        return InternalPowr<T>(x, y, double(POWR_ERROR));
    }

    template<typename T>
    static NEATVector powr(const NEATVector& base, const NEATVector& power)
    {
        return processVector(base, power, powr<T>);
    }

    template<typename T>
    static NEATValue pown(const NEATValue& x, const int32_t& y)
    {
        return InternalPown<T>(x, y, double(POWN_ERROR));
    }

    template<typename T>
    static NEATVector pown(const NEATVector& vec, const std::vector<int>& n)
    {
        NEATVector toReturn(vec.GetWidth());
        size_t size = vec.GetSize();
        for(size_t i = 0; i < size; ++i)
        {
            toReturn[i] = pown<T>(vec[i],n[i]);
        }
        return toReturn;
    }

    template<typename T>
    static NEATValue native_powr(const NEATValue& x, const NEATValue& y)
    {
        return InternalPowr<T>(x, y, double(NATIVE_POWR_ERROR));
    }

    template<typename T>
    static NEATVector native_powr(const NEATVector& base, const NEATVector& power)
    {
        return processVector(base, power, native_powr<T>);
    }

    template<typename T>
    static NEATValue half_powr(const NEATValue& x, const NEATValue& y)
    {
        return InternalPowr<T>(x, y, double(HALF_POWR_ERROR));
    }

    template<typename T>
    static NEATVector half_powr(const NEATVector& base, const NEATVector& power)
    {
        return processVector(base, power, half_powr<T>);
    }

    // these functions are used to support LLVM 'select' instruction
    static NEATValue select (const bool& cond, const NEATValue& a, const NEATValue& b);
    static NEATVector select (const bool& cond, const NEATVector& a, const NEATVector& b);
    static NEATVector select (const std::vector<bool>& cond, const NEATVector& a, const NEATVector& b);

    // these functions are used to support OpenCL built-in 'select'
    template<typename T>
    static NEATValue select (const NEATValue& a, const NEATValue& b, const int64_t& c)
    {
        bool cond = bool(c); // For a scalar type, result = c ? b : a
        return NEATALU::select(cond, b, a);
    }
    template<typename T>
    static NEATVector select(const NEATVector& a, const NEATVector& b, const std::vector<int64_t>& c)
    {
        std::vector<bool> cond;
        //For each component of a vector type, result[i] = if MSB of c[i] is set ? b[i] : a[i]
        for(size_t i = 0; i < c.size(); ++i)
            cond.push_back(bool(c[i] & int64_t(INT64_MIN)));

        return NEATALU::select(cond, b, a);
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////             Geometric functions            ////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    // dot
    ///////////////////////////////////////////////////////////////////////////////
    template<typename T>
    static NEATValue dot(const NEATValue& val1, const NEATValue& val2)
    {
        typedef typename superT<T>::type SuperT;

        SuperT vals[4];
        SuperT min=SuperT(0), max=SuperT(0);

        if(CheckAUU(val1) || (CheckAUU(val2)))
            return NEATValue(NEATValue::UNKNOWN);

        if (val1.IsNaN<T>() || val2.IsNaN<T>())
            return NEATValue::NaN<T>();

        if (val1.IsFinite<T>() && val2.IsFinite<T>())
        {
            NEATValue flushedX = flush<T>(val1);
            NEATValue flushedY = flush<T>(val2);
            // calculating all the possible combinations for mul
            vals[0] = RefALU::mul((SuperT)*flushedX.GetMin<T>(), (SuperT)*flushedY.GetMax<T>());
            vals[1] = RefALU::mul((SuperT)*flushedX.GetMin<T>(), (SuperT)*flushedY.GetMin<T>());
            vals[2] = RefALU::mul((SuperT)*flushedX.GetMax<T>(), (SuperT)*flushedY.GetMin<T>());
            vals[3] = RefALU::mul((SuperT)*flushedX.GetMax<T>(), (SuperT)*flushedY.GetMax<T>());

            Combine(vals, 4, min, max);

        } else
            return NEATValue(NEATValue::UNKNOWN);

        vals[0] = min;
        vals[1] = max;

        return ComputeResult(vals, 2, NEATALU::MUL_ERROR); // mul is the only op here
    }

    template<typename T>
    static NEATValue dot(const NEATVector& vec1, const NEATVector& vec2)
    {
        typedef typename superT<T>::type SuperT;

        if( (unsigned)vec1.GetSize() != (unsigned)vec2.GetSize() )
            throw Exception::InvalidArgument("[NEATALU::dot] wrong vector size\n");

        SuperT vals[4];
        SuperT min=SuperT(0), max=SuperT(0);
        SuperT maxAbs = SuperT(0);

        for(uint32_t i=0; i<vec1.GetSize(); i++)
        {
            if(CheckAUU(vec1[i]) || (CheckAUU(vec2[i])))
                return NEATValue(NEATValue::UNKNOWN);

            if (vec1[i].IsNaN<T>() || vec2[i].IsNaN<T>())
                return NEATValue::NaN<T>();

            if (vec1[i].IsFinite<T>() && vec2[i].IsFinite<T>())
            {
                NEATValue flushedX = flush<T>(vec1[i]);
                NEATValue flushedY = flush<T>(vec2[i]);
                // find abs max for max ulp calculation
                SuperT a0 = RefALU::abs((SuperT)*flushedX.GetMin<T>());
                SuperT a1 = RefALU::abs((SuperT)*flushedX.GetMax<T>());
                SuperT a2 = RefALU::abs((SuperT)*flushedY.GetMin<T>());
                SuperT a3 = RefALU::abs((SuperT)*flushedY.GetMax<T>());

                // TODO: implement RefALU::fmax here ?
                a0 = (a0 > a1 ? a0 : a1);
                a2 = (a2 > a3 ? a2 : a3);
                a0 = (a0 > a2 ? a0 : a2);
                if( a0 > maxAbs)
                    maxAbs = a0;

                // calculating all the possible combinations for mul
                vals[0] = RefALU::mul((SuperT)*flushedX.GetMin<T>(), (SuperT)*flushedY.GetMax<T>());
                vals[1] = RefALU::mul((SuperT)*flushedX.GetMin<T>(), (SuperT)*flushedY.GetMin<T>());
                vals[2] = RefALU::mul((SuperT)*flushedX.GetMax<T>(), (SuperT)*flushedY.GetMin<T>());
                vals[3] = RefALU::mul((SuperT)*flushedX.GetMax<T>(), (SuperT)*flushedY.GetMax<T>());

                SuperT localMin=SuperT(0), localMax=SuperT(0);
                Combine(vals, 4, localMin, localMax);

                // accumulate result
                min = RefALU::add(min, localMin);
                max = RefALU::add(max, localMax);

            } else
                return NEATValue(NEATValue::UNKNOWN);
        }

        //ulps is 2*vecSize - 1 (n + n-1 max # of errors)
        SuperT ulps = 2.*SuperT(vec1.GetSize())-1.;

        maxAbs = maxAbs*maxAbs;
        ExpandFloatInterval<SuperT>(&min, &max, maxAbs, ulps);

        T minD = castDown(min);
        T maxD = castDown(max);

        minD = RefALU::flush<T>(minD);
        maxD = RefALU::flush<T>(maxD);

        return NEATValue(minD, maxD);
    }


    ///////////////////////////////////////////////////////////////////////////////
    // cross
    ///////////////////////////////////////////////////////////////////////////////
    // Returns the cross product of p0.xyz and p1.xyz. float3 and float4 inputs are
    // possible. The w component of float4 result returned will be 0.0
    ///////////////////////////////////////////////////////////////////////////////
    template<typename T>
    static NEATValue crossOneItem(const NEATValue& in0, const NEATValue& in1, const NEATValue& in2, const NEATValue& in3)
    {
        typedef typename superT<T>::type sT;
        NEATValue res;

        // calculating ulps for each item of resulting vector as like as conformance test does
        sT vS0 = RefALU::fmax( RefALU::fabs(sT(*in0.GetMax<T>())), RefALU::fabs(sT(*in0.GetMin<T>())));
        sT vS1 = RefALU::fmax( RefALU::fabs(sT(*in1.GetMax<T>())), RefALU::fabs(sT(*in1.GetMin<T>())));
        sT vS2 = RefALU::fmax( RefALU::fabs(sT(*in2.GetMax<T>())), RefALU::fabs(sT(*in2.GetMin<T>())));
        sT vS3 = RefALU::fmax( RefALU::fabs(sT(*in3.GetMax<T>())), RefALU::fabs(sT(*in3.GetMin<T>())));

        sT item4ulp = RefALU::fmax(vS0, RefALU::fmax(vS1, RefALU::fmax(vS2, vS3)));
        sT ulpsCross = sT(NEATALU::CROSS_ERROR) * sT(ComputeUlp(1.0));
        sT delta = item4ulp * item4ulp * ulpsCross;

        sT vals[4];
        sT min0=sT(0), max0=sT(0);
        sT min1=sT(0), max1=sT(0);

        // res[i] = (vecA[j] * vecB[k]) - (vecA[k] * vecB[j]);
        NEATValue flushed0 = flush<T>(in0);
        NEATValue flushed1 = flush<T>(in1);
        NEATValue flushed2 = flush<T>(in2);
        NEATValue flushed3 = flush<T>(in3);

        sT minA = sT(*flushed0.GetMin<T>());
        sT maxA = sT(*flushed0.GetMax<T>());
        sT minB = sT(*flushed1.GetMin<T>());
        sT maxB = sT(*flushed1.GetMax<T>());

        // (vecA[j] * vecB[k])
        vals[0] = RefALU::mul<sT>(minA, maxB);
        vals[1] = RefALU::mul<sT>(minA, minB);
        vals[2] = RefALU::mul<sT>(maxA, minB);
        vals[3] = RefALU::mul<sT>(maxA, maxB);
        
        res.SetStatus(Combine(vals, 4, min0, max0));

        if(res.GetStatus() != NEATValue::INTERVAL &&
           res.GetStatus() != NEATValue::ACCURATE)
        return res;

        minA = sT(*flushed2.GetMin<T>());
        maxA = sT(*flushed2.GetMax<T>());
        minB = sT(*flushed3.GetMin<T>());
        maxB = sT(*flushed3.GetMax<T>());

        // (vecA[k] * vecB[j])
        vals[0] = RefALU::mul<sT>(minA, maxB);
        vals[1] = RefALU::mul<sT>(minA, minB);
        vals[2] = RefALU::mul<sT>(maxA, minB);
        vals[3] = RefALU::mul<sT>(maxA, maxB);

        res.SetStatus(Combine(vals, 4, min1, max1));

        if(res.GetStatus() != NEATValue::INTERVAL &&
           res.GetStatus() != NEATValue::ACCURATE)
        return res;

        // (vecA[j] * vecB[k]) - (vecA[k] * vecB[j]);
        sT a = RefALU::neg(max1);
        max1 = RefALU::neg(min1);
        min1 = a;
        vals[0] = RefALU::add(min0, min1);
        vals[1] = RefALU::add(max0, max1);

        res.SetStatus(Combine(vals, 2, min0, max0));

        if(res.GetStatus() != NEATValue::INTERVAL &&
           res.GetStatus() != NEATValue::ACCURATE)
        return res;

        // expand interval
        min0 -= delta;
        max0 += delta;

        T minD = castDown(min0);
        if (min0 > sT(minD)) {
            minD += ComputeUlp(minD);
        }
        T maxD = castDown(max0);
        if (max0 < sT(maxD)) {
            maxD -= ComputeUlp(maxD);
        }

        minD = RefALU::flush<T>(minD);
        maxD = RefALU::flush<T>(maxD);

        return NEATValue(minD, maxD);
    }

    template<typename T>
    static NEATVector cross(const NEATVector& vec1, const NEATVector& vec2)
    {
        // float3 and float4 inputs are allowed, vector size is checked by plug-in
        NEATVector res(vec1.GetWidth());

        // res[0] = (vec1[1] * vec2[2]) - (vec1[2] * vec2[1]);
        // res[1] = (vec1[2] * vec2[0]) - (vec1[0] * vec2[2]);
        // res[2] = (vec1[0] * vec2[1]) - (vec1[1] * vec2[0]);
        // "bad" value of vec1[0] makes res[1] and res[2] "bad",
        // "bad" value of vec1[1] makes res[0] and res[2] "bad" and so on

        if(CheckAUU(vec1[0]) || vec1[0].IsNaN<T>() || 
           CheckAUU(vec2[0]) || vec2[0].IsNaN<T>() ) {
            res[1] = NEATValue(NEATValue::UNKNOWN);
            res[2] = NEATValue(NEATValue::UNKNOWN);
        }
        if(CheckAUU(vec1[1]) || vec1[1].IsNaN<T>() ||
           CheckAUU(vec2[1]) || vec2[1].IsNaN<T>() ) {
            res[0] = NEATValue(NEATValue::UNKNOWN);
            res[2] = NEATValue(NEATValue::UNKNOWN);
        }
        if(CheckAUU(vec1[2]) || vec1[2].IsNaN<T>() ||
           CheckAUU(vec2[2]) || vec2[2].IsNaN<T>() ) {
            res[0] = NEATValue(NEATValue::UNKNOWN);
            res[1] = NEATValue(NEATValue::UNKNOWN);
        }
        // initially each item of result vector is set to UNWRITTEN. Then it 
        // can be set to UNKNOWN, but the others items should be calculated.
        //  so we check if the item set to UNWRITTEN state.

        // out[0] = (vec1[1] * vec2[2]) - (vec1[2] * vec2[1]);
        if (res[0].IsUnwritten()) {
            res[0] = crossOneItem<T>(vec1[1], vec2[2], vec1[2], vec2[1]);
        }
        // out[1] = (vec1[2] * vec2[0]) - (vec1[0] * vec2[2]);
        if (res[1].IsUnwritten()) {
            res[1] = crossOneItem<T>(vec1[2], vec2[0], vec1[0], vec2[2]);
        }
        // out[2] = (vec1[0] * vec2[1]) - (vec1[1] * vec2[0]);
        if (res[2].IsUnwritten()) {
            res[2] = crossOneItem<T>(vec1[0], vec2[1], vec1[1], vec2[0]);
        }

        // The w component of float4 result returned will be 0.0
        if( res.GetWidth() == V4)
            res[3] = NEATValue(T(0));

        return res;
    }
    ///////////////////////////////////////////////////////////////////////////////
    // length, fast_length
    ///////////////////////////////////////////////////////////////////////////////
    template<typename T> 
    static NEATValue localLength(T inMin, T inMax, double ulps)
    {
        T min = RefALU::mul(inMin, inMin);
        T max = RefALU::mul(inMax, inMax);

        T val[2];
        val[0] = RefALU::sqrt(min);
        val[1] = RefALU::sqrt(max);

        return ComputeResult(val, 2, ulps);
    }

    template<typename T>
    static NEATValue length(const NEATValue& p)
    {
        typedef typename superT<T>::type SuperT;
        if (p.IsNaN<T>())
            return NEATValue(NEATValue::NaN<T>());

        if(CheckAUU(p))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(p);

        SuperT min = (SuperT)*flushed.GetMin<T>();
        SuperT max = (SuperT)*flushed.GetMax<T>();

        double maxUlps = NEATALU::SQRT_ERROR +  // error in sqrt
                                         0.25;  // error for multiplications

        return localLength<SuperT>(min, max, maxUlps);
    }

    template<typename T>
    static NEATValue fast_length(const NEATValue& p)
    {
        typedef typename superT<T>::type SuperT;
        if (p.IsNaN<T>())
            return NEATValue(NEATValue::NaN<T>());

        if(CheckAUU(p))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed = flush<T>(p);

        SuperT min = (SuperT)*flushed.GetMin<T>();
        SuperT max = (SuperT)*flushed.GetMax<T>();

        double maxUlps = NEATALU::HALF_SQRT_ERROR +  // error in sqrt
                                         0.25;       // error for multiplications

        return localLength<SuperT>(min, max, maxUlps);
    }

    template<typename T> static 
    NEATValue localLengthVec(const NEATVector& vec1, double ulps)
    {
        typedef typename superT<T>::type SuperT;

        for(size_t i=0; i<vec1.GetSize(); i++) {
            if (vec1[i].IsNaN<T>()) 
                return NEATValue(NEATValue::NaN<T>());

            if(CheckAUU(vec1[i]))
                return NEATValue(NEATValue::UNKNOWN);
        }

        SuperT totalMin=SuperT(0), totalMax=SuperT(0);
        for(size_t i=0; i<vec1.GetSize(); i++)
        {
            NEATValue flushed = flush<T>(vec1[i]);

            SuperT localMin = RefALU::mul<SuperT>((SuperT)*flushed.GetMin<T>(), (SuperT)*flushed.GetMin<T>());
            SuperT localMax = RefALU::mul<SuperT>((SuperT)*flushed.GetMax<T>(), (SuperT)*flushed.GetMax<T>());

            if(localMin > localMax) std::swap(localMin, localMax);

            // accumulate result - the sum of squares
            totalMin = RefALU::add<SuperT>(totalMin, localMin);
            totalMax = RefALU::add<SuperT>(totalMax, localMax);
        }

        // square root of the sum of squares
        SuperT val[2];
        val[0] = RefALU::sqrt<SuperT>(totalMin);
        val[1] = RefALU::sqrt<SuperT>(totalMax);

        return ComputeResult(val, 2, ulps);
    }

    template<typename T>
    static NEATValue length(const NEATVector& vec1)
    {
        double maxUlps = NEATALU::SQRT_ERROR +  // error in sqrt
        0.5 *                                   // effect on e of taking sqrt( x + e )
        ( 0.5 * (double) vec1.GetSize() +       // cumulative error for multiplications
          0.5 * (double) (vec1.GetSize()-1));   // cumulative error for additions

        return localLengthVec<T>(vec1, maxUlps);
    }

    template<typename T>
    static NEATValue fast_length(const NEATVector& vec1)
    {
        double maxUlps = NEATALU::HALF_SQRT_ERROR +   // error in sqrt
        0.5 *                                   // effect on e of taking sqrt( x + e )
        ( 0.5 * (double) vec1.GetSize() +       // cumulative error for multiplications
          0.5 * (double) (vec1.GetSize()-1));   // cumulative error for additions

        return localLengthVec<T>(vec1, maxUlps);
    }
    ///////////////////////////////////////////////////////////////////////////////
    // distance, fast_distance
    ///////////////////////////////////////////////////////////////////////////////
    template<typename T>
    static NEATValue distance(const NEATValue& p0, const NEATValue& p1)
    {
        typedef typename superT<T>::type SuperT;
        if (p0.IsNaN<T>() || p1.IsNaN<T>())
            return NEATValue(NEATValue::NaN<T>());

        if(CheckAUU(p0) || CheckAUU(p1))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushed0 = flush<T>(p0);
        NEATValue flushed1 = flush<T>(p1);

        SuperT min = RefALU::sub((SuperT)*flushed0.GetMin<T>(),(SuperT)*flushed1.GetMin<T>());
        SuperT max = RefALU::sub((SuperT)*flushed0.GetMax<T>(),(SuperT)*flushed1.GetMax<T>());

        double maxUlps = NEATALU::SQRT_ERROR + 1.5; // error in sqrt and
                                                    // cumulative error for multiplication

        return localLength<SuperT>(min, max, maxUlps);
    }

    template<typename T>
    static NEATValue fast_distance(const NEATValue& p0, const NEATValue& p1)
    {
        typedef typename superT<T>::type SuperT;
        if (p0.IsNaN<T>() || p1.IsNaN<T>())
            return NEATValue(NEATValue::NaN<T>());

        if(CheckAUU(p0) || CheckAUU(p1))
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue flushedX = flush<T>(p0);
        NEATValue flushedY = flush<T>(p1);

        SuperT min = RefALU::sub((SuperT)*flushedX.GetMin<T>(),(SuperT)*flushedY.GetMin<T>());
        SuperT max = RefALU::sub((SuperT)*flushedX.GetMax<T>(),(SuperT)*flushedY.GetMax<T>());

        double maxUlps = NEATALU::HALF_SQRT_ERROR + 1.5; // error in sqrt and
                                                         // cumulative error for multiplication

        return localLength<SuperT>(min, max, maxUlps);
    }

    template<typename T>
    static NEATValue localDistanceVec(const NEATVector& vec0, const NEATVector& vec1, double ulps)
    {
        typedef typename superT<T>::type SuperT;

        for(size_t i=0; i<vec0.GetSize(); i++) {
            if (vec0[i].IsNaN<T>() || vec1[i].IsNaN<T>())
                return NEATValue(NEATValue::NaN<T>());

            if(CheckAUU(vec0[i]) || CheckAUU(vec1[i]))
                return NEATValue(NEATValue::UNKNOWN);
        }

        SuperT totalMin=SuperT(0), totalMax=SuperT(0);
        for(size_t i=0; i<vec1.GetSize(); i++)
        {
            NEATValue flushed0 = flush<T>(vec0[i]);
            NEATValue flushed1 = flush<T>(vec1[i]);

            SuperT localMin = RefALU::sub((SuperT)*flushed0.GetMin<T>(), (SuperT)*flushed1.GetMin<T>());
            SuperT localMax = RefALU::sub((SuperT)*flushed0.GetMax<T>(), (SuperT)*flushed1.GetMax<T>());

            localMin = RefALU::mul(localMin, localMin);
            localMax = RefALU::mul(localMax, localMax);

            if(localMin > localMax) std::swap(localMin, localMax);

            // accumulate result - the sum of squares
            totalMin = RefALU::add(totalMin, localMin);
            totalMax = RefALU::add(totalMax, localMax);
        }

        // square root of the sum of squares
        SuperT val[2];
        val[0] = RefALU::sqrt(totalMin);
        val[1] = RefALU::sqrt(totalMax);

        return ComputeResult(val, 2, ulps);
    }

    template<typename T>
    static NEATValue distance(const NEATVector& vec0, const NEATVector& vec1)
    {
        double maxUlps = NEATALU::SQRT_ERROR +  // error in sqrt
        ( 1.5 * (double) vec1.GetSize() +       // cumulative error for multiplications  
                                                // (a-b+0.5ulp)**2 = (a-b)**2 + a*0.5ulp + b*0.5 ulp + 0.5 ulp for multiplication
         0.5 * (double) (vec1.GetSize()-1));    // cumulative error for additions

        return localDistanceVec<T>(vec0, vec1, maxUlps);
    }

    template<typename T>
    static NEATValue fast_distance(const NEATVector& vec0, const NEATVector& vec1)
    {
        double maxUlps = NEATALU::HALF_SQRT_ERROR +  // error in sqrt
        ( 1.5 * (double) vec1.GetSize() +       // cumulative error for multiplications  
                                                // (a-b+0.5ulp)**2 = (a-b)**2 + a*0.5ulp + b*0.5 ulp + 0.5 ulp for multiplication
         0.5 * (double) (vec1.GetSize()-1));    // cumulative error for additions

        return localDistanceVec<T>(vec0, vec1, maxUlps);
    }
    ///////////////////////////////////////////////////////////////////////////////
    // normalize
    ///////////////////////////////////////////////////////////////////////////////
    template<typename T>
    static NEATValue normalize(const NEATValue& p)
    {
        typedef typename superT<T>::type SuperT;

        if (p.IsNaN<T>())
        {
            // openCL spec Version: 1.1
            // 7.5.1 Additional Requirements Beyond C99 TC2
            // normalize (v) returns a vector full of NaNs if any element is a NaN
            return NEATValue(NEATValue::NaN<T>());
        }

        if(CheckAUU(p))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        // openCL spec Version: 1.1
        // 7.5.1 Additional Requirements Beyond C99 TC2
        // normalize (v) returns v if all elements of v are zero
        if (*p.GetMax<T>() == 0 && *p.GetMin<T>() == 0)
            return p;

        // openCL spec Version: 1.1
        // 7.5.1 Additional Requirements Beyond C99 TC2
        // special case for infinity
        if(p.IsAcc() && Utils::IsPInf<T>(*p.GetAcc<T>()))
            return NEATValue(T(1.0));
        if(p.IsAcc() && Utils::IsNInf<T>(*p.GetAcc<T>()))
            return NEATValue(T(-1.0));

        SuperT val[1];
        if(p.IsAcc()) {
             val[0] = RefALU::copysign(SuperT(1.0),SuperT(*p.GetAcc<T>()));
             return ComputeResult(val, 1, NEATALU::NORMALIZE_ERROR);
        } else {
            if(*p.GetMin<T>() > 0) {
                val[0] = SuperT(1.0);
                return ComputeResult(val, 1, NEATALU::NORMALIZE_ERROR);
            }
            else if(*p.GetMax<T>() < 0) {
                val[0] = SuperT(-1.0);
                return ComputeResult(val, 1, NEATALU::NORMALIZE_ERROR);
            }
            else
                return NEATValue(NEATValue::UNKNOWN);
       }
    }

    template<typename T>
    static NEATVector normalize(const NEATVector& p)
    {
        typedef typename superT<T>::type SuperT;

        NEATVector vec1 = p;

        bool flagINF = false;

        uint32_t cntZero = 0;
        for(uint32_t i=0; i<vec1.GetSize(); i++) {
            if (vec1[i].IsNaN<T>())
            {
                // openCL spec Version: 1.1
                // 7.5.1 Additional Requirements Beyond C99 TC2
                // normalize (v) returns a vector full of NaNs if any element is a NaN
                for(uint32_t j=0; j<vec1.GetSize(); j++)
                    vec1[j] = NEATValue(NEATValue::NaN<T>());
                return vec1;
            }

            if(CheckAUU(vec1[i]))
            {
                for(uint32_t j=0; j<vec1.GetSize(); j++)
                    vec1[j] = NEATValue(NEATValue::UNKNOWN);
                return vec1;
            }

            // openCL spec Version: 1.1
            // 7.5.1 Additional Requirements Beyond C99 TC2
            // normalize (v) returns v if all elements of v are zero
            if (*vec1[i].GetMax<T>() == 0 && *vec1[i].GetMin<T>() == 0)
                cntZero++;

            // openCL spec Version: 1.1
            // 7.5.1 Additional Requirements Beyond C99 TC2
            // special case for infinity
            if(vec1[i].IsAcc() && Utils::IsInf<T>(*vec1[i].GetAcc<T>()))
                flagINF = true;
        }

        // normalize (v) returns v if all elements of v are zero
        if (cntZero == vec1.GetSize())
            return vec1;


        // special case for infinity
        // normalize ( v ) for which any element in v is infinite shall proceed as if
        // the elements in v were replaced as follows:
        // for( i = 0; i < sizeof(v) / sizeof(v[0] ); i++ )
        // v[i] = isinf(v[i] ) ? copysign(1.0, v[i]) : 0.0 * v [i];
        if(flagINF)
        {
            for(uint32_t i=0; i<vec1.GetSize(); i++)
            {
                T min = *vec1[i].GetMin<T>();
                T max = *vec1[i].GetMax<T>();
                T minS, maxS;
                if(Utils::IsInf<T>(min))
                {
                    vec1[i] =  NEATValue(NEATValue::UNKNOWN);
                    continue;
                }
                else
                    minS = T(0.0);
                if(Utils::IsInf<T>(max))
                {
                    vec1[i] =  NEATValue(NEATValue::UNKNOWN);
                    continue;
                }
                else
                    maxS = T(0.0);
                T valMin = RefALU::copysign(minS,min);
                T valMax = RefALU::copysign(maxS,max);
                vec1[i] = NEATValue(valMin,valMax);
            }
            return vec1;
        }


        SuperT totalMin=SuperT(0), totalMax=SuperT(0);
        // TODO: check what if all low limits or all high limit of vector all zero
        for(uint32_t i=0; i<vec1.GetSize(); i++)
        {
            SuperT localMin=SuperT(0), localMax=SuperT(0);
            SuperT val[3];

            val[0] = RefALU::mul((SuperT)*vec1[i].GetMin<T>(), (SuperT)*vec1[i].GetMin<T>());
            val[1] = RefALU::mul((SuperT)*vec1[i].GetMax<T>(), (SuperT)*vec1[i].GetMax<T>());

            if(vec1[i].IsInterval() && vec1[i].Includes<T>(T(0))) {
                val[2] = SuperT(0);
                Combine(val, 3, localMin, localMax);
            } else {
                Combine(val, 2, localMin, localMax);
            }

            // accumulate result - the sum of squares
            totalMin = RefALU::add(totalMin, localMin);
            totalMax = RefALU::add(totalMax, localMax);
        }

        // 1 / (square root of the sum of squares)
        totalMin = RefALU::rsqrt(totalMin);
        totalMax = RefALU::rsqrt(totalMax);

        float maxUlps = NEATALU::NORMALIZE_ERROR +  // error in rsqrt + error in multiply
        ( 0.5f * (float) vec1.GetSize() +      // cumulative error for multiplications
         0.5f * (float) (vec1.GetSize()-1));   // cumulative error for additions

        for(uint32_t i=0; i<vec1.GetSize(); i++)
        {
            SuperT localMin = (SuperT)*vec1[i].GetMin<T>();
            SuperT localMax = (SuperT)*vec1[i].GetMax<T>();
            SuperT val[4];

            val[0] = RefALU::mul(localMin, totalMin);
            val[1] = RefALU::mul(localMin, totalMax);
            val[2] = RefALU::mul(localMax, totalMin);
            val[3] = RefALU::mul(localMax, totalMax);

            vec1[i] = ComputeResult(val, 4, maxUlps);
        }

        return vec1;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // fast_normalize
    ///////////////////////////////////////////////////////////////////////////////
    template<typename T>
    static NEATValue fast_normalize(const NEATValue& p)
    {
        typedef typename superT<T>::type SuperT;

        NEATValue flushed = flush<T>(p);

        if (flushed.IsNaN<T>())
        {
            // the behaviour for NaN is not defined in spec, so process as like as nomalize does
            return NEATValue(NEATValue::NaN<T>());
        }

        if(CheckAUU(flushed))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        // openCL spec Version: 1.1
        // 7.5.1 Additional Requirements Beyond C99 TC2
        // normalize (v) returns v if all elements of v are zero
        if (*flushed.GetMax<T>() == 0 && *flushed.GetMin<T>() == 0)
            return flushed;

        SuperT val[4];

        val[0] = RefALU::mul((SuperT)*flushed.GetMin<T>(), (SuperT)*flushed.GetMin<T>());
        val[1] = RefALU::mul((SuperT)*flushed.GetMax<T>(), (SuperT)*flushed.GetMax<T>());

        SuperT totalMin=SuperT(0), totalMax=SuperT(0);
        if(flushed.IsInterval() && flushed.Includes<T>(T(0))) {
            val[2] = SuperT(0);
            Combine(val, 3, totalMin, totalMax);
        } else {
            Combine(val, 2, totalMin, totalMax);
        }

        // openCL spec Version: 1.1, 6.11.5 : If the sum of squares is greater than FLT_MAX
        // then the value of the floating-point values in the result vector are undefined.
        if(totalMin > std::numeric_limits<T>::max() || totalMax > std::numeric_limits<T>::max())
            return NEATValue(NEATValue::ANY);

        if(flushed.IsAcc()) {
             val[0] = RefALU::copysign(SuperT(1.0),SuperT(*flushed.GetAcc<T>()));
             return ComputeResult(val, 1, NEATALU::FAST_NORMALIZE_ERROR);
        } else {
            if(*flushed.GetMin<T>() > 0) {
                val[0] = SuperT(1.0);
                return ComputeResult(val, 1, NEATALU::FAST_NORMALIZE_ERROR);
            }
            else if(*flushed.GetMax<T>() < 0) {
                val[0] = SuperT(-1.0);
                return ComputeResult(val, 1, NEATALU::FAST_NORMALIZE_ERROR);
            }
            else
                return NEATValue(NEATValue::UNKNOWN);
       }
    }

    template<typename T>
    static NEATVector fast_normalize(const NEATVector& p)
    {
        typedef typename superT<T>::type SuperT;

        NEATVector vec1(p.GetWidth());

        uint32_t cntZero = 0;
        for(uint32_t i=0; i<vec1.GetSize(); i++) {
            if (p[i].IsNaN<T>())
            {
                // the behaviour for NaN is not defined in spec, so process as like as nomalize does
                for(uint32_t j=0; j<vec1.GetSize(); j++)
                    vec1[j] = NEATValue(NEATValue::NaN<T>());
                return vec1;
            }

            if(CheckAUU(p[i]))
            {
                for(uint32_t j=0; j<vec1.GetSize(); j++)
                    vec1[j] = NEATValue(NEATValue::UNKNOWN);
                return vec1;
            }

            vec1[i] = flush<T>(p[i]);
            
            if (*vec1[i].GetMax<T>() == 0 && *vec1[i].GetMin<T>() == 0)
                cntZero++;
        }

        // fast_normalize (v) returns v if all elements of v are zero
        if (cntZero == vec1.GetSize())
            return vec1;


        SuperT totalMin=SuperT(0), totalMax=SuperT(0);
        // TODO: check what if all low limits or all high limit of vector all zero
        for(uint32_t i=0; i<vec1.GetSize(); i++)
        {
            SuperT localMin=SuperT(0), localMax=SuperT(0);
            SuperT val[3];

            val[0] = RefALU::mul((SuperT)*vec1[i].GetMin<T>(), (SuperT)*vec1[i].GetMin<T>());
            val[1] = RefALU::mul((SuperT)*vec1[i].GetMax<T>(), (SuperT)*vec1[i].GetMax<T>());

            if(vec1[i].IsInterval() && vec1[i].Includes<T>(T(0))) {
                val[2] = SuperT(0);
                Combine(val, 3, localMin, localMax);
            } else {
                Combine(val, 2, localMin, localMax);
            }

            // accumulate result - the sum of squares
            totalMin = RefALU::add(totalMin, localMin);
            totalMax = RefALU::add(totalMax, localMax);
        }

        // openCL spec Version: 1.1, 6.11.5 : If the sum of squares is greater than FLT_MAX
        // then the value of the floating-point values in the result vector are undefined.
        if(totalMin > std::numeric_limits<T>::max() || totalMax > std::numeric_limits<T>::max()) {
                for(uint32_t j=0; j<vec1.GetSize(); j++)
                    vec1[j] = NEATValue(NEATValue::ANY);
                return vec1;
        }

        // 1 / (square root of the sum of squares)
        totalMin = RefALU::rsqrt(totalMin);
        totalMax = RefALU::rsqrt(totalMax);

        float maxUlps = NEATALU::FAST_NORMALIZE_ERROR +  // error in rsqrt + error in multiply
        ( 0.5f * (float) vec1.GetSize() +      // cumulative error for multiplications
         0.5f * (float) (vec1.GetSize()-1));   // cumulative error for additions

        for(uint32_t i=0; i<vec1.GetSize(); i++)
        {
            SuperT localMin = (SuperT)*vec1[i].GetMin<T>();
            SuperT localMax = (SuperT)*vec1[i].GetMax<T>();
            SuperT val[4];

            val[0] = RefALU::mul(localMin, totalMin);
            val[1] = RefALU::mul(localMin, totalMax);
            val[2] = RefALU::mul(localMax, totalMin);
            val[3] = RefALU::mul(localMax, totalMax);

            vec1[i] = ComputeResult(val, 4, maxUlps);
        }

        return vec1;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////             Common functions               ////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    // Clamp, degrees, min, max, mix, radians, step, smoothstep and sign

    ///////////////////////////////////////////////////////////////////////////////
    // conformance provide for mix function the same precision, i.e.
    // no higher precision used for temporary results
    template<typename T>
    static NEATValue mix(const NEATValue& x, const NEATValue& y, const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        NEATValue res;

        NEATValue flushedA = flush<T>(a);

        // if the interval or accurate "a" is out of range,
        // return values are undefined
        if( (*flushedA.GetMin<T>() > (T)1.f) ||
            (*flushedA.GetMax<T>() < (T)0.f) )
        {
            res = NEATValue(NEATValue::ANY);
        } else
        // if the interval "a" is partly out of range,
        // return values are unknown
        if( (*flushedA.GetMin<T>() < (T)0.f) ||
            (*flushedA.GetMax<T>() > (T)1.f) )
        {
            res = NEATValue(NEATValue::UNKNOWN);
        } else {
            if(x.IsNaN<T>() || y.IsNaN<T>())
                res = NEATValue::NaN<T>();
            else if(CheckAUU(x) || CheckAUU(y) || CheckAUU(flushedA))
                res = NEATValue(NEATValue::UNKNOWN);
            else {
                // result is x+(y-x)*a
                // so we should expand output interval, basing on max value of x and y, not
                // on result value

                SuperT maxAbs = SuperT(0);
                // find abs max for max ulp calculation
                SuperT a0 = RefALU::abs((SuperT)*x.GetMin<T>());
                SuperT a1 = RefALU::abs((SuperT)*x.GetMax<T>());
                SuperT a2 = RefALU::abs((SuperT)*y.GetMin<T>());
                SuperT a3 = RefALU::abs((SuperT)*y.GetMax<T>());

                // TODO: implement RefALU::fmax here ?
                a0 = (a0 > a1 ? a0 : a1);
                a2 = (a2 > a3 ? a2 : a3);
                a0 = (a0 > a2 ? a0 : a2);
                if( a0 > maxAbs)
                    maxAbs = a0;

                SuperT val[4], min=SuperT(0), max=SuperT(0);
                val[0] = RefALU::sub(SuperT(*y.GetMin<T>()), SuperT(*x.GetMin<T>()));
                val[1] = RefALU::sub(SuperT(*y.GetMax<T>()), SuperT(*x.GetMax<T>()));
                Combine(val, 2, min, max);

                val[0] = RefALU::mul(min, SuperT(*flushedA.GetMin<T>()));
                val[1] = RefALU::mul(max, SuperT(*flushedA.GetMax<T>()));
                val[2] = RefALU::mul(min, SuperT(*flushedA.GetMax<T>()));
                val[3] = RefALU::mul(max, SuperT(*flushedA.GetMin<T>()));
                Combine(val, 4, min, max);

                min = RefALU::add(min, SuperT(*x.GetMin<T>()));
                max = RefALU::add(max, SuperT(*x.GetMax<T>()));

                ExpandFloatInterval<SuperT>(&min, &max, maxAbs, NEATALU::MIX_ERROR);

                T minD = castDown(min);
                T maxD = castDown(max);

                minD = RefALU::flush<T>(minD);
                maxD = RefALU::flush<T>(maxD);

                return NEATValue(minD, maxD);
            }
        }
        return res;
    }

    template<typename T>
    static NEATVector mix(const NEATVector& x, const NEATVector& y, const NEATVector& a)
    {
        return processVector(x, y, a, mix<T>);
    }

    template<typename T>
    static NEATVector mix(const NEATVector& x, const NEATVector& y, const NEATValue& a)
    {
        return processVector(x, y, a, mix<T>);
    }


    ///////////////////////////////////////////////////////////////////////////////
    // radians built-in function
    template<typename T>
    static NEATValue radians(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        SuperT val[2];
        const SuperT pi180 = pi/SuperT(180.0);
        val[0] = RefALU::mul(SuperT(*flushed.GetMin<T>()), pi180);
        val[1] = RefALU::mul(SuperT(*flushed.GetMax<T>()), pi180);

        return ComputeResult(val, 2, float(RADIANS_ERROR) );
    }

    template<typename T>
    static NEATVector radians(const NEATVector& vec)
    {
        return processVector(vec,radians<T>);
    }

    ///////////////////////////////////////////////////////////////////////////////
    // degrees built-in function
    template<typename T>
    static NEATValue degrees(const NEATValue& a)
    {
        typedef typename superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        SuperT val[2];
        val[0] = RefALU::mul(SuperT(*flushed.GetMin<T>()), SuperT(div180by_pi));
        val[1] = RefALU::mul(SuperT(*flushed.GetMax<T>()), SuperT(div180by_pi));

        return ComputeResult(val, 2, float(DEGREES_ERROR) );
    }

    template<typename T>
    static NEATVector degrees(const NEATVector& vec)
    {
        return processVector(vec,degrees<T>);
    }
    ///////////////////////////////////////////////////////////////////////////////
    // sign built-in function
    template<typename T>
    static NEATValue sign(const NEATValue& a)
    {
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (a.IsNaN<T>())
            return NEATValue(T(0.0)); // Returns 0.0 if a is NaN

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        if(flushed.IsAcc())
            return NEATValue(RefALU::sign<T>(*flushed.GetAcc<T>()));

        T min = *flushed.GetMin<T>();
        T max = *flushed.GetMax<T>();

        if( Utils::gt(min,T(0.0)) )
            return NEATValue(T(1.0)); // Returns 1.0 if a > 0
        if( Utils::lt(max,T(-0.0)) )
            return NEATValue(T(-1.0)); // Returns -1.0 if a < 0

        if( Utils::eq(min,T(-0.0)) && Utils::eq(max,T(+0.0)) )
            return NEATValue(T(-0.0),T(+0.0));

        // if min == +0.0, max > +0.0 the output range includes +0.0, 1.0
        // and there are no any other values between these points
        // if max == -0.0 and min < -0.0 the output range includes -1.0, -0.0
        // and there are no any other values between these points
        // if min < -0.0 and max > 0.0 he output range includes
        // points -1.0, -0.0, +0.0, 1.0, but there are no any other
        // values between them, so result is unknown
        return NEATValue(NEATValue::UNKNOWN);
    }

    template<typename T>
    static NEATVector sign(const NEATVector& vec)
    {
        return processVector(vec,sign<T>);
    }
    ///////////////////////////////////////////////////////////////////////////////
    // Step built-in function
    template<typename T>
    static NEATValue step (const NEATValue& edge, const NEATValue& x) {
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(edge) || CheckAUU(x)) return NEATValue(NEATValue::UNKNOWN);

        // Flush denormalized values to zero
        NEATValue flushedX = flush<T>(x);
        NEATValue flushedEdge = flush<T>(edge);

        // if NaN, return NaN
        if (flushedEdge.IsNaN<T>() || flushedX.IsNaN<T>())
            return NEATValue::NaN<T>();

        T xMin = *flushedX.GetMin<T>();
        T xMax = *flushedX.GetMax<T>();
        // Get edge interval value: [edgeMin, edgeMax]
        T edgeMin = *flushedEdge.GetMin<T>();
        T edgeMax = *flushedEdge.GetMax<T>();

        // Returns 0.0 if x < edge, otherwise it returns 1.0.
        if (xMax < edgeMin)
            return NEATValue(T(0));
        if (xMin >= edgeMax)
            return NEATValue(T(1));
        // Here intervals intersects, so the result could be both 0 and 1.
        return NEATValue(NEATValue::UNKNOWN);
    }

    // Step function for vector arguments.
    template<typename T>
    static NEATVector step(const NEATVector& edge, const NEATVector& x)
    {
        return processVector(edge, x, step<T>);
    }

    // Step function for first scalar argument and second vector arguments.
    template<typename T>
    static NEATVector step(const NEATValue& edge, const NEATVector& x)
    {
        return processVector(edge, x, step<T>);
    }

    ///////////////////////////////////////////////////////////////////////////////
    // smoothstep built-in function
    template<typename T>
    static NEATValue smoothstep (const NEATValue& edge0, 
                                 const NEATValue& edge1,
                                 const NEATValue& x) {
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(edge0) || CheckAUU(edge1) || CheckAUU(x)) 
            return NEATValue(NEATValue::UNKNOWN);

        // Flush denormalized values to zero
        NEATValue flushedX = flush<T>(x);
        NEATValue flushedEdge0 = flush<T>(edge0);
        NEATValue flushedEdge1 = flush<T>(edge1);

        // Results are undefined if x, edge0 or edge1 is a NaN.
        if (flushedEdge0.IsNaN<T>() || flushedEdge1.IsNaN<T>() || flushedX.IsNaN<T>())
            return NEATValue(NEATValue::ANY);
       
        T edgeMin0 = *flushedEdge0.GetMin<T>();
        T edgeMax0 = *flushedEdge0.GetMax<T>();
        T edgeMin1 = *flushedEdge1.GetMin<T>();
        T edgeMax1 = *flushedEdge1.GetMax<T>();

        // Results are undefined if edge0 >= edge1
        // i.e. if the ranges are overlapped
        if ( edgeMax0 >= edgeMin1)
             return NEATValue(NEATValue::ANY);

        T xMin = *flushedX.GetMin<T>();
        T xMax = *flushedX.GetMax<T>();

        // Returns 0.0 if x <= edge0
        if (Utils::le<T>(xMax,edgeMin0))
            return NEATValue(T(0));
        // Returns 1.0 if x >= edge1
        if (Utils::ge<T>(xMin,edgeMax1))
            return NEATValue(T(1));

        if ( flushedX.IsAcc() && flushedEdge0.IsAcc() && flushedEdge1.IsAcc() )
            return NEATValue(RefALU::flush(RefALU::smoothstep(*flushedEdge0.GetAcc<T>(), *flushedEdge1.GetAcc<T>(), *flushedX.GetAcc<T>())));

        // according to openCL spec Version: 1.1 (6.11.4 Common Functions)
        // smoothstep function is equivalent to: 
        // gentype t;
        // t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
        // return t * t * (3 - 2 * t);

        // compute a = (x - edge0) / (edge1 - edge0) with no range extention
        NEATValue res = InternalDiv<T>(sub<T>(flushedX,flushedEdge0),sub<T>(flushedEdge1,flushedEdge0),0);
        // and compute clamp(a)
        res = clamp<T>(res,NEATValue(T(0)),NEATValue(T(1)));

        NEATValue res1 = mul<T>(NEATValue(T(2)),res); // 2 * t, correctly rounded
        res1 = sub<T>(NEATValue(T(3)),res1); // 3 - 2 * t, correctly rounded

        res = mul<T>(res,res); //  t * t, correctly rounded
        res = mul<T>(res,res1); // t * t * (3 - 2 * t), correctly rounded

        return res;
    }

    // smoothstep function for vector arguments.
    template<typename T>
    static NEATVector smoothstep(const NEATVector& edge0, const NEATVector& edge1, const NEATVector& vec)
    {
        return processVector(edge0, edge1, vec, smoothstep<T>);
    }

    // smoothstep function for first scalar argument and second vector arguments.
    template<typename T>
    static NEATVector smoothstep(const NEATValue& edge0, const NEATValue& edge1, const NEATVector& vec)
    {
        NEATVector toReturn(vec.GetWidth());
        size_t size = vec.GetSize();
        for(uint32_t i = 0; i < uint32_t(size); ++i)
        {
            toReturn[i] = smoothstep<T>(edge0,edge1,vec[i]);
        }
        return toReturn;
    }

    // min function
    template<typename T>
    static NEATValue min (const NEATValue& x, const NEATValue& y) {
        // OCL 1.1 rev 36 Sec 6.11.4
        // Returns y if y < x, otherwise it returns x. If x or y
        // are infinite or NaN, the return values are undefined.

        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(x) || CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);
        // if NaN or +-INF, return ANY
        if (!x.IsFinite<T>()) return NEATValue(NEATValue::ANY);
        // if NaN or +-INF, return ANY
        if (!y.IsFinite<T>()) return NEATValue(NEATValue::ANY);

        NEATValue res = NEATALU::lt<T>(x, y);
        // x overlaps y
        if (res.IsUnknown())
            return NEATValue(
                RefALU::min(*x.GetMin<T>(), *y.GetMin<T>()),
                RefALU::min(*x.GetMax<T>(), *y.GetMax<T>()));
        // x < y
        else if (*res.GetAcc<bool>() == true)
            return NEATValue(x);
        // x >= y
        else return NEATValue(y);
    }

    // min function for vector arguments
    template<typename T>
    static NEATVector min(const NEATVector& x, const NEATVector& y)
    {
        return processVector(x, y, min<T>);
    }

    // min function for vector and scalar arguments
    template<typename T>
    static NEATVector min(const NEATVector& x, const NEATValue& y)
    {
        return processVector(x, y, min<T>);
    }

    template<typename T>
    static NEATValue max (const NEATValue& x, const NEATValue& y) {
        // OCL 1.1 rev 36 Sec 6.11.4
        //  Returns y if x < y, otherwise it returns x. If x or y
        //  are infinite or NaN, the return values are undefined

        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(x) || CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);
        // if NaN or +-INF, return ANY
        if (!x.IsFinite<T>()) return NEATValue(NEATValue::ANY);
        // if NaN or +-INF, return ANY
        if (!y.IsFinite<T>()) return NEATValue(NEATValue::ANY);

        NEATValue res = NEATALU::gt<T>(x, y);
        // x overlaps y
        if (res.IsUnknown())
            return NEATValue(
                RefALU::max(*x.GetMin<T>(), *y.GetMin<T>()),
                RefALU::max(*x.GetMax<T>(), *y.GetMax<T>()));
        // x > y
        if (*res.GetAcc<bool>() == true)
            return NEATValue(x);
        // x <= y
        return NEATValue(y);
    }

    // max function for vector arguments
    template<typename T>
    static NEATVector max(const NEATVector& x, const NEATVector& y)
    {
        return processVector(x, y, max<T>);
    }

    // max function for vector and scalar arguments
    template<typename T>
    static NEATVector max(const NEATVector& x, const NEATValue& y)
    {
        return processVector(x, y, max<T>);
    }

    template<typename T>
    static NEATValue fmax (const NEATValue& x, const NEATValue& y) {
        // OCL 1.1 rev 36 Sec 6.11.2
        // Returns y if x < y, otherwise it returns x. 
        // If one argument is a NaN, fmax() returns the other argument.
        // If both arguments are NaNs, fmax() returns a NaN.

        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(x) || CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);
        if (x.IsNaN<T>())
        {
            if (y.IsNaN<T>())
                return NEATValue::NaN<T>();
            else
                return y;
        }
        if (y.IsNaN<T>())
            return x;

        NEATValue res = NEATALU::gt<T>(x, y);
        // x overlaps y
        if (res.IsUnknown())
            return NEATValue(
            RefALU::max(*x.GetMin<T>(), *y.GetMin<T>()),
            RefALU::max(*x.GetMax<T>(), *y.GetMax<T>()));
        // x > y
        if (*res.GetAcc<bool>() == true)
            return NEATValue(x);
        // x <= y
        return NEATValue(y);
    }

    template<typename T>
    static NEATVector fmax(const NEATVector& x, const NEATValue& y)
    {
        return processVector(x, y, fmax<T>);
    }

    template<typename T>
    static NEATVector fmax(const NEATVector& x, const NEATVector& y)
    {
        return processVector(x, y, fmax<T>);
    }


    template<typename T>
    static NEATValue fmin (const NEATValue& x, const NEATValue& y) {
        // OCL 1.1 rev 36 Sec 6.11.2
        // Returns y if x < y, otherwise it returns x. 
        // If one argument is a NaN, fmin() returns the other argument.
        // If both arguments are NaNs, fmin() returns a NaN.

        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(x) || CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);
        if (x.IsNaN<T>())
        {
            if (y.IsNaN<T>())
                return NEATValue::NaN<T>();
            else
                return y;
        }
        if (y.IsNaN<T>())
            return x;

        NEATValue res = NEATALU::lt<T>(y, x);
        // x overlaps y
        if (res.IsUnknown())
            return NEATValue(
            RefALU::min(*x.GetMin<T>(), *y.GetMin<T>()),
            RefALU::min(*x.GetMax<T>(), *y.GetMax<T>()));
        // y < x
        if (*res.GetAcc<bool>() == true)
            return NEATValue(y);
        // x >= y
        return NEATValue(x);
    }

    template<typename T>
    static NEATVector fmin(const NEATVector& x, const NEATValue& y)
    {
        return processVector(x, y, fmin<T>);
    }

    template<typename T>
    static NEATVector fmin(const NEATVector& x, const NEATVector& y)
    {
        return processVector(x, y, fmin<T>);
    }

    /// fabs implementation
    /// has 0 ulps. relies completely on RefALU
    template<typename T>
    static NEATValue fabs(const NEATValue& a)
    {
        typedef typename  superT<T>::type sT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        // compute boundaries for reference
        sT inp[2];
        sT val[2];
        inp[0] = *flushed.GetMin<T>();
        inp[1] = *flushed.GetMax<T>();

        if(a.Includes<T>(T(0.0)))
        {
            val[0] = +0.0;
            val[1] = RefALU::fmax(
                RefALU::fabs(inp[0]), RefALU::fabs(inp[1]));
        }
        else
        {
            val[0] = RefALU::fabs(inp[0]);
            val[1] = RefALU::fabs(inp[1]);
        }
        return ComputeResult(val, 2, float(FABS_ERROR) );
    }

    template<typename T>
    static NEATVector fabs(const NEATVector& vec)
    {
        return processVector(vec,fabs<T>);
    }


    /// floor implementation
    template<typename T>
    static NEATValue floor(const NEATValue& a)
    {
        typedef typename  superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        // compute boundaries for reference
        SuperT val[2];
        val[0] = RefALU::floor(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::floor(SuperT(*flushed.GetMax<T>()));

        /// Floor is step function.Thus when minimum is not equal to maximum
        /// answer is a set of discrete integers that cannot be represented by
        /// the single interval
        if(val[0] != val[1])
        {
            DEBUG(llvm::dbgs() << "[NEATALU::floor] There should be more than one integer value, but NEAT doesn't support such type of intervals.");
            return NEATValue(NEATValue::UNKNOWN);
        }
        else
            return NEATValue((T)val[0]); // doesn't really matter what to return. min and max are equal

    }

    template<typename T>
    static NEATVector floor(const NEATVector& vec)
    {
        return processVector(vec,floor<T>);
    }

    /// fract implementation
    template<typename T>
    static NEATValue fract(const NEATValue& a, NEATValue* iptr)
    {
        typedef typename  superT<T>::type SuperT;
        *iptr = NEATALU::floor<T>(a);

        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        if (Utils::ge<SuperT>(SuperT(*flushed.GetMax<T>()) - SuperT(*flushed.GetMin<T>()), SuperT(1)))
        {
            Utils::FloatParts<T> one(T(1));
            one.AddUlps(-1);
            return NEATValue(T(0), T(one.val()));
        }

        // compute boundaries for reference
        SuperT val[2], flr[2];
        val[0] = RefALU::fract(SuperT(*flushed.GetMin<T>()), &flr[0]);
        val[1] = RefALU::fract(SuperT(*flushed.GetMax<T>()), &flr[1]);

        if (val[0] > val[1])
            return NEATValue(NEATValue::UNKNOWN);

        NEATValue res = ComputeResult(val, 2, FRACT_ERROR);
        return res;
    }

    template<typename T>
    static NEATVector fract(const NEATVector& vec1, NEATVector& vec2)
    {
        NEATVector toReturn(vec1.GetWidth());
        for(uint32_t i = 0; i < vec1.GetSize(); ++i)
        {
            toReturn[i] = NEATALU::fract<T>(vec1[i], &vec2[i]);
        }
        return toReturn;
    }

    /// hypot implementation
    template<typename T>
    static NEATValue hypot(const NEATValue& x,const NEATValue& y)
    {
        // OCL 1.1 rev 36 Sec 6.11.4
        // Compute the value of the square root of x2+ y2
        // without undue overflow or underflow.
        typedef typename  superT<T>::type SuperT;
        NEATValue res;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(x)) return NEATValue(NEATValue::UNKNOWN);
        if(CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (x.IsNaN<T>())
            return NEATValue::NaN<T>();

        if (y.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Flush denormals to zero
        NEATValue fx = flush<T>(x);
        NEATValue fy = flush<T>(y);

        // special case
        // hack if both values are accurate and zero return exact positive zero
        // in this case ComputeULP would return -0
        // TODO: check
        if(fx.IsAcc() && *fx.GetAcc<T>() == 0.0 &&
           fy.IsAcc() && *fy.GetAcc<T>() == 0.0)
           return NEATValue(T(+0.0));

        // compute boundaries for reference
        SuperT xmin, xmax, ymin, ymax;
        SuperT xa1 = RefALU::fabs(SuperT(*fx.GetMin<T>()));
        SuperT xa2 = RefALU::fabs(SuperT(*fx.GetMax<T>()));
        SuperT ya1 = RefALU::fabs(SuperT(*fy.GetMin<T>()));
        SuperT ya2 = RefALU::fabs(SuperT(*fy.GetMax<T>()));

        // set min/max for absolute values
        xmin = RefALU::fmin(xa1, xa2);
        xmax = RefALU::fmax(xa1, xa2);

        // set min/max for absolute values
        ymin = RefALU::fmin(ya1, ya2);
        ymax = RefALU::fmax(ya1, ya2);

        // check interval contains zero
        if(fx.Includes<T>(T(0.0)))
        {   // set min boundary to zero
            xmin = 0.0;
        }

        // check interval contains zero
        if(fy.Includes<T>(T(0.0)))
        {
            // set min boundary to zero
            ymin = 0.0;
        }

        SuperT val[2];
        val[0]=RefALU::hypot(xmin, ymin);
        val[1]=RefALU::hypot(xmax, ymax);

        NEATValue intvl = ComputeResult(val, 2, HYPOT_ERROR);
        // if lower interval boundary was zero then ComputeResult have added
        // negative ulps to zero and lower boundary is negative.
        // we need to fix it and set lower boundary to exact zero
        // since hypot cannot produce negative values
        res = intvl;
        if(*intvl.GetMin<T>() <= T(0.0))
        {
            // set positive zero
            res.SetIntervalVal<T>(T(+0.0), *intvl.GetMax<T>());
        }
        assert(*res.GetMin<T>() >= 0.0);
        assert(*res.GetMax<T>() >= 0.0);
        return res;
    }

    template<typename T>
    static NEATVector hypot(const NEATVector& x, const NEATVector& y)
    {
        return processVector(x, y, hypot<T>);
    }

    template<typename T>
    static NEATValue nextafter(const NEATValue& x,const NEATValue& y)
    {
        typedef typename  superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(x)) return NEATValue(NEATValue::UNKNOWN);
        if(CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (x.IsNaN<T>())
            return NEATValue::NaN<T>();

        if (y.IsNaN<T>())
            return NEATValue::NaN<T>();

        NEATValue fx = flush<T>(x);
        NEATValue fy = flush<T>(y);

        // compute boundaries for reference
        SuperT xmin = SuperT(*fx.GetMin<T>());
        SuperT xmax = SuperT(*fx.GetMax<T>());
        SuperT ymin = SuperT(*fy.GetMin<T>());
        SuperT ymax = SuperT(*fy.GetMax<T>());

        SuperT val[4];
        val[0]=RefALU::nextafter(xmin, ymin);
        val[1]=RefALU::nextafter(xmax, ymax);
        val[2]=RefALU::nextafter(xmin, ymax);
        val[3]=RefALU::nextafter(xmax, ymin);

        NEATValue res = ComputeResult(val, 4, NEXTAFTER_ERROR);
        return res;
    }

    template<typename T>
    static NEATVector nextafter(const NEATVector& a, const NEATVector& b)
    {
        return processVector(a, b, nextafter<T>);
    }

    template<typename T>
    static NEATValue maxmag(const NEATValue& x,const NEATValue& y)
    {
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(x) || CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue fX = flush<T>(x);
        NEATValue fY = flush<T>(y);

        // if both x and y are accurate values, just return calculated result
        if( fX.IsAcc() && fY.IsAcc() )
            return NEATValue(RefALU::maxmag<T>(*fX.GetAcc<T>(),*fY.GetAcc<T>()));

        // compute boundaries for reference
        T xMin = *fX.GetMin<T>();
        T xMax = *fX.GetMax<T>();
        T yMin = *fY.GetMin<T>();
        T yMax = *fY.GetMax<T>();

        T xAbsMinimum = RefALU::fmin<T>(RefALU::fabs<T>(xMin), RefALU::fabs<T>(xMax));
        T yAbsMaximum = RefALU::fmax<T>(RefALU::fabs<T>(yMin), RefALU::fabs<T>(yMax));
        T xAbsMaximum = RefALU::fmax<T>(RefALU::fabs<T>(xMin), RefALU::fabs<T>(xMax));
        T yAbsMinimum = RefALU::fmin<T>(RefALU::fabs<T>(yMin), RefALU::fabs<T>(yMax));
        // if x is [-2;3], |x| is [0,3], not [2,3]
        if( fX.Includes(T(0.0)) || fX.Includes(T(-0.0)))
            xAbsMinimum = 0;
        if( fY.Includes(T(0.0)) || fY.Includes(T(-0.0)))
            yAbsMinimum = 0;

        // Returns x if | x | > | y |
        if( Utils::gt<T>(xAbsMinimum, yAbsMaximum))
            return fX;

        // Returns y if | y | > | x |
        if( Utils::lt<T>(xAbsMaximum, yAbsMinimum))
            return fY;

        // compute new range limits
        T min = RefALU::maxmag<T>(xMin, yMin);
        T max = RefALU::maxmag<T>(xMax, yMax);
        if (min > max) std::swap(min,max);

        // it could be a gap in output range, for example if x is [-5;-1]
        // y is [0,6], resulting range [-5;6] has a gap [-1;1], or
        // result consists of two ranges [-5;-1] and [1,6]. We return
        // UNKNOWN in this case
        T gapMax = RefALU::fmax<T>(yAbsMinimum, xAbsMinimum);
        T gapMin = -gapMax;

        if(RefALU::fabs(gapMin) != T(0.0) && gapMin > min && gapMax < max )
            return NEATValue(NEATValue::UNKNOWN);

        return NEATValue(min,max);
    }

    template<typename T>
    static NEATVector maxmag(const NEATVector& a, const NEATVector& b)
    {
        return processVector(a, b, maxmag<T>);
    }

    template<typename T>
    static NEATValue minmag(const NEATValue& x,const NEATValue& y)
    {
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(x) || CheckAUU(y)) return NEATValue(NEATValue::UNKNOWN);

        NEATValue fX = flush<T>(x);
        NEATValue fY = flush<T>(y);

        // if both x and y are accurate values, just return calculated result
        if( fX.IsAcc() && fY.IsAcc() )
            return NEATValue(RefALU::minmag<T>(*fX.GetAcc<T>(),*fY.GetAcc<T>()));

        // compute boundaries for reference
        T xMin = *fX.GetMin<T>();
        T xMax = *fX.GetMax<T>();
        T yMin = *fY.GetMin<T>();
        T yMax = *fY.GetMax<T>();

        T xAbsMinimum = RefALU::fmin<T>(RefALU::fabs<T>(xMin), RefALU::fabs<T>(xMax));
        T xAbsMaximum = RefALU::fmax<T>(RefALU::fabs<T>(xMin), RefALU::fabs<T>(xMax));
        T yAbsMaximum = RefALU::fmax<T>(RefALU::fabs<T>(yMin), RefALU::fabs<T>(yMax));
        T yAbsMinimum = RefALU::fmin<T>(RefALU::fabs<T>(yMin), RefALU::fabs<T>(yMax));
        if( fX.Includes(T(0.0)) || fX.Includes(T(-0.0)) )
            xAbsMinimum = 0;
        if( fY.Includes(T(0.0)) || fY.Includes(T(-0.0)) )
            yAbsMinimum = 0;

        // Returns x if | x | < | y |
        if( Utils::lt<T>(xAbsMaximum, yAbsMinimum))
            return fX;

        // Returns y if | y | < | x |
        if( Utils::gt<T>(xAbsMinimum, yAbsMaximum))
            return fY;

        // calculate limitation for x and y ranges
        T minAbs = RefALU::fmin(xAbsMinimum,yAbsMinimum);
        T maxAbs = RefALU::fmin(xAbsMaximum,yAbsMaximum);

        T minX = xMin, maxX = xMax;
        T minY = yMin, maxY = yMax;

        // implement limitation for x range
        T localMin = RefALU::copysign(minAbs,xMin);
        T localMax = RefALU::copysign(maxAbs,xMin);
        if (localMax<localMin) std::swap(localMax, localMin);
        if(xMin < localMin)    
            minX = localMin;

        localMin = RefALU::copysign(minAbs,xMax);
        localMax = RefALU::copysign(maxAbs,xMax);
        if (localMax<localMin) std::swap(localMax, localMin);
        if(xMax > localMax)    
            maxX = localMax;

        // implement limitation for y range
        localMin = RefALU::copysign(minAbs,yMin);
        localMax = RefALU::copysign(maxAbs,yMin);
        if (localMax<localMin) std::swap(localMax, localMin);
        if(yMin < localMin)
            minY = localMin;

        localMin = RefALU::copysign(minAbs,yMax);
        localMax = RefALU::copysign(maxAbs,yMax);
        if (localMax<localMin) std::swap(localMax, localMin);
        if(yMax > localMax)    
            maxY = localMax;

        // there are two resulting ranges, return UNKNOWN
        if( maxX < minY || maxY < minX )
            return NEATValue(NEATValue::UNKNOWN);
       
        // calculate result from limited x and y ranges
        T min = minX, max = maxX;
        if( minY < minX)
            min = minY;
        if( maxY > maxX)
            max = maxY;
        return  NEATValue(min,max);
      
    }

    template<typename T>
    static NEATVector minmag(const NEATVector& a, const NEATVector& b)
    {
        return processVector(a, b, minmag<T>);
    }

    /// @brief Fused multiplication add.
    template <typename T>
    static NEATValue fma ( const NEATValue& a, const NEATValue& b , const NEATValue& c)
    {
        typedef typename superT<T>::type SuperT;
        // Check if all arguments may have any value
        if(a.IsAny() && b.IsAny() && c.IsAny())
            // Then result is any also
            return NEATValue(NEATValue::ANY);

        if (a.IsNaN<T>() || b.IsNaN<T>() || c.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Case when all could be any was tested. Other special statuses lead to unknown result
        if(CheckAUU(a) || CheckAUU(b) || CheckAUU(c))
        {
            return NEATValue(NEATValue::UNKNOWN);
        }

        // Assume that compiler is C99 compliant and edge cases will be handled correctly

        SuperT vals[8];

        // calculating all the possible combinations
        vals[0] = RefALU::add(RefALU::mul((SuperT)*a.GetMin<T>(), (SuperT)*b.GetMax<T>()), (SuperT)*c.GetMin<T>());
        vals[1] = RefALU::add(RefALU::mul((SuperT)*a.GetMin<T>(), (SuperT)*b.GetMin<T>()), (SuperT)*c.GetMin<T>());
        vals[2] = RefALU::add(RefALU::mul((SuperT)*a.GetMax<T>(), (SuperT)*b.GetMin<T>()), (SuperT)*c.GetMin<T>());
        vals[3] = RefALU::add(RefALU::mul((SuperT)*a.GetMax<T>(), (SuperT)*b.GetMax<T>()), (SuperT)*c.GetMin<T>());
        vals[4] = RefALU::add(RefALU::mul((SuperT)*a.GetMin<T>(), (SuperT)*b.GetMax<T>()), (SuperT)*c.GetMax<T>());
        vals[5] = RefALU::add(RefALU::mul((SuperT)*a.GetMin<T>(), (SuperT)*b.GetMin<T>()), (SuperT)*c.GetMax<T>());
        vals[6] = RefALU::add(RefALU::mul((SuperT)*a.GetMax<T>(), (SuperT)*b.GetMin<T>()), (SuperT)*c.GetMax<T>());
        vals[7] = RefALU::add(RefALU::mul((SuperT)*a.GetMax<T>(), (SuperT)*b.GetMax<T>()), (SuperT)*c.GetMax<T>());

        return ComputeResult(vals, 8, NEATALU::FMA_ERROR);
    }

    template<typename T>
    static NEATVector fma(const NEATVector& vec1, const NEATVector& vec2, const NEATVector& vec3)
    {
        return processVector(vec1, vec2, vec3, fma<T>);
    }


    template<typename F, typename I>
    static NEATValue nan(const I& y)
    {
        return NEATValue::NaN<F>();
    }

    template<typename F, typename I>
    static NEATVector nan(const std::vector<I>& vec)
    {
        NEATVector toReturn(Validation::VectorWidthWrapper::ValueOf(vec.size()));
        for(size_t i = 0; i < vec.size(); ++i)
        {
            toReturn[i] = nan<F,I>(vec[i]);
        }
        return toReturn;
    }

    /// rint implementation
    template<typename T>
    static NEATValue rint(const NEATValue& a)
    {
        typedef typename  superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        // compute boundaries for reference
        SuperT val[2];
        val[0] = RefALU::rint(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::rint(SuperT(*flushed.GetMax<T>()));

        /// If minimum is not equal to maximum, the resulting set of discrete 
        /// integers cannot be represented by the single interval
        if(val[0] != val[1])
        {
            DEBUG(llvm::dbgs() << "[NEATALU::rint] There should be more than one integer value, but NEAT doesn't support such type of intervals.");
            return NEATValue(NEATValue::UNKNOWN);
        }
        else
            return NEATValue((T)val[0]); // doesn't really matter what to return. min and max are equal

    }

    template<typename T>
    static NEATVector rint(const NEATVector& vec)
    {
        return processVector(vec,rint<T>);
    }

    /// round implementation
    template<typename T>
    static NEATValue round(const NEATValue& a)
    {
        typedef typename  superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        // openCL spec Version: 1.1
        // 7.5.1 Additional Requirements Beyond C99 TC2
        // round ( -0.5 < x < 0 ) returns -0
        // this requirement is supported by reference function,
        // so we don't have to do it here

        // compute boundaries for reference
        SuperT val[2];
        val[0] = RefALU::round(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::round(SuperT(*flushed.GetMax<T>()));

        /// If minimum is not equal to maximum, the resulting set of discrete 
        /// integers cannot be represented by the single interval
        if(val[0] != val[1])
        {
            DEBUG(llvm::dbgs() << "[NEATALU::round] There should be more than one integer value, but NEAT doesn't support such type of intervals.");
            return NEATValue(NEATValue::UNKNOWN);
        }
        else
            return NEATValue((T)val[0]); // doesn't really matter what to return. min and max are equal

    }

    template<typename T>
    static NEATVector round(const NEATVector& vec)
    {
        return processVector(vec,round<T>);
    }

    /// trunc implementation
    template<typename T>
    static NEATValue trunc(const NEATValue& a)
    {
        typedef typename  superT<T>::type SuperT;
        // check for ANY,UNWRITTEN,UNKNOWN
        if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

        // if NaN, return NaN
        if (a.IsNaN<T>())
            return NEATValue::NaN<T>();

        // Flush denormals to zero
        NEATValue flushed = flush<T>(a);

        // compute boundaries for reference
        SuperT val[2];
        val[0] = RefALU::trunc(SuperT(*flushed.GetMin<T>()));
        val[1] = RefALU::trunc(SuperT(*flushed.GetMax<T>()));

        /// If minimum is not equal to maximum, the resulting set of discrete 
        /// integers cannot be represented by the single interval
        if(val[0] != val[1])
        {
            DEBUG(llvm::dbgs() << "[NEATALU::trunc] There should be more than one integer value, but NEAT doesn't support such type of intervals.");
            return NEATValue(NEATValue::UNKNOWN);
        }
        else
            return NEATValue((T)val[0]); // doesn't really matter what to return. min and max are equal

    }

    template<typename T>
    static NEATVector trunc(const NEATVector& vec)
    {
        return processVector(vec,trunc<T>);
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////             Vector Load/Store              ////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    // vloadn, vstoren, vload_half, vload_halfn, vstore_half[_rte,_rtz,_rtp,_rtn],
    // vstore_halfn[_rte,_rtz,_rtp,_rtn], vloada_halfn, vstorea_halfn[_rte,_rtz,_rtp,_rtn],

private:
    // Helper template to implement vload for all data types and vector sizes.
    template<typename T, uint32_t n>
    static NEATVector vload(size_t offset, const NEATValue* p) {
        NEATVector result(VectorWidthWrapper::ValueOf(n));
        for (unsigned int i = 0; i < n; ++i)
            result[i] = *(p+offset*n+i);
        return result;
    }

public:
    template<uint32_t n, bool aligned>
    static NEATVector vload_half(size_t offset, const uint16_t* p) {
        NEATVector result(VectorWidthWrapper::ValueOf(n));
        for (unsigned int i = 0; i < n; ++i)
            result[i].SetAccurateVal(float(CFloat16(*(p+offset*n+i))));
        return result;
    }

    static NEATValue vload_half(size_t offset, const uint16_t* p) {
        return NEATValue(float(CFloat16(*(p+offset))));
    }

#define NEAT_VSTORE_HALF_RT(__str)                                                              \
    template<typename T, uint32_t n>                                                            \
    static void vstore ##__str (const NEATVector& data, size_t offset, uint16_t* p) {           \
        for (uint32_t i = 0; i < n; ++i) {                                                      \
            /* if the status of NEAT value that is stored is UNWRITTEN or ACCURATE, */          \
            /*then NEAT can guarantee that reference value is correct, otherwise not.*/         \
            if (!data[i].IsAcc() && !data[i].IsUnwritten())                                     \
            {                                                                                   \
                throw Exception::NEATTrackFailure("[NEATALU::vstore_half] Only \"ACCURATE\","   \
                                                  " \"UNWRITTEN\" NEAT values are supported."); \
            }                                                                                   \
        }                                                                                       \
    }                                                                                           \
                                                                                                \
    template<typename T>                                                                        \
    static void vstore ##__str(const NEATValue& data, size_t offset, uint16_t* p) {             \
         /* if the status of NEAT value that is stored is UNWRITTEN or ACCURATE, */             \
        /*then NEAT can guarantee that reference value is correct, otherwise not.*/             \
        if (!data.IsAcc() && !data.IsUnwritten())                                               \
        {                                                                                       \
            throw Exception::NEATTrackFailure("[NEATALU::vstore_half] Only \"ACCURATE\","       \
                                              " \"UNWRITTEN\" NEAT values are supported.");     \
        }                                                                                       \
    }

    NEAT_VSTORE_HALF_RT(_half)
    NEAT_VSTORE_HALF_RT(a_half)
    NEAT_VSTORE_HALF_RT(_half_rte)
    NEAT_VSTORE_HALF_RT(_half_rtz)
    NEAT_VSTORE_HALF_RT(_half_rtp)
    NEAT_VSTORE_HALF_RT(_half_rtn)
    NEAT_VSTORE_HALF_RT(a_half_rte)
    NEAT_VSTORE_HALF_RT(a_half_rtz)
    NEAT_VSTORE_HALF_RT(a_half_rtp)
    NEAT_VSTORE_HALF_RT(a_half_rtn)

#undef NEAT_VSTORE_HALF_RT

    template<typename T>
    static NEATVector vload2(size_t offset, const NEATValue* p) {
        return vload<T, 2>(offset, p);
    }

    template<typename T>
    static NEATVector vload3(size_t offset, const NEATValue* p) {
        return vload<T, 3>(offset, p);
    }

    template<typename T>
    static NEATVector vload4(size_t offset, const NEATValue* p) {
        return vload<T, 4>(offset, p);
    }

    template<typename T>
    static NEATVector vload8(size_t offset, const NEATValue* p) {
        return vload<T, 8>(offset, p);
    }

    template<typename T>
    static NEATVector vload16(size_t offset, const NEATValue* p) {
        return vload<T, 16>(offset, p);
    }

private:
    // Helper template to implement vstore for all data types and vector sizes.
    template<typename T, uint32_t n>
    static void vstore(NEATVector data, size_t offset, const NEATValue* p) {
        for (unsigned int i = 0; i < n; ++i) {
            NEATValue * pData = (NEATValue *)(p+offset*n+i);
            *pData = data[i];
        }
    }
public:
    template<typename T>
    static void vstore2(NEATVector data, size_t offset, const NEATValue* p) {
        vstore<T, 2>(data, offset, p);
    }

    template<typename T>
    static void vstore3(NEATVector data, size_t offset, const NEATValue* p) {
        vstore<T, 3>(data, offset, p);
    }

    template<typename T>
    static void vstore4(NEATVector data, size_t offset, const NEATValue* p) {
        vstore<T, 4>(data, offset, p);
    }

    template<typename T>
    static void vstore8(NEATVector data, size_t offset, const NEATValue* p) {
        vstore<T, 8>(data, offset, p);
    }

    template<typename T>
    static void vstore16(NEATVector data, size_t offset, const NEATValue* p) {
        vstore<T, 16>(data, offset, p);
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////               Conversion              /////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    // convert_destType(srcType)
    template<typename T>
    static NEATValue convert_float(typename NEATType<T>::type* src) {
        IsIntegerType<T> x; UNUSED_ARGUMENT(x);
        return NEATValue((float)*src);
    }

    // Helper function to implement convert float for all data types and vector sizes.
    template<typename T, int n>
    static NEATVector convert_float(typename NEATType<T>::type* src) {
        IsIntegerType<T> x; UNUSED_ARGUMENT(x);
        NEATVector result(VectorWidthWrapper::ValueOf(n));
        for (unsigned int i = 0; i < n; ++i) {
            result[i] = NEATValue((float)src[i]);
        }
        return result;
    }

    template<typename T>
    static NEATValue convert_double(typename NEATType<T>::type* src) {
        IsIntegerType<T> x; UNUSED_ARGUMENT(x);
        return NEATValue((double)*src);
    }

    // Helper function to implement convert float for all data types and vector sizes.
    template<typename T, int n>
    static NEATVector convert_double(typename NEATType<T>::type* src) {
        IsIntegerType<T> x; UNUSED_ARGUMENT(x);
        NEATVector result(VectorWidthWrapper::ValueOf(n));
        for (unsigned int i = 0; i < n; ++i) {
            result[i] = NEATValue((double)src[i]);
        }
        return result;
    }

    /// @brief Calculate NEATVector for pixel read from raw image data. Source image is in format from OpenCL reference
    /// image data has no NEAT. It is accurate as it is stored in OCL Reference and RefALU::ImagesALU
    static NEATVector read_imagef_src_noneat(void *imageData, Conformance::image_descriptor *imageInfo,
        float x, float y, float z, Conformance::image_sampler_data *imageSampler);

    // Internal implementation of built-in functions
    protected:
        ///////////////////////////////////////////////////////////////////////
        ///////////////////////         Math            ///////////////////////
        ///////////////////////////////////////////////////////////////////////

        /// @brief Divides two neat values
        /// @param [in] dividend
        /// @param [in] divisor
        template <typename T>
        static NEATValue InternalDiv ( const NEATValue& a, const NEATValue& b, const double& ulps)
        {
            typedef typename superT<T>::type SuperT;
            // Check if both arguments may have any value
            if(a.IsAny() && b.IsAny())
                // Then result is unknown
                return NEATValue(NEATValue::UNKNOWN);

            if (a.IsNaN<T>() || b.IsNaN<T>())
                return NEATValue::NaN<T>();

            // Case when both could be any was tested. Other special statuses lead to unknown result
            if(CheckAUU(a) || (CheckAUU(b)))
            {
                return NEATValue(NEATValue::UNKNOWN);
            }

            // if a and b are not finite (i.e. NaN or Inf), the result is NaN
            if (!b.IsAcc() && (b.Includes((T)0.0) || b.Includes((T)(-0.0))) )
            {
                // if b (in a/b) has more than one allowed value
                //  and one of the values allowed is +0.0 or -0.0,
                //  then we cannot predict the result (it may be finite and may be inf)
                return NEATValue(NEATValue::UNKNOWN);
            }

            const int RES_COUNT = 8;
            SuperT val[RES_COUNT];

            val[0] = RefALU::div((SuperT)*a.GetMin<T>(),(SuperT)*b.GetMin<T>());
            val[1] = RefALU::div((SuperT)*a.GetMin<T>(),(SuperT)*b.GetMax<T>());
            val[2] = RefALU::div((SuperT)*a.GetMax<T>(),(SuperT)*b.GetMin<T>());
            val[3] = RefALU::div((SuperT)*a.GetMax<T>(),(SuperT)*b.GetMax<T>());
            val[4] = RefALU::mul((SuperT)*a.GetMin<T>(), RefALU::div((SuperT)1.0,(SuperT)*b.GetMin<T>()) );
            val[5] = RefALU::mul((SuperT)*a.GetMin<T>(), RefALU::div((SuperT)1.0,(SuperT)*b.GetMax<T>()) );
            val[6] = RefALU::mul((SuperT)*a.GetMax<T>(), RefALU::div((SuperT)1.0,(SuperT)*b.GetMin<T>()) );
            val[7] = RefALU::mul((SuperT)*a.GetMax<T>(), RefALU::div((SuperT)1.0,(SuperT)*b.GetMax<T>()) );

            return ComputeResult(val, RES_COUNT, ulps);
        }

        template<typename T>
        static NEATValue InternalSin(const NEATValue& a, double ulps)
        {
            typedef typename superT<T>::type SuperT;
            static const T MAX_POINTS[] = { (T)(pi/2.0), (T)(5.0/2.0*pi) };
            static const T MIN_POINTS[] = { (T)(3.0/2.0*pi), (T)(7.0/2.0*pi) };

            // check for ANY,UNWRITTEN,UNKNOWN
            if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

            // Flush denormals to zero
            NEATValue flushed = flush<T>(a);

            // if NaN, return NaN
            if (flushed.IsNaN<T>())
                return NEATValue::NaN<T>();

            // we now want to search if there is a min/max point between min and max
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            if (!flushed.IsAcc()) {
                // diff = max - min
                T diff = RefALU::add( *flushed.GetMax<T>(), RefALU::neg(*flushed.GetMin<T>()) );

                if ( Utils::ge(diff, (T)two_pi) )
                {
                    // surely there will be a max point and a min point in a 2*pi area
                    hasMinPoint = true;
                    hasMaxPoint = true;
                }
                else
                {
                    // we normalize the range to [0..4*pi]
                    // offStart is:  min % (2*PI)
                    T offStart = RefALU::mul<T>( RefALU::frc<T>( RefALU::div<T>( *flushed.GetMin<T>(), (T)two_pi  ) ), (T)two_pi );
                    T offEnd   = RefALU::add<T>( offStart, diff );

                    // we check if the range includes min or max points
                    NEATValue range(offStart, offEnd);
                    hasMinPoint |= range.Includes<T>( MIN_POINTS[0] );
                    hasMinPoint |= range.Includes<T>( MIN_POINTS[1] );
                    hasMaxPoint |= range.Includes<T>( MAX_POINTS[0] );
                    hasMaxPoint |= range.Includes<T>( MAX_POINTS[1] );
                }
            } else {
                //C 99 ISO/IEC 9899:TC2
                // F.9.1.6 sin(+-0) returns +-0
                if(Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0)))
                {
                    return NEATValue(T(-0.0),T(-0.0));
                }
                if(Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0)))
                {
                    return NEATValue(T(+0.0),T(+0.0));
                }
                //C 99 ISO/IEC 9899:TC2
                // F.9.1.6 sin(+-INF) returns NaN
                if((Utils::IsPInf(*flushed.GetAcc<T>())) ||
                    (Utils::IsNInf(*flushed.GetAcc<T>())))
                {
                    return NEATValue::NaN<T>();
                }
            }

            SuperT val[2];
            val[0] = RefALU::sin(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::sin(SuperT(*flushed.GetMax<T>()));

            NEATValue res = ComputeResult(val, 2, ulps);

            return NEATValue( hasMinPoint ? (T)-1.0 : *res.GetMin<T>(), hasMaxPoint ? (T)+1.0 : *res.GetMax<T>() );
        }

        template<typename T>
        static NEATValue InternalCos(const NEATValue& a, double ulps)
        {
            typedef typename superT<T>::type SuperT;
            static const T MAX_POINTS[] = { (T)(0.0), (T)(2.0*pi) };
            static const T MIN_POINTS[] = { (T)(pi), (T)(3.0*pi) };

            // check for ANY,UNWRITTEN,UNKNOWN
            if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

            NEATValue flushed = flush<T>(a);

            // if NaN, return NaN
            if (flushed.IsNaN<T>())
                return NEATValue::NaN<T>();

            // we now want to search if there is a min/max point between min and max
            bool hasMinPoint = false;
            bool hasMaxPoint = false;

            if (!flushed.IsAcc()) {
                // diff = max - min
                T diff = RefALU::add( *flushed.GetMax<T>(), RefALU::neg(*flushed.GetMin<T>()) );

                if ( Utils::ge(diff, (T)two_pi) )
                {
                    // surely there will be a max point and a min point in a 2*pi area
                    hasMinPoint = true;
                    hasMaxPoint = true;
                }
                else
                {
                    // we normalize the range to [0..4*pi]
                    // offStart is:  min % (2*pi)
                    T offStart = RefALU::mul<T>( RefALU::frc<T>( RefALU::div<T>( *flushed.GetMin<T>(), (T)two_pi  ) ), (T)two_pi );
                    T offEnd   = RefALU::add<T>( offStart, diff );

                    // we check if the range includes min or max points
                    NEATValue range(offStart, offEnd);
                    hasMinPoint |= range.Includes<T>( MIN_POINTS[0] );
                    hasMinPoint |= range.Includes<T>( MIN_POINTS[1] );
                    hasMaxPoint |= range.Includes<T>( MAX_POINTS[0] );
                    hasMaxPoint |= range.Includes<T>( MAX_POINTS[1] );
                }
            } else {
                //C 99 ISO/IEC 9899:TC2
                // F.9.1.5 cos(+-0) returns 1
                if((Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0))) ||
                    (Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0))))
                {
                    return NEATValue(T(1.0),T(1.0));
                }
                //C 99 ISO/IEC 9899:TC2
                // F.9.1.5 cos(+-INF) returns NaN
                if((Utils::IsPInf(*flushed.GetAcc<T>())) ||
                    (Utils::IsNInf(*flushed.GetAcc<T>())))
                {
                    return NEATValue::NaN<T>();
                }
            }

            SuperT val[2];

            val[0] = RefALU::cos(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::cos(SuperT(*flushed.GetMax<T>()));

            NEATValue res = ComputeResult(val, 2, ulps);

            return NEATValue( hasMinPoint ? (T)-1.0 : *res.GetMin<T>(), hasMaxPoint ? (T)+1.0 : *res.GetMax<T>() );
        }

        //TODO: it is not clear what should be output range if input range includes
        // n*pi/2, because tan(pi/2) == INF, so current implementation is correct for
        // range [-pi/2;pi/2] only
        template<typename T>
        static NEATValue InternalTan(const NEATValue& a, const double& ulps)
        {
            typedef typename superT<T>::type SuperT;
            // check for ANY,UNWRITTEN,UNKNOWN
            if(CheckAUU(a)) return NEATValue(NEATValue::UNKNOWN);

            NEATValue flushed = flush<T>(a);

            // if NaN, return NaN
            if (flushed.IsNaN<T>())
                return NEATValue::NaN<T>();

            if (flushed.IsAcc())
            {
                //C 99 ISO/IEC 9899:TC2
                // F.9.1.7 tan(+-0) returns +-0
                if(Utils::eq(*flushed.GetAcc<T>(), (T)(-0.0)))
                {
                    return NEATValue(T(-0.0),T(-0.0));
                }
                if(Utils::eq(*flushed.GetAcc<T>(), (T)(+0.0)))
                {
                    return NEATValue(T(+0.0),T(+0.0));
                }
                //C 99 ISO/IEC 9899:TC2
                // F.9.1.7 tan(+-INF) returns NaN
                if((Utils::IsPInf(*flushed.GetAcc<T>())) ||
                    (Utils::IsNInf(*flushed.GetAcc<T>())))
                {
                    return NEATValue::NaN<T>();
                }
            }

            SuperT val[2];

            val[0] = RefALU::tan(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::tan(SuperT(*flushed.GetMax<T>()));

            NEATValue res = ComputeResult(val, 2, ulps);
            return res;
        }

        template<typename T>
        static NEATValue InternalSqrt(const NEATValue& a, const double& ulps)
        {
            typedef typename superT<T>::type SuperT;
            if(CheckAUU(a))
                return NEATValue(NEATValue::UNKNOWN);

            NEATValue flushed = flush<T>(a);

            if (flushed.IsNaN<T>())
            {
                return NEATValue::NaN<T>();
            }

            if (flushed.IsAcc())
            {
                if(*flushed.GetAcc<T>() < 0)
                    return NEATValue::NaN<T>();

                if(*flushed.GetAcc<T>() == 0)
                    return NEATValue(T(0),T(0));
            }

            SuperT val[2];
            val[0] = RefALU::sqrt(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::sqrt(SuperT(*flushed.GetMax<T>()));

            return ComputeResult(val, 2, ulps);
        }

        template<typename T>
        static NEATValue InternalRsqrt(const NEATValue& a, double ulps)
        {
            typedef typename superT<T>::type SuperT;

            if(CheckAUU(a))
                return NEATValue(NEATValue::UNKNOWN);

            NEATValue flushed = flush<T>(a);

            if (flushed.IsNaN<T>())
            {
                return NEATValue::NaN<T>();
            }

            if (flushed.IsAcc())
            {
                if(*flushed.GetAcc<T>() <= 0)
                    return NEATValue::NaN<T>();
            }

            SuperT val[2];
            val[0] = RefALU::rsqrt(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::rsqrt(SuperT(*flushed.GetMax<T>()));

            return ComputeResult(val, 2, ulps);
        }

        template<typename T>
        static NEATValue InternalExp(const NEATValue& a, double ulps)
        {
            typedef typename superT<T>::type SuperT;
            /// Check for ANY, UNWRITTEN, UNKNOWN
            if(CheckAUU(a))
                return NEATValue(NEATValue::UNKNOWN);

            NEATValue flushed = flush<T>(a);

            if(a.IsNaN<T>())
                return NEATValue::NaN<T>();

            if(flushed.IsAcc())
            {
                /// C99 ISO/IEC 9899 TC2
                /// exp(+inf) = +inf
                if(Utils::IsPInf<T>(*flushed.GetAcc<T>()))
                    return NEATValue(Utils::GetPInf<T>());
                /// exp(-inf) = +0
                if(Utils::IsNInf<T>(*flushed.GetAcc<T>()))
                    return NEATValue((T)0.0);
                /// exp(+-0) = 1
                if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)-0.0) || Utils::eq<T>(*flushed.GetAcc<T>(), (T)+0.0))
                    return NEATValue((T)1.0);
            }

            SuperT val[2];

            val[0] = RefALU::exp(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::exp(SuperT(*flushed.GetMax<T>()));

            return ComputeResult(val, 2, ulps);
        }

        template<typename T>
        static NEATValue InternalExp2(const NEATValue& a, double ulps)
        {
            typedef typename superT<T>::type SuperT;
            /// Check for ANY, UNWRITTEN, UNKNOWN
            if(CheckAUU(a))
                return NEATValue(NEATValue::UNKNOWN);

            NEATValue flushed = flush<T>(a);

            if(a.IsNaN<T>())
                return NEATValue::NaN<T>();

            if(flushed.IsAcc())
            {
                /// C99 ISO/IEC 9899 TC2
                /// exp(+inf) = +inf
                if(Utils::IsPInf<T>(*flushed.GetAcc<T>()))
                    return NEATValue(Utils::GetPInf<T>());
                /// exp(-inf) = +0
                if(Utils::IsNInf<T>(*flushed.GetAcc<T>()))
                    return NEATValue((T)0.0);
                /// exp(+-0) = 1
                if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)-0.0) || Utils::eq<T>(*flushed.GetAcc<T>(), (T)+0.0))
                    return NEATValue((T)1.0);
            }

            SuperT val[2];

            val[0] = RefALU::exp2(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::exp2(SuperT(*flushed.GetMax<T>()));

            return ComputeResult(val, 2, ulps);
        }

        template<typename T>
        static NEATValue InternalExp10(const NEATValue& a, double ulps)
        {
            typedef typename superT<T>::type SuperT;
            /// Check for ANY, UNWRITTEN, UNKNOWN
            if(CheckAUU(a))
                return NEATValue(NEATValue::UNKNOWN);

            NEATValue flushed = flush<T>(a);

            if(a.IsNaN<T>())
                return NEATValue::NaN<T>();

            if(flushed.IsAcc())
            {
                // openCL spec Version: 1.1
                // 7.5.1 Additional Requirements Beyond C99 TC2
                if(Utils::IsPInf<T>(*flushed.GetAcc<T>()))
                    return NEATValue(Utils::GetPInf<T>());
                /// exp(-inf) = +0
                if(Utils::IsNInf<T>(*flushed.GetAcc<T>()))
                    return NEATValue((T)0.0);
                /// exp(+-0) = 1
                if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)-0.0) || Utils::eq<T>(*flushed.GetAcc<T>(), (T)+0.0))
                    return NEATValue((T)1.0);
            }

            SuperT val[2];

            val[0] = RefALU::exp10(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::exp10(SuperT(*flushed.GetMax<T>()));

            return ComputeResult(val, 2, ulps);
        }

        template<typename T>
        static NEATValue InternalLog2(const NEATValue& a, double ulps)
        {
            typedef typename superT<T>::type SuperT;
            // Check for ANY, UNWRITTEN, UNKNOWN
            if(CheckAUU(a))
                return NEATValue(NEATValue::UNKNOWN);

            if(a.IsNaN<T>())
                return NEATValue::NaN<T>();

            NEATValue flushed = flush<T>(a);

            if(flushed.IsAcc())
            {
                /// C99 corner cases
                /// log2(+-0) = -inf
                if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)+0.0) || Utils::eq<T>(*flushed.GetAcc<T>(), (T)-0.0))
                    return NEATValue(Utils::GetNInf<T>());
                /// log2(1) = +0
                if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)1.0))
                    return NEATValue((T)0.0);
                /// log2(x < 0) = NaN
                if(Utils::lt<T>(*flushed.GetAcc<T>(), (T)0.0))
                    return NEATValue::NaN<T>();
                /// log2(+inf) = +inf
                if(Utils::IsPInf<T>(*flushed.GetAcc<T>()))
                    return NEATValue(Utils::GetPInf<T>());
            }

            SuperT val[2];

            val[0] = RefALU::log2(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::log2(SuperT(*flushed.GetMax<T>()));

            return ComputeResult(val, 2, ulps);
        }

        template<typename T>
        static NEATValue InternalLog(const NEATValue& a, double ulps)
        {
            typedef typename superT<T>::type SuperT;
            // Check for ANY, UNWRITTEN, UNKNOWN
            if(CheckAUU(a))
                return NEATValue(NEATValue::UNKNOWN);

            if(a.IsNaN<T>())
                return NEATValue::NaN<T>();

            NEATValue flushed = flush<T>(a);

            if(flushed.IsAcc())
            {
                /// log(+-0) = -inf
                if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)+0.0) || Utils::eq<T>(*flushed.GetAcc<T>(), (T)-0.0))
                    return NEATValue(Utils::GetNInf<T>());
                /// log(1) = +0
                if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)1.0))
                    return NEATValue((T)0.0);
                /// log(x < 0) = NaN
                if(Utils::lt<T>(*flushed.GetAcc<T>(), (T)0.0))
                    return NEATValue::NaN<T>();
                /// log(+inf) = +inf
                if(Utils::IsPInf<T>(*flushed.GetAcc<T>()))
                    return NEATValue(Utils::GetPInf<T>());
            }

            SuperT val[2];

            val[0] = RefALU::log(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::log(SuperT(*flushed.GetMax<T>()));

            return ComputeResult(val, 2, ulps);
        }

        template<typename T>
        static NEATValue InternalLog10(const NEATValue& a, double ulps)
        {
            typedef typename superT<T>::type SuperT;
            // Check for ANY, UNWRITTEN, UNKNOWN
            if(CheckAUU(a))
                return NEATValue(NEATValue::UNKNOWN);

            if(a.IsNaN<T>())
                return NEATValue::NaN<T>();

            NEATValue flushed = flush<T>(a);

            if(flushed.IsAcc())
            {
                /// log10(+-0) = -inf
                if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)+0.0) || Utils::eq<T>(*flushed.GetAcc<T>(), (T)-0.0))
                    return NEATValue(Utils::GetNInf<T>());
                /// log10(1) = +0
                if(Utils::eq<T>(*flushed.GetAcc<T>(), (T)1.0))
                    return NEATValue((T)0.0);
                /// log10(x < 0) = NaN
                if(Utils::lt<T>(*flushed.GetAcc<T>(), (T)0.0))
                    return NEATValue::NaN<T>();
                /// log10(+inf) = +inf
                if(Utils::IsPInf<T>(*flushed.GetAcc<T>()))
                    return NEATValue(Utils::GetPInf<T>());
            }

            SuperT val[2];

            val[0] = RefALU::log10(SuperT(*flushed.GetMin<T>()));
            val[1] = RefALU::log10(SuperT(*flushed.GetMax<T>()));

            return ComputeResult(val, 2, ulps);
        }

        template<typename T>
        static NEATValue InternalPow(const NEATValue& x, const NEATValue& y, const double& ulps)
        {
            typedef typename superT<T>::type SuperT;
            /// C99 ISO/IEC 9899 TC2
            /// [C99] 6
            /// pow(+1.0, y) returns 1 for any y, even NaN
            if(x.IsAcc() && Utils::eq<T>(*x.GetAcc<T>(), (T)1.0))
                return NEATValue((T)1.0);
            /// pow(x, +-0) returns 1 for any x, even NaN
            /// [C99] 7
            if(y.IsAcc() && (Utils::eq(*y.GetAcc<T>(),(T)+0.0) || Utils::eq(*y.GetAcc<T>(), (T)-0.0)))
                return NEATValue((T)1.0);
            /// Check for ANY, UNWRITTEN, UNKNOWN
            if(CheckAUU(x) || CheckAUU(y))
                return NEATValue(NEATValue::UNKNOWN);

            NEATValue xFlushed = flush<T>(x);
            NEATValue yFlushed = flush<T>(y);

            if(x.IsNaN<T>() || y.IsNaN<T>())
                return NEATValue::NaN<T>();
            T xMin = *xFlushed.GetMin<T>();
            T xMax = *xFlushed.GetMax<T>();
            T yMin = *yFlushed.GetMin<T>();
            T yMax = *yFlushed.GetMax<T>();
            NEATValue res;
            /// check for accurate value that are compliant with C99
            /// Cases 6 and 7 of C99 were already check. Check others
            bool isSpecial = PowEdgeCases<T>(xMin, xMax, yMin, yMax, res);
            if(isSpecial)
                return res;
            if((xMin <= 0.0) && (ContainsOdd(yMin, yMax)))
            {
                return NEATValue(NEATValue::UNKNOWN);
            }

            SuperT val[4];

            val[0] = RefALU::pow(SuperT(xMin), SuperT(yMin));
            val[1] = RefALU::pow(SuperT(xMin), SuperT(yMax));
            val[2] = RefALU::pow(SuperT(xMax), SuperT(yMin));
            val[3] = RefALU::pow(SuperT(xMax), SuperT(yMax));

            return ComputeResult(val, 4, ulps);
        }

        template<typename T>
        static NEATValue InternalPowr(const NEATValue& x, const NEATValue& y, const double& ulps)
        {
            typedef typename superT<T>::type SuperT;
            /// Check for ANY, UNWRITTEN, UNKNOWN
            if(CheckAUU(x) || CheckAUU(y))
                return NEATValue(NEATValue::UNKNOWN);

            // First check special case from OpenCL specs: 7.5.1 Additional Requirements Beyond C99 TC2
            // powr ( ±0, ±0 ) returns NaN.
            if(y.IsAcc() && (Utils::eq<T>(*y.GetAcc<T>(), T(0.0)) || Utils::eq<T>(*y.GetAcc<T>(), T(-0.0)))) {
                if(x.IsAcc() && (Utils::eq<T>(*x.GetAcc<T>(), (T)0.0) || Utils::eq<T>(*x.GetAcc<T>(), (T)-0.0)))
                    return NEATValue::NaN<T>();
            // powr ( x, ±0 ) is 1 for finite x > 0.
                if(x.IsFinite<T>() && Utils::gt<T>(*x.GetMin<T>(), T(0.0)))
                   return NEATValue((T)1.0);
            // powr ( +Inf, ±0 ) returns NaN.
                if(x.IsAcc() && Utils::eq<T>(*x.GetAcc<T>(), Utils::GetPInf<T>()))
                    return NEATValue::NaN<T>();
            }

            // powr ( ±0, y ) is +Inf for finite y < 0.
            if(x.IsAcc() && (Utils::eq<T>(*x.GetAcc<T>(), (T)0.0) || Utils::eq<T>(*x.GetAcc<T>(), (T)-0.0))) {
                if(y.IsFinite<T>() && Utils::lt<T>(*y.GetMax<T>(), T(0.0)))
                    return NEATValue(Utils::GetPInf<T>());
            // powr ( ±0, -Inf) is +Inf.
                if(y.IsAcc() && Utils::eq<T>(*y.GetAcc<T>(), Utils::GetNInf<T>()))
                    return NEATValue(Utils::GetPInf<T>());
            // powr ( ±0, y ) is +0 for y > 0.
                if(y.IsFinite<T>() && Utils::gt<T>(*y.GetMin<T>(), T(0.0)))
                    return NEATValue(T(+0.0));
            }

            // powr ( +1, y ) is 1 for finite y.
            if(x.IsAcc() && Utils::eq<T>(*x.GetAcc<T>(), (T)+1.0)) {
                if(y.IsFinite<T>())
                    return NEATValue(T(1));
            // powr ( +1, ±Inf ) returns NaN.
                if(y.IsAcc() && (Utils::eq<T>(*y.GetAcc<T>(), Utils::GetPInf<T>()) || Utils::eq<T>(*y.GetAcc<T>(), Utils::GetNInf<T>())))
                    return NEATValue::NaN<T>();
            }

            // powr ( x, y ) returns NaN for x < 0.
            if(Utils::lt<T>(*x.GetMax<T>(), (T)0.0))
                return NEATValue::NaN<T>();

            // powr ( x, NaN ) returns the NaN for x >= 0.
            if(Utils::ge<T>(*x.GetMin<T>(), (T)0.0) && y.IsNaN<T>())
                return NEATValue::NaN<T>();
            // powr ( NaN, y ) returns the NaN.
            if(x.IsNaN<T>())
                return NEATValue::NaN<T>();

            /// C99 ISO/IEC 9899 TC2
            /// [C99] 6
            /// powr(+1.0, y) returns 1 for any y, even NaN
            if(x.IsAcc() && Utils::eq<T>(*x.GetAcc<T>(), (T)1.0))
                return NEATValue((T)1.0);
            /// powr(x, +-0) returns 1 for any x, even NaN
            /// [C99] 7
            if(y.IsAcc() && (Utils::eq(*y.GetAcc<T>(),(T)+0.0) || Utils::eq(*y.GetAcc<T>(),(T)-0.0)))
                return NEATValue((T)1.0);

            NEATValue xFlushed = flush<T>(x);
            NEATValue yFlushed = flush<T>(y);

            if(x.IsNaN<T>() || y.IsNaN<T>())
                return NEATValue::NaN<T>();
            T xMin = *xFlushed.GetMin<T>();
            T xMax = *xFlushed.GetMax<T>();
            T yMin = *yFlushed.GetMin<T>();
            T yMax = *yFlushed.GetMax<T>();
            NEATValue res;
            /// check for accurate value that are compliant with C99
            /// Cases 6 and 7 of C99 were already check. Check others
            bool isSpecial = PowEdgeCases<T>(xMin, xMax, yMin, yMax, res);
            if(isSpecial)
                return res;
            if((xMin <= 0.0) && (ContainsOdd(yMin, yMax)))
            {
                return NEATValue(NEATValue::UNKNOWN);
            }

            SuperT val[4];

            val[0] = RefALU::powr(SuperT(xMin), SuperT(yMin));
            val[1] = RefALU::powr(SuperT(xMin), SuperT(yMax));
            val[2] = RefALU::powr(SuperT(xMax), SuperT(yMin));
            val[3] = RefALU::powr(SuperT(xMax), SuperT(yMax));

            return ComputeResult(val, 4, ulps);
        }

        template<typename T>
        static NEATValue InternalPown(const NEATValue& x, const int32_t& y, const double& ulps)
        {
            typedef typename superT<T>::type SuperT;
    
            /// Check for ANY, UNWRITTEN, UNKNOWN
            if(CheckAUU(x))
                return NEATValue(NEATValue::UNKNOWN);
    
            /// 7.5.1 Additional Requirements Beyond C99 TC2
            /// pown(x, 0) returns 1 for any x, even NaN or inf
            if(y == 0)
                return NEATValue((T)1.0);

            NEATValue flushed = flush<T>(x);

            if(flushed.IsAcc())
            {
                T tmp;
                memcpy (&tmp, flushed.GetAcc<T>(), sizeof(T));
                if (Utils::eq(tmp,T(+0.0)) || Utils::eq(tmp, (T)-0.0))
                {
                    if(y < 0)  {
                        if( y&1 ) {
                            if (Utils::eq(tmp,T(-0.0)))
                                //pown ( +-0,  n ) is +-inf for odd n < 0
                                return NEATValue(Utils::GetNInf<T>());
                            else
                                return NEATValue(Utils::GetPInf<T>());
                        } else {
                            //pown ( +-0,  n ) is +inf for even n < 0
                            return NEATValue(Utils::GetPInf<T>());
                        }
                    } else {
                        if( y&1 ) {
                            //pown ( +-0,  n ) is +-0 for odd n > 0                        
                            return flushed;
                        } else {
                            //pown ( +-0,  n ) is +0 for even n > 0
                            return NEATValue(T(+0.0));
                        }
                    }
                }
            }

            if(flushed.IsNaN<T>())
                return NEATValue::NaN<T>();
    
            T xMin = *flushed.GetMin<T>();
            T xMax = *flushed.GetMax<T>();
     
            NEATValue res;
    
            SuperT val[4];
    
            val[0] = RefALU::pown(SuperT(xMin), y);
            val[1] = RefALU::pown(SuperT(xMax), y);
            val[2] = val[0]; // additional values to enlarge the result interval
            val[3] = val[0]; // if edge values are included in source interval
    
            if( y == 0)
            /// 7.5.1 Additional Requirements Beyond C99 TC2
            /// pown(x, 0) returns 1 for any x, even NaN or inf
                return NEATValue((T)1.0);
            else if( y < 0) {
                if( y&1 ) {
                    // pown ( +-0,  n ) is +-inf for odd n < 0
                    if(flushed.Includes(T(-0.0)))
                        val[2] = Utils::GetNInf<T>();
                    if(flushed.Includes(T(+0.0)))
                        val[3] = Utils::GetPInf<T>();
                } else {
                    // pown ( +-0,  n ) is +inf for even n < 0
                    if(flushed.Includes(T(0.0)))
                        val[3] = Utils::GetPInf<T>();
                }
            } else {
                if( y&1 ) {
                    //pown ( +-0,  n ) is +-0 for odd n > 0
                    if(flushed.Includes(T(-0.0)))
                        val[2] = T(-0.0);
                    if(flushed.Includes(T(+0.0)))
                        val[3] = T(+0.0);
                } else {
                    //pown ( +-0,  n ) is +0 for even n > 0
                    if(flushed.Includes(T(+0.0)) || flushed.Includes(T(-0.0)))
                    val[3] = T(+0.0);
                }
            }

            return ComputeResult(val, 4, ulps);
        }
    };

    template<>
    NEATVector NEATALU::vload_half<3, true>(size_t offset, const uint16_t* p);

    template<>
    NEATValue NEATALU::convert_float<double>(NEATType<double>::type* src);

#define DECLARE_CONVERT_FLOAT(n)                                           \
    template<>                                                      \
    NEATVector NEATALU::convert_float<double, n>(NEATType<double>::type* src);

    DECLARE_CONVERT_FLOAT(2)
    DECLARE_CONVERT_FLOAT(3)
    DECLARE_CONVERT_FLOAT(4)
    DECLARE_CONVERT_FLOAT(8)
    DECLARE_CONVERT_FLOAT(16)

    template<>
    NEATValue NEATALU::convert_double<float>(NEATType<float>::type* src);

#define DECLARE_CONVERT_DOUBLE(n)                                           \
    template<>                                                      \
    NEATVector NEATALU::convert_double<float, n>(NEATType<float>::type* src);

    DECLARE_CONVERT_DOUBLE(2)
    DECLARE_CONVERT_DOUBLE(3)
    DECLARE_CONVERT_DOUBLE(4)
    DECLARE_CONVERT_DOUBLE(8)
    DECLARE_CONVERT_DOUBLE(16)

}
#endif // __NEATALU_H__
