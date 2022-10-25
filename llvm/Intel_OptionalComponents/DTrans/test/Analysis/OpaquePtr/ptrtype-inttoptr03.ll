; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test the PtrTypeAnalyzer type collection for inttoptr instructions that get
; inferred based on users of the instruction result.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

define internal "intel_dtrans_func_index"="1" i8* @test01(i64 %size) !intel.dtrans.func.type !2 {
  %ptr = tail call noalias i8* @malloc(i64 %size)
  %int = ptrtoint i8* %ptr to i64
  %add = add i64 %int, 71
  %and = and i64 %add, -64
  %itp1 = inttoptr i64 %and to i8*
  %itp2 = inttoptr i64 %and to i8**

  %gep = getelementptr inbounds i8*, i8** %itp2, i64 -1
  store i8* %ptr, i8** %gep
  ret i8* %itp1
}

; CHECK-LABEL: @test01
; CHECK-NONOPAQUE:  %itp1 = inttoptr i64 %and to i8*
; CHECK-OPAQUE:  %itp1 = inttoptr i64 %and to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8*
; CHECK-NEXT:      No element pointees.

; CHECK-NONOPAQUE:  %itp2 = inttoptr i64 %and to i8**
; CHECK-OPAQUE:  %itp2 = inttoptr i64 %and to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8**
; CHECK-NEXT:      No element pointees.

; CHECK-NONOPAQUE:  %gep = getelementptr inbounds i8*, i8** %itp2, i64 -1
; CHECK-OPAQUE:  %gep = getelementptr inbounds ptr, ptr %itp2, i64 -1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8**
; CHECK-NEXT:      No element pointees.

declare !intel.dtrans.func.type !3 "intel_dtrans_func_index"="1" i8* @malloc(i64)

!intel.dtrans.types = !{}
!1 = !{i8 0, i32 1}  ; i8*
!2 = distinct !{!1}
!3 = distinct !{!1}
