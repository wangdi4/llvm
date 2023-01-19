; RUN: opt -disable-output 2>&1 -passes='print<dpcpp-kernel-barrier-wi-analysis>' %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the WIRelatedValue pass
;; The case: kernel "main" with call to workgroup reserve pipe built-in
;; The expected result:
;;  WI related Values analysis data collected as follow
;;      1. "res_write_pipe_id" is WI unrelated
;;      2. "res_read_pipe_id" is WI unrelated
;;*****************************************************************************

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%opencl.reserve_id_t.5 = type opaque
%opencl.pipe_wo_t.6 = type opaque
%opencl.pipe_ro_t.7 = type opaque

; CHECK: @main
define void @main(%opencl.pipe_wo_t.6 addrspace(1)* %out_pipe, %opencl.pipe_ro_t.7 addrspace(1)* %in_pipe) #0 {
  call void @dummy_barrier.()
  %out_pipe.addr = alloca %opencl.pipe_wo_t.6 addrspace(1)*, align 8
  %in_pipe.addr = alloca %opencl.pipe_ro_t.7 addrspace(1)*, align 8
  store %opencl.pipe_wo_t.6 addrspace(1)* %out_pipe, %opencl.pipe_wo_t.6 addrspace(1)** %out_pipe.addr, align 8
  %a = load %opencl.pipe_wo_t.6 addrspace(1)*, %opencl.pipe_wo_t.6 addrspace(1)** %out_pipe.addr, align 8
  %res_write_pipe_id = call %opencl.reserve_id_t.5* @__work_group_reserve_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)* %a, i32 1024, i32 1, i32 1) #0
  store %opencl.pipe_ro_t.7 addrspace(1)* %in_pipe, %opencl.pipe_ro_t.7 addrspace(1)** %in_pipe.addr, align 8
  %b = load %opencl.pipe_ro_t.7 addrspace(1)*, %opencl.pipe_ro_t.7 addrspace(1)** %in_pipe.addr, align 8
  %res_read_pipe_id = call %opencl.reserve_id_t.5* @__work_group_reserve_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)* %b, i32 1024, i32 1, i32 1) #0
  call void @_Z7barrierj(i32 1)
  ret void
}

; CHECK: WI related Values
; CHECK: res_write_pipe_id is not WI related
; CHECK: res_read_pipe_id is not WI related

declare %opencl.reserve_id_t.5* @__work_group_reserve_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)*, i32, i32, i32) #0
declare %opencl.reserve_id_t.5* @__work_group_reserve_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)*, i32, i32, i32) #0

declare void @_Z7barrierj(i32)
declare void @dummy_barrier.()

attributes #0 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{void (%opencl.pipe_wo_t.6 addrspace(1)*, %opencl.pipe_ro_t.7 addrspace(1)*)* @main}
