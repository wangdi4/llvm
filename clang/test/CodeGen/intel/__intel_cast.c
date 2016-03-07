// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

float f32;
double f64;

unsigned int u32;
unsigned __int64 u64;

int foo() {
  f32 += 1.0f;
  f64 += 1.0;

  u32 = __intel_castf32_u32(f32);
  u64 = __intel_castf64_u64(f64);

  if (u32 != 0x3f800000 || u64 != 0x3ff0000000000000) {
      return 1;
  }

  u32 |= 0x80000000;
  u64 |= 0x8000000000000000;

  f32 = __intel_castu32_f32(u32);
  f64 = __intel_castu64_f64(u64);

  if (f32 != -1.0f || f64 != -1.0) {
      return 2;
  }

  f32 *= .125f;
  f32 *= 3.0f;
  f64 *= .375;

  u32 = __intel_castf32_u32(f32);
  u64 = __intel_castf64_u64(f64);

  if (u32 != 0xbec00000 || u64 != 0xbfd8000000000000) {
      return 3;
  }

  u32 ^= 0x80000000;
  u64 ^= 0x8000000000000000;

  f32 = __intel_castu32_f32(u32);
  f64 = __intel_castu64_f64(u64);

  if (f32 != .375f || f64 != .375) {
      return 4;
  }
  return 0;
}

// CHECK-LABEL: foo
// CHECK: bitcast float {{.+}} to i32
// CHECK: bitcast double {{.+}} to i64
// CHECK: bitcast i32 {{.+}} to float
// CHECK: bitcast i64 {{.+}} to double
// CHECK: bitcast float {{.+}} to i32
// CHECK: bitcast double {{.+}} to i64
// CHECK: bitcast i32 {{.+}} to float
// CHECK: bitcast i64 {{.+}} to double

int bar() {
  u32 = __intel_castf32_u32(1ULL);
  u64 = __intel_castf64_u64(1ULL);
  f32 = __intel_castu32_f32(1ULL);
  f64 = __intel_castu64_f64(1ULL);
  u32 = __intel_castf32_u32(1.0f);
  u64 = __intel_castf64_u64(1.0f);
  f32 = __intel_castu32_f32(1.0f);
  f64 = __intel_castu64_f64(1.0f);
  u32 = __intel_castf32_u32(1.0);
  u64 = __intel_castf64_u64(1.0);
  f32 = __intel_castu32_f32(1.0);
  f64 = __intel_castu64_f64(1.0);
  u32 = __intel_castf32_u32(u32);
  u64 = __intel_castf64_u64(u32);
  f32 = __intel_castu32_f32(u32);
  f64 = __intel_castu64_f64(u32);
  u32 = __intel_castf32_u32(u64);
  u64 = __intel_castf64_u64(u64);
  f32 = __intel_castu32_f32(u64);
  f64 = __intel_castu64_f64(u64);
  f32 = __intel_castu32_f32(f32);
  f64 = __intel_castu64_f64(f32);
  u32 = __intel_castf32_u32(f64);
  u64 = __intel_castf64_u64(f64);
  f32 = __intel_castu32_f32(f64);
  f64 = __intel_castu64_f64(f64);
  return 0;
}

// CHECK-LABEL: bar
// CHECK: uitofp i32 {{.+}} to float
// CHECK: bitcast float {{.+}} to i32
// CHECK: uitofp i32 {{.+}} to double
// CHECK: bitcast double {{.+}} to i64
