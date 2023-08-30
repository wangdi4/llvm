; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

; CHECK-NOT: opencl-vec-uniform-return
; CHECK: declare ptr @__work_group_reserve_write_pipe(ptr addrspace(1), i32, i32, i32) [[ATTR:#[0-9]*]]
; CHECK: declare ptr @__work_group_reserve_read_pipe(ptr addrspace(1), i32, i32, i32) [[ATTR]]
; CHECK-NOT: opencl-vec-uniform-return
; CHECK: attributes [[ATTR]] = { {{.*}}opencl-vec-uniform-return{{.*}} }
; CHECK-NOT: opencl-vec-uniform-return

; Function Attrs: convergent
define void @test_pipe_workgroup_write_int(ptr addrspace(1) %src, ptr addrspace(1) %out_pipe) #0 !recommended_vector_length !1 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %0 = tail call ptr @__work_group_reserve_write_pipe(ptr addrspace(1) %out_pipe, i32 0, i32 4, i32 4) #0
  tail call void @__work_group_commit_write_pipe(ptr addrspace(1) %out_pipe, ptr %0, i32 4, i32 4) #0
  ret void
}
; Function Attrs: convergent
declare ptr @__work_group_reserve_write_pipe(ptr addrspace(1), i32, i32, i32) #0
declare void @__work_group_commit_write_pipe(ptr addrspace(1), ptr, i32, i32) #0

; Function Attrs: convergent
define void @test_pipe_workgroup_read_int(ptr addrspace(1) %in_pipe, ptr addrspace(1) %dst) #0 !recommended_vector_length !1 !kernel_arg_base_type !5 !arg_type_null_val !6 {
entry:
  %0 = tail call ptr @__work_group_reserve_read_pipe(ptr addrspace(1) %in_pipe, i32 0, i32 4, i32 4) #0
  tail call void @__work_group_commit_read_pipe(ptr addrspace(1) %in_pipe, ptr %0, i32 4, i32 4) #0
  ret void
}
declare ptr @__work_group_reserve_read_pipe(ptr addrspace(1), i32, i32, i32) #0
declare void @__work_group_commit_read_pipe(ptr addrspace(1), ptr, i32, i32) #0

attributes #0 = { convergent }

!opencl.ocl.version = !{!0}
!sycl.kernels = !{!2}

!0 = !{i32 2, i32 0}
!1 = !{i32 4}
!2 = !{ptr @test_pipe_workgroup_write_int, ptr @test_pipe_workgroup_read_int}
!3 = !{!"int*", !"int"}
!4 = !{ptr addrspace(1) null, target("spirv.Pipe", 1) zeroinitializer}
!5 = !{!"int", !"int*"}
!6 = !{target("spirv.Pipe", 0) zeroinitializer, ptr addrspace(1) null}


; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_write_int {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_write_int {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_write_int {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_write_int {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_write_int {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_write_int {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_write_int {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_read_int {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_read_int {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_read_int {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_read_int {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_read_int {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_read_int {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_test_pipe_workgroup_read_int {{.*}} br
; DEBUGIFY-NOT: WARNING
