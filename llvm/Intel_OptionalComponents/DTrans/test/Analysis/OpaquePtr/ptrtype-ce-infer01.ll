; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery where a BitcastOperator needs to be
; inferred using another constant operator expression

%test01.struct.anon = type { i32, i32 }
%test01.union.u = type { double }

@g_u01 = internal global %test01.union.u zeroinitializer
; When opaque pointers are enabled the bitcast operator will no longer exist causing
; the global to also be seen as being used as a 'test01.struct.anon' type.
; Without opaque pointers, the 'test01.struct.anon' type will be seen as
; the type of the bitcast operator.

; CHECK-LABEL: @g_u01 = internal global %test01.union.u
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %test01.struct.anon*{{ *$}}
; CHECK-NEXT:      %test01.union.u*{{ *$}}
; CHECK-NEXT:    No element pointees.

define i32 @test01() {
  %value = load i32, ptr getelementptr (%test01.struct.anon,
                                        ptr @g_u01,
                                        i64 0, i32 1)
  ret i32 %value
  }

; CHECK-LABEL: i32 @test01()
; CHECK: CE: ptr getelementptr (%test01.struct.anon, ptr @g_u01, i64 0, i32 1)
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:     Aliased types:
; CHECK-NEXT:       i32*
; CHECK-NEXT:     Element pointees:
; CHECK-NEXT:       %test01.struct.anon @ 1


!1 = !{i32 0, i32 0}  ; i32
!2 = !{double 0.0e+00, i32 0}  ; double
!3 = !{!"S", %test01.struct.anon zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!4 = !{!"S", %test01.union.u zeroinitializer, i32 1, !2} ; { double }

!intel.dtrans.types = !{!3, !4}
