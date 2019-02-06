; RUN: %oclopt -prelegbools -verify -S %s -o - | FileCheck %s

; CHECK-LABEL: @_ZGVdN8uuu_test
; CHECK: %[[SEXT1:.*]] = sext <16 x i1> %{{.*}} to <16 x i16>
; CHECK-NEXT: %[[SHUF1:.*]] = shufflevector <16 x i16> %[[SEXT1]], <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>

; CHECK: %[[SEXT2:.*]] = sext <16 x i1> %{{.*}} to <16 x i16>
; CHECK-NEXT: %[[SHUF2:.*]] = shufflevector <16 x i16> %[[SEXT2]], <16 x i16> undef, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>

; ModuleID = 'main'
source_filename = "8"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @test(<2 x i16> addrspace(1)* %srcA, <2 x i16> addrspace(1)* %srcB, <2 x i16> addrspace(1)* %dst) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !13 !no_barrier_path !14 !vectorized_width !15 !scalarized_kernel !16 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  %sext4 = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext4, 32
  %arrayidx = getelementptr inbounds <2 x i16>, <2 x i16> addrspace(1)* %srcA, i64 %idxprom
  %0 = load <2 x i16>, <2 x i16> addrspace(1)* %arrayidx, align 4, !tbaa !17
  %scalar = extractelement <2 x i16> %0, i32 0
  %scalar7 = extractelement <2 x i16> %0, i32 1
  %arrayidx2 = getelementptr inbounds <2 x i16>, <2 x i16> addrspace(1)* %srcB, i64 %idxprom
  %1 = load <2 x i16>, <2 x i16> addrspace(1)* %arrayidx2, align 4, !tbaa !17
  %scalar8 = extractelement <2 x i16> %1, i32 0
  %scalar9 = extractelement <2 x i16> %1, i32 1
  %cmp10 = icmp slt i16 %scalar, %scalar8
  %cmp11 = icmp slt i16 %scalar7, %scalar9
  %sext12 = sext i1 %cmp10 to i16
  %sext13 = sext i1 %cmp11 to i16
  %assembled.vect = insertelement <2 x i16> undef, i16 %sext12, i32 0
  %assembled.vect14 = insertelement <2 x i16> %assembled.vect, i16 %sext13, i32 1
  %arrayidx4 = getelementptr inbounds <2 x i16>, <2 x i16> addrspace(1)* %dst, i64 %idxprom
  store <2 x i16> %assembled.vect14, <2 x i16> addrspace(1)* %arrayidx4, align 4, !tbaa !17
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

define [7 x i64] @WG.boundaries.test(<2 x i16> addrspace(1)*, <2 x i16> addrspace(1)*, <2 x i16> addrspace(1)*) !ocl_recommended_vector_length !20 {
entry:
  %3 = call i64 @_Z14get_local_sizej(i32 0)
  %4 = call i64 @get_base_global_id.(i32 0)
  %5 = call i64 @_Z14get_local_sizej(i32 1)
  %6 = call i64 @get_base_global_id.(i32 1)
  %7 = call i64 @_Z14get_local_sizej(i32 2)
  %8 = call i64 @get_base_global_id.(i32 2)
  %9 = insertvalue [7 x i64] undef, i64 %3, 2
  %10 = insertvalue [7 x i64] %9, i64 %4, 1
  %11 = insertvalue [7 x i64] %10, i64 %5, 4
  %12 = insertvalue [7 x i64] %11, i64 %6, 3
  %13 = insertvalue [7 x i64] %12, i64 %7, 6
  %14 = insertvalue [7 x i64] %13, i64 %8, 5
  %15 = insertvalue [7 x i64] %14, i64 1, 0
  ret [7 x i64] %15
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @_ZGVdN8uuu_test(<2 x i16> addrspace(1)* %srcA, <2 x i16> addrspace(1)* %srcB, <2 x i16> addrspace(1)* %dst) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !16 !no_barrier_path !14 !ocl_recommended_vector_length !20 !vectorized_width !20 !vectorization_dimension !21 !scalarized_kernel !4 !can_unite_workgroups !22 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  %broadcast.splatinsert = insertelement <8 x i64> undef, i64 %call, i32 0
  %broadcast.splat = shufflevector <8 x i64> %broadcast.splatinsert, <8 x i64> undef, <8 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index1 = phi i32 [ 0, %entry ], [ %index.next, %vector.body ]
  %vec.ind = phi <8 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, %entry ], [ %vec.ind.next, %vector.body ]
  %0 = zext <8 x i32> %vec.ind to <8 x i64>
  %1 = add <8 x i64> %broadcast.splat, %0
  %2 = shl <8 x i64> %1, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %3 = getelementptr inbounds <2 x i16>, <2 x i16> addrspace(1)* %srcA, i64 0, i64 0
  %Ind_0. = ashr exact <8 x i64> %2, <i64 31, i64 31, i64 31, i64 31, i64 31, i64 31, i64 31, i64 31>
  %Ind_1. = or <8 x i64> %Ind_0., <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %interleaved. = shufflevector <8 x i64> %Ind_0., <8 x i64> %Ind_1., <16 x i32> <i32 0, i32 8, i32 1, i32 9, i32 2, i32 10, i32 3, i32 11, i32 4, i32 12, i32 5, i32 13, i32 6, i32 14, i32 7, i32 15>
  %mm_vectorGEP2 = getelementptr inbounds i16, i16 addrspace(1)* %3, <16 x i64> %interleaved.
  %wide.masked.gather = call <16 x i16> @llvm.masked.gather.v16i16.v16p1i16(<16 x i16 addrspace(1)*> %mm_vectorGEP2, i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x i16> undef)
  %4 = getelementptr inbounds <2 x i16>, <2 x i16> addrspace(1)* %srcB, i64 0, i64 0
  %Ind_0.5 = ashr exact <8 x i64> %2, <i64 31, i64 31, i64 31, i64 31, i64 31, i64 31, i64 31, i64 31>
  %Ind_1.6 = or <8 x i64> %Ind_0.5, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %interleaved.7 = shufflevector <8 x i64> %Ind_0.5, <8 x i64> %Ind_1.6, <16 x i32> <i32 0, i32 8, i32 1, i32 9, i32 2, i32 10, i32 3, i32 11, i32 4, i32 12, i32 5, i32 13, i32 6, i32 14, i32 7, i32 15>
  %mm_vectorGEP8 = getelementptr inbounds i16, i16 addrspace(1)* %4, <16 x i64> %interleaved.7
  %wide.masked.gather9 = call <16 x i16> @llvm.masked.gather.v16i16.v16p1i16(<16 x i16 addrspace(1)*> %mm_vectorGEP8, i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x i16> undef)
  %5 = icmp slt <16 x i16> %wide.masked.gather, %wide.masked.gather9
  %6 = shufflevector <16 x i1> %5, <16 x i1> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>
  %7 = icmp slt <16 x i16> %wide.masked.gather, %wide.masked.gather9
  %8 = shufflevector <16 x i1> %7, <16 x i1> undef, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>
  %9 = sext <8 x i1> %6 to <8 x i16>
  %10 = sext <8 x i1> %8 to <8 x i16>
  %wide.insert = shufflevector <8 x i16> %9, <8 x i16> undef, <16 x i32> <i32 0, i32 undef, i32 1, i32 undef, i32 2, i32 undef, i32 3, i32 undef, i32 4, i32 undef, i32 5, i32 undef, i32 6, i32 undef, i32 7, i32 undef>
  %extended. = shufflevector <8 x i16> %10, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %wide.insert12 = shufflevector <16 x i16> %wide.insert, <16 x i16> %extended., <16 x i32> <i32 0, i32 16, i32 2, i32 17, i32 4, i32 18, i32 6, i32 19, i32 8, i32 20, i32 10, i32 21, i32 12, i32 22, i32 14, i32 23>
  %11 = getelementptr inbounds <2 x i16>, <2 x i16> addrspace(1)* %dst, i64 0, i64 0
  %Ind_0.14 = ashr exact <8 x i64> %2, <i64 31, i64 31, i64 31, i64 31, i64 31, i64 31, i64 31, i64 31>
  %Ind_1.15 = or <8 x i64> %Ind_0.14, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %interleaved.16 = shufflevector <8 x i64> %Ind_0.14, <8 x i64> %Ind_1.15, <16 x i32> <i32 0, i32 8, i32 1, i32 9, i32 2, i32 10, i32 3, i32 11, i32 4, i32 12, i32 5, i32 13, i32 6, i32 14, i32 7, i32 15>
  %mm_vectorGEP17 = getelementptr inbounds i16, i16 addrspace(1)* %11, <16 x i64> %interleaved.16
  call void @llvm.masked.scatter.v16i16.v16p1i16(<16 x i16> %wide.insert12, <16 x i16 addrspace(1)*> %mm_vectorGEP17, i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
  %index.next = add i32 %index1, 8
  %vec.ind.next = add <8 x i32> %vec.ind, <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  %12 = icmp eq i32 %index1, 0
  br i1 %12, label %return, label %vector.body

return:                                           ; preds = %vector.body
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nounwind readonly
declare <16 x i16> @llvm.masked.gather.v16i16.v16p1i16(<16 x i16 addrspace(1)*>, i32, <16 x i1>, <16 x i16>) #3

; Function Attrs: nounwind
declare void @llvm.masked.scatter.v16i16.v16p1i16(<16 x i16>, <16 x i16 addrspace(1)*>, i32, <16 x i1>) #2

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8uuu_test" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { nounwind readonly }
attributes #4 = { convergent nounwind readnone }

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
!3 = !{!"clang version 8.0.0"}
!4 = !{void (<2 x i16> addrspace(1)*, <2 x i16> addrspace(1)*, <2 x i16> addrspace(1)*)* @test}
!5 = !{i32 1, i32 1, i32 1}
!6 = !{!"none", !"none", !"none"}
!7 = !{!"short2*", !"short2*", !"short2*"}
!8 = !{!"short __attribute__((ext_vector_type(2)))*", !"short __attribute__((ext_vector_type(2)))*", !"short __attribute__((ext_vector_type(2)))*"}
!9 = !{!"", !"", !""}
!10 = !{i1 false, i1 false, i1 false}
!11 = !{i32 0, i32 0, i32 0}
!12 = !{!"srcA", !"srcB", !"dst"}
!13 = !{void (<2 x i16> addrspace(1)*, <2 x i16> addrspace(1)*, <2 x i16> addrspace(1)*)* @_ZGVdN8uuu_test}
!14 = !{i1 true}
!15 = !{i32 1}
!16 = !{null}
!17 = !{!18, !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{i32 8}
!21 = !{i32 0}
!22 = !{i1 false}
