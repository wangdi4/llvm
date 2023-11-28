// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

// This contains the C declarations for the functions in the file
// ll_intrinsics.ll ; Note that this file is included into other builtins
// which already declare all of the OpenCL types.

_1i16 __ocl_zext_v1i8_v1i16(_1i8);
_1i16 __ocl_sext_v1i8_v1i16(_1i8);
_1i8 __ocl_trunc_v1i16_v1i8(_1i16);
_1i32 __ocl_zext_v1i8_v1i32(_1i8);
_1i32 __ocl_sext_v1i8_v1i32(_1i8);
_1i8 __ocl_trunc_v1i32_v1i8(_1i32);
_1i64 __ocl_zext_v1i8_v1i64(_1i8);
_1i64 __ocl_sext_v1i8_v1i64(_1i8);
_1i8 __ocl_trunc_v1i64_v1i8(_1i64);
_1i32 __ocl_zext_v1i16_v1i32(_1i16);
_1i32 __ocl_sext_v1i16_v1i32(_1i16);
_1i16 __ocl_trunc_v1i32_v1i16(_1i32);
_1i64 __ocl_zext_v1i16_v1i64(_1i16);
_1i64 __ocl_sext_v1i16_v1i64(_1i16);
_1i16 __ocl_trunc_v1i64_v1i16(_1i64);
_1i64 __ocl_zext_v1i32_v1i64(_1i32);
_1i64 __ocl_sext_v1i32_v1i64(_1i32);
_1i32 __ocl_trunc_v1i64_v1i32(_1i64);
_2i16 __ocl_zext_v2i8_v2i16(_2i8);
_2i16 __ocl_sext_v2i8_v2i16(_2i8);
_2i8 __ocl_trunc_v2i16_v2i8(_2i16);
_2i32 __ocl_zext_v2i8_v2i32(_2i8);
_2i32 __ocl_sext_v2i8_v2i32(_2i8);
_2i8 __ocl_trunc_v2i32_v2i8(_2i32);
_2i64 __ocl_zext_v2i8_v2i64(_2i8);
_2i64 __ocl_sext_v2i8_v2i64(_2i8);
_2i8 __ocl_trunc_v2i64_v2i8(_2i64);
_2i32 __ocl_zext_v2i16_v2i32(_2i16);
_2i32 __ocl_sext_v2i16_v2i32(_2i16);
_2i16 __ocl_trunc_v2i32_v2i16(_2i32);
_2i64 __ocl_zext_v2i16_v2i64(_2i16);
_2i64 __ocl_sext_v2i16_v2i64(_2i16);
_2i16 __ocl_trunc_v2i64_v2i16(_2i64);
_2i64 __ocl_zext_v2i32_v2i64(_2i32);
_2i64 __ocl_sext_v2i32_v2i64(_2i32);
_2i32 __ocl_trunc_v2i64_v2i32(_2i64);
_3i16 __ocl_zext_v3i8_v3i16(_3i8);
_3i16 __ocl_sext_v3i8_v3i16(_3i8);
_3i8 __ocl_trunc_v3i16_v3i8(_3i16);
_3i32 __ocl_zext_v3i8_v3i32(_3i8);
_3i32 __ocl_sext_v3i8_v3i32(_3i8);
_3i8 __ocl_trunc_v3i32_v3i8(_3i32);
_3i64 __ocl_zext_v3i8_v3i64(_3i8);
_3i64 __ocl_sext_v3i8_v3i64(_3i8);
_3i8 __ocl_trunc_v3i64_v3i8(_3i64);
_3i32 __ocl_zext_v3i16_v3i32(_3i16);
_3i32 __ocl_sext_v3i16_v3i32(_3i16);
_3i16 __ocl_trunc_v3i32_v3i16(_3i32);
_3i64 __ocl_zext_v3i16_v3i64(_3i16);
_3i64 __ocl_sext_v3i16_v3i64(_3i16);
_3i16 __ocl_trunc_v3i64_v3i16(_3i64);
_3i64 __ocl_zext_v3i32_v3i64(_3i32);
_3i64 __ocl_sext_v3i32_v3i64(_3i32);
_3i32 __ocl_trunc_v3i64_v3i32(_3i64);
_4i16 __ocl_zext_v4i8_v4i16(_4i8);
_4i16 __ocl_sext_v4i8_v4i16(_4i8);
_4i8 __ocl_trunc_v4i16_v4i8(_4i16);
_4i32 __ocl_zext_v4i8_v4i32(_4i8);
_4i32 __ocl_sext_v4i8_v4i32(_4i8);
_4i8 __ocl_trunc_v4i32_v4i8(_4i32);
_4i64 __ocl_zext_v4i8_v4i64(_4i8);
_4i64 __ocl_sext_v4i8_v4i64(_4i8);
_4i8 __ocl_trunc_v4i64_v4i8(_4i64);
_4i32 __ocl_zext_v4i16_v4i32(_4i16);
_4i32 __ocl_sext_v4i16_v4i32(_4i16);
_4i16 __ocl_trunc_v4i32_v4i16(_4i32);
_4i64 __ocl_zext_v4i16_v4i64(_4i16);
_4i64 __ocl_sext_v4i16_v4i64(_4i16);
_4i16 __ocl_trunc_v4i64_v4i16(_4i64);
_4i64 __ocl_zext_v4i32_v4i64(_4i32);
_4i64 __ocl_sext_v4i32_v4i64(_4i32);
_4i32 __ocl_trunc_v4i64_v4i32(_4i64);
_8i16 __ocl_zext_v8i8_v8i16(_8i8);
_8i16 __ocl_sext_v8i8_v8i16(_8i8);
_8i8 __ocl_trunc_v8i16_v8i8(_8i16);
_8i32 __ocl_zext_v8i8_v8i32(_8i8);
_8i32 __ocl_sext_v8i8_v8i32(_8i8);
_8i8 __ocl_trunc_v8i32_v8i8(_8i32);
_8i64 __ocl_zext_v8i8_v8i64(_8i8);
_8i64 __ocl_sext_v8i8_v8i64(_8i8);
_8i8 __ocl_trunc_v8i64_v8i8(_8i64);
_8i32 __ocl_zext_v8i16_v8i32(_8i16);
_8i32 __ocl_sext_v8i16_v8i32(_8i16);
_8i16 __ocl_trunc_v8i32_v8i16(_8i32);
_8i64 __ocl_zext_v8i16_v8i64(_8i16);
_8i64 __ocl_sext_v8i16_v8i64(_8i16);
_8i16 __ocl_trunc_v8i64_v8i16(_8i64);
_8i64 __ocl_zext_v8i32_v8i64(_8i32);
_8i64 __ocl_sext_v8i32_v8i64(_8i32);
_8i32 __ocl_trunc_v8i64_v8i32(_8i64);
_16i16 __ocl_zext_v16i8_v16i16(_16i8);
_16i16 __ocl_sext_v16i8_v16i16(_16i8);
_16i8 __ocl_trunc_v16i16_v16i8(_16i16);
_16i32 __ocl_zext_v16i8_v16i32(_16i8);
_16i32 __ocl_sext_v16i8_v16i32(_16i8);
_16i8 __ocl_trunc_v16i32_v16i8(_16i32);
_16i64 __ocl_zext_v16i8_v16i64(_16i8);
_16i64 __ocl_sext_v16i8_v16i64(_16i8);
_16i8 __ocl_trunc_v16i64_v16i8(_16i64);
_16i32 __ocl_zext_v16i16_v16i32(_16i16);
_16i32 __ocl_sext_v16i16_v16i32(_16i16);
_16i16 __ocl_trunc_v16i32_v16i16(_16i32);
_16i64 __ocl_zext_v16i16_v16i64(_16i16);
_16i64 __ocl_sext_v16i16_v16i64(_16i16);
_16i16 __ocl_trunc_v16i64_v16i16(_16i64);
_16i64 __ocl_zext_v16i32_v16i64(_16i32);
_16i64 __ocl_sext_v16i32_v16i64(_16i32);
_16i32 __ocl_trunc_v16i64_v16i32(_16i64);
void __ocl_expand_mask_4x16(ushort, ushort *, ushort *, ushort *, ushort *);
