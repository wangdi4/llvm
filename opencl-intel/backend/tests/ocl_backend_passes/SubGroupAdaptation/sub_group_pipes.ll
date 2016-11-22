; RUN: opt -sub-group-adaptation -verify -S < %s | FileCheck %s
;;*****************************************************************************
;; This test checks the SubGroupAdaptation pass
;; Four cases:
;;    sub_group_reserve_write_pipe
;;    sub_group_commit_write_pipe
;;    sub_group_reserve_read_pipe
;;    sub_group_commit_read_pipe
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

%opencl.pipe_t = type opaque
%opencl.reserve_id_t = type opaque

; CHECK: @sg_test_pipes
; CHECK: entry
; CHECK: call %opencl.reserve_id_t* @__work_group_reserve_read_pipe(%opencl.pipe_t addrspace(1)* %0, i32 %1, i32 16)
; CHECK: call %opencl.reserve_id_t* @__work_group_reserve_write_pipe(%opencl.pipe_t addrspace(1)* %3, i32 %4, i32 16)
; CHECK: call void @__work_group_commit_read_pipe(%opencl.pipe_t addrspace(1)* %6, %opencl.reserve_id_t* %7, i32 16)
; CHECK: call void @__work_group_commit_write_pipe(%opencl.pipe_t addrspace(1)* %8, %opencl.reserve_id_t* %9, i32 16)

; Function Attrs: nounwind
define spir_kernel void @sg_test_pipes(%opencl.pipe_t addrspace(1)* %num_packets, i32 %pipes) #0 {
entry:
  %num_packets.addr = alloca %opencl.pipe_t addrspace(1)*, align 16
  %pipes.addr = alloca i32, align 4
  %res_rrp = alloca %opencl.reserve_id_t*, align 8
  %res_rwp = alloca %opencl.reserve_id_t*, align 8
  store %opencl.pipe_t addrspace(1)* %num_packets, %opencl.pipe_t addrspace(1)** %num_packets.addr, align 16
  store i32 %pipes, i32* %pipes.addr, align 4
  %0 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %num_packets.addr, align 16
  %1 = load i32, i32* %pipes.addr, align 4
  %2 = call %opencl.reserve_id_t* @__sub_group_reserve_read_pipe(%opencl.pipe_t addrspace(1)* %0, i32 %1, i32 16)
  store %opencl.reserve_id_t* %2, %opencl.reserve_id_t** %res_rrp, align 8
  %3 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %num_packets.addr, align 16
  %4 = load i32, i32* %pipes.addr, align 4
  %5 = call %opencl.reserve_id_t* @__sub_group_reserve_write_pipe(%opencl.pipe_t addrspace(1)* %3, i32 %4, i32 16)
  store %opencl.reserve_id_t* %5, %opencl.reserve_id_t** %res_rwp, align 8
  %6 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %num_packets.addr, align 16
  %7 = load %opencl.reserve_id_t*, %opencl.reserve_id_t** %res_rrp, align 8
  call void @__sub_group_commit_read_pipe(%opencl.pipe_t addrspace(1)* %6, %opencl.reserve_id_t* %7, i32 16)
  %8 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %num_packets.addr, align 16
  %9 = load %opencl.reserve_id_t*, %opencl.reserve_id_t** %res_rwp, align 8
  call void @__sub_group_commit_write_pipe(%opencl.pipe_t addrspace(1)* %8, %opencl.reserve_id_t* %9, i32 16)
  ret void
}

declare %opencl.reserve_id_t* @__sub_group_reserve_read_pipe(%opencl.pipe_t addrspace(1)*, i32, i32)

declare %opencl.reserve_id_t* @__sub_group_reserve_write_pipe(%opencl.pipe_t addrspace(1)*, i32, i32)

declare void @__sub_group_commit_read_pipe(%opencl.pipe_t addrspace(1)*, %opencl.reserve_id_t*, i32)

declare void @__sub_group_commit_write_pipe(%opencl.pipe_t addrspace(1)*, %opencl.reserve_id_t*, i32)

; CHECK: declare %opencl.reserve_id_t* @__work_group_reserve_read_pipe(%opencl.pipe_t addrspace(1)*, i32, i32)
; CHECK: declare %opencl.reserve_id_t* @__work_group_reserve_write_pipe(%opencl.pipe_t addrspace(1)*, i32, i32)
; CHECK: declare void @__work_group_commit_read_pipe(%opencl.pipe_t addrspace(1)*, %opencl.reserve_id_t*, i32)
; CHECK: declare void @__work_group_commit_write_pipe(%opencl.pipe_t addrspace(1)*, %opencl.reserve_id_t*, i32)

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (%opencl.pipe_t addrspace(1)*, i32)* @sg_test_pipes, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 0}
!2 = !{!"kernel_arg_access_qual", !"read_only", !"none"}
!3 = !{!"kernel_arg_type", !"int4", !"uint"}
!4 = !{!"kernel_arg_base_type", !"int4", !"uint"}
!5 = !{!"kernel_arg_type_qual", !"pipe", !""}
!6 = !{i32 1, i32 2}
!7 = !{i32 2, i32 0}
!8 = !{}

;;; --- OpenCL source (compilation options: "-cl-std=CL2.0 -D__OPENCL_C_VERSION__=200"
;;;__kernel void sg_test_pipes(pipe int4 num_packets, uint pipes)
;;;{
;;;    reserve_id_t res_rrp = sub_group_reserve_read_pipe(num_packets, pipes);
;;;	reserve_id_t res_rwp = sub_group_reserve_write_pipe(num_packets, pipes);
;;;	sub_group_commit_read_pipe(num_packets, res_rrp);
;;;	sub_group_commit_write_pipe(num_packets, res_rwp);
;;;}
