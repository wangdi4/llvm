; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

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
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field
; CHECK: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data:  Global instance | Field address taken call{{ *$}}
; CHECK: End LLVMType: %struct.test01


declare !intel.dtrans.func.type !4 double @strtod(i8* "intel_dtrans_func_index"="1", i8** "intel_dtrans_func_index"="2")

!1 = !{i8 0, i32 1}  ; i8*
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!1, !3}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !2} ; { i8*, i64, i64 }

!intel.dtrans.types = !{!5}
