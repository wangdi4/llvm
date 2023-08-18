; This test checks that peeling loop is generated and get_global_id call is
; replaced with InitGID.
;
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test(ptr addrspace(1) noalias %dst) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !8 !no_barrier_path !9 !opencl.stats.Vectorizer.CanVect !1 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !6 !vectorized_width !1 !scalar_kernel !10 !kernel_execution_length !11 !kernel_has_barrier !5 !kernel_has_global_sync !5 !max_wg_dimensions !1 !arg_type_null_val !19 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %ptridx = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %call
  store i32 3, ptr addrspace(1) %ptridx, align 4, !tbaa !12, !nontemporal !1
  ret void

; CHECK-LABEL: WGLoopsEntry:
; CHECK-NEXT: %init.gid.dim0 = extractvalue [7 x i64] %early_exit_call, 1
; CHECK-NEXT: %loop.size.dim0 = extractvalue [7 x i64] %early_exit_call, 2
; CHECK-NEXT: %max.gid.dim0 = add i64 %init.gid.dim0, %loop.size.dim0
; CHECK-NEXT: [[GEP:%.*]] = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %init.gid.dim0
; CHECK-NEXT: %peel.ptr2int = ptrtoint ptr addrspace(1) [[GEP]] to i64
; CHECK-NEXT: %peel.quotient = ashr i64 %peel.ptr2int, 2
; CHECK-NEXT: %peel.quotient.multiplier = mul i64 %peel.quotient, 15
; CHECK-NEXT: %peel.count.dynamic = and i64 %peel.quotient.multiplier, 15
; CHECK-NEXT: %peel.ptr2int.and.req.align = and i64 %peel.ptr2int, 3
; CHECK-NEXT: %peel.ptr2int.req.aligned = icmp eq i64 %peel.ptr2int.and.req.align, 0
; CHECK-NEXT: %peel.size = select i1 %peel.ptr2int.req.aligned, i64 %peel.count.dynamic, i64 0
; CHECK-NEXT: %peel.actual.size = call i64 @llvm.umin.i64(i64 %loop.size.dim0, i64 %peel.size)
; CHECK-NEXT: %max.peel.gid = add i64 %peel.actual.size, %init.gid.dim0
; CHECK-NEXT: %vector.scalar.size = sub i64 %loop.size.dim0, %peel.actual.size
; CHECK-NEXT: %vector.size = ashr i64 %vector.scalar.size, 4
; CHECK-NEXT: %num.vector.wi = shl i64 %vector.size, 4
; CHECK-NEXT: %max.vector.gid = add i64 %num.vector.wi, %max.peel.gid
; CHECK-NEXT: %scalar.size = sub i64 %vector.scalar.size, %num.vector.wi
; CHECK: br label %peel_if

; CHECK-LABEL: peel_if:
; CHECK-NEXT: [[PeelCMP:%[0-9]+]] = icmp ne i64 %peel.actual.size, 0
; CHECK-NEXT: br i1 [[PeelCMP]], label %peel_pre_head, label %vect_if

; CHECK-LABEL: peel_pre_head:
; CHECK-NEXT: br label %remainder_pre_entry

; CHECK-LABEL: peel_exit:
; CHECK-NEXT: br label %vect_if

; CHECK-LABEL: remainder_pre_entry:
; CHECK-NEXT: %is.peel.loop = phi i1 [ false, %remainder_if ], [ true, %peel_pre_head ]
; CHECK-NEXT: %peel.remainder.init.gid = phi i64 [ %init.gid.dim0, %peel_pre_head ], [ %max.vector.gid, %remainder_if ]
; CHECK-NEXT: %peel.remainder.max.gid = phi i64 [ %max.peel.gid, %peel_pre_head ], [ %max.gid.dim0, %remainder_if ]
; CHECK-NEXT: br label %dim_0_pre_head

; CHECK-LABEL: dim_0_pre_head:
; CHECK-NEXT: br label %scalar_kernel_entry

; CHECK-LABEL: scalar_kernel_entry:
; CHECK: br i1 %{{.*}}, label %dim_0_exit, label %scalar_kernel_entry, !llvm.loop [[LOOP:![0-9]+]]

; CHECK-LABEL: dim_0_exit:
; CHECK-NEXT: br i1 %is.peel.loop, label %peel_exit, label %ret
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

define [7 x i64] @WG.boundaries.test(ptr addrspace(1) %0) !recommended_vector_length !16 {
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
define void @_ZGVeN16u_test(ptr addrspace(1) noalias %dst) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !10 !no_barrier_path !9 !opencl.stats.Vectorizer.CanVect !1 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !6 !vectorized_width !16 !scalar_kernel !0 !kernel_execution_length !17 !kernel_has_barrier !5 !kernel_has_global_sync !5 !max_wg_dimensions !1 !recommended_vector_length !16 !vectorization_dimension !6 !can_unite_workgroups !9 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %scalar.gep = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %call
  store <16 x i32> <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>, ptr addrspace(1) %scalar.gep, align 4, !nontemporal !1, !intel.preferred_alignment !18
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

; CHECK: [[LOOP]] = distinct !{[[LOOP]], [[LOOPMD:![0-9]+]]}
; CHECK-NEXT: [[LOOPMD]] = !{!"llvm.loop.unroll.disable"}

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"int*"}
!4 = !{!""}
!5 = !{i1 false}
!6 = !{i32 0}
!7 = !{!"dst"}
!8 = !{ptr @_ZGVeN16u_test}
!9 = !{i1 true}
!10 = !{null}
!11 = !{i32 4}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{i32 16}
!17 = !{i32 5}
!18 = !{i32 64}
!19 = !{ptr addrspace(1) null}

; DEBUGIFY-COUNT-46: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY: WARNING: Missing line 22
; DEBUGIFY-NOT: WARNING
