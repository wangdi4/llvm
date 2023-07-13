; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a void pointer is passed to a function that should be marked
; as safe by the safety analyzer.

; Test the simple case where the void pointer argument gets used as the expected
; type.
%struct.test01 = type { i32, i32 }
define void @use_test01(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !3 {
  ; This is needed to establish %struct.test01* as an aliased type.
  %field = getelementptr %struct.test01, ptr %p, i64 0, i32 0
  store i32 0, ptr %field
  ret void
}
define void @test01() {
  %p = call ptr @malloc(i64 8)
  ; This is needed to establish %struct.test01* as an aliased type.
  %field = getelementptr %struct.test01, ptr %p, i64 0, i32 1
  store i32 0, ptr %field

  ; This is the instruction we're actually interested in.
  call void @use_test01(ptr %p)
  call void @free(ptr %p)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01

declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !5 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = distinct !{!2}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!6}
