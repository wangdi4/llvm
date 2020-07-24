; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test load & store that involves multiple entries in the pointee element list
; to verify that it is safe, and the "Field info" is appropriately set.

%struct.test01 = type { i32, i32, i32 }
define i32 @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %pField0 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %pField1 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField2 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 2
  store i32 0, i32* %pField1
  %pField = select i1 undef, i32* %pField0, i32* %pField2
  %val = load i32, i32* %pField
  %inc = add i32 1, %val
  store i32 %inc, i32* %pField
  ret i32 %val
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read Written ComplexUse{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Read Written ComplexUse{{ *$}}
; CHECK: Safety data: No issues found


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !1, !3}  ; i32 (%struct.test01*)
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!dtrans_types = !{!5}
