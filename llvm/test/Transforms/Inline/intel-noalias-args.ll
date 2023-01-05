; RUN: opt -passes=inline -enable-noalias-to-md-conversion -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @bar(i32* nocapture %p, i32* nocapture %q) {
entry:
  %ptr = load i32, i32* %q
  store i32 %ptr, i32* %p
  ret void
}

; CHECK-LABEL: @foo
define dso_local void @foo(i32* nocapture %p, i32* nocapture %q) {
entry:
  call void @bar(i32* %p, i32* %q), !intel.args.alias.scope !0
; CHECK:   [[PTR:%ptr.*]] = load i32, i32* %q, align 4, !alias.scope !0, !noalias !3
; CHECK:   store i32 [[PTR]], i32* %p, align 4, !alias.scope !3, !noalias !0
  ret void
}

!0 = !{!1, !4}
!1 = !{!2}
!2 = distinct !{!2, !3, !"foo: rp"}
!3 = distinct !{!3, !"foo"}
!4 = !{!5}
!5 = distinct !{!5, !3, !"foo: rq"}

; CHECK: !0 = !{!1}
; CHECK: !1 = distinct !{!1, !2, !"foo: rq"}
; CHECK: !2 = distinct !{!2, !"foo"}
; CHECK: !3 = !{!4}
; CHECK: !4 = distinct !{!4, !2, !"foo: rp"}

