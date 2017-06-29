; RUN: oclopt -deduce-max-dim -S < %s -runtimelib %p/../../vectorizer/Full/runtime.bc | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(i32 addrspace(1)* noalias %A, i32 %B) nounwind {
entry:
  %call = tail call i32 @_Z10atomic_addPU3AS1Vii(i32 addrspace(1)* %A, i32 %B) nounwind
  ret void
}

declare i32 @_Z10atomic_addPU3AS1Vii(i32 addrspace(1)*, i32)

define void @__Vectorized_.A(i32 addrspace(1)* noalias %A, i32 %B) nounwind {
entry:
  %0 = tail call i32 @_Z10atomic_addPU3AS1Vii(i32 addrspace(1)* %A, i32 %B) nounwind
  %1 = tail call i32 @_Z10atomic_addPU3AS1Vii(i32 addrspace(1)* %A, i32 %B) nounwind
  %2 = tail call i32 @_Z10atomic_addPU3AS1Vii(i32 addrspace(1)* %A, i32 %B) nounwind
  %3 = tail call i32 @_Z10atomic_addPU3AS1Vii(i32 addrspace(1)* %A, i32 %B) nounwind
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

!0 = !{void (i32 addrspace(1)*, i32)* @A, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int"}
!4 = !{!"kernel_arg_type_qual", !"", !""}
!5 = !{!"kernel_arg_base_type", !"int*", !"int"}
!6 = !{!"kernel_arg_name", !"A", !"B"}
!7 = !{i32 1, i32 0}
!8 = !{i32 0, i32 0}
!9 = !{}
!10 = !{void (i32 addrspace(1)*, i32)* @A, !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21}
; CHECK: !11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21}
!12 = !{!"local_buffer_size", null}
!13 = !{!"barrier_buffer_size", null}
!14 = !{!"kernel_execution_length", i32 2}
!15 = !{!"max_wg_dimensions", null}
; CHECK: !15 = !{!"max_wg_dimensions", null}
!16 = !{!"kernel_has_barrier", i1 false}
!17 = !{!"no_barrier_path", i1 true}
!18 = !{!"vectorized_kernel", void (i32 addrspace(1)*, i32)* @__Vectorized_.A}
!19 = !{!"vectorized_width", i32 1}
!20 = !{!"kernel_wrapper", null}
!21 = !{!"scalarized_kernel", null}
!22 = !{void (i32 addrspace(1)*, i32)* @__Vectorized_.A, !23}
!23 = !{!12, !13, !24, !15, !16, !25, !26, !27, !20, !28}
; CHECK: !23 = !{!12, !13, !24, !15, !16, !25, !26, !27, !20, !28}
!24 = !{!"kernel_execution_length", i32 5}
!25 = !{!"no_barrier_path", null}
!26 = !{!"vectorized_kernel", null}
!27 = !{!"vectorized_width", i32 4}
!28 = !{!"scalarized_kernel", void (i32 addrspace(1)*, i32)* @A}
