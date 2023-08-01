; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -debug-only=hir-dd-test -S < %s 2>&1 | FileCheck %s

; Test check that we bailout when trying to compute the AbsDelta
; between A[i+N] and A[i+K] rather than computing it as -(k - n).


;     BEGIN REGION { }
;        + DO i1 = 0, 99, 1   <DO_LOOP>
;        |   %4 = (@A)[0][i1 + sext.i32.i64(%K)];
;        |   (@A)[0][i1 + sext.i32.i64(%N)] = 2 * %4;
;        + END LOOP
;     END REGION


; CHECK:  Test SIV
; CHECK:   src = i1 + sext.i32.i64(%K)
; CHECK:   dst = i1 + sext.i32.i64(%N)
; CHECK: Strong SIV test
; CHECK: Coeff = 1
; CHECK: SrcConst = sext.i32.i64(%K)
; CHECK: DstConst = sext.i32.i64(%N)
; CHECK: Delta = sext.i32.i64(%K) + -1 * sext.i32.i64(%N)
; CHECK-NOT: AbsDelta =
; CHECK:  ~DDTest called


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local noundef i32 @_Z3fooii(i32 noundef %N, i32 noundef %K) local_unnamed_addr {
entry:
  %0 = sext i32 %K to i64
  %1 = sext i32 %N to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %0
  %2 = load i32, ptr %arrayidx5, align 4
  ret i32 %2

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %3 = add nsw i64 %indvars.iv, %0
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %3
  %4 = load i32, ptr %arrayidx, align 4
  %mul = shl nsw i32 %4, 1
  %5 = add nsw i64 %indvars.iv, %1
  %arrayidx3 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %5
  store i32 %mul, ptr %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}
