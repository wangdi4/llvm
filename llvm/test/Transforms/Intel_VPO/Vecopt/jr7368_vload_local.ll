; RUN: opt -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -vplan-vec %s | FileCheck %s
; RUN: opt -S -passes="mem2reg,loop-simplify,lcssa,vpo-cfg-restructuring,vplan-vec" %s | FileCheck %s
; CHECK-LABEL: vector.body
; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noduplicate nounwind
define void @test_fn(<2 x i8> addrspace(3)* noalias %sSharedStorage, <2 x i8> addrspace(1)* %src, i32 addrspace(1)* %offsets, i32 addrspace(1)* %alignmentOffsets, <2 x i8> addrspace(1)* %results) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !13 !no_barrier_path !14 !vectorized_width !15 !scalarized_kernel !16 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
  %call1 = tail call i64 @_Z12get_local_idj(i32 0) #5
  %conv2 = trunc i64 %call1 to i32
  %cmp = icmp eq i32 %conv2, 0
  br i1 %cmp, label %for.body, label %if.end

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds <2 x i8>, <2 x i8> addrspace(1)* %src, i64 %indvars.iv
  %0 = bitcast <2 x i8> addrspace(1)* %arrayidx to i16 addrspace(1)*
  %1 = load i16, i16 addrspace(1)* %0, align 2, !tbaa !17
  %arrayidx7 = getelementptr inbounds <2 x i8>, <2 x i8> addrspace(3)* %sSharedStorage, i64 %indvars.iv
  %2 = bitcast <2 x i8> addrspace(3)* %arrayidx7 to i16 addrspace(3)*
  store i16 %1, i16 addrspace(3)* %2, align 2, !tbaa !17
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4096
  br i1 %exitcond, label %if.end, label %for.body

if.end:                                           ; preds = %for.body, %entry
  tail call void @_Z7barrierj(i32 1) #6
  %sext = shl i64 %call, 32
  %idxprom8 = ashr exact i64 %sext, 32
  %arrayidx9 = getelementptr inbounds i32, i32 addrspace(1)* %offsets, i64 %idxprom8
  %3 = load i32, i32 addrspace(1)* %arrayidx9, align 4, !tbaa !20
  %conv10 = zext i32 %3 to i64
  %arrayidx12 = getelementptr inbounds i32, i32 addrspace(1)* %alignmentOffsets, i64 %idxprom8
  %4 = load i32, i32 addrspace(1)* %arrayidx12, align 4, !tbaa !20
  %idx.ext = zext i32 %4 to i64
  %add.ptr = getelementptr inbounds <2 x i8>, <2 x i8> addrspace(3)* %sSharedStorage, i64 0, i64 %idx.ext
  %call13 = tail call <2 x i8> @_Z6vload2mPU3AS3Kc(i64 %conv10, i8 addrspace(3)* %add.ptr) #6
  %arrayidx15 = getelementptr inbounds <2 x i8>, <2 x i8> addrspace(1)* %results, i64 %idxprom8
  store <2 x i8> %call13, <2 x i8> addrspace(1)* %arrayidx15, align 2, !tbaa !17
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent noduplicate
declare void @_Z7barrierj(i32) local_unnamed_addr #2

; Function Attrs: convergent
declare <2 x i8> @_Z6vload2mPU3AS3Kc(i64, i8 addrspace(3)*) local_unnamed_addr #3

; Function Attrs: convergent noduplicate nounwind
define void @_ZGVdN8uuuuu_test_fn(<2 x i8> addrspace(3)* noalias %sSharedStorage, <2 x i8> addrspace(1)* %src, i32 addrspace(1)* %offsets, i32 addrspace(1)* %alignmentOffsets, <2 x i8> addrspace(1)* %results) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !16 !no_barrier_path !14 !ocl_recommended_vector_length !22 !vectorized_width !22 !vectorization_dimension !23 !scalarized_kernel !4 !can_unite_workgroups !14 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
  %call1 = tail call i64 @_Z12get_local_idj(i32 0) #5
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(<2 x i8> addrspace(3)* %sSharedStorage, <2 x i8> addrspace(1)* %src, i32 addrspace(1)* %offsets, i32 addrspace(1)* %alignmentOffsets, <2 x i8> addrspace(1)* %results) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add1 = add nuw i64 %0, %call1
  %1 = sext i32 %index to i64
  %add = add nuw i64 %1, %call
  %conv2 = trunc i64 %add1 to i32
  %cmp = icmp eq i32 %conv2, 0
  br i1 %cmp, label %for.body, label %if.end

for.body:                                         ; preds = %for.body, %simd.loop
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %simd.loop ]
  %arrayidx = getelementptr inbounds <2 x i8>, <2 x i8> addrspace(1)* %src, i64 %indvars.iv
  %2 = bitcast <2 x i8> addrspace(1)* %arrayidx to i16 addrspace(1)*
  %3 = load i16, i16 addrspace(1)* %2, align 2, !tbaa !17
  %arrayidx7 = getelementptr inbounds <2 x i8>, <2 x i8> addrspace(3)* %sSharedStorage, i64 %indvars.iv
  %4 = bitcast <2 x i8> addrspace(3)* %arrayidx7 to i16 addrspace(3)*
  store i16 %3, i16 addrspace(3)* %4, align 2, !tbaa !17
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4096
  br i1 %exitcond, label %if.end, label %for.body

if.end:                                           ; preds = %for.body, %simd.loop
  tail call void @_Z7barrierj(i32 1) #6
  %sext = shl i64 %add, 32
  %idxprom8 = ashr exact i64 %sext, 32
  %arrayidx9 = getelementptr inbounds i32, i32 addrspace(1)* %offsets, i64 %idxprom8
  %5 = load i32, i32 addrspace(1)* %arrayidx9, align 4, !tbaa !20
  %conv10 = zext i32 %5 to i64
  %arrayidx12 = getelementptr inbounds i32, i32 addrspace(1)* %alignmentOffsets, i64 %idxprom8
  %6 = load i32, i32 addrspace(1)* %arrayidx12, align 4, !tbaa !20
  %idx.ext = zext i32 %6 to i64
  %add.ptr = getelementptr inbounds <2 x i8>, <2 x i8> addrspace(3)* %sSharedStorage, i64 0, i64 %idx.ext
  %call13 = tail call <2 x i8> @_Z6vload2mPU3AS3Kc(i64 %conv10, i8 addrspace(3)* %add.ptr) #6
  %arrayidx15 = getelementptr inbounds <2 x i8>, <2 x i8> addrspace(1)* %results, i64 %idxprom8
  store <2 x i8> %call13, <2 x i8> addrspace(1)* %arrayidx15, align 2, !tbaa !17
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %if.end
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !24

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"(), "DIR.QUAL.LIST.END"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { convergent noduplicate nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="16" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8uuuuu_test_fn" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent noduplicate "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
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
!opencl.kernels = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 73a7cd4b8b270182f03b0d325c3fd4cd6e6dbf56) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bee4537ea28bde70841c48e6a4811ac4f86f36d9)"}
!4 = !{void (<2 x i8> addrspace(3)*, <2 x i8> addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, <2 x i8> addrspace(1)*)* @test_fn}
!5 = !{i32 3, i32 1, i32 1, i32 1, i32 1}
!6 = !{!"none", !"none", !"none", !"none", !"none"}
!7 = !{!"char2*", !"char2*", !"uint*", !"uint*", !"char2*"}
!8 = !{!"char __attribute__((ext_vector_type(2)))*", !"char __attribute__((ext_vector_type(2)))*", !"uint*", !"uint*", !"char __attribute__((ext_vector_type(2)))*"}
!9 = !{!"", !"", !"", !"", !""}
!10 = !{i1 false, i1 false, i1 false, i1 false, i1 false}
!11 = !{i32 0, i32 0, i32 0, i32 0, i32 0}
!12 = !{!"sSharedStorage", !"src", !"offsets", !"alignmentOffsets", !"results"}
!13 = !{void (<2 x i8> addrspace(3)*, <2 x i8> addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, <2 x i8> addrspace(1)*)* @_ZGVdN8uuuuu_test_fn}
!14 = !{i1 false}
!15 = !{i32 1}
!16 = !{null}
!17 = !{!18, !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !18, i64 0}
!22 = !{i32 8}
!23 = !{i32 0}
!24 = distinct !{!24, !25}
!25 = !{!"llvm.loop.unroll.disable"}
