; RUN: opt -S -vplan-enable-all-zero-bypass-non-loops=false -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -vplan-vec %s | FileCheck %s
; RUN: opt -S -vplan-enable-all-zero-bypass-non-loops=false -passes="mem2reg,loop-simplify,lcssa,vpo-cfg-restructuring,vplan-vec" %s | FileCheck %s
; CHECK-LABEL: vector.body
; ModuleID = 'main'
source_filename = "5"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @pragma_unroll(i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !12 !vectorized_kernel !13 !no_barrier_path !14 !vectorized_width !7 !scalarized_kernel !15 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %0 = mul i64 %call, 100
  %mul = add i64 %0, 100
  %cmp7 = icmp eq i64 %mul, 0
  br i1 %cmp7, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %1 = add i64 %0, 99
  %xtraiter = and i64 %mul, 4
  %2 = icmp ult i64 %1, 7
  br i1 %2, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body.lr.ph.new

for.body.lr.ph.new:                               ; preds = %for.body.lr.ph
  %unroll_iter = sub i64 %mul, %xtraiter
  br label %for.body

for.cond.for.cond.cleanup_crit_edge.unr-lcssa:    ; preds = %for.body, %for.body.lr.ph
  %i.08.unr = phi i64 [ 0, %for.body.lr.ph ], [ %inc.7, %for.body ]
  %lcmp.mod = icmp eq i64 %xtraiter, 0
  br i1 %lcmp.mod, label %for.cond.cleanup, label %for.body.epil

for.body.epil:                                    ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, %for.body.epil
  %i.08.epil = phi i64 [ %inc.epil, %for.body.epil ], [ %i.08.unr, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ]
  %epil.iter = phi i64 [ %epil.iter.sub, %for.body.epil ], [ %xtraiter, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ]
  %conv.epil = trunc i64 %i.08.epil to i32
  %arrayidx.epil = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %i.08.epil
  store i32 %conv.epil, i32 addrspace(1)* %arrayidx.epil, align 4, !tbaa !16
  %inc.epil = add nuw i64 %i.08.epil, 1
  %epil.iter.sub = add i64 %epil.iter, -1
  %epil.iter.cmp = icmp eq i64 %epil.iter.sub, 0
  br i1 %epil.iter.cmp, label %for.cond.cleanup, label %for.body.epil, !llvm.loop !20

for.cond.cleanup:                                 ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, %for.body.epil, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph.new
  %i.08 = phi i64 [ 0, %for.body.lr.ph.new ], [ %inc.7, %for.body ]
  %niter = phi i64 [ %unroll_iter, %for.body.lr.ph.new ], [ %niter.nsub.7, %for.body ]
  %conv = trunc i64 %i.08 to i32
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %i.08
  store i32 %conv, i32 addrspace(1)* %arrayidx, align 4, !tbaa !16
  %inc = or i64 %i.08, 1
  %conv.1 = trunc i64 %inc to i32
  %arrayidx.1 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc
  store i32 %conv.1, i32 addrspace(1)* %arrayidx.1, align 4, !tbaa !16
  %inc.1 = or i64 %i.08, 2
  %conv.2 = trunc i64 %inc.1 to i32
  %arrayidx.2 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.1
  store i32 %conv.2, i32 addrspace(1)* %arrayidx.2, align 4, !tbaa !16
  %inc.2 = or i64 %i.08, 3
  %conv.3 = trunc i64 %inc.2 to i32
  %arrayidx.3 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.2
  store i32 %conv.3, i32 addrspace(1)* %arrayidx.3, align 4, !tbaa !16
  %inc.3 = or i64 %i.08, 4
  %conv.4 = trunc i64 %inc.3 to i32
  %arrayidx.4 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.3
  store i32 %conv.4, i32 addrspace(1)* %arrayidx.4, align 4, !tbaa !16
  %inc.4 = or i64 %i.08, 5
  %conv.5 = trunc i64 %inc.4 to i32
  %arrayidx.5 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.4
  store i32 %conv.5, i32 addrspace(1)* %arrayidx.5, align 4, !tbaa !16
  %inc.5 = or i64 %i.08, 6
  %conv.6 = trunc i64 %inc.5 to i32
  %arrayidx.6 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.5
  store i32 %conv.6, i32 addrspace(1)* %arrayidx.6, align 4, !tbaa !16
  %inc.6 = or i64 %i.08, 7
  %conv.7 = trunc i64 %inc.6 to i32
  %arrayidx.7 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.6
  store i32 %conv.7, i32 addrspace(1)* %arrayidx.7, align 4, !tbaa !16
  %inc.7 = add i64 %i.08, 8
  %niter.nsub.7 = add i64 %niter, -8
  %niter.ncmp.7 = icmp eq i64 %niter.nsub.7, 0
  br i1 %niter.ncmp.7, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body, !llvm.loop !22
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

define [7 x i64] @WG.boundaries.pragma_unroll(i32 addrspace(1)*) !ocl_recommended_vector_length !23 {
entry:
  %1 = call i64 @_Z14get_local_sizej(i32 0)
  %2 = call i64 @get_base_global_id.(i32 0)
  %3 = call i64 @_Z14get_local_sizej(i32 1)
  %4 = call i64 @get_base_global_id.(i32 1)
  %5 = call i64 @_Z14get_local_sizej(i32 2)
  %6 = call i64 @get_base_global_id.(i32 2)
  %7 = insertvalue [7 x i64] undef, i64 %1, 2
  %8 = insertvalue [7 x i64] %7, i64 %2, 1
  %9 = insertvalue [7 x i64] %8, i64 %3, 4
  %10 = insertvalue [7 x i64] %9, i64 %4, 3
  %11 = insertvalue [7 x i64] %10, i64 %5, 6
  %12 = insertvalue [7 x i64] %11, i64 %6, 5
  %13 = insertvalue [7 x i64] %12, i64 1, 0
  ret [7 x i64] %13
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @_ZGVdN8u_pragma_unroll(i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !12 !vectorized_kernel !15 !no_barrier_path !14 !ocl_recommended_vector_length !23 !vectorized_width !23 !vectorization_dimension !6 !scalarized_kernel !5 !can_unite_workgroups !11 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(i32 addrspace(1)* %dst) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add = add nuw i64 %0, %call
  %1 = mul i64 %add, 100
  %mul = add i64 %1, 100
  %cmp7 = icmp eq i64 %mul, 0
  br i1 %cmp7, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %simd.loop
  %2 = add i64 %1, 99
  %xtraiter = and i64 %mul, 4
  %3 = icmp ult i64 %2, 7
  br i1 %3, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body.lr.ph.new

for.body.lr.ph.new:                               ; preds = %for.body.lr.ph
  %unroll_iter = sub i64 %mul, %xtraiter
  br label %for.body

for.cond.for.cond.cleanup_crit_edge.unr-lcssa:    ; preds = %for.body, %for.body.lr.ph
  %i.08.unr = phi i64 [ 0, %for.body.lr.ph ], [ %inc.7, %for.body ]
  %lcmp.mod = icmp eq i64 %xtraiter, 0
  br i1 %lcmp.mod, label %for.cond.cleanup, label %for.body.epil

for.body.epil:                                    ; preds = %for.body.epil, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa
  %i.08.epil = phi i64 [ %inc.epil, %for.body.epil ], [ %i.08.unr, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ]
  %epil.iter = phi i64 [ %epil.iter.sub, %for.body.epil ], [ %xtraiter, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ]
  %conv.epil = trunc i64 %i.08.epil to i32
  %arrayidx.epil = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %i.08.epil
  store i32 %conv.epil, i32 addrspace(1)* %arrayidx.epil, align 4, !tbaa !16
  %inc.epil = add nuw i64 %i.08.epil, 1
  %epil.iter.sub = add i64 %epil.iter, -1
  %epil.iter.cmp = icmp eq i64 %epil.iter.sub, 0
  br i1 %epil.iter.cmp, label %for.cond.cleanup, label %for.body.epil, !llvm.loop !24

for.cond.cleanup:                                 ; preds = %for.body.epil, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, %simd.loop
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %for.cond.cleanup
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !25

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"(), "DIR.QUAL.LIST.END"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph.new
  %i.08 = phi i64 [ 0, %for.body.lr.ph.new ], [ %inc.7, %for.body ]
  %niter = phi i64 [ %unroll_iter, %for.body.lr.ph.new ], [ %niter.nsub.7, %for.body ]
  %conv = trunc i64 %i.08 to i32
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %i.08
  store i32 %conv, i32 addrspace(1)* %arrayidx, align 4, !tbaa !16
  %inc = or i64 %i.08, 1
  %conv.1 = trunc i64 %inc to i32
  %arrayidx.1 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc
  store i32 %conv.1, i32 addrspace(1)* %arrayidx.1, align 4, !tbaa !16
  %inc.1 = or i64 %i.08, 2
  %conv.2 = trunc i64 %inc.1 to i32
  %arrayidx.2 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.1
  store i32 %conv.2, i32 addrspace(1)* %arrayidx.2, align 4, !tbaa !16
  %inc.2 = or i64 %i.08, 3
  %conv.3 = trunc i64 %inc.2 to i32
  %arrayidx.3 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.2
  store i32 %conv.3, i32 addrspace(1)* %arrayidx.3, align 4, !tbaa !16
  %inc.3 = or i64 %i.08, 4
  %conv.4 = trunc i64 %inc.3 to i32
  %arrayidx.4 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.3
  store i32 %conv.4, i32 addrspace(1)* %arrayidx.4, align 4, !tbaa !16
  %inc.4 = or i64 %i.08, 5
  %conv.5 = trunc i64 %inc.4 to i32
  %arrayidx.5 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.4
  store i32 %conv.5, i32 addrspace(1)* %arrayidx.5, align 4, !tbaa !16
  %inc.5 = or i64 %i.08, 6
  %conv.6 = trunc i64 %inc.5 to i32
  %arrayidx.6 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.5
  store i32 %conv.6, i32 addrspace(1)* %arrayidx.6, align 4, !tbaa !16
  %inc.6 = or i64 %i.08, 7
  %conv.7 = trunc i64 %inc.6 to i32
  %arrayidx.7 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %inc.6
  store i32 %conv.7, i32 addrspace(1)* %arrayidx.7, align 4, !tbaa !16
  %inc.7 = add i64 %i.08, 8
  %niter.nsub.7 = add i64 %niter, -8
  %niter.ncmp.7 = icmp eq i64 %niter.nsub.7, 0
  br i1 %niter.ncmp.7, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body, !llvm.loop !26
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8u_pragma_unroll" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}
!opencl.gen_addr_space_pointer_counter = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-cl-std=CL2.0"}
!4 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 73a7cd4b8b270182f03b0d325c3fd4cd6e6dbf56) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bee4537ea28bde70841c48e6a4811ac4f86f36d9)"}
!5 = !{void (i32 addrspace(1)*)* @pragma_unroll}
!6 = !{i32 0}
!7 = !{i32 1}
!8 = !{!"none"}
!9 = !{!"uint*"}
!10 = !{!""}
!11 = !{i1 false}
!12 = !{!"dst"}
!13 = !{void (i32 addrspace(1)*)* @_ZGVdN8u_pragma_unroll}
!14 = !{i1 true}
!15 = !{null}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.unroll.disable"}
!22 = distinct !{!22, !21}
!23 = !{i32 8}
!24 = distinct !{!24, !21}
!25 = distinct !{!25, !21}
!26 = distinct !{!26, !21}
