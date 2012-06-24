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

File Name:  RefALU.h

\*****************************************************************************/

#ifndef _REFALU_H_
#define _REFALU_H_

#include "assert.h"
#include "dxfloat.h"
#include "FloatOperations.h"
#include "Conformance/reference_math.h"

namespace Validation
{

#ifndef M_PI
    #define M_PI    3.14159265358979323846264338327950288
#endif
#ifndef M_PIL
    #define M_PIL   3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L
#endif
#ifndef M_PI_4
    #define M_PI_4 (M_PI/4)
#endif

    class RefALU
    {
    protected:
        static bool FTZmode;

    public:

        static void SetFTZmode(bool mode) {
            FTZmode = mode;
        }

        static bool GetFTZmode(void) {
            return FTZmode;
        }

        static long double precision_cast(double a)
        {
            return (long double)(a);
        }

        static double precision_cast(float a)
        {
            return (double)(a);
        }

        static float  precision_cast(CFloat16 a)
        {
            return (float)a;
        }

        template<typename T>
        static bool le(const T& a, const T& b)
        {
            return Utils::le(a, b);
        }

        template<typename T>
        static bool ge(const T& a, const T& b)
        {
            return Utils::ge(a, b);
        }

        template<typename T>
        static bool eq(const T& a, const T& b)
        {
            return Utils::eq(a, b);
        }

        template<typename T>
        static bool gt(const T& a, const T& b)
        {
            return Utils::gt(a, b);
        }

        template<typename T>
        static bool lt(const T& a, const T& b)
        {
            return Utils::lt(a, b);
        }

        template<typename T>
        static T add( const T& a, const T& b)
        {
           return a + b;
        }

        template<typename T>
        static T sub( const T& a, const T& b)
        {
            return a - b;
        }

        template<typename T>
        static T div( const T& a, const T& b)
        {
            T ans = (T)(a / b);
            return ans;
        }

        template<typename T>
        static T mul( const T& a, const T& b)
        {
            return (T)(a * b);
        }

        template<typename T>
        static T neg(const T& a)
        {
            return -a;
        }

////////////////////////////////////////////////////////////////////////
/// math functions begin
        template<typename T> static T abs(const T& a);
        template<typename T> static T copysign(const T& x, const T& y);

        template<typename T> static T acos(const T& a);
        template<typename T> static T asin(const T& a);
        template<typename T> static T atan(const T& a);
        template<typename T> static T sinh(const T& a);
        template<typename T> static T cosh(const T& a);
        template<typename T> static T tanh(const T& a);
        template<typename T> static T sinpi(const T& a);
        template<typename T> static T cospi(const T& a);
        template<typename T> static T cos(const T& a);
        template<typename T> static T sin(const T& a);
        template<typename T> static T tan(const T& a);
        template<typename T> static T tanpi(const T& a);
        template<typename T> static T acospi(const T& a);
        template<typename T> static T asinpi(const T& a);
        template<typename T> static T atanpi(const T& a);
        template<typename T> static T atan2(const T& y, const T& x);
        template<typename T> static T atan2pi(const T& y, const T& x);
        template<typename T> static T floor(const T& a);
        template<typename T> static T fract(const T& a, T* b);
        template<typename T> static T hypot(const T& x, const T& y);
        template<typename T> static T asinh(const T& x);
        template<typename T> static T acosh(const T& x);
        template<typename T> static T atanh(const T& x);
        template<typename T> static T fmax(const T& x, const T& y)
        {
            //    OCL 6.11.2, 9.3.2
            //    Returns y if x < y, otherwise it returns x. If one
            //    argument is a NaN, fmax() returns the other
            //    argument. If both arguments are NaNs, fmax()
            //    returns a NaN.
            if( Utils::IsNaN(x) && Utils::IsNaN(y))
                return Utils::GetNaN<T>();

            if( Utils::IsNaN(y) )
                return x;

            if( Utils::IsNaN(x) )
                return y;

            return x >= y ? x : y;
        }

        template<typename T> static T fmin(const T& x, const T& y)
        {
            //    OCL 6.11.2, 9.3.2
            //    Returns y if y < x, otherwise it returns x. If one
            //    argument is a NaN, fmin() returns the other
            //    argument. If both arguments are NaNs, fmin()
            //    returns a NaN.
            if( Utils::IsNaN(x) && Utils::IsNaN(y))
                return Utils::GetNaN<T>();

            if( Utils::IsNaN(y) )
                return x;

            if( Utils::IsNaN(x) )
                return y;

            return x <= y ? x : y;
        }

        /////////////// Common functions //////////////////////////////
        template<typename T> static T clamp( const T& x, const T& in_min, const T& in_max);
        
        template<typename T>
        static T degrees(const T& a)
        {
            const T pi180 = T(180.0L/M_PIL);
            return (a * pi180);
        }

        template<typename T> static T max(const T& x, const T& y){
            //  OCL spec 6.11.4
            // Returns y if x < y, otherwise it returns x. If x or y
            // are infinite or NaN, the return values are undefined

            if (Utils::IsNaN(x) || Utils::IsNaN(y))
                return Utils::GetNaN<T>();
            if (Utils::IsPInf(x) || Utils::IsPInf(y))
                return Utils::GetPInf<T>();
            if (Utils::IsNInf(x) || Utils::IsNInf(y))
                return Utils::GetNInf<T>();

            return ( x < y ) ? y : x;
        }

        template<typename T> static T min(const T& x, const T& y){
            //  OCL spec 6.11.4
            //  Returns y if y < x, otherwise it returns x. If x or y
            //  are infinite or NaN, the return values are undefined.
            //
            if (Utils::IsNaN(x) || Utils::IsNaN(y))
                return Utils::GetNaN<T>();
            if (Utils::IsPInf(x) || Utils::IsPInf(y))
                return Utils::GetPInf<T>();
            if (Utils::IsNInf(x) || Utils::IsNInf(y))
                return Utils::GetNInf<T>();
            return ( y < x ) ? y : x;
        }

        template<typename T>
        static T radians(const T& a)
        {
            const T pi180 = T(M_PIL/180.0L);
            return (a * pi180);
        }

       // Returns 0.0 if x <= edge0 and 1.0 if x >= edge1 and performs smooth Hermite interpolation 
       // between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want 
       // a threshold function with a smooth transition. Results are undefined if edge0 >= edge1.
        template<typename T> static T smoothstep(const T& edge0, const T& edge1, const T& x){
            if( edge0 >= edge1 )
                return T(-1.0); // Results are undefined, so -1 is ok here
            if( Utils::le<T>(x,edge0) )
                return T(0.0);
            if( Utils::ge<T>(x,edge1) )
                return T(1.0);
            T t = clamp<T>((x-edge0)/(edge1-edge0),T(0.0),T(1.0));
            return t*t*(3-2*t);
        }

        template<typename T> static T step(const T& edge, const T& x){
            return (x < edge) ? 0.0 : 1.0;
        }

        // Returns 1.0 if x > 0, -0.0 if x = -0.0, +0.0 if x = +0.0,
        // or -1.0 if x < -0. Returns 0.0 if x is a NaN.
        template<typename T> static T sign(const T& x){
            if (Utils::IsNaN(x))
                return T(0.0);            
            if (Utils::eq<T>(x,T(+0.0)))
                return T(+0.0);
            if (Utils::eq<T>(x,T(-0.0)))
                return T(-0.0);
            if (Utils::gt<T>(x,T(+0.0)))
                return T(1.0);
            if (Utils::lt<T>(x,T(-0.0)))
                return T(-1.0);
            assert(false);
            return T(-2.0); // we do never go here, this return is for correct compilation
        }
/* trigonometric functions end */

        template<typename T> static T sqrt(const T& a);
        template<typename T> static T rsqrt(const T& a);
        template<typename T> static T cbrt(const T& a);

        template<typename T> static T fabs(const T& a);

        /// flushes denorms to 0
        template<typename T>
        static T flush(const T& a)
        {
            T returnVal = a;
            if ( FTZmode && a != 0 && ::fabs( a ) < std::numeric_limits<T>::min() )
            {
                returnVal = a > 0.0 ? T(+0.0) : T(-0.0);
            }
            return returnVal;
        }

        template<typename T>  static T fmod(const T& a, const T& b);

        template<typename T>
        static T round_ni(const T& a)
        {
            return ::floorf( flush(precision_cast(a)));
        }

        template<typename T> static T ceil(const T& a);
        /// Computes base-e exponential of a
        template<typename T> static T exp(const T& a);
        /// Exponential base 2 function
        template<typename T> static T exp2(const T& a);
        /// Exponential base 10 function
        template<typename T> static T exp10(const T& a);
        /// Computes e**a-1
        template<typename T> static T expm1(const T& a);

        /// Computes natural logarithm
        template<typename T> static T log (const T& x);
        /// Computes base-2 logarithm of x
        template<typename T> static T log2 (const T& x);
        /// Computes base-10 logarithm of x
        template<typename T> static T log10 (const T& x);
        /// Computes the natural logarithm of 1 plus the x
        template<typename T> static T log1p (const T& x);
        /// Returns signed exponent of of x
        template<typename T> static T logb (const T& x);

        /// Computes x to the power of y
        template<typename T> static T pow (const T& x, const T& y);
        /// Computes x to the power of y, where x >= 0
        template<typename T> static T powr (const T& x, const T& y);
        /// Computes x to the power of y, y is integer
        template<typename T> static T pown (const T& x, const int& y);
        /// Compute the value r such that r = x - n*y, where n
        /// is the integer nearest the exact value of x/y. If there
        /// are two integers closest to x/y, n shall be the even
        ///one. If r is zero, it is given the same sign as x.
        template<typename T> static T remainder(const T& x, const T& y);
        /// This calculates the same value that is returned by the remainder function.
        /// remquo also calculates the lower seven bits of the
        /// integral quotient x/y, and gives that value the same
        /// sign as x/y. It stores this signed value in the object pointed to by quo
        template<typename T> static T remquo(const T& x, const T& y, int32_t * quo);
        /// Extracts mantissa and exponent from x
        template<typename T>  static T frexp(const T& x, int * exp);
        /// Multiply x by 2 to the power n
        template<typename T>  static T ldexp(const T& x, const int& exp);
        /// breaks the argument x into integral and fractional parts, 
        /// each of which has the same sign as the argument
        template<typename T>  static T modf(const T& x, T * iptr);
        /// Round to integral value using the round to zero rounding mode
        template<typename T>  static T trunc(const T& x);
        /// Return the integral value nearest to x rounding halfway cases away from zero, 
        /// regardless of the current rounding direction.
        template<typename T>  static T round(const T& x);
        /// Round to integral value (using round to nearest even rounding mode) in floating-point forma
        template<typename T>  static T rint(const T& x);
        /// Return the exponent as an integer value
        template<typename T>  static int ilogb(const T& x);
        /// Compute x to the power 1/y
        template<typename T>  static T rootn(const T& x, const int& y);
        
        /// Returns a quiet NaN. The nancode may be placed in the significand of the resulting NaN        
        static float nan(const uint16_t& x) {
            uint16_t u = x & 0x01ff;
            u |= 0x7e00; // make quiet NaN
            CFloat16 f(u);
            return float(f);
        }
        static double nan(const uint32_t& x) {
            return Conformance::reference_nan(x);

        }
        static long double nan(const uint64_t& x) {
            return Conformance::reference_nanl(x);
        }

        template<typename T>
        static T frc(const T& a )
        {
            return sub( a, round_ni( a ) );
        }

        template<typename T> static T nextafter(const T& x, const T& y);
        template<typename T> static T fdim(const T& x, const T& y);
        template<typename T> static T maxmag(const T& x, const T& y);
        template<typename T> static T minmag(const T& x, const T& y);

        template<typename T> static T lgamma(const T& x);
        template<typename T> static T lgamma_r(const T& x, int32_t* signp);

        template<typename T> static T mad(const T& a, const T& b, const T& c);

        template<typename T>
        static int isInf(const T& x)
        {
            return Utils::IsNInf(x) || Utils::IsPInf(x);
        }

        template<typename T>
        static int isNormal(const T& x)
        {
            return !Utils::IsDenorm(x) && !Utils::IsNaN(x) && !Utils::IsNInf(x) && !Utils::IsPInf(x);
        }

        template<typename T>
        static int isNan(const T& x)
        {
            return Utils::IsNaN(x);
        }

        template<typename T>
        static T fma(const T& a, const T& b, const T& c)
        {
            return precision_cast(a) * precision_cast(b) + c;
        }
        /// Test for sign bit.
        template<typename T>  static T signbit(const T& x);

    };
}

#endif

