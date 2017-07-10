; RUN: oclopt -deduce-max-dim -S < %s -runtimelib %p/../../vectorizer/Full/runtime.bc | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Call to an internal function (which was not inlined)

define i32 @foo(i32 %X) readnone nounwind {
entry:
  ret i32 %X
}

; CHECK-NOT: max_wg_dimensions
define void @A(i32 addrspace(1)* nocapture %A, i32 addrspace(1)* nocapture %B) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_execution_length !14 !kernel_has_barrier !16 !no_barrier_path !17 !vectorized_width !19 {
entry:
  %gid = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %B, i64 %gid
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 1
  %call = call i32 @foo(i32 %0) readnone nounwind
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %A, i64 %gid
  store i32 %call, i32 addrspace(1)* %arrayidx1, align 1
  ret void
}
declare i64 @_Z13get_global_idj(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!9}
!llvm.functions_info = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @A}
!1 = !{i32 1, i32 1}
!2 = !{!"none", !"none"}
!3 = !{!"int*", !"int*"}
!4 = !{!"", !""}
!5 = !{!"int*", !"int*"}
!6 = !{!"A", !"B"}
!7 = !{i32 1, i32 0}
!8 = !{i32 0, i32 0}
!9 = !{}
!14 = !{i32 2}
!16 = !{i1 false}
!17 = !{i1 true}
!19 = !{i32 1}
