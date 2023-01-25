; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-barrier-wi-analysis>' %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the WIRelatedValue pass can print unnamed values.
;; The case: kernel "main" with PHINode "%4", which one of its two entries
;;           is value "%2" that is related to work-item global id.
;; The expected result:
;;  WI related Values analysis data collected as follow
;;      "%2" is non-uniform value
;;      "%3" is non-uniform value
;;      "%4" is uniform value
;;      "  br i1 %4, label %L1, label %L2" is uniform value
;;      "%5" is non-uniform value (i.e. WI related)
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK-LABEL: @main
define void @main(i32 %0) #0 {
entry:
  %1 = call i32 @_Z13get_global_idj(i32 0)
  %2 = icmp ult i32 %1, 0
  %3 = icmp ult i32 %0, 0
  br i1 %3, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %4 = phi i1 [ %2, %L1 ], [ true, %L2 ]
  call void @_Z18work_group_barrierj(i32 2)
  ret void
; CHECK: WI related Values
; CHECK: %1 is WI related
; CHECK: %2 is WI related
; CHECK: %3 is not WI related
; CHECK: "  br i1 %3, label %L1, label %L2" is not WI related
; CHECK: %4 is WI related
}

declare i32 @_Z13get_global_idj(i32)
declare void @_Z18work_group_barrierj(i32)

attributes #0 = { "sycl_kernel" }
