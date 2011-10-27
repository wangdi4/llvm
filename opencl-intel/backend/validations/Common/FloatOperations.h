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

File Name:  FloatOperations.h

\*****************************************************************************/
#ifndef __FLOATOPERATIONS_H__
#define __FLOATOPERATIONS_H__

#include "llvm/System/DataTypes.h"      // LLVM data types
#include "dxfloat.h"
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <cstring>
#include "Exception.h"
#include <assert.h>

/// Used to prevent "Unreferenced local variable" warning.
/// It is caused by template argument restriction code described below.
#define UNUSED_ARGUMENT(x) (void)x

namespace Validation
{
    /// @brief Used to restrict function to work with floating point values only
    /// To use this trick, declare variable of type IsFloatType in the function, using
    /// function template argument type. Type definition is specified only
    /// for floating point types - float, double and CFloat16
    /// This trick causes Unreferenced local variable warning that can be disabled
    /// using UNUSED_ARGUMENT macro defined above
    template <typename T>
    class IsFloatType;

    /// Definitions for floating point types.
    template <> class IsFloatType<float> {};
    template <> class IsFloatType<double> {};
    template <> class IsFloatType<CFloat16> {};

    /// @brief Used to restrict function to work with integer values only
    template <typename T>
    class IsIntegerType;

    /// Definitions for integer types
    template<> class IsIntegerType<int8_t> {};
    template<> class IsIntegerType<uint8_t> {};
    template<> class IsIntegerType<int16_t> {};
    template<> class IsIntegerType<uint16_t> {};
    template<> class IsIntegerType<int32_t> {};
    template<> class IsIntegerType<uint32_t> {};
    template<> class IsIntegerType<int64_t> {};
    template<> class IsIntegerType<uint64_t> {};
    template<> class IsIntegerType<char> {};
#ifdef _WIN32
    template<> class IsIntegerType<long> {};
    template<> class IsIntegerType<unsigned long> {};
#else
    template<> class IsIntegerType<long long> {};
    template<> class IsIntegerType<unsigned long long> {};
#endif

    /// @brief Used to restrict function to work with scalar values only
    template <typename T>
    class IsScalarType;
    template <> class  IsScalarType<float> {};
    template <> class  IsScalarType<double> {};
    template <> class  IsScalarType<CFloat16> {};
    template <> class  IsScalarType<bool> {};
    template <> class  IsScalarType<int8_t> {};
    template <> class  IsScalarType<uint8_t> {};
    template <> class  IsScalarType<int16_t> {};
    template <> class  IsScalarType<uint16_t> {};
    template <> class  IsScalarType<int32_t> {};
    template <> class  IsScalarType<uint32_t> {};
    template <> class  IsScalarType<int64_t> {};
    template <> class  IsScalarType<uint64_t> {};


    namespace Utils
    {

        static const int SIGNIFICAND_BITS_FLOAT = 23;
        static const int SIGNIFICAND_BITS_DOUBLE = 52;

        static double AsFloat(uint64_t u);
        static float AsFloat(uint32_t u);

        template<typename T>
        struct IntStorage;

        template<>
        struct IntStorage<float>
        {
            IntStorage() {val = 0xBAADF00D; }
            IntStorage(uint32_t in_val) {val = in_val;}
            uint32_t val;
        };
        
        template<>
        struct IntStorage<double>
        {
            IntStorage() {val = 0xBAADF00DBAADF00DUL; }
            IntStorage(uint64_t in_val) {val = in_val;}
            uint64_t val;
        };

        template<typename FloatT>
        struct FloatParts
        {
            /// @brief the float's sign. True for positive
            bool sign;
            /// @brief the exponent
            IntStorage<FloatT> exp;
            /// @brief the mantissa */
            IntStorage<FloatT> mant;

            FloatParts(FloatT f)
            {
                IntStorage<FloatT> u = AsUInt(f);
                ImportFromUInt(u);
            }

            void AddUlps(long ulp)
            {
                IntStorage<FloatT> u = 0;
                u.val += exp.val << GetSignificandSize();

                // Check whether sign will be changed
                if(mant.val == 0 && exp.val == 0 && ulp < 0) {                    
                    u.val += abs(ulp);
                    u.val += GetSignMask().val;
                    sign = true;
                    ImportFromUInt(u);
                } else if(mant.val == 0 && exp.val == 0 && ulp > 0) {
                    u.val += abs(ulp);
                    sign = true;
                    ImportFromUInt(u);
                } else {

                if(((uint64_t)abs(ulp) > mant.val + (exp.val << GetSignificandSize())) && 
                //if(((uint64_t)abs(ulp) > mant.val) && 
                 (((ulp > 0) && (!sign)) || ((ulp < 0) && (sign))))
                {
                    ulp = abs((long)mant.val - abs((long)ulp));
                    mant.val = 0;
                    sign = !sign;
                }

                u.val += mant.val;
                u.val += sign ? 0 : GetSignMask().val;
                if(sign)
                    u.val += ulp;
                else
                    u.val -= ulp;
                ImportFromUInt(u);
                }
            }

            FloatT    val() const
            {
                IntStorage<FloatT> u = 0;

                u.val += exp.val << GetSignificandSize();
                assert((u.val & ~GetExpMask().val) == 0); // Make sure exp wasn't too big
                u.val += sign ? 0 : GetSignMask().val;
                u.val += mant.val;

                return AsFloat(u.val);
            }

            static IntStorage<FloatT> GetSignMask();
            static IntStorage<FloatT> GetExpMask();
            static IntStorage<FloatT> GetMantMask();
            static IntStorage<FloatT> GetMinNormalized();
            static int      GetSignificandSize();
        private:
            void ImportFromUInt(IntStorage<FloatT> u)
            {
                sign = !( u.val & GetSignMask().val );
                exp.val = (u.val & GetExpMask().val) >> GetSignificandSize();
                mant.val = u.val & GetMantMask().val;
            }
        };
        template<>
        int      FloatParts<float>::GetSignificandSize();
        template<>
        int     FloatParts<double>::GetSignificandSize();

        template<>
        IntStorage<float> FloatParts<float>::GetMinNormalized();
        template<>
        IntStorage<double> FloatParts<double>::GetMinNormalized();

        template<>
        IntStorage<float> FloatParts<float>::GetSignMask();
        template<>
        IntStorage<double> FloatParts<double>::GetSignMask();

        template<>
        IntStorage<float> FloatParts<float>::GetExpMask();
        template<>
        IntStorage<double> FloatParts<double>::GetExpMask();

        template<>
        IntStorage<float> FloatParts<float>::GetMantMask();
        template<>
        IntStorage<double> FloatParts<double>::GetMantMask();
        
        template<typename T>
        bool IsNaN(T a)
        {
            throw Exception::IllegalFunctionCall("IsNaN was called with for integer type parameter");
        }

        template<typename T>
        bool IsPInf(T a)
        {
            throw Exception::IllegalFunctionCall("IsPInf was called with for integer type parameter");
        }

        template<typename T>
        bool IsNInf(T a)
        {
            throw Exception::IllegalFunctionCall("IsNInf was called with for integer type parameter");
        }

        template<typename T>
        bool IsInf(T a)
        {
            throw Exception::IllegalFunctionCall("IsInf was called with for integer type parameter");
        }

        template<typename T>
        bool IsDenorm( T a )
        {
            throw Exception::IllegalFunctionCall("IsDenorm was called with for integer type parameter");
        }

        /// general template specialization for integer types
        /// operator ==
        template<typename T>
        bool eq(T a, T b)
        {
            IsIntegerType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            return (a == b);
        }

        /// operator >=
        template<typename T>
        bool ge(T a, T b)
        {
            IsIntegerType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            return (a >= b);
        }

        /// operator <=
        template<typename T>
        bool le(T a, T b)
        {
            IsIntegerType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            return (a <= b);
        }

        /// operator >
        template<typename T>
        bool gt(T a, T b)
        {
            IsIntegerType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            return (a > b);
        }

        /// operator <
        template<typename T>
        bool lt(T a, T b)
        {
            IsIntegerType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            return (a < b);
        }

        template<>
        bool lt(CFloat16 a, CFloat16 b);
        template<>
        bool le(CFloat16 a, CFloat16 b);
        template<>
        bool gt(CFloat16 a, CFloat16 b);
        template<>
        bool ge(CFloat16 a, CFloat16 b);
        template<>
        bool eq(CFloat16 a, CFloat16 b);
        template<>
        bool lt(float a, float b);
        template<>
        bool le(float a, float b);
        template<>
        bool gt(float a, float b);
        template<>
        bool ge(float a, float b);
        template<>
        bool eq(float a, float b);
        template<>
        bool lt(double a, double b);
        template<>
        bool le(double a, double b);
        template<>
        bool gt(double a, double b);
        template<>
        bool ge(double a, double b);
        template<>
        bool eq(double a, double b);
        template<>
        bool lt(long double a, long double b);
        template<>
        bool le(long double a, long double b);
        template<>
        bool gt(long double a, long double b);
        template<>
        bool ge(long double a, long double b);
        template<>
        bool eq(long double a, long double b);

        template<typename T>
        bool lt_float(T a, T b)
        {
            IsFloatType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            if (IsNaN(a) || IsNaN(b))
                return false;

            FloatParts<T> ap(a);
            FloatParts<T> bp(b);

            if (ap.sign != bp.sign)
            {
                // if the values are not of the same sign
                //  we should return true if a is negative and false if it is positive
                return !ap.sign;
            }
            // aSign == bSign

            if (ap.exp.val != bp.exp.val)
            {
                return ap.sign ? (ap.exp.val < bp.exp.val) : (ap.exp.val > bp.exp.val);
            }
            // aExp == bExp

            if (ap.mant.val != bp.mant.val)
            {
                return ap.sign ? (ap.mant.val < bp.mant.val) : (ap.mant.val > bp.mant.val);
            }

            // values are bitwise equal
            return false;
        }

        /// operator ==
        template<typename T>
        bool eq_float(T a, T b)
        {
            IsFloatType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            if (IsNaN(a) || IsNaN(b))
                return false;

            return !lt_float<T>(a,b) && !lt_float<T>(b,a);
        }

        /// operator >=
        template<typename T>
        bool ge_float(T a, T b)
        {
            IsFloatType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            if (IsNaN(a) || IsNaN(b))
                return false;

            return !lt_float(a,b);
        }

        /// operator <=
        template<typename T>
        bool le_float(T a, T b)
        {
            IsFloatType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            return ge_float(b,a);
        }

        /// operator >
        template<typename T>
        bool gt_float(T a, T b)
        {
            IsFloatType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            return lt_float(b,a);
        }

        ///////////////////////////////////////////////////////////////////////
        /// Constants for unique float precision values (from inteloutputerrors.h)
        /** \brief a mask of float's sign bit */
        const uint32_t FLOAT_SIGN_MASK = 0x80000000;
        ///	@brief a mask of float's exponent bits
        const uint32_t FLOAT_EXP_MASK = 0x7F800000;
        ///	@brief a mask of float's mantissa bits
        const uint32_t FLOAT_MANT_MASK = 0x007FFFFF;
        /// @brief a mask of float's minimal normalized number
        const uint32_t FLOAT_MIN_NORMALIZED = 0x00800000;
        /// @brief a mask of double's sign bits
        const uint64_t DOUBLE_SIGN_MASK = 0x8000000000000000;
        ///	@brief a mask of double's exponent bits
        const uint64_t DOUBLE_EXP_MASK = 0x7FF0000000000000;
        ///	@brief a mask of double's mantissa bits
        const uint64_t DOUBLE_MANT_MASK = 0x000FFFFFFFFFFFFF;
        /// @brief a mask of double's minimal normalized number
        const uint64_t DOUBLE_MIN_NORMALIZED = 0x0010000000000000;

        /// @brief reinterpret double as uint64_t
        inline uint64_t AsUInt(double d)
        {
            // to suppress gcc warning: dereferencing type-punned pointer will break strict-aliasing rules
            uint64_t x;
            assert(sizeof(d)==sizeof(x)); 
            memcpy(&x, &d, sizeof(d));
            return x;
        }

        /// @brief reinterpret float as uint32_t
        inline uint32_t AsUInt(float f)
        {
            // to suppress gcc warning: dereferencing type-punned pointer will break strict-aliasing rules
            uint32_t x;
           assert(sizeof(f)==sizeof(x));
	    memcpy(&x, &f, sizeof(f));
	    return x;
        }

        template<>
        bool IsNaN( float a );

        template<>
        bool IsInf( float a );

        template<>
        bool IsPInf( float a );

        template<>
        bool IsNInf( float a );

        template<>
        bool IsDenorm( float a );

        template<>
        bool IsNaN( double a );

        template<>
        bool IsInf( double a );

        template<>
        bool IsPInf( double a );

        template<>
        bool IsNInf( double a );

        template<>
        bool IsDenorm( double a );

        template<>
        bool IsNaN( long double a );

        template<>
        bool IsInf( long double a );

        template<>
        bool IsPInf( long double a );

        template<>
        bool IsNInf( long double a );

        template<>
        bool IsDenorm( long double a );

        template<>
        bool IsNaN( CFloat16 a );

        template<>
        bool IsInf( CFloat16 a );

        template<>
        bool IsPInf( CFloat16 a );

        template<>
        bool IsNInf( CFloat16 a );
        
        template<>
        bool IsDenorm( CFloat16 a );
        
        static double AsFloat(uint64_t u)
        {
            // TODO: TEST THIS CONVERSION
            // to suppress gcc warning: dereferencing type-punned pointer will break strict-aliasing rules
            double x;
            assert(sizeof(u)==sizeof(x));
            memcpy(&x, &u, sizeof(u));
            return x;
        }

        static float AsFloat(uint32_t u)
        {
            // to suppress gcc warning: dereferencing type-punned pointer will break strict-aliasing rules
            float x;
            memcpy(&x, &u, sizeof(u));
            return x;
        }

        template<typename T>
        T Get1MinusEps();

        template<>
        inline float Get1MinusEps()
        {
            return AsFloat((uint32_t)0x3F7FFFFF);
        }
        
        // TODO: test error for this function
        template<>
        inline double Get1MinusEps()
        {
            return AsFloat((uint64_t)0x3FEFFFFFFFFFFFFF);
        }

        template<typename T>
        bool IsInteger(T val)
        {
            return (fmod(val, (T)1.0) == 0);
        }
        template<typename T>
        bool IsOdd(T val)
        {
            return (fmod(fabs(val), (T)2.0) == 1.0);
        }
        template<typename T>
        bool IsEven(T val)
        {
            return (fmod(fabs(val), (T)2.0) == 0.0);
        }

        /// @brief returns negative infinity of specified type
        template<typename T>
        T GetNInf();
        /// @brief returns positive infinity of specified type
        template<typename T>
        T GetPInf();
        /// @brief returns NaN of specified type
        template<typename T>
        T GetNaN();

        /// @brief computes difference in float ULPs for reference and testVal
        /// 1 ULP is taken from reference
        double ulpsDiffSamePrecision(double reference, double testVal);
        /// @brief computes difference in double ULPs for reference and testVal
        /// 1 ULP is taken from reference
        double ulpsDiffSamePrecision( long double reference,  long double testVal );
        /// @brief computes difference in float ULPs for reference and testVal
        /// 1 ULP is taken from reference
        float ulpsDiff(double ref, float test);
        /// @brief computes difference in double ULPs for reference and testVal
        /// 1 ULP is taken from reference
        float ulpsDiff(long double ref, double test);
        /// @brief computes difference in float ULPs for reference and test
        /// test is assumed to be denormal, 1 ULP is 2**(-126-23)
        float ulpsDiffDenormal(double ref, float test);
        /// @brief computes difference in float ULPs for reference and test
        /// test is assumed to be denormal, 1 ULP is 2**(-1022-52)
        float ulpsDiffDenormal(long double ref, double test);

        template<>
        inline float GetNInf()
        { return -std::numeric_limits<float>::infinity(); }

        template<>
        inline double GetNInf()
        { return -std::numeric_limits<double>::infinity(); }

        template<>
        inline long double GetNInf()
        { return -std::numeric_limits<long double>::infinity(); }

        template<>
        inline CFloat16 GetNInf()
        { return CFloat16::GetNInf(); }

        template<>
        inline float GetPInf()
        { return std::numeric_limits<float>::infinity(); }

        template<>
        inline double GetPInf()
        { return std::numeric_limits<double>::infinity(); }

        template<>
        inline long double GetPInf()
        { return std::numeric_limits<long double>::infinity(); }

        template<>
        inline CFloat16 GetPInf()
        { return CFloat16::GetPInf(); }

        template<>
        inline float GetNaN()
        {   return std::numeric_limits<float>::quiet_NaN(); }

        template<>
        inline double GetNaN()
        {   return std::numeric_limits<double>::quiet_NaN(); }
        
        template<>
        inline long double GetNaN()
        {   return std::numeric_limits<long double>::quiet_NaN(); }

        template<>
        inline CFloat16 GetNaN()
        {   return CFloat16::GetNaN(); }

        /// "Super" precision types for floating point type
        template<typename T>
        class superT;

        /// float is "super" format for CFloat16
        template<>
        class superT<CFloat16>{
        public:
            typedef float type;
        };

        /// double is "super" format for floats
        template<>
        class superT<float>{
        public:
            typedef double type;
        };
        
        /// long double is "super" format for double
        template<>
        class superT<double>{
        public:
            typedef long double type ;
        };

        // checks if floating point values are equal within tolerance range
        // TODO: enable for CFloat16
        template<typename T>
        inline bool eq_tol(const T& a, const T& b, const double& tol)
        {
            IsFloatType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            typedef typename superT<T>::type SuperT;
            assert(tol >= 0.0);
            return (::fabs(Utils::ulpsDiffSamePrecision((SuperT) a, (SuperT)b)) <= tol);
        }

        /// TODO: enable double with tolerance when Utils:: functions are done for long doubles
        ///  !!!!!!! HACK. Currently eq_tol works as eq. Since long doubles are not supported Visual studio
        /// LONG double specialization
        template<>
        inline bool eq_tol<double>(const double& a, const double& b, const double&)
        {
            // TODO: Enable for long doubles
            return Utils::eq( a, b);
        }
        ///  !!! HACK. Currently eq_tol works as eq.
        /// CFloat16 specialization
        template<>
        inline bool eq_tol<CFloat16>(const CFloat16& a, const CFloat16& b, const double&)
        {
            // TODO: Enable for CFloat16 when ulpsDiffSamePrecision works with CFloat16
            return Utils::eq( a, b);
        }

    } 

} // End of Validation namespace
#endif // __UTILS_H__
