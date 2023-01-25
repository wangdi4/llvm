; RUN: opt -passes=sycl-kernel-linear-id-resolver %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-linear-id-resolver %s -S | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK:     @test_global
; CHECK-NOT: %call = call i64 @_Z20get_global_linear_idv()
; CHECK:     [[gid2:%[a-zA-Z0-9_\.]+]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK:     [[gid1:%[a-zA-Z0-9_\.]+]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK:     [[gid0:%[a-zA-Z0-9_\.]+]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK:     [[goff2:%[a-zA-Z0-9_\.]+]] = call i64 @_Z17get_global_offsetj(i32 2)
; CHECK:     [[goff1:%[a-zA-Z0-9_\.]+]] = call i64 @_Z17get_global_offsetj(i32 1)
; CHECK:     [[goff0:%[a-zA-Z0-9_\.]+]] = call i64 @_Z17get_global_offsetj(i32 0)
; CHECK:     [[gsz1:%[a-zA-Z0-9_\.]+]] = call i64 @_Z15get_global_sizej(i32 1)
; CHECK:     [[gsz0:%[a-zA-Z0-9_\.]+]] = call i64 @_Z15get_global_sizej(i32 0)
; CHECK:     [[op0:%[a-zA-Z0-9_\.]+]] = sub i64 [[gid2]], [[goff2]]
; CHECK:     [[op1:%[a-zA-Z0-9_\.]+]] = mul i64 [[op0]], [[gsz1]]
; CHECK:     [[op2:%[a-zA-Z0-9_\.]+]] = sub i64 [[gid1]], [[goff1]]
; CHECK:     [[op3:%[a-zA-Z0-9_\.]+]] = add i64 [[op1]], [[op2]]
; CHECK:     [[op4:%[a-zA-Z0-9_\.]+]] = mul i64 [[op3]], [[gsz0]]
; CHECK:     [[op5:%[a-zA-Z0-9_\.]+]] = sub i64 [[gid0]], [[goff0]]
; CHECK:     [[res:%[a-zA-Z0-9_\.]+]] = add i64 [[op4]], [[op5]]

define void @test_global() {
entry:
  %call = call i64 @_Z20get_global_linear_idv()
  ret void
}

declare i64 @_Z20get_global_linear_idv()

!opencl.compiler.options = !{!0}
!0 = !{!"-cl-std=CL2.0"}

; DEBUGIFY-NOT: WARNING
