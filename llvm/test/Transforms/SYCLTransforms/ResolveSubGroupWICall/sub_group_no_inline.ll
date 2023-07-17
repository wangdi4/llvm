;; Verify the cases in which subgroup calls can't be inlined into the kernel body
; RUN: opt -passes='debugify,sycl-kernel-resolve-sub-group-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-resolve-sub-group-wi-call' -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z22get_max_sub_group_sizev() local_unnamed_addr #2

; Function Attrs: convergent
declare i32 @_Z16get_sub_group_idv() local_unnamed_addr #2

declare i64 @_Z14get_local_sizej(i32)

; Function Attrs: convergent nounwind
define void @testKernel(ptr addrspace(1) noalias %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !18 !no_barrier_path !19 !kernel_has_sub_groups !19 !vectorized_width !26 !vectorization_dimension !16 !can_unite_workgroups !15 !arg_type_null_val !28 {
entry:
  ret void
}

; Function Attrs: convergent nounwind
define void @__Vectorized_.testKernel(ptr addrspace(1) noalias %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !20 !no_barrier_path !19 !kernel_has_sub_groups !19 !vectorized_width !25 !vectorization_dimension !16 !can_unite_workgroups !15 {
entry:
; CHECK-LABEL: @__Vectorized_.testKernel
; CHECK: call i32 @foo1(i64 16)
  %call1 = tail call i32 @foo1() #4
  ret void
}

; Function Attrs: convergent nounwind
define void @testKernel1(ptr addrspace(1) noalias %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !20 !no_barrier_path !19 !kernel_has_sub_groups !19 !vectorized_width !27 !vectorization_dimension !16 !can_unite_workgroups !15 {
entry:
; CHECK-LABEL: @testKernel1
; CHECK: call i32 @foo1(i64 8)
  %call1 = tail call i32 @foo1() #4
  ret void
}

; CHECK-NOT: define i32 @foo1()
; CHECK-NOT: define i32 @foo()

define i32 @foo1() {
entry:
%call = tail call i32 @foo()
ret i32 %call
}

define i32 @foo() {
entry:
 %call = tail call i32 @_Z22get_max_sub_group_sizev()
 %call1 = tail call i32 @_Z16get_sub_group_idv()
; CHECK-NOT: @_Z22get_max_sub_group_sizev()
; CHECK-NOT: @_Z16get_sub_group_idv()
; CHECK: %max.sg.size = trunc i64 %vf to i32
; CHECK: %lid2 = call i64 @_Z12get_local_idj(i32 2)
; CHECK: %lid1 = call i64 @_Z12get_local_idj(i32 1)
; CHECK: %lid0 = call i64 @_Z12get_local_idj(i32 0)
; CHECK: %lsz2 = call i64 @_Z14get_local_sizej(i32 2)
; CHECK: %lsz1 = call i64 @_Z14get_local_sizej(i32 1)
; CHECK: %lsz0 = call i64 @_Z14get_local_sizej(i32 0)
; CHECK: %sg.id.op0 = mul i64 %lid2, %lsz1
; CHECK: %sg.id.op1 = add i64 %sg.id.op0, %lid1
; CHECK: %sg.id.op2 = sub i64 %lsz0, 1
; CHECK: %sg.id.op3 = udiv i64 %sg.id.op2, %vf
; CHECK: %sg.id.op4 = add i64 %sg.id.op3, 1
; CHECK: %sg.id.op5 = mul i64 %sg.id.op4, %sg.id.op1
; CHECK: %sg.id.op6 = udiv i64 %lid0, %vf
; CHECK: %sg.id.res = add i64 %sg.id.op5, %sg.id.op6
; CHECK: %sg.id.res.trunc = trunc i64 %sg.id.res to i32
; CHECK: ret i32 %max.sg.size
 ret i32 %call
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-sign
ed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"=
"false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-pro
tector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone }
attributes #4 = { convergent nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!sycl.kernels = !{!3}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{ptr @testKernel, ptr @testKernel1}
!11 = !{i32 1}
!12 = !{!"none"}
!13 = !{!"uint*"}
!14 = !{!""}
!15 = !{i1 false}
!16 = !{i32 0}
!17 = !{!"sub_groups_sizes"}
!18 = !{ptr @__Vectorized_.testKernel}
!19 = !{i1 true}
!20 = !{null}
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !23, i64 0}
!23 = !{!"omnipotent char", !24, i64 0}
!24 = !{!"Simple C/C++ TBAA"}
!25 = !{i32 16}
!26 = !{i32 1}
!27 = !{i32 8}
!28 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
