// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-ONE
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-TWO
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-THREE
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-FOUR
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-REG1

void bar();
// CHECK-LABEL: foo
void foo()
{
// CHECK-ONE: [[S_OUTER:%s_outer.*]] = alloca i32,
// CHECK-TWO: [[S_INNER:%s_inner.*]] = alloca i32,
// CHECK-THREE: [[S_OTHER_INNER:%s_other_inner.*]] = alloca i32,
// CHECK-FOUR: [[P_ONE:%p_one.*]] = alloca i32,
// CHECK-REG1: [[S_OUTER:%s_outer.*]] = alloca i32,
// CHECK-REG2: [[S_INNER:%s_inner.*]] = alloca i32,
// CHECK-REG3: [[S_OTHER_INNER:%s_other_inner.*]] = alloca i32,
// CHECK-REG4: [[P_ONE:%p_one.*]] = alloca i32,
   int s_outer = 2;
// CHECK-ONE: directive(metadata !"DIR.OMP.PARALLEL")
// CHECK-ONE: opndlist(metadata !"QUAL.OMP.SHARED", i32* [[S_OUTER]])
// CHECK-TWO: directive(metadata !"DIR.OMP.PARALLEL")
// CHECK-TWO: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[S_INNER]])
// CHECK-THREE: directive(metadata !"DIR.OMP.PARALLEL")
// CHECK-THREE: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[S_OTHER_INNER]])
// CHECK-FOUR: directive(metadata !"DIR.OMP.PARALLEL")
// CHECK-FOUR: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[P_ONE]])
// CHECK-REG1: region.entry() [ "DIR.OMP.PARALLEL"(), {{.*}}"QUAL.OMP.SHARED"(i32* [[S_OUTER]]
// CHECK-REG2: region.entry() [ "DIR.OMP.PARALLEL"(), {{.*}}"QUAL.OMP.PRIVATE"(i32* [[S_INNER]]
// CHECK-REG3: region.entry() [ "DIR.OMP.PARALLEL"(), {{.*}}"QUAL.OMP.PRIVATE"(i32* [[S_OTHER_INNER]]
// CHECK-REG4: region.entry() [ "DIR.OMP.PARALLEL"(), {{.*}}"QUAL.OMP.PRIVATE"(i32* [[P_ONE]]
   #pragma omp parallel
   {
     int s_inner = 3;
     int s_other_inner = 4;

// CHECK-ONE: directive(metadata !"DIR.OMP.PARALLEL")
// CHECK-ONE: opndlist(metadata !"QUAL.OMP.SHARED", i32* [[S_OUTER]])
// CHECK-TWO: directive(metadata !"DIR.OMP.PARALLEL")
// CHECK-TWO: opndlist(metadata !"QUAL.OMP.SHARED", i32* [[S_INNER]])
// CHECK-THREE: directive(metadata !"DIR.OMP.PARALLEL")
// CHECK-THREE: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[S_OTHER_INNER]])
// CHECK-FOUR: directive(metadata !"DIR.OMP.PARALLEL")
// CHECK-FOUR: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[P_ONE]])
// CHECK-REG1: region.entry() [ "DIR.OMP.PARALLEL"(), {{.*}}"QUAL.OMP.SHARED"(i32* [[S_OUTER]]
// CHECK-REG2: region.entry() [ "DIR.OMP.PARALLEL"(), {{.*}}"QUAL.OMP.SHARED"(i32* [[S_INNER]]
// CHECK-REG3: region.entry() [ "DIR.OMP.PARALLEL"(), {{.*}}"QUAL.OMP.PRIVATE"(i32* [[S_OTHER_INNER]]
// CHECK-REG4: region.entry() [ "DIR.OMP.PARALLEL"(), {{.*}}"QUAL.OMP.PRIVATE"(i32* [[P_ONE]]
     #pragma omp parallel private(s_other_inner)
     {
       int p_one = 1;
       bar(s_outer+s_inner+p_one+s_other_inner);
     }

   }

   // CHECK-ONE-NOT: "QUAL.OMP.PRIVATE"{{.*}}omp.iv
   {
    int j,a=3;
    #pragma omp parallel
    {
      #pragma omp for schedule(static, 1) firstprivate(a)
      for ( j = 0; j < 10; j++ ) { }
    }
  }
}
