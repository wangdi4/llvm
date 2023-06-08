; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-barrier-wi-analysis>' %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the WIRelatedValue pass
;; The case: kernel "main" with call to atomic built-in %a
;; The expected result:
;;      //TODO: 0. Kernel "main" was not changed
;;  WI related Values analysis data collected as follow
;;      1. "%a" is non-uniform value (i.e. WI related)
;;*****************************************************************************

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

declare void @_Z7barrierj(i32)
; write_pipe
declare i32 @__write_pipe_2(ptr addrspace(1), ptr addrspace(1), i32)
; read_pipe
declare i32 @__read_pipe_2(ptr addrspace(1), ptr addrspace(1), i32)
; reserve_write_pipe
declare ptr @__reserve_write_pipe(ptr addrspace(1), i32, i32)
; write_pipe w\ reserve_id
declare i32 @__write_pipe_4(ptr addrspace(1), ptr, i32, ptr, i32)
; reserve_write_pipe
declare void @__commit_write_pipe(ptr addrspace(1), ptr, i32)
; reserve_read_pipe
declare ptr @__reserve_read_pipe(ptr addrspace(1), i32, i32)
; read_pipe w\ reserve_id
declare i32 @__read_pipe_4(ptr addrspace(1), ptr, i32, ptr, i32)
; commit_read_pipe
declare void @__commit_read_pipe(ptr, ptr, i32)

define void @pipe_builtins(ptr addrspace(1) %pipe, ptr addrspace(1) %data) !kernel_arg_base_type !2 !arg_type_null_val !3 !kernel_arg_access_qual !4 {
  %basic_write_res = tail call i32 @__write_pipe_2(ptr addrspace(1) %pipe, ptr addrspace(1) %data, i32 4)
  %basic_read_res = tail call i32 @__read_pipe_2(ptr addrspace(1) %pipe, ptr addrspace(1) %data, i32 4)

  %reserved_write_rid = tail call ptr @__reserve_write_pipe(ptr addrspace(1) %pipe, i32 1, i32 4)
  %reserved_write_res = tail call i32 @__write_pipe_4(ptr addrspace(1) %pipe, ptr %reserved_write_rid, i32 0, ptr addrspace(1) %data, i32 4)
  ; the following line is just for the sake of completeness
  tail call void @__commit_write_pipe(ptr addrspace(1) %pipe, ptr %reserved_write_rid, i32 4)

  %reserved_read_rid = tail call ptr @__reserve_read_pipe(ptr addrspace(1) %pipe, i32 1, i32 4)
  %reserved_read_res = tail call i32 @__read_pipe_4(ptr addrspace(1) %pipe, ptr %reserved_write_rid, i32 0, ptr addrspace(1) %data, i32 4)
  ; the following line is just for the sake of completeness
  tail call void @__commit_read_pipe(ptr addrspace(1) %pipe, ptr %reserved_read_rid, i32 4)

  call void @_Z7barrierj(i32 2)
  ret void
}

; CHECK: basic_write_res is WI related
; CHECK: basic_read_res is WI related
; CHECK: reserved_write_rid is WI related
; CHECK: reserved_write_res is WI related
; CHECK: reserved_read_rid is WI related
; CHECK: reserved_read_res is WI related

!opencl.ocl.version = !{!0}
!opencl.compiler.options = !{!1}
!0 = !{i32 2, i32 0}
!1 = !{!"-cl-std=CL2.0"}
!2 = !{!"int*", !"int*"}
!3 = !{target("spirv.Pipe", 1) zeroinitializer, target("spirv.Pipe", 0) zeroinitializer}
!4 = !{!"write_only", !"read_only"}
