target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test(ptr addrspace(1) noundef align 4 %dst) #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !3 !kernel_arg_name !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !13 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8, !tbaa !14
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !tbaa !14
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 0
  store i32 2, ptr addrspace(1) %arrayidx, align 4, !tbaa !18
  ret void
}

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.stat.type = !{!3}
!sycl.stat.exec_time = !{!4}
!sycl.stat.run_time_version = !{!5}
!sycl.stat.workload_name = !{!6}
!sycl.stat.module_name = !{!7}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!""}
!4 = !{!"2022-07-11 15:10:54"}
!5 = !{!"2022.14.7.0"}
!6 = !{!"framework_test_type"}
!7 = !{!"framework_test_type1"}
!8 = !{i32 1}
!9 = !{!"none"}
!10 = !{!"int*"}
!11 = !{!"dst"}
!12 = !{i1 false}
!13 = !{i32 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"any pointer", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !{!19, !19, i64 0}
!19 = !{!"int", !16, i64 0}
