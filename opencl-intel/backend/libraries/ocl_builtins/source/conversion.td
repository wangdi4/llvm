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

// Temporarily add prototype defs so that math builtins can call conversion
// functions.
// TODO: migrate implementations.
let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64], [1, 2, 3, 4, 8, 16, 32, 64]>.Tout,
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
// conversion to integer has satured mode.
def : OclBuiltin<"convert_$ReturnType",         (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType",         (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType",         (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType",         (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType",         (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType",         (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType",         (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType",         (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
// conversions from floating to integer also honor rounding mode as well as saturated mode.
let ExceptionTypes = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [1, 2, 3, 4, 8, 16, 32, 64]>.Tout in {
def : OclBuiltin<"convert_$ReturnType_rte",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}
// conversion to floating may not has saturated mode.
def : OclBuiltin<"convert_$ReturnType",         (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType",         (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType",         (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
// the following conversions make no sense but are added for completeness.
//
// conversions from int to integer with rounding mode and/or saturated mode.
let ExceptionTypes = ExpandTypesByVFAndFlatten<[v1f16, v1f32, v1f64], [1, 2, 3, 4, 8, 16, 32, 64]>.Tout in {
def : OclBuiltin<"convert_$ReturnType_rte",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rte",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtz",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtp",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_rtn",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}
// conversion to floating make nosense with saturated mode.
def : OclBuiltin<"convert_$ReturnType_sat",     (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs f16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rte", (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}
