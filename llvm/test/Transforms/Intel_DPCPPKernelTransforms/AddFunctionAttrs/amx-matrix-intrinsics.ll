; Checks the AMX matrix intrinsics have attributes:
; - convergent
; - "kernel-call-once"
; - "kernel-uniform-call"

; RUN: opt -passes=dpcpp-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-function-attrs -S < %s | FileCheck %s

define void @load_mad_store(i32 addrspace(4)* %ptr, i64 %Stride, <616 x i8> %x, <560 x i8> %y) {
  %1 = call <110 x i32> @llvm.experimental.matrix.load.v110i32.p4i32(i32 addrspace(4)* %ptr, i64 %Stride, i1 false, i32 11, i32 10, metadata !"matrix.rowmajor", metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  %2 = call <110 x i32> @llvm.experimental.matrix.mad.v110i32.v616i8.v560i8(<616 x i8> %x, metadata !"matrix.rowmajor", <560 x i8> %y, metadata !"matrix.packed.b", <110 x i32> %1, metadata !"matrix.rowmajor", i32 11, i32 56, i32 10, metadata !"scope.subgroup")
  call void @llvm.experimental.matrix.store.v110i32.p4i32(<110 x i32> %2, i32 addrspace(4)* %ptr, i64 %Stride, i1 false, i32 11, i32 10, metadata !"matrix.rowmajor", metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ret void
}

declare <110 x i32> @llvm.experimental.matrix.load.v110i32.p4i32(i32 addrspace(4)*, i64, i1, i32, i32, metadata, metadata, metadata, metadata) #5
declare <110 x i32> @llvm.experimental.matrix.mad.v110i32.v616i8.v560i8(<616 x i8>, metadata, <560 x i8>, metadata, <110 x i32>, metadata, i32, i32, i32, metadata) #5
declare void @llvm.experimental.matrix.store.v110i32.p4i32(<110 x i32>, i32 addrspace(4)*, i64, i1, i32, i32, metadata, metadata, metadata, metadata) #5

; CHECK-DAG: declare {{.*}} @llvm.experimental.matrix.load.{{.*}} #[[#ATTR_ID:]]
; CHECK-DAG: declare {{.*}} @llvm.experimental.matrix.mad.{{.*}} #[[#ATTR_ID:]]
; CHECK-DAG: declare {{.*}} @llvm.experimental.matrix.store.{{.*}} #[[#ATTR_ID:]]

; CHECK: attributes #[[#ATTR_ID]] = {
; CHECK-DAG: "opencl-vec-uniform-return"
; CHECK-DAG: "kernel-call-once"
; CHECK-DAG: "kernel-uniform-call"
; CHECK-DAG: convergent
; CHECK-SAME: }

; DEBUGIFY-NOT: WARNING
