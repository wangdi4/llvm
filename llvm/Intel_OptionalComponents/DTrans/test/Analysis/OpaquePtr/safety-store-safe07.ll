; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-outofboundsok=false -dtrans-print-types -disable-output %s 2>&1 | FileCheck -match-full-lines %s
; RUN: opt -dtransop-allow-typed-pointers -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-outofboundsok=false -dtrans-print-types -disable-output %s 2>&1 | FileCheck -match-full-lines %s

target triple = "x86_64-unknown-linux-gnu"

; Verify storing to element zero of a structure that is contained within an array
; using the address of an array element results in the field being set as "Written"
; and "NonGEPAccess"

%struct.test = type { i64, i64, i64 }
@gStructArray = internal global [10 x %struct.test] zeroinitializer

define void @test(i64 %idx) {
  ; Element-zero access: Store to a field of a structure, where the GEP indexing is
  ; to an array element address. This access should also cause NonGEP to be set on
  ; the field because the GEP does not directly address the field number.
  %addr0 = getelementptr inbounds [10 x %struct.test], ptr @gStructArray, i64 0, i64 %idx
  store i64 99, ptr %addr0

  ; Store a field from a structure, where the GEP indexing is the field address.
  %addr1 = getelementptr inbounds [10 x %struct.test], ptr @gStructArray, i64 0, i64 %idx, i32 1
  store i64 55, ptr %addr1

  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test = type { i64, i64, i64 }
; CHECK:  Number of fields: 3
; CHECK:  0)Field LLVM Type: i64
; CHECK:    DTrans Type: i64
; CHECK:    Field info: Written NonGEPAccess
; CHECK:  1)Field LLVM Type: i64
; CHECK:    DTrans Type: i64
; CHECK:    Field info: Written
; CHECK:  2)Field LLVM Type: i64
; CHECK:    DTrans Type: i64
; CHECK:    Field info:
; CHECK:  Safety data: Global instance | Global array
; CHECK: End LLVMType: %struct.test = type { i64, i64, i64 }

!intel.dtrans.types = !{!2}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }
