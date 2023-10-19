; RUN: opt -passes="hir-ssa-deconstruction,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that first rval operand of first instruction simplifies to 67108863.
; Previously it was incorrectly simplifying to 274877906688.

; Note that there is a doubly nested loop in incoming IR but the inner loop is
; optimized away by framework as it becomes empty after parsing.

; The SCEV form of first rval operand %shr.i is ({4294967295,+,-1}<nsw><%for.inner> /u 64)
; where %for.inner represents the inner loop header.

; It simplifies to a constant in the outer loop because the inner loop trip count is 4.

; CHECK: + DO i1 = 0, %.pr, 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>  <LEGAL_MAX_TC = 2147483648>
; CHECK: |   %xor.narrow.i.i = 67108863  ^  %t0;
; CHECK: |   %t1 = (@a)[0][trunc.i64.i8(%xor.narrow.i.i)];
; CHECK: |   %idxprom.i6.i = 255  ^  trunc.i32.i8(%t1);
; CHECK: |   %t3 = (@a)[0][%idxprom.i6.i];
; CHECK: |   %t0 = %t3;
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = external dso_local local_unnamed_addr global [256 x i32], align 16

; Function Attrs: nofree nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @main(i32 %.pr, i32 %c.promoted) local_unnamed_addr #0 {
  %j = alloca i32, align 4
  %cmp15 = icmp sgt i32 %.pr, -1
  br i1 %cmp15, label %for.outer.preheader, label %for.end6

for.outer.preheader:                    ; preds = %entry
  br label %for.outer

for.outer:                              ; preds = %for.end, %for.outer.preheader
  %t0 = phi i32 [ %t3, %for.end ], [ %c.promoted, %for.outer.preheader ]
  %dec51017 = phi i32 [ %dec5, %for.end ], [ %.pr, %for.outer.preheader ]
  br label %for.inner

for.inner:                                          ; preds = %for.inner, %for.outer
  %b.09 = phi i32 [ 0, %for.outer ], [ %dec, %for.inner ]
  %storemerge8 = phi i32 [ 0, %for.outer ], [ %inc, %for.inner ]
  %dec = add nsw i32 %b.09, -1
  %inc = add nuw nsw i32 %storemerge8, 1
  %exitcond.not = icmp eq i32 %inc, 4
  br i1 %exitcond.not, label %for.end, label %for.inner

for.end:                                          ; preds = %for.inner
  %dec.lcssa = phi i32 [ %dec, %for.inner ]
  %conv18 = zext i32 %dec.lcssa to i64
  %shr.i = lshr i64 %conv18, 6
  %.tr.i.i = zext i32 %t0 to i64
  %xor.narrow.i.i = xor i64 %shr.i, %.tr.i.i
  %idxprom.i.i = and i64 %xor.narrow.i.i, 255
  %arrayidx.i.i = getelementptr inbounds [256 x i32], ptr @a, i64 0, i64 %idxprom.i.i
  %t1 = load i32, ptr %arrayidx.i.i, align 4
  %shr1.i = lshr i64 %conv18, 24
  %t2 = and i32 %t1, 255
  %.tr.i4.i.masked = zext i32 %t2 to i64
  %idxprom.i6.i = xor i64 %shr1.i, %.tr.i4.i.masked
  %arrayidx.i7.i = getelementptr inbounds [256 x i32], ptr @a, i64 0, i64 %idxprom.i6.i
  %t3 = load i32, ptr %arrayidx.i7.i, align 4
  %dec5 = add nsw i32 %dec51017, -1
  %cmp = icmp sgt i32 %dec51017, 0
  br i1 %cmp, label %for.outer, label %for.cond.for.end6_crit_edge

for.cond.for.end6_crit_edge:                      ; preds = %for.end
  %.lcssa = phi i32 [ %t3, %for.end ]
  br label %for.end6

for.end6:                                         ; preds = %for.cond.for.end6_crit_edge, %entry
  ret i32 0
}

