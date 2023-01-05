//RUN: %clang_cc1 -fhls -fopenmp -fintel-compatibility -fopenmp-late-outline -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -fopenmp -fintel-compatibility -fopenmp-late-outline -debug-info-kind=limited -emit-llvm -o %t %s

// There is no clear specification yet which of our added loop pragmas
// should be allowed on OpenMP pragmas. At least unroll and ivdep seem
// to be required.

void bar(int i);

//CHECK-LABEL: foo_unroll
void foo_unroll()
{
  //CHECK: [[TK2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.SIMD
  //CHECK: br{{.*}}!llvm.loop [[MD3:![0-9]+]]
  //CHECK: call void @llvm.directive.region.exit(token [[TK2]]){{.*}}END.SIMD
  #pragma omp simd simdlen(16)
  #pragma unroll 4
  for (int i=0;i<64;++i) { bar(i); }
}

//CHECK-LABEL: foo_ivdep
void foo_ivdep()
{
  //CHECK: [[TK4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.SIMD
  //CHECK: br{{.*}}!llvm.loop [[MDIV1:![0-9]+]]
  //CHECK: call void @llvm.directive.region.exit(token [[TK4]]){{.*}}END.SIMD
  #pragma omp simd simdlen(16)
  #pragma ivdep safelen(4)
  for (int i=0;i<64;++i) { bar(i); }
}

//CHECK: [[MD3]] = distinct !{[[MD3]], [[MD4:![0-9]+]], [[MD5:![0-9]+]]}
//CHECK: [[MD4]] = !{!"llvm.loop.vectorize.enable", i1 true}
//CHECK-NOT !{!"llvm.loop.vectorize.enable", i1 true}
//CHECK-NOT !{!"llvm.loop.isvectorized"}
//CHECK: [[MD5]] = !{!"llvm.loop.unroll.count", i32 4}

//CHECK: [[MDIV1]] = distinct !{[[MDIV1]], [[MD4]], [[MDIV1A:![0-9]+]]}
//CHECK: [[MDIV1A]] = !{!"llvm.loop.ivdep.safelen", i32 4}
