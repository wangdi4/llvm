; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s 

; Test pointer type recovery on bitcast operator when used
; on function types.


%struct.test01 = type { i32 }

; Test a call done via a bitcast operator.
; Argument types need to be inferred based on expected types of @indirect01
define internal i32 @call_directly01(ptr "intel_dtrans_func_index"="1" %obj1, ptr "intel_dtrans_func_index"="2" %obj2) !intel.dtrans.func.type !3 {
  ; Use bitcast operator on a function call
  %res1 = call i32 bitcast (ptr @indirect01 to ptr)(ptr %obj1, ptr %obj2)
  ret i32 %res1
}
; CHECK-LABEL: Input Parameters: call_directly01
; CHECK:    Arg 0: ptr %obj1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK: Arg 1: ptr %obj2
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; There will not be a constant expression in the form using opaque pointers to be checked.
; CHECK: %res1 = call i32 @indirect01(ptr %obj1, ptr %obj2)


; Test passing a bitcast function pointer.
define internal void @test01() {
  %st1 = alloca %struct.test01
  %st2 = alloca %struct.test01
  %tmp1 = bitcast ptr %st1 to ptr
  %tmp2 = bitcast ptr %st2 to ptr
  ; use bitcast operator to pass a function pointer.
  call void @call_indirect01(ptr bitcast (ptr @indirect01 to ptr), ptr %tmp1, ptr %tmp2)
  ret void
}
; CHECK-LABEL: void @test01() {

; There will not be a constant expression in the form using opaque pointers to be checked.
; CHECK: void @call_indirect01(ptr @indirect01, ptr %tmp1, ptr %tmp2)


; These are support routines for above tests. There is nothing of interest to CHECK on these.
define internal void @call_indirect01(ptr "intel_dtrans_func_index"="1" %fptr, ptr "intel_dtrans_func_index"="2" %obj1, ptr "intel_dtrans_func_index"="3" %obj2) !intel.dtrans.func.type !6 {
  ; Indirect call with metadata info.
  %res2 = call i32 %fptr(ptr %obj1, ptr %obj2), !intel_dtrans_type !4
  ret void
}

define internal i32 @indirect01(ptr "intel_dtrans_func_index"="1" %arg1, ptr "intel_dtrans_func_index"="2" %arg2) !intel.dtrans.func.type !8 {
  %f1 = getelementptr %struct.test01, ptr %arg1, i64 0, i32 0
  %f2 = getelementptr %struct.test01, ptr %arg2, i64 0, i32 0
  %v1 = load i32, ptr %f1
  %v2 = load i32, ptr %f2
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
