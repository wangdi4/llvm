target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
declare !kernel_arg_addr_space !9 !kernel_arg_access_qual !10 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !4 !kernel_arg_name !12 !kernel_arg_host_accessible !13 !kernel_arg_pipe_depth !14 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !no_barrier_path !15 !kernel_has_sub_groups !13 !kernel_has_global_sync !13 !kernel_execution_length !16 !max_wg_dimensions !14 !local_buffer_size !14 !barrier_buffer_size !14 !vectorized_width !9 !kernel_wrapper !17 !private_memory_size !14 dso_local void @__test_separated_args(i32 addrspace(1)* noalias noundef align 4, i8 addrspace(3)* noalias, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* noalias, i64* noalias, [4 x i64], i8* noalias, {}* noalias) local_unnamed_addr #0

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.experimental.noalias.scope.decl(metadata) #1

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(i8* noalias %UniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) #0 !kernel_arg_addr_space !9 !kernel_arg_access_qual !10 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !4 !kernel_arg_name !12 !kernel_arg_host_accessible !13 !kernel_arg_pipe_depth !14 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !no_barrier_path !15 !kernel_has_sub_groups !13 !kernel_has_global_sync !13 !kernel_execution_length !16 !max_wg_dimensions !14 !local_buffer_size !14 !barrier_buffer_size !14 !vectorized_width !9 !private_memory_size !14 {
wrapper_entry:
  %0 = bitcast i8* %UniformArgs to i32 addrspace(1)**
  %explicit_0 = load i32 addrspace(1)*, i32 addrspace(1)** %0, align 8, !restrict !18
  store i32 2, i32 addrspace(1)* %explicit_0, align 4, !tbaa !19
  ret void
}

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" }
attributes #1 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}
!dpcpp.stat.type = !{!4}
!dpcpp.stat.exec_time = !{!5}
!dpcpp.stat.run_time_version = !{!6}
!dpcpp.stat.workload_name = !{!7}
!dpcpp.stat.module_name = !{!8}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @__test_separated_args}
!4 = !{!""}
!5 = !{!"2022-07-11 15:10:54"}
!6 = !{!"2022.14.7.0"}
!7 = !{!"framework_test_type"}
!8 = !{!"framework_test_type1"}
!9 = !{i32 1}
!10 = !{!"none"}
!11 = !{!"int*"}
!12 = !{!"dst"}
!13 = !{i1 false}
!14 = !{i32 0}
!15 = !{i1 true}
!16 = !{i32 2}
!17 = !{void (i8*, i64*, {}*)* @test}
!18 = !{null}
!19 = !{!20, !20, i64 0}
!20 = !{!"int", !21, i64 0}
!21 = !{!"omnipotent char", !22, i64 0}
!22 = !{!"Simple C/C++ TBAA"}
