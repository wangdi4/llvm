; ModuleID = 'sub_group_reduce_min_uint.ll'
source_filename = "sub_group_reduce_min_uint.ll"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test_redmin(i32 addrspace(1)* noundef %in, <4 x i32> addrspace(1)* noundef %xy, i32 addrspace(1)* noundef %out) #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #4
  %conv = trunc i64 %call to i32
  %call1 = call spir_func i32 @_Z22get_sub_group_local_idv() #5
  %idxprom = sext i32 %conv to i64
  %arrayidx = getelementptr inbounds <4 x i32>, <4 x i32> addrspace(1)* %xy, i64 %idxprom
  %0 = load <4 x i32>, <4 x i32> addrspace(1)* %arrayidx, align 16
  %1 = insertelement <4 x i32> %0, i32 %call1, i64 0
  store <4 x i32> %1, <4 x i32> addrspace(1)* %arrayidx, align 16
  %call2 = call spir_func i32 @_Z16get_sub_group_idv() #5
  %idxprom3 = sext i32 %conv to i64
  %arrayidx4 = getelementptr inbounds <4 x i32>, <4 x i32> addrspace(1)* %xy, i64 %idxprom3
  %2 = load <4 x i32>, <4 x i32> addrspace(1)* %arrayidx4, align 16
  %3 = insertelement <4 x i32> %2, i32 %call2, i64 1
  store <4 x i32> %3, <4 x i32> addrspace(1)* %arrayidx4, align 16
  %idxprom5 = sext i32 %conv to i64
  %arrayidx6 = getelementptr inbounds i32, i32 addrspace(1)* %in, i64 %idxprom5
  %4 = load i32, i32 addrspace(1)* %arrayidx6, align 4, !tbaa !12
  %call7 = call spir_func i32 @_Z20sub_group_reduce_minj(i32 noundef %4) #5
  %idxprom8 = sext i32 %conv to i64
  %arrayidx9 = getelementptr inbounds i32, i32 addrspace(1)* %out, i64 %idxprom8
  store i32 %call7, i32 addrspace(1)* %arrayidx9, align 4, !tbaa !12
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: convergent nounwind readnone willreturn
declare spir_func i64 @_Z13get_global_idj(i32 noundef) #2

; Function Attrs: convergent
declare spir_func i32 @_Z22get_sub_group_local_idv() #3

; Function Attrs: convergent
declare spir_func i32 @_Z16get_sub_group_idv() #3

; Function Attrs: convergent
declare spir_func i32 @_Z20sub_group_reduce_minj(i32 noundef) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #4 = { convergent nounwind readnone willreturn }
attributes #5 = { convergent nounwind }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 3, i32 0}
!1 = !{!"-cl-std=CL3.0"}
!2 = !{!"clang version 15.0"}
!3 = !{i32 1, i32 1, i32 1}
!4 = !{!"none", !"none", !"none"}
!5 = !{!"Type*", !"int4*", !"Type*"}
!6 = !{!"uint*", !"int __attribute__((ext_vector_type(4)))*", !"uint*"}
!7 = !{!"const", !"", !""}
!8 = !{!"in", !"xy", !"out"}
!9 = !{i1 false, i1 false, i1 false}
!10 = !{i32 0, i32 0, i32 0}
!11 = !{!"", !"", !""}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
