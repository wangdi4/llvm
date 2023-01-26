; RUN: opt < %s -passes='sroa<intel-preserve-cfg>' -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='sroa<intel-modify-cfg>' -S 2>&1 | FileCheck %s

; This test checks that SROA was not applied to the functions @test0 when
; the options intel-preserve-cfg and intel-modify-cfg were set. The reason
; is that the attribute "inlinehint" is not set for the function.

target datalayout = "e-p:64:64:64-p1:16:16:16-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-n8:16:32:64"

declare void @llvm.lifetime.start.p0(i64, ptr nocapture)
declare void @llvm.lifetime.end.p0(i64, ptr nocapture)

; CHECK-LABEL: @test0(
; CHECK:  entry:
; CHECK: store float 0.000000e+00, ptr %a2
; CHECK:  %v2 = load float, ptr %a2
; CHECK:  %v2.int = bitcast float %v2 to i32
; CHECK:  %sum1 = add i32 %v1, %v2.int

define i32 @test0() {
entry:
  %a1 = alloca i32
  %a2 = alloca float

  call void @llvm.lifetime.start.p0(i64 4, ptr %a1)

  store i32 0, ptr %a1
  %v1 = load i32, ptr %a1

  call void @llvm.lifetime.end.p0(i64 4, ptr %a1)

  call void @llvm.lifetime.start.p0(i64 4, ptr %a2)

  store float 0.0, ptr %a2
  %v2 = load float , ptr %a2
  %v2.int = bitcast float %v2 to i32
  %sum1 = add i32 %v1, %v2.int

  call void @llvm.lifetime.end.p0(i64 4, ptr %a2)

  ret i32 %sum1
}