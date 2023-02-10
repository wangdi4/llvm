; RUN: opt -passes='sycl-kernel-resolve-sub-group-wi-call' -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s
; RUN: opt -passes='debugify,sycl-kernel-resolve-sub-group-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; Checks that "phi(undef, get_sub_group_rowslice_id())" pattern can be correctly resolved.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test() {
entry:
  br label %pred

pred:                                             ; preds = %pred.call.continue, %entry
  br i1 false, label %pred.call.continue, label %pred.call.if

pred.call.if:                                     ; preds = %pred
; CHECK-LABEL: pred.call.if:
; CHECK-NEXT: %0 = add i64 0, 0
; CHECK-NOT: call{{.*}}get_sub_group_rowslice_id
  %0 = add i64 0, 0
  %1 = call i64 @get_sub_group_rowslice_id.v256i8.i64(<256 x i8> zeroinitializer, i32 16, i32 16, i64 %0)
  br label %pred.call.continue

pred.call.continue:                               ; preds = %pred.call.if, %pred
; CHECK-LABEL: pred.call.continue:
; CHECK-NEXT: [[MATRIX:%.*]] = phi <256 x i8> [ zeroinitializer, %pred.call.if ], [ undef, %pred ]
; CHECK-NEXT: [[INDEX:%.*]] = phi i64 [ %0, %pred.call.if ], [ undef, %pred ]
; CHECK: call <8 x i8> @llvm.experimental.matrix.extract.row.slice.v8i8.v256i8(<256 x i8> [[MATRIX]]
  %2 = phi i64 [ undef, %pred ], [ %1, %pred.call.if ]
  %3 = call <8 x i8> @_ZGVbM8u_sub_group_rowslice_extractelement.i8(i64 %2)
  br label %pred
}

declare i64 @get_sub_group_rowslice_id.v256i8.i64(<256 x i8>, i32, i32, i64)
declare <8 x i8> @_ZGVbM8u_sub_group_rowslice_extractelement.i8(i64)

; The loss of debug info on subgroup builtins are expected.
; DEBUGIFY-COUNT-2: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
