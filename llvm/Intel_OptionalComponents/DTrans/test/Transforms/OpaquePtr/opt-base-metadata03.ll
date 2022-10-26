; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that metadata gets created for the new types created during
; type transformation for vector types.

%struct.test01a = type { i32 }
%struct.test01b = type { %struct.test01a*, <4 x %struct.test01a*> }

@globVar01b = global %struct.test01b zeroinitializer

!intel.dtrans.types = !{!4, !5}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{!"V", i32 4, !2}  ; <4 x %struct.test01a*>
!4 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i32 }
!5 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !3} ; { %struct.test01a*, <4 x %struct.test01a*> }

; CHECK: !intel.dtrans.types = !{![[S01A:[0-9]+]], ![[S01B:[0-9]+]]}

; CHECK: ![[S01A]] = !{!"S", %__DTT_struct.test01a zeroinitializer, i32 1, ![[I32:[0-9]+]]}
; CHECK: ![[I32]] = !{i32 0, i32 0}

; CHECK: ![[S01B]] = !{!"S", %__DDT_struct.test01b zeroinitializer, i32 2, ![[PTR_S01A:[0-9]+]], ![[VECT_4xPTR_S01A:[0-9]+]]}
; CHECK: ![[PTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 1}
; CHECK: ![[VECT_4xPTR_S01A]] = !{!"V", i32 4, ![[PTR_S01A]]}
