target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ch = addrspace(1) global target("spirv.Channel") zeroinitializer, align 8, !packet_size !0, !packet_align !0

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test(i32 %data) #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !6 !kernel_arg_name !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 {
entry:
  %data.addr = alloca i32, align 4
  store i32 %data, ptr %data.addr, align 4, !tbaa !9
  %0 = load target("spirv.Channel"), ptr addrspace(1) @ch, align 4, !tbaa !13
  %1 = load i32, ptr %data.addr, align 4, !tbaa !9
  call void @_Z19write_channel_intel11ocl_channelii(target("spirv.Channel") %0, i32 %1) #2
  ret void
}

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(target("spirv.Channel"), i32) #1

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}

!0 = !{i32 4}
!1 = !{i32 2, i32 0}
!2 = !{!"-cl-std=CL2.0"}
!3 = !{i32 0}
!4 = !{!"none"}
!5 = !{!"int"}
!6 = !{!""}
!7 = !{!"data"}
!8 = !{i1 false}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!11, !11, i64 0}
