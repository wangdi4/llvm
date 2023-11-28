; Check that trivial chain optimization works when the value of a global is
; used only by functions without side effect.
; RUN: opt -S -passes=globalopt -enable-intel-advanced-opts < %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: @a
@a = internal global i32 1
; CHECK: @b
@b = internal global i32 1
; CHECK-NOT: @c
@c = internal global i32 1

define void @store_global(i32 %arg) #0 {
; CHECK-LABEL: define void @store_global(
; CHECK-NEXT:    store i32 %arg, ptr @b
; CHECK-NEXT:    ret void
;
  store i32 %arg, ptr @a
  store i32 %arg, ptr @b
  store i32 %arg, ptr @c
  ret void
}

; The return value of @pure_function has no user at all.
define void @call_unused_return_value() #0 {
; CHECK-LABEL: define void @call_unused_return_value(
; CHECK-NEXT:    ret void
;
  %1 = load i32, ptr @a
  %2 = add i32 %1, 1
  call i1 @pure_function(i32 %2)
  ret void
}

; The return value of @pure_function is used to compute the return value of
; the caller.
define i1 @call_return_value_used() #0 {
; CHECK-LABEL: define i1 @call_return_value_used(
; CHECK-NEXT:    [[TMP1:%.*]] = load i32, ptr @b
; CHECK-NEXT:    [[TMP2:%.*]] = add i32 [[TMP1]], 1
; CHECK-NEXT:    [[TMP3:%.*]] = call i1 @pure_function(i32 [[TMP2]])
; CHECK-NEXT:    [[TMP4:%.*]] = xor i1 [[TMP3]], true
; CHECK-NEXT:    ret i1 [[TMP4]]
;
  %1 = load i32, ptr @b
  %2 = add i32 %1, 1
  %3 = call i1 @pure_function(i32 %2)
  %4 = xor i1 %3, true
  ret i1 %4
}

; The value of @c is passed into @pure_function. Its return value is stored
; back but not actually used.
define void @call_return_value_writeback() #0 {
; CHECK-LABEL: define void @call_return_value_writeback(
; CHECK-NEXT:    ret void
;
  %1 = load i32, ptr @c
  %2 = add i32 %1, 1
  %3 = call i1 @pure_function(i32 %2)
  %4 = sext i1 %3 to i32
  store i32 %4, ptr @c
  ret void
}

define i1 @pure_function(i32 %arg) #1 {
; CHECK-LABEL: define i1 @pure_function(
; CHECK-NEXT:    [[RESULT:%.*]] = icmp eq i32 %arg, 1
; CHECK-NEXT:    ret i1 [[RESULT]]
;
  %result = icmp eq i32 %arg, 1
  ret i1 %result
}

attributes #0 = { "target-features"="+avx2" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) }
