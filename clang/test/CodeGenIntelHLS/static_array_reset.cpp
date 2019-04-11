//RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -debug-info-kind=limited -emit-llvm -o %t %s

//CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{staticreset:2}
//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{staticreset:0}
//CHECK: [[ANN3:@.str[\.]*[0-9]*]] = {{.*}}{staticreset:1}
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4,8}{staticreset:1}

//CHECK: @llvm.global.annotations

void foo()
{
//CHECK-SAME: array_one{{.*}}[[ANN1]]{{.*}}i32 14
  static int array_one[16];
//CHECK-SAME: array_two{{.*}}[[ANN2]]{{.*}}i32 16
  static int array_two[32] __attribute__((static_array_reset(0)));
//CHECK-SAME: array_thr{{.*}}[[ANN3]]{{.*}}i32 18
  static int array_thr[64] __attribute__((static_array_reset(1)));
}

void bar()
{
//CHECK-SAME: array_fou{{.*}}[[ANN3]]{{.*}}i32 24
  static int array_fou[48] __attribute__((address_space(4),
                                          static_array_reset(1)));
}

//CHECK-SAME: array_gl_one{{.*}}[[ANN2]]{{.*}}i32 29
static int array_gl_one[8] __attribute__((static_array_reset(0)));
//CHECK-SAME: array_gl_two{{.*}}[[ANN4]]{{.*}}i32 31
static int array_gl_two[8] __attribute__((static_array_reset(1), memory));
int use() { return array_gl_one[0] + array_gl_two[1]; }
