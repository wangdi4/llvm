; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-outofboundsok=true -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-outofboundsok=true -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that returning a pointer which is the address of a field results in the
; "Field address taken return" safety flag.

; Test with returning a field address
%struct.test01 = type { i32, i32 }
define i32* @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %addr = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  ret i32* %addr
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Field address taken return{{ *$}}


; Test with returning a field from within a nested structure
%struct.test02a = type { i32, %struct.test02b }
%struct.test02b = type { i32, i32 }
define i32* @test02(%struct.test02a* %pStruct) !dtrans_type !7 {
  %addr = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 1, i32 1
  ret i32* %addr
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: 0)Field LLVM Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Nested structure | Field address taken return{{ *$}}


; Test with returning an address that is the nested structure's address
%struct.test03a = type { i32, %struct.test03b }
%struct.test03b = type { i32, i32 }
define %struct.test03b* @test03(%struct.test03a* %pStruct) !dtrans_type !11 {
  %addr = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 1
  ret %struct.test03b* %addr
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: 0)Field LLVM Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: %struct.test03b
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Contains nested structure | Field address taken return{{ *$}}

; This is marked as "Field address taken return" due to "-dtrans-outofboundsok=true",
; otherwise it is safe.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Nested structure | Field address taken return{{ *$}}


; Test with returning the address obtained using a GEPOperator
%struct.test04a = type { i32, %struct.test04b }
%struct.test04b = type { i32, i32 }
@var04 = internal global %struct.test04a zeroinitializer
define %struct.test04b* @test04() !dtrans_type !16 {
  ret %struct.test04b* getelementptr (%struct.test04a, %struct.test04a* @var04, i64 0, i32 1)
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: 0)Field LLVM Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: %struct.test04b
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Global instance | Contains nested structure | Field address taken return{{ *$}}

; This is marked as "Field address taken return" due to "-dtrans-outofboundsok=true",
; otherwise it is safe.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Global instance | Nested structure | Field address taken return{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; i32* (%struct.test01*)
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!7 = !{!"F", i1 false, i32 1, !3, !8}  ; i32* (%struct.test02a*)
!8 = !{!9, i32 1}  ; %struct.test02a*
!9 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!10 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!11 = !{!"F", i1 false, i32 1, !12, !13}  ; %struct.test03b* (%struct.test03a*)
!12 = !{!10, i32 1}  ; %struct.test03b*
!13 = !{!14, i32 1}  ; %struct.test03a*
!14 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!15 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!16 = !{!"F", i1 false, i32 0, !17}  ; %struct.test04b* ()
!17 = !{!15, i32 1}  ; %struct.test04b*
!18 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!19 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !6} ; { i32, %struct.test02b }
!20 = !{!"S", %struct.test02b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!21 = !{!"S", %struct.test03a zeroinitializer, i32 2, !1, !10} ; { i32, %struct.test03b }
!22 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!23 = !{!"S", %struct.test04a zeroinitializer, i32 2, !1, !15} ; { i32, %struct.test04b }
!24 = !{!"S", %struct.test04b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!18, !19, !20, !21, !22, !23, !24}
