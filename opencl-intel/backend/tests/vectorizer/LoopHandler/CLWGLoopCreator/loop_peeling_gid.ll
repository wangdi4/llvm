; This test checks that peeling loop is generated and get_global_id call is
; replaced with InitGID.
;
; IR is dumped at the beginning of CLWGLoopCreator::runOnModule() from source:
;
; kernel void test(global int *dst) {
;   size_t gid = get_global_id(0);
;   dst[gid] = 3;
; }
;
; RUN: %oclopt -cl-loop-creator -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test(i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !12 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !13 !vectorized_kernel !14 !no_barrier_path !15 !opencl.stats.Vectorizer.CanVect !7 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !12 !vectorized_width !7 !scalarized_kernel !16 !kernel_execution_length !17 !kernel_has_barrier !11 !kernel_has_global_sync !11 !max_wg_dimensions !7 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %ptridx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  store i32 3, i32 addrspace(1)* %ptridx, align 4, !tbaa !18, !nontemporal !7
  ret void

; CHECK-LABEL: WGLoopsEntry:
; CHECK-NEXT: [[InitGID0:%[0-9]+]] = extractvalue [7 x i64] %early_exit_call, 1
; CHECK-NEXT: [[LocalSize0:%[0-9]+]] = extractvalue [7 x i64] %early_exit_call, 2
; CHECK-NEXT: [[GEP:%[0-9]+]] = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 [[InitGID0]]
; CHECK-NEXT: [[BITCAST:%[0-9]+]] = bitcast i32 addrspace(1)* [[GEP]] to <16 x i32> addrspace(1)*
; CHECK-NEXT: %peel.ptr2int = ptrtoint <16 x i32> addrspace(1)* [[BITCAST]] to i64
; CHECK-NEXT: %peel.quotient = ashr i64 %peel.ptr2int, 2
; CHECK-NEXT: %peel.quotient.multiplier = mul i64 %peel.quotient, 15
; CHECK-NEXT: %peel.count.dynamic = and i64 %peel.quotient.multiplier, 15
; CHECK-NEXT: %peel.ptr2int.and.req.align = and i64 %peel.ptr2int, 3
; CHECK-NEXT: %peel.ptr2int.req.aligned = icmp eq i64 %peel.ptr2int.and.req.align, 0
; CHECK-NEXT: %peel.size = select i1 %peel.ptr2int.req.aligned, i64 %peel.count.dynamic, i64 0
; CHECK-NEXT: %vector.scalar.size = sub i64 [[LocalSize0]], %peel.size
; CHECK-NEXT: %vector.size = ashr i64 %vector.scalar.size, 4
; CHECK-NEXT: %num.vector.wi = shl i64 %vector.size, 4
; CHECK-NEXT: %max.peel.gid = add i64 %peel.size, [[InitGID0]]
; CHECK-NEXT: %max.vector.gid = add i64 %num.vector.wi, %max.peel.gid
; CHECK-NEXT: %scalar.size = sub i64 %vector.scalar.size, %num.vector.wi
; CHECK: br label %peel_if

; CHECK-LABEL: peel_if:
; CHECK-NEXT: [[PeelCMP:%[0-9]+]] = icmp ne i64 %peel.size, 0
; CHECK-NEXT: br i1 [[PeelCMP]], label %peel_pre_head, label %vect_if

; CHECK-LABEL: peel_pre_head:
; CHECK-NEXT: br label %remainder_pre_entry

; CHECK-LABEL: peel_exit:
; CHECK-NEXT: br label %vect_if

; CHECK-LABEL: remainder_pre_entry:
; CHECK-NEXT: %is.peel.loop = phi i1 [ false, %remainder_if ], [ true, %peel_pre_head ]
; CHECK-NEXT: %scalar.init.gid = phi i64 [ %max.vector.gid, %remainder_if ], [ [[InitGID0]], %peel_pre_head ]
; CHECK-NEXT: %scalar.loop.size = phi i64 [ %scalar.size, %remainder_if ], [ %peel.size, %peel_pre_head ]
; CHECK-NEXT: br label %dim_0_pre_head

; CHECK-LABEL: dim_0_pre_head:
; CHECK-NEXT: br label %scalar_kernel_entry

; CHECK-LABEL: dim_0_exit:
; CHECK-NEXT: br i1 %is.peel.loop, label %peel_exit, label %ret
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

define [7 x i64] @WG.boundaries.test(i32 addrspace(1)* %0) !ocl_recommended_vector_length !22 {
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

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeN16u_test(i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !12 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !13 !vectorized_kernel !16 !no_barrier_path !15 !ocl_recommended_vector_length !22 !opencl.stats.Vectorizer.CanVect !7 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !12 !vectorized_width !22 !vectorization_dimension !12 !scalarized_kernel !4 !can_unite_workgroups !15 !kernel_execution_length !23 !kernel_has_barrier !11 !kernel_has_global_sync !11 !max_wg_dimensions !7 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %scalar.gep = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  %0 = bitcast i32 addrspace(1)* %scalar.gep to <16 x i32> addrspace(1)*
  store <16 x i32> <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>, <16 x i32> addrspace(1)* %0, align 4, !nontemporal !7, !intel.preferred_alignment !24
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVeN16u_test" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.kernels = !{!4}
!opencl.stats.Vectorizer.CanVect = !{!5}
!opencl.stats.Vectorizer.Chosen_Vectorization_Dim = !{!6}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{void (i32 addrspace(1)*)* @test}
!5 = !{!"Code is vectorizable"}
!6 = !{!"The chosen vectorization dimension"}
!7 = !{i32 1}
!8 = !{!"none"}
!9 = !{!"int*"}
!10 = !{!""}
!11 = !{i1 false}
!12 = !{i32 0}
!13 = !{!"dst"}
!14 = !{void (i32 addrspace(1)*)* @_ZGVeN16u_test}
!15 = !{i1 true}
!16 = !{null}
!17 = !{i32 4}
!18 = !{!19, !19, i64 0}
!19 = !{!"int", !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C/C++ TBAA"}
!22 = !{i32 16}
!23 = !{i32 5}
!24 = !{i32 64}
