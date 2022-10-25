; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that storing a type that is an array of the expected type
; is treated as a safe compatible type.

%struct.test = type { i64, i64, i64, ptr }
@gv = internal global %struct.test zeroinitializer

define void @test() {
  %ar = alloca [8192 x i8]
  %ar2 = alloca [2 x [128 x i8]]
  %field = getelementptr %struct.test, ptr @gv, i64 0, i32 3
  %elem = getelementptr inbounds [8192 x i8],  ptr %ar, i64 0, i64 0
  store i8* %elem, i8** %field
  ret void
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test
; CHECK: Safety data: Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{!"S", %struct.test zeroinitializer, i32 4, !1, !1, !1, !2} ; {i64, i64, i64, i8*}

!intel.dtrans.types = !{!3}
