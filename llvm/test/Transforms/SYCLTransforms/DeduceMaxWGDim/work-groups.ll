; RUN: opt -passes=sycl-kernel-deduce-max-dim -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-deduce-max-dim -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; CHECK-NOT: max_wg_dimensions

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.scratch = internal addrspace(3) global [1024 x i8] undef, align 1

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(ptr addrspace(1) noundef align 4 %data1, ptr addrspace(1) noundef align 4 %data2, ptr addrspace(1) noundef align 4 %data3) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !arg_type_null_val !11 !no_barrier_path !12 !kernel_has_sub_groups !12 !kernel_has_global_sync !12 !kernel_execution_length !13 !arg_type_null_val !18 {
entry:
  %call = tail call i32 @_Z21work_group_reduce_addi(i32 noundef 3) #3
  store i32 %call, ptr addrspace(1) %data1, align 4, !tbaa !14
  %call1 = tail call i32 @_Z29work_group_scan_inclusive_addi(i32 noundef 3) #3
  store i32 %call1, ptr addrspace(1) %data2, align 4, !tbaa !14
  %cast.data = addrspacecast ptr addrspace(1) %data3 to ptr addrspace(4)
  tail call void @_Z66__devicelib_default_work_group_joint_sort_ascending_p1i32_u32_p1i8PU3AS4ijPU3AS4c(ptr addrspace(4) %cast.data, i32 5, ptr addrspace(4) addrspacecast (ptr addrspace(3) @test.scratch to ptr addrspace(4))) #3
  ret void
}

; Function Attrs: convergent nounwind
declare i32 @_Z21work_group_reduce_addi(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent nounwind
declare i32 @_Z29work_group_scan_inclusive_addi(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent
declare void @_Z66__devicelib_default_work_group_joint_sort_ascending_p1i32_u32_p1i8PU3AS4ijPU3AS4c(ptr addrspace(4), i32, ptr addrspace(4)) local_unnamed_addr #2

attributes #0 = { convergent norecurse nounwind "kernel-call-once" "kernel-convergent-call" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent "kernel-call-once" "kernel-convergent-call" "prefer-vector-width"="512" }
attributes #3 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }

!llvm.linker.options = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!3 = !{ptr @test}
!4 = !{i32 1, i32 1, i32 1}
!5 = !{!"none", !"none", !"none"}
!6 = !{!"int*", !"int*", !"int*"}
!7 = !{!"", !"", !""}
!8 = !{!"data1", !"data2", !"data3"}
!9 = !{i1 false, i1 false, i1 false}
!10 = !{i32 0, i32 0, i32 0}
!11 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}
!12 = !{i1 false}
!13 = !{i32 7}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
