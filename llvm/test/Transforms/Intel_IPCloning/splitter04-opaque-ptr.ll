; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt < %s -opaque-pointers -passes='module(ip-cloning)' -force-ip-manyreccalls-splitting -debug-only=ipcloning -S 2>&1 | FileCheck %s

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
; CHECK-DAG: MRCS:   %i = alloca i16, align 2
; CHECK-DAG: MRCS:   %i8 = alloca %struct._PixelPacket, align 2
; CHECK-DAG: MRCS:   %i13 = load ptr, ptr %i11, align 8
; CHECK-DAG: MRCS:   %i178 = phi ptr [ %i139, %bb165 ], [ %i173, %bb172 ]
; CHECK-DAG: MRCS:   %i182 = phi ptr [ %i130, %bb165 ], [ %i66, %bb172 ]
; CHECK-DAG: MRCS:   %i183 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 6
; CHECK-DAG: MRCS:   %i184 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 7
; CHECK: MRCS: End split insts
; CHECK: MRCS: EXIT: canReloadPHI: Could not find storebacks for PHINode
; CHECK: MRCS: Ignore previous EXIT. Saving PHI as SplitValue
; CHECK: MRCS: Validated split insts:   %i182 = phi ptr [ %i130, %bb165 ], [ %i66, %bb172 ]
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
; CHECK: store i32 0, ptr [[R0:%[0-9]+]], align 4
; CHECK: [[R1:%[0-9]+]] = call{{.*}}@GetVirtualPixelsFromNexus.1({{.*}} [[R0]]) #[[A0]]
; CHECK: call{{.*}}@GetVirtualPixelsFromNexus.2({{.*}} [[R1]])
; CHECK: define internal ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef %arg1, i64 noundef %arg2, i64 noundef %arg3, i64 noundef %arg4, i64 noundef %arg5, ptr noundef %arg6, ptr noundef %arg7)
; CHECK: define internal ptr @GetVirtualPixelsFromNexus.1(ptr noundef %0, i32 noundef %1, i64 noundef %2, i64 noundef %3, i64 noundef %4, i64 noundef %5, ptr noundef %6, ptr noundef %7, ptr %8) #[[A1:[0-9]+]]
; CHECK: define internal ptr @GetVirtualPixelsFromNexus.2(ptr noundef %0, i32 noundef %1, i64 noundef %2, i64 noundef %3, i64 noundef %4, i64 noundef %5, ptr noundef %6, ptr noundef %7, ptr %8) #[[A1]]
; CHECK: store i32 0, ptr [[R2:%[0-9]+]], align 4
; CHECK: [[R3:%[0-9]+]] = call{{.*}}@GetVirtualPixelsFromNexus.1({{.*}} [[R2]])
; CHECK: call{{.*}}@GetVirtualPixelsFromNexus.2({{.*}} [[R3]])
; CHECK: store i32 0, ptr [[R4:%[0-9]+]], align 4
; CHECK: [[R5:%[0-9]+]] = call{{.*}}@GetVirtualPixelsFromNexus.1({{.*}} [[R4]])
; CHECK: call{{.*}}@GetVirtualPixelsFromNexus.2({{.*}} [[R5]])
; CHECK: attributes #[[A1]] = { "ip-clone-split-function" }
; CHECK: attributes #[[A0]] = { "prefer-inline-mrc-split" }

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

define internal ptr @MeanShiftImage(ptr %arg, i64 %arg1, i64 %arg2, double %arg3, ptr %arg4) {
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
  %i153 = call i32 @GetOneCacheViewVirtualPixel(ptr %i29, i64 %i152, i64 %i139, ptr nonnull %i7, ptr %arg4)
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

define internal i32 @GetOneCacheViewVirtualPixel(ptr noalias nocapture readonly %arg, i64 %arg1, i64 %arg2, ptr noalias nocapture %arg3, ptr %arg4) {
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
  %i24 = tail call ptr @GetVirtualPixelsFromNexus(ptr %i5, i32 %i20, i64 %arg1, i64 %arg2, i64 1, i64 1, ptr %i23, ptr %arg4)
  %i25 = icmp eq ptr %i24, null
  br i1 %i25, label %bb35, label %bb26

bb26:                                             ; preds = %bb
  %i27 = getelementptr inbounds %struct._PixelPacket, ptr %i24, i64 0, i32 0
  %i28 = load i16, ptr %i27, align 2
  store i16 %i28, ptr %i9, align 2
  %i29 = getelementptr inbounds %struct._PixelPacket, ptr %i24, i64 0, i32 1
  %i30 = load i16, ptr %i29, align 2
  store i16 %i30, ptr %i12, align 2
  %i31 = getelementptr inbounds %struct._PixelPacket, ptr %i24, i64 0, i32 2
  %i32 = load i16, ptr %i31, align 2
  store i16 %i32, ptr %i15, align 2
  %i33 = getelementptr inbounds %struct._PixelPacket, ptr %i24, i64 0, i32 3
  %i34 = load i16, ptr %i33, align 2
  store i16 %i34, ptr %i18, align 2
  br label %bb35

bb35:                                             ; preds = %bb26, %bb
  %i36 = phi i32 [ 1, %bb26 ], [ 0, %bb ]
  ret i32 %i36
}

define internal ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef %arg1, i64 noundef %arg2, i64 noundef %arg3, i64 noundef %arg4, i64 noundef %arg5, ptr noundef %arg6, ptr noundef %arg7) {
bb:
  %i = alloca i16, align 2
  %i8 = alloca %struct._PixelPacket, align 2
  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %i)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i8)
  %i11 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 49
  %i13 = load ptr, ptr %i11, align 8
  %i14 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 3
  %i15 = load i32, ptr %i14, align 8
  %i16 = icmp eq i32 %i15, 0
  br i1 %i16, label %bb624, label %bb17

bb17:                                             ; preds = %bb
  %i18 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 38
  %i19 = load ptr, ptr %i18, align 8
  %i20 = icmp eq ptr %i19, null
  br i1 %i20, label %bb21, label %bb25

bb21:                                             ; preds = %bb17
  %i22 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 73
  %i23 = load ptr, ptr %i22, align 8
  %i24 = icmp ne ptr %i23, null
  br label %bb25

bb25:                                             ; preds = %bb21, %bb17
  %i26 = phi i1 [ true, %bb17 ], [ %i24, %bb21 ]
  %i27 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1
  %i28 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 0
  store i64 %arg4, ptr %i28, align 8
  %i29 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 1
  store i64 %arg5, ptr %i29, align 8
  %i30 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 2
  store i64 %arg2, ptr %i30, align 8
  %i31 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 3
  store i64 %arg3, ptr %i31, align 8
  %i32 = load i32, ptr %i14, align 8
  %i33 = icmp eq i32 %i32, 1
  br i1 %i33, label %bb37, label %bb34

bb34:                                             ; preds = %bb25
  %i35 = icmp ne i32 %i32, 2
  %i36 = or i1 %i26, %i35
  br i1 %i36, label %bb76, label %bb38

bb37:                                             ; preds = %bb25
  br i1 %i26, label %bb76, label %bb38

bb38:                                             ; preds = %bb37, %bb34
  %i39 = add nsw i64 %arg5, %arg3
  %i40 = icmp sgt i64 %arg2, -1
  br i1 %i40, label %bb41, label %bb76

bb41:                                             ; preds = %bb38
  %i42 = add nsw i64 %arg4, %arg2
  %i43 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 6
  %i44 = load i64, ptr %i43, align 8
  %i45 = icmp sle i64 %i42, %i44
  %i46 = icmp sgt i64 %arg3, -1
  %i47 = and i1 %i45, %i46
  br i1 %i47, label %bb48, label %bb76

bb48:                                             ; preds = %bb41
  %i49 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 7
  %i50 = load i64, ptr %i49, align 8
  %i51 = icmp sgt i64 %i39, %i50
  br i1 %i51, label %bb76, label %bb52

bb52:                                             ; preds = %bb48
  %i53 = icmp eq i64 %arg5, 1
  br i1 %i53, label %bb61, label %bb54

bb54:                                             ; preds = %bb52
  %i55 = icmp eq i64 %arg2, 0
  br i1 %i55, label %bb56, label %bb76

bb56:                                             ; preds = %bb54
  %i57 = icmp eq i64 %i44, %arg4
  br i1 %i57, label %bb61, label %bb58

bb58:                                             ; preds = %bb56
  %i59 = urem i64 %arg4, %i44
  %i60 = icmp eq i64 %i59, 0
  br i1 %i60, label %bb61, label %bb76

bb61:                                             ; preds = %bb58, %bb56, %bb52
  %i62 = mul i64 %i44, %arg3
  %i63 = add i64 %i62, %arg2
  %i64 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 14
  %i65 = load ptr, ptr %i64, align 8
  %i66 = getelementptr inbounds %struct._PixelPacket, ptr %i65, i64 %i63
  %i67 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 4
  store ptr %i66, ptr %i67, align 8
  %i68 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  store ptr null, ptr %i68, align 8
  %i69 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 16
  %i70 = load i32, ptr %i69, align 8
  %i71 = icmp eq i32 %i70, 0
  br i1 %i71, label %bb172, label %bb72

bb72:                                             ; preds = %bb61
  %i73 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 15
  %i74 = load ptr, ptr %i73, align 8
  %i75 = getelementptr inbounds i16, ptr %i74, i64 %i63
  store ptr %i75, ptr %i68, align 8
  br label %bb172

bb76:                                             ; preds = %bb58, %bb54, %bb48, %bb41, %bb38, %bb37, %bb34
  %i77 = mul i64 %arg5, %arg4
  %i78 = shl i64 %i77, 3
  %i79 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 16
  %i80 = load i32, ptr %i79, align 8
  %i81 = icmp eq i32 %i80, 0
  %i82 = mul i64 %i77, 10
  %i83 = select i1 %i81, i64 %i78, i64 %i82
  %i84 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 3
  %i85 = load ptr, ptr %i84, align 8
  %i86 = icmp eq ptr %i85, null
  br i1 %i86, label %bb88, label %bb103

bb88:                                             ; preds = %bb76
  %i89 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 2
  store i64 %i83, ptr %i89, align 8
  %i90 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 0
  store i32 0, ptr %i90, align 8
  %i91 = tail call ptr @AcquireAlignedMemory(i64 noundef 1, i64 noundef %i83)
  store ptr %i91, ptr %i84, align 8
  %i93 = icmp eq ptr %i91, null
  br i1 %i93, label %bb94, label %bb96

bb94:                                             ; preds = %bb88
  store i32 1, ptr %i90, align 8
  %i95 = load i64, ptr %i89, align 8
  store ptr null, ptr %i84, align 8
  br label %bb96

bb96:                                             ; preds = %bb94, %bb88
  %i97 = phi ptr [ null, %bb94 ], [ %i91, %bb88 ]
  %i98 = icmp eq ptr %i97, null
  br i1 %i98, label %bb99, label %bb129

bb99:                                             ; preds = %bb96
  %i100 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 18
  %i101 = getelementptr inbounds [4096 x i8], ptr %i100, i64 0, i64 0
  %i102 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr noundef %arg7, ptr noundef @.str.129, ptr noundef @.str.1.130, i64 noundef 4688, i32 noundef 400, ptr noundef @.str.2.131, ptr noundef @.str.3.132, ptr noundef nonnull %i101)
  store i64 0, ptr %i89, align 8
  br label %bb624

bb103:                                            ; preds = %bb76
  %i104 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 2
  %i105 = load i64, ptr %i104, align 8
  %i106 = icmp ult i64 %i105, %i83
  br i1 %i106, label %bb107, label %bb129

bb107:                                            ; preds = %bb103
  %i108 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 0
  %i109 = load i32, ptr %i108, align 8
  %i110 = icmp eq i32 %i109, 0
  br i1 %i110, label %bb111, label %bb113

bb111:                                            ; preds = %bb107
  %i112 = tail call ptr @RelinquishAlignedMemory(ptr noundef nonnull %i85)
  br label %bb114

bb113:                                            ; preds = %bb107
  br label %bb114

bb114:                                            ; preds = %bb113, %bb111
  store ptr null, ptr %i84, align 8
  %i115 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 4
  store ptr null, ptr %i115, align 8
  %i116 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  store ptr null, ptr %i116, align 8
  store i64 %i83, ptr %i104, align 8
  store i32 0, ptr %i108, align 8
  %i117 = tail call ptr @AcquireAlignedMemory(i64 noundef 1, i64 noundef %i83)
  store ptr %i117, ptr %i84, align 8
  %i119 = icmp eq ptr %i117, null
  br i1 %i119, label %bb120, label %bb122

bb120:                                            ; preds = %bb114
  store i32 1, ptr %i108, align 8
  %i121 = load i64, ptr %i104, align 8
  store ptr null, ptr %i84, align 8
  br label %bb122

bb122:                                            ; preds = %bb120, %bb114
  %i123 = phi ptr [ null, %bb120 ], [ %i117, %bb114 ]
  %i124 = icmp eq ptr %i123, null
  br i1 %i124, label %bb125, label %bb129

bb125:                                            ; preds = %bb122
  %i126 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 18
  %i127 = getelementptr inbounds [4096 x i8], ptr %i126, i64 0, i64 0
  %i128 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr noundef %arg7, ptr noundef @.str.129, ptr noundef @.str.1.130, i64 noundef 4688, i32 noundef 400, ptr noundef @.str.2.131, ptr noundef @.str.3.132, ptr noundef nonnull %i127)
  store i64 0, ptr %i104, align 8
  br label %bb624

bb129:                                            ; preds = %bb122, %bb103, %bb96
  %i130 = phi ptr [ %i123, %bb122 ], [ %i97, %bb96 ], [ %i85, %bb103 ]
  %i131 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 4
  store ptr %i130, ptr %i131, align 8
  %i132 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  store ptr null, ptr %i132, align 8
  %i133 = load i32, ptr %i79, align 8
  %i134 = icmp eq i32 %i133, 0
  br i1 %i134, label %bb138, label %bb135

bb135:                                            ; preds = %bb129
  %i136 = getelementptr inbounds %struct._PixelPacket, ptr %i130, i64 %i77
  %i137 = getelementptr %struct._PixelPacket, ptr %i136, i64 0, i32 0
  store ptr %i137, ptr %i132, align 8
  br label %bb138

bb138:                                            ; preds = %bb135, %bb129
  %i139 = phi ptr [ %i137, %bb135 ], [ null, %bb129 ]
  %i140 = load i32, ptr %i14, align 8
  %i141 = icmp eq i32 %i140, 4
  br i1 %i141, label %bb142, label %bb151

bb142:                                            ; preds = %bb138
  %i143 = getelementptr inbounds %struct._RectangleInfo, ptr %i27, i64 0, i32 3
  %i144 = load i64, ptr %i143, align 8
  %i145 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 6
  %i146 = load i64, ptr %i145, align 8
  %i147 = getelementptr inbounds %struct._RectangleInfo, ptr %i27, i64 0, i32 2
  %i148 = load i64, ptr %i147, align 8
  %i149 = mul i64 %i146, %i144
  %i150 = add i64 %i149, %i148
  br label %bb165

bb151:                                            ; preds = %bb138
  %i152 = getelementptr inbounds %struct._RectangleInfo, ptr %i27, i64 0, i32 3
  %i153 = load i64, ptr %i152, align 8
  %i154 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 6
  %i155 = load i64, ptr %i154, align 8
  %i156 = mul i64 %i155, %i153
  %i157 = getelementptr inbounds %struct._RectangleInfo, ptr %i27, i64 0, i32 2
  %i158 = load i64, ptr %i157, align 8
  %i159 = add i64 %i156, %i158
  %i160 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 14
  %i161 = load ptr, ptr %i160, align 8
  %i162 = getelementptr inbounds %struct._PixelPacket, ptr %i161, i64 %i159
  %i163 = icmp eq ptr %i130, %i162
  %i164 = zext i1 %i163 to i32
  br label %bb165

bb165:                                            ; preds = %bb151, %bb142
  %i166 = phi i64 [ %i150, %bb142 ], [ %i159, %bb151 ]
  %i167 = phi i64 [ %i146, %bb142 ], [ %i155, %bb151 ]
  %i168 = phi i32 [ 1, %bb142 ], [ %i164, %bb151 ]
  %i169 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 5
  store i32 %i168, ptr %i169, align 8
  %i170 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 7
  %i171 = load i64, ptr %i170, align 8
  br label %bb176

bb172:                                            ; preds = %bb72, %bb61
  %i173 = phi ptr [ null, %bb61 ], [ %i75, %bb72 ]
  %i174 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 5
  store i32 1, ptr %i174, align 8
  %i175 = icmp eq ptr %i65, null
  br i1 %i175, label %bb624, label %bb176

bb176:                                            ; preds = %bb172, %bb165
  %i177 = phi i64 [ %i166, %bb165 ], [ %i63, %bb172 ]
  %i178 = phi ptr [ %i139, %bb165 ], [ %i173, %bb172 ]
  %i179 = phi i32 [ %i168, %bb165 ], [ 1, %bb172 ]
  %i180 = phi i64 [ %i171, %bb165 ], [ %i50, %bb172 ]
  %i181 = phi i64 [ %i167, %bb165 ], [ %i44, %bb172 ]
  %i182 = phi ptr [ %i130, %bb165 ], [ %i66, %bb172 ]
  %i183 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 6
  %i184 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 7
  %i185 = icmp sgt i64 %i177, -1
  br i1 %i185, label %bb186, label %bb225

bb186:                                            ; preds = %bb176
  %i187 = mul i64 %i180, %i181
  %i188 = getelementptr inbounds %struct._RectangleInfo, ptr %i27, i64 0, i32 1
  %i189 = load i64, ptr %i188, align 8
  %i190 = add i64 %i189, -1
  %i191 = mul i64 %i190, %i181
  %i192 = getelementptr inbounds %struct._RectangleInfo, ptr %i27, i64 0, i32 0
  %i193 = load i64, ptr %i192, align 8
  %i194 = add nsw i64 %i177, -1
  %i195 = add i64 %i194, %i193
  %i196 = add i64 %i195, %i191
  %i197 = icmp ult i64 %i196, %i187
  %i198 = icmp sgt i64 %arg2, -1
  %i199 = and i1 %i197, %i198
  br i1 %i199, label %bb200, label %bb225

bb200:                                            ; preds = %bb186
  %i201 = add i64 %arg4, %arg2
  %i202 = icmp sgt i64 %i201, %i181
  %i203 = icmp slt i64 %arg3, 0
  %i204 = or i1 %i202, %i203
  %i205 = add i64 %arg5, %arg3
  %i206 = icmp sgt i64 %i205, %i180
  %i207 = select i1 %i204, i1 true, i1 %i206
  br i1 %i207, label %bb225, label %bb208

bb208:                                            ; preds = %bb200
  %i209 = icmp eq i32 %i179, 0
  br i1 %i209, label %bb210, label %bb624

bb210:                                            ; preds = %bb208
  %i211 = tail call fastcc i32 @ReadPixelCachePixels(ptr noundef nonnull %i13, ptr noundef nonnull %arg6, ptr noundef %arg7)
  %i212 = icmp eq i32 %i211, 0
  br i1 %i212, label %bb624, label %bb213

bb213:                                            ; preds = %bb210
  %i214 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 0
  %i215 = load i32, ptr %i214, align 8
  %i216 = icmp eq i32 %i215, 2
  br i1 %i216, label %bb221, label %bb217

bb217:                                            ; preds = %bb213
  %i218 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 1
  %i219 = load i32, ptr %i218, align 4
  %i220 = icmp eq i32 %i219, 12
  br i1 %i220, label %bb221, label %bb224

bb221:                                            ; preds = %bb217, %bb213
  %i222 = tail call fastcc i32 @ReadPixelCacheIndexes(ptr noundef nonnull %i13, ptr noundef nonnull %arg6, ptr noundef %arg7)
  %i223 = icmp eq i32 %i222, 0
  br i1 %i223, label %bb624, label %bb224

bb224:                                            ; preds = %bb221, %bb217
  br label %bb624

bb225:                                            ; preds = %bb200, %bb186, %bb176
  %i226 = tail call ptr @AcquirePixelCacheNexus(i64 noundef 1)
  switch i32 %arg1, label %bb247 [
    i32 10, label %bb227
    i32 11, label %bb232
    i32 8, label %bb237
    i32 9, label %bb242
    i32 12, label %bb242
  ]

bb227:                                            ; preds = %bb225
  %i228 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  store i16 0, ptr %i228, align 2
  %i229 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  store i16 0, ptr %i229, align 2
  %i230 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  store i16 0, ptr %i230, align 2
  %i231 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  store i16 0, ptr %i231, align 2
  br label %bb261

bb232:                                            ; preds = %bb225
  %i233 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  store i16 32767, ptr %i233, align 2
  %i234 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  store i16 32767, ptr %i234, align 2
  %i235 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  store i16 32767, ptr %i235, align 2
  %i236 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  store i16 0, ptr %i236, align 2
  br label %bb261

bb237:                                            ; preds = %bb225
  %i238 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  store i16 0, ptr %i238, align 2
  %i239 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  store i16 0, ptr %i239, align 2
  %i240 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  store i16 0, ptr %i240, align 2
  %i241 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  store i16 -1, ptr %i241, align 2
  br label %bb261

bb242:                                            ; preds = %bb225, %bb225
  %i243 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  store i16 -1, ptr %i243, align 2
  %i244 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  store i16 -1, ptr %i244, align 2
  %i245 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  store i16 -1, ptr %i245, align 2
  %i246 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  store i16 0, ptr %i246, align 2
  br label %bb261

bb247:                                            ; preds = %bb225
  %i248 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 12
  %i249 = getelementptr inbounds %struct._PixelPacket, ptr %i248, i64 0, i32 0
  %i250 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  %i251 = load i16, ptr %i249, align 2
  store i16 %i251, ptr %i250, align 2
  %i252 = getelementptr inbounds %struct._PixelPacket, ptr %i248, i64 0, i32 1
  %i253 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  %i254 = load i16, ptr %i252, align 2
  store i16 %i254, ptr %i253, align 2
  %i255 = getelementptr inbounds %struct._PixelPacket, ptr %i248, i64 0, i32 2
  %i256 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  %i257 = load i16, ptr %i255, align 2
  store i16 %i257, ptr %i256, align 2
  %i258 = getelementptr inbounds %struct._PixelPacket, ptr %i248, i64 0, i32 3
  %i259 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  %i260 = load i16, ptr %i258, align 2
  store i16 %i260, ptr %i259, align 2
  br label %bb261

bb261:                                            ; preds = %bb247, %bb242, %bb237, %bb232, %bb227
  store i16 0, ptr %i, align 2
  %i262 = icmp sgt i64 %arg5, 0
  br i1 %i262, label %bb263, label %bb599

bb263:                                            ; preds = %bb261
  %i264 = and i32 %arg1, -5
  %i265 = icmp eq i32 %i264, 0
  %i266 = icmp sgt i64 %arg4, 0
  %i267 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 0
  %i268 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 21
  br i1 %i266, label %bb269, label %bb599

bb269:                                            ; preds = %bb594, %bb263
  %i270 = phi ptr [ %i596, %bb594 ], [ %i178, %bb263 ]
  %i271 = phi ptr [ %i595, %bb594 ], [ %i182, %bb263 ]
  %i272 = phi i64 [ %i597, %bb594 ], [ 0, %bb263 ]
  %i273 = add nsw i64 %i272, %arg3
  br i1 %i265, label %bb274, label %bb281

bb274:                                            ; preds = %bb269
  %i275 = load i64, ptr %i184, align 8
  %i276 = icmp slt i64 %i273, 0
  %i277 = icmp slt i64 %i273, %i275
  %i278 = add i64 %i275, -1
  %i279 = select i1 %i277, i64 %i273, i64 %i278
  %i280 = select i1 %i276, i64 0, i64 %i279
  br label %bb281

bb281:                                            ; preds = %bb274, %bb269
  %i282 = phi i64 [ %i280, %bb274 ], [ %i273, %bb269 ]
  %i283 = icmp slt i64 %i282, 0
  %i284 = ashr i64 %i282, 63
  %i285 = lshr i64 %i282, 63
  %i286 = and i64 %i282, 7
  %i287 = getelementptr inbounds [64 x i64], ptr @DitherMatrix, i64 0, i64 %i286
  %i288 = add i64 %i282, -32
  br label %bb289

bb289:                                            ; preds = %bb588, %bb281
  %i290 = phi ptr [ %i270, %bb281 ], [ %i590, %bb588 ]
  %i291 = phi ptr [ %i271, %bb281 ], [ %i589, %bb588 ]
  %i292 = phi i64 [ 0, %bb281 ], [ %i592, %bb588 ]
  %i293 = add nsw i64 %i292, %arg2
  %i294 = load i64, ptr %i183, align 8
  %i295 = sub i64 %i294, %i293
  %i296 = sub i64 %arg4, %i292
  %i297 = icmp ult i64 %i295, %i296
  %i298 = select i1 %i297, i64 %i295, i64 %i296
  %i299 = icmp slt i64 %i293, 0
  %i300 = icmp sle i64 %i294, %i293
  %i301 = select i1 %i299, i1 true, i1 %i300
  %i302 = select i1 %i301, i1 true, i1 %i283
  br i1 %i302, label %bb336, label %bb303

bb303:                                            ; preds = %bb289
  %i304 = load i64, ptr %i184, align 8
  %i305 = icmp sge i64 %i282, %i304
  %i306 = icmp eq i64 %i298, 0
  %i307 = select i1 %i305, i1 true, i1 %i306
  br i1 %i307, label %bb336, label %bb308

bb308:                                            ; preds = %bb303
  %i309 = load ptr, ptr %i226, align 8
  %i310 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef %arg1, i64 noundef %i293, i64 noundef %i282, i64 noundef %i298, i64 noundef 1, ptr noundef %i309, ptr noundef %arg7)
  %i311 = icmp eq ptr %i310, null
  br i1 %i311, label %bb594, label %bb312

bb312:                                            ; preds = %bb308
  %i313 = load i32, ptr %i267, align 8
  %i314 = icmp eq i32 %i313, 0
  br i1 %i314, label %bb331, label %bb315

bb315:                                            ; preds = %bb312
  %i316 = load ptr, ptr %i226, align 8
  %i317 = getelementptr inbounds %struct._NexusInfo, ptr %i316, i64 0, i32 6
  %i318 = load ptr, ptr %i317, align 8
  %i321 = shl i64 %i298, 3
  call void @llvm.memcpy.p0.p0.i64(ptr align 2 %i291, ptr nonnull align 2 %i310, i64 %i321, i1 false)
  %i322 = getelementptr inbounds %struct._PixelPacket, ptr %i291, i64 %i298
  %i323 = icmp ne ptr %i290, null
  %i324 = icmp ne ptr %i318, null
  %i325 = select i1 %i323, i1 %i324, i1 false
  br i1 %i325, label %bb326, label %bb588

bb326:                                            ; preds = %bb315
  %i329 = shl i64 %i298, 1
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i290, ptr nonnull align 2 %i318, i64 %i329, i1 false)
  %i330 = getelementptr inbounds i16, ptr %i290, i64 %i298
  br label %bb588

bb331:                                            ; preds = %bb312
  %i334 = shl i64 %i298, 3
  call void @llvm.memcpy.p0.p0.i64(ptr align 2 %i291, ptr nonnull align 2 %i310, i64 %i334, i1 false)
  %i335 = getelementptr inbounds %struct._PixelPacket, ptr %i291, i64 %i298
  br label %bb588

bb336:                                            ; preds = %bb303, %bb289
  switch i32 %arg1, label %bb544 [
    i32 1, label %bb566
    i32 2, label %bb566
    i32 10, label %bb566
    i32 11, label %bb566
    i32 8, label %bb566
    i32 9, label %bb566
    i32 12, label %bb566
    i32 16, label %bb526
    i32 6, label %bb499
    i32 3, label %bb472
    i32 7, label %bb453
    i32 5, label %bb424
    i32 17, label %bb401
    i32 13, label %bb379
    i32 14, label %bb356
    i32 15, label %bb337
  ]

bb337:                                            ; preds = %bb336
  %i338 = sdiv i64 %i293, %i294
  %i339 = ashr i64 %i293, 63
  %i340 = add nsw i64 %i338, %i339
  %i341 = mul nsw i64 %i340, %i294
  %i342 = sub nsw i64 %i293, %i341
  %i343 = load i64, ptr %i184, align 8
  %i344 = icmp slt i64 %i282, %i343
  %i345 = add i64 %i343, -1
  %i346 = select i1 %i344, i64 %i282, i64 %i345
  %i347 = select i1 %i283, i64 0, i64 %i346
  %i348 = load ptr, ptr %i226, align 8
  %i349 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 15, i64 noundef %i342, i64 noundef %i347, i64 noundef 1, i64 noundef 1, ptr noundef %i348, ptr noundef %arg7)
  %i350 = load i32, ptr %i267, align 8
  %i351 = icmp eq i32 %i350, 0
  br i1 %i351, label %bb562, label %bb352

bb352:                                            ; preds = %bb337
  %i353 = load ptr, ptr %i226, align 8
  %i354 = getelementptr inbounds %struct._NexusInfo, ptr %i353, i64 0, i32 6
  %i355 = load ptr, ptr %i354, align 8
  br label %bb562

bb356:                                            ; preds = %bb336
  %i357 = xor i1 %i299, true
  %i358 = icmp sgt i64 %i294, %i293
  %i359 = select i1 %i357, i1 %i358, i1 false
  br i1 %i359, label %bb360, label %bb566

bb360:                                            ; preds = %bb356
  %i361 = sdiv i64 %i293, %i294
  %i362 = lshr i64 %i293, 63
  %i363 = add nuw nsw i64 %i361, %i362
  %i364 = mul nsw i64 %i363, %i294
  %i365 = sub nsw i64 %i293, %i364
  %i366 = load i64, ptr %i184, align 8
  %i367 = sdiv i64 %i282, %i366
  %i368 = add nsw i64 %i367, %i284
  %i369 = mul nsw i64 %i368, %i366
  %i370 = sub nsw i64 %i282, %i369
  %i371 = load ptr, ptr %i226, align 8
  %i372 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 14, i64 noundef %i365, i64 noundef %i370, i64 noundef 1, i64 noundef 1, ptr noundef %i371, ptr noundef %arg7)
  %i373 = load i32, ptr %i267, align 8
  %i374 = icmp eq i32 %i373, 0
  br i1 %i374, label %bb562, label %bb375

bb375:                                            ; preds = %bb360
  %i376 = load ptr, ptr %i226, align 8
  %i377 = getelementptr inbounds %struct._NexusInfo, ptr %i376, i64 0, i32 6
  %i378 = load ptr, ptr %i377, align 8
  br label %bb562

bb379:                                            ; preds = %bb336
  br i1 %i283, label %bb566, label %bb380

bb380:                                            ; preds = %bb379
  %i381 = load i64, ptr %i184, align 8
  %i382 = icmp slt i64 %i282, %i381
  br i1 %i382, label %bb383, label %bb566

bb383:                                            ; preds = %bb380
  %i384 = sdiv i64 %i293, %i294
  %i385 = ashr i64 %i293, 63
  %i386 = add nsw i64 %i384, %i385
  %i387 = mul nsw i64 %i386, %i294
  %i388 = sub nsw i64 %i293, %i387
  %i389 = sdiv i64 %i282, %i381
  %i390 = add nuw nsw i64 %i389, %i285
  %i391 = mul nsw i64 %i390, %i381
  %i392 = sub nsw i64 %i282, %i391
  %i393 = load ptr, ptr %i226, align 8
  %i394 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 13, i64 noundef %i388, i64 noundef %i392, i64 noundef 1, i64 noundef 1, ptr noundef %i393, ptr noundef %arg7)
  %i395 = load i32, ptr %i267, align 8
  %i396 = icmp eq i32 %i395, 0
  br i1 %i396, label %bb562, label %bb397

bb397:                                            ; preds = %bb383
  %i398 = load ptr, ptr %i226, align 8
  %i399 = getelementptr inbounds %struct._NexusInfo, ptr %i398, i64 0, i32 6
  %i400 = load ptr, ptr %i399, align 8
  br label %bb562

bb401:                                            ; preds = %bb336
  %i402 = sdiv i64 %i293, %i294
  %i403 = ashr i64 %i293, 63
  %i404 = add nsw i64 %i402, %i403
  %i405 = load i64, ptr %i184, align 8
  %i406 = sdiv i64 %i282, %i405
  %i407 = add nsw i64 %i406, %i284
  %i408 = xor i64 %i407, %i404
  %i409 = and i64 %i408, 1
  %i410 = icmp eq i64 %i409, 0
  br i1 %i410, label %bb411, label %bb566

bb411:                                            ; preds = %bb401
  %i412 = mul nsw i64 %i407, %i405
  %i413 = sub nsw i64 %i282, %i412
  %i414 = mul nsw i64 %i404, %i294
  %i415 = sub nsw i64 %i293, %i414
  %i416 = load ptr, ptr %i226, align 8
  %i417 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 17, i64 noundef %i415, i64 noundef %i413, i64 noundef 1, i64 noundef 1, ptr noundef %i416, ptr noundef %arg7)
  %i418 = load i32, ptr %i267, align 8
  %i419 = icmp eq i32 %i418, 0
  br i1 %i419, label %bb562, label %bb420

bb420:                                            ; preds = %bb411
  %i421 = load ptr, ptr %i226, align 8
  %i422 = getelementptr inbounds %struct._NexusInfo, ptr %i421, i64 0, i32 6
  %i423 = load ptr, ptr %i422, align 8
  br label %bb562

bb424:                                            ; preds = %bb336
  %i425 = sdiv i64 %i293, %i294
  %i426 = ashr i64 %i293, 63
  %i427 = add nsw i64 %i425, %i426
  %i428 = mul nsw i64 %i427, %i294
  %i429 = sub nsw i64 %i293, %i428
  %i430 = and i64 %i427, 1
  %i431 = icmp eq i64 %i430, 0
  %i432 = xor i64 %i429, -1
  %i433 = add i64 %i294, %i432
  %i434 = select i1 %i431, i64 %i429, i64 %i433
  %i435 = load i64, ptr %i184, align 8
  %i436 = sdiv i64 %i282, %i435
  %i437 = add nsw i64 %i436, %i284
  %i438 = mul nsw i64 %i437, %i435
  %i439 = sub nsw i64 %i282, %i438
  %i440 = and i64 %i437, 1
  %i441 = icmp eq i64 %i440, 0
  %i442 = xor i64 %i439, -1
  %i443 = add i64 %i435, %i442
  %i444 = select i1 %i441, i64 %i439, i64 %i443
  %i445 = load ptr, ptr %i226, align 8
  %i446 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 5, i64 noundef %i434, i64 noundef %i444, i64 noundef 1, i64 noundef 1, ptr noundef %i445, ptr noundef %arg7)
  %i447 = load i32, ptr %i267, align 8
  %i448 = icmp eq i32 %i447, 0
  br i1 %i448, label %bb562, label %bb449

bb449:                                            ; preds = %bb424
  %i450 = load ptr, ptr %i226, align 8
  %i451 = getelementptr inbounds %struct._NexusInfo, ptr %i450, i64 0, i32 6
  %i452 = load ptr, ptr %i451, align 8
  br label %bb562

bb453:                                            ; preds = %bb336
  %i454 = sdiv i64 %i293, %i294
  %i455 = ashr i64 %i293, 63
  %i456 = add nsw i64 %i454, %i455
  %i457 = mul nsw i64 %i456, %i294
  %i458 = sub nsw i64 %i293, %i457
  %i459 = load i64, ptr %i184, align 8
  %i460 = sdiv i64 %i282, %i459
  %i461 = add nsw i64 %i460, %i284
  %i462 = mul nsw i64 %i461, %i459
  %i463 = sub nsw i64 %i282, %i462
  %i464 = load ptr, ptr %i226, align 8
  %i465 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 7, i64 noundef %i458, i64 noundef %i463, i64 noundef 1, i64 noundef 1, ptr noundef %i464, ptr noundef %arg7)
  %i466 = load i32, ptr %i267, align 8
  %i467 = icmp eq i32 %i466, 0
  br i1 %i467, label %bb562, label %bb468

bb468:                                            ; preds = %bb453
  %i469 = load ptr, ptr %i226, align 8
  %i470 = getelementptr inbounds %struct._NexusInfo, ptr %i469, i64 0, i32 6
  %i471 = load ptr, ptr %i470, align 8
  br label %bb562

bb472:                                            ; preds = %bb336
  %i473 = and i64 %i293, 7
  %i474 = getelementptr inbounds [64 x i64], ptr @DitherMatrix, i64 0, i64 %i473
  %i475 = load i64, ptr %i474, align 8
  %i476 = add i64 %i293, -32
  %i477 = add i64 %i476, %i475
  %i478 = icmp slt i64 %i477, 0
  %i479 = icmp slt i64 %i477, %i294
  %i480 = add nsw i64 %i294, -1
  %i481 = select i1 %i479, i64 %i477, i64 %i480
  %i482 = select i1 %i478, i64 0, i64 %i481
  %i483 = load i64, ptr %i184, align 8
  %i484 = load i64, ptr %i287, align 8
  %i485 = add i64 %i288, %i484
  %i486 = icmp slt i64 %i485, 0
  %i487 = icmp slt i64 %i485, %i483
  %i488 = add nsw i64 %i483, -1
  %i489 = select i1 %i487, i64 %i485, i64 %i488
  %i490 = select i1 %i486, i64 0, i64 %i489
  %i491 = load ptr, ptr %i226, align 8
  %i492 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 3, i64 noundef %i482, i64 noundef %i490, i64 noundef 1, i64 noundef 1, ptr noundef %i491, ptr noundef %arg7)
  %i493 = load i32, ptr %i267, align 8
  %i494 = icmp eq i32 %i493, 0
  br i1 %i494, label %bb562, label %bb495

bb495:                                            ; preds = %bb472
  %i496 = load ptr, ptr %i226, align 8
  %i497 = getelementptr inbounds %struct._NexusInfo, ptr %i496, i64 0, i32 6
  %i498 = load ptr, ptr %i497, align 8
  br label %bb562

bb499:                                            ; preds = %bb336
  %i500 = load ptr, ptr %i268, align 8
  %i501 = icmp eq ptr %i500, null
  br i1 %i501, label %bb502, label %bb505

bb502:                                            ; preds = %bb499
  %i503 = call ptr @AcquireRandomInfo()
  store ptr %i503, ptr %i268, align 8
  %i504 = load i64, ptr %i183, align 8
  br label %bb505

bb505:                                            ; preds = %bb502, %bb499
  %i506 = phi i64 [ %i504, %bb502 ], [ %i294, %bb499 ]
  %i507 = phi ptr [ %i503, %bb502 ], [ %i500, %bb499 ]
  %i508 = uitofp i64 %i506 to double
  %i509 = call fast double @GetPseudoRandomValue(ptr noundef %i507)
  %i510 = fmul fast double %i509, %i508
  %i511 = fptosi double %i510 to i64
  %i512 = load ptr, ptr %i268, align 8
  %i513 = load i64, ptr %i184, align 8
  %i514 = uitofp i64 %i513 to double
  %i515 = call fast double @GetPseudoRandomValue(ptr noundef %i512)
  %i516 = fmul fast double %i515, %i514
  %i517 = fptosi double %i516 to i64
  %i518 = load ptr, ptr %i226, align 8
  %i519 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 6, i64 noundef %i511, i64 noundef %i517, i64 noundef 1, i64 noundef 1, ptr noundef %i518, ptr noundef %arg7)
  %i520 = load i32, ptr %i267, align 8
  %i521 = icmp eq i32 %i520, 0
  br i1 %i521, label %bb562, label %bb522

bb522:                                            ; preds = %bb505
  %i523 = load ptr, ptr %i226, align 8
  %i524 = getelementptr inbounds %struct._NexusInfo, ptr %i523, i64 0, i32 6
  %i525 = load ptr, ptr %i524, align 8
  br label %bb562

bb526:                                            ; preds = %bb336
  %i527 = load i64, ptr %i184, align 8
  %i528 = sdiv i64 %i282, %i527
  %i529 = add nsw i64 %i528, %i284
  %i530 = mul nsw i64 %i529, %i527
  %i531 = sub nsw i64 %i282, %i530
  %i532 = icmp sgt i64 %i294, %i293
  %i533 = add i64 %i294, -1
  %i534 = select i1 %i532, i64 %i293, i64 %i533
  %i535 = select i1 %i299, i64 0, i64 %i534
  %i536 = load ptr, ptr %i226, align 8
  %i537 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef 16, i64 noundef %i535, i64 noundef %i531, i64 noundef 1, i64 noundef 1, ptr noundef %i536, ptr noundef %arg7)
  %i538 = load i32, ptr %i267, align 8
  %i539 = icmp eq i32 %i538, 0
  br i1 %i539, label %bb562, label %bb540

bb540:                                            ; preds = %bb526
  %i541 = load ptr, ptr %i226, align 8
  %i542 = getelementptr inbounds %struct._NexusInfo, ptr %i541, i64 0, i32 6
  %i543 = load ptr, ptr %i542, align 8
  br label %bb562

bb544:                                            ; preds = %bb336
  %i545 = icmp sgt i64 %i294, %i293
  %i546 = add i64 %i294, -1
  %i547 = select i1 %i545, i64 %i293, i64 %i546
  %i548 = select i1 %i299, i64 0, i64 %i547
  %i549 = load i64, ptr %i184, align 8
  %i550 = icmp slt i64 %i282, %i549
  %i551 = add i64 %i549, -1
  %i552 = select i1 %i550, i64 %i282, i64 %i551
  %i553 = select i1 %i283, i64 0, i64 %i552
  %i554 = load ptr, ptr %i226, align 8
  %i555 = call ptr @GetVirtualPixelsFromNexus(ptr noundef %arg, i32 noundef %arg1, i64 noundef %i548, i64 noundef %i553, i64 noundef 1, i64 noundef 1, ptr noundef %i554, ptr noundef %arg7)
  %i556 = load i32, ptr %i267, align 8
  %i557 = icmp eq i32 %i556, 0
  br i1 %i557, label %bb562, label %bb558

bb558:                                            ; preds = %bb544
  %i559 = load ptr, ptr %i226, align 8
  %i560 = getelementptr inbounds %struct._NexusInfo, ptr %i559, i64 0, i32 6
  %i561 = load ptr, ptr %i560, align 8
  br label %bb562

bb562:                                            ; preds = %bb558, %bb544, %bb540, %bb526, %bb522, %bb505, %bb495, %bb472, %bb468, %bb453, %bb449, %bb424, %bb420, %bb411, %bb397, %bb383, %bb375, %bb360, %bb352, %bb337
  %i563 = phi ptr [ %i555, %bb544 ], [ %i555, %bb558 ], [ %i519, %bb505 ], [ %i519, %bb522 ], [ %i492, %bb472 ], [ %i492, %bb495 ], [ %i465, %bb453 ], [ %i465, %bb468 ], [ %i446, %bb424 ], [ %i446, %bb449 ], [ %i417, %bb411 ], [ %i417, %bb420 ], [ %i394, %bb383 ], [ %i394, %bb397 ], [ %i372, %bb360 ], [ %i372, %bb375 ], [ %i349, %bb337 ], [ %i349, %bb352 ], [ %i537, %bb526 ], [ %i537, %bb540 ]
  %i564 = phi ptr [ null, %bb544 ], [ %i561, %bb558 ], [ null, %bb505 ], [ %i525, %bb522 ], [ null, %bb472 ], [ %i498, %bb495 ], [ null, %bb453 ], [ %i471, %bb468 ], [ null, %bb424 ], [ %i452, %bb449 ], [ null, %bb411 ], [ %i423, %bb420 ], [ null, %bb383 ], [ %i400, %bb397 ], [ null, %bb360 ], [ %i378, %bb375 ], [ null, %bb337 ], [ %i355, %bb352 ], [ null, %bb526 ], [ %i543, %bb540 ]
  %i565 = icmp eq ptr %i563, null
  br i1 %i565, label %bb594, label %bb566

bb566:                                            ; preds = %bb562, %bb401, %bb380, %bb379, %bb356, %bb336, %bb336, %bb336, %bb336, %bb336, %bb336, %bb336
  %i567 = phi ptr [ %i564, %bb562 ], [ %i, %bb336 ], [ %i, %bb336 ], [ %i, %bb336 ], [ %i, %bb336 ], [ %i, %bb336 ], [ %i, %bb336 ], [ %i, %bb336 ], [ %i, %bb401 ], [ %i, %bb380 ], [ %i, %bb379 ], [ %i, %bb356 ]
  %i568 = phi ptr [ %i563, %bb562 ], [ %i8, %bb336 ], [ %i8, %bb336 ], [ %i8, %bb336 ], [ %i8, %bb336 ], [ %i8, %bb336 ], [ %i8, %bb336 ], [ %i8, %bb336 ], [ %i8, %bb401 ], [ %i8, %bb380 ], [ %i8, %bb379 ], [ %i8, %bb356 ]
  %i569 = getelementptr inbounds %struct._PixelPacket, ptr %i291, i64 1
  %i570 = getelementptr inbounds %struct._PixelPacket, ptr %i568, i64 0, i32 0
  %i571 = getelementptr inbounds %struct._PixelPacket, ptr %i291, i64 0, i32 0
  %i572 = load i16, ptr %i570, align 2
  store i16 %i572, ptr %i571, align 2
  %i573 = getelementptr inbounds %struct._PixelPacket, ptr %i568, i64 0, i32 1
  %i574 = getelementptr inbounds %struct._PixelPacket, ptr %i291, i64 0, i32 1
  %i575 = load i16, ptr %i573, align 2
  store i16 %i575, ptr %i574, align 2
  %i576 = getelementptr inbounds %struct._PixelPacket, ptr %i568, i64 0, i32 2
  %i577 = getelementptr inbounds %struct._PixelPacket, ptr %i291, i64 0, i32 2
  %i578 = load i16, ptr %i576, align 2
  store i16 %i578, ptr %i577, align 2
  %i579 = getelementptr inbounds %struct._PixelPacket, ptr %i568, i64 0, i32 3
  %i580 = getelementptr inbounds %struct._PixelPacket, ptr %i291, i64 0, i32 3
  %i581 = load i16, ptr %i579, align 2
  store i16 %i581, ptr %i580, align 2
  %i582 = icmp ne ptr %i290, null
  %i583 = icmp ne ptr %i567, null
  %i584 = select i1 %i582, i1 %i583, i1 false
  br i1 %i584, label %bb585, label %bb588

bb585:                                            ; preds = %bb566
  %i586 = load i16, ptr %i567, align 2
  %i587 = getelementptr inbounds i16, ptr %i290, i64 1
  store i16 %i586, ptr %i290, align 2
  br label %bb588

bb588:                                            ; preds = %bb585, %bb566, %bb331, %bb326, %bb315
  %i589 = phi ptr [ %i322, %bb315 ], [ %i322, %bb326 ], [ %i569, %bb566 ], [ %i569, %bb585 ], [ %i335, %bb331 ]
  %i590 = phi ptr [ %i290, %bb315 ], [ %i330, %bb326 ], [ %i290, %bb566 ], [ %i587, %bb585 ], [ %i290, %bb331 ]
  %i591 = phi i64 [ %i298, %bb315 ], [ %i298, %bb326 ], [ 1, %bb566 ], [ 1, %bb585 ], [ %i298, %bb331 ]
  %i592 = add i64 %i591, %i292
  %i593 = icmp slt i64 %i592, %arg4
  br i1 %i593, label %bb289, label %bb594

bb594:                                            ; preds = %bb588, %bb562, %bb308
  %i595 = phi ptr [ %i291, %bb562 ], [ %i291, %bb308 ], [ %i589, %bb588 ]
  %i596 = phi ptr [ %i290, %bb562 ], [ %i290, %bb308 ], [ %i590, %bb588 ]
  %i597 = add nuw nsw i64 %i272, 1
  %i598 = icmp eq i64 %i597, %arg5
  br i1 %i598, label %bb599, label %bb269

bb599:                                            ; preds = %bb594, %bb263, %bb261
  %i600 = load ptr, ptr %i226, align 8
  %i601 = getelementptr inbounds %struct._NexusInfo, ptr %i600, i64 0, i32 3
  %i602 = load ptr, ptr %i601, align 8
  %i603 = icmp eq ptr %i602, null
  br i1 %i603, label %bb619, label %bb606

bb606:                                            ; preds = %bb599
  %i607 = getelementptr inbounds %struct._NexusInfo, ptr %i600, i64 0, i32 0
  %i608 = load i32, ptr %i607, align 8
  %i609 = icmp eq i32 %i608, 0
  br i1 %i609, label %bb610, label %bb612

bb610:                                            ; preds = %bb606
  %i611 = call ptr @RelinquishAlignedMemory(ptr noundef nonnull %i602)
  br label %bb615

bb612:                                            ; preds = %bb606
  %i613 = getelementptr inbounds %struct._NexusInfo, ptr %i600, i64 0, i32 2
  %i614 = load i64, ptr %i613, align 8
  br label %bb615

bb615:                                            ; preds = %bb612, %bb610
  store ptr null, ptr %i601, align 8
  %i616 = getelementptr inbounds %struct._NexusInfo, ptr %i600, i64 0, i32 4
  store ptr null, ptr %i616, align 8
  %i617 = getelementptr inbounds %struct._NexusInfo, ptr %i600, i64 0, i32 6
  store ptr null, ptr %i617, align 8
  %i618 = getelementptr inbounds %struct._NexusInfo, ptr %i600, i64 0, i32 2
  store i64 0, ptr %i618, align 8
  store i32 0, ptr %i607, align 8
  br label %bb619

bb619:                                            ; preds = %bb615, %bb599
  %i620 = getelementptr inbounds %struct._NexusInfo, ptr %i600, i64 0, i32 7
  store i64 -2880220588, ptr %i620, align 8
  %i621 = call ptr @RelinquishMagickMemory(ptr noundef nonnull %i600)
  store ptr null, ptr %i226, align 8
  %i623 = call ptr @RelinquishAlignedMemory(ptr noundef nonnull %i226)
  br label %bb624

bb624:                                            ; preds = %bb619, %bb224, %bb221, %bb210, %bb208, %bb172, %bb125, %bb99, %bb
  %i625 = phi ptr [ %i182, %bb619 ], [ null, %bb ], [ null, %bb172 ], [ %i182, %bb224 ], [ %i182, %bb208 ], [ null, %bb210 ], [ null, %bb221 ], [ null, %bb99 ], [ null, %bb125 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i8)
  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %i)
  ret ptr %i625
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
; end INTEL_FEATURE_SW_ADVANCED
