target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

define dso_local spir_kernel void @test(ptr addrspace(1) noundef align 4 %a) #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #2
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %call
  %0 = load i32, ptr addrspace(1) %arrayidx, align 4, !tbaa !8
  %mul = shl nsw i32 %0, 1
  store i32 %mul, ptr addrspace(1) %arrayidx, align 4, !tbaa !8
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32 noundef) #1

attributes #0 = { convergent norecurse nounwind optnone noinline "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "sycl-optlevel"="0" }
attributes #1 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent nounwind readnone willreturn }

!spirv.Source = !{!0}

!0 = !{i32 4, i32 100000}
!1 = !{i32 1, i32 1}
!2 = !{!"none", !"none"}
!3 = !{!"int*", !"int*"}
!4 = !{!"", !""}
!5 = !{!"a", !"b"}
!6 = !{i1 false, i1 false}
!7 = !{i32 0, i32 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
