<<<<<<< HEAD
// UNSUPPORTED: intel_opencl && i686-pc-windows
// RUN:  %clang_cc1 -fsycl -fsycl-is-device -I %S/Inputs -triple spir64-unknown-unknown-sycldevice -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s
=======
// RUN:  %clang_cc1 -fsycl -fsycl-is-device -triple spir64-unknown-unknown-sycldevice -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s
>>>>>>> e35466be32bd41607a10a39bf36546f7667829fa

// CHECK-NOT: declare dso_local spir_func void {{.+}}test{{.+}}printer{{.+}}
class test {
public:
  virtual void printer();
};

void test::printer() {}
