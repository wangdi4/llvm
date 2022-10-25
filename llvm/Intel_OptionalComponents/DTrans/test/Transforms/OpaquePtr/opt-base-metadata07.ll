; RUN: opt -dtransop-allow-typed-pointers -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that metadata gets created for the new types created during
; type transformation for empty and opaque structure types.

; Case with empty and opaque structure types
%struct.test01a = type {}
%struct.test02a = type opaque
%struct.test03a = type { %struct.test01a*, %struct.test02a* }

@globVar03a = global %struct.test03a zeroinitializer

!intel.dtrans.types = !{!3, !4, !5}

!1 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!2 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!3 = !{!"S", %struct.test01a zeroinitializer, i32 0} ; {}
!4 = !{!"S", %struct.test02a zeroinitializer, i32 -1} ; opaque
!5 = !{!"S", %struct.test03a zeroinitializer, i32 2, !1, !2} ; { %struct.test01a*, %struct.test02a* }

; CHECK: !intel.dtrans.types = !{![[S01A:[0-9]+]], ![[S02A:[0-9]+]], ![[S03A:[0-9]+]]}

; CHECK: ![[S01A]] = !{!"S", %__DTT_struct.test01a zeroinitializer, i32 0}
; CHECK: ![[S02A]] = !{!"S", %__DTT_struct.test02a zeroinitializer, i32 -1}
; CHECK: ![[S03A]] = !{!"S", %__DTT_struct.test03a zeroinitializer, i32 2, ![[PTR_S01A:[0-9]+]], ![[PTR_S02A:[0-9]+]]}
; CHECK: ![[PTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 1}
; CHECK: ![[PTR_S02A]] = !{%__DTT_struct.test02a zeroinitializer, i32 1}
