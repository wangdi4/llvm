; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery for calloc library calls based on
; inference from uses of the value created by the call instruction.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.test01 = type { %struct.test01* }
define internal void @test01(%struct.test01* "intel_dtrans_func_index"="1" %in, i64 %index) !intel.dtrans.func.type !2 {
  %mem_i8 = call i8* @calloc(i64 16, i64 8)
  %array = bitcast i8* %mem_i8 to %struct.test01**
  %array_elem = getelementptr %struct.test01*, %struct.test01** %array, i64 %index

  ; This is the instruction that provides the information need to determine
  ; the allocated type because it relies on the metadata information from
  ; the function signature.
  ; This infers the allocation type based on the type of the value being
  ; stored.
  store %struct.test01* %in, %struct.test01** %array_elem
  ret void
}
; CHECK-LABEL: void @test01
; CHECK-NONOPAQUE:  %mem_i8 = call i8* @calloc(i64 16, i64 8)
; CHECK-OPAQUE:  %mem_i8 = call ptr @calloc(i64 16, i64 8)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01**{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!1 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!2 = distinct !{!1}
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { %struct.test01* }

!intel.dtrans.types = !{!5}
