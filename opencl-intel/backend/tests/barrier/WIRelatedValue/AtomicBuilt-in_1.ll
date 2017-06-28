; RUN: opt -analyze -B-WIAnalysis -verify -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the WIRelatedValue pass
;; The case: kernel "main" with call to atomic built-in %a
;; The expected result:
;;      //TODO: 0. Kernel "main" was not changed
;;  WI related Values analysis data collected as follow
;;      1. "%a" is non-uniform value (i.e. WI related)
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 addrspace(1)* %arg) #0 {
  %a = call i32 @_Z25atomic_fetch_add_explicitPU3AS1VU7_Atomicjjjj(i32 addrspace(1)* %arg, i32 1, i32 0, i32 3) #0
  call void @_Z7barrierj(i32 2)
  ret void
; CHECK: %a = call i32 @_Z25atomic_fetch_add_explicitPU3AS1VU7_Atomicjjjj(i32 addrspace(1)* %arg, i32 1, i32 0, i32 3)
; CHECK: call void @_Z7barrierj(i32 2)
; CHECK: ret void
}

; CHECK: WI related Values
; CHECK: a is WI related

declare void @_Z7barrierj(i32)
declare i32 @_Z25atomic_fetch_add_explicitPU3AS1VU7_Atomicjjjj(i32 addrspace(1)*, i32, i32, i32) #0

attributes #0 = { nounwind }


!opencl.kernels = !{!0}
!opencl.compiler.options = !{}

!0 = !{void (i32 addrspace(1)*)* @main, !1, !1, !"", !"int", !"opencl_main_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0}
!3 = !{i32 3}
!4 = !{!"int"}
!5 = !{!"x"}
