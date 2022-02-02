; RUN: opt -passes=dpcpp-kernel-postvec %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-postvec %s -S | FileCheck %s
; RUN: opt -dpcpp-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that recommended_vector_length metadata is removed from both scalar
; and vectorized kernel.

; CHECK-NOT: recommended_vector_length

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(i32 addrspace(1)* %src) local_unnamed_addr #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !no_barrier_path !9 !kernel_has_sub_groups !9 !vectorized_kernel !10 !vectorized_masked_kernel !11 !vectorized_width !2 !recommended_vector_length !12 !scalar_kernel !13 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !8 {
entry:
  %call = tail call i32 @_Z18get_sub_group_sizev() #5
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call1
  store i32 %call, i32 addrspace(1)* %arrayidx, align 4, !tbaa !14
  ret void
}

; Function Attrs: convergent
declare i32 @_Z18get_sub_group_sizev() local_unnamed_addr #1

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: convergent norecurse nounwind
define dso_local void @_ZGVeN16u_test(i32 addrspace(1)* %src) local_unnamed_addr #3 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !no_barrier_path !9 !kernel_has_sub_groups !9 !vectorized_kernel !13 !vectorized_width !12 !recommended_vector_length !12 !scalar_kernel !1 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !8 !vectorization_dimension !8 !can_unite_workgroups !7 {
entry:
  %alloca.src = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %src, i32 addrspace(1)** %alloca.src, align 8
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6
  %0 = trunc i64 %call1 to i32
  %call = tail call i32 @_Z18get_sub_group_sizev() #5
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.src = load i32 addrspace(1)*, i32 addrspace(1)** %alloca.src, align 8
  br label %VPlannedBB

VPlannedBB:                                       ; preds = %simd.loop.preheader
  %broadcast.splatinsert = insertelement <16 x i32> poison, i32 %0, i32 0
  %broadcast.splat = shufflevector <16 x i32> %broadcast.splatinsert, <16 x i32> poison, <16 x i32> zeroinitializer
  %broadcast.splatinsert3 = insertelement <16 x i32> poison, i32 %call, i32 0
  %broadcast.splat4 = shufflevector <16 x i32> %broadcast.splatinsert3, <16 x i32> poison, <16 x i32> zeroinitializer
  br label %VPlannedBB1

VPlannedBB1:                                      ; preds = %VPlannedBB
  br label %vector.body

vector.body:                                      ; preds = %VPlannedBB5, %VPlannedBB1
  %uni.phi = phi i32 [ 0, %VPlannedBB1 ], [ %5, %VPlannedBB5 ]
  %vec.phi = phi <16 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, %VPlannedBB1 ], [ %4, %VPlannedBB5 ]
  %1 = add nuw <16 x i32> %broadcast.splat, %vec.phi
  %2 = sext <16 x i32> %1 to <16 x i64>
  %.extract.0. = extractelement <16 x i64> %2, i32 0
  %scalar.gep = getelementptr inbounds i32, i32 addrspace(1)* %load.src, i64 %.extract.0.
  %3 = bitcast i32 addrspace(1)* %scalar.gep to <16 x i32> addrspace(1)*
  store <16 x i32> %broadcast.splat4, <16 x i32> addrspace(1)* %3, align 4
  br label %VPlannedBB5

VPlannedBB5:                                      ; preds = %vector.body
  %4 = add nuw <16 x i32> %vec.phi, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %5 = add nuw i32 %uni.phi, 16
  %6 = icmp ult i32 %5, 16
  br i1 false, label %vector.body, label %VPlannedBB6, !llvm.loop !18

VPlannedBB6:                                      ; preds = %VPlannedBB5
  br label %VPlannedBB7

VPlannedBB7:                                      ; preds = %VPlannedBB6
  br label %final.merge

final.merge:                                      ; preds = %VPlannedBB7
  %uni.phi8 = phi i32 [ 16, %VPlannedBB7 ]
  br label %simd.end.region

simd.loop:                                        ; preds = %simd.loop.exit
  %index = phi i32 [ %indvar, %simd.loop.exit ]
  %add = add nuw i32 %0, %index
  %7 = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %load.src, i64 %7
  store i32 %call, i32 addrspace(1)* %arrayidx, align 4, !tbaa !14
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

; Function Attrs: convergent norecurse nounwind
define dso_local void @_ZGVeM16u_test(i32 addrspace(1)* %src, <16 x i32> %mask) local_unnamed_addr #3 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !no_barrier_path !9 !kernel_has_sub_groups !9 !vectorized_kernel !13 !vectorized_width !12 !recommended_vector_length !12 !scalar_kernel !1 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !8 !vectorization_dimension !8 !can_unite_workgroups !7 {
entry:
  %alloca.src = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %src, i32 addrspace(1)** %alloca.src, align 8
  %vec.mask = alloca <16 x i32>, align 64
  %mask.cast = bitcast <16 x i32>* %vec.mask to i32*
  store <16 x i32> %mask, <16 x i32>* %vec.mask, align 64
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6
  %0 = trunc i64 %call1 to i32
  %call = tail call i32 @_Z18get_sub_group_sizev() #5
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.src = load i32 addrspace(1)*, i32 addrspace(1)** %alloca.src, align 8
  br label %VPlannedBB

VPlannedBB:                                       ; preds = %simd.loop.preheader
  %broadcast.splatinsert = insertelement <16 x i32> poison, i32 %0, i32 0
  %broadcast.splat = shufflevector <16 x i32> %broadcast.splatinsert, <16 x i32> poison, <16 x i32> zeroinitializer
  %broadcast.splatinsert6 = insertelement <16 x i32> poison, i32 %call, i32 0
  %broadcast.splat7 = shufflevector <16 x i32> %broadcast.splatinsert6, <16 x i32> poison, <16 x i32> zeroinitializer
  br label %VPlannedBB1

VPlannedBB1:                                      ; preds = %VPlannedBB
  br label %vector.body

vector.body:                                      ; preds = %VPlannedBB8, %VPlannedBB1
  %uni.phi = phi i32 [ 0, %VPlannedBB1 ], [ %8, %VPlannedBB8 ]
  %vec.phi = phi <16 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, %VPlannedBB1 ], [ %7, %VPlannedBB8 ]
  %1 = add nuw <16 x i32> %broadcast.splat, %vec.phi
  %2 = sext <16 x i32> %1 to <16 x i64>
  %.extract.0. = extractelement <16 x i64> %2, i32 0
  %scalar.gep = getelementptr i32, i32* %mask.cast, i32 %uni.phi
  %3 = bitcast i32* %scalar.gep to <16 x i32>*
  %wide.load = load <16 x i32>, <16 x i32>* %3, align 64
  %4 = icmp ne <16 x i32> %wide.load, zeroinitializer
  %5 = xor <16 x i1> %4, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %VPlannedBB3

VPlannedBB3:                                      ; preds = %vector.body
  br label %VPlannedBB4

VPlannedBB4:                                      ; preds = %VPlannedBB3
  %scalar.gep5 = getelementptr inbounds i32, i32 addrspace(1)* %load.src, i64 %.extract.0.
  %6 = bitcast i32 addrspace(1)* %scalar.gep5 to <16 x i32> addrspace(1)*
  call void @llvm.masked.store.v16i32.p1v16i32(<16 x i32> %broadcast.splat7, <16 x i32> addrspace(1)* %6, i32 4, <16 x i1> %4)
  br label %VPlannedBB8

VPlannedBB8:                                      ; preds = %VPlannedBB4
  %7 = add nuw <16 x i32> %vec.phi, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %8 = add nuw i32 %uni.phi, 16
  %9 = icmp ult i32 %8, 16
  br i1 false, label %vector.body, label %VPlannedBB9, !llvm.loop !20

VPlannedBB9:                                      ; preds = %VPlannedBB8
  br label %VPlannedBB10

VPlannedBB10:                                     ; preds = %VPlannedBB9
  br label %final.merge

final.merge:                                      ; preds = %VPlannedBB10
  %uni.phi11 = phi i32 [ 16, %VPlannedBB10 ]
  br label %simd.end.region

simd.loop:                                        ; preds = %simd.loop.exit
  %index = phi i32 [ %indvar, %simd.loop.exit ]
  %add = add nuw i32 %0, %index
  %10 = sext i32 %add to i64
  %mask.gep = getelementptr i32, i32* %mask.cast, i32 %index
  %mask.parm = load i32, i32* %mask.gep, align 4
  %mask.cond = icmp ne i32 %mask.parm, 0
  br i1 %mask.cond, label %simd.loop.then, label %simd.loop.else

simd.loop.then:                                   ; preds = %simd.loop
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %load.src, i64 %10
  store i32 %call, i32 addrspace(1)* %arrayidx, align 4, !tbaa !14
  br label %simd.loop.exit

simd.loop.else:                                   ; preds = %simd.loop
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop.else, %simd.loop.then
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br label %simd.loop

simd.end.region:                                  ; preds = %final.merge
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.store.v16i32.p1v16i32(<16 x i32>, <16 x i32> addrspace(1)*, i32 immarg, <16 x i1>) #4

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "has-sub-groups" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "vector-variants"="_ZGVeN16u_test,_ZGVeM16u_test" }
attributes #1 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent mustprogress nofree nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent norecurse nounwind "frame-pointer"="none" "has-sub-groups" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "vector-variants"="_ZGVeN16u_test,_ZGVeM16u_test" }
attributes #4 = { argmemonly nofree nosync nounwind willreturn writeonly }
attributes #5 = { convergent nounwind }
attributes #6 = { convergent nounwind readnone willreturn }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 2, i32 0}
!1 = !{void (i32 addrspace(1)*)* @test}
!2 = !{i32 1}
!3 = !{!"none"}
!4 = !{!"int*"}
!5 = !{!""}
!6 = !{!"src"}
!7 = !{i1 false}
!8 = !{i32 0}
!9 = !{i1 true}
!10 = !{void (i32 addrspace(1)*)* @_ZGVeN16u_test}
!11 = !{void (i32 addrspace(1)*, <16 x i32>)* @_ZGVeM16u_test}
!12 = !{i32 16}
!13 = !{null}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = distinct !{!18, !19}
!19 = !{!"llvm.loop.isvectorized", i32 1}
!20 = distinct !{!20, !19}

; DEBUGIFY-NOT: WARNING
