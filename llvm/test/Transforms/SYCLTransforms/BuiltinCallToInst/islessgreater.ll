; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test_float(float %0) {
entry:
; CHECK: fcmp one float {{.*}}, 0.000000e+00
; CHECK-NEXT: zext i1 {{.*}} to i32
  %call = call i32 @_Z13islessgreaterff(float %0, float 0.000000e+00)
  ret void
}

declare i32 @_Z13islessgreaterff(float, float)

define void @test_float2(<2 x float> %0) {
entry:
; CHECK: fcmp one <2 x float> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <2 x i1> {{.*}} to <2 x i32>

  %call = call <2 x i32> @_Z13islessgreaterDv2_fS_(<2 x float> %0, <2 x float> zeroinitializer)
  ret void
}

declare <2 x i32> @_Z13islessgreaterDv2_fS_(<2 x float>, <2 x float>)

define void @test_float3(<3 x float> %0) {
entry:
; CHECK: fcmp one <3 x float> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <3 x i1> {{.*}} to <3 x i32>

  %call = call <3 x i32> @_Z13islessgreaterDv3_fS_(<3 x float> %0, <3 x float> zeroinitializer)
  ret void
}

declare <3 x i32> @_Z13islessgreaterDv3_fS_(<3 x float>, <3 x float>)

define void @test_float4(<4 x float> %0) {
entry:
; CHECK: fcmp one <4 x float> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <4 x i1> {{.*}} to <4 x i32>

  %call = call <4 x i32> @_Z13islessgreaterDv4_fS_(<4 x float> %0, <4 x float> zeroinitializer)
  ret void
}

declare <4 x i32> @_Z13islessgreaterDv4_fS_(<4 x float>, <4 x float>)

define void @test_float8(<8 x float> %0) {
entry:
; CHECK: fcmp one <8 x float> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <8 x i1> {{.*}} to <8 x i32>

  %call = call <8 x i32> @_Z13islessgreaterDv8_fS_(<8 x float> %0, <8 x float> zeroinitializer)
  ret void
}

declare <8 x i32> @_Z13islessgreaterDv8_fS_(<8 x float>, <8 x float>)

define void @test_float16(<16 x float> %0) {
entry:
; CHECK: fcmp one <16 x float> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <16 x i1> {{.*}} to <16 x i32>

  %call = call <16 x i32> @_Z13islessgreaterDv16_fS_(<16 x float> %0, <16 x float> zeroinitializer)
  ret void
}

declare <16 x i32> @_Z13islessgreaterDv16_fS_(<16 x float>, <16 x float>)

define void @test_double(double %0) {
entry:
; CHECK: fcmp one double {{.*}}, 0.000000e+00
; CHECK-NEXT: zext i1 {{.*}} to i32

  %call = call i32 @_Z13islessgreaterdd(double %0, double 0.000000e+00)
  ret void
}

declare i32 @_Z13islessgreaterdd(double, double)

define void @test_double2(<2 x double> %0) {
entry:
; CHECK: fcmp one <2 x double> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <2 x i1> {{.*}} to <2 x i64>

  %call = call <2 x i64> @_Z13islessgreaterDv2_dS_(<2 x double> %0, <2 x double> zeroinitializer)
  ret void
}

declare <2 x i64> @_Z13islessgreaterDv2_dS_(<2 x double>, <2 x double>)

define void @test_double3(<3 x double> %0) {
entry:
; CHECK: fcmp one <3 x double> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <3 x i1> {{.*}} to <3 x i64>

  %call = call <3 x i64> @_Z13islessgreaterDv3_dS_(<3 x double> %0, <3 x double> zeroinitializer)
  ret void
}

declare <3 x i64> @_Z13islessgreaterDv3_dS_(<3 x double>, <3 x double>)

define void @test_double4(<4 x double> %0) {
entry:
; CHECK: fcmp one <4 x double> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <4 x i1> {{.*}} to <4 x i64>

  %call = call <4 x i64> @_Z13islessgreaterDv4_dS_(<4 x double> %0, <4 x double> zeroinitializer)
  ret void
}

declare <4 x i64> @_Z13islessgreaterDv4_dS_(<4 x double>, <4 x double>)

define void @test_double8(<8 x double> %0) {
entry:
; CHECK: fcmp one <8 x double> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <8 x i1> {{.*}} to <8 x i64>

  %call = call <8 x i64> @_Z13islessgreaterDv8_dS_(<8 x double> %0, <8 x double> zeroinitializer)
  ret void
}

declare <8 x i64> @_Z13islessgreaterDv8_dS_(<8 x double>, <8 x double>)

define void @test_double16(<16 x double> %0) {
entry:
; CHECK: fcmp one <16 x double> {{.*}}, zeroinitializer
; CHECK-NEXT: sext <16 x i1> {{.*}} to <16 x i64>

  %call = call <16 x i64> @_Z13islessgreaterDv16_dS_(<16 x double> %0, <16 x double> zeroinitializer)
  ret void
}

declare <16 x i64> @_Z13islessgreaterDv16_dS_(<16 x double>, <16 x double>)

; DEBUGIFY-NOT: WARNING
