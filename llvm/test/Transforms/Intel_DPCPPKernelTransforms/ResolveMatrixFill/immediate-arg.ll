; RUN: opt -dpcpp-kernel-resolve-matrix-fill -S %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-resolve-matrix-fill -S %s | FileCheck %s
; RUN: opt -dpcpp-kernel-resolve-matrix-fill -enable-debugify -S %s 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-resolve-matrix-fill -enable-debugify -S %s 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

define void @test() {
; CHECK: [[MAT_INIT:%.*]] = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32 0, i32 12, i32 12, metadata !"matrix.rowmajor", metadata !"scope.subgroup")
; CHECK-NEXT: [[SLICE_LENGTH:%slice.length.*]] = call i64 @llvm.experimental.matrix.wi.slice.length.v144i32(<144 x i32> [[MAT_INIT]], i32 12, i32 12, metadata !"matrix.rowmajor", metadata !"scope.subgroup")
; CHECK-NEXT: br label %[[LOOP_HEADER:matrix.fill.slice.loop.header.*]]

; CHECK: [[LOOP_HEADER]]:
; CHECK-NEXT: [[ELE_INDEX:%element.index.*]] = phi i64 [ 0, %entry ], [ [[ELE_INDEX_INC:%.*]], %[[LOOP_BODY:matrix.fill.slice.loop.*]] ]
; CHECK-NEXT: [[MAT:%mat.*]] = phi <144 x i32> [ [[MAT_INIT]], %entry ], [ [[MAT_UPDATE:%mat.update.*]], %[[LOOP_BODY]] ]
; CHECK-NEXT: [[TMP_2:%.*]] = icmp slt i64 [[ELE_INDEX]], [[SLICE_LENGTH]]
; CHECK-NEXT: br i1 [[TMP_2]], label %[[LOOP_BODY]], label %[[LOOP_END:matrix.fill.slice.loop.end.*]]

; CHECK: [[LOOP_BODY]]:
; CHECK-NEXT: [[MAT_UPDATE:%mat.update.*]] = call <144 x i32> @llvm.experimental.matrix.wi.slice.insertelement.v144i32.i64(<144 x i32> [[MAT]], i32 12, i32 12, i32 42, i64 [[ELE_INDEX]], metadata !"matrix.rowmajor", metadata !"scope.subgroup")
; CHECK-NEXT: [[ELE_INDEX_INC]] = add nuw i64 [[ELE_INDEX]], 1
; CHECK-NEXT: br label %[[LOOP_HEADER]]

; CHECK: [[LOOP_END]]:
; CHECK-NOT: llvm.experimental.matrix.fill
; CHECK: call void @foo(<144 x i32> [[MAT]])
; CHECK: ret void
entry:
  %call.i = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32 42, i32 12, i32 12, metadata !"matrix.rowmajor", metadata !"scope.subgroup")
  call void @foo(<144 x i32> %call.i)
  ret void
}

define void @test_packedb() {
; CHECK-LABEL: @test_packedb(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[MAT_INIT:%.*]] = call <144 x i8> @llvm.experimental.matrix.fill.v144i8.i8(i8 0, i32 12, i32 12, metadata !"matrix.packed.b", metadata !"scope.subgroup")
; CHECK-NEXT:    [[SLICE_LENGTH:%.*]] = call i64 @llvm.experimental.matrix.wi.slice.length.v144i8(<144 x i8> [[MAT_INIT]], i32 12, i32 12, metadata !"matrix.packed.b", metadata !"scope.subgroup")
; CHECK-NEXT:    br label [[MATRIX_FILL_SLICE_LOOP_HEADER:%.*]]
; CHECK:       matrix.fill.slice.loop.header:
; CHECK-NEXT:    [[ELEMENT_INDEX:%.*]] = phi i64 [ 0, [[ENTRY:%.*]] ], [ [[ELEMENT_INDEX_INC:%.*]], [[MATRIX_FILL_SLICE_LOOP:%.*]] ]
; CHECK-NEXT:    [[MAT:%.*]] = phi <144 x i8> [ [[MAT_INIT]], [[ENTRY]] ], [ [[MAT_UPDATE:%.*]], [[MATRIX_FILL_SLICE_LOOP]] ]
; CHECK-NEXT:    [[TMP0:%.*]] = icmp slt i64 [[ELEMENT_INDEX]], [[SLICE_LENGTH]]
; CHECK-NEXT:    br i1 [[TMP0]], label [[MATRIX_FILL_SLICE_LOOP]], label [[MATRIX_FILL_SLICE_LOOP_END:%.*]]
; CHECK:       matrix.fill.slice.loop:
; CHECK-NEXT:    [[MAT_UPDATE]] = call <144 x i8> @llvm.experimental.matrix.wi.slice.insertelement.v144i8.i64(<144 x i8> [[MAT]], i32 12, i32 12, i8 42, i64 [[ELEMENT_INDEX]], metadata !"matrix.packed.b", metadata !"scope.subgroup")
; CHECK-NEXT:    [[ELEMENT_INDEX_INC]] = add nuw i64 [[ELEMENT_INDEX]], 1
; CHECK-NEXT:    br label [[MATRIX_FILL_SLICE_LOOP_HEADER]]
; CHECK:       matrix.fill.slice.loop.end:
; CHECK-NEXT:    call void @fooi8(<144 x i8> [[MAT]])
; CHECK-NEXT:    ret void
;
entry:
  %call.i = call <144 x i8> @llvm.experimental.matrix.fill.v144i8.i8(i8 42, i32 12, i32 12, metadata !"matrix.packed.b", metadata !"scope.subgroup")
  call void @fooi8(<144 x i8> %call.i)
  ret void
}

declare <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32, i32, i32, metadata, metadata)
declare <144 x i8> @llvm.experimental.matrix.fill.v144i8.i8(i8, i32, i32, metadata, metadata)
declare void @foo(<144 x i32>)
declare void @fooi8(<144 x i8>)

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
