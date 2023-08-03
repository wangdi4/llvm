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

#pragma omp declare simd simdlen(4) ompx_processor(skylake_avx512) notinbranch
#pragma omp declare simd simdlen(8) ompx_processor(tigerlake)
int FindPosition(double x) { return 0; }


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

// VCHK-DAG: _ZGVZN4v__Z12FindPositiond
// VCHK-DAG: _ZGVZM8v__Z12FindPositiond
// VCHK-DAG: _ZGVZN8v__Z12FindPositiond

// dispatch
// DCHK-DAG:_ZGVxN4u__Z4funcPi:corei7
// DCHK-DAG:_ZGVYN8u__Z4funcPi:core-avx2
// DCHK-DAG:_ZGVZN16u__Z4funcPi:skylake-avx512
// DCHK-DAG:_ZGVxN4l4__Z4funcPi:corei7
// DCHK-DAG:_ZGVYN8l4__Z4funcPi:core-avx2
// DCHK-DAG:_ZGVZN16l4__Z4funcPi:skylake-avx512
// DCHK-DAG:_ZGVxN4v__Z4funcPi:corei7
// DCHK-DAG:_ZGVYN8v__Z4funcPi:core-avx2
// DCHK-DAG:_ZGVZN16v__Z4funcPi:skylake-avx512,skylake_avx512,tigerlake

// DCHK-DAG: _ZGVxN8v__Z12FindPositiond:corei7
// DCHK-DAG: _ZGVxM8v__Z12FindPositiond:corei7
// DCHK-DAG: _ZGVYN8v__Z12FindPositiond:core-avx2
// DCHK-DAG: _ZGVYM8v__Z12FindPositiond:core-avx2
// DCHK-DAG: _ZGVZN8v__Z12FindPositiond:skylake-avx512,tigerlake
// DCHK-DAG: _ZGVZM8v__Z12FindPositiond:skylake-avx512,tigerlake

// DCHK-DAG: _ZGVxN4v__Z12FindPositiond:corei7
// DCHK-DAG: _ZGVYN4v__Z12FindPositiond:core-avx2
// DCHK-DAG: _ZGVZN4v__Z12FindPositiond:skylake-avx512
