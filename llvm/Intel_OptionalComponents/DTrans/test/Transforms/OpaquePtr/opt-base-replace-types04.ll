; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test the ability to create new LLVM and DTrans types for the types selected by
; the transformation and by the base class converting the dependent types. This
; test is being developed before the IR gets rewritten to use the new types,
; so this will just check the debug traces for the types.
; These cases use literal struct types.

%struct.test01a = type { i32 }
%struct.test01b = type { i32, { i32, i32, %struct.test01a } }

%struct.test02a = type { i32 }
%struct.test02b = type { i32, {i32, %struct.test02a* } }

%struct.test03a = type { i32, {i32, %struct.test03b* } }
%struct.test03b = type { i32 }

@globVar01b = global %struct.test01b zeroinitializer
@globVar02b = global %struct.test02b zeroinitializer
@globVar03a = global %struct.test03a zeroinitializer

; CHECK-LABEL: TypeRemapper types after preparing types:
; CHECK-NEXT: DTransOPTypeRemapper LLVM Type Mappings:
; CHECK-NONOPAQUE-DAG: %struct.test01a = type { i32 } -> %__DTT_struct.test01a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test01b = type { i32, { i32, i32, %struct.test01a } } -> %__DDT_struct.test01b = type { i32, { i32, i32, %__DTT_struct.test01a } }
; CHECK-NONOPAQUE-DAG: %struct.test02a = type { i32 } -> %__DTT_struct.test02a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test02b = type { i32, { i32, %struct.test02a* } } -> %__DDT_struct.test02b = type { i32, { i32, %__DTT_struct.test02a* } }
; CHECK-NONOPAQUE-DAG: %struct.test03a = type { i32, { i32, %struct.test03b* } } -> %__DTT_struct.test03a = type { i32, { i32, %struct.test03b* } }

; CHECK-OPAQUE-DAG: %struct.test01b = type { i32, { i32, i32, %struct.test01a } } -> %__DDT_struct.test01b = type { i32, { i32, i32, %__DTT_struct.test01a } }
; CHECK-OPAQUE-DAG: %struct.test01a = type { i32 } -> %__DTT_struct.test01a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test02a = type { i32 } -> %__DTT_struct.test02a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test03a = type { i32, { i32, ptr } } -> %__DTT_struct.test03a = type { i32, { i32, ptr } }
; CHECK-LABEL: End of DTransOPTypeRemapper LLVM Type Mappings

; CHECK-LABEL: DTransOPTypeRemapper DTrans Type Mappings:
; CHECK-NONOPAQUE-DAG: %struct.test01a = type { i32 } -> %__DTT_struct.test01a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test01b = type { i32, { i32, i32, %struct.test01a } } -> %__DDT_struct.test01b = type { i32, { i32, i32, %__DTT_struct.test01a } }
; CHECK-NONOPAQUE-DAG: %struct.test02a = type { i32 } -> %__DTT_struct.test02a = type { i32 }
; CHECK-NONOPAQUE-DAG: %struct.test02b = type { i32, { i32, %struct.test02a* } } -> %__DDT_struct.test02b = type { i32, { i32, %__DTT_struct.test02a* } }
; CHECK-NONOPAQUE-DAG: %struct.test03a = type { i32, { i32, %struct.test03b* } } -> %__DTT_struct.test03a = type { i32, { i32, %struct.test03b* } }

; CHECK-OPAQUE-DAG: %struct.test01a = type { i32 } -> %__DTT_struct.test01a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test01b = type { i32, { i32, i32, %struct.test01a } } -> %__DDT_struct.test01b = type { i32, { i32, i32, %__DTT_struct.test01a } }
; CHECK-OPAQUE-DAG: %struct.test02a = type { i32 } -> %__DTT_struct.test02a = type { i32 }
; CHECK-OPAQUE-DAG: %struct.test03a = type { i32, { i32, %struct.test03b* } } -> %__DTT_struct.test03a = type { i32, { i32, %struct.test03b* } }
; CHECK-LABEL: End DTransOPTypeRemapper DTrans Type Mappings


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!3 = !{!"L", i32 3, !1, !1, !4}  ; { i32, i32, %struct.test01a }
!4 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!5 = !{!"L", i32 2, !1, !6}  ; {i32, i32,  %struct.test02a* }
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = !{!"L", i32 2, !1, !8}  ; {i32, i32,  %struct.test03b* }
!8 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!9 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i32 }
!10 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !3} ; { i32, { i32, i32, %struct.test01a } }
!11= !{!"S", %struct.test02a zeroinitializer, i32 1, !1} ; { i32 }
!12 = !{!"S", %struct.test02b zeroinitializer, i32 2, !1, !5} ; { i32, {i32, %struct.test02a* } }
!13 = !{!"S", %struct.test03a zeroinitializer, i32 2, !1, !7} ; { i32, {i32, %struct.test03b* } }
!14 = !{!"S", %struct.test03b zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!9, !10, !11, !12, !13, !14}
