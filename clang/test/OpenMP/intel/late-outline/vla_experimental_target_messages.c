// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc \
//RUN:  -fopenmp-experimental-vlas -DEXP \
//RUN:  -disable-llvm-passes -fopenmp -fopenmp-targets=spir64    \
//RUN:  -fopenmp-late-outline -fintel-compatibility -Werror      \
//RUN:  -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 -emit-llvm -disable-llvm-passes   \
//RUN:  -fopenmp-experimental-vlas -DEXP \
//RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline    \
//RUN:  -fintel-compatibility -fopenmp-is-device -verify -o - %s   \
//RUN:  -fopenmp-host-ir-file-path %t_host.bc -Wsource-uses-openmp

#ifdef EXP
//expected-no-diagnostics
#endif // EXP

#pragma omp begin declare target
void foo(int nx, int ny) {
  float x[nx][ny];  // allowed with experimental flag
}
#pragma omp end declare target
// end INTEL_COLLAB
