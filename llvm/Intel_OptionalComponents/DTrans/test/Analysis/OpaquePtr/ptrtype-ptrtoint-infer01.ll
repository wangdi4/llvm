; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR

%struct.test01.base = type { i32, i32 }
%struct.test01.derived1 = type { %struct.test01.base, i32 }
@gTest01 = internal global %struct.test01.base* zeroinitializer, !dtrans_type !3
define void @test01(%struct.test01.derived1* %pDerived) !dtrans_type !4 {
  %pDerived.as.pBase = bitcast %struct.test01.derived1* %pDerived to %struct.test01.base*
  %pti = ptrtoint %struct.test01.base* %pDerived.as.pBase to i64
  store i64 %pti, i64* bitcast (%struct.test01.base** @gTest01 to i64*)
  ret void
}
; CHECK-LABEL: Input Parameters: test01
; CHECK-CUR: %pDerived.as.pBase = bitcast %struct.test01.derived1* %pDerived to %struct.test01.base*
; CHECK-FUT: %pDerived.as.pBase = bitcast p0 %pDerived to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01.base*{{ *$}}
; CHECK-NEXT:   %struct.test01.derived1*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR: %pti = ptrtoint %struct.test01.base* %pDerived.as.pBase to i64
; CHECK-FUT: %pti = ptrtoint p0 %pDerived.as.pBase to i64
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01.base*{{ *$}}
; CHECK-NEXT:   %struct.test01.derived1*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"R", %struct.test01.base zeroinitializer, i32 0}  ; %struct.test01.base
!3 = !{!2, i32 1}  ; %struct.test01.base*
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01.derived1*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01.derived1*
!7 = !{!"R", %struct.test01.derived1 zeroinitializer, i32 0}  ; %struct.test01.derived1
!8 = !{!"S", %struct.test01.base zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test01.derived1 zeroinitializer, i32 2, !2, !1} ; { %struct.test01.base, i32 }

!dtrans_types = !{!8, !9}
