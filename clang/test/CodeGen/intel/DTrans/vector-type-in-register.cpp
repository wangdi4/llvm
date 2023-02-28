// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=LIN64,LINUX,CHECK
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple i386-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=LIN32,LINUX,CHECK
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=WIN64,CHECK
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple i386-windows-mvsc -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=WIN32,CHECK

typedef float __m256 __attribute__ ((__vector_size__ (32)));
__m256 _mm256_unpacklo_ps(__m256 __a, __m256 __b);

__m256 HorizontalSums(__m256& v0, __m256& v1, __m256& v2, __m256& v3) {
  return v0;
}

void forward()
{
  _mm256_unpacklo_ps(__m256{}, __m256{});
  __m256 a;
  a = HorizontalSums(a, a, a, a);
}

// HorizontalSums, all platforms are the same other than mangled name.
// CHECK: define{{.*}} <8 x float> 
// LINUX-SAME: @_Z14HorizontalSumsRDv8_fS0_S0_S0_
// WIN64-SAME: @"?HorizontalSums@@YA?AT__m256@@AEAT1@000@Z"
// WIN32-SAME: @"?HorizontalSums@@YA?AT__m256@@AAT1@000@Z"
// CHECK-SAME:(ptr {{.*}}"intel_dtrans_func_index"="1" %{{.*}}, ptr {{.*}}"intel_dtrans_func_index"="2" %{{.*}}, ptr {{.*}}intel_dtrans_func_index"="3" %{{.*}}, ptr {{.*}}"intel_dtrans_func_index"="4" %{{.*}}){{.*}} !intel.dtrans.func.type ![[HORIZ_SUM:[0-9]+]]


// unpacklo_ps
// LIN64: declare !intel.dtrans.func.type ![[FUNC_INFO:[0-9]+]] noundef <8 x float> @_Z18_mm256_unpacklo_psDv8_fS_(ptr noundef byval(<8 x float>) align 32 "intel_dtrans_func_index"="1", ptr noundef byval(<8 x float>) align 32 "intel_dtrans_func_index"="2")

// The following don't wrap the vector type in a pointer, so they don't get metadata. Tested for completeness.
// LIN32: declare noundef <8 x float> @_Z18_mm256_unpacklo_psDv8_fS_(<8 x float> noundef, <8 x float> noundef)
// WIN64: declare dso_local noundef <8 x float> @"?_mm256_unpacklo_ps@@YA?AT__m256@@T1@0@Z"(<8 x float> noundef, <8 x float> noundef)
// WIN32: declare dso_local noundef <8 x float> @"?_mm256_unpacklo_ps@@YA?AT__m256@@T1@0@Z"(<8 x float> inreg noundef, <8 x float> inreg noundef)

// Metadata
// CHECK: ![[HORIZ_SUM]] = distinct !{![[PTR_TO_VEC:[0-9]+]], ![[PTR_TO_VEC]], ![[PTR_TO_VEC]], ![[PTR_TO_VEC]]}
// CHECK: ![[PTR_TO_VEC]] = !{![[VEC:[0-9]+]], i32 1}
// CHECK: ![[VEC]] = !{!"V", i32 8, ![[FLOAT:[0-9]+]]}
// LIN64: ![[FLOAT]] = !{float 0.0{{.*}}, i32 0}
// LIN64: ![[FUNC_INFO]] = distinct !{![[PTR_TO_VEC]], ![[PTR_TO_VEC]]}
