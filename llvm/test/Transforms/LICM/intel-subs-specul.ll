; CMPLRLLVM-30152
; The store writes to constant offset 0 in the single-element stack allocated
; i32 "var$5". It is conditional on the branch in bb28.
; The store can be moved to the loop exit, as it is speculatable (inbounds to
; non-null memory) and constant.
; accumulateConstantOffset must reduce the subscript to offset 0 by computing
; (offset-LB)*stride, so that
; isDereferenceableAndAlignedPointer can see that the access is always
; dereferenceable.

; RUN: opt -S -licm %s | FileCheck %s
; CHECK-LABEL: loop_test33.loop_body38_crit_edge
; CHECK: store{{.*}}lcssa{{.*}}var$5

source_filename = "gas_dyn2.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @chozdt_() local_unnamed_addr #0 {
alloca_5:
  %"var$5" = alloca i32, align 16
  %"var$5[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"var$5", i64 1)
  br label %loop_body34

bb27:                                             ; preds = %loop_body34
  %int_sext14 = trunc i64 %"$loop_ctr8.064" to i32
  store i32 %int_sext14, i32* %"var$5[]", align 1
  br label %bb28

bb28:                                             ; preds = %loop_body34, %bb27
  br i1 undef, label %loop_test33.loop_body38_crit_edge, label %loop_body34

loop_body34:                                      ; preds = %bb28, %alloca_5
  %"$loop_ctr8.064" = phi i64 [ 1, %alloca_5 ], [ 0, %bb28 ]
  br i1 false, label %bb27, label %bb28

loop_test33.loop_body38_crit_edge:                ; preds = %bb28
  unreachable
}

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

attributes #0 = { "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
