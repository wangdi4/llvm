; ModuleID = 'sub_group_scan_exclusive_min_uint.ll'
source_filename = "sub_group_scan_exclusive_min_uint.ll"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @testKernel(i32 addrspace(1)* noundef %input, i32 addrspace(1)* noundef %results) #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !6 !kernel_arg_name !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #4
  %call1 = call spir_func i32 @_Z22get_sub_group_local_idv() #5
  %idxprom = zext i32 %call1 to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %input, i64 %idxprom
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !10
  %call2 = call spir_func i32 @_Z28sub_group_scan_exclusive_minj(i32 noundef %0) #5
  %1 = load i32, i32 addrspace(1)* %input, align 4, !tbaa !10
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
  %min_val.0 = phi i32 [ %1, %if.then5 ], [ %min_val.1, %for.inc ]
  %i.0 = phi i32 [ 1, %if.then5 ], [ %inc, %for.inc ]
  %cmp6 = icmp ult i32 %i.0, %call1
  br i1 %cmp6, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom7 = zext i32 %i.0 to i64
  %arrayidx8 = getelementptr inbounds i32, i32 addrspace(1)* %input, i64 %idxprom7
  %2 = load i32, i32 addrspace(1)* %arrayidx8, align 4, !tbaa !10
  %cmp9 = icmp ugt i32 %min_val.0, %2
  br i1 %cmp9, label %if.then10, label %if.end

if.then10:                                        ; preds = %for.body
  %idxprom11 = zext i32 %i.0 to i64
  %arrayidx12 = getelementptr inbounds i32, i32 addrspace(1)* %input, i64 %idxprom11
  %3 = load i32, i32 addrspace(1)* %arrayidx12, align 4, !tbaa !10
  br label %if.end

if.end:                                           ; preds = %if.then10, %for.body
  %min_val.1 = phi i32 [ %3, %if.then10 ], [ %min_val.0, %for.body ]
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  br label %if.end13

if.end13:                                         ; preds = %for.end, %if.else
  %min_val.2 = phi i32 [ %min_val.0, %for.end ], [ %1, %if.else ]
  br label %if.end14

if.end14:                                         ; preds = %if.end13, %if.then
  %min_val.3 = phi i32 [ -1, %if.then ], [ %min_val.2, %if.end13 ]
  %cmp15 = icmp eq i32 %min_val.3, %call2
  %cond = zext i1 %cmp15 to i32
  %arrayidx16 = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %call
  store i32 %cond, i32 addrspace(1)* %arrayidx16, align 4, !tbaa !10
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: convergent nounwind readnone willreturn
declare spir_func i64 @_Z13get_global_idj(i32 noundef) #2

; Function Attrs: convergent
declare spir_func i32 @_Z22get_sub_group_local_idv() #3

; Function Attrs: convergent
declare spir_func i32 @_Z28sub_group_scan_exclusive_minj(i32 noundef) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

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
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"clang version 15.0"}
!3 = !{i32 1, i32 1}
!4 = !{!"none", !"none"}
!5 = !{!"uint*", !"uint*"}
!6 = !{!"", !""}
!7 = !{!"input", !"results"}
!8 = !{i1 false, i1 false}
!9 = !{i32 0, i32 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
