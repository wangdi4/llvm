; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase < %s 2>&1 | FileCheck %s

; Test for DTrans base class identification of type dependency mappings that map
; the set of types that need to be changed when DTrans is changing a type.
; These cases use array and vector types.

; CHECK-LABEL: Type dependency direct mapping table:
; CHECK-DAG: %struct.test01a: %struct.test01b, %struct.test01c

; CHECK-LABEL: Type dependency pointer mapping table:
; CHECK-DAG: %struct.test02a: %struct.test02b
; CHECK-DAG: %struct.test03a: %struct.test03b
; CHECK-DAG: %struct.test04a: %struct.test04b
; CHECK-DAG: %struct.test05a: %struct.test05b

; Case where type is used as an array in another type
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, [9 x %struct.test01a] }
%struct.test01c = type { i32, [3 x [9 x %struct.test01a]] }

; Case where type is used as a pointer type within an array in another type
%struct.test02a = type { i32, i32 }
%struct.test02b = type { i32, [4 x %struct.test02a*] }

; Case where type is used as a pointer type within a pointer to an array.
%struct.test03a = type { i32, i32 }
%struct.test03b = type { i32, [8 x %struct.test03a*]* }

; Case where type is used as an array within a pointer to an array. In this
; case, even though the type is not a pointer, it is a pointer dependency
; because the pointer to the array.
%struct.test04a = type { i32, i32 }
%struct.test04b = type { i32, [8 x %struct.test04a]* }

; Case where type is used as a pointer within a vector type
%struct.test05a = type { i32, i32 }
%struct.test05b = type { i32, <2 x %struct.test05a*> }

define void @test01() {
  %local1b = alloca %struct.test01b
  %local1c = alloca %struct.test01c
  %local2b = alloca %struct.test02b
  %local3b = alloca %struct.test03b
  %local4b = alloca %struct.test05b
  ret void
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"A", i32 9, !3}  ; [9 x %struct.test01a]
!3 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!4 = !{!"A", i32 3, !2}  ; [3 x [9 x %struct.test01a]]
!5 = !{!"A", i32 4, !6}  ; [4 x %struct.test02a*]
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = !{!8, i32 1}  ; [8 x %struct.test03a*]*
!8 = !{!"A", i32 8, !9}  ; [8 x %struct.test03a*]
!9 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!10 = !{!11, i32 1}  ; [8 x %struct.test04a]*
!11 = !{!"A", i32 8, !12}  ; [8 x %struct.test04a]
!12 = !{%struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!13 = !{!"V", i32 2, !14}  ; <2 x %struct.test05a*>
!14 = !{%struct.test05a zeroinitializer, i32 1}  ; %struct.test05a*
!15 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!16 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !2} ; { i32, [9 x %struct.test01a] }
!17 = !{!"S", %struct.test01c zeroinitializer, i32 2, !1, !4} ; { i32, [3 x [9 x %struct.test01a]] }
!18 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!19 = !{!"S", %struct.test02b zeroinitializer, i32 2, !1, !5} ; { i32, [4 x %struct.test02a*] }
!20 = !{!"S", %struct.test03a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!21 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !7} ; { i32, [8 x %struct.test03a*]* }
!22 = !{!"S", %struct.test04a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!23 = !{!"S", %struct.test04b zeroinitializer, i32 2, !1, !10} ; { i32, [8 x %struct.test04a]* }
!24 = !{!"S", %struct.test05a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!25 = !{!"S", %struct.test05b zeroinitializer, i32 2, !1, !13} ; { i32, <2 x %struct.test05a*> }

!intel.dtrans.types = !{!15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25}
