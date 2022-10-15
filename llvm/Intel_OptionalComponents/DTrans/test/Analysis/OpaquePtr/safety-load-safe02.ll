; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test a load that involves multiple entries in the pointee element list.
; This also tests that 'unused' element gets cleared in the FieldInfo when the
; field is marked as 'read'

%struct.test01 = type { i32, i32, i32 }
define i32 @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pField0 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %pField2 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 2
  %pField = select i1 undef, i32* %pField0, i32* %pField2
  %value = load i32, i32* %pField
  ret i32 %value
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read ComplexUse{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Read ComplexUse{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!4}
