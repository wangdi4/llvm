; This test verifies that Field-reordering transformation is applied
; correctly to sdiv and udiv instructions with constant and non-constant
; sizes related to %struct.T.
;
; [Note]
; %struct.test is a candidate for DTrans' Field-Reorder (DFR) transformation.
; However, %struct.T is not a DFR candidate. Instead, it is an inclusive struct type
; of %struct.test.
; As a result, we add support for %struct.T type.
;

;  RUN: opt  -whole-program-assume -dtrans-reorderfield-enable-legal-test=0 < %s -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt  -whole-program-assume -dtrans-reorderfield-enable-legal-test=0 < %s -S -passes=dtrans-reorderfields | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }
%struct.T = type { %struct.test, i32 }


define void @bar(%struct.T* %tp, %struct.T* %tp2) {
entry:
  %p1 = ptrtoint %struct.T* %tp2 to i64
  %p2 = ptrtoint %struct.T* %tp to i64
  %diff = sub i64  %p2, %p1
  %num = sdiv i64 %diff, 56
; CHECK: %num = sdiv i64 %diff, 48

  %p3 = ptrtoint %struct.T* %tp2 to i64
  %p4 = ptrtoint %struct.T* %tp to i64
  %diff1 = sub i64  %p3, %p4
  %ld = load i32, i32* @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 56
  %num1 = udiv i64 %diff1, %mul
; CHECK: %0 = sdiv exact i64 %mul, 56
; CHECK: %1 = mul i64 %0, 48
; CHECK: %num1 = udiv i64 %diff1, %1

  ret void
}

