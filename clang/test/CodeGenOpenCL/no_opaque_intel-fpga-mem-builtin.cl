// RUN: %clang_cc1 %s -O0 -triple spir-unknown-unknown-intelfpga -emit-llvm -no-opaque-pointers -o - | FileCheck %s

#define PARAM_1 1U << 7
#define PARAM_2 1U << 8

// CHECK: [[STRUCT:%.*]] = type { i32, float }
struct State {
  int x;
  float y;
};

// CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{params:384}{cache-size:0}
// CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{params:384}{cache-size:127}

// CHECK: define{{.*}}spir_func void @foo(float addrspace(1)* noundef %A, i32 addrspace(1)* noundef %B, %struct.State addrspace(1)* noundef %C, i128 addrspace(1)* noundef %D)
void foo(global float *A, global int *B, global struct State *C, global long long *D) {
  global float *x;
  global int *y;
  global struct State *z;
  global long long *F = D;
  global long long *f;

  // CHECK: [[Aaddr:%.*]] = alloca float addrspace(1)*
  // CHECK: [[Baddr:%.*]] = alloca i32 addrspace(1)*
  // CHECK: [[Caddr:%.*]] = alloca [[STRUCT]] addrspace(1)*
  // CHECK: [[Daddr:%.*]] = alloca float addrspace(1)*
  // CHECK: [[F:%.*]] = alloca i128 addrspace(1)*
  // CHECK: [[f:%.*]] = alloca i128 addrspace(1)*

  // CHECK: [[A:%[0-9]+]] = load float addrspace(1)*, float addrspace(1)** [[Aaddr]]
  // CHECK: [[PTR1:%[0-9]+]] = call float addrspace(1)* @llvm.ptr.annotation{{.*}}[[A]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store float addrspace(1)* [[PTR1]], float addrspace(1)** %x
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 0);

  // CHECK: [[B:%[0-9]+]] = load i32 addrspace(1)*, i32 addrspace(1)** [[Baddr]]
  // CHECK: [[PTR2:%[0-9]+]] = call i32 addrspace(1)* @llvm.ptr.annotation{{.*}}[[B]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store i32 addrspace(1)* [[PTR2]], i32 addrspace(1)** %y
  y = __builtin_intel_fpga_mem(B, PARAM_1 | PARAM_2, 0);

  // CHECK: [[C:%[0-9]+]] = load [[STRUCT]] addrspace(1)*, [[STRUCT]] addrspace(1)** [[Caddr]]
  // CHECK: [[PTR3:%[0-9]+]] = call [[STRUCT]] addrspace(1)* @llvm.ptr.annotation{{.*}}[[C]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store [[STRUCT]] addrspace(1)* [[PTR3]], [[STRUCT]] addrspace(1)** %z
  z = __builtin_intel_fpga_mem(C, PARAM_1 | PARAM_2, 0);

  // CHECK: [[A2:%[0-9]+]] = load float addrspace(1)*, float addrspace(1)** [[Aaddr]]
  // CHECK: [[PTR4:%[0-9]+]] = call float addrspace(1)* @llvm.ptr.annotation{{.*}}[[A2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store float addrspace(1)* [[PTR4]], float addrspace(1)** %x
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 127);

  // CHECK: [[B2:%[0-9]+]] = load i32 addrspace(1)*, i32 addrspace(1)** [[Baddr]]
  // CHECK: [[PTR5:%[0-9]+]] = call i32 addrspace(1)* @llvm.ptr.annotation{{.*}}[[B2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store i32 addrspace(1)* [[PTR5]], i32 addrspace(1)** %y
  y = __builtin_intel_fpga_mem(B, PARAM_1 | PARAM_2, 127);

  // CHECK: [[C2:%[0-9]+]] = load [[STRUCT]] addrspace(1)*, [[STRUCT]] addrspace(1)** [[Caddr]]
  // CHECK: [[PTR6:%[0-9]+]] = call [[STRUCT]] addrspace(1)* @llvm.ptr.annotation{{.*}}[[C2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store [[STRUCT]] addrspace(1)* [[PTR6]], [[STRUCT]] addrspace(1)** %z
  z = __builtin_intel_fpga_mem(C, PARAM_1 | PARAM_2, 127);

  // CHECK: [[F2:%[0-9]+]] = load i128 addrspace(1)*, i128 addrspace(1)** [[F]]
  // CHECK: [[PTR7:%[0-9]+]] = call i128 addrspace(1)* @llvm.ptr.annotation{{.*}}[[F2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store i128 addrspace(1)* [[PTR7]], i128 addrspace(1)** [[f]]
  f = __builtin_intel_fpga_mem(F, PARAM_1 | PARAM_2, 127);
}

// CHECK: attributes [[ATT]] = { memory(none) }
