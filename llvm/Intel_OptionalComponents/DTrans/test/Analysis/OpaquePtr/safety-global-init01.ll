; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that "Has initializer list" safety flag is set on types that have global
; variables with non-zero initializers.

; This case does not have an initializer list.
%struct.test01 = type { i32, i32 }
@g_instance.test01 = internal global %struct.test01 zeroinitializer
define void @test01() {
  %pField1 = getelementptr %struct.test01, %struct.test01* @g_instance.test01, i64 0, i32 1
  store i32 1, i32* %pField1
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Global instance{{ *$}}


; This case is for a pointer, and does not have an initializer list.
%struct.test02 = type { i32, i32 }
@g_ptr.test02 = internal global %struct.test02* null, !dtrans_type !2
define void @test02() {
  %pStruct = load %struct.test02*, %struct.test02** @g_ptr.test02
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Global pointer{{ *$}}


; This case is for an initialized pointer, and zero initialized instance.
; Neither should trigger "Has initializer list".
%struct.test03 = type { i32, i32 }
@g_instance.test03 = internal global %struct.test03 zeroinitializer
@g_ptr.test03 = internal global %struct.test03* @g_instance.test03, !dtrans_type !4
define void @test03() {
  store %struct.test03* @g_instance.test03, %struct.test03** @g_ptr.test03
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Global pointer | Global instance{{ *$}}


; An instance with an initializer should trigger "Has initializer list"
%struct.test04 = type { i32, i32 }
@g_instance.test04 = internal global %struct.test04 { i32 2, i32 3 }
define void @test04() {
  %pField1 = getelementptr %struct.test04, %struct.test04* @g_instance.test04, i64 0, i32 1
  store i32 1, i32* %pField1
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: Safety data: Global instance | Has initializer list{{ *$}}


; An array of instances with initializers should trigger "Has initializer list"
%struct.test05 = type { i32, i32 }
@g_array.test05 = internal global [2 x %struct.test05] [ %struct.test05 { i32 0, i32 1 }, %struct.test05 { i32 2, i32 3 }]
define void @test05() {
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test05
; CHECK: Safety data: Global instance | Has initializer list | Global array{{ *$}}


; An array of pointers should not trigger "Has initializer list"
%struct.test06 = type { i32, i32 }
@g_instance.test06 = internal global %struct.test06 zeroinitializer
@g_array.test06 = internal global [2 x %struct.test06*] [ %struct.test06* @g_instance.test06, %struct.test06* @g_instance.test06], !dtrans_type !6
define void @test06() {
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test06
; CHECK: Safety data: Global pointer | Global instance{{ *$}}


; An instance initialized with undef should not trigger "Has initializer list"
%struct.test07 = type { i32, i32 }
@g_instance.test07 = internal global %struct.test07 undef
define void @test07() {
  %pField1 = getelementptr %struct.test07, %struct.test07* @g_instance.test07, i64 0, i32 1
  store i32 1, i32* %pField1
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test07
; CHECK: Safety data: Global instance{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 1}  ; %struct.test02*
!3 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!4 = !{!5, i32 1}  ; %struct.test03*
!5 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!6 = !{!"A", i32 2, !7}  ; [2 x %struct.test06*]
!7 = !{!8, i32 1}  ; %struct.test06*
!8 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!12 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!13 = !{!"S", %struct.test05 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!14 = !{!"S", %struct.test06 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!15 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!9, !10, !11, !12, !13, !14, !15}