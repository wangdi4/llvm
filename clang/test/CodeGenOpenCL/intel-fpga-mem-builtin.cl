// RUN: %clang_cc1 %s -O0 -triple spir-unknown-unknown-intelfpga -emit-llvm -opaque-pointers -o - | FileCheck %s

#define PARAM_1 1U << 7
#define PARAM_2 1U << 8

struct State {
  int x;
  float y;
};

// CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{params:384}{cache-size:0}
// CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{params:384}{cache-size:127}

// CHECK: define{{.*}}spir_func void @foo(ptr addrspace(1) noundef %A, ptr addrspace(1) noundef %B, ptr addrspace(1) noundef %C, ptr addrspace(1) noundef %D)
void foo(global float *A, global int *B, global struct State *C, global long long *D) {
  global float *x;
  global int *y;
  global struct State *z;
  global long long *F = D;
  global long long *f;

  // CHECK: [[Aaddr:%.*]] = alloca ptr addrspace(1)
  // CHECK: [[Baddr:%.*]] = alloca ptr addrspace(1)
  // CHECK: [[Caddr:%.*]] = alloca ptr addrspace(1)
  // CHECK: [[Daddr:%.*]] = alloca ptr addrspace(1)
  // CHECK: %x = alloca ptr addrspace(1)
  // CHECK: %y = alloca ptr addrspace(1)
  // CHECK: %z = alloca ptr addrspace(1)
  // CHECK: [[F:%.*]] = alloca ptr addrspace(1)
  // CHECK: [[f:%.*]] = alloca ptr addrspace(1)

  // CHECK: [[A:%[0-9]+]] = load ptr addrspace(1), ptr [[Aaddr]]
  // CHECK: [[PTR1:%[0-9]+]] = call ptr addrspace(1) @llvm.ptr.annotation{{.*}}[[A]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr addrspace(1) [[PTR1]], ptr %x
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 0);

  // CHECK: [[B:%[0-9]+]] = load ptr addrspace(1), ptr [[Baddr]]
  // CHECK: [[PTR2:%[0-9]+]] = call ptr addrspace(1) @llvm.ptr.annotation{{.*}}[[B]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr addrspace(1) [[PTR2]], ptr %y
  y = __builtin_intel_fpga_mem(B, PARAM_1 | PARAM_2, 0);

  // CHECK: [[C:%[0-9]+]] = load ptr addrspace(1), ptr [[Caddr]]
  // CHECK: [[PTR3:%[0-9]+]] = call ptr addrspace(1) @llvm.ptr.annotation{{.*}}[[C]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr addrspace(1) [[PTR3]], ptr %z
  z = __builtin_intel_fpga_mem(C, PARAM_1 | PARAM_2, 0);

  // CHECK: [[A2:%[0-9]+]] = load ptr addrspace(1), ptr [[Aaddr]]
  // CHECK: [[PTR4:%[0-9]+]] = call ptr addrspace(1) @llvm.ptr.annotation{{.*}}[[A2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr addrspace(1) [[PTR4]], ptr %x
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 127);

  // CHECK: [[B2:%[0-9]+]] = load ptr addrspace(1), ptr [[Baddr]]
  // CHECK: [[PTR5:%[0-9]+]] = call ptr addrspace(1) @llvm.ptr.annotation{{.*}}[[B2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr addrspace(1) [[PTR5]], ptr %y
  y = __builtin_intel_fpga_mem(B, PARAM_1 | PARAM_2, 127);

  // CHECK: [[C2:%[0-9]+]] = load ptr addrspace(1), ptr [[Caddr]]
  // CHECK: [[PTR6:%[0-9]+]] = call ptr addrspace(1) @llvm.ptr.annotation{{.*}}[[C2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr addrspace(1) [[PTR6]], ptr %z
  z = __builtin_intel_fpga_mem(C, PARAM_1 | PARAM_2, 127);

  // CHECK: [[F2:%[0-9]+]] = load ptr addrspace(1), ptr [[F]]
  // CHECK: [[PTR7:%[0-9]+]] = call ptr addrspace(1) @llvm.ptr.annotation{{.*}}[[F2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr addrspace(1) [[PTR7]], ptr [[f]]
  f = __builtin_intel_fpga_mem(F, PARAM_1 | PARAM_2, 127);
}

// CHECK: attributes [[ATT]] = { memory(none) }
