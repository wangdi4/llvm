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

File Name:  VLoadStore.cpp

\*****************************************************************************/

#include "VLoadStore.h"
#include "OCLBuiltinParser.h"
#include "Conformance/reference_convert.h"

using namespace llvm;
namespace Validation {
namespace OCLBuiltins {

#define FILL_VLOAD(vectorSize, addrSpace, elemType)             \
{                                                               \
    std::string bltName("vload" #vectorSize);                   \
    p.ptrType.AddrSpace = OCLBuiltinParser::addrSpace;          \
    elem.basicType = getBasicType<elemType>();                  \
    p.ptrType.ptrType[0] = elem;                                \
    args[1] = p;                                                \
    OCLBuiltinParser::GetOCLMangledName(bltName, args, mangledName);\
    funcNames[interpreterPrefix+mangledName]        = lle_X_vload<elemType, vectorSize>;\
}

#define FILL_VLOAD_VECTOR(addrSpace, elemType)  \
    FILL_VLOAD(2, addrSpace, elemType)          \
    FILL_VLOAD(3, addrSpace, elemType)          \
    FILL_VLOAD(4, addrSpace, elemType)          \
    FILL_VLOAD(8, addrSpace, elemType)          \
    FILL_VLOAD(16, addrSpace, elemType)

#define FILL_VLOAD_ADDR_SPACE(addrSpace)        \
    FILL_VLOAD_VECTOR(addrSpace, int8_t)        \
    FILL_VLOAD_VECTOR(addrSpace, uint8_t)       \
    FILL_VLOAD_VECTOR(addrSpace, int16_t)       \
    FILL_VLOAD_VECTOR(addrSpace, uint16_t)      \
    FILL_VLOAD_VECTOR(addrSpace, int32_t)       \
    FILL_VLOAD_VECTOR(addrSpace, uint32_t)      \
    FILL_VLOAD_VECTOR(addrSpace, int64_t)       \
    FILL_VLOAD_VECTOR(addrSpace, uint64_t)      \
    FILL_VLOAD_VECTOR(addrSpace, float)         \
    FILL_VLOAD_VECTOR(addrSpace, double)

#define FILL_VLOAD_ALL(SIZE_PARAM)              \
    {                                           \
    OCLBuiltinParser::ArgVector args(2);        \
    OCLBuiltinParser::ARG offset;               \
    offset.genType = OCLBuiltinParser::BASIC;   \
    offset.basicType = OCLBuiltinParser::SIZE_PARAM;  \
    args[0] = offset;                           \
    OCLBuiltinParser::ARG p;                    \
    p.genType = OCLBuiltinParser::POINTER;      \
    p.ptrType.isAddrSpace = true;               \
    p.ptrType.isPointsToConst = true;           \
    p.ptrType.ptrType.resize(1);                \
    OCLBuiltinParser::ARG elem;                 \
    elem.genType = OCLBuiltinParser::BASIC;     \
    FILL_VLOAD_ADDR_SPACE(GLOBAL)               \
    FILL_VLOAD_ADDR_SPACE(LOCAL)                \
    FILL_VLOAD_ADDR_SPACE(CONSTANT)             \
    FILL_VLOAD_ADDR_SPACE(PRIVATE)              \
    }

template<> llvm::GenericValue lle_X_vload_half<3, true>(
    const llvm::FunctionType *FT,
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
    const llvm::FunctionType *FT,
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
    const llvm::FunctionType *FT,
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
llvm::GenericValue lle_X_vstoref_half_ ## RMODE (const llvm::FunctionType *FT,      \
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
llvm::GenericValue lle_X_vstored_half_ ## RMODE (const llvm::FunctionType *FT,      \
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
    const llvm::FunctionType *FT,                                           \
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
    const llvm::FunctionType *FT,                                               \
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

void VLoadStoreMapFiller::addOpenCLBuiltins( std::map<std::string, PBLTFunc>& funcNames )
{
    const std::string interpreterPrefix("lle_X_");
    std::string mangledName;


    funcNames[interpreterPrefix + "_Z7vstore2Dv2_cjPU3AS1c"] = lle_X_vstore<int8_t,     2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_hjPU3AS1h"] = lle_X_vstore<uint8_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_sjPU3AS1s"] = lle_X_vstore<int16_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_tjPU3AS1t"] = lle_X_vstore<uint16_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_ijPU3AS1i"] = lle_X_vstore<int32_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_jjPU3AS1j"] = lle_X_vstore<uint32_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_ljPU3AS1l"] = lle_X_vstore<int64_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_mjPU3AS1m"] = lle_X_vstore<uint64_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_fjPU3AS1f"] = lle_X_vstore<float,      2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_cjPU3AS1c"] = lle_X_vstore<int8_t,     3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_hjPU3AS1h"] = lle_X_vstore<uint8_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_sjPU3AS1s"] = lle_X_vstore<int16_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_tjPU3AS1t"] = lle_X_vstore<uint16_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_ijPU3AS1i"] = lle_X_vstore<int32_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_jjPU3AS1j"] = lle_X_vstore<uint32_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_ljPU3AS1l"] = lle_X_vstore<int64_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_mjPU3AS1m"] = lle_X_vstore<uint64_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_fjPU3AS1f"] = lle_X_vstore<float,      3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_cjPU3AS1c"] = lle_X_vstore<int8_t,     4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_hjPU3AS1h"] = lle_X_vstore<uint8_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_sjPU3AS1s"] = lle_X_vstore<int16_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_tjPU3AS1t"] = lle_X_vstore<uint16_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_ijPU3AS1i"] = lle_X_vstore<int32_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_jjPU3AS1j"] = lle_X_vstore<uint32_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_ljPU3AS1l"] = lle_X_vstore<int64_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_mjPU3AS1m"] = lle_X_vstore<uint64_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_fjPU3AS1f"] = lle_X_vstore<float,      4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_cjPU3AS1c"] = lle_X_vstore<int8_t,     8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_hjPU3AS1h"] = lle_X_vstore<uint8_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_sjPU3AS1s"] = lle_X_vstore<int16_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_tjPU3AS1t"] = lle_X_vstore<uint16_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_ijPU3AS1i"] = lle_X_vstore<int32_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_jjPU3AS1j"] = lle_X_vstore<uint32_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_ljPU3AS1l"] = lle_X_vstore<int64_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_mjPU3AS1m"] = lle_X_vstore<uint64_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_fjPU3AS1f"] = lle_X_vstore<float,      8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_cjPU3AS1c"] = lle_X_vstore<int8_t,     16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_hjPU3AS1h"] = lle_X_vstore<uint8_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_sjPU3AS1s"] = lle_X_vstore<int16_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_tjPU3AS1t"] = lle_X_vstore<uint16_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_ijPU3AS1i"] = lle_X_vstore<int32_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_jjPU3AS1j"] = lle_X_vstore<uint32_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_ljPU3AS1l"] = lle_X_vstore<int64_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_mjPU3AS1m"] = lle_X_vstore<uint64_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_fjPU3AS1f"] = lle_X_vstore<float,      16>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_cjPU3AS3c"] = lle_X_vstore<int8_t,     2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_hjPU3AS3h"] = lle_X_vstore<uint8_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_sjPU3AS3s"] = lle_X_vstore<int16_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_tjPU3AS3t"] = lle_X_vstore<uint16_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_ijPU3AS3i"] = lle_X_vstore<int32_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_jjPU3AS3j"] = lle_X_vstore<uint32_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_ljPU3AS3l"] = lle_X_vstore<int64_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_mjPU3AS3m"] = lle_X_vstore<uint64_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_fjPU3AS3f"] = lle_X_vstore<float,      2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_cjPU3AS3c"] = lle_X_vstore<int8_t,     3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_hjPU3AS3h"] = lle_X_vstore<uint8_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_sjPU3AS3s"] = lle_X_vstore<int16_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_tjPU3AS3t"] = lle_X_vstore<uint16_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_ijPU3AS3i"] = lle_X_vstore<int32_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_jjPU3AS3j"] = lle_X_vstore<uint32_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_ljPU3AS3l"] = lle_X_vstore<int64_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_mjPU3AS3m"] = lle_X_vstore<uint64_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_fjPU3AS3f"] = lle_X_vstore<float,      3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_cjPU3AS3c"] = lle_X_vstore<int8_t,     4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_hjPU3AS3h"] = lle_X_vstore<uint8_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_sjPU3AS3s"] = lle_X_vstore<int16_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_tjPU3AS3t"] = lle_X_vstore<uint16_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_ijPU3AS3i"] = lle_X_vstore<int32_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_jjPU3AS3j"] = lle_X_vstore<uint32_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_ljPU3AS3l"] = lle_X_vstore<int64_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_mjPU3AS3m"] = lle_X_vstore<uint64_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_fjPU3AS3f"] = lle_X_vstore<float,      4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_cjPU3AS3c"] = lle_X_vstore<int8_t,     8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_hjPU3AS3h"] = lle_X_vstore<uint8_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_sjPU3AS3s"] = lle_X_vstore<int16_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_tjPU3AS3t"] = lle_X_vstore<uint16_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_ijPU3AS3i"] = lle_X_vstore<int32_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_jjPU3AS3j"] = lle_X_vstore<uint32_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_ljPU3AS3l"] = lle_X_vstore<int64_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_mjPU3AS3m"] = lle_X_vstore<uint64_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_fjPU3AS3f"] = lle_X_vstore<float,      8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_cjPU3AS3c"] = lle_X_vstore<int8_t,     16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_hjPU3AS3h"] = lle_X_vstore<uint8_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_sjPU3AS3s"] = lle_X_vstore<int16_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_tjPU3AS3t"] = lle_X_vstore<uint16_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_ijPU3AS3i"] = lle_X_vstore<int32_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_jjPU3AS3j"] = lle_X_vstore<uint32_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_ljPU3AS3l"] = lle_X_vstore<int64_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_mjPU3AS3m"] = lle_X_vstore<uint64_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_fjPU3AS3f"] = lle_X_vstore<float,      16>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_cjPc"] = lle_X_vstore<int8_t,     2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_hjPh"] = lle_X_vstore<uint8_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_sjPs"] = lle_X_vstore<int16_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_tjPt"] = lle_X_vstore<uint16_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_ijPi"] = lle_X_vstore<int32_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_jjPj"] = lle_X_vstore<uint32_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_ljPl"] = lle_X_vstore<int64_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_mjPm"] = lle_X_vstore<uint64_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_fjPf"] = lle_X_vstore<float,      2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_cjPc"] = lle_X_vstore<int8_t,     3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_hjPh"] = lle_X_vstore<uint8_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_sjPs"] = lle_X_vstore<int16_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_tjPt"] = lle_X_vstore<uint16_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_ijPi"] = lle_X_vstore<int32_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_jjPj"] = lle_X_vstore<uint32_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_ljPl"] = lle_X_vstore<int64_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_mjPm"] = lle_X_vstore<uint64_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_fjPf"] = lle_X_vstore<float,      3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_cjPc"] = lle_X_vstore<int8_t,     4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_hjPh"] = lle_X_vstore<uint8_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_sjPs"] = lle_X_vstore<int16_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_tjPt"] = lle_X_vstore<uint16_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_ijPi"] = lle_X_vstore<int32_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_jjPj"] = lle_X_vstore<uint32_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_ljPl"] = lle_X_vstore<int64_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_mjPm"] = lle_X_vstore<uint64_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_fjPf"] = lle_X_vstore<float,      4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_cjPc"] = lle_X_vstore<int8_t,     8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_hjPh"] = lle_X_vstore<uint8_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_sjPs"] = lle_X_vstore<int16_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_tjPt"] = lle_X_vstore<uint16_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_ijPi"] = lle_X_vstore<int32_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_jjPj"] = lle_X_vstore<uint32_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_ljPl"] = lle_X_vstore<int64_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_mjPm"] = lle_X_vstore<uint64_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_fjPf"] = lle_X_vstore<float,      8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_cjPc"] = lle_X_vstore<int8_t,     16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_hjPh"] = lle_X_vstore<uint8_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_sjPs"] = lle_X_vstore<int16_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_tjPt"] = lle_X_vstore<uint16_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_ijPi"] = lle_X_vstore<int32_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_jjPj"] = lle_X_vstore<uint32_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_ljPl"] = lle_X_vstore<int64_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_mjPm"] = lle_X_vstore<uint64_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_fjPf"] = lle_X_vstore<float,      16>;

    funcNames[interpreterPrefix + "_Z7vstore2Dv2_djPU3AS1d"]        = lle_X_vstore<double,2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_djPU3AS1d"]        = lle_X_vstore<double,3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_djPU3AS1d"]        = lle_X_vstore<double,4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_djPU3AS1d"]        = lle_X_vstore<double,8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_djPU3AS1d"]      = lle_X_vstore<double,16>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_djPU3AS3d"]        = lle_X_vstore<double,2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_djPU3AS3d"]        = lle_X_vstore<double,3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_djPU3AS3d"]        = lle_X_vstore<double,4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_djPU3AS3d"]        = lle_X_vstore<double,8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_djPU3AS3d"]      = lle_X_vstore<double,16>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_djPd"]             = lle_X_vstore<double,2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_djPd"]             = lle_X_vstore<double,3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_djPd"]             = lle_X_vstore<double,4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_djPd"]             = lle_X_vstore<double,8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_djPd"]           = lle_X_vstore<double,16>;

    funcNames[interpreterPrefix + "_Z10vload_halfjPKU3AS1s"]        = lle_X_vload_half<1, false>;
    funcNames[interpreterPrefix + "_Z10vload_halfjPKU3AS3s"]        = lle_X_vload_half<1, false>;
    funcNames[interpreterPrefix + "_Z10vload_halfjPKU3AS2s"]        = lle_X_vload_half<1, false>;
    funcNames[interpreterPrefix + "_Z10vload_halfjPKs"]             = lle_X_vload_half<1, false>;
    funcNames[interpreterPrefix + "_Z11vload_half2jPKU3AS1s"]       = lle_X_vload_half<2, false>;
    funcNames[interpreterPrefix + "_Z11vload_half3jPKU3AS1s"]       = lle_X_vload_half<3, false>;
    funcNames[interpreterPrefix + "_Z11vload_half4jPKU3AS1s"] =     lle_X_vload_half<4, false>;
    funcNames[interpreterPrefix + "_Z11vload_half8jPKU3AS1s"] =     lle_X_vload_half<8, false>;
    funcNames[interpreterPrefix + "_Z12vload_half16jPKU3AS1s"] =    lle_X_vload_half<16, false>;
    funcNames[interpreterPrefix + "_Z11vload_half2jPKU3AS3s"] =     lle_X_vload_half<2, false>;
    funcNames[interpreterPrefix + "_Z11vload_half3jPKU3AS3s"] =     lle_X_vload_half<3, false>;
    funcNames[interpreterPrefix + "_Z11vload_half4jPKU3AS3s"] =     lle_X_vload_half<4, false>;
    funcNames[interpreterPrefix + "_Z11vload_half8jPKU3AS3s"] =     lle_X_vload_half<8, false>;
    funcNames[interpreterPrefix + "_Z12vload_half16jPKU3AS3s"] =    lle_X_vload_half<16, false>;
    funcNames[interpreterPrefix + "_Z11vload_half2jPKU3AS2s"] =     lle_X_vload_half<2, false>;
    funcNames[interpreterPrefix + "_Z11vload_half3jPKU3AS2s"] =     lle_X_vload_half<3, false>;
    funcNames[interpreterPrefix + "_Z11vload_half4jPKU3AS2s"] =     lle_X_vload_half<4, false>;
    funcNames[interpreterPrefix + "_Z11vload_half8jPKU3AS2s"] =     lle_X_vload_half<8, false>;
    funcNames[interpreterPrefix + "_Z12vload_half16jPKU3AS2s"] =    lle_X_vload_half<16, false>;
    funcNames[interpreterPrefix + "_Z11vload_half2jPKs"] =          lle_X_vload_half<2, false>;
    funcNames[interpreterPrefix + "_Z11vload_half3jPKs"] =          lle_X_vload_half<3, false>;
    funcNames[interpreterPrefix + "_Z11vload_half4jPKs"] =          lle_X_vload_half<4, false>;
    funcNames[interpreterPrefix + "_Z11vload_half8jPKs"] =          lle_X_vload_half<8, false>;
    funcNames[interpreterPrefix + "_Z12vload_half16jPKs"] =         lle_X_vload_half<16, false>;
    funcNames[interpreterPrefix + "_Z11vloada_halfjPKU3AS1s"]       = lle_X_vload_half<1, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half2jPKU3AS1s"]      = lle_X_vload_half<2, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half3jPKU3AS1s"]      = lle_X_vload_half<3, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half4jPKU3AS1s"]      = lle_X_vload_half<4, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half8jPKU3AS1s"]      = lle_X_vload_half<8, true>;
    funcNames[interpreterPrefix + "_Z13vloada_half16jPKU3AS1s"]     = lle_X_vload_half<16, true>;
    funcNames[interpreterPrefix + "_Z11vloada_halfjPKU3AS3s"]       = lle_X_vload_half<1, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half2jPKU3AS3s"]      = lle_X_vload_half<2, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half3jPKU3AS3s"]      = lle_X_vload_half<3, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half4jPKU3AS3s"]      = lle_X_vload_half<4, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half8jPKU3AS3s"]      = lle_X_vload_half<8, true>;
    funcNames[interpreterPrefix + "_Z13vloada_half16jPKU3AS3s"]     = lle_X_vload_half<16, true>;
    funcNames[interpreterPrefix + "_Z11vloada_halfjPKU3AS2s"]       = lle_X_vload_half<1, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half2jPKU3AS2s"]      = lle_X_vload_half<2, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half3jPKU3AS2s"]      = lle_X_vload_half<3, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half4jPKU3AS2s"]      = lle_X_vload_half<4, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half8jPKU3AS2s"]      = lle_X_vload_half<8, true>;
    funcNames[interpreterPrefix + "_Z13vloada_half16jPKU3AS2s"]     = lle_X_vload_half<16, true>;
    funcNames[interpreterPrefix + "_Z11vloada_halfjPKs"]            = lle_X_vload_half<1, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half2jPKs"]           = lle_X_vload_half<2, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half3jPKs"]           = lle_X_vload_half<3, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half4jPKs"]           = lle_X_vload_half<4, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half8jPKs"]           = lle_X_vload_half<8, true>;
    funcNames[interpreterPrefix + "_Z13vloada_half16jPKs"]          = lle_X_vload_half<16, true>;

    funcNames[interpreterPrefix + "_Z11vstore_halffjPU3AS1s"]               = lle_X_vstore_half<float, 1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halffjPU3AS3s"]               = lle_X_vstore_half<float, 1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halffjPs"]                    = lle_X_vstore_half<float, 1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtefjPU3AS1s"]           = lle_X_vstoref_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtefjPU3AS3s"]           = lle_X_vstoref_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtefjPs"]                = lle_X_vstoref_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzfjPU3AS1s"]           = lle_X_vstoref_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzfjPU3AS3s"]           = lle_X_vstoref_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzfjPs"]                = lle_X_vstoref_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpfjPU3AS1s"]           = lle_X_vstoref_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpfjPU3AS3s"]           = lle_X_vstoref_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpfjPs"]                = lle_X_vstoref_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtnfjPU3AS1s"]           = lle_X_vstoref_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtnfjPU3AS3s"]           = lle_X_vstoref_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtnfjPs"]                = lle_X_vstoref_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halfdjPU3AS1s"]               = lle_X_vstore_half<double, 1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtedjPU3AS1s"]           = lle_X_vstored_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzdjPU3AS1s"]           = lle_X_vstored_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpdjPU3AS1s"]           = lle_X_vstored_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtndjPU3AS1s"]           = lle_X_vstored_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halfdjPU3AS3s"]               = lle_X_vstore_half<double, 1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtedjPU3AS3s"]           = lle_X_vstored_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzdjPU3AS3s"]           = lle_X_vstored_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpdjPU3AS3s"]           = lle_X_vstored_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtndjPU3AS3s"]           = lle_X_vstored_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halfdjPs"]                    = lle_X_vstore_half<double, 1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtedjPs"]                = lle_X_vstored_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzdjPs"]                = lle_X_vstored_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpdjPs"]                = lle_X_vstored_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtndjPs"]                = lle_X_vstored_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_fjPU3AS1s"]          = lle_X_vstore_half<float, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_fjPU3AS1s"]          = lle_X_vstore_half<float, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_fjPU3AS1s"]          = lle_X_vstore_half<float, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_fjPU3AS1s"]          = lle_X_vstore_half<float, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_fjPU3AS1s"]        = lle_X_vstore_half<float, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_fjPU3AS1s"]      = lle_X_vstoref_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_fjPU3AS1s"]      = lle_X_vstoref_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_fjPU3AS1s"]      = lle_X_vstoref_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_fjPU3AS1s"]      = lle_X_vstoref_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_fjPU3AS1s"]    = lle_X_vstoref_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_fjPU3AS1s"]      = lle_X_vstoref_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_fjPU3AS1s"]      = lle_X_vstoref_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_fjPU3AS1s"]      = lle_X_vstoref_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_fjPU3AS1s"]      = lle_X_vstoref_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_fjPU3AS1s"]    = lle_X_vstoref_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_fjPU3AS1s"]      = lle_X_vstoref_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_fjPU3AS1s"]      = lle_X_vstoref_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_fjPU3AS1s"]      = lle_X_vstoref_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_fjPU3AS1s"]      = lle_X_vstoref_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_fjPU3AS1s"]    = lle_X_vstoref_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_fjPU3AS1s"]      = lle_X_vstoref_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_fjPU3AS1s"]      = lle_X_vstoref_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_fjPU3AS1s"]      = lle_X_vstoref_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_fjPU3AS1s"]      = lle_X_vstoref_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_fjPU3AS1s"]    = lle_X_vstoref_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_fjPU3AS3s"]          = lle_X_vstore_half<float, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_fjPU3AS3s"]          = lle_X_vstore_half<float, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_fjPU3AS3s"]          = lle_X_vstore_half<float, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_fjPU3AS3s"]          = lle_X_vstore_half<float, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_fjPU3AS3s"]        = lle_X_vstore_half<float, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_fjPU3AS3s"]      = lle_X_vstoref_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_fjPU3AS3s"]      = lle_X_vstoref_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_fjPU3AS3s"]      = lle_X_vstoref_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_fjPU3AS3s"]      = lle_X_vstoref_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_fjPU3AS3s"]    = lle_X_vstoref_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_fjPU3AS3s"]      = lle_X_vstoref_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_fjPU3AS3s"]      = lle_X_vstoref_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_fjPU3AS3s"]      = lle_X_vstoref_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_fjPU3AS3s"]      = lle_X_vstoref_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_fjPU3AS3s"]    = lle_X_vstoref_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_fjPU3AS3s"]      = lle_X_vstoref_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_fjPU3AS3s"]      = lle_X_vstoref_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_fjPU3AS3s"]      = lle_X_vstoref_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_fjPU3AS3s"]      = lle_X_vstoref_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_fjPU3AS3s"]    = lle_X_vstoref_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_fjPU3AS3s"]      = lle_X_vstoref_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_fjPU3AS3s"]      = lle_X_vstoref_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_fjPU3AS3s"]      = lle_X_vstoref_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_fjPU3AS3s"]      = lle_X_vstoref_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_fjPU3AS3s"]    = lle_X_vstoref_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_fjPs"]               = lle_X_vstore_half<float, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_fjPs"]               = lle_X_vstore_half<float, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_fjPs"]               = lle_X_vstore_half<float, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_fjPs"]               = lle_X_vstore_half<float, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_fjPs"]             = lle_X_vstore_half<float, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_fjPs"]           = lle_X_vstoref_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_fjPs"]           = lle_X_vstoref_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_fjPs"]           = lle_X_vstoref_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_fjPs"]           = lle_X_vstoref_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_fjPs"]         = lle_X_vstoref_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_fjPs"]           = lle_X_vstoref_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_fjPs"]           = lle_X_vstoref_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_fjPs"]           = lle_X_vstoref_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_fjPs"]           = lle_X_vstoref_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_fjPs"]         = lle_X_vstoref_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_fjPs"]           = lle_X_vstoref_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_fjPs"]           = lle_X_vstoref_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_fjPs"]           = lle_X_vstoref_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_fjPs"]           = lle_X_vstoref_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_fjPs"]         = lle_X_vstoref_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_fjPs"]           = lle_X_vstoref_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_fjPs"]           = lle_X_vstoref_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_fjPs"]           = lle_X_vstoref_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_fjPs"]           = lle_X_vstoref_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_fjPs"]         = lle_X_vstoref_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_djPU3AS1s"]          = lle_X_vstore_half<double, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_djPU3AS1s"]          = lle_X_vstore_half<double, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_djPU3AS1s"]          = lle_X_vstore_half<double, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_djPU3AS1s"]          = lle_X_vstore_half<double, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_djPU3AS1s"]        = lle_X_vstore_half<double, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_djPU3AS1s"]      = lle_X_vstored_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_djPU3AS1s"]      = lle_X_vstored_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_djPU3AS1s"]      = lle_X_vstored_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_djPU3AS1s"]      = lle_X_vstored_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_djPU3AS1s"]    = lle_X_vstored_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_djPU3AS1s"]      = lle_X_vstored_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_djPU3AS1s"]      = lle_X_vstored_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_djPU3AS1s"]      = lle_X_vstored_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_djPU3AS1s"]      = lle_X_vstored_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_djPU3AS1s"]    = lle_X_vstored_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_djPU3AS1s"]      = lle_X_vstored_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_djPU3AS1s"]      = lle_X_vstored_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_djPU3AS1s"]      = lle_X_vstored_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_djPU3AS1s"]      = lle_X_vstored_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_djPU3AS1s"]    = lle_X_vstored_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_djPU3AS1s"]      = lle_X_vstored_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_djPU3AS1s"]      = lle_X_vstored_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_djPU3AS1s"]      = lle_X_vstored_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_djPU3AS1s"]      = lle_X_vstored_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_djPU3AS1s"]    = lle_X_vstored_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_djPU3AS3s"]          = lle_X_vstore_half<double, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_djPU3AS3s"]          = lle_X_vstore_half<double, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_djPU3AS3s"]          = lle_X_vstore_half<double, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_djPU3AS3s"]          = lle_X_vstore_half<double, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_djPU3AS3s"]        = lle_X_vstore_half<double, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_djPU3AS3s"]      = lle_X_vstored_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_djPU3AS3s"]      = lle_X_vstored_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_djPU3AS3s"]      = lle_X_vstored_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_djPU3AS3s"]      = lle_X_vstored_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_djPU3AS3s"]    = lle_X_vstored_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_djPU3AS3s"]      = lle_X_vstored_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_djPU3AS3s"]      = lle_X_vstored_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_djPU3AS3s"]      = lle_X_vstored_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_djPU3AS3s"]      = lle_X_vstored_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_djPU3AS3s"]    = lle_X_vstored_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_djPU3AS3s"]      = lle_X_vstored_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_djPU3AS3s"]      = lle_X_vstored_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_djPU3AS3s"]      = lle_X_vstored_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_djPU3AS3s"]      = lle_X_vstored_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_djPU3AS3s"]    = lle_X_vstored_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_djPU3AS3s"]      = lle_X_vstored_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_djPU3AS3s"]      = lle_X_vstored_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_djPU3AS3s"]      = lle_X_vstored_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_djPU3AS3s"]      = lle_X_vstored_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_djPU3AS3s"]    = lle_X_vstored_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_djPs"]               = lle_X_vstore_half<double, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_djPs"]               = lle_X_vstore_half<double, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_djPs"]               = lle_X_vstore_half<double, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_djPs"]               = lle_X_vstore_half<double, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_djPs"]             = lle_X_vstore_half<double, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_djPs"]           = lle_X_vstored_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_djPs"]           = lle_X_vstored_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_djPs"]           = lle_X_vstored_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_djPs"]           = lle_X_vstored_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_djPs"]         = lle_X_vstored_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_djPs"]           = lle_X_vstored_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_djPs"]           = lle_X_vstored_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_djPs"]           = lle_X_vstored_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_djPs"]           = lle_X_vstored_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_djPs"]         = lle_X_vstored_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_djPs"]           = lle_X_vstored_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_djPs"]           = lle_X_vstored_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_djPs"]           = lle_X_vstored_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_djPs"]           = lle_X_vstored_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_djPs"]         = lle_X_vstored_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_djPs"]           = lle_X_vstored_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_djPs"]           = lle_X_vstored_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_djPs"]           = lle_X_vstored_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_djPs"]           = lle_X_vstored_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_djPs"]         = lle_X_vstored_half_rtn<16, false>;

    funcNames[interpreterPrefix + "_Z12vstorea_halffjPU3AS1s"]              = lle_X_vstore_half<float, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_fjPU3AS1s"]         = lle_X_vstore_half<float, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_fjPU3AS1s"]         = lle_X_vstore_half<float, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_fjPU3AS1s"]         = lle_X_vstore_half<float, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_fjPU3AS1s"]         = lle_X_vstore_half<float, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_fjPU3AS1s"]       = lle_X_vstore_half<float, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtefjPU3AS1s"]          = lle_X_vstoref_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_fjPU3AS1s"]     = lle_X_vstoref_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_fjPU3AS1s"]     = lle_X_vstoref_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_fjPU3AS1s"]     = lle_X_vstoref_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_fjPU3AS1s"]     = lle_X_vstoref_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_fjPU3AS1s"]   = lle_X_vstoref_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzfjPU3AS1s"]          = lle_X_vstoref_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_fjPU3AS1s"]     = lle_X_vstoref_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_fjPU3AS1s"]     = lle_X_vstoref_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_fjPU3AS1s"]     = lle_X_vstoref_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_fjPU3AS1s"]     = lle_X_vstoref_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_fjPU3AS1s"]   = lle_X_vstoref_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpfjPU3AS1s"]          = lle_X_vstoref_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_fjPU3AS1s"]     = lle_X_vstoref_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_fjPU3AS1s"]     = lle_X_vstoref_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_fjPU3AS1s"]     = lle_X_vstoref_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_fjPU3AS1s"]     = lle_X_vstoref_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_fjPU3AS1s"]   = lle_X_vstoref_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtnfjPU3AS1s"]          = lle_X_vstoref_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_fjPU3AS1s"]     = lle_X_vstoref_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_fjPU3AS1s"]     = lle_X_vstoref_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_fjPU3AS1s"]     = lle_X_vstoref_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_fjPU3AS1s"]     = lle_X_vstoref_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_fjPU3AS1s"]   = lle_X_vstoref_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halffjPU3AS3s"]              = lle_X_vstore_half<float, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_fjPU3AS3s"]         = lle_X_vstore_half<float, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_fjPU3AS3s"]         = lle_X_vstore_half<float, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_fjPU3AS3s"]         = lle_X_vstore_half<float, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_fjPU3AS3s"]         = lle_X_vstore_half<float, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_fjPU3AS3s"]       = lle_X_vstore_half<float, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtefjPU3AS3s"]          = lle_X_vstoref_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_fjPU3AS3s"]     = lle_X_vstoref_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_fjPU3AS3s"]     = lle_X_vstoref_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_fjPU3AS3s"]     = lle_X_vstoref_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_fjPU3AS3s"]     = lle_X_vstoref_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_fjPU3AS3s"]   = lle_X_vstoref_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzfjPU3AS3s"]          = lle_X_vstoref_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_fjPU3AS3s"]     = lle_X_vstoref_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_fjPU3AS3s"]     = lle_X_vstoref_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_fjPU3AS3s"]     = lle_X_vstoref_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_fjPU3AS3s"]     = lle_X_vstoref_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_fjPU3AS3s"]   = lle_X_vstoref_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpfjPU3AS3s"]          = lle_X_vstoref_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_fjPU3AS3s"]     = lle_X_vstoref_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_fjPU3AS3s"]     = lle_X_vstoref_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_fjPU3AS3s"]     = lle_X_vstoref_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_fjPU3AS3s"]     = lle_X_vstoref_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_fjPU3AS3s"]   = lle_X_vstoref_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtnfjPU3AS3s"]          = lle_X_vstoref_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_fjPU3AS3s"]     = lle_X_vstoref_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_fjPU3AS3s"]     = lle_X_vstoref_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_fjPU3AS3s"]     = lle_X_vstoref_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_fjPU3AS3s"]     = lle_X_vstoref_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_fjPU3AS3s"]   = lle_X_vstoref_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halffjPs"]                   = lle_X_vstore_half<float, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_fjPs"]              = lle_X_vstore_half<float, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_fjPs"]              = lle_X_vstore_half<float, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_fjPs"]              = lle_X_vstore_half<float, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_fjPs"]              = lle_X_vstore_half<float, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_fjPs"]            = lle_X_vstore_half<float, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtefjPs"]               = lle_X_vstoref_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_fjPs"]          = lle_X_vstoref_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_fjPs"]          = lle_X_vstoref_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_fjPs"]          = lle_X_vstoref_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_fjPs"]          = lle_X_vstoref_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_fjPs"]        = lle_X_vstoref_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzfjPs"]               = lle_X_vstoref_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_fjPs"]          = lle_X_vstoref_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_fjPs"]          = lle_X_vstoref_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_fjPs"]          = lle_X_vstoref_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_fjPs"]          = lle_X_vstoref_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_fjPs"]        = lle_X_vstoref_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpfjPs"]               = lle_X_vstoref_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_fjPs"]          = lle_X_vstoref_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_fjPs"]          = lle_X_vstoref_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_fjPs"]          = lle_X_vstoref_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_fjPs"]          = lle_X_vstoref_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_fjPs"]        = lle_X_vstoref_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtnfjPs"]               = lle_X_vstoref_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_fjPs"]          = lle_X_vstoref_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_fjPs"]          = lle_X_vstoref_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_fjPs"]          = lle_X_vstoref_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_fjPs"]          = lle_X_vstoref_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_fjPs"]        = lle_X_vstoref_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halfdjPU3AS1s"]              = lle_X_vstore_half<double, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_djPU3AS1s"]         = lle_X_vstore_half<double, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_djPU3AS1s"]         = lle_X_vstore_half<double, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_djPU3AS1s"]         = lle_X_vstore_half<double, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_djPU3AS1s"]         = lle_X_vstore_half<double, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_djPU3AS1s"]       = lle_X_vstore_half<double, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtedjPU3AS1s"]          = lle_X_vstored_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_djPU3AS1s"]     = lle_X_vstored_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_djPU3AS1s"]     = lle_X_vstored_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_djPU3AS1s"]     = lle_X_vstored_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_djPU3AS1s"]     = lle_X_vstored_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_djPU3AS1s"]   = lle_X_vstored_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzdjPU3AS1s"]          = lle_X_vstored_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_djPU3AS1s"]     = lle_X_vstored_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_djPU3AS1s"]     = lle_X_vstored_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_djPU3AS1s"]     = lle_X_vstored_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_djPU3AS1s"]     = lle_X_vstored_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_djPU3AS1s"]   = lle_X_vstored_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpdjPU3AS1s"]          = lle_X_vstored_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_djPU3AS1s"]     = lle_X_vstored_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_djPU3AS1s"]     = lle_X_vstored_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_djPU3AS1s"]     = lle_X_vstored_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_djPU3AS1s"]     = lle_X_vstored_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_djPU3AS1s"]   = lle_X_vstored_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtndjPU3AS1s"]          = lle_X_vstored_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_djPU3AS1s"]     = lle_X_vstored_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_djPU3AS1s"]     = lle_X_vstored_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_djPU3AS1s"]     = lle_X_vstored_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_djPU3AS1s"]     = lle_X_vstored_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_djPU3AS1s"]   = lle_X_vstored_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halfdjPU3AS3s"]              = lle_X_vstore_half<double, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_djPU3AS3s"]         = lle_X_vstore_half<double, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_djPU3AS3s"]         = lle_X_vstore_half<double, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_djPU3AS3s"]         = lle_X_vstore_half<double, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_djPU3AS3s"]         = lle_X_vstore_half<double, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_djPU3AS3s"]       = lle_X_vstore_half<double, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtedjPU3AS3s"]          = lle_X_vstored_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_djPU3AS3s"]     = lle_X_vstored_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_djPU3AS3s"]     = lle_X_vstored_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_djPU3AS3s"]     = lle_X_vstored_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_djPU3AS3s"]     = lle_X_vstored_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_djPU3AS3s"]   = lle_X_vstored_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzdjPU3AS3s"]          = lle_X_vstored_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_djPU3AS3s"]     = lle_X_vstored_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_djPU3AS3s"]     = lle_X_vstored_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_djPU3AS3s"]     = lle_X_vstored_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_djPU3AS3s"]     = lle_X_vstored_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_djPU3AS3s"]   = lle_X_vstored_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpdjPU3AS3s"]          = lle_X_vstored_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_djPU3AS3s"]     = lle_X_vstored_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_djPU3AS3s"]     = lle_X_vstored_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_djPU3AS3s"]     = lle_X_vstored_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_djPU3AS3s"]     = lle_X_vstored_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_djPU3AS3s"]   = lle_X_vstored_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtndjPU3AS3s"]          = lle_X_vstored_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_djPU3AS3s"]     = lle_X_vstored_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_djPU3AS3s"]     = lle_X_vstored_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_djPU3AS3s"]     = lle_X_vstored_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_djPU3AS3s"]     = lle_X_vstored_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_djPU3AS3s"]   = lle_X_vstored_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halfdjPs"]                   = lle_X_vstore_half<double, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_djPs"]              = lle_X_vstore_half<double, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_djPs"]              = lle_X_vstore_half<double, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_djPs"]              = lle_X_vstore_half<double, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_djPs"]              = lle_X_vstore_half<double, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_djPs"]            = lle_X_vstore_half<double, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtedjPs"]               = lle_X_vstored_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_djPs"]          = lle_X_vstored_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_djPs"]          = lle_X_vstored_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_djPs"]          = lle_X_vstored_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_djPs"]          = lle_X_vstored_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_djPs"]        = lle_X_vstored_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzdjPs"]               = lle_X_vstored_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_djPs"]          = lle_X_vstored_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_djPs"]          = lle_X_vstored_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_djPs"]          = lle_X_vstored_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_djPs"]          = lle_X_vstored_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_djPs"]        = lle_X_vstored_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpdjPs"]               = lle_X_vstored_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_djPs"]          = lle_X_vstored_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_djPs"]          = lle_X_vstored_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_djPs"]          = lle_X_vstored_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_djPs"]          = lle_X_vstored_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_djPs"]        = lle_X_vstored_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtndjPs"]               = lle_X_vstored_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_djPs"]          = lle_X_vstored_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_djPs"]          = lle_X_vstored_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_djPs"]          = lle_X_vstored_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_djPs"]          = lle_X_vstored_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_djPs"]        = lle_X_vstored_half_rtn<16, true>;
    FILL_VLOAD_ALL(UINT)

    // Win64 built-ins.
    funcNames[interpreterPrefix + "_Z10vload_halfmPKU3AS1s"] =      lle_X_vload_half<1, false>;
    funcNames[interpreterPrefix + "_Z10vload_halfmPKU3AS3s"] =      lle_X_vload_half<1, false>;
    funcNames[interpreterPrefix + "_Z10vload_halfmPKU3AS2s"] =      lle_X_vload_half<1, false>;
    funcNames[interpreterPrefix + "_Z10vload_halfmPKs"] =           lle_X_vload_half<1, false>;
    funcNames[interpreterPrefix + "_Z11vload_half2mPKU3AS1s"] =     lle_X_vload_half<2, false>;
    funcNames[interpreterPrefix + "_Z11vload_half3mPKU3AS1s"] =     lle_X_vload_half<3, false>;
    funcNames[interpreterPrefix + "_Z11vload_half4mPKU3AS1s"] =     lle_X_vload_half<4, false>;
    funcNames[interpreterPrefix + "_Z11vload_half8mPKU3AS1s"] =     lle_X_vload_half<8, false>;
    funcNames[interpreterPrefix + "_Z12vload_half16mPKU3AS1s"] =    lle_X_vload_half<16, false>;
    funcNames[interpreterPrefix + "_Z11vload_half2mPKU3AS3s"] =     lle_X_vload_half<2, false>;
    funcNames[interpreterPrefix + "_Z11vload_half3mPKU3AS3s"] =     lle_X_vload_half<3, false>;
    funcNames[interpreterPrefix + "_Z11vload_half4mPKU3AS3s"] =     lle_X_vload_half<4, false>;
    funcNames[interpreterPrefix + "_Z11vload_half8mPKU3AS3s"] =     lle_X_vload_half<8, false>;
    funcNames[interpreterPrefix + "_Z12vload_half16mPKU3AS3s"] =    lle_X_vload_half<16, false>;
    funcNames[interpreterPrefix + "_Z11vload_half2mPKU3AS2s"] =     lle_X_vload_half<2, false>;
    funcNames[interpreterPrefix + "_Z11vload_half3mPKU3AS2s"] =     lle_X_vload_half<3, false>;
    funcNames[interpreterPrefix + "_Z11vload_half4mPKU3AS2s"] =     lle_X_vload_half<4, false>;
    funcNames[interpreterPrefix + "_Z11vload_half8mPKU3AS2s"] =     lle_X_vload_half<8, false>;
    funcNames[interpreterPrefix + "_Z12vload_half16mPKU3AS2s"] =    lle_X_vload_half<16, false>;
    funcNames[interpreterPrefix + "_Z11vload_half2mPKs"] =          lle_X_vload_half<2, false>;
    funcNames[interpreterPrefix + "_Z11vload_half3mPKs"] =          lle_X_vload_half<3, false>;
    funcNames[interpreterPrefix + "_Z11vload_half4mPKs"] =          lle_X_vload_half<4, false>;
    funcNames[interpreterPrefix + "_Z11vload_half8mPKs"] =          lle_X_vload_half<8, false>;
    funcNames[interpreterPrefix + "_Z12vload_half16mPKs"] =         lle_X_vload_half<16, false>;

    funcNames[interpreterPrefix + "_Z11vloada_halfmPKU3AS1s"]   = lle_X_vload_half<1, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half2mPKU3AS1s"]  = lle_X_vload_half<2, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half3mPKU3AS1s"]  = lle_X_vload_half<3, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half4mPKU3AS1s"]  = lle_X_vload_half<4, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half8mPKU3AS1s"]  = lle_X_vload_half<8, true>;
    funcNames[interpreterPrefix + "_Z13vloada_half16mPKU3AS1s"] = lle_X_vload_half<16, true>;
    funcNames[interpreterPrefix + "_Z11vloada_halfmPKU3AS3s"]   = lle_X_vload_half<1, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half2mPKU3AS3s"]  = lle_X_vload_half<2, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half3mPKU3AS3s"]  = lle_X_vload_half<3, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half4mPKU3AS3s"]  = lle_X_vload_half<4, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half8mPKU3AS3s"]  = lle_X_vload_half<8, true>;
    funcNames[interpreterPrefix + "_Z13vloada_half16mPKU3AS3s"] = lle_X_vload_half<16, true>;
    funcNames[interpreterPrefix + "_Z11vloada_halfmPKU3AS2s"]   = lle_X_vload_half<1, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half2mPKU3AS2s"]  = lle_X_vload_half<2, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half3mPKU3AS2s"]  = lle_X_vload_half<3, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half4mPKU3AS2s"]  = lle_X_vload_half<4, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half8mPKU3AS2s"]  = lle_X_vload_half<8, true>;
    funcNames[interpreterPrefix + "_Z13vloada_half16mPKU3AS2s"] = lle_X_vload_half<16, true>;
    funcNames[interpreterPrefix + "_Z11vloada_halfmPKs"]        = lle_X_vload_half<1, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half2mPKs"]       = lle_X_vload_half<2, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half3mPKs"]       = lle_X_vload_half<3, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half4mPKs"]       = lle_X_vload_half<4, true>;
    funcNames[interpreterPrefix + "_Z12vloada_half8mPKs"]       = lle_X_vload_half<8, true>;
    funcNames[interpreterPrefix + "_Z13vloada_half16mPKs"]      = lle_X_vload_half<16, true>;

    funcNames[interpreterPrefix + "_Z11vstore_halffmPU3AS1s"]               = lle_X_vstore_half<float, 1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halffmPU3AS3s"]               = lle_X_vstore_half<float, 1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halffmPs"]                    = lle_X_vstore_half<float, 1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtefmPU3AS1s"]           = lle_X_vstoref_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtefmPU3AS3s"]           = lle_X_vstoref_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtefmPs"]                = lle_X_vstoref_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzfmPU3AS1s"]           = lle_X_vstoref_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzfmPU3AS3s"]           = lle_X_vstoref_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzfmPs"]                = lle_X_vstoref_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpfmPU3AS1s"]           = lle_X_vstoref_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpfmPU3AS3s"]           = lle_X_vstoref_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpfmPs"]                = lle_X_vstoref_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtnfmPU3AS1s"]           = lle_X_vstoref_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtnfmPU3AS3s"]           = lle_X_vstoref_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtnfmPs"]                = lle_X_vstoref_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halfdmPU3AS1s"]               = lle_X_vstore_half<double, 1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtedmPU3AS1s"]           = lle_X_vstored_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzdmPU3AS1s"]           = lle_X_vstored_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpdmPU3AS1s"]           = lle_X_vstored_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtndmPU3AS1s"]           = lle_X_vstored_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halfdmPU3AS3s"]               = lle_X_vstore_half<double, 1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtedmPU3AS3s"]           = lle_X_vstored_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzdmPU3AS3s"]           = lle_X_vstored_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpdmPU3AS3s"]           = lle_X_vstored_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtndmPU3AS3s"]           = lle_X_vstored_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z11vstore_halfdmPs"]                    = lle_X_vstore_half<double, 1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtedmPs"]                = lle_X_vstored_half_rte<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtzdmPs"]                = lle_X_vstored_half_rtz<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtpdmPs"]                = lle_X_vstored_half_rtp<1, false>;
    funcNames[interpreterPrefix + "_Z15vstore_half_rtndmPs"]                = lle_X_vstored_half_rtn<1, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_fmPU3AS1s"]          = lle_X_vstore_half<float, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_fmPU3AS1s"]          = lle_X_vstore_half<float, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_fmPU3AS1s"]          = lle_X_vstore_half<float, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_fmPU3AS1s"]          = lle_X_vstore_half<float, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_fmPU3AS1s"]        = lle_X_vstore_half<float, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_fmPU3AS1s"]      = lle_X_vstoref_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_fmPU3AS1s"]      = lle_X_vstoref_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_fmPU3AS1s"]      = lle_X_vstoref_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_fmPU3AS1s"]      = lle_X_vstoref_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_fmPU3AS1s"]    = lle_X_vstoref_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_fmPU3AS1s"]      = lle_X_vstoref_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_fmPU3AS1s"]      = lle_X_vstoref_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_fmPU3AS1s"]      = lle_X_vstoref_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_fmPU3AS1s"]      = lle_X_vstoref_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_fmPU3AS1s"]    = lle_X_vstoref_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_fmPU3AS1s"]      = lle_X_vstoref_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_fmPU3AS1s"]      = lle_X_vstoref_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_fmPU3AS1s"]      = lle_X_vstoref_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_fmPU3AS1s"]      = lle_X_vstoref_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_fmPU3AS1s"]    = lle_X_vstoref_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_fmPU3AS1s"]      = lle_X_vstoref_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_fmPU3AS1s"]      = lle_X_vstoref_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_fmPU3AS1s"]      = lle_X_vstoref_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_fmPU3AS1s"]      = lle_X_vstoref_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_fmPU3AS1s"]    = lle_X_vstoref_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_fmPU3AS3s"]          = lle_X_vstore_half<float, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_fmPU3AS3s"]          = lle_X_vstore_half<float, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_fmPU3AS3s"]          = lle_X_vstore_half<float, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_fmPU3AS3s"]          = lle_X_vstore_half<float, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_fmPU3AS3s"]        = lle_X_vstore_half<float, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_fmPU3AS3s"]      = lle_X_vstoref_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_fmPU3AS3s"]      = lle_X_vstoref_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_fmPU3AS3s"]      = lle_X_vstoref_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_fmPU3AS3s"]      = lle_X_vstoref_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_fmPU3AS3s"]    = lle_X_vstoref_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_fmPU3AS3s"]      = lle_X_vstoref_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_fmPU3AS3s"]      = lle_X_vstoref_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_fmPU3AS3s"]      = lle_X_vstoref_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_fmPU3AS3s"]      = lle_X_vstoref_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_fmPU3AS3s"]    = lle_X_vstoref_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_fmPU3AS3s"]      = lle_X_vstoref_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_fmPU3AS3s"]      = lle_X_vstoref_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_fmPU3AS3s"]      = lle_X_vstoref_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_fmPU3AS3s"]      = lle_X_vstoref_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_fmPU3AS3s"]    = lle_X_vstoref_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_fmPU3AS3s"]      = lle_X_vstoref_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_fmPU3AS3s"]      = lle_X_vstoref_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_fmPU3AS3s"]      = lle_X_vstoref_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_fmPU3AS3s"]      = lle_X_vstoref_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_fmPU3AS3s"]    = lle_X_vstoref_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_fmPs"]               = lle_X_vstore_half<float, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_fmPs"]               = lle_X_vstore_half<float, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_fmPs"]               = lle_X_vstore_half<float, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_fmPs"]               = lle_X_vstore_half<float, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_fmPs"]             = lle_X_vstore_half<float, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_fmPs"]           = lle_X_vstoref_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_fmPs"]           = lle_X_vstoref_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_fmPs"]           = lle_X_vstoref_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_fmPs"]           = lle_X_vstoref_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_fmPs"]         = lle_X_vstoref_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_fmPs"]           = lle_X_vstoref_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_fmPs"]           = lle_X_vstoref_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_fmPs"]           = lle_X_vstoref_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_fmPs"]           = lle_X_vstoref_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_fmPs"]         = lle_X_vstoref_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_fmPs"]           = lle_X_vstoref_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_fmPs"]           = lle_X_vstoref_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_fmPs"]           = lle_X_vstoref_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_fmPs"]           = lle_X_vstoref_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_fmPs"]         = lle_X_vstoref_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_fmPs"]           = lle_X_vstoref_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_fmPs"]           = lle_X_vstoref_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_fmPs"]           = lle_X_vstoref_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_fmPs"]           = lle_X_vstoref_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_fmPs"]         = lle_X_vstoref_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_dmPU3AS1s"]          = lle_X_vstore_half<double, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_dmPU3AS1s"]          = lle_X_vstore_half<double, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_dmPU3AS1s"]          = lle_X_vstore_half<double, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_dmPU3AS1s"]          = lle_X_vstore_half<double, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_dmPU3AS1s"]        = lle_X_vstore_half<double, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_dmPU3AS1s"]      = lle_X_vstored_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_dmPU3AS1s"]      = lle_X_vstored_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_dmPU3AS1s"]      = lle_X_vstored_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_dmPU3AS1s"]      = lle_X_vstored_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_dmPU3AS1s"]    = lle_X_vstored_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_dmPU3AS1s"]      = lle_X_vstored_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_dmPU3AS1s"]      = lle_X_vstored_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_dmPU3AS1s"]      = lle_X_vstored_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_dmPU3AS1s"]      = lle_X_vstored_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_dmPU3AS1s"]    = lle_X_vstored_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_dmPU3AS1s"]      = lle_X_vstored_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_dmPU3AS1s"]      = lle_X_vstored_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_dmPU3AS1s"]      = lle_X_vstored_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_dmPU3AS1s"]      = lle_X_vstored_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_dmPU3AS1s"]    = lle_X_vstored_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_dmPU3AS1s"]      = lle_X_vstored_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_dmPU3AS1s"]      = lle_X_vstored_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_dmPU3AS1s"]      = lle_X_vstored_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_dmPU3AS1s"]      = lle_X_vstored_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_dmPU3AS1s"]    = lle_X_vstored_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_dmPU3AS3s"]          = lle_X_vstore_half<double, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_dmPU3AS3s"]          = lle_X_vstore_half<double, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_dmPU3AS3s"]          = lle_X_vstore_half<double, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_dmPU3AS3s"]          = lle_X_vstore_half<double, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_dmPU3AS3s"]        = lle_X_vstore_half<double, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_dmPU3AS3s"]      = lle_X_vstored_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_dmPU3AS3s"]      = lle_X_vstored_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_dmPU3AS3s"]      = lle_X_vstored_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_dmPU3AS3s"]      = lle_X_vstored_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_dmPU3AS3s"]    = lle_X_vstored_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_dmPU3AS3s"]      = lle_X_vstored_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_dmPU3AS3s"]      = lle_X_vstored_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_dmPU3AS3s"]      = lle_X_vstored_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_dmPU3AS3s"]      = lle_X_vstored_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_dmPU3AS3s"]    = lle_X_vstored_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_dmPU3AS3s"]      = lle_X_vstored_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_dmPU3AS3s"]      = lle_X_vstored_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_dmPU3AS3s"]      = lle_X_vstored_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_dmPU3AS3s"]      = lle_X_vstored_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_dmPU3AS3s"]    = lle_X_vstored_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_dmPU3AS3s"]      = lle_X_vstored_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_dmPU3AS3s"]      = lle_X_vstored_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_dmPU3AS3s"]      = lle_X_vstored_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_dmPU3AS3s"]      = lle_X_vstored_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_dmPU3AS3s"]    = lle_X_vstored_half_rtn<16, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half2Dv2_dmPs"]               = lle_X_vstore_half<double, 2, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half3Dv3_dmPs"]               = lle_X_vstore_half<double, 3, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half4Dv4_dmPs"]               = lle_X_vstore_half<double, 4, false>;
    funcNames[interpreterPrefix + "_Z12vstore_half8Dv8_dmPs"]               = lle_X_vstore_half<double, 8, false>;
    funcNames[interpreterPrefix + "_Z13vstore_half16Dv16_dmPs"]             = lle_X_vstore_half<double, 16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rteDv2_dmPs"]           = lle_X_vstored_half_rte<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rteDv3_dmPs"]           = lle_X_vstored_half_rte<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rteDv4_dmPs"]           = lle_X_vstored_half_rte<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rteDv8_dmPs"]           = lle_X_vstored_half_rte<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rteDv16_dmPs"]         = lle_X_vstored_half_rte<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtzDv2_dmPs"]           = lle_X_vstored_half_rtz<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtzDv3_dmPs"]           = lle_X_vstored_half_rtz<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtzDv4_dmPs"]           = lle_X_vstored_half_rtz<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtzDv8_dmPs"]           = lle_X_vstored_half_rtz<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtzDv16_dmPs"]         = lle_X_vstored_half_rtz<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtpDv2_dmPs"]           = lle_X_vstored_half_rtp<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtpDv3_dmPs"]           = lle_X_vstored_half_rtp<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtpDv4_dmPs"]           = lle_X_vstored_half_rtp<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtpDv8_dmPs"]           = lle_X_vstored_half_rtp<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtpDv16_dmPs"]         = lle_X_vstored_half_rtp<16, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half2_rtnDv2_dmPs"]           = lle_X_vstored_half_rtn<2, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half3_rtnDv3_dmPs"]           = lle_X_vstored_half_rtn<3, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half4_rtnDv4_dmPs"]           = lle_X_vstored_half_rtn<4, false>;
    funcNames[interpreterPrefix + "_Z16vstore_half8_rtnDv8_dmPs"]           = lle_X_vstored_half_rtn<8, false>;
    funcNames[interpreterPrefix + "_Z17vstore_half16_rtnDv16_dmPs"]         = lle_X_vstored_half_rtn<16, false>;

    funcNames[interpreterPrefix + "_Z12vstorea_halffmPU3AS1s"]              = lle_X_vstore_half<float, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_fmPU3AS1s"]         = lle_X_vstore_half<float, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_fmPU3AS1s"]         = lle_X_vstore_half<float, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_fmPU3AS1s"]         = lle_X_vstore_half<float, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_fmPU3AS1s"]         = lle_X_vstore_half<float, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_fmPU3AS1s"]       = lle_X_vstore_half<float, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtefmPU3AS1s"]          = lle_X_vstoref_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_fmPU3AS1s"]     = lle_X_vstoref_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_fmPU3AS1s"]     = lle_X_vstoref_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_fmPU3AS1s"]     = lle_X_vstoref_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_fmPU3AS1s"]     = lle_X_vstoref_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_fmPU3AS1s"]   = lle_X_vstoref_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzfmPU3AS1s"]          = lle_X_vstoref_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_fmPU3AS1s"]     = lle_X_vstoref_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_fmPU3AS1s"]     = lle_X_vstoref_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_fmPU3AS1s"]     = lle_X_vstoref_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_fmPU3AS1s"]     = lle_X_vstoref_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_fmPU3AS1s"]   = lle_X_vstoref_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpfmPU3AS1s"]          = lle_X_vstoref_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_fmPU3AS1s"]     = lle_X_vstoref_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_fmPU3AS1s"]     = lle_X_vstoref_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_fmPU3AS1s"]     = lle_X_vstoref_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_fmPU3AS1s"]     = lle_X_vstoref_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_fmPU3AS1s"]   = lle_X_vstoref_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtnfmPU3AS1s"]          = lle_X_vstoref_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_fmPU3AS1s"]     = lle_X_vstoref_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_fmPU3AS1s"]     = lle_X_vstoref_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_fmPU3AS1s"]     = lle_X_vstoref_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_fmPU3AS1s"]     = lle_X_vstoref_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_fmPU3AS1s"]   = lle_X_vstoref_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halffmPU3AS3s"]              = lle_X_vstore_half<float, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_fmPU3AS3s"]         = lle_X_vstore_half<float, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_fmPU3AS3s"]         = lle_X_vstore_half<float, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_fmPU3AS3s"]         = lle_X_vstore_half<float, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_fmPU3AS3s"]         = lle_X_vstore_half<float, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_fmPU3AS3s"]       = lle_X_vstore_half<float, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtefmPU3AS3s"]          = lle_X_vstoref_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_fmPU3AS3s"]     = lle_X_vstoref_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_fmPU3AS3s"]     = lle_X_vstoref_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_fmPU3AS3s"]     = lle_X_vstoref_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_fmPU3AS3s"]     = lle_X_vstoref_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_fmPU3AS3s"]   = lle_X_vstoref_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzfmPU3AS3s"]          = lle_X_vstoref_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_fmPU3AS3s"]     = lle_X_vstoref_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_fmPU3AS3s"]     = lle_X_vstoref_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_fmPU3AS3s"]     = lle_X_vstoref_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_fmPU3AS3s"]     = lle_X_vstoref_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_fmPU3AS3s"]   = lle_X_vstoref_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpfmPU3AS3s"]          = lle_X_vstoref_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_fmPU3AS3s"]     = lle_X_vstoref_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_fmPU3AS3s"]     = lle_X_vstoref_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_fmPU3AS3s"]     = lle_X_vstoref_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_fmPU3AS3s"]     = lle_X_vstoref_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_fmPU3AS3s"]   = lle_X_vstoref_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtnfmPU3AS3s"]          = lle_X_vstoref_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_fmPU3AS3s"]     = lle_X_vstoref_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_fmPU3AS3s"]     = lle_X_vstoref_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_fmPU3AS3s"]     = lle_X_vstoref_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_fmPU3AS3s"]     = lle_X_vstoref_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_fmPU3AS3s"]   = lle_X_vstoref_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halffmPs"]                   = lle_X_vstore_half<float, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_fmPs"]              = lle_X_vstore_half<float, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_fmPs"]              = lle_X_vstore_half<float, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_fmPs"]              = lle_X_vstore_half<float, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_fmPs"]              = lle_X_vstore_half<float, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_fmPs"]            = lle_X_vstore_half<float, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtefmPs"]               = lle_X_vstoref_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_fmPs"]          = lle_X_vstoref_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_fmPs"]          = lle_X_vstoref_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_fmPs"]          = lle_X_vstoref_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_fmPs"]          = lle_X_vstoref_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_fmPs"]        = lle_X_vstoref_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzfmPs"]               = lle_X_vstoref_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_fmPs"]          = lle_X_vstoref_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_fmPs"]          = lle_X_vstoref_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_fmPs"]          = lle_X_vstoref_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_fmPs"]          = lle_X_vstoref_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_fmPs"]        = lle_X_vstoref_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpfmPs"]               = lle_X_vstoref_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_fmPs"]          = lle_X_vstoref_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_fmPs"]          = lle_X_vstoref_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_fmPs"]          = lle_X_vstoref_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_fmPs"]          = lle_X_vstoref_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_fmPs"]        = lle_X_vstoref_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtnfmPs"]               = lle_X_vstoref_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_fmPs"]          = lle_X_vstoref_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_fmPs"]          = lle_X_vstoref_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_fmPs"]          = lle_X_vstoref_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_fmPs"]          = lle_X_vstoref_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_fmPs"]        = lle_X_vstoref_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halfdmPU3AS1s"]              = lle_X_vstore_half<double, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_dmPU3AS1s"]         = lle_X_vstore_half<double, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_dmPU3AS1s"]         = lle_X_vstore_half<double, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_dmPU3AS1s"]         = lle_X_vstore_half<double, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_dmPU3AS1s"]         = lle_X_vstore_half<double, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_dmPU3AS1s"]       = lle_X_vstore_half<double, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtedmPU3AS1s"]          = lle_X_vstored_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_dmPU3AS1s"]     = lle_X_vstored_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_dmPU3AS1s"]     = lle_X_vstored_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_dmPU3AS1s"]     = lle_X_vstored_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_dmPU3AS1s"]     = lle_X_vstored_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_dmPU3AS1s"]   = lle_X_vstored_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzdmPU3AS1s"]          = lle_X_vstored_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_dmPU3AS1s"]     = lle_X_vstored_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_dmPU3AS1s"]     = lle_X_vstored_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_dmPU3AS1s"]     = lle_X_vstored_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_dmPU3AS1s"]     = lle_X_vstored_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_dmPU3AS1s"]   = lle_X_vstored_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpdmPU3AS1s"]          = lle_X_vstored_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_dmPU3AS1s"]     = lle_X_vstored_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_dmPU3AS1s"]     = lle_X_vstored_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_dmPU3AS1s"]     = lle_X_vstored_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_dmPU3AS1s"]     = lle_X_vstored_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_dmPU3AS1s"]   = lle_X_vstored_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtndmPU3AS1s"]          = lle_X_vstored_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_dmPU3AS1s"]     = lle_X_vstored_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_dmPU3AS1s"]     = lle_X_vstored_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_dmPU3AS1s"]     = lle_X_vstored_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_dmPU3AS1s"]     = lle_X_vstored_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_dmPU3AS1s"]   = lle_X_vstored_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halfdmPU3AS3s"]              = lle_X_vstore_half<double, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_dmPU3AS3s"]         = lle_X_vstore_half<double, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_dmPU3AS3s"]         = lle_X_vstore_half<double, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_dmPU3AS3s"]         = lle_X_vstore_half<double, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_dmPU3AS3s"]         = lle_X_vstore_half<double, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_dmPU3AS3s"]       = lle_X_vstore_half<double, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtedmPU3AS3s"]          = lle_X_vstored_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_dmPU3AS3s"]     = lle_X_vstored_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_dmPU3AS3s"]     = lle_X_vstored_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_dmPU3AS3s"]     = lle_X_vstored_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_dmPU3AS3s"]     = lle_X_vstored_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_dmPU3AS3s"]   = lle_X_vstored_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzdmPU3AS3s"]          = lle_X_vstored_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_dmPU3AS3s"]     = lle_X_vstored_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_dmPU3AS3s"]     = lle_X_vstored_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_dmPU3AS3s"]     = lle_X_vstored_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_dmPU3AS3s"]     = lle_X_vstored_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_dmPU3AS3s"]   = lle_X_vstored_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpdmPU3AS3s"]          = lle_X_vstored_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_dmPU3AS3s"]     = lle_X_vstored_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_dmPU3AS3s"]     = lle_X_vstored_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_dmPU3AS3s"]     = lle_X_vstored_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_dmPU3AS3s"]     = lle_X_vstored_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_dmPU3AS3s"]   = lle_X_vstored_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtndmPU3AS3s"]          = lle_X_vstored_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_dmPU3AS3s"]     = lle_X_vstored_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_dmPU3AS3s"]     = lle_X_vstored_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_dmPU3AS3s"]     = lle_X_vstored_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_dmPU3AS3s"]     = lle_X_vstored_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_dmPU3AS3s"]   = lle_X_vstored_half_rtn<16, true>;
    funcNames[interpreterPrefix + "_Z12vstorea_halfdmPs"]                   = lle_X_vstore_half<double, 1, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half2Dv2_dmPs"]              = lle_X_vstore_half<double, 2, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half3Dv3_dmPs"]              = lle_X_vstore_half<double, 3, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half4Dv4_dmPs"]              = lle_X_vstore_half<double, 4, true>;
    funcNames[interpreterPrefix + "_Z13vstorea_half8Dv8_dmPs"]              = lle_X_vstore_half<double, 8, true>;
    funcNames[interpreterPrefix + "_Z14vstorea_half16Dv16_dmPs"]            = lle_X_vstore_half<double, 16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtedmPs"]               = lle_X_vstored_half_rte<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rteDv2_dmPs"]          = lle_X_vstored_half_rte<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rteDv3_dmPs"]          = lle_X_vstored_half_rte<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rteDv4_dmPs"]          = lle_X_vstored_half_rte<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rteDv8_dmPs"]          = lle_X_vstored_half_rte<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rteDv16_dmPs"]        = lle_X_vstored_half_rte<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtzdmPs"]               = lle_X_vstored_half_rtz<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtzDv2_dmPs"]          = lle_X_vstored_half_rtz<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtzDv3_dmPs"]          = lle_X_vstored_half_rtz<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtzDv4_dmPs"]          = lle_X_vstored_half_rtz<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtzDv8_dmPs"]          = lle_X_vstored_half_rtz<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtzDv16_dmPs"]        = lle_X_vstored_half_rtz<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtpdmPs"]               = lle_X_vstored_half_rtp<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtpDv2_dmPs"]          = lle_X_vstored_half_rtp<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtpDv3_dmPs"]          = lle_X_vstored_half_rtp<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtpDv4_dmPs"]          = lle_X_vstored_half_rtp<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtpDv8_dmPs"]          = lle_X_vstored_half_rtp<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtpDv16_dmPs"]        = lle_X_vstored_half_rtp<16, true>;
    funcNames[interpreterPrefix + "_Z16vstorea_half_rtndmPs"]               = lle_X_vstored_half_rtn<1, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half2_rtnDv2_dmPs"]          = lle_X_vstored_half_rtn<2, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half3_rtnDv3_dmPs"]          = lle_X_vstored_half_rtn<3, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half4_rtnDv4_dmPs"]          = lle_X_vstored_half_rtn<4, true>;
    funcNames[interpreterPrefix + "_Z17vstorea_half8_rtnDv8_dmPs"]          = lle_X_vstored_half_rtn<8, true>;
    funcNames[interpreterPrefix + "_Z18vstorea_half16_rtnDv16_dmPs"]        = lle_X_vstored_half_rtn<16, true>;

    funcNames[interpreterPrefix + "_Z7vstore2Dv2_cmPU3AS1c"] = lle_X_vstore<int8_t,     2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_hmPU3AS1h"] = lle_X_vstore<uint8_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_smPU3AS1s"] = lle_X_vstore<int16_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_tmPU3AS1t"] = lle_X_vstore<uint16_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_imPU3AS1i"] = lle_X_vstore<int32_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_mmPU3AS1m"] = lle_X_vstore<uint32_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_lmPU3AS1l"] = lle_X_vstore<int64_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_mmPU3AS1m"] = lle_X_vstore<uint64_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_fmPU3AS1f"] = lle_X_vstore<float,      2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_cmPU3AS1c"] = lle_X_vstore<int8_t,     3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_hmPU3AS1h"] = lle_X_vstore<uint8_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_smPU3AS1s"] = lle_X_vstore<int16_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_tmPU3AS1t"] = lle_X_vstore<uint16_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_imPU3AS1i"] = lle_X_vstore<int32_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_mmPU3AS1m"] = lle_X_vstore<uint32_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_lmPU3AS1l"] = lle_X_vstore<int64_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_mmPU3AS1m"] = lle_X_vstore<uint64_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_fmPU3AS1f"] = lle_X_vstore<float,      3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_cmPU3AS1c"] = lle_X_vstore<int8_t,     4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_hmPU3AS1h"] = lle_X_vstore<uint8_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_smPU3AS1s"] = lle_X_vstore<int16_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_tmPU3AS1t"] = lle_X_vstore<uint16_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_imPU3AS1i"] = lle_X_vstore<int32_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_mmPU3AS1m"] = lle_X_vstore<uint32_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_lmPU3AS1l"] = lle_X_vstore<int64_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_mmPU3AS1m"] = lle_X_vstore<uint64_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_fmPU3AS1f"] = lle_X_vstore<float,      4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_cmPU3AS1c"] = lle_X_vstore<int8_t,     8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_hmPU3AS1h"] = lle_X_vstore<uint8_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_smPU3AS1s"] = lle_X_vstore<int16_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_tmPU3AS1t"] = lle_X_vstore<uint16_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_imPU3AS1i"] = lle_X_vstore<int32_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_mmPU3AS1m"] = lle_X_vstore<uint32_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_lmPU3AS1l"] = lle_X_vstore<int64_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_mmPU3AS1m"] = lle_X_vstore<uint64_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_fmPU3AS1f"] = lle_X_vstore<float,      8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_cmPU3AS1c"] = lle_X_vstore<int8_t,     16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_hmPU3AS1h"] = lle_X_vstore<uint8_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_smPU3AS1s"] = lle_X_vstore<int16_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_tmPU3AS1t"] = lle_X_vstore<uint16_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_imPU3AS1i"] = lle_X_vstore<int32_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_mmPU3AS1m"] = lle_X_vstore<uint32_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_lmPU3AS1l"] = lle_X_vstore<int64_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_mmPU3AS1m"] = lle_X_vstore<uint64_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_fmPU3AS1f"] = lle_X_vstore<float,      16>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_cmPU3AS3c"] = lle_X_vstore<int8_t,     2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_hmPU3AS3h"] = lle_X_vstore<uint8_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_smPU3AS3s"] = lle_X_vstore<int16_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_tmPU3AS3t"] = lle_X_vstore<uint16_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_imPU3AS3i"] = lle_X_vstore<int32_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_mmPU3AS3m"] = lle_X_vstore<uint32_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_lmPU3AS3l"] = lle_X_vstore<int64_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_mmPU3AS3m"] = lle_X_vstore<uint64_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_fmPU3AS3f"] = lle_X_vstore<float,      2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_cmPU3AS3c"] = lle_X_vstore<int8_t,     3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_hmPU3AS3h"] = lle_X_vstore<uint8_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_smPU3AS3s"] = lle_X_vstore<int16_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_tmPU3AS3t"] = lle_X_vstore<uint16_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_imPU3AS3i"] = lle_X_vstore<int32_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_mmPU3AS3m"] = lle_X_vstore<uint32_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_lmPU3AS3l"] = lle_X_vstore<int64_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_mmPU3AS3m"] = lle_X_vstore<uint64_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_fmPU3AS3f"] = lle_X_vstore<float,      3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_cmPU3AS3c"] = lle_X_vstore<int8_t,     4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_hmPU3AS3h"] = lle_X_vstore<uint8_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_smPU3AS3s"] = lle_X_vstore<int16_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_tmPU3AS3t"] = lle_X_vstore<uint16_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_imPU3AS3i"] = lle_X_vstore<int32_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_mmPU3AS3m"] = lle_X_vstore<uint32_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_lmPU3AS3l"] = lle_X_vstore<int64_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_mmPU3AS3m"] = lle_X_vstore<uint64_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_fmPU3AS3f"] = lle_X_vstore<float,      4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_cmPU3AS3c"] = lle_X_vstore<int8_t,     8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_hmPU3AS3h"] = lle_X_vstore<uint8_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_smPU3AS3s"] = lle_X_vstore<int16_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_tmPU3AS3t"] = lle_X_vstore<uint16_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_imPU3AS3i"] = lle_X_vstore<int32_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_mmPU3AS3m"] = lle_X_vstore<uint32_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_lmPU3AS3l"] = lle_X_vstore<int64_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_mmPU3AS3m"] = lle_X_vstore<uint64_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_fmPU3AS3f"] = lle_X_vstore<float,      8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_cmPU3AS3c"] = lle_X_vstore<int8_t,     16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_hmPU3AS3h"] = lle_X_vstore<uint8_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_smPU3AS3s"] = lle_X_vstore<int16_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_tmPU3AS3t"] = lle_X_vstore<uint16_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_imPU3AS3i"] = lle_X_vstore<int32_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_mmPU3AS3m"] = lle_X_vstore<uint32_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_lmPU3AS3l"] = lle_X_vstore<int64_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_mmPU3AS3m"] = lle_X_vstore<uint64_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_fmPU3AS3f"] = lle_X_vstore<float,      16>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_cmPc"] = lle_X_vstore<int8_t,     2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_hmPh"] = lle_X_vstore<uint8_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_smPs"] = lle_X_vstore<int16_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_tmPt"] = lle_X_vstore<uint16_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_imPi"] = lle_X_vstore<int32_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_mmPm"] = lle_X_vstore<uint32_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_lmPl"] = lle_X_vstore<int64_t,    2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_mmPm"] = lle_X_vstore<uint64_t,   2>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_fmPf"] = lle_X_vstore<float,      2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_cmPc"] = lle_X_vstore<int8_t,     3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_hmPh"] = lle_X_vstore<uint8_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_smPs"] = lle_X_vstore<int16_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_tmPt"] = lle_X_vstore<uint16_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_imPi"] = lle_X_vstore<int32_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_mmPm"] = lle_X_vstore<uint32_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_lmPl"] = lle_X_vstore<int64_t,    3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_mmPm"] = lle_X_vstore<uint64_t,   3>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_fmPf"] = lle_X_vstore<float,      3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_cmPc"] = lle_X_vstore<int8_t,     4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_hmPh"] = lle_X_vstore<uint8_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_smPs"] = lle_X_vstore<int16_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_tmPt"] = lle_X_vstore<uint16_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_imPi"] = lle_X_vstore<int32_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_mmPm"] = lle_X_vstore<uint32_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_lmPl"] = lle_X_vstore<int64_t,    4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_mmPm"] = lle_X_vstore<uint64_t,   4>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_fmPf"] = lle_X_vstore<float,      4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_cmPc"] = lle_X_vstore<int8_t,     8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_hmPh"] = lle_X_vstore<uint8_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_smPs"] = lle_X_vstore<int16_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_tmPt"] = lle_X_vstore<uint16_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_imPi"] = lle_X_vstore<int32_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_mmPm"] = lle_X_vstore<uint32_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_lmPl"] = lle_X_vstore<int64_t,    8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_mmPm"] = lle_X_vstore<uint64_t,   8>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_fmPf"] = lle_X_vstore<float,      8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_cmPc"] = lle_X_vstore<int8_t,     16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_hmPh"] = lle_X_vstore<uint8_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_smPs"] = lle_X_vstore<int16_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_tmPt"] = lle_X_vstore<uint16_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_imPi"] = lle_X_vstore<int32_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_mmPm"] = lle_X_vstore<uint32_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_lmPl"] = lle_X_vstore<int64_t,    16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_mmPm"] = lle_X_vstore<uint64_t,   16>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_fmPf"] = lle_X_vstore<float,      16>;

    funcNames[interpreterPrefix + "_Z7vstore2Dv2_dmPU3AS1d"]        = lle_X_vstore<double,2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_dmPU3AS1d"]        = lle_X_vstore<double,3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_dmPU3AS1d"]        = lle_X_vstore<double,4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_dmPU3AS1d"]        = lle_X_vstore<double,8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_dmPU3AS1d"]      = lle_X_vstore<double,16>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_dmPU3AS3d"]        = lle_X_vstore<double,2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_dmPU3AS3d"]        = lle_X_vstore<double,3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_dmPU3AS3d"]        = lle_X_vstore<double,4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_dmPU3AS3d"]        = lle_X_vstore<double,8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_dmPU3AS3d"]      = lle_X_vstore<double,16>;
    funcNames[interpreterPrefix + "_Z7vstore2Dv2_dmPd"]             = lle_X_vstore<double,2>;
    funcNames[interpreterPrefix + "_Z7vstore3Dv3_dmPd"]             = lle_X_vstore<double,3>;
    funcNames[interpreterPrefix + "_Z7vstore4Dv4_dmPd"]             = lle_X_vstore<double,4>;
    funcNames[interpreterPrefix + "_Z7vstore8Dv8_dmPd"]             = lle_X_vstore<double,8>;
    funcNames[interpreterPrefix + "_Z8vstore16Dv16_dmPd"]           = lle_X_vstore<double,16>;

    FILL_VLOAD_ALL(ULONG) // Win64
}

}
}
