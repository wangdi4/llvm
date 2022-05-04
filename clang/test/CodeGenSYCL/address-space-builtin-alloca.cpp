<<<<<<< HEAD
// RUN: %clang_cc1 -triple spir64-unknown-linux -fsycl-is-device -disable-llvm-passes -emit-llvm -x c++ %s -o - | FileCheck %s
=======
// RUN: %clang_cc1 -triple spir64-unknown-linux -fsycl-is-device -disable-llvm-passes -opaque-pointers -emit-llvm -x c++ %s -o - | FileCheck %s
>>>>>>> 2534c361724a92e3e3a34ffbbe9eb34294fa7b04

// Test to verify that address space cast is generated correctly for __builtin_alloca

__attribute__((sycl_device)) void foo() {
<<<<<<< HEAD
  // CHECK: %TestVar = alloca i32 addrspace(4)*, align 8
  // CHECK: %TestVar.ascast = addrspacecast i32 addrspace(4)** %TestVar to i32 addrspace(4)* addrspace(4)*
  // CHECK: %[[ALLOCA:[0-9]+]] = alloca i8, i64 1, align 8
  // CHECK: %[[ADDRSPCAST:[0-9]+]] = addrspacecast i8* %[[ALLOCA]] to i8* addrspace(4)*
  // CHECK: %[[BITCAST:[0-9]+]] = bitcast i8* addrspace(4)* %[[ADDRSPCAST]] to i32 addrspace(4)*
  // CHECK: store i32 addrspace(4)* %[[BITCAST]], i32 addrspace(4)* addrspace(4)* %TestVar.ascast, align 8
=======
  // CHECK: %TestVar = alloca ptr addrspace(4), align 8
  // CHECK: %TestVar.ascast = addrspacecast ptr %TestVar to ptr addrspace(4)
  // CHECK: %[[ALLOCA:[0-9]+]] = alloca i8, i64 1, align 8
  // CHECK: %[[ADDRSPCAST:[0-9]+]] = addrspacecast ptr %[[ALLOCA]] to ptr addrspace(4)
  // CHECK: store ptr addrspace(4) %[[ADDRSPCAST]], ptr addrspace(4) %TestVar.ascast, align 8
>>>>>>> 2534c361724a92e3e3a34ffbbe9eb34294fa7b04
  int *TestVar = (int *)__builtin_alloca(1);
}
