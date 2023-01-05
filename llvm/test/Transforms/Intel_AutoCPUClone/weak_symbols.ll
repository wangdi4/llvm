; RUN: opt -opaque-pointers -passes=auto-cpu-clone < %s -S | FileCheck %s

; The test checks that functions that have linkonce or weak linkage are not
; multiversioned.


; CHECK: define weak_odr dso_local i32 @_Z3foo_weak_odrv(i32 %a)
; CHECK-NOT: @_Z3foo_weak_odrv.A(i32 %a)
; CHECK-NOT: @_Z3foo_weak_odrv.X(i32 %a)
; CHECK-NOT: @_Z3foo_weak_odrv.resolver(i32 %a)

; CHECK: define weak dso_local i32 @_Z3foo_weakv(i32 %a)
; CHECK-NOT: @_Z3foo_weakv.A(i32 %a)
; CHECK-NOT: @_Z3foo_weakv.X(i32 %a)
; CHECK-NOT: @_Z3foo_weakv.resolver(i32 %a)

; CHECK: define linkonce dso_local i32 @_Z3foo_linkoncev(i32 %a)
; CHECK-NOT: @_Z3foo_linkoncev.A(i32 %a)
; CHECK-NOT: @_Z3foo_linkoncev.X(i32 %a)
; CHECK-NOT: @_Z3foo_linkoncev.resolver(i32 %a)

; CHECK: define linkonce_odr dso_local i32 @_Z3foo_linkonce_odrv(i32 %a)
; CHECK-NOT: @_Z3foo_linkonce_odrv.A(i32 %a)
; CHECK-NOT: @_Z3foo_linkonce_odrv.X(i32 %a)
; CHECK-NOT: @_Z3foo_linkonce_odrv.resolver(i32 %a)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define weak_odr dso_local i32 @_Z3foo_weak_odrv(i32 %a) !llvm.auto.cpu.dispatch !3 {
entry:
  ret i32 %a
}

define weak dso_local i32 @_Z3foo_weakv(i32 %a) !llvm.auto.cpu.dispatch !3 {
entry:
  ret i32 %a
}

define linkonce dso_local i32 @_Z3foo_linkoncev(i32 %a) !llvm.auto.cpu.dispatch !3 {
entry:
  ret i32 %a
}

define linkonce_odr dso_local i32 @_Z3foo_linkonce_odrv(i32 %a) !llvm.auto.cpu.dispatch !3 {
entry:
  ret i32 %a
}

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!""}
!3 = !{!4}
!4 = !{!"auto-cpu-dispatch-target", !"broadwell"}
