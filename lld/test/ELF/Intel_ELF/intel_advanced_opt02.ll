; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; REQUIRES: asserts
; RUN: opt %s -o %t.bc
; RUN: not ld.lld --lto-O2 -e main \
; RUN:    -plugin-opt=fintel-advanced-optim \
; RUN:    -plugin-opt=legacy-pass-manager \
; RUN:    -plugin-opt=fintel-libirc-allowed \
; RUN:    -mllvm -debug-only=whole-program-analysis \
; RUN:    -mllvm -whole-program-advanced-opt-trace \
; RUN:    -mllvm -print-after-all \
; RUN:    -mllvm -whole-program-assume-executable %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; RUN: opt %s -o %t.bc
; RUN: not ld.lld --lto-O2 -e main \
; RUN:    -plugin-opt=fintel-advanced-optim \
; RUN:    -plugin-opt=new-pass-manager  \
; RUN:    -plugin-opt=fintel-libirc-allowed \
; RUN:    -mllvm -debug-only=whole-program-analysis \
; RUN:    -mllvm -whole-program-advanced-opt-trace \
; RUN:    -mllvm -print-after-all \
; RUN:    -mllvm -whole-program-assume-executable %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; REQUIRES: x86

; This test case verifies that -plugin-opt=fintel-advanced-optim is communicated
; to the compiler backend.
;
; Test is using 'not' to ignore error about missing library symbol for
; '__intel_new_feature_proc_init' not be

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 @add(i32 %a) #1 {
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @sub(i32 %a) #1 {
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) #1 {
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}

attributes #1 = { "target-features"="+avx,+avx2,+sse4.2" }

; CHECK: Target has Intel SSE42
; CHECK: Target has Intel AVX
; CHECK: Target has Intel AVX2
; CHECK-NOT: Target has Intel AVX512
; end INTEL_FEATURE_SW_ADVANCED
