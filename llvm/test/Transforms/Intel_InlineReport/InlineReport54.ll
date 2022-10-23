; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-META

; Test for inlining part of may recursive call inlining and splitting

; CHECK-OLD: COMPILE FUNC: GetVirtualPixelsFromNexus.split.0
; CHECK-OLD: COMPILE FUNC: GetVirtualPixelsFromNexus.split.1
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-OLD: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable
; CHECK-OLD: COMPILE FUNC: GetOneCacheViewVirtualPixel
; CHECK-OLD: INLINE: GetVirtualPixelsFromNexus.split.0 {{.*}}Callsite inlined for many recursive calls splitting
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable
; CHECK-OLD: COMPILE FUNC: MeanShiftImage
; CHECK-OLD: INLINE: GetOneCacheViewVirtualPixel {{.*}}Callsite inlined for many recursive calls splitting
; CHECK-OLD: INLINE: GetVirtualPixelsFromNexus.split.0 {{.*}}Callsite inlined for many recursive calls splitting
; CHECK-OLD: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable

; CHECK-META: COMPILE FUNC: MeanShiftImage
; CHECK-META: INLINE: GetOneCacheViewVirtualPixel {{.*}}Callsite inlined for many recursive calls splitting
; CHECK-META: INLINE: GetVirtualPixelsFromNexus.split.0 {{.*}}Callsite inlined for many recursive calls splitting
; CHECK-META: COMPILE FUNC: GetOneCacheViewVirtualPixel
; CHECK-META: INLINE: GetVirtualPixelsFromNexus.split.0 {{.*}}Callsite inlined for many recursive calls splitting
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable
; CHECK-META: COMPILE FUNC: GetVirtualPixelsFromNexus.split.0
; CHECK-META: COMPILE FUNC: GetVirtualPixelsFromNexus.split.1
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-META: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-META: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable

; CHECK: define {{.*}} @MeanShiftImage
; CHECK-NOT: call {{.*}} @GetOneCacheViewVirtualPixel
; CHECK-NOT: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK-LABEL: define {{.*}} @GetOneCacheViewVirtualPixel
; CHECK-NOT: call {{.*}} GetVirtualPixelsFromNexus.split.0
; CHECK-LABEL: define {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK-LABEL: define {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.0
; CHECK: call {{.*}} @GetVirtualPixelsFromNexus.split.1

; CHECK-CL: COMPILE FUNC: GetVirtualPixelsFromNexus.split.0
; CHECK-CL: COMPILE FUNC: GetVirtualPixelsFromNexus.split.1
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Callee has recursion
; CHECK-CL: GetVirtualPixelsFromNexus.split.0 {{.*}}Inlining is not profitable
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable
; CHECK-CL: COMPILE FUNC: GetOneCacheViewVirtualPixel
; CHECK-CL: INLINE: GetVirtualPixelsFromNexus.split.0 {{.*}}Callsite inlined for many recursive calls splitting
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable
; CHECK-CL: COMPILE FUNC: MeanShiftImage
; CHECK-CL: INLINE: GetOneCacheViewVirtualPixel {{.*}}Callsite inlined for many recursive calls splitting
; CHECK-CL: INLINE: GetVirtualPixelsFromNexus.split.0 {{.*}}Callsite inlined for many recursive calls splitting
; CHECK-CL: GetVirtualPixelsFromNexus.split.1 {{.*}}Inlining is not profitable

target triple = "x86_64-unknown-linux-gnu"
%struct._ExceptionInfo = type { i32, i32, i8*, i8*, i8*, i32, %struct.SemaphoreInfo*, i64 }
%struct.SemaphoreInfo = type { i64, i32, i64, i64 }
%struct._CacheInfo = type { i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, i32, %struct._MagickPixelPacket, i64, %struct._NexusInfo**, %struct._PixelPacket*, i16*, i32, i32, [4096 x i8], [4096 x i8], %struct._CacheMethods, %struct._RandomInfo*, i64, i8*, i32, i32, i32, i64, %struct.SemaphoreInfo*, %struct.SemaphoreInfo*, i64, i64 }
%struct._MagickPixelPacket = type { i32, i32, i32, double, i64, float, float, float, float, float }
%struct._PixelPacket = type { i16, i16, i16, i16 }
%struct._CacheMethods = type { %struct._PixelPacket* (%struct._Image*, i32, i64, i64, i64, i64, %struct._ExceptionInfo*)*, %struct._PixelPacket* (%struct._Image*)*, i16* (%struct._Image*)*, i32 (%struct._Image*, i32, i64, i64, %struct._PixelPacket*, %struct._ExceptionInfo*)*, %struct._PixelPacket* (%struct._Image*, i64, i64, i64, i64, %struct._ExceptionInfo*)*, i16* (%struct._Image*)*, i32 (%struct._Image*, i64, i64, %struct._PixelPacket*, %struct._ExceptionInfo*)*, %struct._PixelPacket* (%struct._Image*)*, %struct._PixelPacket* (%struct._Image*, i64, i64, i64, i64, %struct._ExceptionInfo*)*, i32 (%struct._Image*, %struct._ExceptionInfo*)*, void (%struct._Image*)* }
%struct._Image = type { i32, i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, %struct._PixelPacket*, %struct._PixelPacket, %struct._PixelPacket, %struct._PixelPacket, double, %struct._ChromaticityInfo, i32, i8*, i32, i8*, i8*, i8*, i64, double, double, %struct._RectangleInfo, %struct._RectangleInfo, %struct._RectangleInfo, double, double, double, i32, i32, i32, i32, i32, i32, %struct._Image*, i64, i64, i64, i64, i64, i64, %struct._ErrorInfo, %struct._TimerInfo, i32 (i8*, i64, i64, i8*)*, i8*, i8*, i8*, %struct._Ascii85Info*, %struct._BlobInfo*, [4096 x i8], [4096 x i8], [4096 x i8], i64, i64, %struct._ExceptionInfo, i32, i64, %struct.SemaphoreInfo*, %struct._ProfileInfo, %struct._ProfileInfo, %struct._ProfileInfo*, i64, i64, %struct._Image*, %struct._Image*, %struct._Image*, i32, i32, %struct._PixelPacket, %struct._Image*, %struct._RectangleInfo, i8*, i8*, i32, i32, i64, i32, i64, i64, i32, i64 }
%struct._ChromaticityInfo = type { %struct._PrimaryInfo, %struct._PrimaryInfo, %struct._PrimaryInfo, %struct._PrimaryInfo }
%struct._PrimaryInfo = type { double, double, double }
%struct._ErrorInfo = type { double, double, double }
%struct._TimerInfo = type { %struct._Timer, %struct._Timer, i32, i64 }
%struct._Timer = type { double, double, double }
%struct._Ascii85Info = type { i64, i64, [10 x i8] }
%struct._BlobInfo = type { i64, i64, i64, i32, i32, i64, i64, i32, i32, i32, i32, i32, %union.FileInfo, %struct.stat, i64 (%struct._Image*, i8*, i64)*, i8*, i32, %struct.SemaphoreInfo*, i64, i64 }
%union.FileInfo = type { %struct._IO_FILE* }
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.stat = type { i64, i64, i64, i32, i32, i32, i32, i64, i64, i64, i64, %struct.timespec, %struct.timespec, %struct.timespec, [3 x i64] }
%struct.timespec = type { i64, i64 }
%struct._ProfileInfo = type { i8*, i64, i8*, i64 }
%struct._RectangleInfo = type { i64, i64, i64, i64 }
%struct._RandomInfo = type { %struct._SignatureInfo.948*, %struct._StringInfo*, %struct._StringInfo*, i64, [4 x i64], double, i64, i16, i16, %struct.SemaphoreInfo*, i64, i64 }
%struct._SignatureInfo.948 = type { i32, i32, %struct._StringInfo*, %struct._StringInfo*, i32*, i32, i32, i64, i32, i64, i64 }
%struct._StringInfo = type { [4096 x i8], i8*, i64, i64 }
%struct._NexusInfo = type { i32, %struct._RectangleInfo, i64, %struct._PixelPacket*, %struct._PixelPacket*, i32, i16*, i64 }
%struct._CacheView = type { %struct._Image*, i32, i64, %struct._NexusInfo**, i32, i64 }

@.str.129 = private unnamed_addr constant [15 x i8] c"magick/cache.c\00", align 1
@.str.1.130 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str.2.131 = private unnamed_addr constant [23 x i8] c"MemoryAllocationFailed\00", align 1
@.str.3.132 = private unnamed_addr constant [5 x i8] c"`%s'\00", align 1
@.str.7.170 = private unnamed_addr constant [22 x i8] c"UnableToGetCacheNexus\00", align 1
@.str.17.1479 = private unnamed_addr constant [16 x i8] c"MeanShift/Image\00", align 1
@.str.18.1467 = private unnamed_addr constant [6 x i8] c"%s/%s\00", align 1
@DitherMatrix = internal unnamed_addr constant [64 x i64] [i64 0, i64 48, i64 12, i64 60, i64 3, i64 51, i64 15, i64 63, i64 32, i64 16, i64 44, i64 28, i64 35, i64 19, i64 47, i64 31, i64 8, i64 56, i64 4, i64 52, i64 11, i64 59, i64 7, i64 55, i64 40, i64 24, i64 36, i64 20, i64 43, i64 27, i64 39, i64 23, i64 2, i64 50, i64 14, i64 62, i64 1, i64 49, i64 13, i64 61, i64 34, i64 18, i64 46, i64 30, i64 33, i64 17, i64 45, i64 29, i64 10, i64 58, i64 6, i64 54, i64 9, i64 57, i64 5, i64 53, i64 42, i64 26, i64 38, i64 22, i64 41, i64 25, i64 37, i64 21], align 16

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0

declare i8* @AcquireAlignedMemory(i64, i64)

declare i32 @ThrowMagickException(%struct._ExceptionInfo* nocapture, i8*, i8*, i64, i32, i8*, i8* nocapture readonly, ...)

declare noalias i8* @RelinquishAlignedMemory(i8* readonly)

declare fastcc i32 @ReadPixelCachePixels(%struct._CacheInfo* noalias, %struct._NexusInfo* noalias nocapture readonly, %struct._ExceptionInfo*)

declare fastcc i32 @ReadPixelCacheIndexes(%struct._CacheInfo* noalias, %struct._NexusInfo* noalias nocapture readonly, %struct._ExceptionInfo*)

declare %struct._NexusInfo** @AcquirePixelCacheNexus(i64)

declare %struct._RandomInfo* @AcquireRandomInfo()

declare double @GetPseudoRandomValue(%struct._RandomInfo* nocapture)

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #0

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0

declare noalias i8* @RelinquishMagickMemory(i8*)

declare %struct._Image* @CloneImage(%struct._Image*, i64, i64, i32, %struct._ExceptionInfo*)

declare i32 @SetImageStorageClass(%struct._Image*, i32)

declare void @InheritException(%struct._ExceptionInfo* nocapture, %struct._ExceptionInfo* nocapture readonly)

declare %struct._CacheView* @AcquireVirtualCacheView(%struct._Image*, %struct._ExceptionInfo* nocapture readnone)

declare %struct._CacheView* @AcquireAuthenticCacheView(%struct._Image*, %struct._ExceptionInfo*)

declare %struct._PixelPacket* @GetCacheViewVirtualPixels(%struct._CacheView* nocapture readonly, i64, i64, i64, i64, %struct._ExceptionInfo*)

declare %struct._PixelPacket* @GetCacheViewAuthenticPixels(%struct._CacheView* nocapture readonly, i64, i64, i64, i64, %struct._ExceptionInfo*)

declare i16* @GetCacheViewVirtualIndexQueue(%struct._CacheView* nocapture readonly)

declare void @GetMagickPixelPacket(%struct._Image* readonly, %struct._MagickPixelPacket* nocapture)

; Function Attrs: nounwind readnone speculatable willreturn
declare double @llvm.rint.f64(double) #1

declare i32 @SyncCacheViewAuthenticPixels(%struct._CacheView* noalias nocapture readonly, %struct._ExceptionInfo*)

declare i64 @FormatLocaleString(i8* noalias nocapture, i64, i8* noalias nocapture readonly, ...)

declare %struct._CacheView* @DestroyCacheView(%struct._CacheView*)

declare %struct._Image* @DestroyImage(%struct._Image*)

define %struct._Image* @MeanShiftImage(%struct._Image* %0, i64 %1, i64 %2, double %3, %struct._ExceptionInfo* %4) {
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
  %21 = tail call %struct._Image* @CloneImage(%struct._Image* nonnull %0, i64 %18, i64 %20, i32 1, %struct._ExceptionInfo* %4)
  %22 = icmp eq %struct._Image* %21, null
  br i1 %22, label %309, label %23

23:                                               ; preds = %16
  %24 = tail call i32 @SetImageStorageClass(%struct._Image* nonnull %21, i32 1)
  %25 = icmp eq i32 %24, 0
  br i1 %25, label %26, label %29

26:                                               ; preds = %23
  %27 = getelementptr inbounds %struct._Image, %struct._Image* %21, i64 0, i32 58
  tail call void @InheritException(%struct._ExceptionInfo* %4, %struct._ExceptionInfo* nonnull %27)
  %28 = tail call %struct._Image* @DestroyImage(%struct._Image* nonnull %21)
  br label %309

29:                                               ; preds = %23
  %30 = tail call %struct._CacheView* @AcquireVirtualCacheView(%struct._Image* nonnull %0, %struct._ExceptionInfo* %4)
  %31 = tail call %struct._CacheView* @AcquireVirtualCacheView(%struct._Image* nonnull %0, %struct._ExceptionInfo* %4)
  %32 = tail call %struct._CacheView* @AcquireAuthenticCacheView(%struct._Image* nonnull %21, %struct._ExceptionInfo* %4)
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
  %79 = call %struct._PixelPacket* @GetCacheViewVirtualPixels(%struct._CacheView* %30, i64 0, i64 %75, i64 %78, i64 1, %struct._ExceptionInfo* %4)
  %80 = load i64, i64* %37, align 8
  %81 = call %struct._PixelPacket* @GetCacheViewAuthenticPixels(%struct._CacheView* %32, i64 0, i64 %75, i64 %80, i64 1, %struct._ExceptionInfo* %4)
  %82 = icmp eq %struct._PixelPacket* %79, null
  %83 = icmp eq %struct._PixelPacket* %81, null
  %84 = or i1 %82, %83
  br i1 %84, label %299, label %85

85:                                               ; preds = %77
  %86 = call i16* @GetCacheViewVirtualIndexQueue(%struct._CacheView* %30)
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
  call void @llvm.lifetime.start.p0i8(i64 56, i8* nonnull %38)
  call void @GetMagickPixelPacket(%struct._Image* %0, %struct._MagickPixelPacket* nonnull %7)
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
  call void @llvm.lifetime.start.p0i8(i64 56, i8* nonnull %45)
  call void @GetMagickPixelPacket(%struct._Image* %0, %struct._MagickPixelPacket* nonnull %8)
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
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %55)
  %154 = add i64 %145, %131
  %155 = call i32 @GetOneCacheViewVirtualPixel(%struct._CacheView* %31, i64 %154, i64 %141, %struct._PixelPacket* nonnull %9, %struct._ExceptionInfo* %4) #2
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
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %55)
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
  call void @llvm.lifetime.end.p0i8(i64 56, i8* nonnull %45)
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
  call void @llvm.lifetime.end.p0i8(i64 56, i8* nonnull %38)
  %280 = add nuw nsw i64 %94, 1
  %281 = load i64, i64* %37, align 8
  %282 = icmp slt i64 %280, %281
  br i1 %282, label %92, label %283

283:                                              ; preds = %249, %85
  %284 = phi i32 [ 1, %85 ], [ %206, %249 ]
  %285 = call i32 @SyncCacheViewAuthenticPixels(%struct._CacheView* %32, %struct._ExceptionInfo* %4)
  %286 = icmp eq i32 %285, 0
  %287 = select i1 %286, i32 0, i32 %284
  %288 = load i32 (i8*, i64, i64, i8*)*, i32 (i8*, i64, i64, i8*)** %65, align 8
  %289 = icmp eq i32 (i8*, i64, i64, i8*)* %288, null
  br i1 %289, label %299, label %290

290:                                              ; preds = %283
  %291 = add nsw i64 %74, 1
  %292 = load i64, i64* %19, align 8
  call void @llvm.lifetime.start.p0i8(i64 4096, i8* nonnull %66)
  %293 = call i64 (i8*, i64, i8*, ...) @FormatLocaleString(i8* nonnull %66, i64 4096, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.18.1467, i64 0, i64 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.17.1479, i64 0, i64 0), i8* nonnull %68)
  %294 = load i32 (i8*, i64, i64, i8*)*, i32 (i8*, i64, i64, i8*)** %65, align 8
  %295 = load i8*, i8** %69, align 8
  %296 = call i32 %294(i8* nonnull %66, i64 %74, i64 %292, i8* %295)
  call void @llvm.lifetime.end.p0i8(i64 4096, i8* nonnull %66)
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
  %306 = call %struct._CacheView* @DestroyCacheView(%struct._CacheView* %32)
  %307 = call %struct._CacheView* @DestroyCacheView(%struct._CacheView* %31)
  %308 = call %struct._CacheView* @DestroyCacheView(%struct._CacheView* %30)
  br label %309

309:                                              ; preds = %305, %26, %16
  %310 = phi %struct._Image* [ null, %26 ], [ %21, %305 ], [ null, %16 ]
  ret %struct._Image* %310
}

define i32 @GetOneCacheViewVirtualPixel(%struct._CacheView* noalias nocapture readonly %0, i64 %1, i64 %2, %struct._PixelPacket* noalias nocapture %3, %struct._ExceptionInfo* %4) {
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
  %26 = alloca i32, align 4
  store i32 0, i32* %26, align 4
  %27 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %7, i32 %22, i64 %1, i64 %2, i64 1, i64 1, %struct._NexusInfo* %25, %struct._ExceptionInfo* %4, i32* %26) #2
  %28 = load i32, i32* %26, align 4
  %29 = icmp eq i32 %28, 1
  br i1 %29, label %30, label %32

30:                                               ; preds = %5
  %31 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %7, i32 %22, i64 %1, i64 %2, i64 1, i64 1, %struct._NexusInfo* %25, %struct._ExceptionInfo* %4, %struct._PixelPacket* %27)
  br label %32

32:                                               ; preds = %5, %30
  %33 = phi %struct._PixelPacket* [ %27, %5 ], [ %31, %30 ]
  %34 = icmp eq %struct._PixelPacket* %33, null
  br i1 %34, label %44, label %35

35:                                               ; preds = %32
  %36 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %33, i64 0, i32 0
  %37 = load i16, i16* %36, align 2
  store i16 %37, i16* %11, align 2
  %38 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %33, i64 0, i32 1
  %39 = load i16, i16* %38, align 2
  store i16 %39, i16* %14, align 2
  %40 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %33, i64 0, i32 2
  %41 = load i16, i16* %40, align 2
  store i16 %41, i16* %17, align 2
  %42 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %33, i64 0, i32 3
  %43 = load i16, i16* %42, align 2
  store i16 %43, i16* %20, align 2
  br label %44

44:                                               ; preds = %35, %32
  %45 = phi i32 [ 1, %35 ], [ 0, %32 ]
  ret i32 %45
}

define %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 %1, i64 %2, i64 %3, i64 %4, i64 %5, %struct._NexusInfo* %6, %struct._ExceptionInfo* %7, i32* %8) {
  %10 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 49
  %11 = bitcast i8** %10 to %struct._CacheInfo**
  %12 = load %struct._CacheInfo*, %struct._CacheInfo** %11, align 8
  %13 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 3
  %14 = load i32, i32* %13, align 8
  %15 = icmp eq i32 %14, 0
  br i1 %15, label %223, label %16

16:                                               ; preds = %9
  %17 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 38
  %18 = load %struct._Image*, %struct._Image** %17, align 8
  %19 = icmp eq %struct._Image* %18, null
  br i1 %19, label %20, label %25

20:                                               ; preds = %16
  %21 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 73
  %22 = load %struct._Image*, %struct._Image** %21, align 8
  %23 = icmp ne %struct._Image* %22, null
  %24 = zext i1 %23 to i32
  br label %25

25:                                               ; preds = %20, %16
  %26 = phi i32 [ 1, %16 ], [ %24, %20 ]
  %27 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1
  %28 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 0
  store i64 %4, i64* %28, align 8
  %29 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 1
  store i64 %5, i64* %29, align 8
  %30 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 2
  store i64 %2, i64* %30, align 8
  %31 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 3
  store i64 %3, i64* %31, align 8
  %32 = load i32, i32* %13, align 8
  %33 = add i32 %32, -1
  %34 = icmp ult i32 %33, 2
  %35 = icmp eq i32 %26, 0
  %36 = and i1 %35, %34
  %37 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %27, i64 0, i32 1
  %38 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %27, i64 0, i32 0
  br i1 %36, label %39, label %80

39:                                               ; preds = %25
  %40 = add nsw i64 %3, %5
  %41 = icmp sgt i64 %2, -1
  br i1 %41, label %42, label %80

42:                                               ; preds = %39
  %43 = add nsw i64 %2, %4
  %44 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 6
  %45 = load i64, i64* %44, align 8
  %46 = icmp sle i64 %43, %45
  %47 = icmp sgt i64 %3, -1
  %48 = and i1 %47, %46
  br i1 %48, label %49, label %80

49:                                               ; preds = %42
  %50 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 7
  %51 = load i64, i64* %50, align 8
  %52 = icmp sgt i64 %40, %51
  br i1 %52, label %80, label %53

53:                                               ; preds = %49
  %54 = icmp eq i64 %5, 1
  br i1 %54, label %62, label %55

55:                                               ; preds = %53
  %56 = icmp eq i64 %2, 0
  br i1 %56, label %57, label %80

57:                                               ; preds = %55
  %58 = icmp eq i64 %45, %4
  br i1 %58, label %62, label %59

59:                                               ; preds = %57
  %60 = urem i64 %4, %45
  %61 = icmp eq i64 %60, 0
  br i1 %61, label %62, label %80

62:                                               ; preds = %59, %57, %53
  %63 = mul i64 %45, %3
  %64 = add i64 %63, %2
  %65 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 14
  %66 = load %struct._PixelPacket*, %struct._PixelPacket** %65, align 8
  %67 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %66, i64 %64
  %68 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 4
  store %struct._PixelPacket* %67, %struct._PixelPacket** %68, align 8
  %69 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  store i16* null, i16** %69, align 8
  %70 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 16
  %71 = load i32, i32* %70, align 8
  %72 = icmp eq i32 %71, 0
  br i1 %72, label %77, label %73

73:                                               ; preds = %62
  %74 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 15
  %75 = load i16*, i16** %74, align 8
  %76 = getelementptr inbounds i16, i16* %75, i64 %64
  store i16* %76, i16** %69, align 8
  br label %77

77:                                               ; preds = %73, %62
  %78 = phi i16* [ %76, %73 ], [ null, %62 ]
  %79 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 5
  store i32 1, i32* %79, align 8
  br label %168

80:                                               ; preds = %59, %55, %49, %42, %39, %25
  %81 = mul i64 %4, %5
  %82 = shl i64 %81, 3
  %83 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 16
  %84 = load i32, i32* %83, align 8
  %85 = icmp eq i32 %84, 0
  %86 = mul i64 %81, 10
  %87 = select i1 %85, i64 %82, i64 %86
  %88 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 3
  %89 = load %struct._PixelPacket*, %struct._PixelPacket** %88, align 8
  %90 = icmp eq %struct._PixelPacket* %89, null
  %91 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 2
  %92 = bitcast %struct._PixelPacket* %89 to i8*
  br i1 %90, label %93, label %109

93:                                               ; preds = %80
  store i64 %87, i64* %91, align 8
  %94 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 0
  store i32 0, i32* %94, align 8
  %95 = tail call i8* @AcquireAlignedMemory(i64 1, i64 %87)
  %96 = bitcast i8* %95 to %struct._PixelPacket*
  store %struct._PixelPacket* %96, %struct._PixelPacket** %88, align 8
  %97 = icmp eq i8* %95, null
  br i1 %97, label %98, label %100

98:                                               ; preds = %93
  store i32 1, i32* %94, align 8
  %99 = load i64, i64* %91, align 8
  store %struct._PixelPacket* null, %struct._PixelPacket** %88, align 8
  br label %100

100:                                              ; preds = %98, %93
  %101 = phi i8* [ null, %98 ], [ %95, %93 ]
  %102 = phi %struct._PixelPacket* [ null, %98 ], [ %96, %93 ]
  %103 = ptrtoint i8* %101 to i64
  %104 = icmp eq %struct._PixelPacket* %102, null
  br i1 %104, label %105, label %136

105:                                              ; preds = %100
  %106 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 18
  %107 = getelementptr inbounds [4096 x i8], [4096 x i8]* %106, i64 0, i64 0
  %108 = tail call i32 (%struct._ExceptionInfo*, i8*, i8*, i64, i32, i8*, i8*, ...) @ThrowMagickException(%struct._ExceptionInfo* %7, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.129, i64 0, i64 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.1.130, i64 0, i64 0), i64 4688, i32 400, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.2.131, i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.3.132, i64 0, i64 0), i8* nonnull %107)
  store i64 0, i64* %91, align 8
  br label %223

109:                                              ; preds = %80
  %110 = ptrtoint %struct._PixelPacket* %89 to i64
  %111 = load i64, i64* %91, align 8
  %112 = icmp ult i64 %111, %87
  br i1 %112, label %113, label %136

113:                                              ; preds = %109
  %114 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 0
  %115 = load i32, i32* %114, align 8
  %116 = icmp eq i32 %115, 0
  br i1 %116, label %117, label %119

117:                                              ; preds = %113
  %118 = tail call i8* @RelinquishAlignedMemory(i8* nonnull %92)
  br label %119

119:                                              ; preds = %113, %117
  store %struct._PixelPacket* null, %struct._PixelPacket** %88, align 8
  %120 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 4
  store %struct._PixelPacket* null, %struct._PixelPacket** %120, align 8
  %121 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  store i16* null, i16** %121, align 8
  store i64 %87, i64* %91, align 8
  store i32 0, i32* %114, align 8
  %122 = tail call i8* @AcquireAlignedMemory(i64 1, i64 %87)
  %123 = bitcast i8* %122 to %struct._PixelPacket*
  store %struct._PixelPacket* %123, %struct._PixelPacket** %88, align 8
  %124 = icmp eq i8* %122, null
  br i1 %124, label %125, label %127

125:                                              ; preds = %119
  store i32 1, i32* %114, align 8
  %126 = load i64, i64* %91, align 8
  store %struct._PixelPacket* null, %struct._PixelPacket** %88, align 8
  br label %127

127:                                              ; preds = %125, %119
  %128 = phi i8* [ null, %125 ], [ %122, %119 ]
  %129 = phi %struct._PixelPacket* [ null, %125 ], [ %123, %119 ]
  %130 = ptrtoint i8* %128 to i64
  %131 = icmp eq %struct._PixelPacket* %129, null
  br i1 %131, label %132, label %136

132:                                              ; preds = %127
  %133 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 18
  %134 = getelementptr inbounds [4096 x i8], [4096 x i8]* %133, i64 0, i64 0
  %135 = tail call i32 (%struct._ExceptionInfo*, i8*, i8*, i64, i32, i8*, i8*, ...) @ThrowMagickException(%struct._ExceptionInfo* %7, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.129, i64 0, i64 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.1.130, i64 0, i64 0), i64 4688, i32 400, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.2.131, i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.3.132, i64 0, i64 0), i8* nonnull %134)
  store i64 0, i64* %91, align 8
  br label %223

136:                                              ; preds = %127, %109, %100
  %137 = phi i64 [ %130, %127 ], [ %103, %100 ], [ %110, %109 ]
  %138 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 4
  %139 = bitcast %struct._PixelPacket** %138 to i64*
  store i64 %137, i64* %139, align 8
  %140 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  store i16* null, i16** %140, align 8
  %141 = load i32, i32* %83, align 8
  %142 = icmp eq i32 %141, 0
  %143 = inttoptr i64 %137 to %struct._PixelPacket*
  br i1 %142, label %147, label %144

144:                                              ; preds = %136
  %145 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %143, i64 %81
  %146 = getelementptr %struct._PixelPacket, %struct._PixelPacket* %145, i64 0, i32 0
  store i16* %146, i16** %140, align 8
  br label %147

147:                                              ; preds = %144, %136
  %148 = phi i16* [ %146, %144 ], [ null, %136 ]
  %149 = load i32, i32* %13, align 8
  %150 = icmp eq i32 %149, 4
  br i1 %150, label %165, label %151

151:                                              ; preds = %147
  %152 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %27, i64 0, i32 3
  %153 = load i64, i64* %152, align 8
  %154 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 6
  %155 = load i64, i64* %154, align 8
  %156 = mul i64 %155, %153
  %157 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %27, i64 0, i32 2
  %158 = load i64, i64* %157, align 8
  %159 = add i64 %156, %158
  %160 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 14
  %161 = load %struct._PixelPacket*, %struct._PixelPacket** %160, align 8
  %162 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %161, i64 %159
  %163 = icmp eq %struct._PixelPacket* %162, %143
  %164 = zext i1 %163 to i32
  br label %165

165:                                              ; preds = %151, %147
  %166 = phi i32 [ %164, %151 ], [ 1, %147 ]
  %167 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 5
  store i32 %166, i32* %167, align 8
  br label %168

168:                                              ; preds = %165, %77
  %169 = phi i16* [ %148, %165 ], [ %78, %77 ]
  %170 = phi i32 [ %166, %165 ], [ 1, %77 ]
  %171 = phi %struct._PixelPacket* [ %143, %165 ], [ %67, %77 ]
  %172 = icmp eq %struct._PixelPacket* %171, null
  br i1 %172, label %223, label %173

173:                                              ; preds = %168
  %174 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %27, i64 0, i32 3
  %175 = load i64, i64* %174, align 8
  %176 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 6
  %177 = load i64, i64* %176, align 8
  %178 = mul i64 %177, %175
  %179 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %27, i64 0, i32 2
  %180 = load i64, i64* %179, align 8
  %181 = add i64 %178, %180
  %182 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 7
  %183 = load i64, i64* %182, align 8
  %184 = icmp sgt i64 %181, -1
  br i1 %184, label %185, label %222

185:                                              ; preds = %173
  %186 = mul i64 %183, %177
  %187 = load i64, i64* %37, align 8
  %188 = add i64 %187, -1
  %189 = mul i64 %188, %177
  %190 = load i64, i64* %38, align 8
  %191 = add nsw i64 %181, -1
  %192 = add i64 %191, %190
  %193 = add i64 %192, %189
  %194 = icmp ult i64 %193, %186
  %195 = icmp sgt i64 %2, -1
  %196 = and i1 %195, %194
  br i1 %196, label %197, label %222

197:                                              ; preds = %185
  %198 = add i64 %4, %2
  %199 = icmp sgt i64 %198, %177
  %200 = icmp slt i64 %3, 0
  %201 = or i1 %200, %199
  %202 = add i64 %5, %3
  %203 = icmp sgt i64 %202, %183
  %204 = or i1 %201, %203
  br i1 %204, label %222, label %205

205:                                              ; preds = %197
  %206 = icmp eq i32 %170, 0
  br i1 %206, label %207, label %223

207:                                              ; preds = %205
  %208 = tail call fastcc i32 @ReadPixelCachePixels(%struct._CacheInfo* nonnull %12, %struct._NexusInfo* nonnull %6, %struct._ExceptionInfo* %7)
  %209 = icmp eq i32 %208, 0
  br i1 %209, label %223, label %210

210:                                              ; preds = %207
  %211 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 0
  %212 = load i32, i32* %211, align 8
  %213 = icmp eq i32 %212, 2
  br i1 %213, label %218, label %214

214:                                              ; preds = %210
  %215 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %12, i64 0, i32 1
  %216 = load i32, i32* %215, align 4
  %217 = icmp eq i32 %216, 12
  br i1 %217, label %218, label %221

218:                                              ; preds = %214, %210
  %219 = tail call fastcc i32 @ReadPixelCacheIndexes(%struct._CacheInfo* nonnull %12, %struct._NexusInfo* nonnull %6, %struct._ExceptionInfo* %7)
  %220 = icmp eq i32 %219, 0
  br i1 %220, label %223, label %221

221:                                              ; preds = %218, %214
  br label %223

222:                                              ; preds = %197, %185, %173
  store i32 1, i32* %8, align 4
  ret %struct._PixelPacket* %171

223:                                              ; preds = %221, %218, %207, %205, %168, %132, %105, %9
  %224 = phi %struct._PixelPacket* [ null, %9 ], [ null, %168 ], [ %171, %221 ], [ %171, %205 ], [ null, %207 ], [ null, %218 ], [ null, %105 ], [ null, %132 ]
  ret %struct._PixelPacket* %224
}

define %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 %1, i64 %2, i64 %3, i64 %4, i64 %5, %struct._NexusInfo* %6, %struct._ExceptionInfo* %7, %struct._PixelPacket* %8) {
  %10 = alloca i16, align 2
  %11 = bitcast i16* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 2, i8* nonnull %11)
  %12 = alloca %struct._PixelPacket, align 2
  %13 = bitcast %struct._PixelPacket* %12 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %13)
  %14 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 49
  %15 = bitcast i8** %14 to %struct._CacheInfo**
  %16 = load %struct._CacheInfo*, %struct._CacheInfo** %15, align 8
  %17 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  %18 = load i16*, i16** %17, align 8
  %19 = getelementptr %struct._CacheInfo, %struct._CacheInfo* %16, i64 0, i32 6
  %20 = getelementptr %struct._CacheInfo, %struct._CacheInfo* %16, i64 0, i32 7
  %21 = tail call %struct._NexusInfo** @AcquirePixelCacheNexus(i64 1)
  %22 = icmp eq %struct._NexusInfo** %21, null
  br i1 %22, label %23, label %27

23:                                               ; preds = %9
  %24 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 53
  %25 = getelementptr inbounds [4096 x i8], [4096 x i8]* %24, i64 0, i64 0
  %26 = tail call i32 (%struct._ExceptionInfo*, i8*, i8*, i64, i32, i8*, i8*, ...) @ThrowMagickException(%struct._ExceptionInfo* %7, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.129, i64 0, i64 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.1.130, i64 0, i64 0), i64 2673, i32 445, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.7.170, i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.3.132, i64 0, i64 0), i8* nonnull %25)
  br label %504

27:                                               ; preds = %9
  switch i32 %1, label %48 [
    i32 10, label %28
    i32 11, label %33
    i32 8, label %38
    i32 9, label %43
    i32 12, label %43
  ]

28:                                               ; preds = %27
  %29 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 2
  store i16 0, i16* %29, align 2
  %30 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 1
  store i16 0, i16* %30, align 2
  %31 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 0
  store i16 0, i16* %31, align 2
  %32 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 3
  store i16 0, i16* %32, align 2
  br label %62

33:                                               ; preds = %27
  %34 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 2
  store i16 32767, i16* %34, align 2
  %35 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 1
  store i16 32767, i16* %35, align 2
  %36 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 0
  store i16 32767, i16* %36, align 2
  %37 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 3
  store i16 0, i16* %37, align 2
  br label %62

38:                                               ; preds = %27
  %39 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 2
  store i16 0, i16* %39, align 2
  %40 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 1
  store i16 0, i16* %40, align 2
  %41 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 0
  store i16 0, i16* %41, align 2
  %42 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 3
  store i16 -1, i16* %42, align 2
  br label %62

43:                                               ; preds = %27, %27
  %44 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 2
  store i16 -1, i16* %44, align 2
  %45 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 1
  store i16 -1, i16* %45, align 2
  %46 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 0
  store i16 -1, i16* %46, align 2
  %47 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 3
  store i16 0, i16* %47, align 2
  br label %62

48:                                               ; preds = %27
  %49 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 12
  %50 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %49, i64 0, i32 0
  %51 = load i16, i16* %50, align 2
  %52 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 0
  store i16 %51, i16* %52, align 2
  %53 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %49, i64 0, i32 1
  %54 = load i16, i16* %53, align 2
  %55 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 1
  store i16 %54, i16* %55, align 2
  %56 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %49, i64 0, i32 2
  %57 = load i16, i16* %56, align 2
  %58 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 2
  store i16 %57, i16* %58, align 2
  %59 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %49, i64 0, i32 3
  %60 = load i16, i16* %59, align 2
  %61 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %12, i64 0, i32 3
  store i16 %60, i16* %61, align 2
  br label %62

62:                                               ; preds = %48, %43, %38, %33, %28
  store i16 0, i16* %10, align 2
  %63 = icmp sgt i64 %5, 0
  br i1 %63, label %64, label %476

64:                                               ; preds = %62
  %65 = and i32 %1, -5
  %66 = icmp eq i32 %65, 0
  %67 = icmp sgt i64 %4, 0
  %68 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %16, i64 0, i32 0
  %69 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %16, i64 0, i32 21
  br label %70

70:                                               ; preds = %471, %64
  %71 = phi i16* [ %18, %64 ], [ %473, %471 ]
  %72 = phi %struct._PixelPacket* [ %8, %64 ], [ %472, %471 ]
  %73 = phi i64 [ 0, %64 ], [ %474, %471 ]
  %74 = add nsw i64 %73, %3
  br i1 %66, label %75, label %82

75:                                               ; preds = %70
  %76 = load i64, i64* %20, align 8
  %77 = icmp slt i64 %74, 0
  %78 = icmp slt i64 %74, %76
  %79 = add i64 %76, -1
  %80 = select i1 %78, i64 %74, i64 %79
  %81 = select i1 %77, i64 0, i64 %80
  br label %82

82:                                               ; preds = %75, %70
  %83 = phi i64 [ %74, %70 ], [ %81, %75 ]
  br i1 %67, label %84, label %471

84:                                               ; preds = %82
  %85 = icmp slt i64 %83, 0
  %86 = ashr i64 %83, 63
  %87 = lshr i64 %83, 63
  %88 = and i64 %83, 7
  %89 = getelementptr inbounds [64 x i64], [64 x i64]* @DitherMatrix, i64 0, i64 %88
  br label %90

90:                                               ; preds = %465, %84
  %91 = phi i16* [ %71, %84 ], [ %467, %465 ]
  %92 = phi %struct._PixelPacket* [ %72, %84 ], [ %468, %465 ]
  %93 = phi i64 [ 0, %84 ], [ %469, %465 ]
  %94 = add nsw i64 %93, %2
  %95 = load i64, i64* %19, align 8
  %96 = sub i64 %95, %94
  %97 = sub i64 %4, %93
  %98 = icmp ult i64 %96, %97
  %99 = select i1 %98, i64 %96, i64 %97
  %100 = icmp slt i64 %94, 0
  %101 = icmp sle i64 %95, %94
  %102 = or i64 %94, %83
  %103 = icmp slt i64 %102, 0
  %104 = or i1 %103, %101
  br i1 %104, label %110, label %105

105:                                              ; preds = %90
  %106 = load i64, i64* %20, align 8
  %107 = icmp sge i64 %83, %106
  %108 = icmp eq i64 %99, 0
  %109 = or i1 %108, %107
  br i1 %109, label %110, label %433

110:                                              ; preds = %105, %90
  switch i32 %1, label %111 [
    i32 1, label %411
    i32 2, label %411
    i32 10, label %411
    i32 11, label %411
    i32 8, label %411
    i32 9, label %411
    i32 12, label %411
    i32 16, label %382
    i32 6, label %136
    i32 3, label %170
    i32 7, label %205
    i32 5, label %231
    i32 17, label %267
    i32 13, label %297
    i32 14, label %326
    i32 15, label %356
  ]

111:                                              ; preds = %110
  %112 = icmp sgt i64 %95, %94
  %113 = add i64 %95, -1
  %114 = select i1 %112, i64 %94, i64 %113
  %115 = select i1 %100, i64 0, i64 %114
  %116 = load i64, i64* %20, align 8
  %117 = icmp slt i64 %83, %116
  %118 = add i64 %116, -1
  %119 = select i1 %117, i64 %83, i64 %118
  %120 = select i1 %85, i64 0, i64 %119
  %121 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %122 = alloca i32, align 4
  store i32 0, i32* %122, align 4
  %123 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 %1, i64 %115, i64 %120, i64 1, i64 1, %struct._NexusInfo* %121, %struct._ExceptionInfo* %7, i32* %122)
  %124 = load i32, i32* %122, align 4
  %125 = icmp eq i32 %124, 1
  br i1 %125, label %126, label %128

126:                                              ; preds = %111
  %127 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 %1, i64 %115, i64 %120, i64 1, i64 1, %struct._NexusInfo* %121, %struct._ExceptionInfo* %7, %struct._PixelPacket* %123)
  br label %128

128:                                              ; preds = %111, %126
  %129 = phi %struct._PixelPacket* [ %123, %111 ], [ %127, %126 ]
  %130 = load i32, i32* %68, align 8
  %131 = icmp eq i32 %130, 0
  br i1 %131, label %407, label %132

132:                                              ; preds = %128
  %133 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %134 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %133, i64 0, i32 6
  %135 = load i16*, i16** %134, align 8
  br label %407

136:                                              ; preds = %110
  %137 = load %struct._RandomInfo*, %struct._RandomInfo** %69, align 8
  %138 = icmp eq %struct._RandomInfo* %137, null
  br i1 %138, label %139, label %142

139:                                              ; preds = %136
  %140 = call %struct._RandomInfo* @AcquireRandomInfo()
  store %struct._RandomInfo* %140, %struct._RandomInfo** %69, align 8
  %141 = load i64, i64* %19, align 8
  br label %142

142:                                              ; preds = %139, %136
  %143 = phi i64 [ %141, %139 ], [ %95, %136 ]
  %144 = phi %struct._RandomInfo* [ %140, %139 ], [ %137, %136 ]
  %145 = uitofp i64 %143 to double
  %146 = call fast double @GetPseudoRandomValue(%struct._RandomInfo* %144)
  %147 = fmul fast double %146, %145
  %148 = fptosi double %147 to i64
  %149 = load %struct._RandomInfo*, %struct._RandomInfo** %69, align 8
  %150 = load i64, i64* %20, align 8
  %151 = uitofp i64 %150 to double
  %152 = call fast double @GetPseudoRandomValue(%struct._RandomInfo* %149)
  %153 = fmul fast double %152, %151
  %154 = fptosi double %153 to i64
  %155 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %156 = alloca i32, align 4
  store i32 0, i32* %156, align 4
  %157 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 6, i64 %148, i64 %154, i64 1, i64 1, %struct._NexusInfo* %155, %struct._ExceptionInfo* %7, i32* %156)
  %158 = load i32, i32* %156, align 4
  %159 = icmp eq i32 %158, 1
  br i1 %159, label %160, label %162

160:                                              ; preds = %142
  %161 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 6, i64 %148, i64 %154, i64 1, i64 1, %struct._NexusInfo* %155, %struct._ExceptionInfo* %7, %struct._PixelPacket* %157)
  br label %162

162:                                              ; preds = %142, %160
  %163 = phi %struct._PixelPacket* [ %157, %142 ], [ %161, %160 ]
  %164 = load i32, i32* %68, align 8
  %165 = icmp eq i32 %164, 0
  br i1 %165, label %407, label %166

166:                                              ; preds = %162
  %167 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %168 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %167, i64 0, i32 6
  %169 = load i16*, i16** %168, align 8
  br label %407

170:                                              ; preds = %110
  %171 = and i64 %94, 7
  %172 = getelementptr inbounds [64 x i64], [64 x i64]* @DitherMatrix, i64 0, i64 %171
  %173 = load i64, i64* %172, align 8
  %174 = add nsw i64 %173, %94
  %175 = add nsw i64 %174, -32
  %176 = icmp slt i64 %174, 32
  %177 = icmp slt i64 %175, %95
  %178 = add nsw i64 %95, -1
  %179 = select i1 %177, i64 %175, i64 %178
  %180 = select i1 %176, i64 0, i64 %179
  %181 = load i64, i64* %20, align 8
  %182 = load i64, i64* %89, align 8
  %183 = add nsw i64 %182, %83
  %184 = add nsw i64 %183, -32
  %185 = icmp slt i64 %183, 32
  %186 = icmp slt i64 %184, %181
  %187 = add nsw i64 %181, -1
  %188 = select i1 %186, i64 %184, i64 %187
  %189 = select i1 %185, i64 0, i64 %188
  %190 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %191 = alloca i32, align 4
  store i32 0, i32* %191, align 4
  %192 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 3, i64 %180, i64 %189, i64 1, i64 1, %struct._NexusInfo* %190, %struct._ExceptionInfo* %7, i32* %191)
  %193 = load i32, i32* %191, align 4
  %194 = icmp eq i32 %193, 1
  br i1 %194, label %195, label %197

195:                                              ; preds = %170
  %196 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 3, i64 %180, i64 %189, i64 1, i64 1, %struct._NexusInfo* %190, %struct._ExceptionInfo* %7, %struct._PixelPacket* %192)
  br label %197

197:                                              ; preds = %170, %195
  %198 = phi %struct._PixelPacket* [ %192, %170 ], [ %196, %195 ]
  %199 = load i32, i32* %68, align 8
  %200 = icmp eq i32 %199, 0
  br i1 %200, label %407, label %201

201:                                              ; preds = %197
  %202 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %203 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %202, i64 0, i32 6
  %204 = load i16*, i16** %203, align 8
  br label %407

205:                                              ; preds = %110
  %206 = sdiv i64 %94, %95
  %207 = ashr i64 %94, 63
  %208 = add nsw i64 %206, %207
  %209 = mul nsw i64 %208, %95
  %210 = sub nsw i64 %94, %209
  %211 = load i64, i64* %20, align 8
  %212 = sdiv i64 %83, %211
  %213 = add nsw i64 %212, %86
  %214 = mul nsw i64 %213, %211
  %215 = sub nsw i64 %83, %214
  %216 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %217 = alloca i32, align 4
  store i32 0, i32* %217, align 4
  %218 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 7, i64 %210, i64 %215, i64 1, i64 1, %struct._NexusInfo* %216, %struct._ExceptionInfo* %7, i32* %217)
  %219 = load i32, i32* %217, align 4
  %220 = icmp eq i32 %219, 1
  br i1 %220, label %221, label %223

221:                                              ; preds = %205
  %222 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 7, i64 %210, i64 %215, i64 1, i64 1, %struct._NexusInfo* %216, %struct._ExceptionInfo* %7, %struct._PixelPacket* %218)
  br label %223

223:                                              ; preds = %205, %221
  %224 = phi %struct._PixelPacket* [ %218, %205 ], [ %222, %221 ]
  %225 = load i32, i32* %68, align 8
  %226 = icmp eq i32 %225, 0
  br i1 %226, label %407, label %227

227:                                              ; preds = %223
  %228 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %229 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %228, i64 0, i32 6
  %230 = load i16*, i16** %229, align 8
  br label %407

231:                                              ; preds = %110
  %232 = sdiv i64 %94, %95
  %233 = ashr i64 %94, 63
  %234 = add nsw i64 %232, %233
  %235 = mul nsw i64 %234, %95
  %236 = sub nsw i64 %94, %235
  %237 = and i64 %234, 1
  %238 = icmp eq i64 %237, 0
  %239 = xor i64 %236, -1
  %240 = add i64 %95, %239
  %241 = select i1 %238, i64 %236, i64 %240
  %242 = load i64, i64* %20, align 8
  %243 = sdiv i64 %83, %242
  %244 = add nsw i64 %243, %86
  %245 = mul nsw i64 %244, %242
  %246 = sub nsw i64 %83, %245
  %247 = and i64 %244, 1
  %248 = icmp eq i64 %247, 0
  %249 = xor i64 %246, -1
  %250 = add i64 %242, %249
  %251 = select i1 %248, i64 %246, i64 %250
  %252 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %253 = alloca i32, align 4
  store i32 0, i32* %253, align 4
  %254 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 5, i64 %241, i64 %251, i64 1, i64 1, %struct._NexusInfo* %252, %struct._ExceptionInfo* %7, i32* %253)
  %255 = load i32, i32* %253, align 4
  %256 = icmp eq i32 %255, 1
  br i1 %256, label %257, label %259

257:                                              ; preds = %231
  %258 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 5, i64 %241, i64 %251, i64 1, i64 1, %struct._NexusInfo* %252, %struct._ExceptionInfo* %7, %struct._PixelPacket* %254)
  br label %259

259:                                              ; preds = %231, %257
  %260 = phi %struct._PixelPacket* [ %254, %231 ], [ %258, %257 ]
  %261 = load i32, i32* %68, align 8
  %262 = icmp eq i32 %261, 0
  br i1 %262, label %407, label %263

263:                                              ; preds = %259
  %264 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %265 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %264, i64 0, i32 6
  %266 = load i16*, i16** %265, align 8
  br label %407

267:                                              ; preds = %110
  %268 = sdiv i64 %94, %95
  %269 = ashr i64 %94, 63
  %270 = add nsw i64 %268, %269
  %271 = load i64, i64* %20, align 8
  %272 = sdiv i64 %83, %271
  %273 = add nsw i64 %272, %86
  %274 = xor i64 %273, %270
  %275 = and i64 %274, 1
  %276 = icmp eq i64 %275, 0
  br i1 %276, label %277, label %411

277:                                              ; preds = %267
  %278 = mul nsw i64 %273, %271
  %279 = sub nsw i64 %83, %278
  %280 = mul nsw i64 %270, %95
  %281 = sub nsw i64 %94, %280
  %282 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %283 = alloca i32, align 4
  store i32 0, i32* %283, align 4
  %284 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 17, i64 %281, i64 %279, i64 1, i64 1, %struct._NexusInfo* %282, %struct._ExceptionInfo* %7, i32* %283)
  %285 = load i32, i32* %283, align 4
  %286 = icmp eq i32 %285, 1
  br i1 %286, label %287, label %289

287:                                              ; preds = %277
  %288 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 17, i64 %281, i64 %279, i64 1, i64 1, %struct._NexusInfo* %282, %struct._ExceptionInfo* %7, %struct._PixelPacket* %284)
  br label %289

289:                                              ; preds = %277, %287
  %290 = phi %struct._PixelPacket* [ %284, %277 ], [ %288, %287 ]
  %291 = load i32, i32* %68, align 8
  %292 = icmp eq i32 %291, 0
  br i1 %292, label %407, label %293

293:                                              ; preds = %289
  %294 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %295 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %294, i64 0, i32 6
  %296 = load i16*, i16** %295, align 8
  br label %407

297:                                              ; preds = %110
  br i1 %85, label %411, label %298

298:                                              ; preds = %297
  %299 = load i64, i64* %20, align 8
  %300 = icmp slt i64 %83, %299
  br i1 %300, label %301, label %411

301:                                              ; preds = %298
  %302 = sdiv i64 %94, %95
  %303 = ashr i64 %94, 63
  %304 = add nsw i64 %302, %303
  %305 = mul nsw i64 %304, %95
  %306 = sub nsw i64 %94, %305
  %307 = sdiv i64 %83, %299
  %308 = add nuw nsw i64 %307, %87
  %309 = mul nsw i64 %308, %299
  %310 = sub nsw i64 %83, %309
  %311 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %312 = alloca i32, align 4
  store i32 0, i32* %312, align 4
  %313 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 13, i64 %306, i64 %310, i64 1, i64 1, %struct._NexusInfo* %311, %struct._ExceptionInfo* %7, i32* %312)
  %314 = load i32, i32* %312, align 4
  %315 = icmp eq i32 %314, 1
  br i1 %315, label %316, label %318

316:                                              ; preds = %301
  %317 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 13, i64 %306, i64 %310, i64 1, i64 1, %struct._NexusInfo* %311, %struct._ExceptionInfo* %7, %struct._PixelPacket* %313)
  br label %318

318:                                              ; preds = %301, %316
  %319 = phi %struct._PixelPacket* [ %313, %301 ], [ %317, %316 ]
  %320 = load i32, i32* %68, align 8
  %321 = icmp eq i32 %320, 0
  br i1 %321, label %407, label %322

322:                                              ; preds = %318
  %323 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %324 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %323, i64 0, i32 6
  %325 = load i16*, i16** %324, align 8
  br label %407

326:                                              ; preds = %110
  %327 = xor i1 %100, true
  %328 = icmp sgt i64 %95, %94
  %329 = and i1 %328, %327
  br i1 %329, label %330, label %411

330:                                              ; preds = %326
  %331 = sdiv i64 %94, %95
  %332 = lshr i64 %94, 63
  %333 = add nuw nsw i64 %331, %332
  %334 = mul nsw i64 %333, %95
  %335 = sub nsw i64 %94, %334
  %336 = load i64, i64* %20, align 8
  %337 = sdiv i64 %83, %336
  %338 = add nsw i64 %337, %86
  %339 = mul nsw i64 %338, %336
  %340 = sub nsw i64 %83, %339
  %341 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %342 = alloca i32, align 4
  store i32 0, i32* %342, align 4
  %343 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 14, i64 %335, i64 %340, i64 1, i64 1, %struct._NexusInfo* %341, %struct._ExceptionInfo* %7, i32* %342)
  %344 = load i32, i32* %342, align 4
  %345 = icmp eq i32 %344, 1
  br i1 %345, label %346, label %348

346:                                              ; preds = %330
  %347 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 14, i64 %335, i64 %340, i64 1, i64 1, %struct._NexusInfo* %341, %struct._ExceptionInfo* %7, %struct._PixelPacket* %343)
  br label %348

348:                                              ; preds = %330, %346
  %349 = phi %struct._PixelPacket* [ %343, %330 ], [ %347, %346 ]
  %350 = load i32, i32* %68, align 8
  %351 = icmp eq i32 %350, 0
  br i1 %351, label %407, label %352

352:                                              ; preds = %348
  %353 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %354 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %353, i64 0, i32 6
  %355 = load i16*, i16** %354, align 8
  br label %407

356:                                              ; preds = %110
  %357 = sdiv i64 %94, %95
  %358 = ashr i64 %94, 63
  %359 = add nsw i64 %357, %358
  %360 = mul nsw i64 %359, %95
  %361 = sub nsw i64 %94, %360
  %362 = load i64, i64* %20, align 8
  %363 = icmp slt i64 %83, %362
  %364 = add i64 %362, -1
  %365 = select i1 %363, i64 %83, i64 %364
  %366 = select i1 %85, i64 0, i64 %365
  %367 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %368 = alloca i32, align 4
  store i32 0, i32* %368, align 4
  %369 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 15, i64 %361, i64 %366, i64 1, i64 1, %struct._NexusInfo* %367, %struct._ExceptionInfo* %7, i32* %368)
  %370 = load i32, i32* %368, align 4
  %371 = icmp eq i32 %370, 1
  br i1 %371, label %372, label %374

372:                                              ; preds = %356
  %373 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 15, i64 %361, i64 %366, i64 1, i64 1, %struct._NexusInfo* %367, %struct._ExceptionInfo* %7, %struct._PixelPacket* %369)
  br label %374

374:                                              ; preds = %356, %372
  %375 = phi %struct._PixelPacket* [ %369, %356 ], [ %373, %372 ]
  %376 = load i32, i32* %68, align 8
  %377 = icmp eq i32 %376, 0
  br i1 %377, label %407, label %378

378:                                              ; preds = %374
  %379 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %380 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %379, i64 0, i32 6
  %381 = load i16*, i16** %380, align 8
  br label %407

382:                                              ; preds = %110
  %383 = load i64, i64* %20, align 8
  %384 = sdiv i64 %83, %383
  %385 = add nsw i64 %384, %86
  %386 = mul nsw i64 %385, %383
  %387 = sub nsw i64 %83, %386
  %388 = icmp sgt i64 %95, %94
  %389 = add i64 %95, -1
  %390 = select i1 %388, i64 %94, i64 %389
  %391 = select i1 %100, i64 0, i64 %390
  %392 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %393 = alloca i32, align 4
  store i32 0, i32* %393, align 4
  %394 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 16, i64 %391, i64 %387, i64 1, i64 1, %struct._NexusInfo* %392, %struct._ExceptionInfo* %7, i32* %393)
  %395 = load i32, i32* %393, align 4
  %396 = icmp eq i32 %395, 1
  br i1 %396, label %397, label %399

397:                                              ; preds = %382
  %398 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 16, i64 %391, i64 %387, i64 1, i64 1, %struct._NexusInfo* %392, %struct._ExceptionInfo* %7, %struct._PixelPacket* %394)
  br label %399

399:                                              ; preds = %382, %397
  %400 = phi %struct._PixelPacket* [ %394, %382 ], [ %398, %397 ]
  %401 = load i32, i32* %68, align 8
  %402 = icmp eq i32 %401, 0
  br i1 %402, label %407, label %403

403:                                              ; preds = %399
  %404 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %405 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %404, i64 0, i32 6
  %406 = load i16*, i16** %405, align 8
  br label %407

407:                                              ; preds = %403, %399, %378, %374, %352, %348, %322, %318, %293, %289, %263, %259, %227, %223, %201, %197, %166, %162, %132, %128
  %408 = phi %struct._PixelPacket* [ %129, %128 ], [ %129, %132 ], [ %163, %162 ], [ %163, %166 ], [ %198, %197 ], [ %198, %201 ], [ %224, %223 ], [ %224, %227 ], [ %260, %259 ], [ %260, %263 ], [ %290, %289 ], [ %290, %293 ], [ %319, %318 ], [ %319, %322 ], [ %349, %348 ], [ %349, %352 ], [ %375, %374 ], [ %375, %378 ], [ %400, %399 ], [ %400, %403 ]
  %409 = phi i16* [ null, %128 ], [ %135, %132 ], [ null, %162 ], [ %169, %166 ], [ null, %197 ], [ %204, %201 ], [ null, %223 ], [ %230, %227 ], [ null, %259 ], [ %266, %263 ], [ null, %289 ], [ %296, %293 ], [ null, %318 ], [ %325, %322 ], [ null, %348 ], [ %355, %352 ], [ null, %374 ], [ %381, %378 ], [ null, %399 ], [ %406, %403 ]
  %410 = icmp eq %struct._PixelPacket* %408, null
  br i1 %410, label %471, label %411

411:                                              ; preds = %407, %326, %298, %297, %267, %110, %110, %110, %110, %110, %110, %110
  %412 = phi i16* [ %409, %407 ], [ %10, %110 ], [ %10, %110 ], [ %10, %110 ], [ %10, %110 ], [ %10, %110 ], [ %10, %110 ], [ %10, %110 ], [ %10, %267 ], [ %10, %298 ], [ %10, %297 ], [ %10, %326 ]
  %413 = phi %struct._PixelPacket* [ %408, %407 ], [ %12, %110 ], [ %12, %110 ], [ %12, %110 ], [ %12, %110 ], [ %12, %110 ], [ %12, %110 ], [ %12, %110 ], [ %12, %267 ], [ %12, %298 ], [ %12, %297 ], [ %12, %326 ]
  %414 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %92, i64 1
  %415 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %413, i64 0, i32 0
  %416 = load i16, i16* %415, align 2
  %417 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %92, i64 0, i32 0
  store i16 %416, i16* %417, align 2
  %418 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %413, i64 0, i32 1
  %419 = load i16, i16* %418, align 2
  %420 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %92, i64 0, i32 1
  store i16 %419, i16* %420, align 2
  %421 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %413, i64 0, i32 2
  %422 = load i16, i16* %421, align 2
  %423 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %92, i64 0, i32 2
  store i16 %422, i16* %423, align 2
  %424 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %413, i64 0, i32 3
  %425 = load i16, i16* %424, align 2
  %426 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %92, i64 0, i32 3
  store i16 %425, i16* %426, align 2
  %427 = icmp ne i16* %91, null
  %428 = icmp ne i16* %412, null
  %429 = and i1 %427, %428
  br i1 %429, label %430, label %465

430:                                              ; preds = %411
  %431 = load i16, i16* %412, align 2
  %432 = getelementptr inbounds i16, i16* %91, i64 1
  store i16 %431, i16* %91, align 2
  br label %465

433:                                              ; preds = %105
  %434 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %435 = alloca i32, align 4
  store i32 0, i32* %435, align 4
  %436 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.0(%struct._Image* %0, i32 %1, i64 %94, i64 %83, i64 %99, i64 1, %struct._NexusInfo* %434, %struct._ExceptionInfo* %7, i32* %435)
  %437 = load i32, i32* %435, align 4
  %438 = icmp eq i32 %437, 1
  br i1 %438, label %439, label %441

439:                                              ; preds = %433
  %440 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus.split.1(%struct._Image* %0, i32 %1, i64 %94, i64 %83, i64 %99, i64 1, %struct._NexusInfo* %434, %struct._ExceptionInfo* %7, %struct._PixelPacket* %436)
  br label %441

441:                                              ; preds = %433, %439
  %442 = phi %struct._PixelPacket* [ %436, %433 ], [ %440, %439 ]
  %443 = icmp eq %struct._PixelPacket* %442, null
  br i1 %443, label %471, label %444

444:                                              ; preds = %441
  %445 = load i32, i32* %68, align 8
  %446 = icmp eq i32 %445, 0
  br i1 %446, label %451, label %447

447:                                              ; preds = %444
  %448 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %449 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %448, i64 0, i32 6
  %450 = load i16*, i16** %449, align 8
  br label %451

451:                                              ; preds = %447, %444
  %452 = phi i16* [ %450, %447 ], [ null, %444 ]
  %453 = bitcast %struct._PixelPacket* %92 to i8*
  %454 = bitcast %struct._PixelPacket* %442 to i8*
  %455 = shl i64 %99, 3
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 %453, i8* nonnull align 2 %454, i64 %455, i1 false)
  %456 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %92, i64 %99
  %457 = icmp ne i16* %91, null
  %458 = icmp ne i16* %452, null
  %459 = and i1 %457, %458
  br i1 %459, label %460, label %465

460:                                              ; preds = %451
  %461 = bitcast i16* %91 to i8*
  %462 = bitcast i16* %452 to i8*
  %463 = shl i64 %99, 1
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 2 %461, i8* nonnull align 2 %462, i64 %463, i1 false)
  %464 = getelementptr inbounds i16, i16* %91, i64 %99
  br label %465

465:                                              ; preds = %460, %451, %430, %411
  %466 = phi i64 [ %99, %451 ], [ %99, %460 ], [ 1, %430 ], [ 1, %411 ]
  %467 = phi i16* [ %91, %451 ], [ %464, %460 ], [ %432, %430 ], [ %91, %411 ]
  %468 = phi %struct._PixelPacket* [ %456, %451 ], [ %456, %460 ], [ %414, %430 ], [ %414, %411 ]
  %469 = add i64 %466, %93
  %470 = icmp slt i64 %469, %4
  br i1 %470, label %90, label %471

471:                                              ; preds = %465, %441, %407, %82
  %472 = phi %struct._PixelPacket* [ %72, %82 ], [ %468, %465 ], [ %92, %441 ], [ %92, %407 ]
  %473 = phi i16* [ %71, %82 ], [ %467, %465 ], [ %91, %441 ], [ %91, %407 ]
  %474 = add nuw nsw i64 %73, 1
  %475 = icmp eq i64 %474, %5
  br i1 %475, label %476, label %70

476:                                              ; preds = %471, %62
  %477 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  %478 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %477, i64 0, i32 3
  %479 = load %struct._PixelPacket*, %struct._PixelPacket** %478, align 8
  %480 = icmp eq %struct._PixelPacket* %479, null
  %481 = bitcast %struct._PixelPacket* %479 to i8*
  br i1 %480, label %496, label %482

482:                                              ; preds = %476
  %483 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %477, i64 0, i32 0
  %484 = load i32, i32* %483, align 8
  %485 = icmp eq i32 %484, 0
  br i1 %485, label %486, label %488

486:                                              ; preds = %482
  %487 = call i8* @RelinquishAlignedMemory(i8* nonnull %481)
  br label %491

488:                                              ; preds = %482
  %489 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %477, i64 0, i32 2
  %490 = load i64, i64* %489, align 8
  br label %491

491:                                              ; preds = %488, %486
  store %struct._PixelPacket* null, %struct._PixelPacket** %478, align 8
  %492 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %477, i64 0, i32 4
  store %struct._PixelPacket* null, %struct._PixelPacket** %492, align 8
  %493 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %477, i64 0, i32 6
  store i16* null, i16** %493, align 8
  %494 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %477, i64 0, i32 2
  store i64 0, i64* %494, align 8
  store i32 0, i32* %483, align 8
  %495 = load %struct._NexusInfo*, %struct._NexusInfo** %21, align 8
  br label %496

496:                                              ; preds = %491, %476
  %497 = phi %struct._NexusInfo* [ %495, %491 ], [ %477, %476 ]
  %498 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %497, i64 0, i32 7
  store i64 -2880220588, i64* %498, align 8
  %499 = bitcast %struct._NexusInfo** %21 to i8**
  %500 = load i8*, i8** %499, align 8
  %501 = call i8* @RelinquishMagickMemory(i8* %500)
  store %struct._NexusInfo* null, %struct._NexusInfo** %21, align 8
  %502 = bitcast %struct._NexusInfo** %21 to i8*
  %503 = call i8* @RelinquishAlignedMemory(i8* nonnull %502)
  br label %504

504:                                              ; preds = %496, %23
  %505 = phi %struct._PixelPacket* [ null, %23 ], [ %8, %496 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %13)
  call void @llvm.lifetime.end.p0i8(i64 2, i8* nonnull %11)
  ret %struct._PixelPacket* %505
}

attributes #0 = { argmemonly nounwind willreturn }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { "prefer-inline-mrc-split" }
; end INTEL_FEATURE_SW_ADVANCED
