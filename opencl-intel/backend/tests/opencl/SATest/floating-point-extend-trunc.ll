; This test checks that the following floating-point conversion symbols are
; resolved and build is successful:
; __gnu_h2f_ieee, __gnu_f2h_ieee, __extenddftf2, __extendsftf2,
; __truncdfhf2, __trunctfdf2, __trunctfhf2, __trunctfsf2
;
; This file is frontend output of the following kernel with env
; VOLCANO_EQUALIZER_STATS=All and CL_CONFIG_DEVICES=fpga-emu:
;
; #pragma OPENCL EXTENSION cl_khr_fp16 : enable
; __kernel void test() {
;   long double ld1 = 1.0;
;   double d1 = ld1;
;   float f1 = ld1;
;   half h1 = ld1;
;
;   float f2 = d1;
;   half h2 = d1;
;
;   half h3 = f2;
;
;   float f4 = h3;
;   double d4 = h3;
;   long double ld4 = h3;
;
;   double d5 = f4;
;   long double ld5 = f4;
;
;   long double ld6 = d5;
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

; Function Attrs: convergent noinline norecurse nounwind
define spir_kernel void @test() #0 !kernel_arg_addr_space !10 !kernel_arg_access_qual !10 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !10 {
entry:
  %ld1 = alloca fp128, align 16
  %d1 = alloca double, align 8
  %f1 = alloca float, align 4
  %h1 = alloca half, align 2
  %f2 = alloca float, align 4
  %h2 = alloca half, align 2
  %h3 = alloca half, align 2
  %f4 = alloca float, align 4
  %d4 = alloca double, align 8
  %ld4 = alloca fp128, align 16
  %d5 = alloca double, align 8
  %ld5 = alloca fp128, align 16
  %ld6 = alloca fp128, align 16
  store fp128 0xL00000000000000003FFF000000000000, fp128* %ld1, align 16
  %0 = load fp128, fp128* %ld1, align 16
  %conv = fptrunc fp128 %0 to double
  store double %conv, double* %d1, align 8
  %1 = load fp128, fp128* %ld1, align 16
  %conv1 = fptrunc fp128 %1 to float
  store float %conv1, float* %f1, align 4
  %2 = load fp128, fp128* %ld1, align 16
  %conv2 = fptrunc fp128 %2 to half
  store half %conv2, half* %h1, align 2
  %3 = load double, double* %d1, align 8
  %conv3 = fptrunc double %3 to float
  store float %conv3, float* %f2, align 4
  %4 = load double, double* %d1, align 8
  %conv4 = fptrunc double %4 to half
  store half %conv4, half* %h2, align 2
  %5 = load float, float* %f2, align 4
  %conv5 = fptrunc float %5 to half
  store half %conv5, half* %h3, align 2
  %6 = load half, half* %h3, align 2
  %conv6 = fpext half %6 to float
  store float %conv6, float* %f4, align 4
  %7 = load half, half* %h3, align 2
  %conv7 = fpext half %7 to double
  store double %conv7, double* %d4, align 8
  %8 = load half, half* %h3, align 2
  %conv8 = fpext half %8 to fp128
  store fp128 %conv8, fp128* %ld4, align 16
  %9 = load float, float* %f4, align 4
  %conv9 = fpext float %9 to double
  store double %conv9, double* %d5, align 8
  %10 = load float, float* %f4, align 4
  %conv10 = fpext float %10 to fp128
  store fp128 %conv10, fp128* %ld5, align 16
  %11 = load double, double* %d5, align 8
  %conv11 = fpext double %11 to fp128
  store fp128 %conv11, fp128* %ld6, align 16
  ret void
}

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.stat.type = !{!5}
!opencl.stat.exec_time = !{!6}
!opencl.stat.run_time_version = !{!7}
!opencl.stat.workload_name = !{!8}
!opencl.stat.module_name = !{!9}

!0 = !{i32 1, i32 2}
!1 = !{!"cl_khr_fp16"}
!2 = !{!"cl_doubles"}
!3 = !{!"-cl-opt-disable"}
!4 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!5 = !{!""}
!6 = !{!"2020-04-28 19:03:19"}
!7 = !{!"2020.10.4.0"}
!8 = !{!"test-fp-conversions-fpga"}
!9 = !{!"test-fp-conversions-fpga1"}
!10 = !{}
