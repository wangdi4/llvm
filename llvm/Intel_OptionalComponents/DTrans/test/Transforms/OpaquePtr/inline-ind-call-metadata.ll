; This test verifies that intel_dtrans_type metadata is not missing
; on indirect calls during inlining.
;
; RUN: opt < %s -S -passes=always-inline 2>&1 | FileCheck %s

; CHECK: [[FP:%.*]] = load ptr, ptr @fptr
; CHECK-NEXT:  invoke i32 [[FP]](ptr null)
; CHECK-NEXT: !intel_dtrans_type !0
; CHECK-NOT: ptr @foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.XalanNode" = type { ptr }
@fptr = internal global ptr null, align 8

define linkonce_odr ptr @foo(ptr %node) alwaysinline {
whpr.continue:
  %fp = load ptr, ptr @fptr, align 8
  %call = call i32 %fp(ptr null), !intel_dtrans_type !0
  ret ptr null
}

define void @bar() personality ptr undef {
invoke.cont3:
  br label %if.end

lpad:                                             ; preds = %if.end23
  %0 = landingpad { ptr, i32 }
          cleanup
  br label %if.end

if.end:                                           ; preds = %lpad, %invoke.cont3
  br label %if.end23

if.end23:                                         ; preds = %if.end
  %call25 = invoke ptr @foo(ptr null)
          to label %invoke.cont24 unwind label %lpad

invoke.cont24:                                    ; preds = %if.end23
  ret void
}

!0 = !{!"F", i1 false, i32 1, !1, !2}
!1 = !{i32 0, i32 0}
!2 = !{%"class.XalanNode" zeroinitializer, i32 1}
