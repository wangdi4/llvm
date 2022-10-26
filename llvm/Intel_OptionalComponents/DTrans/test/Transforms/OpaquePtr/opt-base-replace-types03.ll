; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -passes=dtransop-optbasetest -debug-only=dtransop-optbase -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test the ability to create new LLVM and DTrans types for the types selected by
; the transformation and by the base class converting the dependent types. This
; test is being developed before the IR gets rewritten to use the new types,
; so this will just check the debug traces for the types.

; Case with type to be converted used as a pointer type as part of a function
; type.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32, i32 }
%struct.test01c = type { i32, %struct.test01a* (i32, %struct.test01b*)* }

@globVar01c = global %struct.test01c zeroinitializer

; %struct.test01b will not be changed, because it does not use
; %struct.test01a or %struct.test01c

; CHECK-LABEL: TypeRemapper types after preparing types:
; CHECK-NEXT: DTransOPTypeRemapper LLVM Type Mappings:
; CHECK-NONOPAQUE-DAG: %struct.test01a = type { i32, i32 } -> %__DTT_struct.test01a = type { i32, i32 }
; CHECK-NONOPAQUE-DAG: %struct.test01c = type { i32, %struct.test01a* (i32, %struct.test01b*)* } -> %__DDT_struct.test01c = type { i32, %__DTT_struct.test01a* (i32, %struct.test01b*)* }

; CHECK-OPAQUE-DAG: %struct.test01a = type { i32, i32 } -> %__DTT_struct.test01a = type { i32, i32 }
; CHECK-LABEL: End of DTransOPTypeRemapper LLVM Type Mappings

; CHECK-LABEL: DTransOPTypeRemapper DTrans Type Mappings:
; CHECK-NONOPAQUE-DAG: %struct.test01c = type { i32, %struct.test01a* (i32, %struct.test01b*)* } -> %__DDT_struct.test01c = type { i32, %__DTT_struct.test01a* (i32, %struct.test01b*)* }

; CHECK-OPAQUE-DAG: %struct.test01a = type { i32, i32 } -> %__DTT_struct.test01a = type { i32, i32 }
; CHECK-LABEL: End DTransOPTypeRemapper DTrans Type Mappings

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !1, !4}  ; %struct.test01a* (i32, %struct.test01b*)
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!5 = !{!2, i32 1}  ; %struct.test01a* (i32, %struct.test01b*)*
!6 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!7 = !{!"S", %struct.test01b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!8 = !{!"S", %struct.test01c zeroinitializer, i32 2, !1, !5} ; { i32, %struct.test01a* (i32, %struct.test01b*)* }

!intel.dtrans.types = !{!6, !7, !8}
