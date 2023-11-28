; Currently, trivial chain optimization doesn't work if stores to multiple
; globals are involved in one chain.
; RUN: opt -S -passes=globalopt -enable-intel-advanced-opts < %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; CHECK: @a
@a = internal global i32 1
; CHECK: @b
@b = internal global i32 1

define void @chain_multiple_stores() #0 {
; CHECK-LABEL: define void @chain_multiple_stores()
; CHECK-NEXT:    [[TMP1:%.*]] = load i32, ptr @a
; CHECK-NEXT:    [[TMP2:%.*]] = load i32, ptr @b
; CHECK-NEXT:    [[TMP3:%.*]] = add i32 [[TMP1]], [[TMP2]]
; CHECK-NEXT:    store i32 [[TMP3]], ptr @a, align 4
; CHECK-NEXT:    store i32 [[TMP3]], ptr @b, align 4
; CHECK-NEXT:    ret void
;
  %1 = load i32, ptr @a
  %2 = load i32, ptr @b
  %3 = add i32 %1, %2
  store i32 %3, ptr @a
  store i32 %3, ptr @b
  ret void
}

attributes #0 = { "target-features"="+avx2" }
