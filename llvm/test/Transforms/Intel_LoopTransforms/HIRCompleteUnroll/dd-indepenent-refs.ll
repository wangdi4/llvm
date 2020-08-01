; RUN: opt -hir-ssa-deconstruction -scoped-noalias-aa -hir-temp-cleanup -hir-runtime-dd -hir-loop-interchange -hir-pre-vec-complete-unroll -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-loop-interchange,print<hir>,hir-pre-vec-complete-unroll,print<hir>" -aa-pipeline="scoped-noalias-aa" 2>&1 < %s | FileCheck %s

; Check that loopnest was multiversioned and interchanged before complete unroll.

; CHECK: Function

; CHECK: if (%mv
; CHECK: {
; CHECK: + DO i1 = 0, %stride1 + -1, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 24, 1   <DO_LOOP>
; CHECK: |   |   + DO i3 = 0, 24, 1   <DO_LOOP>
; CHECK: |   |   |   %9 = (%ptr2)[25 * i1 + i2 + %stride2]  *  (%ptr1)[%stride1 * i2 + i3];
; CHECK: |   |   |   %13 = (%ptr3)[25 * i1 + i3]  +  %9;
; CHECK: |   |   |   (%ptr3)[25 * i1 + i3] = %13;
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK: + DO i1 = 0, 24, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 24, 1   <DO_LOOP>
; CHECK: |   |   + DO i3 = 0, %stride1 + -1, 1   <DO_LOOP>
; CHECK: |   |   |   %9 = (%ptr2)[i1 + 25 * i3 + %stride2]  *  (%ptr1)[%stride1 * i1 + i2];
; CHECK: |   |   |   %13 = (%ptr3)[i2 + 25 * i3]  +  %9;
; CHECK: |   |   |   (%ptr3)[i2 + 25 * i3] = %13;
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: }

; Check that i3 loop inside if is unrolled.
; CHECK: Function

; CHECK: if (%mv
; CHECK: DO i2
; CHECK-NOT: DO i3
; CHECK: else

target datalayout = "p:32:32"

define void @foo(float* %ptr1, float* %ptr2, float* %ptr3, i32 %stride1, i32 %stride2) {
entry:
  br label %outer

outer:                                    ; preds = %entry, %middle.exit
  %iv.o = phi i32 [ %iv.next.o, %middle.exit ], [ 0, %entry ]
  %0 = mul nsw i32 %iv.o, %stride1
  %1 = add nuw i32 %iv.o, %stride2
  br label %middle

middle:                                    ; preds = %inner.exit, %inner.exit
  %iv.m = phi i32 [ 0, %outer ], [ %iv.next.m, %inner.exit ]
  %2 = add nsw i32 %iv.m, %0
  %3 = getelementptr inbounds float, float* %ptr1, i32 %2
  br label %inner

inner:                                    ; preds = %inner, %middle
  %iv.i = phi i32 [ 0, %middle ], [ %iv.next.i, %inner ]
  %4 = load float, float* %3, align 4
  %5 = mul nuw nsw i32 %iv.i, 25
  %6 = add i32 %1, %5
  %7 = getelementptr inbounds float, float* %ptr2, i32 %6
  %8 = load float, float* %7, align 4
  %9 = fmul fast float %8, %4
  %10 = add nuw nsw i32 %5, %iv.m
  %11 = getelementptr inbounds float, float* %ptr3, i32 %10
  %12 = load float, float* %11, align 4
  %13 = fadd fast float %12, %9
  store float %13, float* %11, align 4
  %iv.next.i = add nuw nsw i32 %iv.i, 1
  %14 = icmp eq i32 %iv.next.i, %stride1
  br i1 %14, label %inner.exit, label %inner

inner.exit:                                    ; preds = %inner
  %iv.next.m = add nuw nsw i32 %iv.m, 1
  %15 = icmp eq i32 %iv.next.m, 25
  br i1 %15, label %middle.exit, label %middle

middle.exit:                                    ; preds = %inner.exit
  %iv.next.o = add nuw nsw i32 %iv.o, 1
  %16 = icmp eq i32 %iv.next.o, 25
  br i1 %16, label %exit, label %outer

exit:
  ret void
}

