; RUN: opt -dtransop-allow-typed-pointers -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s -check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s -check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s -check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s -check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test that global variables get their type changed when types are remapped
; for cases with pointer types within the structure.

; Simple case where the pointers do not point to structure types
%struct.test01a = type { i32*, i32*, i32* }

; Case where type to be converted has a pointer to another type
%struct.test02a = type { i32, %struct.test02b* }
%struct.test02b = type { i32 }

; Case where type to be converted is pointed-to by another type
%struct.test03a = type { i32 }
%struct.test03b = type { i32, %struct.test03a* }

@globInt = global i32 zeroinitializer
@globVar01a = internal global %struct.test01a { i32* @globInt, i32* @globInt, i32* @globInt }
@globVar02a = internal global %struct.test02a { i32 1, %struct.test02b* @globVar02b }
@globVar02b = internal global %struct.test02b { i32 2 }
@globVar03a = internal global %struct.test03a { i32 3 }
@globVar03b = internal global %struct.test03b { i32 4, %struct.test03a* @globVar03a }

; All globals, except globVar02b should get their type changed.
; CHECK-NONOPAQUE-DAG: @globVar01a = internal global %__DTT_struct.test01a { i32* @globInt, i32* @globInt, i32* @globInt }
; CHECK-NONOPAQUE-DAG: @globVar02a = internal global %__DTT_struct.test02a { i32 1, %struct.test02b* @globVar02b }
; CHECK-NONOPAQUE-DAG: @globVar02b = internal global %struct.test02b { i32 2 }
; CHECK-NONOPAQUE-DAG: @globVar03a = internal global %__DTT_struct.test03a { i32 3 }
; CHECK-NONOPAQUE-DAG: @globVar03b = internal global %__DDT_struct.test03b { i32 4, %__DTT_struct.test03a* @globVar03a }

; The following checks will replace the above checks when opaque pointers are used.
; In this case, the types of structures that contain pointers to types being
; changed do not change.
; CHECK-OPAQUE-DAG: @globVar01a = internal global %__DTT_struct.test01a { ptr @globInt, ptr @globInt, ptr @globInt }
; CHECK-OPAQUE-DAG: @globVar02a = internal global %__DTT_struct.test02a { i32 1, ptr @globVar02b }
; CHECK-OPAQUE-DAG: @globVar02b = internal global %struct.test02b { i32 2 }
; CHECK-OPAQUE-DAG: @globVar03a = internal global %__DTT_struct.test03a { i32 3 }
; CHECK-OPAQUE-DAG: @globVar03b = internal global %struct.test03b { i32 4, ptr @globVar03a }

!intel.dtrans.types = !{!5, !6, !7, !8, !9}

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!4 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!5 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!6 = !{!"S", %struct.test02a zeroinitializer, i32 2, !2, !3} ; { i32, %struct.test02b* }
!7 = !{!"S", %struct.test02b zeroinitializer, i32 1, !2} ; { i32 }
!8 = !{!"S", %struct.test03a zeroinitializer, i32 1, !2} ; { i32 }
!9 = !{!"S", %struct.test03b zeroinitializer, i32 2, !2, !4} ; { i32, %struct.test03a* }
