#ifndef __CLANG_CSA_MATH_H
#define __CLANG_CSA_MATH_H

/* This file provides some libm functionality which LLVM cannot currently not
 * lower to CSA instructions. In the long run, this should be replaced by a
 * real math library. Note that these inline asm statements are converted to
 * MachineInstrs dataflow conversion by CSAExpandInlineAsm.  */

#if defined(__cplusplus)
extern "C" {
double modf(double, double *) throw();
float modff(float, float *) throw();
#else
static double modf(double, double *);
static float modff(float, float *);
#endif

__attribute__((always_inline)) inline double modf(double x, double *iptr) {
  double fpart, ipart;
  asm("modf64 %0, %1, %2" : "=d"(fpart), "=d"(ipart) : "d"(x));
  *iptr = ipart;
  return fpart;
}

__attribute__((always_inline)) inline float modff(float x, float *iptr) {
  float fpart, ipart;
  asm("modf32 %0, %1, %2" : "=c"(fpart), "=c"(ipart) : "c"(x));
  *iptr = ipart;
  return fpart;
}

#if defined(__cplusplus)
} // extern "C"
#endif

#endif //__CLANG_CSA_MATH_H
