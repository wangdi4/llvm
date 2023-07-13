; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s 


; Tests for inferring the type of an input argument declared as an i8*
; based on the usage types of the argument when the argument is passed
; to another function.

%struct.test01 = type { i32, ptr }
%struct.test02 = type { i64, i64 }

; We are going to infer the type of the second argument.
define void @test01(ptr "intel_dtrans_func_index"="1" %in0, ptr "intel_dtrans_func_index"="2" %in1) !intel.dtrans.func.type !5 {
  %cast = bitcast ptr %in1 to ptr
  call void @user(ptr %in0, ptr %cast)
  %gep = getelementptr %struct.test02, ptr %cast, i64 0, i32 0
  store i64 0, ptr %gep
  ret void
}
; CHECK-LABEL: Input Parameters: test01
; CHECK:    Arg 0: ptr %in0
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01*
; CHECK-NEXT:      No element pointees.

; CHECK:    Arg 1: ptr %in1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      No element pointees.

define void @user(ptr "intel_dtrans_func_index"="1" %in0, ptr "intel_dtrans_func_index"="2" %in1) !intel.dtrans.func.type !7 {
  ret void
}

define void @caller() {
  %a1 = alloca %struct.test01
  %gep = getelementptr %struct.test01, ptr %a1, i64 0, i32 1
  %asP8 = load ptr, ptr %gep
  call void @test01(ptr %a1, ptr %asP8)
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
