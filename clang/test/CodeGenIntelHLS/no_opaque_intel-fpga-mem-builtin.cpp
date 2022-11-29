//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

#define PARAM_1 1U << 7
#define PARAM_2 1U << 8

// CHECK: [[STRUCT:%.*]] = type { i32, float }
struct State {
  int x;
  float y;
};

// CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{params:384}{cache-size:0}
// CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{params:384}{cache-size:127}

// CHECK: define{{.*}}void @{{.*}}(float* noundef %A, i32* noundef %B, [[STRUCT]]* noundef %C, [[STRUCT]]* noundef nonnull align 4 dereferenceable(8) %D)
void foo(float *A, int *B, State *C, State &D) {
  float *x;
  int *y;
  State *z;
  double F = 0.0;
  double *f;

  // CHECK: [[Aaddr:%.*]] = alloca float*
  // CHECK: [[Baddr:%.*]] = alloca i32*
  // CHECK: [[Caddr:%.*]] = alloca [[STRUCT]]*
  // CHECK: [[Daddr:%.*]] = alloca [[STRUCT]]*
  // CHECK: [[F:%.*]] = alloca double
  // CHECK: [[f:%.*]] = alloca double*

  // CHECK: [[A:%[0-9]+]] = load float*, float** [[Aaddr]]
  // CHECK: [[PTR1:%[0-9]+]] = call float* @llvm.ptr.annotation{{.*}}[[A]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store float* [[PTR1]], float** %x
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 0);

  // CHECK: [[B:%[0-9]+]] = load i32*, i32** [[Baddr]]
  // CHECK: [[PTR2:%[0-9]+]] = call i32* @llvm.ptr.annotation{{.*}}[[B]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store i32* [[PTR2]], i32** %y
  y = __builtin_intel_fpga_mem(B, PARAM_1 | PARAM_2, 0);

  // CHECK: [[C:%[0-9]+]] = load [[STRUCT]]*, [[STRUCT]]** [[Caddr]]
  // CHECK: [[PTR3:%[0-9]+]] = call [[STRUCT]]* @llvm.ptr.annotation{{.*}}[[C]]{{.*}}[[ANN1]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store [[STRUCT]]* [[PTR3]], [[STRUCT]]** %z
  z = __builtin_intel_fpga_mem(C, PARAM_1 | PARAM_2, 0);

  // CHECK: [[A2:%[0-9]+]] = load float*, float** [[Aaddr]]
  // CHECK: [[PTR4:%[0-9]+]] = call float* @llvm.ptr.annotation{{.*}}[[A2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store float* [[PTR4]], float** %x
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 127);

  // CHECK: [[B2:%[0-9]+]] = load i32*, i32** [[Baddr]]
  // CHECK: [[PTR5:%[0-9]+]] = call i32* @llvm.ptr.annotation{{.*}}[[B2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store i32* [[PTR5]], i32** %y
  y = __builtin_intel_fpga_mem(B, PARAM_1 | PARAM_2, 127);

  // CHECK: [[C2:%[0-9]+]] = load [[STRUCT]]*, [[STRUCT]]** [[Caddr]]
  // CHECK: [[PTR6:%[0-9]+]] = call [[STRUCT]]* @llvm.ptr.annotation{{.*}}[[C2]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store [[STRUCT]]* [[PTR6]], [[STRUCT]]** %z
  z = __builtin_intel_fpga_mem(C, PARAM_1 | PARAM_2, 127);

  // CHECK: [[D:%[0-9]+]] = load [[STRUCT]]*, [[STRUCT]]** [[Daddr]]
  // CHECK: [[PTR7:%[0-9]+]] = call [[STRUCT]]* @llvm.ptr.annotation{{.*}}[[D]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store [[STRUCT]]* [[PTR7]], [[STRUCT]]** %z
  z = __builtin_intel_fpga_mem(&D, PARAM_1 | PARAM_2, 127);

  // CHECK: [[PTR8:%[0-9]+]] = call double* @llvm.ptr.annotation{{.*}}[[F]]{{.*}}[[ANN2]]{{.*}}[[ATT:#[0-9]+]]
  // CHECK: store double* [[PTR8]], double** [[f]]
  f = __builtin_intel_fpga_mem(&F, PARAM_1 | PARAM_2, 127);
}

// CHECK: attributes [[ATT]] = { memory(none) }
