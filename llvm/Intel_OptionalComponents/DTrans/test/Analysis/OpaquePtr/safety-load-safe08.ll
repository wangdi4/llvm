; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-outofboundsok=false -dtrans-print-types -disable-output %s 2>&1 | FileCheck -match-full-lines %s

; Verify loading element zero of an array within a structure that is contained
; within an array results in the field being set as "Read" and "NonGEPAccess" for
; the structure field.

%struct.test = type { [4 x i64], i64, i64 }
@gStructArray = internal global [10 x %struct.test] zeroinitializer

define i64 @test(i64 %idx) {
  ; Element-zero access: Load a field from a structure, where the GEP indexing is
  ; to an array element address. This access should also cause NonGEP to be set on
  ; the field because the GEP does not directly address the field number.
  %addr0 = getelementptr inbounds [10 x %struct.test], ptr @gStructArray, i64 0, i64 %idx
  %v0 = load i64, ptr %addr0

  ; Load a field from a structure, where the GEP indexing is the field address.
  %addr1 = getelementptr inbounds [10 x %struct.test], ptr @gStructArray, i64 0, i64 %idx, i32 1
  %v1 = load i64, ptr %addr1
  %sum = add i64 %v0, %v1

  ret i64 %sum
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test = type { [4 x i64], i64, i64 }
; CHECK:  Number of fields: 3
; CHECK:  0)Field LLVM Type: [4 x i64]
; CHECK:    DTrans Type: [4 x i64]
; CHECK:    Field info: Read NonGEPAccess
; CHECK:  1)Field LLVM Type: i64
; CHECK:    DTrans Type: i64
; CHECK:    Field info: Read
; CHECK:  2)Field LLVM Type: i64
; CHECK:    DTrans Type: i64
; CHECK:    Field info:
; CHECK:  Safety data: Global instance | Global array
; CHECK: End LLVMType: %struct.test = type { [4 x i64], i64, i64 }

!1 = !{!"A", i32 4, !2}  ; [4 x i64]
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !2} ; { [4 x i64], i64, i64 }

!intel.dtrans.types = !{!3}
