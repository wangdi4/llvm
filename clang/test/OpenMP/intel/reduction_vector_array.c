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
// CHECK-LABEL: foo1
void foo1()
{
  int i;
  DistSIMDType arr_of_vec[46];
  DistSIMDType vec;
  ushort arr[6];
  ushort elem;
  signed short selem;
  // CHECK: REDUCTION.MIN:UNSIGNED{{.*}}arr_of_vec
  #pragma omp parallel reduction(min:arr_of_vec)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.MIN:UNSIGNED{{.*}}vec
  #pragma omp parallel reduction(min:vec)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.MIN:UNSIGNED{{.*}}arr
  #pragma omp parallel reduction(min:arr)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.MIN:UNSIGNED{{.*}}elem
  #pragma omp parallel reduction(min:elem)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.MAX:UNSIGNED{{.*}}arr_of_vec
  #pragma omp parallel reduction(max:arr_of_vec)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.MAX:UNSIGNED{{.*}}vec
  #pragma omp parallel reduction(max:vec)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.MAX:UNSIGNED{{.*}}arr
  #pragma omp parallel reduction(max:arr)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.MAX:UNSIGNED{{.*}}elem
  #pragma omp parallel reduction(max:elem)
  for(i=0;i<10;++i) {}
  // CHECK-NOT: REDUCTION.MIN:UNSIGNED{{.*}}selem
  #pragma omp parallel reduction(min:selem)
  for(i=0;i<10;++i) {}
  // CHECK-NOT: REDUCTION.MAX:UNSIGNED{{.*}}selem
  #pragma omp parallel reduction(max:selem)
  for(i=0;i<10;++i) {}
}
