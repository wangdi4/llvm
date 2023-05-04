; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt -opaque-pointers < %s -S -passes='tilemvinlmarker,cgscc(inline)' -tile-candidate-mark -debug-only=tilemvinlmarker -tile-candidate-test -tile-candidate-min=4 -tile-candidate-arg-min=3 -tile-candidate-sub-arg-min=2 2>&1 | FileCheck %s

; Check that the loop indices and increments are correctly identified for
;   the loops within the tile candidates.
; Check that the loop index and loop increment for each subscript arg is:
;   correctly identified.
; Check that the tile candidates are correctly identified.

; This test is an extention of TileMVInl03.ll which adds the inlining after
; the tiling and cloning for inlining.

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

; Check that the guards for the tile choices are correctly identified.

; CHECK: TMVINL: GVMAP mymod_mp_mytester_
; CHECK: TMVINL:   %t6 = icmp sgt i32 %t5, 5
; CHECK: TMVINL: GVMAP mymod_mp_myglobal_
; CHECK: TMVINL:   %t61 = icmp eq i32 %t51, 1
; CHECK: TMVINL: GVMAP mymod_mp_mybool_
; CHECK: TMVINL:   %t53 = load i1, ptr @mymod_mp_mybool_, align 8
; CHECK: TMVINL: GVMAP mymod_mp_mynnodes_
; CHECK: TMVINL:   %t8 = icmp eq i32 %t7, -2
; CHECK: TMVINL: CONDMAP T   %t6 = icmp sgt i32 %t5, 5
; CHECK: TMVINL: LI   %t5 = load i32, ptr @mymod_mp_mytester_, align 8
; CHECK: TMVINL: CONDMAP T   %t61 = icmp eq i32 %t51, 1
; CHECK: TMVINL: LI   %t51 = load i32, ptr @mymod_mp_myglobal_, align 8
; CHECK: TMVINL: CONDMAP T   %t53 = load i1, ptr @mymod_mp_mybool_, align 8
; CHECK: TMVINL: CONDMAP F   %t8 = icmp eq i32 %t7, -2
; CHECK: TMVINL: LI   %t7 = load i32, ptr @mymod_mp_mynnodes_, align 8

; Check that the global variables were validated (determined not to be
; assigned in the root routine @leapfrog_).

; CHECK: TMVINL: Validated GVM

; Check that the tile choices are correctly identified.

; CHECK: TMVINL: Marked leapfrog_ TO fun0_ FOR INLINING
; CHECK: TMVINL: Marked leapfrog_ TO fun1_ FOR INLINING
; CHECK: TMVINL: Marked leapfrog_ TO fun2_ FOR INLINING
; CHECK: TMVINL: Marked leapfrog_ TO extra_ FOR INLINING
; CHECK: TMVINL: Marked leapfrog_ TO switch_ FOR INLINING
; CHECK: TMVINL: Marked switch_ TO fun00_ FOR INLINING
; CHECK: TMVINL: Marked switch_ TO fun01_ FOR INLINING

; Check that the conditionals with globals are simplified.

; CHECK: TMVINL: Testing       %t6 = icmp sgt i32 %t5, 5
; CHECK: TMVINL: Against (T)   %t6 = icmp sgt i32 %t5, 5
; CHECK: TMVINST: Provably TRUE   GV = mymod_mp_mytester_
; CHECK: TMVINL: Testing       %t61 = icmp eq i32 %t51, 1
; CHECK: TMVINL: Against (T)   %t61 = icmp eq i32 %t51, 1
; CHECK: TMVINST: Provably TRUE   GV = mymod_mp_myglobal_
; CHECK: TMVINL: Testing       %t62 = icmp sge i32 %t52, 1
; CHECK: TMVINL: Against (T)   %t61 = icmp eq i32 %t51, 1
; CHECK: TMVINST: Provably TRUE   GV = mymod_mp_myglobal_
; CHECK: TMVINL: Testing       %t53 = load i1, ptr @mymod_mp_mybool_, align 8
; CHECK: TMVINL: Against (T)   %t53 = load i1, ptr @mymod_mp_mybool_, align 8
; CHECK: TMVINST: Provably TRUE   GV = mymod_mp_mybool_
; CHECK: TMVINL: Testing       %t8 = icmp eq i32 %t7, -2
; CHECK: TMVINL: Against (F)   %t8 = icmp eq i32 %t7, -2
; CHECK: TMVINST: Provably FALSE  GV = mymod_mp_mynnodes_

; Check that the skeleton graph shows the right tile choices.

; CHECK: TMVINL: Root: leapfrog_
; CHECK: TMVINL:  T fun0_
; CHECK: TMVINL:  T fun1_
; CHECK: TMVINL:  T fun2_
; CHECK: TMVINL:  T extra_
; CHECK: TMVINL:  T switch_
; CHECK: TMVINL: SubRoot: switch_
; CHECK: TMVINL:  T fun00_
; CHECK: TMVINL:  T fun01_
; CHECK: TMVINL: NewRoot: leapfrog_.1
; CHECK: TMVINL:  fun0_
; CHECK: TMVINL:  fun1_
; CHECK: TMVINL:  fun2_
; CHECK: TMVINL:  extra_
; CHECK: TMVINL:  switch_.2
; CHECK: TMVINL: NewSubRoot: switch_.2
; CHECK: TMVINL:  fun00_
; CHECK: TMVINL:  fun01_
; CHECK: TMVINL: Multiversioning complete

; Check the IR (There isn't much to check beyond the condition, as everything
; gets inlined into @MAIN__.)

; CHECK: define{{.*}}@MAIN__({{.*}})
; CHECK: br label %.clone.tile.cond
; CHECK: [[L2:[0-9]+]]:
; CHECK: br label %[[L1:[0-9]+]]
; CHECK: .clone.tile.cond:
; CHECK: %[[V1:[0-9]+]] = load i32, ptr @mymod_mp_mytester_, align 8
; CHECK: %clone.tile.cmp = icmp sgt i32 %[[V1]], 5
; CHECK: %[[V2:[0-9]+]] = load i32, ptr @mymod_mp_myglobal_, align 8
; CHECK: %clone.tile.cmp1 = icmp eq i32 %[[V2]], 1
; CHECK: %.clone.tile.and = and i1 %clone.tile.cmp, %clone.tile.cmp1
; CHECK: %[[V3:[0-9]+]] = load i1, ptr @mymod_mp_mybool_, align 8
; CHECK: %clone.tile.cmp2 = icmp ne i1 %[[V3]], false
; CHECK: %.clone.tile.and3 = and i1 %clone.tile.cmp, %clone.tile.cmp2
; CHECK: %[[V4:[0-9]+]] = load i32, ptr @mymod_mp_mynnodes_, align 8
; CHECK: %clone.tile.cmp4 = icmp ne i32 %[[V4]], -2
; CHECK: %.clone.tile.and5 = and i1 %clone.tile.cmp, %clone.tile.cmp4
; CHECK: %.clone.tile.cmp = icmp ne i1 %clone.tile.cmp, false
; CHECK: br i1 %.clone.tile.cmp, label %[[L2]], label %.clone.tile.call
; CHECK: .clone.tile.call:

@"main_$A" = internal global [100 x double] zeroinitializer, align 16
@"main_$B" = internal global [100 x double] zeroinitializer, align 16
@anon.0 = internal unnamed_addr constant [0 x i8] zeroinitializer
@anon.1 = internal unnamed_addr constant i32 2
@anon.2 = internal unnamed_addr constant i32 100
@mymod_mp_myglobal_ = internal global i32 0, align 8
@mymod_mp_mynnodes_ = internal global i32 0, align 8
@mymod_mp_mytester_ = internal global i32 0, align 8
@mymod_mp_mybool_ = internal global i1 false, align 8

declare dso_local i32 @for_stop_core_quiet(ptr, i32, i32, i64, i32, i32, ...) local_unnamed_addr

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

declare dso_local i32 @for_read_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr

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
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i7)
  store double 5.000000e+00, ptr %i8, align 8
  %i9 = add nuw nsw i64 %i7, 1
  %i10 = icmp eq i64 %i9, %i5
  br i1 %i10, label %bb11, label %bb6

bb11:                                             ; preds = %bb6, %bb
  ret void
}

define internal fastcc void @init_() unnamed_addr {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca [4 x i8], align 1
  %i2 = alloca { ptr }, align 8
  %i3 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 0
  store i8 9, ptr %i3, align 1
  %i4 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 1
  store i8 3, ptr %i4, align 1
  %i5 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 2
  store i8 1, ptr %i5, align 1
  %i6 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 3
  store i8 0, ptr %i6, align 1
  %i7 = getelementptr inbounds { ptr }, ptr %i2, i64 0, i32 0
  store ptr @mymod_mp_mynnodes_, ptr %i7, align 8
  %i10 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_read_seq_lis(ptr nonnull %i, i32 5, i64 1239157112576, ptr nonnull %i3, ptr nonnull %i2)
  ret void
}

define dso_local void @MAIN__() local_unnamed_addr {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca i32, align 8
  %i2 = alloca [4 x i8], align 1
  %i3 = alloca { ptr }, align 8
  %i4 = alloca [4 x i8], align 1
  %i5 = alloca { ptr }, align 8
  %i6 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.1)
  %i7 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 0
  store i8 16, ptr %i7, align 1
  %i8 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 1
  store i8 3, ptr %i8, align 1
  %i9 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 2
  store i8 1, ptr %i9, align 1
  %i10 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 3
  store i8 0, ptr %i10, align 1
  store ptr %i1, ptr %i3, align 8
  %i14 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_read_seq_lis(ptr nonnull %i, i32 5, i64 1239157112576, ptr nonnull %i7, ptr nonnull %i3)
  %i15 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 0
  store i8 9, ptr %i15, align 1
  %i16 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 1
  store i8 3, ptr %i16, align 1
  %i17 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 2
  store i8 1, ptr %i17, align 1
  %i18 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 3
  store i8 0, ptr %i18, align 1
  %i19 = getelementptr inbounds { ptr }, ptr %i5, i64 0, i32 0
  store ptr @mymod_mp_myglobal_, ptr %i19, align 8
  %i21 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_read_seq_lis(ptr nonnull %i, i32 5, i64 1239157112576, ptr nonnull %i15, ptr nonnull %i5)
  call fastcc void @init_()
  call fastcc void @leapfrog_(ptr getelementptr inbounds ([100 x double], ptr @"main_$A", i64 0, i64 0), ptr getelementptr inbounds ([100 x double], ptr @"main_$B", i64 0, i64 0), ptr nonnull @anon.2, ptr nonnull %i1)
  ret void
}

define internal fastcc void @leapfrog_(ptr noalias nocapture %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture readonly %arg3) unnamed_addr {
bb:
  %t5 = load i32, ptr @mymod_mp_mytester_, align 8
  %t6 = icmp sgt i32 %t5, 5
  br i1 %t6, label %L70, label %L71

L70:                                              ; preds = %bb
  tail call fastcc void @fun0_(ptr %arg, ptr %arg1, ptr %arg2)
  br label %L71

L71:                                              ; preds = %L70, %bb
  %t51 = load i32, ptr @mymod_mp_myglobal_, align 8
  %t61 = icmp eq i32 %t51, 1
  br i1 %t61, label %L72, label %L73

L72:                                              ; preds = %L71
  tail call fastcc void @fun1_(ptr %arg, ptr %arg1, ptr %arg2)
  br label %L73

L73:                                              ; preds = %L72, %L71
  %t52 = load i32, ptr @mymod_mp_myglobal_, align 8
  %t62 = icmp sge i32 %t52, 1
  br i1 %t62, label %L74, label %L75

L74:                                              ; preds = %L73
  tail call fastcc void @fun2_(ptr %arg, ptr %arg1, ptr %arg2)
  br label %L75

L75:                                              ; preds = %L74, %L73
  %t53 = load i1, ptr @mymod_mp_mybool_, align 8
  br i1 %t53, label %L76, label %L77

L76:                                              ; preds = %L75
  tail call fastcc void @extra_(ptr %arg, ptr %arg2)
  br label %L77

L77:                                              ; preds = %L76, %L75
  tail call fastcc void @switch_(ptr %arg, ptr %arg1, ptr %arg2, ptr %arg3)
  br label %L8

L8:                                               ; preds = %L77
  br label %L9

L9:                                               ; preds = %L8
  %t7 = load i32, ptr @mymod_mp_mynnodes_, align 8
  %t8 = icmp eq i32 %t7, -2
  br i1 %t8, label %L12, label %L14

L12:                                              ; preds = %L9
  %t9 = tail call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr getelementptr inbounds ([0 x i8], ptr @anon.0, i64 0, i64 0), i32 0, i32 0, i64 1239157112576, i32 0, i32 0)
  br label %L14

L14:                                              ; preds = %L12, %L9
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
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i9)
  %i11 = load double, ptr %i10, align 8
  %i12 = add nuw nsw i64 %i8, 1
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i12)
  %i14 = load double, ptr %i13, align 8
  %i15 = fadd double %i11, %i14
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i8)
  %i17 = load double, ptr %i16, align 8
  %i18 = fadd double %i15, %i17
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i8)
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
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i9)
  %i11 = load double, ptr %i10, align 8
  %i12 = add nuw nsw i64 %i8, 1
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i12)
  %i14 = load double, ptr %i13, align 8
  %i15 = fadd double %i11, %i14
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i8)
  %i17 = load double, ptr %i16, align 8
  %i18 = fadd double %i15, %i17
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i8)
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
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i8)
  %i10 = load double, ptr %i9, align 8
  %i11 = add nuw nsw i64 %i7, 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i11)
  %i13 = load double, ptr %i12, align 8
  %i14 = fadd double %i10, %i13
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i7)
  %i16 = load double, ptr %i15, align 8
  %i17 = fadd double %i14, %i16
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i7)
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
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i8)
  %i10 = load double, ptr %i9, align 8
  %i11 = add nuw nsw i64 %i7, 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i11)
  %i13 = load double, ptr %i12, align 8
  %i14 = fadd double %i10, %i13
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i7)
  %i16 = load double, ptr %i15, align 8
  %i17 = fadd double %i14, %i16
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i7)
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
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i8)
  %i10 = load double, ptr %i9, align 8
  %i11 = add nuw nsw i64 %i7, 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i11)
  %i13 = load double, ptr %i12, align 8
  %i14 = fadd double %i10, %i13
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i7)
  %i16 = load double, ptr %i15, align 8
  %i17 = fadd double %i14, %i16
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i7)
  store double %i17, ptr %i18, align 8
  %i19 = icmp eq i64 %i11, %i5
  br i1 %i19, label %bb20, label %bb6

bb20:                                             ; preds = %bb6, %bb
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED
