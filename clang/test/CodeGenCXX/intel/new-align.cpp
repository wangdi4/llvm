// RUN: %clang_cc1 -O0 -emit-llvm %s -o - -triple=x86_64-unknown-linux-gnu -fintel-compatibility -faligned-allocation| FileCheck %s
// This test fails in xmain. CQ376359.

struct __declspec(align(4096)) A {
  int a;
  bool b;
  char c;
};

void *allocate_foo(unsigned long _S, std::align_val_t _A);
void release_foo(void *_P, std::align_val_t _A);

void *operator new(unsigned long _S, std::align_val_t _A) {
  void *_P;
  _P = allocate_foo(_S, _A);
  return _P;
}

// CHECK: define {{.+}}@_Znw{{.*}}({{.+}},{{.+}})

void operator delete(void *_P, std::align_val_t _A) {
  release_foo(_P, _A);
}

// CHECK: define {{.+}}@_Zdl{{.*}}({{.+}},{{.+}})

int main() {
  A *p = new A;
  delete p;
}
// CHECK-LABEL:define {{.+}}main
// CHECK:   call {{.+}}@_Znw{{.*}}({{.+}},{{.+}})
