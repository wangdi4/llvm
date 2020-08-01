; RUN: opt -hir-ssa-deconstruction -disable-output -hir-runtime-dd -hir-loop-interchange -debug-only=hir-loop-interchange -scoped-noalias-aa < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,hir-loop-interchange" -aa-pipeline="basic-aa,scoped-noalias-aa" -disable-output -debug-only=hir-loop-interchange < %s 2>&1 | FileCheck %s

; REQUIRES: asserts

; Before:
; BEGIN REGION { }
; + DO i1 = 0, zext.i32.i64((1 + %loop)) + -2, 1   <DO_LOOP>
; |   + DO i2 = 0, 24, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 24, 1   <DO_LOOP>
; |   |   |   + DO i4 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   |   |   |   %mul18 = (%vy)[sext.i32.i64(%n) * i2 + i3]  *  (%cx)[i1 + i2 + 25 * i4 + 1];
; |   |   |   |   %add23 = (%px)[i3 + 25 * i4]  +  %mul18;
; |   |   |   |   (%px)[i3 + 25 * i4] = %add23;
; |   |   |   + END LOOP
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP
; END REGION

; CHECK: Loopnest Interchanged: ( 1 2 3 4 ) --> ( 4 1 2 3 )

;Module Before HIR; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(float* nocapture %px, float* nocapture readonly %vy, float* nocapture readonly %cx, i32 %loop, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp54 = icmp slt i32 %loop, 1
  br i1 %cmp54, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %cmp1049 = icmp sgt i32 %n, 0
  %0 = sext i32 %n to i64
  %1 = add i32 %loop, 1
  %wide.trip.count67 = zext i32 %1 to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret i32 undef

for.body:                                         ; preds = %for.cond.cleanup3, %for.body.lr.ph
  %indvars.iv65 = phi i64 [ %indvars.iv.next66, %for.cond.cleanup3 ], [ 1, %for.body.lr.ph ]
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next66 = add nuw nsw i64 %indvars.iv65, 1
  %exitcond68 = icmp eq i64 %indvars.iv.next66, %wide.trip.count67
  br i1 %exitcond68, label %for.cond.cleanup.loopexit, label %for.body

for.body4:                                        ; preds = %for.cond.cleanup7, %for.body
  %indvars.iv60 = phi i64 [ 0, %for.body ], [ %indvars.iv.next61, %for.cond.cleanup7 ]
  %2 = mul nsw i64 %indvars.iv60, %0
  %3 = add nuw nsw i64 %indvars.iv60, %indvars.iv65
  %4 = trunc i64 %3 to i32
  br label %for.body8

for.cond.cleanup7:                                ; preds = %for.cond.cleanup11
  %indvars.iv.next61 = add nuw nsw i64 %indvars.iv60, 1
  %exitcond64 = icmp eq i64 %indvars.iv.next61, 25
  br i1 %exitcond64, label %for.cond.cleanup3, label %for.body4

for.body8:                                        ; preds = %for.cond.cleanup11, %for.body4
  %indvars.iv56 = phi i64 [ 0, %for.body4 ], [ %indvars.iv.next57, %for.cond.cleanup11 ]
  br i1 %cmp1049, label %for.body12.lr.ph, label %for.cond.cleanup11

for.body12.lr.ph:                                 ; preds = %for.body8
  %5 = add nsw i64 %indvars.iv56, %2
  %arrayidx = getelementptr inbounds float, float* %vy, i64 %5
  %6 = trunc i64 %indvars.iv56 to i32
  br label %for.body12

for.cond.cleanup11.loopexit:                      ; preds = %for.body12
  br label %for.cond.cleanup11

for.cond.cleanup11:                               ; preds = %for.cond.cleanup11.loopexit, %for.body8
  %indvars.iv.next57 = add nuw nsw i64 %indvars.iv56, 1
  %exitcond59 = icmp eq i64 %indvars.iv.next57, 25
  br i1 %exitcond59, label %for.cond.cleanup7, label %for.body8

for.body12:                                       ; preds = %for.body12, %for.body12.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body12.lr.ph ], [ %indvars.iv.next, %for.body12 ]
  %7 = load float, float* %arrayidx, align 4
  %8 = trunc i64 %indvars.iv to i32
  %mul13 = mul nsw i32 %8, 25
  %add15 = add i32 %mul13, %4
  %9 = zext i32 %add15 to i64
  %arrayidx17 = getelementptr inbounds float, float* %cx, i64 %9
  %10 = load float, float* %arrayidx17, align 4
  %mul18 = fmul float %7, %10
  %add20 = add nuw nsw i32 %mul13, %6
  %11 = zext i32 %add20 to i64
  %arrayidx22 = getelementptr inbounds float, float* %px, i64 %11
  %12 = load float, float* %arrayidx22, align 4
  %add23 = fadd float %12, %mul18
  store float %add23, float* %arrayidx22, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %0
  br i1 %exitcond, label %for.cond.cleanup11.loopexit, label %for.body12
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

