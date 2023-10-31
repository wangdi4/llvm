; Checks the AMX matrix intrinsic call sites have attributes:
; - convergent
; - "kernel-call-once"
; - "kernel-uniform-call"
; - "opencl-vec-uniform-return"

; RUN: opt -passes=sycl-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-function-attrs -S < %s | FileCheck %s

define void @load_mad_store(ptr addrspace(4) %ptr, i64 %Stride, <616 x i8> %x, <560 x i8> %y) {
  %1 = call <110 x i32> @llvm.experimental.matrix.load.v110i32.p4(ptr addrspace(4) %ptr, i64 %Stride, i1 false, i32 11, i32 10, metadata !"matrix.rowmajor", metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  %2 = call <110 x i32> @llvm.experimental.matrix.mad.v110i32.v616i8.v560i8(<616 x i8> %x, <560 x i8> %y, <110 x i32> %1, i32 11, i32 56, i32 10, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  call void @llvm.experimental.matrix.store.v110i32.p4(<110 x i32> %2, ptr addrspace(4) %ptr, i64 %Stride, i1 false, i32 11, i32 10, metadata !"matrix.rowmajor", metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ret void
}

declare <110 x i32> @llvm.experimental.matrix.load.v110i32.p4(ptr addrspace(4), i64, i1, i32, i32, metadata, metadata, metadata, metadata)
declare <110 x i32> @llvm.experimental.matrix.mad.v110i32.v616i8.v560i8(<616 x i8>, <560 x i8>, <110 x i32>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)
declare void @llvm.experimental.matrix.store.v110i32.p4(<110 x i32>, ptr addrspace(4), i64, i1, i32, i32, metadata, metadata, metadata, metadata)

; CHECK: call {{.*}} @llvm.experimental.matrix.load.{{.*}} #[[#ATTR_ID:]]
; CHECK: call {{.*}} @llvm.experimental.matrix.mad.{{.*}} #[[#ATTR_ID:]]
; CHECK: call {{.*}} @llvm.experimental.matrix.store.{{.*}} #[[#ATTR_ID:]]

; CHECK: attributes #[[#ATTR_ID]] = { convergent "kernel-call-once" "kernel-uniform-call" "opencl-vec-uniform-return" }

!0 = !{!"int*", !"long", !"char616", !"char560"}
!1 = !{ptr addrspace(4) null, i64 0, <616 x i8> zeroinitializer, <560 x i8> zeroinitializer}

; DEBUGIFY-NOT: WARNING
