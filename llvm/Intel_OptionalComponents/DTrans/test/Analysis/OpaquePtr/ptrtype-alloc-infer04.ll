; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test an allocation that is not a structure type and gets written to a field
; that does not look like a pointer type because of a union in the source code.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; In this case the pointer type for the store instruction is %union.test01*. The
; pointer type analyzer should not treat the value operand as being
; %union.test01 because that will break the allocation analyzer of the safety
; analyzer which is expecting a pointer type to be allocated.
%struct.test01 = type { i32, %union.test01 }
%union.test01 = type { double }
define internal void @test01(i8* "intel_dtrans_func_index"="1" %pStr, %struct.test01* "intel_dtrans_func_index"="2" %pStruct) personality i32 (...)* @__gxx_personality_v0 !intel.dtrans.func.type !6 {

  %len = call i64 @strlen(i8* %pStr)
  %len1 = add i64 %len, 1
  %mem = invoke i8* @_Znam(i64 %len1)
          to label %good unwind label %fail
good:
  %uaddr = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %uaddr.p8 = bitcast %union.test01* %uaddr to i8**
  store i8* %mem, i8** %uaddr.p8
  br label %done

done:
  ret void

fail:
  %lp = landingpad { i8*, i32 }
  cleanup
  resume { i8*, i32 } %lp
}
; CHECK-LABEL: define internal void @test01
; CHECK-NONOPAQUE: %mem = invoke i8* @_Znam(i64 %len1)
; CHECK-OPAQUE: %mem = invoke ptr @_Znam(i64 %len1)
; CHECK-NEXT: to label %good unwind label %fail
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:  i8*{{ *$}}
; CHECK-NEXT: No element pointees.

declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !7 i64 @strlen(i8* "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" i8* @_Znam(i64)

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
