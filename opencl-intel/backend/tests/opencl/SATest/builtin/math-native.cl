TYPE __attribute__((overloadable)) native_acos(TYPE);
TYPE __attribute__((overloadable)) native_acosh(TYPE);
TYPE __attribute__((overloadable)) native_acospi(TYPE);
TYPE __attribute__((overloadable)) native_asin(TYPE);
TYPE __attribute__((overloadable)) native_asinh(TYPE);
TYPE __attribute__((overloadable)) native_asinpi(TYPE);
TYPE __attribute__((overloadable)) native_atan(TYPE);
TYPE __attribute__((overloadable)) native_atan2(TYPE, TYPE);
TYPE __attribute__((overloadable)) native_atanh(TYPE);
TYPE __attribute__((overloadable)) native_atanpi(TYPE);
TYPE __attribute__((overloadable)) native_atan2pi(TYPE, TYPE);
TYPE __attribute__((overloadable)) native_cbrt(TYPE);
TYPE __attribute__((overloadable)) native_cosh(TYPE);
TYPE __attribute__((overloadable)) native_cospi(TYPE);
TYPE __attribute__((overloadable)) native_erfc(TYPE);
TYPE __attribute__((overloadable)) native_erf(TYPE);
TYPE __attribute__((overloadable)) native_expm1(TYPE);
TYPE __attribute__((overloadable)) native_fdim(TYPE, TYPE);
TYPE __attribute__((overloadable)) native_fmax(TYPE, TYPE);
TYPE __attribute__((overloadable)) native_fmin(TYPE, TYPE);
TYPE __attribute__((overloadable)) native_fmod(TYPE, TYPE);
TYPE __attribute__((overloadable)) native_hypot(TYPE, TYPE);
TYPE __attribute__((overloadable)) native_ilogb(TYPE);
TYPE __attribute__((overloadable)) native_log1p(TYPE);
TYPE __attribute__((overloadable)) native_pow(TYPE, TYPE);
TYPE __attribute__((overloadable)) native_pown(TYPE, int);
TYPE __attribute__((overloadable)) native_rcbrt(TYPE);
TYPE __attribute__((overloadable)) native_rootn(TYPE, int);
TYPE __attribute__((overloadable)) native_sinh(TYPE);
TYPE __attribute__((overloadable)) native_sinpi(TYPE);
TYPE __attribute__((overloadable)) native_tanh(TYPE);
TYPE __attribute__((overloadable)) native_tanpi(TYPE);

kernel void test(global TYPE *A, global TYPE *B, global TYPE *C,
                 global TYPE *dst) {
  size_t i = get_global_id(0);
  dst[i] = native_acos(A[i]) + native_acosh(A[i]) + native_acospi(A[i]) +
           native_asin(A[i]) + native_asinh(A[i]) + native_asinpi(A[i]) +
           native_atan(A[i]) + native_atan2(A[i], B[i]) + native_atanh(A[i]) +
           native_atanpi(A[i]) + native_atan2pi(A[i], B[i]) +
           native_cbrt(A[i]) + native_cos(A[i]) + native_cosh(A[i]) +
           native_cospi(A[i]) + native_divide(A[i], B[i]) + native_erfc(A[i]) +
           native_erf(A[i]) + native_exp(A[i]) + native_exp2(A[i]) +
           native_exp10(A[i]) + native_expm1(A[i]) + native_fdim(A[i], B[i]) +
           native_fmax(A[i], B[i]) + native_fmax(A[i], (TYPE)1.0) +
           native_fmin(A[i], B[i]) + native_fmin(A[i], (TYPE)1.0) +
           native_fmod(A[i], B[i]) + native_hypot(A[i], B[i]) +
           native_ilogb(A[i]) + native_log(A[i]) + native_log2(A[i]) +
           native_log10(A[i]) + native_log1p(A[i]) + native_pow(A[i], B[i]) +
           native_pown(A[i], 1) + native_powr(A[i], B[i]) + native_rcbrt(A[i]) +
           native_recip(A[i]) + native_rootn(A[i], 1) + native_rsqrt(A[i]) +
           native_sin(A[i]) + native_sinh(A[i]) + native_sinpi(A[i]) +
           native_sqrt(A[i]) + native_tan(A[i]) + native_tanh(A[i]) +
           native_tanpi(A[i]);
#ifdef MASKED
  // Add subgroup call in order to enable masked vectorized kernel.
  dst[i] += get_sub_group_size();
#endif
}
