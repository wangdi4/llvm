; RUN: opt -passes='sycl-kernel-resolve-sub-group-wi-call' -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s
; RUN: opt -passes='debugify,sycl-kernel-resolve-sub-group-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; Checks that "phi(undef, get_sub_group_rowslice_id())" pattern can be correctly resolved for rowslice insertion calls,
; where get_sub_group_rowslice_id, sub_group_rowslice_insertelement and sub_group_insert_rowslice_to_matrix lies in three
; different basic blocks.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @_ZGVeN8uuuu__ZTSZZ19matrix_verify_logicItLm16ELm16EEvN2cl4sycl5queueER10big_matrixIT_XT0_EXT1_EERNS1_8nd_rangeILi2EEEfENKUlRNS1_7handlerEE_clESB_E12logic_matrix() {
entry:
  br i1 false, label %pred.call.continue, label %pred.call.if

pred.call.if:                                     ; preds = %entry
; CHECK-LABEL: pred.call.if:
; CHECK-NEXT: %index = add i64 0, 0
; CHECK-NOT: call{{.*}}get_sub_group_rowslice_id
  %index = add i64 0, 0
  %0 = call i64 @get_sub_group_rowslice_id.v128i16.i64(<128 x i16> zeroinitializer, i32 16, i32 8, i64 %index)
  br label %pred.call.continue

pred.call.continue:                               ; preds = %pred.call.if, %entry
; CHECK-LABEL: pred.call.continue:
; CHECK-NEXT: [[MATRIX:%.*]] = phi <128 x i16> [ zeroinitializer, %pred.call.if ], [ undef, %entry ]
; CHECK-NEXT: [[INDEX:%.*]] = phi i64 [ %index, %pred.call.if ], [ undef, %entry ]
; CHECK-NOT: call void {{.*}}sub_group_rowslice_insertelement
  %1 = phi i64 [ undef, %entry ], [ %0, %pred.call.if ]
  call void @_ZGVbM8uv_sub_group_rowslice_insertelement.i16(i64 %1, <8 x i16> zeroinitializer)
  br label %end

end:                                              ; preds = %pred.call.continue
; CHECK-LABEL: end:
; CHECK-NEXT: [[TRUNC:%.*]] = trunc i64 [[INDEX]] to i32
; CHECK-NEXT: [[BASEID:%.*]] = mul nsw i32 [[TRUNC]], 8
; CHECK-NEXT: [[ROWINDEX:%.*]] = udiv i32 [[BASEID]], 8
; CHECK-NEXT: [[COLINDEX:%.*]] = urem i32 [[BASEID]], 8
; CHECK-NEXT: call <128 x i16> @llvm.experimental.matrix.insert.row.slice.v128i16.v8i16(<128 x i16> [[MATRIX]]
  %2 = call <128 x i16> @sub_group_insert_rowslice_to_matrix.v128i16(i64 %1)
  ret void
}

declare i64 @get_sub_group_rowslice_id.v128i16.i64(<128 x i16>, i32, i32, i64)
declare void @_ZGVbM8uv_sub_group_rowslice_insertelement.i16(i64, <8 x i16>)
declare <128 x i16> @sub_group_insert_rowslice_to_matrix.v128i16(i64)

; The loss of debug info on subgroup builtins are expected.
; DEBUGIFY-COUNT-3: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
