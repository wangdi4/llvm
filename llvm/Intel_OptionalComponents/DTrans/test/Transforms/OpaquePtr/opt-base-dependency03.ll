; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase < %s 2>&1 | FileCheck %s

; Test for DTrans base class identification of type dependency mappings that map
; the set of types that need to be changed when DTrans is changing a type.
; These cases use function pointer types.

; CHECK-LABEL: Type dependency pointer mapping table:
; CHECK-DAG: %struct.test01b: %struct.test01c
; CHECK-DAG: %struct.test01a: %struct.test01c

; Case with type to be converted used as a pointer type as part of a function type.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32, i32 }
%struct.test01c = type { i32, %struct.test01a* (i32, %struct.test01b*)* }

define void @test01() {
  %local1c = alloca %struct.test01c
  ret void
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !1, !4}  ; %struct.test01a* (i32, %struct.test01b*)
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!5 = !{!2, i32 1}  ; %struct.test01a* (i32, %struct.test01b*)*
!6 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!7 = !{!"S", %struct.test01b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!8 = !{!"S", %struct.test01c zeroinitializer, i32 2, !1, !5} ; { i32, %struct.test01a* (i32, %struct.test01b*)* }

!intel.dtrans.types = !{!6, !7, !8}
