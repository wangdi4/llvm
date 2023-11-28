; Test to verify that TLI mappings are inserted for @llvm.ldexp intrinsic calls
; when SVML library is available.

; RUN: opt -vector-library=SVML       -passes=inject-tli-mappings -S < %s | FileCheck %s

; CHECK-LABEL: @llvm.compiler.used = appending global
; CHECK-SAME:      [24 x ptr] [
; CHECK-SAME:        ptr @__svml_ldexp2,
; CHECK-SAME:        ptr @__svml_ldexp4,
; CHECK-SAME:        ptr @__svml_ldexp8,
; CHECK-SAME:        ptr @__svml_ldexp16,
; CHECK-SAME:        ptr @__svml_ldexp32,
; CHECK-SAME:        ptr @__svml_ldexp64,
; CHECK-SAME:        ptr @__svml_ldexpf2,
; CHECK-SAME:        ptr @__svml_ldexpf4,
; CHECK-SAME:        ptr @__svml_ldexpf8,
; CHECK-SAME:        ptr @__svml_ldexpf16,
; CHECK-SAME:        ptr @__svml_ldexpf32,
; CHECK-SAME:        ptr @__svml_ldexpf64 
; CHECK-SAME:      ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare double @llvm.ldexp.f64(double, i32) #0
declare float @llvm.ldexp.f32(float, i32) #0

define void @lldexp_f64(ptr nocapture %varray) {
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
; CHECK: call double @llvm.ldexp.f64.i32(double %{{.*}}, i32 %{{.*}}) #[[LDEXPF64:[0-9]+]]
  %call = tail call double @llvm.ldexp.f64(double %conv, i32 %tmp)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @lldexpf_f32(ptr nocapture %varray) {
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
; CHECK: call float @llvm.ldexp.f32.i32(float %{{.*}}, i32 %{{.*}}) #[[LDEXPF32:[0-9]+]]
  %call = tail call float @llvm.ldexp.f32(float %conv, i32 %tmp)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

; CHECK: attributes #[[LDEXPF64]] = { "vector-function-abi-variant"="_ZGV_LLVM_N2vv_llvm.ldexp.f64.i32(__svml_ldexp2),_ZGV_LLVM_N4vv_llvm.ldexp.f64.i32(__svml_ldexp4),_ZGV_LLVM_N8vv_llvm.ldexp.f64.i32(__svml_ldexp8),_ZGV_LLVM_N16vv_llvm.ldexp.f64.i32(__svml_ldexp16),_ZGV_LLVM_N32vv_llvm.ldexp.f64.i32(__svml_ldexp32),_ZGV_LLVM_N64vv_llvm.ldexp.f64.i32(__svml_ldexp64),_ZGV_LLVM_M2vv_llvm.ldexp.f64.i32(__svml_ldexp2_mask),_ZGV_LLVM_M4vv_llvm.ldexp.f64.i32(__svml_ldexp4_mask),_ZGV_LLVM_M8vv_llvm.ldexp.f64.i32(__svml_ldexp8_mask),_ZGV_LLVM_M16vv_llvm.ldexp.f64.i32(__svml_ldexp16_mask),_ZGV_LLVM_M32vv_llvm.ldexp.f64.i32(__svml_ldexp32_mask),_ZGV_LLVM_M64vv_llvm.ldexp.f64.i32(__svml_ldexp64_mask)" }
; CHECK: attributes #[[LDEXPF32]] = { "vector-function-abi-variant"="_ZGV_LLVM_N2vv_llvm.ldexp.f32.i32(__svml_ldexpf2),_ZGV_LLVM_N4vv_llvm.ldexp.f32.i32(__svml_ldexpf4),_ZGV_LLVM_N8vv_llvm.ldexp.f32.i32(__svml_ldexpf8),_ZGV_LLVM_N16vv_llvm.ldexp.f32.i32(__svml_ldexpf16),_ZGV_LLVM_N32vv_llvm.ldexp.f32.i32(__svml_ldexpf32),_ZGV_LLVM_N64vv_llvm.ldexp.f32.i32(__svml_ldexpf64),_ZGV_LLVM_M2vv_llvm.ldexp.f32.i32(__svml_ldexpf2_mask),_ZGV_LLVM_M4vv_llvm.ldexp.f32.i32(__svml_ldexpf4_mask),_ZGV_LLVM_M8vv_llvm.ldexp.f32.i32(__svml_ldexpf8_mask),_ZGV_LLVM_M16vv_llvm.ldexp.f32.i32(__svml_ldexpf16_mask),_ZGV_LLVM_M32vv_llvm.ldexp.f32.i32(__svml_ldexpf32_mask),_ZGV_LLVM_M64vv_llvm.ldexp.f32.i32(__svml_ldexpf64_mask)" }
