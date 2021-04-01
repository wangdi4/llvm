// RUN: %clang_cc1 -O0 -emit-llvm -o - -std=c++17 -fsycl-is-device \
// RUN: -fenable-variant-function-pointers  -fsycl-explicit-simd \
// RUN:  -triple spir64-unknown-linux-sycldevice %s | FileCheck %s

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

typedef int(*func)(int, int);
typedef int(*fptr)(int);
extern SYCL_EXTERNAL int bar(int, int);
extern SYCL_EXTERNAL int zoo(int);
extern SYCL_EXTERNAL int moo(int);
const func two = &bar;
__attribute__((opencl_private)) fptr one_one;
//CHECK: @"_Z3zooi$SIMDTable" = weak global [1 x i32 (i32)*] [i32 (i32)* @_Z3zooi]
//CHECK: @"_Z3barii$SIMDTable" = weak global [1 x i32 (i32, i32)*] [i32 (i32, i32)* @_Z3barii]
//CHECK: @"_Z3mooi$SIMDTable" = weak global [1 x i32 (i32)*] [i32 (i32)* @_Z3mooi]

extern void test1();
void test(int i);
//CHECK: define {{.*}}_ZZ4mainENK3$_0clEv
int main()
{
  kernel_single_task<class kernel_function>([]() {
   test(10);
//CHECK: store{{.*}}@"_Z3zooi$SIMDTable"
   one_one = &zoo;
//CHECK:call{{.*}}@__intel_indirect_call_0
   one_one(1);
   test1();
  });
}

//CHECK: spir_func void @_Z4testi
void test(int i) {
  fptr fp = &zoo;
  func p = bar;
  fp = &zoo;
  p = bar;
//CHECK: [[FP:%fp]] = alloca i32 (i32)*,
//CHECK: %[[FP_CAST:.+]] = addrspacecast i32 (i32)** [[FP]] to i32 (i32)* addrspace(4)*
//CHECK: [[P:%p]] = alloca i32 (i32, i32)*,
//CHECK: %[[P_CAST:.+]] = addrspacecast i32 (i32, i32)** [[P]] to i32 (i32, i32)* addrspace(4)*
//CHECK: store i32 (i32)* bitcast ([1 x i32 (i32)*]* @"_Z3zooi$SIMDTable" to i32 (i32)*), i32 (i32)* addrspace(4)* %[[FP_CAST]],
//CHECK: store i32 (i32, i32)* bitcast ([1 x i32 (i32, i32)*]* @"_Z3barii$SIMDTable" to i32 (i32, i32)*), i32 (i32, i32)* addrspace(4)* %[[P_CAST]],
//CHECK: store i32 (i32)* bitcast ([1 x i32 (i32)*]* @"_Z3zooi$SIMDTable" to i32 (i32)*), i32 (i32)* addrspace(4)* %[[FP_CAST]],
//CHECK: store i32 (i32, i32)* bitcast ([1 x i32 (i32, i32)*]* @"_Z3barii$SIMDTable" to i32 (i32, i32)*), i32 (i32, i32)* addrspace(4)* %[[P_CAST]],
//CHECK: [[L2:%[0-9]+]] = load i32 (i32, i32)*, i32 (i32, i32)* addrspace(4)* %[[P_CAST]],
//CHECK: [[L3:%[0-9]+]] = addrspacecast i32 (i32, i32)* [[L2]] to i32 (i32, i32)* addrspace(4)*
//CHECK: [[L4:%[0-9]+]] = call i32  @__intel_indirect_call_1(i32 (i32, i32)* addrspace(4)* [[L3]], i32 1, i32 1)
  p(1, 1);
//CHECK: [[L5:%[0-9]+]] = load i32 (i32)*, i32 (i32)* addrspace(4)* %[[FP_CAST]], align 8
//CHECK: [[L6:%[0-9]+]] = addrspacecast i32 (i32)* [[L5]] to i32 (i32)* addrspace(4)*
//CHECK: [[L7:%[0-9]+]] = call i32  @__intel_indirect_call_2(i32 (i32)* addrspace(4)* [[L6]], i32 10
  fp(10);
//CHECK: store i32 (i32, i32)* null, i32 (i32, i32)* addrspace(4)* %[[P_CAST]],
//CHECK: [[L8:%[0-9]+]] = load i32 (i32, i32)*, i32 (i32, i32)* addrspace(4)* %[[P_CAST]],
//CHECK: [[L9:%[0-9]+]] = addrspacecast i32 (i32, i32)* [[L8]] to i32 (i32, i32)* addrspace(4)*
//CHECK: [[L10:%[0-9]+]] = call i32 @__intel_indirect_call_1(i32 (i32, i32)* addrspace(4)* [[L9]], i32 1, i32 1)
  p = nullptr;
  p(1,1);
//CHECK:call spir_func i32 @_Z3barii(i32 1, i32 2)
  two(1,2);
//CHECK:select{{.*}}@"_Z3mooi$SIMDTable"{{.*}}@"_Z3zooi$SIMDTable"
  fptr one = i ? &moo : &zoo;
//CHECK:call{{.*}}@__intel_indirect_call_2
  one(1);
}

//CHECK: declare spir_func i32 @_Z3zooi(i32) #[[ATT4:[0-9]+]]

extern void qoo(func pt);
extern void test1(){
//CHECK:store{{.*}}@"_Z3barii$SIMDTable"
  func p2 = &bar;
  qoo(p2);
}

//CHECK: declare spir_func i32 @_Z3barii(i32, i32) #[[ATT5:[0-9]+]]
//CHECK: declare spir_func i32 @_Z3mooi(i32) #[[ATT6:[0-9]+]]
extern void qoo(func pt) {
//CHECK:call{{.*}}@__intel_indirect_call_1
  pt(1,1);
}


//CHECK: attributes #[[ATT4]] = {{.*}}"vector_functions_ptrs"="_Z3zooi$SIMDTable()"
//CHECK: attributes #[[ATT5]] = {{.*}}"vector_functions_ptrs"="_Z3barii$SIMDTable()"
//CHECK: attributes #[[ATT6]] = {{.*}}"vector_functions_ptrs"="_Z3mooi$SIMDTable()"
