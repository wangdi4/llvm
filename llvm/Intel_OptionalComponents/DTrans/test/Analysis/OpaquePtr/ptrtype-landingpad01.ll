; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test pointer type recovery on landing pad instructions

%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32 }
define i32 @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStructA, %struct.test01b* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !4 {
  %addr1 = getelementptr %struct.test01a, %struct.test01a* %pStructA, i64 0, i32 0
  %addr2 = getelementptr %struct.test01b, %struct.test01b* %pStructB, i64 0, i32 0
  %val1 = load i32, i32* %addr1
  %val2 = load i32, i32* %addr2
  %add = add i32 %val1, %val2
  ret i32 %add
}

define void @test01i(%struct.test01a* "intel_dtrans_func_index"="1" %s1, %struct.test01b* "intel_dtrans_func_index"="2" %s2) personality i32 (...)* @__gxx_personality_v0 !intel.dtrans.func.type !5 {
  %sum = invoke i32 @test01(%struct.test01a* %s1, %struct.test01b* %s2) to label %good unwind label %bad
good:
  ret void

bad:
  %lp = landingpad { i8*, i32 }
        cleanup
  resume { i8*, i32 } %lp
}
; CHECK-LABEL: void @test01
; CHECK-NONOPAQUE: %lp = landingpad { i8*, i32 }
; CHECK-NONOPAQUE-NEXT: cleanup
; CHECK-OPAQUE: %lp = landingpad { ptr, i32 }
; CHECK-OPAQUE-NEXT: cleanup
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees


declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @_Znwm(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!4 = distinct !{!2, !3}
!5 = distinct !{!2, !3}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!8, !9}

