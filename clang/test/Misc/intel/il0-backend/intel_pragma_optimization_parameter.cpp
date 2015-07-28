// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma intel optimization_parameter target_arch=
// expected-no-diagnostics

#pragma intel optimization_parameter target_arch=ATOM
struct AAAAAA{                  
  int a;
};
// CHECK: atom1{{.*}} {
void atom1() {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPT_PARAM_TARGET_ARCH", metadata !"ATOM")
}
// CHECK: }

// CHECK: atom2{{.*}} {
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
void atom2() {}
// CHECK: }

#pragma intel optimization_parameter target_arch=10
struct AAAAAA1{               
#pragma intel optimization_parameter target_arch=AVX
  int a;
};
// CHECK: avx1{{.*}} {
void avx1() {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPT_PARAM_TARGET_ARCH", metadata !"AVX")
}
// CHECK: }

// CHECK: avx2{{.*}} {
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
void avx2() {}
// CHECK: }

#pragma intel optimization_parameter 
#pragma intel optimization_parameter target_ar
#pragma intel optimization_parameter wfrrhff (10)+
// CHECK: aaaa{{.*}} {
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
void aaaa() {
  ;
}
// CHECK: }

#pragma intel optimization_parameter target_arch X86

// CHECK: main{{.*}} {
int main() {
  int i = 13, j;
struct S {
  int a;
};
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPT_PARAM_TARGET_ARCH", metadata !"X86")

  return (0);
}
// CHECK: }

// CHECK: x86{{.*}} {
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
void x86() {}
// CHECK: }

struct S {
#pragma intel optimization_parameter target_arch=SSE2
  int a;
  void sse21(){
  }
  void sse22();
};
// CHECK: sse22{{.*}} {
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
void S::sse22(){sse21();}
// CHECK: }
// CHECK: sse21{{.*}} {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"OPT_PARAM_TARGET_ARCH", metadata !"SSE2")
// CHECK: }

static int a;

