; RUN: opt -analyze -B-WIAnalysis -verify -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the WIRelatedValue pass
;; The case: kernel "main" with PHINode "%isOk", which one of its two entries
;;           is value "%y" that is related to work-item local id
;; The expected result:
;;      //TODO: 0. Kernel "main" was not changed
;;  WI related Values analysis data collected as follow
;;      1. "%isOk" is non-uniform value (i.e. WI related)
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %y = icmp ult i32 %lid, 0
  %check = icmp ult i32 %x, 0
  br i1 %check, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %isOk = phi i1 [ %y, %L1 ], [ true, %L2 ]
  call void @_Z7barrierj(i32 2)
  ret void
; CHECK: WI related Values
; CHECK: isOk is WI related
}

declare i32 @_Z12get_local_idj(i32)
declare void @_Z7barrierj(i32)

!opencl.kernels = !{!0}
!opencl.compiler.options = !{}

!0 = !{void (i32)* @main, !1, !1, !"", !"int", !"opencl_main_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0}
!3 = !{i32 3}
!4 = !{!"int"}
!5 = !{!"x"}
