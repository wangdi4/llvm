; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -disable-output -dtransop-optbasetest -debug-only=dtransop-optbase -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a,struct.test10a,struct.test11a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a,struct.test10a,struct.test11a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -dtransop-optbasetest -debug-only=dtransop-optbase -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a,struct.test10a,struct.test11a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a,struct.test10a,struct.test11a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test the ability to create new LLVM and DTrans types for the types selected by
; the transformation and by the base class converting the dependent types. This
; test is being developed before the IR gets rewritten to use the new types,
; so this will just check the debug traces for the types.

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

; Case with self & circular references. This case also shows where a
; dependent type affects another type.
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

@globVar01a = global %struct.test01a zeroinitializer
@globVar02b = global %struct.test02b zeroinitializer
@globVar03b = global %struct.test03b zeroinitializer
@globVar04b = global %struct.test04b zeroinitializer
@globVar05b = global %struct.test05b zeroinitializer
@globVar06b = global %struct.test06b zeroinitializer
@globVar06c = global %struct.test06c zeroinitializer
@globVar06d = global %struct.test06d zeroinitializer
@globVar06e = global %struct.test06e zeroinitializer
@globVar07a = global %struct.test07a zeroinitializer
@globVar08c = global %struct.test08c zeroinitializer
@globVar09b = global %struct.test09b zeroinitializer
@globVar10b = global %struct.test10b zeroinitializer
@globVar11b = global %struct.test11b zeroinitializer


; CHECK-LABEL: TypeRemapper types after preparing types:
; CHECK-NEXT: DTransOPTypeRemapper LLVM Type Mappings:
; Types that get remapped when not using opaque pointers.
; The types whose names end in 'a' were remapped by the transformation pass, and
; the rest are computed by the base class based on dependencies.

; CHECK-NONOPAQUE-DAG: %struct.test01a = type { i32, i32, i32 } -> %__DTT_struct.test01a = type { i32, i32, i32 }
; CHECK-NONOPAQUE-DAG: %struct.test02a = type { i32, %struct.test02b } -> %__DTT_struct.test02a = type { i32, %struct.test02b }
; CHECK-NONOPAQUE-DAG: %struct.test03a = type { i32 } -> %__DTT_struct.test03a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test03b = type { i32, %struct.test03a } -> %__DDT_struct.test03b = type { i32, %__DTT_struct.test03a }
; CHECK-NONOPAQUE-DAG: %struct.test04a = type { i32, %struct.test04b* } -> %__DTT_struct.test04a = type { i32, %struct.test04b* }
; CHECK-NONOPAQUE-DAG: %struct.test05a = type { i32 } -> %__DTT_struct.test05a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test05b = type { i32, %struct.test05a* } -> %__DDT_struct.test05b = type { i32, %__DTT_struct.test05a* }
; CHECK-NONOPAQUE-DAG: %struct.test06a = type { i32 } -> %__DTT_struct.test06a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test06b = type { %struct.test06a* } -> %__DDT_struct.test06b = type { %__DTT_struct.test06a* }
; CHECK-NONOPAQUE-DAG: %struct.test06c = type { %struct.test06a* } -> %__DDT_struct.test06c = type { %__DTT_struct.test06a* }
; CHECK-NONOPAQUE-DAG: %struct.test06d = type { %struct.test06a } -> %__DDT_struct.test06d = type { %__DTT_struct.test06a }
; CHECK-NONOPAQUE-DAG: %struct.test06e = type { %struct.test06a } -> %__DDT_struct.test06e = type { %__DTT_struct.test06a }
; CHECK-NONOPAQUE-DAG: %struct.test07a = type { i32, %struct.test07c } -> %__DTT_struct.test07a = type { i32, %struct.test07c }
; CHECK-NONOPAQUE-DAG: %struct.test07b = type { i32, %struct.test07a* } -> %__DDT_struct.test07b = type { i32, %__DTT_struct.test07a* }
; CHECK-NONOPAQUE-DAG: %struct.test08a = type { i32, %struct.test08a*, %struct.test08b* } -> %__DTT_struct.test08a = type { i32, %__DTT_struct.test08a*, %__DDT_struct.test08b* }
; CHECK-NONOPAQUE-DAG: %struct.test08b = type { i32, %struct.test08b*, %struct.test08c* } -> %__DDT_struct.test08b = type { i32, %__DDT_struct.test08b*, %__DDT_struct.test08c* }
; CHECK-NONOPAQUE-DAG: %struct.test08c = type { i32, %struct.test08c*, %struct.test08a* } -> %__DDT_struct.test08c = type { i32, %__DDT_struct.test08c*, %__DTT_struct.test08a* }
; CHECK-NONOPAQUE-DAG: %struct.test09a = type { i32, i32 } -> %__DTT_struct.test09a = type { i32, i32 }
; CHECK-NONOPAQUE-DAG: %struct.test09b = type { i32, %struct.test09a** } -> %__DDT_struct.test09b = type { i32, %__DTT_struct.test09a** }
; CHECK-NONOPAQUE-DAG: %struct.test10a = type {} -> %__DTT_struct.test10a = type {}
; CHECK-NONOPAQUE-DAG: %struct.test10b = type { %struct.test10a* } -> %__DDT_struct.test10b = type { %__DTT_struct.test10a* }
; CHECK-NONOPAQUE-DAG: %struct.test11a = type opaque -> %__DTT_struct.test11a = type opaque
; CHECK-NONOPAQUE-DAG: %struct.test11b = type { %struct.test11a* } -> %__DDT_struct.test11b = type { %__DTT_struct.test11a* }

; The following are checks that will replace the above checks when opaque pointers are in use.
; Types with pointer dependencies no longer need to be remapped when opaque pointers are used.
; CHECK-OPAQUE-DAG: %struct.test01a = type { i32, i32, i32 } -> %__DTT_struct.test01a = type { i32, i32, i32 }
; CHECK-OPAQUE-DAG: %struct.test02a = type { i32, %struct.test02b } -> %__DTT_struct.test02a = type { i32, %struct.test02b }
; CHECK-OPAQUE-DAG: %struct.test03a = type { i32 } -> %__DTT_struct.test03a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test03b = type { i32, %struct.test03a } -> %__DDT_struct.test03b = type { i32, %__DTT_struct.test03a }
; CHECK-OPAQUE-DAG: %struct.test04a = type { i32, ptr } -> %__DTT_struct.test04a = type { i32, ptr }
; CHECK-OPAQUE-DAG: %struct.test05a = type { i32 } -> %__DTT_struct.test05a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test06a = type { i32 } -> %__DTT_struct.test06a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test06d = type { %struct.test06a } -> %__DDT_struct.test06d = type { %__DTT_struct.test06a }
; CHECK-OPAQUE-DAG: %struct.test06e = type { %struct.test06a } -> %__DDT_struct.test06e = type { %__DTT_struct.test06a }
; CHECK-OPAQUE-DAG: %struct.test07a = type { i32, %struct.test07c } -> %__DTT_struct.test07a = type { i32, %struct.test07c }
; CHECK-OPAQUE-DAG: %struct.test08a = type { i32, ptr, ptr } -> %__DTT_struct.test08a = type { i32, ptr, ptr }
; CHECK-OPAQUE-DAG: %struct.test09a = type { i32, i32 } -> %__DTT_struct.test09a = type { i32, i32 }
; CHECK-OPAQUE-DAG: %struct.test10a = type {} -> %__DTT_struct.test10a = type {}
; CHECK-OPAQUE-DAG: %struct.test11a = type opaque -> %__DTT_struct.test11a = type opaque

; CHECK-LABEL: End of DTransOPTypeRemapper LLVM Type Mappings

; Next is the DTransType representation of the types.
; CHECK-LABEL: DTransOPTypeRemapper DTrans Type Mappings:

; Types that get remapped when not using opaque pointers.

; CHECK-NONOPAQUE-DAG: %struct.test01a = type { i32, i32, i32 } -> %__DTT_struct.test01a = type { i32, i32, i32 }
; CHECK-NONOPAQUE-DAG: %struct.test02a = type { i32, %struct.test02b } -> %__DTT_struct.test02a = type { i32, %struct.test02b }
; CHECK-NONOPAQUE-DAG: %struct.test03a = type { i32 } -> %__DTT_struct.test03a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test03b = type { i32, %struct.test03a } -> %__DDT_struct.test03b = type { i32, %__DTT_struct.test03a }
; CHECK-NONOPAQUE-DAG: %struct.test04a = type { i32, %struct.test04b* } -> %__DTT_struct.test04a = type { i32, %struct.test04b* }
; CHECK-NONOPAQUE-DAG: %struct.test05a = type { i32 } -> %__DTT_struct.test05a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test05b = type { i32, %struct.test05a* } -> %__DDT_struct.test05b = type { i32, %__DTT_struct.test05a* }
; CHECK-NONOPAQUE-DAG: %struct.test06a = type { i32 } -> %__DTT_struct.test06a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test06b = type { %struct.test06a* } -> %__DDT_struct.test06b = type { %__DTT_struct.test06a* }
; CHECK-NONOPAQUE-DAG: %struct.test06c = type { %struct.test06a* } -> %__DDT_struct.test06c = type { %__DTT_struct.test06a* }
; CHECK-NONOPAQUE-DAG: %struct.test06d = type { %struct.test06a } -> %__DDT_struct.test06d = type { %__DTT_struct.test06a }
; CHECK-NONOPAQUE-DAG: %struct.test06e = type { %struct.test06a } -> %__DDT_struct.test06e = type { %__DTT_struct.test06a }
; CHECK-NONOPAQUE-DAG: %struct.test07a = type { i32, %struct.test07c } -> %__DTT_struct.test07a = type { i32, %struct.test07c }
; CHECK-NONOPAQUE-DAG: %struct.test07b = type { i32, %struct.test07a* } -> %__DDT_struct.test07b = type { i32, %__DTT_struct.test07a* }
; CHECK-NONOPAQUE-DAG: %struct.test08a = type { i32, %struct.test08a*, %struct.test08b* } -> %__DTT_struct.test08a = type { i32, %__DTT_struct.test08a*, %__DDT_struct.test08b* }
; CHECK-NONOPAQUE-DAG: %struct.test08b = type { i32, %struct.test08b*, %struct.test08c* } -> %__DDT_struct.test08b = type { i32, %__DDT_struct.test08b*, %__DDT_struct.test08c* }
; CHECK-NONOPAQUE-DAG: %struct.test08c = type { i32, %struct.test08c*, %struct.test08a* } -> %__DDT_struct.test08c = type { i32, %__DDT_struct.test08c*, %__DTT_struct.test08a* }
; CHECK-NONOPAQUE-DAG: %struct.test09a = type { i32, i32 } -> %__DTT_struct.test09a = type { i32, i32 }
; CHECK-NONOPAQUE-DAG: %struct.test09b = type { i32, %struct.test09a** } -> %__DDT_struct.test09b = type { i32, %__DTT_struct.test09a** }
; CHECK-NONOPAQUE-DAG: %struct.test10a = type {} -> %__DTT_struct.test10a = type {}
; CHECK-NONOPAQUE-DAG: %struct.test10b = type { %struct.test10a* } -> %__DDT_struct.test10b = type { %__DTT_struct.test10a* }
; CHECK-NONOPAQUE-DAG: %struct.test11a = type opaque -> %__DTT_struct.test11a = type opaque
; CHECK-NONOPAQUE-DAG: %struct.test11b = type { %struct.test11a* } -> %__DDT_struct.test11b = type { %__DTT_struct.test11a* }

; The following are checks that will replace the above checks when opaque pointers are in use.
; Types with pointer dependencies no longer need to be remapped when opaque pointers are used.
; CHECK-OPAQUE-DAG: %struct.test01a = type { i32, i32, i32 } -> %__DTT_struct.test01a = type { i32, i32, i32 }
; CHECK-OPAQUE-DAG: %struct.test02a = type { i32, %struct.test02b } -> %__DTT_struct.test02a = type { i32, %struct.test02b }
; CHECK-OPAQUE-DAG: %struct.test03a = type { i32 } -> %__DTT_struct.test03a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test03b = type { i32, %struct.test03a } -> %__DDT_struct.test03b = type { i32, %__DTT_struct.test03a }
; CHECK-OPAQUE-DAG: %struct.test04a = type { i32, %struct.test04b* } -> %__DTT_struct.test04a = type { i32, %struct.test04b* }
; CHECK-OPAQUE-DAG: %struct.test05a = type { i32 } -> %__DTT_struct.test05a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test06a = type { i32 } -> %__DTT_struct.test06a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test06d = type { %struct.test06a } -> %__DDT_struct.test06d = type { %__DTT_struct.test06a }
; CHECK-OPAQUE-DAG: %struct.test06e = type { %struct.test06a } -> %__DDT_struct.test06e = type { %__DTT_struct.test06a }
; CHECK-OPAQUE-DAG: %struct.test07a = type { i32, %struct.test07c } -> %__DTT_struct.test07a = type { i32, %struct.test07c }
; CHECK-OPAQUE-DAG: %struct.test08a = type { i32, %struct.test08a*, %struct.test08b* } -> %__DTT_struct.test08a = type { i32, %__DTT_struct.test08a*, %struct.test08b* }
; CHECK-OPAQUE-DAG: %struct.test09a = type { i32, i32 } -> %__DTT_struct.test09a = type { i32, i32 }
; CHECK-OPAQUE-DAG: %struct.test10a = type {} -> %__DTT_struct.test10a = type {}
; CHECK-OPAQUE-DAG: %struct.test11a = type opaque -> %__DTT_struct.test11a = type opaque
; CHECK-LABEL: End DTransOPTypeRemapper DTrans Type Mappings



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

!intel.dtrans.types = !{!16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41}
