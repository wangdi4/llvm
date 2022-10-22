; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop-typelist=struct.other -passes=dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop-max-struct=1 -passes=dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s

; In this case, both %struct.test and %struct.other are candidates for deletion
; based on the selection heuristics.
; 1) Verify that the command line flag, -dtrans-deletefieldop-typelist, can be
; used to select the specific structure to transform, and that the other one does
; not get selected.

; 2) Verify that the command line flag, -dtrans-deletefieldop-max-structure, can
; used to limit the transformation to just one structure. This flag will cause
; the candidate structures to be sorted by name to make struct.other be a
; deterministic choice.

%struct.test = type { i32, i64, i32, %struct.other* }
%struct.other = type { i32, %struct.test* }

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !7 {
  ; Allocate two structures.
  %p1 = call i8* @malloc(i64 24)
  %p_test = bitcast i8* %p1 to %struct.test*
  %p2 = call i8* @malloc(i64 16)
  %p_other = bitcast i8* %p2 to %struct.other*

  ; Store a pointer to each structures in the other
  %pp_test = getelementptr %struct.other, %struct.other* %p_other, i64 0, i32 1
  store %struct.test* %p_test, %struct.test** %pp_test
  %pp_other = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 3
  store %struct.other* %p_other, %struct.other** %pp_other

  ; Re-load p_test from p_other and call doSomething
  %pp_test2 = getelementptr %struct.other, %struct.other* %p_other, i64 0, i32 1
  %p_test2 = load %struct.test*, %struct.test** %pp_test2
  %ret = call i1 @doSomething(%struct.test* %p_test2, %struct.other* %p_other)

  ; Free the structures
  call void @free(i8* %p1)
  call void @free(i8* %p2)
  ret i32 0
}

define i1 @doSomething(%struct.test* "intel_dtrans_func_index"="1" %p_test, %struct.other* "intel_dtrans_func_index"="2" %p_other_in) !intel.dtrans.func.type !5 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ; load a pointer to Other
  %pp_other = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 3
  %p_other = load %struct.other*, %struct.other** %pp_other

  %cmp = icmp eq %struct.other* %p_other_in, %p_other

  ret i1 %cmp
}

; To avoid output order dependencies, check that struct.test is not printed before
; or after struct.other.

; CHECK: Delete field for opaque pointers: looking for candidate structures
; CHECK-NOT: Selected for deletion: %struct.test
; CHECK: Selected for deletion: %struct.other
; CHECK-NOT: Selected for deletion: %struct.test


declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !10 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.other zeroinitializer, i32 1}  ; %struct.other*
!4 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!5 = distinct !{!4, !3}
!6 = !{i8 0, i32 2}  ; i8**
!7 = distinct !{!6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = distinct !{!8}
!11 = !{!"S", %struct.test zeroinitializer, i32 4, !1, !2, !1, !3} ; { i32, i64, i32, %struct.other* }
!12 = !{!"S", %struct.other zeroinitializer, i32 2, !1, !4} ; { i32, %struct.test* }

!intel.dtrans.types = !{!11, !12}
