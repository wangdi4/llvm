; REQUIRES: asserts
;
; Test DTrans integration in the old pass manager.
;
; This test is to verify that the DTrans opaque pointer pipeline
; passes are run.

; RUN: opt -disable-verify -debug-pass=Executions -whole-program-assume    \
; RUN:     -std-link-opts -disable-output  %s -enable-dtrans               \
; RUN:     -dtrans-opaque-pointer-pipeline 2>&1 \
; RUN:     | FileCheck %s

; CHECK: Executing Pass 'DTrans delete field with opaque pointer support'
; CHECK: Executing Pass 'DTrans array of structures to structure of arrays with opaque pointer support'
; CHECK: Executing Pass 'DTrans dynamic cloning with opaque pointer support'
; CHECK: Executing Pass 'DTrans annotator cleaner'

declare void @bar() local_unnamed_addr

; Function Attrs: noinline uwtable
define void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  br label %loop
loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
  %iv.next = add i32 %iv, 1
  tail call void @bar()
  %cmp = icmp eq i32 %iv, %n
  br i1 %cmp, label %exit, label %loop
exit:
  ret void
}

define i32 @main() {
  call void @foo(i32 1)
  ret i32 0
}

attributes #0 = { noinline uwtable }
