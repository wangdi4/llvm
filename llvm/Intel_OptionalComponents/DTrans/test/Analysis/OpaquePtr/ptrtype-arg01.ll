; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on function argument types

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test argument collection for simple pointer to scalar type.
define internal void @test01(i32* %arg01) !dtrans_type !1 {
  %v1 = load i32, i32* %arg01
  ret void
}
; CHECK-LABEL:  Input Parameters: test01
; CHECK-CUR:    Arg 0: i32* %arg01
; CHECK-FUT:    Arg 0: p0 %arg01
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i32*
; CHECK-NEXT: No element pointees.


; Test argument collection for simple pointer to pointer to structure type.
; Also, checks the analysis of the load instruction.
%struct.test02 = type { i32, i32 }
define internal void @test02(%struct.test02** %arg02) !dtrans_type !5 {
  %v2 = load %struct.test02*, %struct.test02** %arg02
  ret void
}
; CHECK-LABEL:  Input Parameters: test02
; CHECK-CUR:    Arg 0: %struct.test02** %arg02
; CHECK-FUT:    Arg 0: p0 %arg02
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test02**
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %v2 = load %struct.test02*, %struct.test02** %arg02
; CHECK-FUT:  %v2 = load p0, p0 %arg02
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test02*
; CHECK-NEXT: No element pointees.


; Test where input argument is declared as void*, and inference will need
; to be done to collect the argument type.
; TODO: This test is a placeholder for now, because it requires analysis
; of bitcast and inferring types from uses to be implemented to get the
; complete results.
%struct.test03 = type { i32, i32 }
define internal void @test03(i8* %arg03) !dtrans_type !11 {
  %local = alloca i64
  %pti = ptrtoint i8* %arg03 to i64
  store i64 %pti, i64* %local
  %bc = bitcast i8* %arg03 to %struct.test03*
  call void @helper_test03(%struct.test03* %bc)
  ret void
}
define internal void @helper_test03(%struct.test03* %in) !dtrans_type !8 {
  ret void
}
; CHECK-LABEL:  Input Parameters: test03
; CHECK-CUR:    Arg 0: i8* %arg03
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; TODO:         %struct.test03*
; CHECK-NEXT:   i8*
; CHECK-NEXT: No element pointees.


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; void (i32*)
!2 = !{!"void", i32 0}  ; void
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{i32 0, i32 0}  ; i32
!5 = !{!"F", i1 false, i32 1, !2, !6}  ; void (%struct.test02**)
!6 = !{!7, i32 2}  ; %struct.test02**
!7 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!8 = !{!"F", i1 false, i32 1, !2, !9}  ; void (%struct.test03*)
!9 = !{!10, i32 1}  ; %struct.test03*
!10 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!11 = !{!"F", i1 false, i32 1, !2, !12}  ; void (i8*)
!12 = !{i8 0, i32 1}  ; i8*
!13 = !{i8 0, i32 0}  ; i8
!14 = !{!"S", %struct.test02 zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!15 = !{!"S", %struct.test03 zeroinitializer, i32 2, !4, !4} ; { i32, i32 }

!dtrans_types = !{!14, !15}
