// RUN: %libomptarget-compile-generic
// RUN: env OMP_NUM_TEAMS=1 OMP_TEAMS_THREAD_LIMIT=1 LIBOMPTARGET_INFO=16 \
// RUN:   %libomptarget-run-generic 2>&1 | %fcheck-generic

// INTEL_CUSTOMIZATION
// This test requires next-gen plugin which We do not use
// UNSUPPORTED: x86_64-pc-linux-gnu
// end INTEL_CUSTOMIZATION

#define N 256

int main() {
  // CHECK: Launching kernel [[KERNEL:.+_main_.+]] with 1 blocks and 1 threads
#pragma omp target teams
#pragma omp parallel
  {}
}
