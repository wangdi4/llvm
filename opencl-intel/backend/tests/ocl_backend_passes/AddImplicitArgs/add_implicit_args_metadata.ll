; RUN: opt -add-implicit-args -verify -S < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; adding implicit arguments to the metadata of a function

declare void @functionWithoutBody(i32 %x, i32 %y)

define void @functionWithBody(i32 %x) {
entry:
  %res = add i32 100, 10
  ret void
}

!opencl.kernels = !{!0, !1}

!0 = !{void (i32)* @functionWithBody}
!1 = !{void (i32, i32)* @functionWithoutBody}


; CHECK:        declare void @functionWithoutBody(i32, i32)
; CHECK:        define void @functionWithBody(i32 %x,
; CHECK:          i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* noalias %pWorkDim
; CHECK:          i32* noalias %pWGId,
; CHECK:          [4 x i32] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          {}* noalias %RuntimeHandle) {
; CHECK-NEXT:   entry:
; CHECK-NEXT:   %res = add i32 100, 10
; CHECK-NEXT:   ret void

; CHECK:        !opencl.kernels = !{!0, !1}
; CHECK:        !0 = !{void (i32, i8 addrspace(3)*, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }*, i32*, [4 x i32], i8*, {}*)* @functionWithBody}
; CHECK-NEXT:   !1 = !{void (i32, i32)* @functionWithoutBody}
