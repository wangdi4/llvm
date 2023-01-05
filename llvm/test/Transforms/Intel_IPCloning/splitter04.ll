; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt < %s -passes='module(ip-cloning)' -force-ip-manyreccalls-splitting -debug-only=ipcloning -S 2>&1 | FileCheck %s

; Revision of splitter01.ll after 20220613 pulldown

; Test for splitting part of inlining and splitting for many recursive calls
; functions.

; Check that MeanShiftImage and GetOneCacheViewVirtualPixel are not split
; because thy do not have unique switch or return blocks

; CHECK: MRCS: START ANALYSIS: MeanShiftImage
; CHECK: MRCS: EXIT: canSplitBlocks: No switch or unique return block
; CHECK: MRCS: START ANALYSIS: GetOneCacheViewVirtualPixel
; CHECK: MRCS: EXIT: canSplitBlocks: No switch or unique return block

; Check that GetVirtualPixelsFromNexus is split into
; GetVirtualPixelsFromNexus.1 and GetVirtualPixelsFromNexus.2

; CHECK: MRCS: START ANALYSIS: GetVirtualPixelsFromNexus
; CHECK: MRCS: Can split blocks: Visited: 44 NonVisted: 57
; CHECK: MRCS: Begin 7 split insts
; CHECK-DAG: MRCS:   %9 = alloca i16, align 2
; CHECK-DAG: MRCS:   %10 = alloca %struct._PixelPacket, align 2
; CHECK-DAG: MRCS:   %15 = load %struct._CacheInfo*, %struct._CacheInfo** %14, align 8
; CHECK-DAG: MRCS:   %180 = phi i16* [ %141, %167 ], [ %175, %174 ]
; CHECK-DAG: MRCS:   %184 = phi %struct._PixelPacket* [ %132, %167 ], [ %68, %174 ]
; CHECK-DAG: MRCS:   %185 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 6
; CHECK-DAG: MRCS:   %186 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 7
; CHECK: MRCS: End split insts
; CHECK: MRCS: EXIT: canReloadPHI: Could not find storebacks for PHINode
; CHECK: MRCS: Ignore previous EXIT. Saving PHI as SplitValue
; CHECK: MRCS: Validated split insts:   %184 = phi %struct._PixelPacket* [ %132, %167 ], [ %68, %174 ]
; CHECK: MRCS: Can split GetVirtualPixelsFromNexus
; CHECK: MRCS: Split GetVirtualPixelsFromNexus into GetVirtualPixelsFromNexus.1 and GetVirtualPixelsFromNexus.2

; Check that calls to GetVirtualPixelsFromNexus.1 are inlined and that
; MeanShiftImage TO GetOneCacheViewVirtualPixel is inlined.

; CHECK-DAG: MRCS: Inline{{.*}}TO GetVirtualPixelsFromNexus.1
; CHECK-DAG: MRCS: Inline MeanShiftImage TO GetOneCacheViewVirtualPixel

; Check that GetVirtualPixelsFromNexus.1 is not split because it has
; multiple return blocks.

; CHECK: MRCS: START ANALYSIS: GetVirtualPixelsFromNexus.1
; CHECK: MRCS: EXIT: canSplitBlocks: Multiple return blocks

; Check that GetVirtualPixelsFromNexus.2 is not split because it has
; no switch or unique return block.

; CHECK: MRCS: START ANALYSIS: GetVirtualPixelsFromNexus.2
; CHECK: MRCS: EXIT: canSplitBlocks: No switch or unique return block

; Check the IR:
; Call sites to @GetVirtualPixelsFromNexus should be split
; Definitions of @GetVirtualPixelsFromNexus.1 and
;   @GetVirtualPixelsFromNexus.2 should have an extra argument.

; CHECK: define{{.*}}@MeanShiftImage
; CHECK: call {{.*}} @GetOneCacheViewVirtualPixel{{.*}} #[[A0:[0-9]+]]
; CHECK: define{{.*}}@GetOneCacheViewVirtualPixel
; CHECK: store i32 0, i32* [[R0:%[0-9]+]], align 4
; CHECK: [[R1:%[0-9]+]] = call{{.*}}@GetVirtualPixelsFromNexus.1({{.*}} [[R0]]) #[[A0]]
; CHECK: call{{.*}}@GetVirtualPixelsFromNexus.2({{.*}} [[R1]])
; CHECK: define internal %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef %1, i64 noundef %2, i64 noundef %3, i64 noundef %4, i64 noundef %5, %struct._NexusInfo* noundef %6, %struct._ExceptionInfo* noundef %7)
; CHECK: define internal %struct._PixelPacket* @GetVirtualPixelsFromNexus.1(%struct._Image* noundef %0, i32 noundef %1, i64 noundef %2, i64 noundef %3, i64 noundef %4, i64 noundef %5, %struct._NexusInfo* noundef %6, %struct._ExceptionInfo* noundef %7, i32* %8) #[[A1:[0-9]+]]
; CHECK: define internal %struct._PixelPacket* @GetVirtualPixelsFromNexus.2(%struct._Image* noundef %0, i32 noundef %1, i64 noundef %2, i64 noundef %3, i64 noundef %4, i64 noundef %5, %struct._NexusInfo* noundef %6, %struct._ExceptionInfo* noundef %7, %struct._PixelPacket* %8) #[[A1]]
; CHECK: store i32 0, i32* [[R2:%[0-9]+]], align 4
; CHECK: [[R3:%[0-9]+]] = call{{.*}}@GetVirtualPixelsFromNexus.1({{.*}} [[R2]])
; CHECK: call{{.*}}@GetVirtualPixelsFromNexus.2({{.*}} [[R3]])
; CHECK: store i32 0, i32* [[R4:%[0-9]+]], align 4
; CHECK: [[R5:%[0-9]+]] = call{{.*}}@GetVirtualPixelsFromNexus.1({{.*}} [[R4]])
; CHECK: call{{.*}}@GetVirtualPixelsFromNexus.2({{.*}} [[R5]])
; CHECK: attributes #[[A1]] = { "ip-clone-split-function" }
; CHECK: attributes #[[A0]] = { "prefer-inline-mrc-split" }

@.str.129 = private unnamed_addr constant [15 x i8] c"magick/cache.c\00", align 1
@.str.1.130 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str.2.131 = private unnamed_addr constant [23 x i8] c"MemoryAllocationFailed\00", align 1
@.str.3.132 = private unnamed_addr constant [5 x i8] c"`%s'\00", align 1
@.str.7.170 = private unnamed_addr constant [22 x i8] c"UnableToGetCacheNexus\00", align 1
@.str.17.1479 = private unnamed_addr constant [16 x i8] c"MeanShift/Image\00", align 1
@.str.18.1467 = private unnamed_addr constant [6 x i8] c"%s/%s\00", align 1

@DitherMatrix = internal unnamed_addr constant [64 x i64] [i64 0, i64 48, i64 12, i64 60, i64 3, i64 51, i64 15, i64 63, i64 32, i64 16, i64 44, i64 28, i64 35, i64 19, i64 47, i64 31, i64 8, i64 56, i64 4, i64 52, i64 11, i64 59, i64 7, i64 55, i64 40, i64 24, i64 36, i64 20, i64 43, i64 27, i64 39, i64 23, i64 2, i64 50, i64 14, i64 62, i64 1, i64 49, i64 13, i64 61, i64 34, i64 18, i64 46, i64 30, i64 33, i64 17, i64 45, i64 29, i64 10, i64 58, i64 6, i64 54, i64 9, i64 57, i64 5, i64 53, i64 42, i64 26, i64 38, i64 22, i64 41, i64 25, i64 37, i64 21], align 16

%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.timespec = type { i64, i64 }
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct.stat = type { i64, i64, i64, i32, i32, i32, i32, i64, i64, i64, i64, %struct.timespec, %struct.timespec, %struct.timespec, [3 x i64] }
%union.FileInfo = type { %struct._IO_FILE* }
%struct._Timer = type { double, double, double }
%struct._PrimaryInfo = type { double, double, double }
%struct._ProfileInfo = type { i8*, i64, i8*, i64 }
%struct.SemaphoreInfo = type { i64, i32, i64, i64 }
%struct._ExceptionInfo = type { i32, i32, i8*, i8*, i8*, i32, %struct.SemaphoreInfo*, i64 }
%struct._BlobInfo = type { i64, i64, i64, i32, i32, i64, i64, i32, i32, i32, i32, i32, %union.FileInfo, %struct.stat, i64 (%struct._Image*, i8*, i64)*, i8*, i32, %struct.SemaphoreInfo*, i64, i64 }
%struct._Ascii85Info = type { i64, i64, [10 x i8] }
%struct._TimerInfo = type { %struct._Timer, %struct._Timer, i32, i64 }
%struct._ErrorInfo = type { double, double, double }
%struct._RectangleInfo = type { i64, i64, i64, i64 }
%struct._ChromaticityInfo = type { %struct._PrimaryInfo, %struct._PrimaryInfo, %struct._PrimaryInfo, %struct._PrimaryInfo }
%struct._PixelPacket = type { i16, i16, i16, i16 }
%struct._Image = type { i32, i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, %struct._PixelPacket*, %struct._PixelPacket, %struct._PixelPacket, %struct._PixelPacket, double, %struct._ChromaticityInfo, i32, i8*, i32, i8*, i8*, i8*, i64, double, double, %struct._RectangleInfo, %struct._RectangleInfo, %struct._RectangleInfo, double, double, double, i32, i32, i32, i32, i32, i32, %struct._Image*, i64, i64, i64, i64, i64, i64, %struct._ErrorInfo, %struct._TimerInfo, i32 (i8*, i64, i64, i8*)*, i8*, i8*, i8*, %struct._Ascii85Info*, %struct._BlobInfo*, [4096 x i8], [4096 x i8], [4096 x i8], i64, i64, %struct._ExceptionInfo, i32, i64, %struct.SemaphoreInfo*, %struct._ProfileInfo, %struct._ProfileInfo, %struct._ProfileInfo*, i64, i64, %struct._Image*, %struct._Image*, %struct._Image*, i32, i32, %struct._PixelPacket, %struct._Image*, %struct._RectangleInfo, i8*, i8*, i32, i32, i64, i32, i64, i64, i32, i64 }
%struct._NexusInfo = type { i32, %struct._RectangleInfo, i64, %struct._PixelPacket*, %struct._PixelPacket*, i32, i16*, i64 }

%struct._StringInfo = type { [4096 x i8], i8*, i64, i64 }
%struct._SignatureInfo.948 = type { i32, i32, %struct._StringInfo*, %struct._StringInfo*, i32*, i32, i32, i64, i32, i64, i64 }
%struct._RandomInfo = type { %struct._SignatureInfo.948*, %struct._StringInfo*, %struct._StringInfo*, i64, [4 x i64], double, i64, i16, i16, %struct.SemaphoreInfo*, i64, i64 }
%struct._CacheMethods = type { %struct._PixelPacket* (%struct._Image*, i32, i64, i64, i64, i64, %struct._ExceptionInfo*)*, %struct._PixelPacket* (%struct._Image*)*, i16* (%struct._Image*)*, i32 (%struct._Image*, i32, i64, i64, %struct._PixelPacket*, %struct._ExceptionInfo*)*, %struct._PixelPacket* (%struct._Image*, i64, i64, i64, i64, %struct._ExceptionInfo*)*, i16* (%struct._Image*)*, i32 (%struct._Image*, i64, i64, %struct._PixelPacket*, %struct._ExceptionInfo*)*, %struct._PixelPacket* (%struct._Image*)*, %struct._PixelPacket* (%struct._Image*, i64, i64, i64, i64, %struct._ExceptionInfo*)*, i32 (%struct._Image*, %struct._ExceptionInfo*)*, void (%struct._Image*)* }
%struct._MagickPixelPacket = type { i32, i32, i32, double, i64, float, float, float, float, float }
%struct._CacheInfo = type { i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, i32, %struct._MagickPixelPacket, i64, %struct._NexusInfo**, %struct._PixelPacket*, i16*, i32, i32, [4096 x i8], [4096 x i8], %struct._CacheMethods, %struct._RandomInfo*, i64, i8*, i32, i32, i32, i64, %struct.SemaphoreInfo*, %struct.SemaphoreInfo*, i64, i64 }
%struct._CacheView = type { %struct._Image*, i32, i64, %struct._NexusInfo**, i32, i64 }

declare void @llvm.lifetime.start.p0i8(i64 immarg %0, i8* nocapture %1)
declare i8* @AcquireAlignedMemory(i64 %0, i64 %1)
declare i32 @ThrowMagickException(%struct._ExceptionInfo* nocapture %0, i8* %1, i8* %2, i64 %3, i32 %4, i8* %5, i8* nocapture readonly %6, ...)
declare noalias i8* @RelinquishAlignedMemory(i8* readonly %0)
declare fastcc i32 @ReadPixelCachePixels(%struct._CacheInfo* noalias %0, %struct._NexusInfo* noalias nocapture readonly %1, %struct._ExceptionInfo* %2)
declare fastcc i32 @ReadPixelCacheIndexes(%struct._CacheInfo* noalias %0, %struct._NexusInfo* noalias nocapture readonly %1, %struct._ExceptionInfo* %2)
declare %struct._NexusInfo** @AcquirePixelCacheNexus(i64 %0)
declare %struct._RandomInfo* @AcquireRandomInfo()
declare double @GetPseudoRandomValue(%struct._RandomInfo* nocapture %0)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly %0, i8* noalias nocapture readonly %1, i64 %2, i1 immarg %3)
declare void @llvm.lifetime.end.p0i8(i64 immarg %0, i8* nocapture %1) #2
declare noalias i8* @RelinquishMagickMemory(i8* %0)
declare %struct._Image* @CloneImage(%struct._Image* %0, i64 %1, i64 %2, i32 %3, %struct._ExceptionInfo* %4)
declare i32 @SetImageStorageClass(%struct._Image* %0, i32 %1)
declare void @InheritException(%struct._ExceptionInfo* nocapture %0, %struct._ExceptionInfo* nocapture readonly %1)
declare %struct._CacheView* @AcquireVirtualCacheView(%struct._Image* %0, %struct._ExceptionInfo* nocapture readnone %1)
declare %struct._CacheView* @AcquireAuthenticCacheView(%struct._Image* %0, %struct._ExceptionInfo* %1)
declare %struct._PixelPacket* @GetCacheViewVirtualPixels(%struct._CacheView* nocapture readonly %0, i64 %1, i64 %2, i64 %3, i64 %4, %struct._ExceptionInfo* %5)
declare %struct._PixelPacket* @GetCacheViewAuthenticPixels(%struct._CacheView* nocapture readonly %0, i64 %1, i64 %2, i64 %3, i64 %4, %struct._ExceptionInfo* %5)
declare i16* @GetCacheViewVirtualIndexQueue(%struct._CacheView* nocapture readonly %0)
declare void @GetMagickPixelPacket(%struct._Image* readonly %0, %struct._MagickPixelPacket* nocapture %1)
declare double @llvm.rint.f64(double %0)
declare i32 @SyncCacheViewAuthenticPixels(%struct._CacheView* noalias nocapture readonly %0, %struct._ExceptionInfo* %1)
declare i64 @FormatLocaleString(i8* noalias nocapture %0, i64 %1, i8* noalias nocapture readonly %2, ...)
declare %struct._CacheView* @DestroyCacheView(%struct._CacheView* %0)
declare %struct._Image* @DestroyImage(%struct._Image* %0)

define internal %struct._Image* @MeanShiftImage(%struct._Image* %0, i64 %1, i64 %2, double %3, %struct._ExceptionInfo* %4) #0 {
  %6 = alloca [4096 x i8], align 16
  %7 = alloca %struct._MagickPixelPacket, align 8
  %8 = alloca %struct._MagickPixelPacket, align 8
  %9 = alloca %struct._PixelPacket, align 2
  %10 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 59
  %11 = load i32, i32* %10, align 8
  %12 = icmp eq i32 %11, 0
  br i1 %12, label %16, label %13

13:                                               ; preds = %5
  %14 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 53
  %15 = getelementptr inbounds [4096 x i8], [4096 x i8]* %14, i64 0, i64 0
  br label %16

16:                                               ; preds = %13, %5
  %17 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 7
  %18 = load i64, i64* %17, align 8
  %19 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 8
  %20 = load i64, i64* %19, align 8
  %21 = tail call %struct._Image* @CloneImage(%struct._Image* nonnull %0, i64 %18, i64 %20, i32 1, %struct._ExceptionInfo* %4) #19
  %22 = icmp eq %struct._Image* %21, null
  br i1 %22, label %309, label %23

23:                                               ; preds = %16
  %24 = tail call i32 @SetImageStorageClass(%struct._Image* nonnull %21, i32 1) #19
  %25 = icmp eq i32 %24, 0
  br i1 %25, label %26, label %29

26:                                               ; preds = %23
  %27 = getelementptr inbounds %struct._Image, %struct._Image* %21, i64 0, i32 58
  tail call void @InheritException(%struct._ExceptionInfo* %4, %struct._ExceptionInfo* nonnull %27) #19
  %28 = tail call %struct._Image* @DestroyImage(%struct._Image* nonnull %21) #19
  br label %309

29:                                               ; preds = %23
  %30 = tail call %struct._CacheView* @AcquireVirtualCacheView(%struct._Image* nonnull %0, %struct._ExceptionInfo* %4) #19
  %31 = tail call %struct._CacheView* @AcquireVirtualCacheView(%struct._Image* nonnull %0, %struct._ExceptionInfo* %4) #19
  %32 = tail call %struct._CacheView* @AcquireAuthenticCacheView(%struct._Image* nonnull %21, %struct._ExceptionInfo* %4) #19
  %33 = getelementptr inbounds %struct._Image, %struct._Image* %21, i64 0, i32 8
  %34 = load i64, i64* %33, align 8
  %35 = icmp sgt i64 %34, 0
  br i1 %35, label %36, label %305

36:                                               ; preds = %29
  %37 = getelementptr inbounds %struct._Image, %struct._Image* %21, i64 0, i32 7
  %38 = bitcast %struct._MagickPixelPacket* %7 to i8*
  %39 = getelementptr %struct._Image, %struct._Image* %0, i64 0, i32 1
  %40 = getelementptr inbounds %struct._MagickPixelPacket, %struct._MagickPixelPacket* %7, i64 0, i32 5
  %41 = getelementptr inbounds %struct._MagickPixelPacket, %struct._MagickPixelPacket* %7, i64 0, i32 6
  %42 = getelementptr inbounds %struct._MagickPixelPacket, %struct._MagickPixelPacket* %7, i64 0, i32 7
  %43 = getelementptr inbounds %struct._MagickPixelPacket, %struct._MagickPixelPacket* %7, i64 0, i32 8
  %44 = getelementptr inbounds %struct._MagickPixelPacket, %struct._MagickPixelPacket* %7, i64 0, i32 9
  %45 = bitcast %struct._MagickPixelPacket* %8 to i8*
  %46 = sdiv i64 %2, 2
  %47 = sub nsw i64 0, %46
  %48 = icmp slt i64 %46, %47
  %49 = sdiv i64 %1, 2
  %50 = sub nsw i64 0, %49
  %51 = icmp slt i64 %49, %50
  %52 = lshr i64 %1, 1
  %53 = lshr i64 %2, 1
  %54 = mul i64 %53, %52
  %55 = bitcast %struct._PixelPacket* %9 to i8*
  %56 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %9, i64 0, i32 2
  %57 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %9, i64 0, i32 1
  %58 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %9, i64 0, i32 0
  %59 = fmul fast double %3, %3
  %60 = getelementptr inbounds %struct._MagickPixelPacket, %struct._MagickPixelPacket* %8, i64 0, i32 5
  %61 = getelementptr inbounds %struct._MagickPixelPacket, %struct._MagickPixelPacket* %8, i64 0, i32 6
  %62 = getelementptr inbounds %struct._MagickPixelPacket, %struct._MagickPixelPacket* %8, i64 0, i32 7
  %63 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %9, i64 0, i32 3
  %64 = getelementptr inbounds %struct._MagickPixelPacket, %struct._MagickPixelPacket* %8, i64 0, i32 8
  %65 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 47
  %66 = getelementptr inbounds [4096 x i8], [4096 x i8]* %6, i64 0, i64 0
  %67 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 53
  %68 = getelementptr inbounds [4096 x i8], [4096 x i8]* %67, i64 0, i64 0
  %69 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 48
  %70 = add nsw i64 %49, 1
  %71 = add nsw i64 %46, 1
  br label %72

72:                                               ; preds = %299, %36
  %73 = phi i32 [ 1, %36 ], [ %301, %299 ]
  %74 = phi i64 [ 0, %36 ], [ %300, %299 ]
  %75 = phi i64 [ 0, %36 ], [ %302, %299 ]
  %76 = icmp eq i32 %73, 0
  br i1 %76, label %299, label %77

77:                                               ; preds = %72
  %78 = load i64, i64* %17, align 8
  %79 = call %struct._PixelPacket* @GetCacheViewVirtualPixels(%struct._CacheView* %30, i64 0, i64 %75, i64 %78, i64 1, %struct._ExceptionInfo* %4) #19
  %80 = load i64, i64* %37, align 8
  %81 = call %struct._PixelPacket* @GetCacheViewAuthenticPixels(%struct._CacheView* %32, i64 0, i64 %75, i64 %80, i64 1, %struct._ExceptionInfo* %4) #19
  %82 = icmp eq %struct._PixelPacket* %79, null
  %83 = icmp eq %struct._PixelPacket* %81, null
  %84 = or i1 %82, %83
  br i1 %84, label %299, label %85

85:                                               ; preds = %77
  %86 = call i16* @GetCacheViewVirtualIndexQueue(%struct._CacheView* %30) #19
  %87 = load i64, i64* %37, align 8
  %88 = icmp sgt i64 %87, 0
  br i1 %88, label %89, label %283

89:                                               ; preds = %85
  %90 = icmp ne i16* %86, null
  %91 = sitofp i64 %75 to double
  br label %92

92:                                               ; preds = %249, %89
  %93 = phi i32 [ 1, %89 ], [ %206, %249 ]
  %94 = phi i64 [ 0, %89 ], [ %280, %249 ]
  %95 = phi %struct._PixelPacket* [ %81, %89 ], [ %279, %249 ]
  %96 = phi %struct._PixelPacket* [ %79, %89 ], [ %278, %249 ]
  call void @llvm.lifetime.start.p0i8(i64 56, i8* nonnull %38) #19
  call void @GetMagickPixelPacket(%struct._Image* %0, %struct._MagickPixelPacket* nonnull %7) #19
  %97 = load i32, i32* %39, align 4
  %98 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %96, i64 0, i32 2
  %99 = load i16, i16* %98, align 2
  %100 = uitofp i16 %99 to float
  store float %100, float* %40, align 8
  %101 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %96, i64 0, i32 1
  %102 = load i16, i16* %101, align 2
  %103 = uitofp i16 %102 to float
  store float %103, float* %41, align 4
  %104 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %96, i64 0, i32 0
  %105 = load i16, i16* %104, align 2
  %106 = uitofp i16 %105 to float
  store float %106, float* %42, align 8
  %107 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %96, i64 0, i32 3
  %108 = load i16, i16* %107, align 2
  %109 = uitofp i16 %108 to float
  store float %109, float* %43, align 4
  %110 = icmp eq i32 %97, 12
  %111 = and i1 %90, %110
  br i1 %111, label %112, label %116

112:                                              ; preds = %92
  %113 = getelementptr inbounds i16, i16* %86, i64 %94
  %114 = load i16, i16* %113, align 2
  %115 = uitofp i16 %114 to float
  store float %115, float* %44, align 8
  br label %116

116:                                              ; preds = %112, %92
  %117 = sitofp i64 %94 to double
  br label %121

118:                                              ; preds = %205
  %119 = add nuw nsw i64 %122, 1
  %120 = icmp eq i64 %119, 100
  br i1 %120, label %249, label %121

121:                                              ; preds = %118, %116
  %122 = phi i64 [ 0, %116 ], [ %119, %118 ]
  %123 = phi double [ %117, %116 ], [ %212, %118 ]
  %124 = phi double [ %91, %116 ], [ %213, %118 ]
  %125 = phi i32 [ %93, %116 ], [ %206, %118 ]
  call void @llvm.lifetime.start.p0i8(i64 56, i8* nonnull %45) #19
  call void @GetMagickPixelPacket(%struct._Image* %0, %struct._MagickPixelPacket* nonnull %8) #19
  %126 = load float, float* %40, align 8
  %127 = load float, float* %41, align 4
  %128 = load float, float* %42, align 8
  br i1 %48, label %205, label %129

129:                                              ; preds = %121
  %130 = call fast double @llvm.rint.f64(double %123)
  %131 = fptosi double %130 to i64
  %132 = call fast double @llvm.rint.f64(double %124)
  %133 = fptosi double %132 to i64
  br i1 %51, label %205, label %134

134:                                              ; preds = %202, %129
  %135 = phi i64 [ %203, %202 ], [ %47, %129 ]
  %136 = phi i64 [ %199, %202 ], [ 0, %129 ]
  %137 = phi double [ %198, %202 ], [ 0.000000e+00, %129 ]
  %138 = phi double [ %197, %202 ], [ 0.000000e+00, %129 ]
  %139 = phi i32 [ %196, %202 ], [ %125, %129 ]
  %140 = mul nsw i64 %135, %135
  %141 = add i64 %135, %133
  %142 = sitofp i64 %135 to double
  %143 = fadd fast double %124, %142
  br label %144

144:                                              ; preds = %195, %134
  %145 = phi i64 [ %50, %134 ], [ %200, %195 ]
  %146 = phi i64 [ %136, %134 ], [ %199, %195 ]
  %147 = phi double [ %137, %134 ], [ %198, %195 ]
  %148 = phi double [ %138, %134 ], [ %197, %195 ]
  %149 = phi i32 [ %139, %134 ], [ %196, %195 ]
  %150 = mul nsw i64 %145, %145
  %151 = add nuw nsw i64 %150, %140
  %152 = icmp sgt i64 %151, %54
  br i1 %152, label %195, label %153

153:                                              ; preds = %144
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %55) #19
  %154 = add i64 %145, %131
  %155 = call i32 @GetOneCacheViewVirtualPixel(%struct._CacheView* %31, i64 %154, i64 %141, %struct._PixelPacket* nonnull %9, %struct._ExceptionInfo* %4) #19
  %156 = load float, float* %40, align 8
  %157 = load i16, i16* %56, align 2
  %158 = uitofp i16 %157 to float
  %159 = fsub fast float %156, %158
  %160 = fmul fast float %159, %159
  %161 = load float, float* %41, align 4
  %162 = load i16, i16* %57, align 2
  %163 = uitofp i16 %162 to float
  %164 = fsub fast float %161, %163
  %165 = fmul fast float %164, %164
  %166 = fadd fast float %165, %160
  %167 = load float, float* %42, align 8
  %168 = load i16, i16* %58, align 2
  %169 = uitofp i16 %168 to float
  %170 = fsub fast float %167, %169
  %171 = fmul fast float %170, %170
  %172 = fadd fast float %166, %171
  %173 = fpext float %172 to double
  %174 = fcmp fast ult double %59, %173
  br i1 %174, label %191, label %175

175:                                              ; preds = %153
  %176 = sitofp i64 %145 to double
  %177 = fadd fast double %147, %123
  %178 = fadd fast double %177, %176
  %179 = fadd fast double %143, %148
  %180 = load float, float* %60, align 8
  %181 = fadd fast float %180, %158
  store float %181, float* %60, align 8
  %182 = load float, float* %61, align 4
  %183 = fadd fast float %182, %163
  store float %183, float* %61, align 4
  %184 = load float, float* %62, align 8
  %185 = fadd fast float %184, %169
  store float %185, float* %62, align 8
  %186 = load i16, i16* %63, align 2
  %187 = uitofp i16 %186 to float
  %188 = load float, float* %64, align 4
  %189 = fadd fast float %188, %187
  store float %189, float* %64, align 4
  %190 = add nsw i64 %146, 1
  br label %191

191:                                              ; preds = %175, %153
  %192 = phi double [ %179, %175 ], [ %148, %153 ]
  %193 = phi double [ %178, %175 ], [ %147, %153 ]
  %194 = phi i64 [ %190, %175 ], [ %146, %153 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %55) #19
  br label %195

195:                                              ; preds = %191, %144
  %196 = phi i32 [ %155, %191 ], [ %149, %144 ]
  %197 = phi double [ %192, %191 ], [ %148, %144 ]
  %198 = phi double [ %193, %191 ], [ %147, %144 ]
  %199 = phi i64 [ %194, %191 ], [ %146, %144 ]
  %200 = add i64 %145, 1
  %201 = icmp eq i64 %200, %70
  br i1 %201, label %202, label %144

202:                                              ; preds = %195
  %203 = add i64 %135, 1
  %204 = icmp eq i64 %203, %71
  br i1 %204, label %205, label %134

205:                                              ; preds = %202, %129, %121
  %206 = phi i32 [ %125, %121 ], [ %125, %129 ], [ %196, %202 ]
  %207 = phi double [ 0.000000e+00, %121 ], [ 0.000000e+00, %129 ], [ %197, %202 ]
  %208 = phi double [ 0.000000e+00, %121 ], [ 0.000000e+00, %129 ], [ %198, %202 ]
  %209 = phi i64 [ 0, %121 ], [ 0, %129 ], [ %199, %202 ]
  %210 = sitofp i64 %209 to double
  %211 = fdiv fast double 1.000000e+00, %210
  %212 = fmul fast double %211, %208
  %213 = fmul fast double %211, %207
  %214 = load float, float* %60, align 8
  %215 = fpext float %214 to double
  %216 = fmul fast double %211, %215
  %217 = fptrunc double %216 to float
  store float %217, float* %40, align 8
  %218 = load float, float* %61, align 4
  %219 = fpext float %218 to double
  %220 = fmul fast double %211, %219
  %221 = fptrunc double %220 to float
  store float %221, float* %41, align 4
  %222 = load float, float* %62, align 8
  %223 = fpext float %222 to double
  %224 = fmul fast double %211, %223
  %225 = fptrunc double %224 to float
  store float %225, float* %42, align 8
  %226 = load float, float* %64, align 4
  %227 = fpext float %226 to double
  %228 = fmul fast double %211, %227
  %229 = fptrunc double %228 to float
  store float %229, float* %43, align 4
  %230 = fsub fast double %212, %123
  %231 = fmul fast double %230, %230
  %232 = fsub fast double %213, %124
  %233 = fmul fast double %232, %232
  %234 = fsub fast float %217, %126
  %235 = fpext float %234 to double
  %236 = fmul fast double %235, %235
  %237 = fsub fast float %221, %127
  %238 = fpext float %237 to double
  %239 = fmul fast double %238, %238
  %240 = fsub fast float %225, %128
  %241 = fpext float %240 to double
  %242 = fmul fast double %241, %241
  %243 = fadd fast double %239, %236
  %244 = fadd fast double %243, %242
  %245 = fmul fast double %244, 0x3EEFC05F809F40DF
  %246 = fadd fast double %231, %233
  %247 = fadd fast double %246, %245
  %248 = fcmp fast ugt double %247, 3.000000e+00
  call void @llvm.lifetime.end.p0i8(i64 56, i8* nonnull %45) #19
  br i1 %248, label %118, label %249

249:                                              ; preds = %205, %118
  %250 = fcmp fast ugt float %217, 0.000000e+00
  %251 = fcmp fast ult float %217, 6.553500e+04
  %252 = fadd fast float %217, 5.000000e-01
  %253 = fptoui float %252 to i16
  %254 = select i1 %251, i16 %253, i16 -1
  %255 = select i1 %250, i16 %254, i16 0
  %256 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %95, i64 0, i32 2
  store i16 %255, i16* %256, align 2
  %257 = fcmp fast ugt float %221, 0.000000e+00
  %258 = fcmp fast ult float %221, 6.553500e+04
  %259 = fadd fast float %221, 5.000000e-01
  %260 = fptoui float %259 to i16
  %261 = select i1 %258, i16 %260, i16 -1
  %262 = select i1 %257, i16 %261, i16 0
  %263 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %95, i64 0, i32 1
  store i16 %262, i16* %263, align 2
  %264 = fcmp fast ugt float %225, 0.000000e+00
  %265 = fcmp fast ult float %225, 6.553500e+04
  %266 = fadd fast float %225, 5.000000e-01
  %267 = fptoui float %266 to i16
  %268 = select i1 %265, i16 %267, i16 -1
  %269 = select i1 %264, i16 %268, i16 0
  %270 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %95, i64 0, i32 0
  store i16 %269, i16* %270, align 2
  %271 = fcmp fast ugt float %229, 0.000000e+00
  %272 = fcmp fast ult float %229, 6.553500e+04
  %273 = fadd fast float %229, 5.000000e-01
  %274 = fptoui float %273 to i16
  %275 = select i1 %272, i16 %274, i16 -1
  %276 = select i1 %271, i16 %275, i16 0
  %277 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %95, i64 0, i32 3
  store i16 %276, i16* %277, align 2
  %278 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %96, i64 1
  %279 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %95, i64 1
  call void @llvm.lifetime.end.p0i8(i64 56, i8* nonnull %38) #19
  %280 = add nuw nsw i64 %94, 1
  %281 = load i64, i64* %37, align 8
  %282 = icmp slt i64 %280, %281
  br i1 %282, label %92, label %283

283:                                              ; preds = %249, %85
  %284 = phi i32 [ 1, %85 ], [ %206, %249 ]
  %285 = call i32 @SyncCacheViewAuthenticPixels(%struct._CacheView* %32, %struct._ExceptionInfo* %4) #19
  %286 = icmp eq i32 %285, 0
  %287 = select i1 %286, i32 0, i32 %284
  %288 = load i32 (i8*, i64, i64, i8*)*, i32 (i8*, i64, i64, i8*)** %65, align 8
  %289 = icmp eq i32 (i8*, i64, i64, i8*)* %288, null
  br i1 %289, label %299, label %290

290:                                              ; preds = %283
  %291 = add nsw i64 %74, 1
  %292 = load i64, i64* %19, align 8
  call void @llvm.lifetime.start.p0i8(i64 4096, i8* nonnull %66) #19
  %293 = call i64 (i8*, i64, i8*, ...) @FormatLocaleString(i8* nonnull %66, i64 4096, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.18.1467, i64 0, i64 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.17.1479, i64 0, i64 0), i8* nonnull %68) #19
  %294 = load i32 (i8*, i64, i64, i8*)*, i32 (i8*, i64, i64, i8*)** %65, align 8
  %295 = load i8*, i8** %69, align 8
  %296 = call i32 %294(i8* nonnull %66, i64 %74, i64 %292, i8* %295) #19
  call void @llvm.lifetime.end.p0i8(i64 4096, i8* nonnull %66) #19
  %297 = icmp eq i32 %296, 0
  %298 = select i1 %297, i32 0, i32 %287
  br label %299

299:                                              ; preds = %290, %283, %77, %72
  %300 = phi i64 [ %74, %72 ], [ %74, %77 ], [ %74, %283 ], [ %291, %290 ]
  %301 = phi i32 [ 0, %72 ], [ 0, %77 ], [ %287, %283 ], [ %298, %290 ]
  %302 = add nuw nsw i64 %75, 1
  %303 = load i64, i64* %33, align 8
  %304 = icmp slt i64 %302, %303
  br i1 %304, label %72, label %305

305:                                              ; preds = %299, %29
  %306 = call %struct._CacheView* @DestroyCacheView(%struct._CacheView* %32) #19
  %307 = call %struct._CacheView* @DestroyCacheView(%struct._CacheView* %31) #19
  %308 = call %struct._CacheView* @DestroyCacheView(%struct._CacheView* %30) #19
  br label %309

309:                                              ; preds = %305, %26, %16
  %310 = phi %struct._Image* [ null, %26 ], [ %21, %305 ], [ null, %16 ]
  ret %struct._Image* %310
}

define internal i32 @GetOneCacheViewVirtualPixel(%struct._CacheView* noalias nocapture readonly %0, i64 %1, i64 %2, %struct._PixelPacket* noalias nocapture %3, %struct._ExceptionInfo* %4) #0 {
  %6 = getelementptr inbounds %struct._CacheView, %struct._CacheView* %0, i64 0, i32 0
  %7 = load %struct._Image*, %struct._Image** %6, align 8
  %8 = getelementptr inbounds %struct._Image, %struct._Image* %7, i64 0, i32 12
  %9 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %8, i64 0, i32 0
  %10 = load i16, i16* %9, align 2
  %11 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %3, i64 0, i32 0
  store i16 %10, i16* %11, align 2
  %12 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %8, i64 0, i32 1
  %13 = load i16, i16* %12, align 2
  %14 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %3, i64 0, i32 1
  store i16 %13, i16* %14, align 2
  %15 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %8, i64 0, i32 2
  %16 = load i16, i16* %15, align 2
  %17 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %3, i64 0, i32 2
  store i16 %16, i16* %17, align 2
  %18 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %8, i64 0, i32 3
  %19 = load i16, i16* %18, align 2
  %20 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %3, i64 0, i32 3
  store i16 %19, i16* %20, align 2
  %21 = getelementptr inbounds %struct._CacheView, %struct._CacheView* %0, i64 0, i32 1
  %22 = load i32, i32* %21, align 8
  %23 = getelementptr inbounds %struct._CacheView, %struct._CacheView* %0, i64 0, i32 3
  %24 = load %struct._NexusInfo**, %struct._NexusInfo*** %23, align 8
  %25 = load %struct._NexusInfo*, %struct._NexusInfo** %24, align 8
  %26 = tail call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %7, i32 %22, i64 %1, i64 %2, i64 1, i64 1, %struct._NexusInfo* %25, %struct._ExceptionInfo* %4) #19
  %27 = icmp eq %struct._PixelPacket* %26, null
  br i1 %27, label %37, label %28

28:                                               ; preds = %5
  %29 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %26, i64 0, i32 0
  %30 = load i16, i16* %29, align 2
  store i16 %30, i16* %11, align 2
  %31 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %26, i64 0, i32 1
  %32 = load i16, i16* %31, align 2
  store i16 %32, i16* %14, align 2
  %33 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %26, i64 0, i32 2
  %34 = load i16, i16* %33, align 2
  store i16 %34, i16* %17, align 2
  %35 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %26, i64 0, i32 3
  %36 = load i16, i16* %35, align 2
  store i16 %36, i16* %20, align 2
  br label %37

37:                                               ; preds = %28, %5
  %38 = phi i32 [ 1, %28 ], [ 0, %5 ]
  ret i32 %38
}

define internal %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef %1, i64 noundef %2, i64 noundef %3, i64 noundef %4, i64 noundef %5, %struct._NexusInfo* noundef %6, %struct._ExceptionInfo* noundef %7) #15 {
  %9 = alloca i16, align 2
  %10 = alloca %struct._PixelPacket, align 2
  %11 = bitcast i16* %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 2, i8* nonnull %11) #62
  %12 = bitcast %struct._PixelPacket* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %12) #62
  %13 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 49
  %14 = bitcast i8** %13 to %struct._CacheInfo**
  %15 = load %struct._CacheInfo*, %struct._CacheInfo** %14, align 8
  %16 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 3
  %17 = load i32, i32* %16, align 8
  %18 = icmp eq i32 %17, 0
  br i1 %18, label %626, label %19

19:                                               ; preds = %8
  %20 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 38
  %21 = load %struct._Image*, %struct._Image** %20, align 8
  %22 = icmp eq %struct._Image* %21, null
  br i1 %22, label %23, label %27

23:                                               ; preds = %19
  %24 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 73
  %25 = load %struct._Image*, %struct._Image** %24, align 8
  %26 = icmp ne %struct._Image* %25, null
  br label %27

27:                                               ; preds = %23, %19
  %28 = phi i1 [ true, %19 ], [ %26, %23 ]
  %29 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1
  %30 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 0
  store i64 %4, i64* %30, align 8
  %31 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 1
  store i64 %5, i64* %31, align 8
  %32 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 2
  store i64 %2, i64* %32, align 8
  %33 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 3
  store i64 %3, i64* %33, align 8
  %34 = load i32, i32* %16, align 8
  %35 = icmp eq i32 %34, 1
  br i1 %35, label %39, label %36

36:                                               ; preds = %27
  %37 = icmp ne i32 %34, 2
  %38 = or i1 %28, %37
  br i1 %38, label %78, label %40

39:                                               ; preds = %27
  br i1 %28, label %78, label %40

40:                                               ; preds = %39, %36
  %41 = add nsw i64 %5, %3
  %42 = icmp sgt i64 %2, -1
  br i1 %42, label %43, label %78

43:                                               ; preds = %40
  %44 = add nsw i64 %4, %2
  %45 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 6
  %46 = load i64, i64* %45, align 8
  %47 = icmp sle i64 %44, %46
  %48 = icmp sgt i64 %3, -1
  %49 = and i1 %47, %48
  br i1 %49, label %50, label %78

50:                                               ; preds = %43
  %51 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 7
  %52 = load i64, i64* %51, align 8
  %53 = icmp sgt i64 %41, %52
  br i1 %53, label %78, label %54

54:                                               ; preds = %50
  %55 = icmp eq i64 %5, 1
  br i1 %55, label %63, label %56

56:                                               ; preds = %54
  %57 = icmp eq i64 %2, 0
  br i1 %57, label %58, label %78

58:                                               ; preds = %56
  %59 = icmp eq i64 %46, %4
  br i1 %59, label %63, label %60

60:                                               ; preds = %58
  %61 = urem i64 %4, %46
  %62 = icmp eq i64 %61, 0
  br i1 %62, label %63, label %78

63:                                               ; preds = %60, %58, %54
  %64 = mul i64 %46, %3
  %65 = add i64 %64, %2
  %66 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 14
  %67 = load %struct._PixelPacket*, %struct._PixelPacket** %66, align 8
  %68 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %67, i64 %65
  %69 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 4
  store %struct._PixelPacket* %68, %struct._PixelPacket** %69, align 8
  %70 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  store i16* null, i16** %70, align 8
  %71 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 16
  %72 = load i32, i32* %71, align 8
  %73 = icmp eq i32 %72, 0
  br i1 %73, label %174, label %74

74:                                               ; preds = %63
  %75 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 15
  %76 = load i16*, i16** %75, align 8
  %77 = getelementptr inbounds i16, i16* %76, i64 %65
  store i16* %77, i16** %70, align 8
  br label %174

78:                                               ; preds = %60, %56, %50, %43, %40, %39, %36
  %79 = mul i64 %5, %4
  %80 = shl i64 %79, 3
  %81 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 16
  %82 = load i32, i32* %81, align 8
  %83 = icmp eq i32 %82, 0
  %84 = mul i64 %79, 10
  %85 = select i1 %83, i64 %80, i64 %84
  %86 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 3
  %87 = load %struct._PixelPacket*, %struct._PixelPacket** %86, align 8
  %88 = icmp eq %struct._PixelPacket* %87, null
  %89 = bitcast %struct._PixelPacket* %87 to i8*
  br i1 %88, label %90, label %105

90:                                               ; preds = %78
  %91 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 2
  store i64 %85, i64* %91, align 8
  %92 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 0
  store i32 0, i32* %92, align 8
  %93 = tail call i8* @AcquireAlignedMemory(i64 noundef 1, i64 noundef %85) #63
  %94 = bitcast i8* %93 to %struct._PixelPacket*
  store %struct._PixelPacket* %94, %struct._PixelPacket** %86, align 8
  %95 = icmp eq i8* %93, null
  br i1 %95, label %96, label %98

96:                                               ; preds = %90
  store i32 1, i32* %92, align 8
  %97 = load i64, i64* %91, align 8
  store %struct._PixelPacket* null, %struct._PixelPacket** %86, align 8
  br label %98

98:                                               ; preds = %96, %90
  %99 = phi %struct._PixelPacket* [ null, %96 ], [ %94, %90 ]
  %100 = icmp eq %struct._PixelPacket* %99, null
  br i1 %100, label %101, label %131

101:                                              ; preds = %98
  %102 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 18
  %103 = getelementptr inbounds [4096 x i8], [4096 x i8]* %102, i64 0, i64 0
  %104 = tail call i32 (%struct._ExceptionInfo*, i8*, i8*, i64, i32, i8*, i8*, ...) @ThrowMagickException(%struct._ExceptionInfo* noundef %7, i8* noundef getelementptr inbounds ([15 x i8], [15 x i8]* @.str.129, i64 0, i64 0), i8* noundef getelementptr inbounds ([1 x i8], [1 x i8]* @.str.1.130, i64 0, i64 0), i64 noundef 4688, i32 noundef 400, i8* noundef getelementptr inbounds ([23 x i8], [23 x i8]* @.str.2.131, i64 0, i64 0), i8* noundef getelementptr inbounds ([5 x i8], [5 x i8]* @.str.3.132, i64 0, i64 0), i8* noundef nonnull %103) #62
  store i64 0, i64* %91, align 8
  br label %626

105:                                              ; preds = %78
  %106 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 2
  %107 = load i64, i64* %106, align 8
  %108 = icmp ult i64 %107, %85
  br i1 %108, label %109, label %131

109:                                              ; preds = %105
  %110 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 0
  %111 = load i32, i32* %110, align 8
  %112 = icmp eq i32 %111, 0
  br i1 %112, label %113, label %115

113:                                              ; preds = %109
  %114 = tail call i8* @RelinquishAlignedMemory(i8* noundef nonnull %89) #62
  br label %116

115:                                              ; preds = %109
  br label %116

116:                                              ; preds = %115, %113
  store %struct._PixelPacket* null, %struct._PixelPacket** %86, align 8
  %117 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 4
  store %struct._PixelPacket* null, %struct._PixelPacket** %117, align 8
  %118 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  store i16* null, i16** %118, align 8
  store i64 %85, i64* %106, align 8
  store i32 0, i32* %110, align 8
  %119 = tail call i8* @AcquireAlignedMemory(i64 noundef 1, i64 noundef %85) #63
  %120 = bitcast i8* %119 to %struct._PixelPacket*
  store %struct._PixelPacket* %120, %struct._PixelPacket** %86, align 8
  %121 = icmp eq i8* %119, null
  br i1 %121, label %122, label %124

122:                                              ; preds = %116
  store i32 1, i32* %110, align 8
  %123 = load i64, i64* %106, align 8
  store %struct._PixelPacket* null, %struct._PixelPacket** %86, align 8
  br label %124

124:                                              ; preds = %122, %116
  %125 = phi %struct._PixelPacket* [ null, %122 ], [ %120, %116 ]
  %126 = icmp eq %struct._PixelPacket* %125, null
  br i1 %126, label %127, label %131

127:                                              ; preds = %124
  %128 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 18
  %129 = getelementptr inbounds [4096 x i8], [4096 x i8]* %128, i64 0, i64 0
  %130 = tail call i32 (%struct._ExceptionInfo*, i8*, i8*, i64, i32, i8*, i8*, ...) @ThrowMagickException(%struct._ExceptionInfo* noundef %7, i8* noundef getelementptr inbounds ([15 x i8], [15 x i8]* @.str.129, i64 0, i64 0), i8* noundef getelementptr inbounds ([1 x i8], [1 x i8]* @.str.1.130, i64 0, i64 0), i64 noundef 4688, i32 noundef 400, i8* noundef getelementptr inbounds ([23 x i8], [23 x i8]* @.str.2.131, i64 0, i64 0), i8* noundef getelementptr inbounds ([5 x i8], [5 x i8]* @.str.3.132, i64 0, i64 0), i8* noundef nonnull %129) #62
  store i64 0, i64* %106, align 8
  br label %626

131:                                              ; preds = %124, %105, %98
  %132 = phi %struct._PixelPacket* [ %125, %124 ], [ %99, %98 ], [ %87, %105 ]
  %133 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 4
  store %struct._PixelPacket* %132, %struct._PixelPacket** %133, align 8
  %134 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  store i16* null, i16** %134, align 8
  %135 = load i32, i32* %81, align 8
  %136 = icmp eq i32 %135, 0
  br i1 %136, label %140, label %137

137:                                              ; preds = %131
  %138 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %132, i64 %79
  %139 = getelementptr %struct._PixelPacket, %struct._PixelPacket* %138, i64 0, i32 0
  store i16* %139, i16** %134, align 8
  br label %140

140:                                              ; preds = %137, %131
  %141 = phi i16* [ %139, %137 ], [ null, %131 ]
  %142 = load i32, i32* %16, align 8
  %143 = icmp eq i32 %142, 4
  br i1 %143, label %144, label %153

144:                                              ; preds = %140
  %145 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %29, i64 0, i32 3
  %146 = load i64, i64* %145, align 8
  %147 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 6
  %148 = load i64, i64* %147, align 8
  %149 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %29, i64 0, i32 2
  %150 = load i64, i64* %149, align 8
  %151 = mul i64 %148, %146
  %152 = add i64 %151, %150
  br label %167

153:                                              ; preds = %140
  %154 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %29, i64 0, i32 3
  %155 = load i64, i64* %154, align 8
  %156 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 6
  %157 = load i64, i64* %156, align 8
  %158 = mul i64 %157, %155
  %159 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %29, i64 0, i32 2
  %160 = load i64, i64* %159, align 8
  %161 = add i64 %158, %160
  %162 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 14
  %163 = load %struct._PixelPacket*, %struct._PixelPacket** %162, align 8
  %164 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %163, i64 %161
  %165 = icmp eq %struct._PixelPacket* %132, %164
  %166 = zext i1 %165 to i32
  br label %167

167:                                              ; preds = %153, %144
  %168 = phi i64 [ %152, %144 ], [ %161, %153 ]
  %169 = phi i64 [ %148, %144 ], [ %157, %153 ]
  %170 = phi i32 [ 1, %144 ], [ %166, %153 ]
  %171 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 5
  store i32 %170, i32* %171, align 8
  %172 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 7
  %173 = load i64, i64* %172, align 8
  br label %178

174:                                              ; preds = %74, %63
  %175 = phi i16* [ null, %63 ], [ %77, %74 ]
  %176 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 5
  store i32 1, i32* %176, align 8
  %177 = icmp eq %struct._PixelPacket* %67, null
  br i1 %177, label %626, label %178

178:                                              ; preds = %174, %167
  %179 = phi i64 [ %168, %167 ], [ %65, %174 ]
  %180 = phi i16* [ %141, %167 ], [ %175, %174 ]
  %181 = phi i32 [ %170, %167 ], [ 1, %174 ]
  %182 = phi i64 [ %173, %167 ], [ %52, %174 ]
  %183 = phi i64 [ %169, %167 ], [ %46, %174 ]
  %184 = phi %struct._PixelPacket* [ %132, %167 ], [ %68, %174 ]
  %185 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 6
  %186 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 7
  %187 = icmp sgt i64 %179, -1
  br i1 %187, label %188, label %227

188:                                              ; preds = %178
  %189 = mul i64 %182, %183
  %190 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %29, i64 0, i32 1
  %191 = load i64, i64* %190, align 8
  %192 = add i64 %191, -1
  %193 = mul i64 %192, %183
  %194 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %29, i64 0, i32 0
  %195 = load i64, i64* %194, align 8
  %196 = add nsw i64 %179, -1
  %197 = add i64 %196, %195
  %198 = add i64 %197, %193
  %199 = icmp ult i64 %198, %189
  %200 = icmp sgt i64 %2, -1
  %201 = and i1 %199, %200
  br i1 %201, label %202, label %227

202:                                              ; preds = %188
  %203 = add i64 %4, %2
  %204 = icmp sgt i64 %203, %183
  %205 = icmp slt i64 %3, 0
  %206 = or i1 %204, %205
  %207 = add i64 %5, %3
  %208 = icmp sgt i64 %207, %182
  %209 = select i1 %206, i1 true, i1 %208
  br i1 %209, label %227, label %210

210:                                              ; preds = %202
  %211 = icmp eq i32 %181, 0
  br i1 %211, label %212, label %626

212:                                              ; preds = %210
  %213 = tail call fastcc i32 @ReadPixelCachePixels(%struct._CacheInfo* noundef nonnull %15, %struct._NexusInfo* noundef nonnull %6, %struct._ExceptionInfo* noundef %7)
  %214 = icmp eq i32 %213, 0
  br i1 %214, label %626, label %215

215:                                              ; preds = %212
  %216 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 0
  %217 = load i32, i32* %216, align 8
  %218 = icmp eq i32 %217, 2
  br i1 %218, label %223, label %219

219:                                              ; preds = %215
  %220 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 1
  %221 = load i32, i32* %220, align 4
  %222 = icmp eq i32 %221, 12
  br i1 %222, label %223, label %226

223:                                              ; preds = %219, %215
  %224 = tail call fastcc i32 @ReadPixelCacheIndexes(%struct._CacheInfo* noundef nonnull %15, %struct._NexusInfo* noundef nonnull %6, %struct._ExceptionInfo* noundef %7)
  %225 = icmp eq i32 %224, 0
  br i1 %225, label %626, label %226

226:                                              ; preds = %223, %219
  br label %626

227:                                              ; preds = %202, %188, %178
  %228 = tail call %struct._NexusInfo** @AcquirePixelCacheNexus(i64 noundef 1)
  switch i32 %1, label %249 [
    i32 10, label %229
    i32 11, label %234
    i32 8, label %239
    i32 9, label %244
    i32 12, label %244
  ]

229:                                              ; preds = %227
  %230 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  store i16 0, i16* %230, align 2
  %231 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  store i16 0, i16* %231, align 2
  %232 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0
  store i16 0, i16* %232, align 2
  %233 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  store i16 0, i16* %233, align 2
  br label %263

234:                                              ; preds = %227
  %235 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  store i16 32767, i16* %235, align 2
  %236 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  store i16 32767, i16* %236, align 2
  %237 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0
  store i16 32767, i16* %237, align 2
  %238 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  store i16 0, i16* %238, align 2
  br label %263

239:                                              ; preds = %227
  %240 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  store i16 0, i16* %240, align 2
  %241 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  store i16 0, i16* %241, align 2
  %242 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0
  store i16 0, i16* %242, align 2
  %243 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  store i16 -1, i16* %243, align 2
  br label %263

244:                                              ; preds = %227, %227
  %245 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  store i16 -1, i16* %245, align 2
  %246 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  store i16 -1, i16* %246, align 2
  %247 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0 
  store i16 -1, i16* %247, align 2
  %248 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  store i16 0, i16* %248, align 2
  br label %263

249:                                              ; preds = %227
  %250 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 12
  %251 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %250, i64 0, i32 0
  %252 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0
  %253 = load i16, i16* %251, align 2
  store i16 %253, i16* %252, align 2
  %254 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %250, i64 0, i32 1
  %255 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  %256 = load i16, i16* %254, align 2
  store i16 %256, i16* %255, align 2
  %257 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %250, i64 0, i32 2
  %258 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  %259 = load i16, i16* %257, align 2
  store i16 %259, i16* %258, align 2
  %260 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %250, i64 0, i32 3
  %261 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  %262 = load i16, i16* %260, align 2
  store i16 %262, i16* %261, align 2
  br label %263

263:                                              ; preds = %249, %244, %239, %234, %229
  store i16 0, i16* %9, align 2
  %264 = icmp sgt i64 %5, 0
  br i1 %264, label %265, label %601

265:                                              ; preds = %263
  %266 = and i32 %1, -5
  %267 = icmp eq i32 %266, 0
  %268 = icmp sgt i64 %4, 0
  %269 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 0
  %270 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 21
  br i1 %268, label %271, label %601

271:                                              ; preds = %596, %265
  %272 = phi i16* [ %598, %596 ], [ %180, %265 ]
  %273 = phi %struct._PixelPacket* [ %597, %596 ], [ %184, %265 ]
  %274 = phi i64 [ %599, %596 ], [ 0, %265 ]
  %275 = add nsw i64 %274, %3
  br i1 %267, label %276, label %283

276:                                              ; preds = %271
  %277 = load i64, i64* %186, align 8
  %278 = icmp slt i64 %275, 0
  %279 = icmp slt i64 %275, %277
  %280 = add i64 %277, -1
  %281 = select i1 %279, i64 %275, i64 %280
  %282 = select i1 %278, i64 0, i64 %281
  br label %283

283:                                              ; preds = %276, %271
  %284 = phi i64 [ %282, %276 ], [ %275, %271 ]
  %285 = icmp slt i64 %284, 0
  %286 = ashr i64 %284, 63
  %287 = lshr i64 %284, 63
  %288 = and i64 %284, 7
  %289 = getelementptr inbounds [64 x i64], [64 x i64]* @DitherMatrix, i64 0, i64 %288
  %290 = add i64 %284, -32
  br label %291

291:                                              ; preds = %590, %283
  %292 = phi i16* [ %272, %283 ], [ %592, %590 ]
  %293 = phi %struct._PixelPacket* [ %273, %283 ], [ %591, %590 ]
  %294 = phi i64 [ 0, %283 ], [ %594, %590 ]
  %295 = add nsw i64 %294, %2
  %296 = load i64, i64* %185, align 8
  %297 = sub i64 %296, %295
  %298 = sub i64 %4, %294
  %299 = icmp ult i64 %297, %298
  %300 = select i1 %299, i64 %297, i64 %298
  %301 = icmp slt i64 %295, 0
  %302 = icmp sle i64 %296, %295
  %303 = select i1 %301, i1 true, i1 %302
  %304 = select i1 %303, i1 true, i1 %285
  br i1 %304, label %338, label %305

305:                                              ; preds = %291
  %306 = load i64, i64* %186, align 8
  %307 = icmp sge i64 %284, %306
  %308 = icmp eq i64 %300, 0
  %309 = select i1 %307, i1 true, i1 %308
  br i1 %309, label %338, label %310

310:                                              ; preds = %305
  %311 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %312 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef %1, i64 noundef %295, i64 noundef %284, i64 noundef %300, i64 noundef 1, %struct._NexusInfo* noundef %311, %struct._ExceptionInfo* noundef %7) #70
  %313 = icmp eq %struct._PixelPacket* %312, null
  br i1 %313, label %596, label %314

314:                                              ; preds = %310
  %315 = load i32, i32* %269, align 8
  %316 = icmp eq i32 %315, 0
  br i1 %316, label %333, label %317

317:                                              ; preds = %314
  %318 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %319 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %318, i64 0, i32 6
  %320 = load i16*, i16** %319, align 8
  %321 = bitcast %struct._PixelPacket* %293 to i8*
  %322 = bitcast %struct._PixelPacket* %312 to i8*
  %323 = shl i64 %300, 3
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 %321, i8* nonnull align 2 %322, i64 %323, i1 false)
  %324 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %293, i64 %300
  %325 = icmp ne i16* %292, null
  %326 = icmp ne i16* %320, null
  %327 = select i1 %325, i1 %326, i1 false
  br i1 %327, label %328, label %590

328:                                              ; preds = %317
  %329 = bitcast i16* %292 to i8*
  %330 = bitcast i16* %320 to i8*
  %331 = shl i64 %300, 1
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 2 %329, i8* nonnull align 2 %330, i64 %331, i1 false)
  %332 = getelementptr inbounds i16, i16* %292, i64 %300
  br label %590

333:                                              ; preds = %314
  %334 = bitcast %struct._PixelPacket* %293 to i8*
  %335 = bitcast %struct._PixelPacket* %312 to i8*
  %336 = shl i64 %300, 3
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 %334, i8* nonnull align 2 %335, i64 %336, i1 false)
  %337 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %293, i64 %300
  br label %590

338:                                              ; preds = %305, %291
  switch i32 %1, label %546 [
    i32 1, label %568
    i32 2, label %568
    i32 10, label %568
    i32 11, label %568
    i32 8, label %568
    i32 9, label %568
    i32 12, label %568
    i32 16, label %528
    i32 6, label %501
    i32 3, label %474
    i32 7, label %455
    i32 5, label %426
    i32 17, label %403
    i32 13, label %381
    i32 14, label %358
    i32 15, label %339
  ]

339:                                              ; preds = %338
  %340 = sdiv i64 %295, %296
  %341 = ashr i64 %295, 63
  %342 = add nsw i64 %340, %341
  %343 = mul nsw i64 %342, %296
  %344 = sub nsw i64 %295, %343
  %345 = load i64, i64* %186, align 8
  %346 = icmp slt i64 %284, %345
  %347 = add i64 %345, -1
  %348 = select i1 %346, i64 %284, i64 %347
  %349 = select i1 %285, i64 0, i64 %348
  %350 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %351 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef 15, i64 noundef %344, i64 noundef %349, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %350, %struct._ExceptionInfo* noundef %7) #70
  %352 = load i32, i32* %269, align 8
  %353 = icmp eq i32 %352, 0
  br i1 %353, label %564, label %354

354:                                              ; preds = %339
  %355 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %356 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %355, i64 0, i32 6
  %357 = load i16*, i16** %356, align 8
  br label %564

358:                                              ; preds = %338
  %359 = xor i1 %301, true
  %360 = icmp sgt i64 %296, %295
  %361 = select i1 %359, i1 %360, i1 false
  br i1 %361, label %362, label %568

362:                                              ; preds = %358
  %363 = sdiv i64 %295, %296
  %364 = lshr i64 %295, 63
  %365 = add nuw nsw i64 %363, %364
  %366 = mul nsw i64 %365, %296
  %367 = sub nsw i64 %295, %366
  %368 = load i64, i64* %186, align 8
  %369 = sdiv i64 %284, %368
  %370 = add nsw i64 %369, %286
  %371 = mul nsw i64 %370, %368
  %372 = sub nsw i64 %284, %371
  %373 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %374 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef 14, i64 noundef %367, i64 noundef %372, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %373, %struct._ExceptionInfo* noundef %7) #70
  %375 = load i32, i32* %269, align 8
  %376 = icmp eq i32 %375, 0
  br i1 %376, label %564, label %377

377:                                              ; preds = %362
  %378 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %379 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %378, i64 0, i32 6
  %380 = load i16*, i16** %379, align 8
  br label %564

381:                                              ; preds = %338
  br i1 %285, label %568, label %382

382:                                              ; preds = %381
  %383 = load i64, i64* %186, align 8
  %384 = icmp slt i64 %284, %383
  br i1 %384, label %385, label %568

385:                                              ; preds = %382
  %386 = sdiv i64 %295, %296
  %387 = ashr i64 %295, 63
  %388 = add nsw i64 %386, %387
  %389 = mul nsw i64 %388, %296
  %390 = sub nsw i64 %295, %389
  %391 = sdiv i64 %284, %383
  %392 = add nuw nsw i64 %391, %287
  %393 = mul nsw i64 %392, %383
  %394 = sub nsw i64 %284, %393
  %395 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %396 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef 13, i64 noundef %390, i64 noundef %394, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %395, %struct._ExceptionInfo* noundef %7) #70
  %397 = load i32, i32* %269, align 8
  %398 = icmp eq i32 %397, 0
  br i1 %398, label %564, label %399

399:                                              ; preds = %385
  %400 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %401 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %400, i64 0, i32 6
  %402 = load i16*, i16** %401, align 8
  br label %564

403:                                              ; preds = %338
  %404 = sdiv i64 %295, %296
  %405 = ashr i64 %295, 63
  %406 = add nsw i64 %404, %405
  %407 = load i64, i64* %186, align 8
  %408 = sdiv i64 %284, %407
  %409 = add nsw i64 %408, %286
  %410 = xor i64 %409, %406
  %411 = and i64 %410, 1
  %412 = icmp eq i64 %411, 0
  br i1 %412, label %413, label %568

413:                                              ; preds = %403
  %414 = mul nsw i64 %409, %407
  %415 = sub nsw i64 %284, %414
  %416 = mul nsw i64 %406, %296
  %417 = sub nsw i64 %295, %416
  %418 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %419 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef 17, i64 noundef %417, i64 noundef %415, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %418, %struct._ExceptionInfo* noundef %7) #70
  %420 = load i32, i32* %269, align 8
  %421 = icmp eq i32 %420, 0
  br i1 %421, label %564, label %422

422:                                              ; preds = %413
  %423 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %424 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %423, i64 0, i32 6
  %425 = load i16*, i16** %424, align 8
  br label %564

426:                                              ; preds = %338
  %427 = sdiv i64 %295, %296
  %428 = ashr i64 %295, 63
  %429 = add nsw i64 %427, %428
  %430 = mul nsw i64 %429, %296
  %431 = sub nsw i64 %295, %430
  %432 = and i64 %429, 1
  %433 = icmp eq i64 %432, 0
  %434 = xor i64 %431, -1
  %435 = add i64 %296, %434
  %436 = select i1 %433, i64 %431, i64 %435
  %437 = load i64, i64* %186, align 8
  %438 = sdiv i64 %284, %437
  %439 = add nsw i64 %438, %286
  %440 = mul nsw i64 %439, %437
  %441 = sub nsw i64 %284, %440
  %442 = and i64 %439, 1
  %443 = icmp eq i64 %442, 0
  %444 = xor i64 %441, -1
  %445 = add i64 %437, %444
  %446 = select i1 %443, i64 %441, i64 %445
  %447 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %448 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef 5, i64 noundef %436, i64 noundef %446, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %447, %struct._ExceptionInfo* noundef %7) #70
  %449 = load i32, i32* %269, align 8
  %450 = icmp eq i32 %449, 0
  br i1 %450, label %564, label %451

451:                                              ; preds = %426
  %452 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %453 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %452, i64 0, i32 6
  %454 = load i16*, i16** %453, align 8
  br label %564

455:                                              ; preds = %338
  %456 = sdiv i64 %295, %296
  %457 = ashr i64 %295, 63
  %458 = add nsw i64 %456, %457
  %459 = mul nsw i64 %458, %296
  %460 = sub nsw i64 %295, %459
  %461 = load i64, i64* %186, align 8
  %462 = sdiv i64 %284, %461
  %463 = add nsw i64 %462, %286
  %464 = mul nsw i64 %463, %461
  %465 = sub nsw i64 %284, %464
  %466 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %467 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef 7, i64 noundef %460, i64 noundef %465, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %466, %struct._ExceptionInfo* noundef %7) #70
  %468 = load i32, i32* %269, align 8
  %469 = icmp eq i32 %468, 0
  br i1 %469, label %564, label %470

470:                                              ; preds = %455
  %471 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %472 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %471, i64 0, i32 6
  %473 = load i16*, i16** %472, align 8
  br label %564

474:                                              ; preds = %338
  %475 = and i64 %295, 7
  %476 = getelementptr inbounds [64 x i64], [64 x i64]* @DitherMatrix, i64 0, i64 %475
  %477 = load i64, i64* %476, align 8
  %478 = add i64 %295, -32
  %479 = add i64 %478, %477
  %480 = icmp slt i64 %479, 0
  %481 = icmp slt i64 %479, %296
  %482 = add nsw i64 %296, -1
  %483 = select i1 %481, i64 %479, i64 %482
  %484 = select i1 %480, i64 0, i64 %483
  %485 = load i64, i64* %186, align 8
  %486 = load i64, i64* %289, align 8
  %487 = add i64 %290, %486
  %488 = icmp slt i64 %487, 0
  %489 = icmp slt i64 %487, %485
  %490 = add nsw i64 %485, -1
  %491 = select i1 %489, i64 %487, i64 %490
  %492 = select i1 %488, i64 0, i64 %491
  %493 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %494 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef 3, i64 noundef %484, i64 noundef %492, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %493, %struct._ExceptionInfo* noundef %7) #70
  %495 = load i32, i32* %269, align 8
  %496 = icmp eq i32 %495, 0
  br i1 %496, label %564, label %497

497:                                              ; preds = %474
  %498 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %499 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %498, i64 0, i32 6
  %500 = load i16*, i16** %499, align 8
  br label %564

501:                                              ; preds = %338
  %502 = load %struct._RandomInfo*, %struct._RandomInfo** %270, align 8
  %503 = icmp eq %struct._RandomInfo* %502, null
  br i1 %503, label %504, label %507

504:                                              ; preds = %501
  %505 = call %struct._RandomInfo* @AcquireRandomInfo() #62
  store %struct._RandomInfo* %505, %struct._RandomInfo** %270, align 8
  %506 = load i64, i64* %185, align 8
  br label %507

507:                                              ; preds = %504, %501
  %508 = phi i64 [ %506, %504 ], [ %296, %501 ]
  %509 = phi %struct._RandomInfo* [ %505, %504 ], [ %502, %501 ]
  %510 = uitofp i64 %508 to double
  %511 = call fast double @GetPseudoRandomValue(%struct._RandomInfo* noundef %509) #62
  %512 = fmul fast double %511, %510
  %513 = fptosi double %512 to i64
  %514 = load %struct._RandomInfo*, %struct._RandomInfo** %270, align 8
  %515 = load i64, i64* %186, align 8
  %516 = uitofp i64 %515 to double
  %517 = call fast double @GetPseudoRandomValue(%struct._RandomInfo* noundef %514) #62
  %518 = fmul fast double %517, %516
  %519 = fptosi double %518 to i64
  %520 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %521 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef 6, i64 noundef %513, i64 noundef %519, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %520, %struct._ExceptionInfo* noundef %7) #70
  %522 = load i32, i32* %269, align 8
  %523 = icmp eq i32 %522, 0
  br i1 %523, label %564, label %524

524:                                              ; preds = %507
  %525 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %526 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %525, i64 0, i32 6
  %527 = load i16*, i16** %526, align 8
  br label %564

528:                                              ; preds = %338
  %529 = load i64, i64* %186, align 8
  %530 = sdiv i64 %284, %529
  %531 = add nsw i64 %530, %286
  %532 = mul nsw i64 %531, %529
  %533 = sub nsw i64 %284, %532
  %534 = icmp sgt i64 %296, %295
  %535 = add i64 %296, -1
  %536 = select i1 %534, i64 %295, i64 %535
  %537 = select i1 %301, i64 0, i64 %536
  %538 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %539 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef 16, i64 noundef %537, i64 noundef %533, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %538, %struct._ExceptionInfo* noundef %7) #70
  %540 = load i32, i32* %269, align 8
  %541 = icmp eq i32 %540, 0
  br i1 %541, label %564, label %542

542:                                              ; preds = %528
  %543 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %544 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %543, i64 0, i32 6
  %545 = load i16*, i16** %544, align 8
  br label %564

546:                                              ; preds = %338
  %547 = icmp sgt i64 %296, %295
  %548 = add i64 %296, -1
  %549 = select i1 %547, i64 %295, i64 %548
  %550 = select i1 %301, i64 0, i64 %549
  %551 = load i64, i64* %186, align 8
  %552 = icmp slt i64 %284, %551
  %553 = add i64 %551, -1
  %554 = select i1 %552, i64 %284, i64 %553
  %555 = select i1 %285, i64 0, i64 %554
  %556 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %557 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* noundef %0, i32 noundef %1, i64 noundef %550, i64 noundef %555, i64 noundef 1, i64 noundef 1, %struct._NexusInfo* noundef %556, %struct._ExceptionInfo* noundef %7) #70
  %558 = load i32, i32* %269, align 8
  %559 = icmp eq i32 %558, 0
  br i1 %559, label %564, label %560

560:                                              ; preds = %546
  %561 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %562 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %561, i64 0, i32 6
  %563 = load i16*, i16** %562, align 8
  br label %564

564:                                              ; preds = %560, %546, %542, %528, %524, %507, %497, %474, %470, %455, %451, %426, %422, %413, %399, %385, %377, %362, %354, %339
  %565 = phi %struct._PixelPacket* [ %557, %546 ], [ %557, %560 ], [ %521, %507 ], [ %521, %524 ], [ %494, %474 ], [ %494, %497 ], [ %467, %455 ], [ %467, %470 ], [ %448, %426 ], [ %448, %451 ], [ %419, %413 ], [ %419, %422 ], [ %396, %385 ], [ %396, %399 ], [ %374, %362 ], [ %374, %377 ], [ %351, %339 ], [ %351, %354 ], [ %539, %528 ], [ %539, %542 ]
  %566 = phi i16* [ null, %546 ], [ %563, %560 ], [ null, %507 ], [ %527, %524 ], [ null, %474 ], [ %500, %497 ], [ null, %455 ], [ %473, %470 ], [ null, %426 ], [ %454, %451 ], [ null, %413 ], [ %425, %422 ], [ null, %385 ], [ %402, %399 ], [ null, %362 ], [ %380, %377 ], [ null, %339 ], [ %357, %354 ], [ null, %528 ], [ %545, %542 ]
  %567 = icmp eq %struct._PixelPacket* %565, null
  br i1 %567, label %596, label %568

568:                                              ; preds = %564, %403, %382, %381, %358, %338, %338, %338, %338, %338, %338, %338
  %569 = phi i16* [ %566, %564 ], [ %9, %338 ], [ %9, %338 ], [ %9, %338 ], [ %9, %338 ], [ %9, %338 ], [ %9, %338 ], [ %9, %338 ], [ %9, %403 ], [ %9, %382 ], [ %9, %381 ], [ %9, %358 ]
  %570 = phi %struct._PixelPacket* [ %565, %564 ], [ %10, %338 ], [ %10, %338 ], [ %10, %338 ], [ %10, %338 ], [ %10, %338 ], [ %10, %338 ], [ %10, %338 ], [ %10, %403 ], [ %10, %382 ], [ %10, %381 ], [ %10, %358 ]
  %571 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %293, i64 1
  %572 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %570, i64 0, i32 0
  %573 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %293, i64 0, i32 0
  %574 = load i16, i16* %572, align 2
  store i16 %574, i16* %573, align 2
  %575 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %570, i64 0, i32 1
  %576 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %293, i64 0, i32 1
  %577 = load i16, i16* %575, align 2
  store i16 %577, i16* %576, align 2
  %578 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %570, i64 0, i32 2
  %579 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %293, i64 0, i32 2
  %580 = load i16, i16* %578, align 2
  store i16 %580, i16* %579, align 2
  %581 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %570, i64 0, i32 3
  %582 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %293, i64 0, i32 3
  %583 = load i16, i16* %581, align 2
  store i16 %583, i16* %582, align 2
  %584 = icmp ne i16* %292, null
  %585 = icmp ne i16* %569, null
  %586 = select i1 %584, i1 %585, i1 false
  br i1 %586, label %587, label %590

587:                                              ; preds = %568
  %588 = load i16, i16* %569, align 2
  %589 = getelementptr inbounds i16, i16* %292, i64 1
  store i16 %588, i16* %292, align 2
  br label %590

590:                                              ; preds = %587, %568, %333, %328, %317
  %591 = phi %struct._PixelPacket* [ %324, %317 ], [ %324, %328 ], [ %571, %568 ], [ %571, %587 ], [ %337, %333 ]
  %592 = phi i16* [ %292, %317 ], [ %332, %328 ], [ %292, %568 ], [ %589, %587 ], [ %292, %333 ]
  %593 = phi i64 [ %300, %317 ], [ %300, %328 ], [ 1, %568 ], [ 1, %587 ], [ %300, %333 ]
  %594 = add i64 %593, %294
  %595 = icmp slt i64 %594, %4
  br i1 %595, label %291, label %596

596:                                              ; preds = %590, %564, %310
  %597 = phi %struct._PixelPacket* [ %293, %564 ], [ %293, %310 ], [ %591, %590 ]
  %598 = phi i16* [ %292, %564 ], [ %292, %310 ], [ %592, %590 ]
  %599 = add nuw nsw i64 %274, 1
  %600 = icmp eq i64 %599, %5
  br i1 %600, label %601, label %271

601:                                              ; preds = %596, %265, %263
  %602 = load %struct._NexusInfo*, %struct._NexusInfo** %228, align 8
  %603 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %602, i64 0, i32 3
  %604 = load %struct._PixelPacket*, %struct._PixelPacket** %603, align 8
  %605 = icmp eq %struct._PixelPacket* %604, null
  %606 = bitcast %struct._PixelPacket* %604 to i8*
  %607 = bitcast %struct._NexusInfo* %602 to i8*
  br i1 %605, label %621, label %608

608:                                              ; preds = %601
  %609 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %602, i64 0, i32 0
  %610 = load i32, i32* %609, align 8
  %611 = icmp eq i32 %610, 0
  br i1 %611, label %612, label %614

612:                                              ; preds = %608
  %613 = call i8* @RelinquishAlignedMemory(i8* noundef nonnull %606) #62
  br label %617

614:                                              ; preds = %608
  %615 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %602, i64 0, i32 2
  %616 = load i64, i64* %615, align 8
  br label %617

617:                                              ; preds = %614, %612
  store %struct._PixelPacket* null, %struct._PixelPacket** %603, align 8
  %618 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %602, i64 0, i32 4
  store %struct._PixelPacket* null, %struct._PixelPacket** %618, align 8
  %619 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %602, i64 0, i32 6
  store i16* null, i16** %619, align 8
  %620 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %602, i64 0, i32 2
  store i64 0, i64* %620, align 8
  store i32 0, i32* %609, align 8
  br label %621

621:                                              ; preds = %617, %601
  %622 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %602, i64 0, i32 7
  store i64 -2880220588, i64* %622, align 8
  %623 = call i8* @RelinquishMagickMemory(i8* noundef nonnull %607) #62
  store %struct._NexusInfo* null, %struct._NexusInfo** %228, align 8
  %624 = bitcast %struct._NexusInfo** %228 to i8*
  %625 = call i8* @RelinquishAlignedMemory(i8* noundef nonnull %624) #62
  br label %626

626:                                              ; preds = %621, %226, %223, %212, %210, %174, %127, %101, %8
  %627 = phi %struct._PixelPacket* [ %184, %621 ], [ null, %8 ], [ null, %174 ], [ %184, %226 ], [ %184, %210 ], [ null, %212 ], [ null, %223 ], [ null, %101 ], [ null, %127 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %12) #62
  call void @llvm.lifetime.end.p0i8(i64 2, i8* nonnull %11) #62
  ret %struct._PixelPacket* %627
}
; end INTEL_FEATURE_SW_ADVANCED
