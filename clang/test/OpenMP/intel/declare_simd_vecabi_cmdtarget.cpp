// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -fvecabi-cmdtarget -target-cpu corei7 "-ax=core-avx2,skylake-avx512" \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s --check-prefix VCHK

// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -fvecabi-cmdtarget -target-cpu corei7 "-ax=core-avx2,skylake-avx512" \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s  --check-prefix DCHK


#pragma omp declare simd ompx_processor(skylake_avx512) notinbranch // 1
#pragma omp declare simd ompx_processor(tigerlake) notinbranch      // 2
#pragma omp declare simd linear(p) notinbranch                      // 3
#pragma omp declare simd uniform(p) notinbranch                     // 4
int func(int* p) { return 1; }

// variants
// VCHK-DAG: _ZGVxN4u__Z4funcPi
// VCHK-DAG: _ZGVYN8u__Z4funcPi
// VCHK-DAG: _ZGVZN16u__Z4funcPi
// VCHK-DAG: _ZGVxN4l4__Z4funcPi
// VCHK-DAG: _ZGVYN8l4__Z4funcPi
// VCHK-DAG: _ZGVZN16l4__Z4funcPi
// VCHK-DAG: _ZGVxN4v__Z4funcPi
// VCHK-DAG: _ZGVYN8v__Z4funcPi
// VCHK-DAG: _ZGVZN16v__Z4funcPi

// dispatch
// DCHK-DAG:_ZGVxN4u__Z4funcPi:core_i7_sse4_2
// DCHK-DAG:_ZGVYN8u__Z4funcPi:haswell
// DCHK-DAG:_ZGVZN16u__Z4funcPi:skylake_avx512
// DCHK-DAG:_ZGVxN4l4__Z4funcPi:core_i7_sse4_2
// DCHK-DAG:_ZGVYN8l4__Z4funcPi:haswell
// DCHK-DAG:_ZGVZN16l4__Z4funcPi:skylake_avx512
// DCHK-DAG:_ZGVxN4v__Z4funcPi:core_i7_sse4_2
// DCHK-DAG:_ZGVYN8v__Z4funcPi:haswell
<<<<<<< HEAD
// DCHK-DAG:_ZGVZN16v__Z4funcPi:{{tigerlake,skylake_avx512|skylake_avx512,tigerlake}}
=======
// DCHK-DAG:_ZGVZN16v__Z4funcPi:skylake_avx512,tigerlake
>>>>>>> d730c179e9ca95d25ecf7788f4c60634e0f94029
