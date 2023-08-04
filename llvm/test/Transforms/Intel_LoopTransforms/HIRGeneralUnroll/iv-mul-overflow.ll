; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll" -print-before=hir-general-unroll -print-after=hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s

; Verify that the test case executes successfully. Verifier was asserting that
; we did not set ConstantSymbase for the modified refs. The issue was that the
; IV coefficient in the loop is big enough that multiplying it with the UF 
; overflows the result. The new IV coefficient becomes zero which is equivalent
; to removing the IV and making the ref constant.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 283, 1   <DO_LOOP>
; CHECK: |   %sext = 4611686018427387904 * i1  >>  63;
; CHECK: |   %1 = %sext  ^  i1;
; CHECK: |   (@a)[0][i1] = %1;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: + DO i1 = 0, 34, 1   <DO_LOOP> <nounroll>
; CHECK: |   %sext = 0  >>  63;
; CHECK: |   %1 = %sext  ^  8 * i1;
; CHECK: |   (@a)[0][8 * i1] = %1;
; CHECK: |   %sext = 4611686018427387904  >>  63;
; CHECK: |   %1 = %sext  ^  8 * i1 + 1;
; CHECK: |   (@a)[0][8 * i1 + 1] = %1;
; CHECK: |   %sext = -9223372036854775808  >>  63;
; CHECK: |   %1 = %sext  ^  8 * i1 + 2;
; CHECK: |   (@a)[0][8 * i1 + 2] = %1;
; CHECK: |   %sext = -4611686018427387904  >>  63;
; CHECK: |   %1 = %sext  ^  8 * i1 + 3;
; CHECK: |   (@a)[0][8 * i1 + 3] = %1;
; CHECK: |   %sext = 0  >>  63;
; CHECK: |   %1 = %sext  ^  8 * i1 + 4;
; CHECK: |   (@a)[0][8 * i1 + 4] = %1;
; CHECK: |   %sext = 4611686018427387904  >>  63;
; CHECK: |   %1 = %sext  ^  8 * i1 + 5;
; CHECK: |   (@a)[0][8 * i1 + 5] = %1;
; CHECK: |   %sext = -9223372036854775808  >>  63;
; CHECK: |   %1 = %sext  ^  8 * i1 + 6;
; CHECK: |   (@a)[0][8 * i1 + 6] = %1;
; CHECK: |   %sext = -4611686018427387904  >>  63;
; CHECK: |   %1 = %sext  ^  8 * i1 + 7;
; CHECK: |   (@a)[0][8 * i1 + 7] = %1;
; CHECK: + END LOOP


; CHECK: + DO i1 = 280, 283, 1   <DO_LOOP>
; CHECK: |   %sext = 4611686018427387904 * i1  >>  63;
; CHECK: |   %1 = %sext  ^  i1;
; CHECK: |   (@a)[0][i1] = %1;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [284 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @subx(i32 %n) {
entry:
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx2 = getelementptr inbounds [284 x i32], ptr @a, i64 0, i64 %indvars.iv
  %and = shl i64 %indvars.iv, 62
  %sext = ashr i64 %and, 63
  %1 = xor i64 %sext, %indvars.iv
  %conv = trunc i64 %1 to i32
  store i32 %conv, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 284
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}


