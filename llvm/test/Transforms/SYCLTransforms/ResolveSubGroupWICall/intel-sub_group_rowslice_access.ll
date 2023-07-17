; RUN: opt -passes='sycl-kernel-resolve-sub-group-wi-call' -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s
; RUN: opt -passes='debugify,sycl-kernel-resolve-sub-group-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test(<144 x i32> %mat, i64 %element.index) #0 !recommended_vector_length !1 !vectorized_width !2 !vectorized_kernel !3 {
entry:
; Resolving in scalar context.
; CHECK-LABEL: define void @test
; CHECK: [[TRUNC:%.*]] = trunc i64 %element.index to i32
; CHECK-NEXT: [[BASEID:%rowslice.baseid.*]] = mul nsw i32 [[TRUNC]], 1
; CHECK-NEXT: [[ROW_INDEX:%rowslice.row.index.*]] = udiv i32 [[BASEID]], 12
; CHECK-NEXT: [[COL_INDEX:%rowslice.col.index.*]] = urem i32 [[BASEID]], 12
; CHECK-NEXT: [[ROWSLICE:%.*]] = call <1 x i32> @llvm.experimental.matrix.extract.row.slice.v1i32.v144i32(<144 x i32> %mat, i32 [[ROW_INDEX]], i32 [[COL_INDEX]], i32 1, i32 12, i32 12, metadata !"matrix.rowmajor")
; CHECK-NEXT: [[ELEM:%.*]] = extractelement <1 x i32> [[ROWSLICE]], i32 0
; CHECK-NEXT: [[VAL:%.*]] = mul i32 [[ELEM]], 42
; CHECK-NEXT: [[ROWSLICE_UPDATE:%.*]] = insertelement <1 x i32> undef, i32 [[VAL]], i32 0
; CHECK-NEXT: [[TRUNC1:%.*]] = trunc i64 %element.index to i32
; CHECK-NEXT: [[BASEID1:%rowslice.baseid.*]] = mul nsw i32 [[TRUNC1]], 1
; CHECK-NEXT: [[ROW_INDEX1:%rowslice.row.index.*]] = udiv i32 [[BASEID1]], 12
; CHECK-NEXT: [[COL_INDEX1:%rowslice.col.index.*]] = urem i32 [[BASEID1]], 12
; CHECK-NEXT: [[MAT:%.*]] = call <144 x i32> @llvm.experimental.matrix.insert.row.slice.v144i32.v1i32(<144 x i32> %mat, <1 x i32> [[ROWSLICE_UPDATE]], i32 [[ROW_INDEX1]], i32 [[COL_INDEX1]], i32 1, i32 12, i32 12, metadata !"matrix.rowmajor")
  %rowslice.id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
  %extract.elem = call i32 @sub_group_rowslice_extractelement.i32(i64 %rowslice.id)
  %val = mul i32 %extract.elem, 42
  %rowslice.id1 = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
  call void @sub_group_rowslice_insertelement.i32(i64 %rowslice.id1, i32 %val)
  %mat.update = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 %rowslice.id1)
  ret void
}

declare i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32>, i32, i32, i64) #1

declare i32 @sub_group_rowslice_extractelement.i32(i64) #2

declare void @sub_group_rowslice_insertelement.i32(i64, i32) #3

declare <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64) #1

define void @_ZGVeN16uu_test(<144 x i32> %mat, i64 %element.index) #4 !recommended_vector_length !1 !vectorized_width !1 !scalar_kernel !0 {
entry:
  %alloca.mat = alloca <144 x i32>, align 1024
  store <144 x i32> %mat, ptr %alloca.mat, align 1024
  %alloca.element.index = alloca i64, align 8
  store i64 %element.index, ptr %alloca.element.index, align 8
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.element.index = load i64, ptr %alloca.element.index, align 8
  %load.mat = load <144 x i32>, ptr %alloca.mat, align 1024
  br label %VPlannedBB

VPlannedBB:                                       ; preds = %simd.loop.preheader
  br label %VPlannedBB1

VPlannedBB1:                                      ; preds = %VPlannedBB
  br label %vector.body

vector.body:                                      ; preds = %VPlannedBB3, %VPlannedBB1
; Resolving in vector context.
; CHECK-LABEL: define void @_ZGVeN16uu_test
; CHECK: vector.body:
; CHECK: [[TRUNC2:%.*]] = trunc i64 %load.element.index to i32
; CHECK-NEXT: [[BASEID2:%rowslice.baseid.*]] = mul nsw i32 [[TRUNC2]], 16
; CHECK-NEXT: [[ROW_INDEX2:%rowslice.row.index.*]] = udiv i32 [[BASEID2]], 12
; CHECK-NEXT: [[COL_INDEX2:%rowslice.col.index.*]] = urem i32 [[BASEID2]], 12
; CHECK-NEXT: [[ROWSLICE2:%.*]] = call <16 x i32> @llvm.experimental.matrix.extract.row.slice.v16i32.v144i32(<144 x i32> %load.mat, i32 [[ROW_INDEX2]], i32 [[COL_INDEX2]], i32 16, i32 12, i32 12, metadata !"matrix.rowmajor")
; CHECK-NEXT: [[VAL2:%.*]] = mul <16 x i32> [[ROWSLICE2]], <i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42>
; CHECK-NEXT: [[TRUNC3:%.*]] = trunc i64 %load.element.index to i32
; CHECK-NEXT: [[BASEID3:%rowslice.baseid.*]] = mul nsw i32 [[TRUNC3]], 16
; CHECK-NEXT: [[ROW_INDEX3:%rowslice.row.index.*]] = udiv i32 [[BASEID3]], 12
; CHECK-NEXT: [[COL_INDEX3:%rowslice.col.index.*]] = urem i32 [[BASEID3]], 12
; CHECK-NEXT: [[MAT3:%.*]] = call <144 x i32> @llvm.experimental.matrix.insert.row.slice.v144i32.v16i32(<144 x i32> %load.mat, <16 x i32> [[VAL2]], i32 [[ROW_INDEX3]], i32 [[COL_INDEX3]], i32 16, i32 12, i32 12, metadata !"matrix.rowmajor")
  %uni.phi = phi i32 [ 0, %VPlannedBB1 ], [ %6, %VPlannedBB3 ]
  %vec.phi = phi <16 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, %VPlannedBB1 ], [ %5, %VPlannedBB3 ]
  %0 = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %load.mat, i32 12, i32 12, i64 %load.element.index)
  %1 = call <16 x i32> @_ZGVbN16u_sub_group_rowslice_extractelement.i32(i64 %0)
  %2 = mul <16 x i32> %1, <i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42>
  %3 = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %load.mat, i32 12, i32 12, i64 %load.element.index)
  call void @_ZGVbN16uv_sub_group_rowslice_insertelement.i32(i64 %3, <16 x i32> %2)
  %4 = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 %3)
  br label %VPlannedBB3

VPlannedBB3:                                      ; preds = %vector.body
  %5 = add nuw <16 x i32> %vec.phi, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %6 = add nuw i32 %uni.phi, 16
  %7 = icmp ult i32 %6, 16
  br i1 false, label %vector.body, label %VPlannedBB4, !llvm.loop !4

VPlannedBB4:                                      ; preds = %VPlannedBB3
  br label %VPlannedBB5

VPlannedBB5:                                      ; preds = %VPlannedBB4
  br label %final.merge

final.merge:                                      ; preds = %VPlannedBB5
  %uni.phi6 = phi i32 [ 16, %VPlannedBB5 ]
  br label %simd.end.region

simd.loop:                                        ; preds = %simd.loop.exit
  %index = phi i32 [ %indvar, %simd.loop.exit ]
  %rowslice.id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %load.mat, i32 12, i32 12, i64 %load.element.index)
  %extract.elem = call i32 @sub_group_rowslice_extractelement.i32(i64 %rowslice.id)
  %val = mul i32 %extract.elem, 42
  %rowslice.id1 = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %load.mat, i32 12, i32 12, i64 %load.element.index)
  call void @sub_group_rowslice_insertelement.i32(i64 %rowslice.id1, i32 %val)
  %mat.update = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 %rowslice.id1)
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br label %simd.loop

simd.end.region:                                  ; preds = %final.merge
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #5

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #5

declare <16 x i32> @_ZGVbN16u_sub_group_rowslice_extractelement.i32(i64) #2

declare void @_ZGVbN16uv_sub_group_rowslice_insertelement.i32(i64, <16 x i32>) #3

attributes #0 = { "vector-variants"="_ZGVeN16uu_test" }
attributes #1 = { "kernel-uniform-call" "opencl-vec-uniform-return" }
attributes #2 = { "kernel-call-once" "vector-variants"="_ZGVbM16u_sub_group_rowslice_extractelement.i32,_ZGVbN16u_sub_group_rowslice_extractelement.i32" }
attributes #3 = { "kernel-call-once" "vector-variants"="_ZGVbM16uv_sub_group_rowslice_insertelement.i32,_ZGVbN16uv_sub_group_rowslice_insertelement.i32" }
attributes #4 = { "may-have-openmp-directive"="true" "vector-variants"="_ZGVeN16uu_test" }
attributes #5 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i32 16}
!2 = !{i32 1}
!3 = !{ptr @_ZGVeN16uu_test}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.isvectorized", i32 1}

; The loss of debug info on @get_sub_group_rowslice_id and
; @sub_group_rowslice_insertelement is expected.
; DEBUGIFY-COUNT-9: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
