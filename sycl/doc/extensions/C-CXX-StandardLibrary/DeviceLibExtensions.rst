Device library extensions
===================================

Device compiler that indicates support for a particular extension is
supposed to support *all* the corresponding functions.

cl_intel_devicelib_cassert
==========================

.. code:
   void __devicelib_assert_fail(__generic const char *expr,
                                __generic const char *file,
                                int32_t line,
                                __generic const char *func,
                                size_t gid0, size_t gid1, size_t gid2,
                                size_t lid0, size_t lid1, size_t lid2);
Semantic:
the function is called when an assertion expression `expr` is false,
and it indicates that a program does not execute as expected.
The function should print a message containing the information
provided in the arguments. In addition to that, the function is free
to terminate the current kernel invocation.

Arguments:

  - `expr` is a string representation of the assertion condition
  - `file` and `line` are the source code location of the assertion
  - `func` (optional, may be NULL)  name of the function containing the assertion
  - `gidX` current work-item global id
  - `lidX` current work-item local id

Example of a message:
.. code:
   foo.cpp:42: void foo(int): global id: [0,0,0], local id: [0,0,0] Assertion `buf[wiID] == 0 && "Invalid value"` failed.

cl_intel_devicelib_math
==========================

.. code:
   double __devicelib_log(double x);
   float  __devicelib_logf(float x);
   double __devicelib_sin(double x);
   float  __devicelib_sinf(float x);
   double __devicelib_cos(double x);
   float  __devicelib_cosf(float x);
   double __devicelib_tan(double x);
   float  __devicelib_tanf(float x);
   double __devicelib_acos(double x);
   float  __devicelib_acosf(float x);
   double __devicelib_pow(double x, double y);
   float  __devicelib_powf(float x, float y);
   double __devicelib_sqrt(double x);
   float  __devicelib_sqrtf(float x);
   double __devicelib_cbrt(double x);
   float  __devicelib_cbrtf(float x);
   double __devicelib_hypot(double x, double y);
   float  __devicelib_hypotf(float x, float y);
   double __devicelib_erf(double x);
   float  __devicelib_erff(float x);
   double __devicelib_erfc(double x);
   float  __devicelib_erfcf(float x);
   double __devicelib_tgamma(double x);
   float  __devicelib_tgammaf(float x);
   double __devicelib_lgamma(double x);
   float  __devicelib_lgammaf(float x);
   double __devicelib_fmod(double x, double y);
   float  __devicelib_fmodf(float x, float y);
   double __devicelib_remainder(double x, double y);
   float  __devicelib_remainderf(float x, float y);
   double __devicelib_remquo(double x, double y, int *q);
   float  __devicelib_remquof(float x, float y, int *q);
   double __devicelib_nextafter(double x, double y);
   float  __devicelib_nextafterf(float x, float y);
   double __devicelib_fdim(double x, double y);
   float  __devicelib_fdimf(float x, float y);
   double __devicelib_fma(double x, double y, double z);
   float  __devicelib_fmaf(float x, float y, float z);
   float  __devicelib_asinf(float x);
   double __devicelib_asin(double x);
   float  __devicelib_atanf(float x);
   double __devicelib_atan(double x);
   float  __devicelib_atan2f(float x, float y);
   double __devicelib_atan2(double x, double y);
   float  __devicelib_coshf(float x);
   double __devicelib_cosh(double x);
   float  __devicelib_sinhf(float x);
   double __devicelib_sinh(double x);
   float  __devicelib_tanhf(float x);
   double __devicelib_tanh(double x);
   float  __devicelib_acoshf(float x);
   double __devicelib_acosh(double x);
   float  __devicelib_asinhf(float x);
   double __devicelib_asinh(double x);
   float  __devicelib_atanhf(float x);
   double __devicelib_atanh(double x);
   float  __devicelib_frexpf(float x, int *exp);
   double __devicelib_frexp(double x, int *exp);
   float  __devicelib_ldexpf(float x, int exp);
   double __devicelib_ldexp(double x, int exp);
   float  __devicelib_log10f(float x);
   double __devicelib_log10(double x);
   float  __devicelib_modff(float x, float *intpart);
   double __devicelib_modf(double x, double *intpart);
   float  __devicelib_expf(float x);
   double __devicelib_exp(double x);
   float  __devicelib_exp2f(float x);
   double __devicelib_exp2(double x);
   float  __devicelib_expm1f(float x);
   double __devicelib_expm1(double x);
   int    __devicelib_ilogbf(float x);
   int    __devicelib_ilogb(double x);
   float  __devicelib_log1pf(float x);
   double __devicelib_log1p(double x);
   float  __devicelib_log2f(float x);
   double __devicelib_log2(double x);
   float  __devicelib_logbf(float x);
   double __devicelib_logb(double x);

Semantic:
Those __devicelib_* functions have the same semantic as corresponding math functions
from <math.h> or <cmath>. Please refer to: http://www.cplusplus.com/reference/cmath/
for details.

Arguments:
Those __devicelib_* functions have the same argument type and return type as corresponding
math functions from <math.h> or <cmath>.

Note:
Currently, not all functions from <math.h> or <cmath> are supported in this extension. Following
functions will be supported in the future:
.. code:
   int    __devicelib_abs(int x);
   float  __devicelib_ceilf(float x);
   double __devicelib_ceil(double x);
   float  __devicelib_copysignf(float x, float y);
   double __devicelib_copysign(double x, double y);
   float  __devicelib_fabsf(float x);
   double __devicelib_fabs(double x);
   float  __devicelib_floorf(float x);
   double __devicelib_floor(double x);
   float  __devicelib_fmaxf(float x, float y);
   double __devicelib_fmax(double x, double y);
   float  __devicelib_fminf(float x, float y);
   double __devicelib_fmin(double x, double y);
   float  __devicelib_nextafterf(float x, float y);
   double __devicelib_nextafter(double x, double y);
   float  __devicelib_rintf(float x);
   double __devicelib_rint(double x);
   float  __devicelib_roundf(float x);
   double __devicelib_round(double x);
   float  __devicelib_truncf(float x);
   double __devicelib_trunc(double x);

cl_intel_devicelib_complex
==========================

.. code:
   double __devicelib_cimag(double __complex__ z);
   float  __devicelib_cimagf(float __complex__ z);
   double __devicelib_creal(double __complex__ z);
   float  __devicelib_crealf(float __complex__ z);
   double __devicelib_carg(double __complex__ z);
   float  __devicelib_cargf(float __complex__ z);
   double __devicelib_cabs(double __complex__ z);
   float  __devicelib_cabsf(float __complex__ z);
   double __complex__ __devicelib_cproj(double __complex__ z);
   float  __complex__ __devicelib_cprojf(float __complex__ z);
   double __complex__ __devicelib_cexp(double __complex__ z);
   float  __complex__ __devicelib_cexpf(float __complex__ z);
   double __complex__ __devicelib_clog(double __complex__ z);
   float  __complex__ __devicelib_clogf(float __complex__ z);
   double __complex__ __devicelib_cpow(double __complex__ x, double __complex__ y);
   float  __complex__ __devicelib_cpowf(float __complex__ x, float __complex__ y);
   double __complex__ __devicelib_cpolar(double x, double y);
   float  __complex__ __devicelib_cpolarf(float x, float y);
   double __complex__ __devicelib_csqrt(double __complex__ z);
   float  __complex__ __devicelib_csqrtf(float __complex__ z);
   double __complex__ __devicelib_csinh(double __complex__ z);
   float  __complex__ __devicelib_csinhf(float __complex__ z);
   double __complex__ __devicelib_ccosh(double __complex__ z);
   float  __complex__ __devicelib_ccoshf(float __complex__ z);
   double __complex__ __devicelib_ctanh(double __complex__ z);
   float  __complex__ __devicelib_ctanhf(float __complex__ z);
   double __complex__ __devicelib_csin(double __complex__ z);
   float  __complex__ __devicelib_csinf(float __complex__ z);
   double __complex__ __devicelib_ccos(double __complex__ z);
   float  __complex__ __devicelib_ccosf(float __complex__ z);
   double __complex__ __devicelib_ctan(double __complex__ z);
   float  __complex__ __devicelib_ctanf(float __complex__ z);
   double __complex__ __devicelib_cacos(double __complex__ z);
   float  __complex__ __devicelib_cacosf(float __complex__ z);
   double __complex__ __devicelib_casinh(double __complex__ z);
   float  __complex__ __devicelib_casinhf(float __complex__ z);
   double __complex__ __devicelib_casin(double __complex__ z);
   float  __complex__ __devicelib_casinf(float __complex__ z);
   double __complex__ __devicelib_cacosh(double __complex__ z);
   float  __complex__ __devicelib_cacoshf(float __complex__ z);
   double __complex__ __devicelib_catanh(double __complex__ z);
   float  __complex__ __devicelib_catanhf(float __complex__ z);
   double __complex__ __devicelib_catan(double __complex__ z);
   float  __complex__ __devicelib_catanf(float __complex__ z);
   double __complex__ __devicelib_muldc3(double a, double b, double c, double d);
   float  __complex__ __devicelib_mulsc3(float a, float b, float c, float d);
   double __complex__ __devicelib_divdc3(double a, double b, double c, double d);
   float  __complex__ __devicelib_divsc3(float a, float b, float c, float d);

Semantic:
Those __devicelib_* functions have the same semantic as corresponding complex math functions
from <complex.h>. Please refer to: https://en.cppreference.com/w/c/numeric/complex for details.

Arguments:
Those __devicelib_* functions have the same argument type and return type as corresponding
complex math functions from <complex.h>. The "float __complex__" or "double __complex__" type
is C99 complex type and it is an alias to "struct {float, float}" or "struct {double, double}"
in LLVM IR and SPIR-V.
