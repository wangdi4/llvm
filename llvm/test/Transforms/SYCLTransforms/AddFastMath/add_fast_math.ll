; RUN: opt -S -passes=sycl-kernel-add-fast-math %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes=sycl-kernel-add-fast-math %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @func(float %a, float %b, double %c, double %d, i32 %i) {
; CHECK-LABEL: define void @func
; CHECK-NEXT:  entry:
; CHECK-NEXT:  %0 = fadd fast float %a, %b
; CHECK-NEXT:  %1 = fsub fast float %a, %b
; CHECK-NEXT:  %2 = fmul fast float %a, %b
; CHECK-NEXT:  %3 = fdiv fast float %a, %b
; CHECK-NEXT:  %4 = frem fast float %a, %b
; CHECK-NEXT:  %5 = fcmp fast olt float %a, %b
; CHECK-NEXT:  %6 = fcmp fast ogt float %a, %b
; CHECK-NEXT:  %7 = call fast float @foo(i32 %i)
; CHECK-NEXT:  %8 = call fast <2 x float> @foo_vec(i32 %i)
; CHECK-NEXT:  %9 = fadd fast double %c, %d
; CHECK-NEXT:  %10 = fsub fast double %c, %d
; CHECK-NEXT:  %11 = fmul fast double %c, %d
; CHECK-NEXT:  %12 = fdiv fast double %c, %d
; CHECK-NEXT:  %13 = frem fast double %c, %d
; CHECK-NEXT:  %14 = fcmp fast ole double %c, %d
; CHECK-NEXT:  %15 = fcmp fast oge double %c, %d
; CHECK-NEXT:  %16 = call fast double @bar(i32 %i)
; CHECK-NEXT:  %17 = call fast <2 x double> @bar_vec(i32 %i)
; CHECK-NEXT:  %18 = fadd fast <2 x float> <float 1.000000e+00, float 2.000000e+00>, <float 2.000000e+00, float 3.000000e+00>
; CHECK-NEXT:  %19 = call i32 @baz(float %a)
; CHECK-NEXT:  ret void
entry:
  %0 = fadd float %a, %b
  %1 = fsub float %a, %b
  %2 = fmul float %a, %b
  %3 = fdiv float %a, %b
  %4 = frem float %a, %b
  %5 = fcmp olt float %a, %b
  %6 = fcmp ogt float %a, %b
  %7 = call float @foo(i32 %i)
  %8 = call <2 x float> @foo_vec(i32 %i)
  %9 = fadd double %c, %d
  %10 = fsub double %c, %d
  %11 = fmul double %c, %d
  %12 = fdiv double %c, %d
  %13 = frem double %c, %d
  %14 = fcmp ole double %c, %d
  %15 = fcmp oge double %c, %d
  %16 = call double @bar(i32 %i)
  %17 = call <2 x double> @bar_vec(i32 %i)
  %18 = fadd <2 x float> <float 1.000000e+00, float 2.000000e+00>, <float 2.000000e+00, float 3.000000e+00>
  %19 = call i32 @baz(float %a)
  ret void
}

declare float @foo(i32)
declare <2 x float> @foo_vec(i32)
declare double @bar(i32)
declare <2 x double> @bar_vec(i32)
declare i32 @baz(float)

; DEBUGIFY-NOT: WARNING
