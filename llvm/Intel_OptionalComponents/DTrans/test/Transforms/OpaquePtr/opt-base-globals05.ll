; RUN: opt -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that global variables get their type changed when types are remapped
; for cases where the global variable type is a pointer type to check that
; the metadata information gets updated.

%struct.test01a = type { i32, i32, i32 }

@globPtr01a = internal global ptr null, !intel_dtrans_type !2
@globalPtrPtr01a = internal global ptr @globPtr01a, !intel_dtrans_type !3

!intel.dtrans.types = !{!4}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{%struct.test01a zeroinitializer, i32 2}  ; %struct.test01a**
!4 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }


; When opaque pointers are used, the types for the variables will change,
; but the metadata attached to the them needs to be updated, because the
; tracked pointer types for DTrans is changing.

; CHECK-DAG: @globPtr01a = internal global ptr null, !intel_dtrans_type ![[PTR_S01A:[0-9]+]]
; CHECK-DAG: @globalPtrPtr01a = internal global ptr @globPtr01a, !intel_dtrans_type ![[PTRPTR_S01A:[0-9]+]]


; CHECK-DAG: ![[PTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 1}
; CHECK-DAG: ![[PTRPTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 2}

