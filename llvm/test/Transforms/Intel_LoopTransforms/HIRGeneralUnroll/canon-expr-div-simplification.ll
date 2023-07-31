; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,print<hir>" -S < %s 2>&1 | FileCheck %s

; HIR-
; + DO i1 = 0, 63, 1   <DO_LOOP>
; |   %or = %1  ||  19;
; |   (@a1_ly)[0][0] = %or;
; |   %xor = (@a1_ly)[0][i1]  ^  (i1 + -24)/u90;
; |   (@a1_ly)[0][i1] = %xor;
; |   %5 = (@a1_ly)[0][0];
; |   %1 = %5;
; + END LOOP

; Verify that we do not simplify the canon expr by dividing numerator and denominator by GCD.

; CHECK: %xor = (@a1_ly)[0][4 * i1]  ^  (4 * i1 + -24)/u90;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a1_ly = common dso_local local_unnamed_addr global [192 x i64] zeroinitializer, align 16
@.str = private unnamed_addr constant [5 x i8] c"%lu\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i64, ptr @a1_ly, align 16
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = phi i64 [ %0, %entry ], [ %5, %for.body ]
  %or = or i64 %1, 19
  store i64 %or, ptr @a1_ly, align 16
  %2 = trunc i64 %indvars.iv to i32
  %3 = add i32 %2, -24
  %div = udiv i32 %3, 90
  %conv1 = zext i32 %div to i64
  %arrayidx = getelementptr inbounds [192 x i64], ptr @a1_ly, i64 0, i64 %indvars.iv
  %4 = load i64, ptr %arrayidx, align 8
  %xor = xor i64 %4, %conv1
  store i64 %xor, ptr %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %5 = load i64, ptr @a1_ly, align 16
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %.lcssa = phi i64 [ %5, %for.body ]
  ret i32 0
}

