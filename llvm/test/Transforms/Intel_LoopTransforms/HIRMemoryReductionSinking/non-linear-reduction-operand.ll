; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-memory-reduction-sinking -print-before=hir-memory-reduction-sinking -print-after=hir-memory-reduction-sinking < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>,hir-memory-reduction-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that we are able to sink invariant memory reduction even if its operand
; (and the dependent reduction's operand) is non-linear (%b).

; Dump Before-

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %b = (@B)[0][i1]
; CHECK: |   %0 = (@A)[0][5];
; CHECK: |   (@A)[0][5] = %b + %0;
; CHECK: |   %1 = (@A)[0][i1];
; CHECK: |   (@A)[0][i1] = %b + %1;
; CHECK: + END LOOP

; Dump After-

; CHECK: modified

; CHECK:   %tmp = 0;
; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %b = (@B)[0][i1];
; CHECK: |   %tmp = %tmp  +  %b;
; CHECK: |   %1 = (@A)[0][i1];
; CHECK: |   (@A)[0][i1] = %b + %1;
; CHECK: + END LOOP
; CHECK:   %0 = (@A)[0][5];
; CHECK:   (@A)[0][5] = %0 + %tmp;


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %b = load i32, i32* %arrayidx6, align 4
  %0 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 5), align 4
  %add3 = add nsw i32 %0, %b
  store i32 %add3, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 5), align 4
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %1, %b
  store i32 %add, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

