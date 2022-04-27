; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -vplan-vec -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-vec-clone,vplan-vec -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test(<144 x i32> %mat, i64 %element.index) !recommended_vector_length !1 {
entry:
; CHECK-LABEL: vector.body:
; CHECK: [[RID:%.*]] = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> [[MAT:%.*]], i32 12, i32 12, i64 [[INDEX:%.*]])
; CHECK-NEXT: [[WIDEN_EXTRACT:%.*]] = call <16 x i32> @_ZGVbN16u_sub_group_rowslice_extractelement.i32(i64 [[RID]])
; CHECK-NEXT: [[WIDEN_MUL:%.*]] = mul <16 x i32> [[WIDEN_EXTRACT]], <i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42>
; CHECK-NEXT: [[RID1:%.*]] = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> [[MAT]], i32 12, i32 12, i64 [[INDEX]])
; CHECK-NEXT: call void @_ZGVbN16uv_sub_group_rowslice_insertelement.i32(i64 [[RID1]], <16 x i32> [[WIDEN_MUL]])
; CHECK-NEXT: [[MAT_UPDATE:%.*]] = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 [[RID1]])

; CHECK-LABEL: simd.loop.header:
; CHECK: call i32 @sub_group_rowslice_extractelement.i32(i64 %rowslice.id) #[[#ATTR0:]]
; CHECK: call void @sub_group_rowslice_insertelement.i32(i64 %rowslice.id1, i32 %val) #[[#ATTR1:]]
  %rowslice.id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
  %extract.elem = call i32 @sub_group_rowslice_extractelement.i32(i64 %rowslice.id)
  %val = mul i32 %extract.elem, 42
  %rowslice.id1 = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
  call void @sub_group_rowslice_insertelement.i32(i64 %rowslice.id1, i32 %val)
  %mat.update = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 %rowslice.id1)
  ret void
}

declare i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32>, i32, i32, i64) #0

declare i32 @sub_group_rowslice_extractelement.i32(i64)

declare void @sub_group_rowslice_insertelement.i32(i64, i32)

declare <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64) #0

; CHECK-DAG: attributes #[[#ATTR0]] = {{{.*}}"kernel-call-once" "vector-variants"="_ZGVbN16u_sub_group_rowslice_extractelement.i32,_ZGVbM16u_sub_group_rowslice_extractelement.i32" }
; CHECK-DAG: attributes #[[#ATTR1]] = {{{.*}}"kernel-call-once" "vector-variants"="_ZGVbN16uv_sub_group_rowslice_insertelement.i32,_ZGVbM16uv_sub_group_rowslice_insertelement.i32" }

attributes #0 = { "kernel-uniform-call" "opencl-vec-uniform-return" }

!sycl.kernels = !{!0}

!0 = !{void (<144 x i32>, i64)* @test}
!1 = !{i32 16}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-7: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test
; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
