; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR

; Tests for inferring the type of an input argument declared as an i8*
; based on the usage types of the argument. In these cases, the i8* is
; just used as an i8*, so the parameter type should not be marked as
; UNHANDLED.

; A case where the incoming argument is just used as an i8*.
define i8 @test01(i8* %arg) !dtrans_type !1 {
  %old = load i8, i8* %arg
  store i8 0, i8* %arg
  ret i8 %old
}
; CHECK-LABEL: Input Parameters: test01
; CHECK-CUR:    Arg 0: i8* %arg
; CHECK-FUT:    Arg 0: p0 %arg
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT:  No element pointees.


; A case where the incoming arguments are just used as i8* types,
; involving 'ptrtoint' instructions.
define i64 @test02(i8* %arg1, i8* %arg2) !dtrans_type !4 {
  %arg1.as.i64 = ptrtoint i8* %arg1 to i64
  %arg2.as.i64 = ptrtoint i8* %arg2 to i64
  %length = sub i64 %arg2.as.i64, %arg1.as.i64
  ret i64 %length
}
; CHECK-LABEL: Input Parameters: test02
; CHECK-CUR:    Arg 0: i8* %arg
; CHECK-FUT:    Arg 0: p0 %arg
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT:  No element pointees.

; CHECK-CUR:    Arg 1: i8* %arg
; CHECK-FUT:    Arg 1: p0 %arg
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT:  No element pointees.


; A case where the incoming argument is never used, so the
; inference will not return a type. This should not trigger
; UNHANDLED because there is no need to identify the type of
; the unused value.
define void @test03(i8* %arg) !dtrans_type !1 {
  ret void
}
; CHECK-LABEL: Input Parameters: test03
; CHECK-CUR:    Arg 0: i8* %arg
; CHECK-FUT:    Arg 0: p0 %arg
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT:  No element pointees.


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; i8 (i8*)
!2 = !{i8 0, i32 0}  ; i8
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{!"F", i1 false, i32 2, !5, !3, !3}  ; i64 (i8*, i8*)
!5 = !{i64 0, i32 0}  ; i64

!dtrans_types = !{}
