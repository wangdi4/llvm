; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete pass correctly transforms
; two structures that have unused fields and meet the necessary safety
; conditions where each of the structures being updated contains a pointer to
; the other.

%struct.test = type { i32, i64, i32, %struct.other* }
%struct.other = type { i32, %struct.test* }

; CHECK-NONOPAQUE-DAG: %__DFT_struct.other = type { %__DFT_struct.test* }
; CHECK-NONOPAQUE-DAG: %__DFT_struct.test = type { i32, i32, %__DFT_struct.other* }

; CHECK-OPAQUE-DAG: %__DFT_struct.other = type { ptr }
; CHECK-OPAQUE-DAG: %__DFT_struct.test = type { i32, i32, ptr }

@result = global i32 zeroinitializer
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

; CHECK-LABEL: define i32 @main
; CHECK-NONOPAQUE: %p1 = call i8* @malloc(i64 16)
; CHECK-NONOPAQUE: %p_test = bitcast i8* %p1 to %__DFT_struct.test*
; CHECK-NONOPAQUE: %p2 = call i8* @malloc(i64 8)
; CHECK-NONOPAQUE: %p_other = bitcast i8* %p2 to %__DFT_struct.other*
; CHECK-NONOPAQUE: %pp_test = getelementptr %__DFT_struct.other, %__DFT_struct.other* %p_other, i64 0, i32 0
; CHECK-NONOPAQUE: store %__DFT_struct.test* %p_test, %__DFT_struct.test** %pp_test
; CHECK-NONOPAQUE: %pp_other = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p_test, i64 0, i32 2
; CHECK-NONOPAQUE: store %__DFT_struct.other* %p_other, %__DFT_struct.other** %pp_other
; CHECK-NONOPAQUE: %pp_test2 = getelementptr %__DFT_struct.other, %__DFT_struct.other* %p_other, i64 0, i32 0
; CHECK-NONOPAQUE: %p_test2 = load %__DFT_struct.test*, %__DFT_struct.test** %pp_test2
; CHECK-NONOPAQUE: %ret = call i1 @doSomething.1(%__DFT_struct.test* %p_test2, %__DFT_struct.other* %p_other)

; CHECK-OPAQUE: %p1 = call ptr @malloc(i64 16)
; CHECK-OPAQUE: %p_test = bitcast ptr %p1 to ptr
; CHECK-OPAQUE: %p2 = call ptr @malloc(i64 8)
; CHECK-OPAQUE: %p_other = bitcast ptr %p2 to ptr
; CHECK-OPAQUE: %pp_test = getelementptr %__DFT_struct.other, ptr %p_other, i64 0, i32 0
; CHECK-OPAQUE: store ptr %p_test, ptr %pp_test
; CHECK-OPAQUE: %pp_other = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 2
; CHECK-OPAQUE: store ptr %p_other, ptr %pp_other
; CHECK-OPAQUE: %pp_test2 = getelementptr %__DFT_struct.other, ptr %p_other, i64 0, i32 0
; CHECK-OPAQUE: %p_test2 = load ptr, ptr %pp_test2
; CHECK-OPAQUE: %ret = call i1 @doSomething(ptr %p_test2, ptr %p_other)

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
  %sum = add i32 %valA, %valC
  store i32 %sum, i32* @result

  ; load a pointer to Other
  %pp_other = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 3
  %p_other = load %struct.other*, %struct.other** %pp_other

  %cmp = icmp eq %struct.other* %p_other_in, %p_other

  ret i1 %cmp
}

; CHECK-NONOPAQUE: define internal i1 @doSomething.1(
; CHECK-NONOPAQUE-SAME:                    %__DFT_struct.test*
; CHECK-NONOPAQUE-SAME:                    %__DFT_struct.other*
; CHECK-NONOPAQUE: %p_test_A = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p_test, i64 0, i32 0
; CHECK-NONOPAQUE-NOT: %p_test_B = getelementptr %__DFT_struct.test,
; CHECK-NONOPAQUE: %p_test_C = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p_test, i64 0, i32 1
; CHECK-NONOPAQUE: %pp_other = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p_test, i64 0, i32 2

; CHECK-OPAQUE: define i1 @doSomething(
; CHECK-OPAQUE: %p_test_A = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 0
; CHECK-OPAQUE-NOT: %p_test_B = getelementptr %__DFT_struct.test,
; CHECK-OPAQUE: %p_test_C = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 1
; CHECK-OPAQUE: %pp_other = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 2



declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !10 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!11, !12}

; Verify that the metadata representation was updated.
; CHECK: !intel.dtrans.types = !{![[S1MD:[0-9]+]], ![[S2MD:[0-9]+]]}
; CHECK: ![[S1MD]] = !{!"S", %__DFT_struct.test zeroinitializer, i32 3, ![[I32MD:[0-9]+]], ![[I32MD]], ![[SOTHERREF:[0-9]+]]}
; CHECK: ![[I32MD]] = !{i32 0, i32 0}
; CHECK: ![[SOTHERREF]] = !{%__DFT_struct.other zeroinitializer, i32 1}
; CHECK: ![[S2MD]] = !{!"S", %__DFT_struct.other zeroinitializer, i32 1, ![[STESTREF:[0-9]+]]}
; CHECK: ![[STESTREF]] = !{%__DFT_struct.test zeroinitializer, i32 1}

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
