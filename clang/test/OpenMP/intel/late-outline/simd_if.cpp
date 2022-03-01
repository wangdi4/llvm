// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -fopenmp-version=51 -triple x86_64-unknown-linux-gnu %s \
// RUN: | FileCheck %s

void bar();

//CHECK: define {{.*}}foo
void foo(int AAA)
{
  //CHECK: [[CMP:%.*]] = icmp {{.*}}123
  //CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.IF"(i1 [[CMP]])
  #pragma omp simd if (AAA > 123)
  for (int I = 0; I < 10; ++I)
    bar();

  //CHECK: [[CMP:%.*]] = icmp {{.*}}124
  //CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.IF"(i1 [[CMP]])
  #pragma omp simd if (simd:AAA > 124)
  for (int I = 0; I < 10; ++I)
    bar();

  //CHECK: [[CMP:%.*]] = icmp {{.*}}125
  //CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.IF"(i1 [[CMP]])
  #pragma omp for simd if (AAA > 125)
  for (int I = 0; I < 10; ++I)
    bar();

  //CHECK: [[CMP:%.*]] = icmp {{.*}}126
  //CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.IF"(i1 [[CMP]])
  #pragma omp for simd if (simd:AAA > 126)
  for (int I = 0; I < 10; ++I)
    bar();

  //CHECK: [[CMP:%.*]] = icmp {{.*}}127
  //CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.IF"(i1
  #pragma omp parallel for simd if (simd:AAA > 127)
  for (int I = 0; I < 10; ++I)
    bar();

  //CHECK: [[SCMP:%.*]] = icmp {{.*}}129
  //CHECK: [[PCMP:%.*]] = icmp {{.*}}128
  //CHECK: "DIR.OMP.PARALLEL.LOOP"{{.*}}"QUAL.OMP.IF"(i1 [[PCMP]])
  //CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.IF"(i1
  #pragma omp parallel for simd if (parallel:AAA > 128) if (simd:AAA > 129)
  for (int I = 0; I < 10; ++I)
    bar();

  //CHECK: [[SCMP:%.*]] = icmp {{.*}}130
  //CHECK: "DIR.OMP.PARALLEL.LOOP"
  //CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.IF"(i1
  #pragma omp parallel for simd if (simd:AAA > 130)
  for (int I = 0; I < 10; ++I)
    bar();

  //CHECK: [[PCMP:%.*]] = icmp {{.*}}131
  //CHECK: "DIR.OMP.PARALLEL.LOOP"{{.*}}"QUAL.OMP.IF"(i1 [[PCMP]])
  //CHECK: "DIR.OMP.SIMD"
  #pragma omp parallel for simd if (parallel:AAA > 131)
  for (int I = 0; I < 10; ++I)
    bar();

  //CHECK: [[PCMP:%.*]] = icmp {{.*}}132
  //CHECK: "DIR.OMP.PARALLEL.LOOP"{{.*}}"QUAL.OMP.IF"(i1
  //CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.IF"(i1
  #pragma omp parallel for simd if (AAA > 132)
  for (int I = 0; I < 10; ++I)
    bar();
}
// end INTEL_COLLAB
