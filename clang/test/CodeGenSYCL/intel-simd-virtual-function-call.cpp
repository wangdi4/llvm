// RUN: %clang_cc1 -O0 -emit-llvm -o - -std=c++17 -fsycl-is-device \
// RUN: -fenable-variant-virtual-calls \
// RUN:  -triple spir64-unknown-linux-sycldevice %s | FileCheck %s

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

extern SYCL_EXTERNAL int zoo (int);
//CHECK: @_ZTV1B = linkonce_odr unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI1B to i8*), i8* bitcast ([1 x i32 (%class._ZTS1B.B addrspace(4)*, i32)*]* @"_ZN1B3fooEi$SIMDTable" to i8*)] }
//CHECK: @"_ZN1B3fooEi$SIMDTable" = weak global [1 x i32 (%class._ZTS1B.B addrspace(4)*, i32)*] [i32 (%class._ZTS1B.B addrspace(4)*, i32)* @_ZN1B3fooEi]
//CHECK: @_ZTV1A = dso_local unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i8* bitcast ([1 x i32 (%class._ZTS1A.A addrspace(4)*, i32)*]* @"_ZN1A3fooEi$SIMDTable" to i8*)] }
//CHECK: @"_ZN1A3fooEi$SIMDTable" = weak global [1 x i32 (%class._ZTS1A.A addrspace(4)*, i32)*] [i32 (%class._ZTS1A.A addrspace(4)*, i32)* @_ZN1A3fooEi]

class A {
public:
    virtual int foo(int X);
};

int A::foo(int X){
  return X+1;
}

class B : public A {
public:
   int foo(int X) {
     return (X*X);
   }
};
//CHECK: define linkonce_odr spir_func i32 @_ZN1B3fooEi(%class._ZTS1B.B addrspace(4)* {{[^,]*}} %this, i32 %X) unnamed_addr #[[ATT4:[0-9]+]]
//CHECK: define dso_local spir_func i32 @_ZN1A3fooEi(%class._ZTS1A.A addrspace(4)* {{[^,]*}} %this, i32 %X) unnamed_addr #[[ATT5:[0-9]+]]
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


//CHECK: attributes #[[ATT4]] = {{.*}}"vector_functions_ptrs"="_ZN1B3fooEi$SIMDTable()" }
//CHECK: attributes #[[ATT5]] = {{.*}}"vector_functions_ptrs"="_ZN1A3fooEi$SIMDTable()" }
