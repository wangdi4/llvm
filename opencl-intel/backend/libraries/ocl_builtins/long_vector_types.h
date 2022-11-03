// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

typedef long long32 __attribute__((ext_vector_type(32)));
typedef long long64 __attribute__((ext_vector_type(64)));
typedef long long128 __attribute__((ext_vector_type(128)));
typedef long long256 __attribute__((ext_vector_type(256)));
typedef long long512 __attribute__((ext_vector_type(512)));
typedef long long1024 __attribute__((ext_vector_type(1024)));

typedef ulong ulong32 __attribute__((ext_vector_type(32)));
typedef ulong ulong64 __attribute__((ext_vector_type(64)));
typedef ulong ulong128 __attribute__((ext_vector_type(128)));
typedef ulong ulong256 __attribute__((ext_vector_type(256)));
typedef ulong ulong512 __attribute__((ext_vector_type(512)));
typedef ulong ulong1024 __attribute__((ext_vector_type(1024)));

typedef half half32 __attribute__((ext_vector_type(32)));
typedef half half64 __attribute__((ext_vector_type(64)));
typedef half half128 __attribute__((ext_vector_type(128)));
typedef half half256 __attribute__((ext_vector_type(256)));
typedef half half512 __attribute__((ext_vector_type(512)));
typedef half half1024 __attribute__((ext_vector_type(1024)));

typedef float float32 __attribute__((ext_vector_type(32)));
typedef float float64 __attribute__((ext_vector_type(64)));
typedef float float128 __attribute__((ext_vector_type(128)));
typedef float float256 __attribute__((ext_vector_type(256)));
typedef float float512 __attribute__((ext_vector_type(512)));
typedef float float1024 __attribute__((ext_vector_type(1024)));

typedef double double32 __attribute__((ext_vector_type(32)));
typedef double double64 __attribute__((ext_vector_type(64)));
typedef double double128 __attribute__((ext_vector_type(128)));
typedef double double256 __attribute__((ext_vector_type(256)));
typedef double double512 __attribute__((ext_vector_type(512)));
typedef double double1024 __attribute__((ext_vector_type(1024)));

typedef uint uint32 __attribute__((ext_vector_type(32)));
typedef uint uint64 __attribute__((ext_vector_type(64)));
typedef uint uint128 __attribute__((ext_vector_type(128)));
typedef uint uint256 __attribute__((ext_vector_type(256)));
typedef uint uint512 __attribute__((ext_vector_type(512)));
typedef uint uint1024 __attribute__((ext_vector_type(1024)));

typedef int int32 __attribute__((ext_vector_type(32)));
typedef int int64 __attribute__((ext_vector_type(64)));
typedef int int128 __attribute__((ext_vector_type(128)));
typedef int int256 __attribute__((ext_vector_type(256)));
typedef int int512 __attribute__((ext_vector_type(512)));
typedef int int1024 __attribute__((ext_vector_type(1024)));

typedef short short32 __attribute__((ext_vector_type(32)));
typedef short short64 __attribute__((ext_vector_type(64)));
typedef short short128 __attribute__((ext_vector_type(128)));
typedef short short256 __attribute__((ext_vector_type(256)));
typedef short short512 __attribute__((ext_vector_type(512)));
typedef short short1024 __attribute__((ext_vector_type(1024)));

typedef ushort ushort32 __attribute__((ext_vector_type(32)));
typedef ushort ushort64 __attribute__((ext_vector_type(64)));
typedef ushort ushort128 __attribute__((ext_vector_type(128)));
typedef ushort ushort256 __attribute__((ext_vector_type(256)));
typedef ushort ushort512 __attribute__((ext_vector_type(512)));
typedef ushort ushort1024 __attribute__((ext_vector_type(1024)));

typedef char char32 __attribute__((ext_vector_type(32)));
typedef char char64 __attribute__((ext_vector_type(64)));
typedef char char128 __attribute__((ext_vector_type(128)));
typedef char char256 __attribute__((ext_vector_type(256)));
typedef char char512 __attribute__((ext_vector_type(512)));
typedef char char1024 __attribute__((ext_vector_type(1024)));

typedef uchar uchar32 __attribute__((ext_vector_type(32)));
typedef uchar uchar64 __attribute__((ext_vector_type(64)));
typedef uchar uchar128 __attribute__((ext_vector_type(128)));
typedef uchar uchar256 __attribute__((ext_vector_type(256)));
typedef uchar uchar512 __attribute__((ext_vector_type(512)));
typedef uchar uchar1024 __attribute__((ext_vector_type(1024)));

typedef long long12 __attribute__((ext_vector_type(12)));
typedef long long24 __attribute__((ext_vector_type(24)));
typedef long long48 __attribute__((ext_vector_type(48)));
typedef long long96 __attribute__((ext_vector_type(96)));
typedef long long192 __attribute__((ext_vector_type(192)));

typedef ulong ulong12 __attribute__((ext_vector_type(12)));
typedef ulong ulong24 __attribute__((ext_vector_type(24)));
typedef ulong ulong48 __attribute__((ext_vector_type(48)));
typedef ulong ulong96 __attribute__((ext_vector_type(96)));
typedef ulong ulong192 __attribute__((ext_vector_type(192)));

typedef float float12 __attribute__((ext_vector_type(12)));
typedef float float24 __attribute__((ext_vector_type(24)));
typedef float float48 __attribute__((ext_vector_type(48)));
typedef float float96 __attribute__((ext_vector_type(96)));
typedef float float192 __attribute__((ext_vector_type(192)));

typedef double double12 __attribute__((ext_vector_type(12)));
typedef double double24 __attribute__((ext_vector_type(24)));
typedef double double48 __attribute__((ext_vector_type(48)));
typedef double double96 __attribute__((ext_vector_type(96)));
typedef double double192 __attribute__((ext_vector_type(192)));

typedef uint uint12 __attribute__((ext_vector_type(12)));
typedef uint uint24 __attribute__((ext_vector_type(24)));
typedef uint uint48 __attribute__((ext_vector_type(48)));
typedef uint uint96 __attribute__((ext_vector_type(96)));
typedef uint uint192 __attribute__((ext_vector_type(192)));

typedef int int12 __attribute__((ext_vector_type(12)));
typedef int int24 __attribute__((ext_vector_type(24)));
typedef int int48 __attribute__((ext_vector_type(48)));
typedef int int96 __attribute__((ext_vector_type(96)));
typedef int int192 __attribute__((ext_vector_type(192)));

typedef short short12 __attribute__((ext_vector_type(12)));
typedef short short24 __attribute__((ext_vector_type(24)));
typedef short short48 __attribute__((ext_vector_type(48)));
typedef short short96 __attribute__((ext_vector_type(96)));
typedef short short192 __attribute__((ext_vector_type(192)));

typedef ushort ushort12 __attribute__((ext_vector_type(12)));
typedef ushort ushort24 __attribute__((ext_vector_type(24)));
typedef ushort ushort48 __attribute__((ext_vector_type(48)));
typedef ushort ushort96 __attribute__((ext_vector_type(96)));
typedef ushort ushort192 __attribute__((ext_vector_type(192)));

typedef half half12 __attribute__((ext_vector_type(12)));
typedef half half24 __attribute__((ext_vector_type(24)));
typedef half half48 __attribute__((ext_vector_type(48)));
typedef half half96 __attribute__((ext_vector_type(96)));
typedef half half192 __attribute__((ext_vector_type(192)));

typedef char char12 __attribute__((ext_vector_type(12)));
typedef char char24 __attribute__((ext_vector_type(24)));
typedef char char48 __attribute__((ext_vector_type(48)));
typedef char char96 __attribute__((ext_vector_type(96)));
typedef char char192 __attribute__((ext_vector_type(192)));

typedef uchar uchar12 __attribute__((ext_vector_type(12)));
typedef uchar uchar24 __attribute__((ext_vector_type(24)));
typedef uchar uchar48 __attribute__((ext_vector_type(48)));
typedef uchar uchar96 __attribute__((ext_vector_type(96)));
typedef uchar uchar192 __attribute__((ext_vector_type(192)));
