// RUN: %clang_cc1 -fms-compatibility -fintel-compatibility -triple=i686-windows-msvc %s -emit-llvm -o - | FileCheck %s

int main(void)
{
  double  _Complex c1, c2;
  c1 = c1 * c1;
// CHECK: [[RES1:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[RES2:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[RES3:%.+]] = fsub double [[RES1]], [[RES2]]
// CHECK: [[RES4:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[RES5:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[RES6:%.+]] = fadd double [[RES4]], [[RES5]]
  c2 = c2 / c2;
// CHECK: [[T0:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T1:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T2:%.+]] = fadd double [[T0]], [[T1]]
// CHECK: [[T3:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T4:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T5:%.+]] = fadd double [[T3]], [[T4]]
// CHECK: [[T6:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T7:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T8:%.+]] = fsub double [[T6]], [[T7]]
// CHECK: [[T9:%.+]] = fdiv double [[T2]], [[T5]]
// CHECK: [[T10:%.+]] = fdiv double [[T8]], [[T5]]
}

