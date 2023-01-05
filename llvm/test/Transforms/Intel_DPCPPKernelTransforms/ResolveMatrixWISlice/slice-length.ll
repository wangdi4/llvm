; RUN: opt -passes=dpcpp-kernel-resolve-matrix-wi-slice -S %s | FileCheck %s
; RUN: opt -passes='debugify,dpcpp-kernel-resolve-matrix-wi-slice,check-debugify' -disable-output -S %s 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

define void @test(<144 x i32> %mat) {
entry:
; CHECK: call i64 @get_sub_group_slice_length.(i32 144)
; CHECK-NOT: call i64 @llvm.experimental.matrix.wi.slice.length
  %slice.length = call i64 @llvm.experimental.matrix.wi.slice.length.v144i32(<144 x i32> %mat, i32 12, i32 12, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn
declare i64 @llvm.experimental.matrix.wi.slice.length.v144i32(<144 x i32>, i32, i32, metadata, metadata, metadata) #0

attributes #0 = { nofree nosync nounwind willreturn }

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
