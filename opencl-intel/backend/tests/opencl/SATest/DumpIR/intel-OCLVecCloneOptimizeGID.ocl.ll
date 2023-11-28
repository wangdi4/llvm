source_filename = "1"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test(ptr addrspace(1) %dst) #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) #1
  %conv = trunc i64 %call to i32
  %idxprom = sext i32 %conv to i64
  %ptridx = getelementptr inbounds i64, ptr addrspace(1) %dst, i64 %idxprom
  store i64 %idxprom, ptr addrspace(1) %ptridx, align 8, !tbaa !10
  ret void
}

; Function Attrs: convergent nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) #1

attributes #0 = { convergent norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!1}

!0 = !{i32 2, i32 0}
!1 = !{}
!3 = !{i32 1}
!4 = !{!"none"}
!5 = !{!"size_t*"}
!6 = !{!"ulong*"}
!7 = !{!""}
!8 = !{i1 false}
!9 = !{i32 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"long", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
