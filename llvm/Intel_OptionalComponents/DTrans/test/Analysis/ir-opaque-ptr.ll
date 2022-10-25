; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtransanalysis>' -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s

; Test that the legacy DTransAnalysis pass does not run the analysis
; using getPointerElementType when the compiler is using opaque pointers.

%struct.test01 = type { i64, ptr }
define void @test1() {
  %p = call ptr @malloc(i64 16)
  %gep0 = getelementptr %struct.test01, ptr %p, i64 0, i32 0
  store i64 0, ptr %gep0
  %gep1 = getelementptr %struct.test01, ptr %p, i64 0, i32 1
  store ptr null, ptr %gep1
  ret void
}

declare ptr @malloc(i64)

; CHECK: dtrans: Pointers are opaque or opaque passes requested ... DTransAnalysis didn't run
