; RUN: opt -disable-hir-opt-predicate-region-simd=false -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the SIMD directives where moved inside the If condition
; that was hoisted outside of the loop.

; HIR before transformation

; BEGIN REGION { }
;       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
;
;       + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
;       |   if (%n != 20)
;       |   {
;       |      (%a)[i1] = i1;
;       |   }
;       + END LOOP
;
;       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;       ret ;
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%n != 20)
; CHECK:       {
; CHECK:          %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
; CHECK:          |   (%a)[i1] = i1;
; CHECK:          + END LOOP
; CHECK:          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:       }
; CHECK:       ret ;
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %a, i32 %n) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  %cmp1 = icmp eq i32 %n, 20
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %omp.inner.for.body.lr.ph ]
  br i1 %cmp1, label %omp.inner.for.inc, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() #1

declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

