// REQUIRES: cpu || gpu
// UNSUPPORTED: cuda,hip
//
// RUN: %{build} -Xclang -fsycl-allow-virtual-functions -o %t.out
// RUN: env IGC_EnableZEBinary=1 %{run} %t.out
//
// Purpose of this test is to check that virtual function calls can be performed
// within kernels and they work correctly.

#include <sycl/sycl.hpp>

#define STANDARD 0
#define BASE 1
#define DERIVED1 2
#define DERIVED2 3

class Standard {
public:
  int display() { return STANDARD; }
};

class Base {
public:
  [[intel::device_indirectly_callable]] virtual int display() { return BASE; }
};

class Derived1 : public Base {
public:
  [[intel::device_indirectly_callable]] int display() override {
    return DERIVED1;
  }
};

class Derived2 : public Base {
public:
  [[intel::device_indirectly_callable]] int display() override {
    return DERIVED2;
  }
};

int main() {
  sycl::queue Q;

  auto *A = sycl::malloc_shared<int>(1, Q);
  Q.single_task([=] {
     Standard s0;
     A[0] = s0.display();
   }).wait();
  assert(A[0] == STANDARD);

  Q.single_task([=] {
     Base b0;
     A[0] = b0.display();
   }).wait();
  assert(A[0] == BASE);

  Q.single_task([=] {
     Derived1 d1;
     Derived2 d2;
     Base *b1 = nullptr;
     // Note: this runtime 'if' is important. It is used to avoid
     // de-virtualization in FE
     if (A[0])
       b1 = &d1;
     else
       b1 = &d2;
     A[0] = b1->display();
   }).wait();
  assert(A[0] == DERIVED1);

  return 0;
}
