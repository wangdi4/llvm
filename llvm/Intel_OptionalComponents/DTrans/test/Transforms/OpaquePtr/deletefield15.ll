; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete pass correctly updates the offsets
; used in byte-flattened GEPs after field deletion within a cloned function.
; (The function cloning only occurs with the non-opaque pointer form because
; the function signature changes. Once opaque pointers are in use, this case
; may no longer be relevant because the signature will not change.)

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

define i32 @doSomething(%struct.test* "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !4 {
  %p = bitcast %struct.test* %p_test to i8*

  ; Get pointers to each field
  %p8_A = getelementptr i8, i8* %p, i64 0
  %p8_B = getelementptr i8, i8* %p, i64 4
  %p8_C = getelementptr i8, i8* %p, i64 12
  %p_test_A = bitcast i8* %p to i32*
  %p_test_B = bitcast i8* %p8_B to i64*
  %p_test_C = bitcast i8* %p8_C to i32*

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C
  %sum = add i32 %valA, %valC

  ; write B
  store i64 3, i64* %p_test_B

  ret i32 %sum
}
; CHECK-LABEL: define internal i32 @doSomething

; CHECK: %p8_A = getelementptr i8, {{.*}} %p, i64 0
; CHECK-NOT: %p8_B = getelementptr i8, {{.*}} %p, i64 4
; CHECK: %p8_C = getelementptr i8, {{.*}} %p, i64 4

; CHECK:  %p_test_A = bitcast
; CHECK-NOT: %p_test_B = bitcast
; CHECK:  %p_test_C = bitcast

; CHECK-NOT: store i64 3, {{.*}} %p_test_B


declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !9 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

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

!intel.dtrans.types = !{!10}
