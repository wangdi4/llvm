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

File Name:  Utils.h

\*****************************************************************************/
#ifndef __UTILS_H__
#define __UTILS_H__

#include "llvm/System/DataTypes.h"      // LLVM data types
#include "dxfloat.h"

namespace Validation
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Auxiliary functions copy-pasted from inteloutputerrors
    /// constants for unique float precision values (from inteloutputerrors.h)
    ///	@brief a mask of float's exponent bits
    const uint32_t FLOAT_EXP_MASK = 0x7F800000;
    ///	@brief a mask of float's mantissa bits
    const uint32_t FLOAT_MANT_MASK = 0x007FFFFF;
    ///	@brief a mask of double's exponent bits
    const uint64_t DOUBLE_EXP_MASK = 0x7FF0000000000000;
    ///	@brief a mask of double's mantissa bits
    const uint64_t DOUBLE_MANT_MASK = 0x000FFFFFFFFFFFFF;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// helper functions for NaNs and infinities handling

    inline uint64_t AsUInt64(double d)
    {
        // to suppress gcc warning: dereferencing type-punned pointer will break strict-aliasing rules
        uint64_t *p = reinterpret_cast<uint64_t*>(&d); 
        return *p;
    }

    inline uint32_t AsUInt(float f)
    {
        // to suppress gcc warning: dereferencing type-punned pointer will break strict-aliasing rules
        uint32_t *p = reinterpret_cast<uint32_t*>(&f); 
        return *p;
    }

    inline bool IsNaN( float a )
    {
        uint32_t u = AsUInt(a);
        return ( ( ( u & FLOAT_EXP_MASK ) == FLOAT_EXP_MASK ) && ( u & FLOAT_MANT_MASK ) );
    }

    inline bool IsInf( float a )
    {
        uint32_t u = AsUInt(a);
        return ( ( ( u & FLOAT_EXP_MASK ) == FLOAT_EXP_MASK ) && ( (u & FLOAT_MANT_MASK) == 0) );
    }

    inline bool IsPInf( float a )
    {
        uint32_t ahex = AsUInt(a);
        return (ahex == 0x7F800000);
    }

    inline bool IsMInf( float a )
    {
        uint32_t ahex = AsUInt(a);
        return (ahex == 0xFF800000);
    }

    inline bool IsDenorm( float a )
    {
        uint32_t u = AsUInt(a);
        return ((u & FLOAT_EXP_MASK) == 0) && ((u & FLOAT_MANT_MASK) != 0);
    }

    inline bool IsNaN( double a )
    {
        uint64_t l = AsUInt64(a);
        return ( ( ( l & DOUBLE_EXP_MASK ) == DOUBLE_EXP_MASK ) && ( l & DOUBLE_MANT_MASK ) );
    }

    inline bool IsInf( double a )
    {
        uint64_t l = AsUInt64(a);
        return ( ( ( l & DOUBLE_EXP_MASK ) == DOUBLE_EXP_MASK ) && ( (l & DOUBLE_MANT_MASK) == 0) );
    }

    inline bool IsPInf( double a )
    {
        uint64_t ahex = AsUInt64(a);
        return (ahex == 0x7FF0000000000000);
    }

    inline bool IsMInf( double a )
    {
        uint64_t ahex = AsUInt64(a);
        return (ahex == 0xFFF0000000000000);
    }

    inline bool IsDenorm( double a )
    {
        uint64_t l = AsUInt64(a);
        return ((l & DOUBLE_EXP_MASK) == 0) && ((l & DOUBLE_MANT_MASK) != 0);
    }

    inline bool IsNaN( CFloat16 a )
    {
        return a.IsNaN();
    }

    inline bool IsInf( CFloat16 a )
    {
        return a.IsNaN();
    }

    inline bool IsPInf( CFloat16 a )
    {
        return a.IsPInf();
    }

    inline bool IsMInf( CFloat16 a )
    {
        return a.IsMInf();
    }

    inline bool IsDenorm( CFloat16 a )
    {
        return a.IsDenorm();
    }

} // End of Validation namespace
#endif // __UTILS_H__
