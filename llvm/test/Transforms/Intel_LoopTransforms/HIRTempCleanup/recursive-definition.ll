; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup -hir-details -disable-output 2>&1 | FileCheck %s

; Verify that lval ref %s.010 is made a self blob after substituting %s.010.out
; by %s.010 in its rval to prevent a recursive definition.

; CHECK: Dump Before

; CHECK: + DO i64 i1 = 0, 1023, 1   <DO_LOOP>
; CHECK: |   %s.010.out = %s.010;
; CHECK: |   %0 = (@a)[0][i1];
; CHECK: |   %1 = (@b)[0][i1];
; CHECK: |   %add = smax((sext.i8.i32(%0) + (-1 * sext.i8.i32(%1))), (sext.i8.i32(%1) + (-1 * sext.i8.i32(%0))))  +  %s.010.out;
; CHECK: |   %s.010 = smax((sext.i8.i32(%0) + (-1 * sext.i8.i32(%1))), (sext.i8.i32(%1) + (-1 * sext.i8.i32(%0)))) + %s.010.out;
; CHECK: |   <LVAL-REG> NON-LINEAR i32 smax((sext.i8.i32(%0) + (-1 * sext.i8.i32(%1))), (sext.i8.i32(%1) + (-1 * sext.i8.i32(%0)))) + %s.010.out
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: |   %s.010 = smax((sext.i8.i32(%0) + (-1 * sext.i8.i32(%1))), (sext.i8.i32(%1) + (-1 * sext.i8.i32(%0)))) + %s.010;
; CHECK-NEXT: |   <LVAL-REG> NON-LINEAR i32 %s.010


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1024 x i8] zeroinitializer, align 64
@b = dso_local local_unnamed_addr global [1024 x i8] zeroinitializer, align 64

define dso_local noundef i32 @_Z3foov() {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %s.010 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i8], [1024 x i8]* @a, i64 0, i64 %indvars.iv
  %0 = load i8, i8* %arrayidx, align 1
  %conv = sext i8 %0 to i32
  %arrayidx2 = getelementptr inbounds [1024 x i8], [1024 x i8]* @b, i64 0, i64 %indvars.iv
  %1 = load i8, i8* %arrayidx2, align 1
  %conv3 = sext i8 %1 to i32
  %sub = sub nsw i32 %conv, %conv3
  %2 = tail call i32 @llvm.abs.i32(i32 %sub, i1 true)
  %add = add nuw nsw i32 %2, %s.010
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

declare i32 @llvm.abs.i32(i32, i1 immarg) #1

attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

