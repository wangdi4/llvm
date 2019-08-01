// RUN: %clang_cc1 -cl-std=CL1.2 -emit-llvm -o - %s | FileCheck %s

#if defined(cl_intel_vec_len_hint)
#pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable
#endif

#if defined(cl_intel_vec_len_hint)
__attribute__((intel_vec_len_hint(0)))
#endif
kernel void kernel0(int a) {}
// CHECK: define {{.*}} void @kernel0(i32 {{[^%]*}}%a) {{[^{]+}} !intel_vec_len_hint ![[MD0:[0-9]+]]

#if defined(cl_intel_vec_len_hint)
__attribute__((intel_vec_len_hint(1)))
#endif
kernel void kernel1(int a) {}
// CHECK: define {{.*}} void @kernel1(i32 {{[^%]*}}%a) {{[^{]+}} !intel_vec_len_hint ![[MD1:[0-9]+]]

#if defined(cl_intel_vec_len_hint)
__attribute__((intel_vec_len_hint(4)))
#endif
kernel void kernel4(int a) {}
// CHECK: define {{.*}} void @kernel4(i32 {{[^%]*}}%a) {{[^{]+}} !intel_vec_len_hint ![[MD4:[0-9]+]]

#if defined(cl_intel_vec_len_hint)
__attribute__((intel_vec_len_hint(8)))
#endif
kernel void kernel8(int a) {}
// CHECK: define {{.*}} void @kernel8(i32 {{[^%]*}}%a) {{[^{]+}} !intel_vec_len_hint ![[MD8:[0-9]+]]

// CHECK: [[MD0]] = !{i32 0}
// CHECK: [[MD1]] = !{i32 1}
// CHECK: [[MD4]] = !{i32 4}
// CHECK: [[MD8]] = !{i32 8}
