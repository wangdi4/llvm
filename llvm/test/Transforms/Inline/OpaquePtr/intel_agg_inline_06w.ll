; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -whole-program-assume -intel-libirc-allowed -agginliner -inline -inline-report=0xe807 -inline-threshold=-50 -S 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers < %s -whole-program-assume -intel-libirc-allowed -passes='module(agginliner),cgscc(inline)' -inline-report=0xe807 -inline-threshold=-50 -S 2>&1 | FileCheck %s

; Check the IR to ensure that there are no calls to the functions we expect
; to be inlined out due to aggressive inlining.
; This test case is similar to intel_agg_inline06.ll, but is derived from
; the IR generated on Windows rather than Linux.

; CHECK-DAG: define {{.*}}__local_stdio_printf_options
; CHECK-DAG: define {{.*}}__local_stdio_scanf_options
; CHECK-DAG: define {{.*}}printf
; CHECK-DAG: define {{.*}}main
; CHECK-NOT: call{{.*}}LBM_allocateGrid
; CHECK-NOT: call{{.*}}LBM_initializeGrid
; CHECK-NOT: call{{.*}}LBM_initializeSpecialCellsForChannel
; CHECK-NOT: call{{.*}}LBM_initializeSpecialCellsForLDC
; CHECK-NOT: call{{.*}}LBM_loadObstacleFile
; CHECK-NOT: call{{.*}}LBM_compareVelocityField
; CHECK-NOT: call{{.*}}LBM_freeGrid
; CHECK-NOT: call{{.*}}LBM_handleInOutFlow
; CHECK-NOT: call{{.*}}LBM_performStreamCollideTRT
; CHECK-NOT: call{{.*}}LBM_showGridStatistics
; CHECK-NOT: call{{.*}}LBM_storeVelocityField
; CHECK-NOT: call{{.*}}LBM_swapGrids
; CHECK-NOT: call{{.*}}MAIN_initialize
; CHECK-NOT: call{{.*}}MAIN_parseCommandLine

%struct.MAIN_Param = type { i32, ptr, i32, i32, ptr }

$__local_stdio_printf_options = comdat any

$__local_stdio_scanf_options = comdat any

@__local_stdio_scanf_options._OptionsStorage = internal global i64 0, align 8
@__local_stdio_printf_options._OptionsStorage = internal global i64 0, align 8
@.str = private unnamed_addr constant [49 x i8] c"LBM_allocateGrid: could not allocate %.1f MByte\0A\00", align 1
@__const.MAIN_printInfo.actionString = private unnamed_addr constant [3 x [32 x i8]] [[32 x i8] c"nothing\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [32 x i8] c"compare\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [32 x i8] c"store\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"], align 16
@__const.MAIN_printInfo.simTypeString = private unnamed_addr constant [3 x [32 x i8]] [[32 x i8] c"lid-driven cavity\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [32 x i8] c"channel flow\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [32 x i8] zeroinitializer], align 16
@.str.6.1 = private unnamed_addr constant [7 x i8] c"<none>\00", align 1
@.str.5.2 = private unnamed_addr constant [174 x i8] c"MAIN_printInfo:\0A\09grid size      : %i x %i x %i = %.2f * 10^6 Cells\0A\09nTimeSteps     : %i\0A\09result file    : %s\0A\09action         : %s\0A\09simulation type: %s\0A\09obstacle file  : %s\0A\0A\00", align 1
@srcGrid = internal global ptr null, align 8
@dstGrid = internal global ptr null, align 8
@.str.9 = private unnamed_addr constant [14 x i8] c"timestep: %i\0A\00", align 1

declare dso_local ptr @__acrt_iob_func(i32) local_unnamed_addr

declare dso_local i32 @__stdio_common_vfprintf(i64, ptr nocapture, ptr readonly, ptr nocapture, ptr) local_unnamed_addr

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start(ptr) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end(ptr) #0

; Function Attrs: norecurse
define internal fastcc void @LBM_allocateGrid(ptr nocapture %arg) unnamed_addr #1 {
bb:
  %i = tail call noalias dereferenceable_or_null(214400000) ptr @malloc(i64 214400000)
  store ptr %i, ptr %arg, align 8
  %i2 = icmp eq ptr %i, null
  br i1 %i2, label %bb3, label %bb4

bb3:                                              ; preds = %bb
  tail call void (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str, double 0x40698EF800000000)
  tail call void @exit(i32 1)
  unreachable

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds i8, ptr %i, i64 3200000
  store ptr %i5, ptr %arg, align 8
  ret void
}

declare dso_local noalias ptr @malloc(i64) local_unnamed_addr

declare dso_local void @exit(i32) local_unnamed_addr

define weak_odr dso_local ptr @__local_stdio_printf_options() local_unnamed_addr comdat {
bb:
  ret ptr @__local_stdio_printf_options._OptionsStorage
}

define weak_odr dso_local ptr @__local_stdio_scanf_options() local_unnamed_addr comdat {
bb:
  ret ptr @__local_stdio_scanf_options._OptionsStorage
}

; Function Attrs: norecurse
define internal void @printf(ptr readonly %arg, ...) unnamed_addr #1 {
bb:
  %i = alloca ptr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i)
  call void @llvm.va_start(ptr nonnull %i)
  %i2 = load ptr, ptr %i, align 8
  %i3 = call ptr @__acrt_iob_func(i32 1)
  %i4 = call ptr @__local_stdio_printf_options()
  %i5 = load i64, ptr %i4, align 8
  %i6 = call i32 @__stdio_common_vfprintf(i64 %i5, ptr %i3, ptr %arg, ptr null, ptr %i2)
  call void @llvm.va_end(ptr nonnull %i)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i)
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_freeGrid(ptr nocapture %arg) unnamed_addr #1 {
bb:
  %i = load ptr, ptr %arg, align 8
  %i1 = getelementptr inbounds double, ptr %i, i64 -400000
  tail call void @free(ptr nonnull %i1)
  store ptr null, ptr %arg, align 8
  ret void
}

declare dso_local void @free(ptr nocapture) local_unnamed_addr

; Function Attrs: norecurse
define internal fastcc void @LBM_initializeGrid(ptr nocapture %arg) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_swapGrids() unnamed_addr #1 {
bb:
  %i = load i64, ptr @srcGrid, align 8
  %i1 = load i64, ptr @dstGrid, align 8
  store i64 %i1, ptr @srcGrid, align 8
  store i64 %i, ptr @dstGrid, align 8
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_loadObstacleFile(ptr nocapture %arg, ptr nocapture readonly %arg1) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_initializeSpecialCellsForLDC(ptr nocapture %arg) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_initializeSpecialCellsForChannel(ptr nocapture %arg) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_performStreamCollideTRT(ptr readonly %arg, ptr %arg1) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_handleInOutFlow(ptr %arg) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_showGridStatistics(ptr nocapture readonly %arg) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_storeVelocityField(ptr nocapture readonly %arg, ptr nocapture readonly %arg1) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @LBM_compareVelocityField(ptr nocapture readonly %arg, ptr nocapture readonly %arg1) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define dso_local i32 @main(i32 %arg, ptr nocapture readonly %arg1) local_unnamed_addr #1 {
bb:
  %i = alloca [3 x [32 x i8]], align 16
  %i2 = alloca [3 x [32 x i8]], align 16
  %i3 = alloca %struct.MAIN_Param, align 8
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %i3)
  call fastcc void @MAIN_parseCommandLine(i32 %arg, ptr %arg1, ptr nonnull %i3)
  %i5 = getelementptr inbounds [3 x [32 x i8]], ptr %i, i64 0, i64 0, i64 0
  call void @llvm.lifetime.start.p0(i64 96, ptr nonnull %i5)
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 16 dereferenceable(96) %i5, ptr nonnull align 16 dereferenceable(96) @__const.MAIN_printInfo.actionString, i64 96, i1 false)
  %i6 = getelementptr inbounds [3 x [32 x i8]], ptr %i2, i64 0, i64 0, i64 0
  call void @llvm.lifetime.start.p0(i64 96, ptr nonnull %i6)
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 16 dereferenceable(96) %i6, ptr nonnull align 16 dereferenceable(96) @__const.MAIN_printInfo.simTypeString, i64 96, i1 false)
  %i7 = getelementptr inbounds %struct.MAIN_Param, ptr %i3, i64 0, i32 0
  %i8 = load i32, ptr %i7, align 8
  %i9 = getelementptr inbounds %struct.MAIN_Param, ptr %i3, i64 0, i32 1
  %i10 = load ptr, ptr %i9, align 8
  %i11 = getelementptr inbounds %struct.MAIN_Param, ptr %i3, i64 0, i32 2
  %i12 = load i32, ptr %i11, align 8
  %i13 = getelementptr inbounds %struct.MAIN_Param, ptr %i3, i64 0, i32 3
  %i14 = load i32, ptr %i13, align 4
  %i15 = getelementptr inbounds %struct.MAIN_Param, ptr %i3, i64 0, i32 4
  %i16 = load ptr, ptr %i15, align 8
  %i17 = icmp eq ptr %i16, null
  %i18 = select i1 %i17, ptr @.str.6.1, ptr %i16
  %i19 = zext i32 %i14 to i64
  %i20 = getelementptr inbounds [3 x [32 x i8]], ptr %i2, i64 0, i64 %i19, i64 0
  %i21 = zext i32 %i12 to i64
  %i22 = getelementptr inbounds [3 x [32 x i8]], ptr %i, i64 0, i64 %i21, i64 0
  call void (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str.5.2, i32 100, i32 100, i32 130, double 0x3FF4CCCCCCCCCCCC, i32 %i8, ptr %i10, ptr nonnull %i22, ptr nonnull %i20, ptr %i18)
  call void @llvm.lifetime.end.p0(i64 96, ptr nonnull %i6)
  call void @llvm.lifetime.end.p0(i64 96, ptr nonnull %i5)
  call fastcc void @MAIN_initialize(ptr nonnull %i3)
  %i23 = icmp slt i32 %i8, 1
  %i24 = icmp slt i32 %i8, 1
  br i1 %i24, label %bb59, label %bb25

bb25:                                             ; preds = %bb
  %i26 = icmp eq i32 %i14, 1
  br i1 %i26, label %bb27, label %bb44

bb27:                                             ; preds = %bb41, %bb25
  %i28 = phi i32 [ %i42, %bb41 ], [ 1, %bb25 ]
  %i29 = load ptr, ptr @srcGrid, align 8
  %i30 = getelementptr inbounds [26000000 x double], ptr %i29, i64 0, i64 0
  call fastcc void @LBM_handleInOutFlow(ptr %i30)
  %i31 = load ptr, ptr @srcGrid, align 8
  %i32 = getelementptr inbounds [26000000 x double], ptr %i31, i64 0, i64 0
  %i33 = load ptr, ptr @dstGrid, align 8
  %i34 = getelementptr inbounds [26000000 x double], ptr %i33, i64 0, i64 0
  call fastcc void @LBM_performStreamCollideTRT(ptr %i32, ptr %i34)
  call fastcc void @LBM_swapGrids()
  %i35 = and i32 %i28, 63
  %i36 = icmp eq i32 %i35, 0
  br i1 %i36, label %bb37, label %bb41

bb37:                                             ; preds = %bb27
  call void (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str.9, i32 %i28)
  %i38 = load ptr, ptr @srcGrid, align 8
  %i39 = load ptr, ptr @srcGrid, align 8
  %i40 = getelementptr inbounds [26000000 x double], ptr %i39, i64 0, i64 0
  call fastcc void @LBM_showGridStatistics(ptr %i40)
  br label %bb41

bb41:                                             ; preds = %bb37, %bb27
  %i42 = add nuw nsw i32 %i28, 1
  %i43 = icmp sgt i32 %i42, %i8
  br i1 %i43, label %bb59, label %bb27

bb44:                                             ; preds = %bb56, %bb25
  %i45 = phi i32 [ %i57, %bb56 ], [ 1, %bb25 ]
  %i46 = load ptr, ptr @srcGrid, align 8
  %i47 = getelementptr inbounds [26000000 x double], ptr %i46, i64 0, i64 0
  %i48 = load ptr, ptr @dstGrid, align 8
  %i49 = getelementptr inbounds [26000000 x double], ptr %i48, i64 0, i64 0
  call fastcc void @LBM_performStreamCollideTRT(ptr %i47, ptr %i49)
  call fastcc void @LBM_swapGrids()
  %i50 = and i32 %i45, 63
  %i51 = icmp eq i32 %i50, 0
  br i1 %i51, label %bb52, label %bb56

bb52:                                             ; preds = %bb44
  call void (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str.9, i32 %i45)
  %i53 = load ptr, ptr @srcGrid, align 8
  %i54 = load ptr, ptr @srcGrid, align 8
  %i55 = getelementptr inbounds [26000000 x double], ptr %i54, i64 0, i64 0
  call fastcc void @LBM_showGridStatistics(ptr %i55)
  br label %bb56

bb56:                                             ; preds = %bb52, %bb44
  %i57 = add nuw nsw i32 %i45, 1
  %i58 = icmp sgt i32 %i57, %i8
  br i1 %i58, label %bb59, label %bb44

bb59:                                             ; preds = %bb56, %bb41, %bb
  %i60 = load ptr, ptr @srcGrid, align 8
  %i61 = getelementptr inbounds [26000000 x double], ptr %i60, i64 0, i64 0
  call fastcc void @LBM_showGridStatistics(ptr %i61)
  switch i32 %i12, label %bb68 [
    i32 1, label %bb62
    i32 2, label %bb65
  ]

bb62:                                             ; preds = %bb59
  %i63 = load ptr, ptr @srcGrid, align 8
  %i64 = getelementptr inbounds [26000000 x double], ptr %i63, i64 0, i64 0
  call fastcc void @LBM_compareVelocityField(ptr %i64, ptr %i10)
  br label %bb68

bb65:                                             ; preds = %bb59
  %i66 = load ptr, ptr @srcGrid, align 8
  %i67 = getelementptr inbounds [26000000 x double], ptr %i66, i64 0, i64 0
  call fastcc void @LBM_storeVelocityField(ptr %i67, ptr %i10)
  br label %bb68

bb68:                                             ; preds = %bb65, %bb62, %bb59
  call fastcc void @LBM_freeGrid(ptr @srcGrid)
  call fastcc void @LBM_freeGrid(ptr @dstGrid)
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %i3)
  ret i32 0
}

; Function Attrs: norecurse
define internal fastcc void @MAIN_parseCommandLine(i32 %arg, ptr nocapture readonly %arg1, ptr nocapture %arg2) unnamed_addr #1 {
bb:
  ret void
}

; Function Attrs: norecurse
define internal fastcc void @MAIN_initialize(ptr nocapture readonly %arg) unnamed_addr #1 {
bb:
  tail call fastcc void @LBM_allocateGrid(ptr @srcGrid)
  tail call fastcc void @LBM_allocateGrid(ptr @dstGrid)
  %i = load ptr, ptr @srcGrid, align 8
  %i1 = getelementptr inbounds [26000000 x double], ptr %i, i64 0, i64 0
  tail call fastcc void @LBM_initializeGrid(ptr %i1)
  %i2 = load ptr, ptr @dstGrid, align 8
  %i3 = getelementptr inbounds [26000000 x double], ptr %i2, i64 0, i64 0
  tail call fastcc void @LBM_initializeGrid(ptr %i3)
  %i4 = getelementptr inbounds %struct.MAIN_Param, ptr %arg, i64 0, i32 4
  %i5 = load ptr, ptr %i4, align 8
  %i6 = icmp eq ptr %i5, null
  br i1 %i6, label %bb13, label %bb7

bb7:                                              ; preds = %bb
  %i8 = load ptr, ptr @srcGrid, align 8
  %i9 = getelementptr inbounds [26000000 x double], ptr %i8, i64 0, i64 0
  tail call fastcc void @LBM_loadObstacleFile(ptr %i9, ptr nonnull %i5)
  %i10 = load ptr, ptr @dstGrid, align 8
  %i11 = getelementptr inbounds [26000000 x double], ptr %i10, i64 0, i64 0
  %i12 = load ptr, ptr %i4, align 8
  tail call fastcc void @LBM_loadObstacleFile(ptr %i11, ptr %i12)
  br label %bb13

bb13:                                             ; preds = %bb7, %bb
  %i14 = getelementptr inbounds %struct.MAIN_Param, ptr %arg, i64 0, i32 3
  %i15 = load i32, ptr %i14, align 4
  %i16 = icmp eq i32 %i15, 1
  %i17 = load ptr, ptr @srcGrid, align 8
  br i1 %i16, label %bb18, label %bb22

bb18:                                             ; preds = %bb13
  %i19 = getelementptr inbounds [26000000 x double], ptr %i17, i64 0, i64 0
  tail call fastcc void @LBM_initializeSpecialCellsForChannel(ptr %i19)
  %i20 = load ptr, ptr @dstGrid, align 8
  %i21 = getelementptr inbounds [26000000 x double], ptr %i20, i64 0, i64 0
  tail call fastcc void @LBM_initializeSpecialCellsForChannel(ptr %i21)
  br label %bb26

bb22:                                             ; preds = %bb13
  %i23 = getelementptr inbounds [26000000 x double], ptr %i17, i64 0, i64 0
  tail call fastcc void @LBM_initializeSpecialCellsForLDC(ptr %i23)
  %i24 = load ptr, ptr @dstGrid, align 8
  %i25 = getelementptr inbounds [26000000 x double], ptr %i24, i64 0, i64 0
  tail call fastcc void @LBM_initializeSpecialCellsForLDC(ptr %i25)
  br label %bb26

bb26:                                             ; preds = %bb22, %bb18
  %i27 = load ptr, ptr @srcGrid, align 8
  %i28 = getelementptr inbounds [26000000 x double], ptr %i27, i64 0, i64 0
  tail call fastcc void @LBM_showGridStatistics(ptr %i28)
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

attributes #0 = { nocallback nofree nosync nounwind willreturn }
attributes #1 = { norecurse }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { argmemonly nofree nounwind willreturn }
; end INTEL_FEATURE_SW_ADVANCED
