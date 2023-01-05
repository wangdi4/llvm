; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt -opaque-pointers < %s -passes='module(ip-cloning)' -ip-gen-cloning-force-enable-dtrans -debug-only=ipcloning -disable-output -S 2>&1 | FileCheck %s

; Various checks for conditions where a function can fail to be recognized
; as a recursive progression clone candidate.

@count = available_externally dso_local local_unnamed_addr global i32 0, align 8

; Check that foofail1 is not a recursive progression clone candidate
; because its argument has more than one use.
; CHECK-LABEL: Cloning Analysis for:  foofail1
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail1(ptr noalias nocapture readonly %arg) {
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
  call void @foofail1(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  %i12 = load i32, ptr %arg, align 4
  store i32 %i12, ptr @count, align 8
  ret void
}

; Check that foofail2 is not a recursive progression clone candidate
; because its argument's use is not a load.
; CHECK-LABEL: Cloning Analysis for:  foofail2
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail2(ptr noalias nocapture readonly %arg) {
bb:
  %i = alloca i32, align 4
  %i1 = load i32, ptr @count, align 8
  %i2 = add nsw i32 %i1, 1
  store i32 5, ptr %arg, align 8
  %i3 = load i32, ptr @count, align 4
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
  call void @foofail2(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail3 is not a recursive progression clone candidate
; because its progression binary operator is not an Add.
; CHECK-LABEL: Cloning Analysis for:  foofail3
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail3(ptr noalias nocapture readonly %arg) {
bb:
  %i = alloca i32, align 4
  %i1 = load i32, ptr @count, align 8
  %i2 = mul nsw i32 %i1, 1
  store i32 %i2, ptr @count, align 8
  %i3 = load i32, ptr %arg, align 4
  %i4 = icmp eq i32 %i3, 8
  br i1 %i4, label %bb5, label %bb7

bb5:                                              ; preds = %bb
  %i6 = mul nsw i32 %i1, 2
  store i32 %i6, ptr @count, align 8
  br label %bb11

bb7:                                              ; preds = %bb
  %i8 = icmp slt i32 %i2, 500000
  br i1 %i8, label %bb9, label %bb11

bb9:                                              ; preds = %bb7
  %i10 = mul nsw i32 %i3, 1
  store i32 %i10, ptr %i, align 4
  call void @foofail3(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail4 is not a recursive progression clone candidate
; because its increment is not a constant.
; CHECK-LABEL: Cloning Analysis for:  foofail4
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail4(ptr noalias nocapture readonly %arg) {
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
  %i10 = add nsw i32 %i3, %i1
  store i32 %i10, ptr %i, align 4
  call void @foofail4(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail5 is not a recursive progression clone candidate
; because its increment expression has more than one use.
; CHECK-LABEL: Cloning Analysis for:  foofail5
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail5(ptr noalias nocapture readonly %arg) {
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
  %i11 = alloca i32, align 4
  store i32 %i10, ptr %i11, align 4
  call void @foofail5(ptr nonnull %i)
  br label %bb12

bb12:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail6 is not a recursive progression clone candidate
; because its increment expression has a use that is not a store.
; CHECK-LABEL: Cloning Analysis for:  foofail6
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail6(ptr noalias nocapture readonly %arg) {
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
  %i11 = add nsw i32 %i10, 150
  store i32 %i11, ptr %i, align 4
  call void @foofail6(ptr nonnull %i)
  br label %bb12

bb12:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail7 is not a recursive progression clone candidate
; because its increment expression has a use which a store back to a
; different location.
; CHECK-LABEL: Cloning Analysis for:  foofail7
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail7(ptr noalias nocapture readonly %arg) {
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
  %i11 = alloca i32, align 4
  store i32 %i10, ptr %i11, align 4
  call void @foofail7(ptr nonnull %i)
  br label %bb12

bb12:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail8 is not a recursive progression clone candidate
; because has more than one recursive callsite.
; CHECK-LABEL: Cloning Analysis for:  foofail8
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail8(ptr noalias nocapture readonly %arg) {
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
  call void @foofail8(ptr nonnull %i)
  call void @foofail8(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail9 is not a recursive progression clone candidate
; because the location that holds the recursive progression is stored to
; in multiple places.
; CHECK-LABEL: Cloning Analysis for:  foofail9
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail9(ptr noalias nocapture readonly %arg) {
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
  call void @foofail9(ptr nonnull %i)
  store i32 %i10, ptr %i, align 4
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail10 is not a recursive progression clone candidate
; because it does not have a recursive callsite.
; CHECK-LABEL: Cloning Analysis for:  foofail10
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail10(ptr noalias nocapture readonly %arg) {
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
  call void @foofail8(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail11 is not a recursive progression clone candidate
; because the recursive progression does not go to the correct argument
; in the recursive call.
; CHECK-LABEL: Cloning Analysis for:  foofail11
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail11(ptr noalias nocapture readonly %arg, ptr noalias nocapture readonly %arg1) {
bb:
  %i = alloca i32, align 4
  %i2 = load i32, ptr @count, align 8
  %i3 = add nsw i32 %i2, 1
  store i32 %i3, ptr @count, align 8
  %i4 = load i32, ptr %arg, align 4
  %i5 = icmp eq i32 %i4, 8
  br i1 %i5, label %bb6, label %bb8

bb6:                                              ; preds = %bb
  %i7 = add nsw i32 %i2, 2
  store i32 %i7, ptr @count, align 8
  br label %bb12

bb8:                                              ; preds = %bb
  %i9 = icmp slt i32 %i3, 500000
  br i1 %i9, label %bb10, label %bb12

bb10:                                             ; preds = %bb8
  %i11 = add nsw i32 %i4, 1
  store i32 %i11, ptr %i, align 4
  call void @foofail11(ptr @count, ptr nonnull %i)
  br label %bb12

bb12:                                             ; preds = %bb10, %bb8, %bb6
  ret void
}

; Check that foofail12 is not a recursive progression clone candidate
; because it has more than one basis callsite.
; CHECK-LABEL: Cloning Analysis for:  foofail12
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail12(ptr noalias nocapture readonly %arg) {
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
  call void @foofail12(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail13 is not a recursive progression clone candidate
; because its basis callsite argument is from an unintialized location.
; CHECK-LABEL: Cloning Analysis for:  foofail13
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail13(ptr noalias nocapture readonly %arg) {
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
  call void @foofail13(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail14 is not a recursive progression clone candidate
; because its basis callsite argument is stored multiple times.
; CHECK-LABEL: Cloning Analysis for:  foofail14
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail14(ptr noalias nocapture readonly %arg) {
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
  call void @foofail14(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail15 is not a recursive progression clone candidate
; because its basis callsite argument is used in places other than the
; initial store and the basis callsite.
; CHECK-LABEL: Cloning Analysis for:  foofail15
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail15(ptr noalias nocapture readonly %arg) {
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
  call void @foofail15(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail16 is not a recursive progression clone candidate
; because its basis callsite argument is not a constant.
; CHECK-LABEL: Cloning Analysis for:  foofail16
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail16(ptr noalias nocapture readonly %arg) {
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
  call void @foofail16(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail17 is not a recursive progression clone candidate
; because it has no constant termination test.
; CHECK-LABEL: Cloning Analysis for:  foofail17
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail17(ptr noalias nocapture readonly %arg) {
bb:
  %i = alloca i32, align 4
  %i1 = load i32, ptr @count, align 8
  %i2 = add nsw i32 %i1, 1
  store i32 %i2, ptr @count, align 8
  %i3 = load i32, ptr %arg, align 4
  %i4 = icmp eq i32 %i3, %i1
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
  call void @foofail17(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail18 is not a recursive progression clone candidate
; because the start value of its sequence precedes its stop value and
; the increment is positive.
; CHECK-LABEL: Cloning Analysis for:  foofail18
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail18(ptr noalias nocapture readonly %arg) {
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
  call void @foofail18(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

; Check that foofail19 is not a recursive progression clone candidate
; because the (stop - start) value in the recursive progression is not a
; multiple of the increment.
; CHECK-LABEL: Cloning Analysis for:  foofail19
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail19(ptr noalias nocapture readonly %arg) {
bb:
  %i = alloca i32, align 4
  %i1 = load i32, ptr @count, align 8
  %i2 = add nsw i32 %i1, 1
  store i32 %i2, ptr @count, align 8
  %i3 = load i32, ptr %arg, align 4
  %i4 = icmp eq i32 %i3, 7
  br i1 %i4, label %bb5, label %bb7

bb5:                                              ; preds = %bb
  %i6 = add nsw i32 %i1, 2
  store i32 %i6, ptr @count, align 8
  br label %bb11

bb7:                                              ; preds = %bb
  %i8 = icmp slt i32 %i2, 500000
  br i1 %i8, label %bb9, label %bb11

bb9:                                              ; preds = %bb7
  %i10 = add nsw i32 %i3, 2
  store i32 %i10, ptr %i, align 4
  call void @foofail19(ptr nonnull %i)
  br label %bb11

bb11:                                             ; preds = %bb9, %bb7, %bb5
  ret void
}

define dso_local void @MAIN__() {
bb:
  %i = alloca i32, align 4
  store i32 1, ptr %i, align 4
  call void @foofail1(ptr nonnull %i)
  %i1 = alloca i32, align 4
  store i32 1, ptr %i1, align 4
  call void @foofail2(ptr nonnull %i1)
  %i2 = alloca i32, align 4
  store i32 2, ptr %i2, align 4
  call void @foofail3(ptr nonnull %i2)
  %i3 = alloca i32, align 4
  store i32 2, ptr %i3, align 4
  call void @foofail4(ptr nonnull %i3)
  %i4 = alloca i32, align 4
  store i32 3, ptr %i4, align 4
  call void @foofail5(ptr nonnull %i4)
  %i5 = alloca i32, align 4
  store i32 3, ptr %i5, align 4
  call void @foofail6(ptr nonnull %i5)
  %i6 = alloca i32, align 4
  store i32 4, ptr %i6, align 4
  call void @foofail7(ptr nonnull %i6)
  %i7 = alloca i32, align 4
  store i32 4, ptr %i7, align 4
  call void @foofail8(ptr nonnull %i7)
  %i8 = alloca i32, align 4
  store i32 5, ptr %i8, align 4
  call void @foofail9(ptr nonnull %i8)
  %i9 = alloca i32, align 4
  store i32 5, ptr %i9, align 4
  call void @foofail10(ptr nonnull %i9)
  %i10 = alloca i32, align 4
  store i32 1, ptr %i10, align 4
  call void @foofail11(ptr nonnull %i10, ptr nonnull %i10)
  %i11 = alloca i32, align 4
  store i32 1, ptr %i11, align 4
  call void @foofail12(ptr nonnull %i11)
  %i12 = alloca i32, align 4
  store i32 2, ptr %i11, align 4
  call void @foofail12(ptr nonnull %i12)
  %i13 = alloca i32, align 4
  call void @foofail13(ptr %i13)
  %i14 = alloca i32, align 4
  store i32 2, ptr %i14, align 4
  store i32 3, ptr %i14, align 4
  call void @foofail14(ptr %i14)
  %i15 = alloca i32, align 4
  %i16 = load i32, ptr %i15, align 8
  store i32 1, ptr %i15, align 4
  call void @foofail15(ptr nonnull %i15)
  call void @foofail16(ptr @count)
  %i17 = alloca i32, align 4
  store i32 4, ptr %i17, align 4
  call void @foofail7(ptr nonnull %i17)
  %i18 = alloca i32, align 4
  store i32 100, ptr %i18, align 4
  call void @foofail18(ptr nonnull %i18)
  %i19 = alloca i32, align 4
  store i32 0, ptr %i19, align 4
  call void @foofail9(ptr nonnull %i)
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
