; RUN: opt -passes=sycl-kernel-convert-vplan-mask %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-convert-vplan-mask %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test(ptr addrspace(1) noalias %img) local_unnamed_addr #0 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %call = tail call signext i16 @_Z23intel_sub_group_shufflesj(i16 signext 0, i32 0) #4
  %call1 = tail call zeroext i16 @_Z29intel_sub_group_block_read_us14ocl_image2d_roDv2_i(ptr addrspace(1) %img, <2 x i32> zeroinitializer) #4
  %call2 = tail call float @_Z21work_group_reduce_minf(float 0.000000e+00) #5
  %call3 = tail call double @_Z21work_group_reduce_addd(double 0.000000e+00) #5
  ret void
}

; Function Attrs: convergent
declare signext i16 @_Z23intel_sub_group_shufflesj(i16 signext, i32) local_unnamed_addr #1

; Function Attrs: convergent
declare zeroext i16 @_Z29intel_sub_group_block_read_us14ocl_image2d_roDv2_i(ptr addrspace(1), <2 x i32>) local_unnamed_addr #1

; Function Attrs: convergent
declare float @_Z21work_group_reduce_minf(float) local_unnamed_addr #2

; Function Attrs: convergent
declare double @_Z21work_group_reduce_addd(double) local_unnamed_addr #2

; Function Attrs: convergent norecurse nounwind
define void @_ZGVcN4u_test(ptr addrspace(1) noalias %img) local_unnamed_addr #0 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
; CHECK: call signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext zeroinitializer, <4 x i32> zeroinitializer, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: call zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img, <2 x i32> zeroinitializer, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: call <4 x float> @_Z21work_group_reduce_minDv4_f(<4 x float> zeroinitializer)
; CHECK: call <4 x double> @_Z21work_group_reduce_addDv4_d(<4 x double> zeroinitializer)
  %0 = call signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext zeroinitializer, <4 x i32> zeroinitializer, <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>) #6
  %1 = call zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img, <2 x i32> zeroinitializer, <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>) #6
  %2 = call <4 x float> @_Z21work_group_reduce_minDv4_f(<4 x float> zeroinitializer) #7
  %3 = call <4 x double> @_Z21work_group_reduce_addDv4_d(<4 x double> zeroinitializer) #7
  ret void
}

; Function Attrs: convergent norecurse nounwind
define void @_ZGVcM4u_test(ptr addrspace(1) noalias %img, <4 x i32> %mask) local_unnamed_addr #0 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %vec.mask = alloca <4 x i32>, align 16
  store <4 x i32> %mask, ptr %vec.mask, align 16
  %0 = sext i32 0 to i64
  %scalar.gep = getelementptr <4 x i32>, ptr %vec.mask, i64 0, i64 %0
  %wide.load = load <4 x i32>, ptr %scalar.gep, align 16
  %1 = icmp ne <4 x i32> %wide.load, zeroinitializer
  %maskext = sext <4 x i1> %1 to <4 x i16>
  %2 = call signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext zeroinitializer, <4 x i32> zeroinitializer, <4 x i16> %maskext) #6
; CHECK: %mask.i32. = sext <4 x i16> %maskext to <4 x i32>
; CHECK: call signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext zeroinitializer, <4 x i32> zeroinitializer, <4 x i32> %mask.i32.)
  %maskext2 = sext <4 x i1> %1 to <4 x i16>
  %3 = call zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img, <2 x i32> zeroinitializer, <4 x i16> %maskext2) #6
; CHECK: %mask.i32.1 = sext <4 x i16> %maskext2 to <4 x i32>
; CHECK: call zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img, <2 x i32> zeroinitializer, <4 x i32> %mask.i32.1)
  %maskext3 = sext <4 x i1> %1 to <4 x i32>
  %maskcast = bitcast <4 x i32> %maskext3 to <4 x float>
  %4 = call <4 x float> @_Z21work_group_reduce_minDv4_fDv4_j(<4 x float> zeroinitializer, <4 x float> %maskcast) #7
; CHECK: %mask.cast.i. = bitcast <4 x float> %maskcast to <4 x i32>
; CHECK: call <4 x float> @_Z21work_group_reduce_minDv4_fDv4_j(<4 x float> zeroinitializer, <4 x i32> %mask.cast.i.)
  %maskext4 = sext <4 x i1> %1 to <4 x i64>
  %maskcast5 = bitcast <4 x i64> %maskext4 to <4 x double>
  %5 = call <4 x double> @_Z21work_group_reduce_addDv4_dDv4_j(<4 x double> zeroinitializer, <4 x double> %maskcast5) #7
; CHECK: %mask.cast.i.2 = bitcast <4 x double> %maskcast5 to <4 x i64>
; CHECK: %mask.i32.3 = trunc <4 x i64> %mask.cast.i.2 to <4 x i32>
; CHECK: call <4 x double> @_Z21work_group_reduce_addDv4_dDv4_j(<4 x double> zeroinitializer, <4 x i32> %mask.i32.3)
  ret void
}

; CHECK: declare <4 x float> @_Z21work_group_reduce_minDv4_f(<4 x float>) local_unnamed_addr #[[ATTR2:[0-9]+]]
; CHECK: declare <4 x double> @_Z21work_group_reduce_addDv4_d(<4 x double>) local_unnamed_addr #[[ATTR2]]
; CHECK: declare signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext, <4 x i32>, <4 x i32>) #[[ATTR1:[0-9]+]]
; CHECK: declare zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1), <2 x i32>, <4 x i32>) #[[ATTR1]]
; CHECK: declare <4 x float> @_Z21work_group_reduce_minDv4_fDv4_j(<4 x float>, <4 x i32>) #[[ATTR2]]
; CHECK: declare <4 x double> @_Z21work_group_reduce_addDv4_dDv4_j(<4 x double>, <4 x i32>) #[[ATTR2]]

; CHECK: attributes #[[ATTR1]] = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
; CHECK: attributes #[[ATTR2]] = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "opencl-vec-uniform-return" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
; CHECK-NOT: "has-vplan-mask"
; CHECK-NOT: "call-params-num"

; Function Attrs: convergent
declare signext <4 x i16> @_Z23intel_sub_group_shuffleDv4_sDv4_jS0_(<4 x i16> signext, <4 x i32>, <4 x i16>) local_unnamed_addr #1

; Function Attrs: convergent
declare zeroext <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1), <2 x i32>, <4 x i16>) local_unnamed_addr #1

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
attributes #4 = { convergent nounwind }
attributes #5 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #6 = { convergent nounwind "has-vplan-mask" "kernel-call-once" }
attributes #7 = { convergent nounwind "call-params-num"="1" "kernel-call-once" "kernel-convergent-call" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{!"image2d_t"}
!2 = !{target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) zeroinitializer}
!3 = !{!"image2d_t", !"int4"}
!4 = !{target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) zeroinitializer, <4 x i32> zeroinitializer}

; DEBUGIFY-NOT: WARNING
