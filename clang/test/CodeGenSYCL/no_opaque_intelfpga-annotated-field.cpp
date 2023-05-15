// RUN: %clangxx %s -fsycl-device-only -fintelfpga -S -o %t.ll
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
// CHECK-NEXT: %_arg_p.addr = alloca i32 addrspace(1)*, align 8
// CHECK-NEXT: %fooA = alloca %struct.fooA, align 8
// CHECK-NEXT: %fooA.ascast = addrspacecast %struct.fooA* %fooA to %struct.fooA addrspace(4)*
// CHECK-NEXT: store i32 addrspace(1)* %_arg_p, i32 addrspace(1)** %_arg_p.addr, align 8, !tbaa !47
// CHECK: %p = getelementptr inbounds %struct.fooA, %struct.fooA addrspace(4)* %fooA.ascast, i32 0, i32 0
// CHECK-NEXT: %1 = bitcast i32 addrspace(4)* addrspace(4)* %p to i8 addrspace(4)*
// CHECK-NEXT: %2 = call i8 addrspace(4)* @llvm.ptr.annotation.p4i8.p1i8(i8 addrspace(4)* %1, i8 addrspace(1)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(1)* @[[STR1]], i32 0, i32 0)
// CHECK-NEXT: %3 = bitcast i8 addrspace(4)* %2 to i32 addrspace(4)* addrspace(4)*
// CHECK-NEXT: %4 = load i32 addrspace(1)*, i32 addrspace(1)** %_arg_p.addr, align 8
// CHECK-NEXT: %5 = addrspacecast i32 addrspace(1)* %4 to i32 addrspace(4)*
// CHECK-NEXT: store i32 addrspace(4)* %5, i32 addrspace(4)* addrspace(4)* %3, align 8
// CHECK-NEXT: call spir_func void @_ZNK4fooAclEv(%struct.fooA addrspace(4)* noundef align 8 dereferenceable_or_null(8) %fooA.ascast)
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
// CHECK-NEXT: %fooB = alloca %struct.fooB, align 4
// CHECK-NEXT: %fooB.ascast = addrspacecast %struct.fooB* %fooB to %struct.fooB addrspace(4)*
// CHECK-NEXT: store float %_arg_f, float* %_arg_f.addr, align 4, !tbaa !83
// CHECK: %f = getelementptr inbounds %struct.fooB, %struct.fooB addrspace(4)* %fooB.ascast, i32 0, i32 0
// CHECK-NEXT: %1 = bitcast float addrspace(4)* %f to i8 addrspace(4)*
// CHECK-NEXT: %2 = call i8 addrspace(4)* @llvm.ptr.annotation.p4i8.p1i8(i8 addrspace(4)* %1, i8 addrspace(1)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(1)* @[[STR2]], i32 0, i32 0)
// CHECK-NEXT: %3 = bitcast i8 addrspace(4)* %2 to float addrspace(4)*
// CHECK-NEXT: %4 = load float, float* %_arg_f.addr, align 4
// CHECK-NEXT: store float %4, float addrspace(4)* %3, align 4
// CHECK-NEXT: call spir_func void @_ZNK4fooBclEv(%struct.fooB addrspace(4)* noundef align 4 dereferenceable_or_null(4) %fooB.ascast)
// CHECK: call void @__itt_offload_wi_finish_wrapper()
struct fooB {
    [[clang::annotate("my_ann_2")]]
    float f;

    fooB(float _f) : f(_f) {}
    void operator()() const {}
};


// CHECK: define dso_local spir_kernel void @_ZTS4fooC
// CHECK: call void @__itt_offload_wi_start_wrapper()
// CHECK-NEXT: %fooC = alloca %struct.fooC, align 1
// CHECK-NEXT: %fooC.ascast = addrspacecast %struct.fooC* %fooC to %struct.fooC addrspace(4)*
// CHECK: %b = getelementptr inbounds %struct.fooC, %struct.fooC addrspace(4)* %fooC.ascast, i32 0, i32 0
// CHECK-NEXT: %1 = bitcast %struct.bar addrspace(4)* %b to i8 addrspace(4)*
// CHECK-NEXT: %2 = call i8 addrspace(4)* @llvm.ptr.annotation.p4i8.p1i8(i8 addrspace(4)* %1, i8 addrspace(1)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(1)* @[[STR3]], i32 0, i32 0)
// CHECK-NEXT: %3 = bitcast i8 addrspace(4)* %2 to %struct.bar addrspace(4)*
// CHECK-NEXT: %4 = bitcast %struct.bar addrspace(4)* %3 to i8 addrspace(4)*
// CHECK-NEXT: %5 = bitcast %struct.bar* %_arg_b to i8*
// CHECK-NEXT: call void @llvm.memcpy.p4i8.p0i8.i64(i8 addrspace(4)* align 1 %4, i8* align 1 %5, i64 1, i1 false)
// CHECK-NEXT: call spir_func void @_ZNK4fooCclEv(%struct.fooC addrspace(4)* noundef align 1 dereferenceable_or_null(1) %fooC.ascast)
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
