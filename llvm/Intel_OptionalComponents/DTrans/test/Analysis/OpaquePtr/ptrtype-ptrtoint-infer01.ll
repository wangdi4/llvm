; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

%struct.test01.base = type { i32, i32 }
%struct.test01.derived1 = type { %struct.test01.base, i32 }
@gTest01 = internal global %struct.test01.base* zeroinitializer, !intel_dtrans_type !3
define void @test01(%struct.test01.derived1* "intel_dtrans_func_index"="1" %pDerived) !intel.dtrans.func.type !5 {
  %pDerived.as.pBase = bitcast %struct.test01.derived1* %pDerived to %struct.test01.base*
  %pti = ptrtoint %struct.test01.base* %pDerived.as.pBase to i64
  store i64 %pti, i64* bitcast (%struct.test01.base** @gTest01 to i64*)
  ret void
}
; CHECK-LABEL: Input Parameters: test01
; CHECK-NONOPAQUE: %pDerived.as.pBase = bitcast %struct.test01.derived1* %pDerived to %struct.test01.base*
; CHECK-OPAQUE: %pDerived.as.pBase = bitcast ptr %pDerived to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01.base*{{ *$}}
; CHECK-NEXT:   %struct.test01.derived1*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: %pti = ptrtoint %struct.test01.base* %pDerived.as.pBase to i64
; CHECK-OPAQUE: %pti = ptrtoint ptr %pDerived.as.pBase to i64
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01.base*{{ *$}}
; CHECK-NEXT:   %struct.test01.derived1*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01.base zeroinitializer, i32 0}  ; %struct.test01.base
!3 = !{%struct.test01.base zeroinitializer, i32 1}  ; %struct.test01.base*
!4 = !{%struct.test01.derived1 zeroinitializer, i32 1}  ; %struct.test01.derived1*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01.base zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!7 = !{!"S", %struct.test01.derived1 zeroinitializer, i32 2, !2, !1} ; { %struct.test01.base, i32 }

!intel.dtrans.types = !{!6, !7}
