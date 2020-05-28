; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; Test was compfailing because we tried to check use of %conv2 in %conv124.
; %conv2 does not have a blob index because it is not used inside the region.
; It is only live out of it.

; CHECK: Function: f

; CHECK: + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK: |   %conv124.out = %conv124;
; CHECK: |   %sub = %t0  -  %conv124.out;
; CHECK: |   %conv124 = trunc.i64.i8(%t0 + -1 * sext.i8.i64(%conv124.out));
; CHECK: |   %conv2 = 72057594037927936 * %t0 + -72057594037927936 * sext.i8.i64(%conv124.out)  >>  56;
; CHECK: |   %and = -1 * %conv124.out + trunc.i64.i8(%t0)  &  %t1;
; CHECK: |   (@b)[0][3 * i1] = %and;
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Function: f

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK: |   %sub = %t0  -  %conv124;
; CHECK: |   %conv2 = 72057594037927936 * %t0 + -72057594037927936 * sext.i8.i64(%conv124)  >>  56;
; CHECK: |   %and = trunc.i64.i8(%t0) + -1 * %conv124  &  %t1;
; CHECK: |   %conv124 = trunc.i64.i8(%t0 + -1 * sext.i8.i64(%conv124));
; CHECK: |   (@b)[0][3 * i1] = %and;
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common dso_local local_unnamed_addr global [1 x i32] zeroinitializer, align 4

; Function Attrs: norecurse nounwind uwtable
define dso_local void @f(i64 %t0, i16 %t1, i8 %e.promoted) {
entry:
  %conv3 = sext i16 %t1 to i64
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %conv124 = phi i8 [ %e.promoted, %entry ], [ %conv1, %for.body ]
  %conv = sext i8 %conv124 to i64
  %sub = sub nsw i64 %t0, %conv
  %conv1 = trunc i64 %sub to i8
  %sext = shl i64 %sub, 56
  %conv2 = ashr exact i64 %sext, 56
  %and = and i64 %conv2, %conv3
  %conv5 = trunc i64 %and to i32
  %arrayidx = getelementptr inbounds [1 x i32], [1 x i32]* @b, i64 0, i64 %indvars.iv
  store i32 %conv5, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp = icmp ult i64 %indvars.iv.next, 8
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  %sub.lcssa = phi i64 [ %sub, %for.body ]
  %conv2.lcssa = phi i64 [ %conv2, %for.body ]
  ret void
}

