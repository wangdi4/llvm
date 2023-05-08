; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@g = global [1 x ptr] [ptr @add]

; CHECK: @g = global [1 x ptr] [ptr @add]

define i32 @add(i32 %A, i32 %B) #0 {
entry:
  %add = add nsw i32 %B, %A
  ret i32 %add
}

define void @test(i32 %arg) #1 {
entry:
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind readnone willreturn mustprogress "referenced-indirectly" }
attributes #1 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @test}

; DEBUGIFY-NOT: WARNING
