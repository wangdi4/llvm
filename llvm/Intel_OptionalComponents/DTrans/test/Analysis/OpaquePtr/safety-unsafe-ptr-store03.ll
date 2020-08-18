; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a store uses a pointer to the start of a structure, but
; stores a type that does not match the type that the structure begins with.
; These cases are casting a pointer to the structure to be a type that does not
; match the field type that starts the structure.


%struct.test01 = type { i32* }
define internal void @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %mem = call i8* @malloc(i64 128)
  %pStruct.as.pp8 = bitcast %struct.test01* %pStruct to i8**
  store i8* %mem, i8** %pStruct.as.pp8
  ret void
}
; This case gets treated as safe by DTrans because the i8* will be compatible
; with the element zero type of the structure.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


%struct.test02 = type { i32* }
define internal void @test02(%struct.test02* %pStruct) !dtrans_type !6 {
  %local = alloca i16
  %pStruct.as.pp16 = bitcast %struct.test02* %pStruct to i16**
  store i16* %local, i16** %pStruct.as.pp16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


%struct.test03 = type { i32*, i32* }
define internal void @test03(%struct.test03* %pStruct) !dtrans_type !9 {
  %local = alloca i64
  %pStruct.as.pp64 = bitcast %struct.test03* %pStruct to i64**
  store i64* %local, i64** %pStruct.as.pp64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}

declare i8* @malloc(i64)

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 1, !3, !7}  ; void (%struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"F", i1 false, i32 1, !3, !10}  ; void (%struct.test03*)
!10 = !{!11, i32 1}  ; %struct.test03*
!11 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!12 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32* }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 1, !1} ; { i32* }
!14 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32*, i32* }

!dtrans_types = !{!12, !13, !14}
