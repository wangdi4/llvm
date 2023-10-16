; RUN: opt -passes=sycl-kernel-deduce-max-dim -sycl-kernel-builtin-lib=%S/builtin-lib.rtl -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-deduce-max-dim -sycl-kernel-builtin-lib=%S/builtin-lib.rtl -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-NOT: max_wg_dimensions

define void @A(ptr addrspace(1) noalias %A, i32 %B) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !5 !kernel_arg_type_qual !4 !kernel_arg_name !6 !kernel_execution_length !14 !kernel_has_barrier !16 !no_barrier_path !17 !vectorized_kernel !18 !vectorized_width !19 !arg_type_null_val !29 {
entry:
  %call = tail call i32 @_Z10atomic_addPU3AS1Vii(ptr addrspace(1) %A, i32 %B) nounwind
  ret void
}

declare i32 @_Z10atomic_addPU3AS1Vii(ptr addrspace(1), i32)

define void @__Vectorized_.A(ptr addrspace(1) noalias %A, i32 %B) nounwind !kernel_execution_length !24 !vectorized_width !27 !scalar_kernel !28 {
entry:
  %0 = tail call i32 @_Z10atomic_addPU3AS1Vii(ptr addrspace(1) %A, i32 %B) nounwind
  %1 = tail call i32 @_Z10atomic_addPU3AS1Vii(ptr addrspace(1) %A, i32 %B) nounwind
  %2 = tail call i32 @_Z10atomic_addPU3AS1Vii(ptr addrspace(1) %A, i32 %B) nounwind
  %3 = tail call i32 @_Z10atomic_addPU3AS1Vii(ptr addrspace(1) %A, i32 %B) nounwind
  ret void
}

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!9}
!llvm.functions_info = !{}

!0 = !{ptr @A}
!1 = !{i32 1, i32 0}
!2 = !{!"none", !"none"}
!3 = !{!"int*", !"int"}
!4 = !{!"", !""}
!5 = !{!"int*", !"int"}
!6 = !{!"A", !"B"}
!7 = !{i32 1, i32 0}
!8 = !{i32 0, i32 0}
!9 = !{}
!14 = !{i32 2}
!16 = !{i1 false}
!17 = !{i1 true}
!18 = !{ptr @__Vectorized_.A}
!19 = !{i32 1}
!24 = !{i32 5}
!27 = !{i32 4}
!28 = !{ptr @A}
!29 = !{ptr addrspace(1) null, i32 0}

; DEBUGIFY-NOT: WARNING
