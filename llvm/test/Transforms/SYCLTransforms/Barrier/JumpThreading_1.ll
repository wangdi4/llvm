; RUN: opt -passes=sycl-kernel-add-function-attrs,jump-threading -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-function-attrs,jump-threading -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the the LLVM pass JumpThreading does not add new barrier instructions.
;; The case: kernel with barrier instructions (just before if()/if()-else basic blocks)
;; Related CQ ticket: CSSD100007107
;; TODO: reduce this test
;; The expected result:
;;      1. same number of barrier instructions
;;*****************************************************************************

; CHECK: @spmv_csr_vector_kernel
; CHECK-NOT: call void @_Z7barrierj(i64 1)
; CHECK: call void @_Z7barrierj(i64 1)
; CHECK: call void @_Z7barrierj(i64 1)
; CHECK: call void @_Z7barrierj(i64 1)
; CHECK: call void @_Z7barrierj(i64 1)
; CHECK: call void @_Z7barrierj(i64 1)
; CHECK: call void @_Z7barrierj(i64 1)
; CHECK-NOT: call void @_Z7barrierj(i64 1)
; CHECK: !sycl.kernels

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

@opencl_spmv_csr_vector_kernel_local_partialSums = internal addrspace(3) global [128 x float] zeroinitializer, align 16


declare i64 @_Z13get_global_idj(i32)

define void @spmv_csr_vector_kernel(ptr addrspace(1) noalias nocapture %val, ptr addrspace(1) noalias nocapture %vec, ptr addrspace(1) noalias nocapture %cols, ptr addrspace(1) noalias nocapture %rowDelimiters, i32 %dim, ptr addrspace(1) noalias nocapture %out) nounwind {
  %1 = call i64 @_Z12get_local_idj(i32 0) nounwind
  %2 = trunc i64 %1 to i32
  %3 = and i32 %2, 31
  %4 = call i64 @_Z14get_local_sizej(i32 0) nounwind
  %5 = call i64 @get_group_id(i32 0) nounwind
  %6 = shl i64 %4, 27
  %7 = ashr i64 %6, 32
  %8 = mul i64 %5, %7
  %9 = sdiv i32 %2, 32
  %10 = zext i32 %9 to i64
  %11 = add i64 %8, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %2 to i64
  %14 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %13
  store volatile float 0.000000e+000, ptr addrspace(3) %14, align 4
  %15 = icmp slt i32 %12, %dim
  br i1 %15, label %16, label %105

; <label>:16                                      ; preds = %0
  %17 = sext i32 %12 to i64
  %18 = getelementptr inbounds i32, ptr addrspace(1) %rowDelimiters, i64 %17
  %19 = load i32, ptr addrspace(1) %18, align 4
  %20 = add nsw i32 %12, 1
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds i32, ptr addrspace(1) %rowDelimiters, i64 %21
  %23 = load i32, ptr addrspace(1) %22, align 4
  %24 = add nsw i32 %19, %3
  br label %25

; <label>:25                                      ; preds = %27, %16
  %mySum.0 = phi float [ 0.000000e+000, %16 ], [ %38, %27 ]
  %j.0 = phi i32 [ %24, %16 ], [ %39, %27 ]
  %26 = icmp slt i32 %j.0, %23
  br i1 %26, label %27, label %40

; <label>:27                                      ; preds = %25
  %28 = sext i32 %j.0 to i64
  %29 = getelementptr inbounds i32, ptr addrspace(1) %cols, i64 %28
  %30 = load i32, ptr addrspace(1) %29, align 4
  %31 = sext i32 %j.0 to i64
  %32 = getelementptr inbounds float, ptr addrspace(1) %val, i64 %31
  %33 = load float, ptr addrspace(1) %32, align 4
  %34 = sext i32 %30 to i64
  %35 = getelementptr inbounds float, ptr addrspace(1) %vec, i64 %34
  %36 = load float, ptr addrspace(1) %35, align 4
  %37 = fmul float %33, %36
  %38 = fadd float %mySum.0, %37
  %39 = add nsw i32 %j.0, 32
  br label %25

; <label>:40                                      ; preds = %25
  %41 = sext i32 %2 to i64
  %42 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %41
  store volatile float %mySum.0, ptr addrspace(3) %42, align 4
  call void @_Z7barrierj(i64 1) nounwind
  %43 = icmp ult i32 %3, 16
  br i1 %43, label %44, label %53

; <label>:44                                      ; preds = %40
  %45 = add nsw i32 %2, 16
  %46 = sext i32 %45 to i64
  %47 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %46
  %48 = load volatile float, ptr addrspace(3) %47, align 4
  %49 = sext i32 %2 to i64
  %50 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %49
  %51 = load volatile float, ptr addrspace(3) %50, align 4
  %52 = fadd float %51, %48
  store volatile float %52, ptr addrspace(3) %50, align 4
  br label %53

; <label>:53                                      ; preds = %44, %40
  call void @_Z7barrierj(i64 1) nounwind
  %54 = icmp ult i32 %3, 8
  br i1 %54, label %55, label %64

; <label>:55                                      ; preds = %53
  %56 = add nsw i32 %2, 8
  %57 = sext i32 %56 to i64
  %58 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %57
  %59 = load volatile float, ptr addrspace(3) %58, align 4
  %60 = sext i32 %2 to i64
  %61 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %60
  %62 = load volatile float, ptr addrspace(3) %61, align 4
  %63 = fadd float %62, %59
  store volatile float %63, ptr addrspace(3) %61, align 4
  br label %64

; <label>:64                                      ; preds = %55, %53
  call void @_Z7barrierj(i64 1) nounwind
  %65 = icmp ult i32 %3, 4
  br i1 %65, label %66, label %75

; <label>:66                                      ; preds = %64
  %67 = add nsw i32 %2, 4
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %68
  %70 = load volatile float, ptr addrspace(3) %69, align 4
  %71 = sext i32 %2 to i64
  %72 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %71
  %73 = load volatile float, ptr addrspace(3) %72, align 4
  %74 = fadd float %73, %70
  store volatile float %74, ptr addrspace(3) %72, align 4
  br label %75

; <label>:75                                      ; preds = %66, %64
  call void @_Z7barrierj(i64 1) nounwind
  %76 = icmp ult i32 %3, 2
  br i1 %76, label %77, label %86

; <label>:77                                      ; preds = %75
  %78 = add nsw i32 %2, 2
  %79 = sext i32 %78 to i64
  %80 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %79
  %81 = load volatile float, ptr addrspace(3) %80, align 4
  %82 = sext i32 %2 to i64
  %83 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %82
  %84 = load volatile float, ptr addrspace(3) %83, align 4
  %85 = fadd float %84, %81
  store volatile float %85, ptr addrspace(3) %83, align 4
  br label %86

; <label>:86                                      ; preds = %77, %75
  call void @_Z7barrierj(i64 1) nounwind
  %87 = icmp eq i32 %3, 0
  br i1 %87, label %88, label %97

; <label>:88                                      ; preds = %86
  %89 = add nsw i32 %2, 1
  %90 = sext i32 %89 to i64
  %91 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %90
  %92 = load volatile float, ptr addrspace(3) %91, align 4
  %93 = sext i32 %2 to i64
  %94 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %93
  %95 = load volatile float, ptr addrspace(3) %94, align 4
  %96 = fadd float %95, %92
  store volatile float %96, ptr addrspace(3) %94, align 4
  br label %97

; <label>:97                                      ; preds = %88, %86
  call void @_Z7barrierj(i64 1) nounwind
  %98 = icmp eq i32 %3, 0
  br i1 %98, label %99, label %105

; <label>:99                                      ; preds = %97
  %100 = sext i32 %2 to i64
  %101 = getelementptr inbounds [128 x float], ptr addrspace(3) @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %100
  %102 = load volatile float, ptr addrspace(3) %101, align 4
  %103 = sext i32 %12 to i64
  %104 = getelementptr inbounds float, ptr addrspace(1) %out, i64 %103
  store float %102, ptr addrspace(1) %104, align 4
  ret void

; <label>:105                                     ; preds = %97, %0
  ret void
}


declare i64 @_Z12get_local_idj(i32)

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_group_id(i32)

declare void @_Z7barrierj(i64)

!sycl.kernels = !{!2}
!opencl_spmv_csr_vector_kernel_locals_anchor = !{!4}

!1 = !{i32 0, i32 0, i32 0}
!2 = !{ptr @spmv_csr_vector_kernel, !1, !1, !"float4",!"float const __attribute__((address_space(1))) *restrict, float const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const, float __attribute__((address_space(1))) *restrict", !"opencl_spmv_csr_vector_kernel_locals_anchor"}
!4 = !{!"opencl_spmv_csr_vector_kernel_local_partialSums"}
; DEBUGIFY-NOT: WARNING
