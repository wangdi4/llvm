; RUN: opt -passes=vec-clone -S < %s | FileCheck %s

; Note: opt report checking removed because it's not supported with optnone attribute

; CHECK-LABEL-NOT: @_ZGVbN4Us1u__Z3fooRii
; CHECK-NOT: "vector-variants"="_ZGVbN4Us1u__Z3fooRii"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @_Z3fooRii(ptr noundef nonnull align 4 dereferenceable(4) %a, i32 noundef %m) #0 {
entry:
  %a.addr = alloca ptr, align 8
  %m.addr = alloca i32, align 4
  store ptr %a, ptr %a.addr, align 8
  call void @llvm.intel.directive.elementsize(ptr %a, i64 4)
  store i32 %m, ptr %m.addr, align 4
  %0 = load ptr, ptr %a.addr, align 8
  %1 = load i32, ptr %0, align 4
  %2 = load i32, ptr %m.addr, align 4
  %add = add nsw i32 %1, %2
  %3 = load ptr, ptr %a.addr, align 8
  store i32 %add, ptr %3, align 4
  %4 = load ptr, ptr %a.addr, align 8
  %5 = load i32, ptr %4, align 4
  ret i32 %5
}

declare void @llvm.intel.directive.elementsize(ptr, i64 immarg) #2

attributes #0 = { "vector-variants"="_ZGVbN4Us1u__Z3fooRii" optnone noinline }
