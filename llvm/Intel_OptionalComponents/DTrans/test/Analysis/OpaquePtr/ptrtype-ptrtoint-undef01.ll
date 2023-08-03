; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test ptrtoint from 'undef' value. With opaque pointers, the
; 'undef' value will not have a distinct type. It may always
; be safe, but for now DTrans will accept this when the value
; does not have users.

%struct.test01 = type { i32, i32 }

define void @test() {
  %u = ptrtoint ptr undef to i64
  ret void
}

; CHECK: %u = ptrtoint ptr undef to i64
; CHECK-NEXT: LocalPointerInfo
; CHECK-NOT: <UNHANDLED>
; CHECK: ret void

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!2}
