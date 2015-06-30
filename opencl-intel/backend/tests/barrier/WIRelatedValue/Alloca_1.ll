; RUN: opt -analyze -B-WIAnalysis -verify -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the WIRelatedValue pass
;; The case: kernel "main" with Alloca "%a", value "%p" pointer to %"a"
;;           and value "%x" a load from "%p"
;; The expected result:
;;      //TODO: 0. Kernel "main" was not changed
;;  WI related Values analysis data collected as follow
;;      1. "%a" is non-uniform value (i.e. WI related)
;;      2. "%x" is non-uniform value (i.e. WI related)
;;      3. "%p" is non-uniform value (i.e. WI related)
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %arg) nounwind {
  %a = alloca [4 x float], align 4
  %p = getelementptr [4 x float]* %a, i32 0, i32 0
  %x = load float* %p, align 4
  call void @_Z7barrierj(i32 2)
  ret void
; CHECK: %a = alloca [4 x float], align 4
; CHECK: %p = getelementptr [4 x float]* %a, i32 0, i32 0
; CHECK: %x = load float* %p, align 4
; CHECK: call void @_Z7barrierj(i32 2)
; CHECK: ret void
}

; CHECK: WI related Values
; CHECK: a is WI related
; CHECK: p is WI related
; CHECK: x is WI related

declare void @_Z7barrierj(i32)

!opencl.kernels = !{!0}
!opencl.compiler.options = !{}

!0 = !{void (i32)* @main, !1, !1, !"", !"int", !"opencl_main_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0}
!3 = !{i32 3}
!4 = !{!"int"}
!5 = !{!"x"}
