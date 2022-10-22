; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the dtrans delete pass correctly transforms
; structures that have unused fields and meet the necessary safety conditions.

%struct.test = type { i32, i64, i32 }
; CHECK: %__DFT_struct.test = type { i32, i32 }

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !6 {
  ; Allocate a structure.
  %p = call i8* @malloc(i64 16)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret i32 %val
}
; CHECK-LABEL: define i32 @main
; CHECK-NONOPAQUE: %p = call i8* @malloc(i64 8)
; CHECK-NONOPAQUE: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK-NONOPAQUE: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

; CHECK-OPAQUE: %p = call ptr @malloc(i64 8)
; CHECK-OPAQUE: %p_test = bitcast ptr %p to ptr
; CHECK-OPAQUE: %val = call i32 @doSomething(ptr %p_test)

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
; CHECK-NONOPAQUE: define internal i32 @doSomething.1
; CHECK-NONOPAQUE: %p_test_A = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p_test, i64 0, i32 0
; CHECK-NONOPAQUE-NOT: %p_test_B = getelementptr
; CHECK-NONOPAQUE: %p_test_C = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p_test, i64 0, i32 1

; CHECK-OPAQUE: define i32 @doSomething
; CHECK-OPAQUE: %p_test_A = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 0
; CHECK-OPAQUE-NOT: %p_test_B = getelementptr
; CHECK-OPAQUE: %p_test_C = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 1

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !9 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!10}

; Verify that the metadata representation was updated.
; CHECK: !intel.dtrans.types = !{![[SMD:[0-9]+]]}
; CHECK: ![[SMD]] = !{!"S", %__DFT_struct.test zeroinitializer, i32 2, ![[I32MD:[0-9]+]], ![[I32MD]]}
; CHECK: ![[I32MD]] = !{i32 0, i32 0}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = distinct !{!3}
!5 = !{i8 0, i32 2}  ; i8**
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = distinct !{!7}
!10 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

