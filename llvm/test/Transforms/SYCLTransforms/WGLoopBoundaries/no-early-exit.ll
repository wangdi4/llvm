; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -debug -disable-output 2>&1 | FileCheck %s

; CHECK: WGLoopBoundaries
; CHECK: found 0 early exit boundaries
; CHECK: found 0 uniform early exit conditions


target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @program(ptr addrspace(1) %out, <4 x i32> %dim, i32 %uni) nounwind !no_barrier_path !1 {
  %id = call i32 @_Z13get_global_idj(i32 0) nounwind
  %outptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %id
  store i32 0, ptr addrspace(1) %outptr
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @program}
!1 = !{i1 true}

; DEBUGIFY-NOT: WARNING
