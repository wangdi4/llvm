; RUN: opt < %s -ip-cloning -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -S 2>&1 | FileCheck %s

; Test that the function foo is recognized as a recursive progression clone
; and eight clones of it are created. Also test that the recursive progression
; is not cyclic.
; This is the same test as ip_cloning_recpro04.ll, but checks for IR without
; requiring asserts.

; CHECK: define dso_local void @MAIN__
; CHECK: define internal void @foo
; CHECK: call void @foo
; CHECK: define internal void @foo.1
; CHECK: call void @foo.2
; CHECK: define internal void @foo.2
; CHECK: call void @foo.3
; CHECK: define internal void @foo.3
; CHECK: call void @foo.4
; CHECK: define internal void @foo.4
; CHECK: call void @foo.5
; CHECK: define internal void @foo.5
; CHECK: call void @foo.6
; CHECK: define internal void @foo.6
; CHECK: call void @foo.7
; CHECK: define internal void @foo.7
; CHECK: call void @foo.8
; CHECK: define internal void @foo.8
; CHECK-NOT: call void @foo

@count = available_externally dso_local local_unnamed_addr global i32 0, align 8

define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 4
  store i32 1, i32* %1, align 4
  call void @foo(i32* nonnull %1)
  ret void
}

; Function Attrs: nounwind
define internal void @foo(i32* noalias nocapture readonly) {
  %2 = alloca i32, align 4
  %3 = load i32, i32* @count, align 8
  %4 = add nsw i32 %3, 1
  store i32 %4, i32* @count, align 8
  %5 = load i32, i32* %0, align 4
  %6 = icmp eq i32 %5, 8
  br i1 %6, label %7, label %9

; <label>:7:                                      ; preds = %1
  %8 = add nsw i32 %3, 2
  store i32 %8, i32* @count, align 8
  br label %13

; <label>:9:                                      ; preds = %1
  %10 = icmp slt i32 %4, 500000
  br i1 %10, label %11, label %13

; <label>:11:                                     ; preds = %9
  %12 = add nsw i32 %5, 1
  store i32 %12, i32* %2, align 4
  call void @foo(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

