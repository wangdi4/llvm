; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt -passes='module(ip-cloning)' -ip-gen-cloning-force-enable-dtrans -debug-only=ipcloning -S < %s 2>&1 | FileCheck %s

; Check that this LIT test does not abort because the first alloca in
; foo() does not feed a recursive callsite for foo()

; CHECK: Enter IP cloning: (Before inlining)
; CHECK: Skipping MAIN__
; CHECK: Cloning Analysis for:  foo
; CHECK: MRC Cloning: Testing: foo
; CHECK: MRC Cloning: Skipping: Not enough recursive callsites
; CHECK: Selected generic cloning (recursive)
; CHECK: Skipping non-candidate foo
; CHECK: Total clones:  0

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
  call void @foo(ptr null)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
