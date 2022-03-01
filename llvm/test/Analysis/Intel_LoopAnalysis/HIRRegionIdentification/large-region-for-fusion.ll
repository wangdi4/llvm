; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -analyze -hir-region-identification -debug-only=hir-region-identification < %s  2>&1 | FileCheck %s
; RUN: opt -passes='print<hir-region-identification>' -debug-only=hir-region-identification < %s  2>&1 | FileCheck %s

; Test checks that single region is formed instead of two.

;         BEGIN REGION { }
;            + DO i1 = 0, 99, 1   <DO_LOOP>
;            |   %0 = (@a)[0][i1];
;            |   (@a)[0][i1] = i1 + %0;
;            + END LOOP
;
;            + DO i1 = 0, 99, 1   <DO_LOOP>
;            |   %2 = (@a)[0][i1];
;            |   (@b)[0][i1] = %2 + 1;
;            + END LOOP
;         END REGION

; CHECK: Region 1
; CHECK-NOT: Region 2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv28 = phi i64 [ 0, %entry ], [ %indvars.iv.next29, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %indvars.iv28
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv28 to i32
  %add = add nsw i32 %0, %1
  store i32 %add, i32* %arrayidx, align 4
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond30.not = icmp eq i64 %indvars.iv.next29, 100
  br i1 %exitcond30.not, label %for.body3.preheader, label %for.body

for.body3.preheader:                              ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx5, align 4
  %add6 = add nsw i32 %2, 1
  %arrayidx8 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %indvars.iv
  store i32 %add6, i32* %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end11, label %for.body3

for.end11:                                        ; preds = %for.body3
  %idxprom12 = sext i32 %n to i64
  %arrayidx13 = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %idxprom12
  %3 = load i32, i32* %arrayidx13, align 4
  %arrayidx15 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %idxprom12
  %4 = load i32, i32* %arrayidx15, align 4
  %add16 = add nsw i32 %4, %3
  ret i32 %add16
}

