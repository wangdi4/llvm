; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that DTrans normalize pass inserts a GEP for the store
; that corresponds to an element zero access when an array is
; at element zero.

%struct.test01outer = type { %struct.test01middle }
%struct.test01middle = type { [4 x %struct.test01inner] }
%struct.test01inner = type { ptr }
%struct.test01inner_impl = type { i32, i32, i32 }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !7 {
  %elem_zero_addr1 = getelementptr %struct.test01outer, ptr %p, i64 0, i32 0, i32 0, i32 0, i32 0
  %val1 = load ptr, ptr %elem_zero_addr1
  store ptr %val1, ptr %p

  %use = getelementptr %struct.test01inner_impl, ptr %val1, i64 0, i32 1
  store i32 1, ptr %use
  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK: %dtnorm = getelementptr %struct.test01outer, ptr %p, i64 0, i32 0, i32 0, i32 0, i32 0
; CHECK: store ptr %val1, ptr %dtnorm

!1 = !{%struct.test01middle zeroinitializer, i32 0}  ; %struct.test01middle
!2 = !{!"A", i32 4, !3}  ; [4 x %struct.test01inner]
!3 = !{%struct.test01inner zeroinitializer, i32 0}  ; %struct.test01inner
!4 = !{%struct.test01inner_impl zeroinitializer, i32 1}  ; %struct.test01inner_impl*
!5 = !{i32 0, i32 0}  ; i32
!6 = !{%struct.test01outer zeroinitializer, i32 1}  ; %struct.test01outer*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01outer zeroinitializer, i32 1, !1} ; { %struct.test01middle }
!9 = !{!"S", %struct.test01middle zeroinitializer, i32 1, !2} ; { [4 x %struct.test01inner] }
!10 = !{!"S", %struct.test01inner zeroinitializer, i32 1, !4} ; { %struct.test01inner_impl* }
!11 = !{!"S", %struct.test01inner_impl zeroinitializer, i32 3, !5, !5, !5} ; { i32, i32, i32 }

!intel.dtrans.types = !{!8, !9, !10, !11}
