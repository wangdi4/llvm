; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -debug -disable-output 2>&1 | FileCheck %s

; CHECK: WGLoopBoundaries
;; boundary output pattern: dim, contains_val(T/F), is_global_id(T/F), IsSigned(T/F), IsUpperBound(T,F)
; CHECK: found 0 early exit boundaries
; CHECK: found 0 uniform early exit conditions


target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @program(ptr addrspace(1) %out, i32 %t1) nounwind {
entry:
  %id = call i32 @_Z13get_global_idj(i32 0) nounwind
  %b1 = icmp eq i32 %id, %t1
  br i1 %b1, label %ret, label %body

body:
  %outptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %id
  store i32 0, ptr addrspace(1) %outptr
  br label %ret

ret:
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @program}

; DEBUGIFY-NOT: WARNING
