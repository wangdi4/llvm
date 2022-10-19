; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on function argument types

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test argument collection for simple pointer to scalar type.
define internal void @test01(i32* "intel_dtrans_func_index"="1" %arg01) !intel.dtrans.func.type !2 {
  %v1 = load i32, i32* %arg01
  ret void
}
; CHECK-LABEL:  Input Parameters: test01
; CHECK-NONOPAQUE:    Arg 0: i32* %arg01
; CHECK-OPAQUE:    Arg 0: ptr %arg01
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test argument collection for simple pointer to pointer to structure type.
; Also, checks the analysis of the load instruction.
%struct.test02 = type { i32, i32 }
define internal void @test02(%struct.test02** "intel_dtrans_func_index"="1" %arg02) !intel.dtrans.func.type !5 {
  %v2 = load %struct.test02*, %struct.test02** %arg02
  ret void
}
; CHECK-LABEL:  Input Parameters: test02
; CHECK-NONOPAQUE:    Arg 0: %struct.test02** %arg02
; CHECK-OPAQUE:    Arg 0: ptr %arg02
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:   %struct.test02**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE:  %v2 = load %struct.test02*, %struct.test02** %arg02
; CHECK-OPAQUE:  %v2 = load ptr, ptr %arg02
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:   %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test where input argument is declared as void*, and inference will need
; to be done to collect the argument type.

%struct.test03 = type { i32, i32 }
define internal void @test03(i8* "intel_dtrans_func_index"="1" %arg03) !intel.dtrans.func.type !7 {
  %local = alloca i64
  %pti = ptrtoint i8* %arg03 to i64
  store i64 %pti, i64* %local
  %bc = bitcast i8* %arg03 to %struct.test03*
  call void @helper_test03(%struct.test03* %bc)
  ret void
}
define internal void @helper_test03(%struct.test03* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !9 {
  ; Need to use the value to establish the callsite parameter as getting used
  ; as the type.
  %field = getelementptr %struct.test03, %struct.test03* %in, i64 0, i32 1
  store i32 0, i32* %field
  ret void
}
; CHECK-LABEL:  Input Parameters: test03
; CHECK-NONOPAQUE:    Arg 0: i8* %arg03
; CHECK-OPAQUE:    Arg 0: ptr %arg03
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:   %struct.test03*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{i32 0, i32 1}  ; i32*
!2 = distinct !{!1}
!3 = !{i32 0, i32 0}  ; i32
!4 = !{%struct.test02 zeroinitializer, i32 2}  ; %struct.test02**
!5 = distinct !{!4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!11 = !{!"S", %struct.test03 zeroinitializer, i32 2, !3, !3} ; { i32, i32 }

!intel.dtrans.types = !{!10, !11}
