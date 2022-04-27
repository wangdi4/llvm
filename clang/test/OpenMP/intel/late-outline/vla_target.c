// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc \
//RUN:  -disable-llvm-passes -fopenmp -fopenmp-targets=spir64    \
//RUN:  -fopenmp-late-outline -fintel-compatibility -Werror      \
//RUN:  -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 -emit-llvm -disable-llvm-passes   \
//RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline     \
//RUN:  -fintel-compatibility -fopenmp-is-device         -o - %s   \
//RUN:  -fopenmp-host-ir-file-path %t_host.bc -Wsource-uses-openmp \
//RUN:  | FileCheck %s

void foo(int nx, int ny, float* restrict array) {
  float (* restrict xp)[nx][ny] = (void*)array;

  //CHECK: TEAMS{{.*}}FIRSTPRIVATE{{.*}}xp
  #pragma omp target
  #pragma omp teams firstprivate (xp) // ok, because it's just firstprivatizing
                                      // a pointer, so no pointee memory
                                      // allocation is involved.
  {
    xp[0][0][0] = 1.0f;
  }

  //CHECK: TEAMS{{.*}}FIRSTPRIVATE{{.*}}xp
  #pragma omp target
  #pragma omp teams distribute firstprivate (xp)
  for (int i=0; i<nx; ++i)
  {
    xp[i][0][0] = 1.0f;
  }

  //CHECK: TEAMS{{.*}}FIRSTPRIVATE{{.*}}xp
  #pragma omp target
  #pragma omp teams distribute simd firstprivate (xp)
  for (int i=0; i<nx; ++i)
  {
    xp[i][0][0] = 1.0f;
  }

  //CHECK: TEAMS{{.*}}FIRSTPRIVATE{{.*}}xp
  #pragma omp target
  #pragma omp teams distribute parallel for simd firstprivate (xp)
  for (int i=0; i<nx; ++i)
  {
    xp[i][0][0] = 1.0f;
  }

  //CHECK: TEAMS{{.*}}FIRSTPRIVATE{{.*}}xp
  #pragma omp target
  #pragma omp teams distribute parallel for firstprivate (xp)
  for (int i=0; i<nx; ++i)
  {
    xp[i][0][0] = 1.0f;
  }

  //CHECK: TEAMS{{.*}}FIRSTPRIVATE{{.*}}xp
  #pragma omp target
  #pragma omp teams loop firstprivate (xp)
  for (int i=0; i<nx; ++i)
  {
    xp[i][0][0] = 1.0f;
  }
}
// end INTEL_COLLAB
