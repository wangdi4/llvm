; REQUIRES: asserts
;
; Test DTrans integration in the old pass manager.
;
; This test is to verify that DTrans analysis is not rerun between
; the individual DTrans passes.

; RUN: opt -disable-verify -debug-pass=Executions -whole-program-assume    \
; RUN:     -enable-dtrans-soatoaos -enable-dtrans-deletefield              \
; RUN:     -enable-resolve-types -enable-dtrans-const-arrays-metadata      \
; RUN:     -std-link-opts -disable-output  %s -enable-dtrans               \
; RUN:     2>&1 \
; RUN:     | FileCheck %s

; Make sure the DTrans transforms are run, without rerunning analysis
; Also, DTrans analysis should not be run prior to the resolve types pass.

; CHECK-NOT: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans resolve types'
; CHECK: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans struct of arrays to array of structs'
; CHECK-NOT: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans weak align'
; CHECK-NOT: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans delete field'
; CHECK-NOT: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans reorder fields' on Module
; CHECK-NOT: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans array of structs to struct of arrays'
; CHECK-NOT: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans eliminate read only field access'
; CHECK-NOT: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans dynamic cloning'
; CHECK-NOT: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans annotator cleaner'
; CHECK: Executing Pass 'DTrans constant arrays metadata'

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
