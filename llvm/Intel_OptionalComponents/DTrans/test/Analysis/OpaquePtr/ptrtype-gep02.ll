; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on getelementptr instructions involving invalid
; or unknown indices

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test an array access that is outside the range of the array.
; This will track as if element 22 is accessed, which is the same as the legacy
; LocalPointerAnalyzer. Although, the access appears to be out of bounds, that is
; only the case, if %in were the start of the array, so it will be treated as an
; element pointee.
define internal i64 @test01([20 x i64]* %in) !dtrans_type !1 {
  %ar_oob =  getelementptr [20 x i64], [20 x i64]* %in, i64 0, i32 22
  %val = load i64, i64*%ar_oob
  ret i64 %val
}
; CHECK-LABEL: i64 @test01
; CHECK-CUR: %ar_oob = getelementptr [20 x i64], [20 x i64]* %in, i64 0, i32 22
; CHECK-FUT: %ar_oob = getelementptr [20 x i64], p0 %in, i64 0, i32 22
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [20 x i64] @ 22


; Test with variable index.
; TODO: The legacy LocalPointerAnalyzer treats this as element 0 when
; -dtrans-outofboundsok=false.
; Otherwise, it does not treat it as an element pointee. For now, the pointer
; type analyzer only has this behavior.
@test_var2 = internal global [20 x i64] zeroinitializer
define internal i64 @test02(i32 %idx) {
  %ar_n =  getelementptr [20 x i64], [20 x i64]* @test_var2, i64 0, i32 %idx
  %val = load i64, i64*%ar_n
  ret i64 %val
}
; CHECK-LABEL: i64 @test02
; CHECK-CUR: %ar_n = getelementptr [20 x i64], [20 x i64]* @test_var2, i64 0, i32 %idx
; CHECK-FUT: %ar_n = getelementptr [20 x i64], p0 @test_var2, i64 0, i32 %idx
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT:  No element pointees.


; Test handling for a zero-sized array element at the end of the structure.
%struct.test03 = type { i64, [0 x i8] }
define void @test03(%struct.test03* %in) !dtrans_type !7 {
  %f1 = getelementptr %struct.test03, %struct.test03* %in, i64 0, i32 1, i32 2
  %v8 = load i8, i8* %f1
  ret void
}
; CHECK-LABEL: void @test03
; CHECK-CUR: %f1 = getelementptr %struct.test03, %struct.test03* %in, i64 0, i32 1, i32 2
; CHECK-FUT: %f1 = getelementptr %struct.test03, p0 %in, i64 0, i32 1, i32 2
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i8*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [0 x i8] @ 2


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; i64 ([20 x i64]*)
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!4, i32 1}  ; [20 x i64]*
!4 = !{!"A", i32 20, !2}  ; [20 x i64]
!5 = !{!"A", i32 0, !6}  ; [0 x i8]
!6 = !{i8 0, i32 0}  ; i8
!7 = !{!"F", i1 false, i32 1, !8, !9}  ; void (%struct.test03*)
!8 = !{!"void", i32 0}  ; void
!9 = !{!10, i32 1}  ; %struct.test03*
!10 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!11 = !{!"S", %struct.test03 zeroinitializer, i32 2, !2, !5} ; { i64, [0 x i8] }

!dtrans_types = !{!11}