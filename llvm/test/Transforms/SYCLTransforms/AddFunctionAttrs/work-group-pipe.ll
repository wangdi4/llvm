; RUN: opt -passes=sycl-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-function-attrs -S < %s | FileCheck %s

; Function Attrs: convergent
define void @test_pipe_workgroup_write_int(ptr addrspace(1) %src, ptr addrspace(1) %out_pipe) #0 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = tail call ptr @__work_group_reserve_write_pipe(ptr addrspace(1) %out_pipe, i32 0, i32 4, i32 4) #0
  tail call void @__work_group_commit_write_pipe(ptr addrspace(1) %out_pipe, ptr %0, i32 4, i32 4) #0
  ret void
}
; Function Attrs: convergent
declare ptr @__work_group_reserve_write_pipe(ptr addrspace(1), i32, i32, i32) #0
declare void @__work_group_commit_write_pipe(ptr addrspace(1), ptr, i32, i32) #0

; Function Attrs: convergent
define void @test_pipe_workgroup_read_int(ptr addrspace(1) %in_pipe, ptr addrspace(1) %dst) #0 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %0 = tail call ptr @__work_group_reserve_read_pipe(ptr addrspace(1) %in_pipe, i32 0, i32 4, i32 4) #0
  tail call void @__work_group_commit_read_pipe(ptr addrspace(1) %in_pipe, ptr %0, i32 4, i32 4) #0
  ret void
}
declare ptr @__work_group_reserve_read_pipe(ptr addrspace(1), i32, i32, i32) #0
declare void @__work_group_commit_read_pipe(ptr addrspace(1), ptr, i32, i32) #0

attributes #0 = { convergent }

; CHECK-NOT: noduplicate
; CHECK: attributes #0 = { convergent "kernel-call-once" "kernel-convergent-call" }
;; Do not expect other attributes to appear
; CHECK-NOT: #1

!opencl.ocl.version = !{!0}

!0 = !{i32 2, i32 0}
!1 = !{!"int*", !"int"}
!2 = !{ptr addrspace(1) null, target("spirv.Pipe", 1) zeroinitializer}
!3 = !{!"int", !"int*"}
!4 = !{target("spirv.Pipe", 0) zeroinitializer, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
