// RUN: %clangxx -fsycl -fsycl-targets=spir64_fpga-unknown-unknown-sycldevice %s -o %t.out
// env SYCL_DEVICE_TYPE=HOST %t.out
// RUN: %ACC_RUN_PLACEHOLDER %t.out

#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/ac_types/ac_int.hpp>
#include <CL/sycl/INTEL/fpga_extensions.hpp>

#include <iostream>
#include <vector>

using namespace sycl;

constexpr access::mode kSyclRead = access::mode::read;
constexpr access::mode kSyclWrite = access::mode::write;
constexpr access::mode kSyclReadWrite = access::mode::read_write;

template <int W, int S, int W2, int S2>
void test_increment(queue &Queue, const ac_int<W, S> &a, ac_int<W2, S2> &b) {
  buffer<ac_int<W, S>, 1> inp(&a, 1);
  buffer<ac_int<W2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class increment>([=] {
      res[0] = x[0];
      res[0]++;
    });
  });
  Queue.wait();
}

template <int W, int S>
void test_decrement(queue &Queue, const ac_int<W, S> &a, ac_int<W, S> &b) {
  buffer<ac_int<W, S>, 1> inp(&a, 1);
  buffer<ac_int<W, S>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class decrement>([=] {
      res[0] = x[0];
      res[0]--;
    });
  });
  Queue.wait();
}

template <int W, int S>
void test_negation(queue &Queue, const ac_int<W, S> &a, ac_int<W, S> &b) {
  buffer<ac_int<W, S>, 1> inp(&a, 1);
  buffer<ac_int<W, S>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class negation>([=] { res[0] = -x[0]; });
  });
  Queue.wait();
}

template <int W, int S>
void test_bitwise_complement(queue &Queue, const ac_int<W, S> &a,
                             ac_int<W, S> &b) {
  buffer<ac_int<W, S>, 1> inp(&a, 1);
  buffer<ac_int<W, S>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bitwise_complement>([=] { res[0] = ~x[0]; });
  });
  Queue.wait();
}

template <int W, int S>
void test_not_operator(queue &Queue, const ac_int<W, S> &a, bool &b) {
  buffer<ac_int<W, S>, 1> inp(&a, 1);
  buffer<bool, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class not_operator>([=] { res[0] = !x[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_add(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
              ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class add>([=] { res[0] = x[0] + y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_add_equal(queue &Queue, const ac_int<W, S> &a,
                    const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

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

template <int W, int S, int W2, int S2, int W3, int S3>
void test_sub(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
              ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sub>([=] { res[0] = x[0] - y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_sub_equal(queue &Queue, const ac_int<W, S> &a,
                    const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

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

template <int W, int S, int W2, int S2, int W3, int S3>
void test_div(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
              ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class div>([=] { res[0] = x[0] / y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_div_equal(queue &Queue, const ac_int<W, S> &a,
                    const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

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

template <int W, int S, int W2, int S2, int W3, int S3>
void test_mult(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
               ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class mult>([=] { res[0] = x[0] * y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_mult_equal(queue &Queue, const ac_int<W, S> &a,
                     const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

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

template <int W, int S, int W2, int S2, int W3, int S3>
void test_modulo(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
                 ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class modulo>([=] { res[0] = x[0] % y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_modulo_equal(queue &Queue, const ac_int<W, S> &a,
                       const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class modulo_equal>([=] {
      res[0] = x[0];
      res[0] %= y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_shift_l(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
                  ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class shift_l>([=] { res[0] = x[0] << y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_shift_l_equal(queue &Queue, const ac_int<W, S> &a,
                        const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class shift_l_equal>([=] {
      res[0] = x[0];
      res[0] <<= y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_shift_r(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
                  ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class shift_r>([=] { res[0] = x[0] >> y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_shift_r_equal(queue &Queue, const ac_int<W, S> &a,
                        const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class shift_r_equal>([=] {
      res[0] = x[0];
      res[0] >>= y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_bitwise_and(queue &Queue, const ac_int<W, S> &a,
                      const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bitwise_and>([=] { res[0] = x[0] & y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_bitwise_and_equal(queue &Queue, const ac_int<W, S> &a,
                            const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bitwise_and_equal>([=] {
      res[0] = x[0];
      res[0] &= y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_bitwise_or(queue &Queue, const ac_int<W, S> &a,
                     const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bitwise_or>([=] { res[0] = x[0] | y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_bitwise_or_equal(queue &Queue, const ac_int<W, S> &a,
                           const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bitwise_or_equal>([=] {
      res[0] = x[0];
      res[0] |= y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_bitwise_xor(queue &Queue, const ac_int<W, S> &a,
                      const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bitwise_xor>([=] { res[0] = x[0] ^ y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_bitwise_xor_equal(queue &Queue, const ac_int<W, S> &a,
                            const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bitwise_xor_equal>([=] {
      res[0] = x[0];
      res[0] ^= y[0];
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2>
void test_equal(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
                bool &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class equal>([=] { res[0] = x[0] == y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2>
void test_not_equal(queue &Queue, const ac_int<W, S> &a,
                    const ac_int<W2, S2> &b, bool &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class not_equal>([=] { res[0] = x[0] != y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2>
void test_gt(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
             bool &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class gt>([=] { res[0] = x[0] > y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2>
void test_gt_equal(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
                   bool &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class gt_equal>([=] { res[0] = x[0] >= y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2>
void test_lt(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
             bool &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class lt>([=] { res[0] = x[0] < y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2>
void test_lt_equal(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
                   bool &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class lt_equal>([=] { res[0] = x[0] <= y[0]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3>
void test_bit_select(queue &Queue, const ac_int<W, S> &a,
                     const ac_int<W2, S2> &b, ac_int<W3, S3> &c) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bit_select>([=] { res[0] = x[0][y[0]]; });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3, int W4, int S4>
void test_bit_copy(queue &Queue, const ac_int<W, S> &a, const ac_int<W2, S2> &b,
                   const ac_int<W3, S3> &c, ac_int<W4, S4> &d) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> inp3(&c, 1);
  buffer<ac_int<W4, S4>, 1> result(&d, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto z = inp3.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bit_copy>([=] {
      auto x_ = x[0];
      auto y_ = y[0];
      auto z_ = z[0];
      res[0] = x[0];
      res[0][y_] = z_;
    });
  });
  Queue.wait();
}

template <int W, int S, int W2, int S2, int W3, int S3, int W4, int S4>
void test_bit_slice(queue &Queue, const ac_int<W, S> &a,
                    const ac_int<W2, S2> &b, const ac_int<W3, S3> &c,
                    ac_int<W4, S4> &d) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<ac_int<W2, S2>, 1> inp2(&b, 1);
  buffer<ac_int<W3, S3>, 1> inp3(&c, 1);
  buffer<ac_int<W4, S4>, 1> result(&d, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto z = inp3.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class bit_slice>([=] {
      res[0] = x[0];
      res[0].set_slc(y[0], z[0]);
    });
  });
  Queue.wait();
}

template <int W, int S, int N>
void test_bit_fill(queue &Queue, ac_int<W, S> &a, const std::vector<int> &arr) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<int, 1> inp2(arr.data(), range<1>(N));

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclReadWrite>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    h.single_task<class bit_fill>([=] {
      int a[N];
      for (int i = 0; i < N; i++) {
        a[i] = y[i];
      }
      x[0].template bit_fill<N>(a);
    });
  });
  Queue.wait();
}

template <int W, int S, int N>
void test_bit_fill_hex(queue &Queue, ac_int<W, S> &a,
                       const std::string &hex_string) {
  buffer<ac_int<W, S>, 1> inp1(&a, 1);
  buffer<char, 1> inp2(hex_string.c_str(), range<1>(N));

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclReadWrite>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    h.single_task<class bit_fill_hex>([=] {
      char str[N + 1];
      int i;
      for (i = 0; i < N; i++) {
        str[i] = y[i];
      }
      str[i] = '\0';
      x[0].bit_fill_hex(str);
    });
  });
  Queue.wait();
}

int main() {
  queue Queue(cl::sycl::INTEL::fpga_emulator_selector{});

  bool failed = false;
  ac_int<10, true> res;
  test_increment<10, true, 10, true>(Queue, ac_int<10, true>(10), res);
  if (res != ac_int<10, true>(11)) {
    std::cout << "test_increment: FAILED"
              << "\n";
    failed = true;
  }

  test_decrement<10, true>(Queue, ac_int<10, true>(10), res);
  if (res != ac_int<10, true>(9)) {
    std::cout << "test_decrement: FAILED"
              << "\n";
    failed = true;
  }

  test_negation<10, true>(Queue, ac_int<10, true>(10), res);
  if (res != ac_int<10, true>(-10)) {
    std::cout << "test_negation: FAILED"
              << "\n";
    failed = true;
  }

  test_bitwise_complement<10, true>(Queue, ac_int<10, true>(10), res);
  if (res !=
      ac_int<10, true>(
          -11)) { // The complement of 10 is the same as 2s complement of 11
    std::cout << "test_bitwise_complement: FAILED"
              << "\n";
    failed = true;
  }

  bool not_res = true;
  test_not_operator<10, true>(Queue, ac_int<10, true>(10), not_res);
  if (not_res != false) {
    std::cout << "test_not_operator: FAILED"
              << "\n";
    failed = true;
  }

  test_add<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                         ac_int<10, true>(10), res);
  if (res != ac_int<10, true>(20)) {
    std::cout << "test_add: FAILED"
              << "\n";
    failed = true;
  }

  test_add_equal<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                               ac_int<10, true>(10), res);
  if (res != ac_int<10, true>(20)) {
    std::cout << "test_add_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_sub<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(11),
                                         ac_int<10, true>(10), res);
  if (res != ac_int<10, true>(1)) {
    std::cout << "test_sub: FAILED"
              << "\n";
    failed = true;
  }

  test_sub_equal<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(11),
                                               ac_int<10, true>(10), res);
  if (res != ac_int<10, true>(1)) {
    std::cout << "test_sub_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_div<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                         ac_int<10, true>(5), res);
  if (res != ac_int<10, true>(2)) {
    std::cout << "test_div: FAILED"
              << "\n";
    failed = true;
  }

  test_div_equal<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                               ac_int<10, true>(5), res);
  if (res != ac_int<10, true>(2)) {
    std::cout << "test_div_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_mult<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                          ac_int<10, true>(2), res);
  if (res != ac_int<10, true>(20)) {
    std::cout << "test_mult: FAILED"
              << "\n";
    failed = true;
  }

  test_mult_equal<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                                ac_int<10, true>(2), res);
  if (res != ac_int<10, true>(20)) {
    std::cout << "test_mult_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_modulo<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                            ac_int<10, true>(5), res);
  if (res != ac_int<10, true>(0)) {
    std::cout << "test_modulo: FAILED"
              << "\n";
    failed = true;
  }

  test_modulo_equal<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                                  ac_int<10, true>(5), res);
  if (res != ac_int<10, true>(0)) {
    std::cout << "test_modulo_equal: FAILED"
              << "\n";
    failed = true;
  }

  ac_int<11, true> shift_res = 0;
  test_shift_l<10, true, 10, true, 11, true>(Queue, ac_int<10, true>(10),
                                             ac_int<10, true>(1), shift_res);
  if (shift_res != ac_int<11, true>(20)) {
    std::cout << "test_shift_l: FAILED"
              << "\n";
    failed = true;
  }

  shift_res = 0;
  test_shift_l_equal<10, true, 10, true, 11, true>(
      Queue, ac_int<10, true>(10), ac_int<10, true>(1), shift_res);
  if (res != ac_int<10, true>(0)) {
    std::cout << "test_shift_l_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_shift_r<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                             ac_int<10, true>(1), res);
  if (res != ac_int<10, true>(5)) {
    std::cout << "test_shift_r: FAILED"
              << "\n";
    failed = true;
  }

  test_shift_r_equal<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                                   ac_int<10, true>(1), res);
  if (res != ac_int<10, true>(5)) {
    std::cout << "test_shift_r_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_bitwise_and<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(3),
                                                 ac_int<10, true>(2), res);
  if (res != ac_int<10, true>(2)) {
    std::cout << "test_bitwise_and: FAILED"
              << "\n";
    failed = true;
  }

  test_bitwise_and_equal<10, true, 10, true, 10, true>(
      Queue, ac_int<10, true>(3), ac_int<10, true>(2), res);
  if (res != ac_int<10, true>(2)) {
    std::cout << "test_bitwise_and_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_bitwise_or<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(2),
                                                ac_int<10, true>(1), res);
  if (res != ac_int<10, true>(3)) {
    std::cout << "test_bitwise_or: FAILED"
              << "\n";
    failed = true;
  }

  test_bitwise_or_equal<10, true, 10, true, 10, true>(
      Queue, ac_int<10, true>(2), ac_int<10, true>(1), res);
  if (res != ac_int<10, true>(3)) {
    std::cout << "test_bitwise_or_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_bitwise_xor<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(3),
                                                 ac_int<10, true>(1), res);
  if (res != ac_int<10, true>(2)) {
    std::cout << "test_bitwise_xor: FAILED"
              << "\n";
    failed = true;
  }

  test_bitwise_xor_equal<10, true, 10, true, 10, true>(
      Queue, ac_int<10, true>(3), ac_int<10, true>(1), res);
  if (res != ac_int<10, true>(2)) {
    std::cout << "test_bitwise_xor_equal: FAILED"
              << "\n";
    failed = true;
  }

  bool comparison_res = false;
  test_equal<10, true, 10, true>(Queue, ac_int<10, true>(5),
                                 ac_int<10, true>(5), comparison_res);
  if (comparison_res != true) {
    std::cout << "test_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_not_equal<10, true, 10, true>(Queue, ac_int<10, true>(3),
                                     ac_int<10, true>(1), comparison_res);
  if (comparison_res != true) {
    std::cout << "test_not_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_gt<10, true, 10, true>(Queue, ac_int<10, true>(5), ac_int<10, true>(1),
                              comparison_res);
  if (comparison_res != true) {
    std::cout << "test_gt: FAILED"
              << "\n";
    failed = true;
  }

  test_gt_equal<10, true, 10, true>(Queue, ac_int<10, true>(5),
                                    ac_int<10, true>(5), comparison_res);
  if (comparison_res != true) {
    std::cout << "test_gt_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_lt<10, true, 10, true>(Queue, ac_int<10, true>(5), ac_int<10, true>(10),
                              comparison_res);
  if (comparison_res != true) {
    std::cout << "test_lt: FAILED"
              << "\n";
    failed = true;
  }

  test_lt_equal<10, true, 10, true>(Queue, ac_int<10, true>(5),
                                    ac_int<10, true>(5), comparison_res);
  if (comparison_res != true) {
    std::cout << "test_lt_equal: FAILED"
              << "\n";
    failed = true;
  }

  test_bit_select<10, true, 10, true, 10, true>(Queue, ac_int<10, true>(10),
                                                ac_int<10, true>(3), res);
  if (res != ac_int<10, true>(1)) {
    std::cout << "test_bit_select: FAILED"
              << "\n";
    failed = true;
  }

  test_bit_copy<10, true, 10, true, 10, true, 10, true>(
      Queue, ac_int<10, true>(10), ac_int<10, true>(2), ac_int<10, true>(1),
      res);
  if (res != ac_int<10, true>(14)) {
    std::cout << "test_bit_copy: FAILED"
              << "\n";
    failed = true;
  }

  test_bit_slice<10, true, 10, true, 2, false, 10, true>(
      Queue, ac_int<10, true>(24), ac_int<10, true>(1), ac_int<2, false>(3),
      res);
  if (res != ac_int<10, true>(30)) {
    std::cout << "test_bit_slice: FAILED"
              << "\n";
    failed = true;
  }

  std::vector<int> vec_inp{0x00000001, 0x00000002};
  ac_int<64, false> bit_fill_res;
  test_bit_fill<64, false, 2>(Queue, bit_fill_res, vec_inp);
  if (bit_fill_res != ac_int<64, false>(4294967298)) {
    std::cout << "test_bit_fill: FAILED"
              << "\n";
    failed = true;
  }

  std::string str_inp{"0x0000000100000001"};
  bit_fill_res = 0;
  test_bit_fill_hex<64, false, 64>(Queue, bit_fill_res, str_inp);
  if (bit_fill_res != ac_int<64, false>(4294967297)) {
    std::cout << "test_bit_fill_hex: FAILED"
              << "\n";
    std::cout << "res: " << bit_fill_res << "\n";
    failed = true;
  }

  if (failed) {
    std::cout << "FAILED\n";
    return -1;
  }

  std::cout << "PASSED\n";
  return 0;
}
