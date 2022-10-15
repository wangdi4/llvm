; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete pass correctly transforms
; structures that have unused fields and meet the necessary safety conditions
; and that other structures that point to the optimized structure are correctly
; updated.

%struct.test = type { i32, i64, i32 }
%struct.other = type { %struct.test* }
; CHECK-NONOPAQUE-DAG: %__DFDT_struct.other = type { %__DFT_struct.test* }
; CHECK-NONOPAQUE-DAG: %__DFT_struct.test = type { i32, i32 }

; CHECK-OPAQUE-DAG: %struct.other = type { ptr }
; CHECK-OPAQUE-DAG: %__DFT_struct.test = type { i32, i32 }

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !8 {
  ; Allocate two structures.
  %p1 = call i8* @malloc(i64 16)
  %p_test = bitcast i8* %p1 to %struct.test*
  %p2 = call i8* @malloc(i64 16)
  %p_other = bitcast i8* %p2 to %struct.other*

  ; Call a helper function to store p_test in the other struct.
  call void @connect(%struct.test* %p_test, %struct.other* %p_other)

  ; Re-load p_test from p_other and call doSomething
  %pp_test = getelementptr %struct.other, %struct.other* %p_other, i64 0, i32 0
  %p_test2 = load %struct.test*, %struct.test** %pp_test
  %val = call i32 @doSomething(%struct.test* %p_test2)

  ; Free the structures
  call void @free(i8* %p1)
  call void @free(i8* %p2)
  ret i32 %val
}

define i32 @doSomething(%struct.test* "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !4 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valA = load i32, i32* %p_test_A
  %valC = load i32, i32* %p_test_C
  %sum = add i32 %valA, %valC

  ret i32 %sum
}

; CHECK-LABEL: define i32 @main
; CHECK-NONOPAQUE: %p1 = call i8* @malloc(i64 8)
; CHECK-NONOPAQUE: %p_test = bitcast i8* %p1 to %__DFT_struct.test*
; CHECK-NONOPAQUE: %p2 = call i8* @malloc(i64 16)
; CHECK-NONOPAQUE: %p_other = bitcast i8* %p2 to %__DFDT_struct.other*
; CHECK-NONOPAQUE: call void @connect.{{[0-9]+}}(%__DFT_struct.test* %p_test, %__DFDT_struct.other* %p_other)
; CHECK-NONOPAQUE: %pp_test = getelementptr %__DFDT_struct.other, %__DFDT_struct.other* %p_other, i64 0, i32 0
; CHECK-NONOPAQUE: %p_test2 = load %__DFT_struct.test*, %__DFT_struct.test** %pp_test
; CHECK-NONOPAQUE: %val = call i32 @doSomething.{{[0-9]+}}(%__DFT_struct.test* %p_test2)

; CHECK-OPAQUE: %p1 = call ptr @malloc(i64 8)
; CHECK-OPAQUE: %p_test = bitcast ptr %p1 to ptr
; CHECK-OPAQUE: %p2 = call ptr @malloc(i64 16)
; CHECK-OPAQUE: %p_other = bitcast ptr %p2 to ptr
; CHECK-OPAQUE: call void @connect(ptr %p_test, ptr %p_other)
; CHECK-OPAQUE: %pp_test = getelementptr %struct.other, ptr %p_other, i64 0, i32 0
; CHECK-OPAQUE: %p_test2 = load ptr, ptr %pp_test
; CHECK-OPAQUE: %val = call i32 @doSomething(ptr %p_test2)

define void @connect(%struct.test* "intel_dtrans_func_index"="1" %p_test, %struct.other* "intel_dtrans_func_index"="2" %p_other) !intel.dtrans.func.type !6 {
  %pp_test = getelementptr %struct.other, %struct.other* %p_other, i64 0, i32 0
  store %struct.test* %p_test, %struct.test** %pp_test
  ret void
}

; The instruction updating is verified in other tests, so here it is
; sufficient to check the function signatures for the cloned version
; when opaque pointres are not in use. When opaque pointers are used
; there is no change to signatures, so they are not checked.

; CHECK-NONOPAQUE: define internal i32 @doSomething.{{[0-9]+}}
; CHECK-NONOPAQUE-SAME: %__DFT_struct.test*
; CHECK-NONOPAQUE: define internal void @connect.{{[0-9]+}}
; CHECK-NONOPAQUE-SAME: %__DFT_struct.test*
; CHECK-NONOPAQUE-SAME: %__DFDT_struct.other*

declare !intel.dtrans.func.type !10 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !11 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

; Verify that the metadata representation was updated.
; CHECK-NONOPAQUE: !intel.dtrans.types = !{![[S1MD:[0-9]+]], ![[S2MD:[0-9]+]]}
; CHECK-NONOPAQUE: ![[S1MD]] = !{!"S", %__DFT_struct.test zeroinitializer, i32 2, ![[I32MD:[0-9]+]], ![[I32MD]]}
; CHECK-NONOPAQUE: ![[I32MD]] = !{i32 0, i32 0}
; CHECK-NONOPAQUE: ![[S2MD]] = !{!"S", %__DFDT_struct.other zeroinitializer, i32 1, ![[SREF:[0-9]+]]}
; CHECK-NONOPAQUE: ![[SREF]] = !{%__DFT_struct.test zeroinitializer, i32 1}

; CHECK-OPAQUE: !intel.dtrans.types = !{![[S1MD:[0-9]+]], ![[S2MD:[0-9]+]]}
; CHECK-OPAQUE: ![[S1MD]] = !{!"S", %__DFT_struct.test zeroinitializer, i32 2, ![[I32MD:[0-9]+]], ![[I32MD]]}
; CHECK-OPAQUE: ![[I32MD]] = !{i32 0, i32 0}
; CHECK-OPAQUE: ![[S2MD]] = !{!"S", %struct.other zeroinitializer, i32 1, ![[SREF:[0-9]+]]}
; CHECK-OPAQUE: ![[SREF]] = !{%__DFT_struct.test zeroinitializer, i32 1}

!intel.dtrans.types = !{!12, !13}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = distinct !{!3}
!5 = !{%struct.other zeroinitializer, i32 1}  ; %struct.other*
!6 = distinct !{!3, !5}
!7 = !{i8 0, i32 2}  ; i8**
!8 = distinct !{!7}
!9 = !{i8 0, i32 1}  ; i8*
!10 = distinct !{!9}
!11 = distinct !{!9}
!12 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }
!13 = !{!"S", %struct.other zeroinitializer, i32 1, !3} ; { %struct.test* }

