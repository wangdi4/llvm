; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-WIAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; XFAIL: *

;;*****************************************************************************
;; This test checks the WIRelatedValue pass
;; The case: kernel "main" with PHINode "%isOk", which both entry values are uniform
;;           and entry basic blocks are related on a comparison between unifrom values
;; The expected result:
;;      //TODO: 0. Kernel "main" was not changed
;;  WI related Values analysis data collected as follow
;;      1. "%isOk" is uniform value (i.e. not WI related)
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f3
2:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  br i1 %check, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
  ret void
; CHECK: WI related Values
; CHECK: isOk is not WI related
}

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
