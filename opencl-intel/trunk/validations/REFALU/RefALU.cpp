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

File Name:  RefALU.cpp

\*****************************************************************************/
#include "RefALU.h"
#include <iostream>

using namespace Validation;

namespace Validation
{

    bool RefALU::FTZmode(false);

    template<>
    float RefALU::abs<float>(const float &a )
    {
        return (float)Conformance::reference_fabs(a);
    }
    template<>
    double RefALU::abs<double>(const double &a )
    {
        return Conformance::reference_fabs(a);
    }
    template<>
    long double RefALU::abs<long double>(const long double &a )
    {
        return Conformance::reference_fabsl(a);
    }


    template<>
    float RefALU::copysign<float>(const float &x,const float &y )
    {
        union { float f; uint32_t u;} ux, uy;
        ux.f = x; uy.f = y;
        ux.u &= 0x7fffffffU;
        ux.u |= uy.u & 0x80000000U;
        return ux.f;
    }
    template<>
    double RefALU::copysign<double>(const double &x,const double &y )
    {
        return Conformance::reference_copysign(x,y);
    }
    template<>
    long double RefALU::copysign<long double>(const long double &x,const long double &y )
    {
        return Conformance::reference_copysignl(x,y);
    }

    template<>
    float RefALU::sin<float>(const float &a)
    {
        return (float)Conformance::reference_sin(a);
    }
    template<>
    double RefALU::sin<double>(const double &a)
    {
        return Conformance::reference_sin(a);
    }
    template<>
    long double RefALU::sin<long double>(const long double &a)
    {
        return Conformance::reference_sinl(a);
    }

    template<>
    float RefALU::cos<float>(const float &a)
    {
        return (float)Conformance::reference_cos(a);
    }
    template<>
    double RefALU::cos<double>(const double &a)
    {
        return Conformance::reference_cos(a);
    }
    template<>
    long double RefALU::cos<long double>(const long double &a)
    {
        return Conformance::reference_cosl(a);
    }

    template<>
    float RefALU::tan<float>(const float &a)
    {
        return (float)Conformance::reference_tan(a);
    }
    template<>
    double RefALU::tan<double>(const double &a)
    {
        return Conformance::reference_tan(a);
    }
    template<>
    long double RefALU::tan<long double>(const long double &a)
    {
        return Conformance::reference_tanl(a);
    }

    template<>
    float RefALU::tanpi<float>(const float &a)
    {
        return (float)Conformance::reference_tanpi(a);
    }
    template<>
    double RefALU::tanpi<double>(const double &a)
    {
        return Conformance::reference_tanpi(a);
    }
    template<>
    long double RefALU::tanpi<long double>(const long double &a)
    {
        return Conformance::reference_tanpil(a);
    }

    template<>
    float RefALU::asin<float>(const float &a)
    {
        return (float)Conformance::reference_asin(a);
    }
    template<>
    double RefALU::asin<double>(const double &a)
    {
        return Conformance::reference_asin(a);
    }
    template<>
    long double RefALU::asin<long double>(const long double &a)
    {
        return Conformance::reference_asinl(a);
    }

    template<>
    float RefALU::acos<float>(const float& a)
    {
        return (float)Conformance::reference_acos(a);
    }
    template<>
    double RefALU::acos<double>(const double& a)
    {
        return Conformance::reference_acos(a);
    }
    template<>
    long double RefALU::acos<long double>(const long double& a)
    {
        return Conformance::reference_acosl(a);
    }

    template<>
    float RefALU::atan<float>(const float &a)
    {
        return (float)Conformance::reference_atan(a);
    }
    template<>
    double RefALU::atan<double>(const double &a)
    {
        return Conformance::reference_atan(a);
    }
    template<>
    long double RefALU::atan<long double>(const long double &a)
    {
        return Conformance::reference_atanl(a);
    }

    template<>
    float RefALU::sinh<float>(const float &a)
    {
        return (float)Conformance::reference_sinh(a);
    }
    template<>
    double RefALU::sinh<double>(const double &a)
    {
        return Conformance::reference_sinh(a);
    }
    template<>
    long double RefALU::sinh<long double>(const long double &a)
    {
        return Conformance::reference_sinhl(a);
    }

    template<>
    float RefALU::cosh<float>(const float &a)
    {
        return (float)Conformance::reference_cosh(a);
    }
    template<>
    double RefALU::cosh<double>(const double &a)
    {
        return Conformance::reference_cosh(a);
    }
    template<>
    long double RefALU::cosh<long double>(const long double &a)
    {
        return Conformance::reference_coshl(a);
    }

    template<>
    float RefALU::tanh<float>(const float &a)
    {
        return (float)Conformance::reference_tanh(a);
    }
    template<>
    double RefALU::tanh<double>(const double &a)
    {
        return Conformance::reference_tanh(a);
    }
    template<>
    long double RefALU::tanh<long double>(const long double &a)
    {
        return Conformance::reference_tanhl(a);
    }

    template<>
    float RefALU::sinpi<float>(const float &a)
    {
        return (float)Conformance::reference_sinpi(a);
    }
    template<>
    double RefALU::sinpi<double>(const double &a)
    {
        return Conformance::reference_sinpi(a);
    }
    template<>
    long double RefALU::sinpi<long double>(const long double &a)
    {
        return Conformance::reference_sinpil(a);
    }

    template<>
    float RefALU::cospi<float>(const float &a)
    {
        return (float)Conformance::reference_cospi(a);
    }
    template<>
    double RefALU::cospi<double>(const double &a)
    {
        return Conformance::reference_cospi(a);
    }
    template<>
    long double RefALU::cospi<long double>(const long double &a)
    {
        return Conformance::reference_cospil(a);
    }

    template<>
    float RefALU::asinpi<float>(const float &a)
    {
        return (float)Conformance::reference_asinpi(a);
    }
    template<>
    double RefALU::asinpi<double>(const double &a)
    {
        return Conformance::reference_asinpi(a);
    }
    template<>
    long double RefALU::asinpi<long double>(const long double &a)
    {
        return Conformance::reference_asinpil(a);
    }

    template<>
    float RefALU::acospi<float>(const float &a)
    {
        return (float)Conformance::reference_acospi(a);
    }
    template<>
    double RefALU::acospi<double>(const double &a)
    {
        return Conformance::reference_acospi(a);
    }
    template<>
    long double RefALU::acospi<long double>(const long double &a)
    {
        return Conformance::reference_acospil(a);
    }

    template<>
    float RefALU::atanpi<float>(const float &a)
    {
        return (float)Conformance::reference_atanpi(a);
    }
    template<>
    double RefALU::atanpi<double>(const double &a)
    {
        return Conformance::reference_atanpi(a);
    }
    template<>
    long double RefALU::atanpi<long double>(const long double &a)
    {
        return Conformance::reference_atanpil(a);
    }

    template<>
    float RefALU::atan2<float>(const float &x, const float &y)
    {
        return (float)Conformance::reference_atan2(x,y);
    }
    template<>
    double RefALU::atan2<double>(const double &x, const double &y)
    {
        return Conformance::reference_atan2(x,y);
    }
    template<>
    long double RefALU::atan2<long double>(const long double &x,const long double &y)
    {
        return Conformance::reference_atan2l(x,y);
    }

    template<>
    float RefALU::atan2pi<float>(const float &x, const float &y)
    {
        return (float)Conformance::reference_atan2pi(x,y);
    }
    template<>
    double RefALU::atan2pi<double>(const double &x, const double &y)
    {
        return Conformance::reference_atan2pi(x,y);
    }
    template<>
    long double RefALU::atan2pi<long double>(const long double &x,const long double &y)
    {
        return Conformance::reference_atan2pil(x,y);
    }

    template<>
    float RefALU::ceil<float>(const float& x)
    {
        return (float)Conformance::reference_ceil(x);
    }
    template<>
    double RefALU::ceil<double>(const double& x)
    {
        return Conformance::reference_ceil(x);
    }
    template<>
    long double RefALU::ceil<long double>(const long double& x)
    {
        return Conformance::reference_ceill(x);
    }

    template<>
    float RefALU::clamp<float>(const float &x, const float &in_min, const float &in_max)
    {
        float ans = x;
        if(x > in_max)
            ans = in_max;
        else if(x < in_min)
            ans = in_min;
        return ans;
    }
    template<>
    double RefALU::clamp<double>(const double &x, const double &in_min, const double &in_max)
    {
        double ans = x;
        if(x > in_max)
            ans = in_max;
        else if(x < in_min)
            ans = in_min;
        return ans;
    }
    template<>
    long double RefALU::clamp<long double>(const long double &x, const long double &in_min, const long double &in_max)
    {
        long double ans = x;
        if(x > in_max)
            ans = in_max;
        else if(x < in_min)
            ans = in_min;
        return ans;
    }

    template<>
    float RefALU::exp<float>(const float& x)
    {
        return (float)Conformance::reference_exp(x);
    }
    template<>
    double RefALU::exp<double>(const double& x)
    {
        return Conformance::reference_exp(x);
    }
    template<>
    long double RefALU::exp<long double>(const long double& x)
    {
        return Conformance::reference_expl( x );
    }

    template<>
    float RefALU::exp2<float>(const float& x)
    {
        return (float)Conformance::reference_exp2(x);
    }
    template<>
    double RefALU::exp2<double>(const double& x)
    {
        return Conformance::reference_exp2(x);
    }
    template<>
    long double RefALU::exp2<long double>(const long double& x)
    {
        return Conformance::reference_exp2l(x);
    }

    template<>
    float RefALU::exp10<float>(const float& x)
    {
        return (float)Conformance::reference_exp10(x);
    }
    template<>
    double RefALU::exp10<double>(const double& x)
    {
        return Conformance::reference_exp10(x);
    }
    template<>
    long double RefALU::exp10<long double>(const long double& x)
    {
        return Conformance::reference_exp10l(x);
    }

    template<>
    float RefALU::expm1<float>(const float& x)
    {
        return (float)Conformance::reference_expm1(x);
    }
    template<>
    double RefALU::expm1<double>(const double& x)
    {
        return Conformance::reference_expm1(x);
    }
    template<>
    long double RefALU::expm1<long double>(const long double& x)
    {
        return Conformance::reference_expm1l(x);
    }

    template<>
    float RefALU::log2<float>(const float& x)
    {
        return (float)Conformance::reference_log2(x);
    }
    template<>
    double RefALU::log2<double>(const double& x)
    {
        return Conformance::reference_log2(x);
    }
    template<>
    long double RefALU::log2<long double>(const long double& x)
    {
        return Conformance::reference_log2l(x);
    }

    template<>
    float RefALU::log10<float>(const float& x)
    {
        return (float)Conformance::reference_log10(x);
    }
    template<>
    double RefALU::log10<double>(const double& x)
    {
        return Conformance::reference_log10(x);
    }
    template<>
    long double RefALU::log10<long double>(const long double& x)
    {
        return Conformance::reference_log10l(x);
    }

    template<>
    float RefALU::log<float>(const float& x)
    {
        return (float)Conformance::reference_log(x);
    }
    template<>
    double RefALU::log<double>(const double& x)
    {
        return Conformance::reference_log(x);
    }
    template<>
    long double RefALU::log<long double>(const long double& x)
    {
        return Conformance::reference_logl(x);
    }

    template<>
    float RefALU::log1p<float>(const float& x)
    {
        return (float)Conformance::reference_log1p(x);
    }
    template<>
    double RefALU::log1p<double>(const double& x)
    {
        return Conformance::reference_log1p(x);
    }
    template<>
    long double RefALU::log1p<long double>(const long double& x)
    {
        return Conformance::reference_log1pl(x);
    }

    template<>
    float RefALU::logb<float>(const float& x)
    {
        return (float)Conformance::reference_logb(x);
    }
    template<>
    double RefALU::logb<double>(const double& x)
    {
        return Conformance::reference_logb(x);
    }
    template<>
    long double RefALU::logb<long double>(const long double& x)
    {
        return Conformance::reference_logbl(x);
    }

    template<>
    int RefALU::ilogb<float>(const float& x)
    {
        return Conformance::reference_ilogb(double(x));
    }
    template<>
    int RefALU::ilogb<double>(const double& x)
    {
        return Conformance::reference_ilogb(x);
    }
    template<>
    int RefALU::ilogb<long double>(const long double& x)
    {
        return Conformance::reference_ilogbl(x);
    }

    template<>
    float RefALU::pow<float>(const float& x, const float& y)
    {
        return (float)Conformance::reference_pow(x, y);
    }
    template<>
    double RefALU::pow<double>(const double& x, const double& y)
    {
        return Conformance::reference_pow(x, y);
    }
    template<>
    long double RefALU::pow<long double>(const long double& x, const long double& y)
    {
        return Conformance::reference_powl(x, y);
    }

    template<>
    float RefALU::powr<float>(const float& x, const float& y)
    {
        return (float)Conformance::reference_powr(x, y);
    }
    template<>
    double RefALU::powr<double>(const double& x, const double& y)
    {
        return Conformance::reference_powr(x, y);
    }
    template<>
    long double RefALU::powr<long double>(const long double& x, const long double& y)
    {
        return Conformance::reference_powrl(x, y);
    }

    template<>
    float RefALU::pown<float>(const float& x, const int& y)
    {
        return (float)Conformance::reference_pown(x, y);
    }
    template<>
    double RefALU::pown<double>(const double& x, const int& y)
    {
        return Conformance::reference_pown(x, y);
    }
    template<>
    long double RefALU::pown<long double>(const long double& x, const int& y)
    {
        return Conformance::reference_pownl(x, y);
    }


    template<>
    float RefALU::cbrt<float>(const float &a)
    {
        return (float)Conformance::reference_cbrt(a);
    }
    template<>
    double RefALU::cbrt<double>(const double &a)
    {
        return Conformance::reference_cbrt(a);
    }
    template<>
    long double RefALU::cbrt<long double>(const long double &a)
    {
        return Conformance::reference_cbrtl(a);
    }

    template<>
    float RefALU::sqrt<float>(const float &a)
    {
        return (float)Conformance::reference_sqrt(a);
    }
    template<>
    double RefALU::sqrt<double>(const double &a)
    {
        return Conformance::reference_sqrt(a);
    }
    template<>
    long double RefALU::sqrt<long double>(const long double &a)
    {
        return Conformance::reference_sqrtl(a);
    }

    template<>
    float RefALU::rsqrt<float>(const float &a)
    {
        return (float)Conformance::reference_rsqrt(a);
    }
    template<>
    double RefALU::rsqrt<double>(const double &a)
    {
        return Conformance::reference_rsqrt(a);
    }
    template<>
    long double RefALU::rsqrt<long double>(const long double &a)
    {
        return Conformance::reference_rsqrtl(a);
    }

    template<>
    float RefALU::fabs<float>(const float& a)
    {
        return (float)Conformance::reference_fabs( a );
    }
    template<>
    double RefALU::fabs<double>(const double& a)
    {
        return Conformance::reference_fabs( a );
    }
    template<>
    long double RefALU::fabs<long double>(const long double& a)
    {
        return Conformance::reference_fabsl( a );
    }

    template<>
    float RefALU::floor<float>(const float& a)
    {
        return (float)Conformance::reference_floor(a);
    }
    template<>
    double RefALU::floor<double>(const double& a)
    {
        return Conformance::reference_floor(a);
    }
    template<>
    long double RefALU::floor<long double>(const long double& a)
    {
        return Conformance::reference_floorl(a);
    }

    template<>
    float RefALU::fract<float>(const float& a, float* b)
    {
        double tmp;
        float res = (float)Conformance::reference_fract(a, &tmp);
        *b = (float)tmp;
        return res;
    }
    template<>
    double RefALU::fract<double>(const double& a, double* b)
    {
        return Conformance::reference_fract(a, b);
    }
    template<>
    long double RefALU::fract<long double>(const long double& a, long double* b)
    {
        return Conformance::reference_fractl(a, b);
    }

    template<>
    float RefALU::hypot<float>(const float& x, const float& y)
    {
        return (float)Conformance::reference_hypot( x, y );
    }
    template<>
    double RefALU::hypot<double>(const double& x, const double& y)
    {
        return Conformance::reference_hypot( x, y );
    }
    template<>
    long double RefALU::hypot<long double>(const long double& x, const long double& y)
    {
        return Conformance::reference_hypotl( x, y );
    }

    template<>
    float RefALU::frexp<float>(const float& x, int * exp)
    {
        return (float)Conformance::reference_frexp(x,exp);
    }
    template<>
    double RefALU::frexp<double>(const double& x, int * exp)
    {
        return Conformance::reference_frexp(x,exp);
    }
    template<>
    long double RefALU::frexp<long double>(const long double& x, int * exp)
    {
        return Conformance::reference_frexpl(x,exp);
    }

    template<>
    float RefALU::ldexp<float>(const float& x, const int& n)
    {
        return (float)Conformance::reference_ldexp(x,n);
    }
    template<>
    double RefALU::ldexp<double>(const double& x, const int& n)
    {
        return Conformance::reference_ldexp(x,n);
    }
    template<>
    long double RefALU::ldexp<long double>(const long double& x, const int& n)
    {
        return Conformance::reference_ldexpl(x,n);
    }

    template<>
    float RefALU::trunc<float>(const float& x)
    {
        return (float)Conformance::reference_trunc(x);
    }
    template<>
    double RefALU::trunc<double>(const double& x)
    {
        return Conformance::reference_trunc(x);
    }
    template<>
    long double RefALU::trunc<long double>(const long double& x)
    {
        return Conformance::reference_truncl(x);
    }

    template<>
    float RefALU::round<float>(const float& x)
    {
        return (float)Conformance::reference_round(x);
    }
    template<>
    double RefALU::round<double>(const double& x)
    {
        return Conformance::reference_round(x);
    }
    template<>
    long double RefALU::round<long double>(const long double& x)
    {
        return Conformance::reference_roundl(x);
    }

    template<>
    float RefALU::rint<float>(const float& x)
    {
        return (float)Conformance::reference_rint(x);
    }
    template<>
    double RefALU::rint<double>(const double& x)
    {
        return Conformance::reference_rint(x);
    }
    template<>
    long double RefALU::rint<long double>(const long double& x)
    {
        return Conformance::reference_rintl(x);
    }

    template<>
    float RefALU::modf<float>(const float& x, float * iptr)
    {
        double res_integral = (double)(*iptr);
        double res_fractional = Conformance::reference_modf(x,&res_integral);
        *iptr = (float)res_integral;        
        return (float)res_fractional;
    }
    template<>
    double RefALU::modf<double>(const double& x, double * iptr)
    {
        return Conformance::reference_modf(x,iptr);
    }
    template<>
    long double RefALU::modf<long double>(const long double& x, long double * iptr)
    {
        return Conformance::reference_modfl(x,iptr);
    }

    template<>
    float RefALU::rootn<float>(const float& x, const int& y)
    {
        return (float)Conformance::reference_rootn(x,y);
    }
    template<>
    double RefALU::rootn<double>(const double& x, const int& y)
    {
        return Conformance::reference_rootn(x,y);
    }
    template<>
    long double RefALU::rootn<long double>(const long double& x, const int& y)
    {
        return Conformance::reference_rootnl(x,y);
    }

    template<>
    float RefALU::asinh<float>(const float& x)
    {
        return (float)Conformance::reference_asinh(x);
    }
    template<>
    double RefALU::asinh<double>(const double& x)
    {
        return Conformance::reference_asinh(x);
    }
    template<>
    long double RefALU::asinh<long double>(const long double& x)
    {
        return Conformance::reference_asinhl(x);
    }

    template<>
    float RefALU::acosh<float>(const float& x)
    {
        return (float)Conformance::reference_acosh(x);
    }
    template<>
    double RefALU::acosh<double>(const double& x)
    {
        return Conformance::reference_acosh(x);
    }
    template<>
    long double RefALU::acosh<long double>(const long double& x)
    {
        return Conformance::reference_acoshl(x);
    }

    template<>
    float RefALU::atanh<float>(const float& x)
    {
        return (float)Conformance::reference_atanh(x);
    }
    template<>
    double RefALU::atanh<double>(const double& x)
    {
        return Conformance::reference_atanh(x);
    }
    template<>
    long double RefALU::atanh<long double>(const long double& x)
    {
        return Conformance::reference_atanhl(x);
    }


    template<>
    float RefALU::signbit<float>(const float& x)
    {
        double res;
        res = Conformance::reference_signbit(x);
        return (float)res;
    }
    template<>
    double RefALU::signbit<double>(const double& x)
    {
        return Conformance::reference_signbit(x);
    }
    template<>
    long double RefALU::signbit<long double>(const long double& x)
    {
        return Conformance::reference_signbitl(x);
    }

    template<>
    float RefALU::nextafter<float>(const float& x, const float& y)
    {
        double res = Conformance::reference_nextafter(x,y);
        return (float)res;
    }
    template<>
    double RefALU::nextafter<double>(const double& x, const double& y)
    {
        return Conformance::reference_nextafter(x,y);
    }
    template<>
    long double RefALU::nextafter<long double>(const long double& x,const long double& y)
    {
        return Conformance::reference_nextafterl(x,y);
    }

    template<>
    float RefALU::fmod<float>(const float& x, const float& y)
    {
        double res = Conformance::reference_fmod(x,y);
        return (float)res;
    }
    template<>
    double RefALU::fmod<double>(const double& x, const double& y)
    {
        return Conformance::reference_fmod(x,y);
    }
    template<>
    long double RefALU::fmod<long double>(const long double& x,const long double& y)
    {
        return Conformance::reference_fmodl(x,y);
    }

    template<>
    float RefALU::fdim<float>(const float& x, const float& y)
    {
        double res = Conformance::reference_fdim(x,y);
        return (float)res;
    }
    template<>
    double RefALU::fdim<double>(const double& x, const double& y)
    {
        return Conformance::reference_fdim(x,y);
    }
    template<>
    long double RefALU::fdim<long double>(const long double& x,const long double& y)
    {
        return Conformance::reference_fdiml(x,y);
    }


    template<>
    float RefALU::lgamma<float>(const float& x)
    {
        return (float)Conformance::reference_lgamma(x);
    }
    template<>
    double RefALU::lgamma<double>(const double& x)
    {
        return Conformance::reference_lgamma(x);
    }
    template<>
    long double RefALU::lgamma<long double>(const long double& x)
    {
        return Conformance::reference_lgammal(x);
    }

    template<>
    float RefALU::lgamma_r<float>(const float& x, int32_t* signp)
    {
        return (float)Conformance::reference_lgamma_r(x, signp);
    }
    template<>
    double RefALU::lgamma_r<double>(const double& x, int32_t* signp)
    {
        return Conformance::reference_lgamma_r(x, signp);
    }
    template<>
    long double RefALU::lgamma_r<long double>(const long double& x, int32_t* signp)
    {
        return Conformance::reference_lgamma_rl(x, signp);
    }


    template<>
    float RefALU::mad<float>(const float& a, const float& b, const float& c)
    {
        return (float)Conformance::reference_mad(a, b, c);
    }
    template<>
    double RefALU::mad<double>(const double& a, const double& b, const double& c)
    {
        return Conformance::reference_mad(a, b, c);
    }
    template<>
    long double RefALU::mad<long double>(const long double& a, const long double& b, const long double& c)
    {
        return Conformance::reference_madl(a, b, c);
    }

    template<>
    float RefALU::maxmag<float>(const float& a, const float& b)
    {
        return (float)Conformance::reference_maxmag(a, b);
    }
    template<>
    double RefALU::maxmag<double>(const double& a, const double& b)
    {
        return Conformance::reference_maxmag(a, b);
    }
    template<>
    long double RefALU::maxmag<long double>(const long double& a, const long double& b)
    {
        return Conformance::reference_maxmagl(a, b);
    }

    template<>
    float RefALU::minmag<float>(const float& a, const float& b)
    {
        return (float)Conformance::reference_minmag(a, b);
    }
    template<>
    double RefALU::minmag<double>(const double& a, const double& b)
    {
        return Conformance::reference_minmag(a, b);
    }
    template<>
    long double RefALU::minmag<long double>(const long double& a, const long double& b)
    {
        return Conformance::reference_minmagl(a, b);
    }

    template<>
    float RefALU::remainder<float>(const float& x, const float& y)
    {
        return (float)Conformance::reference_remainder(x, y);
    }
    template<>
    double RefALU::remainder<double>(const double& x, const double& y)
    {
        return Conformance::reference_remainder(x, y);
    }
    template<>
    long double RefALU::remainder<long double>(const long double& x, const long double& y)
    {
        return Conformance::reference_remainderl(x, y);
    }

    template<>
    float RefALU::remquo<float>(const float& x, const float& y, int32_t* quo)
    {
        return (float)Conformance::reference_remquo(x, y, quo);
    }
    template<>
    double RefALU::remquo<double>(const double& x, const double& y, int32_t* quo)
    {
        return Conformance::reference_remquo(x, y, quo);
    }
    template<>
    long double RefALU::remquo<long double>(const long double& x, const long double& y, int32_t* quo)
    {
        return Conformance::reference_remquo(x, y, quo);
    }

} // end namespace validation