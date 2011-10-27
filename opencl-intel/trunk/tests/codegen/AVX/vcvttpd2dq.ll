; RUN: llc -mcpu=sandybridge < %s | FileCheck %s
define  <4 x i32> @__Vectorized_test_implicit_int_double() nounwind {
; CHECK: vcvttpd2dq %ymm0, %xmm0
  %1 = fptosi <4 x double> undef to <4 x i32>
  ret <4 x i32> %1
}
