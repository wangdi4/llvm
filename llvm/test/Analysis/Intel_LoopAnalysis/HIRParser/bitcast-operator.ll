; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; RUN: opt < %s -passes="convert-to-subscript,hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Check parsing output for the loop verifying that the gep with bitcast operator is parsed cleanly..
; CHECK: + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK: |   %ld = (i32*)(@A)[0][2];
; CHECK: |   %conv = sitofp.i64.float(i1);
; CHECK: |   (@A)[0][2 * i1 + 2] = %conv;
; CHECK: + END LOOP



source_filename = "WeakZeroSrcSIV1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x float] zeroinitializer, align 16
@B = common global [100 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @sub8(i64 %n) #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %.lcssa = phi i32 [ %ld, %for.body ]
  store i32 %.lcssa, ptr bitcast (ptr getelementptr inbounds ([100 x float], ptr @B, i64 1, i64 0) to ptr), align 16
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %ld = load i32, ptr bitcast (ptr getelementptr inbounds ([100 x float], ptr @A, i64 0, i64 2) to ptr), align 8
  %conv = sitofp i64 %i.05 to float
  %mul = shl i64 %i.05, 1
  %add = add nuw nsw i64 %mul, 2
  %arrayidx = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %add
  store float %conv, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond = icmp eq i64 %inc, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

