; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -debug-only=dtrans-deletefieldop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -debug-only=dtrans-deletefieldop -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies the candidate structure selection of the
; DTrans delete fields pass.

; This case checks for the identification of structures with an unused field.

%struct.test = type { i32, i64, i32 }
@result = global i32 zeroinitializer

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !4 {
  ; Allocate a struct and get pointers to its fields.
  %p = call i8* @malloc(i64 16)
  %p_test = bitcast i8* %p to %struct.test*
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  store i32 %argc, i32* %p_test_A
  store i32 %argc, i32* %p_test_C

  %valA = load i32, i32* %p_test_A
  %valC = load i32, i32* %p_test_C
  %sum = add i32 %valA, %valC
  store i32 %sum, i32* @result

  call void @free(i8* %p)
  ret i32 0
}

; CHECK: Delete field for opaque pointers: looking for candidate structures
; CHECK: Selected for deletion: %struct.test

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !7 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!3}
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!8}
