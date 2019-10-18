; REQUIRES: asserts
; RUN: opt < %s -ip-cloning -debug-only=ipcloning -disable-output -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -debug-only=ipcloning -disable-output -S 2>&1 | FileCheck %s

; Various checks for conditions where a function can fail to be recognized
; as a recursive progression clone candidate.

@count = available_externally dso_local local_unnamed_addr global i32 0, align 8

; Check that foofail1 is not a recursive progression clone candidate
; because its argument has more than one use.
; CHECK-LABEL: Cloning Analysis for:  foofail1
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail1(i32* noalias nocapture readonly) {
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
  call void @foofail1(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  %14 = load i32, i32* %0, align 4
  store i32 %14, i32* @count, align 8
  ret void
}

; Check that foofail2 is not a recursive progression clone candidate
; because its argument's use is not a load.
; CHECK-LABEL: Cloning Analysis for:  foofail2
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail2(i32* noalias nocapture readonly) {
  %2 = alloca i32, align 4
  %3 = load i32, i32* @count, align 8
  %4 = add nsw i32 %3, 1
  store i32 5, i32* %0, align 8
  %5 = load i32, i32* @count, align 4
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
  call void @foofail2(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail3 is not a recursive progression clone candidate
; because its progression binary operator is not an Add.
; CHECK-LABEL: Cloning Analysis for:  foofail3
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail3(i32* noalias nocapture readonly) {
  %2 = alloca i32, align 4
  %3 = load i32, i32* @count, align 8
  %4 = mul nsw i32 %3, 1
  store i32 %4, i32* @count, align 8
  %5 = load i32, i32* %0, align 4
  %6 = icmp eq i32 %5, 8
  br i1 %6, label %7, label %9

; <label>:7:                                      ; preds = %1
  %8 = mul nsw i32 %3, 2
  store i32 %8, i32* @count, align 8
  br label %13

; <label>:9:                                      ; preds = %1
  %10 = icmp slt i32 %4, 500000
  br i1 %10, label %11, label %13

; <label>:11:                                     ; preds = %9
  %12 = mul nsw i32 %5, 1
  store i32 %12, i32* %2, align 4
  call void @foofail3(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail4 is not a recursive progression clone candidate
; because its increment is not a constant.
; CHECK-LABEL: Cloning Analysis for:  foofail4
; CHECK-NOT: Selected RecProgression cloning

; Function Attrs: nounwind
define internal void @foofail4(i32* noalias nocapture readonly) {
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
  %12 = add nsw i32 %5, %3
  store i32 %12, i32* %2, align 4
  call void @foofail4(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail5 is not a recursive progression clone candidate
; because its increment expression has more than one use.
; CHECK-LABEL: Cloning Analysis for:  foofail5
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail5(i32* noalias nocapture readonly) {
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
  br label %14

; <label>:9:                                      ; preds = %1
  %10 = icmp slt i32 %4, 500000
  br i1 %10, label %11, label %14

; <label>:11:                                     ; preds = %9
  %12 = add nsw i32 %5, 1
  store i32 %12, i32* %2, align 4
  %13 = alloca i32, align 4
  store i32 %12, i32* %13, align 4
  call void @foofail5(i32* nonnull %2)
  br label %14

; <label>:14:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail6 is not a recursive progression clone candidate
; because its increment expression has a use that is not a store.
; CHECK-LABEL: Cloning Analysis for:  foofail6
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail6(i32* noalias nocapture readonly) {
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
  br label %14

; <label>:9:                                      ; preds = %1
  %10 = icmp slt i32 %4, 500000
  br i1 %10, label %11, label %14

; <label>:11:                                     ; preds = %9
  %12 = add nsw i32 %5, 1
  %13 = add nsw i32 %12, 150
  store i32 %13, i32* %2, align 4
  call void @foofail6(i32* nonnull %2)
  br label %14

; <label>:14:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail7 is not a recursive progression clone candidate
; because its increment expression has a use which a store back to a
; different location.
; CHECK-LABEL: Cloning Analysis for:  foofail7
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail7(i32* noalias nocapture readonly) {
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
  br label %14

; <label>:9:                                      ; preds = %1
  %10 = icmp slt i32 %4, 500000
  br i1 %10, label %11, label %14

; <label>:11:                                     ; preds = %9
  %12 = add nsw i32 %5, 1
  %13 = alloca i32, align 4
  store i32 %12, i32* %13, align 4
  call void @foofail7(i32* nonnull %2)
  br label %14

; <label>:14:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail8 is not a recursive progression clone candidate
; because has more than one recursive callsite.
; CHECK-LABEL: Cloning Analysis for:  foofail8
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail8(i32* noalias nocapture readonly) {
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
  call void @foofail8(i32* nonnull %2)
  call void @foofail8(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail9 is not a recursive progression clone candidate
; because the location that holds the recursive progression is stored to
; in multiple places.
; CHECK-LABEL: Cloning Analysis for:  foofail9
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail9(i32* noalias nocapture readonly) {
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
  call void @foofail9(i32* nonnull %2)
  store i32 %12, i32* %2, align 4
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail10 is not a recursive progression clone candidate
; because it does not have a recursive callsite.
; CHECK-LABEL: Cloning Analysis for:  foofail10
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail10(i32* noalias nocapture readonly) {
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
  call void @foofail8(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail11 is not a recursive progression clone candidate
; because the recursive progression does not go to the correct argument
; in the recursive call.
; CHECK-LABEL: Cloning Analysis for:  foofail11
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail11(i32* noalias nocapture readonly, i32* noalias nocapture readonly) {
  %3 = alloca i32, align 4
  %4 = load i32, i32* @count, align 8
  %5 = add nsw i32 %4, 1
  store i32 %5, i32* @count, align 8
  %6 = load i32, i32* %0, align 4
  %7 = icmp eq i32 %6, 8
  br i1 %7, label %8, label %10

; <label>:8:                                      ; preds =2
  %9 = add nsw i32 %4, 2
  store i32 %9, i32* @count, align 8
  br label %14

; <label>:10:                                     ; preds =2
  %11 = icmp slt i32 %5, 500000
  br i1 %11, label %12, label %14

; <label>:12:                                     ; preds = %10
  %13 = add nsw i32 %6, 1
  store i32 %13, i32* %3, align 4
  call void @foofail11(i32* @count, i32* nonnull %3)
  br label %14

; <label>:14:                                     ; preds = %12, %10, %8
  ret void
}

; Check that foofail12 is not a recursive progression clone candidate
; because it has more than one basis callsite.
; CHECK-LABEL: Cloning Analysis for:  foofail12
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail12(i32* noalias nocapture readonly) {
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
  call void @foofail12(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail13 is not a recursive progression clone candidate
; because its basis callsite argument is from an unintialized location.
; CHECK-LABEL: Cloning Analysis for:  foofail13
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail13(i32* noalias nocapture readonly) {
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
  call void @foofail13(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail14 is not a recursive progression clone candidate
; because its basis callsite argument is stored multiple times.
; CHECK-LABEL: Cloning Analysis for:  foofail14
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail14(i32* noalias nocapture readonly) {
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
  call void @foofail14(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail15 is not a recursive progression clone candidate
; because its basis callsite argument is used in places other than the
; initial store and the basis callsite.
; CHECK-LABEL: Cloning Analysis for:  foofail15
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail15(i32* noalias nocapture readonly) {
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
  call void @foofail15(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail16 is not a recursive progression clone candidate
; because its basis callsite argument is not a constant.
; CHECK-LABEL: Cloning Analysis for:  foofail16
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail16(i32* noalias nocapture readonly) {
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
  call void @foofail16(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail17 is not a recursive progression clone candidate
; because it has no constant termination test.
; CHECK-LABEL: Cloning Analysis for:  foofail17
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail17(i32* noalias nocapture readonly) {
  %2 = alloca i32, align 4
  %3 = load i32, i32* @count, align 8
  %4 = add nsw i32 %3, 1
  store i32 %4, i32* @count, align 8
  %5 = load i32, i32* %0, align 4
  %6 = icmp eq i32 %5, %3
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
  call void @foofail17(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail18 is not a recursive progression clone candidate
; because the start value of its sequence precedes its stop value and
; the increment is positive.
; CHECK-LABEL: Cloning Analysis for:  foofail18
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail18(i32* noalias nocapture readonly) {
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
  call void @foofail18(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; Check that foofail19 is not a recursive progression clone candidate
; because the (stop - start) value in the recursive progression is not a
; multiple of the increment.
; CHECK-LABEL: Cloning Analysis for:  foofail19
; CHECK-NOT: Selected RecProgression cloning

define internal void @foofail19(i32* noalias nocapture readonly) {
  %2 = alloca i32, align 4
  %3 = load i32, i32* @count, align 8
  %4 = add nsw i32 %3, 1
  store i32 %4, i32* @count, align 8
  %5 = load i32, i32* %0, align 4
  %6 = icmp eq i32 %5, 7
  br i1 %6, label %7, label %9

; <label>:7:                                      ; preds = %1
  %8 = add nsw i32 %3, 2
  store i32 %8, i32* @count, align 8
  br label %13

; <label>:9:                                      ; preds = %1
  %10 = icmp slt i32 %4, 500000
  br i1 %10, label %11, label %13

; <label>:11:                                     ; preds = %9
  %12 = add nsw i32 %5, 2
  store i32 %12, i32* %2, align 4
  call void @foofail19(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 4
  store i32 1, i32* %1, align 4
  call void @foofail1(i32* nonnull %1)
  %2 = alloca i32, align 4
  store i32 1, i32* %2, align 4
  call void @foofail2(i32* nonnull %2)
  %3 = alloca i32, align 4
  store i32 2, i32* %3, align 4
  call void @foofail3(i32* nonnull %3)
  %4 = alloca i32, align 4
  store i32 2, i32* %4, align 4
  call void @foofail4(i32* nonnull %4)
  %5 = alloca i32, align 4
  store i32 3, i32* %5, align 4
  call void @foofail5(i32* nonnull %5)
  %6 = alloca i32, align 4
  store i32 3, i32* %6, align 4
  call void @foofail6(i32* nonnull %6)
  %7 = alloca i32, align 4
  store i32 4, i32* %7, align 4
  call void @foofail7(i32* nonnull %7)
  %8 = alloca i32, align 4
  store i32 4, i32* %8, align 4
  call void @foofail8(i32* nonnull %8)
  %9 = alloca i32, align 4
  store i32 5, i32* %9, align 4
  call void @foofail9(i32* nonnull %9)
  %10 = alloca i32, align 4
  store i32 5, i32* %10, align 4
  call void @foofail10(i32* nonnull %10)
  %11 = alloca i32, align 4
  store i32 1, i32* %11, align 4
  call void @foofail11(i32* nonnull %11, i32* nonnull %11)
  %12 = alloca i32, align 4
  store i32 1, i32* %12, align 4
  call void @foofail12(i32* nonnull %12)
  %13 = alloca i32, align 4
  store i32 2, i32* %12, align 4
  call void @foofail12(i32* nonnull %13)
  %14 = alloca i32, align 4
  call void @foofail13(i32* %14)
  %15 = alloca i32, align 4
  store i32 2, i32* %15, align 4
  store i32 3, i32* %15, align 4
  call void @foofail14(i32* %15)
  %16 = alloca i32, align 4
  %17 = load i32, i32* %16, align 8
  store i32 1, i32* %16, align 4
  call void @foofail15(i32* nonnull %16)
  call void @foofail16(i32* @count)
  %18 = alloca i32, align 4
  store i32 4, i32* %18, align 4
  call void @foofail7(i32* nonnull %18)
  %19 = alloca i32, align 4
  store i32 100, i32* %19, align 4
  call void @foofail18(i32* nonnull %19)
  %20 = alloca i32, align 4
  store i32 0, i32* %20, align 4
  call void @foofail9(i32* nonnull %1)
  ret void
}

