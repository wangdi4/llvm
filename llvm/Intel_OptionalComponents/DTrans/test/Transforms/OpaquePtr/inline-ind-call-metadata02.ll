; This test verifies that intel_dtrans_type metadata is not missing
; on indirect calls during inlining.
;
; RUN: opt < %s -opaque-pointers -S -passes=inline 2>&1 | FileCheck %s

; CHECK: define ptr @bar(
; CHECK: [[FP:%.*]] = load ptr, ptr %fptr
; CHECK-NEXT:  call void [[FP]](ptr null, i32 0) [ "funclet"(token %i2) ], !intel_dtrans_type !0

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30145"

%MemoryManager = type { ptr }

define void @foo(i32 %reason, ptr %arg) {
entry:
  %i = load ptr, ptr %arg, align 8
  call void %i(ptr null, i32 0), !intel_dtrans_type !0
  ret void
}

define ptr @bar(ptr %fptr) personality ptr @__CxxFrameHandler3 {
entry:
  %call = invoke ptr @call1(i64 0, ptr null)
          to label %try.cont unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %entry
  %i = catchswitch within none [label %catch3, label %catch] unwind to caller

catch3:                                           ; preds = %catch.dispatch
  %i1 = catchpad within %i [ptr null, i32 1, ptr null]
  unreachable

catch:                                            ; preds = %catch.dispatch
  %i2 = catchpad within %i [ptr null, i32 0, ptr null]
  call void @foo(i32 0, ptr %fptr) [ "funclet"(token %i2) ]
  catchret from %i2 to label %try.cont

try.cont:                                         ; preds = %catch, %entry
  ret ptr null
}

declare i32 @__CxxFrameHandler3(...)
declare ptr @call1()

!0 = !{!"F", i1 false, i32 2, !1, !2, !3}
!1 = !{!"void", i32 0}
!2 = !{%MemoryManager zeroinitializer, i32 1}
!3 = !{i8 0, i32 1}

