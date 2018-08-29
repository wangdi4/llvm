; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>" < %s 2>&1 | FileCheck %s

; Verify that unroll does not trigger loop due to presence of "llvm.loop.unroll.disable" pragma.
; CHECK: BEGIN REGION { }
; CHECK: DO i1 = 0, 99, 1   <DO_LOOP> <nounroll>
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [80 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @subx(i32 %n) {
entry:
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = mul nsw i64 %indvars.iv, %0
  %arrayidx = getelementptr inbounds [80 x i32], [80 x i32]* @a, i64 0, i64 %1
  %2 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [80 x i32], [80 x i32]* @a, i64 0, i64 %indvars.iv
  store i32 %2, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !0

for.end:                                          ; preds = %for.body
  ret i32 0
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.disable"}
