; RUN: %oclopt -resolve-sub-group-wi-call -S < %s | FileCheck %s
; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z18get_num_sub_groupsv() local_unnamed_addr #2

; Function Attrs: convergent
declare i32 @_Z27get_enqueued_num_sub_groupsv() local_unnamed_addr #2

declare i64 @_Z14get_local_sizej(i32)

; Function Attrs: convergent nounwind
; CHECK-LABEL: @__Vectorized_.testKernel
define void @__Vectorized_.testKernel(i32 addrspace(1)* noalias %results) local_unnamed_addr #0 !kernel_arg_addr_space !12 !kernel_arg_access_qual !13 !kernel_arg_type !14 !kernel_arg_base_type !14 !kernel_arg_type_qual !15 !kernel_arg_host_accessible !16 !kernel_arg_pipe_depth !17 !kernel_arg_pipe_io !15 !kernel_arg_buffer_location !15 !kernel_arg_name !18 !vectorized_kernel !21 !no_barrier_path !20 !kernel_has_sub_groups !20 !vectorized_width !26 !vectorization_dimension !17 !can_unite_workgroups !16 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %call1 = tail call i32 @_Z18get_num_sub_groupsv() #4
; CHECK-NOT: @_Z18get_num_sub_groupsv
; CHECK: %lsz0 = call i64 @_Z14get_local_sizej(i32 0)
; CHECK: %lsz1 = call i64 @_Z14get_local_sizej(i32 1)
; CHECK: %lsz2 = call i64 @_Z14get_local_sizej(i32 2)
; CHECK: %mul.lsz0.lsz1 = mul i64 %lsz0, %lsz1
; CHECK: %mul.lsz0.lsz1.lsz2 = mul i64 %mul.lsz0.lsz1, %lsz2
; CHECK: %lszs.div = udiv i64 %mul.lsz0.lsz1.lsz2, 16
; CHECK: %lszs.div.trunc = trunc i64 %lszs.div to i32
  %call2 = tail call i32 @_Z27get_enqueued_num_sub_groupsv() #4
; CHECK-NOT: @_Z27get_enqueued_num_sub_groupsv
; CHECK: %enqdlz0 = call i64 @_Z23get_enqueued_local_sizej(i32 0)
; CHECK: %enqdlz1 = call i64 @_Z23get_enqueued_local_sizej(i32 1)
; CHECK: %enqdlz2 = call i64 @_Z23get_enqueued_local_sizej(i32 2)
; CHECK: %mul.enqdlsz0.enqdlsz1 = mul i64 %enqdlz0, %enqdlz1
; CHECK: %mul.enqdlsz0.enqdlsz1.enqdlsz2 = mul i64 %mul.enqdlsz0.enqdlsz1, %enqdlz2
; CHECK: %lgid.res.div = udiv i64 %mul.enqdlsz0.enqdlsz1.enqdlsz2, 16
; CHECK: %lgid.res.div.trunc = trunc i64 %lgid.res.div to i32
; CHECK: icmp eq i32 %lszs.div.trunc, %lgid.res.div.trunc
  %cmp = icmp eq i32 %call1, %call2
  %cond = zext i1 %cmp to i32
  %temp17 = insertelement <16 x i32> undef, i32 %cond, i32 0
  %vector16 = shufflevector <16 x i32> %temp17, <16 x i32> undef, <16 x i32> zeroinitializer
  %0 = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %call
  %ptrTypeCast = bitcast i32 addrspace(1)* %0 to <16 x i32> addrspace(1)*
  store <16 x i32> %vector16, <16 x i32> addrspace(1)* %ptrTypeCast, align 4
  ret void
}

declare i64 @_Z23get_enqueued_local_sizej(i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-sign
ed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
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
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-cl-std=CL2.0"}
!12 = !{i32 1}
!13 = !{!"none"}
!14 = !{!"uint*"}
!15 = !{!""}
!16 = !{i1 false}
!17 = !{i32 0}
!18 = !{!"results"}
!19 = !{void (i32 addrspace(1)*)* @__Vectorized_.testKernel}
!20 = !{i1 true}
!21 = !{null}
!22 = !{!23, !23, i64 0}
!23 = !{!"int", !24, i64 0}
!24 = !{!"omnipotent char", !25, i64 0}
!25 = !{!"Simple C/C++ TBAA"}
!26 = !{i32 16}

