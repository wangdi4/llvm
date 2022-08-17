;
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec -disable-output -vplan-force-vf=4 < %s 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s --check-prefix=SOA
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec -disable-output -vplan-force-vf=4 < %s 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s --check-prefix=SOA
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s --check-prefix=SOA
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s --check-prefix=SOA
;
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec -disable-output -vplan-force-vf=4 -vplan-enable-soa-hir=0 < %s 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s --check-prefix=NOSOA
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec -disable-output -vplan-force-vf=4 -vplan-enable-soa-hir=0 < %s 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s --check-prefix=NOSOA
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 -vplan-enable-soa-hir=0 < %s 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s --check-prefix=NOSOA
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 -vplan-enable-soa-hir=0 < %s 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s --check-prefix=NOSOA
;
; SOA transformation for privates is not implemented currently in the VPlan HIR
; path. This can lead to performance regressions in cases where we go through
; the HIR vectorizer vs LLVM IR vectorizer when SOA transformation kicks in for
; the LLVM IR path(e.g. a 2x performance regression was seen while enabling
; outer loop vectorization in HIR path in sycl_benchmarks_omp/mceop
; @opt_O3_ipo_iopenmp_zmm_xH_fast). Until we implement SOA transform in HIR
; path the solution enables SOA analysis in HIR path by default and bails out
; of vectorization when we see SOA profitable private arrays. Since SOA private
; transform only applies to explicit SIMD loops, the loop will be vectorized
; later in LLVM IR VPlan path.
;
; SOA:         DO i1 = 0, 99, 1   <DO_LOOP> <simd> <vectorize>
; NOSOA:       DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
;
define void @test_array(i64 %n)  {
entry:
  %arr = alloca [100 x i64], align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"([100 x i64]* %arr, i64 0, i32 100) ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add1, %for.body ]
  %priv.idx = getelementptr inbounds [100 x i64], [100 x i64]* %arr, i64 0, i64 %n
  store i64 %iv, i64* %priv.idx, align 4
  %add1 = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %add1, 100
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
