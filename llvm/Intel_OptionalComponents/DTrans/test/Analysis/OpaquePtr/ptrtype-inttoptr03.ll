; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test the PtrTypeAnalyzer type collection for inttoptr instructions that get
; inferred based on users of the instruction result.


define internal "intel_dtrans_func_index"="1" ptr @test01(i64 %size) !intel.dtrans.func.type !2 {
  %ptr = tail call noalias ptr @malloc(i64 %size)
  %int = ptrtoint ptr %ptr to i64
  %add = add i64 %int, 71
  %and = and i64 %add, -64
  %itp1 = inttoptr i64 %and to ptr
  %itp2 = inttoptr i64 %and to ptr

  %gep = getelementptr inbounds ptr, ptr %itp2, i64 -1
  store ptr %ptr, ptr %gep
  ret ptr %itp1
}

; CHECK-LABEL: @test01
; CHECK:  %itp1 = inttoptr i64 %and to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8*
; CHECK-NEXT:      No element pointees.

; CHECK:  %itp2 = inttoptr i64 %and to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8**
; CHECK-NEXT:      No element pointees.

; CHECK:  %gep = getelementptr inbounds ptr, ptr %itp2, i64 -1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8**
; CHECK-NEXT:      No element pointees.

declare !intel.dtrans.func.type !3 "intel_dtrans_func_index"="1" ptr @malloc(i64)

!intel.dtrans.types = !{}
!1 = !{i8 0, i32 1}  ; i8*
!2 = distinct !{!1}
!3 = distinct !{!1}
