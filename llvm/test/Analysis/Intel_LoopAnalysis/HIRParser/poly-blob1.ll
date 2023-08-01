; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that polynomial blob %4 is parsed correctly.

; CHECK: + DO i1 = 0, undef, 1   <DO_LOOP>
; CHECK: |   %4 = 4 * i1  *  i1 + 4294967295;
; CHECK: |   %5 = %4  *  undef;
; CHECK: |   %6 = %5  /  3;
; CHECK: |   %7 = i1  <<  1;
; CHECK: |   %11 = i1  -  1;
; CHECK: |   %13 = trunc.i64.i32(((1 + sext.i32.i64(%7)) * %11));
; CHECK: |   %18 = (%f)[0][sext.i32.i64((undef + %6 + %13)) + -1];
; CHECK: + END LOOP


; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-6e88369.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

module asm "\09.ident\09\22GCC: (GNU) 4.8.5 LLVM: 4.0.1\22"

; Function Attrs: norecurse nounwind uwtable
define void @irotlt_(ptr noalias nocapture readonly %f) unnamed_addr {
entry:
  br label %"3"

"3":                                              ; preds = %"3", %entry
  %indvars.iv6 = phi i64 [ 0, %entry ], [ %indvars.iv.next7, %"3" ]
  %indvars.iv.next7 = add nuw nsw i64 %indvars.iv6, 1
  %0 = trunc i64 %indvars.iv6 to i32
  %1 = shl i32 %0, 2
  %2 = add i64 %indvars.iv6, 4294967295
  %3 = trunc i64 %2 to i32
  %4 = mul i32 %1, %3
  %5 = mul i32 %4, undef
  %6 = sdiv i32 %5, 3
  %7 = shl nsw i32 %0, 1
  %8 = or i32 %7, 1
  %9 = add i32 undef, %6
  %10 = sext i32 %8 to i64
  %11 = sub nsw i64 %indvars.iv6, 1
  %12 = mul nsw i64 %11, %10
  %13 = trunc i64 %12 to i32
  %14 = add i32 %9, %13
  %15 = sext i32 %14 to i64
  %16 = add nsw i64 %15, -1
  %17 = getelementptr inbounds [0 x double], ptr %f, i64 0, i64 %16
  %18 = load double, ptr %17, align 8
  %19 = icmp eq i64 %indvars.iv6, undef
  br i1 %19, label %return.loopexit, label %"3"

return.loopexit:                                  ; preds = %"3"
  ret void
}

