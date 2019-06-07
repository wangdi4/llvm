; RUN: opt -instcombine -disable-type-lowering-opts=true < %s -S 2>&1 | FileCheck --check-prefix=CHECK-DISABLED %s
; RUN: opt -instcombine -disable-type-lowering-opts=false < %s -S 2>&1 | FileCheck --check-prefix=CHECK-ENABLED %s

; Test the load/store instructions are not converted to use integer type
; when -disable-type-lowering-opts option for DTrans is enabled.

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

; CHECK-DISABLED: %mm = load %class.MemoryManager*, %class.MemoryManager** %fMemoryManager
; CHECK-DISABLED: store %class.MemoryManager* %mm,  %class.MemoryManager** %mm_field
; CHECK-DISABLED-NOT: %mm1 = load i32, i32* %1
; CHECK-DISABLED-NOT: store i32 %mm1, i32* %2

; CHECK-ENABLED: %mm1 = load i32, i32* %1
; CHECK-ENABLED: store i32 %mm1, i32* %2

declare dso_local noalias i8* @malloc(i64)
