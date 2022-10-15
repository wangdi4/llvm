; RUN: opt -dtransop-allow-typed-pointers -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that metadata gets created for the new types created during
; type transformation for array types.

%struct.test01a = type { i32 }
%struct.test01b = type { %struct.test01a, %struct.test01a*, %struct.test01a**, [2 x %struct.test01a], [2 x [4 x %struct.test01a]], [8 x %struct.test01a*], [16 x %struct.test01a**]* }

@globVar01b = global %struct.test01b zeroinitializer

!intel.dtrans.types = !{!11, !12}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = !{%struct.test01a zeroinitializer, i32 2}  ; %struct.test01a**
!5 = !{!"A", i32 2, !2}  ; [2 x %struct.test01a]
!6 = !{!"A", i32 2, !7}  ; [2 x [4 x %struct.test01a]]
!7 = !{!"A", i32 4, !2}  ; [4 x %struct.test01a]
!8 = !{!"A", i32 8, !3}  ; [8 x %struct.test01a*]
!9 = !{!10, i32 1}  ; [16 x %struct.test01a**]*
!10 = !{!"A", i32 16, !4}  ; [16 x %struct.test01a**]
!11 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i32 }
!12 = !{!"S", %struct.test01b zeroinitializer, i32 7, !2, !3, !4, !5, !6, !8, !9} ; { %struct.test01a, %struct.test01a*, %struct.test01a**, [2 x %struct.test01a], [2 x [4 x %struct.test01a]], [8 x %struct.test01a*], [16 x %struct.test01a**]* }


; CHECK: ![[S01A:[0-9]+]] = !{!"S", %__DTT_struct.test01a zeroinitializer, i32 1, ![[I32:[0-9]+]]}
; CHECK: ![[I32]] = !{i32 0, i32 0}

; CHECK: ![[S01B:[0-9]+]] = !{!"S", %__DDT_struct.test01b zeroinitializer, i32 7, ![[NEST_S01A:[0-9]+]], ![[PTR_S01A:[0-9]+]], ![[PTRPTR_S01A:[0-9]+]], ![[ARRAY_2xS01A:[0-9]+]], ![[ARRAY_2xARRAY4xS01A:[0-9]+]], ![[ARRAY_8xPTR_S01A:[0-9]+]], ![[PTR_ARRAY_16xPTRPTR_S01A:[0-9]+]]}

; CHECK: ![[NEST_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 0}
; CHECK: ![[PTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 1}
; CHECK: ![[PTRPTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 2}

; CHECK: ![[ARRAY_2xS01A]] = !{!"A", i32 2, ![[NEST_S01A]]}

; CHECK: ![[ARRAY_2xARRAY4xS01A]] = !{!"A", i32 2, ![[ARRAY4xS01A:[0-9]+]]}
; CHECK: ![[ARRAY4xS01A]] = !{!"A", i32 4, ![[NEST_S01A]]}

; CHECK: ![[ARRAY_8xPTR_S01A]] = !{!"A", i32 8, ![[PTR_S01A]]}

; CHECK: ![[PTR_ARRAY_16xPTRPTR_S01A]] = !{![[ARRAY_16xPTRPTR_S01A:[0-9]+]], i32 1}
; CHECK: ![[ARRAY_16xPTRPTR_S01A]] = !{!"A", i32 16, ![[PTRPTR_S01A]]}
