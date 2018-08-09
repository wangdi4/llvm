// INTEL CONFIDENTIAL
//
// Copyright 2013-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "WorkGroupBuiltinsNames.h"
#include "NameMangleAPI.h"
#include "ParameterType.h"

#define ONE_ARG_FUNCTION(NAME, ARG) {\
            reflection::FunctionDescriptor fd;\
            fd.name = #NAME;\
            fd.parameters.push_back((ARG));\
            this->m_MangledNames.insert(WGBuiltinsNamesDesc(mangle(fd), fd));\
        }
#define TWO_ARG_FUNTION(NAME, ARG1, ARG2) {\
            reflection::FunctionDescriptor fd;\
            fd.name = #NAME;\
            fd.parameters.push_back((ARG1));\
            fd.parameters.push_back((ARG2));\
            this->m_MangledNames.insert(WGBuiltinsNamesDesc(mangle(fd), fd));\
        }

namespace Validation{

    WorkGroupBultinsNames::WorkGroupBultinsNames()
    {
        reflection::RefParamType HalfTy(new reflection::PrimitiveType(reflection::PRIMITIVE_HALF));
        reflection::RefParamType IntTy(new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
        reflection::RefParamType UIntTy(new reflection::PrimitiveType(reflection::PRIMITIVE_UINT));
        reflection::RefParamType LongTy(new reflection::PrimitiveType(reflection::PRIMITIVE_LONG));
        reflection::RefParamType ULongTy(new reflection::PrimitiveType(reflection::PRIMITIVE_ULONG));
        reflection::RefParamType FloatTy(new reflection::PrimitiveType(reflection::PRIMITIVE_FLOAT));
        reflection::RefParamType DoubleTy(new reflection::PrimitiveType(reflection::PRIMITIVE_DOUBLE));
#if defined(__i386__) || defined(i386) || defined(_M_IX86)
        reflection::RefParamType SizeTTy(new reflection::PrimitiveType(reflection::PRIMITIVE_UINT));
#elif defined (__x86_64__) || defined (_M_AMD64) || defined (_M_X64)
        reflection::RefParamType SizeTTy(new reflection::PrimitiveType(reflection::PRIMITIVE_ULONG));
#endif
        reflection::RefParamType pSizeTTy(new reflection::PointerType(SizeTTy, {reflection::ATTR_PRIVATE}));

        ONE_ARG_FUNCTION(work_group_all, IntTy)
        ONE_ARG_FUNCTION(work_group_any, IntTy)
        //work_group_broadcast(gentype, size_t)
        TWO_ARG_FUNTION(work_group_broadcast_1D, HalfTy, SizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_1D, IntTy, SizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_1D, UIntTy, SizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_1D, LongTy, SizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_1D, ULongTy, SizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_1D, FloatTy, SizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_1D, DoubleTy, SizeTTy)
        //work_group_broadcast(gentype, size_t[2])
        TWO_ARG_FUNTION(work_group_broadcast_2D, HalfTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_2D, IntTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_2D, UIntTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_2D, LongTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_2D, ULongTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_2D, FloatTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_2D, DoubleTy, pSizeTTy)
        //work_group_broadcast(gentype, size_t[3])
        TWO_ARG_FUNTION(work_group_broadcast_3D, HalfTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_3D, IntTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_3D, UIntTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_3D, LongTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_3D, ULongTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_3D, FloatTy, pSizeTTy)
        TWO_ARG_FUNTION(work_group_broadcast_3D, DoubleTy, pSizeTTy)
        //work_group_reduce_min(gentype)
        ONE_ARG_FUNCTION(work_group_reduce_min, HalfTy)
        ONE_ARG_FUNCTION(work_group_reduce_min, IntTy)
        ONE_ARG_FUNCTION(work_group_reduce_min, UIntTy)
        ONE_ARG_FUNCTION(work_group_reduce_min, LongTy)
        ONE_ARG_FUNCTION(work_group_reduce_min, ULongTy)
        ONE_ARG_FUNCTION(work_group_reduce_min, FloatTy)
        ONE_ARG_FUNCTION(work_group_reduce_min, DoubleTy)
        //work_group_reduce_max(gentype)
        ONE_ARG_FUNCTION(work_group_reduce_max, HalfTy)
        ONE_ARG_FUNCTION(work_group_reduce_max, IntTy)
        ONE_ARG_FUNCTION(work_group_reduce_max, UIntTy)
        ONE_ARG_FUNCTION(work_group_reduce_max, LongTy)
        ONE_ARG_FUNCTION(work_group_reduce_max, ULongTy)
        ONE_ARG_FUNCTION(work_group_reduce_max, FloatTy)
        ONE_ARG_FUNCTION(work_group_reduce_max, DoubleTy)
        //work_group_reduce_add(gentype)
        ONE_ARG_FUNCTION(work_group_reduce_add, HalfTy)
        ONE_ARG_FUNCTION(work_group_reduce_add, IntTy)
        ONE_ARG_FUNCTION(work_group_reduce_add, UIntTy)
        ONE_ARG_FUNCTION(work_group_reduce_add, LongTy)
        ONE_ARG_FUNCTION(work_group_reduce_add, ULongTy)
        ONE_ARG_FUNCTION(work_group_reduce_add, FloatTy)
        ONE_ARG_FUNCTION(work_group_reduce_add, DoubleTy)
        //work_group_prefixsum_inclusive_max(gentype)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_max, HalfTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_max, IntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_max, UIntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_max, LongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_max, ULongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_max, FloatTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_max, DoubleTy)
        //work_group_prefixsum_inclusive_add(gentype)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_add, HalfTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_add, IntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_add, UIntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_add, LongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_add, ULongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_add, FloatTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_add, DoubleTy)
        //work_group_prefixsum_inclusive_min(gentype)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_min, HalfTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_min, IntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_min, UIntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_min, LongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_min, ULongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_min, FloatTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_inclusive_min, DoubleTy)
        //work_group_prefixsum_exclusive_max(gentype)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_max, HalfTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_max, IntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_max, UIntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_max, LongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_max, ULongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_max, FloatTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_max, DoubleTy)
        //work_group_prefixsum_exclusive_add(gentype)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_add, HalfTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_add, IntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_add, UIntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_add, LongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_add, ULongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_add, FloatTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_add, DoubleTy)
        //work_group_prefixsum_exclusive_min(gentype)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_min, HalfTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_min, IntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_min, UIntTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_min, LongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_min, ULongTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_min, FloatTy)
        ONE_ARG_FUNCTION(work_group_prefixsum_exclusive_min, DoubleTy)
    }

    bool WorkGroupBultinsNames::isWorkGroupBuiltin(std::string MangledName)
    {
        return this->m_MangledNames.find(MangledName)!=this->m_MangledNames.end();
    }

    std::string WorkGroupBultinsNames::getMangledPreExecMethodName(std::string MangledName)
    {
        reflection::FunctionDescriptor fd;
        WGBuiltinsNamesMap::iterator it = this->m_MangledNames.find(MangledName);
        assert(it!=this->m_MangledNames.end() && "Unknow mandgled name. Did you do any checks before?");

        fd = it->second;
        fd.name+="_pre_exec";
        if(it->first == "work_group_all" || it->first == "work_group_any")
            return fd.name;
        return mangle(fd);
    }
}
