; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on function argument types. In this
; case the function metadata is not present, so the result
; analysis of the arguments should be flagged as UNHANDLED.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; In this case, the metadata for the function is intentionally omitted.
; This should cause the argument analysis to be treated as UNHANDLED.
; In the future, it may be possible to try to infer the type based on uses.
define internal void @test01(i32* %arg01) {
  %v1 = load i32, i32* %arg01
  ret void
}
; CHECK-LABEL:  Input Parameters: test01
; CHECK-NONOPAQUE:    Arg 0: i32* %arg01
; CHECK-OPAQUE:    Arg 0: ptr %arg01
; CHECK:    LocalPointerInfo:
; CHECK-SAME: UNHANDLED


; In this case, the metadata for the function is intentionally omitted.
; This should cause the argument analysis to be treated as UNHANDLED.
; In the future, it may be possible to try to infer the type based on uses.
%struct.test02 = type { i32, i32 }
define internal void @test02(%struct.test02** %arg02) {
  %v2 = load %struct.test02*, %struct.test02** %arg02
  ret void
}
; CHECK-LABEL:  Input Parameters: test02
; CHECK-NONOPAQUE:    Arg 0: %struct.test02** %arg02
; CHECK-OPAQUE:    Arg 0: ptr %arg02
; CHECK:    LocalPointerInfo:
; CHECK-SAME: UNHANDLED

; CHECK-NONOPAQUE:  %v2 = load %struct.test02*, %struct.test02** %arg02
; CHECK-OPAQUE:  %v2 = load ptr, ptr %arg02
; CHECK:    LocalPointerInfo:
; CHECK-SAME: DEPENDS ON UNHANDLED


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!2}
