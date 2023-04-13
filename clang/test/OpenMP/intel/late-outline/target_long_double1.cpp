// INTEL_COLLAB

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 \
// RUN:  -D__IMFLONGDOUBLE=80 \
// RUN:  -std=c++17 -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -verify -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 -std=c++17 \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc \
// RUN:  -D__IMFLONGDOUBLE=64 -emit-llvm %s -o %t-targ.ll

// expected-no-diagnostics

// Extracted from float.h
#define  __LIBM_FP64_MIN_SUBNORMAL (((double) 0x1.0p-537) *              \
                                    ((double) 0x1.0p-537))
#define  __LIBM_FP80_MIN_SUBNORMAL (((long double) 0x1.0p-445) *         \
                                    (((((((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)) *   \
                                        (((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500))) *  \
                                       ((((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)) *   \
                                        (((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)))) * \
                                      (((((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)) *   \
                                        (((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500))) *  \
                                       ((((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)) *   \
                                        (((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500))))) *\
                                     ((((((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)) *   \
                                        (((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500))) *  \
                                       ((((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)) *   \
                                        (((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)))) * \
                                      (((((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)) *   \
                                        (((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500))) *  \
                                       ((((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)) *   \
                                        (((long double) 0x1.0p-500) *    \
                                         ((long double) 0x1.0p-500)))))))

#if (__IMFLONGDOUBLE==64)
#define LDBL_TRUE_MIN       __LIBM_FP64_MIN_SUBNORMAL
#elif (__IMFLONGDOUBLE==80)
#define LDBL_TRUE_MIN       __LIBM_FP80_MIN_SUBNORMAL
#endif

static constexpr long double denorm_min() { return __LDBL_DENORM_MIN__; }

static_assert(denorm_min() == LDBL_TRUE_MIN);

// end INTEL_COLLAB
