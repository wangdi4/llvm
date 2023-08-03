; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the AOS-to-SOA transformation rejects structures
; allocated in a function that is not traced back as having been made by
; 'main' at the start of the program.
;
; %struct.test01 is not supported because there is not a direct path seen
; between 'main' and the allocation function.
; %struct.dep01 is not supported because there is a global variable of the type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i64, i32 }
%struct.dep01 = type { i64, ptr }
@glob = internal global %struct.dep01 zeroinitializer
@func_ptr = global ptr @test01, !intel_dtrans_type !6

define i32 @main() {
  call void @indirect_invoker()
  ret i32 0
}

; This function makes is to the allocation site is not easily traced back
; to main.
define void @indirect_invoker() {
  %func_addr = load ptr, ptr @func_ptr
  tail call void %func_addr(i64 16), !intel_dtrans_type !4
  ret void
}

define void @test01(i64 %count) {
  %mem = call ptr @calloc(i64 %count, i64 24)
  store ptr %mem, ptr getelementptr (%struct.dep01, ptr @glob, i64 0, i32 1)
  ret void
}
; CHECK-DAG: AOS-to-SOA rejecting -- Unsupported safety data: %struct.dep01
; CHECK-DAG: AOS-to-SOA rejecting -- Multiple call paths: %struct.test01

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = !{!"F", i1 false, i32 1, !5, !2}  ; void (i64)
!5 = !{!"void", i32 0}  ; void
!6 = !{!4, i32 1}  ; void (i64)*
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }
!10 = !{!"S", %struct.dep01 zeroinitializer, i32 2, !2, !3} ; { i64, %struct.test01* }

!intel.dtrans.types = !{!9, !10}
