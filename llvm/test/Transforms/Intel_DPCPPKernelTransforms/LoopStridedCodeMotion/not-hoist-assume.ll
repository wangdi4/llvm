; RUN: opt -passes=dpcpp-kernel-loop-strided-code-motion -S %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-loop-strided-code-motion -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-loop-strided-code-motion -S %s | FileCheck %s
; RUN: opt -dpcpp-kernel-loop-strided-code-motion -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that loop-strided-code-motion pass doesn't hoist %3 which is used for
; llvm.assume only.

; CHECK-NOT: phi <16 x i64>
; CHECK-NOT: strided.add

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind
define void @_test() local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !no_barrier_path !11 !kernel_has_sub_groups !12 !vectorized_kernel !13 !max_wg_dimensions !14 !vectorization_dimension !15 !barrier_buffer_size !15 !vectorized_width !16 !kernel_execution_length !17 !kernel_has_global_sync !12 !private_memory_size !15 !can_unite_workgroups !12 !scalar_kernel !6 !spirv.ParameterDecorations !18 {
WGLoopsEntry:
  %0 = call i64 @_Z14get_local_sizej(i32 0) #2
  %1 = call i64 @get_base_global_id.(i32 0) #2
  %2 = and i64 %0, -16
  %max.vector.gid = add i64 %2, %1
  br label %entryvector_func

entryvector_func:                                 ; preds = %entryvector_func, %WGLoopsEntry
  %dim_0_vector_ind_var = phi i64 [ %dim_0_vector_inc_ind_var, %entryvector_func ], [ %1, %WGLoopsEntry ]
  %broadcast.splatinsertvector_func = insertelement <16 x i64> poison, i64 %dim_0_vector_ind_var, i64 0
  %broadcast.splatvector_func = shufflevector <16 x i64> %broadcast.splatinsertvector_func, <16 x i64> poison, <16 x i32> zeroinitializer
  %3 = add nuw <16 x i64> %broadcast.splatvector_func, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %4 = icmp ult <16 x i64> %3, <i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648>
  %.extract.15.vector_func = extractelement <16 x i1> %4, i64 15
  %.extract.14.vector_func = extractelement <16 x i1> %4, i64 14
  %.extract.13.vector_func = extractelement <16 x i1> %4, i64 13
  %.extract.12.vector_func = extractelement <16 x i1> %4, i64 12
  %.extract.11.vector_func = extractelement <16 x i1> %4, i64 11
  %.extract.10.vector_func = extractelement <16 x i1> %4, i64 10
  %.extract.9.vector_func = extractelement <16 x i1> %4, i64 9
  %.extract.8.vector_func = extractelement <16 x i1> %4, i64 8
  %.extract.7.vector_func = extractelement <16 x i1> %4, i64 7
  %.extract.6.vector_func = extractelement <16 x i1> %4, i64 6
  %.extract.5.vector_func = extractelement <16 x i1> %4, i64 5
  %.extract.4.vector_func = extractelement <16 x i1> %4, i64 4
  %.extract.3.vector_func = extractelement <16 x i1> %4, i64 3
  %.extract.2.vector_func = extractelement <16 x i1> %4, i64 2
  %.extract.1.vector_func = extractelement <16 x i1> %4, i64 1
  %.extract.0.vector_func = extractelement <16 x i1> %4, i64 0
  tail call void @llvm.assume(i1 %.extract.0.vector_func)
  tail call void @llvm.assume(i1 %.extract.1.vector_func)
  tail call void @llvm.assume(i1 %.extract.2.vector_func)
  tail call void @llvm.assume(i1 %.extract.3.vector_func)
  tail call void @llvm.assume(i1 %.extract.4.vector_func)
  tail call void @llvm.assume(i1 %.extract.5.vector_func)
  tail call void @llvm.assume(i1 %.extract.6.vector_func)
  tail call void @llvm.assume(i1 %.extract.7.vector_func)
  tail call void @llvm.assume(i1 %.extract.8.vector_func)
  tail call void @llvm.assume(i1 %.extract.9.vector_func)
  tail call void @llvm.assume(i1 %.extract.10.vector_func)
  tail call void @llvm.assume(i1 %.extract.11.vector_func)
  tail call void @llvm.assume(i1 %.extract.12.vector_func)
  tail call void @llvm.assume(i1 %.extract.13.vector_func)
  tail call void @llvm.assume(i1 %.extract.14.vector_func)
  tail call void @llvm.assume(i1 %.extract.15.vector_func)
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 16
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %max.vector.gid
  br i1 %dim_0_vector_cmp.to.max, label %exit, label %entryvector_func

exit:                                             ; preds = %entryvector_func
  ret void
}

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #1

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { nounwind "may-have-openmp-directive"="false" "prefer-vector-width"="512" "vector-variants"="_ZGVeN16uuuuuu__test" }
attributes #1 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!spirv.MemoryModel = !{!0}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}
!sycl.kernels = !{!6}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{!"cl_doubles"}
!5 = !{i16 6, i16 14}
!6 = !{void ()* @_test}
!7 = !{i32 1, i32 0, i32 1, i32 0, i32 1, i32 0}
!8 = !{!"none", !"none", !"none", !"none", !"none", !"none"}
!9 = !{!"double*", !"class.cl::sycl::id", !"double*", !"class.cl::sycl::id", !"double*", !"class.cl::sycl::id"}
!10 = !{!"restrict", !"", !"restrict ", !"", !"restrict ", !""}
!11 = !{i1 true}
!12 = !{i1 false}
!13 = !{null}
!14 = !{i32 1}
!15 = !{i32 0}
!16 = !{i32 16}
!17 = !{i32 57}
!18 = !{!19, !22, !24, !22, !24, !22}
!19 = !{!20, !21}
!20 = !{i32 38, i32 4}
!21 = !{i32 44, i32 8}
!22 = !{!23, !21}
!23 = !{i32 38, i32 2}
!24 = !{!20, !25, !21}
!25 = !{i32 38, i32 6}

; DEBUGIFY-NOT: WARNING
