; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -passes='dtrans-deletefield' -S -o - %s | FileCheck %s

; This test checks that the DTrans delete field pass updates the align value on
; the load/store instructions when the structure changes because after fields
; are deleted from the structure, it may no longer be guaranteed that the
; specified alignment will hold. This shows the need to follow the uses of one
; GEP into another GEP to find the load/store of the element using 'align'.

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Field offsets: [0, 8, 16], 24
%struct.test = type { [3 x double], i64 }

define i32 @main(i32 %argc, i8** %argv) {
  %p = call align 16 i8* @malloc(i64 128)
  %p_test = bitcast i8* %p to %struct.test*
  %unknown = zext i32 %argc to i64
  %res = call double @doSomething(i64 %unknown, %struct.test* %p_test)
  call void @free(i8* %p)

  %val = fptoui double %res to i32
  ret i32 %val
}

; Use elements of the array embedded within the structure.
define double @doSomething(i64 %idx, %struct.test* align 16 %p_test) {
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 %idx, i32 0
  %p_test_B0 = getelementptr [3 x double], [3 x double]* %p_test_B, i64 0, i64 0
  %p_test_B1 = getelementptr [3 x double], [3 x double]* %p_test_B, i64 0, i64 1
  %p_test_B2 = getelementptr [3 x double], [3 x double]* %p_test_B, i64 0, i64 2

  %valB0 = load double, double* %p_test_B0, align 16
  %valB1 = load double, double* %p_test_B1, align 8
  %valB2 = load double, double* %p_test_B2, align 16
  %tmp = fadd double %valB0, %valB1
  %sum = fadd double %tmp, %valB2

  ret double %sum
}

; Delete field should reset the alignment of embedded array elements back to
; the default alignment for the type.

; CHECK-LABEL: define internal double @doSomething
; CHECK:   %valB0 = load double, double* %p_test_B0, align 8
; CHECK:   %valB1 = load double, double* %p_test_B1, align 8
; CHECK:   %valB2 = load double, double* %p_test_B2, align 8

declare dso_local noalias noundef align 16 i8* @malloc(i64 noundef)
declare dso_local void @free(i8* nocapture noundef) local_unnamed_addr #16
