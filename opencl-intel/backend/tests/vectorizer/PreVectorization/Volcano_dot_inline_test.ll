
; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/runtime.bc -runtime=ocl -CLBltnPreVec %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"


; CHECK: @test_dot_product_f
; CHECK-NOT: _Z3dotff
; CHECK: ret void
define void @test_dot_product_f(float addrspace(1)* nocapture %src1, float addrspace(1)* nocapture %src2, float addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds float addrspace(1)* %src1, i32 %1
  %3 = load float addrspace(1)* %2, align 4
  %4 = getelementptr inbounds float addrspace(1)* %src2, i32 %1
  %5 = load float addrspace(1)* %4, align 4
  %6 = tail call float @_Z3dotff(float %3, float %5) nounwind
  %7 = getelementptr inbounds float addrspace(1)* %dst, i32 %1
  store float %6, float addrspace(1)* %7, align 4
  ret void
}

declare i32 @get_global_id(i32)

declare float @_Z3dotff(float, float)


; CHECK: @test_dot_product_f2
; CHECK-NOT: _Z3dotDv2_fS_
; CHECK: ret void
define void @test_dot_product_f2(<2 x float> addrspace(1)* nocapture %src1, <2 x float> addrspace(1)* nocapture %src2, float addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds <2 x float> addrspace(1)* %src1, i32 %1
  %3 = load <2 x float> addrspace(1)* %2, align 8
  %4 = getelementptr inbounds <2 x float> addrspace(1)* %src2, i32 %1
  %5 = load <2 x float> addrspace(1)* %4, align 8
  %6 = tail call float @_Z3dotDv2_fS_(<2 x float> %3, <2 x float> %5) nounwind
  %7 = getelementptr inbounds float addrspace(1)* %dst, i32 %1
  store float %6, float addrspace(1)* %7, align 4
  ret void
}

declare float @_Z3dotDv2_fS_(<2 x float>, <2 x float>)


; CHECK: @test_dot_product_f3
; CHECK-NOT: _Z3dotDv3_fS_
; CHECK: ret void
define void @test_dot_product_f3(<3 x float> addrspace(1)* nocapture %src1, <3 x float> addrspace(1)* nocapture %src2, float addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds <3 x float> addrspace(1)* %src1, i32 %1
  %3 = load <3 x float> addrspace(1)* %2, align 16
  %4 = getelementptr inbounds <3 x float> addrspace(1)* %src2, i32 %1
  %5 = load <3 x float> addrspace(1)* %4, align 16
  %6 = tail call float @_Z3dotDv3_fS_(<3 x float> %3, <3 x float> %5) nounwind
  %7 = getelementptr inbounds float addrspace(1)* %dst, i32 %1
  store float %6, float addrspace(1)* %7, align 4
  ret void
}

declare float @_Z3dotDv3_fS_(<3 x float>, <3 x float>)


; CHECK: @test_dot_product_f4
; CHECK-NOT: _Z3dotDv4_fS_
; CHECK: ret void
define void @test_dot_product_f4(<4 x float> addrspace(1)* nocapture %src1, <4 x float> addrspace(1)* nocapture %src2, float addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds <4 x float> addrspace(1)* %src1, i32 %1
  %3 = load <4 x float> addrspace(1)* %2, align 16
  %4 = getelementptr inbounds <4 x float> addrspace(1)* %src2, i32 %1
  %5 = load <4 x float> addrspace(1)* %4, align 16
  %6 = tail call float @_Z3dotDv4_fS_(<4 x float> %3, <4 x float> %5) nounwind
  %7 = getelementptr inbounds float addrspace(1)* %dst, i32 %1
  store float %6, float addrspace(1)* %7, align 4
  ret void
}

declare float @_Z3dotDv4_fS_(<4 x float>, <4 x float>)


; CHECK: @test_dot_product_d
; CHECK-NOT: _Z3dotdd
; CHECK: ret void
define void @test_dot_product_d(double addrspace(1)* nocapture %src1, double addrspace(1)* nocapture %src2, double addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds double addrspace(1)* %src1, i32 %1
  %3 = load double addrspace(1)* %2, align 8
  %4 = getelementptr inbounds double addrspace(1)* %src2, i32 %1
  %5 = load double addrspace(1)* %4, align 8
  %6 = tail call double @_Z3dotdd(double %3, double %5) nounwind
  %7 = getelementptr inbounds double addrspace(1)* %dst, i32 %1
  store double %6, double addrspace(1)* %7, align 8
  ret void
}

declare double @_Z3dotdd(double, double)


; CHECK: @test_dot_product_d2
; CHECK-NOT: _Z3dotDv2_dS_
; CHECK: ret void
define void @test_dot_product_d2(<2 x double> addrspace(1)* nocapture %src1, <2 x double> addrspace(1)* nocapture %src2, double addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds <2 x double> addrspace(1)* %src1, i32 %1
  %3 = load <2 x double> addrspace(1)* %2, align 16
  %4 = getelementptr inbounds <2 x double> addrspace(1)* %src2, i32 %1
  %5 = load <2 x double> addrspace(1)* %4, align 16
  %6 = tail call double @_Z3dotDv2_dS_(<2 x double> %3, <2 x double> %5) nounwind
  %7 = getelementptr inbounds double addrspace(1)* %dst, i32 %1
  store double %6, double addrspace(1)* %7, align 8
  ret void
}

declare double @_Z3dotDv2_dS_(<2 x double>, <2 x double>)


; CHECK: @test_dot_product_d3
; CHECK-NOT: _Z3dotDv3_dS_
; CHECK: ret void
define void @test_dot_product_d3(<3 x double> addrspace(1)* nocapture %src1, <3 x double> addrspace(1)* nocapture %src2, double addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds <3 x double> addrspace(1)* %src1, i32 %1
  %3 = load <3 x double> addrspace(1)* %2, align 32
  %4 = getelementptr inbounds <3 x double> addrspace(1)* %src2, i32 %1
  %5 = load <3 x double> addrspace(1)* %4, align 32
  %6 = tail call double @_Z3dotDv3_dS_(<3 x double> %3, <3 x double> %5) nounwind
  %7 = getelementptr inbounds double addrspace(1)* %dst, i32 %1
  store double %6, double addrspace(1)* %7, align 8
  ret void
}

declare double @_Z3dotDv3_dS_(<3 x double>, <3 x double>)


; CHECK: @test_dot_product_d4
; CHECK-NOT: _Z3dotDv4_dS_
; CHECK: ret void
define void @test_dot_product_d4(<4 x double> addrspace(1)* nocapture %src1, <4 x double> addrspace(1)* nocapture %src2, double addrspace(1)* nocapture %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = getelementptr inbounds <4 x double> addrspace(1)* %src1, i32 %1
  %3 = load <4 x double> addrspace(1)* %2, align 32
  %4 = getelementptr inbounds <4 x double> addrspace(1)* %src2, i32 %1
  %5 = load <4 x double> addrspace(1)* %4, align 32
  %6 = tail call double @_Z3dotDv4_dS_(<4 x double> %3, <4 x double> %5) nounwind
  %7 = getelementptr inbounds double addrspace(1)* %dst, i32 %1
  store double %6, double addrspace(1)* %7, align 8
  ret void
}

declare double @_Z3dotDv4_dS_(<4 x double>, <4 x double>)
