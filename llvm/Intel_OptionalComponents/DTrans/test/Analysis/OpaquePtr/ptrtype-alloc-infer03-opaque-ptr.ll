; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery for realloc library calls based on
; inference from uses of the value created by the call instruction.

; This case should detect the realloc result gets used as a
; %struct.test01*.
%struct.test01 = type { i64, i64 }
define internal "intel_dtrans_func_index"="1" ptr @test01(ptr "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !3 {
  %out = call ptr @realloc(ptr %in, i64 256)
  ret ptr %out
}
; CHECK-LABEL: define internal "intel_dtrans_func_index"="1" ptr @test01
; CHECK: %out = call ptr @realloc(ptr %in, i64 256)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:    %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; In this case, the result of the realloc gets used as a different type
; than the original pointer type into the realloc. The pointer type
; analyzer only collects the type that the pointer is directly used as.
; This means the original input type will not be tracked as a result type
; for the realloc call. The safety analyzer should handle tracking whether the
; types are compatible.
%struct.test02 = type { ptr, ptr }
%struct.test02b = type { ptr }
@testvar02 = global %struct.test02 zeroinitializer
define internal void @test02() {
  %f0 = getelementptr %struct.test02, ptr @testvar02, i64 0, i32 0
  %v0 = load ptr, ptr %f0
  %more = call ptr @realloc(ptr %v0, i64 512)
  %f1 = getelementptr %struct.test02, ptr @testvar02, i64 0, i32 1
  store ptr %more, ptr %f1
  ret void
}
; CHECK-LABEL: define internal void @test02
; CHECK: %more = call ptr @realloc(ptr %v0, i64 512)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:   %struct.test02b*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @realloc(ptr "intel_dtrans_func_index"="2", i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6, !6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !4, !5} ; { %struct.test02*, %struct.test02b* }
!10 = !{!"S", %struct.test02b zeroinitializer, i32 1, !5} ; { %struct.test02b* }

!intel.dtrans.types = !{!8, !9, !10}
