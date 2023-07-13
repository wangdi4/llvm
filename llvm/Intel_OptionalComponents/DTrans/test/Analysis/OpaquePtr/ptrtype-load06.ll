; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery when loading the element-zero member of an array
; using a pointer to the array itself.


; This case loads the structure pointer from the array.
%struct.test01inner = type { ptr }
%struct.test01inner_impl = type { i32, i32, i32 }

define internal void @test01(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !6 {
  %addr1 = getelementptr [2 x %struct.test01inner], ptr %p, i64 0, i32 0, i32 0
  %val1 = load ptr, ptr %addr1

  %addr2 = bitcast ptr %p to ptr
  %val2 = load ptr, ptr %addr2

  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK: %val1 = load ptr, ptr %addr1
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.

; CHECK: %val2 = load ptr, ptr %addr2
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.


; This case loads the element-zero member of the stucture contained within the
; array.
%struct.test02inner = type { %struct.test02inner_impl }
%struct.test02inner_impl = type { ptr, ptr, ptr }

define internal void @test02(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !12 {
  %addr1 = getelementptr [2 x %struct.test02inner], ptr %p, i64 0, i32 0, i32 0, i32 0
  %val1 = load ptr, ptr %addr1

  %addr2 = bitcast ptr %p to ptr
  %val2 = load ptr, ptr %addr2

  ret void
}
; CHECK-LABEL: define internal void @test02
; CHECK: %val1 = load ptr, ptr %addr1
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      i32*
; CHECK-NEXT:    No element pointees.

; CHECK: %val2 = load ptr, ptr %addr2
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      i32*
; CHECK-NEXT:    No element pointees.


!1 = !{%struct.test01inner_impl zeroinitializer, i32 1}  ; %struct.test01inner_impl*
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!4, i32 1}  ; [2 x %struct.test01inner]*
!4 = !{!"A", i32 2, !5}  ; [2 x %struct.test01inner]
!5 = !{%struct.test01inner zeroinitializer, i32 0}  ; %struct.test01inner
!6 = distinct !{!3}
!7 = !{%struct.test02inner_impl zeroinitializer, i32 0}  ; %struct.test02inner_impl
!8 = !{i32 0, i32 1}  ; i32*
!9 = !{!10, i32 1}  ; [2 x %struct.test02inner]*
!10 = !{!"A", i32 2, !11}  ; [2 x %struct.test02inner]
!11 = !{%struct.test02inner zeroinitializer, i32 0}  ; %struct.test02inner
!12 = distinct !{!9}
!13 = !{!"S", %struct.test01inner zeroinitializer, i32 1, !1} ; { %struct.test01inner_impl* }
!14 = !{!"S", %struct.test01inner_impl zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!15 = !{!"S", %struct.test02inner zeroinitializer, i32 1, !7} ; { %struct.test02inner_impl }
!16 = !{!"S", %struct.test02inner_impl zeroinitializer, i32 3, !8, !8, !8} ; { i32*, i32*, i32* }

!intel.dtrans.types = !{!13, !14, !15, !16}
