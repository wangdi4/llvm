<<<<<<< HEAD
// UNSUPPORTED: intel_opencl && i686-pc-windows
// RUN: %clang -fsycl-device-only -c %s  -o %t.ll -Xclang -fsycl-int-header=%t.hpp -emit-llvm -S
// RUN: FileCheck < %t.ll %s --check-prefix=CHECK
=======
// RUN:  %clang_cc1 -fsycl -fsycl-is-device -I %S/Inputs -triple spir64-unknown-unknown-sycldevice -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s
>>>>>>> 9f4212465204294b7e3936d2bd991e3e040f20db

// CHECK-NOT: declare dso_local spir_func void {{.+}}test{{.+}}printer{{.+}}
class test {
public:
  virtual void printer();
};

void test::printer() {}
