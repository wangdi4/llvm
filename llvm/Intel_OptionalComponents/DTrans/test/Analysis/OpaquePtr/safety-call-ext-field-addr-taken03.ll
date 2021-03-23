; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Passing a pointer-to-pointer type that is a member of a structure
; to a library call.

%struct.test01 = type { i8*, i64, i64 }
@str01 = internal global [200 x i8] zeroinitializer
@net01 = internal global %struct.test01 zeroinitializer
define void @test01() {
  %tmp = call double @strtod(i8* getelementptr ([200 x i8], [200 x i8]* @str01, i64 0, i32 0),
                             i8** getelementptr (%struct.test01, %struct.test01* @net01, i64 0, i32 0))
  ret void
}
; In this case, there is no special handling needed related to the "Address
; taken" flag, as seen in safety-call-ext-field-addr-taken02.ll, because the
; structure does not begin with an array of i8 elements.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field
; CHECK: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data:  Global instance | Field address taken call{{ *$}}


declare double @strtod(i8*, i8**)

!1 = !{i8 0, i32 1}  ; i8*
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"F", i1 false, i32 2, !4, !1, !5}  ; double (i8*, i8**)
!4 = !{double 0.0e+00, i32 0}  ; double
!5 = !{i8 0, i32 2}  ; i8**
!6 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !2} ; { i8*, i64, i64 }
!7 = !{!"strtod", !3}

!dtrans_types = !{!6}
!dtrans_decl_types = !{!7}
