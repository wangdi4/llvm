; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

; This test checks that the DTrans delete field pass updates the align value on
; the load/store instructions when the structure changes because after fields
; are deleted from the structure, it may no longer be guaranteed that the
; specified alignment will hold.

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Field offsets: 0, 4, 12, 16, 20, 28
%struct.test = type <{ i32, i64, i32, i32, i64, i32 }>
; CHECK: %__DFT_struct.test = type <{ i32, i32 }>

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !4 {
  ; Allocate an array of structures.
  %p = call align 16 i8* @malloc(i64 64)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %unknown = zext i32 %argc to i64
  %val = call i32 @doSomething(i64 %unknown, %struct.test* %p_test)

  ; Free the structures
  call void @free(i8* %p)

  ret i32 %val
}

define i32 @doSomething(i64 %idx, %struct.test* "intel_dtrans_func_index"="1" align 16 %p_test) !intel.dtrans.func.type !6 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 2
  %p_test_D = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 3
  %p_test_E = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 4

  ; read and write A and D.
  store i32 1, i32* %p_test_A, align 16
  %valA = load i32, i32* %p_test_A, align 16
  store i32 2, i32* %p_test_D, align 8
  %valD = load i32, i32* %p_test_D, align 8
  %sum = add i32 %valA, %valD

  ret i32 %sum
}

; For packed structures, delete field should set the default aligmnent.

; CHECK-LABEL: define internal i32 @doSomething
; CHECK:  store i32 1, i32* %p_test_A, align 1
; CHECK:  %valA = load i32, i32* %p_test_A, align 1
; CHECK:  store i32 2, i32* %p_test_D, align 1
; CHECK:  %valD = load i32, i32* %p_test_D, align 1

declare !intel.dtrans.func.type !8 dso_local noalias noundef align 16 "intel_dtrans_func_index"="1" i8* @malloc(i64 noundef) #0
declare !intel.dtrans.func.type !9 dso_local void @free(i8* "intel_dtrans_func_index"="1" nocapture noundef) #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!3}
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = distinct !{!7}
!10 = !{!"S", %struct.test zeroinitializer, i32 6, !1, !2, !1, !1, !2, !1} ; <{ i32, i64, i32, i32, i64, i32 }>

!intel.dtrans.types = !{!10}
