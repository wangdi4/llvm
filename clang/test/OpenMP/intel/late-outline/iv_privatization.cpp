// RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses -fopenmp-late-outline -fopenmp-targets=x86_64 %s -emit-llvm -o - | FileCheck %s

// Verify that the normalized IV is privatized in the outer regions:
// CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),{{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv{{.*}} ]
// CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),{{.*}}"QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv{{.*}} ]
// CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),{{.*}}"QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv{{.*}} ]

void foo() {
#pragma omp target teams distribute parallel for
  for (int i = 0; i < 1000; ++i);
}
