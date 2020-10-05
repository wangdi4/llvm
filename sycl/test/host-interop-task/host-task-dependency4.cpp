// RUN: %clangxx -fsycl -fsycl-targets=%sycl_triple %s -o %t.out

<<<<<<< HEAD
// RUN: %CPU_RUN_PLACEHOLDER %t.out
// RUN: %GPU_RUN_PLACEHOLDER %t.out
// RUN: %ACC_RUN_PLACEHOLDER %t.out
=======
// RUNx: %CPU_RUN_PLACEHOLDER %t.out
// RUNx: %GPU_RUN_PLACEHOLDER %t.out
// RUNx: %ACC_RUN_PLACEHOLDER %t.out
>>>>>>> 972628b8b308c7a37d074abeb586bbf50d100512

#include <CL/sycl.hpp>

cl::sycl::event submit(cl::sycl::queue &Q, cl::sycl::buffer<int> &B) {
  return Q.submit([&](cl::sycl::handler &CGH) {
    auto A = B.template get_access<cl::sycl::access::mode::read_write>(CGH);
    CGH.codeplay_host_task([=]() { (void)A; });
  });
}

int main() {
  cl::sycl::queue Q;
  int Status = 0;
  cl::sycl::buffer<int> A{&Status, 1};
  cl::sycl::vector_class<cl::sycl::event> Events;

  Events.push_back(submit(Q, A));
  Events.push_back(submit(Q, A));
  Q.submit([&](sycl::handler &CGH) {
     CGH.depends_on(Events);
     CGH.codeplay_host_task([&] { printf("all done\n"); });
   }).wait_and_throw();

  return 0;
}
