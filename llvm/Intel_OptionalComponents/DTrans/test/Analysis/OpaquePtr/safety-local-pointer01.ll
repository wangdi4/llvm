; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test setting of "Local pointer" safety bit


; Should identify as "Local pointer" when allocating pointer to structure.
%struct.test01 = type { i32, i32 }
define void @test01() {
  %local = alloca %struct.test01*, !dtrans_type !2
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Local pointer


; Should identify as "Local pointer" when allocating pointer to array
; of structures because this is not directly instantiating the structure.
%struct.test02 = type { i32, i32 }
define void @test02() {
  %local = alloca [4 x %struct.test02]*, !dtrans_type !4
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Local pointer


; Should identify as "Local pointer" when allocating array of
; pointers to structure.
%struct.test03 = type { i32, i32 }
define void @test03() {
  %local = alloca [4 x %struct.test03*], !dtrans_type !7
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Local pointer


; Should identify as "Local pointer" when allocating pointer to pointer
; to structure.
%struct.test04 = type { i32, i32 }
define void @test04() {
  %local = alloca %struct.test04**, !dtrans_type !10
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: Safety data: Local pointer


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!5, i32 1}  ; [4 x %struct.test02]*
!5 = !{!"A", i32 4, !6}  ; [4 x %struct.test02]
!6 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!7 = !{!"A", i32 4, !8}  ; [4 x %struct.test03*]
!8 = !{!9, i32 1}  ; %struct.test03*
!9 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!10 = !{!11, i32 2}  ; %struct.test04**
!11 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!12 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!14 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!15 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!12, !13, !14, !15}
