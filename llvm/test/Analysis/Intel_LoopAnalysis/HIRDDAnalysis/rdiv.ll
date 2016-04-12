
;RUN: opt -basicaa -analyze -hir-dd-analysis -hir-dd-analysis-verify=Region %s | FileCheck %s 
; For DDtest from 12 to 37 we should use RDIV/MIV not SIV test
; Even though its "i2" in both refs, it is an i2 corresponding to a
; different loop nest. SIV applies only when loop bounds are same.
;;          BEGIN REGION { }
;<57>         + DO i1 = 0, %N + -1, 1   <DO_LOOP>
;<3>          |   if {al:8}(%N > 0)
;<3>          |   {
;<58>         |      + DO i2 = 0, %N + -1, 1   <DO_LOOP>
;<9>          |      |   %conv7 = sitofp.i32.double(i2);
;<12>         |      |   {al:8}(%A)[i2] = %conv7;
;<58>         |      + END LOOP
;<3>          |   }
;<25>         |   if {al:8}(%N > 0)
;<25>         |   {
;<59>         |      + DO i2 = 0, %N + -1, 1   <DO_LOOP>
;<33>         |      |   %0 = {al:8}(%A)[i2];
;<34>         |      |   %mul = %0  *  2.000000e+00;
;<37>         |      |   {al:8}(%A)[i2] = %mul;
;<59>         |      + END LOOP
;<25>         |   }
;<57>         + END LOOP
;          END REGION

; Two self output edges
; CHECK: {al:8}(%A)[i2] --> {al:8}(%A)[i2] OUTPUT
; CHECK: {al:8}(%A)[i2] --> {al:8}(%A)[i2] OUTPUT

; Two crossnest output edges
; CHECK: {al:8}(%A)[i2] --> {al:8}(%A)[i2] OUTPUT
; CHECK: {al:8}(%A)[i2] --> {al:8}(%A)[i2] OUTPUT
; ModuleID = 'alt.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(double* %A, i64 %N) #0 {
entry:
  %cmp.5 = icmp sgt i64 %N, 0
  br i1 %cmp.5, label %for.body.lr.ph, label %for.end.22

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc.20
  %j.06 = phi i32 [ 0, %for.body.lr.ph ], [ %inc21, %for.inc.20 ]
  %cmp4.1 = icmp sgt i64 %N, 0
  br i1 %cmp4.1, label %for.body.6.lr.ph, label %for.end

for.body.6.lr.ph:                                 ; preds = %for.body
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6.lr.ph, %for.inc
  %i.02 = phi i32 [ 0, %for.body.6.lr.ph ], [ %inc, %for.inc ]
  %conv7 = sitofp i32 %i.02 to double
  %idxprom = sext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds double, double* %A, i64 %idxprom
  store double %conv7, double* %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body.6
  %inc = add nsw i32 %i.02, 1
  %conv3 = sext i32 %inc to i64
  %cmp4 = icmp slt i64 %conv3, %N
  br i1 %cmp4, label %for.body.6, label %for.cond.2.for.end_crit_edge

for.cond.2.for.end_crit_edge:                     ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.2.for.end_crit_edge, %for.body
  %cmp10.3 = icmp sgt i64 %N, 0
  br i1 %cmp10.3, label %for.body.12.lr.ph, label %for.end.19

for.body.12.lr.ph:                                ; preds = %for.end
  br label %for.body.12

for.body.12:                                      ; preds = %for.body.12.lr.ph, %for.inc.17
  %i.14 = phi i32 [ 0, %for.body.12.lr.ph ], [ %inc18, %for.inc.17 ]
  %idxprom13 = sext i32 %i.14 to i64
  %arrayidx14 = getelementptr inbounds double, double* %A, i64 %idxprom13
  %0 = load double, double* %arrayidx14, align 8
  %mul = fmul double %0, 2.000000e+00
  %idxprom15 = sext i32 %i.14 to i64
  %arrayidx16 = getelementptr inbounds double, double* %A, i64 %idxprom15
  store double %mul, double* %arrayidx16, align 8
  br label %for.inc.17

for.inc.17:                                       ; preds = %for.body.12
  %inc18 = add nsw i32 %i.14, 1
  %conv9 = sext i32 %inc18 to i64
  %cmp10 = icmp slt i64 %conv9, %N
  br i1 %cmp10, label %for.body.12, label %for.cond.8.for.end.19_crit_edge

for.cond.8.for.end.19_crit_edge:                  ; preds = %for.inc.17
  br label %for.end.19

for.end.19:                                       ; preds = %for.cond.8.for.end.19_crit_edge, %for.end
  br label %for.inc.20

for.inc.20:                                       ; preds = %for.end.19
  %inc21 = add nsw i32 %j.06, 1
  %conv = sext i32 %inc21 to i64
  %cmp = icmp slt i64 %conv, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end.22_crit_edge

for.cond.for.end.22_crit_edge:                    ; preds = %for.inc.20
  br label %for.end.22

for.end.22:                                       ; preds = %for.cond.for.end.22_crit_edge, %entry
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 1049) (llvm/branches/loopopt 1210)"}
