; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt < %s -passes='module(ip-cloning)' -force-ip-manyreccalls-splitting -debug-only=ipcloning -S 2>&1 | FileCheck %s

; Test for splitting part of inlining and splitting for many recursive calls
; functions.

; Check that GetVirtualPixelsFromNexus is not a candidate because it has non CallInst
; uses

; CHECK: MRCS: START ANALYSIS: GetVirtualPixelsFromNexus
; CHECK: MRCS: Can split blocks: Visited: 42 NonVisted: 60
; CHECK: MRCS: EXIT: Non-CallInst Use

@.str.129 = private unnamed_addr constant [15 x i8] c"magick/cache.c\00", align 1
@.str.1.130 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str.2.131 = private unnamed_addr constant [23 x i8] c"MemoryAllocationFailed\00", align 1
@.str.3.132 = private unnamed_addr constant [5 x i8] c"`%s'\00", align 1
@.str.7.170 = private unnamed_addr constant [22 x i8] c"UnableToGetCacheNexus\00", align 1

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
declare void @myindfunc(i16** %0)

@myotheruse = global %struct._PixelPacket* (%struct._Image*, i32, i64, i64, i64, i64, %struct._NexusInfo*, %struct._ExceptionInfo*)*  @GetVirtualPixelsFromNexus, align 8

define internal %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 %1, i64 %2, i64 %3, i64 %4, i64 %5, %struct._NexusInfo* %6, %struct._ExceptionInfo* %7) #0 {
  %9 = alloca i16, align 2
  %10 = alloca %struct._PixelPacket, align 2
  %11 = bitcast i16* %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 2, i8* nonnull %11) #19
  %12 = bitcast %struct._PixelPacket* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %12) #19
  %13 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 49
  %14 = bitcast i8** %13 to %struct._CacheInfo**
  %15 = load %struct._CacheInfo*, %struct._CacheInfo** %14, align 8
  %16 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 3
  %17 = load i32, i32* %16, align 8
  %18 = icmp eq i32 %17, 0
  br i1 %18, label %633, label %19

19:                                               ; preds = %8
  %20 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 38
  %21 = load %struct._Image*, %struct._Image** %20, align 8
  %22 = icmp eq %struct._Image* %21, null
  br i1 %22, label %23, label %28

23:                                               ; preds = %19
  %24 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 73
  %25 = load %struct._Image*, %struct._Image** %24, align 8
  %26 = icmp ne %struct._Image* %25, null
  %27 = zext i1 %26 to i32
  br label %28

28:                                               ; preds = %23, %19
  %29 = phi i32 [ 1, %19 ], [ %27, %23 ]
  %30 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1
  %31 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 0
  store i64 %4, i64* %31, align 8
  %32 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 1
  store i64 %5, i64* %32, align 8
  %33 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 2
  store i64 %2, i64* %33, align 8
  %34 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 1, i32 3
  store i64 %3, i64* %34, align 8
  %35 = load i32, i32* %16, align 8
  %36 = add i32 %35, -1
  %37 = icmp ult i32 %36, 2
  %38 = icmp eq i32 %29, 0
  %39 = and i1 %38, %37
  %40 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %30, i64 0, i32 1
  %41 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %30, i64 0, i32 0
  br i1 %39, label %42, label %83

42:                                               ; preds = %28
  %43 = add nsw i64 %3, %5
  %44 = icmp sgt i64 %2, -1
  br i1 %44, label %45, label %83

45:                                               ; preds = %42
  %46 = add nsw i64 %2, %4
  %47 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 6
  %48 = load i64, i64* %47, align 8
  %49 = icmp sle i64 %46, %48
  %50 = icmp sgt i64 %3, -1
  %51 = and i1 %50, %49
  br i1 %51, label %52, label %83

52:                                               ; preds = %45
  %53 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 7
  %54 = load i64, i64* %53, align 8
  %55 = icmp sgt i64 %43, %54
  br i1 %55, label %83, label %56

56:                                               ; preds = %52
  %57 = icmp eq i64 %5, 1
  br i1 %57, label %65, label %58

58:                                               ; preds = %56
  %59 = icmp eq i64 %2, 0
  br i1 %59, label %60, label %83

60:                                               ; preds = %58
  %61 = icmp eq i64 %48, %4
  br i1 %61, label %65, label %62

62:                                               ; preds = %60
  %63 = urem i64 %4, %48
  %64 = icmp eq i64 %63, 0
  br i1 %64, label %65, label %83

65:                                               ; preds = %62, %60, %56
  %66 = mul i64 %48, %3
  %67 = add i64 %66, %2
  %68 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 14
  %69 = load %struct._PixelPacket*, %struct._PixelPacket** %68, align 8
  %70 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %69, i64 %67
  %71 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 4
  store %struct._PixelPacket* %70, %struct._PixelPacket** %71, align 8
  %72 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  store i16* null, i16** %72, align 8
  %73 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 16
  %74 = load i32, i32* %73, align 8
  %75 = icmp eq i32 %74, 0
  br i1 %75, label %80, label %76

76:                                               ; preds = %65
  %77 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 15
  %78 = load i16*, i16** %77, align 8
  %79 = getelementptr inbounds i16, i16* %78, i64 %67
  store i16* %79, i16** %72, align 8
  br label %80

80:                                               ; preds = %76, %65
  %81 = phi i16* [ %79, %76 ], [ null, %65 ]
  %82 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 5
  store i32 1, i32* %82, align 8
  br label %172

83:                                               ; preds = %62, %58, %52, %45, %42, %28
  %84 = mul i64 %4, %5
  %85 = shl i64 %84, 3
  %86 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 16
  %87 = load i32, i32* %86, align 8
  %88 = icmp eq i32 %87, 0
  %89 = mul i64 %84, 10
  %90 = select i1 %88, i64 %85, i64 %89
  %91 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 3
  %92 = load %struct._PixelPacket*, %struct._PixelPacket** %91, align 8
  %93 = icmp eq %struct._PixelPacket* %92, null
  %94 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 2
  %95 = bitcast %struct._PixelPacket* %92 to i8*
  br i1 %93, label %96, label %112

96:                                               ; preds = %83
  store i64 %90, i64* %94, align 8
  %97 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 0
  store i32 0, i32* %97, align 8
  %98 = tail call i8* @AcquireAlignedMemory(i64 1, i64 %90) #34
  %99 = bitcast i8* %98 to %struct._PixelPacket*
  store %struct._PixelPacket* %99, %struct._PixelPacket** %91, align 8
  %100 = icmp eq i8* %98, null
  br i1 %100, label %101, label %103

101:                                              ; preds = %96
  store i32 1, i32* %97, align 8
  %102 = load i64, i64* %94, align 8
  store %struct._PixelPacket* null, %struct._PixelPacket** %91, align 8
  br label %103

103:                                              ; preds = %101, %96
  %104 = phi i8* [ null, %101 ], [ %98, %96 ]
  %105 = phi %struct._PixelPacket* [ null, %101 ], [ %99, %96 ]
  %106 = ptrtoint i8* %104 to i64
  %107 = icmp eq %struct._PixelPacket* %105, null
  br i1 %107, label %108, label %140

108:                                              ; preds = %103
  %109 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 18
  %110 = getelementptr inbounds [4096 x i8], [4096 x i8]* %109, i64 0, i64 0
  %111 = tail call i32 (%struct._ExceptionInfo*, i8*, i8*, i64, i32, i8*, i8*, ...) @ThrowMagickException(%struct._ExceptionInfo* %7, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.129, i64 0, i64 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.1.130, i64 0, i64 0), i64 4688, i32 400, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.2.131, i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.3.132, i64 0, i64 0), i8* nonnull %110) #19
  store i64 0, i64* %94, align 8
  br label %633

112:                                              ; preds = %83
  %113 = ptrtoint %struct._PixelPacket* %92 to i64
  %114 = load i64, i64* %94, align 8
  %115 = icmp ult i64 %114, %90
  br i1 %115, label %116, label %140

116:                                              ; preds = %112
  %117 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 0
  %118 = load i32, i32* %117, align 8
  %119 = icmp eq i32 %118, 0
  br i1 %119, label %120, label %122

120:                                              ; preds = %116
  %121 = tail call i8* @RelinquishAlignedMemory(i8* nonnull %95) #19
  br label %123

122:                                              ; preds = %116
  br label %123

123:                                              ; preds = %122, %120
  store %struct._PixelPacket* null, %struct._PixelPacket** %91, align 8
  %124 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 4
  store %struct._PixelPacket* null, %struct._PixelPacket** %124, align 8
  %125 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  store i16* null, i16** %125, align 8
  store i64 %90, i64* %94, align 8
  store i32 0, i32* %117, align 8
  %126 = tail call i8* @AcquireAlignedMemory(i64 1, i64 %90) #34
  %127 = bitcast i8* %126 to %struct._PixelPacket*
  store %struct._PixelPacket* %127, %struct._PixelPacket** %91, align 8
  %128 = icmp eq i8* %126, null
  br i1 %128, label %129, label %131

129:                                              ; preds = %123
  store i32 1, i32* %117, align 8
  %130 = load i64, i64* %94, align 8
  store %struct._PixelPacket* null, %struct._PixelPacket** %91, align 8
  br label %131

131:                                              ; preds = %129, %123
  %132 = phi i8* [ null, %129 ], [ %126, %123 ]
  %133 = phi %struct._PixelPacket* [ null, %129 ], [ %127, %123 ]
  %134 = ptrtoint i8* %132 to i64
  %135 = icmp eq %struct._PixelPacket* %133, null
  br i1 %135, label %136, label %140

136:                                              ; preds = %131
  %137 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 18
  %138 = getelementptr inbounds [4096 x i8], [4096 x i8]* %137, i64 0, i64 0
  %139 = tail call i32 (%struct._ExceptionInfo*, i8*, i8*, i64, i32, i8*, i8*, ...) @ThrowMagickException(%struct._ExceptionInfo* %7, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.129, i64 0, i64 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.1.130, i64 0, i64 0), i64 4688, i32 400, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.2.131, i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.3.132, i64 0, i64 0), i8* nonnull %138) #19
  store i64 0, i64* %94, align 8
  br label %633

140:                                              ; preds = %131, %112, %103
  %141 = phi i64 [ %134, %131 ], [ %106, %103 ], [ %113, %112 ]
  %142 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 4
  %143 = bitcast %struct._PixelPacket** %142 to i64*
  store i64 %141, i64* %143, align 8
  %144 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 6
  store i16* null, i16** %144, align 8
  %145 = load i32, i32* %86, align 8
  %146 = icmp eq i32 %145, 0
  %147 = inttoptr i64 %141 to %struct._PixelPacket*
  br i1 %146, label %151, label %148

148:                                              ; preds = %140
  %149 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %147, i64 %84
  %150 = getelementptr %struct._PixelPacket, %struct._PixelPacket* %149, i64 0, i32 0
  store i16* %150, i16** %144, align 8
  br label %151

151:                                              ; preds = %148, %140
  %152 = phi i16* [ %150, %148 ], [ null, %140 ]
  %153 = load i32, i32* %16, align 8
  %154 = icmp eq i32 %153, 4
  br i1 %154, label %169, label %155

155:                                              ; preds = %151
  %156 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %30, i64 0, i32 3
  %157 = load i64, i64* %156, align 8
  %158 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 6
  %159 = load i64, i64* %158, align 8
  %160 = mul i64 %159, %157
  %161 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %30, i64 0, i32 2
  %162 = load i64, i64* %161, align 8
  %163 = add i64 %160, %162
  %164 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 14
  %165 = load %struct._PixelPacket*, %struct._PixelPacket** %164, align 8
  %166 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %165, i64 %163
  %167 = icmp eq %struct._PixelPacket* %166, %147
  %168 = zext i1 %167 to i32
  br label %169

169:                                              ; preds = %155, %151
  %170 = phi i32 [ %168, %155 ], [ 1, %151 ]
  %171 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %6, i64 0, i32 5
  store i32 %170, i32* %171, align 8
  br label %172

172:                                              ; preds = %169, %80
  %173 = phi i16* [ %152, %169 ], [ %81, %80 ]
  %174 = phi i32 [ %170, %169 ], [ 1, %80 ]
  %175 = phi %struct._PixelPacket* [ %147, %169 ], [ %70, %80 ]
  %176 = icmp eq %struct._PixelPacket* %175, null
  br i1 %176, label %633, label %177

177:                                              ; preds = %172
  %178 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %30, i64 0, i32 3
  %179 = load i64, i64* %178, align 8
  %180 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 6
  %181 = load i64, i64* %180, align 8
  %182 = mul i64 %181, %179
  %183 = getelementptr inbounds %struct._RectangleInfo, %struct._RectangleInfo* %30, i64 0, i32 2
  %184 = load i64, i64* %183, align 8
  %185 = add i64 %182, %184
  %186 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 7
  %187 = load i64, i64* %186, align 8
  %188 = icmp sgt i64 %185, -1
  br i1 %188, label %189, label %226

189:                                              ; preds = %177
  %190 = mul i64 %187, %181
  %191 = load i64, i64* %40, align 8
  %192 = add i64 %191, -1
  %193 = mul i64 %192, %181
  %194 = load i64, i64* %41, align 8
  %195 = add nsw i64 %185, -1
  %196 = add i64 %195, %194
  %197 = add i64 %196, %193
  %198 = icmp ult i64 %197, %190
  %199 = icmp sgt i64 %2, -1
  %200 = and i1 %199, %198
  br i1 %200, label %201, label %226

201:                                              ; preds = %189
  %202 = add i64 %4, %2
  %203 = icmp sgt i64 %202, %181
  %204 = icmp slt i64 %3, 0
  %205 = or i1 %204, %203
  %206 = add i64 %5, %3
  %207 = icmp sgt i64 %206, %187
  %208 = or i1 %205, %207
  br i1 %208, label %226, label %209

209:                                              ; preds = %201
  %210 = icmp eq i32 %174, 0
  br i1 %210, label %211, label %633

211:                                              ; preds = %209
  %212 = tail call fastcc i32 @ReadPixelCachePixels(%struct._CacheInfo* nonnull %15, %struct._NexusInfo* nonnull %6, %struct._ExceptionInfo* %7)
  %213 = icmp eq i32 %212, 0
  br i1 %213, label %633, label %214

214:                                              ; preds = %211
  %215 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 0
  %216 = load i32, i32* %215, align 8
  %217 = icmp eq i32 %216, 2
  br i1 %217, label %222, label %218

218:                                              ; preds = %214
  %219 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 1
  %220 = load i32, i32* %219, align 4
  %221 = icmp eq i32 %220, 12
  br i1 %221, label %222, label %225

222:                                              ; preds = %218, %214
  %223 = tail call fastcc i32 @ReadPixelCacheIndexes(%struct._CacheInfo* nonnull %15, %struct._NexusInfo* nonnull %6, %struct._ExceptionInfo* %7)
  %224 = icmp eq i32 %223, 0
  br i1 %224, label %633, label %225

225:                                              ; preds = %222, %218
  br label %633

226:                                              ; preds = %201, %189, %177
  %227 = tail call %struct._NexusInfo** @AcquirePixelCacheNexus(i64 1)
  %228 = icmp eq %struct._NexusInfo** %227, null
  br i1 %228, label %229, label %233

229:                                              ; preds = %226
  %230 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 53
  %231 = getelementptr inbounds [4096 x i8], [4096 x i8]* %230, i64 0, i64 0
  %232 = tail call i32 (%struct._ExceptionInfo*, i8*, i8*, i64, i32, i8*, i8*, ...) @ThrowMagickException(%struct._ExceptionInfo* %7, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.129, i64 0, i64 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.1.130, i64 0, i64 0), i64 2673, i32 445, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.7.170, i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.3.132, i64 0, i64 0), i8* nonnull %231) #19
  br label %633

233:                                              ; preds = %226
  switch i32 %1, label %254 [
    i32 10, label %234
    i32 11, label %239
    i32 8, label %244
    i32 9, label %249
    i32 12, label %249
  ]

234:                                              ; preds = %233
  %235 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  store i16 0, i16* %235, align 2
  %236 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  store i16 0, i16* %236, align 2
  %237 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0
  store i16 0, i16* %237, align 2
  %238 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  store i16 0, i16* %238, align 2
  br label %268

239:                                              ; preds = %233
  %240 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  store i16 32767, i16* %240, align 2
  %241 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  store i16 32767, i16* %241, align 2
  %242 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0
  store i16 32767, i16* %242, align 2
  %243 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  store i16 0, i16* %243, align 2
  br label %268

244:                                              ; preds = %233
  %245 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  store i16 0, i16* %245, align 2
  %246 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  store i16 0, i16* %246, align 2
  %247 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0
  store i16 0, i16* %247, align 2
  %248 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  store i16 -1, i16* %248, align 2
  br label %268

249:                                              ; preds = %233, %233
  %250 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  store i16 -1, i16* %250, align 2
  %251 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  store i16 -1, i16* %251, align 2
  %252 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0
  store i16 -1, i16* %252, align 2
  %253 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  store i16 0, i16* %253, align 2
  br label %268

254:                                              ; preds = %233
  %255 = getelementptr inbounds %struct._Image, %struct._Image* %0, i64 0, i32 12
  %256 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %255, i64 0, i32 0
  %257 = load i16, i16* %256, align 2
  %258 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 0
  store i16 %257, i16* %258, align 2
  %259 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %255, i64 0, i32 1
  %260 = load i16, i16* %259, align 2
  %261 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 1
  store i16 %260, i16* %261, align 2
  %262 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %255, i64 0, i32 2
  %263 = load i16, i16* %262, align 2
  %264 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 2
  store i16 %263, i16* %264, align 2
  %265 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %255, i64 0, i32 3
  %266 = load i16, i16* %265, align 2
  %267 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %10, i64 0, i32 3
  store i16 %266, i16* %267, align 2
  br label %268

268:                                              ; preds = %254, %249, %244, %239, %234
  store i16 0, i16* %9, align 2
  %269 = icmp sgt i64 %5, 0
  br i1 %269, label %270, label %605

270:                                              ; preds = %268
  %271 = and i32 %1, -5
  %272 = icmp eq i32 %271, 0
  %273 = icmp sgt i64 %4, 0
  %274 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 0
  %275 = getelementptr inbounds %struct._CacheInfo, %struct._CacheInfo* %15, i64 0, i32 21
  br label %276

276:                                              ; preds = %600, %270
  %277 = phi i16* [ %173, %270 ], [ %602, %600 ]
  %278 = phi %struct._PixelPacket* [ %175, %270 ], [ %601, %600 ]
  %279 = phi i64 [ 0, %270 ], [ %603, %600 ]
  %280 = add nsw i64 %279, %3
  br i1 %272, label %281, label %288

281:                                              ; preds = %276
  %282 = load i64, i64* %186, align 8
  %283 = icmp slt i64 %280, 0
  %284 = icmp slt i64 %280, %282
  %285 = add i64 %282, -1
  %286 = select i1 %284, i64 %280, i64 %285
  %287 = select i1 %283, i64 0, i64 %286
  br label %288

288:                                              ; preds = %281, %276
  %289 = phi i64 [ %280, %276 ], [ %287, %281 ]
  br i1 %273, label %290, label %600

290:                                              ; preds = %288
  %291 = icmp slt i64 %289, 0
  %292 = ashr i64 %289, 63
  %293 = lshr i64 %289, 63
  %294 = and i64 %289, 7
  %295 = getelementptr inbounds [64 x i64], [64 x i64]* @DitherMatrix, i64 0, i64 %294
  br label %296

296:                                              ; preds = %594, %290
  %297 = phi i16* [ %277, %290 ], [ %596, %594 ]
  %298 = phi %struct._PixelPacket* [ %278, %290 ], [ %597, %594 ]
  %299 = phi i64 [ 0, %290 ], [ %598, %594 ]
  %300 = add nsw i64 %299, %2
  %301 = load i64, i64* %180, align 8
  %302 = sub i64 %301, %300
  %303 = sub i64 %4, %299
  %304 = icmp ult i64 %302, %303
  %305 = select i1 %304, i64 %302, i64 %303
  %306 = icmp slt i64 %300, 0
  %307 = icmp sle i64 %301, %300
  %308 = or i64 %300, %289
  %309 = icmp slt i64 %308, 0
  %310 = or i1 %309, %307
  br i1 %310, label %316, label %311

311:                                              ; preds = %296
  %312 = load i64, i64* %186, align 8
  %313 = icmp sge i64 %289, %312
  %314 = icmp eq i64 %305, 0
  %315 = or i1 %314, %313
  br i1 %315, label %316, label %569

316:                                              ; preds = %311, %296
  switch i32 %1, label %317 [
    i32 1, label %547
    i32 2, label %547
    i32 10, label %547
    i32 11, label %547
    i32 8, label %547
    i32 9, label %547
    i32 12, label %547
    i32 16, label %525
    i32 6, label %335
    i32 3, label %362
    i32 7, label %390
    i32 5, label %409
    i32 17, label %438
    i32 13, label %461
    i32 14, label %483
    i32 15, label %506
  ]

317:                                              ; preds = %316
  %318 = icmp sgt i64 %301, %300
  %319 = add i64 %301, -1
  %320 = select i1 %318, i64 %300, i64 %319
  %321 = select i1 %306, i64 0, i64 %320
  %322 = load i64, i64* %186, align 8
  %323 = icmp slt i64 %289, %322
  %324 = add i64 %322, -1
  %325 = select i1 %323, i64 %289, i64 %324
  %326 = select i1 %291, i64 0, i64 %325
  %327 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %328 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 %1, i64 %321, i64 %326, i64 1, i64 1, %struct._NexusInfo* %327, %struct._ExceptionInfo* %7)
  %329 = load i32, i32* %274, align 8
  %330 = icmp eq i32 %329, 0
  br i1 %330, label %543, label %331

331:                                              ; preds = %317
  %332 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %333 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %332, i64 0, i32 6
  %334 = load i16*, i16** %333, align 8
  br label %543

335:                                              ; preds = %316
  %336 = load %struct._RandomInfo*, %struct._RandomInfo** %275, align 8
  %337 = icmp eq %struct._RandomInfo* %336, null
  br i1 %337, label %338, label %341

338:                                              ; preds = %335
  %339 = call %struct._RandomInfo* @AcquireRandomInfo() #19
  store %struct._RandomInfo* %339, %struct._RandomInfo** %275, align 8
  %340 = load i64, i64* %180, align 8
  br label %341

341:                                              ; preds = %338, %335
  %342 = phi i64 [ %340, %338 ], [ %301, %335 ]
  %343 = phi %struct._RandomInfo* [ %339, %338 ], [ %336, %335 ]
  %344 = uitofp i64 %342 to double
  %345 = call fast double @GetPseudoRandomValue(%struct._RandomInfo* %343) #19
  %346 = fmul fast double %345, %344
  %347 = fptosi double %346 to i64
  %348 = load %struct._RandomInfo*, %struct._RandomInfo** %275, align 8
  %349 = load i64, i64* %186, align 8
  %350 = uitofp i64 %349 to double
  %351 = call fast double @GetPseudoRandomValue(%struct._RandomInfo* %348) #19
  %352 = fmul fast double %351, %350
  %353 = fptosi double %352 to i64
  %354 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %355 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 6, i64 %347, i64 %353, i64 1, i64 1, %struct._NexusInfo* %354, %struct._ExceptionInfo* %7)
  %356 = load i32, i32* %274, align 8
  %357 = icmp eq i32 %356, 0
  br i1 %357, label %543, label %358

358:                                              ; preds = %341
  %359 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %360 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %359, i64 0, i32 6
  %361 = load i16*, i16** %360, align 8
  br label %543

362:                                              ; preds = %316
  %363 = and i64 %300, 7
  %364 = getelementptr inbounds [64 x i64], [64 x i64]* @DitherMatrix, i64 0, i64 %363
  %365 = load i64, i64* %364, align 8
  %366 = add nsw i64 %365, %300
  %367 = add nsw i64 %366, -32
  %368 = icmp slt i64 %366, 32
  %369 = icmp slt i64 %367, %301
  %370 = add nsw i64 %301, -1
  %371 = select i1 %369, i64 %367, i64 %370
  %372 = select i1 %368, i64 0, i64 %371
  %373 = load i64, i64* %186, align 8
  %374 = load i64, i64* %295, align 8
  %375 = add nsw i64 %374, %289
  %376 = add nsw i64 %375, -32
  %377 = icmp slt i64 %375, 32
  %378 = icmp slt i64 %376, %373
  %379 = add nsw i64 %373, -1
  %380 = select i1 %378, i64 %376, i64 %379
  %381 = select i1 %377, i64 0, i64 %380
  %382 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %383 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 3, i64 %372, i64 %381, i64 1, i64 1, %struct._NexusInfo* %382, %struct._ExceptionInfo* %7)
  %384 = load i32, i32* %274, align 8
  %385 = icmp eq i32 %384, 0
  br i1 %385, label %543, label %386

386:                                              ; preds = %362
  %387 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %388 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %387, i64 0, i32 6
  %389 = load i16*, i16** %388, align 8
  br label %543

390:                                              ; preds = %316
  %391 = sdiv i64 %300, %301
  %392 = ashr i64 %300, 63
  %393 = add nsw i64 %391, %392
  %394 = mul nsw i64 %393, %301
  %395 = sub nsw i64 %300, %394
  %396 = load i64, i64* %186, align 8
  %397 = sdiv i64 %289, %396
  %398 = add nsw i64 %397, %292
  %399 = mul nsw i64 %398, %396
  %400 = sub nsw i64 %289, %399
  %401 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %402 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 7, i64 %395, i64 %400, i64 1, i64 1, %struct._NexusInfo* %401, %struct._ExceptionInfo* %7)
  %403 = load i32, i32* %274, align 8
  %404 = icmp eq i32 %403, 0
  br i1 %404, label %543, label %405

405:                                              ; preds = %390
  %406 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %407 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %406, i64 0, i32 6
  %408 = load i16*, i16** %407, align 8
  br label %543

409:                                              ; preds = %316
  %410 = sdiv i64 %300, %301
  %411 = ashr i64 %300, 63
  %412 = add nsw i64 %410, %411
  %413 = mul nsw i64 %412, %301
  %414 = sub nsw i64 %300, %413
  %415 = and i64 %412, 1
  %416 = icmp eq i64 %415, 0
  %417 = xor i64 %414, -1
  %418 = add i64 %301, %417
  %419 = select i1 %416, i64 %414, i64 %418
  %420 = load i64, i64* %186, align 8
  %421 = sdiv i64 %289, %420
  %422 = add nsw i64 %421, %292
  %423 = mul nsw i64 %422, %420
  %424 = sub nsw i64 %289, %423
  %425 = and i64 %422, 1
  %426 = icmp eq i64 %425, 0
  %427 = xor i64 %424, -1
  %428 = add i64 %420, %427
  %429 = select i1 %426, i64 %424, i64 %428
  %430 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %431 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 5, i64 %419, i64 %429, i64 1, i64 1, %struct._NexusInfo* %430, %struct._ExceptionInfo* %7)
  %432 = load i32, i32* %274, align 8
  %433 = icmp eq i32 %432, 0
  br i1 %433, label %543, label %434

434:                                              ; preds = %409
  %435 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %436 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %435, i64 0, i32 6
  %437 = load i16*, i16** %436, align 8
  br label %543

438:                                              ; preds = %316
  %439 = sdiv i64 %300, %301
  %440 = ashr i64 %300, 63
  %441 = add nsw i64 %439, %440
  %442 = load i64, i64* %186, align 8
  %443 = sdiv i64 %289, %442
  %444 = add nsw i64 %443, %292
  %445 = xor i64 %444, %441
  %446 = and i64 %445, 1
  %447 = icmp eq i64 %446, 0
  br i1 %447, label %448, label %547

448:                                              ; preds = %438
  %449 = mul nsw i64 %444, %442
  %450 = sub nsw i64 %289, %449
  %451 = mul nsw i64 %441, %301
  %452 = sub nsw i64 %300, %451
  %453 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %454 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 17, i64 %452, i64 %450, i64 1, i64 1, %struct._NexusInfo* %453, %struct._ExceptionInfo* %7)
  %455 = load i32, i32* %274, align 8
  %456 = icmp eq i32 %455, 0
  br i1 %456, label %543, label %457

457:                                              ; preds = %448
  %458 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %459 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %458, i64 0, i32 6
  %460 = load i16*, i16** %459, align 8
  br label %543

461:                                              ; preds = %316
  br i1 %291, label %547, label %462

462:                                              ; preds = %461
  %463 = load i64, i64* %186, align 8
  %464 = icmp slt i64 %289, %463
  br i1 %464, label %465, label %547

465:                                              ; preds = %462
  %466 = sdiv i64 %300, %301
  %467 = ashr i64 %300, 63
  %468 = add nsw i64 %466, %467
  %469 = mul nsw i64 %468, %301
  %470 = sub nsw i64 %300, %469
  %471 = sdiv i64 %289, %463
  %472 = add nuw nsw i64 %471, %293
  %473 = mul nsw i64 %472, %463
  %474 = sub nsw i64 %289, %473
  %475 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %476 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 13, i64 %470, i64 %474, i64 1, i64 1, %struct._NexusInfo* %475, %struct._ExceptionInfo* %7)
  %477 = load i32, i32* %274, align 8
  %478 = icmp eq i32 %477, 0
  br i1 %478, label %543, label %479

479:                                              ; preds = %465
  %480 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %481 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %480, i64 0, i32 6
  %482 = load i16*, i16** %481, align 8
  br label %543

483:                                              ; preds = %316
  %484 = xor i1 %306, true
  %485 = icmp sgt i64 %301, %300
  %486 = and i1 %485, %484
  br i1 %486, label %487, label %547

487:                                              ; preds = %483
  %488 = sdiv i64 %300, %301
  %489 = lshr i64 %300, 63
  %490 = add nuw nsw i64 %488, %489
  %491 = mul nsw i64 %490, %301
  %492 = sub nsw i64 %300, %491
  %493 = load i64, i64* %186, align 8
  %494 = sdiv i64 %289, %493
  %495 = add nsw i64 %494, %292
  %496 = mul nsw i64 %495, %493
  %497 = sub nsw i64 %289, %496
  %498 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %499 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 14, i64 %492, i64 %497, i64 1, i64 1, %struct._NexusInfo* %498, %struct._ExceptionInfo* %7)
  %500 = load i32, i32* %274, align 8
  %501 = icmp eq i32 %500, 0
  br i1 %501, label %543, label %502

502:                                              ; preds = %487
  %503 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %504 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %503, i64 0, i32 6
  %505 = load i16*, i16** %504, align 8
  br label %543

506:                                              ; preds = %316
  %507 = sdiv i64 %300, %301
  %508 = ashr i64 %300, 63
  %509 = add nsw i64 %507, %508
  %510 = mul nsw i64 %509, %301
  %511 = sub nsw i64 %300, %510
  %512 = load i64, i64* %186, align 8
  %513 = icmp slt i64 %289, %512
  %514 = add i64 %512, -1
  %515 = select i1 %513, i64 %289, i64 %514
  %516 = select i1 %291, i64 0, i64 %515
  %517 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %518 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 15, i64 %511, i64 %516, i64 1, i64 1, %struct._NexusInfo* %517, %struct._ExceptionInfo* %7)
  %519 = load i32, i32* %274, align 8
  %520 = icmp eq i32 %519, 0
  br i1 %520, label %543, label %521

521:                                              ; preds = %506
  %522 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %523 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %522, i64 0, i32 6
  %524 = load i16*, i16** %523, align 8
  br label %543

525:                                              ; preds = %316
  %526 = load i64, i64* %186, align 8
  %527 = sdiv i64 %289, %526
  %528 = add nsw i64 %527, %292
  %529 = mul nsw i64 %528, %526
  %530 = sub nsw i64 %289, %529
  %531 = icmp sgt i64 %301, %300
  %532 = add i64 %301, -1
  %533 = select i1 %531, i64 %300, i64 %532
  %534 = select i1 %306, i64 0, i64 %533
  %535 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %536 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 16, i64 %534, i64 %530, i64 1, i64 1, %struct._NexusInfo* %535, %struct._ExceptionInfo* %7)
  %537 = load i32, i32* %274, align 8
  %538 = icmp eq i32 %537, 0
  br i1 %538, label %543, label %539

539:                                              ; preds = %525
  %540 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %541 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %540, i64 0, i32 6
  %542 = load i16*, i16** %541, align 8
  br label %543

543:                                              ; preds = %539, %525, %521, %506, %502, %487, %479, %465, %457, %448, %434, %409, %405, %390, %386, %362, %358, %341, %331, %317
  %544 = phi %struct._PixelPacket* [ %328, %317 ], [ %328, %331 ], [ %355, %341 ], [ %355, %358 ], [ %383, %362 ], [ %383, %386 ], [ %402, %390 ], [ %402, %405 ], [ %431, %409 ], [ %431, %434 ], [ %454, %448 ], [ %454, %457 ], [ %476, %465 ], [ %476, %479 ], [ %499, %487 ], [ %499, %502 ], [ %518, %506 ], [ %518, %521 ], [ %536, %525 ], [ %536, %539 ]
  %545 = phi i16* [ null, %317 ], [ %334, %331 ], [ null, %341 ], [ %361, %358 ], [ null, %362 ], [ %389, %386 ], [ null, %390 ], [ %408, %405 ], [ null, %409 ], [ %437, %434 ], [ null, %448 ], [ %460, %457 ], [ null, %465 ], [ %482, %479 ], [ null, %487 ], [ %505, %502 ], [ null, %506 ], [ %524, %521 ], [ null, %525 ], [ %542, %539 ]
  %546 = icmp eq %struct._PixelPacket* %544, null
  br i1 %546, label %600, label %547

547:                                              ; preds = %543, %483, %462, %461, %438, %316, %316, %316, %316, %316, %316, %316
  %548 = phi i16* [ %545, %543 ], [ %9, %316 ], [ %9, %316 ], [ %9, %316 ], [ %9, %316 ], [ %9, %316 ], [ %9, %316 ], [ %9, %316 ], [ %9, %438 ], [ %9, %462 ], [ %9, %461 ], [ %9, %483 ]
  %549 = phi %struct._PixelPacket* [ %544, %543 ], [ %10, %316 ], [ %10, %316 ], [ %10, %316 ], [ %10, %316 ], [ %10, %316 ], [ %10, %316 ], [ %10, %316 ], [ %10, %438 ], [ %10, %462 ], [ %10, %461 ], [ %10, %483 ]
  %550 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %298, i64 1
  %551 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %549, i64 0, i32 0
  %552 = load i16, i16* %551, align 2
  %553 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %298, i64 0, i32 0
  store i16 %552, i16* %553, align 2
  %554 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %549, i64 0, i32 1
  %555 = load i16, i16* %554, align 2
  %556 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %298, i64 0, i32 1
  store i16 %555, i16* %556, align 2
  %557 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %549, i64 0, i32 2
  %558 = load i16, i16* %557, align 2
  %559 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %298, i64 0, i32 2
  store i16 %558, i16* %559, align 2
  %560 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %549, i64 0, i32 3
  %561 = load i16, i16* %560, align 2
  %562 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %298, i64 0, i32 3
  store i16 %561, i16* %562, align 2
  %563 = icmp ne i16* %297, null
  %564 = icmp ne i16* %548, null
  %565 = and i1 %563, %564
  br i1 %565, label %566, label %594

566:                                              ; preds = %547
  %567 = load i16, i16* %548, align 2
  %568 = getelementptr inbounds i16, i16* %297, i64 1
  store i16 %567, i16* %297, align 2
  br label %594

569:                                              ; preds = %311
  %570 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %571 = call %struct._PixelPacket* @GetVirtualPixelsFromNexus(%struct._Image* %0, i32 %1, i64 %300, i64 %289, i64 %305, i64 1, %struct._NexusInfo* %570, %struct._ExceptionInfo* %7)
  %572 = icmp eq %struct._PixelPacket* %571, null
  br i1 %572, label %600, label %573

573:                                              ; preds = %569
  %574 = load i32, i32* %274, align 8
  %575 = icmp eq i32 %574, 0
  br i1 %575, label %580, label %576

576:                                              ; preds = %573
  %577 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %578 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %577, i64 0, i32 6
  %579 = load i16*, i16** %578, align 8
  br label %580

580:                                              ; preds = %576, %573
  %581 = phi i16* [ %579, %576 ], [ null, %573 ]
  %582 = bitcast %struct._PixelPacket* %298 to i8*
  %583 = bitcast %struct._PixelPacket* %571 to i8*
  %584 = shl i64 %305, 3
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 %582, i8* nonnull align 2 %583, i64 %584, i1 false)
  %585 = getelementptr inbounds %struct._PixelPacket, %struct._PixelPacket* %298, i64 %305
  %586 = icmp ne i16* %297, null
  %587 = icmp ne i16* %581, null
  %588 = and i1 %586, %587
  br i1 %588, label %589, label %594

589:                                              ; preds = %580
  %590 = bitcast i16* %297 to i8*
  %591 = bitcast i16* %581 to i8*
  %592 = shl i64 %305, 1
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 2 %590, i8* nonnull align 2 %591, i64 %592, i1 false)
  %593 = getelementptr inbounds i16, i16* %297, i64 %305
  br label %594

594:                                              ; preds = %589, %580, %566, %547
  %595 = phi i64 [ %305, %580 ], [ %305, %589 ], [ 1, %566 ], [ 1, %547 ]
  %596 = phi i16* [ %297, %580 ], [ %593, %589 ], [ %568, %566 ], [ %297, %547 ]
  %597 = phi %struct._PixelPacket* [ %585, %580 ], [ %585, %589 ], [ %550, %566 ], [ %550, %547 ]
  %598 = add i64 %595, %299
  %599 = icmp slt i64 %598, %4
  br i1 %599, label %296, label %600

600:                                              ; preds = %594, %569, %543, %288
  %601 = phi %struct._PixelPacket* [ %278, %288 ], [ %597, %594 ], [ %298, %569 ], [ %298, %543 ]
  %602 = phi i16* [ %277, %288 ], [ %596, %594 ], [ %297, %569 ], [ %297, %543 ]
  %603 = add nuw nsw i64 %279, 1
  %604 = icmp eq i64 %603, %5
  br i1 %604, label %605, label %276

605:                                              ; preds = %600, %268
  %606 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  %607 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %606, i64 0, i32 3
  %608 = load %struct._PixelPacket*, %struct._PixelPacket** %607, align 8
  %609 = icmp eq %struct._PixelPacket* %608, null
  %610 = bitcast %struct._PixelPacket* %608 to i8*
  br i1 %609, label %625, label %611

611:                                              ; preds = %605
  %612 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %606, i64 0, i32 0
  %613 = load i32, i32* %612, align 8
  %614 = icmp eq i32 %613, 0
  br i1 %614, label %615, label %617

615:                                              ; preds = %611
  %616 = call i8* @RelinquishAlignedMemory(i8* nonnull %610) #19
  br label %620

617:                                              ; preds = %611
  %618 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %606, i64 0, i32 2
  %619 = load i64, i64* %618, align 8
  br label %620

620:                                              ; preds = %617, %615
  store %struct._PixelPacket* null, %struct._PixelPacket** %607, align 8
  %621 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %606, i64 0, i32 4
  store %struct._PixelPacket* null, %struct._PixelPacket** %621, align 8
  %622 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %606, i64 0, i32 6
  store i16* null, i16** %622, align 8
  %623 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %606, i64 0, i32 2
  store i64 0, i64* %623, align 8
  store i32 0, i32* %612, align 8
  %624 = load %struct._NexusInfo*, %struct._NexusInfo** %227, align 8
  br label %625

625:                                              ; preds = %620, %605
  %626 = phi %struct._NexusInfo* [ %624, %620 ], [ %606, %605 ]
  %627 = getelementptr inbounds %struct._NexusInfo, %struct._NexusInfo* %626, i64 0, i32 7
  store i64 -2880220588, i64* %627, align 8
  %628 = bitcast %struct._NexusInfo** %227 to i8**
  %629 = load i8*, i8** %628, align 8
  %630 = call i8* @RelinquishMagickMemory(i8* %629) #19
  store %struct._NexusInfo* null, %struct._NexusInfo** %227, align 8
  %631 = bitcast %struct._NexusInfo** %227 to i8*
  %632 = call i8* @RelinquishAlignedMemory(i8* nonnull %631) #19
  br label %633

633:                                              ; preds = %625, %229, %225, %222, %211, %209, %172, %136, %108, %8
  %634 = phi %struct._PixelPacket* [ null, %229 ], [ %175, %625 ], [ null, %8 ], [ null, %172 ], [ %175, %225 ], [ %175, %209 ], [ null, %211 ], [ null, %222 ], [ null, %108 ], [ null, %136 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %12) #19
  call void @llvm.lifetime.end.p0i8(i64 2, i8* nonnull %11) #19
  ret %struct._PixelPacket* %634
}
; end INTEL_FEATURE_SW_ADVANCED
