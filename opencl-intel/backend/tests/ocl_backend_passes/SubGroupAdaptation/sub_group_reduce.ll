; RUN: opt -sub-group-adaptation -verify -S < %s | FileCheck %s
;;*****************************************************************************
;; This test checks the SubGroupAdaptation pass
;; Four cases:
;;    sub_group_reduce_add
;;    sub_group_reduce_min
;;    sub_group_reduce_max
;;*****************************************************************************
; ModuleID = 'Program'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; CHECK: @sg_test_scan
; CHECK: entry
; CHECK: %call1 = call spir_func i32 @_Z21work_group_reduce_addi(i32 %2)
; CHECK: %call6 = call spir_func i32 @_Z21work_group_reduce_minj(i32 %7)
; CHECK: %call11 = call spir_func i64 @_Z21work_group_reduce_maxl(i64 %12)
; CHECK: %call16 = call spir_func i64 @_Z21work_group_reduce_addm(i64 %17)
; CHECK: %call21 = call spir_func float @_Z21work_group_reduce_minf(float %22)
; CHECK: %call26 = call spir_func double @_Z21work_group_reduce_maxd(double %27)

; Function Attrs: nounwind
define spir_kernel void @sg_test_scan(i32 addrspace(1)* %bIn, i32 addrspace(1)* %bOut, i32 addrspace(1)* %cIn, i32 addrspace(1)* %cOut, i64 addrspace(1)* %dIn, i64 addrspace(1)* %dOut, i64 addrspace(1)* %iIn, i64 addrspace(1)* %iOut, float addrspace(1)* %fIn, float addrspace(1)* %fOut, double addrspace(1)* %gIn, double addrspace(1)* %gOut) #0 {
entry:
  %bIn.addr = alloca i32 addrspace(1)*, align 8
  %bOut.addr = alloca i32 addrspace(1)*, align 8
  %cIn.addr = alloca i32 addrspace(1)*, align 8
  %cOut.addr = alloca i32 addrspace(1)*, align 8
  %dIn.addr = alloca i64 addrspace(1)*, align 8
  %dOut.addr = alloca i64 addrspace(1)*, align 8
  %iIn.addr = alloca i64 addrspace(1)*, align 8
  %iOut.addr = alloca i64 addrspace(1)*, align 8
  %fIn.addr = alloca float addrspace(1)*, align 8
  %fOut.addr = alloca float addrspace(1)*, align 8
  %gIn.addr = alloca double addrspace(1)*, align 8
  %gOut.addr = alloca double addrspace(1)*, align 8
  %tid = alloca i32, align 4
  store i32 addrspace(1)* %bIn, i32 addrspace(1)** %bIn.addr, align 8
  store i32 addrspace(1)* %bOut, i32 addrspace(1)** %bOut.addr, align 8
  store i32 addrspace(1)* %cIn, i32 addrspace(1)** %cIn.addr, align 8
  store i32 addrspace(1)* %cOut, i32 addrspace(1)** %cOut.addr, align 8
  store i64 addrspace(1)* %dIn, i64 addrspace(1)** %dIn.addr, align 8
  store i64 addrspace(1)* %dOut, i64 addrspace(1)** %dOut.addr, align 8
  store i64 addrspace(1)* %iIn, i64 addrspace(1)** %iIn.addr, align 8
  store i64 addrspace(1)* %iOut, i64 addrspace(1)** %iOut.addr, align 8
  store float addrspace(1)* %fIn, float addrspace(1)** %fIn.addr, align 8
  store float addrspace(1)* %fOut, float addrspace(1)** %fOut.addr, align 8
  store double addrspace(1)* %gIn, double addrspace(1)** %gIn.addr, align 8
  store double addrspace(1)* %gOut, double addrspace(1)** %gOut.addr, align 8
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %tid, align 4
  %0 = load i32* %tid, align 4
  %idxprom = sext i32 %0 to i64
  %1 = load i32 addrspace(1)** %bIn.addr, align 8
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %1, i64 %idxprom
  %2 = load i32 addrspace(1)* %arrayidx, align 4
  %call1 = call spir_func i32 @_Z20sub_group_reduce_addi(i32 %2)
  %3 = load i32* %tid, align 4
  %idxprom2 = sext i32 %3 to i64
  %4 = load i32 addrspace(1)** %bOut.addr, align 8
  %arrayidx3 = getelementptr inbounds i32 addrspace(1)* %4, i64 %idxprom2
  store i32 %call1, i32 addrspace(1)* %arrayidx3, align 4
  %5 = load i32* %tid, align 4
  %idxprom4 = sext i32 %5 to i64
  %6 = load i32 addrspace(1)** %cIn.addr, align 8
  %arrayidx5 = getelementptr inbounds i32 addrspace(1)* %6, i64 %idxprom4
  %7 = load i32 addrspace(1)* %arrayidx5, align 4
  %call6 = call spir_func i32 @_Z20sub_group_reduce_minj(i32 %7)
  %8 = load i32* %tid, align 4
  %idxprom7 = sext i32 %8 to i64
  %9 = load i32 addrspace(1)** %cOut.addr, align 8
  %arrayidx8 = getelementptr inbounds i32 addrspace(1)* %9, i64 %idxprom7
  store i32 %call6, i32 addrspace(1)* %arrayidx8, align 4
  %10 = load i32* %tid, align 4
  %idxprom9 = sext i32 %10 to i64
  %11 = load i64 addrspace(1)** %dIn.addr, align 8
  %arrayidx10 = getelementptr inbounds i64 addrspace(1)* %11, i64 %idxprom9
  %12 = load i64 addrspace(1)* %arrayidx10, align 8
  %call11 = call spir_func i64 @_Z20sub_group_reduce_maxl(i64 %12)
  %13 = load i32* %tid, align 4
  %idxprom12 = sext i32 %13 to i64
  %14 = load i64 addrspace(1)** %dOut.addr, align 8
  %arrayidx13 = getelementptr inbounds i64 addrspace(1)* %14, i64 %idxprom12
  store i64 %call11, i64 addrspace(1)* %arrayidx13, align 8
  %15 = load i32* %tid, align 4
  %idxprom14 = sext i32 %15 to i64
  %16 = load i64 addrspace(1)** %iIn.addr, align 8
  %arrayidx15 = getelementptr inbounds i64 addrspace(1)* %16, i64 %idxprom14
  %17 = load i64 addrspace(1)* %arrayidx15, align 8
  %call16 = call spir_func i64 @_Z20sub_group_reduce_addm(i64 %17)
  %18 = load i32* %tid, align 4
  %idxprom17 = sext i32 %18 to i64
  %19 = load i64 addrspace(1)** %iOut.addr, align 8
  %arrayidx18 = getelementptr inbounds i64 addrspace(1)* %19, i64 %idxprom17
  store i64 %call16, i64 addrspace(1)* %arrayidx18, align 8
  %20 = load i32* %tid, align 4
  %idxprom19 = sext i32 %20 to i64
  %21 = load float addrspace(1)** %fIn.addr, align 8
  %arrayidx20 = getelementptr inbounds float addrspace(1)* %21, i64 %idxprom19
  %22 = load float addrspace(1)* %arrayidx20, align 4
  %call21 = call spir_func float @_Z20sub_group_reduce_minf(float %22)
  %23 = load i32* %tid, align 4
  %idxprom22 = sext i32 %23 to i64
  %24 = load float addrspace(1)** %fOut.addr, align 8
  %arrayidx23 = getelementptr inbounds float addrspace(1)* %24, i64 %idxprom22
  store float %call21, float addrspace(1)* %arrayidx23, align 4
  %25 = load i32* %tid, align 4
  %idxprom24 = sext i32 %25 to i64
  %26 = load double addrspace(1)** %gIn.addr, align 8
  %arrayidx25 = getelementptr inbounds double addrspace(1)* %26, i64 %idxprom24
  %27 = load double addrspace(1)* %arrayidx25, align 8
  %call26 = call spir_func double @_Z20sub_group_reduce_maxd(double %27)
  %28 = load i32* %tid, align 4
  %idxprom27 = sext i32 %28 to i64
  %29 = load double addrspace(1)** %gOut.addr, align 8
  %arrayidx28 = getelementptr inbounds double addrspace(1)* %29, i64 %idxprom27
  store double %call26, double addrspace(1)* %arrayidx28, align 8
  ret void
}

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) #1

declare spir_func i32 @_Z20sub_group_reduce_addi(i32) #2

declare spir_func i32 @_Z20sub_group_reduce_minj(i32) #2

declare spir_func i64 @_Z20sub_group_reduce_maxl(i64) #2

declare spir_func i64 @_Z20sub_group_reduce_addm(i64) #2

declare spir_func float @_Z20sub_group_reduce_minf(float) #2

declare spir_func double @_Z20sub_group_reduce_maxd(double) #2


; CHECK: declare spir_func i64 @_Z13get_global_idj(i32)
; CHECK: declare spir_func i32 @_Z21work_group_reduce_addi(i32)
; CHECK: declare spir_func i32 @_Z21work_group_reduce_minj(i32)
; CHECK: declare spir_func i64 @_Z21work_group_reduce_maxl(i64)
; CHECK: declare spir_func i64 @_Z21work_group_reduce_addm(i64)
; CHECK: declare spir_func float @_Z21work_group_reduce_minf(float)
; CHECK: declare spir_func double @_Z21work_group_reduce_maxd(double)

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!8}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i64 addrspace(1)*, i64 addrspace(1)*, i64 addrspace(1)*, i64 addrspace(1)*, float addrspace(1)*, float addrspace(1)*, double addrspace(1)*, double addrspace(1)*)* @sg_test_scan, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int*", !"uint*", !"uint*", !"long*", !"long*", !"ulong*", !"ulong*", !"float*", !"float*", !"double*", !"double*"}
!4 = !{!"kernel_arg_base_type", !"int*", !"int*", !"uint*", !"uint*", !"long*", !"long*", !"ulong*", !"ulong*", !"float*", !"float*", !"double*", !"double*"}
!5 = !{!"kernel_arg_type_qual", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !""}
!6 = !{i32 1, i32 2}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"cl_doubles"}

;;;__attribute__((overloadable)) int sub_group_reduce_add(int);
;;;__attribute__((overloadable)) uint sub_group_reduce_min(uint);
;;;__attribute__((overloadable)) long sub_group_reduce_max(long);
;;;__attribute__((overloadable)) ulong sub_group_reduce_add(ulong);
;;;__attribute__((overloadable)) float sub_group_reduce_min(float);
;;;__attribute__((overloadable)) double sub_group_reduce_max(double);
;;;
;;; --- OpenCL source (compilation options: "-cl-std=CL2.0 -D__OPENCL_C_VERSION__=200"
;;;__kernel void sg_test_scan(__global int *bIn, __global int *bOut, __global uint *cIn,
;;;__global uint *cOut, __global long *dIn, __global long *dOut, __global ulong *iIn, __global ulong *iOut,
;;;__global float *fIn, __global float *fOut, __global double *gIn, __global double *gOut) {
;;;  int  tid = get_global_id(0);
;;;  bOut[tid] = sub_group_reduce_add(bIn[tid]);
;;;  cOut[tid] = sub_group_reduce_min(cIn[tid]);
;;;  dOut[tid] = sub_group_reduce_max(dIn[tid]);
;;;  iOut[tid] = sub_group_reduce_add(iIn[tid]);
;;;  fOut[tid] = sub_group_reduce_min(fIn[tid]);
;;;  gOut[tid] = sub_group_reduce_max(gIn[tid]);
;;;}