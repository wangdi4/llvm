// RUN: %clangxx -fsycl -fsycl-targets=spir64_fpga-unknown-unknown-sycldevice %s -o %t.out
// env SYCL_DEVICE_TYPE=HOST %t.out
// RUN: %ACC_RUN_PLACEHOLDER %t.out

#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/ac_types/ac_fixed_math.hpp>
#include <CL/sycl/INTEL/fpga_extensions.hpp>

#include <iostream>
#include <vector>

using namespace sycl;

constexpr access::mode kSyclRead = access::mode::read;
constexpr access::mode kSyclWrite = access::mode::write;
constexpr access::mode kSyclReadWrite = access::mode::read_write;

template <int W, int I, int S, int W2, int I2, int S2>
void test_sqrt(queue &Queue, const ac_fixed<W, I, S> &a,
               ac_fixed<W2, I2, S2> &b) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sqrt>([=]() { res[0] = sqrt_fixed(x[0]); });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2>
void test_reciprocal(queue &Queue, const ac_fixed<W, I, S> &a,
                     ac_fixed<W2, I2, S2> &b) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class reciprocal>([=]() { res[0] = reciprocal_fixed(x[0]); });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2>
void test_reciprocal_sqrt(queue &Queue, const ac_fixed<W, I, S> &a,
                          ac_fixed<W2, I2, S2> &b) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class reciprocal_sqrt>(
        [=]() { res[0] = reciprocal_sqrt_fixed(x[0]); });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2>
void test_sin(queue &Queue, const ac_fixed<W, I, S> &a,
              ac_fixed<W2, I2, S2> &b) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sin>([=]() { res[0] = sin_fixed(x[0]); });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2>
void test_cos(queue &Queue, const ac_fixed<W, I, S> &a,
              ac_fixed<W2, I2, S2> &b) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class cos>([=]() { res[0] = cos_fixed(x[0]); });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2, int W3, int I3, int S3>
void test_sincos(queue &Queue, const ac_fixed<W, I, S> &a,
                 ac_fixed<W2, I2, S2> &b, ac_fixed<W3, I3, S3> &c) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> res_b(&b, 1);
  buffer<ac_fixed<W3, I3, S3>, 1> res_c(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res_sin = res_b.template get_access<kSyclWrite>(h);
    auto res_cos = res_c.template get_access<kSyclWrite>(h);
    h.single_task<class sincos>([=]() {
      typename ac_fixed_math_private::rt<W, I, S>::sin_r s;
      typename ac_fixed_math_private::rt<W, I, S>::cos_r c;
      sincos_fixed(x[0], s, c);
      res_sin[0] = s;
      res_cos[0] = c;
    });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2>
void test_sinpi(queue &Queue, const ac_fixed<W, I, S> &a,
                ac_fixed<W2, I2, S2> &b) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sinpi>([=]() { res[0] = sinpi_fixed(x[0]); });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2>
void test_cospi(queue &Queue, const ac_fixed<W, I, S> &a,
                ac_fixed<W2, I2, S2> &b) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class cospi>([=]() { res[0] = cospi_fixed(x[0]); });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2, int W3, int I3, int S3>
void test_sincospi(queue &Queue, const ac_fixed<W, I, S> &a,
                   ac_fixed<W2, I2, S2> &b, ac_fixed<W3, I3, S3> &c) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> res_b(&b, 1);
  buffer<ac_fixed<W3, I3, S3>, 1> res_c(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res_sin = res_b.template get_access<kSyclReadWrite>(h);
    auto res_cos = res_c.template get_access<kSyclReadWrite>(h);
    h.single_task<class sincospi>([=]() {
      typename ac_fixed_math_private::rt<W, I, S>::sinpi_r s;
      typename ac_fixed_math_private::rt<W, I, S>::cospi_r c;
      sincospi_fixed(x[0], s, c);
      res_sin[0] = s;
      res_cos[0] = c;
    });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2>
void test_exp(queue &Queue, const ac_fixed<W, I, S> &a,
              ac_fixed<W2, I2, S2> &b) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class exp>([=]() { res[0] = exp_fixed(x[0]); });
  });
  Queue.wait();
}

template <int W, int I, int S, int W2, int I2, int S2>
void test_log(queue &Queue, const ac_fixed<W, I, S> &a,
              ac_fixed<W2, I2, S2> &b) {
  buffer<ac_fixed<W, I, S>, 1> inp1(&a, 1);
  buffer<ac_fixed<W2, I2, S2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp1.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class log>([=]() { res[0] = log_fixed(x[0]); });
  });
  Queue.wait();
}

int main() {
  queue Queue(cl::sycl::INTEL::fpga_emulator_selector{});

  bool failed = false;
  ac_fixed<32, 8, true> res;

  res = 0;
  test_sqrt<32, 8, true, 32, 8, true>(Queue, ac_fixed<32, 8, true>(9), res);
  if (res != ac_fixed<32, 8, true>(3)) {
    std::cout << "test_sqrt: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_reciprocal<32, 8, true, 32, 8, true>(Queue, ac_fixed<32, 8, true>(10),
                                            res);
  if (res != ac_fixed<32, 8, true>(.10000002384185791015625)) {
    std::cout << "test_reciprocal: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_reciprocal_sqrt<32, 8, true, 32, 8, true>(
      Queue, ac_fixed<32, 8, true>(100), res);
  if (res != ac_fixed<32, 8, true>(.10000002384185791015625)) {
    std::cout << "test_reciprocal_sqrt: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  // Trig functions require parameter I to be equal to 3 if the ac_fixed is
  // signed
  ac_fixed<32, 3, true> res_sin = 0;
  test_sin<32, 3, true, 32, 3, true>(Queue, ac_fixed<32, 3, true>(3), res_sin);
  if (res_sin != ac_fixed<32, 3, true>(0.141120008)) {
    std::cout << "test_sin: FAILED"
              << "\n";
    std::cout << "res: " << res_sin << "\n";
    failed = true;
  }

  ac_fixed<32, 3, true> res_cos = 0;
  test_cos<32, 3, true, 32, 3, true>(Queue, ac_fixed<32, 3, true>(3), res_cos);
  if (res_cos != ac_fixed<32, 3, true>(-0.989992497)) {
    std::cout << "test_cos: FAILED"
              << "\n";
    std::cout << "res: " << res_cos << "\n";
    failed = true;
  }

  ac_fixed<32, 3, true> res_ = 0, res_b = 0;
  test_sincos<32, 3, true, 32, 3, true, 32, 3, true>(
      Queue, ac_fixed<32, 3, true>(1.0472), res_, res_b);
  if (res_ != ac_fixed<32, 3, true>(.8660266287624835968017578125) ||
      res_b != ac_fixed<32, 3, true>(.49999787844717502593994140625)) {
    std::cout << "test_sincos: FAILED"
              << "\n";
    std::cout << "res_sin: " << res_ << "\n";
    std::cout << "res_cos: " << res_b << "\n";
    failed = true;
  }

  res = 0;
  test_sinpi<32, 8, true, 32, 8, true>(Queue, ac_fixed<32, 8, true>(1.8), res);
  if (res != ac_fixed<32, 8, true>(-.58778536319732666015625)) {
    std::cout << "test_sinpi: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_cospi<32, 8, true, 32, 8, true>(Queue, ac_fixed<32, 8, true>(1.8), res);
  if (res != ac_fixed<32, 8, true>(.809016883373260498046875)) {
    std::cout << "test_cospi: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  ac_fixed<32, 8, true> res_c = 0;
  test_sincospi<32, 8, true, 32, 8, true, 32, 8, true>(
      Queue, ac_fixed<32, 8, true>(1.8), res, res_c);
  if (res != ac_fixed<32, 8, true>(-.58778536319732666015625) ||
      res_c != ac_fixed<32, 8, true>(.809016883373260498046875)) {
    std::cout << "test_sincospi: FAILED"
              << "\n";
    std::cout << "res_sin: " << res << "\n";
    std::cout << "res_cos: " << res_c << "\n";
    failed = true;
  }

  res = 0;
  test_exp<32, 8, true, 32, 8, true>(Queue, ac_fixed<32, 8, true>(2), res);
  if (res != ac_fixed<32, 8, true>(7.38905609893)) {
    std::cout << "test_exp: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_log<32, 8, true, 32, 8, true>(Queue, ac_fixed<32, 8, true>(100), res);
  if (res != ac_fixed<32, 8, true>(4.605170190334320068359375)) {
    std::cout << "test_log: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  if (failed) {
    std::cout << "FAILED\n";
    return -1;
  }

  std::cout << "PASSED\n";
  return 0;
}
