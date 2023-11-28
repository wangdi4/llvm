// RUN: %clang_cc1 -triple spir64 -fsycl-is-device -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s

// CHECK: [[ANNOT:.+]] = private unnamed_addr addrspace(1) constant {{.*}}c"my_annotation\00"

struct HasField {
  // This caused an assertion on creating a bitcast here,
  // since the address space didn't match.
  [[clang::annotate("my_annotation")]]
  int *a;
};

// INTEL_COLLAB
struct HasFieldWithArgs {
  // This caused an assertion on creating a bitcast here,
  // since the address space didn't match.
  [[clang::annotate("annotation-with-args", 4)]]
  int *a;
};
// end INTEL_COLLAB

__attribute__((sycl_device)) void foo(int *b) {
  struct HasField f;
  // CHECK: %[[A:.+]] = getelementptr inbounds %struct.HasField, ptr addrspace(4) %{{.+}}
  // CHECK: %[[CALL:.+]] = call ptr addrspace(4) @llvm.ptr.annotation.p4.p1(ptr addrspace(4) %[[A]], ptr addrspace(1) [[ANNOT]]
  // CHECK: store ptr addrspace(4) %{{[0-9]+}}, ptr addrspace(4) %[[CALL]]
  f.a = b;
  // INTEL_COLLAB
  struct HasFieldWithArgs fArgs;
  // CHECK: %[[A:.+]] = getelementptr inbounds %struct.HasFieldWithArgs, ptr addrspace(4) %fArgs.ascast, i32 0, i32 0
  // CHECK: %[[CALL:.+]] = call ptr addrspace(4) @llvm.ptr.annotation.p4.p1
  // CHECK-SAME: (ptr addrspace(4) %[[A]], ptr addrspace(1) @.str.2, ptr addrspace(1) @.str.1, i32 17, ptr addrspace(1) @.args)
  fArgs.a = b;
  // end INTEL_COLLAB
}
