; RUN: opt -S -dtransop-allow-typed-pointers -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a,struct.test10a,struct.test11a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -dtransop-allow-typed-pointers -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a,struct.test10a,struct.test11a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -opaque-pointers -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a,struct.test10a,struct.test11a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPAQUE
; RUN: opt -S -opaque-pointers -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a,struct.test10a,struct.test11a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test that structure types get replaced within a function
; when the type is changed by the transformation or because
; the base class identified the type as a dependent type.

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

; Case where type is pointed-to by another type, and contains another type
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

; Case with an empty structure
%struct.test10a = type {}
%struct.test10b = type { %struct.test10a* }

; Case with an opaque structure
%struct.test11a = type opaque
%struct.test11b = type { %struct.test11a* }

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
  %local10a = alloca %struct.test01a
  %local10b = alloca %struct.test10b
  %local11b = alloca %struct.test11b
  ret void;
}

; CHECK-LABEL: define void @test01()

; All types, except for struct.test02b and struct.test04b, should be changed.
; The 2 unchanged types do not have a dependency on a type being changed.

; CHECK-NONOPAQUE: %local1a = alloca %__DTT_struct.test01a
; CHECK-NONOPAQUE: %local2b = alloca %struct.test02b
; CHECK-NONOPAQUE: %local3b = alloca %__DDT_struct.test03b
; CHECK-NONOPAQUE: %local4b = alloca %struct.test04b
; CHECK-NONOPAQUE: %local5b = alloca %__DDT_struct.test05b
; CHECK-NONOPAQUE: %local6b = alloca %__DDT_struct.test06b
; CHECK-NONOPAQUE: %local6c = alloca %__DDT_struct.test06c
; CHECK-NONOPAQUE: %local6d = alloca %__DDT_struct.test06d
; CHECK-NONOPAQUE: %local6e = alloca %__DDT_struct.test06e
; CHECK-NONOPAQUE: %local7a = alloca %__DTT_struct.test07a
; CHECK-NONOPAQUE: %local8c = alloca %__DDT_struct.test08c
; CHECK-NONOPAQUE: %local9b = alloca %__DDT_struct.test09b
; CHECK-NONOPAQUE: %local10a = alloca %__DTT_struct.test01a
; CHECK-NONOPAQUE: %local10b = alloca %__DDT_struct.test10b
; CHECK-NONOPAQUE: %local11b = alloca %__DDT_struct.test11b

; The following chekcs can replace the above checks when opaque pointers are in use.
; In this case, types purely haivng pointer dependencies do not get changed so many
; of the 'alloca' instructions will stay the same.
; CHECK-OPAQUE: %local1a = alloca %__DTT_struct.test01a
; CHECK-OPAQUE: %local2b = alloca %struct.test02b
; CHECK-OPAQUE: %local3b = alloca %__DDT_struct.test03b
; CHECK-OPAQUE: %local4b = alloca %struct.test04b
; CHECK-OPAQUE: %local5b = alloca %struct.test05b
; CHECK-OPAQUE: %local6b = alloca %struct.test06b
; CHECK-OPAQUE: %local6c = alloca %struct.test06c
; CHECK-OPAQUE: %local6d = alloca %__DDT_struct.test06d
; CHECK-OPAQUE: %local6e = alloca %__DDT_struct.test06e
; CHECK-OPAQUE: %local7a = alloca %__DTT_struct.test07a
; CHECK-OPAQUE: %local8c = alloca %struct.test08c
; CHECK-OPAQUE: %local9b = alloca %struct.test09b
; CHECK-OPAQUE: %local10a = alloca %__DTT_struct.test01a
; CHECK-OPAQUE: %local10b = alloca %struct.test10b
; CHECK-OPAQUE: %local11b = alloca %struct.test11b

!intel.dtrans.types = !{!16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41}

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
!14 = !{%struct.test10a zeroinitializer, i32 1}  ; %struct.test10a*
!15 = !{%struct.test11a zeroinitializer, i32 1}  ; %struct.test11a*
!16 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!17 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test02b }
!18 = !{!"S", %struct.test02b zeroinitializer, i32 1, !1} ; { i32 }
!19 = !{!"S", %struct.test03a zeroinitializer, i32 1, !1} ; { i32 }
!20 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !3} ; { i32, %struct.test03a }
!21 = !{!"S", %struct.test04a zeroinitializer, i32 2, !1, !4} ; { i32, %struct.test04b* }
!22 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32 }
!23 = !{!"S", %struct.test05a zeroinitializer, i32 1, !1} ; { i32 }
!24 = !{!"S", %struct.test05b zeroinitializer, i32 2, !1, !5} ; { i32, %struct.test05a* }
!25 = !{!"S", %struct.test06a zeroinitializer, i32 1, !1} ; { i32 }
!26 = !{!"S", %struct.test06b zeroinitializer, i32 1, !6} ; { %struct.test06a* }
!27 = !{!"S", %struct.test06c zeroinitializer, i32 1, !6} ; { %struct.test06a* }
!28 = !{!"S", %struct.test06d zeroinitializer, i32 1, !7} ; { %struct.test06a }
!29 = !{!"S", %struct.test06e zeroinitializer, i32 1, !7} ; { %struct.test06a }
!30 = !{!"S", %struct.test07a zeroinitializer, i32 2, !1, !8} ; { i32, %struct.test07c }
!31 = !{!"S", %struct.test07b zeroinitializer, i32 2, !1, !9} ; { i32, %struct.test07a* }
!32 = !{!"S", %struct.test07c zeroinitializer, i32 1, !1} ; { i32 }
!33 = !{!"S", %struct.test08a zeroinitializer, i32 3, !1, !10, !11} ; { i32, %struct.test08a*, %struct.test08b* }
!34 = !{!"S", %struct.test08b zeroinitializer, i32 3, !1, !11, !12} ; { i32, %struct.test08b*, %struct.test08c* }
!35 = !{!"S", %struct.test08c zeroinitializer, i32 3, !1, !12, !10} ; { i32, %struct.test08c*, %struct.test08a* }
!36 = !{!"S", %struct.test09a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!37 = !{!"S", %struct.test09b zeroinitializer, i32 2, !1, !13} ; { i32, %struct.test09a** }
!38 = !{!"S", %struct.test10a zeroinitializer, i32 0} ; {}
!39 = !{!"S", %struct.test10b zeroinitializer, i32 1, !14} ; { %struct.test10a* }
!40 = !{!"S", %struct.test11a zeroinitializer, i32 0} ; opaque
!41 = !{!"S", %struct.test11b zeroinitializer, i32 1, !15} ; { %struct.test11a* }
