// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma optimize

#pragma optimize("",on)
// CHECK: define void @{{.*}}f1
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f1(){}
#pragma optimize("asq" "asda",off) // expected-warning {{'on' or 'off' expected}}
// CHECK: define void @{{.*}}f2
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f2(){}
// CHECK: define i32 @main()
int main() {
  int i = 13, j;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
// CHECK: ret i32 0
#pragma optimize("",on)
  return (0);
}

struct S {
#pragma optimize("" off)
  int a;
};
// CHECK: define void @{{.*}}f3
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOOPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f3(){}

static int a;

#pragma optimize "" off)
// CHECK: define void @{{.*}}f4
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOOPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f4(){}
#pragma optimize ( "asa") // expected-warning {{'on' or 'off' expected}}
// CHECK: define void @{{.*}}f5
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f5(){}
#pragma optimize ( asa, off) // expected-warning {{expected a string}}
// CHECK: define void @{{.*}}f6
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f6(){}
#pragma optimize ("", on
// CHECK: define void @{{.*}}f7
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f7(){}
#pragma optimize ("", on)
// CHECK: define void @{{.*}}f8
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f8(){}
#pragma optimize ("", on)
// CHECK: define void @{{.*}}f9
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f9(){}
#pragma optimize (on) // expected-warning {{expected a string}}
// CHECK: define void @{{.*}}f10
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f10(){}
#pragma optimize ("", on  off // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK: define i32 @{{.*}}www
int www() {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPTIMIZE")
   return a;
}
// CHECK: }

#pragma optimize ("", off)
// CHECK: define void @{{.*}}aaa
void aaa() {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOOPTIMIZE")
  return;
}
// CHECK: }

