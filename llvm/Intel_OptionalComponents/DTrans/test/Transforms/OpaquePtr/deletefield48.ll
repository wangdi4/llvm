; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

; This test checks that the DTrans delete field pass updates the align value on
; the load/store instructions when the structure changes because after fields
; are deleted from the structure, it may no longer be guaranteed that the
; specified alignment will hold. This test also demonstrates that the alignment
; update applies to both fields that are before or after the deleted field
; because the structure may be used as an array.

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Field offsets: 0, 8, 16, 20, 24
%struct.test = type { i16, i64, i16, i32, i64 }
; CHECK: %__DFT_struct.test = type { i16, i64, i16 }

@someValue = internal global i64 zeroinitializer, align 16
define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !5 {
  ; Allocate an array of structures.
  %p = call align 16 ptr @malloc(i64 96)

  ; Call a function to do something.
  %unknown = zext i32 %argc to i64
  %res = call i16 @doSomething(i64 %unknown, ptr %p)

  ; Free the structures
  call void @free(ptr %p)

  %val = zext i16 %res to i32
  ret i32 %val
}

define i16 @doSomething(i64 %idx, ptr "intel_dtrans_func_index"="1" align 16 %p_test) !intel.dtrans.func.type !7 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, ptr %p_test, i64 %idx, i32 0
  %p_test_B = getelementptr %struct.test, ptr %p_test, i64 %idx, i32 1
  %p_test_C = getelementptr %struct.test, ptr %p_test, i64 %idx, i32 2
  %p_test_D = getelementptr %struct.test, ptr %p_test, i64 %idx, i32 3
  %p_test_E = getelementptr %struct.test, ptr %p_test, i64 %idx, i32 4

  ; read and write A and C.
  store i16 1, ptr %p_test_A, align 16
  %valA = load i16, ptr %p_test_A, align 16
  store i16 2, ptr %p_test_C, align 8
  %valC = load i16, ptr %p_test_C, align 8
  %sum = add i16 %valA, %valC

  ; read B and store it somewhere else to prevent the field from being deleted.
  %valB = load i64, ptr %p_test_B, align 8
  store i64 %valB, ptr @someValue, align 16

  ret i16 %sum
}

; Delete field should reset the alignment of structure elements back to the
; default alignment for the type.

; CHECK-LABEL: define i16 @doSomething
; CHECK:  store i16 1, ptr %p_test_A, align 2
; CHECK:  %valA = load i16, ptr %p_test_A, align 2
; CHECK:  store i16 2, ptr %p_test_C, align 2
; CHECK:  %valC = load i16, ptr %p_test_C, align 2

; CHECK:   %valB = load i64, ptr %p_test_B, align 8
; CHECK:  store i64 %valB, ptr @someValue, align 16

declare !intel.dtrans.func.type !9 dso_local noalias noundef align 16 "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) #0
declare !intel.dtrans.func.type !10 dso_local void @free(ptr "intel_dtrans_func_index"="1" nocapture noundef) local_unnamed_addr #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i16 0, i32 0}  ; i16
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i8 0, i32 2}  ; i8**
!5 = distinct !{!4}
!6 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!7 = distinct !{!6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = distinct !{!8}
!11 = !{!"S", %struct.test zeroinitializer, i32 5, !1, !2, !1, !3, !2} ; { i16, i64, i16, i32, i64 }

!intel.dtrans.types = !{!11}
