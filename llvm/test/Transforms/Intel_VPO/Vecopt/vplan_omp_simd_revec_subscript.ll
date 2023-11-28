; RUN: opt %s -disable-output -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-nested-simd-strategy=frominside -vplan-force-vf=2 2>&1 | FileCheck %s --check-prefix=HIR
; RUN: opt %s -S -passes="hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,instcombine" -vplan-nested-simd-strategy=frominside -vplan-force-vf=2 | FileCheck %s --check-prefix=LLVM

define void @foo(ptr %arr) {
; HIR:      + DO i2 = 0, 511, 1   <DO_LOOP> <novectorize>
; HIR-NEXT: |   %wide.insert = shufflevector i2,  undef,  <i32 0, i32 undef, i32 1, i32 undef>;
; HIR-NEXT: |   %extractsubvec. = shufflevector %wide.insert,  undef,  <i32 0, i32 1>;
; HIR-NEXT: |   %shuffle = shufflevector %extractsubvec.,  poison,  zeroinitializer;
; HIR-NEXT: |   %.replicated = shufflevector %shuffle,  undef,  <i32 0, i32 1, i32 0, i32 1>;
; HIR-NEXT: |   %.vec = <i64 2, i64 2, i64 2, i64 2>  *  %.replicated;
; HIR-NEXT: |   %.scal = <i64 2, i64 2>  *  %shuffle;
; HIR-NEXT: |   %wide.insert4 = shufflevector i1 + <i64 0, i64 1>,  undef,  <i32 0, i32 undef, i32 1, i32 undef>;
; HIR-NEXT: |   %shuffle5 = shufflevector %wide.insert4,  poison,  <i64 0, i64 0, i64 2, i64 2>;
; HIR-NEXT: |   (<4 x i64>*)(%arr)[zeroinitializer][%.vec + <i64 0, i64 1, i64 0, i64 1>][%shuffle5] = zeroinitializer;
; HIR-NEXT: |   %.vec6 = i2 + 1 < 512;
; HIR-NEXT: |   %phi.temp = i2 + 1;
; HIR-NEXT: + END LOOP
;
; Check that no crash happens in InstCombinePass.
; LLVM: define void @foo(ptr %arr) {
;
DIR.OMP.SIMD.1:
  br label %DIR.OMP.SIMD.141

DIR.OMP.SIMD.141:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.OMP.SIMD.242

DIR.OMP.SIMD.242:                                 ; preds = %DIR.OMP.END.SIMD.3, %DIR.OMP.SIMD.141
  %exitcond39.not = phi i1 [ false, %DIR.OMP.SIMD.141 ], [ true, %DIR.OMP.END.SIMD.3 ]
  %.omp.iv.local.037 = phi i64 [ 0, %DIR.OMP.SIMD.141 ], [ 1, %DIR.OMP.END.SIMD.3 ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body6

omp.inner.for.body6:                              ; preds = %omp.inner.for.body6, %DIR.OMP.SIMD.242
  %.omp.iv2.local.034 = phi i64 [ 0, %DIR.OMP.SIMD.242 ], [ %add11, %omp.inner.for.body6 ]
  %arrayidx10 = getelementptr [1024 x [1024 x i64]], ptr %arr, i64 0, i64 %.omp.iv2.local.034, i64 %.omp.iv.local.037
  store i64 0, ptr %arrayidx10, align 8
  %add11 = add i64 %.omp.iv2.local.034, 1
  %exitcond.not = icmp eq i64 %.omp.iv2.local.034, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body6

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body6
  call void @llvm.directive.region.exit(token none) [ "DIR.OMP.END.SIMD"() ]
  br i1 %exitcond39.not, label %DIR.OMP.END.SIMD.2, label %DIR.OMP.SIMD.242

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
