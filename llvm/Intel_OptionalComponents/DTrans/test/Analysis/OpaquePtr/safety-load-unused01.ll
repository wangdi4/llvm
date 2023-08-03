; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -dtrans-identify-unused-values=false -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NOUNUSED
; RUN: opt -whole-program-assume -intel-libirc-allowed -dtrans-identify-unused-values=true -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-UNUSED

; Test to check the behavior of setting 'UnusedValue' on a field that is read.

%struct.test01 = type { i32, i32, i32 }
@glob01 = internal global %struct.test01 zeroinitializer

define i32 @test01(i32 %in) {
  %pField0 = getelementptr %struct.test01, ptr @glob01, i64 0, i32 0
  %pField1 = getelementptr %struct.test01, ptr @glob01, i64 0, i32 1
  %pField2 = getelementptr %struct.test01, ptr @glob01, i64 0, i32 2
  store i32 %in, ptr %pField0
  store i32 %in, ptr %pField1
  store i32 %in, ptr %pField2

  %vField0 = load i32, ptr %pField0
  %vField1 = load i32, ptr %pField1
  %vField2 = load i32, ptr %pField2

  ; This use of field 1 should not prevent setting 'UnusedValue' when using
  ; '-dtrans-identify-unused-values=true' on the field because the result of
  ; this add instruction is not used.
  %unused = add i32 %vField0, %vField1

  %sum = add i32 %vField0, %vField2
  ret i32 %sum
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read Written{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK-NOUNUSED: Field info: Read Written{{ *$}}
; CHECK-UNUSED: Field info: Read Written UnusedValue{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Read Written{{ *$}}
; CHECK: Safety data: Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test01

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!2}
