; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -disable-output -dtransop-optbasetest -debug-only=dtransop-optbase < %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase < %s 2>&1 | FileCheck %s

; Test for DTrans base class identification of type dependency mappings that map
; the set of types that need to be changed when DTrans is changing a type.

; CHECK-LABEL: Type dependency direct mapping table:
; CHECK-NOT: %struct.test01a
; CHECK-DAG: %struct.test02b: %struct.test02a
; CHECK-DAG: %struct.test03a: %struct.test03b
; CHECK-DAG: %struct.test06a: %struct.test06d, %struct.test06e
; CHECK-DAG: %struct.test07c: %struct.test07a

; CHECK-LABEL: Type dependency pointer mapping table:
; CHECK-NOT: %struct.test01a
; CHECK-DAG: %struct.test04b: %struct.test04a
; CHECK-DAG: %struct.test05a: %struct.test05b
; CHECK-DAG: %struct.test06a: %struct.test06b, %struct.test06c
; CHECK-DAG: %struct.test07a: %struct.test07b
; CHECK-DAG: %struct.test08a: %struct.test08c
; CHECK-DAG: %struct.test08b: %struct.test08a
; CHECK-DAG: %struct.test08c: %struct.test08b
; CHECK-DAG: %struct.test09a: %struct.test09b

; Simple case that does not involve pointers
%struct.test01a = type { i32, i32, i32 }

; Case where type containing another type will get converted
%struct.test02a = type { i32, %struct.test02b }
%struct.test02b = type { i32 }

; Case where type to be converted is within another type
%struct.test03a = type { i32 }
%struct.test03b = type { i32, %struct.test03a }

; Case where type to be converted has a pointer to another type
%struct.test04a = type { i32, %struct.test04b* }
%struct.test04b = type { i32 }

; Case where type to be converted is pointed-to by another type
%struct.test05a = type { i32 }
%struct.test05b = type { i32, %struct.test05a* }

; Case where a type has multiple dependent types
%struct.test06a = type { i32 }
%struct.test06b = type { %struct.test06a* }
%struct.test06c = type { %struct.test06a* }
%struct.test06d = type { %struct.test06a }
%struct.test06e = type { %struct.test06a }

; Case where type is pointed-to by another type, and contains another type.
%struct.test07a = type { i32, %struct.test07c }
%struct.test07b = type { i32, %struct.test07a* }
%struct.test07c = type { i32 }

; Case with self & circular references
%struct.test08a = type { i32, %struct.test08a*, %struct.test08b* }
%struct.test08b = type { i32, %struct.test08b*, %struct.test08c* }
%struct.test08c = type { i32, %struct.test08c*, %struct.test08a* }

; Case with pointer-to-pointer reference
%struct.test09a = type { i32, i32 }
%struct.test09b = type { i32, %struct.test09a** }

define void @test01() {
  %local1a = alloca %struct.test01a
  %local2b = alloca %struct.test02b
  %local3b = alloca %struct.test03b
  %local4b = alloca %struct.test04b
  %local5b = alloca %struct.test05b
  %local6b = alloca %struct.test06b
  %local6c = alloca %struct.test06c
  %local6d = alloca %struct.test06d
  %local6e = alloca %struct.test06e
  %local7a = alloca %struct.test07a
  %local8c = alloca %struct.test08c
  %local9b = alloca %struct.test09b
  ret void;
}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!3 = !{%struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!4 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!5 = !{%struct.test05a zeroinitializer, i32 1}  ; %struct.test05a*
!6 = !{%struct.test06a zeroinitializer, i32 1}  ; %struct.test06a*
!7 = !{%struct.test06a zeroinitializer, i32 0}  ; %struct.test06a
!8 = !{%struct.test07c zeroinitializer, i32 0}  ; %struct.test07c
!9 = !{%struct.test07a zeroinitializer, i32 1}  ; %struct.test07a*
!10 = !{%struct.test08a zeroinitializer, i32 1}  ; %struct.test08a*
!11 = !{%struct.test08b zeroinitializer, i32 1}  ; %struct.test08b*
!12 = !{%struct.test08c zeroinitializer, i32 1}  ; %struct.test08c*
!13 = !{%struct.test09a zeroinitializer, i32 2}  ; %struct.test09a**
!14 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!15 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test02b }
!16 = !{!"S", %struct.test02b zeroinitializer, i32 1, !1} ; { i32 }
!17 = !{!"S", %struct.test03a zeroinitializer, i32 1, !1} ; { i32 }
!18 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !3} ; { i32, %struct.test03a }
!19 = !{!"S", %struct.test04a zeroinitializer, i32 2, !1, !4} ; { i32, %struct.test04b* }
!20 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32 }
!21 = !{!"S", %struct.test05a zeroinitializer, i32 1, !1} ; { i32 }
!22 = !{!"S", %struct.test05b zeroinitializer, i32 2, !1, !5} ; { i32, %struct.test05a* }
!23 = !{!"S", %struct.test06a zeroinitializer, i32 1, !1} ; { i32 }
!24 = !{!"S", %struct.test06b zeroinitializer, i32 1, !6} ; { %struct.test06a* }
!25 = !{!"S", %struct.test06c zeroinitializer, i32 1, !6} ; { %struct.test06a* }
!26 = !{!"S", %struct.test06d zeroinitializer, i32 1, !7} ; { %struct.test06a }
!27 = !{!"S", %struct.test06e zeroinitializer, i32 1, !7} ; { %struct.test06a }
!28 = !{!"S", %struct.test07a zeroinitializer, i32 2, !1, !8} ; { i32, %struct.test07c }
!29 = !{!"S", %struct.test07b zeroinitializer, i32 2, !1, !9} ; { i32, %struct.test07a* }
!30 = !{!"S", %struct.test07c zeroinitializer, i32 1, !1} ; { i32 }
!31 = !{!"S", %struct.test08a zeroinitializer, i32 3, !1, !10, !11} ; { i32, %struct.test08a*, %struct.test08b* }
!32 = !{!"S", %struct.test08b zeroinitializer, i32 3, !1, !11, !12} ; { i32, %struct.test08b*, %struct.test08c* }
!33 = !{!"S", %struct.test08c zeroinitializer, i32 3, !1, !12, !10} ; { i32, %struct.test08c*, %struct.test08a* }
!34 = !{!"S", %struct.test09a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!35 = !{!"S", %struct.test09b zeroinitializer, i32 2, !1, !13} ; { i32, %struct.test09a** }

!intel.dtrans.types = !{!14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35}
