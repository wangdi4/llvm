; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that the inner loop is turned into unknown loop because the upper is
; parsed as non-linear.
; It was previously being parsed as the following which was causing assertion-

; |      + DO i2 = 0, umax(4, (1 + %0)) + -4, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; |      |   %inc = i2 + 3  +  1;
; |      |   %0 = trunc.i64.i8(5 * i1);
; |      + END LOOP

; This can happen when a loop invariant computation (%0) is not hoisted out of
; the loop and becomes a blob during parsing.

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %inc.lcssa12 = 3;
; CHECK: |   if (-1 * i1 + -1 == 0)
; CHECK: |   {
; CHECK: |      (@h)[0][5 * i1] = 0;
; CHECK: |
; CHECK: |      + UNKNOWN LOOP i2
; CHECK: |      |   <i2 = 0>
; CHECK: |      |   for.body4:
; CHECK: |      |   %inc = i2 + 3  +  1;
; CHECK: |      |   if (i2 + 4 <=u 5 * i1)
; CHECK: |      |   {
; CHECK: |      |      <i2 = i2 + 1>
; CHECK: |      |      goto for.body4;
; CHECK: |      |   }
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      %inc.lcssa12 = %inc;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@d = dso_local local_unnamed_addr global i8 0, align 1
@a = dso_local local_unnamed_addr global i32 0, align 4
@h = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16

define dso_local i32 @main() {
entry:
  store i32 0, i32* @a, align 4
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc5
  %cmp28 = phi i1 [ true, %entry ], [ false, %for.inc5 ]
  %cmp = phi i1 [ true, %entry ], [ false, %for.inc5 ]
  %indvars.iv = phi i64 [ 0, %entry ], [ 5, %for.inc5 ]
  br i1 %cmp28, label %for.inc5, label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %arrayidx = getelementptr inbounds [20 x i32], [20 x i32]* @h, i64 0, i64 %indvars.iv
  store i32 0, i32* %arrayidx, align 4
  br label %for.body4

for.body4:                                        ; preds = %for.body4.lr.ph, %for.body4
  %storemerge79 = phi i8 [ 3, %for.body4.lr.ph ], [ %inc, %for.body4 ]
  %inc = add i8 %storemerge79, 1
  %0 = trunc i64 %indvars.iv to i8
  %cmp2 = icmp ugt i8 %inc, %0
  br i1 %cmp2, label %for.inc5.loopexit, label %for.body4

for.inc5.loopexit:                                ; preds = %for.body4
  %inc.lcssa = phi i8 [ %inc, %for.body4 ]
  br label %for.inc5

for.inc5:                                         ; preds = %for.inc5.loopexit, %for.cond1.preheader
  %inc.lcssa12 = phi i8 [ 3, %for.cond1.preheader ], [ %inc.lcssa, %for.inc5.loopexit ]
  br i1 %cmp, label %for.cond1.preheader, label %for.end6

for.end6:                                         ; preds = %for.inc5
  %inc.lcssa12.lcssa = phi i8 [ %inc.lcssa12, %for.inc5 ]
  store i8 %inc.lcssa12.lcssa, i8* @d, align 1
  store i32 10, i32* @a, align 4
  ret i32 0
}

