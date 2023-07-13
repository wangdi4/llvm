; REQUIRES: asserts
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -debug-only=dtrans-deletefieldop -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies the candidate structure selection of the
; DTrans delete fields pass.

; The case checks that we don't select a structure where all the fields are read
; and written.

%struct.test = type { i32, i64, i32 }
@result = global i32 zeroinitializer

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !4 {
  ; Allocate a struct and get pointers to its fields.
  %p = call ptr @malloc(i64 16)
  %p_test_A = getelementptr %struct.test, ptr %p, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, ptr %p, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, ptr %p, i64 0, i32 2

  store i32 %argc, ptr %p_test_A
  store i32 %argc, ptr %p_test_C

  %valA = load i32, ptr %p_test_A
  %valC = load i32, ptr %p_test_C
  %sum = add i32 %valA, %valC
  store i32 %sum, ptr @result

  %val = sext i32 %argc to i64
  store i64 %val, ptr %p_test_B
  %valB = load i64, ptr %p_test_B
  %trunc = trunc i64 %valB to i32
  store i32 %trunc, ptr @result

  call void @free(ptr %p)
  ret i32 0
}

; CHECK: Delete field for opaque pointers: looking for candidate structures
; CHECK-NOT: Selected for deletion: %struct.test
; CHECK: No candidates found.

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @malloc(i64)
declare !intel.dtrans.func.type !7 void @free(ptr "intel_dtrans_func_index"="1")

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!3}
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!8}
