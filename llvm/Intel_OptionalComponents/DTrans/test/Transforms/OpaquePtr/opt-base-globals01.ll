; RUN: opt -dtransop-allow-typed-pointers -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that global variables get their type changed when types are remapped
; for cases that do not involve pointers.

; Simple case that does not involve pointers
%struct.test01a = type { i32, i32, i32 }

; Case where type containing another type will get converted
%struct.test02a = type { i32, %struct.test02b }
%struct.test02b = type { i32 }

; Case where type to be converted is within another type
%struct.test03a = type { i32 }
%struct.test03b = type { i32, %struct.test03a }

@globVar01a = internal global %struct.test01a zeroinitializer
@globVar02a = internal global %struct.test02a zeroinitializer
@globVar02b = internal global %struct.test02b zeroinitializer
@globVar03a = internal global %struct.test03a zeroinitializer
@globVar03b = internal global %struct.test03b zeroinitializer

; All globals, except globVar02b should get their type changed.
; CHECK-DAG: @globVar01a = internal global %__DTT_struct.test01a zeroinitializer
; CHECK-DAG: @globVar02a = internal global %__DTT_struct.test02a zeroinitializer
; CHECK-DAG: @globVar02b = internal global %struct.test02b zeroinitializer
; CHECK-DAG: @globVar03a = internal global %__DTT_struct.test03a zeroinitializer
; CHECK-DAG: @globVar03b = internal global %__DDT_struct.test03b zeroinitializer

!intel.dtrans.types = !{!4, !5, !6, !7, !8}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!3 = !{%struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!4 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!5 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test02b }
!6 = !{!"S", %struct.test02b zeroinitializer, i32 1, !1} ; { i32 }
!7 = !{!"S", %struct.test03a zeroinitializer, i32 1, !1} ; { i32 }
!8 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !3} ; { i32, %struct.test03a }

