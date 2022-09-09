; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -passes='dtrans-deletefield' -S -o - %s | FileCheck %s

; This test checks that the DTrans delete field pass updates the align value on
; the load/store instructions when the structure changes because after fields
; are deleted from the structure, it may no longer be guaranteed that the
; specified alignment will hold. This tests the handling with nested structures.

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Field offsets: 0, 32, 40, 44, 56
%struct.test = type { %struct.inner, i32, i64, i32, i64}

; Field offsets: 0, 8, 16, 24
%struct.inner = type { i32, i64, i32, i64 }

; CHECK-DAG: %__DFDT_struct.test = type { %__DFT_struct.inner, i32, i64, i32, i64 }
; CHECK-DAG: %__DFT_struct.inner = type { i32, i32 }

@someValue = internal global i64 zeroinitializer, align 16
define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate an array of structures.
  %p = call align 16 i8* @malloc(i64 128)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %unknown = zext i32 %argc to i64
  %val = call i32 @doSomething(i64 %unknown, %struct.test* %p_test)

  ; Free the structures
  call void @free(i8* %p)

  ret i32 %val
}

define i32 @doSomething(i64 %idx, %struct.test* align 16 %p_test) {
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 0
  %pA = getelementptr %struct.inner, %struct.inner* %p_test_A, i64 0, i32 0
  %pB = getelementptr %struct.inner, %struct.inner* %p_test_A, i64 0, i32 2
  %valA = load i32, i32* %pA
  %valB = load i32, i32* %pB
  %sum = add i32 %valA, %valB

  ret i32 %sum
}
; Delete field should reset the alignment of structure elements back to the
; default alignment for the type.

; CHECK-LABEL: define internal i32 @doSomething
; CHECK:  %valA = load i32, i32* %pA, align 4
; CHECK:  %valB = load i32, i32* %pB, align 4

define i64 @user(i64 %idx, %struct.test* align 16 %p_test) {
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 2
  %p_test_D = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 3
  %p_test_E = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 4
  %valB = load i32, i32* %p_test_B, align 16
  %valC = load i64, i64* %p_test_C, align 8
  %valD = load i32, i32* %p_test_D, align 8
  %valE = load i64, i64* %p_test_E, align 8
  %tmp0 = zext i32 %valB to i64
  %tmp1 = zext i32 %valD to i64
  %tmp2 = add i64 %tmp0, %tmp1
  %tmp3 = add i64 %tmp2, %valC
  %tmp4 = add i64 %tmp3, %valE
  ret i64 %tmp4
}
; CHECK-LABEL: define internal i64 @user
; CHECK:  %valB = load i32, i32* %p_test_B, align 4
; CHECK:  %valC = load i64, i64* %p_test_C, align 8
; CHECK:  %valD = load i32, i32* %p_test_D, align 4
; CHECK:  %valE = load i64, i64* %p_test_E, align 8

declare dso_local noalias noundef align 16 i8* @malloc(i64 noundef)
declare dso_local void @free(i8* nocapture noundef) local_unnamed_addr #16
