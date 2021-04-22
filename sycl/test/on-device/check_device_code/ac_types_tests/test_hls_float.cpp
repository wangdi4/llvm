// RUN: %clangxx -fsycl -fsycl-targets=spir64_fpga-unknown-unknown-sycldevice %s -o %t.out
// env SYCL_DEVICE_TYPE=HOST %t.out
// RUN: %ACC_RUN_PLACEHOLDER %t.out

#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/ac_types/hls_float.hpp>
#include <CL/sycl/INTEL/ac_types/hls_float_math.hpp>
#include <CL/sycl/INTEL/fpga_extensions.hpp>

using namespace cl::sycl;

constexpr access::mode kSyclRead = access::mode::read;
constexpr access::mode kSyclWrite = access::mode::write;
constexpr access::mode kSyclReadWrite = access::mode::read_write;

template <int E, int M, int E2, int M2, int E3, int M3>
void test_add(queue &Queue, const ihc::hls_float<E, M> &a,
              const ihc::hls_float<E2, M2> &b, ihc::hls_float<E3, M3> &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<ihc::hls_float<E3, M3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class add>([=] { res[0] = x[0] + y[0]; });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2, int E3, int M3>
void test_sub(queue &Queue, const ihc::hls_float<E, M> &a,
              const ihc::hls_float<E2, M2> &b, ihc::hls_float<E3, M3> &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<ihc::hls_float<E3, M3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sub>([=] { res[0] = x[0] - y[0]; });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2, int E3, int M3>
void test_mult(queue &Queue, const ihc::hls_float<E, M> &a,
               const ihc::hls_float<E2, M2> &b, ihc::hls_float<E3, M3> &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<ihc::hls_float<E3, M3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class mult>([=] { res[0] = x[0] * y[0]; });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2, int E3, int M3>
void test_div(queue &Queue, const ihc::hls_float<E, M> &a,
              const ihc::hls_float<E2, M2> &b, ihc::hls_float<E3, M3> &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<ihc::hls_float<E3, M3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class div>([=] { res[0] = x[0] / y[0]; });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_gt(queue &Queue, const ihc::hls_float<E, M> &a,
             const ihc::hls_float<E2, M2> &b, bool &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class gt>([=] { res[0] = x[0] > y[0]; });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_lt(queue &Queue, const ihc::hls_float<E, M> &a,
             const ihc::hls_float<E2, M2> &b, bool &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class lt>([=] { res[0] = x[0] < y[0]; });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_ge(queue &Queue, const ihc::hls_float<E, M> &a,
             const ihc::hls_float<E2, M2> &b, bool &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class ge>([=] { res[0] = x[0] >= y[0]; });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_le(queue &Queue, const ihc::hls_float<E, M> &a,
             const ihc::hls_float<E2, M2> &b, bool &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class le>([=] { res[0] = x[0] <= y[0]; });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_eq(queue &Queue, const ihc::hls_float<E, M> &a,
             const ihc::hls_float<E2, M2> &b, bool &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<bool, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class eq>([=] { res[0] = x[0] == y[0]; });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_sqrt(queue &Queue, const ihc::hls_float<E, M> &a,
               ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sqrt>([=] { res[0] = ihc_sqrt(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_cbrt(queue &Queue, const ihc::hls_float<E, M> &a,
               ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class cbrt>([=] { res[0] = ihc_cbrt(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_recip(queue &Queue, const ihc::hls_float<E, M> &a,
                ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class recip>([=] { res[0] = ihc_recip(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_rsqrt(queue &Queue, const ihc::hls_float<E, M> &a,
                ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class rsqrt>([=] { res[0] = ihc_rsqrt(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2, int E3, int M3>
void test_hypot(queue &Queue, const ihc::hls_float<E, M> &a,
                const ihc::hls_float<E2, M2> &b, ihc::hls_float<E3, M3> &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<ihc::hls_float<E3, M3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class hypot>([=] { res[0] = ihc_hypot(x[0], y[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_exp(queue &Queue, const ihc::hls_float<E, M> &a,
              ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class exp>([=] { res[0] = ihc_exp(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_exp2(queue &Queue, const ihc::hls_float<E, M> &a,
               ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class exp2>([=] { res[0] = ihc_exp2(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_exp10(queue &Queue, const ihc::hls_float<E, M> &a,
                ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class exp10>([=] { res[0] = ihc_exp10(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_expm1(queue &Queue, const ihc::hls_float<E, M> &a,
                ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class expm1>([=] { res[0] = ihc_expm1(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_log(queue &Queue, const ihc::hls_float<E, M> &a,
              ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class log>([=] { res[0] = ihc_log(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_log2(queue &Queue, const ihc::hls_float<E, M> &a,
               ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class log2>([=] { res[0] = ihc_log2(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_log10(queue &Queue, const ihc::hls_float<E, M> &a,
                ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class log10>([=] { res[0] = ihc_log10(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_log1p(queue &Queue, const ihc::hls_float<E, M> &a,
                ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class log1p>([=] { res[0] = ihc_log1p(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2, int E3, int M3>
void test_pow(queue &Queue, const ihc::hls_float<E, M> &a,
              const ihc::hls_float<E2, M2> &b, ihc::hls_float<E3, M3> &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<ihc::hls_float<E3, M3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class pow>([=] { res[0] = ihc_pow(x[0], y[0]); });
  });
  Queue.wait();
}

template <int E, int M, int W, int S, int E3, int M3>
void test_pown(queue &Queue, const ihc::hls_float<E, M> &a,
               const ac_int<W, S> &b, ihc::hls_float<E3, M3> &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ac_int<W, S>, 1> inp2(&b, 1);
  buffer<ihc::hls_float<E3, M3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class pown>([=] { res[0] = ihc_pown(x[0], y[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E3, int M3>
void test_pown2(queue &Queue, const ihc::hls_float<E, M> &a, const int &b,
                ihc::hls_float<E3, M3> &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<int, 1> inp2(&b, 1);
  buffer<ihc::hls_float<E3, M3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class pown2>([=] { res[0] = ihc_pown(x[0], y[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_sin(queue &Queue, const ihc::hls_float<E, M> &a,
              ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sin>([=] { res[0] = ihc_sin(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_sinpi(queue &Queue, const ihc::hls_float<E, M> &a,
                ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class sinpi>([=] { res[0] = ihc_sinpi(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_cos(queue &Queue, const ihc::hls_float<E, M> &a,
              ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class cos>([=] { res[0] = ihc_cos(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_cospi(queue &Queue, const ihc::hls_float<E, M> &a,
                ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class cospi>([=] { res[0] = ihc_cospi(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2, int E3, int M3>
void test_sincos(queue &Queue, const ihc::hls_float<E, M> &a,
                 ihc::hls_float<E2, M2> &res_sin_arg,
                 ihc::hls_float<E3, M3> &res_cos_arg) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> res_sin(&res_sin_arg, 1);
  buffer<ihc::hls_float<E3, M3>, 1> res_cos(&res_cos_arg, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res_sin_ = res_sin.template get_access<kSyclWrite>(h);
    auto res_cos_ = res_cos.template get_access<kSyclWrite>(h);
    h.single_task<class sincos>(
        [=] { res_sin_[0] = ihc_sincos(x[0], res_cos_[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2, int E3, int M3>
void test_sincospi(queue &Queue, const ihc::hls_float<E, M> &a,
                   ihc::hls_float<E2, M2> &res_sin_arg,
                   ihc::hls_float<E3, M3> &res_cos_arg) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> res_sin(&res_sin_arg, 1);
  buffer<ihc::hls_float<E3, M3>, 1> res_cos(&res_cos_arg, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res_sin_ = res_sin.template get_access<kSyclWrite>(h);
    auto res_cos_ = res_cos.template get_access<kSyclWrite>(h);
    h.single_task<class sincospi>(
        [=] { res_sin_[0] = ihc_sincospi(x[0], res_cos_[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_asin(queue &Queue, const ihc::hls_float<E, M> &a,
               ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class asin>([=] { res[0] = ihc_asin(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_asinpi(queue &Queue, const ihc::hls_float<E, M> &a,
                 ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class asinpi>([=] { res[0] = ihc_asinpi(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_acos(queue &Queue, const ihc::hls_float<E, M> &a,
               ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class acos>([=] { res[0] = ihc_acos(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_acospi(queue &Queue, const ihc::hls_float<E, M> &a,
                 ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class acospi>([=] { res[0] = ihc_acospi(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_atan(queue &Queue, const ihc::hls_float<E, M> &a,
               ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class atan>([=] { res[0] = ihc_atan(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2>
void test_atanpi(queue &Queue, const ihc::hls_float<E, M> &a,
                 ihc::hls_float<E2, M2> &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class atanpi>([=] { res[0] = ihc_atanpi(x[0]); });
  });
  Queue.wait();
}

template <int E, int M, int E2, int M2, int E3, int M3>
void test_atan2(queue &Queue, const ihc::hls_float<E, M> &a,
                const ihc::hls_float<E2, M2> &b, ihc::hls_float<E3, M3> &c) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<ihc::hls_float<E2, M2>, 1> inp2(&b, 1);
  buffer<ihc::hls_float<E3, M3>, 1> result(&c, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto y = inp2.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class atan2>([=] { res[0] = ihc_atan2(x[0], y[0]); });
  });
  Queue.wait();
}

template <int E, int M>
void test_cast_from_int(queue &Queue, const int &a, ihc::hls_float<E, M> &b) {
  buffer<int, 1> inp(&a, 1);
  buffer<ihc::hls_float<E, M>, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class cast_from_int>(
        [=] { res[0] = (ihc::hls_float<E, M>)x[0]; });
  });
  Queue.wait();
}

template <int E, int M>
void test_cast_to_int(queue &Queue, const ihc::hls_float<E, M> &a, int &b) {
  buffer<ihc::hls_float<E, M>, 1> inp(&a, 1);
  buffer<int, 1> result(&b, 1);

  Queue.submit([&](handler &h) {
    auto x = inp.template get_access<kSyclRead>(h);
    auto res = result.template get_access<kSyclWrite>(h);
    h.single_task<class cast_to_int>([=] { res[0] = (int)x[0]; });
  });
  Queue.wait();
}

int main() {
  queue Queue(cl::sycl::INTEL::fpga_emulator_selector{});

  ihc::hls_float<8, 7> res;
  bool failed = false;

  res = 0;
  test_add<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(10),
                       ihc::hls_float<8, 7>(10), res);
  if (res != ihc::hls_float<8, 7>(20)) {
    std::cout << "test_add: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_sub<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(10), ihc::hls_float<8, 7>(5),
                       res);
  if (res != ihc::hls_float<8, 7>(5)) {
    std::cout << "test_sub: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_mult<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(2), ihc::hls_float<8, 7>(3),
                        res);
  if (res != ihc::hls_float<8, 7>(6)) {
    std::cout << "test_mult: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_div<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(10), ihc::hls_float<8, 7>(5),
                       res);
  if (res != ihc::hls_float<8, 7>(2)) {
    std::cout << "test_div: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  bool comparison_op_test_res = false;
  test_gt<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(10), ihc::hls_float<8, 7>(5),
                      comparison_op_test_res);
  if (comparison_op_test_res != true) {
    std::cout << "test_gt: FAILED"
              << "\n";
    std::cout << "res: " << comparison_op_test_res << "\n";
    failed = true;
  }

  comparison_op_test_res = false;
  test_lt<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(5), ihc::hls_float<8, 7>(15),
                      comparison_op_test_res);
  if (comparison_op_test_res != true) {
    std::cout << "test_lt: FAILED"
              << "\n";
    std::cout << "res: " << comparison_op_test_res << "\n";
    failed = true;
  }

  comparison_op_test_res = false;
  test_ge<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(15), ihc::hls_float<8, 7>(10),
                      comparison_op_test_res);
  if (comparison_op_test_res != true) {
    std::cout << "test_ge: FAILED"
              << "\n";
    std::cout << "res: " << comparison_op_test_res << "\n";
    failed = true;
  }

  comparison_op_test_res = false;
  test_le<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(15), ihc::hls_float<8, 7>(25),
                      comparison_op_test_res);
  if (comparison_op_test_res != true) {
    std::cout << "test_le: FAILED"
              << "\n";
    std::cout << "res: " << comparison_op_test_res << "\n";
    failed = true;
  }

  comparison_op_test_res = false;
  test_eq<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(15), ihc::hls_float<8, 7>(15),
                      comparison_op_test_res);
  if (comparison_op_test_res != true) {
    std::cout << "test_eq: FAILED"
              << "\n";
    std::cout << "res: " << comparison_op_test_res << "\n";
    failed = true;
  }

  res = 0;
  test_sqrt<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(144), res);
  if (res != ihc::hls_float<8, 7>(12)) {
    std::cout << "test_sqrt: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_cbrt<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(27), res);
  if (res != ihc::hls_float<8, 7>(3)) {
    std::cout << "test_cbrt: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_recip<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(10), res);
  if (res != ihc::hls_float<8, 7>(0.1)) {
    std::cout << "test_recip: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_rsqrt<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(100), res);
  if (res != ihc::hls_float<8, 7>(0.1)) {
    std::cout << "test_rsqrt: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_hypot<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(3),
                         ihc::hls_float<8, 7>(4), res);
  if (res != ihc::hls_float<8, 7>(5)) {
    std::cout << "test_hypot: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_exp<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(2), res);
  if (res != ihc::hls_float<8, 7>(7.3750)) {
    std::cout << "test_exp: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_exp2<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(2), res);
  if (res != ihc::hls_float<8, 7>(4)) {
    std::cout << "test_exp2: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_exp10<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(2), res);
  if (res != ihc::hls_float<8, 7>(100)) {
    std::cout << "test_exp10: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_expm1<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(2), res);
  if (res != ihc::hls_float<8, 7>(6.3750)) {
    std::cout << "test_expm1: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_log<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(100), res);
  if (res != ihc::hls_float<8, 7>(4.593750)) {
    std::cout << "test_log: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_log2<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(100), res);
  if (res != ihc::hls_float<8, 7>(6.656250)) {
    std::cout << "test_log2: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_log10<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(100), res);
  if (res != ihc::hls_float<8, 7>(2.0)) {
    std::cout << "test_log10: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_log1p<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(100), res);
  if (res != ihc::hls_float<8, 7>(4.6250)) {
    std::cout << "test_log1p: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_pow<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(2), ihc::hls_float<8, 7>(2),
                       res);
  if (res != ihc::hls_float<8, 7>(4)) {
    std::cout << "test_pow: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_pown<8, 7, 16, true>(Queue, ihc::hls_float<8, 7>(2), ac_int<16, true>(2),
                            res);
  if (res != ihc::hls_float<8, 7>(4)) {
    std::cout << "test_pown: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_pown2<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(2), 2, res);
  if (res != ihc::hls_float<8, 7>(4)) {
    std::cout << "test_pown2: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_sin<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(100), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(-0.5078125)) {
    std::cout << "test_sin: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_sinpi<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(1.8), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(-0.593750)) {
    std::cout << "test_sinpi: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_cos<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(100), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(0.86328125)) {
    std::cout << "test_cos: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_cospi<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(1.8), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(0.80468750)) {
    std::cout << "test_cospi: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  ihc::hls_float<8, 7> res_sin = 0, res_cos = 0;
  test_sincos<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(1.0472), res_sin,
                          res_cos);
  if (res_sin.is_nan() || res_cos.is_nan() ||
      res_sin != ihc::hls_float<8, 7>(0.86718750) ||
      res_cos != ihc::hls_float<8, 7>(0.5)) {
    std::cout << "test_sincos: FAILED"
              << "\n";
    std::cout << "res_sin: " << res_sin << "\n";
    std::cout << "res_cos: " << res_cos << "\n";
    failed = true;
  }

  res_sin = 0;
  res_cos = 0;
  test_sincospi<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(1.8), res_sin, res_cos);
  if (res_sin.is_nan() || res_cos.is_nan() ||
      res_sin != ihc::hls_float<8, 7>(-0.593750) ||
      res_cos != ihc::hls_float<8, 7>(0.80468750)) {
    std::cout << "test_sincospi: FAILED"
              << "\n";
    std::cout << "res_sin: " << res_sin << "\n";
    std::cout << "res_cos: " << res_cos << "\n";
    failed = true;
  }

  res = 0;
  test_asin<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(1), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(1.5703125)) {
    std::cout << "test_asin: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_asinpi<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(1), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(0.50)) {
    std::cout << "test_asinpi: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_acos<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(0.5), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(1.0468750)) {
    std::cout << "test_acos: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_acospi<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(0.5), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(0.33398437)) {
    std::cout << "test_acospi: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_atan<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(100), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(1.56250)) {
    std::cout << "test_atan: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_atanpi<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(100), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(0.49804687)) {
    std::cout << "test_atanpi: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  res = 0;
  test_atan2<8, 7, 8, 7>(Queue, ihc::hls_float<8, 7>(2),
                         ihc::hls_float<8, 7>(4), res);
  if (res.is_nan() || res != ihc::hls_float<8, 7>(0.46289062)) {
    std::cout << "test_atan2: FAILED"
              << "\n";
    std::cout << "res: " << res << "\n";
    failed = true;
  }

  int res_int = 0;
  test_cast_to_int<8, 7>(Queue, ihc::hls_float<8, 7>(2), res_int);
  if (res_int != 2) {
    std::cout << "test_cast_to_int: FAILED"
              << "\n";
    std::cout << "res: " << res_int << "\n";
    failed = true;
  }

  res = 0;
  test_cast_from_int<8, 7>(Queue, 2, res);
  if (res != ihc::hls_float<8, 7>(2)) {
    std::cout << "test_cast_from_int: FAILED"
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
