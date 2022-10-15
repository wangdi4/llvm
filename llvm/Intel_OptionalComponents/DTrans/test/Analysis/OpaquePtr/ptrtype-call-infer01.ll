; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type inference of bitcast for a type that represents
; a function pointer type used in an indirect function call.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%class.test01base = type { i32(...)** }
%class.test01sub1 = type { %class.test01base, i64 }
%class.test01sub2 = type { %class.test01sub1, i32, i32 }

define internal void @test01(%class.test01base* "intel_dtrans_func_index"="1" %in, i32 %x) !intel.dtrans.func.type !8 {
  %vtable_ptr = bitcast %class.test01base* %in to void (%class.test01base*, i32)***
  %vtable = load void (%class.test01base*, i32)**, void (%class.test01base*, i32)*** %vtable_ptr
  %vfn = getelementptr void (%class.test01base*, i32)*, void (%class.test01base*, i32)** %vtable, i64 1
  %fptr = load void (%class.test01base*, i32)*, void (%class.test01base*, i32)** %vfn
  tail call void %fptr(%class.test01base* %in, i32 %x), !intel_dtrans_type !9
  ret void
}
; CHECK-NONOPAQUE: %vtable_ptr = bitcast %class.test01base* %in to void (%class.test01base*, i32)***
; CHECK-OPAQUE: %vtable_ptr = bitcast ptr %in to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %class.test01base*{{ *$}}
; CHECK-NEXT:    i32 (...)***{{ *$}}
; CHECK-NEXT:   void (%class.test01base*, i32)***{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:    %class.test01base @ 0

; CHECK-NONOPAQUE: %vtable = load void (%class.test01base*, i32)**, void (%class.test01base*, i32)*** %vtable_ptr
; CHECK-OPAQUE: %vtable = load ptr, ptr %vtable_ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)**{{ *$}}
; CHECK-NEXT:   void (%class.test01base*, i32)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: %vfn = getelementptr void (%class.test01base*, i32)*, void (%class.test01base*, i32)** %vtable, i64 1
; CHECK-OPAQUE: %vfn = getelementptr ptr, ptr %vtable, i64 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)**{{ *$}}
; CHECK-NEXT:   void (%class.test01base*, i32)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: %fptr = load void (%class.test01base*, i32)*, void (%class.test01base*, i32)** %vfn
; CHECK-OPAQUE: %fptr = load ptr, ptr %vfn
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
