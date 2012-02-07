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

File Name:  FloatOperations.cpp

\*****************************************************************************/
#include "FloatOperations.h"
#include "Conformance/reference_math.h"

namespace Validation
{
    namespace Utils
    {
        template<>
        IntStorage<float> FloatParts<float>::GetMinNormalized() { return FLOAT_MIN_NORMALIZED; }
        template<>
        IntStorage<double> FloatParts<double>::GetMinNormalized() {return DOUBLE_MIN_NORMALIZED; }

        template<>
        IntStorage<float> FloatParts<float>::GetSignMask()  { return FLOAT_SIGN_MASK; }
        template<>
        IntStorage<double> FloatParts<double>::GetSignMask() { return DOUBLE_SIGN_MASK; }

        template<>
        IntStorage<float> FloatParts<float>::GetExpMask() { return FLOAT_EXP_MASK; }
        template<>
        IntStorage<double> FloatParts<double>::GetExpMask() { return DOUBLE_EXP_MASK; }

        template<>
        IntStorage<float> FloatParts<float>::GetMantMask() { return FLOAT_MANT_MASK; }
        template<>
        IntStorage<double> FloatParts<double>::GetMantMask() { return DOUBLE_MANT_MASK; }

        template<>
        int     FloatParts<float>::GetSignificandSize() { return SIGNIFICAND_BITS_FLOAT; }
        template<>
        int     FloatParts<double>::GetSignificandSize() { return SIGNIFICAND_BITS_DOUBLE; }

        /// Define comparison functions for CFloat16 type
#define DEFINE_CFLOAT16_CMP(FUNC)\
        template<>\
        bool FUNC(CFloat16 a, CFloat16 b)\
        {\
            float aFloat = a;\
            float bFloat = b;\
            return FUNC##_float(aFloat, bFloat);\
        }
        DEFINE_CFLOAT16_CMP(lt)
        DEFINE_CFLOAT16_CMP(le)
        DEFINE_CFLOAT16_CMP(gt)
        DEFINE_CFLOAT16_CMP(ge)
        DEFINE_CFLOAT16_CMP(eq)

        /// Define comparison functions for float and double
#define DEFINE_FLOAT_CMP(FUNC)\
        template<>\
        bool FUNC(float a, float b) { return FUNC##_float(a, b); }\
        template<>\
        bool FUNC(double a, double b) { return FUNC##_float(a,b); }

        DEFINE_FLOAT_CMP(lt)
        DEFINE_FLOAT_CMP(le)
        DEFINE_FLOAT_CMP(gt)
        DEFINE_FLOAT_CMP(ge)
        DEFINE_FLOAT_CMP(eq)

        template<>
        bool lt(long double a, long double b)
        {
            throw Exception::NotImplemented("lt function was called for long doubles. Long doubles comparison is not supported yet");
        }

        template<>
        bool le(long double a, long double b)
        {
            throw Exception::NotImplemented("le function was called for long doubles. Long doubles comparison is not supported yet");
        }

        template<>
        bool gt(long double a, long double b)
        {
            throw Exception::NotImplemented("gt function was called for long doubles. Long doubles comparison is not supported yet");
        }

        template<>
        bool ge(long double a, long double b)
        {
            throw Exception::NotImplemented("ge function was called for long doubles. Long doubles comparison is not supported yet");
        }

        template<>
        bool eq(long double a, long double b)
        {
            throw Exception::NotImplemented("eq function was called for long doubles. Long doubles comparison is not supported yet");
        }

        template<>
        bool IsNaN( float a )
        {
            uint32_t u = AsUInt(a);
            return ( ( ( u & FLOAT_EXP_MASK ) == FLOAT_EXP_MASK ) && ( u & FLOAT_MANT_MASK ) );
        }

        template<>
        bool IsInf( float a )
        {
            uint32_t u = AsUInt(a);
            return ( ( ( u & FLOAT_EXP_MASK ) == FLOAT_EXP_MASK ) && ( (u & FLOAT_MANT_MASK) == 0) );
        }

        template<>
        bool IsPInf( float a )
        {
            uint32_t ahex = AsUInt(a);
            return (ahex == 0x7F800000);
        }

        template<>
        bool IsNInf( float a )
        {
            uint32_t ahex = AsUInt(a);
            return (ahex == 0xFF800000);
        }

        template<>
        bool IsDenorm( float a )
        {
            uint32_t u = AsUInt(a);
            return ((u & FLOAT_EXP_MASK) == 0) && ((u & FLOAT_MANT_MASK) != 0);
        }

        template<>
        bool IsNaN( double a )
        {
            uint64_t l = AsUInt(a);
            return ( ( ( l & DOUBLE_EXP_MASK ) == DOUBLE_EXP_MASK ) && ( l & DOUBLE_MANT_MASK ) );
        }

        template<>
        bool IsInf( double a )
        {
            uint64_t l = AsUInt(a);
            return ( ( ( l & DOUBLE_EXP_MASK ) == DOUBLE_EXP_MASK ) && ( (l & DOUBLE_MANT_MASK) == 0) );
        }

        template<>
        bool IsPInf( double a )
        {
            uint64_t ahex = AsUInt(a);
            return (ahex == 0x7FF0000000000000);
        }

        template<>
        bool IsNInf( double a )
        {
            uint64_t ahex = AsUInt(a);
            return (ahex == 0xFFF0000000000000);
        }

        template<>
        bool IsDenorm( double a )
        {
            uint64_t l = AsUInt(a);
            return ((l & DOUBLE_EXP_MASK) == 0) && ((l & DOUBLE_MANT_MASK) != 0);
        }

        template<>
        bool IsNaN( long double a )
        {
            throw Exception::NotImplemented("IsNaN for long double");
        }

        template<>
        bool IsInf( long double a )
        {
               throw Exception::NotImplemented("IsInf for long double");
        }

        template<>
        bool IsPInf( long double a )
        {
           throw Exception::NotImplemented("IsPInf for long double");
        }

        template<>
        bool IsNInf( long double a )
        {
           throw Exception::NotImplemented("IsNInf for long double");
        }

        template<>
        bool IsDenorm( long double a )
        {
            throw Exception::NotImplemented("IsDenorm for long double");
        }

        template<>
        bool IsNaN( CFloat16 a )
        {
            return a.IsNaN();
        }

        template<>
        bool IsInf( CFloat16 a )
        {
            return (a.IsPInf() && a.IsNInf());
        }

        template<>
        bool IsPInf( CFloat16 a )
        {
            return a.IsPInf();
        }

        template<>
        bool IsNInf( CFloat16 a )
        {
            return a.IsNInf();
        }

        template<>
        bool IsDenorm( CFloat16 a )
        {
            return a.IsDenorm();
        }

        double ulpsDiffSamePrecision(double reference, double testVal)
        {
            union{ double d; uint64_t u; }u;     
            u.d = reference;

            // Note: This function presumes that someone has 
            // already tested whether the result is correctly
            // rounded before calling this function.  That test:
            //
            //    if( (float) reference == test )
            //        return 0.0f;
            //
            // would ensure that cases like fabs(reference) > FLT_MAX are weeded out before we get here.
            // Otherwise, we'll return inf ulp error here, for what are otherwise correctly rounded
            // results.


            if( IsInf( reference ) )
            {
                if( testVal == reference )
                    return 0.0f;

                return (float) (testVal - reference );
            }

            if( IsInf( testVal) )
            { // infinite test value, but finite (but possibly overflowing in float) reference.
                //
                // The function probably overflowed prematurely here. Formally, the spec says this is
                // an infinite ulp error and should not be tolerated. Unfortunately, this would mean
                // that the internal precision of some half_pow implementations would have to be 29+ bits
                // at half_powr( 0x1.fffffep+31, 4) to correctly determine that 4*log2( 0x1.fffffep+31 )
                // is not exactly 128.0. You might represent this for example as 4*(32 - ~2**-24), which
                // after rounding to single is 4*32 = 128, which will ultimately result in premature
                // overflow, even though a good faith representation would be correct to within 2**-29
                // interally.

                // In the interest of not requiring the implementation go to extraordinary lengths to
                // deliver a half precision function, we allow premature overflow within the limit
                // of the allowed ulp error. Towards, that end, we "pretend" the test value is actually
                // 2**128, the next value that would appear in the number line if float had sufficient range.
                testVal = Conformance::copysign( ldexp( (double)(0x1LL), 128), testVal );


                // Note that the same hack may not work in long double, which is not guaranteed to have
                // more range than double.  It is not clear that premature overflow should be tolerated for
                // double.
            }

            if( u.u & 0x000fffffffffffffULL )
            { // Non-power of two and NaN
                if( IsNaN( reference ) && IsNaN( testVal ) )
                    return 0.0f;    // if we are expecting a NaN, any NaN is fine

                // The unbiased exponent of the ulp unit place
                int ulp_exp = FLT_MANT_DIG - 1 - std::max( Conformance::ilogb( reference), FLT_MIN_EXP-1 );

                // Scale the exponent of the error
                return (float) ldexp( testVal - reference, ulp_exp );
            }

            // reference is a normal power of two or a zero
            // The unbiased exponent of the ulp unit place
            int ulp_exp =  FLT_MANT_DIG - 1 - std::max( Conformance::ilogb( reference) - 1, FLT_MIN_EXP-1 );

            // Scale the exponent of the error
            return /*(float)*/ ldexp( testVal - reference, ulp_exp );
        }

        double ulpsDiffSamePrecision( long double reference,  long double testVal )
        {
            //Check for Non-power-of-two and NaN
            // head to tail 128-bit long double is appropriate here.
            // Simply using a 64-bit double to check double is not appropriate here.
            // If you are stuck in this situation, you may need to switch to mpfr+gmp to get more precise answers.
            // This will unfortunately run rather slowly and require some work.
            if( sizeof( double) >= sizeof( long double ) )
            {
                printf( "This architecture needs something higher precision than double to check the precision of double.\nTest FAILED.\n" );
                abort();
            }

            // Note: This function presumes that someone has already 
            // tested whether the result is correctly
            // rounded before calling this function.  That test:
            //
            //    if( (float) reference == test )
            //        return 0.0f;
            //
            // would ensure that cases like fabs(reference) > FLT_MAX are weeded out before we get here.
            // Otherwise, we'll return inf ulp error here, for what are otherwise correctly rounded
            // results.

            int x;
            if( 0.5L != frexpl( reference, &x) )
            { // Non-power of two and NaN
                if( IsInf( reference ) )
                {
                    if( testVal == reference )
                        return 0.0f;

                    return (float) ( testVal - reference );
                }

                if( IsNaN( reference ) && IsNaN( testVal ) )
                    return 0.0f;    // if we are expecting a NaN, any NaN is fine

                // The unbiased exponent of the ulp unit place
                int ulp_exp = DBL_MANT_DIG - 1 - std::max( Conformance::ilogbl( reference), DBL_MIN_EXP-1 );

                // Scale the exponent of the error
                return (float) ldexpl( testVal - reference, ulp_exp );
            }

            // reference is a normal power of two or a zero
            // The unbiased exponent of the ulp unit place
            int ulp_exp =  DBL_MANT_DIG - 1 - std::max( Conformance::ilogbl( reference) - 1, DBL_MIN_EXP-1 );

            // Scale the exponent of the error
            return /*(float)*/ ldexpl( testVal - reference, ulp_exp );
        }

        float ulpsDiff(double ref, float test)
        {
            double testDouble = double(test);
            double res = ulpsDiffSamePrecision(ref, testDouble);
            return (float) res;
        }

        float ulpsDiff(long double ref, double test)
        {
            long double testDouble = (long double)test;
            return (float)(ulpsDiffSamePrecision(ref, testDouble));
        }

        float ulpsDiffDenormal(double ref, float test)
        {
            union {uint32_t u; float f;} convert;

            convert.f = float(ref);
            int32_t aInt = (int32_t)convert.u;
            if (aInt < 0)
                aInt = 0x80000000 - aInt;

            convert.f = test;
            int32_t bInt = (int32_t)convert.u;
            if (bInt < 0)
                bInt = 0x80000000 - bInt;
            return float(abs(aInt - bInt));
        }

        float ulpsDiffDenormal(long double ref, double test)
        {
            union {uint64_t u; double f;} convert;

            convert.f = double(ref);
            int64_t aInt = (int64_t)convert.u;
            if (aInt < 0)
                aInt = 0x8000000000000000 - aInt;

            convert.f = test;
            int64_t bInt = (int64_t)convert.u;
            if (bInt < 0)
                bInt = 0x8000000000000000 - bInt;
            return float(abs(int(aInt - bInt)));
        }

    }
}

