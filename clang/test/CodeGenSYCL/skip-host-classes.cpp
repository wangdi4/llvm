<<<<<<< HEAD
// RUN: %clang -fsycl-device-only -c %s  -o %t.ll -Xclang -fsycl-int-header=%t.hpp -emit-llvm -S
=======
// UNSUPPORTED: intel_opencl && i686-pc-windows
// RUN: %clang --sycl -c %s  -o %t.ll -Xclang -fsycl-int-header=%t.hpp -emit-llvm -S
>>>>>>> b53e0fd964396f29a0c8d3c263a02a96d63be3fb
// RUN: FileCheck < %t.ll %s --check-prefix=CHECK

// CHECK-NOT: declare dso_local spir_func void {{.+}}test{{.+}}printer{{.+}}
class test {
public:
  virtual void printer();
};

void test::printer() {}
