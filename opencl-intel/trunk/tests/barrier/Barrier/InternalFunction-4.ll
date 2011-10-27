; RUN: llvm-as %s -o %t.bc
; RUN: opt -b-p  -print-module -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f3
2:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L1:
  call void @dummybarrier.()
  %lid = call i32 @get_local_id(i32 0)
  %y = xor i32 %x, %lid
  br label %L2
L2:
  call void @barrier(i32 1)
  %z = call i32 @foo(i32 %y)
  br label %L3
L3:
  call void @dummybarrier.()
  %w = and i32 %z, %z
  call void @barrier(i32 1)
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: xor
; CHECK: call i32 @foo_New
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: ret
}

; CHECK: @foo
define i32 @foo(i32 %x) nounwind {
L1:
  call void @dummybarrier.()
  br label %L2
L2:
  call void @barrier(i32 2)
  ret i32 0
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: ret
}

; CHECK: @foo_New(i32 %x, i32 %offset, i32 %offset1)
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: ret

declare void @barrier(i32)
declare i32 @get_local_id(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
