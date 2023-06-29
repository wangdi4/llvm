// REQUIRES: cpu || gpu
// UNSUPPORTED: cuda,hip
//
// RUN: %{build} -Xclang -fsycl-allow-virtual-functions -o %t.out
// RUN: env IGC_EnableZEBinary=1 %{run} %t.out
//
// Purpose of this test is to check that we can successfully compile and
// execute an example, where some of virtual functions are not intended to be
// used on device and use features which are illegal there.

#include <sycl/sycl.hpp>

#define BASE 1
#define DERIVED1 2
#define DERIVED2 3

class Base {
public:
  [[intel::device_indirectly_callable]] virtual int display() { return BASE; }

  virtual void allocate_on_device(char *Storage, sycl::queue Q) {
    if (!Storage)
      throw std::invalid_argument("Storage can't be nullptr");
    Q.single_task([=] { new (Storage) Base; }).wait();
  }
};

class Derived1 : public Base {
public:
  [[intel::device_indirectly_callable]] int display() override {
    return DERIVED1;
  }

  void allocate_on_device(char *Storage, sycl::queue Q) override {
    if (!Storage)
      throw std::invalid_argument("Storage can't be nullptr");
    Q.single_task([=] { new (Storage) Derived1; }).wait();
  }
};

class Derived2 : public Base {
public:
  [[intel::device_indirectly_callable]] int display() override {
    return DERIVED2;
  }

  void allocate_on_device(char *Storage, sycl::queue Q) override {
    if (!Storage)
      throw std::invalid_argument("Storage can't be nullptr");
    Q.single_task([=] { new (Storage) Derived2; }).wait();
  }
};

int main() {
  sycl::queue Q;

  auto *Storage =
      sycl::malloc_device<char>(sizeof(Derived1) + sizeof(Derived2), Q);
  auto *Ptrs = sycl::malloc_device<Base *>(2, Q);
  auto *Result = sycl::malloc_shared<int>(2, Q);

  Derived1 D1;
  Derived2 D2;
  D1.allocate_on_device(&Storage[0], Q);
  D2.allocate_on_device(&Storage[sizeof(Derived1)], Q);

  Q.single_task([=] {
     Ptrs[0] = reinterpret_cast<Base *>(&Storage[0]);
     Ptrs[1] = reinterpret_cast<Base *>(&Storage[sizeof(Derived1)]);
   }).wait();

  Q.single_task([=] {
     for (int i = 0; i < 2; ++i) {
       Result[i] = Ptrs[i]->display();
     }
   }).wait();

  assert(Result[0] == DERIVED1);
  assert(Result[1] == DERIVED2);

  return 0;
}
