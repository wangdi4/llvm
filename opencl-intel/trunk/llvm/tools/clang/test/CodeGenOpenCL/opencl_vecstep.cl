// RUN: %clang_cc1 %s -emit-llvm -O0 -o - | FileCheck %s

typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int8 __attribute__((ext_vector_type(8)));
typedef int int16 __attribute__((ext_vector_type(16)));

void foo(int3 arg1, int8 arg2) {
  int4 auto1;
  int16 *auto2;
  int auto3;
  
  int res1 = vec_step(arg1);
// CHECK: store i32 4, i32* %res1
  int res2 = vec_step(arg2);
// CHECK: store i32 8, i32* %res2
  int res3 = vec_step(auto1);
// CHECK: store i32 4, i32* %res3
  int res4 = vec_step(auto2);
// CHECK: store i32 1, i32* %res4
  int res5 = vec_step(auto3);
// CHECK: store i32 1, i32* %res5
  int res6 = vec_step(int2);
// CHECK: store i32 2, i32* %res6
  int res7 = vec_step(int3);
// CHECK: store i32 4, i32* %res7
  int res8 = vec_step(int4);
// CHECK: store i32 4, i32* %res8
  int res9 = vec_step(int8);
// CHECK: store i32 8, i32* %res9
  int res10 = vec_step(int16);
// CHECK: store i32 16, i32* %res10
  int res11 = vec_step(&auto1);
// CHECK: store i32 1, i32* %res11
  int res12 = vec_step(*auto2);
// CHECK: store i32 16, i32* %res12
}
