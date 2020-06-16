; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

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
; CHECK-FUT-NEXT:  %test01.struct.anon*{{ *$}}
; CHECK-NEXT:      %test01.union.u*{{ *$}}
; CHECK-NEXT:    No element pointees.

define i32 @test01() {
  %value = load i32, i32* getelementptr (%test01.struct.anon,
                                       %test01.struct.anon*
                                         bitcast (%test01.union.u* @g_u01 to
                                                  %test01.struct.anon*),
                                       i64 0, i32 1)
  ret i32 %value
  }

; CHECK-LABEL: i32 @test01()
; CHECK-CUR: CE: i32* getelementptr (%test01.struct.anon, %test01.struct.anon* bitcast (%test01.union.u* @g_u01 to %test01.struct.anon*), i64 0, i32 1)
; CHECK-FUT: CE: p0 getelementptr inbounds (%test01.struct.anon, p0 @g_u01, i64 0, i32 1)
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:     Aliased types:
; CHECK-NEXT:       i32*
; CHECK-NEXT:     Element pointees:
; CHECK-NEXT:       %test01.struct.anon @ 1

; The bitcast will only appear in the form without opaque pointers
; CHECK-CUR: CE: %test01.struct.anon* bitcast (%test01.union.u* @g_u01 to %test01.struct.anon*)
; CHECK-CUR-NEXT:   LocalPointerInfo:
; CHECK-CUR-NEXT:      Aliased types:
; CHECK-CUR-NEXT:        %test01.struct.anon*
; CHECK-CUR-NEXT:        %test01.union.u*
; CHECK-CUR-NEXT:      No element pointees.


!1 = !{double 0.0e+00, i32 0}  ; double
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"S", %test01.union.u zeroinitializer, i32 1, !1} ; { double }
!4 = !{!"S", %test01.struct.anon zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!dtrans_types = !{!3, !4}

