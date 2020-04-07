; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Tests function result and argument types on cases where the
; type involves pointers, but the type itself is not a pointer type.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.


; Test with a call that has an array of pointers argument.
define internal void @test01() {
  %x = alloca i32
  %ar = insertvalue [2 x i32*] undef, i32* %x, 1
  %res = call i32* @support_test01([2 x i32*] %ar)
  ret void
}
; CHECK-LABEL: internal void @test01()
; CHECK-CUR: %ar = insertvalue [2 x i32*] undef, i32* %x, 1
; CHECK-FUT: %ar = insertvalue [2 x p0] undef, p0 %x, 1
; CHECK:    LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   [2 x i32*]{{ *$}}
; CHECK-NEXT: No element pointees.

define internal i32* @support_test01([2 x i32*] %ar) !dtrans_type !1 {
  %res = extractvalue [2 x i32*] %ar, 1
  ret i32* %res
}


; Test with a call that returns a vector of pointers.
%struct.test02 = type { i16 }
@var_test02 = global [1 x %struct.test02] zeroinitializer
define internal void @test02() {
  %x = call <2 x i16*> @support_test02()
  ret void
}
; CHECK-LABEL: internal void @test02()
; CHECK-CUR: %x = call <2 x i16*> @support_test02()
; CHECK-FUT: %x = call <2 x p0> @support_test02()
; CHECK:    LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   <2 x i16*>{{ *$}}
; CHECK-NEXT: No element pointees.

define <2 x i16*> @support_test02() !dtrans_type !5 {
  %vec1 = getelementptr [1 x %struct.test02], [1 x %struct.test02]* @var_test02, <2 x i16> zeroinitializer, <2 x i64> zeroinitializer
  %vec2 = bitcast <2 x %struct.test02*> %vec1 to <2 x i16*>
  ret <2 x i16*> %vec2
}


; Test with a call that returns a literal structure containing a pointer.
define internal void @test03() {
  %x = call {i8*, i32} @support_test03()
  ret void
}
; CHECK-LABEL: internal void @test03()
; CHECK-CUR: %x = call { i8*, i32 } @support_test03()
; CHECK-FUT: %x = call { p0, i32 } @support_test03()
; CHECK:    LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 }{{ *$}}
; CHECK-NEXT: No element pointees.

define internal {i8*, i32} @support_test03() !dtrans_type !8 {
  %partial = insertvalue {i8*, i32} undef, i8* null, 0
  %res = insertvalue {i8*, i32} %partial, i32 512, 1
  ret {i8*, i32} %res
}


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; i32* ([2 x i32*])
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{!"A", i32 2, !2}  ; [2 x i32*]
!4 = !{i16 0, i32 0}  ; i16
!5 = !{!"F", i1 false, i32 0, !6}  ; <2 x i16*> ()
!6 = !{!"V", i32 2, !7}  ; <2 x i16*>
!7 = !{i16 0, i32 1}  ; i16*
!8 = !{!"F", i1 false, i32 0, !9}  ; {i8*, i32} ()
!9 = !{!"L", i32 2, !10, !11}  ; {i8*, i32}
!10 = !{i8 0, i32 1}  ; i8*
!11 = !{i32 0, i32 0}  ; i32
!12 = !{!"S", %struct.test02 zeroinitializer, i32 1, !4} ; { i16 }

!dtrans_types = !{!12}
