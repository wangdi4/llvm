; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-identify-unused-values=false -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-identify-unused-values=true -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that a whole structure load does not set the 'UnusedValue' flag on fields.

%struct.test01 = type { i32, i32 }
@glob01 = internal global %struct.test01 zeroinitializer

define void @test01(i32 %in) {
  %pField0 = getelementptr %struct.test01, %struct.test01* @glob01, i64 0, i32 0
  %pField1 = getelementptr %struct.test01, %struct.test01* @glob01, i64 0, i32 1
  store i32 %in, i32* %pField0
  store i32 %in, i32* %pField1

  ; The 'UnusedValue' flag will not be set because the unused field analysis
  ; is not run when the entire aggregate is loaded since that is a rare case,
  ; and the transformations are inhibited when the 'Whole structure reference'
  ; safety bit is set.
  %t = load %struct.test01, %struct.test01* @glob01
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read Written{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Read Written{{ *$}}
; CHECK: Safety data: Whole structure reference | Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!2}
