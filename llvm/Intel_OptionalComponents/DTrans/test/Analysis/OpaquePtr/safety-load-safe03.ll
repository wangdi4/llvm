; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Load element zero of a nested structure using a pointer to the structure
%struct.test01a = type { %struct.test01b }
%struct.test01b = type { %struct.test01c, i64 }
%struct.test01c = type { i32, i32 }
define void @test01(%struct.test01a* %pStruct) !dtrans_type !5 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 0
  %pField.as.p32 = bitcast %struct.test01b* %pField to i32*
  %vField = load i32, i32* %pField.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Nested structure | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01c
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read UnusedValue{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Nested structure{{ *$}}


; Load element zero of a nested structure using a pointer to the structure, when
; element zero is an array
%struct.test02a = type { [2 x %struct.test02b] }
%struct.test02b = type { [6 x %struct.test02c], i64 }
%struct.test02c = type { i32, i32 }
define void @test02(%struct.test02a* %pStruct) !dtrans_type !13 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 0
  %pField.as.p32 = bitcast [2 x %struct.test02b]* %pField to i32*
  %vField = load i32, i32* %pField.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Nested structure | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02c
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read UnusedValue{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Nested structure{{ *$}}



!1 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{!"R", %struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!3 = !{i64 0, i32 0}  ; i64
!4 = !{i32 0, i32 0}  ; i32
!5 = !{!"F", i1 false, i32 1, !6, !7}  ; void (%struct.test01a*)
!6 = !{!"void", i32 0}  ; void
!7 = !{!8, i32 1}  ; %struct.test01a*
!8 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!9 = !{!"A", i32 2, !10}  ; [2 x %struct.test02b]
!10 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!11 = !{!"A", i32 6, !12}  ; [6 x %struct.test02c]
!12 = !{!"R", %struct.test02c zeroinitializer, i32 0}  ; %struct.test02c
!13 = !{!"F", i1 false, i32 1, !6, !14}  ; void (%struct.test02a*)
!14 = !{!15, i32 1}  ; %struct.test02a*
!15 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!16 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b }
!17 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !3} ; { %struct.test01c, i64 }
!18 = !{!"S", %struct.test01c zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!19 = !{!"S", %struct.test02a zeroinitializer, i32 1, !9} ; { [2 x %struct.test02b] }
!20 = !{!"S", %struct.test02b zeroinitializer, i32 2, !11, !3} ; { [6 x %struct.test02c], i64 }
!21 = !{!"S", %struct.test02c zeroinitializer, i32 2, !4, !4} ; { i32, i32 }

!dtrans_types = !{!16, !17, !18, !19, !20, !21}
