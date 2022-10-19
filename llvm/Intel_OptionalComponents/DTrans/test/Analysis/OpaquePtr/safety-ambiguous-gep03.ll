; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

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
; CHECK: LLVMType: %test01.struct.anon
; CHECK: Safety data: Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %test01.struct.anon

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %test01.union.u
; CHECK: Safety data: Ambiguous GEP | Global instance{{ *$}}
; CHECK: End LLVMType: %test01.union.u


!1 = !{i32 0, i32 0}  ; i32
!2 = !{double 0.0e+00, i32 0}  ; double
!3 = !{!"S", %test01.struct.anon zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!4 = !{!"S", %test01.union.u zeroinitializer, i32 1, !2} ; { double }

!intel.dtrans.types = !{!3, !4}
