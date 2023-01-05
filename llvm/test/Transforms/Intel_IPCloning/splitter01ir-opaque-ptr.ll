; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -passes='module(ip-cloning)' -force-ip-manyreccalls-splitting -S 2>&1 | FileCheck %s

; Test for splitting part of inlining and splitting for many recursive calls
; functions.

; This is the same test as splitter01.ll, but does not require asserts.

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
; CHECK: define internal ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 %arg1, i64 %arg2, i64 %arg3, i64 %arg4, i64 %arg5, ptr %arg6, ptr %arg7)
; CHECK: define internal ptr @GetVirtualPixelsFromNexus.1(ptr %0, i32 %1, i64 %2, i64 %3, i64 %4, i64 %5, ptr %6, ptr %7, ptr %8) #[[A1:[0-9]+]]
; CHECK: define internal ptr @GetVirtualPixelsFromNexus.2(ptr %0, i32 %1, i64 %2, i64 %3, i64 %4, i64 %5, ptr %6, ptr %7, ptr %8) #[[A1]]
; CHECK: store i32 0, ptr [[R2:%[0-9]+]], align 4
; CHECK: [[R3:%[0-9]+]] = call{{.*}}@GetVirtualPixelsFromNexus.1({{.*}} [[R2]])
; CHECK: call{{.*}}@GetVirtualPixelsFromNexus.2({{.*}} [[R3]])
; CHECK: store i32 0, ptr [[R4:%[0-9]+]], align 4
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

%struct._IO_marker = type { ptr, ptr, i32 }
%struct.timespec = type { i64, i64 }
%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct.stat = type { i64, i64, i64, i32, i32, i32, i32, i64, i64, i64, i64, %struct.timespec, %struct.timespec, %struct.timespec, [3 x i64] }
%union.FileInfo = type { ptr }
%struct._Timer = type { double, double, double }
%struct._PrimaryInfo = type { double, double, double }
%struct._ProfileInfo = type { ptr, i64, ptr, i64 }
%struct.SemaphoreInfo = type { i64, i32, i64, i64 }
%struct._ExceptionInfo = type { i32, i32, ptr, ptr, ptr, i32, ptr, i64 }
%struct._BlobInfo = type { i64, i64, i64, i32, i32, i64, i64, i32, i32, i32, i32, i32, %union.FileInfo, %struct.stat, ptr, ptr, i32, ptr, i64, i64 }
%struct._Ascii85Info = type { i64, i64, [10 x i8] }
%struct._TimerInfo = type { %struct._Timer, %struct._Timer, i32, i64 }
%struct._ErrorInfo = type { double, double, double }
%struct._RectangleInfo = type { i64, i64, i64, i64 }
%struct._ChromaticityInfo = type { %struct._PrimaryInfo, %struct._PrimaryInfo, %struct._PrimaryInfo, %struct._PrimaryInfo }
%struct._PixelPacket = type { i16, i16, i16, i16 }
%struct._Image = type { i32, i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, ptr, %struct._PixelPacket, %struct._PixelPacket, %struct._PixelPacket, double, %struct._ChromaticityInfo, i32, ptr, i32, ptr, ptr, ptr, i64, double, double, %struct._RectangleInfo, %struct._RectangleInfo, %struct._RectangleInfo, double, double, double, i32, i32, i32, i32, i32, i32, ptr, i64, i64, i64, i64, i64, i64, %struct._ErrorInfo, %struct._TimerInfo, ptr, ptr, ptr, ptr, ptr, ptr, [4096 x i8], [4096 x i8], [4096 x i8], i64, i64, %struct._ExceptionInfo, i32, i64, ptr, %struct._ProfileInfo, %struct._ProfileInfo, ptr, i64, i64, ptr, ptr, ptr, i32, i32, %struct._PixelPacket, ptr, %struct._RectangleInfo, ptr, ptr, i32, i32, i64, i32, i64, i64, i32, i64 }
%struct._NexusInfo = type { i32, %struct._RectangleInfo, i64, ptr, ptr, i32, ptr, i64 }
%struct._StringInfo = type { [4096 x i8], ptr, i64, i64 }
%struct._SignatureInfo.948 = type { i32, i32, ptr, ptr, ptr, i32, i32, i64, i32, i64, i64 }
%struct._RandomInfo = type { ptr, ptr, ptr, i64, [4 x i64], double, i64, i16, i16, ptr, i64, i64 }
%struct._CacheMethods = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct._MagickPixelPacket = type { i32, i32, i32, double, i64, float, float, float, float, float }
%struct._CacheInfo = type { i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, i32, %struct._MagickPixelPacket, i64, ptr, ptr, ptr, i32, i32, [4096 x i8], [4096 x i8], %struct._CacheMethods, ptr, i64, ptr, i32, i32, i64, ptr, ptr, i64, i64 }
%struct._CacheView = type { ptr, i32, i64, ptr, i32, i64 }

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

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
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
  %i36 = bitcast ptr %i5 to ptr
  %i37 = getelementptr %struct._Image, ptr %arg, i64 0, i32 1
  %i38 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 5
  %i39 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 6
  %i40 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 7
  %i41 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 8
  %i42 = getelementptr inbounds %struct._MagickPixelPacket, ptr %i5, i64 0, i32 9
  %i43 = bitcast ptr %i6 to ptr
  %i44 = sdiv i64 %arg2, 2
  %i45 = sub nsw i64 0, %i44
  %i46 = icmp slt i64 %i44, %i45
  %i47 = sdiv i64 %arg1, 2
  %i48 = sub nsw i64 0, %i47
  %i49 = icmp slt i64 %i47, %i48
  %i50 = lshr i64 %arg1, 1
  %i51 = lshr i64 %arg2, 1
  %i52 = mul i64 %i51, %i50
  %i53 = bitcast ptr %i7 to ptr
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
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %i36)
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
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %i43)
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
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i53)
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
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i53)
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
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %i43)
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
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %i36)
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
  %i291 = call i64 (ptr, i64, ptr, ...) @FormatLocaleString(ptr nonnull %i64, i64 4096, ptr getelementptr inbounds ([6 x i8], ptr @.str.18.1467, i64 0, i64 0), ptr getelementptr inbounds ([16 x i8], ptr @.str.17.1479, i64 0, i64 0), ptr nonnull %i66)
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

define internal ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 %arg1, i64 %arg2, i64 %arg3, i64 %arg4, i64 %arg5, ptr %arg6, ptr %arg7) {
bb:
  %i = alloca i16, align 2
  %i8 = alloca %struct._PixelPacket, align 2
  %i9 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 2, ptr nonnull %i9)
  %i10 = bitcast ptr %i8 to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i10)
  %i11 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 49
  %i12 = bitcast ptr %i11 to ptr
  %i13 = load ptr, ptr %i12, align 8
  %i14 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 3
  %i15 = load i32, ptr %i14, align 8
  %i16 = icmp eq i32 %i15, 0
  br i1 %i16, label %bb631, label %bb17

bb17:                                             ; preds = %bb
  %i18 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 38
  %i19 = load ptr, ptr %i18, align 8
  %i20 = icmp eq ptr %i19, null
  br i1 %i20, label %bb21, label %bb26

bb21:                                             ; preds = %bb17
  %i22 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 73
  %i23 = load ptr, ptr %i22, align 8
  %i24 = icmp ne ptr %i23, null
  %i25 = zext i1 %i24 to i32
  br label %bb26

bb26:                                             ; preds = %bb21, %bb17
  %i27 = phi i32 [ 1, %bb17 ], [ %i25, %bb21 ]
  %i28 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1
  %i29 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 0
  store i64 %arg4, ptr %i29, align 8
  %i30 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 1
  store i64 %arg5, ptr %i30, align 8
  %i31 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 2
  store i64 %arg2, ptr %i31, align 8
  %i32 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 1, i32 3
  store i64 %arg3, ptr %i32, align 8
  %i33 = load i32, ptr %i14, align 8
  %i34 = add i32 %i33, -1
  %i35 = icmp ult i32 %i34, 2
  %i36 = icmp eq i32 %i27, 0
  %i37 = and i1 %i36, %i35
  %i38 = getelementptr inbounds %struct._RectangleInfo, ptr %i28, i64 0, i32 1
  %i39 = getelementptr inbounds %struct._RectangleInfo, ptr %i28, i64 0, i32 0
  br i1 %i37, label %bb40, label %bb81

bb40:                                             ; preds = %bb26
  %i41 = add nsw i64 %arg3, %arg5
  %i42 = icmp sgt i64 %arg2, -1
  br i1 %i42, label %bb43, label %bb81

bb43:                                             ; preds = %bb40
  %i44 = add nsw i64 %arg2, %arg4
  %i45 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 6
  %i46 = load i64, ptr %i45, align 8
  %i47 = icmp sle i64 %i44, %i46
  %i48 = icmp sgt i64 %arg3, -1
  %i49 = and i1 %i48, %i47
  br i1 %i49, label %bb50, label %bb81

bb50:                                             ; preds = %bb43
  %i51 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 7
  %i52 = load i64, ptr %i51, align 8
  %i53 = icmp sgt i64 %i41, %i52
  br i1 %i53, label %bb81, label %bb54

bb54:                                             ; preds = %bb50
  %i55 = icmp eq i64 %arg5, 1
  br i1 %i55, label %bb63, label %bb56

bb56:                                             ; preds = %bb54
  %i57 = icmp eq i64 %arg2, 0
  br i1 %i57, label %bb58, label %bb81

bb58:                                             ; preds = %bb56
  %i59 = icmp eq i64 %i46, %arg4
  br i1 %i59, label %bb63, label %bb60

bb60:                                             ; preds = %bb58
  %i61 = urem i64 %arg4, %i46
  %i62 = icmp eq i64 %i61, 0
  br i1 %i62, label %bb63, label %bb81

bb63:                                             ; preds = %bb60, %bb58, %bb54
  %i64 = mul i64 %i46, %arg3
  %i65 = add i64 %i64, %arg2
  %i66 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 14
  %i67 = load ptr, ptr %i66, align 8
  %i68 = getelementptr inbounds %struct._PixelPacket, ptr %i67, i64 %i65
  %i69 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 4
  store ptr %i68, ptr %i69, align 8
  %i70 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  store ptr null, ptr %i70, align 8
  %i71 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 16
  %i72 = load i32, ptr %i71, align 8
  %i73 = icmp eq i32 %i72, 0
  br i1 %i73, label %bb78, label %bb74

bb74:                                             ; preds = %bb63
  %i75 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 15
  %i76 = load ptr, ptr %i75, align 8
  %i77 = getelementptr inbounds i16, ptr %i76, i64 %i65
  store ptr %i77, ptr %i70, align 8
  br label %bb78

bb78:                                             ; preds = %bb74, %bb63
  %i79 = phi ptr [ %i77, %bb74 ], [ null, %bb63 ]
  %i80 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 5
  store i32 1, ptr %i80, align 8
  br label %bb170

bb81:                                             ; preds = %bb60, %bb56, %bb50, %bb43, %bb40, %bb26
  %i82 = mul i64 %arg4, %arg5
  %i83 = shl i64 %i82, 3
  %i84 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 16
  %i85 = load i32, ptr %i84, align 8
  %i86 = icmp eq i32 %i85, 0
  %i87 = mul i64 %i82, 10
  %i88 = select i1 %i86, i64 %i83, i64 %i87
  %i89 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 3
  %i90 = load ptr, ptr %i89, align 8
  %i91 = icmp eq ptr %i90, null
  %i92 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 2
  %i93 = bitcast ptr %i90 to ptr
  br i1 %i91, label %bb94, label %bb110

bb94:                                             ; preds = %bb81
  store i64 %i88, ptr %i92, align 8
  %i95 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 0
  store i32 0, ptr %i95, align 8
  %i96 = tail call ptr @AcquireAlignedMemory(i64 1, i64 %i88)
  %i97 = bitcast ptr %i96 to ptr
  store ptr %i97, ptr %i89, align 8
  %i98 = icmp eq ptr %i96, null
  br i1 %i98, label %bb99, label %bb101

bb99:                                             ; preds = %bb94
  store i32 1, ptr %i95, align 8
  %i100 = load i64, ptr %i92, align 8
  store ptr null, ptr %i89, align 8
  br label %bb101

bb101:                                            ; preds = %bb99, %bb94
  %i102 = phi ptr [ null, %bb99 ], [ %i96, %bb94 ]
  %i103 = phi ptr [ null, %bb99 ], [ %i97, %bb94 ]
  %i104 = ptrtoint ptr %i102 to i64
  %i105 = icmp eq ptr %i103, null
  br i1 %i105, label %bb106, label %bb138

bb106:                                            ; preds = %bb101
  %i107 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 18
  %i108 = getelementptr inbounds [4096 x i8], ptr %i107, i64 0, i64 0
  %i109 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr %arg7, ptr getelementptr inbounds ([15 x i8], ptr @.str.129, i64 0, i64 0), ptr getelementptr inbounds ([1 x i8], ptr @.str.1.130, i64 0, i64 0), i64 4688, i32 400, ptr getelementptr inbounds ([23 x i8], ptr @.str.2.131, i64 0, i64 0), ptr getelementptr inbounds ([5 x i8], ptr @.str.3.132, i64 0, i64 0), ptr nonnull %i108)
  store i64 0, ptr %i92, align 8
  br label %bb631

bb110:                                            ; preds = %bb81
  %i111 = ptrtoint ptr %i90 to i64
  %i112 = load i64, ptr %i92, align 8
  %i113 = icmp ult i64 %i112, %i88
  br i1 %i113, label %bb114, label %bb138

bb114:                                            ; preds = %bb110
  %i115 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 0
  %i116 = load i32, ptr %i115, align 8
  %i117 = icmp eq i32 %i116, 0
  br i1 %i117, label %bb118, label %bb120

bb118:                                            ; preds = %bb114
  %i119 = tail call ptr @RelinquishAlignedMemory(ptr nonnull %i93)
  br label %bb121

bb120:                                            ; preds = %bb114
  br label %bb121

bb121:                                            ; preds = %bb120, %bb118
  store ptr null, ptr %i89, align 8
  %i122 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 4
  store ptr null, ptr %i122, align 8
  %i123 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  store ptr null, ptr %i123, align 8
  store i64 %i88, ptr %i92, align 8
  store i32 0, ptr %i115, align 8
  %i124 = tail call ptr @AcquireAlignedMemory(i64 1, i64 %i88)
  %i125 = bitcast ptr %i124 to ptr
  store ptr %i125, ptr %i89, align 8
  %i126 = icmp eq ptr %i124, null
  br i1 %i126, label %bb127, label %bb129

bb127:                                            ; preds = %bb121
  store i32 1, ptr %i115, align 8
  %i128 = load i64, ptr %i92, align 8
  store ptr null, ptr %i89, align 8
  br label %bb129

bb129:                                            ; preds = %bb127, %bb121
  %i130 = phi ptr [ null, %bb127 ], [ %i124, %bb121 ]
  %i131 = phi ptr [ null, %bb127 ], [ %i125, %bb121 ]
  %i132 = ptrtoint ptr %i130 to i64
  %i133 = icmp eq ptr %i131, null
  br i1 %i133, label %bb134, label %bb138

bb134:                                            ; preds = %bb129
  %i135 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 18
  %i136 = getelementptr inbounds [4096 x i8], ptr %i135, i64 0, i64 0
  %i137 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr %arg7, ptr getelementptr inbounds ([15 x i8], ptr @.str.129, i64 0, i64 0), ptr getelementptr inbounds ([1 x i8], ptr @.str.1.130, i64 0, i64 0), i64 4688, i32 400, ptr getelementptr inbounds ([23 x i8], ptr @.str.2.131, i64 0, i64 0), ptr getelementptr inbounds ([5 x i8], ptr @.str.3.132, i64 0, i64 0), ptr nonnull %i136)
  store i64 0, ptr %i92, align 8
  br label %bb631

bb138:                                            ; preds = %bb129, %bb110, %bb101
  %i139 = phi i64 [ %i132, %bb129 ], [ %i104, %bb101 ], [ %i111, %bb110 ]
  %i140 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 4
  %i141 = bitcast ptr %i140 to ptr
  store i64 %i139, ptr %i141, align 8
  %i142 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 6
  store ptr null, ptr %i142, align 8
  %i143 = load i32, ptr %i84, align 8
  %i144 = icmp eq i32 %i143, 0
  %i145 = inttoptr i64 %i139 to ptr
  br i1 %i144, label %bb149, label %bb146

bb146:                                            ; preds = %bb138
  %i147 = getelementptr inbounds %struct._PixelPacket, ptr %i145, i64 %i82
  %i148 = getelementptr %struct._PixelPacket, ptr %i147, i64 0, i32 0
  store ptr %i148, ptr %i142, align 8
  br label %bb149

bb149:                                            ; preds = %bb146, %bb138
  %i150 = phi ptr [ %i148, %bb146 ], [ null, %bb138 ]
  %i151 = load i32, ptr %i14, align 8
  %i152 = icmp eq i32 %i151, 4
  br i1 %i152, label %bb167, label %bb153

bb153:                                            ; preds = %bb149
  %i154 = getelementptr inbounds %struct._RectangleInfo, ptr %i28, i64 0, i32 3
  %i155 = load i64, ptr %i154, align 8
  %i156 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 6
  %i157 = load i64, ptr %i156, align 8
  %i158 = mul i64 %i157, %i155
  %i159 = getelementptr inbounds %struct._RectangleInfo, ptr %i28, i64 0, i32 2
  %i160 = load i64, ptr %i159, align 8
  %i161 = add i64 %i158, %i160
  %i162 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 14
  %i163 = load ptr, ptr %i162, align 8
  %i164 = getelementptr inbounds %struct._PixelPacket, ptr %i163, i64 %i161
  %i165 = icmp eq ptr %i164, %i145
  %i166 = zext i1 %i165 to i32
  br label %bb167

bb167:                                            ; preds = %bb153, %bb149
  %i168 = phi i32 [ %i166, %bb153 ], [ 1, %bb149 ]
  %i169 = getelementptr inbounds %struct._NexusInfo, ptr %arg6, i64 0, i32 5
  store i32 %i168, ptr %i169, align 8
  br label %bb170

bb170:                                            ; preds = %bb167, %bb78
  %i171 = phi ptr [ %i150, %bb167 ], [ %i79, %bb78 ]
  %i172 = phi i32 [ %i168, %bb167 ], [ 1, %bb78 ]
  %i173 = phi ptr [ %i145, %bb167 ], [ %i68, %bb78 ]
  %i174 = icmp eq ptr %i173, null
  br i1 %i174, label %bb631, label %bb175

bb175:                                            ; preds = %bb170
  %i176 = getelementptr inbounds %struct._RectangleInfo, ptr %i28, i64 0, i32 3
  %i177 = load i64, ptr %i176, align 8
  %i178 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 6
  %i179 = load i64, ptr %i178, align 8
  %i180 = mul i64 %i179, %i177
  %i181 = getelementptr inbounds %struct._RectangleInfo, ptr %i28, i64 0, i32 2
  %i182 = load i64, ptr %i181, align 8
  %i183 = add i64 %i180, %i182
  %i184 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 7
  %i185 = load i64, ptr %i184, align 8
  %i186 = icmp sgt i64 %i183, -1
  br i1 %i186, label %bb187, label %bb224

bb187:                                            ; preds = %bb175
  %i188 = mul i64 %i185, %i179
  %i189 = load i64, ptr %i38, align 8
  %i190 = add i64 %i189, -1
  %i191 = mul i64 %i190, %i179
  %i192 = load i64, ptr %i39, align 8
  %i193 = add nsw i64 %i183, -1
  %i194 = add i64 %i193, %i192
  %i195 = add i64 %i194, %i191
  %i196 = icmp ult i64 %i195, %i188
  %i197 = icmp sgt i64 %arg2, -1
  %i198 = and i1 %i197, %i196
  br i1 %i198, label %bb199, label %bb224

bb199:                                            ; preds = %bb187
  %i200 = add i64 %arg4, %arg2
  %i201 = icmp sgt i64 %i200, %i179
  %i202 = icmp slt i64 %arg3, 0
  %i203 = or i1 %i202, %i201
  %i204 = add i64 %arg5, %arg3
  %i205 = icmp sgt i64 %i204, %i185
  %i206 = or i1 %i203, %i205
  br i1 %i206, label %bb224, label %bb207

bb207:                                            ; preds = %bb199
  %i208 = icmp eq i32 %i172, 0
  br i1 %i208, label %bb209, label %bb631

bb209:                                            ; preds = %bb207
  %i210 = tail call fastcc i32 @ReadPixelCachePixels(ptr nonnull %i13, ptr nonnull %arg6, ptr %arg7)
  %i211 = icmp eq i32 %i210, 0
  br i1 %i211, label %bb631, label %bb212

bb212:                                            ; preds = %bb209
  %i213 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 0
  %i214 = load i32, ptr %i213, align 8
  %i215 = icmp eq i32 %i214, 2
  br i1 %i215, label %bb220, label %bb216

bb216:                                            ; preds = %bb212
  %i217 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 1
  %i218 = load i32, ptr %i217, align 4
  %i219 = icmp eq i32 %i218, 12
  br i1 %i219, label %bb220, label %bb223

bb220:                                            ; preds = %bb216, %bb212
  %i221 = tail call fastcc i32 @ReadPixelCacheIndexes(ptr nonnull %i13, ptr nonnull %arg6, ptr %arg7)
  %i222 = icmp eq i32 %i221, 0
  br i1 %i222, label %bb631, label %bb223

bb223:                                            ; preds = %bb220, %bb216
  br label %bb631

bb224:                                            ; preds = %bb199, %bb187, %bb175
  %i225 = tail call ptr @AcquirePixelCacheNexus(i64 1)
  %i226 = icmp eq ptr %i225, null
  br i1 %i226, label %bb227, label %bb231

bb227:                                            ; preds = %bb224
  %i228 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 53
  %i229 = getelementptr inbounds [4096 x i8], ptr %i228, i64 0, i64 0
  %i230 = tail call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr %arg7, ptr getelementptr inbounds ([15 x i8], ptr @.str.129, i64 0, i64 0), ptr getelementptr inbounds ([1 x i8], ptr @.str.1.130, i64 0, i64 0), i64 2673, i32 445, ptr getelementptr inbounds ([22 x i8], ptr @.str.7.170, i64 0, i64 0), ptr getelementptr inbounds ([5 x i8], ptr @.str.3.132, i64 0, i64 0), ptr nonnull %i229)
  br label %bb631

bb231:                                            ; preds = %bb224
  switch i32 %arg1, label %bb252 [
    i32 10, label %bb232
    i32 11, label %bb237
    i32 8, label %bb242
    i32 9, label %bb247
    i32 12, label %bb247
  ]

bb232:                                            ; preds = %bb231
  %i233 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  store i16 0, ptr %i233, align 2
  %i234 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  store i16 0, ptr %i234, align 2
  %i235 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  store i16 0, ptr %i235, align 2
  %i236 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  store i16 0, ptr %i236, align 2
  br label %bb266

bb237:                                            ; preds = %bb231
  %i238 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  store i16 32767, ptr %i238, align 2
  %i239 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  store i16 32767, ptr %i239, align 2
  %i240 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  store i16 32767, ptr %i240, align 2
  %i241 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  store i16 0, ptr %i241, align 2
  br label %bb266

bb242:                                            ; preds = %bb231
  %i243 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  store i16 0, ptr %i243, align 2
  %i244 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  store i16 0, ptr %i244, align 2
  %i245 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  store i16 0, ptr %i245, align 2
  %i246 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  store i16 -1, ptr %i246, align 2
  br label %bb266

bb247:                                            ; preds = %bb231, %bb231
  %i248 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  store i16 -1, ptr %i248, align 2
  %i249 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  store i16 -1, ptr %i249, align 2
  %i250 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  store i16 -1, ptr %i250, align 2
  %i251 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  store i16 0, ptr %i251, align 2
  br label %bb266

bb252:                                            ; preds = %bb231
  %i253 = getelementptr inbounds %struct._Image, ptr %arg, i64 0, i32 12
  %i254 = getelementptr inbounds %struct._PixelPacket, ptr %i253, i64 0, i32 0
  %i255 = load i16, ptr %i254, align 2
  %i256 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 0
  store i16 %i255, ptr %i256, align 2
  %i257 = getelementptr inbounds %struct._PixelPacket, ptr %i253, i64 0, i32 1
  %i258 = load i16, ptr %i257, align 2
  %i259 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 1
  store i16 %i258, ptr %i259, align 2
  %i260 = getelementptr inbounds %struct._PixelPacket, ptr %i253, i64 0, i32 2
  %i261 = load i16, ptr %i260, align 2
  %i262 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 2
  store i16 %i261, ptr %i262, align 2
  %i263 = getelementptr inbounds %struct._PixelPacket, ptr %i253, i64 0, i32 3
  %i264 = load i16, ptr %i263, align 2
  %i265 = getelementptr inbounds %struct._PixelPacket, ptr %i8, i64 0, i32 3
  store i16 %i264, ptr %i265, align 2
  br label %bb266

bb266:                                            ; preds = %bb252, %bb247, %bb242, %bb237, %bb232
  store i16 0, ptr %i, align 2
  %i267 = icmp sgt i64 %arg5, 0
  br i1 %i267, label %bb268, label %bb603

bb268:                                            ; preds = %bb266
  %i269 = and i32 %arg1, -5
  %i270 = icmp eq i32 %i269, 0
  %i271 = icmp sgt i64 %arg4, 0
  %i272 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 0
  %i273 = getelementptr inbounds %struct._CacheInfo, ptr %i13, i64 0, i32 21
  br label %bb274

bb274:                                            ; preds = %bb598, %bb268
  %i275 = phi ptr [ %i171, %bb268 ], [ %i600, %bb598 ]
  %i276 = phi ptr [ %i173, %bb268 ], [ %i599, %bb598 ]
  %i277 = phi i64 [ 0, %bb268 ], [ %i601, %bb598 ]
  %i278 = add nsw i64 %i277, %arg3
  br i1 %i270, label %bb279, label %bb286

bb279:                                            ; preds = %bb274
  %i280 = load i64, ptr %i184, align 8
  %i281 = icmp slt i64 %i278, 0
  %i282 = icmp slt i64 %i278, %i280
  %i283 = add i64 %i280, -1
  %i284 = select i1 %i282, i64 %i278, i64 %i283
  %i285 = select i1 %i281, i64 0, i64 %i284
  br label %bb286

bb286:                                            ; preds = %bb279, %bb274
  %i287 = phi i64 [ %i278, %bb274 ], [ %i285, %bb279 ]
  br i1 %i271, label %bb288, label %bb598

bb288:                                            ; preds = %bb286
  %i289 = icmp slt i64 %i287, 0
  %i290 = ashr i64 %i287, 63
  %i291 = lshr i64 %i287, 63
  %i292 = and i64 %i287, 7
  %i293 = getelementptr inbounds [64 x i64], ptr @DitherMatrix, i64 0, i64 %i292
  br label %bb294

bb294:                                            ; preds = %bb592, %bb288
  %i295 = phi ptr [ %i275, %bb288 ], [ %i594, %bb592 ]
  %i296 = phi ptr [ %i276, %bb288 ], [ %i595, %bb592 ]
  %i297 = phi i64 [ 0, %bb288 ], [ %i596, %bb592 ]
  %i298 = add nsw i64 %i297, %arg2
  %i299 = load i64, ptr %i178, align 8
  %i300 = sub i64 %i299, %i298
  %i301 = sub i64 %arg4, %i297
  %i302 = icmp ult i64 %i300, %i301
  %i303 = select i1 %i302, i64 %i300, i64 %i301
  %i304 = icmp slt i64 %i298, 0
  %i305 = icmp sle i64 %i299, %i298
  %i306 = or i64 %i298, %i287
  %i307 = icmp slt i64 %i306, 0
  %i308 = or i1 %i307, %i305
  br i1 %i308, label %bb314, label %bb309

bb309:                                            ; preds = %bb294
  %i310 = load i64, ptr %i184, align 8
  %i311 = icmp sge i64 %i287, %i310
  %i312 = icmp eq i64 %i303, 0
  %i313 = or i1 %i312, %i311
  br i1 %i313, label %bb314, label %bb567

bb314:                                            ; preds = %bb309, %bb294
  switch i32 %arg1, label %bb315 [
    i32 1, label %bb545
    i32 2, label %bb545
    i32 10, label %bb545
    i32 11, label %bb545
    i32 8, label %bb545
    i32 9, label %bb545
    i32 12, label %bb545
    i32 16, label %bb523
    i32 6, label %bb333
    i32 3, label %bb360
    i32 7, label %bb388
    i32 5, label %bb407
    i32 17, label %bb436
    i32 13, label %bb459
    i32 14, label %bb481
    i32 15, label %bb504
  ]

bb315:                                            ; preds = %bb314
  %i316 = icmp sgt i64 %i299, %i298
  %i317 = add i64 %i299, -1
  %i318 = select i1 %i316, i64 %i298, i64 %i317
  %i319 = select i1 %i304, i64 0, i64 %i318
  %i320 = load i64, ptr %i184, align 8
  %i321 = icmp slt i64 %i287, %i320
  %i322 = add i64 %i320, -1
  %i323 = select i1 %i321, i64 %i287, i64 %i322
  %i324 = select i1 %i289, i64 0, i64 %i323
  %i325 = load ptr, ptr %i225, align 8
  %i326 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 %arg1, i64 %i319, i64 %i324, i64 1, i64 1, ptr %i325, ptr %arg7)
  %i327 = load i32, ptr %i272, align 8
  %i328 = icmp eq i32 %i327, 0
  br i1 %i328, label %bb541, label %bb329

bb329:                                            ; preds = %bb315
  %i330 = load ptr, ptr %i225, align 8
  %i331 = getelementptr inbounds %struct._NexusInfo, ptr %i330, i64 0, i32 6
  %i332 = load ptr, ptr %i331, align 8
  br label %bb541

bb333:                                            ; preds = %bb314
  %i334 = load ptr, ptr %i273, align 8
  %i335 = icmp eq ptr %i334, null
  br i1 %i335, label %bb336, label %bb339

bb336:                                            ; preds = %bb333
  %i337 = call ptr @AcquireRandomInfo()
  store ptr %i337, ptr %i273, align 8
  %i338 = load i64, ptr %i178, align 8
  br label %bb339

bb339:                                            ; preds = %bb336, %bb333
  %i340 = phi i64 [ %i338, %bb336 ], [ %i299, %bb333 ]
  %i341 = phi ptr [ %i337, %bb336 ], [ %i334, %bb333 ]
  %i342 = uitofp i64 %i340 to double
  %i343 = call fast double @GetPseudoRandomValue(ptr %i341)
  %i344 = fmul fast double %i343, %i342
  %i345 = fptosi double %i344 to i64
  %i346 = load ptr, ptr %i273, align 8
  %i347 = load i64, ptr %i184, align 8
  %i348 = uitofp i64 %i347 to double
  %i349 = call fast double @GetPseudoRandomValue(ptr %i346)
  %i350 = fmul fast double %i349, %i348
  %i351 = fptosi double %i350 to i64
  %i352 = load ptr, ptr %i225, align 8
  %i353 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 6, i64 %i345, i64 %i351, i64 1, i64 1, ptr %i352, ptr %arg7)
  %i354 = load i32, ptr %i272, align 8
  %i355 = icmp eq i32 %i354, 0
  br i1 %i355, label %bb541, label %bb356

bb356:                                            ; preds = %bb339
  %i357 = load ptr, ptr %i225, align 8
  %i358 = getelementptr inbounds %struct._NexusInfo, ptr %i357, i64 0, i32 6
  %i359 = load ptr, ptr %i358, align 8
  br label %bb541

bb360:                                            ; preds = %bb314
  %i361 = and i64 %i298, 7
  %i362 = getelementptr inbounds [64 x i64], ptr @DitherMatrix, i64 0, i64 %i361
  %i363 = load i64, ptr %i362, align 8
  %i364 = add nsw i64 %i363, %i298
  %i365 = add nsw i64 %i364, -32
  %i366 = icmp slt i64 %i364, 32
  %i367 = icmp slt i64 %i365, %i299
  %i368 = add nsw i64 %i299, -1
  %i369 = select i1 %i367, i64 %i365, i64 %i368
  %i370 = select i1 %i366, i64 0, i64 %i369
  %i371 = load i64, ptr %i184, align 8
  %i372 = load i64, ptr %i293, align 8
  %i373 = add nsw i64 %i372, %i287
  %i374 = add nsw i64 %i373, -32
  %i375 = icmp slt i64 %i373, 32
  %i376 = icmp slt i64 %i374, %i371
  %i377 = add nsw i64 %i371, -1
  %i378 = select i1 %i376, i64 %i374, i64 %i377
  %i379 = select i1 %i375, i64 0, i64 %i378
  %i380 = load ptr, ptr %i225, align 8
  %i381 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 3, i64 %i370, i64 %i379, i64 1, i64 1, ptr %i380, ptr %arg7)
  %i382 = load i32, ptr %i272, align 8
  %i383 = icmp eq i32 %i382, 0
  br i1 %i383, label %bb541, label %bb384

bb384:                                            ; preds = %bb360
  %i385 = load ptr, ptr %i225, align 8
  %i386 = getelementptr inbounds %struct._NexusInfo, ptr %i385, i64 0, i32 6
  %i387 = load ptr, ptr %i386, align 8
  br label %bb541

bb388:                                            ; preds = %bb314
  %i389 = sdiv i64 %i298, %i299
  %i390 = ashr i64 %i298, 63
  %i391 = add nsw i64 %i389, %i390
  %i392 = mul nsw i64 %i391, %i299
  %i393 = sub nsw i64 %i298, %i392
  %i394 = load i64, ptr %i184, align 8
  %i395 = sdiv i64 %i287, %i394
  %i396 = add nsw i64 %i395, %i290
  %i397 = mul nsw i64 %i396, %i394
  %i398 = sub nsw i64 %i287, %i397
  %i399 = load ptr, ptr %i225, align 8
  %i400 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 7, i64 %i393, i64 %i398, i64 1, i64 1, ptr %i399, ptr %arg7)
  %i401 = load i32, ptr %i272, align 8
  %i402 = icmp eq i32 %i401, 0
  br i1 %i402, label %bb541, label %bb403

bb403:                                            ; preds = %bb388
  %i404 = load ptr, ptr %i225, align 8
  %i405 = getelementptr inbounds %struct._NexusInfo, ptr %i404, i64 0, i32 6
  %i406 = load ptr, ptr %i405, align 8
  br label %bb541

bb407:                                            ; preds = %bb314
  %i408 = sdiv i64 %i298, %i299
  %i409 = ashr i64 %i298, 63
  %i410 = add nsw i64 %i408, %i409
  %i411 = mul nsw i64 %i410, %i299
  %i412 = sub nsw i64 %i298, %i411
  %i413 = and i64 %i410, 1
  %i414 = icmp eq i64 %i413, 0
  %i415 = xor i64 %i412, -1
  %i416 = add i64 %i299, %i415
  %i417 = select i1 %i414, i64 %i412, i64 %i416
  %i418 = load i64, ptr %i184, align 8
  %i419 = sdiv i64 %i287, %i418
  %i420 = add nsw i64 %i419, %i290
  %i421 = mul nsw i64 %i420, %i418
  %i422 = sub nsw i64 %i287, %i421
  %i423 = and i64 %i420, 1
  %i424 = icmp eq i64 %i423, 0
  %i425 = xor i64 %i422, -1
  %i426 = add i64 %i418, %i425
  %i427 = select i1 %i424, i64 %i422, i64 %i426
  %i428 = load ptr, ptr %i225, align 8
  %i429 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 5, i64 %i417, i64 %i427, i64 1, i64 1, ptr %i428, ptr %arg7)
  %i430 = load i32, ptr %i272, align 8
  %i431 = icmp eq i32 %i430, 0
  br i1 %i431, label %bb541, label %bb432

bb432:                                            ; preds = %bb407
  %i433 = load ptr, ptr %i225, align 8
  %i434 = getelementptr inbounds %struct._NexusInfo, ptr %i433, i64 0, i32 6
  %i435 = load ptr, ptr %i434, align 8
  br label %bb541

bb436:                                            ; preds = %bb314
  %i437 = sdiv i64 %i298, %i299
  %i438 = ashr i64 %i298, 63
  %i439 = add nsw i64 %i437, %i438
  %i440 = load i64, ptr %i184, align 8
  %i441 = sdiv i64 %i287, %i440
  %i442 = add nsw i64 %i441, %i290
  %i443 = xor i64 %i442, %i439
  %i444 = and i64 %i443, 1
  %i445 = icmp eq i64 %i444, 0
  br i1 %i445, label %bb446, label %bb545

bb446:                                            ; preds = %bb436
  %i447 = mul nsw i64 %i442, %i440
  %i448 = sub nsw i64 %i287, %i447
  %i449 = mul nsw i64 %i439, %i299
  %i450 = sub nsw i64 %i298, %i449
  %i451 = load ptr, ptr %i225, align 8
  %i452 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 17, i64 %i450, i64 %i448, i64 1, i64 1, ptr %i451, ptr %arg7)
  %i453 = load i32, ptr %i272, align 8
  %i454 = icmp eq i32 %i453, 0
  br i1 %i454, label %bb541, label %bb455

bb455:                                            ; preds = %bb446
  %i456 = load ptr, ptr %i225, align 8
  %i457 = getelementptr inbounds %struct._NexusInfo, ptr %i456, i64 0, i32 6
  %i458 = load ptr, ptr %i457, align 8
  br label %bb541

bb459:                                            ; preds = %bb314
  br i1 %i289, label %bb545, label %bb460

bb460:                                            ; preds = %bb459
  %i461 = load i64, ptr %i184, align 8
  %i462 = icmp slt i64 %i287, %i461
  br i1 %i462, label %bb463, label %bb545

bb463:                                            ; preds = %bb460
  %i464 = sdiv i64 %i298, %i299
  %i465 = ashr i64 %i298, 63
  %i466 = add nsw i64 %i464, %i465
  %i467 = mul nsw i64 %i466, %i299
  %i468 = sub nsw i64 %i298, %i467
  %i469 = sdiv i64 %i287, %i461
  %i470 = add nuw nsw i64 %i469, %i291
  %i471 = mul nsw i64 %i470, %i461
  %i472 = sub nsw i64 %i287, %i471
  %i473 = load ptr, ptr %i225, align 8
  %i474 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 13, i64 %i468, i64 %i472, i64 1, i64 1, ptr %i473, ptr %arg7)
  %i475 = load i32, ptr %i272, align 8
  %i476 = icmp eq i32 %i475, 0
  br i1 %i476, label %bb541, label %bb477

bb477:                                            ; preds = %bb463
  %i478 = load ptr, ptr %i225, align 8
  %i479 = getelementptr inbounds %struct._NexusInfo, ptr %i478, i64 0, i32 6
  %i480 = load ptr, ptr %i479, align 8
  br label %bb541

bb481:                                            ; preds = %bb314
  %i482 = xor i1 %i304, true
  %i483 = icmp sgt i64 %i299, %i298
  %i484 = and i1 %i483, %i482
  br i1 %i484, label %bb485, label %bb545

bb485:                                            ; preds = %bb481
  %i486 = sdiv i64 %i298, %i299
  %i487 = lshr i64 %i298, 63
  %i488 = add nuw nsw i64 %i486, %i487
  %i489 = mul nsw i64 %i488, %i299
  %i490 = sub nsw i64 %i298, %i489
  %i491 = load i64, ptr %i184, align 8
  %i492 = sdiv i64 %i287, %i491
  %i493 = add nsw i64 %i492, %i290
  %i494 = mul nsw i64 %i493, %i491
  %i495 = sub nsw i64 %i287, %i494
  %i496 = load ptr, ptr %i225, align 8
  %i497 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 14, i64 %i490, i64 %i495, i64 1, i64 1, ptr %i496, ptr %arg7)
  %i498 = load i32, ptr %i272, align 8
  %i499 = icmp eq i32 %i498, 0
  br i1 %i499, label %bb541, label %bb500

bb500:                                            ; preds = %bb485
  %i501 = load ptr, ptr %i225, align 8
  %i502 = getelementptr inbounds %struct._NexusInfo, ptr %i501, i64 0, i32 6
  %i503 = load ptr, ptr %i502, align 8
  br label %bb541

bb504:                                            ; preds = %bb314
  %i505 = sdiv i64 %i298, %i299
  %i506 = ashr i64 %i298, 63
  %i507 = add nsw i64 %i505, %i506
  %i508 = mul nsw i64 %i507, %i299
  %i509 = sub nsw i64 %i298, %i508
  %i510 = load i64, ptr %i184, align 8
  %i511 = icmp slt i64 %i287, %i510
  %i512 = add i64 %i510, -1
  %i513 = select i1 %i511, i64 %i287, i64 %i512
  %i514 = select i1 %i289, i64 0, i64 %i513
  %i515 = load ptr, ptr %i225, align 8
  %i516 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 15, i64 %i509, i64 %i514, i64 1, i64 1, ptr %i515, ptr %arg7)
  %i517 = load i32, ptr %i272, align 8
  %i518 = icmp eq i32 %i517, 0
  br i1 %i518, label %bb541, label %bb519

bb519:                                            ; preds = %bb504
  %i520 = load ptr, ptr %i225, align 8
  %i521 = getelementptr inbounds %struct._NexusInfo, ptr %i520, i64 0, i32 6
  %i522 = load ptr, ptr %i521, align 8
  br label %bb541

bb523:                                            ; preds = %bb314
  %i524 = load i64, ptr %i184, align 8
  %i525 = sdiv i64 %i287, %i524
  %i526 = add nsw i64 %i525, %i290
  %i527 = mul nsw i64 %i526, %i524
  %i528 = sub nsw i64 %i287, %i527
  %i529 = icmp sgt i64 %i299, %i298
  %i530 = add i64 %i299, -1
  %i531 = select i1 %i529, i64 %i298, i64 %i530
  %i532 = select i1 %i304, i64 0, i64 %i531
  %i533 = load ptr, ptr %i225, align 8
  %i534 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 16, i64 %i532, i64 %i528, i64 1, i64 1, ptr %i533, ptr %arg7)
  %i535 = load i32, ptr %i272, align 8
  %i536 = icmp eq i32 %i535, 0
  br i1 %i536, label %bb541, label %bb537

bb537:                                            ; preds = %bb523
  %i538 = load ptr, ptr %i225, align 8
  %i539 = getelementptr inbounds %struct._NexusInfo, ptr %i538, i64 0, i32 6
  %i540 = load ptr, ptr %i539, align 8
  br label %bb541

bb541:                                            ; preds = %bb537, %bb523, %bb519, %bb504, %bb500, %bb485, %bb477, %bb463, %bb455, %bb446, %bb432, %bb407, %bb403, %bb388, %bb384, %bb360, %bb356, %bb339, %bb329, %bb315
  %i542 = phi ptr [ %i326, %bb315 ], [ %i326, %bb329 ], [ %i353, %bb339 ], [ %i353, %bb356 ], [ %i381, %bb360 ], [ %i381, %bb384 ], [ %i400, %bb388 ], [ %i400, %bb403 ], [ %i429, %bb407 ], [ %i429, %bb432 ], [ %i452, %bb446 ], [ %i452, %bb455 ], [ %i474, %bb463 ], [ %i474, %bb477 ], [ %i497, %bb485 ], [ %i497, %bb500 ], [ %i516, %bb504 ], [ %i516, %bb519 ], [ %i534, %bb523 ], [ %i534, %bb537 ]
  %i543 = phi ptr [ null, %bb315 ], [ %i332, %bb329 ], [ null, %bb339 ], [ %i359, %bb356 ], [ null, %bb360 ], [ %i387, %bb384 ], [ null, %bb388 ], [ %i406, %bb403 ], [ null, %bb407 ], [ %i435, %bb432 ], [ null, %bb446 ], [ %i458, %bb455 ], [ null, %bb463 ], [ %i480, %bb477 ], [ null, %bb485 ], [ %i503, %bb500 ], [ null, %bb504 ], [ %i522, %bb519 ], [ null, %bb523 ], [ %i540, %bb537 ]
  %i544 = icmp eq ptr %i542, null
  br i1 %i544, label %bb598, label %bb545

bb545:                                            ; preds = %bb541, %bb481, %bb460, %bb459, %bb436, %bb314, %bb314, %bb314, %bb314, %bb314, %bb314, %bb314
  %i546 = phi ptr [ %i543, %bb541 ], [ %i, %bb314 ], [ %i, %bb314 ], [ %i, %bb314 ], [ %i, %bb314 ], [ %i, %bb314 ], [ %i, %bb314 ], [ %i, %bb314 ], [ %i, %bb436 ], [ %i, %bb460 ], [ %i, %bb459 ], [ %i, %bb481 ]
  %i547 = phi ptr [ %i542, %bb541 ], [ %i8, %bb314 ], [ %i8, %bb314 ], [ %i8, %bb314 ], [ %i8, %bb314 ], [ %i8, %bb314 ], [ %i8, %bb314 ], [ %i8, %bb314 ], [ %i8, %bb436 ], [ %i8, %bb460 ], [ %i8, %bb459 ], [ %i8, %bb481 ]
  %i548 = getelementptr inbounds %struct._PixelPacket, ptr %i296, i64 1
  %i549 = getelementptr inbounds %struct._PixelPacket, ptr %i547, i64 0, i32 0
  %i550 = load i16, ptr %i549, align 2
  %i551 = getelementptr inbounds %struct._PixelPacket, ptr %i296, i64 0, i32 0
  store i16 %i550, ptr %i551, align 2
  %i552 = getelementptr inbounds %struct._PixelPacket, ptr %i547, i64 0, i32 1
  %i553 = load i16, ptr %i552, align 2
  %i554 = getelementptr inbounds %struct._PixelPacket, ptr %i296, i64 0, i32 1
  store i16 %i553, ptr %i554, align 2
  %i555 = getelementptr inbounds %struct._PixelPacket, ptr %i547, i64 0, i32 2
  %i556 = load i16, ptr %i555, align 2
  %i557 = getelementptr inbounds %struct._PixelPacket, ptr %i296, i64 0, i32 2
  store i16 %i556, ptr %i557, align 2
  %i558 = getelementptr inbounds %struct._PixelPacket, ptr %i547, i64 0, i32 3
  %i559 = load i16, ptr %i558, align 2
  %i560 = getelementptr inbounds %struct._PixelPacket, ptr %i296, i64 0, i32 3
  store i16 %i559, ptr %i560, align 2
  %i561 = icmp ne ptr %i295, null
  %i562 = icmp ne ptr %i546, null
  %i563 = and i1 %i561, %i562
  br i1 %i563, label %bb564, label %bb592

bb564:                                            ; preds = %bb545
  %i565 = load i16, ptr %i546, align 2
  %i566 = getelementptr inbounds i16, ptr %i295, i64 1
  store i16 %i565, ptr %i295, align 2
  br label %bb592

bb567:                                            ; preds = %bb309
  %i568 = load ptr, ptr %i225, align 8
  %i569 = call ptr @GetVirtualPixelsFromNexus(ptr %arg, i32 %arg1, i64 %i298, i64 %i287, i64 %i303, i64 1, ptr %i568, ptr %arg7)
  %i570 = icmp eq ptr %i569, null
  br i1 %i570, label %bb598, label %bb571

bb571:                                            ; preds = %bb567
  %i572 = load i32, ptr %i272, align 8
  %i573 = icmp eq i32 %i572, 0
  br i1 %i573, label %bb578, label %bb574

bb574:                                            ; preds = %bb571
  %i575 = load ptr, ptr %i225, align 8
  %i576 = getelementptr inbounds %struct._NexusInfo, ptr %i575, i64 0, i32 6
  %i577 = load ptr, ptr %i576, align 8
  br label %bb578

bb578:                                            ; preds = %bb574, %bb571
  %i579 = phi ptr [ %i577, %bb574 ], [ null, %bb571 ]
  %i580 = bitcast ptr %i296 to ptr
  %i581 = bitcast ptr %i569 to ptr
  %i582 = shl i64 %i303, 3
  call void @llvm.memcpy.p0.p0.i64(ptr align 2 %i580, ptr nonnull align 2 %i581, i64 %i582, i1 false)
  %i583 = getelementptr inbounds %struct._PixelPacket, ptr %i296, i64 %i303
  %i584 = icmp ne ptr %i295, null
  %i585 = icmp ne ptr %i579, null
  %i586 = and i1 %i584, %i585
  br i1 %i586, label %bb587, label %bb592

bb587:                                            ; preds = %bb578
  %i588 = bitcast ptr %i295 to ptr
  %i589 = bitcast ptr %i579 to ptr
  %i590 = shl i64 %i303, 1
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i588, ptr nonnull align 2 %i589, i64 %i590, i1 false)
  %i591 = getelementptr inbounds i16, ptr %i295, i64 %i303
  br label %bb592

bb592:                                            ; preds = %bb587, %bb578, %bb564, %bb545
  %i593 = phi i64 [ %i303, %bb578 ], [ %i303, %bb587 ], [ 1, %bb564 ], [ 1, %bb545 ]
  %i594 = phi ptr [ %i295, %bb578 ], [ %i591, %bb587 ], [ %i566, %bb564 ], [ %i295, %bb545 ]
  %i595 = phi ptr [ %i583, %bb578 ], [ %i583, %bb587 ], [ %i548, %bb564 ], [ %i548, %bb545 ]
  %i596 = add i64 %i593, %i297
  %i597 = icmp slt i64 %i596, %arg4
  br i1 %i597, label %bb294, label %bb598

bb598:                                            ; preds = %bb592, %bb567, %bb541, %bb286
  %i599 = phi ptr [ %i276, %bb286 ], [ %i595, %bb592 ], [ %i296, %bb567 ], [ %i296, %bb541 ]
  %i600 = phi ptr [ %i275, %bb286 ], [ %i594, %bb592 ], [ %i295, %bb567 ], [ %i295, %bb541 ]
  %i601 = add nuw nsw i64 %i277, 1
  %i602 = icmp eq i64 %i601, %arg5
  br i1 %i602, label %bb603, label %bb274

bb603:                                            ; preds = %bb598, %bb266
  %i604 = load ptr, ptr %i225, align 8
  %i605 = getelementptr inbounds %struct._NexusInfo, ptr %i604, i64 0, i32 3
  %i606 = load ptr, ptr %i605, align 8
  %i607 = icmp eq ptr %i606, null
  %i608 = bitcast ptr %i606 to ptr
  br i1 %i607, label %bb623, label %bb609

bb609:                                            ; preds = %bb603
  %i610 = getelementptr inbounds %struct._NexusInfo, ptr %i604, i64 0, i32 0
  %i611 = load i32, ptr %i610, align 8
  %i612 = icmp eq i32 %i611, 0
  br i1 %i612, label %bb613, label %bb615

bb613:                                            ; preds = %bb609
  %i614 = call ptr @RelinquishAlignedMemory(ptr nonnull %i608)
  br label %bb618

bb615:                                            ; preds = %bb609
  %i616 = getelementptr inbounds %struct._NexusInfo, ptr %i604, i64 0, i32 2
  %i617 = load i64, ptr %i616, align 8
  br label %bb618

bb618:                                            ; preds = %bb615, %bb613
  store ptr null, ptr %i605, align 8
  %i619 = getelementptr inbounds %struct._NexusInfo, ptr %i604, i64 0, i32 4
  store ptr null, ptr %i619, align 8
  %i620 = getelementptr inbounds %struct._NexusInfo, ptr %i604, i64 0, i32 6
  store ptr null, ptr %i620, align 8
  %i621 = getelementptr inbounds %struct._NexusInfo, ptr %i604, i64 0, i32 2
  store i64 0, ptr %i621, align 8
  store i32 0, ptr %i610, align 8
  %i622 = load ptr, ptr %i225, align 8
  br label %bb623

bb623:                                            ; preds = %bb618, %bb603
  %i624 = phi ptr [ %i622, %bb618 ], [ %i604, %bb603 ]
  %i625 = getelementptr inbounds %struct._NexusInfo, ptr %i624, i64 0, i32 7
  store i64 -2880220588, ptr %i625, align 8
  %i626 = bitcast ptr %i225 to ptr
  %i627 = load ptr, ptr %i626, align 8
  %i628 = call ptr @RelinquishMagickMemory(ptr %i627)
  store ptr null, ptr %i225, align 8
  %i629 = bitcast ptr %i225 to ptr
  %i630 = call ptr @RelinquishAlignedMemory(ptr nonnull %i629)
  br label %bb631

bb631:                                            ; preds = %bb623, %bb227, %bb223, %bb220, %bb209, %bb207, %bb170, %bb134, %bb106, %bb
  %i632 = phi ptr [ null, %bb227 ], [ %i173, %bb623 ], [ null, %bb ], [ null, %bb170 ], [ %i173, %bb223 ], [ %i173, %bb207 ], [ null, %bb209 ], [ null, %bb220 ], [ null, %bb106 ], [ null, %bb134 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i10)
  call void @llvm.lifetime.end.p0(i64 2, ptr nonnull %i9)
  ret ptr %i632
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nofree nounwind willreturn }
; end INTEL_FEATURE_SW_ADVANCED
