; RUN: opt -aa-pipeline=basic-aa -passes='arg-noalias-prop' -S %s | FileCheck %s

declare noalias ptr @malloc(i64)

; CHECK: define internal void @foo(ptr nocapture readonly %arg1, ptr noalias nocapture %arg2
define internal void @foo(ptr nocapture readonly %arg1, ptr nocapture %arg2) {
  ret void
}

define void @bar(ptr %arg) {
entry:
  br label %loop

loop:
  %p1 = phi ptr [ %p2, %loop ], [ %arg, %entry ]
  %p2 = phi ptr [ %p1, %loop ], [ null, %entry ]
  %p3 = call ptr @malloc(i64 1024)
  call void @foo(ptr %p1, ptr %p3)
  br label %loop
}
