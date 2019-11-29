; Checks that IPO Prefetch pass will NOT trigger without -xCORE-AVX2 flag.
;
;

; *** Run command section ***
; RUN: opt < %s -intel-ipoprefetch -ipo-prefetch-be-lit-friendly=1 -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(intel-ipoprefetch)' -ipo-prefetch-be-lit-friendly=1 -S 2>&1 | FileCheck %s
;

; *** Check section 1 ***
; The LLVM-IR check below ensures that a call to the Prefetch.Backbone function is not inserted inside host
; _Z6searchP7state_tiiiii.
; CHECK: define internal i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5)
; CHECK-NOT: call void @Prefetch.Backbone(%struct.state_t* %0)
;

; *** Check section 2 ***
; The LLVM-IR check below ensures that a call to the Prefetch.Backbone function is not inserted inside host
; _Z7qsearchP7state_tiiii.
; CHECK:define internal i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4)
; CHECK-NOT: call void @Prefetch.Backbone(%struct.state_t* %0)
;

; *** Check section 3 ***
; The LLVM-IR check below ensures the prefetch function is not generated.
; CHECK-NOT: define internal void @Prefetch.Backbone(%struct.state_t* nocapture %0)
;

; ModuleID = '<stdin>'
source_filename = "<stdin>"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.state_t = type { i32, [64 x i32], i64, i64, i64, [13 x i64], i32, i32, [13 x i32], i32, i32, i32, i32, i32, i32, i32, i64, i64, [64 x %struct.move_x], [64 x i32], [64 x i32], [64 x %struct.anon], i64, i64, i32, [64 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i64] }
%struct.move_x = type { i32, i32, i32, i32, i64, i64 }
%struct.anon = type { i32, i32, i32, i32 }
%struct.gamestate_t = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i32], [1000 x %struct.move_x], i64, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.anon.0 = type { i32, i32, i32 }
%struct.ttentry_t = type { [4 x %struct.ttbucket_t] }
%struct.ttbucket_t = type { i32, i16, i16, i8, i8 }
@.str.4 = private unnamed_addr constant [20 x i8] c"Workload not found\0A\00", align 1
@material = internal constant [14 x i32] [i32 0, i32 85, i32 -85, i32 305, i32 -305, i32 40000, i32 -40000, i32 490, i32 -490, i32 935, i32 -935, i32 330, i32 -330, i32 0], align 16
@history_hit = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16
@history_tot = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16
@_ZL8rc_index = internal unnamed_addr constant [14 x i32] [i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3, i32 4, i32 4, i32 5, i32 5, i32 2, i32 2, i32 0], align 16
@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@stdin = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@contempt = internal global i32 0, align 4
@TTSize = internal global i32 0, align 4
@state = internal global %struct.state_t zeroinitializer, align 8
@gamestate = internal global %struct.gamestate_t zeroinitializer, align 8
@TTable = internal global %struct.ttentry_t* null, align 8
@TTAge = internal global i32 0, align 4

; Function Attrs: nofree norecurse nounwind uwtable
define internal i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nocapture %0, i32* nocapture %1, i32 %2, i32 %3, i32* nocapture %4, i32* nocapture %5, i32* nocapture %6, i32* nocapture %7, i32* nocapture %8, i32 %9) #0 {
  store i32 1, i32* %6, align 4, !tbaa !4
  %11 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 26, !intel-tbaa !8
  %12 = load i32, i32* %11, align 4, !tbaa !8
  %13 = add i32 %12, 1
  store i32 %13, i32* %11, align 4, !tbaa !8
  %14 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !19
  %15 = load i32, i32* %14, align 4, !tbaa !19
  %16 = icmp eq i32 %15, 0
  %17 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %18 = load i64, i64* %17, align 8, !tbaa !20
  %19 = zext i1 %16 to i64
  %20 = add i64 %18, %19
  %21 = trunc i64 %20 to i32
  %22 = load %struct.ttentry_t*, %struct.ttentry_t** @TTable, align 8, !tbaa !21
  %23 = load i32, i32* @TTSize, align 4, !tbaa !4
  %24 = urem i32 %21, %23
  %25 = zext i32 %24 to i64
  %26 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %22, i64 %25
  %27 = lshr i64 %20, 32
  %28 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %26, i64 0, i32 0, !intel-tbaa !23
  %29 = trunc i64 %27 to i32
  br label %32

30:                                               ; preds = %32
  %31 = icmp eq i64 %38, 4
  br i1 %31, label %132, label %32

32:                                               ; preds = %30, %10
  %33 = phi i64 [ 0, %10 ], [ %38, %30 ]
  %34 = getelementptr inbounds [4 x %struct.ttbucket_t], [4 x %struct.ttbucket_t]* %28, i64 0, i64 %33, !intel-tbaa !28
  %35 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34, i64 0, i32 0, !intel-tbaa !29
  %36 = load i32, i32* %35, align 4, !tbaa !30
  %37 = icmp eq i32 %36, %29
  %38 = add nuw nsw i64 %33, 1
  br i1 %37, label %39, label %30

39:                                               ; preds = %32
  %40 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 27, !intel-tbaa !31
  %41 = load i32, i32* %40, align 8, !tbaa !31
  %42 = add i32 %41, 1
  store i32 %42, i32* %40, align 8, !tbaa !31
  %43 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34, i64 0, i32 4
  %44 = load i8, i8* %43, align 1, !tbaa !32
  %45 = lshr i8 %44, 5
  %46 = and i8 %45, 3
  %47 = zext i8 %46 to i32
  %48 = load i32, i32* @TTAge, align 4, !tbaa !4
  %49 = icmp eq i32 %48, %47
  br i1 %49, label %56, label %50

50:                                               ; preds = %39
  %51 = trunc i32 %48 to i8
  %52 = shl i8 %51, 5
  %53 = and i8 %52, 96
  %54 = and i8 %44, -97
  %55 = or i8 %53, %54
  store i8 %55, i8* %43, align 1, !tbaa !32
  br label %56

56:                                               ; preds = %50, %39
  %57 = phi i8 [ %44, %39 ], [ %55, %50 ]
  %58 = and i8 %57, 6
  %59 = icmp eq i8 %58, 2
  br i1 %59, label %60, label %72

60:                                               ; preds = %56
  %61 = add nsw i32 %9, -16
  %62 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34, i64 0, i32 3, !intel-tbaa !33
  %63 = load i8, i8* %62, align 4, !tbaa !34
  %64 = zext i8 %63 to i32
  %65 = icmp sgt i32 %61, %64
  br i1 %65, label %72, label %66

66:                                               ; preds = %60
  %67 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34, i64 0, i32 1, !intel-tbaa !35
  %68 = load i16, i16* %67, align 4, !tbaa !36
  %69 = sext i16 %68 to i32
  %70 = icmp slt i32 %69, %3
  br i1 %70, label %71, label %72

71:                                               ; preds = %66
  store i32 0, i32* %6, align 4, !tbaa !4
  br label %72

72:                                               ; preds = %71, %66, %60, %56
  %73 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34, i64 0, i32 3, !intel-tbaa !33
  %74 = load i8, i8* %73, align 4, !tbaa !34
  %75 = zext i8 %74 to i32
  %76 = icmp slt i32 %75, %9
  br i1 %76, label %109, label %77

77:                                               ; preds = %72
  %78 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34, i64 0, i32 1, !intel-tbaa !35
  %79 = load i16, i16* %78, align 4, !tbaa !36
  %80 = sext i16 %79 to i32
  store i32 %80, i32* %1, align 4, !tbaa !4
  %81 = icmp sgt i16 %79, 31500
  br i1 %81, label %82, label %87

82:                                               ; preds = %77
  %83 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !37
  %84 = load i32, i32* %83, align 8, !tbaa !37
  %85 = add nsw i32 %80, 1
  %86 = sub i32 %85, %84
  store i32 %86, i32* %1, align 4, !tbaa !4
  br label %94

87:                                               ; preds = %77
  %88 = icmp slt i16 %79, -31500
  br i1 %88, label %89, label %94

89:                                               ; preds = %87
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !37
  %91 = load i32, i32* %90, align 8, !tbaa !37
  %92 = add nsw i32 %80, -1
  %93 = add i32 %92, %91
  store i32 %93, i32* %1, align 4, !tbaa !4
  br label %94

94:                                               ; preds = %89, %87, %82
  %95 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34, i64 0, i32 2, !intel-tbaa !38
  %96 = load i16, i16* %95, align 2, !tbaa !39
  %97 = zext i16 %96 to i32
  store i32 %97, i32* %4, align 4, !tbaa !4
  %98 = and i8 %57, 1
  %99 = zext i8 %98 to i32
  store i32 %99, i32* %5, align 4, !tbaa !4
  %100 = lshr i8 %57, 3
  %101 = and i8 %100, 1
  %102 = zext i8 %101 to i32
  store i32 %102, i32* %7, align 4, !tbaa !4
  %103 = lshr i8 %57, 4
  %104 = and i8 %103, 1
  %105 = zext i8 %104 to i32
  store i32 %105, i32* %8, align 4, !tbaa !4
  %106 = lshr i8 %57, 1
  %107 = and i8 %106, 3
  %108 = zext i8 %107 to i32
  br label %132

109:                                              ; preds = %72
  %110 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34, i64 0, i32 2, !intel-tbaa !38
  %111 = load i16, i16* %110, align 2, !tbaa !39
  %112 = zext i16 %111 to i32
  store i32 %112, i32* %4, align 4, !tbaa !4
  %113 = and i8 %57, 1
  %114 = zext i8 %113 to i32
  store i32 %114, i32* %5, align 4, !tbaa !4
  %115 = lshr i8 %57, 3
  %116 = and i8 %115, 1
  %117 = zext i8 %116 to i32
  store i32 %117, i32* %7, align 4, !tbaa !4
  %118 = lshr i8 %57, 4
  %119 = and i8 %118, 1
  %120 = zext i8 %119 to i32
  store i32 %120, i32* %8, align 4, !tbaa !4
  %121 = lshr i8 %57, 1
  %122 = and i8 %121, 3
  switch i8 %122, label %125 [
    i8 1, label %123
    i8 2, label %124
  ]

123:                                              ; preds = %109
  store i32 -1000000, i32* %1, align 4, !tbaa !4
  br label %129

124:                                              ; preds = %109
  store i32 1000000, i32* %1, align 4, !tbaa !4
  br label %129

125:                                              ; preds = %109
  %126 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34, i64 0, i32 1, !intel-tbaa !35
  %127 = load i16, i16* %126, align 4, !tbaa !36
  %128 = sext i16 %127 to i32
  store i32 %128, i32* %1, align 4, !tbaa !4
  br label %129

129:                                              ; preds = %125, %124, %123
  %130 = trunc i32 %9 to i8
  store i8 %130, i8* %73, align 4, !tbaa !34
  %131 = and i8 %57, -7
  store i8 %131, i8* %43, align 1, !tbaa !32
  br label %132

132:                                              ; preds = %129, %94, %30
  %133 = phi i32 [ %108, %94 ], [ 0, %129 ], [ 4, %30 ]
  ret i32 %133
}

; Function Attrs: nounwind uwtable
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: uwtable
define internal i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5) #2 {
  %7 = alloca [240 x i32], align 16
  %8 = alloca [240 x i32], align 16
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  %12 = alloca i32, align 4
  %13 = alloca i32, align 4
  %14 = alloca i32, align 4
  %15 = alloca i32, align 4
  %16 = alloca [240 x i32], align 16
  %17 = alloca i32, align 4
  %18 = alloca i32, align 4
  %19 = bitcast [240 x i32]* %7 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %19) #8
  %20 = bitcast [240 x i32]* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %20) #8
  %21 = bitcast i32* %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %21) #8
  %22 = bitcast i32* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %22) #8
  %23 = bitcast i32* %11 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %23) #8
  %24 = bitcast i32* %12 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %24) #8
  %25 = bitcast i32* %13 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %25) #8
  %26 = bitcast i32* %14 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %26) #8
  %27 = bitcast i32* %15 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %27) #8
  %28 = bitcast [240 x i32]* %16 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %28) #8
  %29 = icmp slt i32 %3, 1
  br i1 %29, label %34, label %30

30:                                               ; preds = %6
  %31 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !37
  %32 = load i32, i32* %31, align 8, !tbaa !37
  %33 = icmp sgt i32 %32, 59
  br i1 %33, label %34, label %36

34:                                               ; preds = %30, %6
  %35 = tail call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 0, i32 0)
  br label %1121

36:                                               ; preds = %30
  %37 = getelementptr %struct.state_t, %struct.state_t* %0, i64 0, i32 22, !intel-tbaa !40
  %38 = load i64, i64* %37, align 8, !tbaa !40
  %39 = add i64 %38, 1
  store i64 %39, i64* %37, align 8, !tbaa !40
  %40 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %39)
  %41 = icmp eq i32 %40, 0
  br i1 %41, label %42, label %1121

42:                                               ; preds = %36
  %43 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nonnull @gamestate, %struct.state_t* nonnull %0)
  %44 = icmp eq i32 %43, 0
  br i1 %44, label %45, label %49

45:                                               ; preds = %42
  %46 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15, !intel-tbaa !41
  %47 = load i32, i32* %46, align 4, !tbaa !41
  %48 = icmp sgt i32 %47, 99
  br i1 %48, label %49, label %57

49:                                               ; preds = %45, %42
  %50 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4, !tbaa !42
  %51 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !19
  %52 = load i32, i32* %51, align 4, !tbaa !19
  %53 = icmp eq i32 %50, %52
  %54 = load i32, i32* @contempt, align 4, !tbaa !4
  %55 = sub nsw i32 0, %54
  %56 = select i1 %53, i32 %54, i32 %55
  br label %1121

57:                                               ; preds = %45
  %58 = load i32, i32* %31, align 8, !tbaa !37
  %59 = add nsw i32 %58, -32000
  %60 = icmp sgt i32 %59, %1
  br i1 %60, label %61, label %63

61:                                               ; preds = %57
  %62 = icmp slt i32 %59, %2
  br i1 %62, label %63, label %1121

63:                                               ; preds = %61, %57
  %64 = phi i32 [ %59, %61 ], [ %1, %57 ]
  %65 = sub i32 31999, %58
  %66 = icmp slt i32 %65, %2
  br i1 %66, label %67, label %69

67:                                               ; preds = %63
  %68 = icmp sgt i32 %65, %64
  br i1 %68, label %69, label %1121

69:                                               ; preds = %67, %63
  %70 = phi i32 [ %65, %67 ], [ %2, %63 ]
  %71 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nonnull %0, i32* nonnull %10, i32 %64, i32 %70, i32* nonnull %13, i32* nonnull %11, i32* nonnull %12, i32* nonnull %14, i32* nonnull %15, i32 %3)
  switch i32 %71, label %85 [
    i32 3, label %72
    i32 1, label %74
    i32 2, label %77
    i32 0, label %80
    i32 4, label %84
  ]

72:                                               ; preds = %69
  %73 = load i32, i32* %10, align 4, !tbaa !4
  br label %1121

74:                                               ; preds = %69
  %75 = load i32, i32* %10, align 4, !tbaa !4
  %76 = icmp sgt i32 %75, %64
  br i1 %76, label %85, label %1121

77:                                               ; preds = %69
  %78 = load i32, i32* %10, align 4, !tbaa !4
  %79 = icmp slt i32 %78, %70
  br i1 %79, label %85, label %1121

80:                                               ; preds = %69
  %81 = load i32, i32* %10, align 4, !tbaa !4
  %82 = icmp slt i32 %81, %70
  %83 = select i1 %82, i32 %5, i32 1
  br label %85

84:                                               ; preds = %69
  store i32 65535, i32* %13, align 4, !tbaa !4
  store i32 0, i32* %11, align 4, !tbaa !4
  store i32 0, i32* %14, align 4, !tbaa !4
  store i32 0, i32* %15, align 4, !tbaa !4
  br label %85

85:                                               ; preds = %84, %80, %77, %74, %69
  %86 = phi i32 [ %5, %69 ], [ %5, %84 ], [ 0, %74 ], [ 1, %77 ], [ %83, %80 ]
  %87 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25, !intel-tbaa !46
  %88 = load i32, i32* %31, align 8, !tbaa !37
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %89, !intel-tbaa !47
  %91 = load i32, i32* %90, align 4, !tbaa !48
  %92 = call i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nonnull %0)
  %93 = icmp ne i32 %91, 0
  %94 = xor i1 %93, true
  %95 = add nsw i32 %64, 1
  %96 = icmp eq i32 %70, %95
  %97 = and i1 %96, %94
  br i1 %97, label %98, label %122

98:                                               ; preds = %85
  %99 = icmp slt i32 %3, 5
  br i1 %99, label %100, label %112

100:                                              ; preds = %98
  %101 = add nsw i32 %92, -75
  %102 = icmp slt i32 %101, %70
  br i1 %102, label %108, label %103

103:                                              ; preds = %100
  %104 = load i32, i32* %13, align 4, !tbaa !4
  %105 = load i32, i32* %11, align 4, !tbaa !4
  %106 = load i32, i32* %14, align 4, !tbaa !4
  %107 = load i32, i32* %15, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %101, i32 %64, i32 %70, i32 %104, i32 %105, i32 %106, i32 %107, i32 %3)
  br label %1121

108:                                              ; preds = %100
  %109 = icmp slt i32 %92, %70
  br i1 %109, label %110, label %122

110:                                              ; preds = %108
  %111 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %64, i32 %70, i32 0, i32 0)
  br label %1121

112:                                              ; preds = %98
  %113 = icmp slt i32 %3, 9
  br i1 %113, label %114, label %122

114:                                              ; preds = %112
  %115 = add nsw i32 %92, -125
  %116 = icmp slt i32 %115, %70
  br i1 %116, label %122, label %117

117:                                              ; preds = %114
  %118 = load i32, i32* %13, align 4, !tbaa !4
  %119 = load i32, i32* %11, align 4, !tbaa !4
  %120 = load i32, i32* %14, align 4, !tbaa !4
  %121 = load i32, i32* %15, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %115, i32 %64, i32 %70, i32 %118, i32 %119, i32 %120, i32 %121, i32 %3)
  br label %1121

122:                                              ; preds = %114, %112, %108, %85
  %123 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8, !intel-tbaa !49
  %124 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 9, !intel-tbaa !50
  %125 = load i32, i32* %124, align 4, !tbaa !51
  %126 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 7, !intel-tbaa !50
  %127 = load i32, i32* %126, align 4, !tbaa !51
  %128 = add nsw i32 %127, %125
  %129 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 11, !intel-tbaa !50
  %130 = load i32, i32* %129, align 4, !tbaa !51
  %131 = add nsw i32 %128, %130
  %132 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 3, !intel-tbaa !50
  %133 = load i32, i32* %132, align 4, !tbaa !51
  %134 = add nsw i32 %131, %133
  %135 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 10, !intel-tbaa !50
  %136 = load i32, i32* %135, align 8, !tbaa !51
  %137 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 8, !intel-tbaa !50
  %138 = load i32, i32* %137, align 8, !tbaa !51
  %139 = add nsw i32 %138, %136
  %140 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 12, !intel-tbaa !50
  %141 = load i32, i32* %140, align 8, !tbaa !51
  %142 = add nsw i32 %139, %141
  %143 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 4, !intel-tbaa !50
  %144 = load i32, i32* %143, align 8, !tbaa !51
  %145 = add nsw i32 %142, %144
  store i32 0, i32* %11, align 4, !tbaa !4
  %146 = icmp eq i32 %4, 0
  br i1 %146, label %147, label %231

147:                                              ; preds = %122
  %148 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !19
  %149 = load i32, i32* %148, align 4, !tbaa !19
  %150 = icmp eq i32 %149, 0
  %151 = select i1 %150, i32 %145, i32 %134
  %152 = icmp eq i32 %151, 0
  %153 = or i1 %93, %152
  %154 = xor i1 %153, true
  %155 = load i32, i32* %12, align 4
  %156 = icmp ne i32 %155, 0
  %157 = and i1 %156, %154
  %158 = icmp sgt i32 %3, 4
  %159 = and i1 %158, %96
  %160 = and i1 %159, %157
  br i1 %160, label %161, label %231

161:                                              ; preds = %147
  %162 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !tbaa !52
  %163 = icmp eq i32 %162, 2
  br i1 %163, label %164, label %183

164:                                              ; preds = %161
  %165 = add nsw i32 %3, -24
  %166 = icmp slt i32 %165, 1
  %167 = add nsw i32 %70, -1
  br i1 %166, label %168, label %170

168:                                              ; preds = %164
  %169 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %167, i32 %70, i32 0, i32 0)
  br label %172

170:                                              ; preds = %164
  %171 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %167, i32 %70, i32 %165, i32 1, i32 %86)
  br label %172

172:                                              ; preds = %170, %168
  %173 = phi i32 [ %169, %168 ], [ %171, %170 ]
  %174 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !53
  %175 = icmp eq i32 %174, 0
  br i1 %175, label %176, label %1121

176:                                              ; preds = %172
  %177 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !tbaa !52
  %178 = icmp eq i32 %177, 2
  %179 = icmp slt i32 %173, %70
  %180 = and i1 %179, %178
  br i1 %180, label %248, label %181

181:                                              ; preds = %176
  %182 = load i32, i32* %148, align 4, !tbaa !19
  br label %183

183:                                              ; preds = %181, %161
  %184 = phi i32 [ %182, %181 ], [ %149, %161 ]
  %185 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 10, !intel-tbaa !54
  %186 = load i32, i32* %185, align 8, !tbaa !54
  store i32 0, i32* %185, align 8, !tbaa !54
  %187 = xor i32 %184, 1
  store i32 %187, i32* %148, align 4, !tbaa !19
  %188 = load i32, i32* %31, align 8, !tbaa !37
  %189 = add nsw i32 %188, 1
  store i32 %189, i32* %31, align 8, !tbaa !37
  %190 = load i32, i32* %46, align 4, !tbaa !41
  %191 = add nsw i32 %190, 1
  store i32 %191, i32* %46, align 4, !tbaa !41
  %192 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19, !intel-tbaa !55
  %193 = sext i32 %188 to i64
  %194 = getelementptr inbounds [64 x i32], [64 x i32]* %192, i64 0, i64 %193, !intel-tbaa !47
  store i32 0, i32* %194, align 4, !tbaa !56
  %195 = sext i32 %189 to i64
  %196 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %195, !intel-tbaa !47
  store i32 0, i32* %196, align 4, !tbaa !48
  %197 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 20, !intel-tbaa !57
  %198 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %193, !intel-tbaa !47
  %199 = load i32, i32* %198, align 4, !tbaa !58
  %200 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %195, !intel-tbaa !47
  store i32 %199, i32* %200, align 4, !tbaa !58
  %201 = add nsw i32 %3, -16
  %202 = icmp slt i32 %201, 1
  %203 = sub nsw i32 0, %70
  %204 = sub i32 1, %70
  br i1 %202, label %205, label %207

205:                                              ; preds = %183
  %206 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %203, i32 %204, i32 0, i32 0)
  br label %211

207:                                              ; preds = %183
  %208 = icmp eq i32 %86, 0
  %209 = zext i1 %208 to i32
  %210 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %203, i32 %204, i32 %201, i32 1, i32 %209)
  br label %211

211:                                              ; preds = %207, %205
  %212 = phi i32 [ %206, %205 ], [ %210, %207 ]
  %213 = sub nsw i32 0, %212
  %214 = load i32, i32* %46, align 4, !tbaa !41
  %215 = add nsw i32 %214, -1
  store i32 %215, i32* %46, align 4, !tbaa !41
  %216 = load i32, i32* %31, align 8, !tbaa !37
  %217 = add nsw i32 %216, -1
  store i32 %217, i32* %31, align 8, !tbaa !37
  %218 = load i32, i32* %148, align 4, !tbaa !19
  %219 = xor i32 %218, 1
  store i32 %219, i32* %148, align 4, !tbaa !19
  store i32 %186, i32* %185, align 8, !tbaa !54
  %220 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !53
  %221 = icmp eq i32 %220, 0
  br i1 %221, label %222, label %1121

222:                                              ; preds = %211
  %223 = icmp sgt i32 %70, %213
  br i1 %223, label %228, label %224

224:                                              ; preds = %222
  %225 = load i32, i32* %13, align 4, !tbaa !4
  %226 = load i32, i32* %11, align 4, !tbaa !4
  %227 = load i32, i32* %15, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %213, i32 %64, i32 %70, i32 %225, i32 %226, i32 0, i32 %227, i32 %3)
  br label %1121

228:                                              ; preds = %222
  %229 = icmp sgt i32 %212, 31400
  br i1 %229, label %230, label %248

230:                                              ; preds = %228
  store i32 1, i32* %11, align 4, !tbaa !4
  br label %248

231:                                              ; preds = %147, %122
  %232 = icmp slt i32 %3, 13
  %233 = and i1 %232, %96
  %234 = add nsw i32 %70, -300
  %235 = icmp slt i32 %92, %234
  %236 = and i1 %233, %235
  br i1 %236, label %237, label %248

237:                                              ; preds = %231
  %238 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %64, i32 %70, i32 0, i32 0)
  %239 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !53
  %240 = icmp eq i32 %239, 0
  br i1 %240, label %241, label %1121

241:                                              ; preds = %237
  %242 = icmp sgt i32 %238, %64
  br i1 %242, label %248, label %243

243:                                              ; preds = %241
  %244 = load i32, i32* %13, align 4, !tbaa !4
  %245 = load i32, i32* %11, align 4, !tbaa !4
  %246 = load i32, i32* %14, align 4, !tbaa !4
  %247 = load i32, i32* %15, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %64, i32 %64, i32 %70, i32 %244, i32 %245, i32 %246, i32 %247, i32 %3)
  br label %1121

248:                                              ; preds = %241, %231, %230, %228, %176
  %249 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 0
  br i1 %93, label %250, label %255

250:                                              ; preds = %248
  %251 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %249, i32 %91)
  %252 = icmp eq i32 %251, 0
  br i1 %252, label %280, label %253

253:                                              ; preds = %250
  store i32 0, i32* %9, align 4, !tbaa !4
  %254 = icmp sgt i32 %251, 0
  br i1 %254, label %257, label %280

255:                                              ; preds = %248
  %256 = call i32 @_Z3genP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %249)
  br label %280

257:                                              ; preds = %257, %253
  %258 = phi i32 [ %270, %257 ], [ 0, %253 ]
  %259 = phi i32 [ %276, %257 ], [ 0, %253 ]
  %260 = sext i32 %259 to i64
  %261 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %260, !intel-tbaa !59
  %262 = load i32, i32* %261, align 4, !tbaa !59
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %262)
  %263 = load i32, i32* %9, align 4, !tbaa !4
  %264 = sext i32 %263 to i64
  %265 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %264, !intel-tbaa !59
  %266 = load i32, i32* %265, align 4, !tbaa !59
  %267 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %266)
  %268 = icmp ne i32 %267, 0
  %269 = zext i1 %268 to i32
  %270 = add nuw nsw i32 %258, %269
  %271 = load i32, i32* %9, align 4, !tbaa !4
  %272 = sext i32 %271 to i64
  %273 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %272, !intel-tbaa !59
  %274 = load i32, i32* %273, align 4, !tbaa !59
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %274)
  %275 = load i32, i32* %9, align 4, !tbaa !4
  %276 = add nsw i32 %275, 1
  store i32 %276, i32* %9, align 4, !tbaa !4
  %277 = icmp slt i32 %276, %251
  %278 = icmp ult i32 %270, 2
  %279 = and i1 %277, %278
  br i1 %279, label %257, label %280

280:                                              ; preds = %257, %255, %253, %250
  %281 = phi i32 [ 0, %250 ], [ %256, %255 ], [ %251, %253 ], [ %251, %257 ]
  %282 = phi i32 [ 0, %250 ], [ %256, %255 ], [ 0, %253 ], [ %270, %257 ]
  %283 = getelementptr inbounds [240 x i32], [240 x i32]* %8, i64 0, i64 0
  %284 = load i32, i32* %13, align 4, !tbaa !4
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %249, i32* nonnull %283, i32 %281, i32 %284)
  %285 = icmp sgt i32 %3, 19
  br i1 %285, label %286, label %336

286:                                              ; preds = %280
  %287 = icmp ne i32 %70, %95
  %288 = load i32, i32* %13, align 4
  %289 = icmp eq i32 %288, 65535
  %290 = and i1 %287, %289
  br i1 %290, label %291, label %336

291:                                              ; preds = %286
  store i32 0, i32* %9, align 4, !tbaa !4
  %292 = icmp sgt i32 %281, 0
  br i1 %292, label %293, label %324

293:                                              ; preds = %291
  %294 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %295 = sext i32 %281 to i64
  br label %296

296:                                              ; preds = %318, %293
  %297 = phi i64 [ 0, %293 ], [ %320, %318 ]
  %298 = phi i32 [ 0, %293 ], [ %319, %318 ]
  %299 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %297, !intel-tbaa !59
  %300 = load i32, i32* %299, align 4, !tbaa !59
  %301 = lshr i32 %300, 19
  %302 = and i32 %301, 15
  %303 = icmp eq i32 %302, 13
  br i1 %303, label %318, label %304

304:                                              ; preds = %296
  %305 = lshr i32 %300, 6
  %306 = and i32 %305, 63
  %307 = zext i32 %306 to i64
  %308 = getelementptr inbounds [64 x i32], [64 x i32]* %294, i64 0, i64 %307, !intel-tbaa !47
  %309 = load i32, i32* %308, align 4, !tbaa !61
  %310 = sext i32 %309 to i64
  %311 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %310, !intel-tbaa !62
  %312 = load i32, i32* %311, align 4, !tbaa !62
  %313 = icmp slt i32 %312, 0
  %314 = sub nsw i32 0, %312
  %315 = select i1 %313, i32 %314, i32 %312
  %316 = icmp sgt i32 %302, %315
  %317 = select i1 %316, i32 1, i32 %298
  br label %318

318:                                              ; preds = %304, %296
  %319 = phi i32 [ %298, %296 ], [ %317, %304 ]
  %320 = add nuw nsw i64 %297, 1
  %321 = icmp eq i64 %320, %295
  br i1 %321, label %322, label %296

322:                                              ; preds = %318
  store i32 %281, i32* %9, align 4, !tbaa !4
  %323 = icmp eq i32 %319, 0
  br i1 %323, label %324, label %336

324:                                              ; preds = %322, %291
  %325 = bitcast i32* %17 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %325) #8
  %326 = bitcast i32* %18 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %326) #8
  %327 = ashr i32 %3, 1
  %328 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %327, i32 0, i32 %86)
  %329 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* %0, i32* nonnull %17, i32 0, i32 0, i32* nonnull %18, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32 0)
  %330 = icmp eq i32 %329, 4
  br i1 %330, label %333, label %331

331:                                              ; preds = %324
  %332 = load i32, i32* %18, align 4, !tbaa !4
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %249, i32* nonnull %283, i32 %281, i32 %332)
  br label %335

333:                                              ; preds = %324
  %334 = load i32, i32* %13, align 4, !tbaa !4
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %249, i32* nonnull %283, i32 %281, i32 %334)
  br label %335

335:                                              ; preds = %333, %331
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %326) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %325) #8
  br label %336

336:                                              ; preds = %335, %322, %286, %280
  %337 = load i32, i32* %11, align 4
  %338 = or i32 %337, %91
  %339 = icmp eq i32 %338, 0
  %340 = icmp sgt i32 %3, 15
  %341 = and i1 %340, %339
  %342 = icmp sgt i32 %282, 8
  %343 = and i1 %342, %341
  br i1 %343, label %344, label %508

344:                                              ; preds = %336
  %345 = load i32, i32* %31, align 8, !tbaa !37
  %346 = add nsw i32 %345, -1
  %347 = sext i32 %346 to i64
  %348 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %347, !intel-tbaa !47
  %349 = load i32, i32* %348, align 4, !tbaa !48
  %350 = icmp eq i32 %349, 0
  br i1 %350, label %351, label %508

351:                                              ; preds = %344
  %352 = icmp slt i32 %345, 3
  br i1 %352, label %367, label %353

353:                                              ; preds = %351
  %354 = add nsw i32 %345, -2
  %355 = sext i32 %354 to i64
  %356 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %355, !intel-tbaa !47
  %357 = load i32, i32* %356, align 4, !tbaa !48
  %358 = icmp eq i32 %357, 0
  br i1 %358, label %359, label %508

359:                                              ; preds = %353
  %360 = icmp slt i32 %345, 4
  br i1 %360, label %367, label %361

361:                                              ; preds = %359
  %362 = add nsw i32 %345, -3
  %363 = sext i32 %362 to i64
  %364 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %363, !intel-tbaa !47
  %365 = load i32, i32* %364, align 4, !tbaa !48
  %366 = icmp eq i32 %365, 0
  br i1 %366, label %367, label %508

367:                                              ; preds = %361, %359, %351
  store i32 -1, i32* %9, align 4, !tbaa !4
  %368 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %369 = icmp eq i32 %368, 0
  br i1 %369, label %508, label %370

370:                                              ; preds = %367
  %371 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %372 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %373 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %374 = add nsw i32 %3, -16
  %375 = sub nsw i32 0, %70
  %376 = sub i32 50, %64
  %377 = icmp sgt i32 %374, 0
  %378 = icmp slt i32 %374, 1
  %379 = sub i32 1, %70
  %380 = icmp eq i32 %86, 0
  %381 = zext i1 %380 to i32
  br i1 %378, label %382, label %443

382:                                              ; preds = %437, %370
  %383 = phi i32 [ %438, %437 ], [ 0, %370 ]
  %384 = phi i32 [ %386, %437 ], [ 0, %370 ]
  %385 = phi i32 [ %423, %437 ], [ -32000, %370 ]
  %386 = add nuw nsw i32 %384, 1
  %387 = load i32, i32* %9, align 4, !tbaa !4
  %388 = sext i32 %387 to i64
  %389 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %388, !intel-tbaa !59
  %390 = load i32, i32* %389, align 4, !tbaa !59
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %390)
  %391 = load i32, i32* %9, align 4, !tbaa !4
  %392 = sext i32 %391 to i64
  %393 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %392, !intel-tbaa !59
  %394 = load i32, i32* %393, align 4, !tbaa !59
  %395 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %394)
  %396 = icmp eq i32 %395, 0
  br i1 %396, label %421, label %397

397:                                              ; preds = %382
  %398 = load i64, i64* %371, align 8, !tbaa !20
  %399 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !64
  %400 = load i32, i32* %31, align 8, !tbaa !37
  %401 = add i32 %400, -1
  %402 = add i32 %401, %399
  %403 = sext i32 %402 to i64
  %404 = getelementptr inbounds [1000 x i64], [1000 x i64]* %372, i64 0, i64 %403, !intel-tbaa !65
  store i64 %398, i64* %404, align 8, !tbaa !66
  %405 = load i32, i32* %9, align 4, !tbaa !4
  %406 = sext i32 %405 to i64
  %407 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %406, !intel-tbaa !59
  %408 = load i32, i32* %407, align 4, !tbaa !59
  %409 = sext i32 %401 to i64
  %410 = getelementptr inbounds [64 x i32], [64 x i32]* %373, i64 0, i64 %409, !intel-tbaa !47
  store i32 %408, i32* %410, align 4, !tbaa !56
  %411 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %412 = load i32, i32* %31, align 8, !tbaa !37
  %413 = sext i32 %412 to i64
  %414 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %413, !intel-tbaa !47
  store i32 %411, i32* %414, align 4, !tbaa !48
  %415 = icmp ne i32 %411, 0
  %416 = or i1 %377, %415
  %417 = zext i1 %416 to i32
  %418 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %375, i32 %376, i32 %417)
  %419 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %375, i32 %379, i32 0, i32 0)
  %420 = sub nsw i32 0, %419
  br label %421

421:                                              ; preds = %397, %382
  %422 = phi i32 [ 1, %397 ], [ 0, %382 ]
  %423 = phi i32 [ %420, %397 ], [ %385, %382 ]
  %424 = load i32, i32* %9, align 4, !tbaa !4
  %425 = sext i32 %424 to i64
  %426 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %425, !intel-tbaa !59
  %427 = load i32, i32* %426, align 4, !tbaa !59
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %427)
  %428 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !53
  %429 = icmp eq i32 %428, 0
  br i1 %429, label %430, label %508

430:                                              ; preds = %421
  %431 = icmp sge i32 %423, %70
  %432 = icmp ne i32 %422, 0
  %433 = and i1 %432, %431
  br i1 %433, label %434, label %437

434:                                              ; preds = %430
  %435 = add nsw i32 %383, 1
  %436 = icmp sgt i32 %383, 0
  br i1 %436, label %498, label %437

437:                                              ; preds = %434, %430
  %438 = phi i32 [ %435, %434 ], [ %383, %430 ]
  %439 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %440 = icmp ne i32 %439, 0
  %441 = icmp ult i32 %386, 3
  %442 = and i1 %440, %441
  br i1 %442, label %382, label %508

443:                                              ; preds = %502, %370
  %444 = phi i32 [ %503, %502 ], [ 0, %370 ]
  %445 = phi i32 [ %447, %502 ], [ 0, %370 ]
  %446 = phi i32 [ %484, %502 ], [ -32000, %370 ]
  %447 = add nuw nsw i32 %445, 1
  %448 = load i32, i32* %9, align 4, !tbaa !4
  %449 = sext i32 %448 to i64
  %450 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %449, !intel-tbaa !59
  %451 = load i32, i32* %450, align 4, !tbaa !59
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %451)
  %452 = load i32, i32* %9, align 4, !tbaa !4
  %453 = sext i32 %452 to i64
  %454 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %453, !intel-tbaa !59
  %455 = load i32, i32* %454, align 4, !tbaa !59
  %456 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %455)
  %457 = icmp eq i32 %456, 0
  br i1 %457, label %482, label %458

458:                                              ; preds = %443
  %459 = load i64, i64* %371, align 8, !tbaa !20
  %460 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !64
  %461 = load i32, i32* %31, align 8, !tbaa !37
  %462 = add i32 %461, -1
  %463 = add i32 %462, %460
  %464 = sext i32 %463 to i64
  %465 = getelementptr inbounds [1000 x i64], [1000 x i64]* %372, i64 0, i64 %464, !intel-tbaa !65
  store i64 %459, i64* %465, align 8, !tbaa !66
  %466 = load i32, i32* %9, align 4, !tbaa !4
  %467 = sext i32 %466 to i64
  %468 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %467, !intel-tbaa !59
  %469 = load i32, i32* %468, align 4, !tbaa !59
  %470 = sext i32 %462 to i64
  %471 = getelementptr inbounds [64 x i32], [64 x i32]* %373, i64 0, i64 %470, !intel-tbaa !47
  store i32 %469, i32* %471, align 4, !tbaa !56
  %472 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %473 = load i32, i32* %31, align 8, !tbaa !37
  %474 = sext i32 %473 to i64
  %475 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %474, !intel-tbaa !47
  store i32 %472, i32* %475, align 4, !tbaa !48
  %476 = icmp ne i32 %472, 0
  %477 = or i1 %377, %476
  %478 = zext i1 %477 to i32
  %479 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %375, i32 %376, i32 %478)
  %480 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %375, i32 %379, i32 %374, i32 0, i32 %381)
  %481 = sub nsw i32 0, %480
  br label %482

482:                                              ; preds = %458, %443
  %483 = phi i32 [ 1, %458 ], [ 0, %443 ]
  %484 = phi i32 [ %481, %458 ], [ %446, %443 ]
  %485 = load i32, i32* %9, align 4, !tbaa !4
  %486 = sext i32 %485 to i64
  %487 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %486, !intel-tbaa !59
  %488 = load i32, i32* %487, align 4, !tbaa !59
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %488)
  %489 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !53
  %490 = icmp eq i32 %489, 0
  br i1 %490, label %491, label %508

491:                                              ; preds = %482
  %492 = icmp sge i32 %484, %70
  %493 = icmp ne i32 %483, 0
  %494 = and i1 %493, %492
  br i1 %494, label %495, label %502

495:                                              ; preds = %491
  %496 = add nsw i32 %444, 1
  %497 = icmp sgt i32 %444, 0
  br i1 %497, label %498, label %502

498:                                              ; preds = %495, %434
  %499 = load i32, i32* %13, align 4, !tbaa !4
  %500 = load i32, i32* %11, align 4, !tbaa !4
  %501 = load i32, i32* %15, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %70, i32 %64, i32 %70, i32 %499, i32 %500, i32 0, i32 %501, i32 %3)
  br label %1121

502:                                              ; preds = %495, %491
  %503 = phi i32 [ %496, %495 ], [ %444, %491 ]
  %504 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %505 = icmp ne i32 %504, 0
  %506 = icmp ult i32 %447, 3
  %507 = and i1 %505, %506
  br i1 %507, label %443, label %508

508:                                              ; preds = %502, %482, %437, %421, %367, %361, %353, %344, %336
  %509 = phi i32 [ -32000, %344 ], [ -32000, %361 ], [ -32000, %353 ], [ -32000, %336 ], [ -32000, %367 ], [ %423, %421 ], [ %423, %437 ], [ %484, %482 ], [ %484, %502 ]
  %510 = load i32, i32* %14, align 4, !tbaa !4
  %511 = load i32, i32* %15, align 4
  %512 = or i32 %511, %510
  %513 = load i32, i32* %11, align 4
  %514 = or i32 %512, %513
  %515 = icmp eq i32 %514, 0
  %516 = and i1 %285, %515
  %517 = icmp sgt i32 %282, 1
  %518 = and i1 %517, %516
  %519 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4
  %520 = icmp ne i32 %519, 2
  %521 = and i1 %520, %518
  br i1 %521, label %522, label %612

522:                                              ; preds = %508
  %523 = add nsw i32 %3, -24
  %524 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %523, i32 0, i32 %86)
  %525 = icmp sgt i32 %524, %64
  br i1 %525, label %526, label %612

526:                                              ; preds = %522
  store i32 -1, i32* %9, align 4, !tbaa !4
  %527 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %528 = icmp ne i32 %527, 0
  %529 = load i32, i32* %14, align 4
  %530 = icmp slt i32 %529, 2
  %531 = and i1 %528, %530
  br i1 %531, label %532, label %612

532:                                              ; preds = %526
  %533 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %534 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %535 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %536 = add nsw i32 %3, -16
  %537 = sub nsw i32 0, %70
  %538 = sub i32 50, %64
  %539 = icmp sgt i32 %536, 0
  %540 = sub nsw i32 0, %64
  %541 = xor i32 %64, -1
  %542 = icmp eq i32 %86, 0
  %543 = zext i1 %542 to i32
  %544 = sub i32 49, %64
  %545 = add nsw i32 %64, -50
  br label %546

546:                                              ; preds = %597, %532
  %547 = phi i32 [ 0, %532 ], [ %600, %597 ]
  %548 = phi i32 [ %509, %532 ], [ %599, %597 ]
  %549 = phi i32 [ 1, %532 ], [ %598, %597 ]
  %550 = load i32, i32* %9, align 4, !tbaa !4
  %551 = sext i32 %550 to i64
  %552 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %551, !intel-tbaa !59
  %553 = load i32, i32* %552, align 4, !tbaa !59
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %553)
  %554 = load i32, i32* %9, align 4, !tbaa !4
  %555 = sext i32 %554 to i64
  %556 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %555, !intel-tbaa !59
  %557 = load i32, i32* %556, align 4, !tbaa !59
  %558 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %557)
  %559 = icmp eq i32 %558, 0
  br i1 %559, label %597, label %560

560:                                              ; preds = %546
  %561 = load i64, i64* %533, align 8, !tbaa !20
  %562 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !64
  %563 = load i32, i32* %31, align 8, !tbaa !37
  %564 = add i32 %563, -1
  %565 = add i32 %564, %562
  %566 = sext i32 %565 to i64
  %567 = getelementptr inbounds [1000 x i64], [1000 x i64]* %534, i64 0, i64 %566, !intel-tbaa !65
  store i64 %561, i64* %567, align 8, !tbaa !66
  %568 = load i32, i32* %9, align 4, !tbaa !4
  %569 = sext i32 %568 to i64
  %570 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %569, !intel-tbaa !59
  %571 = load i32, i32* %570, align 4, !tbaa !59
  %572 = sext i32 %564 to i64
  %573 = getelementptr inbounds [64 x i32], [64 x i32]* %535, i64 0, i64 %572, !intel-tbaa !47
  store i32 %571, i32* %573, align 4, !tbaa !56
  %574 = add nsw i32 %547, 1
  %575 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %576 = load i32, i32* %31, align 8, !tbaa !37
  %577 = sext i32 %576 to i64
  %578 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %577, !intel-tbaa !47
  store i32 %575, i32* %578, align 4, !tbaa !48
  %579 = icmp ne i32 %575, 0
  %580 = or i1 %539, %579
  %581 = zext i1 %580 to i32
  %582 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %537, i32 %538, i32 %581)
  %583 = icmp eq i32 %549, 0
  br i1 %583, label %591, label %584

584:                                              ; preds = %560
  %585 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %541, i32 %540, i32 %536, i32 0, i32 %543)
  %586 = sub nsw i32 0, %585
  %587 = icmp slt i32 %64, %586
  br i1 %587, label %588, label %589

588:                                              ; preds = %584
  store i32 1, i32* %14, align 4, !tbaa !4
  br label %597

589:                                              ; preds = %584
  store i32 0, i32* %14, align 4, !tbaa !4
  %590 = add nsw i32 %547, 11
  br label %597

591:                                              ; preds = %560
  %592 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %544, i32 %538, i32 %536, i32 0, i32 0)
  %593 = sub nsw i32 0, %592
  %594 = icmp slt i32 %545, %593
  br i1 %594, label %595, label %597

595:                                              ; preds = %591
  store i32 0, i32* %14, align 4, !tbaa !4
  %596 = add nsw i32 %547, 11
  br label %597

597:                                              ; preds = %595, %591, %589, %588, %546
  %598 = phi i32 [ %549, %546 ], [ 0, %591 ], [ 0, %595 ], [ 0, %588 ], [ 0, %589 ]
  %599 = phi i32 [ %548, %546 ], [ %593, %591 ], [ %593, %595 ], [ %586, %588 ], [ %586, %589 ]
  %600 = phi i32 [ %547, %546 ], [ %574, %591 ], [ %596, %595 ], [ %574, %588 ], [ %590, %589 ]
  %601 = load i32, i32* %9, align 4, !tbaa !4
  %602 = sext i32 %601 to i64
  %603 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %602, !intel-tbaa !59
  %604 = load i32, i32* %603, align 4, !tbaa !59
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %604)
  %605 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %606 = icmp ne i32 %605, 0
  %607 = load i32, i32* %14, align 4
  %608 = icmp slt i32 %607, 2
  %609 = and i1 %606, %608
  %610 = icmp slt i32 %600, 3
  %611 = and i1 %610, %609
  br i1 %611, label %546, label %612

612:                                              ; preds = %597, %526, %522, %508
  %613 = phi i32 [ %509, %508 ], [ %509, %522 ], [ %509, %526 ], [ %599, %597 ]
  br i1 %96, label %619, label %614

614:                                              ; preds = %612
  %615 = load i32, i32* %31, align 8, !tbaa !37
  %616 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4, !tbaa !67
  %617 = shl nsw i32 %616, 1
  %618 = icmp sle i32 %615, %617
  br label %619

619:                                              ; preds = %614, %612
  %620 = phi i1 [ false, %612 ], [ %618, %614 ]
  store i32 -1, i32* %9, align 4, !tbaa !4
  %621 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %622 = icmp eq i32 %621, 0
  br i1 %622, label %1089, label %623

623:                                              ; preds = %619
  %624 = icmp eq i32 %282, 1
  %625 = and i1 %93, %624
  %626 = select i1 %625, i32 4, i32 0
  %627 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %628 = or i32 %626, 2
  %629 = select i1 %620, i32 %628, i32 %626
  %630 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %631 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %632 = select i1 %620, i32 3, i32 1
  %633 = add nsw i32 %145, %134
  %634 = icmp eq i32 %633, 1
  %635 = sdiv i32 %3, 4
  %636 = add nsw i32 %635, 1
  %637 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 0
  %638 = icmp sgt i32 %3, 24
  %639 = icmp slt i32 %3, 9
  %640 = icmp slt i32 %3, 13
  %641 = add nsw i32 %92, 100
  %642 = add nsw i32 %92, 300
  %643 = add nsw i32 %92, 75
  %644 = add nsw i32 %92, 200
  %645 = select i1 %620, i32 4, i32 2
  %646 = add i32 %3, -4
  %647 = sub nsw i32 0, %70
  %648 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %649 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %650 = icmp sgt i32 %3, 4
  %651 = icmp eq i32 %86, 0
  %652 = zext i1 %651 to i32
  br label %653

653:                                              ; preds = %1080, %623
  %654 = phi i32 [ %64, %623 ], [ %1087, %1080 ]
  %655 = phi i32 [ %613, %623 ], [ %1086, %1080 ]
  %656 = phi i32 [ 1, %623 ], [ %1085, %1080 ]
  %657 = phi i32 [ -32000, %623 ], [ %1084, %1080 ]
  %658 = phi i32 [ 1, %623 ], [ %1083, %1080 ]
  %659 = phi i32 [ 1, %623 ], [ %1082, %1080 ]
  %660 = icmp ne i32 %658, 0
  %661 = icmp sgt i32 %659, %636
  %662 = add nsw i32 %654, 1
  %663 = icmp eq i32 %70, %662
  br label %664

664:                                              ; preds = %793, %653
  %665 = phi i32 [ %656, %653 ], [ 0, %793 ]
  %666 = load i32, i32* %31, align 8, !tbaa !37
  %667 = icmp slt i32 %666, 60
  %668 = load i32, i32* %9, align 4, !tbaa !4
  %669 = sext i32 %668 to i64
  br i1 %667, label %670, label %750

670:                                              ; preds = %664
  %671 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %669, !intel-tbaa !59
  %672 = load i32, i32* %671, align 4, !tbaa !59
  %673 = lshr i32 %672, 6
  %674 = and i32 %673, 63
  %675 = zext i32 %674 to i64
  %676 = getelementptr inbounds [64 x i32], [64 x i32]* %627, i64 0, i64 %675, !intel-tbaa !47
  %677 = load i32, i32* %676, align 4, !tbaa !61
  %678 = add nsw i32 %677, 1
  %679 = and i32 %678, -2
  %680 = icmp eq i32 %679, 2
  br i1 %680, label %681, label %688

681:                                              ; preds = %670
  %682 = lshr i32 %672, 3
  %683 = and i32 %682, 7
  switch i32 %683, label %684 [
    i32 1, label %687
    i32 6, label %687
  ]

684:                                              ; preds = %681
  %685 = and i32 %672, 61440
  %686 = icmp eq i32 %685, 0
  br i1 %686, label %688, label %687

687:                                              ; preds = %684, %681, %681
  br label %688

688:                                              ; preds = %687, %684, %670
  %689 = phi i32 [ %626, %684 ], [ %626, %670 ], [ %629, %687 ]
  %690 = lshr i32 %672, 19
  %691 = and i32 %690, 15
  %692 = icmp eq i32 %691, 13
  br i1 %692, label %723, label %693

693:                                              ; preds = %688
  %694 = add nsw i32 %666, -1
  %695 = sext i32 %694 to i64
  %696 = getelementptr inbounds [64 x i32], [64 x i32]* %630, i64 0, i64 %695, !intel-tbaa !47
  %697 = load i32, i32* %696, align 4, !tbaa !56
  %698 = lshr i32 %697, 19
  %699 = and i32 %698, 15
  %700 = icmp eq i32 %699, 13
  br i1 %700, label %723, label %701

701:                                              ; preds = %693
  %702 = zext i32 %691 to i64
  %703 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %702, !intel-tbaa !62
  %704 = load i32, i32* %703, align 4, !tbaa !62
  %705 = zext i32 %699 to i64
  %706 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %705, !intel-tbaa !62
  %707 = load i32, i32* %706, align 4, !tbaa !62
  %708 = icmp eq i32 %704, %707
  br i1 %708, label %709, label %723

709:                                              ; preds = %701
  %710 = and i32 %672, 63
  %711 = and i32 %697, 63
  %712 = icmp eq i32 %710, %711
  br i1 %712, label %713, label %723

713:                                              ; preds = %709
  %714 = load i32, i32* %631, align 4, !tbaa !19
  %715 = icmp eq i32 %714, 0
  %716 = zext i1 %715 to i32
  %717 = lshr i32 %672, 12
  %718 = and i32 %717, 15
  %719 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %716, i32 %674, i32 %710, i32 %718)
  %720 = icmp sgt i32 %719, 0
  %721 = select i1 %720, i32 %632, i32 0
  %722 = add nsw i32 %689, %721
  br label %723

723:                                              ; preds = %713, %709, %701, %693, %688
  %724 = phi i32 [ %689, %709 ], [ %689, %701 ], [ %689, %693 ], [ %689, %688 ], [ %722, %713 ]
  %725 = load i32, i32* %14, align 4, !tbaa !4
  %726 = icmp eq i32 %725, 1
  %727 = icmp ne i32 %724, 0
  %728 = and i1 %727, %726
  %729 = and i1 %660, %728
  br i1 %729, label %730, label %731

730:                                              ; preds = %723
  store i32 1, i32* %15, align 4, !tbaa !4
  br label %736

731:                                              ; preds = %723
  %732 = icmp eq i32 %724, 0
  %733 = and i1 %732, %726
  %734 = and i1 %660, %733
  br i1 %734, label %735, label %736

735:                                              ; preds = %731
  store i32 0, i32* %15, align 4, !tbaa !4
  br label %739

736:                                              ; preds = %731, %730
  %737 = icmp slt i32 %724, 4
  %738 = select i1 %737, i32 %724, i32 4
  br label %739

739:                                              ; preds = %736, %735
  %740 = phi i32 [ %632, %735 ], [ %738, %736 ]
  %741 = load i32, i32* %9, align 4, !tbaa !4
  %742 = sext i32 %741 to i64
  %743 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %742, !intel-tbaa !59
  %744 = load i32, i32* %743, align 4, !tbaa !59
  %745 = lshr i32 %744, 19
  %746 = and i32 %745, 15
  switch i32 %746, label %747 [
    i32 13, label %750
    i32 1, label %750
    i32 2, label %750
  ]

747:                                              ; preds = %739
  %748 = add nsw i32 %740, 4
  %749 = select i1 %634, i32 %748, i32 %740
  br label %750

750:                                              ; preds = %747, %739, %739, %739, %664
  %751 = phi i64 [ %742, %747 ], [ %742, %739 ], [ %742, %739 ], [ %742, %739 ], [ %669, %664 ]
  %752 = phi i32 [ %749, %747 ], [ %740, %739 ], [ %740, %739 ], [ %740, %739 ], [ 0, %664 ]
  %753 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %751, !intel-tbaa !59
  %754 = load i32, i32* %753, align 4, !tbaa !59
  %755 = and i32 %754, 7864320
  %756 = icmp eq i32 %755, 6815744
  %757 = xor i1 %756, true
  %758 = xor i1 %661, true
  %759 = or i1 %757, %758
  %760 = select i1 %757, i1 false, i1 true
  %761 = select i1 %757, i32 %665, i32 %656
  br i1 %759, label %799, label %762

762:                                              ; preds = %750
  %763 = lshr i32 %754, 6
  %764 = and i32 %763, 63
  %765 = zext i32 %764 to i64
  %766 = getelementptr inbounds [64 x i32], [64 x i32]* %627, i64 0, i64 %765, !intel-tbaa !47
  %767 = load i32, i32* %766, align 4, !tbaa !61
  %768 = add nsw i32 %767, -1
  %769 = and i32 %754, 63
  %770 = load i32, i32* %637, align 8, !tbaa !68
  %771 = sext i32 %770 to i64
  %772 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %771, !intel-tbaa !69
  %773 = sext i32 %768 to i64
  %774 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %772, i64 0, i64 %773, !intel-tbaa !72
  %775 = zext i32 %769 to i64
  %776 = getelementptr inbounds [64 x i32], [64 x i32]* %774, i64 0, i64 %775, !intel-tbaa !47
  %777 = load i32, i32* %776, align 4, !tbaa !73
  %778 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %771, !intel-tbaa !69
  %779 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %778, i64 0, i64 %773, !intel-tbaa !72
  %780 = getelementptr inbounds [64 x i32], [64 x i32]* %779, i64 0, i64 %775, !intel-tbaa !47
  %781 = load i32, i32* %780, align 4, !tbaa !73
  %782 = sub nsw i32 %781, %777
  %783 = mul nsw i32 %777, %636
  %784 = icmp sge i32 %783, %782
  %785 = or i1 %638, %784
  %786 = icmp ne i32 %752, 0
  %787 = or i1 %786, %785
  %788 = xor i1 %787, true
  %789 = and i1 %663, %788
  %790 = and i32 %754, 61440
  %791 = icmp eq i32 %790, 0
  %792 = and i1 %791, %789
  br i1 %792, label %793, label %799

793:                                              ; preds = %762
  %794 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %795 = icmp eq i32 %794, 0
  br i1 %795, label %796, label %664

796:                                              ; preds = %793
  %797 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !53
  %798 = icmp eq i32 %797, 0
  br label %1108

799:                                              ; preds = %762, %750
  %800 = phi i1 [ true, %762 ], [ %760, %750 ]
  %801 = phi i32 [ %665, %762 ], [ %761, %750 ]
  br i1 %639, label %802, label %805

802:                                              ; preds = %799
  %803 = icmp slt i32 %643, %654
  %804 = icmp slt i32 %644, %654
  br label %810

805:                                              ; preds = %799
  %806 = icmp slt i32 %641, %654
  %807 = icmp slt i32 %642, %654
  %808 = and i1 %640, %806
  %809 = and i1 %640, %807
  br label %810

810:                                              ; preds = %805, %802
  %811 = phi i1 [ %803, %802 ], [ %808, %805 ]
  %812 = phi i1 [ %804, %802 ], [ %809, %805 ]
  br i1 %800, label %825, label %813

813:                                              ; preds = %810
  %814 = load i32, i32* %631, align 4, !tbaa !19
  %815 = icmp eq i32 %814, 0
  %816 = zext i1 %815 to i32
  %817 = lshr i32 %754, 6
  %818 = and i32 %817, 63
  %819 = and i32 %754, 63
  %820 = lshr i32 %754, 12
  %821 = and i32 %820, 15
  %822 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %816, i32 %818, i32 %819, i32 %821)
  %823 = load i32, i32* %9, align 4, !tbaa !4
  %824 = sext i32 %823 to i64
  br label %825

825:                                              ; preds = %813, %810
  %826 = phi i64 [ %751, %810 ], [ %824, %813 ]
  %827 = phi i32 [ -1000000, %810 ], [ %822, %813 ]
  %828 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %826, !intel-tbaa !59
  %829 = load i32, i32* %828, align 4, !tbaa !59
  call void @_Z4makeP7state_ti(%struct.state_t* nonnull %0, i32 %829)
  %830 = load i32, i32* %9, align 4, !tbaa !4
  %831 = sext i32 %830 to i64
  %832 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %831, !intel-tbaa !59
  %833 = load i32, i32* %832, align 4, !tbaa !59
  %834 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* nonnull %0, i32 %833)
  %835 = icmp eq i32 %834, 0
  br i1 %835, label %976, label %836

836:                                              ; preds = %825
  %837 = call i32 @_Z8in_checkP7state_t(%struct.state_t* nonnull %0)
  %838 = icmp ne i32 %837, 0
  %839 = select i1 %838, i32 %645, i32 0
  %840 = add nsw i32 %752, %839
  %841 = or i32 %837, %91
  %842 = icmp eq i32 %841, 0
  %843 = and i1 %663, %842
  br i1 %843, label %844, label %868

844:                                              ; preds = %836
  %845 = icmp slt i32 %827, 86
  %846 = and i1 %812, %845
  br i1 %846, label %847, label %856

847:                                              ; preds = %844
  %848 = load i32, i32* %9, align 4, !tbaa !4
  %849 = sext i32 %848 to i64
  %850 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %849, !intel-tbaa !59
  %851 = load i32, i32* %850, align 4, !tbaa !59
  %852 = and i32 %851, 61440
  %853 = icmp eq i32 %852, 0
  br i1 %853, label %854, label %856

854:                                              ; preds = %847
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %851)
  %855 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  br label %1080

856:                                              ; preds = %847, %844
  %857 = icmp slt i32 %827, -50
  %858 = and i1 %811, %857
  br i1 %858, label %859, label %868

859:                                              ; preds = %856
  %860 = load i32, i32* %9, align 4, !tbaa !4
  %861 = sext i32 %860 to i64
  %862 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %861, !intel-tbaa !59
  %863 = load i32, i32* %862, align 4, !tbaa !59
  %864 = and i32 %863, 61440
  %865 = icmp eq i32 %864, 0
  br i1 %865, label %866, label %868

866:                                              ; preds = %859
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %863)
  %867 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  br label %1080

868:                                              ; preds = %859, %856, %836
  %869 = add i32 %646, %840
  %870 = sub nsw i32 0, %654
  %871 = sub i32 130, %654
  %872 = icmp sgt i32 %869, 0
  %873 = or i1 %838, %872
  %874 = zext i1 %873 to i32
  %875 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %647, i32 %871, i32 %874)
  %876 = load i32, i32* %31, align 8, !tbaa !37
  %877 = sext i32 %876 to i64
  %878 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %877, !intel-tbaa !47
  store i32 %837, i32* %878, align 4, !tbaa !48
  %879 = load i64, i64* %648, align 8, !tbaa !20
  %880 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !64
  %881 = add i32 %876, -1
  %882 = add i32 %881, %880
  %883 = sext i32 %882 to i64
  %884 = getelementptr inbounds [1000 x i64], [1000 x i64]* %649, i64 0, i64 %883, !intel-tbaa !65
  store i64 %879, i64* %884, align 8, !tbaa !66
  %885 = load i32, i32* %9, align 4, !tbaa !4
  %886 = sext i32 %885 to i64
  %887 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %886, !intel-tbaa !59
  %888 = load i32, i32* %887, align 4, !tbaa !59
  %889 = sext i32 %881 to i64
  %890 = getelementptr inbounds [64 x i32], [64 x i32]* %630, i64 0, i64 %889, !intel-tbaa !47
  store i32 %888, i32* %890, align 4, !tbaa !56
  %891 = icmp sgt i32 %659, 3
  %892 = and i1 %650, %891
  br i1 %892, label %893, label %927

893:                                              ; preds = %868
  %894 = or i32 %840, %837
  %895 = icmp eq i32 %894, 0
  %896 = and i1 %663, %895
  %897 = icmp slt i32 %827, -50
  %898 = and i1 %897, %896
  br i1 %898, label %899, label %927

899:                                              ; preds = %893
  %900 = and i32 %888, 63
  %901 = zext i32 %900 to i64
  %902 = getelementptr inbounds [64 x i32], [64 x i32]* %627, i64 0, i64 %901, !intel-tbaa !47
  %903 = load i32, i32* %902, align 4, !tbaa !61
  %904 = add nsw i32 %903, -1
  %905 = load i32, i32* %637, align 8, !tbaa !68
  %906 = sext i32 %905 to i64
  %907 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %906, !intel-tbaa !69
  %908 = sext i32 %904 to i64
  %909 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %907, i64 0, i64 %908, !intel-tbaa !72
  %910 = getelementptr inbounds [64 x i32], [64 x i32]* %909, i64 0, i64 %901, !intel-tbaa !47
  %911 = load i32, i32* %910, align 4, !tbaa !73
  %912 = shl i32 %911, 7
  %913 = add i32 %912, 128
  %914 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %906, !intel-tbaa !69
  %915 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %914, i64 0, i64 %908, !intel-tbaa !72
  %916 = getelementptr inbounds [64 x i32], [64 x i32]* %915, i64 0, i64 %901, !intel-tbaa !47
  %917 = load i32, i32* %916, align 4, !tbaa !73
  %918 = add nsw i32 %917, 1
  %919 = sdiv i32 %913, %918
  %920 = icmp slt i32 %919, 80
  %921 = and i32 %888, 61440
  %922 = icmp eq i32 %921, 0
  %923 = and i1 %922, %920
  br i1 %923, label %924, label %927

924:                                              ; preds = %899
  %925 = add nsw i32 %840, -4
  %926 = add i32 %646, %925
  br label %927

927:                                              ; preds = %924, %899, %893, %868
  %928 = phi i32 [ 4, %924 ], [ 0, %899 ], [ 0, %893 ], [ 0, %868 ]
  %929 = phi i32 [ %925, %924 ], [ %840, %899 ], [ %840, %893 ], [ %840, %868 ]
  %930 = phi i32 [ %926, %924 ], [ %869, %899 ], [ %869, %893 ], [ %869, %868 ]
  %931 = icmp eq i32 %658, 1
  %932 = icmp slt i32 %930, 1
  br i1 %931, label %933, label %940

933:                                              ; preds = %927
  br i1 %932, label %934, label %937

934:                                              ; preds = %933
  %935 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %647, i32 %870, i32 0, i32 0)
  %936 = sub nsw i32 0, %935
  br label %972

937:                                              ; preds = %933
  %938 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %647, i32 %870, i32 %930, i32 0, i32 %652)
  %939 = sub nsw i32 0, %938
  br label %972

940:                                              ; preds = %927
  %941 = xor i32 %654, -1
  br i1 %932, label %942, label %944

942:                                              ; preds = %940
  %943 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %941, i32 %870, i32 0, i32 0)
  br label %946

944:                                              ; preds = %940
  %945 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %941, i32 %870, i32 %930, i32 0, i32 1)
  br label %946

946:                                              ; preds = %944, %942
  %947 = phi i32 [ %943, %942 ], [ %945, %944 ]
  %948 = sub nsw i32 0, %947
  %949 = icmp slt i32 %657, %948
  %950 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %951 = icmp eq i32 %950, 0
  %952 = and i1 %949, %951
  %953 = icmp slt i32 %654, %948
  %954 = and i1 %953, %952
  br i1 %954, label %955, label %972

955:                                              ; preds = %946
  %956 = icmp eq i32 %928, 0
  br i1 %956, label %959, label %957

957:                                              ; preds = %955
  %958 = add nsw i32 %929, %928
  br label %961

959:                                              ; preds = %955
  %960 = icmp sgt i32 %70, %948
  br i1 %960, label %961, label %972

961:                                              ; preds = %959, %957
  %962 = phi i32 [ %958, %957 ], [ %929, %959 ]
  %963 = add i32 %646, %962
  %964 = icmp slt i32 %963, 1
  br i1 %964, label %965, label %968

965:                                              ; preds = %961
  %966 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %647, i32 %870, i32 0, i32 0)
  %967 = sub nsw i32 0, %966
  br label %972

968:                                              ; preds = %961
  %969 = lshr exact i32 %928, 2
  %970 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %647, i32 %870, i32 %963, i32 0, i32 %969)
  %971 = sub nsw i32 0, %970
  br label %972

972:                                              ; preds = %968, %965, %959, %946, %937, %934
  %973 = phi i32 [ %936, %934 ], [ %939, %937 ], [ %948, %946 ], [ %967, %965 ], [ %971, %968 ], [ %948, %959 ]
  %974 = icmp sgt i32 %973, %657
  %975 = select i1 %974, i32 %973, i32 %657
  br label %976

976:                                              ; preds = %972, %825
  %977 = phi i32 [ %975, %972 ], [ %657, %825 ]
  %978 = phi i32 [ 1, %972 ], [ 0, %825 ]
  %979 = phi i32 [ 0, %972 ], [ %801, %825 ]
  %980 = phi i32 [ %973, %972 ], [ %655, %825 ]
  %981 = load i32, i32* %9, align 4, !tbaa !4
  %982 = sext i32 %981 to i64
  %983 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %982, !intel-tbaa !59
  %984 = load i32, i32* %983, align 4, !tbaa !59
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %984)
  %985 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !53
  %986 = icmp eq i32 %985, 0
  br i1 %986, label %987, label %1121

987:                                              ; preds = %976
  %988 = icmp eq i32 %978, 0
  br i1 %988, label %1075, label %989

989:                                              ; preds = %987
  %990 = icmp sgt i32 %980, %654
  br i1 %990, label %991, label %1065

991:                                              ; preds = %989
  %992 = icmp slt i32 %980, %70
  %993 = load i32, i32* %9, align 4, !tbaa !4
  %994 = sext i32 %993 to i64
  %995 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %994
  %996 = load i32, i32* %995, align 4, !tbaa !59
  br i1 %992, label %1062, label %997

997:                                              ; preds = %991
  call fastcc void @_ZL12history_goodP7state_tii(%struct.state_t* nonnull %0, i32 %996, i32 %3)
  %998 = add i32 %659, -1
  %999 = icmp sgt i32 %998, 0
  br i1 %999, label %1000, label %1052

1000:                                             ; preds = %997
  %1001 = add nsw i32 %3, 3
  %1002 = sdiv i32 %1001, 4
  %1003 = sext i32 %998 to i64
  br label %1004

1004:                                             ; preds = %1049, %1000
  %1005 = phi i64 [ 0, %1000 ], [ %1050, %1049 ]
  %1006 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %1005, !intel-tbaa !59
  %1007 = load i32, i32* %1006, align 4, !tbaa !59
  %1008 = and i32 %1007, 7925760
  %1009 = icmp eq i32 %1008, 6815744
  br i1 %1009, label %1010, label %1049

1010:                                             ; preds = %1004
  %1011 = lshr i32 %1007, 6
  %1012 = and i32 %1011, 63
  %1013 = zext i32 %1012 to i64
  %1014 = getelementptr inbounds [64 x i32], [64 x i32]* %627, i64 0, i64 %1013, !intel-tbaa !47
  %1015 = load i32, i32* %1014, align 4, !tbaa !61
  %1016 = add nsw i32 %1015, -1
  %1017 = and i32 %1007, 63
  %1018 = load i32, i32* %637, align 8, !tbaa !68
  %1019 = sext i32 %1018 to i64
  %1020 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %1019, !intel-tbaa !69
  %1021 = sext i32 %1016 to i64
  %1022 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1020, i64 0, i64 %1021, !intel-tbaa !72
  %1023 = zext i32 %1017 to i64
  %1024 = getelementptr inbounds [64 x i32], [64 x i32]* %1022, i64 0, i64 %1023, !intel-tbaa !47
  %1025 = load i32, i32* %1024, align 4, !tbaa !73
  %1026 = add nsw i32 %1025, %1002
  store i32 %1026, i32* %1024, align 4, !tbaa !73
  %1027 = icmp sgt i32 %1026, 16384
  br i1 %1027, label %1028, label %1049

1028:                                             ; preds = %1010
  %1029 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %1019, !intel-tbaa !69
  br label %1030

1030:                                             ; preds = %1046, %1028
  %1031 = phi i64 [ 0, %1028 ], [ %1047, %1046 ]
  %1032 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1029, i64 0, i64 %1031, !intel-tbaa !72
  %1033 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1020, i64 0, i64 %1031, !intel-tbaa !72
  br label %1034

1034:                                             ; preds = %1034, %1030
  %1035 = phi i64 [ 0, %1030 ], [ %1044, %1034 ]
  %1036 = getelementptr inbounds [64 x i32], [64 x i32]* %1032, i64 0, i64 %1035, !intel-tbaa !47
  %1037 = load i32, i32* %1036, align 4, !tbaa !73
  %1038 = add nsw i32 %1037, 1
  %1039 = ashr i32 %1038, 1
  store i32 %1039, i32* %1036, align 4, !tbaa !73
  %1040 = getelementptr inbounds [64 x i32], [64 x i32]* %1033, i64 0, i64 %1035, !intel-tbaa !47
  %1041 = load i32, i32* %1040, align 4, !tbaa !73
  %1042 = add nsw i32 %1041, 1
  %1043 = ashr i32 %1042, 1
  store i32 %1043, i32* %1040, align 4, !tbaa !73
  %1044 = add nuw nsw i64 %1035, 1
  %1045 = icmp eq i64 %1044, 64
  br i1 %1045, label %1046, label %1034

1046:                                             ; preds = %1034
  %1047 = add nuw nsw i64 %1031, 1
  %1048 = icmp eq i64 %1047, 12
  br i1 %1048, label %1049, label %1030

1049:                                             ; preds = %1046, %1010, %1004
  %1050 = add nuw nsw i64 %1005, 1
  %1051 = icmp eq i64 %1050, %1003
  br i1 %1051, label %1052, label %1004

1052:                                             ; preds = %1049, %997
  %1053 = load i32, i32* %9, align 4, !tbaa !4
  %1054 = sext i32 %1053 to i64
  %1055 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %1054, !intel-tbaa !59
  %1056 = load i32, i32* %1055, align 4, !tbaa !59
  %1057 = call zeroext i16 @_Z12compact_movei(i32 %1056)
  %1058 = zext i16 %1057 to i32
  %1059 = load i32, i32* %11, align 4, !tbaa !4
  %1060 = load i32, i32* %14, align 4, !tbaa !4
  %1061 = load i32, i32* %15, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %980, i32 %64, i32 %70, i32 %1058, i32 %1059, i32 %1060, i32 %1061, i32 %3)
  br label %1121

1062:                                             ; preds = %991
  %1063 = call zeroext i16 @_Z12compact_movei(i32 %996)
  %1064 = zext i16 %1063 to i32
  store i32 %1064, i32* %13, align 4, !tbaa !4
  br label %1065

1065:                                             ; preds = %1062, %989
  %1066 = phi i32 [ %980, %1062 ], [ %654, %989 ]
  %1067 = load i32, i32* %9, align 4, !tbaa !4
  %1068 = sext i32 %1067 to i64
  %1069 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %1068, !intel-tbaa !59
  %1070 = load i32, i32* %1069, align 4, !tbaa !59
  %1071 = add nsw i32 %659, -1
  %1072 = sext i32 %1071 to i64
  %1073 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %1072, !intel-tbaa !59
  store i32 %1070, i32* %1073, align 4, !tbaa !59
  %1074 = add nsw i32 %659, 1
  br label %1075

1075:                                             ; preds = %1065, %987
  %1076 = phi i32 [ %1074, %1065 ], [ %659, %987 ]
  %1077 = phi i32 [ 0, %1065 ], [ %658, %987 ]
  %1078 = phi i32 [ %1066, %1065 ], [ %654, %987 ]
  %1079 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  br label %1080

1080:                                             ; preds = %1075, %866, %854
  %1081 = phi i32 [ %1079, %1075 ], [ %867, %866 ], [ %855, %854 ]
  %1082 = phi i32 [ %1076, %1075 ], [ %659, %866 ], [ %659, %854 ]
  %1083 = phi i32 [ %1077, %1075 ], [ %658, %866 ], [ %658, %854 ]
  %1084 = phi i32 [ %977, %1075 ], [ %654, %866 ], [ %654, %854 ]
  %1085 = phi i32 [ %979, %1075 ], [ 0, %866 ], [ 0, %854 ]
  %1086 = phi i32 [ %980, %1075 ], [ %655, %866 ], [ %655, %854 ]
  %1087 = phi i32 [ %1078, %1075 ], [ %654, %866 ], [ %654, %854 ]
  %1088 = icmp eq i32 %1081, 0
  br i1 %1088, label %1089, label %653

1089:                                             ; preds = %1080, %619
  %1090 = phi i32 [ -32000, %619 ], [ %1084, %1080 ]
  %1091 = phi i32 [ 1, %619 ], [ %1085, %1080 ]
  %1092 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !53
  %1093 = icmp eq i32 %1092, 0
  %1094 = icmp ne i32 %1091, 0
  %1095 = and i1 %1094, %1093
  br i1 %1095, label %1096, label %1108

1096:                                             ; preds = %1089
  %1097 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %1098 = icmp eq i32 %1097, 0
  %1099 = load i32, i32* %11, align 4, !tbaa !4
  %1100 = load i32, i32* %14, align 4, !tbaa !4
  %1101 = load i32, i32* %15, align 4, !tbaa !4
  br i1 %1098, label %1107, label %1102

1102:                                             ; preds = %1096
  %1103 = load i32, i32* %31, align 8, !tbaa !37
  %1104 = add nsw i32 %1103, -32000
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %1104, i32 %64, i32 %70, i32 0, i32 %1099, i32 %1100, i32 %1101, i32 %3)
  %1105 = load i32, i32* %31, align 8, !tbaa !37
  %1106 = add nsw i32 %1105, -32000
  br label %1121

1107:                                             ; preds = %1096
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 0, i32 %64, i32 %70, i32 0, i32 %1099, i32 %1100, i32 %1101, i32 %3)
  br label %1121

1108:                                             ; preds = %1089, %796
  %1109 = phi i1 [ %798, %796 ], [ %1093, %1089 ]
  %1110 = phi i32 [ %657, %796 ], [ %1090, %1089 ]
  %1111 = load i32, i32* %46, align 4, !tbaa !41
  %1112 = icmp sgt i32 %1111, 98
  %1113 = xor i1 %1109, true
  %1114 = or i1 %1112, %1113
  %1115 = select i1 %1112, i32 0, i32 %1110
  br i1 %1114, label %1121, label %1116

1116:                                             ; preds = %1108
  %1117 = load i32, i32* %13, align 4, !tbaa !4
  %1118 = load i32, i32* %11, align 4, !tbaa !4
  %1119 = load i32, i32* %14, align 4, !tbaa !4
  %1120 = load i32, i32* %15, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %1110, i32 %64, i32 %70, i32 %1117, i32 %1118, i32 %1119, i32 %1120, i32 %3)
  br label %1121

1121:                                             ; preds = %1116, %1108, %1107, %1102, %1052, %976, %498, %243, %237, %224, %211, %172, %117, %110, %103, %77, %74, %72, %67, %61, %49, %36, %34
  %1122 = phi i32 [ %35, %34 ], [ %56, %49 ], [ %73, %72 ], [ 0, %36 ], [ %59, %61 ], [ %65, %67 ], [ %75, %74 ], [ %78, %77 ], [ %70, %498 ], [ %92, %103 ], [ %111, %110 ], [ %92, %117 ], [ 0, %172 ], [ %213, %224 ], [ 0, %211 ], [ %64, %243 ], [ 0, %237 ], [ %980, %1052 ], [ %1106, %1102 ], [ 0, %1107 ], [ %1115, %1108 ], [ %1110, %1116 ], [ 0, %976 ]
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %28) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %27) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %26) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %25) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %24) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %23) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %22) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %21) #8
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %20) #8
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %19) #8
  ret i32 %1122
}

; Function Attrs: uwtable
define internal i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4) #2 {
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca [240 x i32], align 16
  %11 = alloca [240 x i32], align 16
  %12 = bitcast i32* %6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %12) #8
  %13 = bitcast i32* %7 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %13) #8
  %14 = bitcast i32* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %14) #8
  %15 = bitcast i32* %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %15) #8
  %16 = bitcast [240 x i32]* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %16) #8
  %17 = bitcast [240 x i32]* %11 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %17) #8
  %18 = getelementptr %struct.state_t, %struct.state_t* %0, i64 0, i32 22, !intel-tbaa !40
  %19 = load i64, i64* %18, align 8, !tbaa !40
  %20 = add i64 %19, 1
  store i64 %20, i64* %18, align 8, !tbaa !40
  %21 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 23, !intel-tbaa !74
  %22 = load i64, i64* %21, align 8, !tbaa !74
  %23 = add i64 %22, 1
  store i64 %23, i64* %21, align 8, !tbaa !74
  %24 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !37
  %25 = load i32, i32* %24, align 8, !tbaa !37
  %26 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 24, !intel-tbaa !75
  %27 = load i32, i32* %26, align 8, !tbaa !75
  %28 = icmp sgt i32 %25, %27
  br i1 %28, label %29, label %30

29:                                               ; preds = %5
  store i32 %25, i32* %26, align 8, !tbaa !75
  br label %30

30:                                               ; preds = %29, %5
  %31 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %20)
  %32 = icmp eq i32 %31, 0
  br i1 %32, label %33, label %420

33:                                               ; preds = %30
  %34 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nonnull @gamestate, %struct.state_t* nonnull %0)
  %35 = icmp eq i32 %34, 0
  br i1 %35, label %36, label %40

36:                                               ; preds = %33
  %37 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15, !intel-tbaa !41
  %38 = load i32, i32* %37, align 4, !tbaa !41
  %39 = icmp sgt i32 %38, 99
  br i1 %39, label %40, label %48

40:                                               ; preds = %36, %33
  %41 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4, !tbaa !42
  %42 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !19
  %43 = load i32, i32* %42, align 4, !tbaa !19
  %44 = icmp eq i32 %41, %43
  %45 = load i32, i32* @contempt, align 4, !tbaa !4
  %46 = sub nsw i32 0, %45
  %47 = select i1 %44, i32 %45, i32 %46
  br label %420

48:                                               ; preds = %36
  %49 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nonnull %0, i32* nonnull %8, i32 %1, i32 %2, i32* nonnull %7, i32* nonnull %9, i32* nonnull %9, i32* nonnull %9, i32* nonnull %9, i32 0)
  switch i32 %49, label %59 [
    i32 3, label %50
    i32 1, label %52
    i32 2, label %55
    i32 4, label %58
  ]

50:                                               ; preds = %48
  %51 = load i32, i32* %8, align 4, !tbaa !4
  br label %420

52:                                               ; preds = %48
  %53 = load i32, i32* %8, align 4, !tbaa !4
  %54 = icmp sgt i32 %53, %1
  br i1 %54, label %59, label %420

55:                                               ; preds = %48
  %56 = load i32, i32* %8, align 4, !tbaa !4
  %57 = icmp slt i32 %56, %2
  br i1 %57, label %59, label %420

58:                                               ; preds = %48
  store i32 65535, i32* %7, align 4, !tbaa !4
  br label %59

59:                                               ; preds = %58, %55, %52, %48
  %60 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4, !tbaa !67
  %61 = shl nsw i32 %60, 1
  %62 = icmp slt i32 %61, %4
  br i1 %62, label %66, label %63

63:                                               ; preds = %59
  %64 = load i32, i32* %24, align 8, !tbaa !37
  %65 = icmp sgt i32 %64, 60
  br i1 %65, label %66, label %68

66:                                               ; preds = %63, %59
  %67 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %1, i32 %2, i32 0)
  br label %420

68:                                               ; preds = %63
  %69 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25, !intel-tbaa !46
  %70 = sext i32 %64 to i64
  %71 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %70, !intel-tbaa !47
  %72 = load i32, i32* %71, align 4, !tbaa !48
  %73 = call i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nonnull %0)
  %74 = add nsw i32 %73, 50
  %75 = icmp ne i32 %72, 0
  br i1 %75, label %150, label %76

76:                                               ; preds = %68
  %77 = icmp slt i32 %73, %2
  br i1 %77, label %80, label %78

78:                                               ; preds = %76
  %79 = load i32, i32* %7, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %73, i32 %1, i32 %2, i32 %79, i32 0, i32 0, i32 0, i32 0)
  br label %420

80:                                               ; preds = %76
  %81 = icmp sgt i32 %73, %1
  br i1 %81, label %147, label %82

82:                                               ; preds = %80
  %83 = add nsw i32 %73, 985
  %84 = icmp sgt i32 %83, %1
  br i1 %84, label %87, label %85

85:                                               ; preds = %82
  %86 = load i32, i32* %7, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %83, i32 %1, i32 %2, i32 %86, i32 0, i32 0, i32 0, i32 0)
  br label %420

87:                                               ; preds = %82
  %88 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8, !intel-tbaa !49
  %89 = getelementptr inbounds [13 x i32], [13 x i32]* %88, i64 0, i64 0
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !19
  %91 = load i32, i32* %90, align 4, !tbaa !19
  %92 = icmp eq i32 %91, 0
  br i1 %92, label %120, label %93

93:                                               ; preds = %87
  %94 = getelementptr inbounds i32, i32* %89, i64 10
  %95 = load i32, i32* %94, align 4, !tbaa !4
  %96 = icmp eq i32 %95, 0
  br i1 %96, label %97, label %147

97:                                               ; preds = %93
  %98 = getelementptr inbounds i32, i32* %89, i64 8
  %99 = load i32, i32* %98, align 4, !tbaa !4
  %100 = icmp eq i32 %99, 0
  br i1 %100, label %101, label %115

101:                                              ; preds = %97
  %102 = getelementptr inbounds i32, i32* %89, i64 12
  %103 = load i32, i32* %102, align 4, !tbaa !4
  %104 = icmp eq i32 %103, 0
  br i1 %104, label %105, label %112

105:                                              ; preds = %101
  %106 = getelementptr inbounds i32, i32* %89, i64 4
  %107 = load i32, i32* %106, align 4, !tbaa !4
  %108 = icmp eq i32 %107, 0
  br i1 %108, label %109, label %112

109:                                              ; preds = %105
  %110 = add nsw i32 %73, 135
  %111 = icmp sgt i32 %110, %1
  br i1 %111, label %147, label %420

112:                                              ; preds = %105, %101
  %113 = add nsw i32 %73, 380
  %114 = icmp sgt i32 %113, %1
  br i1 %114, label %147, label %420

115:                                              ; preds = %97
  %116 = add nsw i32 %73, 540
  %117 = icmp sgt i32 %116, %1
  br i1 %117, label %147, label %118

118:                                              ; preds = %115
  %119 = load i32, i32* %7, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %116, i32 %1, i32 %2, i32 %119, i32 0, i32 0, i32 0, i32 0)
  br label %420

120:                                              ; preds = %87
  %121 = getelementptr inbounds i32, i32* %89, i64 9
  %122 = load i32, i32* %121, align 4, !tbaa !4
  %123 = icmp eq i32 %122, 0
  br i1 %123, label %124, label %147

124:                                              ; preds = %120
  %125 = getelementptr inbounds i32, i32* %89, i64 7
  %126 = load i32, i32* %125, align 4, !tbaa !4
  %127 = icmp eq i32 %126, 0
  br i1 %127, label %128, label %142

128:                                              ; preds = %124
  %129 = getelementptr inbounds i32, i32* %89, i64 11
  %130 = load i32, i32* %129, align 4, !tbaa !4
  %131 = icmp eq i32 %130, 0
  br i1 %131, label %132, label %139

132:                                              ; preds = %128
  %133 = getelementptr inbounds i32, i32* %89, i64 3
  %134 = load i32, i32* %133, align 4, !tbaa !4
  %135 = icmp eq i32 %134, 0
  br i1 %135, label %136, label %139

136:                                              ; preds = %132
  %137 = add nsw i32 %73, 135
  %138 = icmp sgt i32 %137, %1
  br i1 %138, label %147, label %420

139:                                              ; preds = %132, %128
  %140 = add nsw i32 %73, 380
  %141 = icmp sgt i32 %140, %1
  br i1 %141, label %147, label %420

142:                                              ; preds = %124
  %143 = add nsw i32 %73, 540
  %144 = icmp sgt i32 %143, %1
  br i1 %144, label %147, label %145

145:                                              ; preds = %142
  %146 = load i32, i32* %7, align 4, !tbaa !4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %143, i32 %1, i32 %2, i32 %146, i32 0, i32 0, i32 0, i32 0)
  br label %420

147:                                              ; preds = %142, %139, %136, %120, %115, %112, %109, %93, %80
  %148 = phi i32 [ %73, %80 ], [ %1, %120 ], [ %1, %136 ], [ %1, %139 ], [ %1, %142 ], [ %1, %93 ], [ %1, %109 ], [ %1, %112 ], [ %1, %115 ]
  %149 = sub nsw i32 %148, %74
  br label %150

150:                                              ; preds = %147, %68
  %151 = phi i32 [ %148, %147 ], [ %1, %68 ]
  %152 = phi i32 [ %149, %147 ], [ 0, %68 ]
  %153 = load i32, i32* %7, align 4, !tbaa !4
  %154 = icmp sgt i32 %3, -6
  %155 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 0
  br i1 %154, label %156, label %161

156:                                              ; preds = %150
  br i1 %75, label %157, label %159

157:                                              ; preds = %156
  %158 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %155, i32 %72)
  br label %166

159:                                              ; preds = %156
  %160 = call i32 @_Z12gen_capturesP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %155)
  br label %166

161:                                              ; preds = %150
  br i1 %75, label %164, label %162

162:                                              ; preds = %161
  %163 = call i32 @_Z12gen_capturesP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %155)
  br label %166

164:                                              ; preds = %161
  %165 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %155, i32 %72)
  br label %166

166:                                              ; preds = %164, %162, %159, %157
  %167 = phi i1 [ false, %157 ], [ true, %159 ], [ false, %164 ], [ false, %162 ]
  %168 = phi i32 [ %158, %157 ], [ %160, %159 ], [ %165, %164 ], [ %163, %162 ]
  %169 = getelementptr inbounds [240 x i32], [240 x i32]* %11, i64 0, i64 0
  %170 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %171 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 0
  %172 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %173 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %174 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %175 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %176 = sub nsw i32 0, %2
  %177 = add nsw i32 %4, 1
  %178 = icmp sgt i32 %3, -1
  br label %179

179:                                              ; preds = %405, %166
  %180 = phi i32 [ %153, %166 ], [ %399, %405 ]
  %181 = phi i32 [ 1, %166 ], [ %400, %405 ]
  %182 = phi i32 [ -32000, %166 ], [ %401, %405 ]
  %183 = phi i32 [ 1, %166 ], [ %406, %405 ]
  %184 = phi i32 [ %168, %166 ], [ %194, %405 ]
  %185 = phi i32 [ %151, %166 ], [ %402, %405 ]
  %186 = icmp eq i32 %183, 2
  br i1 %186, label %187, label %189

187:                                              ; preds = %179
  %188 = call i32 @_Z15gen_good_checksP7state_tPi(%struct.state_t* %0, i32* nonnull %155)
  br label %193

189:                                              ; preds = %179
  %190 = icmp eq i32 %183, 3
  br i1 %190, label %191, label %193

191:                                              ; preds = %189
  %192 = call i32 @_Z9gen_quietP7state_tPi(%struct.state_t* %0, i32* nonnull %155)
  br label %193

193:                                              ; preds = %191, %189, %187
  %194 = phi i32 [ %188, %187 ], [ %192, %191 ], [ %184, %189 ]
  %195 = load i32, i32* %7, align 4, !tbaa !4
  call fastcc void @_ZL16fast_order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %155, i32* nonnull %169, i32 %194, i32 %195)
  store i32 -1, i32* %6, align 4, !tbaa !4
  %196 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %169, i32* nonnull %155, i32 %194)
  %197 = icmp eq i32 %196, 0
  br i1 %197, label %398, label %198

198:                                              ; preds = %193
  %199 = icmp eq i32 %183, 1
  %200 = icmp eq i32 %183, 3
  %201 = or i32 %183, 1
  %202 = icmp eq i32 %201, 3
  br label %203

203:                                              ; preds = %393, %198
  %204 = phi i32 [ %185, %198 ], [ %395, %393 ]
  %205 = phi i32 [ %182, %198 ], [ %379, %393 ]
  %206 = phi i32 [ %181, %198 ], [ %377, %393 ]
  %207 = phi i32 [ %180, %198 ], [ %394, %393 ]
  br i1 %75, label %208, label %210

208:                                              ; preds = %203
  %209 = load i32, i32* %6, align 4, !tbaa !4
  br label %336

210:                                              ; preds = %203
  br i1 %199, label %211, label %251

211:                                              ; preds = %248, %210
  %212 = load i32, i32* %6, align 4, !tbaa !4
  %213 = sext i32 %212 to i64
  %214 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %213
  %215 = load i32, i32* %214, align 4, !tbaa !59
  %216 = lshr i32 %215, 19
  %217 = and i32 %216, 15
  %218 = zext i32 %217 to i64
  %219 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %218
  %220 = load i32, i32* %219, align 4, !tbaa !62
  %221 = icmp slt i32 %220, 0
  %222 = sub nsw i32 0, %220
  %223 = select i1 %221, i32 %222, i32 %220
  %224 = icmp sle i32 %223, %152
  %225 = and i32 %215, 61440
  %226 = icmp eq i32 %225, 0
  %227 = and i1 %226, %224
  br i1 %227, label %398, label %228

228:                                              ; preds = %211
  %229 = lshr i32 %215, 6
  %230 = and i32 %229, 63
  %231 = zext i32 %230 to i64
  %232 = getelementptr inbounds [64 x i32], [64 x i32]* %170, i64 0, i64 %231, !intel-tbaa !47
  %233 = load i32, i32* %232, align 4, !tbaa !61
  %234 = sext i32 %233 to i64
  %235 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %234, !intel-tbaa !62
  %236 = load i32, i32* %235, align 4, !tbaa !62
  %237 = icmp slt i32 %236, 0
  %238 = sub nsw i32 0, %236
  %239 = select i1 %237, i32 %238, i32 %236
  %240 = icmp slt i32 %223, %239
  br i1 %240, label %241, label %336

241:                                              ; preds = %228
  %242 = load i32, i32* %172, align 4, !tbaa !19
  %243 = icmp eq i32 %242, 0
  %244 = zext i1 %243 to i32
  %245 = and i32 %215, 63
  %246 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %244, i32 %230, i32 %245, i32 0)
  %247 = icmp slt i32 %246, -50
  br i1 %247, label %248, label %336

248:                                              ; preds = %241
  %249 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %169, i32* nonnull %155, i32 %194)
  %250 = icmp eq i32 %249, 0
  br i1 %250, label %398, label %211

251:                                              ; preds = %268, %210
  br i1 %202, label %252, label %271

252:                                              ; preds = %251
  %253 = load i32, i32* %6, align 4, !tbaa !4
  %254 = sext i32 %253 to i64
  %255 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %254, !intel-tbaa !59
  %256 = load i32, i32* %255, align 4, !tbaa !59
  %257 = lshr i32 %256, 19
  %258 = and i32 %257, 15
  %259 = icmp eq i32 %258, 13
  br i1 %259, label %271, label %260

260:                                              ; preds = %252
  %261 = zext i32 %258 to i64
  %262 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %261, !intel-tbaa !62
  %263 = load i32, i32* %262, align 4, !tbaa !62
  %264 = icmp slt i32 %263, 0
  %265 = sub nsw i32 0, %263
  %266 = select i1 %264, i32 %265, i32 %263
  %267 = icmp sgt i32 %266, %152
  br i1 %267, label %268, label %271

268:                                              ; preds = %325, %273, %260
  %269 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %169, i32* nonnull %155, i32 %194)
  %270 = icmp eq i32 %269, 0
  br i1 %270, label %398, label %251

271:                                              ; preds = %260, %252, %251
  %272 = phi i1 [ true, %252 ], [ false, %251 ], [ true, %260 ]
  br i1 %200, label %273, label %299

273:                                              ; preds = %271
  %274 = load i32, i32* %6, align 4, !tbaa !4
  %275 = sext i32 %274 to i64
  %276 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %275, !intel-tbaa !59
  %277 = load i32, i32* %276, align 4, !tbaa !59
  %278 = lshr i32 %277, 6
  %279 = and i32 %278, 63
  %280 = zext i32 %279 to i64
  %281 = getelementptr inbounds [64 x i32], [64 x i32]* %170, i64 0, i64 %280, !intel-tbaa !47
  %282 = load i32, i32* %281, align 4, !tbaa !61
  %283 = add nsw i32 %282, -1
  %284 = and i32 %277, 63
  %285 = load i32, i32* %171, align 8, !tbaa !68
  %286 = sext i32 %285 to i64
  %287 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %286, !intel-tbaa !69
  %288 = sext i32 %283 to i64
  %289 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %287, i64 0, i64 %288, !intel-tbaa !72
  %290 = zext i32 %284 to i64
  %291 = getelementptr inbounds [64 x i32], [64 x i32]* %289, i64 0, i64 %290, !intel-tbaa !47
  %292 = load i32, i32* %291, align 4, !tbaa !73
  %293 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %286, !intel-tbaa !69
  %294 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %293, i64 0, i64 %288, !intel-tbaa !72
  %295 = getelementptr inbounds [64 x i32], [64 x i32]* %294, i64 0, i64 %290, !intel-tbaa !47
  %296 = load i32, i32* %295, align 4, !tbaa !73
  %297 = sub nsw i32 %296, %292
  %298 = icmp slt i32 %292, %297
  br i1 %298, label %268, label %299

299:                                              ; preds = %273, %271
  %300 = load i32, i32* %6, align 4, !tbaa !4
  %301 = sext i32 %300 to i64
  br i1 %272, label %325, label %302

302:                                              ; preds = %299
  %303 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %301, !intel-tbaa !59
  %304 = load i32, i32* %303, align 4, !tbaa !59
  %305 = lshr i32 %304, 19
  %306 = and i32 %305, 15
  %307 = zext i32 %306 to i64
  %308 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %307, !intel-tbaa !62
  %309 = load i32, i32* %308, align 4, !tbaa !62
  %310 = icmp slt i32 %309, 0
  %311 = sub nsw i32 0, %309
  %312 = select i1 %310, i32 %311, i32 %309
  %313 = lshr i32 %304, 6
  %314 = and i32 %313, 63
  %315 = zext i32 %314 to i64
  %316 = getelementptr inbounds [64 x i32], [64 x i32]* %170, i64 0, i64 %315, !intel-tbaa !47
  %317 = load i32, i32* %316, align 4, !tbaa !61
  %318 = sext i32 %317 to i64
  %319 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %318, !intel-tbaa !62
  %320 = load i32, i32* %319, align 4, !tbaa !62
  %321 = icmp slt i32 %320, 0
  %322 = sub nsw i32 0, %320
  %323 = select i1 %321, i32 %322, i32 %320
  %324 = icmp slt i32 %312, %323
  br i1 %324, label %325, label %336

325:                                              ; preds = %302, %299
  %326 = load i32, i32* %172, align 4, !tbaa !19
  %327 = icmp eq i32 %326, 0
  %328 = zext i1 %327 to i32
  %329 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %301, !intel-tbaa !59
  %330 = load i32, i32* %329, align 4, !tbaa !59
  %331 = lshr i32 %330, 6
  %332 = and i32 %331, 63
  %333 = and i32 %330, 63
  %334 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* %0, i32 %328, i32 %332, i32 %333, i32 0)
  %335 = icmp slt i32 %334, -50
  br i1 %335, label %268, label %336

336:                                              ; preds = %325, %302, %241, %228, %208
  %337 = phi i32 [ %209, %208 ], [ %212, %228 ], [ %212, %241 ], [ %300, %302 ], [ %300, %325 ]
  %338 = sext i32 %337 to i64
  %339 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %338, !intel-tbaa !59
  %340 = load i32, i32* %339, align 4, !tbaa !59
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %340)
  %341 = load i32, i32* %339, align 4, !tbaa !59
  %342 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %341)
  %343 = icmp eq i32 %342, 0
  br i1 %343, label %376, label %344

344:                                              ; preds = %336
  %345 = load i64, i64* %173, align 8, !tbaa !20
  %346 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !64
  %347 = load i32, i32* %24, align 8, !tbaa !37
  %348 = add i32 %347, -1
  %349 = add i32 %348, %346
  %350 = sext i32 %349 to i64
  %351 = getelementptr inbounds [1000 x i64], [1000 x i64]* %174, i64 0, i64 %350, !intel-tbaa !65
  store i64 %345, i64* %351, align 8, !tbaa !66
  %352 = load i32, i32* %339, align 4, !tbaa !59
  %353 = sext i32 %348 to i64
  %354 = getelementptr inbounds [64 x i32], [64 x i32]* %175, i64 0, i64 %353, !intel-tbaa !47
  store i32 %352, i32* %354, align 4, !tbaa !56
  %355 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %356 = load i32, i32* %24, align 8, !tbaa !37
  %357 = sext i32 %356 to i64
  %358 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %357, !intel-tbaa !47
  store i32 %355, i32* %358, align 4, !tbaa !48
  %359 = sub nsw i32 0, %204
  %360 = sub i32 60, %204
  %361 = icmp ne i32 %355, 0
  %362 = zext i1 %361 to i32
  %363 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %176, i32 %360, i32 %362)
  br i1 %200, label %364, label %367

364:                                              ; preds = %344
  %365 = sub nsw i32 0, %363
  %366 = icmp slt i32 %204, %365
  br i1 %366, label %372, label %376

367:                                              ; preds = %344
  %368 = or i32 %355, %72
  %369 = icmp eq i32 %368, 0
  %370 = select i1 %369, i32 -8, i32 -1
  %371 = add nsw i32 %370, %3
  br label %372

372:                                              ; preds = %367, %364
  %373 = phi i32 [ %3, %364 ], [ %371, %367 ]
  %374 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %176, i32 %359, i32 %373, i32 %177)
  %375 = sub nsw i32 0, %374
  br label %376

376:                                              ; preds = %372, %364, %336
  %377 = phi i32 [ %206, %336 ], [ 0, %372 ], [ 0, %364 ]
  %378 = phi i32 [ 0, %336 ], [ 1, %372 ], [ 1, %364 ]
  %379 = phi i32 [ %205, %336 ], [ %375, %372 ], [ %205, %364 ]
  %380 = load i32, i32* %339, align 4, !tbaa !59
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %380)
  %381 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !53
  %382 = icmp eq i32 %381, 0
  br i1 %382, label %383, label %420

383:                                              ; preds = %376
  %384 = icmp sgt i32 %379, %204
  %385 = icmp ne i32 %378, 0
  %386 = and i1 %385, %384
  br i1 %386, label %387, label %393

387:                                              ; preds = %383
  %388 = load i32, i32* %339, align 4, !tbaa !59
  %389 = call zeroext i16 @_Z12compact_movei(i32 %388)
  %390 = zext i16 %389 to i32
  %391 = icmp slt i32 %379, %2
  br i1 %391, label %393, label %392

392:                                              ; preds = %387
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %379, i32 %1, i32 %2, i32 %390, i32 0, i32 0, i32 0, i32 0)
  br label %420

393:                                              ; preds = %387, %383
  %394 = phi i32 [ %207, %383 ], [ %390, %387 ]
  %395 = phi i32 [ %204, %383 ], [ %379, %387 ]
  %396 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %169, i32* nonnull %155, i32 %194)
  %397 = icmp eq i32 %396, 0
  br i1 %397, label %398, label %203

398:                                              ; preds = %393, %268, %248, %211, %193
  %399 = phi i32 [ %180, %193 ], [ %207, %211 ], [ %207, %248 ], [ %207, %268 ], [ %394, %393 ]
  %400 = phi i32 [ %181, %193 ], [ %206, %211 ], [ %206, %248 ], [ %206, %268 ], [ %377, %393 ]
  %401 = phi i32 [ %182, %193 ], [ %205, %211 ], [ %205, %248 ], [ %205, %268 ], [ %379, %393 ]
  %402 = phi i32 [ %185, %193 ], [ %204, %211 ], [ %204, %248 ], [ %204, %268 ], [ %395, %393 ]
  %403 = icmp eq i32 %183, 1
  %404 = and i1 %167, %403
  br i1 %404, label %405, label %407

405:                                              ; preds = %407, %398
  %406 = phi i32 [ 2, %398 ], [ 3, %407 ]
  br label %179

407:                                              ; preds = %398
  %408 = and i1 %167, %186
  %409 = and i1 %178, %408
  %410 = icmp sgt i32 %74, %402
  %411 = and i1 %409, %410
  br i1 %411, label %405, label %412

412:                                              ; preds = %407
  %413 = icmp ne i32 %400, 0
  %414 = and i1 %75, %413
  br i1 %414, label %415, label %418

415:                                              ; preds = %412
  %416 = load i32, i32* %24, align 8, !tbaa !37
  %417 = add nsw i32 %416, -32000
  br label %418

418:                                              ; preds = %415, %412
  %419 = phi i32 [ %417, %415 ], [ %402, %412 ]
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %419, i32 %1, i32 %2, i32 %399, i32 0, i32 0, i32 0, i32 0)
  br label %420

420:                                              ; preds = %418, %392, %376, %145, %139, %136, %118, %112, %109, %85, %78, %66, %55, %52, %50, %40, %30
  %421 = phi i32 [ %47, %40 ], [ %67, %66 ], [ %379, %392 ], [ %419, %418 ], [ %73, %78 ], [ %83, %85 ], [ %51, %50 ], [ 0, %30 ], [ %53, %52 ], [ %56, %55 ], [ %143, %145 ], [ %116, %118 ], [ %110, %109 ], [ %113, %112 ], [ %137, %136 ], [ %140, %139 ], [ 0, %376 ]
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %17) #8
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %16) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %15) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %14) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %13) #8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %12) #8
  ret i32 %421
}

; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z11check_legalP7state_ti(%struct.state_t*, i32) #0

; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z11is_attackedP7state_tii(%struct.state_t* nocapture readonly, i32, i32) #0

; Function Attrs: norecurse nounwind readonly uwtable
declare i64 @_Z11FileAttacksyj(i64, i32) #3

; Function Attrs: norecurse nounwind readonly uwtable
declare i64 @_Z11RankAttacksyj(i64, i32) #3

; Function Attrs: norecurse nounwind readnone uwtable
declare zeroext i16 @_Z12compact_movei(i32) #4

; Function Attrs: norecurse nounwind readnone uwtable
declare i32 @_Z4logLi(i32) #4

; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z12gen_capturesP7state_tPi(%struct.state_t*, i32*) #0

; Function Attrs: norecurse nounwind readonly uwtable
declare i64 @_Z11RookAttacksP7state_ti(%struct.state_t* nocapture readonly, i32) #3

; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t*, i32*, i32) #0

; Function Attrs: norecurse nounwind readonly uwtable
declare i64 @_Z13BishopAttacksP7state_ti(%struct.state_t* nocapture readonly, i32) #3

; Function Attrs: norecurse nounwind readonly uwtable
declare i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nocapture readonly) #3

; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z15gen_good_checksP7state_tPi(%struct.state_t*, i32*) #0

; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z3genP7state_tPi(%struct.state_t*, i32*) #0

; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z3seeP7state_tiiii(%struct.state_t*, i32, i32, i32, i32) #0

; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z4evalP7state_tiii(%struct.state_t*, i32, i32, i32) #0

; Function Attrs: nounwind readnone speculatable willreturn
declare void @_Z4makeP7state_ti(%struct.state_t*, i32) #5

; Function Attrs: nounwind readnone speculatable willreturn
declare void @_Z6unmakeP7state_ti(%struct.state_t*, i32) #5

; Function Attrs: nounwind readnone speculatable willreturn
declare void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nocapture, i32, i32, i32, i32, i32, i32, i32, i32) #5

; Function Attrs: norecurse nounwind readonly uwtable
declare i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nocapture readonly, %struct.state_t* nocapture readonly) #3

declare i32 @_Z8in_checkP7state_t(%struct.state_t*)

declare i32 @_Z9gen_quietP7state_tPi(%struct.state_t*, i32* nonnull)

declare void @_ZL10PrefetchL3P7state_t(i32, i64)

declare void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t*, i32* nonnull, i32* nonnull, i32, i32)

declare void @_ZL12history_goodP7state_tii(%struct.state_t* nonnull, i32, i32)

declare i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull, i32* nonnull, i32* nonnull, i32)

declare void @_ZL16fast_order_movesP7state_tPiS1_ij(%struct.state_t*, i32* nonnull, i32* nonnull, i32, i32)

declare i32 @_ZL17search_time_checkP7state_t(i64)

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #6

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #6

; Function Attrs: nofree nounwind uwtable
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #7

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local i32 @main() local_unnamed_addr #4 {
entry:
  %puts = tail call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([20 x i8], [20 x i8]* @.str.4, i64 0, i64 0))
  ret i32 0
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #5 = { nounwind readnone speculatable willreturn }
attributes #6 = { argmemonly nounwind willreturn }
attributes #7 = { nofree nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #8 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !5, i64 4356}
!9 = !{!"struct@_ZTS7state_t", !5, i64 0, !10, i64 4, !11, i64 264, !11, i64 272, !11, i64 280, !12, i64 288, !5, i64 392, !5, i64 396, !13, i64 400, !5, i64 452, !5, i64 456, !5, i64 460, !5, i64 464, !5, i64 468, !5, i64 472, !5, i64 476, !11, i64 480, !11, i64 488, !14, i64 496, !10, i64 2544, !10, i64 2800, !16, i64 3056, !11, i64 4080, !11, i64 4088, !5, i64 4096, !10, i64 4100, !5, i64 4356, !5, i64 4360, !5, i64 4364, !5, i64 4368, !5, i64 4372, !5, i64 4376, !5, i64 4380, !5, i64 4384, !5, i64 4388, !5, i64 4392, !18, i64 4400}
!10 = !{!"array@_ZTSA64_i", !5, i64 0}
!11 = !{!"long long", !6, i64 0}
!12 = !{!"array@_ZTSA13_y", !11, i64 0}
!13 = !{!"array@_ZTSA13_i", !5, i64 0}
!14 = !{!"array@_ZTSA64_6move_x", !15, i64 0}
!15 = !{!"struct@_ZTS6move_x", !5, i64 0, !5, i64 4, !5, i64 8, !5, i64 12, !11, i64 16, !11, i64 24}
!16 = !{!"array@_ZTSA64_N7state_tUt_E", !17, i64 0}
!17 = !{!"struct@_ZTSN7state_tUt_E", !5, i64 0, !5, i64 4, !5, i64 8, !5, i64 12}
!18 = !{!"array@_ZTSA1000_y", !11, i64 0}
!19 = !{!9, !5, i64 460}
!20 = !{!9, !11, i64 480}
!21 = !{!22, !22, i64 0}
!22 = !{!"pointer@_ZTSP9ttentry_t", !6, i64 0}
!23 = !{!24, !25, i64 0}
!24 = !{!"struct@_ZTS9ttentry_t", !25, i64 0}
!25 = !{!"array@_ZTSA4_10ttbucket_t", !26, i64 0}
!26 = !{!"struct@_ZTS10ttbucket_t", !5, i64 0, !27, i64 4, !27, i64 6, !6, i64 8, !6, i64 9, !6, i64 9, !6, i64 9, !6, i64 9, !6, i64 9}
!27 = !{!"short", !6, i64 0}
!28 = !{!25, !26, i64 0}
!29 = !{!26, !5, i64 0}
!30 = !{!24, !5, i64 0}
!31 = !{!9, !5, i64 4360}
!32 = !{!26, !6, i64 9}
!33 = !{!26, !6, i64 8}
!34 = !{!24, !6, i64 8}
!35 = !{!26, !27, i64 4}
!36 = !{!24, !27, i64 4}
!37 = !{!9, !5, i64 472}
!38 = !{!26, !27, i64 6}
!39 = !{!24, !27, i64 6}
!40 = !{!9, !11, i64 4080}
!41 = !{!9, !5, i64 476}
!42 = !{!43, !5, i64 12}
!43 = !{!"struct@_ZTS11gamestate_t", !5, i64 0, !5, i64 4, !5, i64 8, !5, i64 12, !5, i64 16, !5, i64 20, !5, i64 24, !5, i64 28, !5, i64 32, !5, i64 36, !5, i64 40, !5, i64 44, !5, i64 48, !5, i64 52, !5, i64 56, !5, i64 60, !44, i64 64, !45, i64 4064, !11, i64 36064, !5, i64 36072, !5, i64 36076, !5, i64 36080, !5, i64 36084, !5, i64 36088, !5, i64 36092, !5, i64 36096, !5, i64 36100}
!44 = !{!"array@_ZTSA1000_i", !5, i64 0}
!45 = !{!"array@_ZTSA1000_6move_x", !15, i64 0}
!46 = !{!9, !10, i64 4100}
!47 = !{!10, !5, i64 0}
!48 = !{!9, !5, i64 4100}
!49 = !{!9, !13, i64 400}
!50 = !{!13, !5, i64 0}
!51 = !{!9, !5, i64 400}
!52 = !{!43, !5, i64 4}
!53 = !{!43, !5, i64 36096}
!54 = !{!9, !5, i64 456}
!55 = !{!9, !10, i64 2544}
!56 = !{!9, !5, i64 2544}
!57 = !{!9, !10, i64 2800}
!58 = !{!9, !5, i64 2800}
!59 = !{!60, !5, i64 0}
!60 = !{!"array@_ZTSA240_i", !5, i64 0}
!61 = !{!9, !5, i64 4}
!62 = !{!63, !5, i64 0}
!63 = !{!"array@_ZTSA14_i", !5, i64 0}
!64 = !{!43, !5, i64 60}
!65 = !{!18, !11, i64 0}
!66 = !{!9, !11, i64 4400}
!67 = !{!43, !5, i64 20}
!68 = !{!9, !5, i64 0}
!69 = !{!70, !71, i64 0}
!70 = !{!"array@_ZTSA8_A12_A64_i", !71, i64 0}
!71 = !{!"array@_ZTSA12_A64_i", !10, i64 0}
!72 = !{!71, !10, i64 0}
!73 = !{!70, !5, i64 0}
!74 = !{!9, !11, i64 4088}
!75 = !{!9, !5, i64 4096}
