; RUN: opt < %s -hir-ssa-deconstruction -hir-cost-model-throttling=0 | opt -analyze -hir-parser -hir-cost-model-throttling=0 | FileCheck %s

; Check parsing output for the loop verifying that %sub6 is reverse engineered and parsed currectly in terms of loop IVs %i.043 and %j.040. 

; CHECK: + DO i1 = 0, 255, 1   <DO_LOOP>
; CHECK: |   %j.040 = 0;
; CHECK: |   + DO i2 = 0, 255, 1   <DO_LOOP>
; CHECK: |   |   if (undef #UNDEF# undef)
; CHECK: |   |   {
; CHECK: |   |   }
; CHECK: |   |   else
; CHECK: |   |   {
; CHECK: |   |      @printf(&((@.str)[0][0]),  i1,  i2,  -1 * umax(%i.043, %j.040) + 255);
; CHECK: |   |   }
; CHECK: |   |   %j.040 = i2 + 1;
; CHECK: |   + END LOOP
; CHECK: |   %i.043 = i1 + 1;
; CHECK: + END LOOP



; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = external unnamed_addr constant [27 x i8], align 1

; Function Attrs: nounwind uwtable
define void @main() #0 {
entry:
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  br i1 undef, label %for.body.i38.preheader, label %for.body.i

for.body.i38.preheader:                           ; preds = %for.body.i
  br label %for.body.i38

for.body.i38:                                     ; preds = %for.body.i38, %for.body.i38.preheader
  br i1 undef, label %for.cond1.preheader.preheader, label %for.body.i38

for.cond1.preheader.preheader:                    ; preds = %for.body.i38
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc10, %for.cond1.preheader.preheader
  %i.043 = phi i64 [ %inc11, %for.inc10 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.cond1.preheader
  %j.040 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.inc ]
  %cmp5 = icmp ult i64 %j.040, %i.043
  %0 = select i1 %cmp5, i64 %i.043, i64 %j.040
  %sub6 = sub i64 255, %0
  br i1 undef, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body3
  tail call void (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([27 x i8], [27 x i8]* @.str, i64 0, i64 0), i64 %i.043, i64 %j.040, i64 %sub6)
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body3
  %inc = add nuw nsw i64 %j.040, 1
  %exitcond = icmp eq i64 %inc, 256
  br i1 %exitcond, label %for.inc10, label %for.body3

for.inc10:                                        ; preds = %for.inc
  %inc11 = add nuw nsw i64 %i.043, 1
  %exitcond44 = icmp eq i64 %inc11, 256
  br i1 %exitcond44, label %for.end12, label %for.cond1.preheader

for.end12:                                        ; preds = %for.inc10
  ret void
}

; Function Attrs: nounwind
declare void @printf(i8* nocapture readonly, ...) #1

