; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test for "Whole structure reference" safety flag set due to return instruction

; Return an instance of a structure.
%struct.test01 = type { i32, i32 }
define %struct.test01 @test2(%struct.test01 %s) {
  ret %struct.test01 %s
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Whole structure reference
; CHECK: End LLVMType: %struct.test01


; Return an array of structures
%struct.test02 = type { i8 }
define [4 x %struct.test02] @test02([4 x %struct.test02] %a) {
  ret [4 x %struct.test02] %a
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Whole structure reference
; CHECK: End LLVMType: %struct.test02


; Return a literal structure
; This case is included here to ensure that the analysis does not crash, and
; does not need to have any output checked because DTrans does not support
; transformations on literal structures.
define { i32, i32 } @test03() {
  ret { i32, i32 } { i32 0, i32 1 }
}

; Return a literal structure that embeds a structure that DTrans needs to track
; safety data on.
; TODO: In this case, the structure is nested inside a literal structure,
; but the original DTrans analysis and the new DTrans analysis does not treat
; the structure as being a 'Nested structure'. Seems that it should.
%struct.test04 = type { i8, i8 }
define { i8, %struct.test04 } @test04() {
  ret { i8, %struct.test04 } { i8 2, %struct.test04 { i8 1, i8 2 } }
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: Whole structure reference
; CHECK: End LLVMType: %struct.test04


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 0}  ; i8
!3 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!4 = !{!"S", %struct.test02 zeroinitializer, i32 1, !2} ; { i8 }
!5 = !{!"S", %struct.test04 zeroinitializer, i32 2, !2, !2} ; { i8, i8 }

!intel.dtrans.types = !{!3, !4, !5}
