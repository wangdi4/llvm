; RUN: llvm-as %S/../builtin-lib.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

; This test checks that WorkItemAnalysis pass calculates WI dependency for gep with structure base type.
; Test was generated from this soure code
;
;  typedef struct Node{
;      int global_id;
;      int position_in_list;
;      __global struct Node* pNext;
;  } Node;
;
;  __kernel void create_linked_lists(__global Node* pNodes, volatile __attribute__((nosvm)) __global int* allocation_index, int list_length)
;  {
;      size_t i = get_global_id(0);
;      __global Node *pNode = &pNodes[i];
;
;      pNode->global_id = i;
;      pNode->position_in_list = 0;
;
;      __global Node *pNew;
;      for(int j = 1; j < list_length; j++)
;      {
;              pNew = &pNodes[ atomic_inc(allocation_index) ];// allocate a new node
;              pNew->global_id = i;
;              pNew->position_in_list = j;
;              pNode->pNext = pNew;  // link new node onto end of list
;              pNode = pNew;   // move to end of list
;      }
;  }

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

%struct.Node = type { i32, i32, ptr addrspace(1) }

; Function Attrs: nounwind
declare void @create_linked_lists(ptr addrspace(1), ptr addrspace(1), i32) #0

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

declare i32 @_Z10atomic_incPU3AS1Vi(ptr addrspace(1))

declare i64 @_Z14get_local_sizej(i32)

; CHECK: WorkItemAnalysis for function __Vectorized_.create_linked_lists:
; CHECK-NEXT: SEQ   %call = tail call i64 @_Z13get_global_idj(i32 0) #1
; CHECK-NEXT: PTR   %arrayidx = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNodes, i64 %call
; CHECK-NEXT: SEQ   %conv = trunc i64 %call to i32
; CHECK-NEXT: RND   store i32 %conv, ptr addrspace(1) %arrayidx, align 4
; CHECK-NEXT: RND   %position_in_list = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNodes, i64 %call, i32 1
; CHECK-NEXT: RND   store i32 0, ptr addrspace(1) %position_in_list, align 4
; CHECK-NEXT: UNI   %cmp1 = icmp sgt i32 %list_length, 1
; CHECK-NEXT: UNI   br i1 %cmp1, label %for.body.preheader, label %for.end
; CHECK-NEXT: UNI   br label %for.body
; CHECK-NEXT: UNI   %j.03 = phi i32 [ %inc, %for.body ], [ 1, %for.body.preheader ]
; CHECK-NEXT: RND   %pNode.02 = phi ptr addrspace(1) [ %arrayidx3, %for.body ], [ %arrayidx, %for.body.preheader ]
; CHECK-NEXT: RND   %call2 = tail call i32 @_Z10atomic_incPU3AS1Vi(ptr addrspace(1) %allocation_index) #0
; CHECK-NEXT: RND   %idxprom = sext i32 %call2 to i64
; CHECK-NEXT: RND   %arrayidx3 = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNodes, i64 %idxprom
; CHECK-NEXT: RND   store i32 %conv, ptr addrspace(1) %arrayidx3, align 4
; CHECK-NEXT: RND   %position_in_list6 = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNodes, i64 %idxprom, i32 1
; CHECK-NEXT: RND   store i32 %j.03, ptr addrspace(1) %position_in_list6, align 4
; CHECK-NEXT: RND   %pNext = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNode.02, i64 0, i32 2
; CHECK-NEXT: RND   store ptr addrspace(1) %arrayidx3, ptr addrspace(1) %pNext, align 8
; CHECK-NEXT: UNI   %inc = add nsw i32 %j.03, 1
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %inc, %list_length
; CHECK-NEXT: UNI   br i1 %exitcond, label %for.end.loopexit, label %for.body
; CHECK-NEXT: UNI   br label %for.end
; CHECK-NEXT: UNI   ret void

; Function Attrs: nounwind
define void @__Vectorized_.create_linked_lists(ptr addrspace(1) %pNodes, ptr addrspace(1) %allocation_index, i32 %list_length) #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #1
  %arrayidx = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNodes, i64 %call
  %conv = trunc i64 %call to i32
  store i32 %conv, ptr addrspace(1) %arrayidx, align 4
  %position_in_list = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNodes, i64 %call, i32 1
  store i32 0, ptr addrspace(1) %position_in_list, align 4
  %cmp1 = icmp sgt i32 %list_length, 1
  br i1 %cmp1, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %j.03 = phi i32 [ %inc, %for.body ], [ 1, %for.body.preheader ]
  %pNode.02 = phi ptr addrspace(1) [ %arrayidx3, %for.body ], [ %arrayidx, %for.body.preheader ]
  %call2 = tail call i32 @_Z10atomic_incPU3AS1Vi(ptr addrspace(1) %allocation_index) #0
  %idxprom = sext i32 %call2 to i64
  %arrayidx3 = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNodes, i64 %idxprom
  store i32 %conv, ptr addrspace(1) %arrayidx3, align 4
  %position_in_list6 = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNodes, i64 %idxprom, i32 1
  store i32 %j.03, ptr addrspace(1) %position_in_list6, align 4
  %pNext = getelementptr inbounds %struct.Node, ptr addrspace(1) %pNode.02, i64 0, i32 2
  store ptr addrspace(1) %arrayidx3, ptr addrspace(1) %pNext, align 8
  %inc = add nsw i32 %j.03, 1
  %exitcond = icmp eq i32 %inc, %list_length
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!10}
!opencl.kernel_info = !{!11}
!opencl.module_info_list = !{!26}
!llvm.functions_info = !{}

!0 = !{ptr @create_linked_lists, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"Node*", !"int*", !"int"}
!4 = !{!"kernel_arg_type_qual", !"", !"volatile", !""}
!5 = !{!"kernel_arg_base_type", !"struct Node*", !"int*", !"int"}
!6 = !{!"kernel_arg_name", !"pNodes", !"allocation_index", !"list_length"}
!7 = !{i32 1, i32 2}
!8 = !{i32 2, i32 0}
!9 = !{}
!10 = !{!"-cl-std=CL2.0"}
!11 = !{ptr @create_linked_lists, !12}
!12 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25}
!13 = !{!"local_buffer_size", null}
!14 = !{!"barrier_buffer_size", null}
!15 = !{!"kernel_execution_length", null}
!16 = !{!"max_wg_dimensions", null}
!17 = !{!"kernel_has_barrier", null}
!18 = !{!"kernel_has_global_sync", null}
!19 = !{!"no_barrier_path", i1 true}
!20 = !{!"vectorized_kernel", null}
!21 = !{!"vectorized_width", null}
!22 = !{!"kernel_wrapper", null}
!23 = !{!"scalar_kernel", null}
!24 = !{!"block_literal_size", null}
!25 = !{!"private_memory_size", null}
!26 = !{!27, !28, !29}
!27 = !{!"global_variable_total_size", null}
!28 = !{!"gen_addr_space_pointer_counter", null}
!29 = !{!"gen_addr_space_pointer_warnings"}
