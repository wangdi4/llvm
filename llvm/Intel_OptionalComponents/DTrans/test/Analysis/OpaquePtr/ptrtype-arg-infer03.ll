; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Tests for inferring the type of an input argument declared as an i8*
; based on the usage types of the argument when the argument is passed
; to another function.

%struct.test01 = type { i32, i8* }
%struct.test02 = type { i64, i64 }

; We are going to infer the type of the second argument.
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %in0, i8* "intel_dtrans_func_index"="2" %in1) !intel.dtrans.func.type !5 {
  %cast = bitcast i8* %in1 to %struct.test02*
  call void @user(%struct.test01* %in0, %struct.test02* %cast)
  %gep = getelementptr %struct.test02, %struct.test02* %cast, i64 0, i32 0
  store i64 0, i64* %gep
  ret void
}
; CHECK-LABEL: Input Parameters: test01
; CHECK-NONOPAQUE: Arg 0: %struct.test01* %in0
; CHECK-OPAQUE:    Arg 0: ptr %in0
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01*
; CHECK-NEXT:      No element pointees.

; CHECK-NONOPAQUE: Arg 1: i8* %in1
; CHECK-OPAQUE:    Arg 1: ptr %in1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      No element pointees.

define void @user(%struct.test01* "intel_dtrans_func_index"="1" %in0, %struct.test02* "intel_dtrans_func_index"="2" %in1) !intel.dtrans.func.type !7 {
  ret void
}

define void @caller() {
  %a1 = alloca %struct.test01
  %gep = getelementptr %struct.test01, %struct.test01* %a1, i64 0, i32 1
  %asP8 = load i8*, i8** %gep
  call void @test01(%struct.test01* %a1, i8* %asP8)
  ret void
}



!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4, !2}
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!4, !6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i8* }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !3, !3} ; { i64, i64 }

!intel.dtrans.types = !{!8, !9}
