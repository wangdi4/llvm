; RUN: opt -S -mattr=+avx512vl,+avx512cd -vplan-force-vf=4 -vplan-num-gathers-threshold=4 -vplan-num-vconflict-threshold=2 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -S -mattr=+avx512vl,+avx512cd -vplan-force-vf=4 -vplan-num-gathers-threshold=4 -vplan-num-vconflict-threshold=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=VEC

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test has 4 gathers and 2 vconflict idioms and includes positive and
; negative tests for vectorization using the gather/vconflict thresholds.

; CHECK-NOT: llvm.x86.avx512.conflict.q.256
; VEC: llvm.x86.avx512.conflict.q.256

; Function Attrs: argmemonly nofree norecurse nosync nounwind uwtable
define dso_local void @foo(i32* noalias nocapture noundef %a, i32* noalias nocapture noundef readonly %b, i32* noalias nocapture noundef readonly %c, i32* noalias nocapture noundef %d, i32* noalias nocapture noundef readonly %e) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i64 %idxprom1
  %1 = load i32, i32* %arrayidx2, align 4
  %arrayidx4 = getelementptr inbounds i32, i32* %c, i64 %idxprom1
  %2 = load i32, i32* %arrayidx4, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, i32* %arrayidx2, align 4
  %arrayidx8 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx8, align 4
  %idxprom9 = sext i32 %3 to i64
  %arrayidx10 = getelementptr inbounds i32, i32* %d, i64 %idxprom9
  %4 = load i32, i32* %arrayidx10, align 4
  %arrayidx12 = getelementptr inbounds i32, i32* %e, i64 %idxprom9
  %5 = load i32, i32* %arrayidx12, align 4
  %add13 = add nsw i32 %5, %4
  store i32 %add13, i32* %arrayidx10, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 128
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}
