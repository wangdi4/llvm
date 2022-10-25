; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -disable-output -dtransop-optbasetest -debug-only=dtransop-optbase < %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase < %s 2>&1 | FileCheck %s

; Test for DTrans base class identification of type dependency mappings that map
; the set of types that need to be changed when DTrans is changing a type.
; These cases use literal struct types.

; CHECK-LABEL: Type dependency direct mapping table:
; CHECK-DAG: %struct.test01a: %struct.test01b

; CHECK-LABEL: Type dependency pointer mapping table:
; CHECK-DAG: %struct.test02a: %struct.test02b

%struct.test01a = type { i32 }
%struct.test01b = type { i32, { i32, i32, %struct.test01a } }

%struct.test02a = type { i32 }
%struct.test02b = type { i32, {i32, %struct.test02a* } }

define void @test03() {
  %local1b = alloca %struct.test01b
  %local2b = alloca %struct.test02b
  ret void;
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!3 = !{!"L", i32 3, !1, !1, !4}  ; { i32, i32, %struct.test01a }
!4 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!5 = !{!"L", i32 2, !1, !6}  ; {i32, i32,  %struct.test02a* }
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i32 }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !3} ; { i32, { i32, i32, %struct.test01a } }
!9 = !{!"S", %struct.test02a zeroinitializer, i32 1, !1} ; { i32 }
!10 = !{!"S", %struct.test02b zeroinitializer, i32 2, !1, !5} ; { i32, {i32, %struct.test02a* } }

!intel.dtrans.types = !{!7, !8, !7, !8, !9, !10}
