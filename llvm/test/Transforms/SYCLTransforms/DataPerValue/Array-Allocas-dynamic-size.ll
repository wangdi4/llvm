; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerValue pass
;; The case: kernel "main" with the following alloca instructions (values):
;;      "%q" of i32 pointer with dynamic number of elements
;;      "%p" of i8 pointer with 3 element
;; The expected result:
;;      0. Kernel "main" was not changed
;;      1. "%q" value has offset 0 in the special buffer
;;      2. "%p" value has offset 16384 in the special buffer
;;      3. Buffer Total Size is 16400
;;*****************************************************************************


; ModuleID = './Array-Allocas-dynamic-size.ll'
source_filename = "./Array-Allocas-dynamic-size.ll"
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; Function Attrs: nounwind
; CHECK: @main
define void @main(i64 %x) #0 {
L0:
  %q = alloca i32, i64 %x, align 4
  %p = alloca ptr addrspace(1), i32 3, align 8
  ret void
; CHECK: L0:
; CHECK: %q = alloca i32, i64 %x, align 4
; CHECK: %p = alloca ptr addrspace(1), i32 3, align 8
; CHECK: ret void
}


; CHECK: -q (0)
; CHECK: -p (16384)

; CHECK: Function Equivalence Classes:
; CHECK-NEXT: [main]: main

; CHECK-NEXT: Buffer Total Size:
; CHECK-NEXT: leader(main) : (16400)
; CHECK-NEXT: DONE


!sycl.kernels = !{!0}

!0 = !{ptr @main}
