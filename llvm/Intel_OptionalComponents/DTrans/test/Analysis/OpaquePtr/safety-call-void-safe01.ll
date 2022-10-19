; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a void pointer is passed to a function that should be marked
; as safe by the safety analyzer.

; Test the simple case where the void pointer argument gets used as the expected
; type.
%struct.test01 = type { i32, i32 }
define void @use_test01(i8* "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !3 {
  %p2 = bitcast i8* %p to %struct.test01*
  ; This is needed to establish %struct.test01* as an aliased type.
  %field = getelementptr %struct.test01, %struct.test01* %p2, i64 0, i32 0
  store i32 0, i32* %field
  ret void
}
define void @test01() {
  %p = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p to %struct.test01*
  ; This is needed to establish %struct.test01* as an aliased type.
  %field = getelementptr %struct.test01, %struct.test01* %tmp, i64 0, i32 1
  store i32 0, i32* %field

  ; This is the instruction we're actually interested in.
  call void @use_test01(i8* %p)
  call void @free(i8* %p)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01

declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !5 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = distinct !{!2}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!6}
