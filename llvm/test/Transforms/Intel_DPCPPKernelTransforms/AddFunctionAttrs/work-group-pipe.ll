; RUN: opt -passes=dpcpp-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-function-attrs -S < %s | FileCheck %s

%opencl.reserve_id_t.5 = type opaque
%opencl.pipe_wo_t.6 = type opaque
%opencl.pipe_ro_t.7 = type opaque

; Function Attrs: convergent
define void @test_pipe_workgroup_write_int(i32 addrspace(1)* %src, %opencl.pipe_wo_t.6 addrspace(1)* %out_pipe) #0 {
entry:
  %0 = tail call %opencl.reserve_id_t.5* @__work_group_reserve_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)* %out_pipe, i32 0, i32 4, i32 4) #0
  tail call void @__work_group_commit_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)* %out_pipe, %opencl.reserve_id_t.5* %0, i32 4, i32 4) #0
  ret void
}
; Function Attrs: convergent
declare %opencl.reserve_id_t.5* @__work_group_reserve_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)*, i32, i32, i32) #0
declare void @__work_group_commit_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)*, %opencl.reserve_id_t.5*, i32, i32) #0

; Function Attrs: convergent
define void @test_pipe_workgroup_read_int(%opencl.pipe_ro_t.7 addrspace(1)* %in_pipe, i32 addrspace(1)* %dst) #0 {
entry:
  %0 = tail call %opencl.reserve_id_t.5* @__work_group_reserve_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)* %in_pipe, i32 0, i32 4, i32 4) #0
  tail call void @__work_group_commit_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)* %in_pipe, %opencl.reserve_id_t.5* %0, i32 4, i32 4) #0
  ret void
}
declare %opencl.reserve_id_t.5* @__work_group_reserve_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)*, i32, i32, i32) #0
declare void @__work_group_commit_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)*, %opencl.reserve_id_t.5*, i32, i32) #0

attributes #0 = { convergent }

; CHECK-NOT: noduplicate
; CHECK: attributes #0 = { convergent "kernel-call-once" "kernel-convergent-call" }
;; Do not expect other attributes to appear
; CHECK-NOT: #1

!opencl.ocl.version = !{!0}

!0 = !{i32 2, i32 0}

; DEBUGIFY-NOT: WARNING
