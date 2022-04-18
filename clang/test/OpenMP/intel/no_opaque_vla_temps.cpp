// RUN: %clang_cc1 -no-opaque-pointers -fopenmp -fopenmp-late-outline \
// RUN: -fopenmp-stable-file-id -triple x86_64-unknown-linux-gnu \
// RUN: -emit-llvm %s -o - | FileCheck %s
//
// Verify stable ordering of VLA temp references within parallel regions.

void foo(int n) {
  long double vla1[n];
  long double vla2[n];
  long double vla3[n];
  long double vla4[n];
  long double vla5[n];
  long double vla6[n];
  long double vla7[n];
  long double vla8[n];
  long double vla9[n];
  long double vla10[n];

  // CHECK: store i64 %{{[0-9]*}}, i64* [[VLATEMP1:%omp.vla.tmp[0-9]*]]
  // CHECK-NEXT: store i64 %{{[0-9]*}}, i64* [[VLATEMP2:%omp.vla.tmp[0-9]*]]
  // CHECK-NEXT: store i64 %{{[0-9]*}}, i64* [[VLATEMP3:%omp.vla.tmp[0-9]*]]
  // CHECK-NEXT: store i64 %{{[0-9]*}}, i64* [[VLATEMP4:%omp.vla.tmp[0-9]*]]
  // CHECK-NEXT: store i64 %{{[0-9]*}}, i64* [[VLATEMP5:%omp.vla.tmp[0-9]*]]
  // CHECK-NEXT: store i64 %{{[0-9]*}}, i64* [[VLATEMP6:%omp.vla.tmp[0-9]*]]
  // CHECK-NEXT: store i64 %{{[0-9]*}}, i64* [[VLATEMP7:%omp.vla.tmp[0-9]*]]
  // CHECK-NEXT: store i64 %{{[0-9]*}}, i64* [[VLATEMP8:%omp.vla.tmp[0-9]*]]
  // CHECK-NEXT: store i64 %{{[0-9]*}}, i64* [[VLATEMP9:%omp.vla.tmp[0-9]*]]
  // CHECK-NEXT: store i64 %{{[0-9]*}}, i64* [[VLATEMP10:%omp.vla.tmp[0-9]*]]
  // CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP1]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP2]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP3]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP4]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP5]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP6]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP7]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP8]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP9]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[VLATEMP10]])
  // CHECK-NEXT: load i64, i64* [[VLATEMP1]]
  // CHECK-NEXT: load i64, i64* [[VLATEMP2]]
  // CHECK-NEXT: load i64, i64* [[VLATEMP3]]
  // CHECK-NEXT: load i64, i64* [[VLATEMP4]]
  // CHECK-NEXT: load i64, i64* [[VLATEMP5]]
  // CHECK-NEXT: load i64, i64* [[VLATEMP6]]
  // CHECK-NEXT: load i64, i64* [[VLATEMP7]]
  // CHECK-NEXT: load i64, i64* [[VLATEMP8]]
  // CHECK-NEXT: load i64, i64* [[VLATEMP9]]
  // CHECK-NEXT: load i64, i64* [[VLATEMP10]]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()
  #pragma omp parallel for
  for(int i = 0; i < 10; ++i)
    vla1[i] += (vla2[i] + vla3[i] + vla4[i] + vla5[i] + vla6[i] + vla7[i] +
                vla8[i] + vla9[i] + vla10[i]);
};

