; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on function argument types

; Test argument collection for simple pointer to scalar type.
define internal void @test01(ptr "intel_dtrans_func_index"="1" %arg01) !intel.dtrans.func.type !2 {
  %v1 = load i32, ptr %arg01
  ret void
}
; CHECK-LABEL:  Input Parameters: test01
; CHECK: Arg 0: ptr %arg01
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test argument collection for simple pointer to pointer to structure type.
; Also, checks the analysis of the load instruction.
%struct.test02 = type { i32, i32 }
define internal void @test02(ptr "intel_dtrans_func_index"="1" %arg02) !intel.dtrans.func.type !5 {
  %v2 = load ptr, ptr %arg02
  ret void
}
; CHECK-LABEL:  Input Parameters: test02
; CHECK: Arg 0: ptr %arg02
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:   %struct.test02**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK:  %v2 = load ptr, ptr %arg02
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:   %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test where input argument is declared as void*, and inference will need
; to be done to collect the argument type.

%struct.test03 = type { i32, i32 }
define internal void @test03(ptr "intel_dtrans_func_index"="1" %arg03) !intel.dtrans.func.type !7 {
  %local = alloca i64
  %pti = ptrtoint ptr %arg03 to i64
  store i64 %pti, ptr %local
  call void @helper_test03(ptr %arg03)
  ret void
}
define internal void @helper_test03(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !9 {
  ; Need to use the value to establish the callsite parameter as getting used
  ; as the type.
  %field = getelementptr %struct.test03, ptr %in, i64 0, i32 1
  store i32 0, ptr %field
  ret void
}
; CHECK-LABEL:  Input Parameters: test03
; CHECK:    Arg 0: ptr %arg03
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
