// RUN: %clangxx %s -Xclang -opaque-pointers -fsycl-device-only -fintelfpga -S -o %t.ll
// RUN: FileCheck %s --input-file %t.ll

// This test checks that clang emits @llvm.ptr.annotation intrinsic correctly
// when attribute annotate is applied to a kernel functor field.
// It also ensures the kernel lambda is not inlined for FPGA.

#include "Inputs/sycl.hpp"

// CHECK: @[[STR1:.*]] = private unnamed_addr addrspace(1) constant [9 x i8] c"my_ann_1\00", section "llvm.metadata"
// CHECK: @[[STR2:.*]] = private unnamed_addr addrspace(1) constant [9 x i8] c"my_ann_2\00", section "llvm.metadata"
// CHECK: @[[STR3:.*]] = private unnamed_addr addrspace(1) constant [9 x i8] c"my_ann_3\00", section "llvm.metadata"


// CHECK: define dso_local spir_kernel void @_ZTS4fooA
// CHECK: call void @__itt_offload_wi_start_wrapper()
// CHECK-NEXT: %_arg_p.addr = alloca ptr addrspace(1), align 8
// CHECK: store ptr addrspace(1) %_arg_p, ptr %_arg_p.addr, align 8
// CHECK: %0 = call ptr addrspace(4) @llvm.ptr.annotation.p4.p1({{.*}}, ptr addrspace(1) @[[STR1]]
// CHECK-NEXT: %1 = load ptr addrspace(1), ptr %_arg_p.addr, align 8
// CHECK-NEXT: %2 = addrspacecast ptr addrspace(1) %1 to ptr addrspace(4)
// CHECK-NEXT: store ptr addrspace(4) %2, ptr addrspace(4) %0, align 8
// CHECK-NEXT: call spir_func void @_ZNK4fooAclEv(
// CHECK: call void @__itt_offload_wi_finish_wrapper()
struct fooA {
    [[clang::annotate("my_ann_1")]]
    int *p;

    fooA(int *_p) : p(_p) {}
    void operator()() const {}
};


// CHECK: define dso_local spir_kernel void @_ZTS4fooB
// CHECK: call void @__itt_offload_wi_start_wrapper()
// CHECK-NEXT: %_arg_f.addr = alloca float, align 4
// CHECK: store float %_arg_f, ptr %_arg_f.addr, align 4
// CHECK: %0 = call ptr addrspace(4) @llvm.ptr.annotation.p4.p1({{.*}}, ptr addrspace(1) @[[STR2]]
// CHECK-NEXT: %1 = load float, ptr %_arg_f.addr, align 4
// CHECK-NEXT: store float %1, ptr addrspace(4) %0, align 4
// CHECK-NEXT: call spir_func void @_ZNK4fooBclEv(
// CHECK: call void @__itt_offload_wi_finish_wrapper()
struct fooB {
    [[clang::annotate("my_ann_2")]]
    float f;

    fooB(float _f) : f(_f) {}
    void operator()() const {}
};


// CHECK: define dso_local spir_kernel void @_ZTS4fooC
// CHECK: call void @__itt_offload_wi_start_wrapper()
// CHECK: %0 = call ptr addrspace(4) @llvm.ptr.annotation.p4.p1({{.*}}, ptr addrspace(1) @[[STR3]]
// CHECK-NEXT: call void @llvm.memcpy.p4.p0.i64(ptr addrspace(4) align 1 %0, ptr align 1 %_arg_b, i64 1, i1 false)
// CHECK-NEXT: call spir_func void @_ZNK4fooCclEv(
// CHECK: call void @__itt_offload_wi_finish_wrapper()

struct bar {
  _BitInt(5) a;
};

struct fooC {
    [[clang::annotate("my_ann_3")]]
    bar b;

    fooC(bar _b) : b(_b) {}
    void operator()() const {}
};

int main() {
  sycl::handler h;
  h.single_task(fooA{nullptr});
  h.single_task(fooB{2.0});
  h.single_task(fooC{{3}});
  return 0;
}
