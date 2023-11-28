target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind optnone noinline
define dso_local spir_kernel void @test(ptr addrspace(1) noundef align 4 %a, ptr addrspace(1) noundef align 4 %b) #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #1
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %call
  %0 = load i32, ptr addrspace(1) %arrayidx, align 4, !tbaa !9
  %mul = shl nsw i32 %0, 1
  %arrayidx1 = getelementptr inbounds i32, ptr addrspace(1) %b, i64 %call
  store i32 %mul, ptr addrspace(1) %arrayidx1, align 4, !tbaa !9
  ret void
}

; Function Attrs: convergent nounwind readnone willreturn
declare spir_func i64 @_Z13get_global_idj(i32 noundef) #1

attributes #0 = { convergent norecurse nounwind optnone noinline "uniform-work-group-size"="false" }
attributes #1 = { convergent nounwind readnone willreturn }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!spirv.Source = !{!1}

!0 = !{i32 2, i32 0}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 1}
!3 = !{!"none", !"none"}
!4 = !{!"int*", !"int*"}
!5 = !{!"", !""}
!6 = !{!"a", !"b"}
!7 = !{i1 false, i1 false}
!8 = !{i32 0, i32 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
