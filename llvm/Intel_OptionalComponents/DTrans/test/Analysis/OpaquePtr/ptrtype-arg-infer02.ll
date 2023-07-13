; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s 

; Tests for inferring the type of an input argument declared as an i8*
; based on the usage types of the argument. In these cases, the i8* is
; just used as an i8*, so the parameter type should not be marked as
; UNHANDLED.

; A case where the incoming argument is just used as an i8*.
define i8 @test01(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !2 {
  %old = load i8, ptr %arg
  store i8 0, ptr %arg
  ret i8 %old
}
; CHECK-LABEL: Input Parameters: test01
; CHECK:    Arg 0: ptr %arg
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT:  No element pointees.


; A case where the incoming arguments are just used as i8* types,
; involving 'ptrtoint' instructions.
define i64 @test02(ptr "intel_dtrans_func_index"="1" %arg1, ptr "intel_dtrans_func_index"="2" %arg2) !intel.dtrans.func.type !3 {
  %arg1.as.i64 = ptrtoint ptr %arg1 to i64
  %arg2.as.i64 = ptrtoint ptr %arg2 to i64
  %length = sub i64 %arg2.as.i64, %arg1.as.i64
  ret i64 %length
}
; CHECK-LABEL: Input Parameters: test02
; CHECK:    Arg 0: ptr %arg
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT:  No element pointees.

; CHECK:    Arg 1: ptr %arg
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT:  No element pointees.


; A case where the incoming argument is never used, so the
; inference will not return a type. This should not trigger
; UNHANDLED because there is no need to identify the type of
; the unused value.
define void @test03(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !4 {
  ret void
}
; CHECK-LABEL: Input Parameters: test03
; CHECK:    Arg 0: ptr %arg
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT:  No element pointees.


!1 = !{i8 0, i32 1}  ; i8*
!2 = distinct !{!1}
!3 = distinct !{!1, !1}
!4 = distinct !{!1}

!intel.dtrans.types = !{}
