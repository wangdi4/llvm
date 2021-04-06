// RUN: %clangxx -fsycl -fsycl-targets=spir64_fpga-unknown-unknown-sycldevice %s -o %t.out
// env SYCL_DEVICE_TYPE=HOST %t.out
// RUN: %ACC_RUN_PLACEHOLDER %t.out

#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/ac_types/ac_complex.hpp>
#include <CL/sycl/INTEL/fpga_extensions.hpp>

#include <iostream>
#include <vector>

using namespace sycl;

constexpr access::mode kSyclRead = access::mode::read;
constexpr access::mode kSyclWrite = access::mode::write;
constexpr access::mode kSyclReadWrite = access::mode::read_write;

template <int W, int S>
void test_not_operator(queue &Queue, const ac_complex<ac_int<W, S>> &a,
                       bool &b) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp(&a, 1);
  buffer<bool, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class not_operator>([=] { res[0] = !x[0]; });
  });
  Queue.wait();
}

void test_not_operator2(queue &Queue, const ac_complex<int> &a, bool &b) {
  buffer<ac_complex<int>, 1> inp(&a, 1);
  buffer<bool, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class not_operator2>([=] { res[0] = !x[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_add(queue &Queue, const ac_complex<ac_int<W, S>> &a,
              const ac_complex<ac_int<W2, S2>> &b,
              ac_complex<ac_int<W3, S3>> &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<ac_complex<ac_int<W3, S3>>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class add>([=] { res[0] = x[0] + y[0]; });
  });
  Queue.wait();
}

void test_add2(queue &Queue, const ac_complex<int> &a, const ac_complex<int> &b,
               ac_complex<int> &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<ac_complex<int>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class add2>([=] { res[0] = x[0] + y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_add_equal(queue &Queue, const ac_complex<ac_int<W, S>> &a,
                    const ac_complex<ac_int<W2, S2>> &b,
                    ac_complex<ac_int<W3, S3>> &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<ac_complex<ac_int<W3, S3>>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class add_equal>([=] {
      res[0] = x[0];
      res[0] += y[0];
    });
  });
  Queue.wait();
}

void test_add_equal2(queue &Queue, const ac_complex<int> &a,
                     const ac_complex<int> &b, ac_complex<int> &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<ac_complex<int>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class add_equal2>([=] {
      res[0] = x[0];
      res[0] += y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_sub(queue &Queue, const ac_complex<ac_int<W, S>> &a,
              const ac_complex<ac_int<W2, S2>> &b,
              ac_complex<ac_int<W3, S3>> &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<ac_complex<ac_int<W3, S3>>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sub>([=] { res[0] = x[0] - y[0]; });
  });
  Queue.wait();
}

void test_sub2(queue &Queue, const ac_complex<int> &a, const ac_complex<int> &b,
               ac_complex<int> &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<ac_complex<int>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sub2>([=] { res[0] = x[0] - y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_sub_equal(queue &Queue, const ac_complex<ac_int<W, S>> &a,
                    const ac_complex<ac_int<W2, S2>> &b,
                    ac_complex<ac_int<W3, S3>> &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<ac_complex<ac_int<W3, S3>>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sub_equal>([=] {
      res[0] = x[0];
      res[0] -= y[0];
    });
  });
  Queue.wait();
}

void test_sub_equal2(queue &Queue, const ac_complex<int> &a,
                     const ac_complex<int> &b, ac_complex<int> &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<ac_complex<int>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sub_equal2>([=] {
      res[0] = x[0];
      res[0] -= y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_div(queue &Queue, const ac_complex<ac_int<W, S>> &a,
              const ac_complex<ac_int<W2, S2>> &b,
              ac_complex<ac_int<W3, S3>> &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<ac_complex<ac_int<W3, S3>>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class div>([=] { res[0] = x[0] / y[0]; });
  });
  Queue.wait();
}

void test_div2(queue &Queue, const ac_complex<int> &a, const ac_complex<int> &b,
               ac_complex<int> &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<ac_complex<int>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class div2>([=] { res[0] = x[0] / y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_div_equal(queue &Queue, const ac_complex<ac_int<W, S>> &a,
                    const ac_complex<ac_int<W2, S2>> &b,
                    ac_complex<ac_int<W3, S3>> &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<ac_complex<ac_int<W3, S3>>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class div_equal>([=] {
      res[0] = x[0];
      res[0] /= y[0];
    });
  });
  Queue.wait();
}

void test_div_equal2(queue &Queue, const ac_complex<int> &a,
                     const ac_complex<int> &b, ac_complex<int> &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<ac_complex<int>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class div_equal2>([=] {
      res[0] = x[0];
      res[0] /= y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_mult(queue &Queue, const ac_complex<ac_int<W, S>> &a,
               const ac_complex<ac_int<W2, S2>> &b,
               ac_complex<ac_int<W3, S3>> &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<ac_complex<ac_int<W3, S3>>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class mult>([=] { res[0] = x[0] * y[0]; });
  });
  Queue.wait();
}

void test_mult2(queue &Queue, const ac_complex<int> &a,
                const ac_complex<int> &b, ac_complex<int> &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<ac_complex<int>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class mult2>([=] { res[0] = x[0] * y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_mult_equal(queue &Queue, const ac_complex<ac_int<W, S>> &a,
                     const ac_complex<ac_int<W2, S2>> &b,
                     ac_complex<ac_int<W3, S3>> &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<ac_complex<ac_int<W3, S3>>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class mult_equal>([=] {
      res[0] = x[0];
      res[0] *= y[0];
    });
  });
  Queue.wait();
}

void test_mult_equal2(queue &Queue, const ac_complex<int> &a,
                      const ac_complex<int> &b, ac_complex<int> &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<ac_complex<int>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class mult_equal2>([=] {
      res[0] = x[0];
      res[0] *= y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2>
void test_equal(queue &Queue, const ac_complex<ac_int<W, S>> &a,
                const ac_complex<ac_int<W2, S2>> &b, bool &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class equal>([=] { res[0] = x[0] == y[0]; });
  });
  Queue.wait();
}

void test_equal2(queue &Queue, const ac_complex<int> &a,
                 const ac_complex<int> &b, bool &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class equal2>([=] { res[0] = x[0] == y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2>
void test_not_equal(queue &Queue, const ac_complex<ac_int<W, S>> &a,
                    const ac_complex<ac_int<W2, S2>> &b, bool &c) {
  buffer<ac_complex<ac_int<W, S>>, 1> inp1(&a, 1);
  buffer<ac_complex<ac_int<W2, S2>>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class not_equal>([=] { res[0] = x[0] != y[0]; });
  });
  Queue.wait();
}

void test_not_equal2(queue &Queue, const ac_complex<int> &a,
                     const ac_complex<int> &b, bool &c) {
  buffer<ac_complex<int>, 1> inp1(&a, 1);
  buffer<ac_complex<int>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class not_equal2>([=] { res[0] = x[0] != y[0]; });
  });
  Queue.wait();
}

int main() {
  queue Queue(cl::sycl::INTEL::fpga_emulator_selector{});

  bool failed = false;
  ac_complex<ac_int<10, true>> res;
  ac_complex<int> res_type2;

  bool not_res = true;
  test_not_operator<10, true>(Queue, ac_complex<ac_int<10, true>>(10, 20),
                              not_res);
  if (not_res != false) {
    std::cout << "test_not_operator: FAILED"
              << "\n";
    failed = true;
  }

  not_res = true;
  test_not_operator2(Queue, ac_complex<int>(10, 20), not_res);
  if (not_res != false) {
    std::cout << "test_not_operator2: FAILED"
              << "\n";
    failed = true;
  }

  res = 0;
  test_add<10, true, 10, true, 10, true>(
      Queue, ac_complex<ac_int<10, true>>(10, 20),
      ac_complex<ac_int<10, true>>(10, 20), res);
  if (res != ac_complex<ac_int<10, true>>(20, 40)) {
    std::cout << "test_add: FAILED"
              << "\n";
    failed = true;
  }

  res_type2 = 0;
  test_add2(Queue, ac_complex<int>(10, 20), ac_complex<int>(10, 20), res_type2);
  if (res_type2 != ac_complex<int>(20, 40)) {
    std::cout << "test_add2: FAILED"
              << "\n";
    failed = true;
  }

  res = 0;
  test_add_equal<10, true, 10, true, 10, true>(
      Queue, ac_complex<ac_int<10, true>>(10, 20),
      ac_complex<ac_int<10, true>>(10, 20), res);
  if (res != ac_complex<ac_int<10, true>>(20, 40)) {
    std::cout << "test_add_equal: FAILED"
              << "\n";
    failed = true;
  }

  res_type2 = 0;
  test_add_equal2(Queue, ac_complex<int>(10, 20), ac_complex<int>(10, 20),
                  res_type2);
  if (res_type2 != ac_complex<ac_int<10, true>>(20, 40)) {
    std::cout << "test_add_equal2: FAILED"
              << "\n";
    failed = true;
  }

  res = 0;
  test_sub<10, true, 10, true, 10, true>(
      Queue, ac_complex<ac_int<10, true>>(11, 22),
      ac_complex<ac_int<10, true>>(10, 20), res);
  if (res != ac_complex<ac_int<10, true>>(1, 2)) {
    std::cout << "test_sub: FAILED"
              << "\n";
    failed = true;
  }

  res_type2 = 0;
  test_sub2(Queue, ac_complex<int>(11, 22), ac_complex<int>(10, 20), res_type2);
  if (res_type2 != ac_complex<int>(1, 2)) {
    std::cout << "test_sub2: FAILED"
              << "\n";
    failed = true;
  }

  res = 0;
  test_sub_equal<10, true, 10, true, 10, true>(
      Queue, ac_complex<ac_int<10, true>>(11, 22),
      ac_complex<ac_int<10, true>>(10, 20), res);
  if (res != ac_complex<ac_int<10, true>>(1, 2)) {
    std::cout << "test_sub_equal: FAILED"
              << "\n";
    failed = true;
  }

  res_type2 = 0;
  test_sub_equal2(Queue, ac_complex<int>(11, 22), ac_complex<int>(10, 20),
                  res_type2);
  if (res_type2 != ac_complex<int>(1, 2)) {
    std::cout << "test_sub_equal2: FAILED"
              << "\n";
    failed = true;
  }

  res = 0;
  test_div<10, true, 10, true, 10, true>(
      Queue, ac_complex<ac_int<10, true>>(10, 20),
      ac_complex<ac_int<10, true>>(1, 2), res);
  if (res != ac_complex<ac_int<10, true>>(10, 0)) {
    std::cout << "test_div: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res_type2 = 0;
  test_div2(Queue, ac_complex<int>(10, 20), ac_complex<int>(1, 2), res_type2);
  if (res_type2 != ac_complex<int>(10, 0)) {
    std::cout << "test_div2: FAILED"
              << "\n";
    std::cout << "res_type2: " << res_type2 << "\n";
    failed = true;
  }

  res = 0;
  test_div_equal<10, true, 10, true, 10, true>(
      Queue, ac_complex<ac_int<10, true>>(10, 20),
      ac_complex<ac_int<10, true>>(5, 5), res);
  if (res != ac_complex<ac_int<10, true>>(3, 1)) {
    std::cout << "test_div_equal: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res_type2 = 0;
  test_div_equal2(Queue, ac_complex<int>(10, 20), ac_complex<int>(5, 5),
                  res_type2);
  if (res_type2 != ac_complex<int>(3, 1)) {
    std::cout << "test_div_equal2: FAILED"
              << "\n";
    std::cout << "res_type2: " << res_type2 << "\n";
    failed = true;
  }

  res = 0;
  test_mult<10, true, 10, true, 10, true>(
      Queue, ac_complex<ac_int<10, true>>(10, 20),
      ac_complex<ac_int<10, true>>(5, 10), res);
  if (res != ac_complex<ac_int<10, true>>(-150, 200)) {
    std::cout << "test_mult: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res_type2 = 0;
  test_mult2(Queue, ac_complex<int>(10, 20), ac_complex<int>(5, 10), res_type2);
  if (res_type2 != ac_complex<int>(-150, 200)) {
    std::cout << "test_mult2: FAILED"
              << "\n";
    std::cout << "res_type2: " << res_type2 << "\n";
    failed = true;
  }

  res = 0;
  test_mult_equal<10, true, 10, true, 10, true>(
      Queue, ac_complex<ac_int<10, true>>(10, 20),
      ac_complex<ac_int<10, true>>(2, 20), res);
  if (res != ac_complex<ac_int<10, true>>(-380, 240)) {
    std::cout << "test_mult_equal: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res_type2 = 0;
  test_mult_equal2(Queue, ac_complex<int>(10, 20), ac_complex<int>(2, 20),
                   res_type2);
  if (res_type2 != ac_complex<int>(-380, 240)) {
    std::cout << "test_mult_equal2: FAILED"
              << "\n";
    std::cout << "res_type2: " << res_type2 << "\n";
    failed = true;
  }

  bool comparison_res = false;
  test_equal<10, true, 10, true>(Queue, ac_complex<ac_int<10, true>>(5, 20),
                                 ac_complex<ac_int<10, true>>(5, 20),
                                 comparison_res);
  if (comparison_res != true) {
    std::cout << "test_equal: FAILED"
              << "\n";
    failed = true;
  }

  comparison_res = false;
  test_equal2(Queue, ac_complex<int>(5, 20), ac_complex<int>(5, 20),
              comparison_res);
  if (comparison_res != true) {
    std::cout << "test_equal2: FAILED"
              << "\n";
    failed = true;
  }

  comparison_res = false;
  test_not_equal<10, true, 10, true>(Queue, ac_complex<ac_int<10, true>>(3, 20),
                                     ac_complex<ac_int<10, true>>(1, 20),
                                     comparison_res);
  if (comparison_res != true) {
    std::cout << "test_not_equal: FAILED"
              << "\n";
    failed = true;
  }

  comparison_res = false;
  test_not_equal2(Queue, ac_complex<int>(3, 20), ac_complex<int>(1, 20),
                  comparison_res);
  if (comparison_res != true) {
    std::cout << "test_not_equal2: FAILED"
              << "\n";
    failed = true;
  }

  if (failed) {
    std::cout << "FAILED\n";
    return -1;
  }

  std::cout << "PASSED\n";
  return 0;
}
