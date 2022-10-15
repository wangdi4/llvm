; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery for calloc library calls based on
; inference from uses of the value produced by the call instruction.

%struct.test01 = type { ptr }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %in, i64 %index) !intel.dtrans.func.type !2 {
  %array = call ptr @calloc(i64 16, i64 8)
  %array_elem = getelementptr ptr, ptr %array, i64 %index

  ; This is the instruction that provides the information needed to determine
  ; the allocated type because it relies on the metadata information from the
  ; function signature. This infers the allocation type based on the type of
  ; the value being stored.
  store ptr %in, ptr %array_elem
  ret void
}
; CHECK-LABEL: void @test01
; CHECK:  %array = call ptr @calloc(i64 16, i64 8)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01**{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64)

!1 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!2 = distinct !{!1}
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { %struct.test01* }

!intel.dtrans.types = !{!5}
