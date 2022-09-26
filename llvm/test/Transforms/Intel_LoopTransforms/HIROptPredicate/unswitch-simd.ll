; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Check that SIMD loop will be unswitched by hir-opt-predicate if the SIMD
; directives are part of the pre-header and post-exit nodes of the loop.

; Before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |      %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(8),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0])1) ]
;       |   + DO i2 = 0, %m + -1, 1   <DO_LOOP> <simd>
;       |   |   @llvm.lifetime.start.p0i8(4,  &((i8*)(%j.linear.iv)[0]));
;       |   |   if (%n != 20)
;       |   |   {
;       |   |      (%a)[i2] = i2;
;       |   |   }
;       |   |   @llvm.lifetime.end.p0i8(4,  &((i8*)(%j.linear.iv)[0]));
;       |   + END LOOP
;       |      @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ]
;       + END LOOP
; END REGION

; After transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%n != 20)
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |      %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(8),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0])1) ]
; CHECK:          |   + DO i2 = 0, %m + -1, 1   <DO_LOOP> <simd>
; CHECK:          |   |   @llvm.lifetime.start.p0i8(4,  &((i8*)(%j.linear.iv)[0]));
; CHECK:          |   |   (%a)[i2] = i2;
; CHECK:          |   |   @llvm.lifetime.end.p0i8(4,  &((i8*)(%j.linear.iv)[0]));
; CHECK:          |   + END LOOP
; CHECK:          |      @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ]
; CHECK:          + END LOOP
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |      %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(8),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0])1) ]
; CHECK:          |   + DO i2 = 0, %m + -1, 1   <DO_LOOP> <simd>
; CHECK:          |   |   @llvm.lifetime.start.p0i8(4,  &((i8*)(%j.linear.iv)[0]));
; CHECK:          |   |   @llvm.lifetime.end.p0i8(4,  &((i8*)(%j.linear.iv)[0]));
; CHECK:          |   + END LOOP
; CHECK:          |      @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ]
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %a, i32 %n, i64 %m) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp eq i32 %n, 20
  %j.linear.iv = alloca i32, align 4
  br label %outer.loop.ph

outer.loop.ph:
  br label %outer.loop.for.body

outer.loop.for.body:
  %indvars.iv = phi i64 [ 0, %outer.loop.ph ], [ %inc, %outer.loop.end ]
  %0 = bitcast i32* %j.linear.iv to i8*
  %cmp2 = icmp sgt i64 %m, 0
  br i1 %cmp2, label %omp.inner.for.body.lr.ph, label %outer.loop.end

omp.inner.for.body.lr.ph:
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %j.linear.iv, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lr.ph
  %indvars.iv2 = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %omp.inner.for.body.lr.ph ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  br i1 %cmp1, label %omp.inner.for.inc, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv2
  %2 = trunc i64 %indvars.iv2 to i32
  store i32 %2, i32* %arrayidx, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv2, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %m
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0)
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %outer.loop.end

outer.loop.end:
  %inc = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i64 %inc, 100
  br i1 %cmp, label %exit, label %outer.loop.for.body

exit:
  ret void
}

declare token @llvm.directive.region.entry() #1

declare void @llvm.directive.region.exit(token) #1

declare void @llvm.lifetime.start.p0i8(i64, i8*)
declare void @llvm.lifetime.end.p0i8(i64, i8*)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

