target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64_x86_64-unknown-unknown"

define dso_local spir_kernel void @test(ptr addrspace(1) noundef align 4 %a) #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #1
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %gid.0 = phi i64 [ %call, %entry ], [ %add4, %for.body ]
  %sum.0 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %call1 = call spir_func i64 @_Z15get_global_sizej(i32 noundef 0) #1
  %cmp = icmp ult i64 %gid.0, %call1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %add = add nsw i32 %sum.0, 1
  %call2 = call spir_func i64 @_Z14get_num_groupsj(i32 noundef 0) #1
  %call3 = call spir_func i64 @_Z14get_local_sizej(i32 noundef 0) #1
  %mul = mul i64 %call2, %call3
  %add4 = add i64 %gid.0, %mul
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %gid.0
  store i32 %sum.0, ptr addrspace(1) %arrayidx, align 4, !tbaa !8
  ret void
}

; Function Attrs: convergent nounwind willreturn memory(none)
declare spir_func i64 @_Z13get_global_idj(i32 noundef) #1

; Function Attrs: convergent nounwind willreturn memory(none)
declare spir_func i64 @_Z15get_global_sizej(i32 noundef) #1

; Function Attrs: convergent nounwind willreturn memory(none)
declare spir_func i64 @_Z14get_num_groupsj(i32 noundef) #1

; Function Attrs: convergent nounwind willreturn memory(none)
declare spir_func i64 @_Z14get_local_sizej(i32 noundef) #1

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent nounwind willreturn memory(none) }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}

!0 = !{i32 2, i32 0}
!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"int*"}
!4 = !{!""}
!5 = !{!"a"}
!6 = !{i1 false}
!7 = !{i32 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
