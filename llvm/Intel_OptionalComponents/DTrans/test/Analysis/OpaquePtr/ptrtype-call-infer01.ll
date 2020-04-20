; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type inference of bitcast for a type that represents
; a function pointer type used in an indirect function call.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%class.test01base = type { i32(...)** }
%class.test01sub1 = type { %class.test01base, i64 }
%class.test01sub2 = type { %class.test01sub1, i32, i32 }

define internal void @test01(%class.test01base* %in, i32 %x) !dtrans_type !7 {
  %vtable_ptr = bitcast %class.test01base* %in to void (%class.test01base*, i32)***
  %vtable = load void (%class.test01base*, i32)**, void (%class.test01base*, i32)*** %vtable_ptr
  %vfn = getelementptr void (%class.test01base*, i32)*, void (%class.test01base*, i32)** %vtable, i64 1
  %fptr = load void (%class.test01base*, i32)*, void (%class.test01base*, i32)** %vfn
  tail call void %fptr(%class.test01base* %in, i32 %x), !dtrans_type !7
  ret void
}
; CHECK-CUR: %vtable_ptr = bitcast %class.test01base* %in to void (%class.test01base*, i32)***
; CHECK-FUT: %vtable_ptr = bitcast p0 %in to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %class.test01base*{{ *$}}
; CHECK-NEXT:   void (%class.test01base*, i32)***{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR: %vtable = load void (%class.test01base*, i32)**, void (%class.test01base*, i32)*** %vtable_ptr
; CHECK-FUT: %vtable = load p0, p0 %vtable_ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   void (%class.test01base*, i32)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR: %vfn = getelementptr void (%class.test01base*, i32)*, void (%class.test01base*, i32)** %vtable, i64 1
; CHECK-FUT: %vfn = getelementptr p0, p0 %vtable, i64 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   void (%class.test01base*, i32)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR: %fptr = load void (%class.test01base*, i32)*, void (%class.test01base*, i32)** %vfn
; CHECK-FUT: %fptr = load p0, p0 %vfn
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   void (%class.test01base*, i32)*{{ *$}}
; CHECK-NEXT: No element pointees.

!1 = !{!"F", i1 true, i32 0, !2}  ; i32(...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32(...)**
!4 = !{!"R", %class.test01base zeroinitializer, i32 0}  ; %class.test01base
!5 = !{i64 0, i32 0}  ; i64
!6 = !{!"R", %class.test01sub1 zeroinitializer, i32 0}  ; %class.test01sub1
!7 = !{!"F", i1 false, i32 2, !8, !9, !2}  ; void (%class.test01base*, i32)
!8 = !{!"void", i32 0}  ; void
!9 = !{!4, i32 1}  ; %class.test01base*
!10 = !{!"S", %class.test01base zeroinitializer, i32 1, !3} ; { i32(...)** }
!11 = !{!"S", %class.test01sub1 zeroinitializer, i32 2, !4, !5} ; { %class.test01base, i64 }
!12 = !{!"S", %class.test01sub2 zeroinitializer, i32 3, !6, !2, !2} ; { %class.test01sub1, i32, i32 }

!dtrans_types = !{!10, !11, !12}
