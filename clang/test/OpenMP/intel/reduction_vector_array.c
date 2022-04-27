// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fintel-compatibility -fopenmp \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s

typedef unsigned short ushort;
typedef ushort ushort8 __attribute__((ext_vector_type(8)));
typedef ushort8 DistSIMDType;

// CHECK-LABEL: foo
void foo()
{
  DistSIMDType distances_local_even[46];
  int i;
  // CHECK: REDUCTION.MIN{{.*}}ptr %distances_local_even
  #pragma omp parallel reduction(min:distances_local_even)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.MAX{{.*}}ptr %distances_local_even
  #pragma omp parallel reduction(max:distances_local_even)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.ADD{{.*}}ptr %distances_local_even
  #pragma omp parallel reduction(+:distances_local_even)
  for(i=0;i<10;++i) {}
  // CHECK: REDUCTION.SUB{{.*}}ptr %distances_local_even
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
