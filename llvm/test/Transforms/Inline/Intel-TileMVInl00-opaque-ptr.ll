; REQUIRES: asserts
; RUN: opt -force-opaque-pointers < %s -S -tilemvinlmarker -debug-only=tilemvinlmarker -tile-candidate-test -tile-candidate-min=4 -tile-candidate-arg-min=3 -tile-candidate-sub-arg-min=2 -tile-candidate-mark=false 2>&1 | FileCheck %s
; RUN: opt -force-opaque-pointers < %s -S -passes='tilemvinlmarker' -debug-only=tilemvinlmarker -tile-candidate-test -tile-candidate-min=4 -tile-candidate-arg-min=3 -tile-candidate-sub-arg-min=2 -tile-candidate-mark=false 2>&1 | FileCheck %s

; Check that the loop indices and increments are correctly identified for
;   the loops within the tile candidates.
; Check that the loop index and loop increment for each subscript arg is:
;   correctly identified.
; Check that the tile candidates are correctly identified.

; CHECK: TMVINL: fun01_ Loop Index   %i12 = add nuw nsw i64 %i8, 1
; CHECK: TMVINL: fun01_ Loop Inc   %i8 = phi i64 [ 3, %bb4 ], [ %i12, %bb7 ]
; CHECK: TMVINL: fun01_ Arg %0(2,1)
; CHECK: TMVINL: Tile Candidate fun01_
; CHECK: TMVINL: fun00_ Loop Index   %i12 = add nuw nsw i64 %i8, 1
; CHECK: TMVINL: fun00_ Loop Inc   %i8 = phi i64 [ 3, %bb4 ], [ %i12, %bb7 ]
; CHECK: TMVINL: fun00_ Arg %0(0,1)
; CHECK: TMVINL: fun00_ Arg %1(2,1)
; CHECK: TMVINL: Tile Candidate fun00_
; CHECK: TMVINL: fun1_ Loop Index   %i11 = add nuw nsw i64 %i7, 1
; CHECK: TMVINL: fun1_ Loop Inc   %i7 = phi i64 [ 2, %bb4 ], [ %i11, %bb6 ]
; CHECK: TMVINL: fun1_ Arg %0(2,1)
; CHECK: TMVINL: Tile Candidate fun1_
; CHECK: TMVINL: fun2_ Loop Index   %i11 = add nuw nsw i64 %i7, 1
; CHECK: TMVINL: fun2_ Loop Inc   %i7 = phi i64 [ 2, %bb4 ], [ %i11, %bb6 ]
; CHECK: TMVINL: fun2_ Arg %0(2,1)
; CHECK: TMVINL: Tile Candidate fun2_
; CHECK: TMVINL: fun0_ Loop Index   %i11 = add nuw nsw i64 %i7, 1
; CHECK: TMVINL: fun0_ Loop Inc   %i7 = phi i64 [ 2, %bb4 ], [ %i11, %bb6 ]
; CHECK: TMVINL: fun0_ Arg %0(0,1)
; CHECK: TMVINL: fun0_ Arg %1(2,1)
; CHECK: TMVINL: Tile Candidate fun0_

; Check that the tile choices are correctly identified.

; CHECK: TMVINL: Tile Choice fun0_
; CHECK: TMVINL: Tile Choice fun1_
; CHECK: TMVINL: Tile Choice fun2_
; CHECK: TMVINL: Tile Choice extra_
; CHECK: TMVINL: Tile Choice switch_
; CHECK: TMVINL: Tile Choice fun00_
; CHECK: TMVINL: Tile Choice fun01_

; Check that guards for the tile choices are not identified, because there
; are none.

; CHECK-NOT: TMVINL: GVMAP
; CHECK-NOT: TMVINL: CONDMAP

; Check that the global variables were validated, because there were none.

; CHECK: TMVINL: Validated GVM

; Check that no simplification of conditionals with globals was done.

; CHECK-NOT: TMVINL: Testing
; CHECK-NOT: Against
; CHECK-NOT: Provably

; Check that the tile choices were marked for inlining, since the internal
; option -tile-candidate-mark is false.

; CHECK-NOT: TMVINL: Marked leapfrog_ TO fun0_ FOR INLINING
; CHECK-NOT: TMVINL: Marked leapfrog_ TO fun1_ FOR INLINING
; CHECK-NOT: TMVINL: Marked leapfrog_ TO fun2_ FOR INLINING
; CHECK-NOT: TMVINL: Marked leapfrog_ TO extra_ FOR INLINING
; CHECK-NOT: TMVINL: Marked switch_ TO fun00_ FOR INLINING
; CHECK-NOT: TMVINL: Marked switch_ TO fun01_ FOR INLINING

; Check that the skeleton graph does not show the tile choices, since the
; internal option -tile-candidate-mark is not passed.

; CHECK: TMVINL: Root: leapfrog_
; CHECK-NOT: TMVINL: T fun0_
; CHECK-NOT: TMVINL: T fun1_
; CHECK-NOT: TMVINL: T fun2_
; CHECK-NOT: TMVINL: T extra_
; CHECK-NOT: TMVINL: T switch_
; CHECK: TMVINL: SubRoot: switch_
; CHECK-NOT: TMVINL: T fun00_
; CHECK-NOT: TMVINL: T fun01_

; Check the IR. The calls for tiling should not be marked with the
; "prefer-inline-tile-choice" attribute.

; CHECK: define{{.*}}@MAIN__({{.*}})
; CHECK: call{{.*}}@leapfrog_({{.*}}){{ *$}}
; CHECK: define{{.*}}@leapfrog_({{.*}})
; CHECK-NOT: call{{.*}}@fun0_({{.*}}) #1{{ *$}}
; CHECK-NOT: call{{.*}}@fun1_({{.*}}) #1{{ *$}}
; CHECK-NOT: call{{.*}}@fun2_({{.*}}) #1{{ *$}}
; CHECK-NOT: call{{.*}}@extra_({{.*}}) #1{{ *$}}
; CHECK-NOT: call{{.*}}@switch_({{.*}}) #1{{ *$}}
; CHECK: define{{.*}}@switch_({{.*}})
; CHECK-NOT: call{{.*}}@fun00_({{.*}}) #1{{ *$}}
; CHECK-NOT: call{{.*}}@fun01_({{.*}}) #1{{ *$}}
; CHECK-NOT: attributes #1 = { "prefer-inline-tile-choice" }

@anon.0 = internal unnamed_addr constant i32 2
@"main_$A" = internal global [100 x [100 x double]] zeroinitializer, align 16
@"main_$B" = internal global [100 x [100 x double]] zeroinitializer, align 16
@anon.1 = internal unnamed_addr constant i32 100

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

declare dso_local i32 @for_read_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr

define dso_local void @MAIN__() local_unnamed_addr {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca i32, align 8
  %i2 = alloca [4 x i8], align 1
  %i3 = alloca { ptr }, align 8
  %i4 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.0)
  %i5 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 0
  store i8 16, ptr %i5, align 1
  %i6 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 1
  store i8 3, ptr %i6, align 1
  %i7 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 2
  store i8 1, ptr %i7, align 1
  %i8 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 3
  store i8 0, ptr %i8, align 1
  store ptr %i1, ptr %i3, align 8
  %i12 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_read_seq_lis(ptr nonnull %i, i32 5, i64 1239157112576, ptr nonnull %i5, ptr nonnull %i3)
  call fastcc void @leapfrog_(ptr getelementptr inbounds ([100 x [100 x double]], ptr @"main_$A", i64 0, i64 0, i64 0), ptr getelementptr inbounds ([100 x [100 x double]], ptr @"main_$B", i64 0, i64 0, i64 0), ptr nonnull @anon.1, ptr nonnull %i1)
  ret void
}

define internal void @extra_(ptr noalias nocapture %arg, ptr noalias nocapture readonly %arg1) {
bb:
  %i = load i32, ptr %arg1, align 4
  %i2 = icmp slt i32 %i, 1
  br i1 %i2, label %bb11, label %bb3

bb3:                                              ; preds = %bb
  %i4 = add nuw i32 %i, 1
  %i5 = zext i32 %i4 to i64
  br label %bb6

bb6:                                              ; preds = %bb6, %bb3
  %i7 = phi i64 [ 1, %bb3 ], [ %i9, %bb6 ]
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i7)
  store double 5.000000e+00, ptr %i8, align 8
  %i9 = add nuw nsw i64 %i7, 1
  %i10 = icmp eq i64 %i9, %i5
  br i1 %i10, label %bb11, label %bb6

bb11:                                             ; preds = %bb6, %bb
  ret void
}

define internal fastcc void @leapfrog_(ptr noalias nocapture %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture readonly %arg3) unnamed_addr {
bb:
  tail call fastcc void @fun0_(ptr %arg, ptr %arg1, ptr %arg2)
  tail call fastcc void @fun1_(ptr %arg, ptr %arg1, ptr %arg2)
  tail call fastcc void @fun2_(ptr %arg, ptr %arg1, ptr %arg2)
  tail call fastcc void @extra_(ptr %arg, ptr %arg2)
  tail call fastcc void @switch_(ptr %arg, ptr %arg1, ptr %arg2, ptr %arg3)
  ret void
}

define internal fastcc void @switch_(ptr noalias nocapture %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture readonly %arg3) unnamed_addr {
bb:
  %i = load i32, ptr %arg3, align 4
  %i4 = and i32 %i, 1
  %i5 = icmp eq i32 %i4, 0
  br i1 %i5, label %bb7, label %bb6

bb6:                                              ; preds = %bb
  tail call fastcc void @fun00_(ptr %arg, ptr %arg1, ptr %arg2)
  br label %bb8

bb7:                                              ; preds = %bb
  tail call fastcc void @fun01_(ptr %arg, ptr %arg1, ptr %arg2)
  br label %bb8

bb8:                                              ; preds = %bb7, %bb6
  ret void
}

define internal fastcc void @fun01_(ptr noalias nocapture readonly %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 5
  br i1 %i3, label %bb21, label %bb4

bb4:                                              ; preds = %bb
  %i5 = add nsw i32 %i, -1
  %i6 = zext i32 %i5 to i64
  br label %bb7

bb7:                                              ; preds = %bb7, %bb4
  %i8 = phi i64 [ 3, %bb4 ], [ %i12, %bb7 ]
  %i9 = add nsw i64 %i8, -1
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i9)
  %i11 = load double, ptr %i10, align 8
  %i12 = add nuw nsw i64 %i8, 1
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i12)
  %i14 = load double, ptr %i13, align 8
  %i15 = fadd double %i11, %i14
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i8)
  %i17 = load double, ptr %i16, align 8
  %i18 = fadd double %i15, %i17
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg1, i64 %i8)
  store double %i18, ptr %i19, align 8
  %i20 = icmp eq i64 %i12, %i6
  br i1 %i20, label %bb21, label %bb7

bb21:                                             ; preds = %bb7, %bb
  ret void
}

define internal fastcc void @fun00_(ptr noalias nocapture %arg, ptr noalias nocapture readonly %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 5
  br i1 %i3, label %bb21, label %bb4

bb4:                                              ; preds = %bb
  %i5 = add nsw i32 %i, -1
  %i6 = zext i32 %i5 to i64
  br label %bb7

bb7:                                              ; preds = %bb7, %bb4
  %i8 = phi i64 [ 3, %bb4 ], [ %i12, %bb7 ]
  %i9 = add nsw i64 %i8, -1
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg1, i64 %i9)
  %i11 = load double, ptr %i10, align 8
  %i12 = add nuw nsw i64 %i8, 1
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg1, i64 %i12)
  %i14 = load double, ptr %i13, align 8
  %i15 = fadd double %i11, %i14
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg1, i64 %i8)
  %i17 = load double, ptr %i16, align 8
  %i18 = fadd double %i15, %i17
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i8)
  store double %i18, ptr %i19, align 8
  %i20 = icmp eq i64 %i12, %i6
  br i1 %i20, label %bb21, label %bb7

bb21:                                             ; preds = %bb7, %bb
  ret void
}

define internal fastcc void @fun1_(ptr noalias nocapture readonly %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 3
  br i1 %i3, label %bb20, label %bb4

bb4:                                              ; preds = %bb
  %i5 = zext i32 %i to i64
  br label %bb6

bb6:                                              ; preds = %bb6, %bb4
  %i7 = phi i64 [ 2, %bb4 ], [ %i11, %bb6 ]
  %i8 = add nsw i64 %i7, -1
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i8)
  %i10 = load double, ptr %i9, align 8
  %i11 = add nuw nsw i64 %i7, 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i11)
  %i13 = load double, ptr %i12, align 8
  %i14 = fadd double %i10, %i13
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i7)
  %i16 = load double, ptr %i15, align 8
  %i17 = fadd double %i14, %i16
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg1, i64 %i7)
  store double %i17, ptr %i18, align 8
  %i19 = icmp eq i64 %i11, %i5
  br i1 %i19, label %bb20, label %bb6

bb20:                                             ; preds = %bb6, %bb
  ret void
}

define internal fastcc void @fun2_(ptr noalias nocapture readonly %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 3
  br i1 %i3, label %bb20, label %bb4

bb4:                                              ; preds = %bb
  %i5 = zext i32 %i to i64
  br label %bb6

bb6:                                              ; preds = %bb6, %bb4
  %i7 = phi i64 [ 2, %bb4 ], [ %i11, %bb6 ]
  %i8 = add nsw i64 %i7, -1
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i8)
  %i10 = load double, ptr %i9, align 8
  %i11 = add nuw nsw i64 %i7, 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i11)
  %i13 = load double, ptr %i12, align 8
  %i14 = fadd double %i10, %i13
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i7)
  %i16 = load double, ptr %i15, align 8
  %i17 = fadd double %i14, %i16
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg1, i64 %i7)
  store double %i17, ptr %i18, align 8
  %i19 = icmp eq i64 %i11, %i5
  br i1 %i19, label %bb20, label %bb6

bb20:                                             ; preds = %bb6, %bb
  ret void
}

define internal fastcc void @fun0_(ptr noalias nocapture %arg, ptr noalias nocapture readonly %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 3
  br i1 %i3, label %bb20, label %bb4

bb4:                                              ; preds = %bb
  %i5 = zext i32 %i to i64
  br label %bb6

bb6:                                              ; preds = %bb6, %bb4
  %i7 = phi i64 [ 2, %bb4 ], [ %i11, %bb6 ]
  %i8 = add nsw i64 %i7, -1
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg1, i64 %i8)
  %i10 = load double, ptr %i9, align 8
  %i11 = add nuw nsw i64 %i7, 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg1, i64 %i11)
  %i13 = load double, ptr %i12, align 8
  %i14 = fadd double %i10, %i13
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg1, i64 %i7)
  %i16 = load double, ptr %i15, align 8
  %i17 = fadd double %i14, %i16
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr %arg, i64 %i7)
  store double %i17, ptr %i18, align 8
  %i19 = icmp eq i64 %i11, %i5
  br i1 %i19, label %bb20, label %bb6

bb20:                                             ; preds = %bb6, %bb
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
