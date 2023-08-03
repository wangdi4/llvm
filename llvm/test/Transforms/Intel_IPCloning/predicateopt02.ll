; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt -passes='module(ip-cloning)' -ip-manyreccalls-predicateopt -debug-only=ipcloning -S < %s 2>&1 | FileCheck %s

; Check that predicate opt does not seg fault even though we cannot determine
; the innnermost loop nest.

; CHECK: MRC Cloning: OK: GetVirtualPixelsFromNexus
; CHECK: Selected many recursive calls predicate opt
; CHECK: MRC Predicate Opt: Could not find spine

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS18_MagickPixelPacket._MagickPixelPacket = type { i32, i32, i32, double, i64, float, float, float, float, float }
%struct._ZTS12_PixelPacket._PixelPacket = type { i16, i16, i16, i16 }
%struct._ZTS6_Image._Image = type { i32, i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, ptr, %struct._ZTS12_PixelPacket._PixelPacket, %struct._ZTS12_PixelPacket._PixelPacket, %struct._ZTS12_PixelPacket._PixelPacket, double, %struct._ZTS17_ChromaticityInfo._ChromaticityInfo, i32, ptr, i32, ptr, ptr, ptr, i64, double, double, %struct._ZTS14_RectangleInfo._RectangleInfo, %struct._ZTS14_RectangleInfo._RectangleInfo, %struct._ZTS14_RectangleInfo._RectangleInfo, double, double, double, i32, i32, i32, i32, i32, i32, ptr, i64, i64, i64, i64, i64, i64, %struct._ZTS10_ErrorInfo._ErrorInfo, %struct._ZTS10_TimerInfo._TimerInfo, ptr, ptr, ptr, ptr, ptr, ptr, [4096 x i8], [4096 x i8], [4096 x i8], i64, i64, %struct._ZTS14_ExceptionInfo._ExceptionInfo, i32, i64, ptr, %struct._ZTS12_ProfileInfo._ProfileInfo, %struct._ZTS12_ProfileInfo._ProfileInfo, ptr, i64, i64, ptr, ptr, ptr, i32, i32, %struct._ZTS12_PixelPacket._PixelPacket, ptr, %struct._ZTS14_RectangleInfo._RectangleInfo, ptr, ptr, i32, i32, i64, i32, i64, i64, i32, i64 }
%struct._ZTS17_ChromaticityInfo._ChromaticityInfo = type { %struct._ZTS12_PrimaryInfo._PrimaryInfo, %struct._ZTS12_PrimaryInfo._PrimaryInfo, %struct._ZTS12_PrimaryInfo._PrimaryInfo, %struct._ZTS12_PrimaryInfo._PrimaryInfo }
%struct._ZTS12_PrimaryInfo._PrimaryInfo = type { double, double, double }
%struct._ZTS10_ErrorInfo._ErrorInfo = type { double, double, double }
%struct._ZTS10_TimerInfo._TimerInfo = type { %struct._ZTS6_Timer._Timer, %struct._ZTS6_Timer._Timer, i32, i64 }
%struct._ZTS6_Timer._Timer = type { double, double, double }
%struct._ZTS14_ExceptionInfo._ExceptionInfo = type { i32, i32, ptr, ptr, ptr, i32, ptr, i64 }
%struct._ZTS12_ProfileInfo._ProfileInfo = type { ptr, i64, ptr, i64 }
%struct._ZTS14_RectangleInfo._RectangleInfo = type { i64, i64, i64, i64 }
%struct._ZTS10_CacheView._CacheView = type { ptr, i32, i64, ptr, i32, i64 }
%struct._ZTS10_CacheInfo._CacheInfo = type { i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, i32, %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, i64, ptr, ptr, ptr, i32, i32, [4096 x i8], [4096 x i8], %struct._ZTS13_CacheMethods._CacheMethods, ptr, i64, ptr, i32, i32, i32, i64, ptr, ptr, i64, i64 }
%struct._ZTS13_CacheMethods._CacheMethods = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct._ZTS10_NexusInfo._NexusInfo = type { i32, %struct._ZTS14_RectangleInfo._RectangleInfo, i64, ptr, ptr, i32, ptr, i64 }
%struct._ZTS11_RandomInfo._RandomInfo = type { ptr, ptr, ptr, i64, [4 x i64], double, i64, i16, i16, ptr, i64, i64 }
%struct._ZTS13SemaphoreInfo.SemaphoreInfo = type { i64, i32, i64, i64 }
%struct._ZTS12_Ascii85Info._Ascii85Info = type { i64, i64, [10 x i8] }
%struct._ZTS9_BlobInfo._BlobInfo = type { i64, i64, i64, i32, i32, i64, i64, i32, i32, i32, i32, i32, %union._ZTS8FileInfo.FileInfo, %struct._ZTS4stat.stat, ptr, ptr, i32, ptr, i64, i64 }
%union._ZTS8FileInfo.FileInfo = type { ptr }
%struct._ZTS4stat.stat = type { i64, i64, i64, i32, i32, i32, i32, i64, i64, i64, i64, %struct._ZTS8timespec.timespec, %struct._ZTS8timespec.timespec, %struct._ZTS8timespec.timespec, [3 x i64] }
%struct._ZTS8timespec.timespec = type { i64, i64 }

@.str.137 = private unnamed_addr constant [15 x i8] c"magick/cache.c\00", align 1
@.str.1.138 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str.2.139 = private unnamed_addr constant [23 x i8] c"MemoryAllocationFailed\00", align 1
@.str.3.140 = private unnamed_addr constant [5 x i8] c"`%s'\00", align 1
@.str.17.1497 = private unnamed_addr constant [16 x i8] c"MeanShift/Image\00", align 1
@.str.18.1485 = private unnamed_addr constant [6 x i8] c"%s/%s\00", align 1
@DitherMatrix = internal unnamed_addr constant [64 x i64] [i64 0, i64 48, i64 12, i64 60, i64 3, i64 51, i64 15, i64 63, i64 32, i64 16, i64 44, i64 28, i64 35, i64 19, i64 47, i64 31, i64 8, i64 56, i64 4, i64 52, i64 11, i64 59, i64 7, i64 55, i64 40, i64 24, i64 36, i64 20, i64 43, i64 27, i64 39, i64 23, i64 2, i64 50, i64 14, i64 62, i64 1, i64 49, i64 13, i64 61, i64 34, i64 18, i64 46, i64 30, i64 33, i64 17, i64 45, i64 29, i64 10, i64 58, i64 6, i64 54, i64 9, i64 57, i64 5, i64 53, i64 42, i64 26, i64 38, i64 22, i64 41, i64 25, i64 37, i64 21], align 16, !intel_dtrans_type !0

declare dso_local void @__intel_new_feature_proc_init(i32, i64)
declare void @dummy()

; Function Attrs: mustprogress nofree nounwind willreturn allocsize(0,1) memory(write, inaccessiblemem: readwrite) uwtable
declare "intel_dtrans_func_index"="1" ptr @AcquireAlignedMemory(i64 noundef, i64 noundef) #0

; Function Attrs: nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @AcquireAuthenticCacheView(ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3") #1

; Function Attrs: nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @AcquirePixelCacheNexus(i64 noundef) #1

; Function Attrs: nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @AcquireRandomInfo() #1

; Function Attrs: nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @AcquireVirtualCacheView(ptr noundef "intel_dtrans_func_index"="2", ptr nocapture readnone "intel_dtrans_func_index"="3") #1

; Function Attrs: nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @CloneImage(ptr noundef "intel_dtrans_func_index"="2", i64 noundef, i64 noundef, i32 noundef, ptr noundef "intel_dtrans_func_index"="3") #1

; Function Attrs: nounwind uwtable
declare void @DefaultFatalErrorHandler(i32 noundef, ptr noundef "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") #1

; Function Attrs: nounwind uwtable
declare void @DefaultWarningHandler(i32, ptr noundef "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") #1

; Function Attrs: nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @DestroyCacheView(ptr noundef "intel_dtrans_func_index"="2") #1

; Function Attrs: nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @DestroyImage(ptr noundef "intel_dtrans_func_index"="2") #1

; Function Attrs: nofree nounwind uwtable
declare i64 @FormatLocaleString(ptr noalias nocapture noundef "intel_dtrans_func_index"="1", i64 noundef, ptr noalias nocapture noundef readonly "intel_dtrans_func_index"="2", ...) #2

; Function Attrs: hot nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @GetCacheViewAuthenticPixels(ptr nocapture noundef readonly "intel_dtrans_func_index"="2", i64 noundef, i64 noundef, i64 noundef, i64 noundef, ptr noundef "intel_dtrans_func_index"="3") #3

; Function Attrs: nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @GetCacheViewVirtualIndexQueue(ptr nocapture noundef readonly "intel_dtrans_func_index"="2") #1

; Function Attrs: hot nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @GetCacheViewVirtualPixels(ptr nocapture noundef readonly "intel_dtrans_func_index"="2", i64 noundef, i64 noundef, i64 noundef, i64 noundef, ptr noundef "intel_dtrans_func_index"="3") #3

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable
declare void @GetMagickPixelPacket(ptr noundef readonly "intel_dtrans_func_index"="1", ptr nocapture noundef writeonly "intel_dtrans_func_index"="2") #4

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
declare double @GetPseudoRandomValue(ptr nocapture noundef "intel_dtrans_func_index"="1") #5

; Function Attrs: nounwind uwtable
declare void @InheritException(ptr nocapture noundef "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2") #1

; Function Attrs: nounwind uwtable
declare fastcc i32 @ReadPixelCacheIndexes(ptr noalias noundef "intel_dtrans_func_index"="1", ptr noalias nocapture noundef readonly "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3") unnamed_addr #1

; Function Attrs: nounwind uwtable
declare fastcc i32 @ReadPixelCachePixels(ptr noalias noundef "intel_dtrans_func_index"="1", ptr noalias nocapture noundef readonly "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3") unnamed_addr #1

; Function Attrs: mustprogress nounwind willreturn uwtable
declare noalias "intel_dtrans_func_index"="1" ptr @RelinquishAlignedMemory(ptr noundef readonly "intel_dtrans_func_index"="2") #6

; Function Attrs: nounwind uwtable
declare "intel_dtrans_func_index"="1" ptr @RelinquishMagickMemory(ptr noundef "intel_dtrans_func_index"="2") #1

; Function Attrs: nounwind uwtable
declare i32 @SetImageStorageClass(ptr noundef "intel_dtrans_func_index"="1", i32 noundef) #1

; Function Attrs: hot nounwind uwtable
declare i32 @SyncCacheViewAuthenticPixels(ptr noalias nocapture noundef readonly "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") #3

; Function Attrs: nounwind uwtable
declare i32 @ThrowMagickException(ptr noundef "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", i64 noundef, i32 noundef, ptr noundef "intel_dtrans_func_index"="4", ptr noundef "intel_dtrans_func_index"="5", ...) #1

; Function Attrs: nounwind uwtable
declare i64 @analyzeImage(ptr nocapture noundef readonly "intel_dtrans_func_index"="1", i32 noundef, ptr nocapture noundef readnone "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3") #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.experimental.noalias.scope.decl(metadata) #7

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #8

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #8

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #9

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.rint.f64(double) #10

; Function Attrs: nounwind uwtable
define internal "intel_dtrans_func_index"="1" ptr @MeanShiftImage(ptr noundef "intel_dtrans_func_index"="2" %arg, i64 noundef %arg1, i64 noundef %arg2, double noundef %arg3, ptr noundef "intel_dtrans_func_index"="3" %arg4) #1 !intel.dtrans.func.type !42 {
bb:
  %i = alloca [4096 x i8], align 16, !intel_dtrans_type !13
  %i5 = alloca %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, align 8
  %i6 = alloca %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, align 8
  %i7 = alloca %struct._ZTS12_PixelPacket._PixelPacket, align 2
  %i8 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 59, !intel-tbaa !44
  %i9 = load i32, ptr %i8, align 8, !tbaa !44
  %i10 = icmp eq i32 %i9, 0
  br i1 %i10, label %bb14, label %bb11

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 53, !intel-tbaa !73
  %i13 = getelementptr inbounds [4096 x i8], ptr %i12, i64 0, i64 0
  br label %bb14

bb14:                                             ; preds = %bb11, %bb
  %i15 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 7, !intel-tbaa !74
  %i16 = load i64, ptr %i15, align 8, !tbaa !74
  %i17 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 8, !intel-tbaa !75
  %i18 = load i64, ptr %i17, align 8, !tbaa !75
  %i19 = tail call ptr @CloneImage(ptr noundef nonnull %arg, i64 noundef %i16, i64 noundef %i18, i32 noundef 1, ptr noundef %arg4) #11
  %i20 = icmp eq ptr %i19, null
  br i1 %i20, label %bb301, label %bb21

bb21:                                             ; preds = %bb14
  %i22 = tail call i32 @SetImageStorageClass(ptr noundef nonnull %i19, i32 noundef 1) #11
  %i23 = icmp eq i32 %i22, 0
  br i1 %i23, label %bb24, label %bb27

bb24:                                             ; preds = %bb21
  %i25 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %i19, i64 0, i32 58, !intel-tbaa !76
  tail call void @InheritException(ptr noundef %arg4, ptr noundef nonnull %i25) #11
  %i26 = tail call ptr @DestroyImage(ptr noundef nonnull %i19) #11
  br label %bb301

bb27:                                             ; preds = %bb21
  %i28 = tail call ptr @AcquireVirtualCacheView(ptr noundef nonnull %arg, ptr noundef %arg4) #11
  %i29 = tail call ptr @AcquireVirtualCacheView(ptr noundef nonnull %arg, ptr noundef %arg4) #11
  %i30 = tail call ptr @AcquireAuthenticCacheView(ptr noundef nonnull %i19, ptr noundef %arg4) #11
  %i31 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %i19, i64 0, i32 8, !intel-tbaa !75
  %i32 = load i64, ptr %i31, align 8, !tbaa !75
  %i33 = icmp sgt i64 %i32, 0
  br i1 %i33, label %bb34, label %bb297

bb34:                                             ; preds = %bb27
  %i35 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %i19, i64 0, i32 7
  %i36 = getelementptr i8, ptr %arg, i64 4
  %i37 = getelementptr inbounds %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, ptr %i5, i64 0, i32 5
  %i38 = getelementptr inbounds %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, ptr %i5, i64 0, i32 6
  %i39 = getelementptr inbounds %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, ptr %i5, i64 0, i32 7
  %i40 = getelementptr inbounds %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, ptr %i5, i64 0, i32 8
  %i41 = getelementptr inbounds %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, ptr %i5, i64 0, i32 9
  %i42 = sdiv i64 %arg2, 2
  %i43 = sub nsw i64 0, %i42
  %i44 = icmp slt i64 %i42, 0
  %i45 = sdiv i64 %arg1, 2
  %i46 = sub nsw i64 0, %i45
  %i47 = icmp slt i64 %i45, 0
  %i48 = lshr i64 %arg1, 1
  %i49 = lshr i64 %arg2, 1
  %i50 = mul i64 %i49, %i48
  %i51 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i7, i64 0, i32 2
  %i52 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i7, i64 0, i32 1
  %i53 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i7, i64 0, i32 0
  %i54 = fmul fast double %arg3, %arg3
  %i55 = getelementptr inbounds %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, ptr %i6, i64 0, i32 5
  %i56 = getelementptr inbounds %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, ptr %i6, i64 0, i32 6
  %i57 = getelementptr inbounds %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, ptr %i6, i64 0, i32 7
  %i58 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i7, i64 0, i32 3
  %i59 = getelementptr inbounds %struct._ZTS18_MagickPixelPacket._MagickPixelPacket, ptr %i6, i64 0, i32 8
  %i60 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 47
  %i61 = getelementptr inbounds [4096 x i8], ptr %i, i64 0, i64 0
  %i62 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 53
  %i63 = getelementptr inbounds [4096 x i8], ptr %i62, i64 0, i64 0
  %i64 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 48
  br label %bb65

bb65:                                             ; preds = %bb291, %bb34
  %i66 = phi i32 [ 1, %bb34 ], [ %i293, %bb291 ]
  %i67 = phi i64 [ 0, %bb34 ], [ %i292, %bb291 ]
  %i68 = phi i64 [ 0, %bb34 ], [ %i294, %bb291 ]
  %i69 = icmp eq i32 %i66, 0
  br i1 %i69, label %bb291, label %bb70

bb70:                                             ; preds = %bb65
  %i71 = load i64, ptr %i15, align 8, !tbaa !74
  %i72 = call ptr @GetCacheViewVirtualPixels(ptr noundef %i28, i64 noundef 0, i64 noundef %i68, i64 noundef %i71, i64 noundef 1, ptr noundef %arg4) #12
  %i73 = load i64, ptr %i35, align 8, !tbaa !74
  %i74 = call ptr @GetCacheViewAuthenticPixels(ptr noundef %i30, i64 noundef 0, i64 noundef %i68, i64 noundef %i73, i64 noundef 1, ptr noundef %arg4) #12
  %i75 = icmp eq ptr %i72, null
  %i76 = icmp eq ptr %i74, null
  %i77 = select i1 %i75, i1 true, i1 %i76
  br i1 %i77, label %bb291, label %bb78

bb78:                                             ; preds = %bb70
  %i79 = call ptr @GetCacheViewVirtualIndexQueue(ptr noundef %i28) #11
  %i80 = load i64, ptr %i35, align 8, !tbaa !74
  %i81 = icmp sgt i64 %i80, 0
  br i1 %i81, label %bb82, label %bb275

bb82:                                             ; preds = %bb78
  %i83 = icmp ne ptr %i79, null
  %i84 = sitofp i64 %i68 to double
  br label %bb85

bb85:                                             ; preds = %bb242, %bb82
  %i86 = phi i32 [ 1, %bb82 ], [ %i195, %bb242 ]
  %i87 = phi i64 [ 0, %bb82 ], [ %i272, %bb242 ]
  %i88 = phi ptr [ %i74, %bb82 ], [ %i271, %bb242 ]
  %i89 = phi ptr [ %i72, %bb82 ], [ %i270, %bb242 ]
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %i5) #11
  call void @GetMagickPixelPacket(ptr noundef %arg, ptr noundef nonnull %i5) #11
  %i90 = load i32, ptr %i36, align 4, !tbaa !77
  %i91 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i89, i64 0, i32 2, !intel-tbaa !78
  %i92 = load i16, ptr %i91, align 2, !tbaa !78
  %i93 = uitofp i16 %i92 to float
  store float %i93, ptr %i37, align 8, !tbaa !79
  %i94 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i89, i64 0, i32 1, !intel-tbaa !82
  %i95 = load i16, ptr %i94, align 2, !tbaa !82
  %i96 = uitofp i16 %i95 to float
  store float %i96, ptr %i38, align 4, !tbaa !83
  %i97 = load i16, ptr %i89, align 2, !tbaa !84
  %i98 = uitofp i16 %i97 to float
  store float %i98, ptr %i39, align 8, !tbaa !85
  %i99 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i89, i64 0, i32 3, !intel-tbaa !86
  %i100 = load i16, ptr %i99, align 2, !tbaa !86
  %i101 = uitofp i16 %i100 to float
  store float %i101, ptr %i40, align 4, !tbaa !87
  %i102 = icmp eq i32 %i90, 12
  %i103 = and i1 %i102, %i83
  br i1 %i103, label %bb104, label %bb108

bb104:                                            ; preds = %bb85
  %i105 = getelementptr inbounds i16, ptr %i79, i64 %i87, !intel-tbaa !88
  %i106 = load i16, ptr %i105, align 2, !tbaa !88
  %i107 = uitofp i16 %i106 to float
  store float %i107, ptr %i41, align 8, !tbaa !89
  br label %bb108

bb108:                                            ; preds = %bb104, %bb85
  %i109 = sitofp i64 %i87 to double
  br label %bb110

bb110:                                            ; preds = %bb239, %bb108
  %i111 = phi i64 [ 0, %bb108 ], [ %i240, %bb239 ]
  %i112 = phi double [ %i109, %bb108 ], [ %i201, %bb239 ]
  %i113 = phi double [ %i84, %bb108 ], [ %i202, %bb239 ]
  %i114 = phi i32 [ %i86, %bb108 ], [ %i195, %bb239 ]
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %i6) #11
  call void @GetMagickPixelPacket(ptr noundef %arg, ptr noundef nonnull %i6) #11
  %i115 = load float, ptr %i37, align 8, !tbaa.struct !90
  %i116 = load float, ptr %i38, align 4, !tbaa.struct !92
  %i117 = load float, ptr %i39, align 8, !tbaa.struct !93
  br i1 %i44, label %bb194, label %bb118

bb118:                                            ; preds = %bb110
  %i119 = call fast double @llvm.rint.f64(double %i112)
  %i120 = fptosi double %i119 to i64
  %i121 = call fast double @llvm.rint.f64(double %i113)
  %i122 = fptosi double %i121 to i64
  br i1 %i47, label %bb194, label %bb123

bb123:                                            ; preds = %bb191, %bb118
  %i124 = phi i64 [ %i192, %bb191 ], [ %i43, %bb118 ]
  %i125 = phi i64 [ %i188, %bb191 ], [ 0, %bb118 ]
  %i126 = phi double [ %i187, %bb191 ], [ 0.000000e+00, %bb118 ]
  %i127 = phi double [ %i186, %bb191 ], [ 0.000000e+00, %bb118 ]
  %i128 = phi i32 [ %i185, %bb191 ], [ %i114, %bb118 ]
  %i129 = mul nsw i64 %i124, %i124
  %i130 = add i64 %i124, %i122
  %i131 = sitofp i64 %i124 to double
  %i132 = fadd fast double %i113, %i131
  br label %bb133

bb133:                                            ; preds = %bb184, %bb123
  %i134 = phi i64 [ %i46, %bb123 ], [ %i189, %bb184 ]
  %i135 = phi i64 [ %i125, %bb123 ], [ %i188, %bb184 ]
  %i136 = phi double [ %i126, %bb123 ], [ %i187, %bb184 ]
  %i137 = phi double [ %i127, %bb123 ], [ %i186, %bb184 ]
  %i138 = phi i32 [ %i128, %bb123 ], [ %i185, %bb184 ]
  %i139 = mul nsw i64 %i134, %i134
  %i140 = add nuw nsw i64 %i139, %i129
  %i141 = icmp sgt i64 %i140, %i50
  br i1 %i141, label %bb184, label %bb142

bb142:                                            ; preds = %bb133
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i7) #11
  %i143 = add i64 %i134, %i120
  %i144 = call i32 @GetOneCacheViewVirtualPixel(ptr noundef %i29, i64 noundef %i143, i64 noundef %i130, ptr noundef nonnull %i7, ptr noundef %arg4) #11
  %i145 = load float, ptr %i37, align 8, !tbaa !79
  %i146 = load i16, ptr %i51, align 2, !tbaa !78
  %i147 = uitofp i16 %i146 to float
  %i148 = fsub fast float %i145, %i147
  %i149 = fmul fast float %i148, %i148
  %i150 = load float, ptr %i38, align 4, !tbaa !83
  %i151 = load i16, ptr %i52, align 2, !tbaa !82
  %i152 = uitofp i16 %i151 to float
  %i153 = fsub fast float %i150, %i152
  %i154 = fmul fast float %i153, %i153
  %i155 = fadd fast float %i154, %i149
  %i156 = load float, ptr %i39, align 8, !tbaa !85
  %i157 = load i16, ptr %i53, align 2, !tbaa !84
  %i158 = uitofp i16 %i157 to float
  %i159 = fsub fast float %i156, %i158
  %i160 = fmul fast float %i159, %i159
  %i161 = fadd fast float %i155, %i160
  %i162 = fpext float %i161 to double
  %i163 = fcmp fast ult double %i54, %i162
  br i1 %i163, label %bb180, label %bb164

bb164:                                            ; preds = %bb142
  %i165 = sitofp i64 %i134 to double
  %i166 = fadd fast double %i136, %i112
  %i167 = fadd fast double %i166, %i165
  %i168 = fadd fast double %i132, %i137
  %i169 = load float, ptr %i55, align 8, !tbaa !79
  %i170 = fadd fast float %i169, %i147
  store float %i170, ptr %i55, align 8, !tbaa !79
  %i171 = load float, ptr %i56, align 4, !tbaa !83
  %i172 = fadd fast float %i171, %i152
  store float %i172, ptr %i56, align 4, !tbaa !83
  %i173 = load float, ptr %i57, align 8, !tbaa !85
  %i174 = fadd fast float %i173, %i158
  store float %i174, ptr %i57, align 8, !tbaa !85
  %i175 = load i16, ptr %i58, align 2, !tbaa !86
  %i176 = uitofp i16 %i175 to float
  %i177 = load float, ptr %i59, align 4, !tbaa !87
  %i178 = fadd fast float %i177, %i176
  store float %i178, ptr %i59, align 4, !tbaa !87
  %i179 = add nsw i64 %i135, 1
  br label %bb180

bb180:                                            ; preds = %bb164, %bb142
  %i181 = phi double [ %i168, %bb164 ], [ %i137, %bb142 ]
  %i182 = phi double [ %i167, %bb164 ], [ %i136, %bb142 ]
  %i183 = phi i64 [ %i179, %bb164 ], [ %i135, %bb142 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i7) #11
  br label %bb184

bb184:                                            ; preds = %bb180, %bb133
  %i185 = phi i32 [ %i144, %bb180 ], [ %i138, %bb133 ]
  %i186 = phi double [ %i181, %bb180 ], [ %i137, %bb133 ]
  %i187 = phi double [ %i182, %bb180 ], [ %i136, %bb133 ]
  %i188 = phi i64 [ %i183, %bb180 ], [ %i135, %bb133 ]
  %i189 = add nsw i64 %i134, 1
  %i190 = icmp sgt i64 %i189, %i45
  br i1 %i190, label %bb191, label %bb133, !llvm.loop !94

bb191:                                            ; preds = %bb184
  %i192 = add nsw i64 %i124, 1
  %i193 = icmp sgt i64 %i192, %i42
  br i1 %i193, label %bb194, label %bb123, !llvm.loop !96

bb194:                                            ; preds = %bb191, %bb118, %bb110
  %i195 = phi i32 [ %i114, %bb110 ], [ %i114, %bb118 ], [ %i185, %bb191 ]
  %i196 = phi double [ 0.000000e+00, %bb110 ], [ 0.000000e+00, %bb118 ], [ %i186, %bb191 ]
  %i197 = phi double [ 0.000000e+00, %bb110 ], [ 0.000000e+00, %bb118 ], [ %i187, %bb191 ]
  %i198 = phi i64 [ 0, %bb110 ], [ 0, %bb118 ], [ %i188, %bb191 ]
  %i199 = sitofp i64 %i198 to double
  %i200 = fdiv fast double 1.000000e+00, %i199
  %i201 = fmul fast double %i200, %i197
  %i202 = fmul fast double %i200, %i196
  %i203 = load float, ptr %i55, align 8, !tbaa !79
  %i204 = fpext float %i203 to double
  %i205 = fmul fast double %i200, %i204
  %i206 = fptrunc double %i205 to float
  store float %i206, ptr %i37, align 8, !tbaa !79
  %i207 = load float, ptr %i56, align 4, !tbaa !83
  %i208 = fpext float %i207 to double
  %i209 = fmul fast double %i200, %i208
  %i210 = fptrunc double %i209 to float
  store float %i210, ptr %i38, align 4, !tbaa !83
  %i211 = load float, ptr %i57, align 8, !tbaa !85
  %i212 = fpext float %i211 to double
  %i213 = fmul fast double %i200, %i212
  %i214 = fptrunc double %i213 to float
  store float %i214, ptr %i39, align 8, !tbaa !85
  %i215 = load float, ptr %i59, align 4, !tbaa !87
  %i216 = fpext float %i215 to double
  %i217 = fmul fast double %i200, %i216
  %i218 = fptrunc double %i217 to float
  store float %i218, ptr %i40, align 4, !tbaa !87
  %i219 = fsub fast double %i201, %i112
  %i220 = fmul fast double %i219, %i219
  %i221 = fsub fast double %i202, %i113
  %i222 = fmul fast double %i221, %i221
  %i223 = fsub fast float %i206, %i115
  %i224 = fpext float %i223 to double
  %i225 = fmul fast double %i224, %i224
  %i226 = fsub fast float %i210, %i116
  %i227 = fpext float %i226 to double
  %i228 = fmul fast double %i227, %i227
  %i229 = fsub fast float %i214, %i117
  %i230 = fpext float %i229 to double
  %i231 = fmul fast double %i230, %i230
  %i232 = fadd fast double %i228, %i225
  %i233 = fadd fast double %i232, %i231
  %i234 = fmul fast double %i233, 0x3EEFC05F809F40DF
  %i235 = fadd fast double %i220, %i222
  %i236 = fadd fast double %i235, %i234
  %i237 = fcmp fast ugt double %i236, 3.000000e+00
  br i1 %i237, label %bb239, label %bb238

bb238:                                            ; preds = %bb194
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %i6) #11
  br label %bb242

bb239:                                            ; preds = %bb194
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %i6) #11
  %i240 = add nuw nsw i64 %i111, 1
  %i241 = icmp eq i64 %i240, 100
  br i1 %i241, label %bb242, label %bb110, !llvm.loop !97

bb242:                                            ; preds = %bb239, %bb238
  %i243 = fcmp fast ugt float %i206, 0.000000e+00
  %i244 = fcmp fast ult float %i206, 6.553500e+04
  %i245 = fadd fast float %i206, 5.000000e-01
  %i246 = fptoui float %i245 to i16
  %i247 = select i1 %i244, i16 %i246, i16 -1
  %i248 = select i1 %i243, i16 %i247, i16 0
  %i249 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i88, i64 0, i32 2, !intel-tbaa !78
  store i16 %i248, ptr %i249, align 2, !tbaa !78, !alias.scope !98, !noalias !101
  %i250 = fcmp fast ugt float %i210, 0.000000e+00
  %i251 = fcmp fast ult float %i210, 6.553500e+04
  %i252 = fadd fast float %i210, 5.000000e-01
  %i253 = fptoui float %i252 to i16
  %i254 = select i1 %i251, i16 %i253, i16 -1
  %i255 = select i1 %i250, i16 %i254, i16 0
  %i256 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i88, i64 0, i32 1, !intel-tbaa !82
  store i16 %i255, ptr %i256, align 2, !tbaa !82, !alias.scope !98, !noalias !101
  %i257 = fcmp fast ugt float %i214, 0.000000e+00
  %i258 = fcmp fast ult float %i214, 6.553500e+04
  %i259 = fadd fast float %i214, 5.000000e-01
  %i260 = fptoui float %i259 to i16
  %i261 = select i1 %i258, i16 %i260, i16 -1
  %i262 = select i1 %i257, i16 %i261, i16 0
  store i16 %i262, ptr %i88, align 2, !tbaa !84, !alias.scope !98, !noalias !101
  %i263 = fcmp fast ugt float %i218, 0.000000e+00
  %i264 = fcmp fast ult float %i218, 6.553500e+04
  %i265 = fadd fast float %i218, 5.000000e-01
  %i266 = fptoui float %i265 to i16
  %i267 = select i1 %i264, i16 %i266, i16 -1
  %i268 = select i1 %i263, i16 %i267, i16 0
  %i269 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i88, i64 0, i32 3, !intel-tbaa !86
  store i16 %i268, ptr %i269, align 2, !tbaa !86, !alias.scope !98, !noalias !101
  %i270 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i89, i64 1
  %i271 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i88, i64 1
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %i5) #11
  %i272 = add nuw nsw i64 %i87, 1
  %i273 = load i64, ptr %i35, align 8, !tbaa !74
  %i274 = icmp slt i64 %i272, %i273
  br i1 %i274, label %bb85, label %bb275, !llvm.loop !104

bb275:                                            ; preds = %bb242, %bb78
  %i276 = phi i32 [ 1, %bb78 ], [ %i195, %bb242 ]
  %i277 = call i32 @SyncCacheViewAuthenticPixels(ptr noundef %i30, ptr noundef %arg4) #12
  %i278 = icmp eq i32 %i277, 0
  %i279 = select i1 %i278, i32 0, i32 %i276
  %i280 = load ptr, ptr %i60, align 8, !tbaa !105
  %i281 = icmp eq ptr %i280, null
  br i1 %i281, label %bb291, label %bb282

bb282:                                            ; preds = %bb275
  %i283 = add nsw i64 %i67, 1
  %i284 = load i64, ptr %i17, align 8, !tbaa !75
  call void @llvm.lifetime.start.p0(i64 4096, ptr nonnull %i) #11
  %i285 = call i64 (ptr, i64, ptr, ...) @FormatLocaleString(ptr noundef nonnull %i61, i64 noundef 4096, ptr noundef nonnull @.str.18.1485, ptr noundef nonnull @.str.17.1497, ptr noundef nonnull %i63) #11
  %i286 = load ptr, ptr %i60, align 8, !tbaa !105
  %i287 = load ptr, ptr %i64, align 8, !tbaa !106
  %i288 = call i32 %i286(ptr noundef nonnull %i61, i64 noundef %i67, i64 noundef %i284, ptr noundef %i287) #11, !intel_dtrans_type !27
  call void @llvm.lifetime.end.p0(i64 4096, ptr nonnull %i) #11
  %i289 = icmp eq i32 %i288, 0
  %i290 = select i1 %i289, i32 0, i32 %i279
  br label %bb291

bb291:                                            ; preds = %bb282, %bb275, %bb70, %bb65
  %i292 = phi i64 [ %i67, %bb65 ], [ %i67, %bb70 ], [ %i67, %bb275 ], [ %i283, %bb282 ]
  %i293 = phi i32 [ 0, %bb65 ], [ 0, %bb70 ], [ %i279, %bb275 ], [ %i290, %bb282 ]
  %i294 = add nuw nsw i64 %i68, 1
  %i295 = load i64, ptr %i31, align 8, !tbaa !75
  %i296 = icmp slt i64 %i294, %i295
  br i1 %i296, label %bb65, label %bb297, !llvm.loop !107

bb297:                                            ; preds = %bb291, %bb27
  %i298 = call ptr @DestroyCacheView(ptr noundef %i30) #11
  %i299 = call ptr @DestroyCacheView(ptr noundef %i29) #11
  %i300 = call ptr @DestroyCacheView(ptr noundef %i28) #11
  br label %bb301

bb301:                                            ; preds = %bb297, %bb24, %bb14
  %i302 = phi ptr [ null, %bb24 ], [ %i19, %bb297 ], [ null, %bb14 ]
  ret ptr %i302
}

; Function Attrs: nounwind uwtable
define internal i32 @GetOneCacheViewVirtualPixel(ptr noalias nocapture noundef readonly "intel_dtrans_func_index"="1" %arg, i64 noundef %arg1, i64 noundef %arg2, ptr noalias nocapture noundef writeonly "intel_dtrans_func_index"="2" %arg3, ptr noundef "intel_dtrans_func_index"="3" %arg4) #1 !intel.dtrans.func.type !108 {
bb:
  call void @dummy()
  %i = getelementptr inbounds %struct._ZTS10_CacheView._CacheView, ptr %arg, i64 0, i32 0, !intel-tbaa !110
  %i5 = load ptr, ptr %i, align 8, !tbaa !110
  %i6 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %i5, i64 0, i32 12, !intel-tbaa !113
  %i7 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i6, i64 0, i32 0
  %i8 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %arg3, i64 0, i32 0
  %i9 = load i16, ptr %i7, align 2, !tbaa !88
  store i16 %i9, ptr %i8, align 2, !tbaa !88
  %i10 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i6, i64 0, i32 1
  %i11 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %arg3, i64 0, i32 1
  %i12 = load i16, ptr %i10, align 2, !tbaa !88
  store i16 %i12, ptr %i11, align 2, !tbaa !88
  %i13 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i6, i64 0, i32 2
  %i14 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %arg3, i64 0, i32 2
  %i15 = load i16, ptr %i13, align 2, !tbaa !88
  store i16 %i15, ptr %i14, align 2, !tbaa !88
  %i16 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i6, i64 0, i32 3
  %i17 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %arg3, i64 0, i32 3
  %i18 = load i16, ptr %i16, align 2, !tbaa !88
  store i16 %i18, ptr %i17, align 2, !tbaa !88
  %i19 = getelementptr inbounds %struct._ZTS10_CacheView._CacheView, ptr %arg, i64 0, i32 1, !intel-tbaa !114
  %i20 = load i32, ptr %i19, align 8, !tbaa !114
  %i21 = getelementptr inbounds %struct._ZTS10_CacheView._CacheView, ptr %arg, i64 0, i32 3, !intel-tbaa !115
  %i22 = load ptr, ptr %i21, align 8, !tbaa !115
  %i23 = load ptr, ptr %i22, align 8, !tbaa !116
  %i24 = tail call ptr @GetVirtualPixelsFromNexus(ptr noundef %i5, i32 noundef %i20, i64 noundef %arg1, i64 noundef %arg2, i64 noundef 1, i64 noundef 1, ptr noundef %i23, ptr noundef %arg4) #12
  %i25 = icmp eq ptr %i24, null
  br i1 %i25, label %bb35, label %bb26

bb26:                                             ; preds = %bb
  %i27 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i24, i64 0, i32 0
  %i28 = load i16, ptr %i27, align 2, !tbaa !88
  store i16 %i28, ptr %i8, align 2, !tbaa !88
  %i29 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i24, i64 0, i32 1
  %i30 = load i16, ptr %i29, align 2, !tbaa !88
  store i16 %i30, ptr %i11, align 2, !tbaa !88
  %i31 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i24, i64 0, i32 2
  %i32 = load i16, ptr %i31, align 2, !tbaa !88
  store i16 %i32, ptr %i14, align 2, !tbaa !88
  %i33 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i24, i64 0, i32 3
  %i34 = load i16, ptr %i33, align 2, !tbaa !88
  store i16 %i34, ptr %i17, align 2, !tbaa !88
  br label %bb35

bb35:                                             ; preds = %bb26, %bb
  %i36 = phi i32 [ 1, %bb26 ], [ 0, %bb ]
  ret i32 %i36
}

; Function Attrs: hot nounwind uwtable
define internal "intel_dtrans_func_index"="1" ptr @GetVirtualPixelsFromNexus(ptr noundef "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1, i64 noundef %arg2, i64 noundef %arg3, i64 noundef %arg4, i64 noundef %arg5, ptr nocapture noundef "intel_dtrans_func_index"="3" %arg6, ptr noundef "intel_dtrans_func_index"="4" %arg7) #3 !intel.dtrans.func.type !118 {
bb:
  %i = alloca i16, align 2
  %i8 = alloca %struct._ZTS12_PixelPacket._PixelPacket, align 2
  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %i) #11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i8) #11
  %i9 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 49, !intel-tbaa !120
  %i10 = load ptr, ptr %i9, align 8, !tbaa !120
  %i11 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 3, !intel-tbaa !121
  %i12 = load i32, ptr %i11, align 8, !tbaa !121, !alias.scope !132, !noalias !135
  %i13 = icmp eq i32 %i12, 0
  br i1 %i13, label %bb597, label %bb14

bb14:                                             ; preds = %bb
  %i15 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 38, !intel-tbaa !142
  %i16 = load ptr, ptr %i15, align 8, !tbaa !142
  %i17 = icmp eq ptr %i16, null
  br i1 %i17, label %bb18, label %bb22

bb18:                                             ; preds = %bb14
  %i19 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 73, !intel-tbaa !143
  %i20 = load ptr, ptr %i19, align 8, !tbaa !143
  %i21 = icmp ne ptr %i20, null
  br label %bb22

bb22:                                             ; preds = %bb18, %bb14
  %i23 = phi i1 [ true, %bb14 ], [ %i21, %bb18 ]
  %i24 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 1
  store i64 %arg4, ptr %i24, align 8, !tbaa.struct !144
  %i25 = getelementptr inbounds i8, ptr %i24, i64 8
  store i64 %arg5, ptr %i25, align 8, !tbaa.struct !146
  %i26 = getelementptr inbounds i8, ptr %i24, i64 16
  store i64 %arg2, ptr %i26, align 8, !tbaa.struct !147
  %i27 = getelementptr inbounds i8, ptr %i24, i64 24
  store i64 %arg3, ptr %i27, align 8, !tbaa.struct !148
  %i28 = load i32, ptr %i11, align 8, !tbaa !121
  %i29 = icmp eq i32 %i28, 1
  br i1 %i29, label %bb33, label %bb30

bb30:                                             ; preds = %bb22
  %i31 = icmp ne i32 %i28, 2
  %i32 = or i1 %i23, %i31
  br i1 %i32, label %bb72, label %bb34

bb33:                                             ; preds = %bb22
  br i1 %i23, label %bb72, label %bb34

bb34:                                             ; preds = %bb33, %bb30
  %i35 = add nsw i64 %arg5, %arg3
  %i36 = icmp sgt i64 %arg2, -1
  br i1 %i36, label %bb37, label %bb72

bb37:                                             ; preds = %bb34
  %i38 = add nsw i64 %arg4, %arg2
  %i39 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 6, !intel-tbaa !149
  %i40 = load i64, ptr %i39, align 8, !tbaa !149
  %i41 = icmp sle i64 %i38, %i40
  %i42 = icmp sgt i64 %arg3, -1
  %i43 = and i1 %i41, %i42
  br i1 %i43, label %bb44, label %bb72

bb44:                                             ; preds = %bb37
  %i45 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 7, !intel-tbaa !150
  %i46 = load i64, ptr %i45, align 8, !tbaa !150
  %i47 = icmp sgt i64 %i35, %i46
  br i1 %i47, label %bb72, label %bb48

bb48:                                             ; preds = %bb44
  %i49 = icmp eq i64 %arg5, 1
  br i1 %i49, label %bb57, label %bb50

bb50:                                             ; preds = %bb48
  %i51 = icmp eq i64 %arg2, 0
  br i1 %i51, label %bb52, label %bb72

bb52:                                             ; preds = %bb50
  %i53 = icmp eq i64 %i40, %arg4
  br i1 %i53, label %bb57, label %bb54

bb54:                                             ; preds = %bb52
  %i55 = urem i64 %arg4, %i40
  %i56 = icmp eq i64 %i55, 0
  br i1 %i56, label %bb57, label %bb72

bb57:                                             ; preds = %bb54, %bb52, %bb48
  %i58 = mul i64 %i40, %arg3
  %i59 = add i64 %i58, %arg2
  %i60 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 14, !intel-tbaa !151
  %i61 = load ptr, ptr %i60, align 8, !tbaa !151
  %i62 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i61, i64 %i59, !intel-tbaa !152
  %i63 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 4, !intel-tbaa !153
  store ptr %i62, ptr %i63, align 8, !tbaa !153
  %i64 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 6, !intel-tbaa !155
  store ptr null, ptr %i64, align 8, !tbaa !155
  %i65 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 16, !intel-tbaa !156
  %i66 = load i32, ptr %i65, align 8, !tbaa !156
  %i67 = icmp eq i32 %i66, 0
  br i1 %i67, label %bb154, label %bb68

bb68:                                             ; preds = %bb57
  %i69 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 15, !intel-tbaa !157
  %i70 = load ptr, ptr %i69, align 8, !tbaa !157
  %i71 = getelementptr inbounds i16, ptr %i70, i64 %i59, !intel-tbaa !88
  store ptr %i71, ptr %i64, align 8, !tbaa !155
  br label %bb154

bb72:                                             ; preds = %bb54, %bb50, %bb44, %bb37, %bb34, %bb33, %bb30
  %i73 = mul i64 %arg5, %arg4
  %i74 = shl i64 %i73, 3
  %i75 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 16, !intel-tbaa !156
  %i76 = load i32, ptr %i75, align 8, !tbaa !156
  %i77 = icmp eq i32 %i76, 0
  %i78 = mul i64 %i73, 10
  %i79 = select i1 %i77, i64 %i74, i64 %i78
  %i80 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 3, !intel-tbaa !158
  %i81 = load ptr, ptr %i80, align 8, !tbaa !158
  %i82 = icmp eq ptr %i81, null
  br i1 %i82, label %bb83, label %bb94

bb83:                                             ; preds = %bb72
  %i84 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 2, !intel-tbaa !159
  store i64 %i79, ptr %i84, align 8, !tbaa !159
  %i85 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 0, !intel-tbaa !160
  store i32 0, ptr %i85, align 8, !tbaa !160, !noalias !161
  %i86 = tail call ptr @AcquireAlignedMemory(i64 noundef 1, i64 noundef %i79) #13, !noalias !161
  store ptr %i86, ptr %i80, align 8, !tbaa !158, !noalias !161
  %i87 = icmp eq ptr %i86, null
  br i1 %i87, label %bb88, label %bb116

bb88:                                             ; preds = %bb83
  store i32 1, ptr %i85, align 8, !tbaa !160, !noalias !161
  %i89 = load i64, ptr %i84, align 8, !tbaa !159, !noalias !161
  store ptr null, ptr %i80, align 8, !tbaa !158, !noalias !161
  br label %bb90

bb90:                                             ; preds = %bb88
  %i91 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 18, !intel-tbaa !164
  %i92 = getelementptr inbounds [4096 x i8], ptr %i91, i64 0, i64 0
  %i93 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr noundef %arg7, ptr noundef nonnull @.str.137, ptr noundef nonnull @.str.1.138, i64 noundef 4688, i32 noundef 400, ptr noundef nonnull @.str.2.139, ptr noundef nonnull @.str.3.140, ptr noundef nonnull %i92) #11
  store i64 0, ptr %i84, align 8, !tbaa !159
  br label %bb597

bb94:                                             ; preds = %bb72
  %i95 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 2, !intel-tbaa !159
  %i96 = load i64, ptr %i95, align 8, !tbaa !159
  %i97 = icmp ult i64 %i96, %i79
  br i1 %i97, label %bb98, label %bb116

bb98:                                             ; preds = %bb94
  %i99 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 0, !intel-tbaa !160
  %i100 = load i32, ptr %i99, align 8, !tbaa !160
  %i101 = icmp eq i32 %i100, 0
  br i1 %i101, label %bb102, label %bb104

bb102:                                            ; preds = %bb98
  %i103 = tail call ptr @RelinquishAlignedMemory(ptr noundef nonnull %i81) #11
  br label %bb105

bb104:                                            ; preds = %bb98
  br label %bb105

bb105:                                            ; preds = %bb104, %bb102
  store ptr null, ptr %i80, align 8, !tbaa !158
  %i106 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 4, !intel-tbaa !153
  store ptr null, ptr %i106, align 8, !tbaa !153
  %i107 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 6, !intel-tbaa !155
  store ptr null, ptr %i107, align 8, !tbaa !155
  store i64 %i79, ptr %i95, align 8, !tbaa !159
  store i32 0, ptr %i99, align 8, !tbaa !160, !noalias !165
  %i108 = tail call ptr @AcquireAlignedMemory(i64 noundef 1, i64 noundef %i79) #13, !noalias !165
  store ptr %i108, ptr %i80, align 8, !tbaa !158, !noalias !165
  %i109 = icmp eq ptr %i108, null
  br i1 %i109, label %bb110, label %bb116

bb110:                                            ; preds = %bb105
  store i32 1, ptr %i99, align 8, !tbaa !160, !noalias !165
  %i111 = load i64, ptr %i95, align 8, !tbaa !159, !noalias !165
  store ptr null, ptr %i80, align 8, !tbaa !158, !noalias !165
  br label %bb112

bb112:                                            ; preds = %bb110
  %i113 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 18, !intel-tbaa !164
  %i114 = getelementptr inbounds [4096 x i8], ptr %i113, i64 0, i64 0
  %i115 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr noundef %arg7, ptr noundef nonnull @.str.137, ptr noundef nonnull @.str.1.138, i64 noundef 4688, i32 noundef 400, ptr noundef nonnull @.str.2.139, ptr noundef nonnull @.str.3.140, ptr noundef nonnull %i114) #11
  store i64 0, ptr %i95, align 8, !tbaa !159
  br label %bb597

bb116:                                            ; preds = %bb105, %bb94, %bb83
  %i117 = phi ptr [ %i108, %bb105 ], [ %i86, %bb83 ], [ %i81, %bb94 ]
  %i118 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 4, !intel-tbaa !153
  store ptr %i117, ptr %i118, align 8, !tbaa !153
  %i119 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 6, !intel-tbaa !155
  store ptr null, ptr %i119, align 8, !tbaa !155
  %i120 = load i32, ptr %i75, align 8, !tbaa !156
  %i121 = icmp eq i32 %i120, 0
  br i1 %i121, label %bb124, label %bb122

bb122:                                            ; preds = %bb116
  %i123 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i117, i64 %i73, !intel-tbaa !152
  store ptr %i123, ptr %i119, align 8, !tbaa !155
  br label %bb124

bb124:                                            ; preds = %bb122, %bb116
  %i125 = phi ptr [ %i123, %bb122 ], [ null, %bb116 ]
  tail call void @llvm.experimental.noalias.scope.decl(metadata !168)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !171)
  %i126 = load i32, ptr %i11, align 8, !tbaa !121, !alias.scope !168, !noalias !171
  %i127 = icmp eq i32 %i126, 4
  br i1 %i127, label %bb128, label %bb135

bb128:                                            ; preds = %bb124
  %i129 = load i64, ptr %i27, align 8, !tbaa !173
  %i130 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 6
  %i131 = load i64, ptr %i130, align 8, !tbaa !149, !alias.scope !132, !noalias !135
  %i132 = load i64, ptr %i26, align 8, !tbaa !174
  %i133 = mul i64 %i131, %i129
  %i134 = add i64 %i133, %i132
  br label %bb147

bb135:                                            ; preds = %bb124
  %i136 = load i64, ptr %i27, align 8, !tbaa !173, !alias.scope !171, !noalias !168
  %i137 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 6, !intel-tbaa !149
  %i138 = load i64, ptr %i137, align 8, !tbaa !149, !alias.scope !168, !noalias !171
  %i139 = mul i64 %i138, %i136
  %i140 = load i64, ptr %i26, align 8, !tbaa !174, !alias.scope !171, !noalias !168
  %i141 = add i64 %i139, %i140
  %i142 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 14, !intel-tbaa !151
  %i143 = load ptr, ptr %i142, align 8, !tbaa !151, !alias.scope !168, !noalias !171
  %i144 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i143, i64 %i141, !intel-tbaa !152
  %i145 = icmp eq ptr %i117, %i144
  %i146 = zext i1 %i145 to i32
  br label %bb147

bb147:                                            ; preds = %bb135, %bb128
  %i148 = phi i64 [ %i134, %bb128 ], [ %i141, %bb135 ]
  %i149 = phi i64 [ %i131, %bb128 ], [ %i138, %bb135 ]
  %i150 = phi i32 [ 1, %bb128 ], [ %i146, %bb135 ]
  %i151 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 5, !intel-tbaa !175
  store i32 %i150, ptr %i151, align 8, !tbaa !175
  %i152 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 7
  %i153 = load i64, ptr %i152, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  br label %bb158

bb154:                                            ; preds = %bb68, %bb57
  %i155 = phi ptr [ null, %bb57 ], [ %i71, %bb68 ]
  %i156 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %arg6, i64 0, i32 5, !intel-tbaa !175
  store i32 1, ptr %i156, align 8, !tbaa !175
  %i157 = icmp eq ptr %i61, null
  br i1 %i157, label %bb597, label %bb158

bb158:                                            ; preds = %bb154, %bb147
  %i159 = phi i64 [ %i148, %bb147 ], [ %i59, %bb154 ]
  %i160 = phi ptr [ %i125, %bb147 ], [ %i155, %bb154 ]
  %i161 = phi i32 [ %i150, %bb147 ], [ 1, %bb154 ]
  %i162 = phi i64 [ %i153, %bb147 ], [ %i46, %bb154 ]
  %i163 = phi i64 [ %i149, %bb147 ], [ %i40, %bb154 ]
  %i164 = phi ptr [ %i117, %bb147 ], [ %i62, %bb154 ]
  %i165 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 6, !intel-tbaa !149
  %i166 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 7, !intel-tbaa !150
  %i167 = icmp sgt i64 %i159, -1
  br i1 %i167, label %bb168, label %bb206

bb168:                                            ; preds = %bb158
  %i169 = mul i64 %i162, %i163
  %i170 = load i64, ptr %i25, align 8, !tbaa !176
  %i171 = add i64 %i170, -1
  %i172 = mul i64 %i171, %i163
  %i173 = getelementptr inbounds %struct._ZTS14_RectangleInfo._RectangleInfo, ptr %i24, i64 0, i32 0, !intel-tbaa !177
  %i174 = load i64, ptr %i173, align 8, !tbaa !178
  %i175 = add nsw i64 %i159, -1
  %i176 = add i64 %i175, %i174
  %i177 = add i64 %i176, %i172
  %i178 = icmp ult i64 %i177, %i169
  %i179 = icmp sgt i64 %arg2, -1
  %i180 = and i1 %i178, %i179
  br i1 %i180, label %bb181, label %bb206

bb181:                                            ; preds = %bb168
  %i182 = add i64 %arg4, %arg2
  %i183 = icmp sgt i64 %i182, %i163
  %i184 = icmp slt i64 %arg3, 0
  %i185 = or i1 %i183, %i184
  %i186 = add i64 %arg5, %arg3
  %i187 = icmp sgt i64 %i186, %i162
  %i188 = select i1 %i185, i1 true, i1 %i187
  br i1 %i188, label %bb206, label %bb189

bb189:                                            ; preds = %bb181
  %i190 = icmp eq i32 %i161, 0
  br i1 %i190, label %bb191, label %bb597

bb191:                                            ; preds = %bb189
  %i192 = tail call fastcc i32 @ReadPixelCachePixels(ptr noundef nonnull %i10, ptr noundef nonnull %arg6, ptr noundef %arg7), !intel.args.alias.scope !179
  %i193 = icmp eq i32 %i192, 0
  br i1 %i193, label %bb597, label %bb194

bb194:                                            ; preds = %bb191
  %i195 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 0, !intel-tbaa !180
  %i196 = load i32, ptr %i195, align 8, !tbaa !180, !alias.scope !132, !noalias !135
  %i197 = icmp eq i32 %i196, 2
  br i1 %i197, label %bb202, label %bb198

bb198:                                            ; preds = %bb194
  %i199 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 1, !intel-tbaa !181
  %i200 = load i32, ptr %i199, align 4, !tbaa !181, !alias.scope !132, !noalias !135
  %i201 = icmp eq i32 %i200, 12
  br i1 %i201, label %bb202, label %bb205

bb202:                                            ; preds = %bb198, %bb194
  %i203 = tail call fastcc i32 @ReadPixelCacheIndexes(ptr noundef nonnull %i10, ptr noundef nonnull %arg6, ptr noundef %arg7), !intel.args.alias.scope !179
  %i204 = icmp eq i32 %i203, 0
  br i1 %i204, label %bb597, label %bb205

bb205:                                            ; preds = %bb202, %bb198
  br label %bb597

bb206:                                            ; preds = %bb181, %bb168, %bb158
  %i207 = tail call ptr @AcquirePixelCacheNexus(i64 noundef 1)
  switch i32 %arg1, label %bb228 [
    i32 10, label %bb208
    i32 11, label %bb213
    i32 8, label %bb218
    i32 9, label %bb223
    i32 12, label %bb223
  ]

bb208:                                            ; preds = %bb206
  %i209 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 2, !intel-tbaa !78
  store i16 0, ptr %i209, align 2, !tbaa !78
  %i210 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 1, !intel-tbaa !82
  store i16 0, ptr %i210, align 2, !tbaa !82
  %i211 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 0, !intel-tbaa !84
  store i16 0, ptr %i211, align 2, !tbaa !84
  %i212 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 3, !intel-tbaa !86
  store i16 0, ptr %i212, align 2, !tbaa !86
  br label %bb242

bb213:                                            ; preds = %bb206
  %i214 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 2, !intel-tbaa !78
  store i16 32767, ptr %i214, align 2, !tbaa !78
  %i215 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 1, !intel-tbaa !82
  store i16 32767, ptr %i215, align 2, !tbaa !82
  %i216 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 0, !intel-tbaa !84
  store i16 32767, ptr %i216, align 2, !tbaa !84
  %i217 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 3, !intel-tbaa !86
  store i16 0, ptr %i217, align 2, !tbaa !86
  br label %bb242

bb218:                                            ; preds = %bb206
  %i219 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 2, !intel-tbaa !78
  store i16 0, ptr %i219, align 2, !tbaa !78
  %i220 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 1, !intel-tbaa !82
  store i16 0, ptr %i220, align 2, !tbaa !82
  %i221 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 0, !intel-tbaa !84
  store i16 0, ptr %i221, align 2, !tbaa !84
  %i222 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 3, !intel-tbaa !86
  store i16 -1, ptr %i222, align 2, !tbaa !86
  br label %bb242

bb223:                                            ; preds = %bb206, %bb206
  %i224 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 2, !intel-tbaa !78
  store i16 -1, ptr %i224, align 2, !tbaa !78
  %i225 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 1, !intel-tbaa !82
  store i16 -1, ptr %i225, align 2, !tbaa !82
  %i226 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 0, !intel-tbaa !84
  store i16 -1, ptr %i226, align 2, !tbaa !84
  %i227 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i8, i64 0, i32 3, !intel-tbaa !86
  store i16 0, ptr %i227, align 2, !tbaa !86
  br label %bb242

bb228:                                            ; preds = %bb206
  %i229 = getelementptr inbounds %struct._ZTS6_Image._Image, ptr %arg, i64 0, i32 12, !intel-tbaa !113
  %i230 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i229, i64 0, i32 0
  %i231 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i8, i64 0, i32 0
  %i232 = load i16, ptr %i230, align 2, !tbaa !88
  store i16 %i232, ptr %i231, align 2, !tbaa !88
  %i233 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i229, i64 0, i32 1
  %i234 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i8, i64 0, i32 1
  %i235 = load i16, ptr %i233, align 2, !tbaa !88
  store i16 %i235, ptr %i234, align 2, !tbaa !88
  %i236 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i229, i64 0, i32 2
  %i237 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i8, i64 0, i32 2
  %i238 = load i16, ptr %i236, align 2, !tbaa !88
  store i16 %i238, ptr %i237, align 2, !tbaa !88
  %i239 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i229, i64 0, i32 3
  %i240 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i8, i64 0, i32 3
  %i241 = load i16, ptr %i239, align 2, !tbaa !88
  store i16 %i241, ptr %i240, align 2, !tbaa !88
  br label %bb242

bb242:                                            ; preds = %bb228, %bb223, %bb218, %bb213, %bb208
  store i16 0, ptr %i, align 2, !tbaa !88
  %i243 = icmp sgt i64 %arg5, 0
  br i1 %i243, label %bb244, label %bb573

bb244:                                            ; preds = %bb242
  %i245 = and i32 %arg1, -5
  %i246 = icmp eq i32 %i245, 0
  %i247 = icmp sgt i64 %arg4, 0
  %i248 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 0
  %i249 = getelementptr inbounds %struct._ZTS10_CacheInfo._CacheInfo, ptr %i10, i64 0, i32 21
  br i1 %i247, label %bb250, label %bb573

bb250:                                            ; preds = %bb568, %bb244
  %i251 = phi ptr [ %i570, %bb568 ], [ %i160, %bb244 ]
  %i252 = phi ptr [ %i569, %bb568 ], [ %i164, %bb244 ]
  %i253 = phi i64 [ %i571, %bb568 ], [ 0, %bb244 ]
  %i254 = add nsw i64 %i253, %arg3
  br i1 %i246, label %bb255, label %bb262

bb255:                                            ; preds = %bb250
  %i256 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i257 = icmp slt i64 %i254, 0
  %i258 = icmp slt i64 %i254, %i256
  %i259 = add i64 %i256, -1
  %i260 = select i1 %i258, i64 %i254, i64 %i259
  %i261 = select i1 %i257, i64 0, i64 %i260
  br label %bb262

bb262:                                            ; preds = %bb255, %bb250
  %i263 = phi i64 [ %i261, %bb255 ], [ %i254, %bb250 ]
  %i264 = icmp slt i64 %i263, 0
  %i265 = ashr i64 %i263, 63
  %i266 = lshr i64 %i263, 63
  %i267 = and i64 %i263, 7
  %i268 = getelementptr inbounds [64 x i64], ptr @DitherMatrix, i64 0, i64 %i267
  %i269 = add i64 %i263, -32
  br label %bb270

bb270:                                            ; preds = %bb562, %bb262
  %i271 = phi ptr [ %i251, %bb262 ], [ %i564, %bb562 ]
  %i272 = phi ptr [ %i252, %bb262 ], [ %i563, %bb562 ]
  %i273 = phi i64 [ 0, %bb262 ], [ %i566, %bb562 ]
  %i274 = add nsw i64 %i273, %arg2
  %i275 = load i64, ptr %i165, align 8, !tbaa !149, !alias.scope !132, !noalias !135
  %i276 = sub i64 %i275, %i274
  %i277 = sub i64 %arg4, %i273
  %i278 = icmp ult i64 %i276, %i277
  %i279 = select i1 %i278, i64 %i276, i64 %i277
  %i280 = icmp slt i64 %i274, 0
  %i281 = icmp sle i64 %i275, %i274
  %i282 = select i1 %i280, i1 true, i1 %i281
  %i283 = select i1 %i282, i1 true, i1 %i264
  br i1 %i283, label %bb311, label %bb284

bb284:                                            ; preds = %bb270
  %i285 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i286 = icmp sge i64 %i263, %i285
  %i287 = icmp eq i64 %i279, 0
  %i288 = select i1 %i286, i1 true, i1 %i287
  br i1 %i288, label %bb311, label %bb289

bb289:                                            ; preds = %bb284
  %i290 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i291 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef %arg1, i64 noundef %i274, i64 noundef %i263, i64 noundef %i279, i64 noundef 1, ptr noundef %i290, ptr noundef %arg7) #14
  %i292 = icmp eq ptr %i291, null
  br i1 %i292, label %bb568, label %bb293

bb293:                                            ; preds = %bb289
  %i294 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !184, !noalias !187
  %i295 = icmp eq i32 %i294, 0
  br i1 %i295, label %bb308, label %bb296

bb296:                                            ; preds = %bb293
  %i297 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i298 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i297, i64 0, i32 6, !intel-tbaa !155
  %i299 = load ptr, ptr %i298, align 8, !tbaa !155, !noalias !132
  %i300 = shl i64 %i279, 3
  call void @llvm.memcpy.p0.p0.i64(ptr align 2 %i272, ptr nonnull align 2 %i291, i64 %i300, i1 false)
  %i301 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i272, i64 %i279, !intel-tbaa !152
  %i302 = icmp ne ptr %i271, null
  %i303 = icmp ne ptr %i299, null
  %i304 = select i1 %i302, i1 %i303, i1 false
  br i1 %i304, label %bb305, label %bb562

bb305:                                            ; preds = %bb296
  %i306 = shl i64 %i279, 1
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i271, ptr nonnull align 2 %i299, i64 %i306, i1 false)
  %i307 = getelementptr inbounds i16, ptr %i271, i64 %i279, !intel-tbaa !88
  br label %bb562

bb308:                                            ; preds = %bb293
  %i309 = shl i64 %i279, 3
  call void @llvm.memcpy.p0.p0.i64(ptr align 2 %i272, ptr nonnull align 2 %i291, i64 %i309, i1 false)
  %i310 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i272, i64 %i279, !intel-tbaa !152
  br label %bb562

bb311:                                            ; preds = %bb284, %bb270
  switch i32 %arg1, label %bb519 [
    i32 1, label %bb541
    i32 2, label %bb541
    i32 10, label %bb541
    i32 11, label %bb541
    i32 8, label %bb541
    i32 9, label %bb541
    i32 12, label %bb541
    i32 16, label %bb501
    i32 6, label %bb474
    i32 3, label %bb447
    i32 7, label %bb428
    i32 5, label %bb399
    i32 17, label %bb376
    i32 13, label %bb354
    i32 14, label %bb331
    i32 15, label %bb312
  ]

bb312:                                            ; preds = %bb311
  %i313 = sdiv i64 %i274, %i275
  %i314 = ashr i64 %i274, 63
  %i315 = add nsw i64 %i313, %i314
  %i316 = mul nsw i64 %i315, %i275
  %i317 = sub nsw i64 %i274, %i316
  %i318 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i319 = icmp slt i64 %i263, %i318
  %i320 = add i64 %i318, -1
  %i321 = select i1 %i319, i64 %i263, i64 %i320
  %i322 = select i1 %i264, i64 0, i64 %i321
  %i323 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i324 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 15, i64 noundef %i317, i64 noundef %i322, i64 noundef 1, i64 noundef 1, ptr noundef %i323, ptr noundef %arg7) #14
  %i325 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !188, !noalias !187
  %i326 = icmp eq i32 %i325, 0
  br i1 %i326, label %bb537, label %bb327

bb327:                                            ; preds = %bb312
  %i328 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i329 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i328, i64 0, i32 6, !intel-tbaa !155
  %i330 = load ptr, ptr %i329, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb331:                                            ; preds = %bb311
  %i332 = xor i1 %i280, true
  %i333 = icmp sgt i64 %i275, %i274
  %i334 = select i1 %i332, i1 %i333, i1 false
  br i1 %i334, label %bb335, label %bb541

bb335:                                            ; preds = %bb331
  %i336 = sdiv i64 %i274, %i275
  %i337 = lshr i64 %i274, 63
  %i338 = add nuw nsw i64 %i336, %i337
  %i339 = mul nsw i64 %i338, %i275
  %i340 = sub nsw i64 %i274, %i339
  %i341 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i342 = sdiv i64 %i263, %i341
  %i343 = add nsw i64 %i342, %i265
  %i344 = mul nsw i64 %i343, %i341
  %i345 = sub nsw i64 %i263, %i344
  %i346 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i347 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 14, i64 noundef %i340, i64 noundef %i345, i64 noundef 1, i64 noundef 1, ptr noundef %i346, ptr noundef %arg7) #14
  %i348 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !191, !noalias !187
  %i349 = icmp eq i32 %i348, 0
  br i1 %i349, label %bb537, label %bb350

bb350:                                            ; preds = %bb335
  %i351 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i352 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i351, i64 0, i32 6, !intel-tbaa !155
  %i353 = load ptr, ptr %i352, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb354:                                            ; preds = %bb311
  br i1 %i264, label %bb541, label %bb355

bb355:                                            ; preds = %bb354
  %i356 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i357 = icmp slt i64 %i263, %i356
  br i1 %i357, label %bb358, label %bb541

bb358:                                            ; preds = %bb355
  %i359 = sdiv i64 %i274, %i275
  %i360 = ashr i64 %i274, 63
  %i361 = add nsw i64 %i359, %i360
  %i362 = mul nsw i64 %i361, %i275
  %i363 = sub nsw i64 %i274, %i362
  %i364 = udiv i64 %i263, %i356
  %i365 = add nuw nsw i64 %i364, %i266
  %i366 = mul nsw i64 %i365, %i356
  %i367 = sub nsw i64 %i263, %i366
  %i368 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i369 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 13, i64 noundef %i363, i64 noundef %i367, i64 noundef 1, i64 noundef 1, ptr noundef %i368, ptr noundef %arg7) #14
  %i370 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !194, !noalias !187
  %i371 = icmp eq i32 %i370, 0
  br i1 %i371, label %bb537, label %bb372

bb372:                                            ; preds = %bb358
  %i373 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i374 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i373, i64 0, i32 6, !intel-tbaa !155
  %i375 = load ptr, ptr %i374, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb376:                                            ; preds = %bb311
  %i377 = sdiv i64 %i274, %i275
  %i378 = ashr i64 %i274, 63
  %i379 = add nsw i64 %i377, %i378
  %i380 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i381 = sdiv i64 %i263, %i380
  %i382 = add nsw i64 %i381, %i265
  %i383 = xor i64 %i382, %i379
  %i384 = and i64 %i383, 1
  %i385 = icmp eq i64 %i384, 0
  br i1 %i385, label %bb386, label %bb541

bb386:                                            ; preds = %bb376
  %i387 = mul nsw i64 %i382, %i380
  %i388 = sub nsw i64 %i263, %i387
  %i389 = mul nsw i64 %i379, %i275
  %i390 = sub nsw i64 %i274, %i389
  %i391 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i392 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 17, i64 noundef %i390, i64 noundef %i388, i64 noundef 1, i64 noundef 1, ptr noundef %i391, ptr noundef %arg7) #14
  %i393 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !197, !noalias !187
  %i394 = icmp eq i32 %i393, 0
  br i1 %i394, label %bb537, label %bb395

bb395:                                            ; preds = %bb386
  %i396 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i397 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i396, i64 0, i32 6, !intel-tbaa !155
  %i398 = load ptr, ptr %i397, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb399:                                            ; preds = %bb311
  %i400 = sdiv i64 %i274, %i275
  %i401 = ashr i64 %i274, 63
  %i402 = add nsw i64 %i400, %i401
  %i403 = mul nsw i64 %i402, %i275
  %i404 = sub nsw i64 %i274, %i403
  %i405 = and i64 %i402, 1
  %i406 = icmp eq i64 %i405, 0
  %i407 = xor i64 %i404, -1
  %i408 = add i64 %i275, %i407
  %i409 = select i1 %i406, i64 %i404, i64 %i408
  %i410 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i411 = sdiv i64 %i263, %i410
  %i412 = add nsw i64 %i411, %i265
  %i413 = mul nsw i64 %i412, %i410
  %i414 = sub nsw i64 %i263, %i413
  %i415 = and i64 %i412, 1
  %i416 = icmp eq i64 %i415, 0
  %i417 = xor i64 %i414, -1
  %i418 = add i64 %i410, %i417
  %i419 = select i1 %i416, i64 %i414, i64 %i418
  %i420 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i421 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 5, i64 noundef %i409, i64 noundef %i419, i64 noundef 1, i64 noundef 1, ptr noundef %i420, ptr noundef %arg7) #14
  %i422 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !200, !noalias !187
  %i423 = icmp eq i32 %i422, 0
  br i1 %i423, label %bb537, label %bb424

bb424:                                            ; preds = %bb399
  %i425 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i426 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i425, i64 0, i32 6, !intel-tbaa !155
  %i427 = load ptr, ptr %i426, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb428:                                            ; preds = %bb311
  %i429 = sdiv i64 %i274, %i275
  %i430 = ashr i64 %i274, 63
  %i431 = add nsw i64 %i429, %i430
  %i432 = mul nsw i64 %i431, %i275
  %i433 = sub nsw i64 %i274, %i432
  %i434 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i435 = sdiv i64 %i263, %i434
  %i436 = add nsw i64 %i435, %i265
  %i437 = mul nsw i64 %i436, %i434
  %i438 = sub nsw i64 %i263, %i437
  %i439 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i440 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 7, i64 noundef %i433, i64 noundef %i438, i64 noundef 1, i64 noundef 1, ptr noundef %i439, ptr noundef %arg7) #14
  %i441 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !203, !noalias !187
  %i442 = icmp eq i32 %i441, 0
  br i1 %i442, label %bb537, label %bb443

bb443:                                            ; preds = %bb428
  %i444 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i445 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i444, i64 0, i32 6, !intel-tbaa !155
  %i446 = load ptr, ptr %i445, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb447:                                            ; preds = %bb311
  %i448 = and i64 %i274, 7
  %i449 = getelementptr inbounds [64 x i64], ptr @DitherMatrix, i64 0, i64 %i448, !intel-tbaa !206
  %i450 = load i64, ptr %i449, align 8, !tbaa !206
  %i451 = add i64 %i274, -32
  %i452 = add i64 %i451, %i450
  %i453 = icmp slt i64 %i452, 0
  %i454 = icmp slt i64 %i452, %i275
  %i455 = add nsw i64 %i275, -1
  %i456 = select i1 %i454, i64 %i452, i64 %i455
  %i457 = select i1 %i453, i64 0, i64 %i456
  %i458 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i459 = load i64, ptr %i268, align 8, !tbaa !206
  %i460 = add i64 %i269, %i459
  %i461 = icmp slt i64 %i460, 0
  %i462 = icmp slt i64 %i460, %i458
  %i463 = add nsw i64 %i458, -1
  %i464 = select i1 %i462, i64 %i460, i64 %i463
  %i465 = select i1 %i461, i64 0, i64 %i464
  %i466 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i467 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 3, i64 noundef %i457, i64 noundef %i465, i64 noundef 1, i64 noundef 1, ptr noundef %i466, ptr noundef %arg7) #14
  %i468 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !208, !noalias !187
  %i469 = icmp eq i32 %i468, 0
  br i1 %i469, label %bb537, label %bb470

bb470:                                            ; preds = %bb447
  %i471 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i472 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i471, i64 0, i32 6, !intel-tbaa !155
  %i473 = load ptr, ptr %i472, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb474:                                            ; preds = %bb311
  %i475 = load ptr, ptr %i249, align 8, !tbaa !211, !alias.scope !132, !noalias !135
  %i476 = icmp eq ptr %i475, null
  br i1 %i476, label %bb477, label %bb480

bb477:                                            ; preds = %bb474
  %i478 = call ptr @AcquireRandomInfo() #11
  store ptr %i478, ptr %i249, align 8, !tbaa !211, !alias.scope !132, !noalias !135
  %i479 = load i64, ptr %i165, align 8, !tbaa !149, !alias.scope !132, !noalias !135
  br label %bb480

bb480:                                            ; preds = %bb477, %bb474
  %i481 = phi i64 [ %i479, %bb477 ], [ %i275, %bb474 ]
  %i482 = phi ptr [ %i478, %bb477 ], [ %i475, %bb474 ]
  %i483 = uitofp i64 %i481 to double
  %i484 = call fast double @GetPseudoRandomValue(ptr noundef %i482) #11
  %i485 = fmul fast double %i484, %i483
  %i486 = fptosi double %i485 to i64
  %i487 = load ptr, ptr %i249, align 8, !tbaa !211, !alias.scope !132, !noalias !135
  %i488 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i489 = uitofp i64 %i488 to double
  %i490 = call fast double @GetPseudoRandomValue(ptr noundef %i487) #11
  %i491 = fmul fast double %i490, %i489
  %i492 = fptosi double %i491 to i64
  %i493 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i494 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 6, i64 noundef %i486, i64 noundef %i492, i64 noundef 1, i64 noundef 1, ptr noundef %i493, ptr noundef %arg7) #14
  %i495 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !212, !noalias !187
  %i496 = icmp eq i32 %i495, 0
  br i1 %i496, label %bb537, label %bb497

bb497:                                            ; preds = %bb480
  %i498 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i499 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i498, i64 0, i32 6, !intel-tbaa !155
  %i500 = load ptr, ptr %i499, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb501:                                            ; preds = %bb311
  %i502 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i503 = sdiv i64 %i263, %i502
  %i504 = add nsw i64 %i503, %i265
  %i505 = mul nsw i64 %i504, %i502
  %i506 = sub nsw i64 %i263, %i505
  %i507 = icmp sgt i64 %i275, %i274
  %i508 = add i64 %i275, -1
  %i509 = select i1 %i507, i64 %i274, i64 %i508
  %i510 = select i1 %i280, i64 0, i64 %i509
  %i511 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i512 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 16, i64 noundef %i510, i64 noundef %i506, i64 noundef 1, i64 noundef 1, ptr noundef %i511, ptr noundef %arg7) #14
  %i513 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !215, !noalias !187
  %i514 = icmp eq i32 %i513, 0
  br i1 %i514, label %bb537, label %bb515

bb515:                                            ; preds = %bb501
  %i516 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i517 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i516, i64 0, i32 6, !intel-tbaa !155
  %i518 = load ptr, ptr %i517, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb519:                                            ; preds = %bb311
  %i520 = icmp sgt i64 %i275, %i274
  %i521 = add i64 %i275, -1
  %i522 = select i1 %i520, i64 %i274, i64 %i521
  %i523 = select i1 %i280, i64 0, i64 %i522
  %i524 = load i64, ptr %i166, align 8, !tbaa !150, !alias.scope !132, !noalias !135
  %i525 = icmp slt i64 %i263, %i524
  %i526 = add i64 %i524, -1
  %i527 = select i1 %i525, i64 %i263, i64 %i526
  %i528 = select i1 %i264, i64 0, i64 %i527
  %i529 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i530 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef %arg1, i64 noundef %i523, i64 noundef %i528, i64 noundef 1, i64 noundef 1, ptr noundef %i529, ptr noundef %arg7) #14
  %i531 = load i32, ptr %i248, align 8, !tbaa !180, !alias.scope !218, !noalias !187
  %i532 = icmp eq i32 %i531, 0
  br i1 %i532, label %bb537, label %bb533

bb533:                                            ; preds = %bb519
  %i534 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182, !noalias !183
  %i535 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i534, i64 0, i32 6, !intel-tbaa !155
  %i536 = load ptr, ptr %i535, align 8, !tbaa !155, !noalias !132
  br label %bb537

bb537:                                            ; preds = %bb533, %bb519, %bb515, %bb501, %bb497, %bb480, %bb470, %bb447, %bb443, %bb428, %bb424, %bb399, %bb395, %bb386, %bb372, %bb358, %bb350, %bb335, %bb327, %bb312
  %i538 = phi ptr [ %i530, %bb519 ], [ %i530, %bb533 ], [ %i494, %bb480 ], [ %i494, %bb497 ], [ %i467, %bb447 ], [ %i467, %bb470 ], [ %i440, %bb428 ], [ %i440, %bb443 ], [ %i421, %bb399 ], [ %i421, %bb424 ], [ %i392, %bb386 ], [ %i392, %bb395 ], [ %i369, %bb358 ], [ %i369, %bb372 ], [ %i347, %bb335 ], [ %i347, %bb350 ], [ %i324, %bb312 ], [ %i324, %bb327 ], [ %i512, %bb501 ], [ %i512, %bb515 ]
  %i539 = phi ptr [ null, %bb519 ], [ %i536, %bb533 ], [ null, %bb480 ], [ %i500, %bb497 ], [ null, %bb447 ], [ %i473, %bb470 ], [ null, %bb428 ], [ %i446, %bb443 ], [ null, %bb399 ], [ %i427, %bb424 ], [ null, %bb386 ], [ %i398, %bb395 ], [ null, %bb358 ], [ %i375, %bb372 ], [ null, %bb335 ], [ %i353, %bb350 ], [ null, %bb312 ], [ %i330, %bb327 ], [ null, %bb501 ], [ %i518, %bb515 ]
  %i540 = icmp eq ptr %i538, null
  br i1 %i540, label %bb568, label %bb541

bb541:                                            ; preds = %bb537, %bb376, %bb355, %bb354, %bb331, %bb311, %bb311, %bb311, %bb311, %bb311, %bb311, %bb311
  %i542 = phi ptr [ %i539, %bb537 ], [ %i, %bb311 ], [ %i, %bb311 ], [ %i, %bb311 ], [ %i, %bb311 ], [ %i, %bb311 ], [ %i, %bb311 ], [ %i, %bb311 ], [ %i, %bb376 ], [ %i, %bb355 ], [ %i, %bb354 ], [ %i, %bb331 ]
  %i543 = phi ptr [ %i538, %bb537 ], [ %i8, %bb311 ], [ %i8, %bb311 ], [ %i8, %bb311 ], [ %i8, %bb311 ], [ %i8, %bb311 ], [ %i8, %bb311 ], [ %i8, %bb311 ], [ %i8, %bb376 ], [ %i8, %bb355 ], [ %i8, %bb354 ], [ %i8, %bb331 ]
  %i544 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i272, i64 1
  %i545 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i543, i64 0, i32 0
  %i546 = load i16, ptr %i545, align 2, !tbaa !88, !alias.scope !221, !noalias !222
  store i16 %i546, ptr %i272, align 2, !tbaa !88, !alias.scope !221, !noalias !222
  %i547 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i543, i64 0, i32 1
  %i548 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i272, i64 0, i32 1
  %i549 = load i16, ptr %i547, align 2, !tbaa !88, !alias.scope !221, !noalias !222
  store i16 %i549, ptr %i548, align 2, !tbaa !88, !alias.scope !221, !noalias !222
  %i550 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i543, i64 0, i32 2
  %i551 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i272, i64 0, i32 2
  %i552 = load i16, ptr %i550, align 2, !tbaa !88, !alias.scope !221, !noalias !222
  store i16 %i552, ptr %i551, align 2, !tbaa !88, !alias.scope !221, !noalias !222
  %i553 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i543, i64 0, i32 3
  %i554 = getelementptr inbounds { i16, i16, i16, i16 }, ptr %i272, i64 0, i32 3
  %i555 = load i16, ptr %i553, align 2, !tbaa !88, !alias.scope !221, !noalias !222
  store i16 %i555, ptr %i554, align 2, !tbaa !88, !alias.scope !221, !noalias !222
  %i556 = icmp ne ptr %i271, null
  %i557 = icmp ne ptr %i542, null
  %i558 = select i1 %i556, i1 %i557, i1 false
  br i1 %i558, label %bb559, label %bb562

bb559:                                            ; preds = %bb541
  %i560 = load i16, ptr %i542, align 2, !tbaa !88, !alias.scope !223, !noalias !224
  %i561 = getelementptr inbounds i16, ptr %i271, i64 1
  store i16 %i560, ptr %i271, align 2, !tbaa !88, !alias.scope !225, !noalias !226
  br label %bb562

bb562:                                            ; preds = %bb559, %bb541, %bb308, %bb305, %bb296
  %i563 = phi ptr [ %i301, %bb296 ], [ %i301, %bb305 ], [ %i544, %bb541 ], [ %i544, %bb559 ], [ %i310, %bb308 ]
  %i564 = phi ptr [ %i271, %bb296 ], [ %i307, %bb305 ], [ %i271, %bb541 ], [ %i561, %bb559 ], [ %i271, %bb308 ]
  %i565 = phi i64 [ %i279, %bb296 ], [ %i279, %bb305 ], [ 1, %bb541 ], [ 1, %bb559 ], [ %i279, %bb308 ]
  %i566 = add i64 %i565, %i273
  %i567 = icmp slt i64 %i566, %arg4
  br i1 %i567, label %bb270, label %bb568, !llvm.loop !227

bb568:                                            ; preds = %bb562, %bb537, %bb289
  %i569 = phi ptr [ %i272, %bb537 ], [ %i272, %bb289 ], [ %i563, %bb562 ]
  %i570 = phi ptr [ %i271, %bb537 ], [ %i271, %bb289 ], [ %i564, %bb562 ]
  %i571 = add nuw nsw i64 %i253, 1
  %i572 = icmp eq i64 %i571, %arg5
  br i1 %i572, label %bb573, label %bb250, !llvm.loop !228

bb573:                                            ; preds = %bb568, %bb244, %bb242
  %i574 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182
  %i575 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i574, i64 0, i32 3, !intel-tbaa !158
  %i576 = load ptr, ptr %i575, align 8, !tbaa !158, !noalias !182
  %i577 = icmp eq ptr %i576, null
  br i1 %i577, label %bb592, label %bb578

bb578:                                            ; preds = %bb573
  %i579 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i574, i64 0, i32 0, !intel-tbaa !160
  %i580 = load i32, ptr %i579, align 8, !tbaa !160, !noalias !182
  %i581 = icmp eq i32 %i580, 0
  br i1 %i581, label %bb582, label %bb584

bb582:                                            ; preds = %bb578
  %i583 = call ptr @RelinquishAlignedMemory(ptr noundef nonnull %i576) #11, !noalias !182
  br label %bb587

bb584:                                            ; preds = %bb578
  %i585 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i574, i64 0, i32 2, !intel-tbaa !159
  %i586 = load i64, ptr %i585, align 8, !tbaa !159, !noalias !182
  br label %bb587

bb587:                                            ; preds = %bb584, %bb582
  store ptr null, ptr %i575, align 8, !tbaa !158, !noalias !182
  %i588 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i574, i64 0, i32 4, !intel-tbaa !153
  store ptr null, ptr %i588, align 8, !tbaa !153, !noalias !182
  %i589 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i574, i64 0, i32 6, !intel-tbaa !155
  store ptr null, ptr %i589, align 8, !tbaa !155, !noalias !182
  %i590 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i574, i64 0, i32 2, !intel-tbaa !159
  store i64 0, ptr %i590, align 8, !tbaa !159, !noalias !182
  store i32 0, ptr %i579, align 8, !tbaa !160, !noalias !182
  %i591 = load ptr, ptr %i207, align 8, !tbaa !116, !alias.scope !182
  br label %bb592

bb592:                                            ; preds = %bb587, %bb573
  %i593 = phi ptr [ %i591, %bb587 ], [ %i574, %bb573 ]
  %i594 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, ptr %i574, i64 0, i32 7, !intel-tbaa !229
  store i64 -2880220588, ptr %i594, align 8, !tbaa !229, !noalias !182
  %i595 = call ptr @RelinquishMagickMemory(ptr noundef %i593) #11, !noalias !182
  store ptr null, ptr %i207, align 8, !tbaa !116, !alias.scope !182
  %i596 = call ptr @RelinquishAlignedMemory(ptr noundef nonnull %i207) #11
  br label %bb597

bb597:                                            ; preds = %bb592, %bb205, %bb202, %bb191, %bb189, %bb154, %bb112, %bb90, %bb
  %i598 = phi ptr [ %i164, %bb592 ], [ null, %bb ], [ null, %bb154 ], [ %i164, %bb205 ], [ %i164, %bb189 ], [ null, %bb191 ], [ null, %bb202 ], [ null, %bb90 ], [ null, %bb112 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i8) #11
  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %i) #11
  ret ptr %i598
}

attributes #0 = { mustprogress nofree nounwind willreturn allocsize(0,1) memory(write, inaccessiblemem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nofree nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { hot nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { mustprogress nounwind willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #8 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #9 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #10 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #11 = { nounwind }
attributes #12 = { hot nounwind }
attributes #13 = { nounwind allocsize(0,1) }
attributes #14 = { hot }

!intel.dtrans.types = !{!2, !6, !8, !19, !33, !34, !2}
!llvm.ident = !{!35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35, !35}
!llvm.module.flags = !{!36, !37, !38, !39, !40, !41}

!0 = !{!"A", i32 64, !1}
!1 = !{i64 0, i32 0}
!2 = !{!"S", %struct._ZTS18_MagickPixelPacket._MagickPixelPacket zeroinitializer, i32 10, !3, !3, !3, !4, !1, !5, !5, !5, !5, !5}
!3 = !{i32 0, i32 0}
!4 = !{double 0.000000e+00, i32 0}
!5 = !{float 0.000000e+00, i32 0}
!6 = !{!"S", %struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 4, !7, !7, !7, !7}
!7 = !{i16 0, i32 0}
!8 = !{!"S", %struct._ZTS10_CacheInfo._CacheInfo zeroinitializer, i32 32, !3, !3, !1, !3, !3, !3, !1, !1, !1, !1, !3, !9, !1, !10, !11, !12, !3, !3, !13, !13, !15, !16, !1, !17, !3, !3, !3, !1, !18, !18, !1, !1}
!9 = !{%struct._ZTS18_MagickPixelPacket._MagickPixelPacket zeroinitializer, i32 0}
!10 = !{%struct._ZTS10_NexusInfo._NexusInfo zeroinitializer, i32 2}
!11 = !{%struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 1}
!12 = !{i16 0, i32 1}
!13 = !{!"A", i32 4096, !14}
!14 = !{i8 0, i32 0}
!15 = !{%struct._ZTS13_CacheMethods._CacheMethods zeroinitializer, i32 0}
!16 = !{%struct._ZTS11_RandomInfo._RandomInfo zeroinitializer, i32 1}
!17 = !{i8 0, i32 1}
!18 = !{%struct._ZTS13SemaphoreInfo.SemaphoreInfo zeroinitializer, i32 1}
!19 = !{!"S", %struct._ZTS6_Image._Image zeroinitializer, i32 85, !3, !3, !3, !1, !3, !3, !3, !1, !1, !1, !1, !11, !20, !20, !20, !4, !21, !3, !17, !3, !17, !17, !17, !1, !4, !4, !22, !22, !22, !4, !4, !4, !3, !3, !3, !3, !3, !3, !23, !1, !1, !1, !1, !1, !1, !24, !25, !26, !17, !17, !17, !28, !29, !13, !13, !13, !1, !1, !30, !3, !1, !18, !31, !31, !32, !1, !1, !23, !23, !23, !3, !3, !20, !23, !22, !17, !17, !3, !3, !1, !3, !1, !1, !3, !1}
!20 = !{%struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 0}
!21 = !{%struct._ZTS17_ChromaticityInfo._ChromaticityInfo zeroinitializer, i32 0}
!22 = !{%struct._ZTS14_RectangleInfo._RectangleInfo zeroinitializer, i32 0}
!23 = !{%struct._ZTS6_Image._Image zeroinitializer, i32 1}
!24 = !{%struct._ZTS10_ErrorInfo._ErrorInfo zeroinitializer, i32 0}
!25 = !{%struct._ZTS10_TimerInfo._TimerInfo zeroinitializer, i32 0}
!26 = !{!27, i32 1}
!27 = !{!"F", i1 false, i32 4, !3, !17, !1, !1, !17}
!28 = !{%struct._ZTS12_Ascii85Info._Ascii85Info zeroinitializer, i32 1}
!29 = !{%struct._ZTS9_BlobInfo._BlobInfo zeroinitializer, i32 1}
!30 = !{%struct._ZTS14_ExceptionInfo._ExceptionInfo zeroinitializer, i32 0}
!31 = !{%struct._ZTS12_ProfileInfo._ProfileInfo zeroinitializer, i32 0}
!32 = !{%struct._ZTS12_ProfileInfo._ProfileInfo zeroinitializer, i32 1}
!33 = !{!"S", %struct._ZTS14_RectangleInfo._RectangleInfo zeroinitializer, i32 4, !1, !1, !1, !1}
!34 = !{!"S", %struct._ZTS10_CacheView._CacheView zeroinitializer, i32 -1}
!35 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!36 = !{i32 1, !"wchar_size", i32 4}
!37 = !{i32 1, !"Virtual Function Elim", i32 0}
!38 = !{i32 7, !"uwtable", i32 2}
!39 = !{i32 1, !"ThinLTO", i32 0}
!40 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!41 = !{i32 1, !"LTOPostLink", i32 1}
!42 = distinct !{!23, !23, !43}
!43 = !{%struct._ZTS14_ExceptionInfo._ExceptionInfo zeroinitializer, i32 1}
!44 = !{!45, !46, i64 12976}
!45 = !{!"struct@_Image", !46, i64 0, !46, i64 4, !46, i64 8, !48, i64 16, !46, i64 24, !46, i64 28, !46, i64 32, !48, i64 40, !48, i64 48, !48, i64 56, !48, i64 64, !49, i64 72, !50, i64 80, !50, i64 88, !50, i64 96, !52, i64 104, !53, i64 112, !46, i64 208, !55, i64 216, !46, i64 224, !56, i64 232, !56, i64 240, !56, i64 248, !48, i64 256, !52, i64 264, !52, i64 272, !57, i64 280, !57, i64 312, !57, i64 344, !52, i64 376, !52, i64 384, !52, i64 392, !46, i64 400, !46, i64 404, !46, i64 408, !46, i64 412, !46, i64 416, !46, i64 420, !58, i64 424, !48, i64 432, !48, i64 440, !48, i64 448, !48, i64 456, !48, i64 464, !48, i64 472, !59, i64 480, !60, i64 504, !62, i64 568, !55, i64 576, !55, i64 584, !55, i64 592, !63, i64 600, !64, i64 608, !65, i64 616, !65, i64 4712, !65, i64 8808, !48, i64 12904, !48, i64 12912, !66, i64 12920, !46, i64 12976, !48, i64 12984, !68, i64 12992, !69, i64 13000, !69, i64 13032, !71, i64 13064, !48, i64 13072, !48, i64 13080, !58, i64 13088, !58, i64 13096, !58, i64 13104, !46, i64 13112, !46, i64 13116, !50, i64 13120, !58, i64 13128, !57, i64 13136, !55, i64 13168, !55, i64 13176, !46, i64 13184, !46, i64 13188, !72, i64 13192, !46, i64 13200, !48, i64 13208, !48, i64 13216, !46, i64 13224, !48, i64 13232}
!46 = !{!"omnipotent char", !47, i64 0}
!47 = !{!"Simple C/C++ TBAA"}
!48 = !{!"long", !46, i64 0}
!49 = !{!"pointer@_ZTSP12_PixelPacket", !46, i64 0}
!50 = !{!"struct@_PixelPacket", !51, i64 0, !51, i64 2, !51, i64 4, !51, i64 6}
!51 = !{!"short", !46, i64 0}
!52 = !{!"double", !46, i64 0}
!53 = !{!"struct@_ChromaticityInfo", !54, i64 0, !54, i64 24, !54, i64 48, !54, i64 72}
!54 = !{!"struct@_PrimaryInfo", !52, i64 0, !52, i64 8, !52, i64 16}
!55 = !{!"pointer@_ZTSPv", !46, i64 0}
!56 = !{!"pointer@_ZTSPc", !46, i64 0}
!57 = !{!"struct@_RectangleInfo", !48, i64 0, !48, i64 8, !48, i64 16, !48, i64 24}
!58 = !{!"pointer@_ZTSP6_Image", !46, i64 0}
!59 = !{!"struct@_ErrorInfo", !52, i64 0, !52, i64 8, !52, i64 16}
!60 = !{!"struct@_TimerInfo", !61, i64 0, !61, i64 24, !46, i64 48, !48, i64 56}
!61 = !{!"struct@_Timer", !52, i64 0, !52, i64 8, !52, i64 16}
!62 = !{!"unspecified pointer", !46, i64 0}
!63 = !{!"pointer@_ZTSP12_Ascii85Info", !46, i64 0}
!64 = !{!"pointer@_ZTSP9_BlobInfo", !46, i64 0}
!65 = !{!"array@_ZTSA4096_c", !46, i64 0}
!66 = !{!"struct@_ExceptionInfo", !46, i64 0, !67, i64 4, !56, i64 8, !56, i64 16, !55, i64 24, !46, i64 32, !68, i64 40, !48, i64 48}
!67 = !{!"int", !46, i64 0}
!68 = !{!"pointer@_ZTSP13SemaphoreInfo", !46, i64 0}
!69 = !{!"struct@_ProfileInfo", !56, i64 0, !48, i64 8, !70, i64 16, !48, i64 24}
!70 = !{!"pointer@_ZTSPh", !46, i64 0}
!71 = !{!"pointer@_ZTSP12_ProfileInfo", !46, i64 0}
!72 = !{!"long long", !46, i64 0}
!73 = !{!45, !65, i64 616}
!74 = !{!45, !48, i64 40}
!75 = !{!45, !48, i64 48}
!76 = !{!45, !66, i64 12920}
!77 = !{!45, !46, i64 4}
!78 = !{!50, !51, i64 4}
!79 = !{!80, !81, i64 32}
!80 = !{!"struct@_MagickPixelPacket", !46, i64 0, !46, i64 4, !46, i64 8, !52, i64 16, !48, i64 24, !81, i64 32, !81, i64 36, !81, i64 40, !81, i64 44, !81, i64 48}
!81 = !{!"float", !46, i64 0}
!82 = !{!50, !51, i64 2}
!83 = !{!80, !81, i64 36}
!84 = !{!50, !51, i64 0}
!85 = !{!80, !81, i64 40}
!86 = !{!50, !51, i64 6}
!87 = !{!80, !81, i64 44}
!88 = !{!51, !51, i64 0}
!89 = !{!80, !81, i64 48}
!90 = !{i64 0, i64 4, !91, i64 4, i64 4, !91, i64 8, i64 4, !91, i64 12, i64 4, !91, i64 16, i64 4, !91}
!91 = !{!81, !81, i64 0}
!92 = !{i64 0, i64 4, !91, i64 4, i64 4, !91, i64 8, i64 4, !91, i64 12, i64 4, !91}
!93 = !{i64 0, i64 4, !91, i64 4, i64 4, !91, i64 8, i64 4, !91}
!94 = distinct !{!94, !95}
!95 = !{!"llvm.loop.mustprogress"}
!96 = distinct !{!96, !95}
!97 = distinct !{!97, !95}
!98 = !{!99}
!99 = distinct !{!99, !100, !"MeanShiftImage: q"}
!100 = distinct !{!100, !"MeanShiftImage"}
!101 = !{!102, !103}
!102 = distinct !{!102, !100, !"MeanShiftImage: indexes"}
!103 = distinct !{!103, !100, !"MeanShiftImage: p"}
!104 = distinct !{!104, !95}
!105 = !{!45, !62, i64 568}
!106 = !{!45, !55, i64 576}
!107 = distinct !{!107, !95}
!108 = distinct !{!109, !11, !43}
!109 = !{%struct._ZTS10_CacheView._CacheView zeroinitializer, i32 1}
!110 = !{!111, !58, i64 0}
!111 = !{!"struct@_CacheView", !58, i64 0, !46, i64 8, !48, i64 16, !112, i64 24, !46, i64 32, !48, i64 40}
!112 = !{!"pointer@_ZTSPP10_NexusInfo", !46, i64 0}
!113 = !{!45, !50, i64 80}
!114 = !{!111, !46, i64 8}
!115 = !{!111, !112, i64 24}
!116 = !{!117, !117, i64 0}
!117 = !{!"pointer@_ZTSP10_NexusInfo", !46, i64 0}
!118 = distinct !{!11, !23, !119, !43}
!119 = !{%struct._ZTS10_NexusInfo._NexusInfo zeroinitializer, i32 1}
!120 = !{!45, !55, i64 584}
!121 = !{!122, !46, i64 16}
!122 = !{!"struct@_CacheInfo", !46, i64 0, !46, i64 4, !48, i64 8, !46, i64 16, !46, i64 20, !46, i64 24, !48, i64 32, !48, i64 40, !72, i64 48, !72, i64 56, !46, i64 64, !80, i64 72, !48, i64 128, !112, i64 136, !49, i64 144, !123, i64 152, !46, i64 160, !67, i64 164, !65, i64 168, !65, i64 4264, !124, i64 8360, !131, i64 8448, !48, i64 8456, !55, i64 8464, !46, i64 8472, !46, i64 8476, !67, i64 8480, !48, i64 8488, !68, i64 8496, !68, i64 8504, !48, i64 8512, !48, i64 8520}
!123 = !{!"pointer@_ZTSPt", !46, i64 0}
!124 = !{!"struct@_CacheMethods", !62, i64 0, !125, i64 8, !126, i64 16, !62, i64 24, !127, i64 32, !128, i64 40, !62, i64 48, !129, i64 56, !127, i64 64, !62, i64 72, !130, i64 80}
!125 = !{!"pointer@_ZTSPFPK12_PixelPacketPK6_ImageE", !46, i64 0}
!126 = !{!"pointer@_ZTSPFPKtPK6_ImageE", !46, i64 0}
!127 = !{!"pointer@_ZTSPFP12_PixelPacketP6_ImagellmmP14_ExceptionInfoE", !46, i64 0}
!128 = !{!"pointer@_ZTSPFPtPK6_ImageE", !46, i64 0}
!129 = !{!"pointer@_ZTSPFP12_PixelPacketPK6_ImageE", !46, i64 0}
!130 = !{!"pointer@_ZTSPFvP6_ImageE", !46, i64 0}
!131 = !{!"pointer@_ZTSP11_RandomInfo", !46, i64 0}
!132 = !{!133}
!133 = distinct !{!133, !134, !"GetVirtualPixelsFromNexus: cache_info"}
!134 = distinct !{!134, !"GetVirtualPixelsFromNexus"}
!135 = !{!136, !137, !138, !139, !140, !141}
!136 = distinct !{!136, !134, !"GetVirtualPixelsFromNexus: virtual_nexus"}
!137 = distinct !{!137, !134, !"GetVirtualPixelsFromNexus: pixels"}
!138 = distinct !{!138, !134, !"GetVirtualPixelsFromNexus: virtual_indexes"}
!139 = distinct !{!139, !134, !"GetVirtualPixelsFromNexus: p"}
!140 = distinct !{!140, !134, !"GetVirtualPixelsFromNexus: indexes"}
!141 = distinct !{!141, !134, !"GetVirtualPixelsFromNexus: q"}
!142 = !{!45, !58, i64 424}
!143 = !{!45, !58, i64 13128}
!144 = !{i64 0, i64 8, !145, i64 8, i64 8, !145, i64 16, i64 8, !145, i64 24, i64 8, !145}
!145 = !{!48, !48, i64 0}
!146 = !{i64 0, i64 8, !145, i64 8, i64 8, !145, i64 16, i64 8, !145}
!147 = !{i64 0, i64 8, !145, i64 8, i64 8, !145}
!148 = !{i64 0, i64 8, !145}
!149 = !{!122, !48, i64 32}
!150 = !{!122, !48, i64 40}
!151 = !{!122, !49, i64 144}
!152 = !{!50, !50, i64 0}
!153 = !{!154, !49, i64 56}
!154 = !{!"struct@_NexusInfo", !46, i64 0, !57, i64 8, !72, i64 40, !49, i64 48, !49, i64 56, !46, i64 64, !123, i64 72, !48, i64 80}
!155 = !{!154, !123, i64 72}
!156 = !{!122, !46, i64 160}
!157 = !{!122, !123, i64 152}
!158 = !{!154, !49, i64 48}
!159 = !{!154, !72, i64 40}
!160 = !{!154, !46, i64 0}
!161 = !{!162}
!162 = distinct !{!162, !163, !"AcquireCacheNexusPixels: %cache_info"}
!163 = distinct !{!163, !"AcquireCacheNexusPixels"}
!164 = !{!122, !65, i64 168}
!165 = !{!166}
!166 = distinct !{!166, !167, !"AcquireCacheNexusPixels: %cache_info"}
!167 = distinct !{!167, !"AcquireCacheNexusPixels"}
!168 = !{!169}
!169 = distinct !{!169, !170, !"IsAuthenticPixelCache: %cache_info"}
!170 = distinct !{!170, !"IsAuthenticPixelCache"}
!171 = !{!172}
!172 = distinct !{!172, !170, !"IsAuthenticPixelCache: %nexus_info"}
!173 = !{!154, !48, i64 32}
!174 = !{!154, !48, i64 24}
!175 = !{!154, !46, i64 64}
!176 = !{!154, !48, i64 16}
!177 = !{!57, !48, i64 0}
!178 = !{!154, !48, i64 8}
!179 = !{!132, null, null}
!180 = !{!122, !46, i64 0}
!181 = !{!122, !46, i64 4}
!182 = !{!136}
!183 = !{!133, !137, !138, !139, !140, !141}
!184 = !{!185, !133}
!185 = distinct !{!185, !186, !"GetVirtualIndexesFromNexus: cache_info"}
!186 = distinct !{!186, !"GetVirtualIndexesFromNexus"}
!187 = !{}
!188 = !{!189, !133}
!189 = distinct !{!189, !190, !"GetVirtualIndexesFromNexus: cache_info"}
!190 = distinct !{!190, !"GetVirtualIndexesFromNexus"}
!191 = !{!192, !133}
!192 = distinct !{!192, !193, !"GetVirtualIndexesFromNexus: cache_info"}
!193 = distinct !{!193, !"GetVirtualIndexesFromNexus"}
!194 = !{!195, !133}
!195 = distinct !{!195, !196, !"GetVirtualIndexesFromNexus: cache_info"}
!196 = distinct !{!196, !"GetVirtualIndexesFromNexus"}
!197 = !{!198, !133}
!198 = distinct !{!198, !199, !"GetVirtualIndexesFromNexus: cache_info"}
!199 = distinct !{!199, !"GetVirtualIndexesFromNexus"}
!200 = !{!201, !133}
!201 = distinct !{!201, !202, !"GetVirtualIndexesFromNexus: cache_info"}
!202 = distinct !{!202, !"GetVirtualIndexesFromNexus"}
!203 = !{!204, !133}
!204 = distinct !{!204, !205, !"GetVirtualIndexesFromNexus: cache_info"}
!205 = distinct !{!205, !"GetVirtualIndexesFromNexus"}
!206 = !{!207, !48, i64 0}
!207 = !{!"array@_ZTSA64_l", !48, i64 0}
!208 = !{!209, !133}
!209 = distinct !{!209, !210, !"GetVirtualIndexesFromNexus: cache_info"}
!210 = distinct !{!210, !"GetVirtualIndexesFromNexus"}
!211 = !{!122, !131, i64 8448}
!212 = !{!213, !133}
!213 = distinct !{!213, !214, !"GetVirtualIndexesFromNexus: cache_info"}
!214 = distinct !{!214, !"GetVirtualIndexesFromNexus"}
!215 = !{!216, !133}
!216 = distinct !{!216, !217, !"GetVirtualIndexesFromNexus: cache_info"}
!217 = distinct !{!217, !"GetVirtualIndexesFromNexus"}
!218 = !{!219, !133}
!219 = distinct !{!219, !220, !"GetVirtualIndexesFromNexus: cache_info"}
!220 = distinct !{!220, !"GetVirtualIndexesFromNexus"}
!221 = !{!139, !141}
!222 = !{!133, !136, !137, !138, !140}
!223 = !{!138}
!224 = !{!133, !136, !137, !139, !140, !141}
!225 = !{!140}
!226 = !{!133, !136, !137, !138, !139, !141}
!227 = distinct !{!227, !95}
!228 = distinct !{!228, !95}
!229 = !{!154, !48, i64 80}
; end INTEL_FEATURE_SW_ADVANCED
