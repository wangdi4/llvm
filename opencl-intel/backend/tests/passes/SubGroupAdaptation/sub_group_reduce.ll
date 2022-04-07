; RUN: %oclopt -sub-group-adaptation -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -sub-group-adaptation -verify -S < %s | FileCheck %s
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
; CHECK: call spir_func i32 @_Z21work_group_reduce_addi(i32 %0)
; CHECK: call spir_func i32 @_Z21work_group_reduce_minj(i32 %1)
; CHECK: call spir_func i64 @_Z21work_group_reduce_maxl(i64 %2)
; CHECK: call spir_func i64 @_Z21work_group_reduce_addm(i64 %3)
; CHECK: call spir_func float @_Z21work_group_reduce_minf(float %4)
; CHECK: call spir_func double @_Z21work_group_reduce_maxd(double %5)

; Function Attrs: nounwind
define spir_kernel void @sg_test_scan(i32 addrspace(1)* nocapture readonly %bIn, i32 addrspace(1)* nocapture %bOut, i32 addrspace(1)* nocapture readonly %cIn, i32 addrspace(1)* nocapture %cOut, i64 addrspace(1)* nocapture readonly %dIn, i64 addrspace(1)* nocapture %dOut, i64 addrspace(1)* nocapture readonly %iIn, i64 addrspace(1)* nocapture %iOut, float addrspace(1)* nocapture readonly %fIn, float addrspace(1)* nocapture %fOut, double addrspace(1)* nocapture readonly %gIn, double addrspace(1)* nocapture %gOut) #0 {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %bIn, i64 %idxprom
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !11
  %call1 = tail call spir_func i32 @_Z20sub_group_reduce_addi(i32 %0) #4
  %arrayidx3 = getelementptr inbounds i32, i32 addrspace(1)* %bOut, i64 %idxprom
  store i32 %call1, i32 addrspace(1)* %arrayidx3, align 4, !tbaa !11
  %arrayidx5 = getelementptr inbounds i32, i32 addrspace(1)* %cIn, i64 %idxprom
  %1 = load i32, i32 addrspace(1)* %arrayidx5, align 4, !tbaa !11
  %call6 = tail call spir_func i32 @_Z20sub_group_reduce_minj(i32 %1) #4
  %arrayidx8 = getelementptr inbounds i32, i32 addrspace(1)* %cOut, i64 %idxprom
  store i32 %call6, i32 addrspace(1)* %arrayidx8, align 4, !tbaa !11
  %arrayidx10 = getelementptr inbounds i64, i64 addrspace(1)* %dIn, i64 %idxprom
  %2 = load i64, i64 addrspace(1)* %arrayidx10, align 8, !tbaa !15
  %call11 = tail call spir_func i64 @_Z20sub_group_reduce_maxl(i64 %2) #4
  %arrayidx13 = getelementptr inbounds i64, i64 addrspace(1)* %dOut, i64 %idxprom
  store i64 %call11, i64 addrspace(1)* %arrayidx13, align 8, !tbaa !15
  %arrayidx15 = getelementptr inbounds i64, i64 addrspace(1)* %iIn, i64 %idxprom
  %3 = load i64, i64 addrspace(1)* %arrayidx15, align 8, !tbaa !15
  %call16 = tail call spir_func i64 @_Z20sub_group_reduce_addm(i64 %3) #4
  %arrayidx18 = getelementptr inbounds i64, i64 addrspace(1)* %iOut, i64 %idxprom
  store i64 %call16, i64 addrspace(1)* %arrayidx18, align 8, !tbaa !15
  %arrayidx20 = getelementptr inbounds float, float addrspace(1)* %fIn, i64 %idxprom
  %4 = load float, float addrspace(1)* %arrayidx20, align 4, !tbaa !17
  %call21 = tail call spir_func float @_Z20sub_group_reduce_minf(float %4) #4
  %arrayidx23 = getelementptr inbounds float, float addrspace(1)* %fOut, i64 %idxprom
  store float %call21, float addrspace(1)* %arrayidx23, align 4, !tbaa !17
  %arrayidx25 = getelementptr inbounds double, double addrspace(1)* %gIn, i64 %idxprom
  %5 = load double, double addrspace(1)* %arrayidx25, align 8, !tbaa !19
  %call26 = tail call spir_func double @_Z20sub_group_reduce_maxd(double %5) #4
  %arrayidx28 = getelementptr inbounds double, double addrspace(1)* %gOut, i64 %idxprom
  store double %call26, double addrspace(1)* %arrayidx28, align 8, !tbaa !19
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

; CHECK-NOT: declare spir_func i32 @_Z20sub_group_reduce_addi(i32)
; CHECK: declare spir_func i32 @_Z21work_group_reduce_addi(i32)
; CHECK-NOT: declare spir_func i32 @_Z20sub_group_reduce_minj(i32)
; CHECK: declare spir_func i32 @_Z21work_group_reduce_minj(i32)
; CHECK-NOT: declare spir_func i64 @_Z20sub_group_reduce_maxl(i64)
; CHECK: declare spir_func i64 @_Z21work_group_reduce_maxl(i64)
; CHECK-NOT: declare spir_func i64 @_Z20sub_group_reduce_addm(i64)
; CHECK: declare spir_func i64 @_Z21work_group_reduce_addm(i64)
; CHECK-NOT: declare spir_func float @_Z20sub_group_reduce_minf(float)
; CHECK: declare spir_func float @_Z21work_group_reduce_minf(float)
; CHECK-NOT: declare spir_func double @_Z20sub_group_reduce_maxd(double)
; CHECK: declare spir_func double @_Z21work_group_reduce_maxd(double)

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!sycl.kernels = !{!0}
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
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
!15 = !{!16, !16, i64 0}
!16 = !{!"long", !13, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"float", !13, i64 0}
!19 = !{!20, !20, i64 0}
!20 = !{!"double", !13, i64 0}

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

; DEBUGIFY-NOT: WARNING
