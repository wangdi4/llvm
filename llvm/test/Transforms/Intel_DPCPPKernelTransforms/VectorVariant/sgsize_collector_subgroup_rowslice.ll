; RUN: opt %s -dpcpp-kernel-sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -dpcpp-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector -S | FileCheck %s

define void @test(<144 x i32> %mat, i64 %element.index) !recommended_vector_length !{i32 16} {
entry:
  %rowslice.id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index) #1
  %extract.elem = call i32 @sub_group_rowslice_extractelement.i32(i64 %rowslice.id) #2
  %val = mul i32 %extract.elem, 42
  %rowslice.id1 = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index) #1
  call void @sub_group_rowslice_insertelement.i32(i64 %rowslice.id1, i32 %val) #2
  %mat.update = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 %rowslice.id1) #1
  ret void
}

; CHECK-DAG: declare i32 @sub_group_rowslice_extractelement.i32(i64) #[[#ATTR0:]]
; CHECK-DAG: declare void @sub_group_rowslice_insertelement.i32(i64, i32) #[[#ATTR1:]]

declare i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32>, i32, i32, i64)

declare i32 @sub_group_rowslice_extractelement.i32(i64)

declare void @sub_group_rowslice_insertelement.i32(i64, i32)

declare <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64)

; CHECK-DAG: attributes #[[#ATTR0]] = { "vector-variants"="_ZGVbM16u_sub_group_rowslice_extractelement.i32,_ZGVbN16u_sub_group_rowslice_extractelement.i32" }
; CHECK-DAG: attributes #[[#ATTR1]] = { "vector-variants"="_ZGVbM16uv_sub_group_rowslice_insertelement.i32,_ZGVbN16uv_sub_group_rowslice_insertelement.i32" }

attributes #0 = { nofree nosync nounwind willreturn }
attributes #1 = { "kernel-uniform-call" "opencl-vec-uniform-return" }
attributes #2 = { "kernel-call-once" }

!sycl.kernels = !{!0}

!0 = !{void (<144 x i32>, i64)* @test}

; DEBUGIFY-NOT: WARNING
