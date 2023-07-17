; RUN: opt -passes='debugify,sycl-kernel-resolve-sub-group-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-resolve-sub-group-wi-call' -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #2

; Function Attrs: convergent
declare i32 @_Z16get_sub_group_idv() local_unnamed_addr #2

; Function Attrs: convergent
declare i32 @_Z18get_sub_group_sizev() local_unnamed_addr #2

declare i64 @_Z14get_local_sizej(i32)

; Function Attrs: convergent nounwind
define void @testKernel(ptr addrspace(1) %sub_group_local_ids, ptr addrspace(1) %sub_groups_ids, ptr addrspace(1) %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !4 !no_barrier_path !19 !kernel_has_sub_groups !19 !vectorized_width !30 !vectorization_dimension !26 !can_unite_workgroups !29 !arg_type_null_val !31 {
entry:
  ret void
}

; CHECK-LABEL: @__Vectorized_.testKernel
; Function Attrs: convergent nounwind
define void @__Vectorized_.testKernel(ptr addrspace(1) %sub_group_local_ids, ptr addrspace(1) %sub_groups_ids, ptr addrspace(1) %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !20 !no_barrier_path !19 !kernel_has_sub_groups !19 !vectorized_width !28 !vectorization_dimension !26 !can_unite_workgroups !29 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %call1 = tail call i32 @_Z22get_sub_group_local_idv() #4
  %temp2 = insertelement <16 x i32> undef, i32 %call1, i32 0
  %vector1 = shufflevector <16 x i32> %temp2, <16 x i32> undef, <16 x i32> zeroinitializer
  %0 = add <16 x i32> %vector1, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %1 = getelementptr inbounds i32, ptr addrspace(1) %sub_group_local_ids, i64 %call
  store <16 x i32> %0, ptr addrspace(1) %1, align 4
  %call2 = tail call i32 @_Z16get_sub_group_idv() #4
; CHECK-NOT: @_Z16get_sub_group_idv
; CHECK: %lid2 = call i64 @_Z12get_local_idj(i32 2)
; CHECK: %lid1 = call i64 @_Z12get_local_idj(i32 1)
; CHECK: %lid0 = call i64 @_Z12get_local_idj(i32 0)
; CHECK: %lsz2 = call i64 @_Z14get_local_sizej(i32 2)
; CHECK: %lsz1 = call i64 @_Z14get_local_sizej(i32 1)
; CHECK: %lsz0 = call i64 @_Z14get_local_sizej(i32 0)
; CHECK: %sg.id.op0 = mul i64 %lid2, %lsz1
; CHECK: %sg.id.op1 = add i64 %sg.id.op0, %lid1
; CHECK: %sg.id.op2 = sub i64 %lsz0, 1
; CHECK: %sg.id.op3 = udiv i64 %sg.id.op2, 16
; CHECK: %sg.id.op4 = add i64 %sg.id.op3, 1
; CHECK: %sg.id.op5 = mul i64 %sg.id.op4, %sg.id.op1
; CHECK: %sg.id.op6 = udiv i64 %lid0, 16
; CHECK: %sg.id.res = add i64 %sg.id.op5, %sg.id.op6
; CHECK: %sg.id.res.trunc = trunc i64 %sg.id.res to i32
; CHECK: insertelement <16 x i32> undef
; CHECK-SAME: i32 %sg.id.res.trunc, i32 0
  %temp20 = insertelement <16 x i32> undef, i32 %call2, i32 0
  %vector19 = shufflevector <16 x i32> %temp20, <16 x i32> undef, <16 x i32> zeroinitializer
  %2 = getelementptr inbounds i32, ptr addrspace(1) %sub_groups_ids, i64 %call
  store <16 x i32> %vector19, ptr addrspace(1) %2, align 4
  %call4 = tail call i32 @_Z18get_sub_group_sizev() #4
; CHECK-NOT: @_Z18get_sub_group_sizev
; CHECK: [[local_size:%[0-9]+]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK: %uniform.id.max = and i64 -16, %3
; CHECK: %nonuniform.size = sub i64 [[local_size]], %uniform.id.max
; CHECK: [[local_id:%[0-9]+]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK: [[cond:%[0-9]+]] = icmp ult i64 [[local_id]], %uniform.id.max
; CHECK: [[result:%[0-9]+]] = select i1 [[cond]], i64 16, i64 %nonuniform.size
; CHECK: %subgroup.size = trunc i64 [[result]] to i32
  %temp23 = insertelement <16 x i32> undef, i32 %call4, i32 0
  %vector22 = shufflevector <16 x i32> %temp23, <16 x i32> undef, <16 x i32> zeroinitializer
  %3 = getelementptr inbounds i32, ptr addrspace(1) %sub_groups_sizes, i64 %call
  store <16 x i32> %vector22, ptr addrspace(1) %3, align 4
  ret void
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
!3 = !{ptr @testKernel}
!4 = !{ptr @__Vectorized_.testKernel}
!11 = !{i32 1, i32 1, i32 1}
!12 = !{!"none", !"none", !"none"}
!13 = !{!"uint*", !"uint*", !"uint*"}
!14 = !{!"", !"", !""}
!15 = !{i1 false, i1 false, i1 false}
!16 = !{i32 0, i32 0, i32 0}
!17 = !{!"sub_group_local_ids", !"sub_groups_ids", !"sub_groups_sizes"}
!19 = !{i1 true}
!20 = !{null}
!21 = !{i32 1}
!22 = !{!23, !23, i64 0}
!23 = !{!"int", !24, i64 0}
!24 = !{!"omnipotent char", !25, i64 0}
!25 = !{!"Simple C/C++ TBAA"}
!26 = !{i32 0}
!27 = !{i32 3}
!28 = !{i32 16}
!29 = !{i1 false}
!30 = !{i32 1}
!31 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
