; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks that when a pointer-to-pointer type is also used as a
; pointer-to-pointer to the element zero type, the dominant type is identified
; as being the pointer-to-pointer of the outer type, rather than being treated
; as an ambiguous type.

%struct.outer = type { %struct.inner, i64 }
%struct.inner = type { i32, i32 }

@gVar = internal global ptr zeroinitializer, !intel_dtrans_type !4

define void @test() {
  ; Allocation of set of pointer to pointers
  %ptrtoptr = call ptr @malloc(i64 160)
  store ptr %ptrtoptr, ptr @gVar

  ; Allocate a pointer to a structure
  %elem1 = call ptr @malloc(i64 16)
  store ptr %elem1, ptr %ptrtoptr

  ; Use the pointer-to-pointer of the outer type as a pointer-to-pointer of the
  ; element zero type.
  %pinner = load ptr, ptr %ptrtoptr
  %i1 = getelementptr %struct.inner, ptr %pinner, i64 0, i32 1
  ret void
}

; CHECK:  %ptrtoptr = call ptr @malloc(i64 160)
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.inner**
; CHECK-NEXT:        %struct.outer**
; CHECK-NEXT:        i8*
; CHECK-NEXT:        i8**
; CHECK-NEXT:      No element pointees.
; CHECK-NEXT:      DomTy: %struct.outer**


declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @malloc(i64)

!1 = !{%struct.inner zeroinitializer, i32 0}  ; %struct.inner
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i32 0, i32 0}  ; i32
!4 = !{%struct.outer zeroinitializer, i32 2}  ; %struct.outer**
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.outer zeroinitializer, i32 2, !1, !2} ; { %struct.inner, i64 }
!8 = !{!"S", %struct.inner zeroinitializer, i32 2, !3, !3} ; { i32, i32 }

!intel.dtrans.types = !{!7, !8}
