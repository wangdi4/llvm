
; expect both i1 and i2 references
; RUN: opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze < %s | FileCheck %s
; CHECK: DD graph for function:
; CHECK-DAG: {al:4}(@A)[0][i2] --> {al:4}(@A)[0][i2] OUTPUT
; CHECK-DAG: {al:4}(@A)[0][i1] --> {al:4}(@A)[0][i2] FLOW
; CHECK-DAG: {al:4}(@A)[0][i2] --> {al:4}(@A)[0][i1] ANTI

; same as region wide for this particular single loop nest region
; RUN: opt -hir-dd-analysis  -hir-dd-analysis-verify=L1 -analyze < %s | FileCheck --check-prefix=L1 %s
; L1: DD graph for function:
; L1-DAG: {al:4}(@A)[0][i2] --> {al:4}(@A)[0][i2] OUTPUT
; L1-DAG: {al:4}(@A)[0][i2] --> {al:4}(@A)[0][i2] OUTPUT
; L1-DAG: {al:4}(@A)[0][i1] --> {al:4}(@A)[0][i2] FLOW
; L1-DAG: {al:4}(@A)[0][i2] --> {al:4}(@A)[0][i1] ANTI

; only i2 refs, the a[i1] is in l1
; RUN: opt -hir-dd-analysis -hir-dd-analysis-verify=Innermost -analyze < %s | FileCheck --check-prefix=INNER %s
; INNER-NOT: {al:4}(@A)[0][i1] --> {al:4}(@A)[0][i2] FLOW
; INNER: {al:4}(@A)[0][i2] --> {al:4}(@A)[0][i2] OUTPUT

; same as innermost
; RUN: opt -hir-dd-analysis -hir-dd-analysis-verify=L2 -analyze < %s | FileCheck --check-prefix=L2 %s
; L2-NOT: {al:4}(@A)[0][i1] --> {al:4}(@A)[0][i2] FLOW
; L2: {al:4}(@A)[0][i2] --> {al:4}(@A)[0][i2] OUTPUT

; no graph
; RUN: opt -hir-dd-analysis -hir-dd-analysis-verify=L3 -analyze < %s| FileCheck --check-prefix=NONE %s
; NONE: DD graph for function:
; NONE-NOT: -->

; overwrite innermost answers with region wide
; RUN: opt -hir-dd-analysis -hir-dd-analysis-verify=L2,Region -analyze < %s | FileCheck --check-prefix=INOUT %s
; INOUT: DD graph for function:
; INOUT-DAG: {al:4}(@A)[0][i2] --> {al:4}(@A)[0][i2] OUTPUT
; INOUT-DAG: {al:4}(@A)[0][i1] --> {al:4}(@A)[0][i2] FLOW

; ModuleID = 'test.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [128 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @_Z3foov() #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.3
  ret void

for.body:                                         ; preds = %for.cond.cleanup.3, %entry
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.cond.cleanup.3 ]
  %arrayidx = getelementptr inbounds [128 x i32], [128 x i32]* @A, i64 0, i64 %indvars.iv22
  %0 = trunc i64 %indvars.iv22 to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !1
  br label %for.body.4

for.cond.cleanup.3:                               ; preds = %for.body.4
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next23, 128
  br i1 %exitcond24, label %for.cond.cleanup, label %for.body

for.body.4:                                       ; preds = %for.body.4, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body.4 ]
  %arrayidx6 = getelementptr inbounds [128 x i32], [128 x i32]* @A, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %add = add nsw i32 %1, 1
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 128
  br i1 %exitcond, label %for.cond.cleanup.3, label %for.body.4
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 814) (llvm/branches/loopopt 845)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
