; UNSUPPORTED: enable-opaque-pointers

; CMPLRLLVM-26277: Test to make sure the reorder fields pass does not crash when
; examining the pattern of a subtract instruction followed by a division
; instruction for a case that is not computing the distance between two
; structure pointers.

; RUN: opt  -whole-program-assume < %s -S -dtrans-reorderfields | FileCheck %s
; RUN: opt  -whole-program-assume < %s -S -passes=dtrans-reorderfields | FileCheck %s

; CHECK: %__DFR_struct.test = type { i64, i64, i64, i32, i32, i32, i16 }
; CHECK-NOT: %struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

; This function contains the pattern of a division operation that needs to be
; replaced when the computation is performed on pointers to structure types.
; However, this function is not used for a structure type, and should therefore
; be ignored by the transformation.
define i64 @test(i8* %in, i8** %other, i8 %s) {
  %size = zext i8 %s to i64
  %pti = ptrtoint i8* %in to i64
  %load = load i8*, i8** %other
  %pti2 = ptrtoint i8* %load to i64
  %dist = sub i64 %pti2, %pti
  %count = sdiv i64 %dist, %size
  ret i64 %count
}
; CHECK-LABEL: define i64 @test
; CHECK: %dist = sub i64 %pti2, %pti
; CHECK: %count = sdiv i64 %dist, %size

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %0 = bitcast i8* %call to %struct.test*
  %i = getelementptr %struct.test, %struct.test* %0, i64 0, i32 0
  store i32 10, i32* %i, align 8
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
