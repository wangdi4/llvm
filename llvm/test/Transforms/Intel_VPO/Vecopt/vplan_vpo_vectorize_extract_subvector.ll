; RUN: opt %s -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -VPlanDriver \
; RUN: -vplan-force-vf=2 2>&1 | FileCheck %s --check-prefix=CHECK-VF2

; RUN: opt %s -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -VPlanDriver \
; RUN: -vplan-force-vf=8 2>&1 | FileCheck %s --check-prefix=CHECK-VF8

; CHECK-VF2: %[[ExtractSubvec:.*]] = shufflevector <4 x i32> [[VAR:%.*]], <4 x i32> undef, <2 x i32> <i32 0, i32 1>
; CHECK-VF2-NEXT: %[[CallResult:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec]])
; CHECK-VF2: %[[ExtractSubvec1:.*]] = shufflevector <4 x i32> [[VAR:%.*]], <4 x i32> undef, <2 x i32> <i32 2, i32 3>
; CHECK-VF2-NEXT: %[[CallResult1:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec1]])

; CHECK-VF2: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec]], <4 x float> %[[CallResult]])
; CHECK-VF2: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec1]], <4 x float> %[[CallResult1]])



; CHECK-VF8: %[[ExtractSubvec:.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 0, i32 1>
; CHECK-VF8-NEXT: %[[CallResult:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec]])
; CHECK-VF8: %[[ExtractSubvec1:.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 2, i32 3>
; CHECK-VF8-NEXT: %[[CallResult1:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec1]])
; CHECK-VF8: %[[ExtractSubvec2:.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 4, i32 5>
; CHECK-VF8-NEXT: %[[CallResult2:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec2]])
; CHECK-VF8: %[[ExtractSubvec3:.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 6, i32 7>
; CHECK-VF8-NEXT: %[[CallResult3:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec3]])
; CHECK-VF8: %[[ExtractSubvec4:.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 8, i32 9>
; CHECK-VF8-NEXT: %[[CallResult4:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec4]])
; CHECK-VF8: %[[ExtractSubvec5:.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 10, i32 11>
; CHECK-VF8-NEXT: %[[CallResult5:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec5]])
; CHECK-VF8: %[[ExtractSubvec6:.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 12, i32 13>
; CHECK-VF8-NEXT: %[[CallResult6:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec6]])
; CHECK-VF8: %[[ExtractSubvec7:.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 14, i32 15>
; CHECK-VF8-NEXT: %[[CallResult7:.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* [[srcimg:%.*]], %opencl.sampler_t.7 addrspace(2)* [[sampler:%.*]], <2 x i32> %[[ExtractSubvec7]])

; CHECK-VF8: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec]], <4 x float> %[[CallResult]])
; CHECK-VF8: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec1]], <4 x float> %[[CallResult1]])
; CHECK-VF8: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec2]], <4 x float> %[[CallResult2]])
; CHECK-VF8: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec3]], <4 x float> %[[CallResult3]])
; CHECK-VF8: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec4]], <4 x float> %[[CallResult4]])
; CHECK-VF8: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec5]], <4 x float> %[[CallResult5]])
; CHECK-VF8: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec6]], <4 x float> %[[CallResult6]])
; CHECK-VF8: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* [[dstimg:%.*]], <2 x i32> %[[ExtractSubvec7]], <4 x float> %[[CallResult7]])



; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%opencl.image2d_ro_t.5 = type opaque
%opencl.image2d_wo_t.6 = type opaque
%opencl.sampler_t.7 = type opaque

; Function Attrs: convergent nounwind
define void @test_rgba8888(%opencl.image2d_ro_t.5 addrspace(1)* %srcimg, %opencl.image2d_wo_t.6 addrspace(1)* %dstimg, %opencl.sampler_t.7 addrspace(2)* %sampler) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !13 !no_barrier_path !14 !vectorized_width !15 !scalarized_kernel !16 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %call to i32
  %call1 = tail call i64 @_Z13get_global_idj(i32 1)
  %conv2 = trunc i64 %call1 to i32
  %assembled.vect = insertelement <2 x i32> undef, i32 %conv, i32 0
  %assembled.vect11 = insertelement <2 x i32> %assembled.vect, i32 %conv2, i32 1
  %call9 = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* %srcimg, %opencl.sampler_t.7 addrspace(2)* %sampler, <2 x i32> %assembled.vect11) #6
  tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* %dstimg, <2 x i32> %assembled.vect11, <4 x float> %call9) #7
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i32 @_Z15get_image_width14ocl_image2d_wo(%opencl.image2d_wo_t.6 addrspace(1)*) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i32 @_Z16get_image_height14ocl_image2d_wo(%opencl.image2d_wo_t.6 addrspace(1)*) local_unnamed_addr #1

; Function Attrs: convergent nounwind readonly
declare <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)*, %opencl.sampler_t.7 addrspace(2)*, <2 x i32>) local_unnamed_addr #2

; Function Attrs: convergent
declare void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)*, <2 x i32>, <4 x float>) local_unnamed_addr #3

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @_ZGVdN8uuu_test_rgba8888(%opencl.image2d_ro_t.5 addrspace(1)* %srcimg, %opencl.image2d_wo_t.6 addrspace(1)* %dstimg, %opencl.sampler_t.7 addrspace(2)* %sampler) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !16 !no_barrier_path !14 !ocl_recommended_vector_length !17 !vectorized_width !17 !vectorization_dimension !18 !scalarized_kernel !5 !can_unite_workgroups !19 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(%opencl.image2d_ro_t.5 addrspace(1)* %srcimg, %opencl.image2d_wo_t.6 addrspace(1)* %dstimg, %opencl.sampler_t.7 addrspace(2)* %sampler) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add = add nuw i64 %0, %call
  %conv = trunc i64 %add to i32
  %call1 = tail call i64 @_Z13get_global_idj(i32 1) #5
  %conv2 = trunc i64 %call1 to i32
  %assembled.vect = insertelement <2 x i32> undef, i32 %conv, i32 0
  %assembled.vect11 = insertelement <2 x i32> %assembled.vect, i32 %conv2, i32 1
  %call9 = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* %srcimg, %opencl.sampler_t.7 addrspace(2)* %sampler, <2 x i32> %assembled.vect11) #6
  tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%opencl.image2d_wo_t.6 addrspace(1)* %dstimg, <2 x i32> %assembled.vect11, <4 x float> %call9) #7
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !20

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

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8uuu_test_rgba8888" }

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"cl_images"}
!4 = !{!"clang version 8.0.0"}
!5 = !{void (%opencl.image2d_ro_t.5 addrspace(1)*, %opencl.image2d_wo_t.6 addrspace(1)*, %opencl.sampler_t.7 addrspace(2)*)* @test_rgba8888}
!6 = !{i32 1, i32 1, i32 0}
!7 = !{!"read_only", !"write_only", !"none"}
!8 = !{!"image2d_t", !"image2d_t", !"sampler_t"}
!9 = !{!"", !"", !""}
!10 = !{i1 false, i1 false, i1 false}
!11 = !{i32 0, i32 0, i32 0}
!12 = !{!"srcimg", !"dstimg", !"sampler"}
!13 = !{void (%opencl.image2d_ro_t.5 addrspace(1)*, %opencl.image2d_wo_t.6 addrspace(1)*, %opencl.sampler_t.7 addrspace(2)*)* @_ZGVdN8uuu_test_rgba8888}
!14 = !{i1 true}
!15 = !{i32 1}
!16 = !{null}
!17 = !{i32 8}
!18 = !{i32 0}
!19 = !{i1 false}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.unroll.disable"}
