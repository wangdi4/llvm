; RUN: opt -S -argpromotion < %s | FileCheck %s
; RUN: opt -S -passes=argpromotion < %s | FileCheck %s

; This test case checks that the COMDAT was preserved after argument promotion.

; CHECK: $bar = comdat any
; CHECK: define internal i32 @bar(i32 {{.+}}) comdat

target triple = "x86_64-pc-windows-msvc"

$"bar" = comdat any

define internal i32 @"bar"(i32* %0) comdat {
  %tmp1 = load i32, i32* %0
  ret i32 %tmp1
}

define i32 @foo() {
  %tmp1 = alloca i32
  %tmp2 = call i32 @"bar"(i32* %tmp1)
  ret i32 %tmp2
}
