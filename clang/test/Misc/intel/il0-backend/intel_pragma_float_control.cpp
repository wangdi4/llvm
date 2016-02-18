// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//expected-no-diagnostics
//***INTEL: pragma float_control test

#pragma float_control(push)
// CHECK: define void @{{.*}}f1{{.*}} #0
// CHECK: ret
// CHECK-NEXT: }
void f1(){}
#pragma float_control(except, off)
// CHECK: define void @{{.*}}f2{{.*}} #1
// CHECK: ret
// CHECK-NEXT: }
void f2(){}
#pragma float_control(precise, on, push)
// CHECK: define void @{{.*}}f3{{.*}} #2
// CHECK: ret
// CHECK-NEXT: }
void f3(){}
#pragma float_control(except, on)
// CHECK: define void @{{.*}}f4{{.*}} #3
// CHECK: ret
// CHECK-NEXT: }
void f4(){}
#pragma float_control(except, off)
// CHECK: define void @{{.*}}f5{{.*}} #1
// CHECK: ret
// CHECK-NEXT: }
void f5(){}
#pragma float_control(source, on)
// CHECK: define void @{{.*}}f6{{.*}} #4
// CHECK: ret
// CHECK-NEXT: }
void f6(){}
#pragma float_control(except, on)
// CHECK: define void @{{.*}}f7{{.*}} #3
// CHECK: ret
// CHECK-NEXT: }
void f7(){}
#pragma float_control(except, off)
// CHECK: define void @{{.*}}f8{{.*}} #1
// CHECK: ret
// CHECK-NEXT: }
void f8(){}
#pragma float_control(source, off)
// CHECK: define void @{{.*}}f9{{.*}} #5
// CHECK: ret
// CHECK-NEXT: }
void f9(){}
#pragma float_control(double, on)
// CHECK: define void @{{.*}}f10{{.*}} #6
// CHECK: ret
// CHECK-NEXT: }
void f10(){}
#pragma float_control(except, on)
// CHECK: define void @{{.*}}f11{{.*}} #3
// CHECK: ret
// CHECK-NEXT: }
void f11(){}
#pragma float_control(except, off)
// CHECK: define void @{{.*}}f12{{.*}} #1
// CHECK: ret
// CHECK-NEXT: }
void f12(){}
#pragma float_control(double, off)
// CHECK: define void @{{.*}}f13{{.*}} #7
// CHECK: ret
// CHECK-NEXT: }
void f13(){}
#pragma float_control(extended, on)
// CHECK: define void @{{.*}}f14{{.*}} #8
// CHECK: ret
// CHECK-NEXT: }
void f14(){}
#pragma float_control(except, on)
// CHECK: define void @{{.*}}f15{{.*}} #3
// CHECK: ret
// CHECK-NEXT: }
void f15(){}
#pragma float_control(except, off)
// CHECK: define void @{{.*}}f16{{.*}} #1
// CHECK: ret
// CHECK-NEXT: }
void f16(){}
#pragma float_control(extended, off)
// CHECK: define void @{{.*}}f17{{.*}} #9
// CHECK: ret
// CHECK-NEXT: }
void f17(){}
#pragma float_control(pop)
// CHECK: define void @{{.*}}f18{{.*}} #0
// CHECK: ret
// CHECK-NEXT: }
void f18(){}
#pragma float_control(pop)
// CHECK: define void @{{.*}}f19{{.*}} #0
// CHECK: ret
// CHECK-NEXT: }
void f19(){}
#pragma float_control(pop)
// CHECK: define void @{{.*}}f20{{.*}} #0
// CHECK: ret
// CHECK-NEXT: }
void f20(){}
#pragma float_control(push)
#pragma float_control(precise, on, push)
// CHECK: define void @{{.*}}f21{{.*}} #2
// CHECK: ret
// CHECK-NEXT: }
void f21(){}
#pragma float_control(except, on, push)
// CHECK: define void @{{.*}}f22{{.*}} #3
// CHECK: ret
// CHECK-NEXT: }
void f22(){}
#pragma float_control(pop)
// CHECK: define void @{{.*}}f23{{.*}} #10
// CHECK: ret
// CHECK-NEXT: }
void f23(){}
#pragma float_control(pop)
// CHECK: define void @{{.*}}f24{{.*}} #0
// CHECK: ret
// CHECK-NEXT: }
void f24(){}
#pragma float_control(pop)
// CHECK: define void @{{.*}}f25{{.*}} #0
// CHECK: ret
// CHECK-NEXT: }
void f25(){}

// CHECK: define i32 @main() #11
int main() {
#pragma float_control(push)
#pragma float_control(precise, on, push)
#pragma float_control(except, on, push)
#pragma float_control(pop)
#pragma float_control(pop)
#pragma float_control(pop)
  return (0);
}


// CHECK: attributes #0 = { nounwind
// CHECK-SAME: "less
// CHECK: attributes #1 = { nounwind "INTEL:EXCEPT-OFF"
// CHECK: attributes #2 = { nounwind "INTEL:PRECISE-ON"
// CHECK: attributes #3 = { nounwind "INTEL:EXCEPT-ON"
// CHECK: attributes #4 = { nounwind "INTEL:SOURCE-ON"
// CHECK: attributes #5 = { nounwind "INTEL:SOURCE-OFF"
// CHECK: attributes #6 = { nounwind "INTEL:DOUBLE-ON"
// CHECK: attributes #7 = { nounwind "INTEL:DOUBLE-OFF"
// CHECK: attributes #8 = { nounwind "INTEL:EXTENDED-ON"
// CHECK: attributes #9 = { nounwind "INTEL:EXTENDED-OFF"
// CHECK: attributes #10 = { nounwind "INTEL:EXCEPT-OFF" "INTEL:PRECISE-ON"
// CHECK: attributes #11 = { norecurse nounwind
// CHECK-SAME: "less

