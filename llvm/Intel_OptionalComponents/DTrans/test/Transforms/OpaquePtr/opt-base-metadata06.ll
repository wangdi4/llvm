; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that metadata gets created for the new types created during
; type transformation, and remains the same for unchanged types

%struct.test01a = type { %struct.test01b* }
%struct.test01b = type { i32 }

@globVar01b = global %struct.test01b zeroinitializer

!intel.dtrans.types = !{!3, !4}

!1 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b* }
!4 = !{!"S", %struct.test01b zeroinitializer, i32 1, !2} ; { i32 }

; CHECK: !intel.dtrans.types = !{![[S01A:[0-9]+]], ![[S01B:[0-9]+]]}

; CHECK: ![[S01A]] = !{!"S", %__DTT_struct.test01a zeroinitializer, i32 1, ![[PTR_S01B:[0-9]+]]}
; CHECK: ![[PTR_S01B]] = !{%struct.test01b zeroinitializer, i32 1}

; CHECK: ![[S01B]] = !{!"S", %struct.test01b zeroinitializer, i32 1, ![[I32:[0-9]+]]}
; CHECK: ![[I32]] = !{i32 0, i32 0}
