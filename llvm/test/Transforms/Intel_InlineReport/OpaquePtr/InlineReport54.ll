; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-META

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

%struct._MagickPixelPacket = type { i32, i32, i32, double, i64, float, float, float, float, float }
%struct._PixelPacket = type { i16, i16, i16, i16 }
%struct._Image = type { i32, i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, ptr, %struct._PixelPacket, %struct._PixelPacket, %struct._PixelPacket, double, %struct._ChromaticityInfo, i32, ptr, i32, ptr, ptr, ptr, i64, double, double, %struct._RectangleInfo, %struct._RectangleInfo, %struct._RectangleInfo, double, double, double, i32, i32, i32, i32, i32, i32, ptr, i64, i64, i64, i64, i64, i64, %struct._ErrorInfo, %struct._TimerInfo, ptr, ptr, ptr, ptr, ptr, ptr, [4096 x i8], [4096 x i8], [4096 x i8], i64, i64, %struct._ExceptionInfo, i32, i64, ptr, %struct._ProfileInfo, %struct._ProfileInfo, ptr, i64, i64, ptr, ptr, ptr, i32, i32, %struct._PixelPacket, ptr, %struct._RectangleInfo, ptr, ptr, i32, i32, i64, i32, i64, i64, i32, i64 }
%struct._ChromaticityInfo = type { %struct._PrimaryInfo, %struct._PrimaryInfo, %struct._PrimaryInfo, %struct._PrimaryInfo }
%struct._PrimaryInfo = type { double, double, double }
%struct._ErrorInfo = type { double, double, double }
%struct._TimerInfo = type { %struct._Timer, %struct._Timer, i32, i64 }
%struct._Timer = type { double, double, double }
%struct._ExceptionInfo = type { i32, i32, ptr, ptr, ptr, i32, ptr, i64 }
%struct._ProfileInfo = type { ptr, i64, ptr, i64 }
%struct._RectangleInfo = type { i64, i64, i64, i64 }
%struct._CacheView = type { ptr, i32, i64, ptr, i32, i64 }
%struct._CacheInfo = type { i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, i32, %struct._MagickPixelPacket, i64, ptr, ptr, ptr, i32, i32, [4096 x i8], [4096 x i8], %struct._CacheMethods, ptr, i64, ptr, i32, i32, i32, i64, ptr, ptr, i64, i64 }
%struct._CacheMethods = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct._NexusInfo = type { i32, %struct._RectangleInfo, i64, ptr, ptr, i32, ptr, i64 }

@.str.129 = private unnamed_addr constant [15 x i8] c"magick/cache.c\00", align 1
@.str.1.130 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str.2.131 = private unnamed_addr constant [23 x i8] c"MemoryAllocationFailed\00", align 1
@.str.3.132 = private unnamed_addr constant [5 x i8] c"`%s'\00", align 1
@.str.7.170 = private unnamed_addr constant [22 x i8] c"UnableToGetCacheNexus\00", align 1
@.str.17.1479 = private unnamed_addr constant [16 x i8] c"MeanShift/Image\00", align 1
@.str.18.1467 = private unnamed_addr constant [6 x i8] c"%s/%s\00", align 1
@DitherMatrix = internal unnamed_addr constant [64 x i64] [i64 0, i64 48, i64 12, i64 60, i64 3, i64 51, i64 15, i64 63, i64 32, i64 16, i64 44, i64 28, i64 35, i64 19, i64 47, i64 31, i64 8, i64 56, i64 4, i64 52, i64 11, i64 59, i64 7, i64 55, i64 40, i64 24, i64 36, i64 20, i64 43, i64 27, i64 39, i64 23, i64 2, i64 50, i64 14, i64 62, i64 1, i64 49, i64 13, i64 61, i64 34, i64 18, i64 46, i64 30, i64 33, i64 17, i64 45, i64 29, i64 10, i64 58, i64 6, i64 54, i64 9, i64 57, i64 5, i64 53, i64 42, i64 26, i64 38, i64 22, i64 41, i64 25, i64 37, i64 21], align 16

declare ptr @AcquireAlignedMemory(i64, i64)

declare i32 @ThrowMagickException(ptr nocapture, ptr, ptr, i64, i32, ptr, ptr nocapture readonly, ...)

declare noalias ptr @RelinquishAlignedMemory(ptr readonly)

declare fastcc i32 @ReadPixelCachePixels(ptr noalias, ptr noalias nocapture readonly, ptr)

declare fastcc i32 @ReadPixelCacheIndexes(ptr noalias, ptr noalias nocapture readonly, ptr)

declare ptr @AcquirePixelCacheNexus(i64)

declare ptr @AcquireRandomInfo()

declare double @GetPseudoRandomValue(ptr nocapture)

declare noalias ptr @RelinquishMagickMemory(ptr)

declare ptr @CloneImage(ptr, i64, i64, i32, ptr)

declare i32 @SetImageStorageClass(ptr, i32)

declare void @InheritException(ptr nocapture, ptr nocapture readonly)

declare ptr @AcquireVirtualCacheView(ptr, ptr nocapture readnone)

declare ptr @AcquireAuthenticCacheView(ptr, ptr)

declare ptr @GetCacheViewVirtualPixels(ptr nocapture readonly, i64, i64, i64, i64, ptr)

declare ptr @GetCacheViewAuthenticPixels(ptr nocapture readonly, i64, i64, i64, i64, ptr)

declare ptr @GetCacheViewVirtualIndexQueue(ptr nocapture readonly)

declare void @GetMagickPixelPacket(ptr readonly, ptr nocapture)

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.rint.f64(double) #0

declare i32 @SyncCacheViewAuthenticPixels(ptr noalias nocapture readonly, ptr)

declare i64 @FormatLocaleString(ptr noalias nocapture, i64, ptr noalias nocapture readonly, ...)

declare ptr @DestroyCacheView(ptr)

declare ptr @DestroyImage(ptr)

define ptr @MeanShiftImage(ptr %arg, i64 %arg1, i64 %arg2, double %arg3, ptr %arg4) {
bb:
  %i = alloca [4096 x i8], align 16
  %i5 = alloca %struct._MagickPixelPacket, align 8
  %i6 = alloca %struct._MagickPixelPacket, align 8
  %i7 = alloca %struct._PixelPacket, align 2
  %i8 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 59
  %i9 = load i32, ptr %i8, align 8
  %i10 = icmp eq i32 %i9, 0
  br i1 %i10, label %bb14, label %bb11

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 53
  %i13 = getelementptr inbounds [4096 x i8], ptr %i12, i64 0, i64 0
  br label %bb14

bb14:                                             ; preds = %bb11, %bb
  %i15 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 7
  %i16 = load i64, ptr %i15, align 8
  %i17 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 8
  %i18 = load i64, ptr %i17, align 8
  %i19 = tail call ptr @CloneImage(ptr nonnull %arg, i64 %i16, i64 %i18, i32 1, ptr %arg4)
  %i20 = icmp eq ptr %i19, null
  br i1 %i20, label %bb307, label %bb21

bb21:                                             ; preds = %bb14
  %i22 = tail call i32 @SetImageStorageClass(ptr nonnull %i19, i32 1)
  %i23 = icmp eq i32 %i22, 0
  br i1 %i23, label %bb24, label %bb27

bb24:                                             ; preds = %bb21
  %i25 = getelementptr inbounds %struct._Image, ptr %i19, i64 0, i32 58
  tail call void @InheritException(ptr %arg4, ptr nonnull %i25)
  %i26 = tail call ptr @DestroyImage(ptr nonnull %i19)
  br label %bb307

bb27:                                             ; preds = %bb21
  %i28 = tail call ptr @AcquireVirtualCacheView(ptr nonnull %arg, ptr %arg4)
  %i29 = tail call ptr @AcquireVirtualCacheView(ptr nonnull %arg, ptr %arg4)
  %i30 = tail call ptr @AcquireAuthenticCacheView(ptr nonnull %i19, ptr %arg4)
  %i31 = getelementptr inbounds %struct._Image, ptr %i19, i64 0, i32 8
  %i32 = load i64, ptr %i31, align 8
  %i33 = icmp sgt i64 %i32, 0
  br i1 %i33, label %bb34, label %bb303

bb34:                                             ; preds = %bb27
  %i35 = getelementptr inbounds %struct._Image, ptr %i19, i64 0, i32 7
  %i37 = getelementptr %struct._Image, ptr %arg, i64 0, i32 1
  %i38 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 5
  %i39 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 6
  %i40 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 7
  %i41 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 8
  %i42 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 9
  %i44 = sdiv i64 %arg2, 2
  %i45 = sub nsw i64 0, %i44
  %i46 = icmp slt i64 %i44, %i45
  %i47 = sdiv i64 %arg1, 2
  %i48 = sub nsw i64 0, %i47
  %i49 = icmp slt i64 %i47, %i48
  %i50 = lshr i64 %arg1, 1
  %i51 = lshr i64 %arg2, 1
  %i52 = mul i64 %i51, %i50
  %i54 = getelementptr inbounds %struct._PixelPacket, ptr %i7, i64 0, i32 2
  %i55 = getelementptr inbounds %struct._PixelPacket, ptr %i7, i64 0, i32 1
  %i56 = getelementptr inbounds %struct._PixelPacket, ptr %i7, i64 0, i32 0
  %i57 = fmul fast double %arg3, %arg3
  %i58 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i6, i64 0, i32 5
  %i59 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i6, i64 0, i32 6
  %i60 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i6, i64 0, i32 7
  %i61 = getelementptr inbounds %struct._PixelPacket, ptr %i7, i64 0, i32 3
  %i62 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i6, i64 0, i32 8
  %i63 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 47
  %i64 = getelementptr inbounds [4096 x i8], ptr %i, i64 0, i64 0
  %i65 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 53
  %i66 = getelementptr inbounds [4096 x i8], ptr %i65, i64 0, i64 0
  %i67 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 48
  %i68 = add nsw i64 %i47, 1
  %i69 = add nsw i64 %i44, 1
  br label %bb70

bb70:                                             ; preds = %bb297, %bb34
  %i71 = phi i32 [ 1, %bb34 ], [ %i299, %bb297 ]
  %i72 = phi i64 [ 0, %bb34 ], [ %i298, %bb297 ]
  %i73 = phi i64 [ 0, %bb34 ], [ %i300, %bb297 ]
  %i74 = icmp eq i32 %i71, 0
  br i1 %i74, label %bb297, label %bb75

bb75:                                             ; preds = %bb70
  %i76 = load i64, ptr %i15, align 8
  %i77 = call ptr @GetCacheViewVirtualPixels(ptr %i28, i64 0, i64 %i73, i64 %i76, i64 1, ptr %arg4)
  %i78 = load i64, ptr %i35, align 8
  %i79 = call ptr @GetCacheViewAuthenticPixels(ptr %i30, i64 0, i64 %i73, i64 %i78, i64 1, ptr %arg4)
  %i80 = icmp eq ptr %i77, null
  %i81 = icmp eq ptr %i79, null
  %i82 = or i1 %i80, %i81
  br i1 %i82, label %bb297, label %bb83

bb83:                                             ; preds = %bb75
  %i84 = call ptr @GetCacheViewVirtualIndexQueue(ptr %i28)
  %i85 = load i64, ptr %i35, align 8
  %i86 = icmp sgt i64 %i85, 0
  br i1 %i86, label %bb87, label %bb281

bb87:                                             ; preds = %bb83
  %i88 = icmp ne ptr %i84, null
  %i89 = sitofp i64 %i73 to double
  br label %bb90

bb90:                                             ; preds = %bb247, %bb87
  %i91 = phi i32 [ 1, %bb87 ], [ %i204, %bb247 ]
  %i92 = phi i64 [ 0, %bb87 ], [ %i278, %bb247 ]
  %i93 = phi ptr [ %i79, %bb87 ], [ %i277, %bb247 ]
  %i94 = phi ptr [ %i77, %bb87 ], [ %i276, %bb247 ]
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %i5)
  call void @GetMagickPixelPacket(ptr %arg, ptr nonnull %i5)
  %i95 = load i32, ptr %i37, align 4
  %i96 = getelementptr inbounds %struct._PixelPacket, ptr %i94, i64 0, i32 2
  %i97 = load i16, ptr %i96, align 2
  %i98 = uitofp i16 %i97 to float
  store float %i98, ptr %i38, align 8
  %i99 = getelementptr inbounds %struct._PixelPacket, ptr %i94, i64 0, i32 1
  %i100 = load i16, ptr %i99, align 2
  %i101 = uitofp i16 %i100 to float
  store float %i101, ptr %i39, align 4
  %i102 = getelementptr inbounds %struct._PixelPacket, ptr %i94, i64 0, i32 0
  %i103 = load i16, ptr %i102, align 2
  %i104 = uitofp i16 %i103 to float
  store float %i104, ptr %i40, align 8
  %i105 = getelementptr inbounds %struct._PixelPacket, ptr %i94, i64 0, i32 3
  %i106 = load i16, ptr %i105, align 2
  %i107 = uitofp i16 %i106 to float
  store float %i107, ptr %i41, align 4
  %i108 = icmp eq i32 %i95, 12
  %i109 = and i1 %i88, %i108
  br i1 %i109, label %bb110, label %bb114

bb110:                                            ; preds = %bb90
  %i111 = getelementptr inbounds i16, ptr %i84, i64 %i92
  %i112 = load i16, ptr %i111, align 2
  %i113 = uitofp i16 %i112 to float
  store float %i113, ptr %i42, align 8
  br label %bb114

bb114:                                            ; preds = %bb110, %bb90
  %i115 = sitofp i64 %i92 to double
  br label %bb119

bb116:                                            ; preds = %bb203
  %i117 = add nuw nsw i64 %i120, 1
  %i118 = icmp eq i64 %i117, 100
  br i1 %i118, label %bb247, label %bb119

bb119:                                            ; preds = %bb116, %bb114
  %i120 = phi i64 [ 0, %bb114 ], [ %i117, %bb116 ]
  %i121 = phi double [ %i115, %bb114 ], [ %i210, %bb116 ]
  %i122 = phi double [ %i89, %bb114 ], [ %i211, %bb116 ]
  %i123 = phi i32 [ %i91, %bb114 ], [ %i204, %bb116 ]
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %i6)
  call void @GetMagickPixelPacket(ptr %arg, ptr nonnull %i6)
  %i124 = load float, ptr %i38, align 8
  %i125 = load float, ptr %i39, align 4
  %i126 = load float, ptr %i40, align 8
  br i1 %i46, label %bb203, label %bb127

bb127:                                            ; preds = %bb119
  %i128 = call fast double @llvm.rint.f64(double %i121)
  %i129 = fptosi double %i128 to i64
  %i130 = call fast double @llvm.rint.f64(double %i122)
  %i131 = fptosi double %i130 to i64
  br i1 %i49, label %bb203, label %bb132

bb132:                                            ; preds = %bb200, %bb127
  %i133 = phi i64 [ %i201, %bb200 ], [ %i45, %bb127 ]
  %i134 = phi i64 [ %i197, %bb200 ], [ 0, %bb127 ]
  %i135 = phi double [ %i196, %bb200 ], [ 0.000000e+00, %bb127 ]
  %i136 = phi double [ %i195, %bb200 ], [ 0.000000e+00, %bb127 ]
  %i137 = phi i32 [ %i194, %bb200 ], [ %i123, %bb127 ]
  %i138 = mul nsw i64 %i133, %i133
  %i139 = add i64 %i133, %i131
  %i140 = sitofp i64 %i133 to double
  %i141 = fadd fast double %i122, %i140
  br label %bb142

bb142:                                            ; preds = %bb193, %bb132
  %i143 = phi i64 [ %i48, %bb132 ], [ %i198, %bb193 ]
  %i144 = phi i64 [ %i134, %bb132 ], [ %i197, %bb193 ]
  %i145 = phi double [ %i135, %bb132 ], [ %i196, %bb193 ]
  %i146 = phi double [ %i136, %bb132 ], [ %i195, %bb193 ]
  %i147 = phi i32 [ %i137, %bb132 ], [ %i194, %bb193 ]
  %i148 = mul nsw i64 %i143, %i143
  %i149 = add nuw nsw i64 %i148, %i138
  %i150 = icmp sgt i64 %i149, %i52
  br i1 %i150, label %bb193, label %bb151

bb151:                                            ; preds = %bb142
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i7)
  %i152 = add i64 %i143, %i129
  %i153 = call i32 @GetOneCacheViewVirtualPixel(ptr %i29, i64 %i152, i64 %i139, ptr nonnull %i7, ptr %arg4) #3
  %i154 = load float, ptr %i38, align 8
  %i155 = load i16, ptr %i54, align 2
  %i156 = uitofp i16 %i155 to float
  %i157 = fsub fast float %i154, %i156
  %i158 = fmul fast float %i157, %i157
  %i159 = load float, ptr %i39, align 4
  %i160 = load i16, ptr %i55, align 2
  %i161 = uitofp i16 %i160 to float
  %i162 = fsub fast float %i159, %i161
  %i163 = fmul fast float %i162, %i162
  %i164 = fadd fast float %i163, %i158
  %i165 = load float, ptr %i40, align 8
  %i166 = load i16, ptr %i56, align 2
  %i167 = uitofp i16 %i166 to float
  %i168 = fsub fast float %i165, %i167
  %i169 = fmul fast float %i168, %i168
  %i170 = fadd fast float %i164, %i169
  %i171 = fpext float %i170 to double
  %i172 = fcmp fast ult double %i57, %i171
  br i1 %i172, label %bb189, label %bb173

bb173:                                            ; preds = %bb151
  %i174 = sitofp i64 %i143 to double
  %i175 = fadd fast double %i145, %i121
  %i176 = fadd fast double %i175, %i174
  %i177 = fadd fast double %i141, %i146
  %i178 = load float, ptr %i58, align 8
  %i179 = fadd fast float %i178, %i156
  store float %i179, ptr %i58, align 8
  %i180 = load float, ptr %i59, align 4
  %i181 = fadd fast float %i180, %i161
  store float %i181, ptr %i59, align 4
  %i182 = load float, ptr %i60, align 8
  %i183 = fadd fast float %i182, %i167
  store float %i183, ptr %i60, align 8
  %i184 = load i16, ptr %i61, align 2
  %i185 = uitofp i16 %i184 to float
  %i186 = load float, ptr %i62, align 4
  %i187 = fadd fast float %i186, %i185
  store float %i187, ptr %i62, align 4
  %i188 = add nsw i64 %i144, 1
  br label %bb189

bb189:                                            ; preds = %bb173, %bb151
  %i190 = phi double [ %i177, %bb173 ], [ %i146, %bb151 ]
  %i191 = phi double [ %i176, %bb173 ], [ %i145, %bb151 ]
  %i192 = phi i64 [ %i188, %bb173 ], [ %i144, %bb151 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i7)
  br label %bb193

bb193:                                            ; preds = %bb189, %bb142
  %i194 = phi i32 [ %i153, %bb189 ], [ %i147, %bb142 ]
  %i195 = phi double [ %i190, %bb189 ], [ %i146, %bb142 ]
  %i196 = phi double [ %i191, %bb189 ], [ %i145, %bb142 ]
  %i197 = phi i64 [ %i192, %bb189 ], [ %i144, %bb142 ]
  %i198 = add i64 %i143, 1
  %i199 = icmp eq i64 %i198, %i68
  br i1 %i199, label %bb200, label %bb142

bb200:                                            ; preds = %bb193
  %i201 = add i64 %i133, 1
  %i202 = icmp eq i64 %i201, %i69
  br i1 %i202, label %bb203, label %bb132

bb203:                                            ; preds = %bb200, %bb127, %bb119
  %i204 = phi i32 [ %i123, %bb119 ], [ %i123, %bb127 ], [ %i194, %bb200 ]
  %i205 = phi double [ 0.000000e+00, %bb119 ], [ 0.000000e+00, %bb127 ], [ %i195, %bb200 ]
  %i206 = phi double [ 0.000000e+00, %bb119 ], [ 0.000000e+00, %bb127 ], [ %i196, %bb200 ]
  %i207 = phi i64 [ 0, %bb119 ], [ 0, %bb127 ], [ %i197, %bb200 ]
  %i208 = sitofp i64 %i207 to double
  %i209 = fdiv fast double 1.000000e+00, %i208
  %i210 = fmul fast double %i209, %i206
  %i211 = fmul fast double %i209, %i205
  %i212 = load float, ptr %i58, align 8
  %i213 = fpext float %i212 to double
  %i214 = fmul fast double %i209, %i213
  %i215 = fptrunc double %i214 to float
  store float %i215, ptr %i38, align 8
  %i216 = load float, ptr %i59, align 4
  %i217 = fpext float %i216 to double
  %i218 = fmul fast double %i209, %i217
  %i219 = fptrunc double %i218 to float
  store float %i219, ptr %i39, align 4
  %i220 = load float, ptr %i60, align 8
  %i221 = fpext float %i220 to double
  %i222 = fmul fast double %i209, %i221
  %i223 = fptrunc double %i222 to float
  store float %i223, ptr %i40, align 8
  %i224 = load float, ptr %i62, align 4
  %i225 = fpext float %i224 to double
  %i226 = fmul fast double %i209, %i225
  %i227 = fptrunc double %i226 to float
  store float %i227, ptr %i41, align 4
  %i228 = fsub fast double %i210, %i121
  %i229 = fmul fast double %i228, %i228
  %i230 = fsub fast double %i211, %i122
  %i231 = fmul fast double %i230, %i230
  %i232 = fsub fast float %i215, %i124
  %i233 = fpext float %i232 to double
  %i234 = fmul fast double %i233, %i233
  %i235 = fsub fast float %i219, %i125
  %i236 = fpext float %i235 to double
  %i237 = fmul fast double %i236, %i236
  %i238 = fsub fast float %i223, %i126
  %i239 = fpext float %i238 to double
  %i240 = fmul fast double %i239, %i239
  %i241 = fadd fast double %i237, %i234
  %i242 = fadd fast double %i241, %i240
  %i243 = fmul fast double %i242, 0x3EEFC05F809F40DF
  %i244 = fadd fast double %i229, %i231
  %i245 = fadd fast double %i244, %i243
  %i246 = fcmp fast ugt double %i245, 3.000000e+00
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %i6)
  br i1 %i246, label %bb116, label %bb247

bb247:                                            ; preds = %bb203, %bb116
  %i248 = fcmp fast ugt float %i215, 0.000000e+00
  %i249 = fcmp fast ult float %i215, 6.553500e+04
  %i250 = fadd fast float %i215, 5.000000e-01
  %i251 = fptoui float %i250 to i16
  %i252 = select i1 %i249, i16 %i251, i16 -1
  %i253 = select i1 %i248, i16 %i252, i16 0
  %i254 = getelementptr inbounds %struct._PixelPacket, ptr %i93, i64 0, i32 2
  store i16 %i253, ptr %i254, align 2
  %i255 = fcmp fast ugt float %i219, 0.000000e+00
  %i256 = fcmp fast ult float %i219, 6.553500e+04
  %i257 = fadd fast float %i219, 5.000000e-01
  %i258 = fptoui float %i257 to i16
  %i259 = select i1 %i256, i16 %i258, i16 -1
  %i260 = select i1 %i255, i16 %i259, i16 0
  %i261 = getelementptr inbounds %struct._PixelPacket, ptr %i93, i64 0, i32 1
  store i16 %i260, ptr %i261, align 2
  %i262 = fcmp fast ugt float %i223, 0.000000e+00
  %i263 = fcmp fast ult float %i223, 6.553500e+04
  %i264 = fadd fast float %i223, 5.000000e-01
  %i265 = fptoui float %i264 to i16
  %i266 = select i1 %i263, i16 %i265, i16 -1
  %i267 = select i1 %i262, i16 %i266, i16 0
  %i268 = getelementptr inbounds %struct._PixelPacket, ptr %i93, i64 0, i32 0
  store i16 %i267, ptr %i268, align 2
  %i269 = fcmp fast ugt float %i227, 0.000000e+00
  %i270 = fcmp fast ult float %i227, 6.553500e+04
  %i271 = fadd fast float %i227, 5.000000e-01
  %i272 = fptoui float %i271 to i16
  %i273 = select i1 %i270, i16 %i272, i16 -1
  %i274 = select i1 %i269, i16 %i273, i16 0
  %i275 = getelementptr inbounds %struct._PixelPacket, ptr %i93, i64 0, i32 3
  store i16 %i274, ptr %i275, align 2
  %i276 = getelementptr inbounds %struct._PixelPacket, ptr %i94, i64 1
  %i277 = getelementptr inbounds %struct._PixelPacket, ptr %i93, i64 1
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %i5)
  %i278 = add nuw nsw i64 %i92, 1
  %i279 = load i64, ptr %i35, align 8
  %i280 = icmp slt i64 %i278, %i279
  br i1 %i280, label %bb90, label %bb281

bb281:                                            ; preds = %bb247, %bb83
  %i282 = phi i32 [ 1, %bb83 ], [ %i204, %bb247 ]
  %i283 = call i32 @SyncCacheViewAuthenticPixels(ptr %i30, ptr %arg4)
  %i284 = icmp eq i32 %i283, 0
  %i285 = select i1 %i284, i32 0, i32 %i282
  %i286 = load ptr, ptr %i63, align 8
  %i287 = icmp eq ptr %i286, null
  br i1 %i287, label %bb297, label %bb288

bb288:                                            ; preds = %bb281
  %i289 = add nsw i64 %i72, 1
  %i290 = load i64, ptr %i17, align 8
  call void @llvm.lifetime.start.p0(i64 4096, ptr nonnull %i64)
  %i291 = call i64 (ptr, i64, ptr, ...) @FormatLocaleString(ptr nonnull %i64, i64 4096, ptr @.str.18.1467, ptr @.str.17.1479, ptr nonnull %i66)
  %i292 = load ptr, ptr %i63, align 8
  %i293 = load ptr, ptr %i67, align 8
  %i294 = call i32 %i292(ptr nonnull %i64, i64 %i72, i64 %i290, ptr %i293)
  call void @llvm.lifetime.end.p0(i64 4096, ptr nonnull %i64)
  %i295 = icmp eq i32 %i294, 0
  %i296 = select i1 %i295, i32 0, i32 %i285
  br label %bb297

bb297:                                            ; preds = %bb288, %bb281, %bb75, %bb70
  %i298 = phi i64 [ %i72, %bb70 ], [ %i72, %bb75 ], [ %i72, %bb281 ], [ %i289, %bb288 ]
  %i299 = phi i32 [ 0, %bb70 ], [ 0, %bb75 ], [ %i285, %bb281 ], [ %i296, %bb288 ]
  %i300 = add nuw nsw i64 %i73, 1
  %i301 = load i64, ptr %i31, align 8
  %i302 = icmp slt i64 %i300, %i301
  br i1 %i302, label %bb70, label %bb303

bb303:                                            ; preds = %bb297, %bb27
  %i304 = call ptr @DestroyCacheView(ptr %i30)
  %i305 = call ptr @DestroyCacheView(ptr %i29)
  %i306 = call ptr @DestroyCacheView(ptr %i28)
  br label %bb307

bb307:                                            ; preds = %bb303, %bb24, %bb14
  %i308 = phi ptr [ null, %bb24 ], [ %i19, %bb303 ], [ null, %bb14 ]
  ret ptr %i308
}

define i32 @GetOneCacheViewVirtualPixel(ptr noalias nocapture readonly %arg, i64 %arg1, i64 %arg2, ptr noalias nocapture %arg3, ptr %arg4) {
bb:
  %i = getelementptr inbounds %struct._CacheView, ptr %arg, i64 0, i32 0
  %i5 = load ptr, ptr %i, align 8
  %i6 = getelementptr inbounds %struct._Image, ptr %i5, i64 0, i32 12
  %i7 = getelementptr inbounds %struct._PixelPacket, ptr %i6, i64 0, i32 0
  %i8 = load i16, ptr %i7, align 2
  %i9 = getelementptr inbounds %struct._PixelPacket, ptr %arg3, i64 0, i32 0
  store i16 %i8, ptr %i9, align 2
  %i10 = getelementptr inbounds %struct._PixelPacket, ptr %i6, i64 0, i32 1
  %i11 = load i16, ptr %i10, align 2
  %i12 = getelementptr inbounds %struct._PixelPacket, ptr %arg3, i64 0, i32 1
  store i16 %i11, ptr %i12, align 2
  %i13 = getelementptr inbounds %struct._PixelPacket, ptr %i6, i64 0, i32 2
  %i14 = load i16, ptr %i13, align 2
  %i15 = getelementptr inbounds %struct._PixelPacket, ptr %arg3, i64 0, i32 2
  store i16 %i14, ptr %i15, align 2
  %i16 = getelementptr inbounds %struct._PixelPacket, ptr %i6, i64 0, i32 3
  %i17 = load i16, ptr %i16, align 2
  %i18 = getelementptr inbounds %struct._PixelPacket, ptr %arg3, i64 0, i32 3
  store i16 %i17, ptr %i18, align 2
  %i19 = getelementptr inbounds %struct._CacheView, ptr %arg, i64 0, i32 1
  %i20 = load i32, ptr %i19, align 8
  %i21 = getelementptr inbounds %struct._CacheView, ptr %arg, i64 0, i32 3
  %i22 = load ptr, ptr %i21, align 8
  %i23 = load ptr, ptr %i22, align 8
  %i24 = alloca i32, align 4
  store i32 0, ptr %i24, align 4
  %i25 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %i5, i32 %i20, i64 %arg1, i64 %arg2, i64 1, i64 1, ptr %i23, ptr %arg4, ptr %i24) #3
  %i26 = load i32, ptr %i24, align 4
  %i27 = icmp eq i32 %i26, 1
  br i1 %i27, label %bb28, label %bb30

bb28:                                             ; preds = %bb
  %i29 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %i5, i32 %i20, i64 %arg1, i64 %arg2, i64 1, i64 1, ptr %i23, ptr %arg4, ptr %i25)
  br label %bb30

bb30:                                             ; preds = %bb28, %bb
  %i31 = phi ptr [ %i25, %bb ], [ %i29, %bb28 ]
  %i32 = icmp eq ptr %i31, null
  br i1 %i32, label %bb42, label %bb33

bb33:                                             ; preds = %bb30
  %i34 = getelementptr inbounds %struct._PixelPacket, ptr %i31, i64 0, i32 0
  %i35 = load i16, ptr %i34, align 2
  store i16 %i35, ptr %i9, align 2
  %i36 = getelementptr inbounds %struct._PixelPacket, ptr %i31, i64 0, i32 1
  %i37 = load i16, ptr %i36, align 2
  store i16 %i37, ptr %i12, align 2
  %i38 = getelementptr inbounds %struct._PixelPacket, ptr %i31, i64 0, i32 2
  %i39 = load i16, ptr %i38, align 2
  store i16 %i39, ptr %i15, align 2
  %i40 = getelementptr inbounds %struct._PixelPacket, ptr %i31, i64 0, i32 3
  %i41 = load i16, ptr %i40, align 2
  store i16 %i41, ptr %i18, align 2
  br label %bb42

bb42:                                             ; preds = %bb33, %bb30
  %i43 = phi i32 [ 1, %bb33 ], [ 0, %bb30 ]
  ret i32 %i43
}

define ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 %arg1, i64 %arg2, i64 %arg3, i64 %arg4, i64 %arg5, ptr %arg6, ptr %arg7, ptr %arg8) {
bb:
  %i = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 49
  %i10 = load ptr, ptr %i, align 8
  %i11 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 3
  %i12 = load i32, ptr %i11, align 8
  %i13 = icmp eq i32 %i12, 0
  br i1 %i13, label %bb221, label %bb14

bb14:                                             ; preds = %bb
  %i15 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 38
  %i16 = load ptr, ptr %i15, align 8
  %i17 = icmp eq ptr %i16, null
  br i1 %i17, label %bb18, label %bb23

bb18:                                             ; preds = %bb14
  %i19 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 73
  %i20 = load ptr, ptr %i19, align 8
  %i21 = icmp ne ptr %i20, null
  %i22 = zext i1 %i21 to i32
  br label %bb23

bb23:                                             ; preds = %bb18, %bb14
  %i24 = phi i32 [ 1, %bb14 ], [ %i22, %bb18 ]
  %i25 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1
  %i26 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 0
  store i64 %arg4, ptr %i26, align 8
  %i27 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 1
  store i64 %arg5, ptr %i27, align 8
  %i28 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 2
  store i64 %arg2, ptr %i28, align 8
  %i29 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 3
  store i64 %arg3, ptr %i29, align 8
  %i30 = load i32, ptr %i11, align 8
  %i31 = add i32 %i30, -1
  %i32 = icmp ult i32 %i31, 2
  %i33 = icmp eq i32 %i24, 0
  %i34 = and i1 %i33, %i32
  %i35 = getelementptr inbounds %struct._RectangleInfo, ptr %i25, i64 0, i32 1
  %i36 = getelementptr inbounds %struct._RectangleInfo, ptr %i25, i64 0, i32 0
  br i1 %i34, label %bb37, label %bb78

bb37:                                             ; preds = %bb23
  %i38 = add nsw i64 %arg3, %arg5
  %i39 = icmp sgt i64 %arg2, -1
  br i1 %i39, label %bb40, label %bb78

bb40:                                             ; preds = %bb37
  %i41 = add nsw i64 %arg2, %arg4
  %i42 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 6
  %i43 = load i64, ptr %i42, align 8
  %i44 = icmp sle i64 %i41, %i43
  %i45 = icmp sgt i64 %arg3, -1
  %i46 = and i1 %i45, %i44
  br i1 %i46, label %bb47, label %bb78

bb47:                                             ; preds = %bb40
  %i48 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 7
  %i49 = load i64, ptr %i48, align 8
  %i50 = icmp sgt i64 %i38, %i49
  br i1 %i50, label %bb78, label %bb51

bb51:                                             ; preds = %bb47
  %i52 = icmp eq i64 %arg5, 1
  br i1 %i52, label %bb60, label %bb53

bb53:                                             ; preds = %bb51
  %i54 = icmp eq i64 %arg2, 0
  br i1 %i54, label %bb55, label %bb78

bb55:                                             ; preds = %bb53
  %i56 = icmp eq i64 %i43, %arg4
  br i1 %i56, label %bb60, label %bb57

bb57:                                             ; preds = %bb55
  %i58 = urem i64 %arg4, %i43
  %i59 = icmp eq i64 %i58, 0
  br i1 %i59, label %bb60, label %bb78

bb60:                                             ; preds = %bb57, %bb55, %bb51
  %i61 = mul i64 %i43, %arg3
  %i62 = add i64 %i61, %arg2
  %i63 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 14
  %i64 = load ptr, ptr %i63, align 8
  %i65 = getelementptr inbounds %struct._PixelPacket, ptr %i64, i64 %i62
  %i66 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 4
  store ptr %i65, ptr %i66, align 8
  %i67 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  store ptr null, ptr %i67, align 8
  %i68 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 16
  %i69 = load i32, ptr %i68, align 8
  %i70 = icmp eq i32 %i69, 0
  br i1 %i70, label %bb75, label %bb71

bb71:                                             ; preds = %bb60
  %i72 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 15
  %i73 = load ptr, ptr %i72, align 8
  %i74 = getelementptr inbounds i16, ptr %i73, i64 %i62
  store ptr %i74, ptr %i67, align 8
  br label %bb75

bb75:                                             ; preds = %bb71, %bb60
  %i76 = phi ptr [ %i74, %bb71 ], [ null, %bb60 ]
  %i77 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 5
  store i32 1, ptr %i77, align 8
  br label %bb166

bb78:                                             ; preds = %bb57, %bb53, %bb47, %bb40, %bb37, %bb23
  %i79 = mul i64 %arg4, %arg5
  %i80 = shl i64 %i79, 3
  %i81 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 16
  %i82 = load i32, ptr %i81, align 8
  %i83 = icmp eq i32 %i82, 0
  %i84 = mul i64 %i79, 10
  %i85 = select i1 %i83, i64 %i80, i64 %i84
  %i86 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 3
  %i87 = load ptr, ptr %i86, align 8
  %i88 = icmp eq ptr %i87, null
  %i89 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 2
  br i1 %i88, label %bb91, label %bb107

bb91:                                             ; preds = %bb78
  store i64 %i85, ptr %i89, align 8
  %i92 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 0
  store i32 0, ptr %i92, align 8
  %i93 = tail call ptr @AcquireAlignedMemory(i64 1, i64 %i85)
  store ptr %i93, ptr %i86, align 8
  %i95 = icmp eq ptr %i93, null
  br i1 %i95, label %bb96, label %bb98

bb96:                                             ; preds = %bb91
  store i32 1, ptr %i92, align 8
  %i97 = load i64, ptr %i89, align 8
  store ptr null, ptr %i86, align 8
  br label %bb98

bb98:                                             ; preds = %bb96, %bb91
  %i99 = phi ptr [ null, %bb96 ], [ %i93, %bb91 ]
  %i100 = phi ptr [ null, %bb96 ], [ %i93, %bb91 ]
  %i101 = ptrtoint ptr %i99 to i64
  %i102 = icmp eq ptr %i100, null
  br i1 %i102, label %bb103, label %bb134

bb103:                                            ; preds = %bb98
  %i104 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 18
  %i105 = getelementptr inbounds [4096 x i8], ptr %i104, i64 0, i64 0
  %i106 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr %arg7, ptr @.str.129, ptr @.str.1.130, i64 4688, i32 400, ptr @.str.2.131, ptr @.str.3.132, ptr nonnull %i105)
  store i64 0, ptr %i89, align 8
  br label %bb221

bb107:                                            ; preds = %bb78
  %i108 = ptrtoint ptr %i87 to i64
  %i109 = load i64, ptr %i89, align 8
  %i110 = icmp ult i64 %i109, %i85
  br i1 %i110, label %bb111, label %bb134

bb111:                                            ; preds = %bb107
  %i112 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 0
  %i113 = load i32, ptr %i112, align 8
  %i114 = icmp eq i32 %i113, 0
  br i1 %i114, label %bb115, label %bb117

bb115:                                            ; preds = %bb111
  %i116 = tail call ptr @RelinquishAlignedMemory(ptr nonnull %i87)
  br label %bb117

bb117:                                            ; preds = %bb115, %bb111
  store ptr null, ptr %i86, align 8
  %i118 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 4
  store ptr null, ptr %i118, align 8
  %i119 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  store ptr null, ptr %i119, align 8
  store i64 %i85, ptr %i89, align 8
  store i32 0, ptr %i112, align 8
  %i120 = tail call ptr @AcquireAlignedMemory(i64 1, i64 %i85)
  store ptr %i120, ptr %i86, align 8
  %i122 = icmp eq ptr %i120, null
  br i1 %i122, label %bb123, label %bb125

bb123:                                            ; preds = %bb117
  store i32 1, ptr %i112, align 8
  %i124 = load i64, ptr %i89, align 8
  store ptr null, ptr %i86, align 8
  br label %bb125

bb125:                                            ; preds = %bb123, %bb117
  %i126 = phi ptr [ null, %bb123 ], [ %i120, %bb117 ]
  %i127 = phi ptr [ null, %bb123 ], [ %i120, %bb117 ]
  %i128 = ptrtoint ptr %i126 to i64
  %i129 = icmp eq ptr %i127, null
  br i1 %i129, label %bb130, label %bb134

bb130:                                            ; preds = %bb125
  %i131 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 18
  %i132 = getelementptr inbounds [4096 x i8], ptr %i131, i64 0, i64 0
  %i133 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr %arg7, ptr @.str.129, ptr @.str.1.130, i64 4688, i32 400, ptr @.str.2.131, ptr @.str.3.132, ptr nonnull %i132)
  store i64 0, ptr %i89, align 8
  br label %bb221

bb134:                                            ; preds = %bb125, %bb107, %bb98
  %i135 = phi i64 [ %i128, %bb125 ], [ %i101, %bb98 ], [ %i108, %bb107 ]
  %i136 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 4
  store i64 %i135, ptr %i136, align 8
  %i138 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  store ptr null, ptr %i138, align 8
  %i139 = load i32, ptr %i81, align 8
  %i140 = icmp eq i32 %i139, 0
  %i141 = inttoptr i64 %i135 to ptr
  br i1 %i140, label %bb145, label %bb142

bb142:                                            ; preds = %bb134
  %i143 = getelementptr inbounds %struct._PixelPacket, ptr %i141, i64 %i79
  %i144 = getelementptr %struct._PixelPacket, ptr %i143, i64 0, i32 0
  store ptr %i144, ptr %i138, align 8
  br label %bb145

bb145:                                            ; preds = %bb142, %bb134
  %i146 = phi ptr [ %i144, %bb142 ], [ null, %bb134 ]
  %i147 = load i32, ptr %i11, align 8
  %i148 = icmp eq i32 %i147, 4
  br i1 %i148, label %bb163, label %bb149

bb149:                                            ; preds = %bb145
  %i150 = getelementptr inbounds %struct._RectangleInfo, ptr %i25, i64 0, i32 3
  %i151 = load i64, ptr %i150, align 8
  %i152 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 6
  %i153 = load i64, ptr %i152, align 8
  %i154 = mul i64 %i153, %i151
  %i155 = getelementptr inbounds %struct._RectangleInfo, ptr %i25, i64 0, i32 2
  %i156 = load i64, ptr %i155, align 8
  %i157 = add i64 %i154, %i156
  %i158 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 14
  %i159 = load ptr, ptr %i158, align 8
  %i160 = getelementptr inbounds %struct._PixelPacket, ptr %i159, i64 %i157
  %i161 = icmp eq ptr %i160, %i141
  %i162 = zext i1 %i161 to i32
  br label %bb163

bb163:                                            ; preds = %bb149, %bb145
  %i164 = phi i32 [ %i162, %bb149 ], [ 1, %bb145 ]
  %i165 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 5
  store i32 %i164, ptr %i165, align 8
  br label %bb166

bb166:                                            ; preds = %bb163, %bb75
  %i167 = phi ptr [ %i146, %bb163 ], [ %i76, %bb75 ]
  %i168 = phi i32 [ %i164, %bb163 ], [ 1, %bb75 ]
  %i169 = phi ptr [ %i141, %bb163 ], [ %i65, %bb75 ]
  %i170 = icmp eq ptr %i169, null
  br i1 %i170, label %bb221, label %bb171

bb171:                                            ; preds = %bb166
  %i172 = getelementptr inbounds %struct._RectangleInfo, ptr %i25, i64 0, i32 3
  %i173 = load i64, ptr %i172, align 8
  %i174 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 6
  %i175 = load i64, ptr %i174, align 8
  %i176 = mul i64 %i175, %i173
  %i177 = getelementptr inbounds %struct._RectangleInfo, ptr %i25, i64 0, i32 2
  %i178 = load i64, ptr %i177, align 8
  %i179 = add i64 %i176, %i178
  %i180 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 7
  %i181 = load i64, ptr %i180, align 8
  %i182 = icmp sgt i64 %i179, -1
  br i1 %i182, label %bb183, label %bb220

bb183:                                            ; preds = %bb171
  %i184 = mul i64 %i181, %i175
  %i185 = load i64, ptr %i35, align 8
  %i186 = add i64 %i185, -1
  %i187 = mul i64 %i186, %i175
  %i188 = load i64, ptr %i36, align 8
  %i189 = add nsw i64 %i179, -1
  %i190 = add i64 %i189, %i188
  %i191 = add i64 %i190, %i187
  %i192 = icmp ult i64 %i191, %i184
  %i193 = icmp sgt i64 %arg2, -1
  %i194 = and i1 %i193, %i192
  br i1 %i194, label %bb195, label %bb220

bb195:                                            ; preds = %bb183
  %i196 = add i64 %arg4, %arg2
  %i197 = icmp sgt i64 %i196, %i175
  %i198 = icmp slt i64 %arg3, 0
  %i199 = or i1 %i198, %i197
  %i200 = add i64 %arg5, %arg3
  %i201 = icmp sgt i64 %i200, %i181
  %i202 = or i1 %i199, %i201
  br i1 %i202, label %bb220, label %bb203

bb203:                                            ; preds = %bb195
  %i204 = icmp eq i32 %i168, 0
  br i1 %i204, label %bb205, label %bb221

bb205:                                            ; preds = %bb203
  %i206 = tail call fastcc i32 @ReadPixelCachePixels(ptr nonnull %i10, ptr nonnull %arg6, ptr %arg7)
  %i207 = icmp eq i32 %i206, 0
  br i1 %i207, label %bb221, label %bb208

bb208:                                            ; preds = %bb205
  %i209 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 0
  %i210 = load i32, ptr %i209, align 8
  %i211 = icmp eq i32 %i210, 2
  br i1 %i211, label %bb216, label %bb212

bb212:                                            ; preds = %bb208
  %i213 = getelementptr inbounds %struct._CacheInfo, ptr %i10, i64 0, i32 1
  %i214 = load i32, ptr %i213, align 4
  %i215 = icmp eq i32 %i214, 12
  br i1 %i215, label %bb216, label %bb219

bb216:                                            ; preds = %bb212, %bb208
  %i217 = tail call fastcc i32 @ReadPixelCacheIndexes(ptr nonnull %i10, ptr nonnull %arg6, ptr %arg7)
  %i218 = icmp eq i32 %i217, 0
  br i1 %i218, label %bb221, label %bb219

bb219:                                            ; preds = %bb216, %bb212
  br label %bb221

bb220:                                            ; preds = %bb195, %bb183, %bb171
  store i32 1, ptr %arg8, align 4
  ret ptr %i169

bb221:                                            ; preds = %bb219, %bb216, %bb205, %bb203, %bb166, %bb130, %bb103, %bb
  %i222 = phi ptr [ null, %bb ], [ null, %bb166 ], [ %i169, %bb219 ], [ %i169, %bb203 ], [ null, %bb205 ], [ null, %bb216 ], [ null, %bb103 ], [ null, %bb130 ]
  ret ptr %i222
}

define ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 %arg1, i64 %arg2, i64 %arg3, i64 %arg4, i64 %arg5, ptr %arg6, ptr %arg7, ptr %arg8) {
bb:
  %i = alloca i16, align 2
  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %i)
  %i10 = alloca %struct._PixelPacket, align 2
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i10)
  %i12 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 49
  %i14 = load ptr, ptr %i12, align 8
  %i15 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  %i16 = load ptr, ptr %i15, align 8
  %i17 = getelementptr %struct._CacheInfo, ptr %i14, i64 0, i32 6
  %i18 = getelementptr %struct._CacheInfo, ptr %i14, i64 0, i32 7
  %i19 = tail call ptr @AcquirePixelCacheNexus(i64 1)
  %i20 = icmp eq ptr %i19, null
  br i1 %i20, label %bb21, label %bb25

bb21:                                             ; preds = %bb
  %i22 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 53
  %i23 = getelementptr inbounds [4096 x i8], ptr %i22, i64 0, i64 0
  %i24 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr %arg7, ptr @.str.129, ptr @.str.1.130, i64 2673, i32 445, ptr @.str.7.170, ptr @.str.3.132, ptr nonnull %i23)
  br label %bb502

bb25:                                             ; preds = %bb
  switch i32 %arg1, label %bb46 [
    i32 10, label %bb26
    i32 11, label %bb31
    i32 8, label %bb36
    i32 9, label %bb41
    i32 12, label %bb41
  ]

bb26:                                             ; preds = %bb25
  %i27 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 2
  store i16 0, ptr %i27, align 2
  %i28 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 1
  store i16 0, ptr %i28, align 2
  %i29 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 0
  store i16 0, ptr %i29, align 2
  %i30 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 3
  store i16 0, ptr %i30, align 2
  br label %bb60

bb31:                                             ; preds = %bb25
  %i32 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 2
  store i16 32767, ptr %i32, align 2
  %i33 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 1
  store i16 32767, ptr %i33, align 2
  %i34 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 0
  store i16 32767, ptr %i34, align 2
  %i35 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 3
  store i16 0, ptr %i35, align 2
  br label %bb60

bb36:                                             ; preds = %bb25
  %i37 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 2
  store i16 0, ptr %i37, align 2
  %i38 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 1
  store i16 0, ptr %i38, align 2
  %i39 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 0
  store i16 0, ptr %i39, align 2
  %i40 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 3
  store i16 -1, ptr %i40, align 2
  br label %bb60

bb41:                                             ; preds = %bb25, %bb25
  %i42 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 2
  store i16 -1, ptr %i42, align 2
  %i43 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 1
  store i16 -1, ptr %i43, align 2
  %i44 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 0
  store i16 -1, ptr %i44, align 2
  %i45 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 3
  store i16 0, ptr %i45, align 2
  br label %bb60

bb46:                                             ; preds = %bb25
  %i47 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 12
  %i48 = getelementptr inbounds %struct._PixelPacket, ptr %i47, i64 0, i32 0
  %i49 = load i16, ptr %i48, align 2
  %i50 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 0
  store i16 %i49, ptr %i50, align 2
  %i51 = getelementptr inbounds %struct._PixelPacket, ptr %i47, i64 0, i32 1
  %i52 = load i16, ptr %i51, align 2
  %i53 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 1
  store i16 %i52, ptr %i53, align 2
  %i54 = getelementptr inbounds %struct._PixelPacket, ptr %i47, i64 0, i32 2
  %i55 = load i16, ptr %i54, align 2
  %i56 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 2
  store i16 %i55, ptr %i56, align 2
  %i57 = getelementptr inbounds %struct._PixelPacket, ptr %i47, i64 0, i32 3
  %i58 = load i16, ptr %i57, align 2
  %i59 = getelementptr inbounds %struct._PixelPacket, ptr %i10, i64 0, i32 3
  store i16 %i58, ptr %i59, align 2
  br label %bb60

bb60:                                             ; preds = %bb46, %bb41, %bb36, %bb31, %bb26
  store i16 0, ptr %i, align 2
  %i61 = icmp sgt i64 %arg5, 0
  br i1 %i61, label %bb62, label %bb474

bb62:                                             ; preds = %bb60
  %i63 = and i32 %arg1, -5
  %i64 = icmp eq i32 %i63, 0
  %i65 = icmp sgt i64 %arg4, 0
  %i66 = getelementptr inbounds %struct._CacheInfo, ptr %i14, i64 0, i32 0
  %i67 = getelementptr inbounds %struct._CacheInfo, ptr %i14, i64 0, i32 21
  br label %bb68

bb68:                                             ; preds = %bb469, %bb62
  %i69 = phi ptr [ %i16, %bb62 ], [ %i471, %bb469 ]
  %i70 = phi ptr [ %arg8, %bb62 ], [ %i470, %bb469 ]
  %i71 = phi i64 [ 0, %bb62 ], [ %i472, %bb469 ]
  %i72 = add nsw i64 %i71, %arg3
  br i1 %i64, label %bb73, label %bb80

bb73:                                             ; preds = %bb68
  %i74 = load i64, ptr %i18, align 8
  %i75 = icmp slt i64 %i72, 0
  %i76 = icmp slt i64 %i72, %i74
  %i77 = add i64 %i74, -1
  %i78 = select i1 %i76, i64 %i72, i64 %i77
  %i79 = select i1 %i75, i64 0, i64 %i78
  br label %bb80

bb80:                                             ; preds = %bb73, %bb68
  %i81 = phi i64 [ %i72, %bb68 ], [ %i79, %bb73 ]
  br i1 %i65, label %bb82, label %bb469

bb82:                                             ; preds = %bb80
  %i83 = icmp slt i64 %i81, 0
  %i84 = ashr i64 %i81, 63
  %i85 = lshr i64 %i81, 63
  %i86 = and i64 %i81, 7
  %i87 = getelementptr inbounds [64 x i64], ptr @DitherMatrix, i64 0, i64 %i86
  br label %bb88

bb88:                                             ; preds = %bb463, %bb82
  %i89 = phi ptr [ %i69, %bb82 ], [ %i465, %bb463 ]
  %i90 = phi ptr [ %i70, %bb82 ], [ %i466, %bb463 ]
  %i91 = phi i64 [ 0, %bb82 ], [ %i467, %bb463 ]
  %i92 = add nsw i64 %i91, %arg2
  %i93 = load i64, ptr %i17, align 8
  %i94 = sub i64 %i93, %i92
  %i95 = sub i64 %arg4, %i91
  %i96 = icmp ult i64 %i94, %i95
  %i97 = select i1 %i96, i64 %i94, i64 %i95
  %i98 = icmp slt i64 %i92, 0
  %i99 = icmp sle i64 %i93, %i92
  %i100 = or i64 %i92, %i81
  %i101 = icmp slt i64 %i100, 0
  %i102 = or i1 %i101, %i99
  br i1 %i102, label %bb108, label %bb103

bb103:                                            ; preds = %bb88
  %i104 = load i64, ptr %i18, align 8
  %i105 = icmp sge i64 %i81, %i104
  %i106 = icmp eq i64 %i97, 0
  %i107 = or i1 %i106, %i105
  br i1 %i107, label %bb108, label %bb431

bb108:                                            ; preds = %bb103, %bb88
  switch i32 %arg1, label %bb109 [
    i32 1, label %bb409
    i32 2, label %bb409
    i32 10, label %bb409
    i32 11, label %bb409
    i32 8, label %bb409
    i32 9, label %bb409
    i32 12, label %bb409
    i32 16, label %bb380
    i32 6, label %bb134
    i32 3, label %bb168
    i32 7, label %bb203
    i32 5, label %bb229
    i32 17, label %bb265
    i32 13, label %bb295
    i32 14, label %bb324
    i32 15, label %bb354
  ]

bb109:                                            ; preds = %bb108
  %i110 = icmp sgt i64 %i93, %i92
  %i111 = add i64 %i93, -1
  %i112 = select i1 %i110, i64 %i92, i64 %i111
  %i113 = select i1 %i98, i64 0, i64 %i112
  %i114 = load i64, ptr %i18, align 8
  %i115 = icmp slt i64 %i81, %i114
  %i116 = add i64 %i114, -1
  %i117 = select i1 %i115, i64 %i81, i64 %i116
  %i118 = select i1 %i83, i64 0, i64 %i117
  %i119 = load ptr, ptr %i19, align 8
  %i120 = alloca i32, align 4
  store i32 0, ptr %i120, align 4
  %i121 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 %arg1, i64 %i113, i64 %i118, i64 1, i64 1, ptr %i119, ptr %arg7, ptr %i120)
  %i122 = load i32, ptr %i120, align 4
  %i123 = icmp eq i32 %i122, 1
  br i1 %i123, label %bb124, label %bb126

bb124:                                            ; preds = %bb109
  %i125 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 %arg1, i64 %i113, i64 %i118, i64 1, i64 1, ptr %i119, ptr %arg7, ptr %i121)
  br label %bb126

bb126:                                            ; preds = %bb124, %bb109
  %i127 = phi ptr [ %i121, %bb109 ], [ %i125, %bb124 ]
  %i128 = load i32, ptr %i66, align 8
  %i129 = icmp eq i32 %i128, 0
  br i1 %i129, label %bb405, label %bb130

bb130:                                            ; preds = %bb126
  %i131 = load ptr, ptr %i19, align 8
  %i132 = getelementptr inbounds %struct._NexusInfo, ptr %i131, i64 0, i32 6
  %i133 = load ptr, ptr %i132, align 8
  br label %bb405

bb134:                                            ; preds = %bb108
  %i135 = load ptr, ptr %i67, align 8
  %i136 = icmp eq ptr %i135, null
  br i1 %i136, label %bb137, label %bb140

bb137:                                            ; preds = %bb134
  %i138 = call ptr @AcquireRandomInfo()
  store ptr %i138, ptr %i67, align 8
  %i139 = load i64, ptr %i17, align 8
  br label %bb140

bb140:                                            ; preds = %bb137, %bb134
  %i141 = phi i64 [ %i139, %bb137 ], [ %i93, %bb134 ]
  %i142 = phi ptr [ %i138, %bb137 ], [ %i135, %bb134 ]
  %i143 = uitofp i64 %i141 to double
  %i144 = call fast double @GetPseudoRandomValue(ptr %i142)
  %i145 = fmul fast double %i144, %i143
  %i146 = fptosi double %i145 to i64
  %i147 = load ptr, ptr %i67, align 8
  %i148 = load i64, ptr %i18, align 8
  %i149 = uitofp i64 %i148 to double
  %i150 = call fast double @GetPseudoRandomValue(ptr %i147)
  %i151 = fmul fast double %i150, %i149
  %i152 = fptosi double %i151 to i64
  %i153 = load ptr, ptr %i19, align 8
  %i154 = alloca i32, align 4
  store i32 0, ptr %i154, align 4
  %i155 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 6, i64 %i146, i64 %i152, i64 1, i64 1, ptr %i153, ptr %arg7, ptr %i154)
  %i156 = load i32, ptr %i154, align 4
  %i157 = icmp eq i32 %i156, 1
  br i1 %i157, label %bb158, label %bb160

bb158:                                            ; preds = %bb140
  %i159 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 6, i64 %i146, i64 %i152, i64 1, i64 1, ptr %i153, ptr %arg7, ptr %i155)
  br label %bb160

bb160:                                            ; preds = %bb158, %bb140
  %i161 = phi ptr [ %i155, %bb140 ], [ %i159, %bb158 ]
  %i162 = load i32, ptr %i66, align 8
  %i163 = icmp eq i32 %i162, 0
  br i1 %i163, label %bb405, label %bb164

bb164:                                            ; preds = %bb160
  %i165 = load ptr, ptr %i19, align 8
  %i166 = getelementptr inbounds %struct._NexusInfo, ptr %i165, i64 0, i32 6
  %i167 = load ptr, ptr %i166, align 8
  br label %bb405

bb168:                                            ; preds = %bb108
  %i169 = and i64 %i92, 7
  %i170 = getelementptr inbounds [64 x i64], ptr @DitherMatrix, i64 0, i64 %i169
  %i171 = load i64, ptr %i170, align 8
  %i172 = add nsw i64 %i171, %i92
  %i173 = add nsw i64 %i172, -32
  %i174 = icmp slt i64 %i172, 32
  %i175 = icmp slt i64 %i173, %i93
  %i176 = add nsw i64 %i93, -1
  %i177 = select i1 %i175, i64 %i173, i64 %i176
  %i178 = select i1 %i174, i64 0, i64 %i177
  %i179 = load i64, ptr %i18, align 8
  %i180 = load i64, ptr %i87, align 8
  %i181 = add nsw i64 %i180, %i81
  %i182 = add nsw i64 %i181, -32
  %i183 = icmp slt i64 %i181, 32
  %i184 = icmp slt i64 %i182, %i179
  %i185 = add nsw i64 %i179, -1
  %i186 = select i1 %i184, i64 %i182, i64 %i185
  %i187 = select i1 %i183, i64 0, i64 %i186
  %i188 = load ptr, ptr %i19, align 8
  %i189 = alloca i32, align 4
  store i32 0, ptr %i189, align 4
  %i190 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 3, i64 %i178, i64 %i187, i64 1, i64 1, ptr %i188, ptr %arg7, ptr %i189)
  %i191 = load i32, ptr %i189, align 4
  %i192 = icmp eq i32 %i191, 1
  br i1 %i192, label %bb193, label %bb195

bb193:                                            ; preds = %bb168
  %i194 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 3, i64 %i178, i64 %i187, i64 1, i64 1, ptr %i188, ptr %arg7, ptr %i190)
  br label %bb195

bb195:                                            ; preds = %bb193, %bb168
  %i196 = phi ptr [ %i190, %bb168 ], [ %i194, %bb193 ]
  %i197 = load i32, ptr %i66, align 8
  %i198 = icmp eq i32 %i197, 0
  br i1 %i198, label %bb405, label %bb199

bb199:                                            ; preds = %bb195
  %i200 = load ptr, ptr %i19, align 8
  %i201 = getelementptr inbounds %struct._NexusInfo, ptr %i200, i64 0, i32 6
  %i202 = load ptr, ptr %i201, align 8
  br label %bb405

bb203:                                            ; preds = %bb108
  %i204 = sdiv i64 %i92, %i93
  %i205 = ashr i64 %i92, 63
  %i206 = add nsw i64 %i204, %i205
  %i207 = mul nsw i64 %i206, %i93
  %i208 = sub nsw i64 %i92, %i207
  %i209 = load i64, ptr %i18, align 8
  %i210 = sdiv i64 %i81, %i209
  %i211 = add nsw i64 %i210, %i84
  %i212 = mul nsw i64 %i211, %i209
  %i213 = sub nsw i64 %i81, %i212
  %i214 = load ptr, ptr %i19, align 8
  %i215 = alloca i32, align 4
  store i32 0, ptr %i215, align 4
  %i216 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 7, i64 %i208, i64 %i213, i64 1, i64 1, ptr %i214, ptr %arg7, ptr %i215)
  %i217 = load i32, ptr %i215, align 4
  %i218 = icmp eq i32 %i217, 1
  br i1 %i218, label %bb219, label %bb221

bb219:                                            ; preds = %bb203
  %i220 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 7, i64 %i208, i64 %i213, i64 1, i64 1, ptr %i214, ptr %arg7, ptr %i216)
  br label %bb221

bb221:                                            ; preds = %bb219, %bb203
  %i222 = phi ptr [ %i216, %bb203 ], [ %i220, %bb219 ]
  %i223 = load i32, ptr %i66, align 8
  %i224 = icmp eq i32 %i223, 0
  br i1 %i224, label %bb405, label %bb225

bb225:                                            ; preds = %bb221
  %i226 = load ptr, ptr %i19, align 8
  %i227 = getelementptr inbounds %struct._NexusInfo, ptr %i226, i64 0, i32 6
  %i228 = load ptr, ptr %i227, align 8
  br label %bb405

bb229:                                            ; preds = %bb108
  %i230 = sdiv i64 %i92, %i93
  %i231 = ashr i64 %i92, 63
  %i232 = add nsw i64 %i230, %i231
  %i233 = mul nsw i64 %i232, %i93
  %i234 = sub nsw i64 %i92, %i233
  %i235 = and i64 %i232, 1
  %i236 = icmp eq i64 %i235, 0
  %i237 = xor i64 %i234, -1
  %i238 = add i64 %i93, %i237
  %i239 = select i1 %i236, i64 %i234, i64 %i238
  %i240 = load i64, ptr %i18, align 8
  %i241 = sdiv i64 %i81, %i240
  %i242 = add nsw i64 %i241, %i84
  %i243 = mul nsw i64 %i242, %i240
  %i244 = sub nsw i64 %i81, %i243
  %i245 = and i64 %i242, 1
  %i246 = icmp eq i64 %i245, 0
  %i247 = xor i64 %i244, -1
  %i248 = add i64 %i240, %i247
  %i249 = select i1 %i246, i64 %i244, i64 %i248
  %i250 = load ptr, ptr %i19, align 8
  %i251 = alloca i32, align 4
  store i32 0, ptr %i251, align 4
  %i252 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 5, i64 %i239, i64 %i249, i64 1, i64 1, ptr %i250, ptr %arg7, ptr %i251)
  %i253 = load i32, ptr %i251, align 4
  %i254 = icmp eq i32 %i253, 1
  br i1 %i254, label %bb255, label %bb257

bb255:                                            ; preds = %bb229
  %i256 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 5, i64 %i239, i64 %i249, i64 1, i64 1, ptr %i250, ptr %arg7, ptr %i252)
  br label %bb257

bb257:                                            ; preds = %bb255, %bb229
  %i258 = phi ptr [ %i252, %bb229 ], [ %i256, %bb255 ]
  %i259 = load i32, ptr %i66, align 8
  %i260 = icmp eq i32 %i259, 0
  br i1 %i260, label %bb405, label %bb261

bb261:                                            ; preds = %bb257
  %i262 = load ptr, ptr %i19, align 8
  %i263 = getelementptr inbounds %struct._NexusInfo, ptr %i262, i64 0, i32 6
  %i264 = load ptr, ptr %i263, align 8
  br label %bb405

bb265:                                            ; preds = %bb108
  %i266 = sdiv i64 %i92, %i93
  %i267 = ashr i64 %i92, 63
  %i268 = add nsw i64 %i266, %i267
  %i269 = load i64, ptr %i18, align 8
  %i270 = sdiv i64 %i81, %i269
  %i271 = add nsw i64 %i270, %i84
  %i272 = xor i64 %i271, %i268
  %i273 = and i64 %i272, 1
  %i274 = icmp eq i64 %i273, 0
  br i1 %i274, label %bb275, label %bb409

bb275:                                            ; preds = %bb265
  %i276 = mul nsw i64 %i271, %i269
  %i277 = sub nsw i64 %i81, %i276
  %i278 = mul nsw i64 %i268, %i93
  %i279 = sub nsw i64 %i92, %i278
  %i280 = load ptr, ptr %i19, align 8
  %i281 = alloca i32, align 4
  store i32 0, ptr %i281, align 4
  %i282 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 17, i64 %i279, i64 %i277, i64 1, i64 1, ptr %i280, ptr %arg7, ptr %i281)
  %i283 = load i32, ptr %i281, align 4
  %i284 = icmp eq i32 %i283, 1
  br i1 %i284, label %bb285, label %bb287

bb285:                                            ; preds = %bb275
  %i286 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 17, i64 %i279, i64 %i277, i64 1, i64 1, ptr %i280, ptr %arg7, ptr %i282)
  br label %bb287

bb287:                                            ; preds = %bb285, %bb275
  %i288 = phi ptr [ %i282, %bb275 ], [ %i286, %bb285 ]
  %i289 = load i32, ptr %i66, align 8
  %i290 = icmp eq i32 %i289, 0
  br i1 %i290, label %bb405, label %bb291

bb291:                                            ; preds = %bb287
  %i292 = load ptr, ptr %i19, align 8
  %i293 = getelementptr inbounds %struct._NexusInfo, ptr %i292, i64 0, i32 6
  %i294 = load ptr, ptr %i293, align 8
  br label %bb405

bb295:                                            ; preds = %bb108
  br i1 %i83, label %bb409, label %bb296

bb296:                                            ; preds = %bb295
  %i297 = load i64, ptr %i18, align 8
  %i298 = icmp slt i64 %i81, %i297
  br i1 %i298, label %bb299, label %bb409

bb299:                                            ; preds = %bb296
  %i300 = sdiv i64 %i92, %i93
  %i301 = ashr i64 %i92, 63
  %i302 = add nsw i64 %i300, %i301
  %i303 = mul nsw i64 %i302, %i93
  %i304 = sub nsw i64 %i92, %i303
  %i305 = sdiv i64 %i81, %i297
  %i306 = add nuw nsw i64 %i305, %i85
  %i307 = mul nsw i64 %i306, %i297
  %i308 = sub nsw i64 %i81, %i307
  %i309 = load ptr, ptr %i19, align 8
  %i310 = alloca i32, align 4
  store i32 0, ptr %i310, align 4
  %i311 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 13, i64 %i304, i64 %i308, i64 1, i64 1, ptr %i309, ptr %arg7, ptr %i310)
  %i312 = load i32, ptr %i310, align 4
  %i313 = icmp eq i32 %i312, 1
  br i1 %i313, label %bb314, label %bb316

bb314:                                            ; preds = %bb299
  %i315 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 13, i64 %i304, i64 %i308, i64 1, i64 1, ptr %i309, ptr %arg7, ptr %i311)
  br label %bb316

bb316:                                            ; preds = %bb314, %bb299
  %i317 = phi ptr [ %i311, %bb299 ], [ %i315, %bb314 ]
  %i318 = load i32, ptr %i66, align 8
  %i319 = icmp eq i32 %i318, 0
  br i1 %i319, label %bb405, label %bb320

bb320:                                            ; preds = %bb316
  %i321 = load ptr, ptr %i19, align 8
  %i322 = getelementptr inbounds %struct._NexusInfo, ptr %i321, i64 0, i32 6
  %i323 = load ptr, ptr %i322, align 8
  br label %bb405

bb324:                                            ; preds = %bb108
  %i325 = xor i1 %i98, true
  %i326 = icmp sgt i64 %i93, %i92
  %i327 = and i1 %i326, %i325
  br i1 %i327, label %bb328, label %bb409

bb328:                                            ; preds = %bb324
  %i329 = sdiv i64 %i92, %i93
  %i330 = lshr i64 %i92, 63
  %i331 = add nuw nsw i64 %i329, %i330
  %i332 = mul nsw i64 %i331, %i93
  %i333 = sub nsw i64 %i92, %i332
  %i334 = load i64, ptr %i18, align 8
  %i335 = sdiv i64 %i81, %i334
  %i336 = add nsw i64 %i335, %i84
  %i337 = mul nsw i64 %i336, %i334
  %i338 = sub nsw i64 %i81, %i337
  %i339 = load ptr, ptr %i19, align 8
  %i340 = alloca i32, align 4
  store i32 0, ptr %i340, align 4
  %i341 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 14, i64 %i333, i64 %i338, i64 1, i64 1, ptr %i339, ptr %arg7, ptr %i340)
  %i342 = load i32, ptr %i340, align 4
  %i343 = icmp eq i32 %i342, 1
  br i1 %i343, label %bb344, label %bb346

bb344:                                            ; preds = %bb328
  %i345 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 14, i64 %i333, i64 %i338, i64 1, i64 1, ptr %i339, ptr %arg7, ptr %i341)
  br label %bb346

bb346:                                            ; preds = %bb344, %bb328
  %i347 = phi ptr [ %i341, %bb328 ], [ %i345, %bb344 ]
  %i348 = load i32, ptr %i66, align 8
  %i349 = icmp eq i32 %i348, 0
  br i1 %i349, label %bb405, label %bb350

bb350:                                            ; preds = %bb346
  %i351 = load ptr, ptr %i19, align 8
  %i352 = getelementptr inbounds %struct._NexusInfo, ptr %i351, i64 0, i32 6
  %i353 = load ptr, ptr %i352, align 8
  br label %bb405

bb354:                                            ; preds = %bb108
  %i355 = sdiv i64 %i92, %i93
  %i356 = ashr i64 %i92, 63
  %i357 = add nsw i64 %i355, %i356
  %i358 = mul nsw i64 %i357, %i93
  %i359 = sub nsw i64 %i92, %i358
  %i360 = load i64, ptr %i18, align 8
  %i361 = icmp slt i64 %i81, %i360
  %i362 = add i64 %i360, -1
  %i363 = select i1 %i361, i64 %i81, i64 %i362
  %i364 = select i1 %i83, i64 0, i64 %i363
  %i365 = load ptr, ptr %i19, align 8
  %i366 = alloca i32, align 4
  store i32 0, ptr %i366, align 4
  %i367 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 15, i64 %i359, i64 %i364, i64 1, i64 1, ptr %i365, ptr %arg7, ptr %i366)
  %i368 = load i32, ptr %i366, align 4
  %i369 = icmp eq i32 %i368, 1
  br i1 %i369, label %bb370, label %bb372

bb370:                                            ; preds = %bb354
  %i371 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 15, i64 %i359, i64 %i364, i64 1, i64 1, ptr %i365, ptr %arg7, ptr %i367)
  br label %bb372

bb372:                                            ; preds = %bb370, %bb354
  %i373 = phi ptr [ %i367, %bb354 ], [ %i371, %bb370 ]
  %i374 = load i32, ptr %i66, align 8
  %i375 = icmp eq i32 %i374, 0
  br i1 %i375, label %bb405, label %bb376

bb376:                                            ; preds = %bb372
  %i377 = load ptr, ptr %i19, align 8
  %i378 = getelementptr inbounds %struct._NexusInfo, ptr %i377, i64 0, i32 6
  %i379 = load ptr, ptr %i378, align 8
  br label %bb405

bb380:                                            ; preds = %bb108
  %i381 = load i64, ptr %i18, align 8
  %i382 = sdiv i64 %i81, %i381
  %i383 = add nsw i64 %i382, %i84
  %i384 = mul nsw i64 %i383, %i381
  %i385 = sub nsw i64 %i81, %i384
  %i386 = icmp sgt i64 %i93, %i92
  %i387 = add i64 %i93, -1
  %i388 = select i1 %i386, i64 %i92, i64 %i387
  %i389 = select i1 %i98, i64 0, i64 %i388
  %i390 = load ptr, ptr %i19, align 8
  %i391 = alloca i32, align 4
  store i32 0, ptr %i391, align 4
  %i392 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 16, i64 %i389, i64 %i385, i64 1, i64 1, ptr %i390, ptr %arg7, ptr %i391)
  %i393 = load i32, ptr %i391, align 4
  %i394 = icmp eq i32 %i393, 1
  br i1 %i394, label %bb395, label %bb397

bb395:                                            ; preds = %bb380
  %i396 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 16, i64 %i389, i64 %i385, i64 1, i64 1, ptr %i390, ptr %arg7, ptr %i392)
  br label %bb397

bb397:                                            ; preds = %bb395, %bb380
  %i398 = phi ptr [ %i392, %bb380 ], [ %i396, %bb395 ]
  %i399 = load i32, ptr %i66, align 8
  %i400 = icmp eq i32 %i399, 0
  br i1 %i400, label %bb405, label %bb401

bb401:                                            ; preds = %bb397
  %i402 = load ptr, ptr %i19, align 8
  %i403 = getelementptr inbounds %struct._NexusInfo, ptr %i402, i64 0, i32 6
  %i404 = load ptr, ptr %i403, align 8
  br label %bb405

bb405:                                            ; preds = %bb401, %bb397, %bb376, %bb372, %bb350, %bb346, %bb320, %bb316, %bb291, %bb287, %bb261, %bb257, %bb225, %bb221, %bb199, %bb195, %bb164, %bb160, %bb130, %bb126
  %i406 = phi ptr [ %i127, %bb126 ], [ %i127, %bb130 ], [ %i161, %bb160 ], [ %i161, %bb164 ], [ %i196, %bb195 ], [ %i196, %bb199 ], [ %i222, %bb221 ], [ %i222, %bb225 ], [ %i258, %bb257 ], [ %i258, %bb261 ], [ %i288, %bb287 ], [ %i288, %bb291 ], [ %i317, %bb316 ], [ %i317, %bb320 ], [ %i347, %bb346 ], [ %i347, %bb350 ], [ %i373, %bb372 ], [ %i373, %bb376 ], [ %i398, %bb397 ], [ %i398, %bb401 ]
  %i407 = phi ptr [ null, %bb126 ], [ %i133, %bb130 ], [ null, %bb160 ], [ %i167, %bb164 ], [ null, %bb195 ], [ %i202, %bb199 ], [ null, %bb221 ], [ %i228, %bb225 ], [ null, %bb257 ], [ %i264, %bb261 ], [ null, %bb287 ], [ %i294, %bb291 ], [ null, %bb316 ], [ %i323, %bb320 ], [ null, %bb346 ], [ %i353, %bb350 ], [ null, %bb372 ], [ %i379, %bb376 ], [ null, %bb397 ], [ %i404, %bb401 ]
  %i408 = icmp eq ptr %i406, null
  br i1 %i408, label %bb469, label %bb409

bb409:                                            ; preds = %bb405, %bb324, %bb296, %bb295, %bb265, %bb108, %bb108, %bb108, %bb108, %bb108, %bb108, %bb108
  %i410 = phi ptr [ %i407, %bb405 ], [ %i, %bb108 ], [ %i, %bb108 ], [ %i, %bb108 ], [ %i, %bb108 ], [ %i, %bb108 ], [ %i, %bb108 ], [ %i, %bb108 ], [ %i, %bb265 ], [ %i, %bb296 ], [ %i, %bb295 ], [ %i, %bb324 ]
  %i411 = phi ptr [ %i406, %bb405 ], [ %i10, %bb108 ], [ %i10, %bb108 ], [ %i10, %bb108 ], [ %i10, %bb108 ], [ %i10, %bb108 ], [ %i10, %bb108 ], [ %i10, %bb108 ], [ %i10, %bb265 ], [ %i10, %bb296 ], [ %i10, %bb295 ], [ %i10, %bb324 ]
  %i412 = getelementptr inbounds %struct._PixelPacket, ptr %i90, i64 1
  %i413 = getelementptr inbounds %struct._PixelPacket, ptr %i411, i64 0, i32 0
  %i414 = load i16, ptr %i413, align 2
  %i415 = getelementptr inbounds %struct._PixelPacket, ptr %i90, i64 0, i32 0
  store i16 %i414, ptr %i415, align 2
  %i416 = getelementptr inbounds %struct._PixelPacket, ptr %i411, i64 0, i32 1
  %i417 = load i16, ptr %i416, align 2
  %i418 = getelementptr inbounds %struct._PixelPacket, ptr %i90, i64 0, i32 1
  store i16 %i417, ptr %i418, align 2
  %i419 = getelementptr inbounds %struct._PixelPacket, ptr %i411, i64 0, i32 2
  %i420 = load i16, ptr %i419, align 2
  %i421 = getelementptr inbounds %struct._PixelPacket, ptr %i90, i64 0, i32 2
  store i16 %i420, ptr %i421, align 2
  %i422 = getelementptr inbounds %struct._PixelPacket, ptr %i411, i64 0, i32 3
  %i423 = load i16, ptr %i422, align 2
  %i424 = getelementptr inbounds %struct._PixelPacket, ptr %i90, i64 0, i32 3
  store i16 %i423, ptr %i424, align 2
  %i425 = icmp ne ptr %i89, null
  %i426 = icmp ne ptr %i410, null
  %i427 = and i1 %i425, %i426
  br i1 %i427, label %bb428, label %bb463

bb428:                                            ; preds = %bb409
  %i429 = load i16, ptr %i410, align 2
  %i430 = getelementptr inbounds i16, ptr %i89, i64 1
  store i16 %i429, ptr %i89, align 2
  br label %bb463

bb431:                                            ; preds = %bb103
  %i432 = load ptr, ptr %i19, align 8
  %i433 = alloca i32, align 4
  store i32 0, ptr %i433, align 4
  %i434 = call ptr @GetVirtualPixelsFromNexus.split.0(ptr %arg, i32 %arg1, i64 %i92, i64 %i81, i64 %i97, i64 1, ptr %i432, ptr %arg7, ptr %i433)
  %i435 = load i32, ptr %i433, align 4
  %i436 = icmp eq i32 %i435, 1
  br i1 %i436, label %bb437, label %bb439

bb437:                                            ; preds = %bb431
  %i438 = call ptr @GetVirtualPixelsFromNexus.split.1(ptr %arg, i32 %arg1, i64 %i92, i64 %i81, i64 %i97, i64 1, ptr %i432, ptr %arg7, ptr %i434)
  br label %bb439

bb439:                                            ; preds = %bb437, %bb431
  %i440 = phi ptr [ %i434, %bb431 ], [ %i438, %bb437 ]
  %i441 = icmp eq ptr %i440, null
  br i1 %i441, label %bb469, label %bb442

bb442:                                            ; preds = %bb439
  %i443 = load i32, ptr %i66, align 8
  %i444 = icmp eq i32 %i443, 0
  br i1 %i444, label %bb449, label %bb445

bb445:                                            ; preds = %bb442
  %i446 = load ptr, ptr %i19, align 8
  %i447 = getelementptr inbounds %struct._NexusInfo, ptr %i446, i64 0, i32 6
  %i448 = load ptr, ptr %i447, align 8
  br label %bb449

bb449:                                            ; preds = %bb445, %bb442
  %i450 = phi ptr [ %i448, %bb445 ], [ null, %bb442 ]
  %i453 = shl i64 %i97, 3
  call void @llvm.memcpy.p0.p0.i64(ptr align 2 %i90, ptr nonnull align 2 %i440, i64 %i453, i1 false)
  %i454 = getelementptr inbounds %struct._PixelPacket, ptr %i90, i64 %i97
  %i455 = icmp ne ptr %i89, null
  %i456 = icmp ne ptr %i450, null
  %i457 = and i1 %i455, %i456
  br i1 %i457, label %bb458, label %bb463

bb458:                                            ; preds = %bb449
  %i461 = shl i64 %i97, 1
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i89, ptr nonnull align 2 %i450, i64 %i461, i1 false)
  %i462 = getelementptr inbounds i16, ptr %i89, i64 %i97
  br label %bb463

bb463:                                            ; preds = %bb458, %bb449, %bb428, %bb409
  %i464 = phi i64 [ %i97, %bb449 ], [ %i97, %bb458 ], [ 1, %bb428 ], [ 1, %bb409 ]
  %i465 = phi ptr [ %i89, %bb449 ], [ %i462, %bb458 ], [ %i430, %bb428 ], [ %i89, %bb409 ]
  %i466 = phi ptr [ %i454, %bb449 ], [ %i454, %bb458 ], [ %i412, %bb428 ], [ %i412, %bb409 ]
  %i467 = add i64 %i464, %i91
  %i468 = icmp slt i64 %i467, %arg4
  br i1 %i468, label %bb88, label %bb469

bb469:                                            ; preds = %bb463, %bb439, %bb405, %bb80
  %i470 = phi ptr [ %i70, %bb80 ], [ %i466, %bb463 ], [ %i90, %bb439 ], [ %i90, %bb405 ]
  %i471 = phi ptr [ %i69, %bb80 ], [ %i465, %bb463 ], [ %i89, %bb439 ], [ %i89, %bb405 ]
  %i472 = add nuw nsw i64 %i71, 1
  %i473 = icmp eq i64 %i472, %arg5
  br i1 %i473, label %bb474, label %bb68

bb474:                                            ; preds = %bb469, %bb60
  %i475 = load ptr, ptr %i19, align 8
  %i476 = getelementptr inbounds %struct._NexusInfo, ptr %i475, i64 0, i32 3
  %i477 = load ptr, ptr %i476, align 8
  %i478 = icmp eq ptr %i477, null
  br i1 %i478, label %bb494, label %bb480

bb480:                                            ; preds = %bb474
  %i481 = getelementptr inbounds %struct._NexusInfo, ptr %i475, i64 0, i32 0
  %i482 = load i32, ptr %i481, align 8
  %i483 = icmp eq i32 %i482, 0
  br i1 %i483, label %bb484, label %bb486

bb484:                                            ; preds = %bb480
  %i485 = call ptr @RelinquishAlignedMemory(ptr nonnull %i477)
  br label %bb489

bb486:                                            ; preds = %bb480
  %i487 = getelementptr inbounds %struct._NexusInfo, ptr %i475, i64 0, i32 2
  %i488 = load i64, ptr %i487, align 8
  br label %bb489

bb489:                                            ; preds = %bb486, %bb484
  store ptr null, ptr %i476, align 8
  %i490 = getelementptr inbounds %struct._NexusInfo, ptr %i475, i64 0, i32 4
  store ptr null, ptr %i490, align 8
  %i491 = getelementptr inbounds %struct._NexusInfo, ptr %i475, i64 0, i32 6
  store ptr null, ptr %i491, align 8
  %i492 = getelementptr inbounds %struct._NexusInfo, ptr %i475, i64 0, i32 2
  store i64 0, ptr %i492, align 8
  store i32 0, ptr %i481, align 8
  %i493 = load ptr, ptr %i19, align 8
  br label %bb494

bb494:                                            ; preds = %bb489, %bb474
  %i495 = phi ptr [ %i493, %bb489 ], [ %i475, %bb474 ]
  %i496 = getelementptr inbounds %struct._NexusInfo, ptr %i495, i64 0, i32 7
  store i64 -2880220588, ptr %i496, align 8
  %i498 = load ptr, ptr %i19, align 8
  %i499 = call ptr @RelinquishMagickMemory(ptr %i498)
  store ptr null, ptr %i19, align 8
  %i501 = call ptr @RelinquishAlignedMemory(ptr nonnull %i19)
  br label %bb502

bb502:                                            ; preds = %bb494, %bb21
  %i503 = phi ptr [ null, %bb21 ], [ %arg8, %bb494 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i10)
  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %i)
  ret ptr %i503
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nofree nounwind willreturn }
attributes #3 = { "prefer-inline-mrc-split" }
; end INTEL_FEATURE_SW_ADVANCED
