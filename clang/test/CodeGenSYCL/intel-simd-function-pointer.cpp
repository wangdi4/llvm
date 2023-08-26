// RUN: %clang_cc1 -O0 -emit-llvm -o - -std=c++17 -fsycl-is-device \
// RUN: -fenable-variant-function-pointers \
// RUN:  -triple spir64-unknown-linux %s | FileCheck %s

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

struct CC {
  double a = 0;
  double b = 0;
};

typedef int(*func)(int, int);
typedef int(*fptr)(int);
typedef int(*fptrs)(CC);
typedef int(*fpi)(int, CC);
typedef int(*fpc)(CC, int);
extern SYCL_EXTERNAL __attribute__((sycl_explicit_simd)) int bar(int, int);
extern SYCL_EXTERNAL __attribute__((sycl_explicit_simd)) int zoo(int);
extern SYCL_EXTERNAL __attribute__((sycl_explicit_simd)) int moo(int);
extern SYCL_EXTERNAL __attribute__((sycl_explicit_simd)) void xoo();
extern SYCL_EXTERNAL __attribute__((sycl_explicit_simd)) int yoo(CC);
extern SYCL_EXTERNAL __attribute__((sycl_explicit_simd)) int roo(CC, int);
extern SYCL_EXTERNAL __attribute__((sycl_explicit_simd)) int too(int, CC);
const func two = &bar;
__attribute__((opencl_private)) __attribute__((sycl_explicit_simd)) fptr one_one;
//CHECK: @"_Z2f1i$SIMDTable" = weak global [1 x ptr] [ptr @_Z2f1i], align 8
//CHECK: @"_Z2f2i$SIMDTable" = weak global [1 x ptr] [ptr @_Z2f2i], align 8
//CHECK: @"_Z2f3i$SIMDTable" = weak global [1 x ptr] [ptr @_Z2f3i], align 8
//CHECK: @__const._Z3xoov.pfarr = private unnamed_addr addrspace(1) constant [3 x ptr] [ptr @"_Z2f1i$SIMDTable", ptr @"_Z2f2i$SIMDTable", ptr @"_Z2f3i$SIMDTable"]
//CHECK: @__const._Z3xoov.obj = private unnamed_addr addrspace(1) constant %struct.C { ptr @"_Z2f1i$SIMDTable", ptr @"_Z2f2i$SIMDTable" }
//CHECK: @"_Z3zooi$SIMDTable" = weak global [1 x ptr] [ptr @_Z3zooi]
//CHECK: @"_Z3barii$SIMDTable" = weak global [1 x ptr] [ptr @_Z3barii]
//CHECK: @"_Z3mooi$SIMDTable" = weak global [1 x ptr] [ptr @_Z3mooi]

int f1(int val) { return val * 2; }
int f2(int val) { return val * 4; }
int f3(int val) { return val * 8; }
int (*f4())(int) { return f1; }

struct C {
  fptr f1;
  fptr f2;
};

extern void xoo() {
// CHECK: call i32 @__intel_indirect_call_0(ptr addrspace(4) {{.*}}, i32 4)
  (f4())(4);
  int (* pfarr[])(int) = {f1, f2, f3};
// CHECK: call i32 @__intel_indirect_call_0(ptr addrspace(4) {{.*}}, i32 5)
  pfarr[0](5);
  C obj = {f1, f2};
// CHECK:  call i32 @__intel_indirect_call_0(ptr addrspace(4) {{.*}}, i32 1)
  obj.f1(1);
}

extern void test1();
void test(int i);
// CHECK: define {{.*}}_ZZ4mainENKUlvE_clEv
int main()
{
  kernel_single_task<class kernel_function>([]() __attribute__((sycl_explicit_simd)) {
    test(10);
    // CHECK: store{{.*}}@"_Z3zooi$SIMDTable"
    one_one = &zoo;
    // CHECK:call{{.*}}@__intel_indirect_call_1
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
//CHECK: [[FP:%fp]] = alloca ptr,
//CHECK: [[P:%p]] = alloca ptr,
//CHECK: %[[FP_CAST:.+]] = addrspacecast ptr [[FP]] to ptr addrspace(4)
//CHECK: %[[P_CAST:.+]] = addrspacecast ptr [[P]] to ptr addrspace(4)
//CHECK: store ptr @"_Z3zooi$SIMDTable", ptr addrspace(4) %[[FP_CAST]],
//CHECK: store ptr @"_Z3barii$SIMDTable", ptr addrspace(4) %[[P_CAST]],
//CHECK: store ptr @"_Z3zooi$SIMDTable", ptr addrspace(4) %[[FP_CAST]],
//CHECK: store ptr @"_Z3barii$SIMDTable", ptr addrspace(4) %[[P_CAST]],
//CHECK: [[L2:%[0-9]+]] = load ptr, ptr addrspace(4) %[[P_CAST]],
//CHECK: [[L3:%[0-9]+]] = addrspacecast ptr [[L2]] to ptr addrspace(4)
//CHECK: [[L4:%[0-9]+]] = call i32  @__intel_indirect_call_2(ptr addrspace(4) [[L3]], i32 1, i32 1)
  p(1, 1);
//CHECK: [[L5:%[0-9]+]] = load ptr, ptr addrspace(4) %[[FP_CAST]], align 8
//CHECK: [[L6:%[0-9]+]] = addrspacecast ptr [[L5]] to ptr addrspace(4)
//CHECK: [[L7:%[0-9]+]] = call i32  @__intel_indirect_call_0(ptr addrspace(4) [[L6]], i32 10
  fp(10);
//CHECK: store ptr null, ptr addrspace(4) %[[P_CAST]],
//CHECK: [[L8:%[0-9]+]] = load ptr, ptr addrspace(4) %[[P_CAST]],
//CHECK: [[L9:%[0-9]+]] = addrspacecast ptr [[L8]] to ptr addrspace(4)
//CHECK: [[L10:%[0-9]+]] = call i32 @__intel_indirect_call_2(ptr addrspace(4) [[L9]], i32 1, i32 1)
  p = nullptr;
  p(1,1);
//CHECK:call{{.*}}@__intel_indirect_call_2(ptr addrspace(4)
  two(1,2);
//CHECK:select{{.*}}@"_Z3mooi$SIMDTable"{{.*}}@"_Z3zooi$SIMDTable"
  fptr one = i ? &moo : &zoo;
//CHECK:call{{.*}}@__intel_indirect_call_0
  one(1);
}

//CHECK: declare spir_func noundef i32 @_Z3zooi(i32 noundef) #[[ATT4:[0-9]+]]
extern void qoo(func pt);
extern void test1(){
//CHECK:store{{.*}}@"_Z3barii$SIMDTable"
  func p2 = &bar;
  qoo(p2);
  xoo();
}

//CHECK: declare spir_func noundef i32 @_Z3barii(i32 noundef, i32 noundef) #[[ATT5:[0-9]+]]
//CHECK: declare spir_func noundef i32 @_Z3mooi(i32 noundef) #[[ATT6:[0-9]+]]
extern void qoo(func pt) {
//CHECK:call{{.*}}@__intel_indirect_call_2
  pt(1,1);
  fptrs one = &yoo;
  CC c{2,3};
  one(c);
//CHECK:call{{.*}}@__intel_indirect_call_3
  fpi x = &too;
  x(1, c);
//CHECK:call{{.*}}@__intel_indirect_call_4
  fpc y = &roo;
  y(c, 1);
//CHECK:call{{.*}}@__intel_indirect_call_5
}
//CHECK: declare spir_func {{.*}} i32 @__intel_indirect_call_3(ptr addrspace(4), ptr noundef byval(%struct.CC) align 8)
//CHECK: declare spir_func noundef i32 @__intel_indirect_call_4(ptr addrspace(4), i32 noundef, ptr noundef byval(%struct.CC) align 8)
//CHECK: declare spir_func noundef i32 @__intel_indirect_call_5(ptr addrspace(4), ptr noundef byval(%struct.CC) align 8, i32 noundef)
//CHECK: attributes #[[ATT4]] = {{.*}}"vector_function_ptrs"="_Z3zooi$SIMDTable()"
//CHECK: attributes #[[ATT5]] = {{.*}}"vector_function_ptrs"="_Z3barii$SIMDTable()"
//CHECK: attributes #[[ATT6]] = {{.*}}"vector_function_ptrs"="_Z3mooi$SIMDTable()"
