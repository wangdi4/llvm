// CQ#366961
// RUN: %clang_cc1 -fintel-compatibility --extended_float_types -emit-llvm %s -o - | FileCheck %s

// CHECK: %struct.anon = type { i32, fp128 }
struct {
  int t;
  _Quad q;
} my_quad;

// CHECK: common global fp128 0xL00000000000000000000000000000000
_Quad q1, q2, result;

// CHECK: common global fp128* null
_Quad *pointer;

void check() {
  // CHECK: store fp128* @q1, fp128** @pointer, align 8
  pointer = &q1;

  // CHECK: %0 = load fp128** @pointer, align 8
  // CHECK: %1 = load fp128* %0, align 16
  // CHECK: store fp128 %1, fp128* @result, align 16
  result = *pointer;

  // CHECK: %2 = load fp128* @q1, align 16
  // CHECK: %3 = load fp128* @q2, align 16
  // CHECK: %mul = fmul fp128 %2, %3
  // CHECK: store fp128 %mul, fp128* @result, align 16
  result = q1 * q2;

  // CHECK: %4 = load fp128* @q1, align 16
  // CHECK: %5 = load fp128* @q2, align 16
  // CHECK: %div = fdiv fp128 %4, %5
  // CHECK: store fp128 %div, fp128* @result, align 16
  result = q1 / q2;

  // CHECK: %6 = load fp128* @q1, align 16
  // CHECK: %7 = load fp128* @q2, align 16
  // CHECK: %add = fadd fp128 %6, %7
  // CHECK: store fp128 %add, fp128* @result, align 16
  result = q1 + q2;

  // CHECK: %8 = load fp128* @q1, align 16
  // CHECK: %9 = load fp128* @q2, align 16
  // CHECK: %sub = fsub fp128 %8, %9
  // CHECK: store fp128 %sub, fp128* @result, align 16
  result = q1 - q2;

  // CHECK: %10 = load fp128* @result, align 16
  // CHECK: %conv = fptosi fp128 %10 to i128
  // CHECK: store i128 %conv, i128* %v, align 16
  __int128 v = result;

  // CHECK: %11 = load i128* %v, align 16
  // CHECK: %conv1 = sitofp i128 %11 to fp128
  // CHECK: store fp128 %conv1, fp128* @result, align 16
  result = v;
}

int check_sizeof_Quad() {
  // CHECK: ret i32 16
  return sizeof(_Quad);
}
