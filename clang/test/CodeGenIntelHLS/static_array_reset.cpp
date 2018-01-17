//RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s

void foo()
{
  static int array_one[16];
  static int array_two[32] __attribute__((static_array_reset(0)));
  static int array_thr[64] __attribute__((static_array_reset(1)));
}

void bar()
{
  static int array_fou[48] __attribute__((address_space(4),
                                          static_array_reset(1)));
}

//CHECK: !hls.staticreset = !{[[A1:![0-9]+]], [[A2:![0-9]+]], [[A3:![0-9]+]],  [[A4:![0-9]+]]}
//CHECK: [[A1]] = !{i32 0, [16 x i32]* {{.*}}array_one{{.*}}, [[AA1:![0-9]+]]}
//CHECK: [[AA1]] = !{!"staticreset", i32 2, !"unset"}
//CHECK: [[A2]] = !{i32 0, [32 x i32]* {{.*}}array_two{{.*}}, [[AA2:![0-9]+]]}
//CHECK: [[AA2]] = !{!"staticreset", i32 0, !"unset"}
//CHECK: [[A3]] = !{i32 0, [64 x i32]* {{.*}}array_thr{{.*}}, [[AA3:![0-9]+]]}
//CHECK: [[AA3]] = !{!"staticreset", i32 1, !"unset"}
//CHECK: [[A4]] = !{i32 4, [48 x i32]{{.*}}array_fou{{.*}}, [[AA3]]}
