; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

%opencl.reserve_id_t.5 = type opaque
%opencl.pipe_wo_t.6 = type opaque
%opencl.pipe_ro_t.7 = type opaque

; CHECK-NOT: opencl-vec-uniform-return
; CHECK: declare %opencl.reserve_id_t.5* @__work_group_reserve_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)*, i32, i32, i32) [[ATTR:#[0-9]*]]
; CHECK: declare %opencl.reserve_id_t.5* @__work_group_reserve_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)*, i32, i32, i32) [[ATTR]]
; CHECK-NOT: opencl-vec-uniform-return
; CHECK: attributes [[ATTR]] = { {{.*}}opencl-vec-uniform-return{{.*}} }
; CHECK-NOT: opencl-vec-uniform-return

; Function Attrs: convergent
define void @test_pipe_workgroup_write_int(i32 addrspace(1)* %src, %opencl.pipe_wo_t.6 addrspace(1)* %out_pipe) #0 !recommended_vector_length !1 {
entry:
  %0 = tail call %opencl.reserve_id_t.5* @__work_group_reserve_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)* %out_pipe, i32 0, i32 4, i32 4) #0
  tail call void @__work_group_commit_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)* %out_pipe, %opencl.reserve_id_t.5* %0, i32 4, i32 4) #0
  ret void
}
; Function Attrs: convergent
declare %opencl.reserve_id_t.5* @__work_group_reserve_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)*, i32, i32, i32) #0
declare void @__work_group_commit_write_pipe(%opencl.pipe_wo_t.6 addrspace(1)*, %opencl.reserve_id_t.5*, i32, i32) #0

; Function Attrs: convergent
define void @test_pipe_workgroup_read_int(%opencl.pipe_ro_t.7 addrspace(1)* %in_pipe, i32 addrspace(1)* %dst) #0 !recommended_vector_length !1 {
entry:
  %0 = tail call %opencl.reserve_id_t.5* @__work_group_reserve_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)* %in_pipe, i32 0, i32 4, i32 4) #0
  tail call void @__work_group_commit_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)* %in_pipe, %opencl.reserve_id_t.5* %0, i32 4, i32 4) #0
  ret void
}
declare %opencl.reserve_id_t.5* @__work_group_reserve_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)*, i32, i32, i32) #0
declare void @__work_group_commit_read_pipe(%opencl.pipe_ro_t.7 addrspace(1)*, %opencl.reserve_id_t.5*, i32, i32) #0

attributes #0 = { convergent }

!opencl.ocl.version = !{!0}
!sycl.kernels = !{!2}

!0 = !{i32 2, i32 0}
!1 = !{i32 4}
!2 = !{void (i32 addrspace(1)*, %opencl.pipe_wo_t.6 addrspace(1)*)* @test_pipe_workgroup_write_int, void (%opencl.pipe_ro_t.7 addrspace(1)*, i32 addrspace(1)*)* @test_pipe_workgroup_read_int}

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
