#include "reference_convert.h"
#include "test_common/compat.h"

namespace Conformance
{
    uint16_t float2half_rte( float f )
    {
        union{ float f; uint32_t u; } u = {f};
        uint32_t sign = (u.u >> 16) & 0x8000;
        float x = fabsf(f);

        //Nan
        if( x != x )
        {
            u.u >>= (24-11);
            u.u &= 0x7fff;
            u.u |= 0x0200;      //silence the NaN
            return (u.u | sign);
        }

        // overflow
        if( x >= MAKE_HEX_FLOAT(0x1.ffep15f, 0x1ffeL, 3) )
            return (0x7c00 | sign);

        // underflow
        if( x <= MAKE_HEX_FLOAT(0x1.0p-25f, 0x1L, -25) )
            return (sign);    // The halfway case can return 0x0001 or 0. 0 is even.

        // very small
        if( x < MAKE_HEX_FLOAT(0x1.8p-24f, 0x18L, -28) )
            return (sign | 1);

        // half denormal
        if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
        {
            u.f = x * MAKE_HEX_FLOAT(0x1.0p-125f, 0x1L, -125);
            return (sign | u.u);
        }

        u.f *= MAKE_HEX_FLOAT(0x1.0p13f, 0x1L, 13);
        u.u &= 0x7f800000;
        x += u.f;
        u.f = x - u.f;
        u.f *= MAKE_HEX_FLOAT(0x1.0p-112f, 0x1L, -112);

        return ((u.u >> (24-11)) | sign);
    }

    uint16_t float2half_rtz( float f )
    {
        union{ float f; uint32_t u; } u = {f};
        uint32_t sign = (u.u >> 16) & 0x8000;
        float x = fabsf(f);

        //Nan
        if( x != x )
        {
            u.u >>= (24-11);
            u.u &= 0x7fff;
            u.u |= 0x0200;      //silence the NaN
            return (u.u | sign);
        }

        // overflow
        if( x >= MAKE_HEX_FLOAT(0x1.0p16f, 0x1L, 16) )
        {
            if( x == INFINITY )
                return (0x7c00 | sign);

            return (0x7bff | sign);
        }

        // underflow
        if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
            return (sign);    // The halfway case can return 0x0001 or 0. 0 is even.

        // half denormal
        if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
        {
            x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
            return ((uint16_t)((int) x | sign));
        }

        u.u &= 0xFFFFE000U;
        u.u -= 0x38000000U;

        return ((u.u >> (24-11)) | sign);
    }

    uint16_t float2half_rtp( float f )
    {
        union{ float f; uint32_t u; } u = {f};
        uint32_t sign = (u.u >> 16) & 0x8000;
        float x = fabsf(f);

        //Nan
        if( x != x )
        {
            u.u >>= (24-11);
            u.u &= 0x7fff;
            u.u |= 0x0200;      //silence the NaN
            return (u.u | sign);
        }

        // overflow
        if( f > MAKE_HEX_FLOAT(0x1.ffcp15f, 0x1ffcL, 3) )
            return (0x7c00);

        if( f <= MAKE_HEX_FLOAT(-0x1.0p16f, -0x1L, 16) )
        {
            if( f == -INFINITY )
                return (0xfc00);

            return (0xfbff);
        }

        // underflow
        if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
        {
            if( f > 0 )
                return (1);
            return (sign);    
        }

        // half denormal
        if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
        {
            x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
            int r = (int) x;
            r += (float) r != x && f > 0.0f;

            return ((uint16_t)( r | sign));
        }

        float g = u.f;
        u.u &= 0xFFFFE000U;
        if( g > u.f )
            u.u += 0x00002000U;
        u.u -= 0x38000000U;

        return ((u.u >> (24-11)) | sign);
    }


    uint16_t float2half_rtn( float f )
    {
        union{ float f; uint32_t u; } u = {f};
        uint32_t sign = (u.u >> 16) & 0x8000;
        float x = fabsf(f);

        //Nan
        if( x != x )
        {
            u.u >>= (24-11);
            u.u &= 0x7fff;
            u.u |= 0x0200;      //silence the NaN
            return (u.u | sign);
        }

        // overflow
        if( f >= MAKE_HEX_FLOAT(0x1.0p16f, 0x1L, 16) )
        {
            if( f == INFINITY )
                return (0x7c00);

            return (0x7bff);
        }

        if( f < MAKE_HEX_FLOAT(-0x1.ffcp15f, -0x1ffcL, 3) )
            return (0xfc00);

        // underflow
        if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
        {
            if( f < 0 )
                return (0x8001);
            return (sign);    
        }

        // half denormal
        if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
        {
            x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
            int r = (int) x;
            r += (float) r != x && f < 0.0f;

            return ((uint16_t)( r | sign));
        }

        u.u &= 0xFFFFE000U;
        if( u.f > f )
            u.u += 0x00002000U;
        u.u -= 0x38000000U;

        return ((u.u >> (24-11)) | sign);
    }

    uint16_t double2half_rte( double f )
    {
        union{ double f; uint64_t u; } u = {f};
        uint64_t sign = (u.u >> 48) & 0x8000;
        double x = fabs(f);

        //Nan
        if( x != x )
        {
            u.u >>= (53-11);
            u.u &= 0x7fff;
            u.u |= 0x0200;      //silence the NaN
            return (u.u | sign);
        }

        // overflow
        if( x >= MAKE_HEX_DOUBLE(0x1.ffep15, 0x1ffeLL, 3) )
            return (0x7c00 | sign);

        // underflow
        if( x <= MAKE_HEX_DOUBLE(0x1.0p-25, 0x1LL, -25) )
            return (sign);    // The halfway case can return 0x0001 or 0. 0 is even.

        // very small
        if( x < MAKE_HEX_DOUBLE(0x1.8p-24, 0x18LL, -28) )
            return (sign | 1);

        // half denormal
        if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1LL, -14) )
        {
            u.f = x * MAKE_HEX_DOUBLE(0x1.0p-1050, 0x1LL, -1050);
            return (sign | u.u);
        }

        u.f *= MAKE_HEX_DOUBLE(0x1.0p42, 0x1LL, 42);
        u.u &= 0x7ff0000000000000ULL;
        x += u.f;
        u.f = x - u.f;
        u.f *= MAKE_HEX_DOUBLE(0x1.0p-1008, 0x1LL, -1008);

        return ((u.u >> (53-11)) | sign);
    }

    uint16_t double2half_rtz( double f )
    {
        union{ double f; uint64_t u; } u = {f};
        uint64_t sign = (u.u >> 48) & 0x8000;
        double x = fabs(f);

        //Nan
        if( x != x )
        {
            u.u >>= (53-11);
            u.u &= 0x7fff;
            u.u |= 0x0200;      //silence the NaN
            return (u.u | sign);
        }

        if( x == INFINITY )
            return (0x7c00 | sign);

        // overflow
        if( x >= MAKE_HEX_DOUBLE(0x1.0p16, 0x1LL, 16) )
            return (0x7bff | sign);

        // underflow
        if( x < MAKE_HEX_DOUBLE(0x1.0p-24, 0x1LL, -24) )
            return (sign);    // The halfway case can return 0x0001 or 0. 0 is even.

        // half denormal
        if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1LL, -14) )
        {
            x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
            return ((uint16_t)((int) x | sign));
        }

        u.u &= 0xFFFFFC0000000000ULL;
        u.u -= 0x3F00000000000000ULL;

        return ((u.u >> (53-11)) | sign);
    }

    uint16_t double2half_rtp( double f )
    {
        union{ double f; uint64_t u; } u = {f};
        uint64_t sign = (u.u >> 48) & 0x8000;
        double x = fabs(f);

        //Nan
        if( x != x )
        {
            u.u >>= (53-11);
            u.u &= 0x7fff;
            u.u |= 0x0200;      //silence the NaN
            return (u.u | sign);
        }

        // overflow
        if( f > MAKE_HEX_DOUBLE(0x1.ffcp15, 0x1ffcLL, 3) )
            return (0x7c00);

        if( f <= MAKE_HEX_DOUBLE(-0x1.0p16, -0x1LL, 16) )
        {
            if( f == -INFINITY )
                return (0xfc00);

            return (0xfbff);
        }

        // underflow
        if( x < MAKE_HEX_DOUBLE(0x1.0p-24, 0x1LL, -24) )
        {
            if( f > 0 )
                return (1);
            return (sign);    
        }

        // half denormal
        if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1LL, -14) )
        {
            x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
            int r = (int) x;
            if( 0 == sign )
                r += (double) r != x;

            return ((uint16_t)( r | sign));
        }

        double g = u.f;
        u.u &= 0xFFFFFC0000000000ULL;
        if( g != u.f && 0 == sign)
            u.u += 0x0000040000000000ULL;
        u.u -= 0x3F00000000000000ULL;

        return ((u.u >> (53-11)) | sign);
    }


    uint16_t double2half_rtn( double f )
    {
        union{ double f; uint64_t u; } u = {f};
        uint64_t sign = (u.u >> 48) & 0x8000;
        double x = fabs(f);

        //Nan
        if( x != x )
        {
            u.u >>= (53-11);
            u.u &= 0x7fff;
            u.u |= 0x0200;      //silence the NaN
            return (u.u | sign);
        }

        // overflow
        if( f >= MAKE_HEX_DOUBLE(0x1.0p16, 0x1LL, 16) )
        {
            if( f == INFINITY )
                return (0x7c00);

            return (0x7bff);
        }

        if( f < MAKE_HEX_DOUBLE(-0x1.ffcp15, -0x1ffcLL, 3) )
            return (0xfc00);

        // underflow
        if( x < MAKE_HEX_DOUBLE(0x1.0p-24, 0x1LL, -24) )
        {
            if( f < 0 )
                return (0x8001);
            return (sign);    
        }

        // half denormal
        if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1LL, -14) )
        {
            x *= MAKE_HEX_DOUBLE(0x1.0p24, 0x1LL, 24);
            int r = (int) x;
            if( sign )
                r += (double) r != x;

            return ((uint16_t)( r | sign));
        }

        double g = u.f;
        u.u &= 0xFFFFFC0000000000ULL;
        if( g < u.f && sign)
            u.u += 0x0000040000000000ULL;
        u.u -= 0x3F00000000000000ULL;

        return ((u.u >> (53-11)) | sign);
    }

}
