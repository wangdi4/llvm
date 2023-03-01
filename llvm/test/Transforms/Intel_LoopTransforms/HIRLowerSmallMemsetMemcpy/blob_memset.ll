; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; The test checks that memset intrinsic got recognized by
; HIR Lower Small Memset/Memcpy pass for non-constant memset.

; HIR before optimization:
;            BEGIN REGION { }
;                  + DO i1 = 0, 9, 1   <DO_LOOP>
;                  |   @llvm.memset.p0.i64(&((%arr)[0][i1][0]),  %c,  32,  0);
;                  + END LOOP
;            END REGION

; HIR after optimization:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 31, 1   <DO_LOOP>
; CHECK:           |   |   (%arr)[0][i1][i2] = %c;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind memory(none) uwtable
define dso_local signext i8 @foo(i32 noundef %n, i8 noundef signext %c) local_unnamed_addr {
entry:
  %arr = alloca [10 x [40 x i8]], align 16
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %idxprom3 = sext i32 %n to i64
  %arrayidx4 = getelementptr inbounds [10 x [40 x i8]], ptr %arr, i64 0, i64 3, i64 %idxprom3
  %0 = load i8, ptr %arrayidx4, align 1
  ret i8 %0

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds [10 x [40 x i8]], ptr %arr, i64 0, i64 %indvars.iv, i64 0
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(32) %arrayidx1, i8 %c, i64 32, i1 false)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

attributes #2 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }

