; This test verifies that Field-reordering transformation applied
; correctly to memmov/memcpy/memset with constant and non-constant
; sizes related to %struct.test.

;  RUN: opt  < %s -whole-program-assume -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt  < %s -whole-program-assume -S -passes=dtrans-reorderfields | FileCheck %s



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define void @foo(%struct.test* %tp1, %struct.test* %tp2, %struct.test* %tp3) {
entry:
  %p1 = bitcast %struct.test* %tp1 to i8*
  %p2 = bitcast %struct.test* %tp2 to i8*
  %call = bitcast %struct.test* %tp2 to i8*
  %call1 = bitcast %struct.test* %tp2 to i8*

  %ld2 = load i32, i32* @G, align 4
  %conv2 = sext i32 %ld2 to i64
  %mul = mul nsw i64 %conv2, 48
  call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %mul, i1 false)
; CHECK:  %0 = sdiv exact i64 %mul, 48
; CHECK:  %1 = mul i64 %0, 40
; CHECK:  void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %1, i1 false)

  call void @llvm.memset.p0i8.i64(i8* %call1, i8 0, i64 48, i1 false)
; CHECK:  void @llvm.memset.p0i8.i64(i8* %call1, i8 0, i64 40, i1 false)

  call void @llvm.memmove.p0i8.p0i8.i64(i8* %p2, i8* %p1, i64 48, i1 false)
; CHECK:  void @llvm.memmove.p0i8.p0i8.i64(i8* %p2, i8* %p1, i64 40, i1 false)

  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 48, i1 false)
; CHECK:  void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 40, i1 false)

  %p3 = bitcast %struct.test* %tp3 to i8*
  %ld1 = load i32, i32* @G, align 4
  %conv1 = sext i32 %ld1 to i64
  %mul1 = mul nsw i64 %conv1, 48
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p3, i8* %p2, i64 %mul1, i1 false)
; CHECK:  %2 = sdiv exact i64 %mul1, 48
; CHECK:  %3 = mul i64 %2, 40
; CHECK:  void @llvm.memcpy.p0i8.p0i8.i64(i8* %p3, i8* %p2, i64 %3, i1 false)

  %mul2 = mul nsw i64 %conv1, 48
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %p2, i8* %p3, i64 %mul2, i1 false)
; CHECK:  %4 = sdiv exact i64 %mul2, 48
; CHECK:  %5 = mul i64 %4, 40
; CHECK:  void @llvm.memmove.p0i8.p0i8.i64(i8* %p2, i8* %p3, i64 %5, i1 false)

  ret void
}

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memmove.p0i8.p0i8.i64(i8* , i8*, i64, i1)
