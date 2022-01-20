; RUN: %oclopt -ocl-postvect %s -S | FileCheck %s
; RUN: %oclopt -ocl-postvect %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-NOT: recommended_vector_length

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !no_barrier_path !9 !kernel_has_sub_groups !7 !vectorized_kernel !10 !vectorized_width !2 !recommended_vector_length !11 !scalar_kernel !12 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  store i32 0, i32 addrspace(1)* %arrayidx, align 4, !tbaa !13
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent norecurse nounwind
define dso_local void @_ZGVeN16u_test(i32 addrspace(1)* noalias %dst) local_unnamed_addr #2 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !no_barrier_path !9 !kernel_has_sub_groups !7 !vectorized_kernel !12 !vectorized_width !11 !recommended_vector_length !11 !scalar_kernel !1 !vectorization_dimension !8 !can_unite_workgroups !9 {
entry:
  %alloca.dst = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %alloca.dst, align 8
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %0 = trunc i64 %call to i32
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.dst = load i32 addrspace(1)*, i32 addrspace(1)** %alloca.dst, align 8
  br label %VPlannedBB

VPlannedBB:                                       ; preds = %simd.loop.preheader
  %broadcast.splatinsert = insertelement <16 x i32> poison, i32 %0, i32 0
  %broadcast.splat = shufflevector <16 x i32> %broadcast.splatinsert, <16 x i32> poison, <16 x i32> zeroinitializer
  br label %VPlannedBB1

VPlannedBB1:                                      ; preds = %VPlannedBB
  br label %vector.body

vector.body:                                      ; preds = %VPlannedBB3, %VPlannedBB1
  %uni.phi = phi i32 [ 0, %VPlannedBB1 ], [ %5, %VPlannedBB3 ]
  %vec.phi = phi <16 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, %VPlannedBB1 ], [ %4, %VPlannedBB3 ]
  %1 = add nuw <16 x i32> %broadcast.splat, %vec.phi
  %2 = sext <16 x i32> %1 to <16 x i64>
  %.extract.0. = extractelement <16 x i64> %2, i32 0
  %scalar.gep = getelementptr inbounds i32, i32 addrspace(1)* %load.dst, i64 %.extract.0.
  %3 = bitcast i32 addrspace(1)* %scalar.gep to <16 x i32> addrspace(1)*
  store <16 x i32> zeroinitializer, <16 x i32> addrspace(1)* %3, align 4
  br label %VPlannedBB3

VPlannedBB3:                                      ; preds = %vector.body
  %4 = add nuw <16 x i32> %vec.phi, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %5 = add nuw i32 %uni.phi, 16
  %6 = icmp ult i32 %5, 16
  br i1 false, label %vector.body, label %VPlannedBB4, !llvm.loop !17

VPlannedBB4:                                      ; preds = %VPlannedBB3
  br label %VPlannedBB5

VPlannedBB5:                                      ; preds = %VPlannedBB4
  br label %final.merge

final.merge:                                      ; preds = %VPlannedBB5
  %uni.phi6 = phi i32 [ 16, %VPlannedBB5 ]
  br label %simd.end.region

simd.loop:                                        ; preds = %simd.loop.exit
  %index = phi i32 [ %indvar, %simd.loop.exit ]
  %add = add nuw i32 %0, %index
  %7 = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %load.dst, i64 %7
  store i32 0, i32 addrspace(1)* %arrayidx, align 4, !tbaa !13
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br label %simd.loop

simd.end.region:                                  ; preds = %final.merge
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "vector-variants"="_ZGVeN16u_test" }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent norecurse nounwind "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "vector-variants"="_ZGVeN16u_test" }
attributes #3 = { convergent nounwind readnone willreturn }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 2, i32 0}
!1 = !{void (i32 addrspace(1)*)* @test}
!2 = !{i32 1}
!3 = !{!"none"}
!4 = !{!"int*"}
!5 = !{!""}
!6 = !{!"dst"}
!7 = !{i1 false}
!8 = !{i32 0}
!9 = !{i1 true}
!10 = !{void (i32 addrspace(1)*)* @_ZGVeN16u_test}
!11 = !{i32 16}
!12 = !{null}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = distinct !{!17, !18}
!18 = !{!"llvm.loop.isvectorized", i32 1}

; DEBUGIFY-NOT: WARNING
