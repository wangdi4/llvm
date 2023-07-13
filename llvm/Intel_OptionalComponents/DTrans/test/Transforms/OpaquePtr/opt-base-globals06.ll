; RUN: opt -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that global variable and alias get their type changed when types
; are remapped.

; Simple case that does not involve pointers
%struct.test01a = type { i32, i32, i32 }

@globVar01a = internal global %struct.test01a { i32 1, i32 2, i32 3 }
@globAlias = internal alias %struct.test01a, ptr @globVar01a


; CHECK-DAG: @globVar01a = internal global %__DTT_struct.test01a { i32 1, i32 2, i32 3 }
; CHECK-DAG: @globAlias = internal alias %__DTT_struct.test01a, ptr @globVar01a

!intel.dtrans.types = !{!2}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
