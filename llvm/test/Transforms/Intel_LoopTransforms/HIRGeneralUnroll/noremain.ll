; Test for General unrolling with single level loop
; with statement , a[i] = a[n*i].
; This test should not produce any remainder loop.

; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-general-unroll,print<hir>" -S < %s 2>&1 | FileCheck %s
; HIR Check.
; CHECK: REGION { modified }
; Check Unrolled loop.
; CHECK:  DO i1 = 0, 34, 1
; CHECK: (@a)[0][8 * {{.*}}(%n) * i1]
; CHECK: END LOOP
; No Remainder loop.
; CHECK-NOT: DO_LOOP
; CHECK-NEXT: END REGION

; Check the proper optreport is emitted for Partially Unrolled loop (without remainder).
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low %s 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #25438: Loop unrolled without remainder by {{[0-9]+}}
; OPTREPORT-NEXT: LOOP END

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [280 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @subx(i32 %n) {
entry:
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = mul nsw i64 %indvars.iv, %0
  %arrayidx = getelementptr inbounds [280 x i32], ptr @a, i64 0, i64 %1
  %2 = load i32, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [280 x i32], ptr @a, i64 0, i64 %indvars.iv
  store i32 %2, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 280
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture)

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture)


