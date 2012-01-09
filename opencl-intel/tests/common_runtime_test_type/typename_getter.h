// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//
// typename_getter.h

#ifndef TYPENAME_GETTER_
#define TYPENAME_GETTER_

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <malloc.h>
#include <gtest/gtest.h>
#include <CL/cl.h>
#include "general_purpose_struct.h"

// TypeNameGetter - returns string representation of T
template <typename T>
class TypeNameGetter{
public:

	//	getCombinedString - return concatenation of prefix and T's type string
	std::string getCombinedString(std::string prefix)
	{
		return std::string(prefix+=getType());
	}

	//	getType - returns type name string
	std::string getType()
    {
		return "should not get here";
    }
};

// scalar types
template <>
std::string TypeNameGetter<cl_int>::getType()
{
	return "int";
}

template <>
std::string TypeNameGetter<cl_uint>::getType()
{
	return "uint";
}

template <>
std::string TypeNameGetter<cl_long>::getType()
{
	return "long";
}

template <>
std::string TypeNameGetter<cl_ulong>::getType()
{
	return "ulong";
}

template <>
std::string TypeNameGetter<cl_char>::getType()
{
	return "char";
}

template <>
std::string TypeNameGetter<cl_uchar>::getType()
{
	return "uchar";
}

template <>
std::string TypeNameGetter<cl_short>::getType()
{
	return "short";
}

template <>
std::string TypeNameGetter<cl_ushort>::getType()
{
	return "ushort";
}

template <>
std::string TypeNameGetter<cl_float>::getType()
{
	return "float";
}

template <>
std::string TypeNameGetter<struct HalfWrapper>::getType()
{
	return "half";
}
	
// vector types

// cl_char
template <>
std::string TypeNameGetter<cl_char2>::getType()
{
	return "char2";
}

template <>
std::string TypeNameGetter<cl_char4>::getType()
{
	return "char4";
}

template <>
std::string TypeNameGetter<cl_char8>::getType()
{
	return "char8";
}

template <>
std::string TypeNameGetter<cl_char16>::getType()
{
	return "char16";
}

// cl_uchar
template <>
std::string TypeNameGetter<cl_uchar2>::getType()
{
	return "uchar2";
}

template <>
std::string TypeNameGetter<cl_uchar4>::getType()
{
	return "uchar4";
}

template <>
std::string TypeNameGetter<cl_uchar8>::getType()
{
	return "uchar8";
}

template <>
std::string TypeNameGetter<cl_uchar16>::getType()
{
	return "uchar16";
}

// cl_short
template <>
std::string TypeNameGetter<cl_short2>::getType()
{
	return "short2";
}

template <>
std::string TypeNameGetter<cl_short4>::getType()
{
	return "short4";
}

template <>
std::string TypeNameGetter<cl_short8>::getType()
{
	return "short8";
}

template <>
std::string TypeNameGetter<cl_short16>::getType()
{
	return "short16";
}

// cl_ushort
template <>
std::string TypeNameGetter<cl_ushort2>::getType()
{
	return "ushort2";
}

template <>
std::string TypeNameGetter<cl_ushort4>::getType()
{
	return "ushort4";
}

template <>
std::string TypeNameGetter<cl_ushort8>::getType()
{
	return "ushort8";
}

template <>
std::string TypeNameGetter<cl_ushort16>::getType()
{
	return "ushort16";
}

// cl_int
template <>
std::string TypeNameGetter<cl_int2>::getType()
{
	return "int2";
}

template <>
std::string TypeNameGetter<cl_int4>::getType()
{
	return "int4";
}

template <>
std::string TypeNameGetter<cl_int8>::getType()
{
	return "int8";
}

template <>
std::string TypeNameGetter<cl_int16>::getType()
{
	return "int16";
}

//	cl_uint
template <>
std::string TypeNameGetter<cl_uint2>::getType()
{
	return "uint2";
}

template <>
std::string TypeNameGetter<cl_uint4>::getType()
{
	return "uint4";
}

template <>
std::string TypeNameGetter<cl_uint8>::getType()
{
	return "uint8";
}

template <>
std::string TypeNameGetter<cl_uint16>::getType()
{
	return "uint16";
}

// cl_long
template <>
std::string TypeNameGetter<cl_long2>::getType()
{
	return "long2";
}

template <>
std::string TypeNameGetter<cl_long4>::getType()
{
	return "long4";
}

template <>
std::string TypeNameGetter<cl_long8>::getType()
{
	return "long8";
}

template <>
std::string TypeNameGetter<cl_long16>::getType()
{
	return "long16";
}

//	cl_ulong
template <>
std::string TypeNameGetter<cl_ulong2>::getType()
{
	return "ulong2";
}

template <>
std::string TypeNameGetter<cl_ulong4>::getType()
{
	return "ulong4";
}

template <>
std::string TypeNameGetter<cl_ulong8>::getType()
{
	return "ulong8";
}

template <>
std::string TypeNameGetter<cl_ulong16>::getType()
{
	return "ulong16";
}

//	cl_float
template <>
std::string TypeNameGetter<cl_float2>::getType()
{
	return "float2";
}

template <>
std::string TypeNameGetter<cl_float4>::getType()
{
	return "float4";
}

template <>
std::string TypeNameGetter<cl_float8>::getType()
{
	return "float8";
}

template <>
std::string TypeNameGetter<cl_float16>::getType()
{
	return "float16";
}

// struct type

template <>
std::string TypeNameGetter<UserDefinedStructure>::getType()
{
	return "UserDefinedStructure";
}

#endif /* TYPENAME_GETTER_ */