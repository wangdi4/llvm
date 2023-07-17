; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,function(require<sycl-kernel-weighted-inst-count-analysis>)' -S %s | FileCheck %s

; FIXME move to a transform pass.

; Check that recommended_vector_length metadata is not added to non-kernel
; function or kernel function which already has vectorized_width metadata.

; CHECK-NOT: recommended_vector_length

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc void @foo(ptr addrspace(1) %dst, i32 %v, i64 %gid) unnamed_addr #0 !kernel_arg_base_type !22 !arg_type_null_val !23 {
entry:
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %gid
  store i32 %v, ptr addrspace(1) %arrayidx, align 4, !tbaa !3
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(ptr addrspace(1) %dst, i32 %v) local_unnamed_addr #1 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_name !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !13 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !no_barrier_path !15 !kernel_has_sub_groups !16 !vectorized_kernel !17 !vectorized_width !18 !scalar_kernel !19 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !20 !arg_type_null_val !24 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #8
  tail call fastcc void @foo(ptr addrspace(1) %dst, i32 %v, i64 %call) #9
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

define [7 x i64] @WG.boundaries.test(ptr addrspace(1) %0, i32 %1) {
entry:
  %2 = call i64 @_Z14get_local_sizej(i32 0)
  %3 = call i64 @get_base_global_id.(i32 0)
  %4 = call i64 @_Z14get_local_sizej(i32 1)
  %5 = call i64 @get_base_global_id.(i32 1)
  %6 = call i64 @_Z14get_local_sizej(i32 2)
  %7 = call i64 @get_base_global_id.(i32 2)
  %8 = insertvalue [7 x i64] undef, i64 %2, 2
  %9 = insertvalue [7 x i64] %8, i64 %3, 1
  %10 = insertvalue [7 x i64] %9, i64 %4, 4
  %11 = insertvalue [7 x i64] %10, i64 %5, 3
  %12 = insertvalue [7 x i64] %11, i64 %6, 6
  %13 = insertvalue [7 x i64] %12, i64 %7, 5
  %14 = insertvalue [7 x i64] %13, i64 1, 0
  ret [7 x i64] %14
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc void @_ZGVeM16vvv_foo(<16 x ptr addrspace(1)> %dst, <16 x i32> %v, <16 x i64> %gid, <16 x i64> %mask) unnamed_addr #3 {
entry:
  %vec.dst = alloca <16 x ptr addrspace(1)>, align 128
  %vec.v = alloca <16 x i32>, align 64
  %vec.gid = alloca <16 x i64>, align 128
  %vec.mask = alloca <16 x i64>, align 128
  store <16 x ptr addrspace(1)> %dst, ptr %vec.dst, align 128
  store <16 x i32> %v, ptr %vec.v, align 64
  store <16 x i64> %gid, ptr %vec.gid, align 128
  store <16 x i64> %mask, ptr %vec.mask, align 128
  %0 = sext i32 0 to i64
  %scalar.gep = getelementptr <16 x i64>, ptr %vec.mask, i64 0, i64 %0
  %wide.load = load <16 x i64>, ptr %scalar.gep, align 128
  %1 = icmp ne <16 x i64> %wide.load, zeroinitializer
  %2 = sext i32 0 to i64
  %scalar.gep5 = getelementptr <16 x ptr addrspace(1)>, ptr %vec.dst, i64 0, i64 %2
  %wide.masked.load = call <16 x ptr addrspace(1)> @llvm.masked.load.v16p1.p0(ptr %scalar.gep5, i32 128, <16 x i1> %1, <16 x ptr addrspace(1)> undef)
  %3 = sext i32 0 to i64
  %scalar.gep6 = getelementptr <16 x i64>, ptr %vec.gid, i64 0, i64 %3
  %wide.masked.load7 = call <16 x i64> @llvm.masked.load.v16i64.p0(ptr %scalar.gep6, i32 128, <16 x i1> %1, <16 x i64> undef)
  %mm_vectorGEP = getelementptr inbounds i32, <16 x ptr addrspace(1)> %wide.masked.load, <16 x i64> %wide.masked.load7
  %4 = sext i32 0 to i64
  %scalar.gep8 = getelementptr <16 x i32>, ptr %vec.v, i64 0, i64 %4
  %wide.masked.load9 = call <16 x i32> @llvm.masked.load.v16i32.p0(ptr %scalar.gep8, i32 64, <16 x i1> %1, <16 x i32> undef)
  call void @llvm.masked.scatter.v16i32.v16p1(<16 x i32> %wide.masked.load9, <16 x ptr addrspace(1)> %mm_vectorGEP, i32 4, <16 x i1> %1)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc void @_ZGVeN16vvv_foo(<16 x ptr addrspace(1)> %dst, <16 x i32> %v, <16 x i64> %gid) unnamed_addr #3 {
entry:
  %vec.dst = alloca <16 x ptr addrspace(1)>, align 128
  %vec.v = alloca <16 x i32>, align 64
  %vec.gid = alloca <16 x i64>, align 128
  store <16 x ptr addrspace(1)> %dst, ptr %vec.dst, align 128
  store <16 x i32> %v, ptr %vec.v, align 64
  store <16 x i64> %gid, ptr %vec.gid, align 128
  %0 = sext i32 0 to i64
  %scalar.gep = getelementptr <16 x ptr addrspace(1)>, ptr %vec.dst, i64 0, i64 %0
  %wide.load = load <16 x ptr addrspace(1)>, ptr %scalar.gep, align 128
  %1 = sext i32 0 to i64
  %scalar.gep3 = getelementptr <16 x i64>, ptr %vec.gid, i64 0, i64 %1
  %wide.load4 = load <16 x i64>, ptr %scalar.gep3, align 128
  %mm_vectorGEP = getelementptr inbounds i32, <16 x ptr addrspace(1)> %wide.load, <16 x i64> %wide.load4
  %2 = sext i32 0 to i64
  %scalar.gep5 = getelementptr <16 x i32>, ptr %vec.v, i64 0, i64 %2
  %wide.load6 = load <16 x i32>, ptr %scalar.gep5, align 64
  call void @llvm.masked.scatter.v16i32.v16p1(<16 x i32> %wide.load6, <16 x ptr addrspace(1)> %mm_vectorGEP, i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @_ZGVeN16uu_test(ptr addrspace(1) %dst, i32 %v) local_unnamed_addr #5 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_name !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !13 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !no_barrier_path !15 !kernel_has_sub_groups !16 !vectorized_kernel !19 !vectorized_width !21 !scalar_kernel !1 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !20 !vectorization_dimension !20 !can_unite_workgroups !15 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #8
  %0 = trunc i64 %call to i32
  %broadcast.splatinsert = insertelement <16 x i32> poison, i32 %0, i64 0
  %broadcast.splat = shufflevector <16 x i32> %broadcast.splatinsert, <16 x i32> poison, <16 x i32> zeroinitializer
  %broadcast.splatinsert3 = insertelement <16 x ptr addrspace(1)> poison, ptr addrspace(1) %dst, i64 0
  %broadcast.splat4 = shufflevector <16 x ptr addrspace(1)> %broadcast.splatinsert3, <16 x ptr addrspace(1)> poison, <16 x i32> zeroinitializer
  %broadcast.splatinsert5 = insertelement <16 x i32> poison, i32 %v, i64 0
  %broadcast.splat6 = shufflevector <16 x i32> %broadcast.splatinsert5, <16 x i32> poison, <16 x i32> zeroinitializer
  %1 = add nuw <16 x i32> %broadcast.splat, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %2 = sext <16 x i32> %1 to <16 x i64>
  call fastcc void @_ZGVeN16vvv_foo(<16 x ptr addrspace(1)> %broadcast.splat4, <16 x i32> %broadcast.splat6, <16 x i64> %2) #10
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind readonly willreturn
declare <16 x ptr addrspace(1)> @llvm.masked.load.v16p1.p0(ptr, i32 immarg, <16 x i1>, <16 x ptr addrspace(1)>) #6

; Function Attrs: argmemonly nofree nosync nounwind readonly willreturn
declare <16 x i64> @llvm.masked.load.v16i64.p0(ptr, i32 immarg, <16 x i1>, <16 x i64>) #6

; Function Attrs: argmemonly nofree nosync nounwind readonly willreturn
declare <16 x i32> @llvm.masked.load.v16i32.p0(ptr, i32 immarg, <16 x i1>, <16 x i32>) #6

; Function Attrs: nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.scatter.v16i32.v16p1(<16 x i32>, <16 x ptr addrspace(1)>, i32 immarg, <16 x i1>) #7

attributes #0 = { convergent noinline norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #2 = { convergent mustprogress nofree nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent noinline norecurse nounwind "frame-pointer"="none" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "widened-size"="16" }
attributes #4 = { nounwind }
attributes #5 = { convergent norecurse nounwind "frame-pointer"="none" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #6 = { argmemonly nofree nosync nounwind readonly willreturn }
attributes #7 = { nofree nosync nounwind willreturn writeonly }
attributes #8 = { convergent nounwind readnone willreturn }
attributes #9 = { convergent }
attributes #10 = { convergent }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}
!opencl.stats.Vectorizer.Chosen_Vectorization_Dim = !{!2}

!0 = !{i32 2, i32 0}
!1 = !{ptr @test}
!2 = !{!"The chosen vectorization dimension"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{i32 1, i32 0}
!8 = !{!"none", !"none"}
!9 = !{!"int*", !"int"}
!10 = !{!"restrict", !""}
!11 = !{!"dst", !"v"}
!12 = !{i1 false, i1 false}
!13 = !{i32 0, i32 0}
!14 = !{!"", !""}
!15 = !{i1 true}
!16 = !{i1 false}
!17 = !{ptr @_ZGVeN16uu_test}
!18 = !{i32 1}
!19 = !{null}
!20 = !{i32 0}
!21 = !{i32 16}
!22 = !{!"int*", !"int", !"long"}
!23 = !{ptr addrspace(1) null, i32 0, i64 0}
!24 = !{ptr addrspace(1) null, i32 0}
