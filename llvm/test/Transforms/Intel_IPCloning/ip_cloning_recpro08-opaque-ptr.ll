; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -passes='module(ip-cloning),cgscc(inline)' -ip-gen-cloning-force-enable-dtrans -S 2>&1 | FileCheck %s

; Test that after @foo is cloned as a recursive progression clone 8 times and
; all of those clones are inlined into @MAIN__, that @MAIN__ has the
; "contains-rec-pro-clone" attribute. Also, check that @bar and @foo, which
; were not recursive progression clones nor had a recursive progression
; inlined into them, are marked with the "noinline" and no attributes,
; respectively. (They are NOT marked with the "contains-rec-pro-clone"
; attribute.)

; CHECK: define dso_local void @MAIN__() [[ATTR1:#[0-9]+]]
; CHECK: define internal void @bar() [[ATTR2:#[0-9]+]]
; CHECK: define internal void @foo(ptr noalias nocapture readonly %arg)
; CHECK: attributes [[ATTR1]] = { "contains-rec-pro-clone" }
; CHECK: attributes [[ATTR2]] = { noinline }

@count = available_externally dso_local local_unnamed_addr global i32 0, align 8

define dso_local void @MAIN__() {
bb:
  %i = alloca i32, align 4
  store i32 1, ptr %i, align 4
  call void @foo(ptr nonnull %i)
  call void @bar()
  ret void
}

define internal void @bar() #0 {
bb:
  %i = load i32, ptr @count, align 8
  %i1 = add nsw i32 %i, 1
  store i32 %i1, ptr @count, align 8
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
  br label %bb12

bb7:                                              ; preds = %bb
  %i8 = icmp slt i32 %i2, 500000
  br i1 %i8, label %bb9, label %bb12

bb9:                                              ; preds = %bb7
  %i10 = add nsw i32 %i3, 1
  store i32 %i10, ptr %i, align 4
  call void @foo(ptr nonnull %i)
  %i11 = add nsw i32 %i10, 1
  br label %bb12

bb12:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

attributes #0 = { noinline }
; end INTEL_FEATURE_SW_ADVANCED
