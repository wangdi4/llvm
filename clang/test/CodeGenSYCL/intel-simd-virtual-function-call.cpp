// RUN: %clang_cc1 -O0 -emit-llvm -o - -std=c++17 -fsycl-is-device \
// RUN: -fenable-variant-virtual-calls \
// RUN:  -triple spir64-unknown-linux %s | FileCheck %s

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

extern SYCL_EXTERNAL int zoo (int);
//CHECK: @_ZTV1B = linkonce_odr unnamed_addr constant { [3 x ptr addrspace(4)] } { [3 x ptr addrspace(4)] [ptr addrspace(4) null, ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr @"_ZN1B3fooEi$SIMDTable" to ptr addrspace(4))] }
//CHECK: @"_ZN1B3fooEi$SIMDTable" = weak global [1 x ptr] [ptr @_ZN1B3fooEi]
//CHECK: @_ZTV1A = unnamed_addr constant { [3 x ptr addrspace(4)] } { [3 x ptr addrspace(4)] [ptr addrspace(4) null, ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr @"_ZN1A3fooEi$SIMDTable" to ptr addrspace(4))] }, align 8
//@"_ZN1A3fooEi$SIMDTable" = weak global [1 x ptr] [ptr @_ZN1A3fooEi]
//CHECK: @"_ZN1A3fooEi$SIMDTable" = weak global [1 x ptr] [ptr @_ZN1A3fooEi]

class A {
public:
    [[intel::device_indirectly_callable]] virtual int foo(int X);
};

[[intel::device_indirectly_callable]] int A::foo(int X){
  return X+1;
}

class B : public A {
public:
   [[intel::device_indirectly_callable]] int foo(int X) override {
     return (X*X);
   }
};
//CHECK-DAG: define linkonce_odr spir_func {{[^,]*}}i32 @_ZN1B3fooEi(ptr addrspace(4) {{[^,]*}}%this, i32 {{[^,]*}}%X) unnamed_addr #[[ATT4:[0-9]+]]
//CHECK-DAG: define dso_local spir_func {{[^,]*}}i32 @_ZN1A3fooEi(ptr addrspace(4) {{[^,]*}}%this, i32 {{[^,]*}}%X) unnamed_addr #[[ATT5:[0-9]+]]
void test()
{
  B bA;
  A* a = &bA;
  int sum=0;
  for (int i=0; i < 4; i++) {
    sum += a->foo(i);
  }
}

int main()
{
  kernel_single_task<class kernel_function>([]() {
   test();
  });
}

//CHECK-DAG: attributes #[[ATT4]] = {{.*}}"vector_function_ptrs"="_ZN1B3fooEi$SIMDTable()" }
//CHECK-DAG: attributes #[[ATT5]] = {{.*}}"vector_function_ptrs"="_ZN1A3fooEi$SIMDTable()" }
