; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we do not collapse loops in the presence of unroll & jam pragma.

; CHECK-NOT: modified
; CHECK: DO i2

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc10, %entry
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.inc10 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x float]], ptr @B, i64 0, i64 %indvars.iv22, i64 %indvars.iv
  %0 = load float, ptr %arrayidx5, align 4
  %arrayidx9 = getelementptr inbounds [100 x [100 x float]], ptr @A, i64 0, i64 %indvars.iv22, i64 %indvars.iv
  %1 = load float, ptr %arrayidx9, align 4
  %add = fadd float %0, %1
  store float %add, ptr %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc10, label %for.body3

for.inc10:                                        ; preds = %for.body3
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next23, 100
  br i1 %exitcond24, label %for.end12, label %for.cond1.preheader, !llvm.loop !8

for.end12:                                        ; preds = %for.inc10
  ret void
}

!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.unroll_and_jam.count", i32 4}
