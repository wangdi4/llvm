; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on getelementptr instructions involving invalid
; or unknown indices


; Test an array access that is outside the range of the array.
; This will track as if element 22 is accessed, which is the same as the legacy
; LocalPointerAnalyzer. Although, the access appears to be out of bounds, that is
; only the case, if %in were the start of the array, so it will be treated as an
; element pointee.
define internal i64 @test01(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !4 {
  %ar_oob =  getelementptr [20 x i64], ptr %in, i64 0, i32 22
  %val = load i64, ptr%ar_oob
  ret i64 %val
}
; CHECK-LABEL: i64 @test01
; CHECK: %ar_oob = getelementptr [20 x i64], ptr %in, i64 0, i32 22
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [20 x i64] @ 22


; Test with variable index.
; TODO: The legacy LocalPointerAnalyzer treats this as element 0 when
; -dtrans-outofboundsok=false.
; Otherwise, it does not treat it as an element pointee. For now, the pointer
; type analyzer only has this behavior.
@test_var2 = internal global [20 x i64] zeroinitializer
define internal i64 @test02(i32 %idx) {
  %ar_n =  getelementptr [20 x i64], ptr @test_var2, i64 0, i32 %idx
  %val = load i64, ptr%ar_n
  ret i64 %val
}
; CHECK-LABEL: i64 @test02
; CHECK: %ar_n = getelementptr [20 x i64], ptr @test_var2, i64 0, i32 %idx
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [20 x i64] @ UnknownOffset


; Test handling for a zero-sized array element at the end of the structure.
%struct.test03 = type { i64, [0 x i8] }
define void @test03(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !8 {
  %f1 = getelementptr %struct.test03, ptr %in, i64 0, i32 1, i32 2
  %v8 = load i8, ptr %f1
  ret void
}
; CHECK-LABEL: void @test03
; CHECK: %f1 = getelementptr %struct.test03, ptr %in, i64 0, i32 1, i32 2
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [0 x i8] @ 2


!1 = !{!2, i32 1}  ; [20 x i64]*
!2 = !{!"A", i32 20, !3}  ; [20 x i64]
!3 = !{i64 0, i32 0}  ; i64
!4 = distinct !{!1}
!5 = !{!"A", i32 0, !6}  ; [0 x i8]
!6 = !{i8 0, i32 0}  ; i8
!7 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test03 zeroinitializer, i32 2, !3, !5} ; { i64, [0 x i8] }

!intel.dtrans.types = !{!9}
