; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -whole-program-assume -passes='module(agginliner),cgscc(inline)' -inline-threshold=-50 -S 2>&1 | FileCheck %s

; Check the IR to ensure that there are no calls to the functions we expect
; to be inlined out due to aggressive inlining.

; CHECK: define{{.*}}@main
; CHECK-NOT: call{{.*}}@LBM_allocateGrid
; CHECK-NOT: call{{.*}}@LBM_initializeGrid
; CHECK-NOT: call{{.*}}@LBM_initializeSpecialCellsForChannel
; CHECK-NOT: call{{.*}}@LBM_initializeSpecialCellsForLDC
; CHECK-NOT: call{{.*}}@LBM_loadObstacleFile
; CHECK-NOT: call{{.*}}@LBM_compareVelocityField
; CHECK-NOT: call{{.*}}@LBM_freeGrid
; CHECK-NOT: call{{.*}}@LBM_handleInOutFlow
; CHECK-NOT: call{{.*}}@LBM_performStreamCollideTRT
; CHECK-NOT: call{{.*}}@LBM_showGridStatistics
; CHECK-NOT: call{{.*}}@LBM_storeVelocityField
; CHECK-NOT: call{{.*}}@LBM_swapGrids
; CHECK-NOT: call{{.*}}@MAIN_initialize
; CHECK-NOT: call{{.*}}@MAIN_parseCommandLine

%struct.MAIN_Param = type { i32, i8*, i32, i32, i8* }
%struct.stat = type { i64, i64, i64, i32, i32, i32, i32, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, [3 x i64] }

@.str = private unnamed_addr constant [49 x i8] c"LBM_allocateGrid: could not allocate %.1f MByte\0A\00", align 1
@__const.MAIN_printInfo.actionString = private unnamed_addr constant [3 x [32 x i8]] [[32 x i8] c"nothing\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [32 x i8] c"compare\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [32 x i8] c"store\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"], align 16
@__const.MAIN_printInfo.simTypeString = private unnamed_addr constant [3 x [32 x i8]] [[32 x i8] c"lid-driven cavity\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [32 x i8] c"channel flow\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [32 x i8] zeroinitializer], align 16
@.str.6.1 = private unnamed_addr constant [7 x i8] c"<none>\00", align 1
@.str.5.2 = private unnamed_addr constant [174 x i8] c"MAIN_printInfo:\0A\09grid size      : %i x %i x %i = %.2f * 10^6 Cells\0A\09nTimeSteps     : %i\0A\09result file    : %s\0A\09action         : %s\0A\09simulation type: %s\0A\09obstacle file  : %s\0A\0A\00", align 1
@srcGrid = internal global [26000000 x double]* null, align 8
@dstGrid = internal global [26000000 x double]* null, align 8
@.str.9 = private unnamed_addr constant [14 x i8] c"timestep: %i\0A\00", align 1

define internal fastcc void @LBM_allocateGrid(double** nocapture %0) unnamed_addr #0 {
  %2 = tail call noalias dereferenceable_or_null(214400000) i8* @malloc(i64 214400000)
  %3 = bitcast double** %0 to i8**
  store i8* %2, i8** %3, align 8
  %4 = icmp eq i8* %2, null
  br i1 %4, label %5, label %7

5:                                                ; preds = %1
  %6 = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([49 x i8], [49 x i8]* @.str, i64 0, i64 0), double 0x40698EF800000000)
  tail call void @exit(i32 1)
  unreachable

7:                                                ; preds = %1
  %8 = getelementptr inbounds i8, i8* %2, i64 3200000
  %9 = bitcast double** %0 to i8**
  store i8* %8, i8** %9, align 8
  ret void
}

declare dso_local noalias i8* @malloc(i64 %0) local_unnamed_addr

declare dso_local i32 @printf(i8* nocapture readonly %0, ...) local_unnamed_addr

declare dso_local void @exit(i32 %0) local_unnamed_addr

define internal fastcc void @LBM_freeGrid(double** nocapture %0) unnamed_addr #0 {
  %2 = load double*, double** %0, align 8
  %3 = getelementptr inbounds double, double* %2, i64 -400000
  %4 = bitcast double* %3 to i8*
  tail call void @free(i8* nonnull %4)
  store double* null, double** %0, align 8
  ret void
}

declare dso_local void @free(i8* nocapture %0) local_unnamed_addr

define internal fastcc void @LBM_initializeGrid(double* nocapture %0) unnamed_addr #0 {
  ret void
}

define internal fastcc void @LBM_swapGrids() unnamed_addr #0 {
  %1 = load i64, i64* bitcast ([26000000 x double]** @srcGrid to i64*), align 8
  %2 = load i64, i64* bitcast ([26000000 x double]** @dstGrid to i64*), align 8
  store i64 %2, i64* bitcast ([26000000 x double]** @srcGrid to i64*), align 8
  store i64 %1, i64* bitcast ([26000000 x double]** @dstGrid to i64*), align 8
  ret void
}

define internal fastcc void @LBM_loadObstacleFile(double* nocapture %0, i8* nocapture readonly %1) unnamed_addr #0 {
  ret void
}

define internal fastcc void @LBM_initializeSpecialCellsForLDC(double* nocapture %0) unnamed_addr #0 {
  ret void
}

define internal fastcc void @LBM_initializeSpecialCellsForChannel(double* nocapture %0) unnamed_addr #0 {
  ret void
}

define internal fastcc void @LBM_performStreamCollideTRT(double* readonly %0, double* %1) unnamed_addr #0 {
  ret void
}

define internal fastcc void @LBM_handleInOutFlow(double* %0) unnamed_addr #0 {
  ret void
}

define internal fastcc void @LBM_showGridStatistics(double* nocapture readonly %0) unnamed_addr #0 {
  ret void
}

define internal fastcc void @LBM_storeVelocityField(double* nocapture readonly %0, i8* nocapture readonly %1) unnamed_addr #0 {
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64 immarg %0, i8* nocapture %1)

declare void @llvm.lifetime.end.p0i8(i64 immarg %0, i8* nocapture %1)

define internal fastcc void @LBM_compareVelocityField(double* nocapture readonly %0, i8* nocapture readonly %1) unnamed_addr #0 {
  ret void
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly %0, i8* noalias nocapture readonly %1, i64 %2, i1 immarg %3)

define dso_local i32 @main(i32 %0, i8** nocapture readonly %1) local_unnamed_addr #0 {
  %3 = alloca [3 x [32 x i8]], align 16
  %4 = alloca [3 x [32 x i8]], align 16
  %5 = alloca %struct.MAIN_Param, align 8
  %6 = bitcast %struct.MAIN_Param* %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %6)
  call fastcc void @MAIN_parseCommandLine(i32 %0, i8** %1, %struct.MAIN_Param* nonnull %5)
  %7 = getelementptr inbounds [3 x [32 x i8]], [3 x [32 x i8]]* %3, i64 0, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 96, i8* nonnull %7)
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 16 dereferenceable(96) %7, i8* nonnull align 16 dereferenceable(96) getelementptr inbounds ([3 x [32 x i8]], [3 x [32 x i8]]* @__const.MAIN_printInfo.actionString, i64 0, i64 0, i64 0), i64 96, i1 false)
  %8 = getelementptr inbounds [3 x [32 x i8]], [3 x [32 x i8]]* %4, i64 0, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 96, i8* nonnull %8)
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 16 dereferenceable(96) %8, i8* nonnull align 16 dereferenceable(96) getelementptr inbounds ([3 x [32 x i8]], [3 x [32 x i8]]* @__const.MAIN_printInfo.simTypeString, i64 0, i64 0, i64 0), i64 96, i1 false)
  %9 = getelementptr inbounds %struct.MAIN_Param, %struct.MAIN_Param* %5, i64 0, i32 0
  %10 = load i32, i32* %9, align 8
  %11 = getelementptr inbounds %struct.MAIN_Param, %struct.MAIN_Param* %5, i64 0, i32 1
  %12 = load i8*, i8** %11, align 8
  %13 = getelementptr inbounds %struct.MAIN_Param, %struct.MAIN_Param* %5, i64 0, i32 2
  %14 = load i32, i32* %13, align 8
  %15 = getelementptr inbounds %struct.MAIN_Param, %struct.MAIN_Param* %5, i64 0, i32 3
  %16 = load i32, i32* %15, align 4
  %17 = getelementptr inbounds %struct.MAIN_Param, %struct.MAIN_Param* %5, i64 0, i32 4
  %18 = load i8*, i8** %17, align 8
  %19 = icmp eq i8* %18, null
  %20 = select i1 %19, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.6.1, i64 0, i64 0), i8* %18
  %21 = zext i32 %16 to i64
  %22 = getelementptr inbounds [3 x [32 x i8]], [3 x [32 x i8]]* %4, i64 0, i64 %21, i64 0
  %23 = zext i32 %14 to i64
  %24 = getelementptr inbounds [3 x [32 x i8]], [3 x [32 x i8]]* %3, i64 0, i64 %23, i64 0
  %25 = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([174 x i8], [174 x i8]* @.str.5.2, i64 0, i64 0), i32 100, i32 100, i32 130, double 0x3FF4CCCCCCCCCCCC, i32 %10, i8* %12, i8* nonnull %24, i8* nonnull %22, i8* %20)
  call void @llvm.lifetime.end.p0i8(i64 96, i8* nonnull %8)
  call void @llvm.lifetime.end.p0i8(i64 96, i8* nonnull %7)
  call fastcc void @MAIN_initialize(%struct.MAIN_Param* nonnull %5)
  %26 = icmp slt i32 %10, 1
  br i1 %26, label %61, label %27

27:                                               ; preds = %2
  %28 = icmp eq i32 %16, 1
  br i1 %28, label %29, label %46

29:                                               ; preds = %43, %27
  %30 = phi i32 [ %44, %43 ], [ 1, %27 ]
  %31 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %32 = getelementptr inbounds [26000000 x double], [26000000 x double]* %31, i64 0, i64 0
  call fastcc void @LBM_handleInOutFlow(double* %32)
  %33 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %34 = getelementptr inbounds [26000000 x double], [26000000 x double]* %33, i64 0, i64 0
  %35 = load [26000000 x double]*, [26000000 x double]** @dstGrid, align 8
  %36 = getelementptr inbounds [26000000 x double], [26000000 x double]* %35, i64 0, i64 0
  call fastcc void @LBM_performStreamCollideTRT(double* %34, double* %36)
  call fastcc void @LBM_swapGrids()
  %37 = and i32 %30, 63
  %38 = icmp eq i32 %37, 0
  br i1 %38, label %39, label %43

39:                                               ; preds = %29
  %40 = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([14 x i8], [14 x i8]* @.str.9, i64 0, i64 0), i32 %30)
  %41 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %42 = getelementptr inbounds [26000000 x double], [26000000 x double]* %41, i64 0, i64 0
  call fastcc void @LBM_showGridStatistics(double* %42)
  br label %43

43:                                               ; preds = %39, %29
  %44 = add nuw nsw i32 %30, 1
  %45 = icmp sgt i32 %44, %10
  br i1 %45, label %61, label %29

46:                                               ; preds = %58, %27
  %47 = phi i32 [ %59, %58 ], [ 1, %27 ]
  %48 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %49 = getelementptr inbounds [26000000 x double], [26000000 x double]* %48, i64 0, i64 0
  %50 = load [26000000 x double]*, [26000000 x double]** @dstGrid, align 8
  %51 = getelementptr inbounds [26000000 x double], [26000000 x double]* %50, i64 0, i64 0
  call fastcc void @LBM_performStreamCollideTRT(double* %49, double* %51)
  call fastcc void @LBM_swapGrids()
  %52 = and i32 %47, 63
  %53 = icmp eq i32 %52, 0
  br i1 %53, label %54, label %58

54:                                               ; preds = %46
  %55 = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([14 x i8], [14 x i8]* @.str.9, i64 0, i64 0), i32 %47)
  %56 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %57 = getelementptr inbounds [26000000 x double], [26000000 x double]* %56, i64 0, i64 0
  call fastcc void @LBM_showGridStatistics(double* %57)
  br label %58

58:                                               ; preds = %54, %46
  %59 = add nuw nsw i32 %47, 1
  %60 = icmp sgt i32 %59, %10
  br i1 %60, label %61, label %46

61:                                               ; preds = %58, %43, %2
  %62 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %63 = getelementptr inbounds [26000000 x double], [26000000 x double]* %62, i64 0, i64 0
  call fastcc void @LBM_showGridStatistics(double* %63)
  switch i32 %14, label %70 [
    i32 1, label %64
    i32 2, label %67
  ]

64:                                               ; preds = %61
  %65 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %66 = getelementptr inbounds [26000000 x double], [26000000 x double]* %65, i64 0, i64 0
  call fastcc void @LBM_compareVelocityField(double* %66, i8* %12)
  br label %70

67:                                               ; preds = %61
  %68 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %69 = getelementptr inbounds [26000000 x double], [26000000 x double]* %68, i64 0, i64 0
  call fastcc void @LBM_storeVelocityField(double* %69, i8* %12)
  br label %70

70:                                               ; preds = %67, %64, %61
  call fastcc void @LBM_freeGrid(double** bitcast ([26000000 x double]** @srcGrid to double**))
  call fastcc void @LBM_freeGrid(double** bitcast ([26000000 x double]** @dstGrid to double**))
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %6)
  ret i32 0
}

define internal fastcc void @MAIN_parseCommandLine(i32 %0, i8** nocapture readonly %1, %struct.MAIN_Param* nocapture %2) unnamed_addr #0 {
  ret void
}

define internal fastcc void @MAIN_initialize(%struct.MAIN_Param* nocapture readonly %0) unnamed_addr #0 {
  tail call fastcc void @LBM_allocateGrid(double** bitcast ([26000000 x double]** @srcGrid to double**))
  tail call fastcc void @LBM_allocateGrid(double** bitcast ([26000000 x double]** @dstGrid to double**))
  %2 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %3 = getelementptr inbounds [26000000 x double], [26000000 x double]* %2, i64 0, i64 0
  tail call fastcc void @LBM_initializeGrid(double* %3)
  %4 = load [26000000 x double]*, [26000000 x double]** @dstGrid, align 8
  %5 = getelementptr inbounds [26000000 x double], [26000000 x double]* %4, i64 0, i64 0
  tail call fastcc void @LBM_initializeGrid(double* %5)
  %6 = getelementptr inbounds %struct.MAIN_Param, %struct.MAIN_Param* %0, i64 0, i32 4
  %7 = load i8*, i8** %6, align 8
  %8 = icmp eq i8* %7, null
  br i1 %8, label %15, label %9

9:                                                ; preds = %1
  %10 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %11 = getelementptr inbounds [26000000 x double], [26000000 x double]* %10, i64 0, i64 0
  tail call fastcc void @LBM_loadObstacleFile(double* %11, i8* nonnull %7)
  %12 = load [26000000 x double]*, [26000000 x double]** @dstGrid, align 8
  %13 = getelementptr inbounds [26000000 x double], [26000000 x double]* %12, i64 0, i64 0
  %14 = load i8*, i8** %6, align 8
  tail call fastcc void @LBM_loadObstacleFile(double* %13, i8* %14)
  br label %15

15:                                               ; preds = %9, %1
  %16 = getelementptr inbounds %struct.MAIN_Param, %struct.MAIN_Param* %0, i64 0, i32 3
  %17 = load i32, i32* %16, align 4
  %18 = icmp eq i32 %17, 1
  %19 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  br i1 %18, label %20, label %24

20:                                               ; preds = %15
  %21 = getelementptr inbounds [26000000 x double], [26000000 x double]* %19, i64 0, i64 0
  tail call fastcc void @LBM_initializeSpecialCellsForChannel(double* %21)
  %22 = load [26000000 x double]*, [26000000 x double]** @dstGrid, align 8
  %23 = getelementptr inbounds [26000000 x double], [26000000 x double]* %22, i64 0, i64 0
  tail call fastcc void @LBM_initializeSpecialCellsForChannel(double* %23)
  br label %28

24:                                               ; preds = %15
  %25 = getelementptr inbounds [26000000 x double], [26000000 x double]* %19, i64 0, i64 0
  tail call fastcc void @LBM_initializeSpecialCellsForLDC(double* %25)
  %26 = load [26000000 x double]*, [26000000 x double]** @dstGrid, align 8
  %27 = getelementptr inbounds [26000000 x double], [26000000 x double]* %26, i64 0, i64 0
  tail call fastcc void @LBM_initializeSpecialCellsForLDC(double* %27)
  br label %28

28:                                               ; preds = %24, %20
  %29 = load [26000000 x double]*, [26000000 x double]** @srcGrid, align 8
  %30 = getelementptr inbounds [26000000 x double], [26000000 x double]* %29, i64 0, i64 0
  tail call fastcc void @LBM_showGridStatistics(double* %30)
  ret void
}

attributes #0 = { norecurse }
; end INTEL_FEATURE_SW_ADVANCED
