; Check that a global variable is optimized out, if it's only used by unused
; loads and stores, or load-compute-store chains that have no side effect.
; RUN: opt -S -passes=globalopt -enable-intel-advanced-opts < %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; All uses of @g are safe so it can be optimized.
; CHECK-NOT: @g
@g = internal global i32 1
; @h is used in a terminator and can't be removed.
; CHECK: @h
@h = internal global i32 1

define void @chain() #0 {
; CHECK-LABEL: define void @chain()
; CHECK-NEXT:    ret void
;
  %1 = load i32, ptr @g
  %2 = add i32 %1, 1
  store i32 %2, ptr @g
  ret void
}

; These functions are present to ensure the optimization works with larger
; number of chains.
define void @chain_placeholder1() #0 {
  %1 = load i32, ptr @g
  %2 = add i32 %1, 1
  store i32 %2, ptr @g
  ret void
}

define void @chain_placeholder2() #0 {
  %1 = load i32, ptr @g
  %2 = add i32 %1, 1
  store i32 %2, ptr @g
  ret void
}

define void @chain_placeholder3() #0 {
  %1 = load i32, ptr @g
  %2 = add i32 %1, 1
  store i32 %2, ptr @g
  ret void
}

; Multiple loads of the same global converging in one instruction.
define void @chain_multiple_loads() #0 {
; CHECK-LABEL: define void @chain_multiple_loads()
; CHECK-NEXT:    call void @unknown_function()
; CHECK-NEXT:    ret void
;
  %load.1 = load i32, ptr @g
  %load.1.compute = mul i32 %load.1, 11
  call void @unknown_function()
  %load.2 = load i32, ptr @g
  %load.2.compute = mul i32 %load.2, 11
  %result = add i32 %load.1.compute, %load.2.compute
  store i32 %result, ptr @g
  ret void
}

define void @load() #0 {
; CHECK-LABEL: define void @load()
; CHECK-NEXT:    ret void
;
  %1 = load i32, ptr @g
  ret void
}

define void @store() #0 {
; CHECK-LABEL: define void @store()
; CHECK-NEXT:    ret void
;
  store i32 1, ptr @g
  ret void
}

define void @store_arg_to_h(i32 %arg) #0 {
; CHECK-LABEL: define void @store_arg_to_h(
; CHECK-NEXT:    store i32 %arg, ptr @h
;
  store i32 %arg, ptr @h
  ret void
}

define i32 @load_and_return_with_inc() #0 {
; CHECK-LABEL: define i32 @load_and_return_with_inc()
; CHECK-NEXT:    [[LOAD:%.*]] = load i32, ptr @h
; CHECK-NEXT:    [[INC:%.*]] = add i32 [[LOAD]], 1
; CHECK-NEXT:    ret i32 [[INC]]
;
  %1 = load i32, ptr @h
  %2 = add i32 %1, 1
  ret i32 %2
}

declare void @unknown_function()

attributes #0 = { "target-features"="+avx2" }
