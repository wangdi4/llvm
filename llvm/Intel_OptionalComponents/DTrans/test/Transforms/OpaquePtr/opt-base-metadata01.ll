; RUN: opt -dtransop-allow-typed-pointers -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that metadata gets created for the new types created during
; type transformation for structure types.

; Case with self & circular references
%struct.test01a = type { i32, %struct.test01a*, %struct.test01b* }
%struct.test01b = type { i32, %struct.test01b*, %struct.test01c* }
%struct.test01c = type { %struct.test01d, %struct.test01c*, %struct.test01a* }
%struct.test01d = type { i64, i64 }

@globVar01a = global %struct.test01a zeroinitializer

!intel.dtrans.types = !{!7, !8, !9, !10}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!4 = !{%struct.test01c zeroinitializer, i32 1}  ; %struct.test01c*
!5 = !{%struct.test01d zeroinitializer, i32 0}  ; %struct.test01d
!6 = !{i64 0, i32 0}  ; i64
!7 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !2, !3} ; { i32, %struct.test01a*, %struct.test01b* }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 3, !1, !3, !4} ; { i32, %struct.test01b*, %struct.test01c* }
!9 = !{!"S", %struct.test01c zeroinitializer, i32 3, !5, !4, !2} ; { %struct.test01d, %struct.test01c*, %struct.test01a* }
!10 = !{!"S", %struct.test01d zeroinitializer, i32 2, !6, !6} ; { i64, i64 }

; CHECK: ![[S01A:[0-9]+]] = !{!"S", %__DTT_struct.test01a zeroinitializer, i32 3, ![[I32:[0-9]+]], ![[PTR_S01A:[0-9]+]], ![[PTR_S01B:[0-9]+]]}
; CHECK: ![[I32]] = !{i32 0, i32 0}
; CHECK: ![[PTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 1}
; CHECK: ![[PTR_S01B]] = !{%__DDT_struct.test01b zeroinitializer, i32 1}

; CHECK: ![[S01B:[0-9]+]] = !{!"S", %__DDT_struct.test01b zeroinitializer, i32 3, ![[I32]], ![[PTR_S01B]], ![[PTR_S01C:[0-9]+]]}
; CHECK: ![[PTR_S01C]] = !{%__DDT_struct.test01c zeroinitializer, i32 1}

; CHECK: ![[S01C:[0-9]+]] = !{!"S", %__DDT_struct.test01c zeroinitializer, i32 3, ![[PTR_S01D:[0-9]+]], ![[PTR_S01C]], ![[PTR_S01A]]}
; CHECK: ![[PTR_S01D]] = !{%struct.test01d zeroinitializer, i32 0}

; CHECK: ![[S01D:[0-9]+]] = !{!"S", %struct.test01d zeroinitializer, i32 2, ![[I64:[0-9]+]], ![[I64]]}
; CHECK: ![[I64]] = !{i64 0, i32 0}
