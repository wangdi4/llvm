; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-outofboundsok=false -dtrans-print-types -disable-output %s 2>&1 | FileCheck -match-full-lines %s

; Verify loading element zero with a mismatched type of a structure that is
; contained within an array results in the MismatchedElementAccess being set
; on the field.

%struct.test = type { i64, i64, i64 }
@gStructArray = internal global [10 x %struct.test] zeroinitializer
@gIntArray = internal global [10 x i64] zeroinitializer

define i32 @test(i64 %idx) {
  ; Element-zero access: Load a field from a structure, where the GEP indexing is
  ; an array element address.
  %addr0 = getelementptr inbounds [10 x %struct.test], ptr @gStructArray, i64 0, i64 %idx
  %v0 = load i32, ptr %addr0

  ; Load a field from a structure, where the GEP indexing is the field address
  %addr1 = getelementptr inbounds [10 x %struct.test], ptr @gStructArray, i64 0, i64 %idx, i32 1
  %v1 = load i32, ptr %addr1
  %sum = add i32 %v0, %v1

  ; Load from an integer array
  %addr2 = getelementptr inbounds [10 x i64], ptr @gIntArray, i64 0, i64 %idx
  %v2 = load i32, ptr %addr2
  %sum1 = add i32 %sum, %v2

  ret i32 %sum
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test = type { i64, i64, i64 }
; CHECK: Name: struct.test
; CHECK: Number of fields: 3
; CHECK: 0)Field LLVM Type: i64
; CHECK:   DTrans Type: i64
; CHECK:   Field info: Read MismatchedElementAccess NonGEPAccess
; CHECK:  1)Field LLVM Type: i64
; CHECK:    DTrans Type: i64
; CHECK:    Field info: Read MismatchedElementAccess
; CHECK:  2)Field LLVM Type: i64
; CHECK:    DTrans Type: i64
; CHECK:    Field info:
; CHECK: Safety data: Mismatched element access | Global instance | Global array
; CHECK: End LLVMType: %struct.test = type { i64, i64, i64 }

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [10 x %struct.test]
; CHECK: Safety data: Mismatched element access | Global instance | Global array
; CHECK: End LLVMType: [10 x %struct.test]

!intel.dtrans.types = !{!2}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }
