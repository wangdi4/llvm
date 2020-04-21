; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on bitcast operator when used
; on function types.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are tests for the future opaque pointer form of IR.
; Lines marked with CHECK should remain the same when changing to use opaque pointers.

%struct.test01 = type { i32 }

; Test a call done via a bitcast operator.
; Argument types need to be inferred based on expected types of @indirect01
define internal i32 @call_directly01(i8* %obj1, i8* %obj2) !dtrans_type !2 {
  ; Use bitcast operator on a function call
  %res1 = call i32 bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*)(i8* %obj1, i8* %obj2)
  ret i32 %res1
}
; CHECK-LABEL: Input Parameters: call_directly01
; CHECK-CUR:    Arg 0: i8* %obj1
; CHECK-FUT:    Arg 0: p0 %obj1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR: Arg 1: i8* %obj2
; CHECK-FUT: Arg 1: p0 %obj2
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR: %res1 = call i32 bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*)(i8* %obj1, i8* %obj2)
; CHECK-CUR:   CE: i32 (i8*, i8*)* bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*)
; CHECK-CUR-NEXT:  LocalPointerInfo:
; CHECK-CUR-NEXT:  Aliased types:
; CHECK-CUR-NEXT:    i32 (%struct.test01*, %struct.test01*)*{{ *$}}
; CHECK-CUR-NEXT:    i32 (i8*, i8*)*{{ *$}}
; CHECK-CUR-NEXT: No element pointees.

; There will not be a constant expression in the form using opaque pointers to be checked.
; CHECK-FUT: %res1 = call i32 @indirect01(p0 %obj1, p0 %obj2)


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
; CHECK-CUR: call void @call_indirect01(i32 (i8*, i8*)* bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*), i8* %tmp1, i8* %tmp2)
; CHECK-CUR:   CE: i32 (i8*, i8*)* bitcast (i32 (%struct.test01*, %struct.test01*)* @indirect01 to i32 (i8*, i8*)*)
; CHECK-CUR-NEXT: LocalPointerInfo:
; CHECK-CUR-NEXT: Aliased types:
; CHECK-CUR-NEXT:   i32 (%struct.test01*, %struct.test01*)*{{ *$}}
; CHECK-CUR-NEXT:   i32 (i8*, i8*)*{{ *$}}
; CHECK-CUR-NEXT: No element pointees.

; There will not be a constant expression in the form using opaque pointers to be checked.
; CHECK-FUT: void @call_indirect01(p0 @indirect01, p0 %tmp1, p0 %tmp2)


; These are support routines for above tests. There is nothing of interest to CHECK on these.
define internal void @call_indirect01(i32 (i8*, i8*)* %fptr, i8* %obj1, i8* %obj2) !dtrans_type !4 {
  ; Indirect call with metadata info.
  %res2 = call i32 %fptr(i8* %obj1, i8* %obj2), !dtrans_type !2
  ret void
}

define internal i32 @indirect01(%struct.test01* %arg1, %struct.test01* %arg2) !dtrans_type !7 {
  %f1 = getelementptr %struct.test01, %struct.test01* %arg1, i64 0, i32 0
  %f2 = getelementptr %struct.test01, %struct.test01* %arg2, i64 0, i32 0
  %v1 = load i32, i32* %f1
  %v2 = load i32, i32* %f2
  %sub = sub i32 %v2, %v1
  ret i32 %sub
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !1, !3, !3}  ; i32 (i8*, i8*)
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{!"F", i1 false, i32 3, !5, !6, !3, !3}  ; void (i32 (i8*, i8*)*, i8*, i8*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!2, i32 1}  ; i32 (i8*, i8*)*
!7 = !{!"F", i1 false, i32 2, !1, !8, !8}  ; i32 (%struct.test01*, %struct.test01*)
!8 = !{!9, i32 1}  ; %struct.test01*
!9 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!10 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32 }

!dtrans_types = !{!10}
