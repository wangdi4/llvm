kernel void test(global TYPE *A, global TYPE *B, global TYPE *C,
                 global TYPE *dst) {
  size_t i = get_global_id(0);
  dst[i] = acos(A[i]) + acosh(A[i]) + acospi(A[i]) + asin(A[i]) + asinh(A[i]) +
           asinpi(A[i]) + atan(A[i]) + atan2(A[i], B[i]) + atanh(A[i]) +
           atanpi(A[i]) + atan2pi(A[i], B[i]) + cbrt(A[i]) + ceil(A[i]) +
           copysign(A[i], B[i]) + cos(A[i]) + cosh(A[i]) + cospi(A[i]) +
           erfc(A[i]) + erf(A[i]) + exp(A[i]) + exp2(A[i]) + exp10(A[i]) +
           expm1(A[i]) + fabs(A[i]) + fdim(A[i], B[i]) + floor(A[i]) +
           fma(A[i], B[i], C[i]) + fmax(A[i], B[i]) + fmax(A[i], (TYPE)1.0) +
           fmin(A[i], B[i]) + fmin(A[i], (TYPE)1.0) + fmod(A[i], B[i]) +
           hypot(A[i], B[i]) + ilogb(A[i]) + ldexp(A[i], 1) + lgamma(A[i]) +
           log(A[i]) + log2(A[i]) + log10(A[i]) + log1p(A[i]) + logb(A[i]) +
           mad(A[i], B[i], C[i]) + maxmag(A[i], B[i]) + minmag(A[i], B[i]) +
           nextafter(A[i], B[i]) + pow(A[i], B[i]) + pown(A[i], 1) +
           powr(A[i], B[i]) + remainder(A[i], B[i]) + rootn(A[i], 1) +
           round(A[i]) + rsqrt(A[i]) + sin(A[i]) + sinh(A[i]) + sinpi(A[i]) +
           sqrt(A[i]) + tan(A[i]) + tanh(A[i]) + tanpi(A[i]) + tgamma(A[i]) +
           trunc(A[i]);
}
