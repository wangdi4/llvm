// RUN: %clang_cc1 -emit-llvm -o - %s -fintel-compatibility -fopenmp -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s

typedef unsigned short ushort;
typedef ushort ushort8 __attribute__((ext_vector_type(8)));
typedef ushort8 DistSIMDType;

// CHECK-LABEL: foo
void foo()
{
  DistSIMDType distances_local_even[46];
  int i;
  // CHECK: REDUCTION.MIN{{.*}}[46 x <8 x i16>]* %distances_local_even
  #pragma omp parallel reduction(min:distances_local_even)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.MAX{{.*}}[46 x <8 x i16>]* %distances_local_even
  #pragma omp parallel reduction(max:distances_local_even)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.ADD{{.*}}[46 x <8 x i16>]* %distances_local_even
  #pragma omp parallel reduction(+:distances_local_even)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.SUB{{.*}}[46 x <8 x i16>]* %distances_local_even
  #pragma omp parallel reduction(-:distances_local_even)
  for(i=0;i<10;++i) {}
}
