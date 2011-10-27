// RUN: %clang_cc1 %s -emit-llvm -O0 -o - | FileCheck %s

typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int8 __attribute__((ext_vector_type(8)));
typedef int int16 __attribute__((ext_vector_type(16)));

void foo(void) {
  int2 auto1 = (int2)(1);
// CHECK: store <2 x i32> <i32 1, i32 1>, <2 x i32>*
  int3 auto2 = (int3)(2);
// CHECK: store <3 x i32> <i32 2, i32 2, i32 2>, <3 x i32>*
  int4 auto3 = (int4)(3);
// CHECK: store <4 x i32> <i32 3, i32 3, i32 3, i32 3>, <4 x i32>*
  int8 auto4 = (int8)(4);
// CHECK: store <8 x i32> <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>, <8 x i32>*
  int16 auto5 = (int16)(5);
// CHECK: store <16 x i32> <i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5>, <16 x i32>*
  int2 auto6 = (int2)(1, 2);
// CHECK: store <2 x i32> <i32 1, i32 2>, <2 x i32>*
  int3 auto7 = (int3)(1, 2, 3);
// CHECK: store <3 x i32> <i32 1, i32 2, i32 3>, <3 x i32>*
  int4 auto8 = (int4)(1, 2, 3, 4);
// CHECK: store <4 x i32> <i32 1, i32 2, i32 3, i32 4>, <4 x i32>*
  int8 auto9 = (int8)(1, 2, 3, 4, 5, 6, 7, 8);
// CHECK: store <8 x i32> <i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8>, <8 x i32>*
  int16 auto10 = (int16)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
// CHECK: store <16 x i32> <i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16>, <16 x i32>*
  int8 auto11 = (int8)(auto8, auto8);
  int8 auto12 = (int8)((int4)(0,1,2,3),(int4)(4,5,6,7));
}
