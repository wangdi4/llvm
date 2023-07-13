; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s 

target triple = "x86_64-unknown-linux-gnu"

%struct.test01.base = type { i32, i32 }
%struct.test01.derived1 = type { %struct.test01.base, i32 }
@gTest01 = internal global ptr zeroinitializer, !intel_dtrans_type !3
define void @test01(ptr "intel_dtrans_func_index"="1" %pDerived) !intel.dtrans.func.type !5 {
  %pDerived.as.pBase = bitcast ptr %pDerived to ptr
  %pti = ptrtoint ptr %pDerived.as.pBase to i64
  store i64 %pti, ptr bitcast (ptr @gTest01 to ptr)
  ret void
}
; CHECK-LABEL: Input Parameters: test01
; CHECK: %pDerived.as.pBase = bitcast ptr %pDerived to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01.base*{{ *$}}
; CHECK-NEXT:   %struct.test01.derived1*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK: %pti = ptrtoint ptr %pDerived.as.pBase to i64
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
