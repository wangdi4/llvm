; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases for stores that are marked 'volatile' which store pointers
; involving aggregate types.

; This case is storing a pointer-to-pointer so does not result in
; 'Volatile data' on the structure type.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01** %ppStruct) !dtrans_type !2 {
  store volatile %struct.test01* null, %struct.test01** %ppStruct
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


; This case is storing the entire structure with a volatile store, so
; will result in 'Volatile data' on the structure type.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* %pStruct) !dtrans_type !6 {
  %local = load %struct.test02, %struct.test02* %pStruct
  store volatile %struct.test02 %local, %struct.test02* %pStruct
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Volatile data | Whole structure reference{{ *$}}


; This case is storing the first member of the structure using that address
; of the structure itself with a volatile store, so will result in the
; 'Volatile data' on the structure type.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* %pStruct) !dtrans_type !9 {
  %pStruct.as.p32 = bitcast %struct.test03* %pStruct to i32*
  store volatile i32 0, i32* %pStruct.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Volatile data{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01**)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 2}  ; %struct.test01**
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 1, !3, !7}  ; void (%struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"F", i1 false, i32 1, !3, !10}  ; void (%struct.test03*)
!10 = !{!11, i32 1}  ; %struct.test03*
!11 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!12 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!14 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!12, !13, !14}
