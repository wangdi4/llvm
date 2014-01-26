; RUN: oclopt -deduce-max-dim -S < %s -runtimelib %p/../../vectorizer/Full/runtime.bc | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(i32 addrspace(1)* nocapture %A, i32 addrspace(1)* nocapture %B) nounwind {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %B, i64 %call
  %0 = load i32 addrspace(1)* %arrayidx, align 1
  %arrayidx1 = getelementptr inbounds i32 addrspace(1)* %A, i64 %call
  %1 = load i32 addrspace(1)* %arrayidx1, align 1
  %add = add nsw i32 %1, %0
  store i32 %add, i32 addrspace(1)* %arrayidx1, align 1
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone

define void @__Vectorized_.A(i32 addrspace(1)* nocapture %A, i32 addrspace(1)* nocapture %B) nounwind {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %0 = getelementptr inbounds i32 addrspace(1)* %B, i64 %call
  %ptrTypeCast = bitcast i32 addrspace(1)* %0 to <4 x i32> addrspace(1)*
  %1 = load <4 x i32> addrspace(1)* %ptrTypeCast, align 1
  %2 = getelementptr inbounds i32 addrspace(1)* %A, i64 %call
  %ptrTypeCast4 = bitcast i32 addrspace(1)* %2 to <4 x i32> addrspace(1)*
  %3 = load <4 x i32> addrspace(1)* %ptrTypeCast4, align 1
  %add5 = add nsw <4 x i32> %3, %1
  store <4 x i32> %add5, <4 x i32> addrspace(1)* %ptrTypeCast4, align 1
  ret void
}

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!9}
!opencl.kernel_info = !{!10, !22}
!opencl.module_info_list = !{}
!llvm.functions_info = !{}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @A, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5, metadata !6}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"int*", metadata !"int*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_base_type", metadata !"int*", metadata !"int*"}
!6 = metadata !{metadata !"kernel_arg_name", metadata !"A", metadata !"B"}
!7 = metadata !{i32 1, i32 0}
!8 = metadata !{i32 0, i32 0}
!9 = metadata !{}
!10 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @A, metadata !11}
!11 = metadata !{metadata !12, metadata !13, metadata !14, metadata !15, metadata !16, metadata !17, metadata !18, metadata !19, metadata !20, metadata !21}
!12 = metadata !{metadata !"local_buffer_size", null}
!13 = metadata !{metadata !"barrier_buffer_size", null}
!14 = metadata !{metadata !"kernel_execution_length", i32 8}
!15 = metadata !{metadata !"max_wg_dimensions", null}
; CHECK: !15 = metadata !{metadata !"max_wg_dimensions", i32 1}
!16 = metadata !{metadata !"kernel_has_barrier", i1 false}
!17 = metadata !{metadata !"no_barrier_path", i1 true}
!18 = metadata !{metadata !"vectorized_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.A}
!19 = metadata !{metadata !"vectorized_width", i32 1}
!20 = metadata !{metadata !"kernel_wrapper", null}
!21 = metadata !{metadata !"scalarized_kernel", null}
!22 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.A, metadata !23}
!23 = metadata !{metadata !12, metadata !13, metadata !24, metadata !15, metadata !16, metadata !25, metadata !26, metadata !27, metadata !20, metadata !28}
!24 = metadata !{metadata !"kernel_execution_length", i32 10}
!25 = metadata !{metadata !"no_barrier_path", null}
!26 = metadata !{metadata !"vectorized_kernel", null}
!27 = metadata !{metadata !"vectorized_width", i32 4}
!28 = metadata !{metadata !"scalarized_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @A}
