; RUN: opt -opaque-pointers -S -whole-program-assume -dtrans-normalizeop < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -S -whole-program-assume -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

; Check that DTrans normalize pass inserts a GEP for the load
; that corresponds to an element zero access.

; Loading a scalar type with an element zero access
%struct.test01outer = type { i32, i32 }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %p)  !intel.dtrans.func.type !3 {
  %elem_zero_addr1 = getelementptr %struct.test01outer, ptr %p, i64 0, i32 0

; Load the same memory location with different forms of the pointer argument.
  %val1 = load i32, ptr %elem_zero_addr1
  %val2 = load i32, ptr %p
  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK: %dtnorm = getelementptr %struct.test01outer, ptr %p, i64 0, i32 0
; CHECK: %val2 = load i32, ptr %dtnorm

; Loading a pointer type with an element zero access
%struct.test02outer = type { ptr, i32 }
define internal void @test02(ptr "intel_dtrans_func_index"="1" %p)  !intel.dtrans.func.type !6 {
  %elem_zero_addr1 = getelementptr %struct.test02outer, ptr %p, i64 0, i32 0

  ; Load the same memory location with different forms of the pointer argument.
  %val1 = load ptr, ptr %elem_zero_addr1
  %val2 = load ptr, ptr %p
  ret void
}
; CHECK-LABEL: define internal void @test02
; CHECK: %dtnorm = getelementptr %struct.test02outer, ptr %p, i64 0, i32 0
; CHECK: %val2 = load ptr, ptr %dtnorm

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01outer zeroinitializer, i32 1}  ; %struct.test01outer*
!3 = distinct !{!2}
!4 = !{i32 0, i32 1}  ; i32*
!5 = !{%struct.test02outer zeroinitializer, i32 1}  ; %struct.test02outer*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01outer zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!8 = !{!"S", %struct.test02outer zeroinitializer, i32 2, !4, !1} ; { i32*, i32 }

!intel.dtrans.types = !{!7, !8}
