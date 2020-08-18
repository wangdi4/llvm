; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of "Ambiguous GEP" safety condition by DTrans safety analyzer
; for a global variable that uses a GEPOperator with an incompatible type.

%test01.struct.anon = type { i32, i32 }
%test01.union.u = type { double }

@g_u01 = internal global %test01.union.u zeroinitializer
define i32 @test01() {
  %value = load i32, i32* getelementptr (%test01.struct.anon,
                                       %test01.struct.anon*
                                         bitcast (%test01.union.u* @g_u01 to
                                                  %test01.struct.anon*),
                                       i64 0, i32 1)
  ret i32 %value
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: test01.struct.anon
; CHECK: Safety data: Ambiguous GEP{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: test01.union.u
; CHECK: Safety data: Ambiguous GEP | Global instance{{ *$}}


!1 = !{double 0.0e+00, i32 0}  ; double
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"S", %test01.union.u zeroinitializer, i32 1, !1} ; { double }
!4 = !{!"S", %test01.struct.anon zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!dtrans_types = !{!3, !4}
