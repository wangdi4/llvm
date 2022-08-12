; This test checks if we correctly widen the insertelement/extractelement instruction



; RUN: opt %s -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -vplan-vec \
; RUN: -vplan-force-vf=2 2>&1 | FileCheck %s --check-prefix=CHECK-VF2
; RUN: opt %s -S -passes="mem2reg,loop-simplify,lcssa,vpo-cfg-restructuring,vplan-vec" \
; RUN: -vplan-force-vf=2 2>&1 | FileCheck %s --check-prefix=CHECK-VF2

; RUN: opt %s -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -vplan-vec \
; RUN: -vplan-force-vf=8 2>&1 | FileCheck %s --check-prefix=CHECK-VF8
; RUN: opt %s -S -passes="mem2reg,loop-simplify,lcssa,vpo-cfg-restructuring,vplan-vec" \
; RUN: -vplan-force-vf=8 2>&1 | FileCheck %s --check-prefix=CHECK-VF8

; CHECK-VF2: [[wideExtract:%.*]] = shufflevector <4 x i32> [[VAR:%.*]], <4 x i32> undef, <2 x i32> <i32 0, i32 2>
; CHECK-VF2-NEXT: [[wideExtract1:%.*]] = shufflevector <4 x i32> [[VAR:%.*]], <4 x i32> undef, <2 x i32> <i32 1, i32 3>
; CHECK-VF2: [[wideInsert:%.*]] = shufflevector <2 x i32> [[VAR:%.*]], <2 x i32> undef, <4 x i32> <i32 0, i32 undef, i32 1, i32 undef>
; CHECK-VF2-NEXT: [[extendedSubVec:%.*]] = shufflevector <2 x i32> [[VAR1:%.*]], <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
; CHECK-VF2-NEXT: [[wideInsert1:%.*]] = shufflevector <4 x i32> [[wideInsert]], <4 x i32> [[extendedSubVec]], <4 x i32> <i32 0, i32 4, i32 2, i32 5>

; CHECK-VF8: [[wideExtract:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>
; CHECK-VF8-NEXT: [[wideExtract1:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>
; CHECK-VF8: [[wideInsert:%.*]] = shufflevector <8 x i32> [[VAR:%.*]], <8 x i32> undef, <16 x i32> <i32 0, i32 undef, i32 1, i32 undef, i32 2, i32 undef, i32 3, i32 undef, i32 4, i32 undef, i32 5, i32 undef, i32 6, i32 undef, i32 7, i32 undef> 
; CHECK-VF8-NEXT: [[extendedSubVec:%.*]] = shufflevector <8 x i32> [[VAR1:%.*]], <8 x i32> undef,  <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> 
; CHECK-VF8-NEXT: [[wideInsert1:%.*]] = shufflevector <16 x i32> [[wideInsert]], <16 x i32> [[extendedSubVec]], <16 x i32> <i32 0, i32 16, i32 2, i32 17, i32 4, i32 18, i32 6, i32 19, i32 8, i32 20, i32 10, i32 21, i32 12, i32 22, i32 14, i32 23> 

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @test_int_add2(<2 x i32> addrspace(1)* %srcA, <2 x i32> addrspace(1)* %srcB, <2 x i32> addrspace(1)* %dst) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !13 !no_barrier_path !14 !vectorized_width !15 !scalarized_kernel !16 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds <2 x i32>, <2 x i32> addrspace(1)* %srcA, i64 %idxprom
  %0 = load <2 x i32>, <2 x i32> addrspace(1)* %arrayidx, align 8, !tbaa !17
  %scalar7 = extractelement <2 x i32> %0, i32 0
  %scalar8 = extractelement <2 x i32> %0, i32 1
  %arrayidx2 = getelementptr inbounds <2 x i32>, <2 x i32> addrspace(1)* %srcB, i64 %idxprom
  %1 = load <2 x i32>, <2 x i32> addrspace(1)* %arrayidx2, align 8, !tbaa !17
  %scalar = extractelement <2 x i32> %1, i32 0
  %scalar6 = extractelement <2 x i32> %1, i32 1
  %add9 = add i32 %scalar, %scalar7
  %add10 = add i32 %scalar6, %scalar8
  %assembled.vect = insertelement <2 x i32> undef, i32 %add9, i32 0
  %assembled.vect11 = insertelement <2 x i32> %assembled.vect, i32 %add10, i32 1
  %arrayidx4 = getelementptr inbounds <2 x i32>, <2 x i32> addrspace(1)* %dst, i64 %idxprom
  store <2 x i32> %assembled.vect11, <2 x i32> addrspace(1)* %arrayidx4, align 8, !tbaa !17
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1


; Function Attrs: convergent nounwind
define void @_ZGVdN8uuu_test_int_add2(<2 x i32> addrspace(1)* %srcA, <2 x i32> addrspace(1)* %srcB, <2 x i32> addrspace(1)* %dst) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !16 !no_barrier_path !14 !ocl_recommended_vector_length !20 !vectorized_width !20 !vectorization_dimension !21 !scalarized_kernel !4 !can_unite_workgroups !22 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(<2 x i32> addrspace(1)* %srcA, <2 x i32> addrspace(1)* %srcB, <2 x i32> addrspace(1)* %dst) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add = add nuw i64 %0, %call
  %sext = shl i64 %add, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds <2 x i32>, <2 x i32> addrspace(1)* %srcA, i64 %idxprom
  %1 = load <2 x i32>, <2 x i32> addrspace(1)* %arrayidx, align 8, !tbaa !17
  %scalar7 = extractelement <2 x i32> %1, i32 0
  %scalar8 = extractelement <2 x i32> %1, i32 1
  %arrayidx2 = getelementptr inbounds <2 x i32>, <2 x i32> addrspace(1)* %srcB, i64 %idxprom
  %2 = load <2 x i32>, <2 x i32> addrspace(1)* %arrayidx2, align 8, !tbaa !17
  %scalar = extractelement <2 x i32> %2, i32 0
  %scalar6 = extractelement <2 x i32> %2, i32 1
  %add9 = add i32 %scalar, %scalar7
  %add10 = add i32 %scalar6, %scalar8
  %assembled.vect = insertelement <2 x i32> undef, i32 %add9, i32 0
  %assembled.vect11 = insertelement <2 x i32> %assembled.vect, i32 %add10, i32 1
  %arrayidx4 = getelementptr inbounds <2 x i32>, <2 x i32> addrspace(1)* %dst, i64 %idxprom
  store <2 x i32> %assembled.vect11, <2 x i32> addrspace(1)* %arrayidx4, align 8, !tbaa !17
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !23

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2


!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"clang version 8.0.0 "}
!4 = !{void (<2 x i32> addrspace(1)*, <2 x i32> addrspace(1)*, <2 x i32> addrspace(1)*)* @test_int_add2}
!5 = !{i32 1, i32 1, i32 1}
!6 = !{!"none", !"none", !"none"}
!7 = !{!"int2*", !"int2*", !"int2*"}
!8 = !{!"int __attribute__((ext_vector_type(2)))*", !"int __attribute__((ext_vector_type(2)))*", !"int __attribute__((ext_vector_type(2)))*"}
!9 = !{!"", !"", !""}
!10 = !{i1 false, i1 false, i1 false}
!11 = !{i32 0, i32 0, i32 0}
!12 = !{!"srcA", !"srcB", !"dst"}
!13 = !{void (<2 x i32> addrspace(1)*, <2 x i32> addrspace(1)*, <2 x i32> addrspace(1)*)* @_ZGVdN8uuu_test_int_add2}
!14 = !{i1 true}
!15 = !{i32 1}
!16 = !{null}
!17 = !{!18, !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{i32 8}
!21 = !{i32 0}
!22 = !{i1 false}
!23 = distinct !{!23, !24}
!24 = !{!"llvm.loop.unroll.disable"}
