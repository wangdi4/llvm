; This test verifies that foo is inlined even though it has dynamic alloca
; instructions.

; RUN: opt < %s -S -inline -inline-report=7 -dtrans-inline-heuristics -inline-for-xmain 2>&1 | FileCheck %s
; Same as above except using metadata
; RUN: opt < %s -S -inlinereportsetup -inline -inline-report=0x86 -dtrans-inline-heuristics -inline-for-xmain -inlinereportemitter 2>&1 | FileCheck %s

; New PM command
; RUN: opt < %s -S -passes='cgscc(inline)' -inline-report=7 -dtrans-inline-heuristics -inline-for-xmain  2>&1 | FileCheck %s
; Same as above except using metadata
; RUN: opt < %s -S -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0x86 -dtrans-inline-heuristics -inline-for-xmain 2>&1 | FileCheck %s

; CHECK: COMPILE FUNC: foo
; CHECK: COMPILE FUNC: bar
; CHECK: {{.*}}INLINE: foo{{.*}}
; CHECK-NOT: foo {{.*}}Callee has dynamic alloca

; CHECK-NOT: call void @foo


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32 %i, double* %dp) {
entry:
  %a = add i32 %i, 1
  br label %bb
bb:
  %p1 = alloca i32
  store i32 20, i32* %p1
  %p2 = alloca double*
  store double* %dp, double** %p2
  call void @baz(i32* %p1, double** %p2)
  br label %return

return:
  ret void
}

define void @bar(i32 %i, double* %ptr) {
entry:
  call void @foo(i32 %i, double* %ptr)
  br label %return

return:
  ret void
}

declare void @baz(i32*, double**)
