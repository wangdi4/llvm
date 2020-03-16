; RUN: %oclopt  --convert-vplan-mask  < %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%opencl.image2d_ro_t.5 = type opaque

; Function Attrs: convergent norecurse nounwind
define void @test(%opencl.image2d_ro_t.5 addrspace(1)* noalias %img) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !13 !vectorized_masked_kernel !14 !no_barrier_path !10 !kernel_has_sub_groups !15 !vectorized_width !6 !scalarized_kernel !16 {
entry:
  %call = tail call signext i16 @_Z23intel_sub_group_shufflesj(i16 signext 0, i32 0) #4
  %call1 = tail call zeroext i16 @_Z29intel_sub_group_block_read_us14ocl_image2d_roDv2_i(%opencl.image2d_ro_t.5 addrspace(1)* %img, <2 x i32> zeroinitializer) #4
  %call2 = tail call float @_Z21work_group_reduce_minf(float 0.000000e+00) #5
  %call3 = tail call double @_Z21work_group_reduce_addd(double 0.000000e+00) #5
  ret void
}

; Function Attrs: convergent
declare signext i16 @_Z23intel_sub_group_shufflesj(i16 signext, i32) local_unnamed_addr #1

; Function Attrs: convergent
declare zeroext i16 @_Z29intel_sub_group_block_read_us14ocl_image2d_roDv2_i(%opencl.image2d_ro_t.5 addrspace(1)*, <2 x i32>) local_unnamed_addr #1

; Function Attrs: convergent
declare float @_Z21work_group_reduce_minf(float) local_unnamed_addr #2

; Function Attrs: convergent
declare double @_Z21work_group_reduce_addd(double) local_unnamed_addr #2

; Function Attrs: convergent norecurse nounwind
define void @_ZGVcN4u_test(%opencl.image2d_ro_t.5 addrspace(1)* noalias %img) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !16 !no_barrier_path !10 !kernel_has_sub_groups !15 !ocl_recommended_vector_length !17 !vectorized_width !17 !vectorization_dimension !11 !scalarized_kernel !5 !can_unite_workgroups !10 {
entry:
  %0 = call signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext zeroinitializer, <4 x i32> zeroinitializer, <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>) #6
; CHECK: %mask.i32. = sext <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1> to <4 x i32>
; CHECK: call signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext zeroinitializer, <4 x i32> zeroinitializer, <4 x i32> %mask.i32.)
  %1 = call zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(%opencl.image2d_ro_t.5 addrspace(1)* %img, <2 x i32> zeroinitializer, <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>) #6
; CHECK: %mask.i32.1 = sext <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1> to <4 x i32>
; CHECK: call zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(%opencl.image2d_ro_t.5 addrspace(1)* %img, <2 x i32> zeroinitializer, <4 x i32> %mask.i32.1)
  %2 = call <4 x float> @_Z21work_group_reduce_minDv4_f(<4 x float> zeroinitializer) #7
; CHECK: call <4 x float> @_Z21work_group_reduce_minDv4_f(<4 x float> zeroinitializer)
  %3 = call <4 x double> @_Z21work_group_reduce_addDv4_d(<4 x double> zeroinitializer) #7
; CHECK: call <4 x double> @_Z21work_group_reduce_addDv4_d(<4 x double> zeroinitializer)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: convergent norecurse nounwind
define void @_ZGVcM4u_test(%opencl.image2d_ro_t.5 addrspace(1)* noalias %img, <4 x i32> %mask) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !vectorized_kernel !16 !no_barrier_path !10 !kernel_has_sub_groups !15 !ocl_recommended_vector_length !17 !vectorized_width !17 !vectorization_dimension !11 !scalarized_kernel !5 !can_unite_workgroups !10 {
entry:
  %vec.mask = alloca <4 x i32>, align 16
  store <4 x i32> %mask, <4 x i32>* %vec.mask, align 16
  %0 = sext i32 0 to i64
  %scalar.gep = getelementptr <4 x i32>, <4 x i32>* %vec.mask, i64 0, i64 %0
  %1 = bitcast i32* %scalar.gep to <4 x i32>*
  %wide.load = load <4 x i32>, <4 x i32>* %1, align 16
  %2 = icmp ne <4 x i32> %wide.load, zeroinitializer
  %maskext = sext <4 x i1> %2 to <4 x i16>
  %3 = call signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext zeroinitializer, <4 x i32> zeroinitializer, <4 x i16> %maskext) #6
; CHECK: %mask.i32. = sext <4 x i16> %maskext to <4 x i32>
; CHECK: call signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext zeroinitializer, <4 x i32> zeroinitializer, <4 x i32> %mask.i32.)
  %maskext2 = sext <4 x i1> %2 to <4 x i16>
  %4 = call zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(%opencl.image2d_ro_t.5 addrspace(1)* %img, <2 x i32> zeroinitializer, <4 x i16> %maskext2) #6
; CHECK: %mask.i32.1 = sext <4 x i16> %maskext2 to <4 x i32>
; CHECK: call zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(%opencl.image2d_ro_t.5 addrspace(1)* %img, <2 x i32> zeroinitializer, <4 x i32> %mask.i32.1)
  %maskext3 = sext <4 x i1> %2 to <4 x i32>
  %maskcast = bitcast <4 x i32> %maskext3 to <4 x float>
  %5 = call <4 x float> @_Z21work_group_reduce_minDv4_fDv4_j(<4 x float> zeroinitializer, <4 x float> %maskcast) #7
; CHECK: %mask.cast.i. = bitcast <4 x float> %maskcast to <4 x i32>
; CHECK: call <4 x float> @_Z21work_group_reduce_minDv4_fDv4_j(<4 x float> zeroinitializer, <4 x i32> %mask.cast.i.)
  %maskext4 = sext <4 x i1> %2 to <4 x i64>
  %maskcast5 = bitcast <4 x i64> %maskext4 to <4 x double>
  %6 = call <4 x double> @_Z21work_group_reduce_addDv4_dDv4_j(<4 x double> zeroinitializer, <4 x double> %maskcast5) #7
; CHECK: %mask.cast.i.2 = bitcast <4 x double> %maskcast5 to <4 x i64>
; CHECK: %mask.i32.3 = trunc <4 x i64> %mask.cast.i.2 to <4 x i32>
; CHECK: call <4 x double> @_Z21work_group_reduce_addDv4_dDv4_j(<4 x double> zeroinitializer, <4 x i32> %mask.i32.3)
  ret void
}

; CHECK: declare <4 x float> @_Z21work_group_reduce_minDv4_f(<4 x float>) local_unnamed_addr #[[ATTR2:[0-9]+]]
; CHECK: declare <4 x double> @_Z21work_group_reduce_addDv4_d(<4 x double>) local_unnamed_addr #[[ATTR2]]
; CHECK: declare signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext, <4 x i32>, <4 x i32>) #[[ATTR1:[0-9]+]]
; CHECK: declare zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(%opencl.image2d_ro_t.5 addrspace(1)*, <2 x i32>, <4 x i32>) #[[ATTR1]]
; CHECK: declare <4 x float> @_Z21work_group_reduce_minDv4_fDv4_j(<4 x float>, <4 x i32>) #[[ATTR2]]
; CHECK: declare <4 x double> @_Z21work_group_reduce_addDv4_dDv4_j(<4 x double>, <4 x i32>) #[[ATTR2]]

; CHECK: attributes #[[ATTR1]] = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
; CHECK: attributes #[[ATTR2]] = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "opencl-vec-uniform-return" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
; CHECK-NOT: "has-vplan-mask"
; CHECK-NOT: "call-params-num"
; Function Attrs: convergent
declare signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext, <4 x i32>, <4 x i16>) local_unnamed_addr #1

; Function Attrs: convergent
declare zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(%opencl.image2d_ro_t.5 addrspace(1)*, <2 x i32>, <4 x i16>) local_unnamed_addr #1

; Function Attrs: convergent
declare <4 x float> @_Z21work_group_reduce_minDv4_f(<4 x float>) local_unnamed_addr #2

; Function Attrs: convergent
declare <4 x double> @_Z21work_group_reduce_addDv4_d(<4 x double>) local_unnamed_addr #2

; Function Attrs: convergent
declare <4 x float> @_Z21work_group_reduce_minDv4_fDv4_j(<4 x float>, <4 x float>) local_unnamed_addr #2

; Function Attrs: convergent
declare <4 x double> @_Z21work_group_reduce_addDv4_dDv4_j(<4 x double>, <4 x double>) local_unnamed_addr #2

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="64" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVcN4u_test,_ZGVcM4u_test" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "opencl-vec-uniform-return" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent nounwind }
attributes #5 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #6 = { convergent nounwind "has-vplan-mask" "kernel-call-once" }
attributes #7 = { convergent nounwind "call-params-num"="1" "kernel-call-once" "kernel-convergent-call" }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"cl_doubles", !"cl_images"}
!3 = !{!"-cl-std=CL2.0"}
!4 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!5 = !{void (%opencl.image2d_ro_t.5 addrspace(1)*)* @test}
!6 = !{i32 1}
!7 = !{!"read_only"}
!8 = !{!"image2d_t"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{!"img"}
!13 = !{void (%opencl.image2d_ro_t.5 addrspace(1)*)* @_ZGVcN4u_test}
!14 = !{void (%opencl.image2d_ro_t.5 addrspace(1)*, <4 x i32>)* @_ZGVcM4u_test}
!15 = !{i1 true}
!16 = !{null}
!17 = !{i32 4}
