; REQUIRES: asserts
; RUN: opt < %s -S -tilemvinlmarker -debug-only=tilemvinlmarker -tile-candidate-test -tile-candidate-min=4 -tile-candidate-arg-min=3 -tile-candidate-sub-arg-min=2 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes='tilemvinlmarker' -debug-only=tilemvinlmarker -tile-candidate-test -tile-candidate-min=4 -tile-candidate-arg-min=3 -tile-candidate-sub-arg-min=2 2>&1 | FileCheck %s

; Check that the loop indices and increments are correctly identified for
;   the loops within the tile candidates.
; Check that the loop index and loop increment for each subscript arg is
;   correctly identified.
; Check that the tile candidates are correctly identified.
; This is a variant of TileMVInl02.ll with @fun3_ introduced, which is a tile
; candidate, but not a tile choice, because it does not have a unique caller.

; CHECK: TMVINL: fun01_ Loop Index   %14 = add nuw nsw i64 %10, 1
; CHECK: TMVINL: fun01_ Loop Inc   %10 = phi i64 [ 3, %6 ], [ %14, %9 ]
; CHECK: TMVINL: fun01_ Arg %0(2,1)
; CHECK: TMVINL: Tile Candidate fun01_
; CHECK: TMVINL: fun00_ Loop Index   %14 = add nuw nsw i64 %10, 1
; CHECK: TMVINL: fun00_ Loop Inc   %10 = phi i64 [ 3, %6 ], [ %14, %9 ]
; CHECK: TMVINL: fun00_ Arg %0(0,1)
; CHECK: TMVINL: fun00_ Arg %1(2,1)
; CHECK: TMVINL: Tile Candidate fun00_
; CHECK: TMVINL: fun1_ Loop Index   %13 = add nuw nsw i64 %9, 1
; CHECK: TMVINL: fun1_ Loop Inc   %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
; CHECK: TMVINL: fun1_ Arg %0(2,1)
; CHECK: TMVINL: Tile Candidate fun1_
; CHECK: TMVINL: fun2_ Loop Index   %13 = add nuw nsw i64 %9, 1
; CHECK: TMVINL: fun2_ Loop Inc   %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
; CHECK: TMVINL: fun2_ Arg %0(2,1)
; CHECK: TMVINL: Tile Candidate fun2_
; CHECK: TMVINL: fun3_ Loop Index   %13 = add nuw nsw i64 %9, 1
; CHECK: TMVINL: fun3_ Loop Inc   %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
; CHECK: TMVINL: fun3_ Arg %0(2,1)
; CHECK: TMVINL: Tile Candidate fun3_
; CHECK: TMVINL: fun0_ Loop Index   %13 = add nuw nsw i64 %9, 1
; CHECK: TMVINL: fun0_ Loop Inc   %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
; CHECK: TMVINL: fun0_ Arg %0(0,1)
; CHECK: TMVINL: fun0_ Arg %1(2,1)
; CHECK: TMVINL: Tile Candidate fun0_

; Check that there are no tile choices, since fun3_ did not have a unique
; caller.

; CHECK-NOT: TMVINL: Tile Choice fun0_
; CHECK-NOT: TMVINL: Tile Choice fun1_
; CHECK-NOT: TMVINL: Tile Choice fun2_
; CHECK-NOT: TMVINL: Tile Choice fun3_
; CHECK-NOT: TMVINL: Tile Choice extra_
; CHECK-NOT: TMVINL: Tile Choice switch_
; CHECK-NOT: TMVINL: Tile Choice fun00_
; CHECK-NOT: TMVINL: Tile Choice fun01_

; Check that guards for the tile choices are not identified, because there
; are none.

; CHECK-NOT: TMVINL: GVMAP
; CHECK-NOT: TMVINL: CONDMAP

; Check that the global variables were not validated, because the tile choices
; were not determined.

; CHECK-NOT: TMVINL: Validated GVM

; Check that no simplification of conditionals with globals was done.

; CHECK-NOT: TMVINL: Testing
; CHECK-NOT: Against
; CHECK-NOT: Provably

; Check that the tile choices were not marked for inlining.

; CHECK-NOT: TMVINL: Marked

; Check that the skeleton graph shows the right tile choices (which are no
; choices).

; CHECK-NOT: TMVINL: Root
; CHECK-NOT: TMVINL: SubRoot

; Check the IR

; CHECK: define{{.*}}@MAIN__({{.*}})
; CHECK: call{{.*}}@leapfrog_({{.*}}){{ *$}}
; CHECK: define{{.*}}@leapfrog_({{.*}})
; CHECK: call{{.*}}@fun0_({{.*}}){{ *$}}
; CHECK: call{{.*}}@fun1_({{.*}}){{ *$}}
; CHECK: call{{.*}}@fun2_({{.*}}){{ *$}}
; CHECK: call{{.*}}@fun3_({{.*}}){{ *$}}
; CHECK: call{{.*}}@extra_({{.*}}){{ *$}}
; CHECK: call{{.*}}@switch_({{.*}}){{ *$}}
; CHECK: define{{.*}}@switch_({{.*}})
; CHECK: call{{.*}}@fun00_({{.*}}){{ *$}}
; CHECK: call{{.*}}@fun01_({{.*}}){{ *$}}
; CHECK: call{{.*}}@fun3_({{.*}}){{ *$}}
; CHECK-NOT: attributes{{.*}}prefer-inline-tile-choice

@anon.0 = internal unnamed_addr constant i32 2
@"main_$A" = internal global [100 x [100 x double]] zeroinitializer, align 16
@"main_$B" = internal global [100 x [100 x double]] zeroinitializer, align 16
@anon.1 = internal unnamed_addr constant i32 100

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

declare dso_local i32 @for_set_reentrancy(i32*) local_unnamed_addr

declare dso_local i32 @for_read_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr

define dso_local void @MAIN__() local_unnamed_addr #2 {
  %1 = alloca [8 x i64], align 16
  %2 = alloca i32, align 8
  %3 = alloca [4 x i8], align 1
  %4 = alloca { i8* }, align 8
  %5 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.0)
  %6 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 0
  store i8 16, i8* %6, align 1
  %7 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 1
  store i8 3, i8* %7, align 1
  %8 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 2
  store i8 1, i8* %8, align 1
  %9 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 3
  store i8 0, i8* %9, align 1
  %10 = bitcast { i8* }* %4 to i32**
  store i32* %2, i32** %10, align 8
  %11 = bitcast [8 x i64]* %1 to i8*
  %12 = bitcast { i8* }* %4 to i8*
  %13 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_read_seq_lis(i8* nonnull %11, i32 5, i64 1239157112576, i8* nonnull %6, i8* nonnull %12)
  call fastcc void @leapfrog_(double* getelementptr inbounds ([100 x [100 x double]], [100 x [100 x double]]* @"main_$A", i64 0, i64 0, i64 0), double* getelementptr inbounds ([100 x [100 x double]], [100 x [100 x double]]* @"main_$B", i64 0, i64 0, i64 0), i32* nonnull @anon.1, i32* nonnull %2)
  ret void
}

define internal void @extra_(double* noalias nocapture %0, i32* noalias nocapture readonly %1) #0 {
  %3 = load i32, i32* %1, align 4
  %4 = icmp slt i32 %3, 1
  br i1 %4, label %13, label %5

5:                                                ; preds = %2
  %6 = add nuw i32 %3, 1
  %7 = zext i32 %6 to i64
  br label %8

8:                                                ; preds = %8, %5
  %9 = phi i64 [ 1, %5 ], [ %11, %8 ]
  %10 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %9)
  store double 5.000000e+00, double* %10, align 8
  %11 = add nuw nsw i64 %9, 1
  %12 = icmp eq i64 %11, %7
  br i1 %12, label %13, label %8

13:                                               ; preds = %8, %2
  ret void
}

define internal fastcc void @leapfrog_(double* noalias nocapture %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2, i32* noalias nocapture readonly %3) unnamed_addr #0 {
  tail call fastcc void @fun0_(double* %0, double* %1, i32* %2)
  tail call fastcc void @fun1_(double* %0, double* %1, i32* %2)
  tail call fastcc void @fun2_(double* %0, double* %1, i32* %2)
  tail call fastcc void @fun3_(double* %0, double* %1, i32* %2)
  tail call fastcc void @extra_(double* %0, i32* %2)
  tail call fastcc void @switch_(double* %0, double* %1, i32* %2, i32* %3)
  ret void
}

define internal fastcc void @switch_(double* noalias nocapture %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2, i32* noalias nocapture readonly %3) unnamed_addr #0 {
  %5 = load i32, i32* %3, align 4
  %6 = and i32 %5, 1
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %9, label %8

8:                                                ; preds = %4
  tail call fastcc void @fun00_(double* %0, double* %1, i32* %2)
  br label %10

9:                                                ; preds = %4
  tail call fastcc void @fun01_(double* %0, double* %1, i32* %2)
  br label %10

10:                                               ; preds = %9, %8
  tail call fastcc void @fun3_(double* %0, double* %1, i32* %2)
  ret void
}

define internal fastcc void @fun01_(double* noalias nocapture readonly %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2) unnamed_addr #0 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 5
  br i1 %5, label %23, label %6

6:                                                ; preds = %3
  %7 = add nsw i32 %4, -1
  %8 = zext i32 %7 to i64
  br label %9

9:                                                ; preds = %9, %6
  %10 = phi i64 [ 3, %6 ], [ %14, %9 ]
  %11 = add nsw i64 %10, -1
  %12 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %11)
  %13 = load double, double* %12, align 8
  %14 = add nuw nsw i64 %10, 1
  %15 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %14)
  %16 = load double, double* %15, align 8
  %17 = fadd double %13, %16
  %18 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %10)
  %19 = load double, double* %18, align 8
  %20 = fadd double %17, %19
  %21 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %10)
  store double %20, double* %21, align 8
  %22 = icmp eq i64 %14, %8
  br i1 %22, label %23, label %9

23:                                               ; preds = %9, %3
  ret void
}

define internal fastcc void @fun00_(double* noalias nocapture %0, double* noalias nocapture readonly %1, i32* noalias nocapture readonly %2) unnamed_addr #0 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 5
  br i1 %5, label %23, label %6

6:                                                ; preds = %3
  %7 = add nsw i32 %4, -1
  %8 = zext i32 %7 to i64
  br label %9

9:                                                ; preds = %9, %6
  %10 = phi i64 [ 3, %6 ], [ %14, %9 ]
  %11 = add nsw i64 %10, -1
  %12 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %11)
  %13 = load double, double* %12, align 8
  %14 = add nuw nsw i64 %10, 1
  %15 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %14)
  %16 = load double, double* %15, align 8
  %17 = fadd double %13, %16
  %18 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %10)
  %19 = load double, double* %18, align 8
  %20 = fadd double %17, %19
  %21 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %10)
  store double %20, double* %21, align 8
  %22 = icmp eq i64 %14, %8
  br i1 %22, label %23, label %9

23:                                               ; preds = %9, %3
  ret void
}

define internal fastcc void @fun1_(double* noalias nocapture readonly %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2) unnamed_addr #0 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 3
  br i1 %5, label %22, label %6

6:                                                ; preds = %3
  %7 = zext i32 %4 to i64
  br label %8

8:                                                ; preds = %8, %6
  %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
  %10 = add nsw i64 %9, -1
  %11 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %10)
  %12 = load double, double* %11, align 8
  %13 = add nuw nsw i64 %9, 1
  %14 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %13)
  %15 = load double, double* %14, align 8
  %16 = fadd double %12, %15
  %17 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %9)
  %18 = load double, double* %17, align 8
  %19 = fadd double %16, %18
  %20 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %9)
  store double %19, double* %20, align 8
  %21 = icmp eq i64 %13, %7
  br i1 %21, label %22, label %8

22:                                               ; preds = %8, %3
  ret void
}

define internal fastcc void @fun2_(double* noalias nocapture readonly %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2) unnamed_addr #0 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 3
  br i1 %5, label %22, label %6

6:                                                ; preds = %3
  %7 = zext i32 %4 to i64
  br label %8

8:                                                ; preds = %8, %6
  %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
  %10 = add nsw i64 %9, -1
  %11 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %10)
  %12 = load double, double* %11, align 8
  %13 = add nuw nsw i64 %9, 1
  %14 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %13)
  %15 = load double, double* %14, align 8
  %16 = fadd double %12, %15
  %17 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %9)
  %18 = load double, double* %17, align 8
  %19 = fadd double %16, %18
  %20 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %9)
  store double %19, double* %20, align 8
  %21 = icmp eq i64 %13, %7
  br i1 %21, label %22, label %8

22:                                               ; preds = %8, %3
  ret void
}

define internal fastcc void @fun3_(double* noalias nocapture readonly %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2) unnamed_addr #0 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 3
  br i1 %5, label %22, label %6

6:                                                ; preds = %3
  %7 = zext i32 %4 to i64
  br label %8

8:                                                ; preds = %8, %6
  %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
  %10 = add nsw i64 %9, -1
  %11 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %10)
  %12 = load double, double* %11, align 8
  %13 = add nuw nsw i64 %9, 1
  %14 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %13)
  %15 = load double, double* %14, align 8
  %16 = fadd double %12, %15
  %17 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %9)
  %18 = load double, double* %17, align 8
  %19 = fadd double %16, %18
  %20 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %9)
  store double %19, double* %20, align 8
  %21 = icmp eq i64 %13, %7
  br i1 %21, label %22, label %8

22:                                               ; preds = %8, %3
  ret void
}

define internal fastcc void @fun0_(double* noalias nocapture %0, double* noalias nocapture readonly %1, i32* noalias nocapture readonly %2) unnamed_addr #0 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 3
  br i1 %5, label %22, label %6

6:                                                ; preds = %3
  %7 = zext i32 %4 to i64
  br label %8

8:                                                ; preds = %8, %6
  %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
  %10 = add nsw i64 %9, -1
  %11 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %10)
  %12 = load double, double* %11, align 8
  %13 = add nuw nsw i64 %9, 1
  %14 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %13)
  %15 = load double, double* %14, align 8
  %16 = fadd double %12, %15
  %17 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %9)
  %18 = load double, double* %17, align 8
  %19 = fadd double %16, %18
  %20 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %9)
  store double %19, double* %20, align 8
  %21 = icmp eq i64 %13, %7
  br i1 %21, label %22, label %8

22:                                               ; preds = %8, %3
  ret void
}

