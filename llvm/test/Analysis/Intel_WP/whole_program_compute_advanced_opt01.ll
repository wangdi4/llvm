; REQUIRES: assert
; RUN: opt %s -disable-output -debug-only=whole-program-analysis -whole-program-advanced-opt-trace -wholeprogramanalysis -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; This test is to verify the module level computation for advanced optimizations
; is checking the target feature information of the functions in the module.
; This should detect SSE4.2, AVX and AVX2, but not AVX512.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 @add(i32 %a) #1 {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @sub(i32 %a) #1 {
entry:
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) #1 {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}

attributes #1 = { "target-features"="+avx,+avx2,+sse4.2" }

; CHECK: Target has SSE42
; CHECK: Target has AVX
; CHECK: Target has AVX2
; CHECK-NOT: Target has AVX512
