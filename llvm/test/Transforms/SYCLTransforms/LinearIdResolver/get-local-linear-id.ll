; RUN: opt -passes=sycl-kernel-linear-id-resolver %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-linear-id-resolver %s -S | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK:     @test_local
; CHECK-NOT: %call = call i64 @_Z19get_local_linear_idv()
; CHECK:     [[lid2:%[a-zA-Z0-9_\.]+]] = call i64 @_Z12get_local_idj(i32 2)
; CHECK:     [[lid1:%[a-zA-Z0-9_\.]+]] = call i64 @_Z12get_local_idj(i32 1)
; CHECK:     [[lid0:%[a-zA-Z0-9_\.]+]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK:     [[lsz1:%[a-zA-Z0-9_\.]+]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK:     [[lsz0:%[a-zA-Z0-9_\.]+]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK:     [[op0:%[a-zA-Z0-9_\.]+]] = mul i64 [[lid2]], [[lsz1]]
; CHECK:     [[op1:%[a-zA-Z0-9_\.]+]] = add i64 [[op0]], [[lid1]]
; CHECK:     [[op2:%[a-zA-Z0-9_\.]+]] = mul i64 [[op1]], [[lsz0]]
; CHECK:     [[res:%[a-zA-Z0-9_\.]+]] = add i64 [[op2]], [[lid0]]

define void @test_local() {
entry:
  %call = call i64 @_Z19get_local_linear_idv()
  ret void
}

declare i64 @_Z19get_local_linear_idv()

!opencl.compiler.options = !{!0}
!0 = !{!"-cl-std=CL2.0"}

; DEBUGIFY-NOT: WARNING
