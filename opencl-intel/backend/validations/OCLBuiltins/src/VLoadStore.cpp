/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Reference OpenCL Builtins                                                   *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  VLoadStore.cpp

\*****************************************************************************/

#include "VLoadStore.h"
#include "Conformance/reference_convert.h"

using namespace llvm;
using namespace Validation::OCLBuiltins;

#ifndef BUILTINS_API
   #if defined(_WIN32)
      #define BUILTINS_API __declspec(dllexport)
   #else
      #define BUILTINS_API
   #endif
#endif

namespace Validation {
namespace OCLBuiltins {

template<> llvm::GenericValue lle_X_vload_half<3, true>(
    llvm::FunctionType *FT,
    const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue R;
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    uint32_t offset  = arg0.IntVal.getZExtValue();
    offset *= 4;
    uint16_t* p = static_cast<uint16_t*>(arg1.PointerVal);
    p += offset;
    R.AggregateVal.resize(3);
    for (uint32_t i = 0; i < 3; ++i, ++p)
    {
        getRef<float, 3> (R, i) = float(CFloat16(*p));
    }
    return R;
}

template<>
uint16_t convert2half<float>(float f)
{
    return Conformance::float2half_rte(f);
}

template<>
uint16_t convert2half<double>(double f)
{
    return Conformance::double2half_rte(f);
}


template<> llvm::GenericValue lle_X_vstore_half<float, 3, true>(
    llvm::FunctionType *FT,
    const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    uint32_t offset  = arg1.IntVal.getZExtValue();
    offset *= 4;
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);
    p += offset;
    for (uint32_t i = 0; i < 3; ++i, ++p)
    {
        *p = CFloat16(getVal<float, 3>(arg0, i)).GetBits();
    }
    return llvm::GenericValue();
}

template<> llvm::GenericValue lle_X_vstore_half<double, 3, true>(
    llvm::FunctionType *FT,
    const std::vector<llvm::GenericValue> &Args)
{
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    uint32_t offset  = arg1.IntVal.getZExtValue();
    offset *= 4;
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);
    p += offset;
    for (uint32_t i = 0; i < 3; ++i, ++p)
    {
        *p = Conformance::double2half_rte(getVal<double, 3>(arg0, i));
    }
    return llvm::GenericValue();
}

#define VSTOREF_HALF_CONVERT(RMODE)                                                 \
template<int n, bool aligned>                                                       \
llvm::GenericValue lle_X_vstoref_half_ ## RMODE (llvm::FunctionType *FT,      \
                                     const std::vector<llvm::GenericValue> &Args)   \
{                                                                                   \
    llvm::GenericValue arg0 = Args[0];                                              \
    llvm::GenericValue arg1 = Args[1];                                              \
    llvm::GenericValue arg2 = Args[2];                                              \
    uint32_t offset  = arg1.IntVal.getZExtValue();                                  \
    offset *= n;                                                                    \
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);                          \
    p += offset;                                                                    \
    for (uint32_t i = 0; i < n; ++i, ++p)                                           \
    {                                                                               \
        *p = Conformance::float2half_ ## RMODE(getVal<float, n>(arg0, i));          \
    }                                                                               \
    return llvm::GenericValue();                                                    \
}

VSTOREF_HALF_CONVERT(rte)
VSTOREF_HALF_CONVERT(rtz)
VSTOREF_HALF_CONVERT(rtp)
VSTOREF_HALF_CONVERT(rtn)

#define VSTORED_HALF_CONVERT(RMODE)                                                 \
template<int n, bool aligned>                                                       \
llvm::GenericValue lle_X_vstored_half_ ## RMODE (llvm::FunctionType *FT,      \
                                    const std::vector<llvm::GenericValue> &Args)    \
{                                                                                   \
    llvm::GenericValue arg0 = Args[0];                                              \
    llvm::GenericValue arg1 = Args[1];                                              \
    llvm::GenericValue arg2 = Args[2];                                              \
    uint32_t offset  = arg1.IntVal.getZExtValue();                                  \
    offset *= n;                                                                    \
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);                          \
    p += offset;                                                                    \
    for (uint32_t i = 0; i < n; ++i, ++p)                                           \
    {                                                                               \
        *p = Conformance::double2half_ ## RMODE(getVal<double, n>(arg0, i));        \
    }                                                                               \
    return llvm::GenericValue();                                                    \
}

VSTORED_HALF_CONVERT(rte)
VSTORED_HALF_CONVERT(rtz)
VSTORED_HALF_CONVERT(rtp)
VSTORED_HALF_CONVERT(rtn)

#define VSTOREAF_HALF_CONVERT(RMODE)                                        \
template<> llvm::GenericValue lle_X_vstoref_half_ ## RMODE <3, true>(       \
    llvm::FunctionType *FT,                                           \
    const std::vector<llvm::GenericValue> &Args)                            \
{                                                                           \
    llvm::GenericValue arg0 = Args[0];                                      \
    llvm::GenericValue arg1 = Args[1];                                      \
    llvm::GenericValue arg2 = Args[2];                                      \
    uint32_t offset  = arg1.IntVal.getZExtValue();                          \
    offset *= 4;                                                            \
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);                  \
    p += offset;                                                            \
    for (uint32_t i = 0; i < 3; ++i, ++p)                                   \
    {                                                                       \
        *p = Conformance::float2half_ ## RMODE(getVal<float, 3>(arg0, i));  \
    }                                                                       \
    return llvm::GenericValue();                                            \
}

VSTOREAF_HALF_CONVERT(rte)
VSTOREAF_HALF_CONVERT(rtz)
VSTOREAF_HALF_CONVERT(rtp)
VSTOREAF_HALF_CONVERT(rtn)

#define VSTOREAD_HALF_CONVERT(RMODE)                                            \
template<> llvm::GenericValue lle_X_vstored_half_ ## RMODE <3, true>(           \
    llvm::FunctionType *FT,                                               \
    const std::vector<llvm::GenericValue> &Args)                                \
{                                                                               \
    llvm::GenericValue arg0 = Args[0];                                          \
    llvm::GenericValue arg1 = Args[1];                                          \
    llvm::GenericValue arg2 = Args[2];                                          \
    uint32_t offset  = arg1.IntVal.getZExtValue();                              \
    offset *= 4;                                                                \
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);                      \
    p += offset;                                                                \
    for (uint32_t i = 0; i < 3; ++i, ++p)                                       \
    {                                                                           \
        *p = Conformance::double2half_ ## RMODE(getVal<double, 3>(arg0, i));    \
    }                                                                           \
    return llvm::GenericValue();                                                \
}

VSTOREAD_HALF_CONVERT(rte)
VSTOREAD_HALF_CONVERT(rtz)
VSTOREAD_HALF_CONVERT(rtp)
VSTOREAD_HALF_CONVERT(rtn)
}
}

extern "C" {
BUILTINS_API void initOCLBuiltinsVLoadStore() {return;}
  

  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,16>(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,16>(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,16>(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,16>(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,16>(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,16>(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,16>(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,16>(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,16>(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,16>(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,16>(FT,Args);}//10
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,16>(FT,Args);}//11
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,16>(FT,Args);}//12
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,16>(FT,Args);}//13
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,16>(FT,Args);}//14
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,16>(FT,Args);}//15
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,16>(FT,Args);}//16
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,16>(FT,Args);}//17
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,16>(FT,Args);}//18
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,16>(FT,Args);}//19
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,16>(FT,Args);}//20
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,16>(FT,Args);}//21
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,16>(FT,Args);}//22
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,16>(FT,Args);}//23
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,16>(FT,Args);}//24
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,16>(FT,Args);}//25
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,16>(FT,Args);}//26
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,16>(FT,Args);}//27
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,16>(FT,Args);}//28
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,16>(FT,Args);}//29
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,16>(FT,Args);}//30
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,16>(FT,Args);}//31
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,16>(FT,Args);}//32
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,16>(FT,Args);}//33
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,16>(FT,Args);}//34
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,16>(FT,Args);}//35
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,16>(FT,Args);}//36
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,16>(FT,Args);}//37
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,16>(FT,Args);}//38
  BUILTINS_API llvm::GenericValue lle_X__Z7vload16mPKd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,16>(FT,Args);}//39
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,2>(FT,Args);}//40
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,2>(FT,Args);}//41
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,2>(FT,Args);}//42
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,2>(FT,Args);}//43
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,2>(FT,Args);}//44
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,2>(FT,Args);}//45
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,2>(FT,Args);}//46
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,2>(FT,Args);}//47
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,2>(FT,Args);}//48
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,2>(FT,Args);}//49
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,2>(FT,Args);}//50
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,2>(FT,Args);}//51
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,2>(FT,Args);}//52
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,2>(FT,Args);}//53
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,2>(FT,Args);}//54
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,2>(FT,Args);}//55
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,2>(FT,Args);}//56
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,2>(FT,Args);}//57
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,2>(FT,Args);}//58
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,2>(FT,Args);}//59
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,2>(FT,Args);}//60
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,2>(FT,Args);}//61
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,2>(FT,Args);}//62
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,2>(FT,Args);}//63
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,2>(FT,Args);}//64
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,2>(FT,Args);}//65
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,2>(FT,Args);}//66
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,2>(FT,Args);}//67
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,2>(FT,Args);}//68
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,2>(FT,Args);}//69
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,2>(FT,Args);}//70
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,2>(FT,Args);}//71
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,2>(FT,Args);}//72
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,2>(FT,Args);}//73
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,2>(FT,Args);}//74
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,2>(FT,Args);}//75
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,2>(FT,Args);}//76
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,2>(FT,Args);}//77
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,2>(FT,Args);}//78
  BUILTINS_API llvm::GenericValue lle_X__Z6vload2mPKd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,2>(FT,Args);}//79
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,3>(FT,Args);}//80
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,3>(FT,Args);}//81
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,3>(FT,Args);}//82
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,3>(FT,Args);}//83
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,3>(FT,Args);}//84
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,3>(FT,Args);}//85
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,3>(FT,Args);}//86
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,3>(FT,Args);}//87
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,3>(FT,Args);}//88
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,3>(FT,Args);}//89
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,3>(FT,Args);}//90
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,3>(FT,Args);}//91
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,3>(FT,Args);}//92
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,3>(FT,Args);}//93
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,3>(FT,Args);}//94
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,3>(FT,Args);}//95
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,3>(FT,Args);}//96
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,3>(FT,Args);}//97
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,3>(FT,Args);}//98
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,3>(FT,Args);}//99
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,3>(FT,Args);}//100
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,3>(FT,Args);}//101
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,3>(FT,Args);}//102
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,3>(FT,Args);}//103
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,3>(FT,Args);}//104
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,3>(FT,Args);}//105
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,3>(FT,Args);}//106
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,3>(FT,Args);}//107
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,3>(FT,Args);}//108
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,3>(FT,Args);}//109
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,3>(FT,Args);}//110
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,3>(FT,Args);}//111
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,3>(FT,Args);}//112
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,3>(FT,Args);}//113
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,3>(FT,Args);}//114
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,3>(FT,Args);}//115
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,3>(FT,Args);}//116
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,3>(FT,Args);}//117
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,3>(FT,Args);}//118
  BUILTINS_API llvm::GenericValue lle_X__Z6vload3mPKd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,3>(FT,Args);}//119
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,4>(FT,Args);}//120
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,4>(FT,Args);}//121
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,4>(FT,Args);}//122
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,4>(FT,Args);}//123
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,4>(FT,Args);}//124
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,4>(FT,Args);}//125
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,4>(FT,Args);}//126
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,4>(FT,Args);}//127
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,4>(FT,Args);}//128
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,4>(FT,Args);}//129
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,4>(FT,Args);}//130
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,4>(FT,Args);}//131
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,4>(FT,Args);}//132
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,4>(FT,Args);}//133
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,4>(FT,Args);}//134
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,4>(FT,Args);}//135
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,4>(FT,Args);}//136
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,4>(FT,Args);}//137
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,4>(FT,Args);}//138
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,4>(FT,Args);}//139
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,4>(FT,Args);}//140
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,4>(FT,Args);}//141
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,4>(FT,Args);}//142
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,4>(FT,Args);}//143
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,4>(FT,Args);}//144
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,4>(FT,Args);}//145
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,4>(FT,Args);}//146
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,4>(FT,Args);}//147
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,4>(FT,Args);}//148
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,4>(FT,Args);}//149
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,4>(FT,Args);}//150
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,4>(FT,Args);}//151
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,4>(FT,Args);}//152
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,4>(FT,Args);}//153
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,4>(FT,Args);}//154
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,4>(FT,Args);}//155
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,4>(FT,Args);}//156
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,4>(FT,Args);}//157
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,4>(FT,Args);}//158
  BUILTINS_API llvm::GenericValue lle_X__Z6vload4mPKd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,4>(FT,Args);}//159
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,8>(FT,Args);}//160
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,8>(FT,Args);}//161
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,8>(FT,Args);}//162
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,8>(FT,Args);}//163
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,8>(FT,Args);}//164
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,8>(FT,Args);}//165
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,8>(FT,Args);}//166
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,8>(FT,Args);}//167
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,8>(FT,Args);}//168
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS2d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,8>(FT,Args);}//169
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,8>(FT,Args);}//170
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,8>(FT,Args);}//171
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,8>(FT,Args);}//172
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,8>(FT,Args);}//173
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,8>(FT,Args);}//174
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,8>(FT,Args);}//175
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,8>(FT,Args);}//176
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,8>(FT,Args);}//177
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,8>(FT,Args);}//178
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,8>(FT,Args);}//179
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,8>(FT,Args);}//180
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,8>(FT,Args);}//181
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,8>(FT,Args);}//182
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,8>(FT,Args);}//183
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,8>(FT,Args);}//184
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,8>(FT,Args);}//185
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,8>(FT,Args);}//186
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,8>(FT,Args);}//187
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,8>(FT,Args);}//188
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,8>(FT,Args);}//189
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int8_t,8>(FT,Args);}//190
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint8_t,8>(FT,Args);}//191
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int16_t,8>(FT,Args);}//192
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint16_t,8>(FT,Args);}//193
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int32_t,8>(FT,Args);}//194
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint32_t,8>(FT,Args);}//195
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<int64_t,8>(FT,Args);}//196
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<uint64_t,8>(FT,Args);}//197
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<float,8>(FT,Args);}//198
  BUILTINS_API llvm::GenericValue lle_X__Z6vload8mPKd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload<double,8>(FT,Args);}//199
  BUILTINS_API llvm::GenericValue lle_X__Z10vload_halfmPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<1,false>(FT,Args);}//200
  BUILTINS_API llvm::GenericValue lle_X__Z10vload_halfmPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<1,false>(FT,Args);}//201
  BUILTINS_API llvm::GenericValue lle_X__Z10vload_halfmPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<1,false>(FT,Args);}//202
  BUILTINS_API llvm::GenericValue lle_X__Z10vload_halfmPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<1,false>(FT,Args);}//203
  BUILTINS_API llvm::GenericValue lle_X__Z12vload_half16mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<16,false>(FT,Args);}//204
  BUILTINS_API llvm::GenericValue lle_X__Z12vload_half16mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<16,false>(FT,Args);}//205
  BUILTINS_API llvm::GenericValue lle_X__Z12vload_half16mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<16,false>(FT,Args);}//206
  BUILTINS_API llvm::GenericValue lle_X__Z12vload_half16mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<16,false>(FT,Args);}//207
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half2mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<2,false>(FT,Args);}//208
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half2mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<2,false>(FT,Args);}//209
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half2mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<2,false>(FT,Args);}//210
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half2mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<2,false>(FT,Args);}//211
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half3mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<3,false>(FT,Args);}//212
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half3mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<3,false>(FT,Args);}//213
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half3mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<3,false>(FT,Args);}//214
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half3mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<3,false>(FT,Args);}//215
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half4mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<4,false>(FT,Args);}//216
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half4mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<4,false>(FT,Args);}//217
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half4mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<4,false>(FT,Args);}//218
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half4mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<4,false>(FT,Args);}//219
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half8mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<8,false>(FT,Args);}//220
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half8mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<8,false>(FT,Args);}//221
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half8mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<8,false>(FT,Args);}//222
  BUILTINS_API llvm::GenericValue lle_X__Z11vload_half8mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<8,false>(FT,Args);}//223
  BUILTINS_API llvm::GenericValue lle_X__Z11vloada_halfmPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<1,true>(FT,Args);}//224
  BUILTINS_API llvm::GenericValue lle_X__Z11vloada_halfmPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<1,true>(FT,Args);}//225
  BUILTINS_API llvm::GenericValue lle_X__Z11vloada_halfmPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<1,true>(FT,Args);}//226
  BUILTINS_API llvm::GenericValue lle_X__Z11vloada_halfmPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<1,true>(FT,Args);}//227
  BUILTINS_API llvm::GenericValue lle_X__Z13vloada_half16mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<16,true>(FT,Args);}//228
  BUILTINS_API llvm::GenericValue lle_X__Z13vloada_half16mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<16,true>(FT,Args);}//229
  BUILTINS_API llvm::GenericValue lle_X__Z13vloada_half16mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<16,true>(FT,Args);}//230
  BUILTINS_API llvm::GenericValue lle_X__Z13vloada_half16mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<16,true>(FT,Args);}//231
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half2mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<2,true>(FT,Args);}//232
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half2mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<2,true>(FT,Args);}//233
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half2mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<2,true>(FT,Args);}//234
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half2mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<2,true>(FT,Args);}//235
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half3mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<3,true>(FT,Args);}//236
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half3mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<3,true>(FT,Args);}//237
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half3mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<3,true>(FT,Args);}//238
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half3mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<3,true>(FT,Args);}//239
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half4mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<4,true>(FT,Args);}//240
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half4mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<4,true>(FT,Args);}//241
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half4mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<4,true>(FT,Args);}//242
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half4mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<4,true>(FT,Args);}//243
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half8mPKU3AS2Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<8,true>(FT,Args);}//244
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half8mPKU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<8,true>(FT,Args);}//245
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half8mPKU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<8,true>(FT,Args);}//246
  BUILTINS_API llvm::GenericValue lle_X__Z12vloada_half8mPKDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vload_half<8,true>(FT,Args);}//247
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_cmPU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,16>(FT,Args);}//248
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_hmPU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,16>(FT,Args);}//249
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_smPU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,16>(FT,Args);}//250
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_tmPU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,16>(FT,Args);}//251
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_imPU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,16>(FT,Args);}//252
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_jmPU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,16>(FT,Args);}//253
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_lmPU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,16>(FT,Args);}//254
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_mmPU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,16>(FT,Args);}//255
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_fmPU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,16>(FT,Args);}//256
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_dmPU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,16>(FT,Args);}//257
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_cmPU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,16>(FT,Args);}//258
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_hmPU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,16>(FT,Args);}//259
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_smPU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,16>(FT,Args);}//260
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_tmPU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,16>(FT,Args);}//261
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_imPU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,16>(FT,Args);}//262
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_jmPU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,16>(FT,Args);}//263
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_lmPU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,16>(FT,Args);}//264
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_mmPU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,16>(FT,Args);}//265
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_fmPU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,16>(FT,Args);}//266
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_dmPU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,16>(FT,Args);}//267
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_cmPc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,16>(FT,Args);}//268
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_hmPh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,16>(FT,Args);}//269
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_smPs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,16>(FT,Args);}//270
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_tmPt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,16>(FT,Args);}//271
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_imPi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,16>(FT,Args);}//272
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_jmPj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,16>(FT,Args);}//273
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_lmPl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,16>(FT,Args);}//274
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_mmPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,16>(FT,Args);}//275
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_fmPf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,16>(FT,Args);}//276
  BUILTINS_API llvm::GenericValue lle_X__Z8vstore16Dv16_dmPd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,16>(FT,Args);}//277
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_cmPU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,2>(FT,Args);}//278
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_hmPU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,2>(FT,Args);}//279
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_smPU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,2>(FT,Args);}//280
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_tmPU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,2>(FT,Args);}//281
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_imPU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,2>(FT,Args);}//282
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_jmPU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,2>(FT,Args);}//283
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_lmPU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,2>(FT,Args);}//284
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_mmPU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,2>(FT,Args);}//285
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_fmPU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,2>(FT,Args);}//286
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_dmPU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,2>(FT,Args);}//287
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_cmPU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,2>(FT,Args);}//288
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_hmPU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,2>(FT,Args);}//289
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_smPU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,2>(FT,Args);}//290
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_tmPU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,2>(FT,Args);}//291
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_imPU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,2>(FT,Args);}//292
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_jmPU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,2>(FT,Args);}//293
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_lmPU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,2>(FT,Args);}//294
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_mmPU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,2>(FT,Args);}//295
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_fmPU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,2>(FT,Args);}//296
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_dmPU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,2>(FT,Args);}//297
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_cmPc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,2>(FT,Args);}//298
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_hmPh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,2>(FT,Args);}//299
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_smPs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,2>(FT,Args);}//300
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_tmPt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,2>(FT,Args);}//301
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_imPi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,2>(FT,Args);}//302
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_jmPj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,2>(FT,Args);}//303
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_lmPl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,2>(FT,Args);}//304
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_mmPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,2>(FT,Args);}//305
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_fmPf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,2>(FT,Args);}//306
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore2Dv2_dmPd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,2>(FT,Args);}//307
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_cmPU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,3>(FT,Args);}//308
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_hmPU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,3>(FT,Args);}//309
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_smPU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,3>(FT,Args);}//310
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_tmPU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,3>(FT,Args);}//311
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_imPU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,3>(FT,Args);}//312
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_jmPU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,3>(FT,Args);}//313
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_lmPU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,3>(FT,Args);}//314
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_mmPU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,3>(FT,Args);}//315
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_fmPU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,3>(FT,Args);}//316
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_dmPU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,3>(FT,Args);}//317
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_cmPU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,3>(FT,Args);}//318
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_hmPU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,3>(FT,Args);}//319
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_smPU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,3>(FT,Args);}//320
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_tmPU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,3>(FT,Args);}//321
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_imPU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,3>(FT,Args);}//322
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_jmPU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,3>(FT,Args);}//323
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_lmPU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,3>(FT,Args);}//324
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_mmPU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,3>(FT,Args);}//325
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_fmPU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,3>(FT,Args);}//326
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_dmPU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,3>(FT,Args);}//327
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_cmPc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,3>(FT,Args);}//328
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_hmPh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,3>(FT,Args);}//329
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_smPs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,3>(FT,Args);}//330
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_tmPt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,3>(FT,Args);}//331
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_imPi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,3>(FT,Args);}//332
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_jmPj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,3>(FT,Args);}//333
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_lmPl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,3>(FT,Args);}//334
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_mmPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,3>(FT,Args);}//335
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_fmPf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,3>(FT,Args);}//336
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore3Dv3_dmPd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,3>(FT,Args);}//337
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_cmPU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,4>(FT,Args);}//338
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_hmPU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,4>(FT,Args);}//339
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_smPU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,4>(FT,Args);}//340
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_tmPU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,4>(FT,Args);}//341
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_imPU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,4>(FT,Args);}//342
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_jmPU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,4>(FT,Args);}//343
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_lmPU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,4>(FT,Args);}//344
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_mmPU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,4>(FT,Args);}//345
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_fmPU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,4>(FT,Args);}//346
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_dmPU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,4>(FT,Args);}//347
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_cmPU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,4>(FT,Args);}//348
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_hmPU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,4>(FT,Args);}//349
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_smPU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,4>(FT,Args);}//350
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_tmPU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,4>(FT,Args);}//351
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_imPU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,4>(FT,Args);}//352
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_jmPU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,4>(FT,Args);}//353
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_lmPU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,4>(FT,Args);}//354
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_mmPU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,4>(FT,Args);}//355
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_fmPU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,4>(FT,Args);}//356
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_dmPU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,4>(FT,Args);}//357
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_cmPc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,4>(FT,Args);}//358
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_hmPh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,4>(FT,Args);}//359
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_smPs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,4>(FT,Args);}//360
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_tmPt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,4>(FT,Args);}//361
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_imPi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,4>(FT,Args);}//362
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_jmPj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,4>(FT,Args);}//363
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_lmPl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,4>(FT,Args);}//364
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_mmPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,4>(FT,Args);}//365
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_fmPf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,4>(FT,Args);}//366
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore4Dv4_dmPd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,4>(FT,Args);}//367
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_cmPU3AS1c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,8>(FT,Args);}//368
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_hmPU3AS1h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,8>(FT,Args);}//369
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_smPU3AS1s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,8>(FT,Args);}//370
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_tmPU3AS1t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,8>(FT,Args);}//371
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_imPU3AS1i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,8>(FT,Args);}//372
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_jmPU3AS1j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,8>(FT,Args);}//373
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_lmPU3AS1l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,8>(FT,Args);}//374
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_mmPU3AS1m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,8>(FT,Args);}//375
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_fmPU3AS1f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,8>(FT,Args);}//376
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_dmPU3AS1d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,8>(FT,Args);}//377
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_cmPU3AS3c( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,8>(FT,Args);}//378
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_hmPU3AS3h( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,8>(FT,Args);}//379
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_smPU3AS3s( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,8>(FT,Args);}//380
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_tmPU3AS3t( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,8>(FT,Args);}//381
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_imPU3AS3i( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,8>(FT,Args);}//382
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_jmPU3AS3j( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,8>(FT,Args);}//383
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_lmPU3AS3l( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,8>(FT,Args);}//384
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_mmPU3AS3m( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,8>(FT,Args);}//385
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_fmPU3AS3f( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,8>(FT,Args);}//386
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_dmPU3AS3d( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,8>(FT,Args);}//387
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_cmPc( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int8_t,8>(FT,Args);}//388
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_hmPh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint8_t,8>(FT,Args);}//389
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_smPs( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int16_t,8>(FT,Args);}//390
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_tmPt( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint16_t,8>(FT,Args);}//391
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_imPi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int32_t,8>(FT,Args);}//392
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_jmPj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint32_t,8>(FT,Args);}//393
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_lmPl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<int64_t,8>(FT,Args);}//394
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_mmPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<uint64_t,8>(FT,Args);}//395
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_fmPf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<float,8>(FT,Args);}//396
  BUILTINS_API llvm::GenericValue lle_X__Z7vstore8Dv8_dmPd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore<double,8>(FT,Args);}//397
  BUILTINS_API llvm::GenericValue lle_X__Z11vstore_halfdmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,1,false>(FT,Args);}//398
  BUILTINS_API llvm::GenericValue lle_X__Z11vstore_halfdmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,1,false>(FT,Args);}//399
  BUILTINS_API llvm::GenericValue lle_X__Z11vstore_halfdmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,1,false>(FT,Args);}//400
  BUILTINS_API llvm::GenericValue lle_X__Z11vstore_halffmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,1,false>(FT,Args);}//401
  BUILTINS_API llvm::GenericValue lle_X__Z11vstore_halffmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,1,false>(FT,Args);}//402
  BUILTINS_API llvm::GenericValue lle_X__Z11vstore_halffmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,1,false>(FT,Args);}//403
  BUILTINS_API llvm::GenericValue lle_X__Z13vstore_half16Dv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,16,false>(FT,Args);}//404
  BUILTINS_API llvm::GenericValue lle_X__Z13vstore_half16Dv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,16,false>(FT,Args);}//405
  BUILTINS_API llvm::GenericValue lle_X__Z13vstore_half16Dv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,16,false>(FT,Args);}//406
  BUILTINS_API llvm::GenericValue lle_X__Z13vstore_half16Dv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,16,false>(FT,Args);}//407
  BUILTINS_API llvm::GenericValue lle_X__Z13vstore_half16Dv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,16,false>(FT,Args);}//408
  BUILTINS_API llvm::GenericValue lle_X__Z13vstore_half16Dv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,16,false>(FT,Args);}//409
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rteDv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<16,false>(FT,Args);}//410
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rteDv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<16,false>(FT,Args);}//411
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rteDv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<16,false>(FT,Args);}//412
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rteDv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<16,false>(FT,Args);}//413
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rteDv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<16,false>(FT,Args);}//414
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rteDv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<16,false>(FT,Args);}//415
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtnDv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<16,false>(FT,Args);}//416
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtnDv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<16,false>(FT,Args);}//417
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtnDv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<16,false>(FT,Args);}//418
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtnDv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<16,false>(FT,Args);}//419
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtnDv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<16,false>(FT,Args);}//420
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtnDv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<16,false>(FT,Args);}//421
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtpDv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<16,false>(FT,Args);}//422
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtpDv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<16,false>(FT,Args);}//423
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtpDv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<16,false>(FT,Args);}//424
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtpDv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<16,false>(FT,Args);}//425
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtpDv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<16,false>(FT,Args);}//426
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtpDv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<16,false>(FT,Args);}//427
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtzDv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<16,false>(FT,Args);}//428
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtzDv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<16,false>(FT,Args);}//429
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtzDv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<16,false>(FT,Args);}//430
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtzDv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<16,false>(FT,Args);}//431
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtzDv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<16,false>(FT,Args);}//432
  BUILTINS_API llvm::GenericValue lle_X__Z17vstore_half16_rtzDv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<16,false>(FT,Args);}//433
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half2Dv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,2,false>(FT,Args);}//434
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half2Dv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,2,false>(FT,Args);}//435
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half2Dv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,2,false>(FT,Args);}//436
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half2Dv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,2,false>(FT,Args);}//437
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half2Dv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,2,false>(FT,Args);}//438
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half2Dv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,2,false>(FT,Args);}//439
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rteDv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<2,false>(FT,Args);}//440
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rteDv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<2,false>(FT,Args);}//441
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rteDv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<2,false>(FT,Args);}//442
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rteDv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<2,false>(FT,Args);}//443
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rteDv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<2,false>(FT,Args);}//444
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rteDv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<2,false>(FT,Args);}//445
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtnDv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<2,false>(FT,Args);}//446
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtnDv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<2,false>(FT,Args);}//447
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtnDv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<2,false>(FT,Args);}//448
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtnDv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<2,false>(FT,Args);}//449
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtnDv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<2,false>(FT,Args);}//450
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtnDv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<2,false>(FT,Args);}//451
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtpDv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<2,false>(FT,Args);}//452
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtpDv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<2,false>(FT,Args);}//453
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtpDv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<2,false>(FT,Args);}//454
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtpDv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<2,false>(FT,Args);}//455
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtpDv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<2,false>(FT,Args);}//456
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtpDv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<2,false>(FT,Args);}//457
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtzDv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<2,false>(FT,Args);}//458
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtzDv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<2,false>(FT,Args);}//459
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtzDv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<2,false>(FT,Args);}//460
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtzDv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<2,false>(FT,Args);}//461
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtzDv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<2,false>(FT,Args);}//462
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half2_rtzDv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<2,false>(FT,Args);}//463
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half3Dv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,3,false>(FT,Args);}//464
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half3Dv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,3,false>(FT,Args);}//465
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half3Dv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,3,false>(FT,Args);}//466
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half3Dv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,3,false>(FT,Args);}//467
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half3Dv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,3,false>(FT,Args);}//468
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half3Dv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,3,false>(FT,Args);}//469
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rteDv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<3,false>(FT,Args);}//470
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rteDv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<3,false>(FT,Args);}//471
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rteDv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<3,false>(FT,Args);}//472
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rteDv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<3,false>(FT,Args);}//473
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rteDv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<3,false>(FT,Args);}//474
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rteDv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<3,false>(FT,Args);}//475
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtnDv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<3,false>(FT,Args);}//476
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtnDv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<3,false>(FT,Args);}//477
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtnDv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<3,false>(FT,Args);}//478
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtnDv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<3,false>(FT,Args);}//479
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtnDv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<3,false>(FT,Args);}//480
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtnDv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<3,false>(FT,Args);}//481
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtpDv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<3,false>(FT,Args);}//482
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtpDv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<3,false>(FT,Args);}//483
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtpDv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<3,false>(FT,Args);}//484
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtpDv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<3,false>(FT,Args);}//485
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtpDv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<3,false>(FT,Args);}//486
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtpDv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<3,false>(FT,Args);}//487
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtzDv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<3,false>(FT,Args);}//488
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtzDv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<3,false>(FT,Args);}//489
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtzDv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<3,false>(FT,Args);}//490
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtzDv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<3,false>(FT,Args);}//491
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtzDv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<3,false>(FT,Args);}//492
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half3_rtzDv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<3,false>(FT,Args);}//493
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half4Dv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,4,false>(FT,Args);}//494
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half4Dv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,4,false>(FT,Args);}//495
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half4Dv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,4,false>(FT,Args);}//496
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half4Dv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,4,false>(FT,Args);}//497
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half4Dv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,4,false>(FT,Args);}//498
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half4Dv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,4,false>(FT,Args);}//499
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rteDv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<4,false>(FT,Args);}//500
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rteDv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<4,false>(FT,Args);}//501
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rteDv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<4,false>(FT,Args);}//502
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rteDv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<4,false>(FT,Args);}//503
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rteDv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<4,false>(FT,Args);}//504
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rteDv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<4,false>(FT,Args);}//505
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtnDv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<4,false>(FT,Args);}//506
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtnDv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<4,false>(FT,Args);}//507
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtnDv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<4,false>(FT,Args);}//508
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtnDv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<4,false>(FT,Args);}//509
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtnDv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<4,false>(FT,Args);}//510
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtnDv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<4,false>(FT,Args);}//511
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtpDv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<4,false>(FT,Args);}//512
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtpDv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<4,false>(FT,Args);}//513
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtpDv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<4,false>(FT,Args);}//514
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtpDv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<4,false>(FT,Args);}//515
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtpDv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<4,false>(FT,Args);}//516
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtpDv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<4,false>(FT,Args);}//517
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtzDv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<4,false>(FT,Args);}//518
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtzDv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<4,false>(FT,Args);}//519
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtzDv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<4,false>(FT,Args);}//520
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtzDv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<4,false>(FT,Args);}//521
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtzDv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<4,false>(FT,Args);}//522
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half4_rtzDv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<4,false>(FT,Args);}//523
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half8Dv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,8,false>(FT,Args);}//524
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half8Dv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,8,false>(FT,Args);}//525
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half8Dv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,8,false>(FT,Args);}//526
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half8Dv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,8,false>(FT,Args);}//527
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half8Dv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,8,false>(FT,Args);}//528
  BUILTINS_API llvm::GenericValue lle_X__Z12vstore_half8Dv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,8,false>(FT,Args);}//529
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rteDv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<8,false>(FT,Args);}//530
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rteDv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<8,false>(FT,Args);}//531
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rteDv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<8,false>(FT,Args);}//532
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rteDv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<8,false>(FT,Args);}//533
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rteDv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<8,false>(FT,Args);}//534
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rteDv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<8,false>(FT,Args);}//535
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtnDv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<8,false>(FT,Args);}//536
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtnDv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<8,false>(FT,Args);}//537
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtnDv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<8,false>(FT,Args);}//538
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtnDv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<8,false>(FT,Args);}//539
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtnDv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<8,false>(FT,Args);}//540
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtnDv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<8,false>(FT,Args);}//541
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtpDv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<8,false>(FT,Args);}//542
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtpDv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<8,false>(FT,Args);}//543
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtpDv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<8,false>(FT,Args);}//544
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtpDv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<8,false>(FT,Args);}//545
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtpDv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<8,false>(FT,Args);}//546
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtpDv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<8,false>(FT,Args);}//547
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtzDv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<8,false>(FT,Args);}//548
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtzDv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<8,false>(FT,Args);}//549
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtzDv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<8,false>(FT,Args);}//550
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtzDv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<8,false>(FT,Args);}//551
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtzDv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<8,false>(FT,Args);}//552
  BUILTINS_API llvm::GenericValue lle_X__Z16vstore_half8_rtzDv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<8,false>(FT,Args);}//553
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtedmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<1,false>(FT,Args);}//554
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtedmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<1,false>(FT,Args);}//555
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtedmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<1,false>(FT,Args);}//556
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtefmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<1,false>(FT,Args);}//557
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtefmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<1,false>(FT,Args);}//558
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtefmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<1,false>(FT,Args);}//559
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtndmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<1,false>(FT,Args);}//560
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtndmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<1,false>(FT,Args);}//561
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtndmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<1,false>(FT,Args);}//562
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtnfmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<1,false>(FT,Args);}//563
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtnfmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<1,false>(FT,Args);}//564
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtnfmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<1,false>(FT,Args);}//565
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtpdmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<1,false>(FT,Args);}//566
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtpdmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<1,false>(FT,Args);}//567
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtpdmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<1,false>(FT,Args);}//568
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtpfmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<1,false>(FT,Args);}//569
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtpfmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<1,false>(FT,Args);}//570
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtpfmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<1,false>(FT,Args);}//571
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtzdmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<1,false>(FT,Args);}//572
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtzdmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<1,false>(FT,Args);}//573
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtzdmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<1,false>(FT,Args);}//574
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtzfmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<1,false>(FT,Args);}//575
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtzfmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<1,false>(FT,Args);}//576
  BUILTINS_API llvm::GenericValue lle_X__Z15vstore_half_rtzfmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<1,false>(FT,Args);}//577
  BUILTINS_API llvm::GenericValue lle_X__Z12vstorea_halfdmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,1,true>(FT,Args);}//578
  BUILTINS_API llvm::GenericValue lle_X__Z12vstorea_halfdmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,1,true>(FT,Args);}//579
  BUILTINS_API llvm::GenericValue lle_X__Z12vstorea_halfdmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,1,true>(FT,Args);}//580
  BUILTINS_API llvm::GenericValue lle_X__Z12vstorea_halffmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,1,true>(FT,Args);}//581
  BUILTINS_API llvm::GenericValue lle_X__Z12vstorea_halffmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,1,true>(FT,Args);}//582
  BUILTINS_API llvm::GenericValue lle_X__Z12vstorea_halffmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,1,true>(FT,Args);}//583
  BUILTINS_API llvm::GenericValue lle_X__Z14vstorea_half16Dv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,16,true>(FT,Args);}//584
  BUILTINS_API llvm::GenericValue lle_X__Z14vstorea_half16Dv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,16,true>(FT,Args);}//585
  BUILTINS_API llvm::GenericValue lle_X__Z14vstorea_half16Dv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,16,true>(FT,Args);}//586
  BUILTINS_API llvm::GenericValue lle_X__Z14vstorea_half16Dv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,16,true>(FT,Args);}//587
  BUILTINS_API llvm::GenericValue lle_X__Z14vstorea_half16Dv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,16,true>(FT,Args);}//588
  BUILTINS_API llvm::GenericValue lle_X__Z14vstorea_half16Dv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,16,true>(FT,Args);}//589
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rteDv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<16,true>(FT,Args);}//590
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rteDv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<16,true>(FT,Args);}//591
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rteDv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<16,true>(FT,Args);}//592
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rteDv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<16,true>(FT,Args);}//593
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rteDv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<16,true>(FT,Args);}//594
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rteDv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<16,true>(FT,Args);}//595
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtnDv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<16,true>(FT,Args);}//596
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtnDv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<16,true>(FT,Args);}//597
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtnDv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<16,true>(FT,Args);}//598
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtnDv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<16,true>(FT,Args);}//599
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtnDv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<16,true>(FT,Args);}//600
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtnDv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<16,true>(FT,Args);}//601
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtpDv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<16,true>(FT,Args);}//602
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtpDv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<16,true>(FT,Args);}//603
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtpDv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<16,true>(FT,Args);}//604
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtpDv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<16,true>(FT,Args);}//605
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtpDv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<16,true>(FT,Args);}//606
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtpDv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<16,true>(FT,Args);}//607
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtzDv16_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<16,true>(FT,Args);}//608
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtzDv16_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<16,true>(FT,Args);}//609
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtzDv16_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<16,true>(FT,Args);}//610
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtzDv16_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<16,true>(FT,Args);}//611
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtzDv16_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<16,true>(FT,Args);}//612
  BUILTINS_API llvm::GenericValue lle_X__Z18vstorea_half16_rtzDv16_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<16,true>(FT,Args);}//613
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half2Dv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,2,true>(FT,Args);}//614
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half2Dv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,2,true>(FT,Args);}//615
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half2Dv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,2,true>(FT,Args);}//616
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half2Dv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,2,true>(FT,Args);}//617
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half2Dv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,2,true>(FT,Args);}//618
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half2Dv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,2,true>(FT,Args);}//619
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rteDv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<2,true>(FT,Args);}//620
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rteDv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<2,true>(FT,Args);}//621
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rteDv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<2,true>(FT,Args);}//622
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rteDv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<2,true>(FT,Args);}//623
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rteDv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<2,true>(FT,Args);}//624
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rteDv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<2,true>(FT,Args);}//625
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtnDv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<2,true>(FT,Args);}//626
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtnDv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<2,true>(FT,Args);}//627
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtnDv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<2,true>(FT,Args);}//628
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtnDv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<2,true>(FT,Args);}//629
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtnDv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<2,true>(FT,Args);}//630
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtnDv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<2,true>(FT,Args);}//631
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtpDv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<2,true>(FT,Args);}//632
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtpDv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<2,true>(FT,Args);}//633
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtpDv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<2,true>(FT,Args);}//634
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtpDv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<2,true>(FT,Args);}//635
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtpDv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<2,true>(FT,Args);}//636
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtpDv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<2,true>(FT,Args);}//637
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtzDv2_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<2,true>(FT,Args);}//638
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtzDv2_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<2,true>(FT,Args);}//639
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtzDv2_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<2,true>(FT,Args);}//640
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtzDv2_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<2,true>(FT,Args);}//641
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtzDv2_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<2,true>(FT,Args);}//642
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half2_rtzDv2_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<2,true>(FT,Args);}//643
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half3Dv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,3,true>(FT,Args);}//644
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half3Dv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,3,true>(FT,Args);}//645
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half3Dv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,3,true>(FT,Args);}//646
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half3Dv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,3,true>(FT,Args);}//647
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half3Dv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,3,true>(FT,Args);}//648
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half3Dv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,3,true>(FT,Args);}//649
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rteDv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<3,true>(FT,Args);}//650
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rteDv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<3,true>(FT,Args);}//651
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rteDv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<3,true>(FT,Args);}//652
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rteDv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<3,true>(FT,Args);}//653
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rteDv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<3,true>(FT,Args);}//654
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rteDv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<3,true>(FT,Args);}//655
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtnDv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<3,true>(FT,Args);}//656
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtnDv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<3,true>(FT,Args);}//657
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtnDv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<3,true>(FT,Args);}//658
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtnDv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<3,true>(FT,Args);}//659
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtnDv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<3,true>(FT,Args);}//660
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtnDv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<3,true>(FT,Args);}//661
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtpDv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<3,true>(FT,Args);}//662
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtpDv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<3,true>(FT,Args);}//663
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtpDv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<3,true>(FT,Args);}//664
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtpDv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<3,true>(FT,Args);}//665
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtpDv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<3,true>(FT,Args);}//666
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtpDv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<3,true>(FT,Args);}//667
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtzDv3_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<3,true>(FT,Args);}//668
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtzDv3_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<3,true>(FT,Args);}//669
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtzDv3_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<3,true>(FT,Args);}//670
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtzDv3_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<3,true>(FT,Args);}//671
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtzDv3_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<3,true>(FT,Args);}//672
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half3_rtzDv3_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<3,true>(FT,Args);}//673
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half4Dv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,4,true>(FT,Args);}//674
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half4Dv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,4,true>(FT,Args);}//675
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half4Dv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,4,true>(FT,Args);}//676
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half4Dv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,4,true>(FT,Args);}//677
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half4Dv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,4,true>(FT,Args);}//678
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half4Dv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,4,true>(FT,Args);}//679
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rteDv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<4,true>(FT,Args);}//680
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rteDv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<4,true>(FT,Args);}//681
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rteDv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<4,true>(FT,Args);}//682
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rteDv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<4,true>(FT,Args);}//683
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rteDv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<4,true>(FT,Args);}//684
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rteDv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<4,true>(FT,Args);}//685
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtnDv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<4,true>(FT,Args);}//686
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtnDv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<4,true>(FT,Args);}//687
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtnDv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<4,true>(FT,Args);}//688
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtnDv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<4,true>(FT,Args);}//689
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtnDv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<4,true>(FT,Args);}//690
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtnDv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<4,true>(FT,Args);}//691
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtpDv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<4,true>(FT,Args);}//692
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtpDv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<4,true>(FT,Args);}//693
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtpDv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<4,true>(FT,Args);}//694
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtpDv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<4,true>(FT,Args);}//695
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtpDv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<4,true>(FT,Args);}//696
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtpDv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<4,true>(FT,Args);}//697
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtzDv4_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<4,true>(FT,Args);}//698
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtzDv4_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<4,true>(FT,Args);}//699
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtzDv4_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<4,true>(FT,Args);}//700
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtzDv4_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<4,true>(FT,Args);}//701
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtzDv4_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<4,true>(FT,Args);}//702
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half4_rtzDv4_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<4,true>(FT,Args);}//703
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half8Dv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,8,true>(FT,Args);}//704
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half8Dv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,8,true>(FT,Args);}//705
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half8Dv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<double,8,true>(FT,Args);}//706
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half8Dv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,8,true>(FT,Args);}//707
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half8Dv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,8,true>(FT,Args);}//708
  BUILTINS_API llvm::GenericValue lle_X__Z13vstorea_half8Dv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstore_half<float,8,true>(FT,Args);}//709
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rteDv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<8,true>(FT,Args);}//710
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rteDv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<8,true>(FT,Args);}//711
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rteDv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rte<8,true>(FT,Args);}//712
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rteDv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<8,true>(FT,Args);}//713
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rteDv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<8,true>(FT,Args);}//714
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rteDv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rte<8,true>(FT,Args);}//715
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtnDv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<8,true>(FT,Args);}//716
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtnDv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<8,true>(FT,Args);}//717
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtnDv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtn<8,true>(FT,Args);}//718
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtnDv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<8,true>(FT,Args);}//719
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtnDv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<8,true>(FT,Args);}//720
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtnDv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtn<8,true>(FT,Args);}//721
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtpDv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<8,true>(FT,Args);}//722
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtpDv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<8,true>(FT,Args);}//723
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtpDv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtp<8,true>(FT,Args);}//724
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtpDv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<8,true>(FT,Args);}//725
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtpDv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<8,true>(FT,Args);}//726
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtpDv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtp<8,true>(FT,Args);}//727
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtzDv8_dmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<8,true>(FT,Args);}//728
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtzDv8_dmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<8,true>(FT,Args);}//729
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtzDv8_dmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstored_half_rtz<8,true>(FT,Args);}//730
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtzDv8_fmPU3AS1Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<8,true>(FT,Args);}//731
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtzDv8_fmPU3AS3Dh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<8,true>(FT,Args);}//732
  BUILTINS_API llvm::GenericValue lle_X__Z17vstorea_half8_rtzDv8_fmPDh( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_vstoref_half_rtz<8,true>(FT,Args);}//733


}

  
