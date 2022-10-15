; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on bitcast operator when used
; on function types.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are tests for the future opaque pointer form of IR.
; Lines marked with CHECK should remain the same when changing to use opaque pointers.

%struct.test01 = type { i32 }

; Test a call done via a bitcast operator.
; Argument types need to be inferred based on expected types of @indirect01
define internal i32 @call_directly01(i8* "intel_dtrans_func_index"="1" %obj1, i8* "intel_dtrans_func_index"="2" %obj2) !intel.dtrans.func.type !3 {
  ; Use bitcast operator on a function call
  %res1 = call i32 bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*)(i8* %obj1, i8* %obj2)
  ret i32 %res1
}
; CHECK-LABEL: Input Parameters: call_directly01
; CHECK-NONOPAQUE:    Arg 0: i8* %obj1
; CHECK-OPAQUE:    Arg 0: ptr %obj1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: Arg 1: i8* %obj2
; CHECK-OPAQUE: Arg 1: ptr %obj2
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: %res1 = call i32 bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*)(i8* %obj1, i8* %obj2)
; CHECK-NONOPAQUE:   CE: i32 (i8*, i8*)* bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*)
; CHECK-NONOPAQUE-NEXT:  LocalPointerInfo:
; CHECK-NONOPAQUE-NEXT:  Aliased types:
; CHECK-NONOPAQUE-NEXT:    i32 (%struct.test01*, %struct.test01*)*{{ *$}}
; CHECK-NONOPAQUE-NEXT:    i32 (i8*, i8*)*{{ *$}}
; CHECK-NONOPAQUE-NEXT: No element pointees.

; There will not be a constant expression in the form using opaque pointers to be checked.
; CHECK-OPAQUE: %res1 = call i32 @indirect01(ptr %obj1, ptr %obj2)


; Test passing a bitcast function pointer.
define internal void @test01() {
  %st1 = alloca %struct.test01
  %st2 = alloca %struct.test01
  %tmp1 = bitcast %struct.test01* %st1 to i8*
  %tmp2 = bitcast %struct.test01* %st2 to i8*
  ; use bitcast operator to pass a function pointer.
  call void @call_indirect01(i32 (i8*, i8*)* bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*), i8* %tmp1, i8* %tmp2)
  ret void
}
; CHECK-LABEL: void @test01() {
; CHECK-NONOPAQUE: call void @call_indirect01(i32 (i8*, i8*)* bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*), i8* %tmp1, i8* %tmp2)
; CHECK-NONOPAQUE:   CE: i32 (i8*, i8*)* bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*)
; CHECK-NONOPAQUE-NEXT: LocalPointerInfo:
; CHECK-NONOPAQUE-NEXT: Aliased types:
; CHECK-NONOPAQUE-NEXT:   i32 (%struct.test01*, %struct.test01*)*{{ *$}}
; CHECK-NONOPAQUE-NEXT:   i32 (i8*, i8*)*{{ *$}}
; CHECK-NONOPAQUE-NEXT: No element pointees.

; There will not be a constant expression in the form using opaque pointers to be checked.
; CHECK-OPAQUE: void @call_indirect01(ptr @indirect01, ptr %tmp1, ptr %tmp2)


; These are support routines for above tests. There is nothing of interest to CHECK on these.
define internal void @call_indirect01(i32 (i8*, i8*)* "intel_dtrans_func_index"="1" %fptr, i8* "intel_dtrans_func_index"="2" %obj1, i8* "intel_dtrans_func_index"="3" %obj2) !intel.dtrans.func.type !6 {
  ; Indirect call with metadata info.
  %res2 = call i32 %fptr(i8* %obj1, i8* %obj2), !intel_dtrans_type !4
  ret void
}

define internal i32 @indirect01(%struct.test01* "intel_dtrans_func_index"="1" %arg1, %struct.test01* "intel_dtrans_func_index"="2" %arg2) !intel.dtrans.func.type !8 {
  %f1 = getelementptr %struct.test01, %struct.test01* %arg1, i64 0, i32 0
  %f2 = getelementptr %struct.test01, %struct.test01* %arg2, i64 0, i32 0
  %v1 = load i32, i32* %f1
  %v2 = load i32, i32* %f2
  %sub = sub i32 %v2, %v1
  ret i32 %sub
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2, !2}
!4 = !{!"F", i1 false, i32 2, !1, !2, !2}  ; i32 (i8*, i8*)
!5 = !{!4, i32 1}  ; i32 (i8*, i8*)*
!6 = distinct !{!5, !2, !2}
!7 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!8 = distinct !{!7, !7}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!9}
