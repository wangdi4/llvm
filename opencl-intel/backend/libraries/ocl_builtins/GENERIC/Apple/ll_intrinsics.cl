
// This contains the C declarations for the functions in the file 
// ll_intrinsics.ll  ; Note that this file is included into other builtins
// which already declare all of the OpenCL types.

short __ocl_zext_v1i8_v1i16(char x) {return (short)0;}
short __ocl_sext_v1i8_v1i16(char x) {return (short)0;}
char __ocl_trunc_v1i16_v1i8(short x) {return (char)0;}
int __ocl_zext_v1i8_v1i32(char x) {return (int)0;}
int __ocl_sext_v1i8_v1i32(char x) {return (int)0;}
char __ocl_trunc_v1i32_v1i8(int x) {return (char)0;}
long __ocl_zext_v1i8_v1i64(char x) {return (long)0;}
long __ocl_sext_v1i8_v1i64(char x) {return (long)0;}
char __ocl_trunc_v1i64_v1i8(long x) {return (char)0;}
int __ocl_zext_v1i16_v1i32(short x) {return (int)0;}
int __ocl_sext_v1i16_v1i32(short x) {return (int)0;}
short __ocl_trunc_v1i32_v1i16(int x) {return (short)0;}
long __ocl_zext_v1i16_v1i64(short x) {return (long)0;}
long __ocl_sext_v1i16_v1i64(short x) {return (long)0;}
short __ocl_trunc_v1i64_v1i16(long x) {return (short)0;}
long __ocl_zext_v1i32_v1i64(int x) {return (long)0;}
long __ocl_sext_v1i32_v1i64(int x) {return (long)0;}
int __ocl_trunc_v1i64_v1i32(long x) {return (int)0;}

short2 __ocl_zext_v2i8_v2i16(char2 x) {return (short2)0;}
short2 __ocl_sext_v2i8_v2i16(char2 x) {return (short2)0;}
char2 __ocl_trunc_v2i16_v2i8(short2 x) {return (char2)0;}
int2 __ocl_zext_v2i8_v2i32(char2 x) {return (int2)0;}
int2 __ocl_sext_v2i8_v2i32(char2 x) {return (int2)0;}
char2 __ocl_trunc_v2i32_v2i8(int2 x) {return (char2)0;}
long2 __ocl_zext_v2i8_v2i64(char2 x) {return (long2)0;}
long2 __ocl_sext_v2i8_v2i64(char2 x) {return (long2)0;}
char2 __ocl_trunc_v2i64_v2i8(long2 x) {return (char2)0;}
int2 __ocl_zext_v2i16_v2i32(short2 x) {return (int2)0;}
int2 __ocl_sext_v2i16_v2i32(short2 x) {return (int2)0;}
short2 __ocl_trunc_v2i32_v2i16(int2 x) {return (short2)0;}
long2 __ocl_zext_v2i16_v2i64(short2 x) {return (long2)0;}
long2 __ocl_sext_v2i16_v2i64(short2 x) {return (long2)0;}
short2 __ocl_trunc_v2i64_v2i16(long2 x) {return (short2)0;}
long2 __ocl_zext_v2i32_v2i64(int2 x) {return (long2)0;}
long2 __ocl_sext_v2i32_v2i64(int2 x) {return (long2)0;}
int2 __ocl_trunc_v2i64_v2i32(long2 x) {return (int2)0;}

short3 __ocl_zext_v3i8_v3i16(char3 x) {return (short3)0;}
short3 __ocl_sext_v3i8_v3i16(char3 x) {return (short3)0;}
char3 __ocl_trunc_v3i16_v3i8(short3 x) {return (char3)0;}
int3 __ocl_zext_v3i8_v3i32(char3 x) {return (int3)0;}
int3 __ocl_sext_v3i8_v3i32(char3 x) {return (int3)0;}
char3 __ocl_trunc_v3i32_v3i8(int3 x) {return (char3)0;}
long3 __ocl_zext_v3i8_v3i64(char3 x) {return (long3)0;}
long3 __ocl_sext_v3i8_v3i64(char3 x) {return (long3)0;}
char3 __ocl_trunc_v3i64_v3i8(long3 x) {return (char3)0;}
int3 __ocl_zext_v3i16_v3i32(short3 x) {return (int3)0;}
int3 __ocl_sext_v3i16_v3i32(short3 x) {return (int3)0;}
short3 __ocl_trunc_v3i32_v3i16(int3 x) {return (short3)0;}
long3 __ocl_zext_v3i16_v3i64(short3 x) {return (long3)0;}
long3 __ocl_sext_v3i16_v3i64(short3 x) {return (long3)0;}
short3 __ocl_trunc_v3i64_v3i16(long3 x) {return (short3)0;}
long3 __ocl_zext_v3i32_v3i64(int3 x) {return (long3)0;}
long3 __ocl_sext_v3i32_v3i64(int3 x) {return (long3)0;}
int3 __ocl_trunc_v3i64_v3i32(long3 x) {return (int3)0;}

short4 __ocl_zext_v4i8_v4i16(char4 x) {return (short4)0;}
short4 __ocl_sext_v4i8_v4i16(char4 x) {return (short4)0;}
char4 __ocl_trunc_v4i16_v4i8(short4 x) {return (char4)0;}
int4 __ocl_zext_v4i8_v4i32(char4 x) {return (int4)0;}
int4 __ocl_sext_v4i8_v4i32(char4 x) {return (int4)0;}
char4 __ocl_trunc_v4i32_v4i8(int4 x) {return (char4)0;}
long4 __ocl_zext_v4i8_v4i64(char4 x) {return (long4)0;}
long4 __ocl_sext_v4i8_v4i64(char4 x) {return (long4)0;}
char4 __ocl_trunc_v4i64_v4i8(long4 x) {return (char4)0;}
int4 __ocl_zext_v4i16_v4i32(short4 x) {return (int4)0;}
int4 __ocl_sext_v4i16_v4i32(short4 x) {return (int4)0;}
short4 __ocl_trunc_v4i32_v4i16(int4 x) {return (short4)0;}
long4 __ocl_zext_v4i16_v4i64(short4 x) {return (long4)0;}
long4 __ocl_sext_v4i16_v4i64(short4 x) {return (long4)0;}
short4 __ocl_trunc_v4i64_v4i16(long4 x) {return (short4)0;}
long4 __ocl_zext_v4i32_v4i64(int4 x) {return (long4)0;}
long4 __ocl_sext_v4i32_v4i64(int4 x) {return (long4)0;}
int4 __ocl_trunc_v4i64_v4i32(long4 x) {return (int4)0;}

short8 __ocl_zext_v8i8_v8i16(char8 x) {return (short8)0;}
short8 __ocl_sext_v8i8_v8i16(char8 x) {return (short8)0;}
char8 __ocl_trunc_v8i16_v8i8(short8 x) {return (char8)0;}
int8 __ocl_zext_v8i8_v8i32(char8 x) {return (int8)0;}
int8 __ocl_sext_v8i8_v8i32(char8 x) {return (int8)0;}
char8 __ocl_trunc_v8i32_v8i8(int8 x) {return (char8)0;}
long8 __ocl_zext_v8i8_v8i64(char8 x) {return (long8)0;}
long8 __ocl_sext_v8i8_v8i64(char8 x) {return (long8)0;}
char8 __ocl_trunc_v8i64_v8i8(long8 x) {return (char8)0;}
int8 __ocl_zext_v8i16_v8i32(short8 x) {return (int8)0;}
int8 __ocl_sext_v8i16_v8i32(short8 x) {return (int8)0;}
short8 __ocl_trunc_v8i32_v8i16(int8 x) {return (short8)0;}
long8 __ocl_zext_v8i16_v8i64(short8 x) {return (long8)0;}
long8 __ocl_sext_v8i16_v8i64(short8 x) {return (long8)0;}
short8 __ocl_trunc_v8i64_v8i16(long8 x) {return (short8)0;}
long8 __ocl_zext_v8i32_v8i64(int8 x) {return (long8)0;}
long8 __ocl_sext_v8i32_v8i64(int8 x) {return (long8)0;}
int8 __ocl_trunc_v8i64_v8i32(long8 x) {return (int8)0;}

short16 __ocl_zext_v16i8_v16i16(char16 x) {return (short16)0;}
short16 __ocl_sext_v16i8_v16i16(char16 x) {return (short16)0;}
char16 __ocl_trunc_v16i16_v16i8(short16 x) {return (char16)0;}
int16 __ocl_zext_v16i8_v16i32(char16 x) {return (int16)0;}
int16 __ocl_sext_v16i8_v16i32(char16 x) {return (int16)0;}
char16 __ocl_trunc_v16i32_v16i8(int16 x) {return (char16)0;}
long16 __ocl_zext_v16i8_v16i64(char16 x) {return (long16)0;}
long16 __ocl_sext_v16i8_v16i64(char16 x) {return (long16)0;}
char16 __ocl_trunc_v16i64_v16i8(long16 x) {return (char16)0;}
int16 __ocl_zext_v16i16_v16i32(short16 x) {return (int16)0;}
int16 __ocl_sext_v16i16_v16i32(short16 x) {return (int16)0;}
short16 __ocl_trunc_v16i32_v16i16(int16 x) {return (short16)0;}
long16 __ocl_zext_v16i16_v16i64(short16 x) {return (long16)0;}
long16 __ocl_sext_v16i16_v16i64(short16 x) {return (long16)0;}
short16 __ocl_trunc_v16i64_v16i16(long16 x) {return (short16)0;}
long16 __ocl_zext_v16i32_v16i64(int16 x) {return (long16)0;}
long16 __ocl_sext_v16i32_v16i64(int16 x) {return (long16)0;}
int16 __ocl_trunc_v16i64_v16i32(long16 x) {return (int16)0;}

float __ocl_trunc_double_float(double x) {return (float)0;}
float2 __ocl_trunc_double2_float2(double2 x) {return (float2)0;}
float3 __ocl_trunc_double3_float3(double3 x) {return (float3)0;}
float4 __ocl_trunc_double4_float4(double4 x) {return (float4)0;}
float8 __ocl_trunc_double8_float8(double8 x) {return (float8)0;}
float16 __ocl_trunc_double16_float16(double16 x) {return (float16)0;}

long8 __ocl_select_v8i64(long8 x, long8 y, long8 c) {return (long8)0;}
int16 __ocl_select_v16i32(int16 x, int16 y, int16 c) {return (int16)0;}

char __ocl_helper_clz_v1u8(char x) {return (char)0;}
short __ocl_helper_clz_v1u16(short x) {return (short)0;}
int __ocl_helper_clz_v1u32(int x) {return (int)0;}
long __ocl_helper_clz_v1u64(long x) {return (long)0;}
char __ocl_helper_ctz_v1u8(char x) {return (char)0;}
short __ocl_helper_ctz_v1u16(short x) {return (short)0;}
int __ocl_helper_ctz_v1u32(int x) {return (int)0;}
long __ocl_helper_ctz_v1u64(long x) {return (long)0;}
