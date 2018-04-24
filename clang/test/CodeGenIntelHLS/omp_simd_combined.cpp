//RUN: %clang_cc1 -fhls -fopenmp -fintel-compatibility -fintel-openmp-region -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -fopenmp -fintel-compatibility -fintel-openmp-region -debug-info-kind=limited -emit-llvm -o - %s

// There is no clear specification yet which of our added loop pragmas
// should be allowed on OpenMP pragmas. At least unroll and ivdep seem
// to be required.

void bar(int i);

//CHECK-LABEL: foo_unroll
void foo_unroll()
{
  //CHECK: [[TK1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.SIMD
  //CHECK: br{{.*}}!llvm.loop [[MD2:![0-9]+]]
  //CHECK: call void @llvm.directive.region.exit(token [[TK1]]){{.*}}END.SIMD
  #pragma unroll 4
  #pragma omp simd simdlen(16)
  for (int i=0;i<64;++i) { bar(i); }

  //CHECK: [[TK2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.SIMD
  //CHECK: br{{.*}}!llvm.loop [[MD4:![0-9]+]]
  //CHECK: call void @llvm.directive.region.exit(token [[TK2]]){{.*}}END.SIMD
  #pragma omp simd simdlen(16)
  #pragma unroll 4
  for (int i=0;i<64;++i) { bar(i); }
}

//CHECK-LABEL: foo_ivdep
void foo_ivdep()
{
  //CHECK: [[TK3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.SIMD
  //CHECK: br{{.*}}!llvm.loop [[MDIV1:![0-9]+]]
  //CHECK: call void @llvm.directive.region.exit(token [[TK3]]){{.*}}END.SIMD
  #pragma ivdep safelen(4)
  #pragma omp simd simdlen(16)
  for (int i=0;i<64;++i) { bar(i); }

  //CHECK: [[TK4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.SIMD
  //CHECK: br{{.*}}!llvm.loop [[MDIV2:![0-9]+]]
  //CHECK: call void @llvm.directive.region.exit(token [[TK4]]){{.*}}END.SIMD
  #pragma omp simd simdlen(16)
  #pragma ivdep safelen(4)
  for (int i=0;i<64;++i) { bar(i); }
}

//CHECK: [[MD2]] = distinct !{[[MD2]], [[MD3:![0-9]+]]}
//CHECK: [[MD3]] = !{!"llvm.loop.unroll.count", i32 4}
//CHECK: [[MD4]] = distinct !{[[MD4]], [[MD3]]}

//CHECK: [[MDIV1]] = distinct !{[[MDIV1]], [[MDIV1A:![0-9]+]]}
//CHECK: [[MDIV1A]] = !{!"llvm.loop.ivdep.safelen", i32 4}
//CHECK: [[MDIV2]] = distinct !{[[MDIV2]], [[MDIV1A]]}
