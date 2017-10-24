// RUN: %clang_cc1 -emit-llvm -o - %s -fintel-compatibility -fopenmp -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s

typedef unsigned short ushort;
typedef ushort ushort8 __attribute__((ext_vector_type(8)));
typedef ushort8 DistSIMDType;

// CHECK-LABEL: foo
void foo()
{
  DistSIMDType foo;
  int var;

  //CHECK: QUAL.OMP.LASTPRIVATE:CONDITIONAL
  #pragma omp parallel for lastprivate(conditional:foo)
  for (var = 0; var < 6; var++) {
    if (var%3==1) {
      foo = 2*var;
    }
  }
}
