; RUN: opt -dtransop-allow-typed-pointers -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test that global variable and alias get their type changed when types
; are remapped.

; Simple case that does not involve pointers
%struct.test01a = type { i32, i32, i32 }

@globVar01a = internal global %struct.test01a { i32 1, i32 2, i32 3 }
@globAlias = internal alias %struct.test01a, %struct.test01a* @globVar01a

; CHECK-NONOPAQUE-DAG: @globVar01a = internal global %__DTT_struct.test01a { i32 1, i32 2, i32 3 }
; CHECK-NONOPAQUE-DAG: @globAlias = internal alias %__DTT_struct.test01a, %__DTT_struct.test01a* @globVar01a

; CHECK-OPAQUE-DAG: @globVar01a = internal global %__DTT_struct.test01a { i32 1, i32 2, i32 3 }
; CHECK-OPAQUE-DAG: @globAlias = internal alias %__DTT_struct.test01a, ptr @globVar01a

!intel.dtrans.types = !{!2}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
