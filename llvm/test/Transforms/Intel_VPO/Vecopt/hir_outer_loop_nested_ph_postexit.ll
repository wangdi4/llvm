; RUN: opt -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to build a VPlan for an outer loop with a nested loop
; which has pre-header and post-exit in HIR.

; Source code
;
; #include <string.h>
; #include <stdio.h>
;
; #define N 101
;
; float A[N][N];
; float B[N][N];
;
; long ub[N];
;
; long foo(long n, long m, long *ub, float a[N][N], bool vec) {
;   long i, j, ret = 0;
;     #pragma omp simd simdlen(4) linear(i, j)
;     for (i = 0; i < n; i++) {
;       for (j = 0; j < m; j++) {
;         a[i][j] = i;
;         ub[j] = ret+j;
;         ret += 2;
;       }
;     }
;     return ret;
; }

; This is the inner loop we are checking in VPlan:
;
;  BB5 (BP: NULL) :
;   float %vp61456 = sitofp i64 %vp53952
;   i64 %vp61776 = add i64 %m i64 -1
;  SUCCESSORS(1):BB6
;
;  BB6 (BP: NULL) :
;   i64 %vp53280 = phi  [ i64 0, BB5 ],  [ i64 %vp8832, BB11 ]
;   float* %vp51760 = getelementptr [101 x float]* %a i64 %vp53952 i64 %vp53280
;   store float %vp61456 float* %vp51760
;   i64 %vp54704 = mul i64 3 i64 %vp53280
;   i64 %vp54896 = add i64 %vp58032 i64 %vp54704
;   i64* %vp55136 = getelementptr i64* %ub i64 %vp53280
;   store i64 %vp54896 i64* %vp55136
;   i64 %vp46464 = mul i64 2 i64 %vp53280
;   i64 %vp46688 = add i64 %vp58032 i64 %vp46464
;   i64 %vp46912 = add i64 %vp46688 i64 2
;   i64 %vp47136 = add i64 %vp53280 i64 1
;   i1 %vp61936 = icmp i64 %vp47136 i64 %vp61776
;  SUCCESSORS(1):BB11
;
;  BB11 (BP: NULL) :
;   <Empty Block>
;   Condition(BB6): i1 %vp61936 = icmp i64 %vp47136 i64 %vp61776
;  SUCCESSORS(2):BB6(i1 %vp61936), BB7(!i1 %vp61936)

; Inner loop PH:
; CHECK:  float %vp{{[0-9]+}} = sitofp i64
; CHECK-NEXT:   i64 %vp{{[0-9]+}} = add i64 %m i64 -1
; CHECK-NEXT:  SUCCESSORS(1):[[H:BB[0-9]+]]
;
; Loop header:
; CHECK:  [[H]] (BP: NULL) :
; CHECK:  SUCCESSORS(2):[[H]](i1 %vp{{[0-9]+}}), [[PE:BB[0-9]+]](!i1 %vp{{[0-9]+}})

; Loop PE:
; CHECK:  [[PE]] (BP: NULL) :
; CHECK-NEXT: i64 %vp{{[0-9]+}} = phi  [ i64 %vp{{[0-9]+}}, BB11 ]
; CHECK-NEXT: i64 %vp{{[0-9]+}} = bitcast i64 %vp{{[0-9]+}}


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [101 x [101 x float]] zeroinitializer
@B = dso_local local_unnamed_addr global [101 x [101 x float]] zeroinitializer
@ub = dso_local local_unnamed_addr global [101 x i64] zeroinitializer

; Function Attrs: nounwind uwtable
define dso_local i64 @_Z3foollPlPA101_fb(i64 %n, i64 %m, i64* nocapture %ub, [101 x float]* nocapture %a, i1 zeroext %vec) local_unnamed_addr #0 {
entry:
  %i = alloca i64
  %j = alloca i64
  %0 = bitcast i64* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %0) #2
  %1 = bitcast i64* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %1) #2
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.118, label %omp.precond.end

DIR.OMP.SIMD.118:
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR"(i64* %i, i32 1), "QUAL.OMP.LINEAR"(i64* %j, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  %cmp622 = icmp sgt i64 %m, 0
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.0 = phi i64 [ %add11, %omp.inner.for.inc ], [ 0, %DIR.OMP.SIMD.118 ]
  %ret.021 = phi i64 [ %ret.1.lcssa, %omp.inner.for.inc ], [ 0, %DIR.OMP.SIMD.118 ]
  br i1 %cmp622, label %for.body.lr.ph, label %omp.inner.for.inc

for.body.lr.ph:
  %conv = sitofp i64 %.omp.iv.0 to float
  br label %for.body

for.body:
  %ret.124 = phi i64 [ %ret.021, %for.body.lr.ph ], [ %add10, %for.body ]
  %storemerge23 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %idx7 = getelementptr inbounds [101 x float], [101 x float]* %a, i64 %.omp.iv.0, i64 %storemerge23
  store float %conv, float* %idx7
  %add8 = add nsw i64 %ret.124, %storemerge23
  %idx9 = getelementptr inbounds i64, i64* %ub, i64 %storemerge23
  store i64 %add8, i64* %idx9
  %add10 = add nsw i64 %ret.124, 2
  %inc = add nuw nsw i64 %storemerge23, 1
  %exitcond = icmp eq i64 %inc, %m
  br i1 %exitcond, label %omp.inner.for.inc.loopexit, label %for.body

omp.inner.for.inc.loopexit:
  %add10.lcssa = phi i64 [ %add10, %for.body ]
  br label %omp.inner.for.inc

omp.inner.for.inc:
  %inc.lcssa25 = phi i64 [ 0, %omp.inner.for.body ], [ %m, %omp.inner.for.inc.loopexit ]
  %ret.1.lcssa = phi i64 [ %ret.021, %omp.inner.for.body ], [ %add10.lcssa, %omp.inner.for.inc.loopexit ]
  %add11 = add nuw nsw i64 %.omp.iv.0, 1
  %exitcond26 = icmp eq i64 %add11, %n
  br i1 %exitcond26, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:
  %inc.lcssa25.lcssa = phi i64 [ %inc.lcssa25, %omp.inner.for.inc ]
  %ret.1.lcssa.lcssa = phi i64 [ %ret.1.lcssa, %omp.inner.for.inc ]
  %.omp.iv.0.lcssa = phi i64 [ %.omp.iv.0, %omp.inner.for.inc ]
  store i64 %inc.lcssa25.lcssa, i64* %j
  store i64 %.omp.iv.0.lcssa, i64* %i
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:
  %ret.2 = phi i64 [ %ret.1.lcssa.lcssa, %omp.loop.exit ], [ 0, %entry ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #2
  ret i64 %ret.2
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA101_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"long", !5, i64 0}
