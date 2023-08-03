; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerValue pass
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. No analysis data was collected to this group
;;  Group-B.1 Values analysis data collected is as follows
;;      2. Data was collected only for kernel "main"
;;      3. Only values "%g", "%m", "%n", "%q" and "%k.i1.vec" was collected to this group
;;      4. "%g" value has offset 0 in the special buffer
;;      4. "%m" value has offset 4 in the special buffer
;;      5. "%n" value has offset 8 in the special buffer
;;      5. "%q" value has offset 16 in the special buffer
;;      5. "%k.i1.vec" value has offset 64 in the special buffer
;;  Group-B.2 Values analysis data collected is as follows
;;      7. No analysis data was collected to this group
;;  Buffer Total Size is 128
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %y = xor i32 %x, %lid
  %g = trunc i32 %y to i4
  %m = trunc i32 %y to i17
  %n = zext i32 %y to i34
  %q = zext i32 %y to i67
  %k.i1 = icmp ne i32 %y, zeroinitializer
  %k.i1.vec = insertelement <10 x i1> undef, i1 %k.i1, i32 0
  br label %L1
L1:
  call void @_Z18work_group_barrierj(i32 1)
  %h = zext i4 %g to i32
  %r = zext i17 %m to i32
  %s = trunc i34 %n to i32
  %t = trunc i67 %q to i32
  %k.i32.vec = zext <10 x i1> %k.i1.vec to <10 x i32>
  br label %L2
L2:
  call void @dummy_barrier.()
  ret void
; CHECK: L0:
; CHECK:   %lid = call i32 @_Z12get_local_idj(i32 0)
; CHECK:   %y = xor i32 %x, %lid
; CHECK:   %g = trunc i32 %y to i4
; CHECK:   %m = trunc i32 %y to i17
; CHECK:   %n = zext i32 %y to i34
; CHECK:   %q = zext i32 %y to i67
; CHECK:   br label %L1
; CHECK: L1:
; CHECK:   call void @_Z18work_group_barrierj(i32 1)
; CHECK:   %h = zext i4 %g to i32
; CHECK:   %r = zext i17 %m to i32
; CHECK:   %s = trunc i34 %n to i32
; CHECK:   %t = trunc i67 %q to i32
; CHECK:   br label %L2
; CHECK: L2:
; CHECK:   call void @dummy_barrier.()
; CHECK:   ret void
}

; CHECK: Group-A Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Group-B.1 Values
; CHECK-NOT: +
; CHECK: +main
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -g (0)
; CHECK: -m (4)
; CHECK: -n (8)
; CHECK: -q (16)
; CHECK: -k.i1.vec (64)
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Function Equivalence Classes:
; CHECK-NEXT: [main]: main

; CHECK-NEXT: Buffer Total Size:
; CHECK-NEXT: leader(main) : (128)
; CHECK-NEXT: DONE

declare void @_Z18work_group_barrierj(i32)
declare void @dummy_barrier.()
declare i32 @_Z12get_local_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @main}
