// RUN: %clang_cc1 %s -o - -O0 -emit-llvm -no-opaque-pointers -cl-std=CL2.0 -gno-column-info      \
// RUN:            -debug-info-kind=limited -triple spir-unknown-unknown      \
// RUN:            -finclude-default-header                                   \
// RUN:   | FileCheck %s
//
// Verify valid source correlation is emitted for a block literal passed to
// enqueue_kernel.
//

kernel void callee(global int* a)
{
  size_t id = get_global_id(0);
  if (id % 2)
    a[id] = 42;
}

kernel void caller(global int* a)
{
  size_t id = get_global_id(0);
  a[id] = id;
  int ret = enqueue_kernel(
              get_default_queue(),
              CLK_ENQUEUE_FLAGS_WAIT_KERNEL,
              ndrange_1D(10),
              ^{callee(a);});
}

// CHECK: define{{.*}}spir_kernel {{.*}}@caller({{.*}}){{.*}}!dbg [[CALLER:![0-9]+]]{{.*}} {
// CHECK:   load i32 addrspace(1)*, i32 addrspace(1)** %a.addr{{.*}}!dbg [[LINE20:![0-9]+]]
// CHECK:   load i32 addrspace(1)*, i32 addrspace(1)** %a.addr{{.*}}!dbg [[LINE25:![0-9]+]]
// CHECK:   store i32 addrspace(1)* %{{[0-9]+}}, i32 addrspace(1)** %block.captured{{.*}}!dbg [[LINE25]]
// CHECK:   call spir_func i32 @__enqueue_kernel_basic({{.*}}), !dbg [[LINE21:![0-9]+]]
// CHECK: }

// CHECK-NOT: !DILocation(line: 0
// CHECK: [[CALLER]] = distinct !DISubprogram(name: "caller"
// CHECK-SAME: line: 17
// CHECK-NOT: !DILocation(line: 0
// CHECK: [[LINE20]] = !DILocation(line: 20, scope: [[CALLER]])
// CHECK-NOT: !DILocation(line: 0
// CHECK: [[LINE21]] = !DILocation(line: 21, scope: [[CALLER]])
// CHECK-NOT: !DILocation(line: 0
// CHECK: [[LINE25]] = !DILocation(line: 25, scope: [[CALLER]])
