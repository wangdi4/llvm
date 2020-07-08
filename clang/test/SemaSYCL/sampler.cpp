<<<<<<< HEAD
// UNSUPPORTED: intel_opencl && i686-pc-windows
// RUN: %clang -S -I %S/Inputs -fsycl-device-only -Xclang -ast-dump %s | FileCheck %s
=======
// RUN: %clang_cc1 -S -I %S/Inputs -fsycl -fsycl-is-device -triple spir64 -ast-dump %s | FileCheck %s
>>>>>>> 9f4212465204294b7e3936d2bd991e3e040f20db

#include <sycl.hpp>

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel(Func kernelFunc) {
  kernelFunc();
}

int main() {
  cl::sycl::sampler Sampler;
  kernel<class use_kernel_for_test>([=]() {
    Sampler.use();
  });
  return 0;
}

// Check declaration of the test kernel
// CHECK: FunctionDecl {{.*}}use_kernel_for_test{{.*}} 'void (sampler_t)'
//
// Check parameters of the test kernel
// CHECK: ParmVarDecl {{.*}} used [[_arg_sampler:[0-9a-zA-Z_]+]] 'sampler_t'
//
// Check that sampler field of the test kernel object is initialized using __init method
// CHECK: CXXMemberCallExpr {{.*}} 'void'
// CHECK-NEXT: MemberExpr {{.*}} 'void (__ocl_sampler_t)' lvalue .__init
// CHECK-NEXT: MemberExpr {{.*}} 'cl::sycl::sampler':'cl::sycl::sampler' lvalue
// CHECK-NEXT: DeclRefExpr {{.*}} '(lambda at {{.*}}sampler.cpp{{.*}})' lvalue Var {{.*}} '(lambda at {{.*}}sampler.cpp{{.*}})'
//
// Check the parameters of __init method
// CHECK-NEXT: ImplicitCastExpr {{.*}} '__ocl_sampler_t':'sampler_t' <LValueToRValue>
// CHECK-NEXT: DeclRefExpr {{.*}} 'sampler_t' lvalue ParmVar {{.*}} '[[_arg_sampler]]' 'sampler_t'
