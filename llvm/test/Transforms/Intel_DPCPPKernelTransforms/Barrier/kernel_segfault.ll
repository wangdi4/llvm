; This IR is dumped from the very beginning of BarrierPass::runOnModule()
;
;---- [xmain version: 5.0] -----------------------------------------------
; $ics mk xmain-50 xmain-50 head -git
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
; extern __constant double const params[81];
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
; RUN: opt -passes=dpcpp-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-barrier %s -S | FileCheck %s
; CHECK-NOT: !5 = !{void (double addrspace(1)*)* @__loopy_kernel_before.BarrierPass}
; CHECK:     !6 = !{void (double addrspace(1)*, [3 x i64]*)* @loopy_kernel}
;
;--------------------------------------------------------------------------

source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@params = external local_unnamed_addr addrspace(2) constant [81 x double], align 8

; Function Attrs: nounwind
define void @loopy_kernel(double addrspace(1)* noalias %out) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_name !12 !no_barrier_path !13 !kernel_execution_length !14 !kernel_has_barrier !11 !kernel_has_global_sync !11 !max_wg_dimensions !7 {
  %early_exit_call = call [7 x i64] @WG.boundaries.loopy_kernel(double addrspace(1)* %out)
  %1 = extractvalue [7 x i64] %early_exit_call, 0
  %2 = trunc i64 %1 to i1
  br i1 %2, label %WGLoopsEntry, label %37

WGLoopsEntry:                                     ; preds = %0
  %3 = extractvalue [7 x i64] %early_exit_call, 1
  %4 = extractvalue [7 x i64] %early_exit_call, 2
  %gid_ref = call i64 @get_base_global_id.(i32 0)
  %dim_0_sub_lid = sub nuw nsw i64 %3, %gid_ref
  br label %dim_0_pre_head

dim_0_pre_head:                                   ; preds = %WGLoopsEntry
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %for.cond.cleanup, %dim_0_pre_head
  %dim_0_ind_var = phi i64 [ 0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %for.cond.cleanup ]
  %dim_0_tid = phi i64 [ %dim_0_sub_lid, %dim_0_pre_head ], [ %dim_0_inc_tid, %for.cond.cleanup ]
  %conv = trunc i64 %dim_0_tid to i32
  %add = sub i32 20, %conv
  %mul3 = mul nsw i32 %conv, 3
  %div = sdiv i32 %mul3, 4
  %add4 = add nsw i32 %add, %div
  %5 = add nsw i32 %div, 21
  %6 = sub i32 %5, %conv
  %xtraiter = and i32 %6, 7
  %7 = icmp ult i32 %add4, 7
  br i1 %7, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body.lr.ph.new

for.body.lr.ph.new:                               ; preds = %scalar_kernel_entry
  %unroll_iter = sub i32 %6, %xtraiter
  br label %for.body

for.cond.for.cond.cleanup_crit_edge.unr-lcssa.loopexit: ; preds = %for.body
  br label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa

for.cond.for.cond.cleanup_crit_edge.unr-lcssa:    ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa.loopexit, %scalar_kernel_entry
  %indvars.iv.unr = phi i64 [ 0, %scalar_kernel_entry ], [ %indvars.iv.next.7, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa.loopexit ]
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod, label %for.cond.cleanup, label %for.body.epil.preheader

for.body.epil.preheader:                          ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa
  br label %for.body.epil

for.body.epil:                                    ; preds = %for.body.epil, %for.body.epil.preheader
  %indvars.iv.epil = phi i64 [ %indvars.iv.next.epil, %for.body.epil ], [ %indvars.iv.unr, %for.body.epil.preheader ]
  %epil.iter = phi i32 [ %epil.iter.sub, %for.body.epil ], [ %xtraiter, %for.body.epil.preheader ]
  %8 = trunc i64 %indvars.iv.epil to i32
  %mul6.epil = shl i32 %8, 2
  %add9.epil = add nsw i32 %mul6.epil, %conv
  %idxprom.epil = sext i32 %add9.epil to i64
  %arrayidx.epil = getelementptr inbounds [81 x double], [81 x double] addrspace(2)* @params, i64 0, i64 %idxprom.epil
  %9 = bitcast double addrspace(2)* %arrayidx.epil to i64 addrspace(2)*
  %10 = load i64, i64 addrspace(2)* %9, align 8, !tbaa !15
  %arrayidx15.epil = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom.epil
  %11 = bitcast double addrspace(1)* %arrayidx15.epil to i64 addrspace(1)*
  store i64 %10, i64 addrspace(1)* %11, align 8, !tbaa !15
  %indvars.iv.next.epil = add nuw nsw i64 %indvars.iv.epil, 1
  %epil.iter.sub = add i32 %epil.iter, -1
  %epil.iter.cmp = icmp eq i32 %epil.iter.sub, 0
  br i1 %epil.iter.cmp, label %for.cond.cleanup.loopexit, label %for.body.epil, !llvm.loop !19

for.cond.cleanup.loopexit:                        ; preds = %for.body.epil
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %4
  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %for.cond.cleanup
  br label %37

for.body:                                         ; preds = %for.body, %for.body.lr.ph.new
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph.new ], [ %indvars.iv.next.7, %for.body ]
  %niter = phi i32 [ %unroll_iter, %for.body.lr.ph.new ], [ %niter.nsub.7, %for.body ]
  %12 = trunc i64 %indvars.iv to i32
  %mul6 = shl i32 %12, 2
  %add9 = add nsw i32 %mul6, %conv
  %idxprom = sext i32 %add9 to i64
  %arrayidx = getelementptr inbounds [81 x double], [81 x double] addrspace(2)* @params, i64 0, i64 %idxprom
  %13 = bitcast double addrspace(2)* %arrayidx to i64 addrspace(2)*
  %14 = load i64, i64 addrspace(2)* %13, align 8, !tbaa !15
  %arrayidx15 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom
  %15 = bitcast double addrspace(1)* %arrayidx15 to i64 addrspace(1)*
  store i64 %14, i64 addrspace(1)* %15, align 8, !tbaa !15
  %mul6.1 = or i32 %mul6, 4
  %add9.1 = add nsw i32 %mul6.1, %conv
  %idxprom.1 = sext i32 %add9.1 to i64
  %arrayidx.1 = getelementptr inbounds [81 x double], [81 x double] addrspace(2)* @params, i64 0, i64 %idxprom.1
  %16 = bitcast double addrspace(2)* %arrayidx.1 to i64 addrspace(2)*
  %17 = load i64, i64 addrspace(2)* %16, align 8, !tbaa !15
  %arrayidx15.1 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom.1
  %18 = bitcast double addrspace(1)* %arrayidx15.1 to i64 addrspace(1)*
  store i64 %17, i64 addrspace(1)* %18, align 8, !tbaa !15
  %mul6.2 = or i32 %mul6, 8
  %add9.2 = add nsw i32 %mul6.2, %conv
  %idxprom.2 = sext i32 %add9.2 to i64
  %arrayidx.2 = getelementptr inbounds [81 x double], [81 x double] addrspace(2)* @params, i64 0, i64 %idxprom.2
  %19 = bitcast double addrspace(2)* %arrayidx.2 to i64 addrspace(2)*
  %20 = load i64, i64 addrspace(2)* %19, align 8, !tbaa !15
  %arrayidx15.2 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom.2
  %21 = bitcast double addrspace(1)* %arrayidx15.2 to i64 addrspace(1)*
  store i64 %20, i64 addrspace(1)* %21, align 8, !tbaa !15
  %mul6.3 = or i32 %mul6, 12
  %add9.3 = add nsw i32 %mul6.3, %conv
  %idxprom.3 = sext i32 %add9.3 to i64
  %arrayidx.3 = getelementptr inbounds [81 x double], [81 x double] addrspace(2)* @params, i64 0, i64 %idxprom.3
  %22 = bitcast double addrspace(2)* %arrayidx.3 to i64 addrspace(2)*
  %23 = load i64, i64 addrspace(2)* %22, align 8, !tbaa !15
  %arrayidx15.3 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom.3
  %24 = bitcast double addrspace(1)* %arrayidx15.3 to i64 addrspace(1)*
  store i64 %23, i64 addrspace(1)* %24, align 8, !tbaa !15
  %mul6.4 = or i32 %mul6, 16
  %add9.4 = add nsw i32 %mul6.4, %conv
  %idxprom.4 = sext i32 %add9.4 to i64
  %arrayidx.4 = getelementptr inbounds [81 x double], [81 x double] addrspace(2)* @params, i64 0, i64 %idxprom.4
  %25 = bitcast double addrspace(2)* %arrayidx.4 to i64 addrspace(2)*
  %26 = load i64, i64 addrspace(2)* %25, align 8, !tbaa !15
  %arrayidx15.4 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom.4
  %27 = bitcast double addrspace(1)* %arrayidx15.4 to i64 addrspace(1)*
  store i64 %26, i64 addrspace(1)* %27, align 8, !tbaa !15
  %mul6.5 = or i32 %mul6, 20
  %add9.5 = add nsw i32 %mul6.5, %conv
  %idxprom.5 = sext i32 %add9.5 to i64
  %arrayidx.5 = getelementptr inbounds [81 x double], [81 x double] addrspace(2)* @params, i64 0, i64 %idxprom.5
  %28 = bitcast double addrspace(2)* %arrayidx.5 to i64 addrspace(2)*
  %29 = load i64, i64 addrspace(2)* %28, align 8, !tbaa !15
  %arrayidx15.5 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom.5
  %30 = bitcast double addrspace(1)* %arrayidx15.5 to i64 addrspace(1)*
  store i64 %29, i64 addrspace(1)* %30, align 8, !tbaa !15
  %mul6.6 = or i32 %mul6, 24
  %add9.6 = add nsw i32 %mul6.6, %conv
  %idxprom.6 = sext i32 %add9.6 to i64
  %arrayidx.6 = getelementptr inbounds [81 x double], [81 x double] addrspace(2)* @params, i64 0, i64 %idxprom.6
  %31 = bitcast double addrspace(2)* %arrayidx.6 to i64 addrspace(2)*
  %32 = load i64, i64 addrspace(2)* %31, align 8, !tbaa !15
  %arrayidx15.6 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom.6
  %33 = bitcast double addrspace(1)* %arrayidx15.6 to i64 addrspace(1)*
  store i64 %32, i64 addrspace(1)* %33, align 8, !tbaa !15
  %mul6.7 = or i32 %mul6, 28
  %add9.7 = add nsw i32 %mul6.7, %conv
  %idxprom.7 = sext i32 %add9.7 to i64
  %arrayidx.7 = getelementptr inbounds [81 x double], [81 x double] addrspace(2)* @params, i64 0, i64 %idxprom.7
  %34 = bitcast double addrspace(2)* %arrayidx.7 to i64 addrspace(2)*
  %35 = load i64, i64 addrspace(2)* %34, align 8, !tbaa !15
  %arrayidx15.7 = getelementptr inbounds double, double addrspace(1)* %out, i64 %idxprom.7
  %36 = bitcast double addrspace(1)* %arrayidx15.7 to i64 addrspace(1)*
  store i64 %35, i64 addrspace(1)* %36, align 8, !tbaa !15
  %indvars.iv.next.7 = add nsw i64 %indvars.iv, 8
  %niter.nsub.7 = add i32 %niter, -8
  %niter.ncmp.7 = icmp eq i32 %niter.nsub.7, 0
  br i1 %niter.ncmp.7, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa.loopexit, label %for.body

; <label>:37:                                     ; preds = %0, %dim_0_exit
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z12get_local_idj(i32) local_unnamed_addr #1

define [7 x i64] @WG.boundaries.loopy_kernel(double addrspace(1)*) {
entry:
  %1 = call i64 @_Z14get_local_sizej(i32 0)
  %2 = call i64 @get_base_global_id.(i32 0)
  %3 = call i64 @_Z14get_local_sizej(i32 1)
  %4 = call i64 @get_base_global_id.(i32 1)
  %5 = call i64 @_Z14get_local_sizej(i32 2)
  %6 = call i64 @get_base_global_id.(i32 2)
  %7 = tail call i64 @_Z12get_local_idj(i32 0) #2
  %8 = trunc i64 %7 to i32
  %9 = mul nsw i32 %8, 3
  %10 = icmp sgt i32 %9, -4
  %11 = xor i1 %10, true
  %12 = sdiv i32 %9, -4
  %13 = add i32 %12, -1
  %14 = icmp sgt i32 %13, %12
  %15 = select i1 %14, i32 %12, i32 %13
  %16 = select i1 %10, i32 -1, i32 %15
  %17 = icmp slt i32 %16, 0
  %18 = and i1 %17, %11
  %19 = select i1 %18, i32 2147483647, i32 %16
  %20 = icmp slt i32 %19, -19
  %21 = xor i1 %20, true
  %22 = add i32 %19, 20
  %23 = add i32 %19, 19
  %24 = icmp sgt i32 %23, %22
  %25 = select i1 %24, i32 %22, i32 %23
  %26 = select i1 %20, i32 -1, i32 %25
  %27 = icmp slt i32 %26, 0
  %28 = and i1 %27, %21
  %29 = sext i32 %26 to i64
  %30 = select i1 %28, i64 2147483647, i64 %29
  %31 = select i1 %20, i64 2147483647, i64 -2
  %32 = add nsw i64 %30, 1
  %33 = add i64 %32, %2
  %34 = icmp sgt i64 %30, %33
  %35 = select i1 %34, i64 %30, i64 %33
  %36 = add i64 %1, %2
  %37 = icmp sgt i64 %2, %35
  %38 = select i1 %37, i64 %2, i64 %35
  %39 = add nsw i64 %31, 1
  %40 = add i64 %39, %2
  %41 = icmp sgt i64 %31, %40
  %42 = select i1 %41, i64 %31, i64 %40
  %43 = icmp slt i64 %36, %42
  %44 = icmp slt i64 %42, 0
  %45 = or i1 %44, %43
  %46 = select i1 %45, i64 %36, i64 %42
  %47 = sub i64 %46, %38
  %48 = icmp sgt i64 %47, 0
  %zext_cast = zext i1 %48 to i64
  %49 = insertvalue [7 x i64] undef, i64 %47, 2
  %50 = insertvalue [7 x i64] %49, i64 %38, 1
  %51 = insertvalue [7 x i64] %50, i64 %3, 4
  %52 = insertvalue [7 x i64] %51, i64 %4, 3
  %53 = insertvalue [7 x i64] %52, i64 %5, 6
  %54 = insertvalue [7 x i64] %53, i64 %6, 5
  %55 = insertvalue [7 x i64] %54, i64 %zext_cast, 0
  ret [7 x i64] %55
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}
!opencl.global_variable_total_size = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"clang version 5.0.1 (cfe/trunk)"}
!5 = !{void (double addrspace(1)*)* @loopy_kernel}
!6 = !{i32 0}
!7 = !{i32 1}
!8 = !{!"none"}
!9 = !{!"double*"}
!10 = !{!"restrict"}
!11 = !{i1 false}
!12 = !{!"out"}
!13 = !{i1 true}
!14 = !{i32 970}
!15 = !{!16, !16, i64 0}
!16 = !{!"double", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}
!19 = distinct !{!19, !20}
!20 = !{!"llvm.loop.unroll.disable"}

; DEBUGIFY-NOT: WARNING
