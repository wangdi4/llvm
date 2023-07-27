; RUN: opt -passes="hir-ssa-deconstruction,hir-last-value-computation,hir-post-vec-complete-unroll,print<hir>,hir-cg" -S 2>&1 < %s | FileCheck %s

; Verify that we skip processing of liveouts from the normal region exit when
; it is optimized away. In this case the early exit becomes unconditional after
; complete unroll of the loop and subsequent simplification of IR.

; Incoming HIR-
; + DO i1 = 0, 1, 1   <DO_MULTI_EXIT_LOOP>
; |   %g.promoted.out = ((-1 * trunc.i64.i8(%1)) + %0) * i1 + 20;
; |   %3 = (@d)[0][3 * i1 + 2];
; |   %add = %3  +  16;
; |   %conv1137 = %conv1137  +  zext.i16.i64(%3) + 16;
; |   if (3 * i1 + 2 >=u 5)
; |   {
; |      goto for.end28;
; |   }
; |   %conv20 = ((-1 * trunc.i64.i8(%1)) + %0) * i1 + 20  -  %1 + -1 * zext.i8.i64(%0);
; |   (@g)[0] = ((-1 * trunc.i64.i8(%1)) + %0) * i1 + %0 + -1 * trunc.i64.i8(%1) + 20;
; + END LOOP


; CHECK: BEGIN REGION { modified }
; CHECK: %3 = (@d)[0][2];
; CHECK: %add = %3  +  16;
; CHECK: %conv1137 = %conv1137  +  zext.i16.i64(%3) + 16;
; CHECK: (@g)[0] = %0 + -1 * trunc.i64.i8(%1) + 20;
; CHECK: %3 = (@d)[0][5];
; CHECK: %add = %3  +  16;
; CHECK: %conv1137 = %conv1137  +  zext.i16.i64(%3) + 16;
; CHECK: %g.promoted.out = ((-1 * trunc.i64.i8(%1)) + %0) + 20;
; CHECK: goto for.end28;
; CHECK: END REGION

; Verify that only one phi operand is added to %add.lcssa for the single
; unconditional exit from region.
; Before the fix, we were adding two operands to the phi one each for early
; region exit and normal region exit.

; CHECK: for.end28:
; CHECK-NEXT: %add.lcssa = phi i64 [ %add, %for.body6.split ], [ %add, %for.body17 ], [ {{%.*}}, %region{{.*}} ]{{[[:space:]]}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i64 0, align 8
@g = dso_local local_unnamed_addr global i8 0, align 1
@l = dso_local local_unnamed_addr global i32 0, align 4
@f = dso_local local_unnamed_addr global i64 0, align 8
@b = dso_local local_unnamed_addr global i8 0, align 1
@d = dso_local local_unnamed_addr global [20 x i16] zeroinitializer, align 16

define dso_local i32 @main() {
entry:
  store i8 20, ptr @g, align 1
  %0 = load i8, ptr @b, align 1
  %conv2 = zext i8 %0 to i64
  %1 = load i64, ptr @a, align 8
  %sub = sub i64 %1, %conv2
  store i64 %sub, ptr @a, align 8
  %2 = trunc i64 %sub to i8
  %l.promoted = load i32, ptr @l, align 4
  br label %for.body6

for.body6:                                        ; preds = %entry, %for.body17
  %g.promoted = phi i8 [ 20, %entry ], [ %conv20, %for.body17 ]
  %indvars.iv = phi i64 [ 2, %entry ], [ %indvars.iv.next, %for.body17 ]
  %conv1137 = phi i32 [ %l.promoted, %entry ], [ %conv11, %for.body17 ]
  %arrayidx = getelementptr inbounds [20 x i16], ptr @d, i64 0, i64 %indvars.iv
  %3 = load i16, ptr %arrayidx, align 2
  %conv7 = zext i16 %3 to i64
  %add = add nuw nsw i64 %conv7, 16
  %4 = trunc i64 %add to i32
  %conv11 = add i32 %conv1137, %4
  %cmp1533 = icmp ult i64 %indvars.iv, 5
  br i1 %cmp1533, label %for.body17, label %for.end28

for.body17:                                       ; preds = %for.body6
  %conv20 = sub i8 %g.promoted, %2
  store i8 %conv20, ptr @g, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp4 = icmp ult i64 %indvars.iv, 3
  br i1 %cmp4, label %for.body6, label %for.end28

for.end28:                                        ; preds = %for.body6, %for.body17
  %add.lcssa = phi i64 [ %add, %for.body6 ], [ %add, %for.body17 ]
  %conv11.lcssa = phi i32 [ %conv11, %for.body6 ], [ %conv11, %for.body17 ]
  %5 = phi i8 [ %conv20, %for.body17 ], [ %g.promoted, %for.body6 ]
  ret i32 0
}



