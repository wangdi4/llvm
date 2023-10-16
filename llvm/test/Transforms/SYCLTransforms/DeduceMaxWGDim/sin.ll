; RUN: opt -passes=sycl-kernel-deduce-max-dim -sycl-kernel-builtin-lib=%S/builtin-lib.rtl -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-deduce-max-dim -sycl-kernel-builtin-lib=%S/builtin-lib.rtl -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: @D{{.*}}!max_wg_dimensions ![[WG_DIM:[0-9]+]]
; CHECK: ![[WG_DIM]] = !{i32 1}

define void @D(ptr addrspace(1) nocapture %A, ptr addrspace(1) nocapture %B) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !5 !kernel_arg_type_qual !4 !kernel_arg_name !6 !kernel_execution_length !14 !kernel_has_barrier !16 !no_barrier_path !17 !vectorized_kernel !18 !vectorized_width !19 !arg_type_null_val !29 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %B, i64 %call
  %0 = load i32, ptr addrspace(1) %arrayidx, align 1
  %conv = sitofp i32 %0 to float
  %call1 = tail call float @_Z3sinf(float %conv) nounwind readnone
  %arrayidx2 = getelementptr inbounds i32, ptr addrspace(1) %A, i64 %call
  %1 = load i32, ptr addrspace(1) %arrayidx2, align 1
  %conv3 = sitofp i32 %1 to float
  %add = fadd float %call1, %conv3
  %conv4 = fptosi float %add to i32
  store i32 %conv4, ptr addrspace(1) %arrayidx2, align 1
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone

declare float @_Z3sinf(float) nounwind readnone

define void @__Vectorized_.D(ptr addrspace(1) nocapture %A, ptr addrspace(1) nocapture %B) nounwind !kernel_execution_length !24 !vectorized_width !27 !scalar_kernel !28 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %0 = getelementptr inbounds i32, ptr addrspace(1) %B, i64 %call
  %1 = load <8 x i32>, ptr addrspace(1) %0, align 1
  %conv8 = sitofp <8 x i32> %1 to <8 x float>
  %2 = call <8 x float> @_Z3sinDv8_f(<8 x float> %conv8)
  %3 = getelementptr inbounds i32, ptr addrspace(1) %A, i64 %call
  %4 = load <8 x i32>, ptr addrspace(1) %3, align 1
  %conv310 = sitofp <8 x i32> %4 to <8 x float>
  %add11 = fadd <8 x float> %2, %conv310
  %conv412 = fptosi <8 x float> %add11 to <8 x i32>
  store <8 x i32> %conv412, ptr addrspace(1) %3, align 1
  ret void
}

declare <8 x float> @_Z3sinDv8_f(<8 x float>) nounwind readnone

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!9}
!llvm.functions_info = !{}

!0 = !{ptr @D}
!1 = !{i32 1, i32 1}
!2 = !{!"none", !"none"}
!3 = !{!"int*", !"int*"}
!4 = !{!"", !""}
!5 = !{!"int*", !"int*"}
!6 = !{!"A", !"B"}
!7 = !{i32 1, i32 0}
!8 = !{i32 0, i32 0}
!9 = !{}
!14 = !{i32 12}
!16 = !{i1 false}
!17 = !{i1 true}
!18 = !{ptr @__Vectorized_.D}
!19 = !{i32 1}
!24 = !{i32 14}
!27 = !{i32 8}
!28 = !{ptr @D}
!29 = !{ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
