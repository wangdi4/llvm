; RUN: opt -passes=dpcpp-kernel-resolve-matrix-wi-slice -S %s | FileCheck %s
; RUN: opt -passes='debugify,dpcpp-kernel-resolve-matrix-wi-slice,check-debugify' -disable-output -S %s 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

define void @test(<144 x i32> %mat, i64 %element.index) {
entry:
; CHECK: [[ROWSLICE_ID:%rowslice.id.*]] = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
; CHECK-NEXT: [[EXTRACT_ELEM:%extract.elem.*]] = call i32 @sub_group_rowslice_extractelement.i32(i64 [[ROWSLICE_ID]])
; CHECK-NOT: call i32 @llvm.experimental.matrix.wi.slice.extractelement
  %the.element = call i32 @llvm.experimental.matrix.wi.slice.extractelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ret void
}

; CHECK-DAG: declare i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32>, i32, i32, i64) #[[#ATTR0:]]
; CHECK-DAG: declare i32 @sub_group_rowslice_extractelement.i32(i64) #[[#ATTR1:]]

; Function Attrs: nofree nosync nounwind willreturn
declare i32 @llvm.experimental.matrix.wi.slice.extractelement.v144i32.i64(<144 x i32>, i32, i32, i64, metadata, metadata, metadata) #0

; CHECK-DAG: attributes #[[#ATTR0]] = { "kernel-uniform-call" "opencl-vec-uniform-return" }
; CHECK-DAG: attributes #[[#ATTR1]] = { "kernel-call-once" }

attributes #0 = { nofree nosync nounwind willreturn }

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
