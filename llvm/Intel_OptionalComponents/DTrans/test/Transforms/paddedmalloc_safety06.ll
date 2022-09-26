; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Test that verifies that padded malloc wasn't applied because there was
; no search loop and no use of malloc.

; RUN: opt -disable-verify -debug-only=dtrans-paddedmalloc -whole-program-assume -passes='lto<O2>' %s -enable-npm-dtrans 2>&1 | FileCheck %s

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

; CHECK:      dtrans-paddedmalloc: Trace for DTrans Padded Malloc
; CHECK-NEXT:   dtrans-paddedmalloc: Padded malloc disabled
