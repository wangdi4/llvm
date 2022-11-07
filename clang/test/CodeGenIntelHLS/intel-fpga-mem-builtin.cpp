//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -emit-llvm -opaque-pointers -o - %s | FileCheck %s

#define PARAM_1 1U << 7
#define PARAM_2 1U << 8

struct State {
  int x;
  float y;
};

// CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{params:384}{cache-size:0}
// CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{params:384}{cache-size:127}

// CHECK: define{{.*}}void @{{.*}}(ptr noundef %A, ptr noundef %B, ptr noundef %C, ptr noundef nonnull align 4 dereferenceable(8) %D)
void foo(float *A, int *B, State *C, State &D) {
  float *x;
  int *y;
  State *z;
  double F = 0.0;
  double *f;

  // CHECK: [[Aaddr:%.*]] = alloca ptr
  // CHECK: [[Baddr:%.*]] = alloca ptr
  // CHECK: [[Caddr:%.*]] = alloca ptr
  // CHECK: [[Daddr:%.*]] = alloca ptr
  // CHECK: [[F:%.*]] = alloca double
  // CHECK: [[f:%.*]] = alloca ptr 

  // CHECK: [[A:%[0-9]+]] = load ptr, ptr [[Aaddr]]
  // CHECK: [[PTR1:%[0-9]+]] = call ptr @llvm.ptr.annotation{{.*}}[[A]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr [[PTR1]], ptr %x
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 0);

  // CHECK: [[B:%[0-9]+]] = load ptr, ptr [[Baddr]]
  // CHECK: [[PTR2:%[0-9]+]] = call ptr @llvm.ptr.annotation{{.*}}[[B]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr [[PTR2]], ptr %y
  y = __builtin_intel_fpga_mem(B, PARAM_1 | PARAM_2, 0);

  // CHECK: [[C:%[0-9]+]] = load ptr, ptr [[Caddr]]
  // CHECK: [[PTR3:%[0-9]+]] = call ptr @llvm.ptr.annotation{{.*}}[[C]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr [[PTR3]], ptr %z
  z = __builtin_intel_fpga_mem(C, PARAM_1 | PARAM_2, 0);

  // CHECK: [[A2:%[0-9]+]] = load ptr, ptr [[Aaddr]]
  // CHECK: [[PTR4:%[0-9]+]] = call ptr @llvm.ptr.annotation{{.*}}[[A2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr [[PTR4]], ptr %x
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 127);

  // CHECK: [[B2:%[0-9]+]] = load ptr, ptr [[Baddr]]
  // CHECK: [[PTR5:%[0-9]+]] = call ptr @llvm.ptr.annotation{{.*}}[[B2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr [[PTR5]], ptr %y
  y = __builtin_intel_fpga_mem(B, PARAM_1 | PARAM_2, 127);

  // CHECK: [[C2:%[0-9]+]] = load ptr, ptr [[Caddr]]
  // CHECK: [[PTR6:%[0-9]+]] = call ptr @llvm.ptr.annotation{{.*}}[[C2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr [[PTR6]], ptr %z
  z = __builtin_intel_fpga_mem(C, PARAM_1 | PARAM_2, 127);

  // CHECK: [[D:%[0-9]+]] = load ptr, ptr [[Daddr]]
  // CHECK: [[PTR7:%[0-9]+]] = call ptr @llvm.ptr.annotation{{.*}}[[D]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr [[PTR7]], ptr %z
  z = __builtin_intel_fpga_mem(&D, PARAM_1 | PARAM_2, 127);

  // CHECK: [[PTR8:%[0-9]+]] = call ptr @llvm.ptr.annotation{{.*}}[[F]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store ptr [[PTR8]], ptr [[f]]
  f = __builtin_intel_fpga_mem(&F, PARAM_1 | PARAM_2, 127);
}

// CHECK: attributes [[ATT]] = { memory(none) }
