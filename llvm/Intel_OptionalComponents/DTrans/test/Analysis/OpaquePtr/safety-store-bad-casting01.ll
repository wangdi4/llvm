; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Store a pointer to a structure to a location that expects a pointer to a
; different type of structure.

; This case will not be able to resolve a dominant aggregate type for the value
; operand of the store because it is declared as one type, but used as another.
%struct.test01a = type { i64*, i64*, i64* }
%struct.test01b = type { i64, i64, i64 }
define i64 @test01(%struct.test01a** %ppStructA) !dtrans_type !3 {
  %pStructB = alloca %struct.test01b
  %pStructB.as.A = bitcast %struct.test01b* %pStructB to %struct.test01a*
  store %struct.test01a* %pStructB.as.A, %struct.test01a** %ppStructA
  ret i64 0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}


; This case will not be able to resolve a dominant type for the pointer operand
; of the store because it is declared at one level of indirection, but used at a
; different level of indirection.
%struct.test02a = type { i64*, i64*, i64* }
define i64 @test02(%struct.test02a*** %pppStructA) !dtrans_type !6 {
  %pStructA = alloca %struct.test02a
  %ppStructA = bitcast %struct.test02a*** %pppStructA to %struct.test02a**
  store %struct.test02a* %pStructA, %struct.test02a** %ppStructA
  ret i64 0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}


; This case stores an arbitrary i64 value to a location that should hold a
; pointer to an aggregate type.
%struct.test03a = type { i64, i64, i64 }
define i64 @test03(%struct.test03a** %ppStructA, i64 %value) !dtrans_type !9 {
  %ppStructA.as.pi64 = bitcast %struct.test03a** %ppStructA to i64*
  store i64 %value, i64* %ppStructA.as.pi64
  ret i64 0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Bad casting{{ *$}}


!1 = !{i64 0, i32 1}  ; i64*
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"F", i1 false, i32 1, !2, !4}  ; i64 (%struct.test01a**)
!4 = !{!5, i32 2}  ; %struct.test01a**
!5 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!6 = !{!"F", i1 false, i32 1, !2, !7}  ; i64 (%struct.test02a***)
!7 = !{!8, i32 3}  ; %struct.test02a***
!8 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!9 = !{!"F", i1 false, i32 2, !2, !10, !2}  ; i64 (%struct.test03a**, i64)
!10 = !{!11, i32 2}  ; %struct.test03a**
!11 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!12 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i64*, i64*, i64* }
!13 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i64, i64, i64 }
!14 = !{!"S", %struct.test02a zeroinitializer, i32 3, !1, !1, !1} ; { i64*, i64*, i64* }
!15 = !{!"S", %struct.test03a zeroinitializer, i32 3, !2, !2, !2} ; { i64, i64, i64 }

!dtrans_types = !{!12, !13, !14, !15}
