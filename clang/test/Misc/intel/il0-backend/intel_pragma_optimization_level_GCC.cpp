// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -pragma-optimization-level=GCC -verify -o - %s | FileCheck %s
//***INTEL: pragma optimization_level

#pragma GCC optimization_level 1
struct AAAAAA{
  int a;
};
// CHECK: define void @{{.*}}f1
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 1)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f1(){}
// CHECK: define void @{{.*}}f2
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 1)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f2(){}
#pragma optimization_level -1 // expected-warning {{expected reset or an integer constant}}
#pragma optimization_level 10 // expected-warning {{optimization level must be between 0 and 3}}
#pragma optimization_level 1+1 // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK: define void @{{.*}}f3
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 1)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f3(){}
#pragma optimization_level reset 
// CHECK: define void @{{.*}}f4
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"RESET")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f4(){}
#pragma optimization_level 1
struct AAAAAA1{
  int a;
};

// CHECK: define void @{{.*}}aaa
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 1)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void aaaa() {
  ;
}
#pragma GCC optimization_level -1 // expected-warning {{expected reset or an integer constant}}
#pragma GCC optimization_level 10 // expected-warning {{optimization level must be between 0 and 3}}
#pragma GCC optimization_level reset
// CHECK: define void @{{.*}}f5
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"RESET")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f5(){}
#pragma intel optimization_level -1 // expected-warning {{expected an integer constant}}
#pragma intel optimization_level 10 // expected-warning {{optimization level must be between 0 and 3}}
#pragma intel optimization_level reset // expected-warning {{expected an integer constant}}



#pragma optimization_level 0
// CHECK: define void @{{.*}}f6
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f6(){}
#pragma optimization_level 3
// CHECK: define void @{{.*}}f7
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 3)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f7(){}
#pragma intel optimization_level 1
// CHECK: define void @{{.*}}f8
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"INTEL_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 1)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f8(){}
#pragma intel optimization_level 2
// CHECK: define void @{{.*}}f9
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"INTEL_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 2)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f9(){}
#pragma GCC optimization_level 0
// CHECK: define void @{{.*}}f10
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f10(){}
#pragma GCC optimization_level 3
// CHECK: define void @{{.*}}f11
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 3)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f11(){}
#pragma GCC optimization_level 1
// CHECK: define void @{{.*}}f12
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 1)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f12(){}
#pragma GCC optimization_level reset
// CHECK: define void @{{.*}}f13
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"RESET")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f13(){}
// CHECK: define void @{{.*}}f14
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"RESET")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
void f14(){}
// CHECK: define i32 @main()
int main() {
  int i = 13, j;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"GCC_OPTIMIZATION_LEVEL", metadata !"CONSTANT", metadata i32 3)
struct S {
#pragma GCC optimization_level 3
  int a;
};

  return (0);
}
// CHECK: }

