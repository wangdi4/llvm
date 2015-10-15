; RUN: oclopt -deduce-max-dim -S < %s -runtimelib %p/../../vectorizer/Full/runtime.bc | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @D(i32 addrspace(1)* nocapture %A, i32 addrspace(1)* nocapture %B) nounwind {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %B, i64 %call
  %0 = load i32 addrspace(1)* %arrayidx, align 1
  %conv = sitofp i32 %0 to float
  %call1 = tail call float @_Z3sinf(float %conv) nounwind readnone
  %arrayidx2 = getelementptr inbounds i32 addrspace(1)* %A, i64 %call
  %1 = load i32 addrspace(1)* %arrayidx2, align 1
  %conv3 = sitofp i32 %1 to float
  %add = fadd float %call1, %conv3
  %conv4 = fptosi float %add to i32
  store i32 %conv4, i32 addrspace(1)* %arrayidx2, align 1
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone

declare float @_Z3sinf(float) nounwind readnone

define void @__Vectorized_.D(i32 addrspace(1)* nocapture %A, i32 addrspace(1)* nocapture %B) nounwind {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %0 = getelementptr inbounds i32 addrspace(1)* %B, i64 %call
  %ptrTypeCast = bitcast i32 addrspace(1)* %0 to <8 x i32> addrspace(1)*
  %1 = load <8 x i32> addrspace(1)* %ptrTypeCast, align 1
  %conv8 = sitofp <8 x i32> %1 to <8 x float>
  %2 = call <8 x float> @_Z3sinDv8_f(<8 x float> %conv8)
  %3 = getelementptr inbounds i32 addrspace(1)* %A, i64 %call
  %ptrTypeCast9 = bitcast i32 addrspace(1)* %3 to <8 x i32> addrspace(1)*
  %4 = load <8 x i32> addrspace(1)* %ptrTypeCast9, align 1
  %conv310 = sitofp <8 x i32> %4 to <8 x float>
  %add11 = fadd <8 x float> %2, %conv310
  %conv412 = fptosi <8 x float> %add11 to <8 x i32>
  store <8 x i32> %conv412, <8 x i32> addrspace(1)* %ptrTypeCast9, align 1
  ret void
}

declare <8 x float> @_Z3sinDv8_f(<8 x float>) nounwind readnone

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

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @D, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int*"}
!4 = !{!"kernel_arg_type_qual", !"", !""}
!5 = !{!"kernel_arg_base_type", !"int*", !"int*"}
!6 = !{!"kernel_arg_name", !"A", !"B"}
!7 = !{i32 1, i32 0}
!8 = !{i32 0, i32 0}
!9 = !{}
!10 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @D, !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21}
!12 = !{!"local_buffer_size", null}
!13 = !{!"barrier_buffer_size", null}
!14 = !{!"kernel_execution_length", i32 12}
!15 = !{!"max_wg_dimensions", null}
; CHECK: !15 = !{!"max_wg_dimensions", i32 1}
!16 = !{!"kernel_has_barrier", i1 false}
!17 = !{!"no_barrier_path", i1 true}
!18 = !{!"vectorized_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.D}
!19 = !{!"vectorized_width", i32 1}
!20 = !{!"kernel_wrapper", null}
!21 = !{!"scalarized_kernel", null}
!22 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.D, !23}
!23 = !{!12, !13, !24, !15, !16, !25, !26, !27, !20, !28}
!24 = !{!"kernel_execution_length", i32 14}
!25 = !{!"no_barrier_path", null}
!26 = !{!"vectorized_kernel", null}
!27 = !{!"vectorized_width", i32 8}
!28 = !{!"scalarized_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @D}
