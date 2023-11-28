; Verify that we bail out in the presence of allocas for variable-length
; arrays.  It's a difficult challenge to handle all instruction and memory
; dependences properly.  Check that the variant is not generated, the
; vector-variants attribute is removed, and the opt-report remark is
; generated.

; RUN: opt -passes=vec-clone -S < %s | FileCheck %s
; RUN: opt -passes='vec-clone,intel-ir-optreport-emitter' -S -intel-opt-report=high < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace --check-prefix=OPT-REPORT

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL-NOT: @_ZGVbN4l_foo
; CHECK-NOT: "vector-variants"="_ZGVbN4l_foo"

; OPT-REPORT:        Global optimization report for : foo
; OPT-REPORT-EMPTY:
; OPT-REPORT-NEXT:   FUNCTION REPORT BEGIN
; OPT-REPORT-NEXT:       remark #15580: 'omp declare' vector variants were skipped due to presence of unsupported variable-length array allocations.
; OPT-REPORT-NEXT:   FUNCTION REPORT END
; OPT-REPORT-NEXT:   =================================================================

define void @foo(i32 %k) #0 {
entry:
  %k.addr = alloca i32, align 4
  store i32 %k, ptr %k.addr, align 4
  %0 = load i32, ptr %k.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  %vla = alloca i32, i64 %1, align 16
  ret void
}

declare ptr @llvm.stacksave()

attributes #0 = { "vector-variants"="_ZGVbN4l_foo" }


; Verify that we handle fixed-length arrays normally.

; CHECK-LABEL: @_ZGVbN4l_bar
; CHECK-LABEL: entry
; CHECK: %arr = alloca i32, i64 1024, align 16
; CHECK-LABEL: simd.loop.header
; CHECK: %0 = call ptr @llvm.stacksave.p0()
; CHECK: "vector-variants"="_ZGVbN4l_bar"

define void @bar(i32 %k) #1 {
entry:
  %k.addr = alloca i32, align 4
  store i32 %k, ptr %k.addr, align 4
  %0 = call ptr @llvm.stacksave()
  %arr = alloca i32, i64 1024, align 16
  ret void
}

attributes #1 = { "vector-variants"="_ZGVbN4l_bar" }
