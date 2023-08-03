; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-barrier-wi-analysis>' %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the WIRelatedValue pass
;; The case: kernel "main" with PHINode "%isOk", which one of its two entries
;;           is value "%y" that is related to work-item global id
;; The expected result:
;;      //TODO: 0. Kernel "main" was not changed
;;  WI related Values analysis data collected as follow
;;      1. "%isOk" is non-uniform value (i.e. WI related)
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK-LABEL: @main
define void @main(i32 %x) {
  %gid = call i32 @_Z13get_global_idj(i32 0)
  %y = icmp ult i32 %gid, 0
  %check = icmp ult i32 %x, 0
  br i1 %check, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %isOk = phi i1 [ %y, %L1 ], [ true, %L2 ]
  call void @_Z18work_group_barrierj(i32 2)
  ret void
; CHECK: WI related Values
; CHECK: %isOk is WI related
}

declare i32 @_Z13get_global_idj(i32)
declare void @_Z18work_group_barrierj(i32)

!sycl.kernels = !{!0}
!0 = !{ptr @main}
