;RUN: opt -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring  -VPlanDriver -vplan-force-vf=2 < %s 2>&1 | FileCheck %s

; CHECK-LABEL: @_ZGVdN8uuuu_test_fn
; CHECK: [[PRIV_STORAGE_PTR:%.*]] = alloca [2 x [16 x <16 x i8>]], align 16
; CHECK: [[PRIV_BC:%.*]] = bitcast [2 x [16 x <16 x i8>]]* [[PRIV_STORAGE_PTR]] to [16 x <16 x i8>]*
; CHECK: [[PRIV_BASE_PTR:%.*]] = getelementptr [16 x <16 x i8>], [16 x <16 x i8>]* [[PRIV_BC]], <2 x i32> <i32 0, i32 1>
; CHECK: [[MEM_BASE_PTRS:%.*]] = getelementptr inbounds [16 x <16 x i8>], <2 x [16 x <16 x i8>]*> %2, <2 x i64> zeroinitializer, <2 x i64> zeroinitializer, <2 x i64> zeroinitializer
; CHECK:  [[E2:%.*]] = extractelement <2 x i8*> [[MEM_BASE_PTRS]], i32 1
; CHECK:  [[E1:%.*]] = extractelement <2 x i8*> [[MEM_BASE_PTRS]], i32 0
; CHECK: call void @llvm.lifetime.start.p0i8(i64 256, i8* nonnull [[E1]]) #4
; CHECK: call void @llvm.lifetime.start.p0i8(i64 256, i8* nonnull [[E2]]) #4
; CHECK: [[DATA:%.*]] = load <16 x i8>, <16 x i8> addrspace(1)* %src, align 16, !tbaa !17
; CHECK: [[R_VAL:%.*]] = shufflevector <16 x i8> [[DATA]], <16 x i8> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[V_GEP:%.*]] = getelementptr inbounds [16 x <16 x i8>], <2 x [16 x <16 x i8>]*> [[PRIV_BASE_PTR]], <2 x i64> zeroinitializer, <2 x i64> zeroinitializer
; CHECK: %6 = bitcast <2 x <16 x i8>*> [[V_GEP]] to <2 x i8*>
; CHECK: [[VEC_BASE_PTR:%.*]] = shufflevector <2 x i8*> %6, <2 x i8*> undef, <32 x i32> <i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
; CHECK:  [[ELEM_BASE_PTR:%.*]] = getelementptr i8, <32 x i8*> [[VEC_BASE_PTR]], <32 x i64> <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15, i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
; CHECK: call void @llvm.masked.scatter.v32i8.v32p0i8(<32 x i8> [[R_VAL]], <32 x i8*> [[ELEM_BASE_PTR]], i32 16, <32 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)


; MeduleID = 'main'
source_filename = "5"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent
declare <16 x i8> @_Z7vload16mPKc(i64, i8*) local_unnamed_addr #3

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @_ZGVdN8uuuu_test_fn(<16 x i8> addrspace(1)* %src, i32 addrspace(1)* %offsets, i32 addrspace(1)* %alignmentOffsets, <16 x i8> addrspace(1)* %results) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !16 !no_barrier_path !14 !ocl_recommended_vector_length !22 !vectorized_width !22 !vectorization_dimension !23  !can_unite_workgroups !24 {
entry:
  %sPrivateStorage = alloca [16 x <16 x i8>], align 16
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(<16 x i8> addrspace(1)* %src, i32 addrspace(1)* %offsets, i32 addrspace(1)* %alignmentOffsets, <16 x i8> addrspace(1)* %results), "QUAL.OMP.PRIVATE"([16 x <16 x i8>]* %sPrivateStorage), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add = add nuw i64 %0, %call
  %1 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 256, i8* nonnull %1) #4
  %2 = load <16 x i8>, <16 x i8> addrspace(1)* %src, align 16, !tbaa !17
  %arrayidx3 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 0
  store <16 x i8> %2, <16 x i8>* %arrayidx3, align 16, !tbaa !17
  %arrayidx.1 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 1
  %3 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.1, align 16, !tbaa !17
  %arrayidx3.1 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 1
  store <16 x i8> %3, <16 x i8>* %arrayidx3.1, align 16, !tbaa !17
  %arrayidx.2 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 2
  %4 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.2, align 16, !tbaa !17
  %arrayidx3.2 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 2
  store <16 x i8> %4, <16 x i8>* %arrayidx3.2, align 16, !tbaa !17
  %arrayidx.3 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 3
  %5 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.3, align 16, !tbaa !17
  %arrayidx3.3 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 3
  store <16 x i8> %5, <16 x i8>* %arrayidx3.3, align 16, !tbaa !17
  %arrayidx.4 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 4
  %6 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.4, align 16, !tbaa !17
  %arrayidx3.4 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 4
  store <16 x i8> %6, <16 x i8>* %arrayidx3.4, align 16, !tbaa !17
  %arrayidx.5 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 5
  %7 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.5, align 16, !tbaa !17
  %arrayidx3.5 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 5
  store <16 x i8> %7, <16 x i8>* %arrayidx3.5, align 16, !tbaa !17
  %arrayidx.6 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 6
  %8 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.6, align 16, !tbaa !17
  %arrayidx3.6 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 6
  store <16 x i8> %8, <16 x i8>* %arrayidx3.6, align 16, !tbaa !17
  %arrayidx.7 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 7
  %9 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.7, align 16, !tbaa !17
  %arrayidx3.7 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 7
  store <16 x i8> %9, <16 x i8>* %arrayidx3.7, align 16, !tbaa !17
  %arrayidx.8 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 8
  %10 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.8, align 16, !tbaa !17
  %arrayidx3.8 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 8
  store <16 x i8> %10, <16 x i8>* %arrayidx3.8, align 16, !tbaa !17
  %arrayidx.9 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 9
  %11 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.9, align 16, !tbaa !17
  %arrayidx3.9 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 9
  store <16 x i8> %11, <16 x i8>* %arrayidx3.9, align 16, !tbaa !17
  %arrayidx.10 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 10
  %12 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.10, align 16, !tbaa !17
  %arrayidx3.10 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 10
  store <16 x i8> %12, <16 x i8>* %arrayidx3.10, align 16, !tbaa !17
  %arrayidx.11 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 11
  %13 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.11, align 16, !tbaa !17
  %arrayidx3.11 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 11
  store <16 x i8> %13, <16 x i8>* %arrayidx3.11, align 16, !tbaa !17
  %arrayidx.12 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 12
  %14 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.12, align 16, !tbaa !17
  %arrayidx3.12 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 12
  store <16 x i8> %14, <16 x i8>* %arrayidx3.12, align 16, !tbaa !17
  %arrayidx.13 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 13
  %15 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.13, align 16, !tbaa !17
  %arrayidx3.13 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 13
  store <16 x i8> %15, <16 x i8>* %arrayidx3.13, align 16, !tbaa !17
  %arrayidx.14 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 14
  %16 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.14, align 16, !tbaa !17
  %arrayidx3.14 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 14
  store <16 x i8> %16, <16 x i8>* %arrayidx3.14, align 16, !tbaa !17
  %arrayidx.15 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %src, i64 15
  %17 = load <16 x i8>, <16 x i8> addrspace(1)* %arrayidx.15, align 16, !tbaa !17
  %arrayidx3.15 = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 15
  store <16 x i8> %17, <16 x i8>* %arrayidx3.15, align 16, !tbaa !17
  %sext = shl i64 %add, 32
  %idxprom4 = ashr exact i64 %sext, 32
  %arrayidx5 = getelementptr inbounds i32, i32 addrspace(1)* %offsets, i64 %idxprom4
  %18 = load i32, i32 addrspace(1)* %arrayidx5, align 4, !tbaa !20
  %conv6 = zext i32 %18 to i64
  %arrayidx8 = getelementptr inbounds i32, i32 addrspace(1)* %alignmentOffsets, i64 %idxprom4
  %19 = load i32, i32 addrspace(1)* %arrayidx8, align 4, !tbaa !20
  %idx.ext = zext i32 %19 to i64
  %add.ptr = getelementptr inbounds [16 x <16 x i8>], [16 x <16 x i8>]* %sPrivateStorage, i64 0, i64 0, i64 %idx.ext
  %call9 = call <16 x i8> @_Z7vload16mPKc(i64 %conv6, i8* nonnull %add.ptr) #6
  %arrayidx11 = getelementptr inbounds <16 x i8>, <16 x i8> addrspace(1)* %results, i64 %idxprom4
  store <16 x i8> %call9, <16 x i8> addrspace(1)* %arrayidx11, align 16, !tbaa !17
  call void @llvm.lifetime.end.p0i8(i64 256, i8* nonnull %1) #4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !25

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8uuuu_test_fn" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind readnone }
attributes #6 = { convergent nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"icx (ICX) 2019.8.2.0"}
!5 = !{i32 1, i32 1, i32 1, i32 1}
!6 = !{!"none", !"none", !"none", !"none"}
!7 = !{!"char16*", !"uint*", !"uint*", !"char16*"}
!8 = !{!"char __attribute__((ext_vector_type(16)))*", !"uint*", !"uint*", !"char __attribute__((ext_vector_type(16)))*"}
!9 = !{!"", !"", !"", !""}
!10 = !{i1 false, i1 false, i1 false, i1 false}
!11 = !{i32 0, i32 0, i32 0, i32 0}
!12 = !{!"src", !"offsets", !"alignmentOffsets", !"results"}
!13 = !{void (<16 x i8> addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, <16 x i8> addrspace(1)*)* @_ZGVdN8uuuu_test_fn}
!14 = !{i1 true}
!15 = !{i32 1}
!16 = !{null}
!17 = !{!18, !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !18, i64 0}
!22 = !{i32 8}
!23 = !{i32 0}
!24 = !{i1 false}
!25 = distinct !{!25, !26}
!26 = !{!"llvm.loop.unroll.disable"}
