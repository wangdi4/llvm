; RUN: opt -aa-pipeline=basic-aa -passes='arg-noalias-prop' -S %s | FileCheck %s

declare noalias i8* @malloc(i64)

; CHECK: define internal void @foo(i8* nocapture readonly %arg1, i8* noalias nocapture %arg2
define internal void @foo(i8* nocapture readonly %arg1, i8* nocapture %arg2) {
  ret void
}

define void @bar(i8* %arg) {
entry:
  br label %loop

loop:
  %p1 = phi i8* [ %p2, %loop ], [ %arg, %entry ]
  %p2 = phi i8* [ %p1, %loop ], [ null, %entry ]
  %p3 = call i8* @malloc(i64 1024)
  call void @foo(i8* %p1, i8* %p3)
  br label %loop
}
