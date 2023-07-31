; RUN: opt -passes="hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

; There is a breakable recurrence from 15:32 across if thenblocks

; BEGIN REGION { }
;        + DO i1 = 0, 99998, 1   <DO_LOOP>
;        |   + DO i2 = 0, 99998, 1   <DO_LOOP>
;        |   |   if (i2 + 1 < 27)
;        |   |   {
;        |   |      %0 = (@B)[0][i1 + 1][i2 + 1];
;        |   |      %1 = (@C)[0][i1 + 1][i2 + 1];
;        |   |      %add = %0  +  %1;
;        |   |      (@A)[0][i1 + 1][i2 + 1] = %add;
;        |   |   }
;        |   |   else
;        |   |   {
;        |   |      %3 = (@A)[0][i1 + 1][i2];
;        |   |      %conv19 = %3  *  2.000000e+00;
;        |   |      (@D)[0][i1 + 1][i2 + 1] = %conv19;
;        |   |   }
;        |   + END LOOP
;        + END LOOP
; END REGION

; CHECK: BEGIN REGION
; CHECK: modified
; CHECK: + DO i1 = 0, 99998, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 99998, 1   <DO_LOOP>
; CHECK: |   |   if (i2 + 1 < 27)
;        |   |   {
;        |   |      %0 = (@B)[0][i1 + 1][i2 + 1];
;        |   |      %1 = (@C)[0][i1 + 1][i2 + 1];
;        |   |      %add = %0  +  %1;
;        |   |      (@A)[0][i1 + 1][i2 + 1] = %add;
;        |   |   }
; CHECK: |   + END LOOP
;        |
;        |
; CHECK: |   + DO i2 = 0, 99998, 1   <DO_LOOP>
; CHECK: |   |   if (i2 + 1 >= 27)
;        |   |   {
;        |   |      %3 = (@A)[0][i1 + 1][i2];
;        |   |      %conv19 = %3  *  2.000000e+00;
;        |   |      (@D)[0][i1 + 1][i2 + 1] = %conv19;
;        |   |   }
; CHECK: |   + END LOOP
;        + END LOOP
; END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [100000 x [100000 x float]] zeroinitializer, align 16
@B = global [100000 x [100000 x float]] zeroinitializer, align 16
@C = global [100000 x [100000 x float]] zeroinitializer, align 16
@D = global [100000 x [100000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @_Z3foov() {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.24, %entry
  %indvars.iv44 = phi i64 [ 1, %entry ], [ %indvars.iv.next45, %for.inc.24 ]
  br label %for.body.3

for.body.3:                                       ; preds = %for.inc, %for.cond.1.preheader
  %indvars.iv = phi i64 [ 1, %for.cond.1.preheader ], [ %indvars.iv.next, %for.inc ]
  %cmp4 = icmp slt i64 %indvars.iv, 27
  br i1 %cmp4, label %if.then, label %if.else

if.then:                                          ; preds = %for.body.3
  %arrayidx6 = getelementptr inbounds [100000 x [100000 x float]], ptr @B, i64 0, i64 %indvars.iv44, i64 %indvars.iv
  %0 = load float, ptr %arrayidx6, align 4
  %arrayidx10 = getelementptr inbounds [100000 x [100000 x float]], ptr @C, i64 0, i64 %indvars.iv44, i64 %indvars.iv
  %1 = load float, ptr %arrayidx10, align 4
  %add = fadd float %0, %1
  %arrayidx14 = getelementptr inbounds [100000 x [100000 x float]], ptr @A, i64 0, i64 %indvars.iv44, i64 %indvars.iv
  store float %add, ptr %arrayidx14, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body.3
  %2 = add nsw i64 %indvars.iv, -1
  %arrayidx18 = getelementptr inbounds [100000 x [100000 x float]], ptr @A, i64 0, i64 %indvars.iv44, i64 %2
  %3 = load float, ptr %arrayidx18, align 4
  %conv19 = fmul float %3, 2.000000e+00
  %arrayidx23 = getelementptr inbounds [100000 x [100000 x float]], ptr @D, i64 0, i64 %indvars.iv44, i64 %indvars.iv
  store float %conv19, ptr %arrayidx23, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100000
  br i1 %exitcond, label %for.inc.24, label %for.body.3

for.inc.24:                                       ; preds = %for.inc
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next45, 100000
  br i1 %exitcond46, label %for.end.26, label %for.cond.1.preheader

for.end.26:                                       ; preds = %for.inc.24
  ret void
}


