; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-memory-reduction-sinking -print-after=hir-memory-reduction-sinking < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that we do not sink invariant reduction if the dependent reduction has a different opcode.

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@B)[0][i1];
; CHECK: |   %add = (@A)[0][%0]  *  %t;
; CHECK: |   (@A)[0][%0] = %add;
; CHECK: |   %add7 = (@A)[0][5]  +  %t;
; CHECK: |   (@A)[0][5] = %add7;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(float %t) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %idxprom1
  %1 = load float, float* %arrayidx2, align 4
  %add = fmul fast float %1, %t
  store float %add, float* %arrayidx2, align 4
  %2 = load float, float* getelementptr inbounds ([100 x float], [100 x float]* @A, i64 0, i64 5), align 4
  %add7 = fadd fast float %2, %t
  store float %add7, float* getelementptr inbounds ([100 x float], [100 x float]* @A, i64 0, i64 5), align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

