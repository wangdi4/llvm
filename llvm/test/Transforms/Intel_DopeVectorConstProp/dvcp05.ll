; REQUIRES: asserts
; RUN: opt < %s -disable-output -dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that dope vector constants get propagated for uplevels #0 and #1 in
; new_solver_IP_specific_, a contained subroutine for new_solver_. Note that
; new_solver_IP_specific_ is called twice, but only needs to be processed by
; once for each dope vector used it in it.

; CHECK: DOPE VECTOR CONSTANT PROPAGATION: BEGIN
; CHECK: DV FOUND: ARG #0 new_solver_ 1 x i32
; CHECK: VALID
; CHECK: LB[0] = 1
; CHECK: ST[0] = 4
; CHECK: EX[0] = 3
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: TESTING UPLEVEL #0 FOR new_solver_IP_specific_
; CHECK: REPLACING 1 LOAD WITH 3
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: DV FOUND: ARG #1 new_solver_ 2 x i32
; CHECK: VALID
; CHECK: LB[0] = 1
; CHECK: ST[0] = 4
; CHECK: EX[0] = 3
; CHECK: LB[1] = 1
; CHECK: ST[1] = 12
; CHECK: EX[1] = 3
; CHECK: REPLACING 1 LOAD WITH 3
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: REPLACING 1 LOAD WITH 12
; CHECK: TESTING UPLEVEL #1 FOR new_solver_IP_specific_
; CHECK: REPLACING 1 LOAD WITH 3
; CHECK: REPLACING 1 LOAD WITH 3
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: REPLACING 1 LOAD WITH 12
; CHECK: DOPE VECTOR CONSTANT PROPAGATION: END

%uplevel_type = type { { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }*, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* }

@anon.9f612ed7d31cf3fc2b70611956e5ab37.0 = internal unnamed_addr constant i32 2
@"main_$PART" = internal global [3 x i32] zeroinitializer, align 16
@"main_$BLOCK" = internal global [3 x [3 x i32]] zeroinitializer, align 16
declare dso_local i32 @for_set_reentrancy(i32*) local_unnamed_addr

; Function Attrs: nofree nounwind
define internal void @new_solver_({ i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* noalias %0, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias %1) {
  %3 = alloca %uplevel_type, align 8
  %4 = getelementptr inbounds %uplevel_type, %uplevel_type* %3, i64 0, i32 0
  store { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %0, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }** %4, align 8
; NOTE: arg #0 is uplevel #0 for the contained subroutine specific
  %5 = getelementptr inbounds %uplevel_type, %uplevel_type* %3, i64 0, i32 1
  store { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %1, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %5, align 8
; NOTE: arg #1 is uplevel #1 for the contained subroutine specific
  %6 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %1, i64 0, i32 0
  %7 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %1, i64 0, i32 6, i64 0
; NOTE: Load the dope vector dimension base for arg #1 into %7
  %8 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %7, i64 0, i32 0
; NOTE: Load the base address for the extent of arg #1 into %8
  %9 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %7, i64 0, i32 1
; NOTE: Load the base address for the stride of arg #1 into %9
  %10 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %0, i64 0, i32 0
  %11 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %0, i64 0, i32 6, i64 0
; NOTE: Load the dope vector dimension base for arg #0 into %11
  %12 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %11, i64 0, i32 1
; Load the base address for the stride of arg #0 into $12
  call void @new_solver_IP_specific_(%uplevel_type* nonnull %3)
  call void @new_solver_IP_specific_(%uplevel_type* nonnull %3)
  %13 = load i32*, i32** %6, align 8
  %14 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %9, i32 0)
  %15 = load i64, i64* %14, align 8
; NOTE: Load extent 0 value for arg #1 into %15
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %9, i32 1)
  %17 = load i64, i64* %16, align 8
; NOTE: Load stride 0 value for arg #1 into %17
  %18 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %8, i32 1)
  %19 = load i64, i64* %18, align 8
; NOTE: Load extent 1 value for arg #1 into %19
  %20 = load i32*, i32** %10, align 8
  %21 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %12, i32 0)
  %22 = load i64, i64* %21, align 8
; NOTE: Load stride 0 value for arg #1 into %22
  %23 = icmp slt i64 %19, 1
; NOTE: Replace %19 with 3
  br i1 %23, label %32, label %24

24:                                               ; preds = %24, %2
  %25 = phi i64 [ %30, %24 ], [ 1, %2 ]
  %26 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %22, i32* %20, i64 %25)
; NOTE: Replace %22 with 4
  %27 = load i32, i32* %26, align 4
  %28 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %17, i32* %13, i64 %25)
; NOTE: Replace %17 with 4
  %29 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %15, i32* %28, i64 1)
; NOTE: Replace %15 with 3
  store i32 %27, i32* %29, align 4
  %30 = add nuw i64 %25, 1
  %31 = icmp eq i64 %25, %19
; NOTE: Replace %19 with 3
  br i1 %31, label %32, label %24

32:                                               ; preds = %24, %2
  ret void
}

; Function Attrs: nofree nounwind
define internal void @new_solver_IP_specific_(%uplevel_type* nest noalias nocapture readonly %0) {
  %2 = getelementptr inbounds %uplevel_type, %uplevel_type* %0, i64 0, i32 0
  %3 = load { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }*, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }** %2, align 8
; NOTE: %3 is the dope vector for uplevel #0
  %4 = getelementptr inbounds %uplevel_type, %uplevel_type* %0, i64 0, i32 1
  %5 = load { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %4, align 8
; NOTE: %5 is the dope vector for uplevel #0
  %6 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %3, i64 0, i32 0
  %7 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %3, i64 0, i32 6, i64 0
; NOTE: %7 is the dope vector dimension base for uplevel #0
  %8 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %7, i64 0, i32 0
; NOTE: %8 is the base address for the extent of uplevel #0
  %9 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %7, i64 0, i32 1
; NOTE: %9 is the base address for the stride of uplevel #0
  %10 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %5, i64 0, i32 0
  %11 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %5, i64 0, i32 6, i64 0
; NOTE: %11 is the dope vector dimension base for uplevel #1
  %12 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %11, i64 0, i32 0
; NOTE: %12 is the base address for the extent of uplevel #1
  %13 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %11, i64 0, i32 1
; NOTE: %13 is the base address for the stride of uplevel #1
  %14 = load i32*, i32** %6, align 8
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %9, i32 0)
  %16 = load i64, i64* %15, align 8
; NOTE: %16 is the stride 0 of uplevel #0
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %8, i32 0)
  %18 = load i64, i64* %17, align 8
; NOTE: %18 is the extent 0 of uplevel #0
  %19 = icmp slt i64 %18, 1
; NOTE: Replace %18 with 3
  br i1 %19, label %36, label %20

20:                                               ; preds = %20, %1
  %21 = phi i64 [ %23, %20 ], [ 1, %1 ]
  %22 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %16, i32* %14, i64 %21)
; NOTE: Replace %16 with 4
  store i32 0, i32* %22, align 4
  %23 = add nuw i64 %21, 1
  %24 = icmp eq i64 %21, %18
; NOTE: Replace %18 with 3
  br i1 %24, label %36, label %20

25:                                               ; preds = %33, %25
  %26 = phi i64 [ 1, %33 ], [ %28, %25 ]
  %27 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %39, i32* %35, i64 %26)
; NOTE: Replace %39 with 4
  store i32 1, i32* %27, align 4
  %28 = add nuw i64 %26, 1
  %29 = icmp eq i64 %26, %41
; NOTE: Replace %41 with 3
  br i1 %29, label %30, label %25

30:                                               ; preds = %25
  %31 = add nuw i64 %34, 1
  %32 = icmp eq i64 %34, %45
; NOTE: Replace %43 with 3
  br i1 %32, label %49, label %33

33:                                               ; preds = %36, %30
  %34 = phi i64 [ %31, %30 ], [ 1, %36 ]
  %35 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %43, i32* %37, i64 %34)
; NOTE: Replace %43 with 12
  br label %25

36:                                               ; preds = %20, %1
  %37 = load i32*, i32** %10, align 8
  %38 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %13, i32 0)
  %39 = load i64, i64* %38, align 8
; NOTE: %39 is the stride 0 of uplevel #1
  %40 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %12, i32 0)
  %41 = load i64, i64* %40, align 8
; NOTE: %41 is the extent 0 of uplevel #1
  %42 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %13, i32 1)
  %43 = load i64, i64* %42, align 8
; NOTE: %43 is the stride 1 of uplevel #1
  %44 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %12, i32 1)
  %45 = load i64, i64* %44, align 8
; NOTE: %45 is the extent 1 of uplevel #1
  %46 = icmp slt i64 %45, 1
; NOTE: Replace %45 with 3
  %47 = icmp slt i64 %41, 1
; NOTE: Replace %41 with 3
  %48 = or i1 %46, %47
  br i1 %48, label %49, label %33

49:                                               ; preds = %36, %30
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

define dso_local void @MAIN__() {
  %1 = alloca { i32 }, align 8
  %2 = alloca [4 x i8], align 1
  %3 = alloca { i32 }, align 8
  %4 = alloca [4 x i8], align 1
  %5 = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
; NOTE: %5 is the dope vector for new_solver actual arg #1
  %6 = alloca { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, align 8
; NOTE: %6 is the dope vector for new_solver actual arg #0
  %7 = alloca [8 x i64], align 16
  %8 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %6, i64 0, i32 0
  %9 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %6, i64 0, i32 1
  %10 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %6, i64 0, i32 2
  %11 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %6, i64 0, i32 3
  %12 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %6, i64 0, i32 4
  %13 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %6, i64 0, i32 6, i64 0
; NOTE: %13 is the dope vector dimension base for new_solver actual arg #0
  %14 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %13, i64 0, i32 0
; NOTE: %14 is the dope vector base for the extent of new_solver actual arg #0
  %15 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %13, i64 0, i32 1
; NOTE: %15 is the dope vector base for the stride of new_solver actual arg #0
  %16 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %13, i64 0, i32 2
; NOTE: %16 is the dope vector base for the lower bound of new_solver actual arg #0
  %17 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %5, i64 0, i32 0
  %18 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %5, i64 0, i32 1
  %19 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %5, i64 0, i32 2
  %20 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %5, i64 0, i32 3
  %21 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %5, i64 0, i32 4
  %22 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %5, i64 0, i32 6, i64 0
; NOTE: %22 is the dope vector dimension base for new_solver actual arg #1
  %23 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %22, i64 0, i32 0
; NOTE: %23 is the dope vector base for the extent of new_solver actual arg #1
  %24 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %22, i64 0, i32 1
; NOTE: %24 is the dope vector base for the stride of new_solver actual arg #1
  %25 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %22, i64 0, i32 2
; NOTE: %25 is the dope vector base for the lower bound of new_solver actual arg #1
  %26 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.9f612ed7d31cf3fc2b70611956e5ab37.0)
  store i64 4, i64* %9, align 8
  store i64 1, i64* %12, align 8
  store i64 0, i64* %10, align 8
  %27 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %15, i32 0)
  store i64 4, i64* %27, align 8
; NOTE: stride 0 of new_solver arg #0 is 4
  %28 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %16, i32 0)
  store i64 1, i64* %28, align 8
; NOTE: lower bound 0 of new_solver arg #0 is 1
  %29 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %14, i32 0)
  store i64 3, i64* %29, align 8
; NOTE: extent 0 of new_solver arg #0 is 3
  store i32* getelementptr inbounds ([3 x i32], [3 x i32]* @"main_$PART", i64 0, i64 0), i32** %8, align 8
  store i64 1, i64* %11, align 8
  store i64 4, i64* %18, align 8
  store i64 2, i64* %21, align 8
  store i64 0, i64* %19, align 8
  %30 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %24, i32 0)
  store i64 4, i64* %30, align 8
; NOTE: stride 0 of new_solver arg #1 is 4
  %31 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %25, i32 0)
  store i64 1, i64* %31, align 8
; NOTE: lower bound 0 of new_solver arg #1 is 1
  %32 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %23, i32 0)
  store i64 3, i64* %32, align 8
; NOTE: extent 0 of new_solver arg #1 is 3
  %33 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %24, i32 1)
  store i64 12, i64* %33, align 8
; NOTE: stride 1 of new_solver arg #1 is 12
  %34 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %25, i32 1)
  store i64 1, i64* %34, align 8
; NOTE: lower bound 1 of new_solver arg #1 is 1
  %35 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %23, i32 1)
  store i64 3, i64* %35, align 8
; NOTE: extent 1 of new_solver arg #1 is 3
  store i32* getelementptr inbounds ([3 x [3 x i32]], [3 x [3 x i32]]* @"main_$BLOCK", i64 0, i64 0, i64 0), i32** %17, align 8
  store i64 1, i64* %20, align 8
  call void @new_solver_({ i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* nonnull %6, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* nonnull %5)
  ret void
}



