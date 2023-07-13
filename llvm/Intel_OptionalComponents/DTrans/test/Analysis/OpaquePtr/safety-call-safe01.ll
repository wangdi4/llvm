; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that calls that have pointers to aggregate types passed and returned do
; not cause safety flags to be set when they are used as the expected types.

%struct.test01 = type { i32, i32 }
define "intel_dtrans_func_index"="1" ptr @test01(ptr "intel_dtrans_func_index"="2" %pStruct, i64 %idx) !intel.dtrans.func.type !3 {
  %res = getelementptr %struct.test01, ptr %pStruct, i64 %idx
  ret ptr %res
}

define void @test01c() {
  %mem = call ptr @malloc(i64 40)
  %elem = call ptr @test01(ptr %mem, i64 3)
  %fieldAddr = getelementptr %struct.test01, ptr %mem, i64 0, i32 1
  store i32 0, ptr %fieldAddr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!6}
