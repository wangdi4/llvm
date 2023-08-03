; RUN: opt -passes="instcombine" < %s -S 2>&1 | FileCheck %s

; Test the load/store instructions are not converted to use integer type as it
; would break DTrans. Originally, there was a transformation doing that which we
; had to customize by introducing an option that disables it. Now, that code has
; been removed, but keep the test in case something similar will ever be
; re-committed.

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%class.MemoryManager = type { ptr }
%class.BaseRefVectorOf = type { ptr, i8, i32, i32 }
%class.ValueStoreCache = type { ptr, ptr, ptr }


define void @test(ptr %this) {
  %call = call ptr @malloc(i64 24)

  %fMemoryManager = getelementptr %class.ValueStoreCache, ptr %this, i32 0, i32 2
  %mm = load ptr, ptr %fMemoryManager, align 4
  store ptr %mm,  ptr %call

  store ptr %call, ptr %this

  ret void
}

; CHECK: %mm = load ptr, ptr %fMemoryManager
; CHECK: store ptr %mm,  ptr %call
; CHECK-NOT: %mm1 = load i32, ptr %1
; CHECK-NOT: store i32 %mm1, ptr %2

declare dso_local noalias ptr @malloc(i64)
