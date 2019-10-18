; REQUIRES: asserts
; RUN: opt < %s -ip-cloning -debug-only=ipcloning -disable-output -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -debug-only=ipcloning -disable-output -S 2>&1 | FileCheck %s

; Various checks for conditions where a function can fail to be recognized
; as a recursive progression clone candidate.

; Check that foofail1 is not a recursive progression clone candidate
; because its basis step is not included in a loop.
; CHECK-LABEL: Cloning Analysis for:  foofail1
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail1(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, 4
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail1(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  ret i32 %9
}

; Check that foofail2 is not a recursive progression clone candidate
; because its recursive processive candidate argument has more than one use.
; CHECK-LABEL: Cloning Analysis for:  foofail2
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail2(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, 4
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail2(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  %10 = add nsw i32 %9, %0
  ret i32 %10
}

; Check that foofail3 is not a recursive progression clone candidate
; because it does not contain a 'srem' instruction.
; CHECK-LABEL: Cloning Analysis for:  foofail3
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail3(i32, i32) unnamed_addr #0 {
  %3 = add nsw i32 %0, 4
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail3(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  %10 = add nsw i32 %9, %0
  ret i32 %10
}

; Check that foofail4 is not a recursive progression clone candidate
; because its 'srem' instruction does not have a constant denominator.
; CHECK-LABEL: Cloning Analysis for:  foofail4
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail4(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, %0
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail4(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  ret i32 %9
}

; Check that foofail5 is not a recursive progression clone candidate
; because it has a call to a function other than itself.
; CHECK-LABEL: Cloning Analysis for:  foofail5
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail5(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, %0
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail3(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  ret i32 %9
}

; Check that foofail6 is not a recursive progression clone candidate
; because it has more than 2 call sites.
; CHECK-LABEL: Cloning Analysis for:  foofail6
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail6(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, %0
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail6(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  %10 = tail call fastcc i32 @foofail6(i32 %7, i32 %1)
  %11 = shl nsw i32 %10, %9
  %12 = tail call fastcc i32 @foofail6(i32 %7, i32 %1)
  %13 = shl nsw i32 %12, %11
  ret i32 %13
}

; Check that foofail7 is not a recursive progression clone candidate
; because the result of the 'srem' is not fed into a binary operator.
; CHECK-LABEL: Cloning Analysis for:  foofail7
; CHECK-NOT: Selected RecProgression cloning

define internal i32 @foofail7(i32, i32, i32) {
  %4 = srem i32 %0, 4
  %5 = icmp sgt i32 %4, 2
  br i1 %5, label %6, label %9

; <label>:6:                                      ; preds = %3
  %7 = shl i32 %1, 1
  %8 = add nsw i32 %7, 3
  br label %12

; <label>:9:                                      ; preds = %3
  %10 = call i32 @foofail7(i32 %4, i32 %1, i32 %2)
  %11 = shl nsw i32 %10, 1
  br label %12

; <label>:12:                                     ; preds = %9, %6
  %13 = phi i32 [ %8, %6 ], [ %11, %9 ]
  ret i32 %13
}


; Check that foofail8 is not a recursive progression clone candidate
; because the binary operator controlling the progression is not an 'add'.
; CHECK-LABEL: Cloning Analysis for:  foofail8
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail8(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, 4
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = mul nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail8(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  ret i32 %9
}

; Check that foofail9 is not a recursive progression clone candidate
; because the increment of the recursive progression is not a constant.
; CHECK-LABEL: Cloning Analysis for:  foofail9
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail9(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, 4
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, %0
  %8 = tail call fastcc i32 @foofail9(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  ret i32 %9
}

; Check that foofail10 is not a recursive progression clone candidate
; because it has different increments in its 2 call sites.
; CHECK-LABEL: Cloning Analysis for:  foofail10
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail10(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, %0
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail10(i32 %7, i32 %1)
  %9 = add nsw i32 %8, 2
  %10 = tail call fastcc i32 @foofail10(i32 %9, i32 %1)
  ret i32 %10
}

; Check that foofail11 is not a recursive progression clone candidate
; because it has more than one basis call site.
; CHECK-LABEL: Cloning Analysis for:  foofail11
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail11(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, 4
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail11(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  ret i32 %9
}

; Check that foofail12 is not a recursive progression clone candidate
; because the initial value from the basis call site is not a constant.
; CHECK-LABEL: Cloning Analysis for:  foofail12
; CHECK-NOT: Selected RecProgression cloning

define internal fastcc i32 @foofail12(i32, i32) unnamed_addr #0 {
  %3 = srem i32 %0, 4
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %6

; <label>:5:                                      ; preds = %2
  ret i32 5

; <label>:6:                                      ; preds = %2
  %7 = add nsw i32 %3, 1
  %8 = tail call fastcc i32 @foofail12(i32 %7, i32 %1)
  %9 = shl nsw i32 %8, 1
  ret i32 %9
}

@myglobal = common dso_local global i32 0, align 4

define dso_local i32 @main() local_unnamed_addr #1 {
  %1 = tail call fastcc i32 @foofail1(i32 0, i32 0)
  br label %2

; <label>:2:                                      ; preds = %2, %0
  %3 = phi i32 [ 0, %0 ], [ %30, %2 ]
  %4 = phi i32 [ %1, %0 ], [ %6, %2 ]
  %5 = tail call fastcc i32 @foofail2(i32 0, i32 %3)
  %6 = add nuw nsw i32 %5, %4
  %7 = tail call fastcc i32 @foofail3(i32 0, i32 %3)
  %8 = add nuw nsw i32 %7, %6
  %9 = tail call fastcc i32 @foofail4(i32 0, i32 %3)
  %10 = add nuw nsw i32 %9, %8
  %11 = tail call fastcc i32 @foofail5(i32 0, i32 %3)
  %12 = add nuw nsw i32 %11, %10
  %13 = tail call fastcc i32 @foofail6(i32 0, i32 %3)
  %14 = add nuw nsw i32 %13, %12
  %15 = tail call fastcc i32 @foofail7(i32 0, i32 0, i32 %3)
  %16 = add nuw nsw i32 %15, %14
  %17 = tail call fastcc i32 @foofail8(i32 0, i32 %3)
  %18 = add nuw nsw i32 %17, %16
  %19 = tail call fastcc i32 @foofail9(i32 0, i32 %3)
  %20 = add nuw nsw i32 %19, %18
  %21 = tail call fastcc i32 @foofail10(i32 0, i32 %3)
  %22 = add nuw nsw i32 %21, %20
  %23 = tail call fastcc i32 @foofail11(i32 0, i32 %3)
  %24 = add nuw nsw i32 %23, %22
  %25 = tail call fastcc i32 @foofail11(i32 0, i32 %3)
  %26 = add nuw nsw i32 %25, %24
  %27 = load i32, i32* @myglobal, align 4
  %28 = tail call fastcc i32 @foofail12(i32 %27, i32 %3)
  %29 = add nuw nsw i32 %28, %16
  %30 = add nuw nsw i32 %3, 1
  %31 = icmp eq i32 %30, 10
  br i1 %31, label %32, label %2

; <label>:32:                                      ; preds = %2
  ret i32 %29
}
