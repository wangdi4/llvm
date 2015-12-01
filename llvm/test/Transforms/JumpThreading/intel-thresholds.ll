; RUN: opt -S -jump-threading < %s | FileCheck %s
;
target triple = "x86_64-unknown-linux-gnu"

; Check that the jump threading block duplication threshold is larger for
; blocks that end in switch statements than for blocks that end in simple
; conditional branches.
;
; CHECK-LABEL: f1
; CHECK-NOT: thread
; 
define void @f1(i1 %arg1, i1 %arg2, i32* %arg3) {
b1:
  br i1 %arg1, label %b2, label %b3

b2:
  br label %b3

b3:
  %cond = phi i1 [ %arg2, %b1 ], [ 0, %b2 ]
  %0 = load volatile i32, i32* %arg3
  %1 = load volatile i32, i32* %arg3
  %2 = load volatile i32, i32* %arg3
  %3 = load volatile i32, i32* %arg3
  %4 = load volatile i32, i32* %arg3
  %5 = load volatile i32, i32* %arg3
  %6 = load volatile i32, i32* %arg3
  %7 = load volatile i32, i32* %arg3
  br i1 %cond, label %b4, label %b5

b4:
  %8 = load volatile i32, i32* %arg3
  br label %b5

b5:
  ret void
}

; CHECK-LABEL: f2
; CHECK: thread
; 
define void @f2(i1 %arg1, i32 %arg2, i32* %arg3) {
b1:
  br i1 %arg1, label %b2, label %b3

b2:
  br label %b3

b3:
  %cond = phi i32 [ %arg2, %b1 ], [ 0, %b2 ]
  %0 = load volatile i32, i32* %arg3
  %1 = load volatile i32, i32* %arg3
  %2 = load volatile i32, i32* %arg3
  %3 = load volatile i32, i32* %arg3
  %4 = load volatile i32, i32* %arg3
  %5 = load volatile i32, i32* %arg3
  %6 = load volatile i32, i32* %arg3
  %7 = load volatile i32, i32* %arg3
  switch i32 %cond, label %b4 [
    i32 0, label %b5
    i32 42, label %b5
  ]

b4:
  %8 = load volatile i32, i32* %arg3
  br label %b5

b5:
  ret void
}

; Now check that the jump threading block duplication threshold is larger for
; blocks that end in indirect branches than for blocks that end in switch
; statements.
;
; CHECK-LABEL: f3
; CHECK-NOT: thread
; 
define void @f3(i1 %arg1, i32 %arg2, i32* %arg3) {
b1:
  br i1 %arg1, label %b2, label %b3

b2:
  br label %b3

b3:
  %cond = phi i32 [ %arg2, %b1 ], [ 0, %b2 ]
  %0 = load volatile i32, i32* %arg3
  %1 = load volatile i32, i32* %arg3
  %2 = load volatile i32, i32* %arg3
  %3 = load volatile i32, i32* %arg3
  %4 = load volatile i32, i32* %arg3
  %5 = load volatile i32, i32* %arg3
  %6 = load volatile i32, i32* %arg3
  %7 = load volatile i32, i32* %arg3
  %8 = load volatile i32, i32* %arg3
  %9 = load volatile i32, i32* %arg3
  %10 = load volatile i32, i32* %arg3
  %11 = load volatile i32, i32* %arg3
  %12 = load volatile i32, i32* %arg3
  switch i32 %cond, label %b4 [
    i32 0, label %b5
    i32 42, label %b5
  ]

b4:
  %13 = load volatile i32, i32* %arg3
  br label %b5

b5:
  ret void
}

; CHECK-LABEL: f4
; CHECK: thread
; 
define void @f4(i1 %arg1, i8* %arg2, i32* %arg3) {
b1:
  br i1 %arg1, label %b2, label %b3

b2:
  br label %b3

b3:
  %cond = phi i8* [ %arg2, %b1 ], [ blockaddress(@f4, %b4), %b2 ]
  %0 = load volatile i32, i32* %arg3
  %1 = load volatile i32, i32* %arg3
  %2 = load volatile i32, i32* %arg3
  %3 = load volatile i32, i32* %arg3
  %4 = load volatile i32, i32* %arg3
  %5 = load volatile i32, i32* %arg3
  %6 = load volatile i32, i32* %arg3
  %7 = load volatile i32, i32* %arg3
  %8 = load volatile i32, i32* %arg3
  %9 = load volatile i32, i32* %arg3
  %10 = load volatile i32, i32* %arg3
  %11 = load volatile i32, i32* %arg3
  %12 = load volatile i32, i32* %arg3
  indirectbr i8* %cond, [ label %b4, label %b5 ]

b4:
  %13 = load volatile i32, i32* %arg3
  br label %b5

b5:
  ret void
}
