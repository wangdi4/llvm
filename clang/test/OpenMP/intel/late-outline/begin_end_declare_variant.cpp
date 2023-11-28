// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 \
//RUN:  -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-is-device \
//RUN:  -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//expected-no-diagnostics

int is_intel() { return 0; }
#pragma omp begin declare variant match(device={arch(gen)}, implementation={extension(match_any)})
int is_intel() { return 1; }
#pragma omp end declare variant

int is_gpu() { return 0; }
#pragma omp begin declare variant match(device={kind(gpu)}, implementation={extension(match_any)})
int is_gpu() { return 1; }
#pragma omp end declare variant

int main( int argc, char* argv[] )
{
  #pragma omp target
  {
    //CHECK: DIR.OMP.TARGET
    //CHECK: call {{.*}}@"_Z47is_intel$ompvariant$S2$s7$Pgen$S3$s9$Pmatch_anyv"
    //CHECK: call {{.*}}@"_Z45is_gpu$ompvariant$S2$s6$Pgpu$S3$s9$Pmatch_anyv"
    //CHECK: DIR.OMP.END.TARGET
    is_intel();
    is_gpu();
  }
  return 0;
}

// end INTEL_COLLAB
