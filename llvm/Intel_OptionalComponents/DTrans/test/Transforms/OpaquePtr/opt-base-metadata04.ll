; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that metadata gets created for the new types created during
; type transformation for function types.

%struct.test01a = type { i32 }
%struct.test01b = type { %struct.test01a*, void (%struct.test01a*)*, %struct.test01a* ()*, %struct.test01a* (i32, ...)*, i32 (metadata)* }

@globVar01b = global %struct.test01b zeroinitializer

!intel.dtrans.types = !{!13, !14}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{!"F", i1 false, i32 1, !4, !2}  ; void (%struct.test01a*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!3, i32 1}  ; void (%struct.test01a*)*
!6 = !{!"F", i1 false, i32 0, !2}  ; %struct.test01a* ()
!7 = !{!6, i32 1}  ; %struct.test01a* ()*
!8 = !{!"F", i1 true, i32 1, !2, !1}  ; %struct.test01a* (i32, ...)
!9 = !{!8, i32 1}  ; %struct.test01a* (i32, ...)*
!10 = !{!"F", i1 false, i32 1, !1, !11}  ; i32 (metadata)
!11 = !{!"metadata", i32 0}  ; metadata
!12 = !{!10, i32 1}  ; i32 (metadata)*
!13 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i32 }
!14 = !{!"S", %struct.test01b zeroinitializer, i32 5, !2, !5, !7, !9, !12} ; { %struct.test01a*, void (%struct.test01a*)*, %struct.test01a* ()*, %struct.test01a* (i32, ...)*, i32 (metadata)* }

; CHECK: !intel.dtrans.types = !{![[S01A:[0-9]+]], ![[S01B:[0-9]+]]}

; CHECK: ![[S01A]] = !{!"S", %__DTT_struct.test01a zeroinitializer, i32 1, ![[I32:[0-9]+]]}
; CHECK: ![[I32]] = !{i32 0, i32 0}

; CHECK: ![[S01B]] = !{!"S", %__DDT_struct.test01b zeroinitializer, i32 5, ![[PTR_S01A:[0-9]+]], ![[VOID_FUNC_PTR:[0-9+]]], ![[S01A_FUNC_PTR:[0-9]+]], ![[S01A_VARARG_FUNC_PTR:[0-9]+]], ![[METADATA_FUNC_PTR:[0-9]+]]}

; CHECK: ![[PTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 1}

; CHECK: ![[VOID_FUNC_PTR]] = !{![[VOID_FUNC:[0-9]+]], i32 1}
; CHECK: ![[VOID_FUNC]] = !{!"F", i1 false, i32 1, ![[VOID_TYPE:[0-9]+]], ![[PTR_S01A]]}
; CHECK: ![[VOID_TYPE]] = !{!"void", i32 0}

; CHECK: ![[S01A_FUNC_PTR]] = !{![[S01A_FUNC:[0-9]+]], i32 1}
; CHECK: ![[S01A_FUNC]] = !{!"F", i1 false, i32 0, ![[PTR_S01A]]}

; CHECK: ![[S01A_VARARG_FUNC_PTR]] = !{![[S01A_VARARG_FUNC:[0-9]+]], i32 1}
; CHECK: ![[S01A_VARARG_FUNC]] = !{!"F", i1 true, i32 1, ![[PTR_S01A]], ![[I32]]}

; CHECK: ![[METADATA_FUNC_PTR]] = !{![[METADATA_FUNC:[0-9]+]], i32 1}
; CHECK: ![[METADATA_FUNC]] = !{!"F", i1 false, i32 1, ![[I32]], ![[METADATA_TYPE:[0-9]+]]}
; CHECK: ![[METADATA_TYPE]] = !{!"metadata", i32 0}
