; REQUIRES: asserts
; RUN: opt < %s -ip-cloning -debug-only=ipcloning -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -debug-only=ipcloning -S 2>&1 | FileCheck %s

; Test that the function foo is recognized as a recursive progression clone
; and four clones of it are created.

; CHECK: Enter IP cloning: (Before inlining)
; CHECK: Cloning Analysis for:  foo
; CHECK: Selected RecProgression cloning
; CHECK: Function: foo.1
; CHECK: ArgPos : 0
; CHECK: Argument : i32 %0
; CHECK: Replacement:  i32 0
; CHECK: Function: foo.2
; CHECK: ArgPos : 0
; CHECK: Argument : i32 %0
; CHECK: Replacement:  i32 1
; CHECK: Function: foo.3
; CHECK: ArgPos : 0
; CHECK: Argument : i32 %0
; CHECK: Replacement:  i32 2
; CHECK: Function: foo.4
; CHECK: ArgPos : 0
; CHECK: Argument : i32 %0
; CHECK: Replacement:  i32 3

; CHECK: define dso_local i32 @main()
; CHECK: tail call i32 @foo.1
; CHECK: define internal i32 @foo.1(i32 %0, i32 %1)
; CHECK: call i32 @foo.2
; CHECK: define internal i32 @foo.2(i32 %0, i32 %1)
; CHECK: call i32 @foo.3
; CHECK: define internal i32 @foo.3(i32 %0, i32 %1)
; CHECK: call i32 @foo.4
; CHECK: define internal i32 @foo.4(i32 %0, i32 %1)
; CHECK: call i32 @foo.1

define internal i32 @foo(i32, i32) {
  %3 = srem i32 %0, 4
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %8

; <label>:5:                                      ; preds = %2
  %6 = shl i32 %1, 1
  %7 = add nsw i32 %6, 3
  br label %12

; <label>:8:                                      ; preds = %2
  %9 = add nsw i32 %3, 1
  %10 = call i32 @foo(i32 %9, i32 %1)
  %11 = shl nsw i32 %10, 1
  br label %12

; <label>:12:                                     ; preds = %8, %5
  %13 = phi i32 [ %7, %5 ], [ %11, %8 ]
  ret i32 %13
}

define dso_local i32 @main() {
  br label %1

; <label>:1:                                      ; preds = %1, %0
  %2 = phi i32 [ 0, %0 ], [ %6, %1 ]
  %3 = phi i32 [ 0, %0 ], [ %5, %1 ]
  %4 = tail call i32 @foo(i32 0, i32 %2)
  %5 = add nsw i32 %4, %3
  %6 = add nuw nsw i32 %2, 1
  %7 = icmp eq i32 %6, 10
  br i1 %7, label %8, label %1

; <label>:8:                                      ; preds = %1
  ret i32 %5
}

