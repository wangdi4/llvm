; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-memory-reduction-sinking -print-after=hir-memory-reduction-sinking < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that we do not sink invariant reduction if the dependent non-linear
; 'reduction like' instruction's temp escapes (is used elsewhere in the loop).

; Here, the reduction temp %0 is used in unrelated store (@B)[0][i1] = %0 + 2.

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   (@A)[0][i1] = %0 + 2;
; CHECK: |   %1 = (@A)[0][5];
; CHECK: |   (@A)[0][5] = %1 + 3;
; CHECK: |   %2 = (@A)[0][8];
; CHECK: |   (@A)[0][8] = %2 + 9;
; CHECK: |   (@B)[0][i1] = %0 + 2;
; CHECK: + END LOOP


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
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, 2
  store i32 %add, i32* %arrayidx, align 4
  %1 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 5), align 4
  %add3 = add nsw i32 %1, 3
  store i32 %add3, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 5), align 4
  %2 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 8), align 16
  %add4 = add nsw i32 %2, 9
  store i32 %add4, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 8), align 16
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

