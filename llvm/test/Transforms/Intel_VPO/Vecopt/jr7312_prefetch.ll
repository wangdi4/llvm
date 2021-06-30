; RUN: opt -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -vplan-vec %s | FileCheck %s
; RUN: opt -S -passes="mem2reg,loop-simplify,lcssa,vpo-cfg-restructuring,vplan-vec" %s | FileCheck %s
; CHECK-LABEL: vector.body
; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @test_fn(i8 addrspace(1)* %src, i8 addrspace(1)* %dst, i8 addrspace(3)* noalias %localBuffer, i32 %copiesPerWorkgroup, i32 %copiesPerWorkItem) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 !kernel_arg_name !12 !vectorized_kernel !13 !no_barrier_path !14 !vectorized_width !15 !scalarized_kernel !16 {
entry:
  %conv = sext i32 %copiesPerWorkItem to i64
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  %mul = mul i64 %call, %conv
  %add.ptr = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %mul
  tail call void @_Z8prefetchPU3AS1Kcm(i8 addrspace(1)* %add.ptr, i64 %conv) #5
  %cmp10 = icmp sgt i32 %copiesPerWorkItem, 0
  br i1 %cmp10, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %0 = add nsw i64 %conv, -1
  %xtraiter = and i64 %conv, 7
  %1 = icmp ult i64 %0, 7
  br i1 %1, label %for.cond.for.end_crit_edge.unr-lcssa, label %for.body.lr.ph.new

for.body.lr.ph.new:                               ; preds = %for.body.lr.ph
  %unroll_iter = sub nsw i64 %conv, %xtraiter
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph.new
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph.new ], [ %indvars.iv.next.7, %for.body ]
  %niter = phi i64 [ %unroll_iter, %for.body.lr.ph.new ], [ %niter.nsub.7, %for.body ]
  %add = add i64 %mul, %indvars.iv
  %arrayidx = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add
  %2 = load i8, i8 addrspace(1)* %arrayidx, align 1, !tbaa !17
  %arrayidx12 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add
  store i8 %2, i8 addrspace(1)* %arrayidx12, align 1, !tbaa !17
  %indvars.iv.next = or i64 %indvars.iv, 1
  %add.1 = add i64 %mul, %indvars.iv.next
  %arrayidx.1 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.1
  %3 = load i8, i8 addrspace(1)* %arrayidx.1, align 1, !tbaa !17
  %arrayidx12.1 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.1
  store i8 %3, i8 addrspace(1)* %arrayidx12.1, align 1, !tbaa !17
  %indvars.iv.next.1 = or i64 %indvars.iv, 2
  %add.2 = add i64 %mul, %indvars.iv.next.1
  %arrayidx.2 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.2
  %4 = load i8, i8 addrspace(1)* %arrayidx.2, align 1, !tbaa !17
  %arrayidx12.2 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.2
  store i8 %4, i8 addrspace(1)* %arrayidx12.2, align 1, !tbaa !17
  %indvars.iv.next.2 = or i64 %indvars.iv, 3
  %add.3 = add i64 %mul, %indvars.iv.next.2
  %arrayidx.3 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.3
  %5 = load i8, i8 addrspace(1)* %arrayidx.3, align 1, !tbaa !17
  %arrayidx12.3 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.3
  store i8 %5, i8 addrspace(1)* %arrayidx12.3, align 1, !tbaa !17
  %indvars.iv.next.3 = or i64 %indvars.iv, 4
  %add.4 = add i64 %mul, %indvars.iv.next.3
  %arrayidx.4 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.4
  %6 = load i8, i8 addrspace(1)* %arrayidx.4, align 1, !tbaa !17
  %arrayidx12.4 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.4
  store i8 %6, i8 addrspace(1)* %arrayidx12.4, align 1, !tbaa !17
  %indvars.iv.next.4 = or i64 %indvars.iv, 5
  %add.5 = add i64 %mul, %indvars.iv.next.4
  %arrayidx.5 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.5
  %7 = load i8, i8 addrspace(1)* %arrayidx.5, align 1, !tbaa !17
  %arrayidx12.5 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.5
  store i8 %7, i8 addrspace(1)* %arrayidx12.5, align 1, !tbaa !17
  %indvars.iv.next.5 = or i64 %indvars.iv, 6
  %add.6 = add i64 %mul, %indvars.iv.next.5
  %arrayidx.6 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.6
  %8 = load i8, i8 addrspace(1)* %arrayidx.6, align 1, !tbaa !17
  %arrayidx12.6 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.6
  store i8 %8, i8 addrspace(1)* %arrayidx12.6, align 1, !tbaa !17
  %indvars.iv.next.6 = or i64 %indvars.iv, 7
  %add.7 = add i64 %mul, %indvars.iv.next.6
  %arrayidx.7 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.7
  %9 = load i8, i8 addrspace(1)* %arrayidx.7, align 1, !tbaa !17
  %arrayidx12.7 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.7
  store i8 %9, i8 addrspace(1)* %arrayidx12.7, align 1, !tbaa !17
  %indvars.iv.next.7 = add nuw nsw i64 %indvars.iv, 8
  %niter.nsub.7 = add i64 %niter, -8
  %niter.ncmp.7 = icmp eq i64 %niter.nsub.7, 0
  br i1 %niter.ncmp.7, label %for.cond.for.end_crit_edge.unr-lcssa, label %for.body

for.cond.for.end_crit_edge.unr-lcssa:             ; preds = %for.body, %for.body.lr.ph
  %indvars.iv.unr = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next.7, %for.body ]
  %lcmp.mod = icmp eq i64 %xtraiter, 0
  br i1 %lcmp.mod, label %for.end, label %for.body.epil

for.body.epil:                                    ; preds = %for.cond.for.end_crit_edge.unr-lcssa, %for.body.epil
  %indvars.iv.epil = phi i64 [ %indvars.iv.next.epil, %for.body.epil ], [ %indvars.iv.unr, %for.cond.for.end_crit_edge.unr-lcssa ]
  %epil.iter = phi i64 [ %epil.iter.sub, %for.body.epil ], [ %xtraiter, %for.cond.for.end_crit_edge.unr-lcssa ]
  %add.epil = add i64 %mul, %indvars.iv.epil
  %arrayidx.epil = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.epil
  %10 = load i8, i8 addrspace(1)* %arrayidx.epil, align 1, !tbaa !17
  %arrayidx12.epil = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.epil
  store i8 %10, i8 addrspace(1)* %arrayidx12.epil, align 1, !tbaa !17
  %indvars.iv.next.epil = add nuw nsw i64 %indvars.iv.epil, 1
  %epil.iter.sub = add i64 %epil.iter, -1
  %epil.iter.cmp = icmp eq i64 %epil.iter.sub, 0
  br i1 %epil.iter.cmp, label %for.end, label %for.body.epil, !llvm.loop !20

for.end:                                          ; preds = %for.cond.for.end_crit_edge.unr-lcssa, %for.body.epil, %entry
  ret void
}

; Function Attrs: convergent
declare void @_Z8prefetchPU3AS1Kcm(i8 addrspace(1)*, i64) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

define [7 x i64] @WG.boundaries.test_fn(i8 addrspace(1)*, i8 addrspace(1)*, i8 addrspace(3)*, i32, i32) !ocl_recommended_vector_length !22 {
entry:
  %5 = call i64 @_Z14get_local_sizej(i32 0)
  %6 = call i64 @get_base_global_id.(i32 0)
  %7 = call i64 @_Z14get_local_sizej(i32 1)
  %8 = call i64 @get_base_global_id.(i32 1)
  %9 = call i64 @_Z14get_local_sizej(i32 2)
  %10 = call i64 @get_base_global_id.(i32 2)
  %11 = insertvalue [7 x i64] undef, i64 %5, 2
  %12 = insertvalue [7 x i64] %11, i64 %6, 1
  %13 = insertvalue [7 x i64] %12, i64 %7, 4
  %14 = insertvalue [7 x i64] %13, i64 %8, 3
  %15 = insertvalue [7 x i64] %14, i64 %9, 6
  %16 = insertvalue [7 x i64] %15, i64 %10, 5
  %17 = insertvalue [7 x i64] %16, i64 1, 0
  ret [7 x i64] %17
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @_ZGVdN8uuuuu_test_fn(i8 addrspace(1)* %src, i8 addrspace(1)* %dst, i8 addrspace(3)* noalias %localBuffer, i32 %copiesPerWorkgroup, i32 %copiesPerWorkItem) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 !kernel_arg_name !12 !vectorized_kernel !16 !no_barrier_path !14 !ocl_recommended_vector_length !22 !vectorized_width !22 !vectorization_dimension !23 !scalarized_kernel !4 !can_unite_workgroups !24 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(i8 addrspace(1)* %src, i8 addrspace(1)* %dst, i8 addrspace(3)* %localBuffer, i32 %copiesPerWorkgroup, i32 %copiesPerWorkItem) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add1 = add nuw i64 %0, %call
  %conv = sext i32 %copiesPerWorkItem to i64
  %mul = mul i64 %add1, %conv
  %add.ptr = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %mul
  tail call void @_Z8prefetchPU3AS1Kcm(i8 addrspace(1)* %add.ptr, i64 %conv) #5
  %cmp10 = icmp sgt i32 %copiesPerWorkItem, 0
  br i1 %cmp10, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %simd.loop
  %1 = add nsw i64 %conv, -1
  %xtraiter = and i64 %conv, 7
  %2 = icmp ult i64 %1, 7
  br i1 %2, label %for.cond.for.end_crit_edge.unr-lcssa, label %for.body.lr.ph.new

for.body.lr.ph.new:                               ; preds = %for.body.lr.ph
  %unroll_iter = sub nsw i64 %conv, %xtraiter
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph.new
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph.new ], [ %indvars.iv.next.7, %for.body ]
  %niter = phi i64 [ %unroll_iter, %for.body.lr.ph.new ], [ %niter.nsub.7, %for.body ]
  %add = add i64 %mul, %indvars.iv
  %arrayidx = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add
  %3 = load i8, i8 addrspace(1)* %arrayidx, align 1, !tbaa !17
  %arrayidx12 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add
  store i8 %3, i8 addrspace(1)* %arrayidx12, align 1, !tbaa !17
  %indvars.iv.next = or i64 %indvars.iv, 1
  %add.1 = add i64 %mul, %indvars.iv.next
  %arrayidx.1 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.1
  %4 = load i8, i8 addrspace(1)* %arrayidx.1, align 1, !tbaa !17
  %arrayidx12.1 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.1
  store i8 %4, i8 addrspace(1)* %arrayidx12.1, align 1, !tbaa !17
  %indvars.iv.next.1 = or i64 %indvars.iv, 2
  %add.2 = add i64 %mul, %indvars.iv.next.1
  %arrayidx.2 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.2
  %5 = load i8, i8 addrspace(1)* %arrayidx.2, align 1, !tbaa !17
  %arrayidx12.2 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.2
  store i8 %5, i8 addrspace(1)* %arrayidx12.2, align 1, !tbaa !17
  %indvars.iv.next.2 = or i64 %indvars.iv, 3
  %add.3 = add i64 %mul, %indvars.iv.next.2
  %arrayidx.3 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.3
  %6 = load i8, i8 addrspace(1)* %arrayidx.3, align 1, !tbaa !17
  %arrayidx12.3 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.3
  store i8 %6, i8 addrspace(1)* %arrayidx12.3, align 1, !tbaa !17
  %indvars.iv.next.3 = or i64 %indvars.iv, 4
  %add.4 = add i64 %mul, %indvars.iv.next.3
  %arrayidx.4 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.4
  %7 = load i8, i8 addrspace(1)* %arrayidx.4, align 1, !tbaa !17
  %arrayidx12.4 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.4
  store i8 %7, i8 addrspace(1)* %arrayidx12.4, align 1, !tbaa !17
  %indvars.iv.next.4 = or i64 %indvars.iv, 5
  %add.5 = add i64 %mul, %indvars.iv.next.4
  %arrayidx.5 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.5
  %8 = load i8, i8 addrspace(1)* %arrayidx.5, align 1, !tbaa !17
  %arrayidx12.5 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.5
  store i8 %8, i8 addrspace(1)* %arrayidx12.5, align 1, !tbaa !17
  %indvars.iv.next.5 = or i64 %indvars.iv, 6
  %add.6 = add i64 %mul, %indvars.iv.next.5
  %arrayidx.6 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.6
  %9 = load i8, i8 addrspace(1)* %arrayidx.6, align 1, !tbaa !17
  %arrayidx12.6 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.6
  store i8 %9, i8 addrspace(1)* %arrayidx12.6, align 1, !tbaa !17
  %indvars.iv.next.6 = or i64 %indvars.iv, 7
  %add.7 = add i64 %mul, %indvars.iv.next.6
  %arrayidx.7 = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.7
  %10 = load i8, i8 addrspace(1)* %arrayidx.7, align 1, !tbaa !17
  %arrayidx12.7 = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.7
  store i8 %10, i8 addrspace(1)* %arrayidx12.7, align 1, !tbaa !17
  %indvars.iv.next.7 = add nuw nsw i64 %indvars.iv, 8
  %niter.nsub.7 = add i64 %niter, -8
  %niter.ncmp.7 = icmp eq i64 %niter.nsub.7, 0
  br i1 %niter.ncmp.7, label %for.cond.for.end_crit_edge.unr-lcssa, label %for.body

for.cond.for.end_crit_edge.unr-lcssa:             ; preds = %for.body, %for.body.lr.ph
  %indvars.iv.unr = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next.7, %for.body ]
  %lcmp.mod = icmp eq i64 %xtraiter, 0
  br i1 %lcmp.mod, label %for.end, label %for.body.epil

for.body.epil:                                    ; preds = %for.body.epil, %for.cond.for.end_crit_edge.unr-lcssa
  %indvars.iv.epil = phi i64 [ %indvars.iv.next.epil, %for.body.epil ], [ %indvars.iv.unr, %for.cond.for.end_crit_edge.unr-lcssa ]
  %epil.iter = phi i64 [ %epil.iter.sub, %for.body.epil ], [ %xtraiter, %for.cond.for.end_crit_edge.unr-lcssa ]
  %add.epil = add i64 %mul, %indvars.iv.epil
  %arrayidx.epil = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %add.epil
  %11 = load i8, i8 addrspace(1)* %arrayidx.epil, align 1, !tbaa !17
  %arrayidx12.epil = getelementptr inbounds i8, i8 addrspace(1)* %dst, i64 %add.epil
  store i8 %11, i8 addrspace(1)* %arrayidx12.epil, align 1, !tbaa !17
  %indvars.iv.next.epil = add nuw nsw i64 %indvars.iv.epil, 1
  %epil.iter.sub = add i64 %epil.iter, -1
  %epil.iter.cmp = icmp eq i64 %epil.iter.sub, 0
  br i1 %epil.iter.cmp, label %for.end, label %for.body.epil, !llvm.loop !25

for.end:                                          ; preds = %for.body.epil, %for.cond.for.end_crit_edge.unr-lcssa, %simd.loop
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %for.end
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !26

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"(), "DIR.QUAL.LIST.END"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8uuuuu_test_fn" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent nounwind readnone }
attributes #5 = { convergent nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.kernels = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 73a7cd4b8b270182f03b0d325c3fd4cd6e6dbf56) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bee4537ea28bde70841c48e6a4811ac4f86f36d9)"}
!4 = !{void (i8 addrspace(1)*, i8 addrspace(1)*, i8 addrspace(3)*, i32, i32)* @test_fn}
!5 = !{i32 1, i32 1, i32 3, i32 0, i32 0}
!6 = !{!"none", !"none", !"none", !"none", !"none"}
!7 = !{!"char*", !"char*", !"char*", !"int", !"int"}
!8 = !{!"const", !"", !"", !"", !""}
!9 = !{i1 false, i1 false, i1 false, i1 false, i1 false}
!10 = !{i32 0, i32 0, i32 0, i32 0, i32 0}
!11 = !{!"", !"", !"", !"", !""}
!12 = !{!"src", !"dst", !"localBuffer", !"copiesPerWorkgroup", !"copiesPerWorkItem"}
!13 = !{void (i8 addrspace(1)*, i8 addrspace(1)*, i8 addrspace(3)*, i32, i32)* @_ZGVdN8uuuuu_test_fn}
!14 = !{i1 true}
!15 = !{i32 1}
!16 = !{null}
!17 = !{!18, !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.unroll.disable"}
!22 = !{i32 8}
!23 = !{i32 0}
!24 = !{i1 false}
!25 = distinct !{!25, !21}
!26 = distinct !{!26, !21}
