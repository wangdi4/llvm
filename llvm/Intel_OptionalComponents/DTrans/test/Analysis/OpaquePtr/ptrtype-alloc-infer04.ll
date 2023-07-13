; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test an allocation that is not a structure type and gets written to a field
; that does not look like a pointer type because of a union in the source code.


; In this case the pointer type for the store instruction is %union.test01*. The
; pointer type analyzer should not treat the value operand as being
; %union.test01 because that will break the allocation analyzer of the safety
; analyzer which is expecting a pointer type to be allocated.
%struct.test01 = type { i32, %union.test01 }
%union.test01 = type { double }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %pStr, ptr "intel_dtrans_func_index"="2" %pStruct) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !6 {

  %len = call i64 @strlen(ptr %pStr)
  %len1 = add i64 %len, 1
  %mem = invoke ptr @_Znam(i64 %len1)
          to label %good unwind label %fail
good:
  %uaddr = getelementptr %struct.test01, ptr %pStruct, i64 0, i32 1
  %uaddr.p8 = bitcast ptr %uaddr to ptr
  store ptr %mem, ptr %uaddr.p8
  br label %done

done:
  ret void

fail:
  %lp = landingpad { ptr, i32 }
  cleanup
  resume { ptr, i32 } %lp
}
; CHECK-LABEL: define internal void @test01
; CHECK: %mem = invoke ptr @_Znam(i64 %len1)
; CHECK-NEXT: to label %good unwind label %fail
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:  i8*{{ *$}}
; CHECK-NEXT: No element pointees.

declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !7 i64 @strlen(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @_Znam(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%union.test01 zeroinitializer, i32 0}  ; %union.test01
!3 = !{double 0.0e+00, i32 0}  ; double
!4 = !{i8 0, i32 1}  ; i8*
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = distinct !{!4, !5}
!7 = distinct !{!4}
!8 = distinct !{!4}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, %union.test01 }
!10 = !{!"S", %union.test01 zeroinitializer, i32 1, !3} ; { double }

!intel.dtrans.types = !{!9, !10}
