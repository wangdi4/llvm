; RUN: %oclopt -B-GroupBuiltins %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -B-GroupBuiltins -verify %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the GroupBuiltins pass
;; Four cases:
;;    work_group_reserve_write_pipe
;;    work_group_commit_write_pipe
;;    work_group_reserve_read_pipe
;;    work_group_commit_read_pipe
;; The expected results:
;;      1. A call to @_Z7barrierj(LOCAL_MEM_FENCE) just before calling any of the work-group level pipe built-ins
;;      2. A call to @dummybarrier.() just after calling any of the built-ins
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl.pipe_t = type opaque
%opencl.reserve_id_t = type opaque

declare %opencl.reserve_id_t* @__work_group_reserve_write_pipe(%opencl.pipe_t addrspace(1)*, i32 , i32)
declare void @__work_group_commit_write_pipe(%opencl.pipe_t addrspace(1)*, %opencl.reserve_id_t* nocapture, i32)
declare %opencl.reserve_id_t* @__work_group_reserve_read_pipe(%opencl.pipe_t addrspace(1)*, i32, i32)
declare void @__work_group_commit_read_pipe(%opencl.pipe_t addrspace(1)*, %opencl.reserve_id_t* nocapture, i32)

; CHECK: @wg_reserve_write
define %opencl.reserve_id_t* @wg_reserve_write(%opencl.pipe_t addrspace(1)* %p, i32 %num_packets) nounwind {
  %rid = tail call %opencl.reserve_id_t* @__work_group_reserve_write_pipe(%opencl.pipe_t addrspace(1)* %p, i32 %num_packets, i32 undef)
  ret %opencl.reserve_id_t* %rid
; CHECK:   call void @_Z7barrierj(i32 1)
; CHECK:   %rid = tail call %opencl.reserve_id_t* @__work_group_reserve_write_pipe(%opencl.pipe_t addrspace(1)* %p, i32 %num_packets, i32 undef)
; CHECK:   call void @dummybarrier.()
; CHECK:   ret %opencl.reserve_id_t* %rid
}

; CHECK: @wg_commit_write
define void @wg_commit_write(%opencl.pipe_t addrspace(1)* %p, %opencl.reserve_id_t* %rid) nounwind {
  tail call void @__work_group_commit_write_pipe(%opencl.pipe_t addrspace(1)* %p, %opencl.reserve_id_t* %rid, i32 undef)
  ret void 
; CHECK:   call void @_Z7barrierj(i32 1)
; CHECK:   tail call void @__work_group_commit_write_pipe(%opencl.pipe_t addrspace(1)* %p, %opencl.reserve_id_t* %rid, i32 undef)
; CHECK:   call void @dummybarrier.()
; CHECK:   ret void
}

; CHECK: @wg_reserve_read
define %opencl.reserve_id_t* @wg_reserve_read(%opencl.pipe_t addrspace(1)* %p, i32 %num_packets) nounwind {
  %rid = tail call %opencl.reserve_id_t* @__work_group_reserve_read_pipe(%opencl.pipe_t addrspace(1)* %p, i32 %num_packets, i32 undef)
  ret %opencl.reserve_id_t* %rid
; CHECK:   call void @_Z7barrierj(i32 1)
; CHECK:   %rid = tail call %opencl.reserve_id_t* @__work_group_reserve_read_pipe(%opencl.pipe_t addrspace(1)* %p, i32 %num_packets, i32 undef)
; CHECK:   call void @dummybarrier.()
; CHECK:   ret %opencl.reserve_id_t* %rid
}

; CHECK: @wg_commit_read
define void @wg_commit_read(%opencl.pipe_t addrspace(1)* %p, %opencl.reserve_id_t* %rid) nounwind {
  tail call void @__work_group_commit_read_pipe(%opencl.pipe_t addrspace(1)* %p, %opencl.reserve_id_t* %rid, i32 undef)
  ret void 
; CHECK:   call void @_Z7barrierj(i32 1)
; CHECK:   tail call void @__work_group_commit_read_pipe(%opencl.pipe_t addrspace(1)* %p, %opencl.reserve_id_t* %rid, i32 undef)
; CHECK:   call void @dummybarrier.()
; CHECK:   ret void
}

!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.compiler.options = !{!1}
!0 = !{i32 2, i32 0}
!1 = !{!"-cl-std=CL2.0"}

;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_reserve_write -- call void @dummybarrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_commit_write -- call void @dummybarrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_reserve_read -- call void @dummybarrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_commit_read -- call void @dummybarrier.()

; DEBUGIFY-NOT: WARNING
