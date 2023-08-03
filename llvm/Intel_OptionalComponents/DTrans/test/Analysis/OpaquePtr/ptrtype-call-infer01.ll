; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s 

; Test pointer type inference of bitcast for a type that represents
; a function pointer type used in an indirect function call.


%class.test01base = type { ptr }
%class.test01sub1 = type { %class.test01base, i64 }
%class.test01sub2 = type { %class.test01sub1, i32, i32 }

define internal void @test01(ptr "intel_dtrans_func_index"="1" %in, i32 %x) !intel.dtrans.func.type !8 {
  %vtable_ptr = bitcast ptr %in to ptr
  %vtable = load ptr, ptr %vtable_ptr
  %vfn = getelementptr ptr, ptr %vtable, i64 1
  %fptr = load ptr, ptr %vfn
  tail call void %fptr(ptr %in, i32 %x), !intel_dtrans_type !9
  ret void
}
; CHECK: %vtable_ptr = bitcast ptr %in to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %class.test01base*{{ *$}}
; CHECK-NEXT:    i32 (...)***{{ *$}}
; CHECK-NEXT:   void (%class.test01base*, i32)***{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:    %class.test01base @ 0

; CHECK: %vtable = load ptr, ptr %vtable_ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)**{{ *$}}
; CHECK-NEXT:   void (%class.test01base*, i32)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK: %vfn = getelementptr ptr, ptr %vtable, i64 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)**{{ *$}}
; CHECK-NEXT:   void (%class.test01base*, i32)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK: %fptr = load ptr, ptr %vfn
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)*{{ *$}}
; CHECK-NEXT:   void (%class.test01base*, i32)*{{ *$}}
; CHECK-NEXT: No element pointees.

!1 = !{!"F", i1 true, i32 0, !2}  ; i32(...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32(...)**
!4 = !{%class.test01base zeroinitializer, i32 0}  ; %class.test01base
!5 = !{i64 0, i32 0}  ; i64
!6 = !{%class.test01sub1 zeroinitializer, i32 0}  ; %class.test01sub1
!7 = !{%class.test01base zeroinitializer, i32 1}  ; %class.test01base*
!8 = distinct !{!7}
!9 = !{!"F", i1 false, i32 2, !10, !7, !2}  ; void (%class.test01base*, i32)
!10 = !{!"void", i32 0}  ; void
!11 = !{!"S", %class.test01base zeroinitializer, i32 1, !3} ; { i32(...)** }
!12 = !{!"S", %class.test01sub1 zeroinitializer, i32 2, !4, !5} ; { %class.test01base, i64 }
!13 = !{!"S", %class.test01sub2 zeroinitializer, i32 3, !6, !2, !2} ; { %class.test01sub1, i32, i32 }

!intel.dtrans.types = !{!11, !12, !13}
