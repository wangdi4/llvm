; RUN: llvm-as %s -o %t.bc
; RUN: opt -simplifycfg -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the the LLVM pass SimplifyCFG does not add new barrier instructions.
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
; CHECK: !opencl.kernels

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

@opencl_spmv_csr_vector_kernel_local_partialSums = internal addrspace(3) global [128 x float] zeroinitializer, align 16


declare i64 @_Z13get_global_idj(i32)




define void @spmv_csr_vector_kernel(float addrspace(1)* noalias nocapture %val, float addrspace(1)* noalias nocapture %vec, i32 addrspace(1)* noalias nocapture %cols, i32 addrspace(1)* noalias nocapture %rowDelimiters, i32 %dim, float addrspace(1)* noalias nocapture %out) nounwind {
  %1 = tail call i64 @_Z12get_local_idj(i32 0) nounwind
  %2 = trunc i64 %1 to i32
  %3 = and i32 %2, 31
  %4 = tail call i64 @_Z14get_local_sizej(i32 0) nounwind
  %5 = tail call i64 @get_group_id(i32 0) nounwind
  %6 = shl i64 %4, 27
  %7 = ashr i64 %6, 32
  %8 = mul i64 %7, %5
  %9 = sdiv i32 %2, 32
  %10 = zext i32 %9 to i64
  %11 = add i64 %8, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %2 to i64
  %14 = getelementptr inbounds [128 x float] addrspace(3)* @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %13
  store volatile float 0.000000e+000, float addrspace(3)* %14, align 4
  %15 = icmp slt i32 %12, %dim
  br i1 %15, label %16, label %83

; <label>:16                                      ; preds = %0
  %17 = sext i32 %12 to i64
  %18 = getelementptr inbounds i32 addrspace(1)* %rowDelimiters, i64 %17
  %19 = load i32 addrspace(1)* %18, align 4
  %20 = add nsw i32 %12, 1
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds i32 addrspace(1)* %rowDelimiters, i64 %21
  %23 = load i32 addrspace(1)* %22, align 4
  %24 = add nsw i32 %19, %3
  %25 = icmp slt i32 %24, %23
  br i1 %25, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %16
  %tmp6 = sext i32 %24 to i64
  %tmp9 = add i32 %24, 32
  %tmp10 = zext i32 %tmp9 to i64
  br label %26

; <label>:26                                      ; preds = %bb.nph, %26
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %26 ]
  %mySum.01 = phi float [ 0.000000e+000, %bb.nph ], [ %33, %26 ]
  %tmp = shl i64 %indvar, 5
  %tmp7 = add i64 %tmp6, %tmp
  %scevgep = getelementptr i32 addrspace(1)* %cols, i64 %tmp7
  %scevgep8 = getelementptr float addrspace(1)* %val, i64 %tmp7
  %27 = load i32 addrspace(1)* %scevgep, align 4
  %28 = load float addrspace(1)* %scevgep8, align 4
  %29 = sext i32 %27 to i64
  %30 = getelementptr inbounds float addrspace(1)* %vec, i64 %29
  %31 = load float addrspace(1)* %30, align 4
  %32 = fmul float %28, %31
  %33 = fadd float %mySum.01, %32
  %tmp11 = add i64 %tmp10, %tmp
  %tmp12 = trunc i64 %tmp11 to i32
  %34 = icmp slt i32 %tmp12, %23
  %indvar.next = add i64 %indvar, 1
  br i1 %34, label %26, label %._crit_edge

._crit_edge:                                      ; preds = %26, %16
  %mySum.0.lcssa = phi float [ 0.000000e+000, %16 ], [ %33, %26 ]
  store volatile float %mySum.0.lcssa, float addrspace(3)* %14, align 4
  tail call void @_Z7barrierj(i64 1) nounwind
  %35 = icmp ult i32 %3, 16
  br i1 %35, label %36, label %43

; <label>:36                                      ; preds = %._crit_edge
  %37 = add nsw i32 %2, 16
  %38 = sext i32 %37 to i64
  %39 = getelementptr inbounds [128 x float] addrspace(3)* @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %38
  %40 = load volatile float addrspace(3)* %39, align 4
  %41 = load volatile float addrspace(3)* %14, align 4
  %42 = fadd float %41, %40
  store volatile float %42, float addrspace(3)* %14, align 4
  br label %43

; <label>:43                                      ; preds = %36, %._crit_edge
  tail call void @_Z7barrierj(i64 1) nounwind
  %44 = icmp ult i32 %3, 8
  br i1 %44, label %45, label %52

; <label>:45                                      ; preds = %43
  %46 = add nsw i32 %2, 8
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds [128 x float] addrspace(3)* @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %47
  %49 = load volatile float addrspace(3)* %48, align 4
  %50 = load volatile float addrspace(3)* %14, align 4
  %51 = fadd float %50, %49
  store volatile float %51, float addrspace(3)* %14, align 4
  br label %52

; <label>:52                                      ; preds = %45, %43
  tail call void @_Z7barrierj(i64 1) nounwind
  %53 = icmp ult i32 %3, 4
  br i1 %53, label %54, label %61

; <label>:54                                      ; preds = %52
  %55 = add nsw i32 %2, 4
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds [128 x float] addrspace(3)* @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %56
  %58 = load volatile float addrspace(3)* %57, align 4
  %59 = load volatile float addrspace(3)* %14, align 4
  %60 = fadd float %59, %58
  store volatile float %60, float addrspace(3)* %14, align 4
  br label %61

; <label>:61                                      ; preds = %54, %52
  tail call void @_Z7barrierj(i64 1) nounwind
  %62 = icmp ult i32 %3, 2
  br i1 %62, label %63, label %70

; <label>:63                                      ; preds = %61
  %64 = add nsw i32 %2, 2
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds [128 x float] addrspace(3)* @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %65
  %67 = load volatile float addrspace(3)* %66, align 4
  %68 = load volatile float addrspace(3)* %14, align 4
  %69 = fadd float %68, %67
  store volatile float %69, float addrspace(3)* %14, align 4
  br label %70

; <label>:70                                      ; preds = %63, %61
  tail call void @_Z7barrierj(i64 1) nounwind
  %71 = icmp eq i32 %3, 0
  br i1 %71, label %72, label %79

; <label>:72                                      ; preds = %70
  %73 = add nsw i32 %2, 1
  %74 = sext i32 %73 to i64
  %75 = getelementptr inbounds [128 x float] addrspace(3)* @opencl_spmv_csr_vector_kernel_local_partialSums, i64 0, i64 %74
  %76 = load volatile float addrspace(3)* %75, align 4
  %77 = load volatile float addrspace(3)* %14, align 4
  %78 = fadd float %77, %76
  store volatile float %78, float addrspace(3)* %14, align 4
  br label %79

; <label>:79                                      ; preds = %72, %70
  %.pr = phi i1 [ %71, %72 ], [ false, %70 ]
  tail call void @_Z7barrierj(i64 1) nounwind
  br i1 %.pr, label %80, label %83

; <label>:80                                      ; preds = %79
  %81 = load volatile float addrspace(3)* %14, align 4
  %82 = getelementptr inbounds float addrspace(1)* %out, i64 %17
  store float %81, float addrspace(1)* %82, align 4
  ret void

; <label>:83                                      ; preds = %79, %0
  ret void
}







declare i64 @_Z12get_local_idj(i32)

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_group_id(i32)

declare void @_Z7barrierj(i64)

!opencl.kernels = !{!2}
!opencl_spmv_csr_vector_kernel_locals_anchor = !{!4}

!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, float addrspace(1)*)* @spmv_csr_vector_kernel, metadata !1, metadata !1, metadata !"float4",metadata !"float const __attribute__((address_space(1))) *restrict, float const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const, float __attribute__((address_space(1))) *restrict", metadata !"opencl_spmv_csr_vector_kernel_locals_anchor"}
!4 = metadata !{metadata !"opencl_spmv_csr_vector_kernel_local_partialSums"}
