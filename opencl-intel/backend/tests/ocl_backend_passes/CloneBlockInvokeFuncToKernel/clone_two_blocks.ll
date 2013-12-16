; Test to check kernel and metadata is created for block_invoke functions
; RUN: opt < %s -cloneblockinvokefunctokernel -S | FileCheck %s

define void @enqueue_simple_block(i32 addrspace(1)* %res) nounwind {
  ret void
}

; test "internal" linkage is dropped
; CHECK: define void @__enqueue_simple_block_block_invoke(i8* %.block_descriptor)
; test kernel function is created
; CHECK: define void @__.kernel__enqueue_simple_block_block_invoke(i8* %.block_descriptor)
define internal void @__enqueue_simple_block_block_invoke(i8* %.block_descriptor) nounwind {
  ret void
}


define void @enqueue_simple_block2(i32 addrspace(1)* %res, float addrspace(1)* %fres) nounwind {
  ret void
}

; CHECK: define void @__.kernel__enqueue_simple_block2_block_invoke(i8* %.block_descriptor, float addrspace(1)* %fres)
define internal void @__enqueue_simple_block2_block_invoke(i8* %.block_descriptor, float addrspace(1)* %fres) nounwind {
  ret void
}

; CHECK: !opencl.kernels = !{!0, !2, !4, !5}
!opencl.kernels = !{!0, !2}

!0 = metadata !{void (i32 addrspace(1)*)* @enqueue_simple_block, metadata !1}
!1 = metadata !{metadata !"argument_attribute", i32 0}
!2 = metadata !{void (i32 addrspace(1)*, float addrspace(1)*)* @enqueue_simple_block2, metadata !3}
!3 = metadata !{metadata !"argument_attribute", i32 0, i32 0}

; CHECK: !4 = metadata !{void (i8*)* @__.kernel__enqueue_simple_block_block_invoke
; CHECK: !5 = metadata !{void (i8*, float addrspace(1)*)* @__.kernel__enqueue_simple_block2_block_invoke
