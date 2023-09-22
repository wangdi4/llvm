; ModuleID = 'sub_group_scan_exclusive_max_uint.ll'
source_filename = "sub_group_scan_exclusive_max_uint.ll"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @testKernel(ptr addrspace(1) noundef %input, ptr addrspace(1) noundef %results) #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #4
  %call1 = call spir_func i32 @_Z22get_sub_group_local_idv() #5
  %idxprom = zext i32 %call1 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %input, i64 %idxprom
  %0 = load i32, ptr addrspace(1) %arrayidx, align 4, !tbaa !9
  %call2 = call spir_func i32 @_Z28sub_group_scan_exclusive_maxj(i32 noundef %0) #5
  %1 = load i32, ptr addrspace(1) %input, align 4, !tbaa !9
  %cmp = icmp eq i32 %call1, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end14

if.else:                                          ; preds = %entry
  %cmp4.not = icmp eq i32 %call1, 1
  br i1 %cmp4.not, label %if.end13, label %if.then5

if.then5:                                         ; preds = %if.else
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.then5
  %max_val.0 = phi i32 [ %1, %if.then5 ], [ %max_val.1, %for.inc ]
  %i.0 = phi i32 [ 1, %if.then5 ], [ %inc, %for.inc ]
  %cmp6 = icmp ult i32 %i.0, %call1
  br i1 %cmp6, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom7 = zext i32 %i.0 to i64
  %arrayidx8 = getelementptr inbounds i32, ptr addrspace(1) %input, i64 %idxprom7
  %2 = load i32, ptr addrspace(1) %arrayidx8, align 4, !tbaa !9
  %cmp9 = icmp ult i32 %max_val.0, %2
  br i1 %cmp9, label %if.then10, label %if.end

if.then10:                                        ; preds = %for.body
  %idxprom11 = zext i32 %i.0 to i64
  %arrayidx12 = getelementptr inbounds i32, ptr addrspace(1) %input, i64 %idxprom11
  %3 = load i32, ptr addrspace(1) %arrayidx12, align 4, !tbaa !9
  br label %if.end

if.end:                                           ; preds = %if.then10, %for.body
  %max_val.1 = phi i32 [ %3, %if.then10 ], [ %max_val.0, %for.body ]
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  br label %if.end13

if.end13:                                         ; preds = %for.end, %if.else
  %max_val.2 = phi i32 [ %max_val.0, %for.end ], [ %1, %if.else ]
  br label %if.end14

if.end14:                                         ; preds = %if.end13, %if.then
  %max_val.3 = phi i32 [ 0, %if.then ], [ %max_val.2, %if.end13 ]
  %cmp15 = icmp eq i32 %max_val.3, %call2
  %cond = zext i1 %cmp15 to i32
  %arrayidx16 = getelementptr inbounds i32, ptr addrspace(1) %results, i64 %call
  store i32 %cond, ptr addrspace(1) %arrayidx16, align 4, !tbaa !9
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent nounwind readnone willreturn
declare spir_func i64 @_Z13get_global_idj(i32 noundef) #2

; Function Attrs: convergent
declare spir_func i32 @_Z22get_sub_group_local_idv() #3

; Function Attrs: convergent
declare spir_func i32 @_Z28sub_group_scan_exclusive_maxj(i32 noundef) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, ptr nocapture) #1

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #4 = { convergent nounwind readnone willreturn }
attributes #5 = { convergent nounwind }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!1}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{i32 1, i32 1}
!3 = !{!"none", !"none"}
!4 = !{!"uint*", !"uint*"}
!5 = !{!"", !""}
!6 = !{!"input", !"results"}
!7 = !{i1 false, i1 false}
!8 = !{i32 0, i32 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
