; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt -opaque-pointers < %s -passes='module(ip-cloning)' -ip-gen-cloning-force-enable-dtrans -debug-only=ipcloning -S 2>&1 | FileCheck %s

; Test that the function foo is recognized as a recursive progression clone
; and eight clones of it are created. Also test that the recursive progression
; is not cyclic.

; CHECK: Cloning Analysis for:  foo
; CHECK: Selected RecProgression cloning
; CHECK: Function: foo.1
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 1
; CHECK: Function: foo.2
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 2
; CHECK: Function: foo.3
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 3
; CHECK: Function: foo.4
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 4
; CHECK: Function: foo.5
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 5
; CHECK: Function: foo.6
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 6
; CHECK: Function: foo.7
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 7
; CHECK: Function: foo.8
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 8

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

define dso_local void @MAIN__() {
bb:
  %i = alloca i32, align 4
  store i32 1, ptr %i, align 4
  call void @foo(ptr nonnull %i)
  ret void
}

define internal void @foo(ptr noalias nocapture readonly %arg) {
bb:
  %i = alloca i32, align 4
  %i1 = load i32, ptr @count, align 8
  %i2 = add nsw i32 %i1, 1
  store i32 %i2, ptr @count, align 8
  %i3 = load i32, ptr %arg, align 4
  %i4 = icmp eq i32 %i3, 8
  br i1 %i4, label %bb5, label %bb7

bb5:                                              ; preds = %bb
  %i6 = add nsw i32 %i1, 2
  store i32 %i6, ptr @count, align 8
  br label %bb11

bb7:                                              ; preds = %bb
  %i8 = icmp slt i32 %i2, 500000
  br i1 %i8, label %bb9, label %bb11

bb9:                                              ; preds = %bb7
  %i10 = add nsw i32 %i3, 1
  store i32 %i10, ptr %i, align 4
  call void @foo(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
