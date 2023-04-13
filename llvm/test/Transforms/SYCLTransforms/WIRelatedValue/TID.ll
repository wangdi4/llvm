; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-barrier-wi-analysis>' %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the WIRelatedValue pass
;; The expected result:
;;   WI related Values
;;   %lid is WI related
;;   %gid is WI related
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK-LABEL: @main
define void @main() #0 {
  %lid = call i64 @_Z12get_local_idj(i32 0)
  %gid = call i64 @_Z13get_global_idj(i32 0)
  call void @_Z18work_group_barrierj(i32 2)
  ret void
; CHECK: WI related Values
; CHECK: %lid is WI related
; CHECK: %gid is WI related
}

declare void @_Z18work_group_barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare i64 @_Z13get_global_idj(i32)

attributes #0 = { "sycl_kernel" }
