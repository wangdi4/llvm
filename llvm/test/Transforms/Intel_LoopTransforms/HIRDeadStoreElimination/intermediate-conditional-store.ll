; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination 2>&1 < %s | FileCheck %s

; Verify that we are able to eliminate the very first store by also
; substituting the intermediate conditional store and load by the temp.

; CHECK: Dump Before

; CHECK: (%A)[0] = 10;
; CHECK: if (%t == 0)
; CHECK: {
; CHECK:   (%A)[0] = 2;
; CHECK: }
; CHECK: %ld = (%A)[0];
; CHECK: (%A)[0] = 5;
; CHECK: ret %ld;


; CHECK: Dump After

; CHECK: %temp = 10;
; CHECK: if (%t == 0)
; CHECK: {
; CHECK:   %temp = 2;
; CHECK: }
; CHECK: (%A)[0] = 5;
; CHECK: ret %temp;


define i32 @foo(ptr %A, i32 %t) {
entry:
  br label %bb

bb:
  store i32 10, ptr %A
  %cmp = icmp eq i32 %t, 0
  br i1 %cmp, label %then, label %merge

then:
  store i32 2, ptr %A
  br label %merge

merge:
  %ld = load i32, ptr %A
  store i32 5, ptr %A
  ret i32 %ld
}

