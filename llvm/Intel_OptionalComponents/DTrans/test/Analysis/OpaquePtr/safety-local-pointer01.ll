; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test setting of "Local pointer" safety bit


; Should identify as "Local pointer" when allocating pointer to structure.
%struct.test01 = type { i32, i32 }
define void @test01() {
  %local = alloca %struct.test01*, !intel_dtrans_type !2
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Local pointer
; CHECK: End LLVMType: %struct.test01


; Should identify as "Local pointer" when allocating pointer to array
; of structures because this is not directly instantiating the structure.
%struct.test02 = type { i32, i32 }
define void @test02() {
  %local = alloca [4 x %struct.test02]*, !intel_dtrans_type !3
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Local pointer
; CHECK: End LLVMType: %struct.test02


; Should identify as "Local pointer" when allocating array of
; pointers to structure.
%struct.test03 = type { i32, i32 }
define void @test03() {
  %local = alloca [4 x %struct.test03*], !intel_dtrans_type !6
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Local pointer
; CHECK: End LLVMType: %struct.test03


; Should identify as "Local pointer" when allocating pointer to pointer
; to structure.
%struct.test04 = type { i32, i32 }
define void @test04() {
  %local = alloca %struct.test04**, !intel_dtrans_type !8
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: Local pointer
; CHECK: End LLVMType: %struct.test04


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!4, i32 1}  ; [4 x %struct.test02]*
!4 = !{!"A", i32 4, !5}  ; [4 x %struct.test02]
!5 = !{%struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!6 = !{!"A", i32 4, !7}  ; [4 x %struct.test03*]
!7 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!8 = !{%struct.test04 zeroinitializer, i32 2}  ; %struct.test04**
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!12 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!9, !10, !11, !12}
