; This IR is dumped before BarrierPass.
;
;----[ Command line: ] ---------------------------------------------------
; $CL_CONFIG_USE_VECTORIZER=False ./wolf_ioc64 -cmd=build -input=kernel_segfault.cl -device=cpu
;
;---- [Kernel source] -----------------------------------------------------
;
; #define lid(N) ((int) get_local_id(N))
; #define gid(N) ((int) get_group_id(N))
; #if __OPENCL_C_VERSION__ < 120
; #pragma OPENCL EXTENSION cl_khr_fp64: enable
; #endif
;
; __constant double const params[81] = {0};
;
; //__kernel void __attribute__ ((reqd_work_group_size(4, 1, 1))) loopy_kernel(__global double *restrict out)
; __kernel void loopy_kernel(__global double *restrict out)
; {
;  for (int i_outer = 0; i_outer <= 20 + -1 * lid(0) + (3 * lid(0) / 4); ++i_outer)
;    out[4 * i_outer + lid(0)] = params[4 * i_outer + lid(0)];
; }
;
;--------------------------------------------------------------------------
;
; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s
;
; CHECK: !sycl.kernels = !{[[KERNELS:![0-9]+]]}
; CHECK: [[KERNELS]] = !{ptr @loopy_kernel}
;
;--------------------------------------------------------------------------

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@params = local_unnamed_addr addrspace(2) constant [81 x double] zeroinitializer, align 8

; Function Attrs: convergent nofree norecurse nounwind memory(argmem: write)
define dso_local void @loopy_kernel(ptr addrspace(1) noalias nocapture noundef writeonly align 8 %out) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 !arg_type_null_val !12 !no_barrier_path !13 !kernel_has_sub_groups !9 !kernel_has_global_sync !9 !kernel_execution_length !14 !max_wg_dimensions !4 !recommended_vector_length !4 {
entry:
  %early_exit_call = call [7 x i64] @WG.boundaries.loopy_kernel(ptr addrspace(1) %out)
  %uniform.early.exit = extractvalue [7 x i64] %early_exit_call, 0
  %0 = trunc i64 %uniform.early.exit to i1
  br i1 %0, label %WGLoopsEntry, label %exit

WGLoopsEntry:                                     ; preds = %entry
  %init.gid.dim0 = extractvalue [7 x i64] %early_exit_call, 1
  %loop.size.dim0 = extractvalue [7 x i64] %early_exit_call, 2
  %max.gid.dim0 = add i64 %init.gid.dim0, %loop.size.dim0
  %base.gid.dim0 = call i64 @get_base_global_id.(i32 0)
  %dim_0_sub_lid = sub nuw nsw i64 %init.gid.dim0, %base.gid.dim0
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %for.cond.cleanup, %WGLoopsEntry
  %dim_0_ind_var = phi i64 [ %init.gid.dim0, %WGLoopsEntry ], [ %dim_0_inc_ind_var, %for.cond.cleanup ]
  %dim_0_tid = phi i64 [ %dim_0_sub_lid, %WGLoopsEntry ], [ %dim_0_inc_tid, %for.cond.cleanup ]
  br label %for.cond

for.cond:                                         ; preds = %for.body, %scalar_kernel_entry
  %i_outer.0 = phi i32 [ 0, %scalar_kernel_entry ], [ %inc, %for.body ]
  %conv = trunc i64 %dim_0_tid to i32
  %mul3 = mul nsw i32 %conv, 3
  %div = sdiv i32 %mul3, 4
  %reass.sub = sub i32 %div, %conv
  %add4 = add i32 %reass.sub, 20
  %cmp.not = icmp sgt i32 %i_outer.0, %add4
  br i1 %cmp.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.cond
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
  br i1 %dim_0_cmp.to.max, label %exit, label %scalar_kernel_entry

for.body:                                         ; preds = %for.cond
  %mul10 = shl nsw i32 %i_outer.0, 2
  %conv12 = trunc i64 %dim_0_tid to i32
  %add13 = add nsw i32 %mul10, %conv12
  %idxprom14 = sext i32 %add13 to i64
  %arrayidx15 = getelementptr inbounds double, ptr addrspace(1) %out, i64 %idxprom14
  store double 0.000000e+00, ptr addrspace(1) %arrayidx15, align 8, !tbaa !15
  %inc = add nuw nsw i32 %i_outer.0, 1
  br label %for.cond

exit:                                             ; preds = %for.cond.cleanup, %entry
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind willreturn memory(none)
declare i64 @_Z12get_local_idj(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent nofree norecurse nounwind memory(argmem: write)
define private [7 x i64] @WG.boundaries.loopy_kernel(ptr addrspace(1) noalias nocapture noundef writeonly align 8 %out) local_unnamed_addr #0 {
entry:
  %local.size0 = tail call i64 @_Z14get_local_sizej(i32 0) #2
  %base.gid0 = tail call i64 @get_base_global_id.(i32 0) #2
  %local.size1 = tail call i64 @_Z14get_local_sizej(i32 1) #2
  %base.gid1 = tail call i64 @get_base_global_id.(i32 1) #2
  %local.size2 = tail call i64 @_Z14get_local_sizej(i32 2) #2
  %base.gid2 = tail call i64 @get_base_global_id.(i32 2) #2
  %0 = insertvalue [7 x i64] undef, i64 %local.size0, 2
  %1 = insertvalue [7 x i64] %0, i64 %base.gid0, 1
  %2 = insertvalue [7 x i64] %1, i64 %local.size1, 4
  %3 = insertvalue [7 x i64] %2, i64 %base.gid1, 3
  %4 = insertvalue [7 x i64] %3, i64 %local.size2, 6
  %5 = insertvalue [7 x i64] %4, i64 %base.gid2, 5
  %6 = insertvalue [7 x i64] %5, i64 1, 0
  ret [7 x i64] %6
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { convergent nofree norecurse nounwind memory(argmem: write) }
attributes #1 = { convergent mustprogress nofree nounwind willreturn memory(none) }
attributes #2 = { nounwind }

!llvm.linker.options = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 3, i32 0}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.0.0.YYYYMMDD)"}
!3 = !{ptr @loopy_kernel}
!4 = !{i32 1}
!5 = !{!"none"}
!6 = !{!"double*"}
!7 = !{!"restrict"}
!8 = !{!"out"}
!9 = !{i1 false}
!10 = !{i32 0}
!11 = !{!""}
!12 = !{ptr addrspace(1) null}
!13 = !{i1 true}
!14 = !{i32 182}
!15 = !{!16, !16, i64 0}
!16 = !{!"double", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-NOT: WARNING
