; RUN: opt -passes=sycl-kernel-resolve-matrix-fill -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-resolve-matrix-fill -enable-debugify -S %s 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

define void @test() {
; CHECK: [[LOAD_DATA:%loaded.fill.data.*]] = load i32, ptr addrspace(4) %load
; CHECK: [[MAT_INIT:%.*]] = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32 0, i32 12, i32 12, metadata !"scope.subgroup", metadata !"matrix.use.a")
; CHECK-NEXT: [[SLICE_LENGTH:%slice.length.*]] = call i64 @llvm.experimental.matrix.wi.slice.length.v144i32(<144 x i32> [[MAT_INIT]], i32 12, i32 12, metadata !"scope.subgroup", metadata !"matrix.use.a")
; CHECK-NEXT: br label %[[LOOP_HEADER:matrix.fill.slice.loop.header.*]]

; CHECK: [[LOOP_HEADER]]:
; CHECK-NEXT: [[ELE_INDEX:%element.index.*]] = phi i64 [ 0, %entry ], [ [[ELE_INDEX_INC:%.*]], %[[LOOP_BODY:matrix.fill.slice.loop.*]] ]
; CHECK-NEXT: [[MAT:%mat.*]] = phi <144 x i32> [ [[MAT_INIT]], %entry ], [ [[MAT_UPDATE:%mat.update.*]], %[[LOOP_BODY]] ]
; CHECK-NEXT: [[TMP_2:%.*]] = icmp slt i64 [[ELE_INDEX]], [[SLICE_LENGTH]]
; CHECK-NEXT: br i1 [[TMP_2]], label %[[LOOP_BODY]], label %[[LOOP_END:matrix.fill.slice.loop.end.*]]

; CHECK: [[LOOP_BODY]]:
; CHECK-NEXT: [[MAT_UPDATE:%mat.update.*]] = call <144 x i32> @llvm.experimental.matrix.wi.slice.insertelement.v144i32.i64(<144 x i32> [[MAT]], i32 12, i32 12, i32 [[LOAD_DATA]], i64 [[ELE_INDEX]], metadata !"scope.subgroup", metadata !"matrix.use.a")
; CHECK-NEXT: [[ELE_INDEX_INC]] = add nuw i64 [[ELE_INDEX]], 1
; CHECK-NEXT: br label %[[LOOP_HEADER]]

; CHECK: [[LOOP_END]]:
; CHECK-NOT: llvm.experimental.matrix.fill
; CHECK: call void @foo(<144 x i32> [[MAT]])
; CHECK: ret void
entry:
  %v.addr.i = alloca ptr addrspace(4), align 8
  %ref.tmp = alloca i32, align 4
  %ref.tmp.ascast = addrspacecast ptr %ref.tmp to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %ref.tmp.ascast, align 4
  %v.addr.ascast.i = addrspacecast ptr %v.addr.i to ptr addrspace(4)
  store ptr addrspace(4) %ref.tmp.ascast, ptr addrspace(4) %v.addr.ascast.i, align 8
  %load = load ptr addrspace(4), ptr addrspace(4) %v.addr.ascast.i, align 8
  %call.i = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.p4(ptr addrspace(4) %load, i32 12, i32 12, metadata !"scope.subgroup", metadata !"matrix.use.a")
  call void @foo(<144 x i32> %call.i)
  ret void
}

declare <144 x i32> @llvm.experimental.matrix.fill.v144i32.p4(ptr addrspace(4), i32, i32, metadata, metadata)
declare void @foo(<144 x i32>)

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
