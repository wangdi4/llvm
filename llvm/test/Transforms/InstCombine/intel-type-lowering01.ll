; RUN: opt -opaque-pointers=0 -passes="instcombine" < %s -S 2>&1 | FileCheck %s

; Test the load/store instructions are not converted to use integer type as it
; would break DTrans. Originally, there was a transformation doing that which we
; had to customize by introducing an option that disables it. Now, that code has
; been removed, but keep the test in case something similar will ever be
; re-committed.

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%class.MemoryManager = type { i32 (...)** }
%class.BaseRefVectorOf = type { %class.MemoryManager*, i8, i32, i32 }
%class.ValueStoreCache = type { %class.BaseRefVectorOf*, i64*, %class.MemoryManager* }


define void @test(%class.ValueStoreCache* %this) {
  %call = call i8* @malloc(i64 24)
  %vec = bitcast i8* %call to %class.BaseRefVectorOf*

  %fMemoryManager = getelementptr %class.ValueStoreCache, %class.ValueStoreCache* %this, i32 0, i32 2
  %mm = load %class.MemoryManager*, %class.MemoryManager** %fMemoryManager, align 4
  %mm_field = getelementptr %class.BaseRefVectorOf, %class.BaseRefVectorOf* %vec, i32 0, i32 0
  store %class.MemoryManager* %mm,  %class.MemoryManager** %mm_field

  %vec_field = getelementptr %class.ValueStoreCache, %class.ValueStoreCache* %this, i32 0, i32 0
  store %class.BaseRefVectorOf* %vec, %class.BaseRefVectorOf** %vec_field

  ret void
}

; CHECK: %mm = load %class.MemoryManager*, %class.MemoryManager** %fMemoryManager
; CHECK: store %class.MemoryManager* %mm,  %class.MemoryManager** %mm_field
; CHECK-NOT: %mm1 = load i32, i32* %1
; CHECK-NOT: store i32 %mm1, i32* %2

declare dso_local noalias i8* @malloc(i64)
