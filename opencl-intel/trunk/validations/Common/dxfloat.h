///////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation, 2002.
//
// dxfloat.h
//
// Floating point
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// CFloat16 - 16 bit floating point number
//
// Represents 16-bit floating point number X with the following format:
//   1 sign bit (s)
//   5 bits of biased exponent (e)
//   10 bits of fraction (f)
//
// The value "v" of X is:
// (a) if 0 < e <= 31, then v = (-1)**s*2**(e-15)*(1.f)
// (c) if e == 0 and f == 0, then v = (-1)**s(0) (zero)
// (d) if e == 0 and f != 0, then v = (-1)**s*2**(e-14)*(0.f)
//
// exponents
// flt32   unbiased    flt16 exp      Float16 frac
//  255
//  254     127
//  ...
//  127+16   16 eMax   31 Max value 1.1111111111
//  127       0        15
//  113     -14 eMin    1           1.abcdefghij
//  112                 0   denorm  0.1abcdefghi
//  111                 0   denorm  0.01abcdefgh
//  110                 0   denorm  0.001abcdefg
//  109                 0   denorm  0.0001abcdef
//  108                 0   denorm  0.00001abcde
//  107                 0   denorm  0.000001abcd
//  106                 0   denorm  0.0000001abc
//  105                 0   denorm  0.00000001ab
//  104                 0   denorm  0.000000001a
//  103                 0   denorm  0.0000000001
//  102                 0           0.0
//  ...
//    0                             0.0
//
//-----------------------------------------------------------------------------
#include <limits>
#include "llvm/System/DataTypes.h"      // llvm data types
#include <iostream>

#ifndef DX_FLOAT_H
#define DX_FLOAT_H

class CFloat16
{
public:
    CFloat16()
    {
        v = 0;
    }
    // Conversion from 32-bit float
    //
    // auxilary union for safe type cast
    union CFloat16Convert
    {
        uint32_t dU32;
        float    dF32;
    };

    CFloat16(const float& fvalue)
    { InitFromFloat(fvalue); }

    CFloat16(const double& dvalue)
    { InitFromFloat((float)dvalue); }

protected:
    void InitFromFloat(const float& fvalue)
    {
        // cast from 32bit float to 32bit uint
        CFloat16Convert tmp16to32;
        tmp16to32.dF32 = fvalue;
        uint32_t u = tmp16to32.dU32;
        
        uint32_t Sign = (u & 0x80000000) >> 16;
        uint32_t MagU = u & 0x7FFFFFFF;     // Absolute value
        if (MagU > m_wMaxNormal)
        {
            /// Not representable by 16 bit float
            /// Check infinite cases
            if(MagU == m_cFloatInf)
            {
                if(Sign)
                    v = m_cMInf;
                else
                    v = m_cPInf;
            }
            /// Check NaN case
            else if(( (0x7FC00000 & u) == 0x7FC00000) && ((0x007FFFFF & u) != 0))
            {
                u >>= (24-11);
                u &= 0x7fff;
                u |= 0x0200;      //silence the NaN
                v = u | Sign;
            }
            else
                v = (uint16_t)(Sign | 0x7fff);
        }
        else
        if (MagU < m_wMinNormal)
        {
            // Denormalized value

            // Make implicit 1 explicit
            uint32_t Frac = (MagU & ((1<<23)-1)) | (1<<23);
            int nshift = (m_eMin + 127 - (MagU >> 23));

            if (nshift < 24)
            {
                MagU = Frac >> nshift;
            }
            else
            {
                MagU = 0;
            }

            // Round to nearest even
            v = (uint16_t)(Sign | ((MagU + (m_cRoundBit-1) +
                       ((MagU >> m_cFracBitsDiff) & 1)) >> m_cFracBitsDiff));
        }
        else
        {
            // Normalized value with Round to nearest even
            v = (uint16_t)(Sign | ((MagU + m_BiasDiffo + (m_cRoundBit-1) +
                       ((MagU >> m_cFracBitsDiff) & 1)) >> m_cFracBitsDiff));
        }
    }

public:
    // Conversion to 32-bit float
    //
    // Note: The infinity value (e=31) is converted the same way as any other
    // normalized value
    //
    operator float() const
    {
        uint64_t tmp;
        /// Work with infinite and NaN cases
        if(IsNaN())
        {
            CFloat16Convert fconv;
            fconv.dF32 = std::numeric_limits<float>::quiet_NaN();
            fconv.dU32 |= (v & m_cSignMask) << 16;
            return fconv.dF32;
        }
        if(IsPInf())
            return std::numeric_limits<float>::infinity();
        if(IsNInf())
            return -std::numeric_limits<float>::infinity();

        if ((v & ~(m_cSignMask | m_cFracMask)) == 0)
        {
            if ((v & m_cFracMask) != 0)
            {
                // Normalizing the denormalized value
                uint32_t exp = (uint32_t) m_eMin;
                uint32_t frac = v & m_cFracMask;
                while ((frac & (m_cFracMask + 1)) == 0)
                {
                    exp--;
                    frac <<= 1;
                }
                frac &= ~(m_cFracMask + 1); // Remove hidden bit
                tmp = ((v & m_cSignMask) << 16) | // Sigh bit
                      ((exp + 127) << 23) |     // Exponent
                      (frac << m_cFracBitsDiff);  // Fraction
            }
            else
            {
                // Zero - only sign bit is used
                tmp = (v & m_cSignMask) << 16;
            }
        }
        else
        {
            tmp = ((v & m_cSignMask) << 16) |                         // Sigh bit
                  ((((v >> m_cFracBits) & ((1 << m_cExpBits) - 1)) -
                    m_cExpBias + 127) << 23) |                        // Exponent
                  ((v & m_cFracMask) << m_cFracBitsDiff);               // Fraction
        }
        // cast from 64bit uint to 32bit float
        CFloat16Convert tmp16to32;
        tmp16to32.dU32 = uint32_t(0x0FFFFFFFFL & tmp);
        return tmp16to32.dF32;
    }

    bool operator==(const CFloat16& num) const
    {
        if(this->IsNaN() && num.IsNaN())
            return false;
        if(v == num.v)
        {
            return true;
        }
        else
            return false;
    }

    bool operator !=(const CFloat16& num)
    {
        return !(*this == num);
    }

    CFloat16(uint16_t in_uint)
    {
        v = in_uint;
    }

    CFloat16(int32_t in_int)
    {
        v = in_int;
    }

    CFloat16(uint64_t in_uint)
    {
        v = in_uint;
    }

    CFloat16(uint32_t in_uint)
    {
        v = in_uint;
    }

    static const CFloat16 GetMin()
    {
        return CFloat16((uint16_t)m_wMinNormal16);
    }

    static const CFloat16 GetMax()
    {
        return CFloat16((uint16_t)m_wMaxNormal16);
    }

    static const CFloat16 GetNInf()
    {
        return CFloat16((uint16_t)m_cMInf);
    }

    static const CFloat16 GetPInf()
    {
        return CFloat16((uint16_t)m_cPInf);
    }

    static const CFloat16 GetNaN()
    {
        return CFloat16((uint16_t)0x7FFF);
    }

    bool IsInf() const
    {
        return ( ( ( v & m_cExpMask ) == m_cExpMask ) && ( (v & m_cFracMask) == 0) );
    }

    bool IsNaN() const
    {
        return ( ( ( v & m_cExpMask ) == m_cExpMask ) && ( v & m_cFracMask) );
    }

    bool IsPInf() const
    {
        return (v == m_cPInf);
    }

    bool IsNInf() const
    {
        return (v == m_cMInf);
    }

    bool IsDenorm() const
    {
        return ( ( ( v & m_cExpMask ) == 0 ) && ( ( v & m_cFracMask ) != 0 ) );
    }

    uint16_t GetBits() const
    {
        return v;
    }

    static const uint32_t m_cFracBits = 10;           // Number of fraction bits
    static const uint32_t m_cExpBits = 5;             // Number of exponent bits
    static const uint32_t m_cSignBit = 15;            // Index of the sign bit
    static const uint32_t m_cSignMask = (1 << m_cSignBit);
    static const uint32_t m_cFracMask = (1 << m_cFracBits) - 1;         // Fraction mask
    static const int32_t  m_cExpBias = (1 << (m_cExpBits - 1)) - 1;     // Exponent bias
    static const int32_t  m_cExpMask = 0x7C00;          // Exponent mask
    static const uint32_t m_cRoundBit = 1 << (23 - m_cFracBits - 1);    // Bit to add for rounding
    static const uint32_t m_eMax =  (uint32_t) m_cExpBias+1;         // Max exponent
    static const int32_t  m_eMin = -m_cExpBias+1;       // Min exponent
    static const uint32_t m_wMaxNormal = ((m_eMax+127) << 23) | 0x7FEFFF;//  <-max nbr that doesnt round to infinity
    static const uint32_t m_wMinNormal = (m_eMin+127) << 23;
    static const uint16_t m_wMaxNormal16 = 0x7BFF;
    static const uint16_t m_wMinNormal16 = 0xFBFF;
    static const uint32_t m_BiasDiffo = ((uint32_t )m_cExpBias-127) << 23;
    static const uint32_t m_cFracBitsDiff = 23 - m_cFracBits;
    static const uint16_t m_cPInf = 0x7c00;
    static const uint16_t m_cMInf = 0xfc00;
    static const uint32_t m_cFloatInf = 0x7F800000;
    // support for serialization
    friend std::istream& operator >> (std::istream&,  CFloat16& );
    friend std::ostream& operator << (std::ostream&,  const CFloat16& );

protected:
    uint16_t  v;
};

// support for serialization
inline std::istream& operator >> (std::istream& is,  CFloat16& val)
{
    float f; 
    is >> f;
    if(is)
        val = CFloat16(f);
    return is;
}
inline std::ostream& operator << (std::ostream& os, const CFloat16& val)
{
    return os << float(val);  
}

namespace std{

template<>
inline CFloat16 numeric_limits<CFloat16>::infinity()  throw ()
{ return CFloat16::GetPInf();
}

template<>
inline CFloat16 numeric_limits<CFloat16>::quiet_NaN()  throw ()
{ return CFloat16::GetNaN();
}

}
#endif

