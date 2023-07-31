; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Test that cloned ifs which are under the same target loop are handled properly.
; In this case we have multiple if (%0 != 0) cases under their target loop DO i2.

; + DO i1 = 0, 2, 1   <DO_LOOP>
; |   + DO i2 = 0, (-1 * i1 + umax((1 + %indvars.iv50), (2 + %indvars.iv50)) + -1)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 1>
; |   |   %1 = (@a)[0][i1 + 2 * i2];
; |   |
; |   |   + DO i3 = 0, 1, 1   <DO_LOOP>
; |   |   |   %cond = 0;
; |   |   |   if (%1 != 0)
; |   |   |   {
; |   |   |      %2 = (@a)[0][i3 + 1];
; |   |   |      %cond = %2;
; |   |   |   }
; |   |   |   if (i1 != 0)
; |   |   |   {
; |   |   |      %cond21 = 0;
; |   |   |      if (%0 != 0)
; |   |   |      {
; |   |   |         %3 = (@a)[0][i3 + 1];
; |   |   |         %cond21 = %3;
; |   |   |      }
; |   |   |      %4 = (@pq)[0];
; |   |   |      (@pq)[0] = zext.i8.i32(%4) + %cond21;
; |   |   |   }
; |   |   + END LOOP
; |   + END LOOP
; |
; |   %indvars.iv50 = i1 + 1;
; + END LOOP

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1
; CHECK: |   if (i1 != 0)
; CHECK: |   {
; CHECK: |      if (%0 != 0)
; CHECK: |      {
; CHECK: |         + DO i2
; CHECK: |         |   if (%1 != 0)
; CHECK: |         |   {
; CHECK: |         |      + DO i3
; CHECK: |         |      + END LOOP
; CHECK: |         |   }
; CHECK: |         |   else
; CHECK: |         |   {
; CHECK: |         |      + DO i3
; CHECK: |         |      + END LOOP
; CHECK: |         |   }
; CHECK: |         + END LOOP
; CHECK: |      }
; CHECK: |      else
; CHECK: |      {
; CHECK: |         + DO i2 = 0
; CHECK: |         |   if (%1 != 0)
; CHECK: |         |   {
; CHECK: |         |      + DO i3 = 0
; CHECK: |         |      + END LOOP
; CHECK: |         |   }
; CHECK: |         |   else
; CHECK: |         |   {
; CHECK: |         |      + DO i3 = 0
; CHECK: |         |      + END LOOP
; CHECK: |         |   }
; CHECK: |         + END LOOP
; CHECK: |      }
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      + DO i2 = 0
; CHECK: |      |   if (%1 != 0)
; CHECK: |      |   {
; CHECK: |      |      + DO i3 = 0
; CHECK: |      |      + END LOOP
; CHECK: |      |   }
; CHECK: |      |   else
; CHECK: |      |   {
; CHECK: |      |      + DO i3 = 0
; CHECK: |      |      + END LOOP
; CHECK: |      |   }
; CHECK: |      + END LOOP
; CHECK: |   }
; CHECK: + END LOOP
; CHECK: END REGION

;Module Before HIR; ModuleID = 'atg_CMPLRS-47426.c'
source_filename = "atg_CMPLRS-47426.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@u = local_unnamed_addr global i8 0, align 1
@f = local_unnamed_addr global i8 0, align 1
@pq = local_unnamed_addr global i8 0, align 1
@a = local_unnamed_addr global [3 x i32] zeroinitializer, align 4

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i8, ptr @u, align 1
  %tobool16 = icmp eq i8 %0, 0
  br label %for.body

for.body:                                         ; preds = %for.end29, %entry
  %indvars.iv50 = phi i64 [ 0, %entry ], [ %indvars.iv.next51, %for.end29 ]
  %tobool14 = icmp eq i64 %indvars.iv50, 0
  br label %for.body7

for.body7:                                        ; preds = %for.body, %for.end
  %indvars.iv48 = phi i64 [ %indvars.iv50, %for.body ], [ %indvars.iv.next49, %for.end ]
  %arrayidx = getelementptr inbounds [3 x i32], ptr @a, i64 0, i64 %indvars.iv48
  %1 = load i32, ptr %arrayidx, align 4
  %tobool = icmp eq i32 %1, 0
  br label %for.body11

for.body11:                                       ; preds = %for.inc, %for.body7
  %k.045 = phi i64 [ 1, %for.body7 ], [ %add24, %for.inc ]
  br i1 %tobool, label %cond.end, label %cond.true

cond.true:                                        ; preds = %for.body11
  %arrayidx12 = getelementptr inbounds [3 x i32], ptr @a, i64 0, i64 %k.045
  %2 = load i32, ptr %arrayidx12, align 4
  %phitmp = trunc i32 %2 to i8
  br label %cond.end

cond.end:                                         ; preds = %for.body11, %cond.true
  %cond = phi i8 [ %phitmp, %cond.true ], [ 0, %for.body11 ]
  br i1 %tobool14, label %for.inc, label %if.then

if.then:                                          ; preds = %cond.end
  br i1 %tobool16, label %cond.end20, label %cond.true17

cond.true17:                                      ; preds = %if.then
  %arrayidx18 = getelementptr inbounds [3 x i32], ptr @a, i64 0, i64 %k.045
  %3 = load i32, ptr %arrayidx18, align 4
  br label %cond.end20

cond.end20:                                       ; preds = %if.then, %cond.true17
  %cond21 = phi i32 [ %3, %cond.true17 ], [ 0, %if.then ]
  %4 = load i8, ptr @pq, align 1
  %conv22 = zext i8 %4 to i32
  %add = add i32 %cond21, %conv22
  %conv23 = trunc i32 %add to i8
  store i8 %conv23, ptr @pq, align 1
  br label %for.inc

for.inc:                                          ; preds = %cond.end, %cond.end20
  %add24 = add nuw nsw i64 %k.045, 1
  %exitcond = icmp eq i64 %add24, 3
  br i1 %exitcond, label %for.end, label %for.body11

for.end:                                          ; preds = %for.inc
  %cond.lcssa = phi i8 [ %cond, %for.inc ]
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 2
  %cmp5 = icmp ugt i64 %indvars.iv.next49, %indvars.iv50
  br i1 %cmp5, label %for.end29, label %for.body7

for.end29:                                        ; preds = %for.end
  %cond.lcssa.lcssa = phi i8 [ %cond.lcssa, %for.end ]
  %indvars.iv.next51 = add nuw nsw i64 %indvars.iv50, 1
  %exitcond52 = icmp eq i64 %indvars.iv.next51, 3
  br i1 %exitcond52, label %for.end34, label %for.body

for.end34:                                        ; preds = %for.end29
  %cond.lcssa.lcssa.lcssa = phi i8 [ %cond.lcssa.lcssa, %for.end29 ]
  store i8 %cond.lcssa.lcssa.lcssa, ptr @f, align 1
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


