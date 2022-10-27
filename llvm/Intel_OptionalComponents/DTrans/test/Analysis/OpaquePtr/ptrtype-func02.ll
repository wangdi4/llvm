; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Tests function result and argument types on cases where the
; type involves pointers, but the type itself is not a pointer type.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
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
; CHECK-NONOPAQUE: %ar = insertvalue [2 x i32*] undef, i32* %x, 1
; CHECK-OPAQUE: %ar = insertvalue [2 x ptr] undef, ptr %x, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   [2 x i32*]{{ *$}}
; CHECK-NEXT: No element pointees.

define internal "intel_dtrans_func_index"="1" i32* @support_test01([2 x i32*] "intel_dtrans_func_index"="2" %ar) !intel.dtrans.func.type !3 {
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
; CHECK-NONOPAQUE: %x = call <2 x i16*> @support_test02()
; CHECK-OPAQUE: %x = call <2 x ptr> @support_test02()
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   <2 x i16*>{{ *$}}
; CHECK-NEXT: No element pointees.

define "intel_dtrans_func_index"="1" <2 x i16*> @support_test02() !intel.dtrans.func.type !7 {
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
; CHECK-NONOPAQUE: %x = call { i8*, i32 } @support_test03()
; CHECK-OPAQUE: %x = call { ptr, i32 } @support_test03()
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 }{{ *$}}
; CHECK-NEXT: No element pointees.

define internal "intel_dtrans_func_index"="1" {i8*, i32} @support_test03() !intel.dtrans.func.type !11 {
  %partial = insertvalue {i8*, i32} undef, i8* null, 0
  %res = insertvalue {i8*, i32} %partial, i32 512, 1
  ret {i8*, i32} %res
}


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{!"A", i32 2, !1}  ; [2 x i32*]
!3 = distinct !{!1, !2}
!4 = !{i16 0, i32 0}  ; i16
!5 = !{!"V", i32 2, !6}  ; <2 x i16*>
!6 = !{i16 0, i32 1}  ; i16*
!7 = distinct !{!5}
!8 = !{!"L", i32 2, !9, !10}  ; {i8*, i32}
!9 = !{i8 0, i32 1}  ; i8*
!10 = !{i32 0, i32 0}  ; i32
!11 = distinct !{!8}
!12 = !{!"S", %struct.test02 zeroinitializer, i32 1, !4} ; { i16 }

!intel.dtrans.types = !{!12}
