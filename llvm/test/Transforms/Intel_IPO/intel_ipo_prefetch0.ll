; Checks that IPO Prefetch pass can properly identify the prefetch opportunity, generate the prefetch
; function, and generate 2 calls to prefetch function in 2 host functions: 1 call to prefetch inside
; each host.
;
; [Note]
; The IPO prefetch pass is a module pass that specifically examines code patterns like those exhibited in
; cpu2017/531.deepsjeng (531), generate a prefetch function based on one of the existing functions in 531
; and insert multiple calls to this prefetch function at specific pattern-matched host locations.
;
; The LIT testcase presented here is a vastly simplified version of 531.
; It takes the following 3 functions after all modules have been linked:
; - _Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(): an existing function that the prefetch function is based upon;
; - _Z7qsearchP7state_tiiii(): host function1 where a call to prefetch function will be inserted;
; - _Z6searchP7state_tiiiii(): host function2 where a call to prefetch function will be inserted;
;
; Only the 3 functions listed above, plus any global data and metadata they use, will appear in this LIT test.
; All other functions are reduced to declarations only. Their function bodies are purged, so are any global variable
; or metadata that are not used.
;

; REQUIRES: asserts

; *** Run command section ***
; RUN: opt < %s -intel-ipoprefetch -ipo-prefetch-be-lit-friendly=1 -ipo-prefetch-suppress-inline-report=0 -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(intel-ipoprefetch)' -ipo-prefetch-be-lit-friendly=1 -ipo-prefetch-suppress-inline-report=0 -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -S 2>&1 | FileCheck %s
;

; *** Check section 1 ***
; The LLVM-IR check below ensure that a call to the Prefetch.Backbone function is inserted inside host
; _Z6searchP7state_tiiiii.
; CHECK: define internal i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5) #{{.}} {
; CHECK: call void @Prefetch.Backbone(%struct.state_t* %0)
;

; *** Check section 2 ***
; The LLVM-IR check below ensure that a call to the Prefetch.Backbone function is inserted inside host
; _Z7qsearchP7state_tiiii.
; CHECK:define internal i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4) #{{.}} {
; CHECK: call void @Prefetch.Backbone(%struct.state_t* %0)
;

; *** Check section 3 ***
; The LLVM-IR check below ensures the prefetch function is generated.
; CHECK: define internal void @Prefetch.Backbone(%struct.state_t* nocapture %0) #{{.}} {
;

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

; Function Attrs: nounwind uwtable
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #0
; Function Attrs: uwtable
declare i32 @_Z11check_legalP7state_ti(%struct.state_t*, i32) #1
; Function Attrs: uwtable
declare i32 @_Z11is_attackedP7state_tii(%struct.state_t* nocapture readonly, i32, i32) #1
; Function Attrs: norecurse nounwind readonly uwtable
declare i64 @_Z11FileAttacksyj(i64, i32) #2
; Function Attrs: norecurse nounwind readonly uwtable
declare i64 @_Z11RankAttacksyj(i64, i32) #2
; Function Attrs: norecurse nounwind readnone uwtable
declare zeroext i16 @_Z12compact_movei(i32) #3
; Function Attrs: norecurse nounwind readnone uwtable
declare i32 @_Z4logLi(i32) #3
; Function Attrs: uwtable
declare i32 @_Z12gen_capturesP7state_tPi(%struct.state_t*, i32*) #1
; Function Attrs: norecurse nounwind readonly uwtable
declare i64 @_Z11RookAttacksP7state_ti(%struct.state_t* nocapture readonly, i32) #2
; Function Attrs: uwtable
declare i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t*, i32*, i32) #1
; Function Attrs: norecurse nounwind readonly uwtable
declare i64 @_Z13BishopAttacksP7state_ti(%struct.state_t* nocapture readonly, i32) #2
; Function Attrs: norecurse nounwind readonly uwtable
declare i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nocapture readonly) #2
; Function Attrs: uwtable
declare i32 @_Z15gen_good_checksP7state_tPi(%struct.state_t*, i32*) #1
; Function Attrs: uwtable
declare i32 @_Z3genP7state_tPi(%struct.state_t*, i32*) #1
; Function Attrs: uwtable
declare i32 @_Z3seeP7state_tiiii(%struct.state_t*, i32, i32, i32, i32) #1
; Function Attrs: uwtable
declare i32 @_Z4evalP7state_tiii(%struct.state_t*, i32, i32, i32) #1
; Function Attrs: nounwind readnone speculatable willreturn
declare void @_Z4makeP7state_ti(%struct.state_t*, i32) #4
; Function Attrs: nounwind readnone speculatable willreturn
declare void @_Z6unmakeP7state_ti(%struct.state_t*, i32) #4
; Function Attrs: nounwind readnone speculatable willreturn
declare void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nocapture, i32, i32, i32, i32, i32, i32, i32, i32) #4
; Function Attrs: norecurse nounwind readonly uwtable
declare i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nocapture readonly, %struct.state_t* nocapture readonly) #2
declare i32 @_Z8in_checkP7state_t(%struct.state_t*)
declare i32 @_Z9gen_quietP7state_tPi(%struct.state_t*, i32* nonnull)
declare void @_ZL10PrefetchL3P7state_t(i32, i64)
declare void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t*, i32* nonnull, i32* nonnull, i32, i32)
declare void @_ZL12history_goodP7state_tii(%struct.state_t* nonnull, i32, i32)
declare i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull, i32* nonnull, i32* nonnull, i32)
declare void @_ZL16fast_order_movesP7state_tPiS1_ij(%struct.state_t*, i32* nonnull, i32* nonnull, i32, i32)
declare i32 @_ZL17search_time_checkP7state_t(i64)
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #5
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #5
; Function Attrs: nofree nounwind uwtable
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #6
; Function Attrs: norecurse nounwind readnone uwtable
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %puts = tail call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([20 x i8], [20 x i8]* @.str.4, i64 0, i64 0))
  ret i32 0
}
; Function Attrs: nofree norecurse nounwind uwtable
define internal i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nocapture %0, i32* nocapture %1, i32 %2, i32 %3, i32* nocapture %4, i32* nocapture %5, i32* nocapture %6, i32* nocapture %7, i32* nocapture %8, i32 %9) #7 {
  store i32 1, i32* %6, align 4, !tbaa !5
  %11 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 26, !intel-tbaa !9
  %12 = load i32, i32* %11, align 4, !tbaa !9
  %13 = add i32 %12, 1
  store i32 %13, i32* %11, align 4, !tbaa !9
  %14 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !20
  %15 = load i32, i32* %14, align 4, !tbaa !20
  %16 = icmp eq i32 %15, 0
  %17 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %18 = load i64, i64* %17, align 8, !tbaa !21
  %19 = zext i1 %16 to i64
  %20 = add i64 %18, %19
  %21 = trunc i64 %20 to i32
  %22 = load %struct.ttentry_t*, %struct.ttentry_t** @TTable, align 8, !tbaa !22
  %23 = load i32, i32* @TTSize, align 4, !tbaa !5
  %24 = urem i32 %21, %23
  %25 = zext i32 %24 to i64
  %26 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %22, i64 %25
  %27 = lshr i64 %20, 32
  %28 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %26, i64 0, i32 0, !intel-tbaa !24
  %29 = trunc i64 %27 to i32
  br label %33

30:                                               ; preds = %33
  %31 = add nuw nsw i64 %34, 1
  %32 = icmp eq i64 %31, 4
  br i1 %32, label %132, label %33

33:                                               ; preds = %30, %10
  %34 = phi i64 [ 0, %10 ], [ %31, %30 ]
  %35 = getelementptr inbounds [4 x %struct.ttbucket_t], [4 x %struct.ttbucket_t]* %28, i64 0, i64 %34, !intel-tbaa !29
  %36 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 0, !intel-tbaa !30
  %37 = load i32, i32* %36, align 4, !tbaa !31
  %38 = icmp eq i32 %37, %29
  br i1 %38, label %39, label %30

39:                                               ; preds = %33
  %40 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 27, !intel-tbaa !32
  %41 = load i32, i32* %40, align 8, !tbaa !32
  %42 = add i32 %41, 1
  store i32 %42, i32* %40, align 8, !tbaa !32
  %43 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 4
  %44 = load i8, i8* %43, align 1, !tbaa !33
  %45 = lshr i8 %44, 5
  %46 = and i8 %45, 3
  %47 = zext i8 %46 to i32
  %48 = load i32, i32* @TTAge, align 4, !tbaa !5
  %49 = icmp eq i32 %48, %47
  br i1 %49, label %56, label %50

50:                                               ; preds = %39
  %51 = trunc i32 %48 to i8
  %52 = shl i8 %51, 5
  %53 = and i8 %52, 96
  %54 = and i8 %44, -97
  %55 = or i8 %53, %54
  store i8 %55, i8* %43, align 1, !tbaa !33
  br label %56

56:                                               ; preds = %50, %39
  %57 = phi i8 [ %44, %39 ], [ %55, %50 ]
  %58 = and i8 %57, 6
  %59 = icmp eq i8 %58, 2
  br i1 %59, label %60, label %72

60:                                               ; preds = %56
  %61 = add nsw i32 %9, -16
  %62 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 3, !intel-tbaa !34
  %63 = load i8, i8* %62, align 4, !tbaa !35
  %64 = zext i8 %63 to i32
  %65 = icmp sgt i32 %61, %64
  br i1 %65, label %72, label %66

66:                                               ; preds = %60
  %67 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !intel-tbaa !36
  %68 = load i16, i16* %67, align 4, !tbaa !37
  %69 = sext i16 %68 to i32
  %70 = icmp slt i32 %69, %3
  br i1 %70, label %71, label %72

71:                                               ; preds = %66
  store i32 0, i32* %6, align 4, !tbaa !5
  br label %72

72:                                               ; preds = %71, %66, %60, %56
  %73 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 3, !intel-tbaa !34
  %74 = load i8, i8* %73, align 4, !tbaa !35
  %75 = zext i8 %74 to i32
  %76 = icmp slt i32 %75, %9
  br i1 %76, label %109, label %77

77:                                               ; preds = %72
  %78 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !intel-tbaa !36
  %79 = load i16, i16* %78, align 4, !tbaa !37
  %80 = sext i16 %79 to i32
  store i32 %80, i32* %1, align 4, !tbaa !5
  %81 = icmp sgt i16 %79, 31500
  br i1 %81, label %82, label %87

82:                                               ; preds = %77
  %83 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !38
  %84 = load i32, i32* %83, align 8, !tbaa !38
  %85 = add nsw i32 %80, 1
  %86 = sub i32 %85, %84
  store i32 %86, i32* %1, align 4, !tbaa !5
  br label %94

87:                                               ; preds = %77
  %88 = icmp slt i16 %79, -31500
  br i1 %88, label %89, label %94

89:                                               ; preds = %87
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !38
  %91 = load i32, i32* %90, align 8, !tbaa !38
  %92 = add nsw i32 %80, -1
  %93 = add i32 %92, %91
  store i32 %93, i32* %1, align 4, !tbaa !5
  br label %94

94:                                               ; preds = %89, %87, %82
  %95 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 2, !intel-tbaa !39
  %96 = load i16, i16* %95, align 2, !tbaa !40
  %97 = zext i16 %96 to i32
  store i32 %97, i32* %4, align 4, !tbaa !5
  %98 = and i8 %57, 1
  %99 = zext i8 %98 to i32
  store i32 %99, i32* %5, align 4, !tbaa !5
  %100 = lshr i8 %57, 3
  %101 = and i8 %100, 1
  %102 = zext i8 %101 to i32
  store i32 %102, i32* %7, align 4, !tbaa !5
  %103 = lshr i8 %57, 4
  %104 = and i8 %103, 1
  %105 = zext i8 %104 to i32
  store i32 %105, i32* %8, align 4, !tbaa !5
  %106 = lshr i8 %57, 1
  %107 = and i8 %106, 3
  %108 = zext i8 %107 to i32
  br label %132

109:                                              ; preds = %72
  %110 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 2, !intel-tbaa !39
  %111 = load i16, i16* %110, align 2, !tbaa !40
  %112 = zext i16 %111 to i32
  store i32 %112, i32* %4, align 4, !tbaa !5
  %113 = and i8 %57, 1
  %114 = zext i8 %113 to i32
  store i32 %114, i32* %5, align 4, !tbaa !5
  %115 = lshr i8 %57, 3
  %116 = and i8 %115, 1
  %117 = zext i8 %116 to i32
  store i32 %117, i32* %7, align 4, !tbaa !5
  %118 = lshr i8 %57, 4
  %119 = and i8 %118, 1
  %120 = zext i8 %119 to i32
  store i32 %120, i32* %8, align 4, !tbaa !5
  %121 = lshr i8 %57, 1
  %122 = and i8 %121, 3
  switch i8 %122, label %125 [
    i8 1, label %123
    i8 2, label %124
  ]

123:                                              ; preds = %109
  store i32 -1000000, i32* %1, align 4, !tbaa !5
  br label %129

124:                                              ; preds = %109
  store i32 1000000, i32* %1, align 4, !tbaa !5
  br label %129

125:                                              ; preds = %109
  %126 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !intel-tbaa !36
  %127 = load i16, i16* %126, align 4, !tbaa !37
  %128 = sext i16 %127 to i32
  store i32 %128, i32* %1, align 4, !tbaa !5
  br label %129

129:                                              ; preds = %125, %124, %123
  %130 = trunc i32 %9 to i8
  store i8 %130, i8* %73, align 4, !tbaa !35
  %131 = and i8 %57, -7
  store i8 %131, i8* %43, align 1, !tbaa !33
  br label %132

132:                                              ; preds = %129, %94, %30
  %133 = phi i32 [ %108, %94 ], [ 0, %129 ], [ 4, %30 ]
  ret i32 %133
}
; Function Attrs: uwtable
define internal i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5) #1 {
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
  %31 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !38
  %32 = load i32, i32* %31, align 8, !tbaa !38
  %33 = icmp sgt i32 %32, 59
  br i1 %33, label %34, label %36

34:                                               ; preds = %30, %6
  %35 = tail call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 0, i32 0)
  br label %1119

36:                                               ; preds = %30
  %37 = getelementptr %struct.state_t, %struct.state_t* %0, i64 0, i32 22, !intel-tbaa !41
  %38 = load i64, i64* %37, align 8, !tbaa !41
  %39 = add i64 %38, 1
  store i64 %39, i64* %37, align 8, !tbaa !41
  %40 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %39)
  %41 = icmp eq i32 %40, 0
  br i1 %41, label %42, label %1119

42:                                               ; preds = %36
  %43 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nonnull @gamestate, %struct.state_t* nonnull %0)
  %44 = icmp eq i32 %43, 0
  br i1 %44, label %45, label %49

45:                                               ; preds = %42
  %46 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15, !intel-tbaa !42
  %47 = load i32, i32* %46, align 4, !tbaa !42
  %48 = icmp sgt i32 %47, 99
  br i1 %48, label %49, label %57

49:                                               ; preds = %45, %42
  %50 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4, !tbaa !43
  %51 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !20
  %52 = load i32, i32* %51, align 4, !tbaa !20
  %53 = icmp eq i32 %50, %52
  %54 = load i32, i32* @contempt, align 4, !tbaa !5
  %55 = sub nsw i32 0, %54
  %56 = select i1 %53, i32 %54, i32 %55
  br label %1119

57:                                               ; preds = %45
  %58 = load i32, i32* %31, align 8, !tbaa !38
  %59 = add nsw i32 %58, -32000
  %60 = icmp sgt i32 %59, %1
  br i1 %60, label %61, label %63

61:                                               ; preds = %57
  %62 = icmp slt i32 %59, %2
  br i1 %62, label %63, label %1119

63:                                               ; preds = %61, %57
  %64 = phi i32 [ %59, %61 ], [ %1, %57 ]
  %65 = sub i32 31999, %58
  %66 = icmp slt i32 %65, %2
  br i1 %66, label %67, label %69

67:                                               ; preds = %63
  %68 = icmp sgt i32 %65, %64
  br i1 %68, label %69, label %1119

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
  %73 = load i32, i32* %10, align 4, !tbaa !5
  br label %1119

74:                                               ; preds = %69
  %75 = load i32, i32* %10, align 4, !tbaa !5
  %76 = icmp sgt i32 %75, %64
  br i1 %76, label %85, label %1119

77:                                               ; preds = %69
  %78 = load i32, i32* %10, align 4, !tbaa !5
  %79 = icmp slt i32 %78, %70
  br i1 %79, label %85, label %1119

80:                                               ; preds = %69
  %81 = load i32, i32* %10, align 4, !tbaa !5
  %82 = icmp slt i32 %81, %70
  %83 = select i1 %82, i32 %5, i32 1
  br label %85

84:                                               ; preds = %69
  store i32 65535, i32* %13, align 4, !tbaa !5
  store i32 0, i32* %11, align 4, !tbaa !5
  store i32 0, i32* %14, align 4, !tbaa !5
  store i32 0, i32* %15, align 4, !tbaa !5
  br label %85

85:                                               ; preds = %84, %80, %77, %74, %69
  %86 = phi i32 [ %5, %69 ], [ %5, %84 ], [ 0, %74 ], [ 1, %77 ], [ %83, %80 ]
  %87 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25, !intel-tbaa !47
  %88 = load i32, i32* %31, align 8, !tbaa !38
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %89, !intel-tbaa !48
  %91 = load i32, i32* %90, align 4, !tbaa !49
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
  %104 = load i32, i32* %13, align 4, !tbaa !5
  %105 = load i32, i32* %11, align 4, !tbaa !5
  %106 = load i32, i32* %14, align 4, !tbaa !5
  %107 = load i32, i32* %15, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %101, i32 %64, i32 %70, i32 %104, i32 %105, i32 %106, i32 %107, i32 %3)
  br label %1119

108:                                              ; preds = %100
  %109 = icmp slt i32 %92, %70
  br i1 %109, label %110, label %122

110:                                              ; preds = %108
  %111 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %64, i32 %70, i32 0, i32 0)
  br label %1119

112:                                              ; preds = %98
  %113 = icmp slt i32 %3, 9
  br i1 %113, label %114, label %122

114:                                              ; preds = %112
  %115 = add nsw i32 %92, -125
  %116 = icmp slt i32 %115, %70
  br i1 %116, label %122, label %117

117:                                              ; preds = %114
  %118 = load i32, i32* %13, align 4, !tbaa !5
  %119 = load i32, i32* %11, align 4, !tbaa !5
  %120 = load i32, i32* %14, align 4, !tbaa !5
  %121 = load i32, i32* %15, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %115, i32 %64, i32 %70, i32 %118, i32 %119, i32 %120, i32 %121, i32 %3)
  br label %1119

122:                                              ; preds = %114, %112, %108, %85
  %123 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8, !intel-tbaa !50
  %124 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 9, !intel-tbaa !51
  %125 = load i32, i32* %124, align 4, !tbaa !52
  %126 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 7, !intel-tbaa !51
  %127 = load i32, i32* %126, align 4, !tbaa !52
  %128 = add nsw i32 %127, %125
  %129 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 11, !intel-tbaa !51
  %130 = load i32, i32* %129, align 4, !tbaa !52
  %131 = add nsw i32 %128, %130
  %132 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 3, !intel-tbaa !51
  %133 = load i32, i32* %132, align 4, !tbaa !52
  %134 = add nsw i32 %131, %133
  %135 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 10, !intel-tbaa !51
  %136 = load i32, i32* %135, align 8, !tbaa !52
  %137 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 8, !intel-tbaa !51
  %138 = load i32, i32* %137, align 8, !tbaa !52
  %139 = add nsw i32 %138, %136
  %140 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 12, !intel-tbaa !51
  %141 = load i32, i32* %140, align 8, !tbaa !52
  %142 = add nsw i32 %139, %141
  %143 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 4, !intel-tbaa !51
  %144 = load i32, i32* %143, align 8, !tbaa !52
  %145 = add nsw i32 %142, %144
  store i32 0, i32* %11, align 4, !tbaa !5
  %146 = icmp eq i32 %4, 0
  br i1 %146, label %147, label %231

147:                                              ; preds = %122
  %148 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !20
  %149 = load i32, i32* %148, align 4, !tbaa !20
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
  %162 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !tbaa !53
  %163 = icmp eq i32 %162, 2
  br i1 %163, label %164, label %183

164:                                              ; preds = %161
  %165 = icmp slt i32 %3, 25
  %166 = add nsw i32 %70, -1
  br i1 %165, label %167, label %169

167:                                              ; preds = %164
  %168 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %166, i32 %70, i32 0, i32 0)
  br label %172

169:                                              ; preds = %164
  %170 = add nsw i32 %3, -24
  %171 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %166, i32 %70, i32 %170, i32 1, i32 %86)
  br label %172

172:                                              ; preds = %169, %167
  %173 = phi i32 [ %168, %167 ], [ %171, %169 ]
  %174 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !54
  %175 = icmp eq i32 %174, 0
  br i1 %175, label %176, label %1119

176:                                              ; preds = %172
  %177 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !tbaa !53
  %178 = icmp eq i32 %177, 2
  %179 = icmp slt i32 %173, %70
  %180 = and i1 %179, %178
  br i1 %180, label %248, label %181

181:                                              ; preds = %176
  %182 = load i32, i32* %148, align 4, !tbaa !20
  br label %183

183:                                              ; preds = %181, %161
  %184 = phi i32 [ %182, %181 ], [ %149, %161 ]
  %185 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 10, !intel-tbaa !55
  %186 = load i32, i32* %185, align 8, !tbaa !55
  store i32 0, i32* %185, align 8, !tbaa !55
  %187 = xor i32 %184, 1
  store i32 %187, i32* %148, align 4, !tbaa !20
  %188 = load i32, i32* %31, align 8, !tbaa !38
  %189 = add nsw i32 %188, 1
  store i32 %189, i32* %31, align 8, !tbaa !38
  %190 = load i32, i32* %46, align 4, !tbaa !42
  %191 = add nsw i32 %190, 1
  store i32 %191, i32* %46, align 4, !tbaa !42
  %192 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19, !intel-tbaa !56
  %193 = sext i32 %188 to i64
  %194 = getelementptr inbounds [64 x i32], [64 x i32]* %192, i64 0, i64 %193, !intel-tbaa !48
  store i32 0, i32* %194, align 4, !tbaa !57
  %195 = sext i32 %189 to i64
  %196 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %195, !intel-tbaa !48
  store i32 0, i32* %196, align 4, !tbaa !49
  %197 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 20, !intel-tbaa !58
  %198 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %193, !intel-tbaa !48
  %199 = load i32, i32* %198, align 4, !tbaa !59
  %200 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %195, !intel-tbaa !48
  store i32 %199, i32* %200, align 4, !tbaa !59
  %201 = icmp slt i32 %3, 17
  %202 = sub nsw i32 0, %70
  %203 = sub i32 1, %70
  br i1 %201, label %204, label %206

204:                                              ; preds = %183
  %205 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %202, i32 %203, i32 0, i32 0)
  br label %211

206:                                              ; preds = %183
  %207 = add nsw i32 %3, -16
  %208 = icmp eq i32 %86, 0
  %209 = zext i1 %208 to i32
  %210 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %202, i32 %203, i32 %207, i32 1, i32 %209)
  br label %211

211:                                              ; preds = %206, %204
  %212 = phi i32 [ %205, %204 ], [ %210, %206 ]
  %213 = sub nsw i32 0, %212
  %214 = load i32, i32* %46, align 4, !tbaa !42
  %215 = add nsw i32 %214, -1
  store i32 %215, i32* %46, align 4, !tbaa !42
  %216 = load i32, i32* %31, align 8, !tbaa !38
  %217 = add nsw i32 %216, -1
  store i32 %217, i32* %31, align 8, !tbaa !38
  %218 = load i32, i32* %148, align 4, !tbaa !20
  %219 = xor i32 %218, 1
  store i32 %219, i32* %148, align 4, !tbaa !20
  store i32 %186, i32* %185, align 8, !tbaa !55
  %220 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !54
  %221 = icmp eq i32 %220, 0
  br i1 %221, label %222, label %1119

222:                                              ; preds = %211
  %223 = icmp sgt i32 %70, %213
  br i1 %223, label %228, label %224

224:                                              ; preds = %222
  %225 = load i32, i32* %13, align 4, !tbaa !5
  %226 = load i32, i32* %11, align 4, !tbaa !5
  %227 = load i32, i32* %15, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %213, i32 %64, i32 %70, i32 %225, i32 %226, i32 0, i32 %227, i32 %3)
  br label %1119

228:                                              ; preds = %222
  %229 = icmp sgt i32 %212, 31400
  br i1 %229, label %230, label %248

230:                                              ; preds = %228
  store i32 1, i32* %11, align 4, !tbaa !5
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
  %239 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !54
  %240 = icmp eq i32 %239, 0
  br i1 %240, label %241, label %1119

241:                                              ; preds = %237
  %242 = icmp sgt i32 %238, %64
  br i1 %242, label %248, label %243

243:                                              ; preds = %241
  %244 = load i32, i32* %13, align 4, !tbaa !5
  %245 = load i32, i32* %11, align 4, !tbaa !5
  %246 = load i32, i32* %14, align 4, !tbaa !5
  %247 = load i32, i32* %15, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %64, i32 %64, i32 %70, i32 %244, i32 %245, i32 %246, i32 %247, i32 %3)
  br label %1119

248:                                              ; preds = %241, %231, %230, %228, %176
  %249 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 0
  br i1 %93, label %250, label %255

250:                                              ; preds = %248
  %251 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %249, i32 %91)
  %252 = icmp eq i32 %251, 0
  br i1 %252, label %280, label %253

253:                                              ; preds = %250
  store i32 0, i32* %9, align 4, !tbaa !5
  %254 = icmp sgt i32 %251, 0
  br i1 %254, label %257, label %280

255:                                              ; preds = %248
  %256 = call i32 @_Z3genP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %249)
  br label %280

257:                                              ; preds = %257, %253
  %258 = phi i32 [ %270, %257 ], [ 0, %253 ]
  %259 = phi i32 [ %276, %257 ], [ 0, %253 ]
  %260 = sext i32 %259 to i64
  %261 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %260, !intel-tbaa !60
  %262 = load i32, i32* %261, align 4, !tbaa !60
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %262)
  %263 = load i32, i32* %9, align 4, !tbaa !5
  %264 = sext i32 %263 to i64
  %265 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %264, !intel-tbaa !60
  %266 = load i32, i32* %265, align 4, !tbaa !60
  %267 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %266)
  %268 = icmp ne i32 %267, 0
  %269 = zext i1 %268 to i32
  %270 = add nuw nsw i32 %258, %269
  %271 = load i32, i32* %9, align 4, !tbaa !5
  %272 = sext i32 %271 to i64
  %273 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %272, !intel-tbaa !60
  %274 = load i32, i32* %273, align 4, !tbaa !60
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %274)
  %275 = load i32, i32* %9, align 4, !tbaa !5
  %276 = add nsw i32 %275, 1
  store i32 %276, i32* %9, align 4, !tbaa !5
  %277 = icmp slt i32 %276, %251
  %278 = icmp ult i32 %270, 2
  %279 = and i1 %277, %278
  br i1 %279, label %257, label %280

280:                                              ; preds = %257, %255, %253, %250
  %281 = phi i32 [ 0, %250 ], [ %256, %255 ], [ %251, %253 ], [ %251, %257 ]
  %282 = phi i32 [ 0, %250 ], [ %256, %255 ], [ 0, %253 ], [ %270, %257 ]
  %283 = getelementptr inbounds [240 x i32], [240 x i32]* %8, i64 0, i64 0
  %284 = load i32, i32* %13, align 4, !tbaa !5
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
  store i32 0, i32* %9, align 4, !tbaa !5
  %292 = icmp sgt i32 %281, 0
  br i1 %292, label %293, label %324

293:                                              ; preds = %291
  %294 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %295 = zext i32 %281 to i64
  br label %296

296:                                              ; preds = %318, %293
  %297 = phi i64 [ 0, %293 ], [ %320, %318 ]
  %298 = phi i32 [ 0, %293 ], [ %319, %318 ]
  %299 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %297, !intel-tbaa !60
  %300 = load i32, i32* %299, align 4, !tbaa !60
  %301 = lshr i32 %300, 19
  %302 = and i32 %301, 15
  %303 = icmp eq i32 %302, 13
  br i1 %303, label %318, label %304

304:                                              ; preds = %296
  %305 = lshr i32 %300, 6
  %306 = and i32 %305, 63
  %307 = zext i32 %306 to i64
  %308 = getelementptr inbounds [64 x i32], [64 x i32]* %294, i64 0, i64 %307, !intel-tbaa !48
  %309 = load i32, i32* %308, align 4, !tbaa !62
  %310 = sext i32 %309 to i64
  %311 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %310, !intel-tbaa !63
  %312 = load i32, i32* %311, align 4, !tbaa !63
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
  store i32 %281, i32* %9, align 4, !tbaa !5
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
  %332 = load i32, i32* %18, align 4, !tbaa !5
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %249, i32* nonnull %283, i32 %281, i32 %332)
  br label %335

333:                                              ; preds = %324
  %334 = load i32, i32* %13, align 4, !tbaa !5
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
  %345 = load i32, i32* %31, align 8, !tbaa !38
  %346 = add nsw i32 %345, -1
  %347 = sext i32 %346 to i64
  %348 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %347, !intel-tbaa !48
  %349 = load i32, i32* %348, align 4, !tbaa !49
  %350 = icmp eq i32 %349, 0
  br i1 %350, label %351, label %508

351:                                              ; preds = %344
  %352 = icmp slt i32 %345, 3
  br i1 %352, label %367, label %353

353:                                              ; preds = %351
  %354 = add nsw i32 %345, -2
  %355 = sext i32 %354 to i64
  %356 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %355, !intel-tbaa !48
  %357 = load i32, i32* %356, align 4, !tbaa !49
  %358 = icmp eq i32 %357, 0
  br i1 %358, label %359, label %508

359:                                              ; preds = %353
  %360 = icmp slt i32 %345, 4
  br i1 %360, label %367, label %361

361:                                              ; preds = %359
  %362 = add nsw i32 %345, -3
  %363 = sext i32 %362 to i64
  %364 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %363, !intel-tbaa !48
  %365 = load i32, i32* %364, align 4, !tbaa !49
  %366 = icmp eq i32 %365, 0
  br i1 %366, label %367, label %508

367:                                              ; preds = %361, %359, %351
  store i32 -1, i32* %9, align 4, !tbaa !5
  %368 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %369 = icmp eq i32 %368, 0
  br i1 %369, label %508, label %370

370:                                              ; preds = %367
  %371 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %372 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %373 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %374 = sub nsw i32 0, %70
  %375 = sub i32 50, %64
  %376 = icmp sgt i32 %3, 16
  %377 = icmp slt i32 %3, 17
  %378 = sub i32 1, %70
  %379 = add nsw i32 %3, -16
  %380 = icmp eq i32 %86, 0
  %381 = zext i1 %380 to i32
  br i1 %377, label %382, label %443

382:                                              ; preds = %437, %370
  %383 = phi i32 [ %438, %437 ], [ 0, %370 ]
  %384 = phi i32 [ %386, %437 ], [ 0, %370 ]
  %385 = phi i32 [ %423, %437 ], [ -32000, %370 ]
  %386 = add nuw nsw i32 %384, 1
  %387 = load i32, i32* %9, align 4, !tbaa !5
  %388 = sext i32 %387 to i64
  %389 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %388, !intel-tbaa !60
  %390 = load i32, i32* %389, align 4, !tbaa !60
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %390)
  %391 = load i32, i32* %9, align 4, !tbaa !5
  %392 = sext i32 %391 to i64
  %393 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %392, !intel-tbaa !60
  %394 = load i32, i32* %393, align 4, !tbaa !60
  %395 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %394)
  %396 = icmp eq i32 %395, 0
  br i1 %396, label %421, label %397

397:                                              ; preds = %382
  %398 = load i64, i64* %371, align 8, !tbaa !21
  %399 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !65
  %400 = load i32, i32* %31, align 8, !tbaa !38
  %401 = add i32 %400, -1
  %402 = add i32 %401, %399
  %403 = sext i32 %402 to i64
  %404 = getelementptr inbounds [1000 x i64], [1000 x i64]* %372, i64 0, i64 %403, !intel-tbaa !66
  store i64 %398, i64* %404, align 8, !tbaa !67
  %405 = load i32, i32* %9, align 4, !tbaa !5
  %406 = sext i32 %405 to i64
  %407 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %406, !intel-tbaa !60
  %408 = load i32, i32* %407, align 4, !tbaa !60
  %409 = sext i32 %401 to i64
  %410 = getelementptr inbounds [64 x i32], [64 x i32]* %373, i64 0, i64 %409, !intel-tbaa !48
  store i32 %408, i32* %410, align 4, !tbaa !57
  %411 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %412 = load i32, i32* %31, align 8, !tbaa !38
  %413 = sext i32 %412 to i64
  %414 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %413, !intel-tbaa !48
  store i32 %411, i32* %414, align 4, !tbaa !49
  %415 = icmp ne i32 %411, 0
  %416 = or i1 %376, %415
  %417 = zext i1 %416 to i32
  %418 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %374, i32 %375, i32 %417)
  %419 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %374, i32 %378, i32 0, i32 0)
  %420 = sub nsw i32 0, %419
  br label %421

421:                                              ; preds = %397, %382
  %422 = phi i32 [ 1, %397 ], [ 0, %382 ]
  %423 = phi i32 [ %420, %397 ], [ %385, %382 ]
  %424 = load i32, i32* %9, align 4, !tbaa !5
  %425 = sext i32 %424 to i64
  %426 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %425, !intel-tbaa !60
  %427 = load i32, i32* %426, align 4, !tbaa !60
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %427)
  %428 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !54
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
  %441 = icmp ult i32 %384, 2
  %442 = and i1 %440, %441
  br i1 %442, label %382, label %508

443:                                              ; preds = %502, %370
  %444 = phi i32 [ %503, %502 ], [ 0, %370 ]
  %445 = phi i32 [ %447, %502 ], [ 0, %370 ]
  %446 = phi i32 [ %484, %502 ], [ -32000, %370 ]
  %447 = add nuw nsw i32 %445, 1
  %448 = load i32, i32* %9, align 4, !tbaa !5
  %449 = sext i32 %448 to i64
  %450 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %449, !intel-tbaa !60
  %451 = load i32, i32* %450, align 4, !tbaa !60
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %451)
  %452 = load i32, i32* %9, align 4, !tbaa !5
  %453 = sext i32 %452 to i64
  %454 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %453, !intel-tbaa !60
  %455 = load i32, i32* %454, align 4, !tbaa !60
  %456 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %455)
  %457 = icmp eq i32 %456, 0
  br i1 %457, label %482, label %458

458:                                              ; preds = %443
  %459 = load i64, i64* %371, align 8, !tbaa !21
  %460 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !65
  %461 = load i32, i32* %31, align 8, !tbaa !38
  %462 = add i32 %461, -1
  %463 = add i32 %462, %460
  %464 = sext i32 %463 to i64
  %465 = getelementptr inbounds [1000 x i64], [1000 x i64]* %372, i64 0, i64 %464, !intel-tbaa !66
  store i64 %459, i64* %465, align 8, !tbaa !67
  %466 = load i32, i32* %9, align 4, !tbaa !5
  %467 = sext i32 %466 to i64
  %468 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %467, !intel-tbaa !60
  %469 = load i32, i32* %468, align 4, !tbaa !60
  %470 = sext i32 %462 to i64
  %471 = getelementptr inbounds [64 x i32], [64 x i32]* %373, i64 0, i64 %470, !intel-tbaa !48
  store i32 %469, i32* %471, align 4, !tbaa !57
  %472 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %473 = load i32, i32* %31, align 8, !tbaa !38
  %474 = sext i32 %473 to i64
  %475 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %474, !intel-tbaa !48
  store i32 %472, i32* %475, align 4, !tbaa !49
  %476 = icmp ne i32 %472, 0
  %477 = or i1 %376, %476
  %478 = zext i1 %477 to i32
  %479 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %374, i32 %375, i32 %478)
  %480 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %374, i32 %378, i32 %379, i32 0, i32 %381)
  %481 = sub nsw i32 0, %480
  br label %482

482:                                              ; preds = %458, %443
  %483 = phi i32 [ 1, %458 ], [ 0, %443 ]
  %484 = phi i32 [ %481, %458 ], [ %446, %443 ]
  %485 = load i32, i32* %9, align 4, !tbaa !5
  %486 = sext i32 %485 to i64
  %487 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %486, !intel-tbaa !60
  %488 = load i32, i32* %487, align 4, !tbaa !60
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %488)
  %489 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !54
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
  %499 = load i32, i32* %13, align 4, !tbaa !5
  %500 = load i32, i32* %11, align 4, !tbaa !5
  %501 = load i32, i32* %15, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %70, i32 %64, i32 %70, i32 %499, i32 %500, i32 0, i32 %501, i32 %3)
  br label %1119

502:                                              ; preds = %495, %491
  %503 = phi i32 [ %496, %495 ], [ %444, %491 ]
  %504 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %505 = icmp ne i32 %504, 0
  %506 = icmp ult i32 %445, 2
  %507 = and i1 %505, %506
  br i1 %507, label %443, label %508

508:                                              ; preds = %502, %482, %437, %421, %367, %361, %353, %344, %336
  %509 = phi i32 [ -32000, %344 ], [ -32000, %361 ], [ -32000, %353 ], [ -32000, %336 ], [ -32000, %367 ], [ %423, %421 ], [ %423, %437 ], [ %484, %482 ], [ %484, %502 ]
  %510 = load i32, i32* %14, align 4, !tbaa !5
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
  br i1 %521, label %522, label %608

522:                                              ; preds = %508
  %523 = add nsw i32 %3, -24
  %524 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %523, i32 0, i32 %86)
  %525 = icmp sgt i32 %524, %64
  br i1 %525, label %526, label %608

526:                                              ; preds = %522
  store i32 -1, i32* %9, align 4, !tbaa !5
  %527 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %528 = icmp ne i32 %527, 0
  %529 = load i32, i32* %14, align 4
  %530 = icmp slt i32 %529, 2
  %531 = and i1 %528, %530
  br i1 %531, label %532, label %608

532:                                              ; preds = %526
  %533 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %534 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %535 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %536 = add nsw i32 %3, -16
  %537 = sub nsw i32 0, %70
  %538 = sub i32 50, %64
  %539 = sub nsw i32 0, %64
  %540 = xor i32 %64, -1
  %541 = icmp eq i32 %86, 0
  %542 = zext i1 %541 to i32
  %543 = sub i32 49, %64
  %544 = add nsw i32 %64, -50
  br label %545

545:                                              ; preds = %593, %532
  %546 = phi i32 [ 0, %532 ], [ %596, %593 ]
  %547 = phi i32 [ %509, %532 ], [ %595, %593 ]
  %548 = phi i32 [ 1, %532 ], [ %594, %593 ]
  %549 = load i32, i32* %9, align 4, !tbaa !5
  %550 = sext i32 %549 to i64
  %551 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %550, !intel-tbaa !60
  %552 = load i32, i32* %551, align 4, !tbaa !60
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %552)
  %553 = load i32, i32* %9, align 4, !tbaa !5
  %554 = sext i32 %553 to i64
  %555 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %554, !intel-tbaa !60
  %556 = load i32, i32* %555, align 4, !tbaa !60
  %557 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %556)
  %558 = icmp eq i32 %557, 0
  br i1 %558, label %593, label %559

559:                                              ; preds = %545
  %560 = load i64, i64* %533, align 8, !tbaa !21
  %561 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !65
  %562 = load i32, i32* %31, align 8, !tbaa !38
  %563 = add i32 %562, -1
  %564 = add i32 %563, %561
  %565 = sext i32 %564 to i64
  %566 = getelementptr inbounds [1000 x i64], [1000 x i64]* %534, i64 0, i64 %565, !intel-tbaa !66
  store i64 %560, i64* %566, align 8, !tbaa !67
  %567 = load i32, i32* %9, align 4, !tbaa !5
  %568 = sext i32 %567 to i64
  %569 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %568, !intel-tbaa !60
  %570 = load i32, i32* %569, align 4, !tbaa !60
  %571 = sext i32 %563 to i64
  %572 = getelementptr inbounds [64 x i32], [64 x i32]* %535, i64 0, i64 %571, !intel-tbaa !48
  store i32 %570, i32* %572, align 4, !tbaa !57
  %573 = add nsw i32 %546, 1
  %574 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %575 = load i32, i32* %31, align 8, !tbaa !38
  %576 = sext i32 %575 to i64
  %577 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %576, !intel-tbaa !48
  store i32 %574, i32* %577, align 4, !tbaa !49
  %578 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %537, i32 %538, i32 1)
  %579 = icmp eq i32 %548, 0
  br i1 %579, label %587, label %580

580:                                              ; preds = %559
  %581 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %540, i32 %539, i32 %536, i32 0, i32 %542)
  %582 = sub nsw i32 0, %581
  %583 = icmp slt i32 %64, %582
  br i1 %583, label %584, label %585

584:                                              ; preds = %580
  store i32 1, i32* %14, align 4, !tbaa !5
  br label %593

585:                                              ; preds = %580
  store i32 0, i32* %14, align 4, !tbaa !5
  %586 = add nsw i32 %546, 11
  br label %593

587:                                              ; preds = %559
  %588 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %543, i32 %538, i32 %536, i32 0, i32 0)
  %589 = sub nsw i32 0, %588
  %590 = icmp slt i32 %544, %589
  br i1 %590, label %591, label %593

591:                                              ; preds = %587
  store i32 0, i32* %14, align 4, !tbaa !5
  %592 = add nsw i32 %546, 11
  br label %593

593:                                              ; preds = %591, %587, %585, %584, %545
  %594 = phi i32 [ %548, %545 ], [ 0, %587 ], [ 0, %591 ], [ 0, %584 ], [ 0, %585 ]
  %595 = phi i32 [ %547, %545 ], [ %589, %587 ], [ %589, %591 ], [ %582, %584 ], [ %582, %585 ]
  %596 = phi i32 [ %546, %545 ], [ %573, %587 ], [ %592, %591 ], [ %573, %584 ], [ %586, %585 ]
  %597 = load i32, i32* %9, align 4, !tbaa !5
  %598 = sext i32 %597 to i64
  %599 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %598, !intel-tbaa !60
  %600 = load i32, i32* %599, align 4, !tbaa !60
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %600)
  %601 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %602 = icmp ne i32 %601, 0
  %603 = load i32, i32* %14, align 4
  %604 = icmp slt i32 %603, 2
  %605 = and i1 %602, %604
  %606 = icmp slt i32 %596, 3
  %607 = and i1 %606, %605
  br i1 %607, label %545, label %608

608:                                              ; preds = %593, %526, %522, %508
  %609 = phi i32 [ %509, %508 ], [ %509, %522 ], [ %509, %526 ], [ %595, %593 ]
  br i1 %96, label %615, label %610

610:                                              ; preds = %608
  %611 = load i32, i32* %31, align 8, !tbaa !38
  %612 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4, !tbaa !68
  %613 = shl nsw i32 %612, 1
  %614 = icmp sle i32 %611, %613
  br label %615

615:                                              ; preds = %610, %608
  %616 = phi i1 [ false, %608 ], [ %614, %610 ]
  store i32 -1, i32* %9, align 4, !tbaa !5
  %617 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %618 = icmp eq i32 %617, 0
  br i1 %618, label %1087, label %619

619:                                              ; preds = %615
  %620 = icmp eq i32 %282, 1
  %621 = and i1 %93, %620
  %622 = select i1 %621, i32 4, i32 0
  %623 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %624 = or i32 %622, 2
  %625 = select i1 %616, i32 %624, i32 %622
  %626 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %627 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %628 = select i1 %616, i32 3, i32 1
  %629 = add nsw i32 %145, %134
  %630 = icmp eq i32 %629, 1
  %631 = sdiv i32 %3, 4
  %632 = add nsw i32 %631, 1
  %633 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 0
  %634 = icmp sgt i32 %3, 24
  %635 = icmp slt i32 %3, 9
  %636 = icmp slt i32 %3, 13
  %637 = add nsw i32 %92, 100
  %638 = add nsw i32 %92, 300
  %639 = add nsw i32 %92, 75
  %640 = add nsw i32 %92, 200
  %641 = select i1 %616, i32 4, i32 2
  %642 = sub nsw i32 0, %70
  %643 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %644 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %645 = icmp sgt i32 %3, 4
  %646 = add i32 %3, -4
  %647 = icmp eq i32 %86, 0
  %648 = zext i1 %647 to i32
  br label %649

649:                                              ; preds = %1078, %619
  %650 = phi i32 [ %64, %619 ], [ %1085, %1078 ]
  %651 = phi i32 [ %609, %619 ], [ %1084, %1078 ]
  %652 = phi i32 [ 1, %619 ], [ %1083, %1078 ]
  %653 = phi i32 [ -32000, %619 ], [ %1082, %1078 ]
  %654 = phi i32 [ 1, %619 ], [ %1081, %1078 ]
  %655 = phi i32 [ 1, %619 ], [ %1080, %1078 ]
  %656 = icmp ne i32 %654, 0
  %657 = icmp sgt i32 %655, %632
  %658 = add nsw i32 %650, 1
  %659 = icmp eq i32 %70, %658
  br label %660

660:                                              ; preds = %789, %649
  %661 = phi i32 [ %652, %649 ], [ 0, %789 ]
  %662 = load i32, i32* %31, align 8, !tbaa !38
  %663 = icmp slt i32 %662, 60
  %664 = load i32, i32* %9, align 4, !tbaa !5
  %665 = sext i32 %664 to i64
  br i1 %663, label %666, label %746

666:                                              ; preds = %660
  %667 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %665, !intel-tbaa !60
  %668 = load i32, i32* %667, align 4, !tbaa !60
  %669 = lshr i32 %668, 6
  %670 = and i32 %669, 63
  %671 = zext i32 %670 to i64
  %672 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %671, !intel-tbaa !48
  %673 = load i32, i32* %672, align 4, !tbaa !62
  %674 = add nsw i32 %673, 1
  %675 = and i32 %674, -2
  %676 = icmp eq i32 %675, 2
  br i1 %676, label %677, label %684

677:                                              ; preds = %666
  %678 = lshr i32 %668, 3
  %679 = and i32 %678, 7
  switch i32 %679, label %680 [
    i32 1, label %683
    i32 6, label %683
  ]

680:                                              ; preds = %677
  %681 = and i32 %668, 61440
  %682 = icmp eq i32 %681, 0
  br i1 %682, label %684, label %683

683:                                              ; preds = %680, %677, %677
  br label %684

684:                                              ; preds = %683, %680, %666
  %685 = phi i32 [ %622, %680 ], [ %622, %666 ], [ %625, %683 ]
  %686 = lshr i32 %668, 19
  %687 = and i32 %686, 15
  %688 = icmp eq i32 %687, 13
  br i1 %688, label %719, label %689

689:                                              ; preds = %684
  %690 = add nsw i32 %662, -1
  %691 = sext i32 %690 to i64
  %692 = getelementptr inbounds [64 x i32], [64 x i32]* %626, i64 0, i64 %691, !intel-tbaa !48
  %693 = load i32, i32* %692, align 4, !tbaa !57
  %694 = lshr i32 %693, 19
  %695 = and i32 %694, 15
  %696 = icmp eq i32 %695, 13
  br i1 %696, label %719, label %697

697:                                              ; preds = %689
  %698 = zext i32 %687 to i64
  %699 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %698, !intel-tbaa !63
  %700 = load i32, i32* %699, align 4, !tbaa !63
  %701 = zext i32 %695 to i64
  %702 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %701, !intel-tbaa !63
  %703 = load i32, i32* %702, align 4, !tbaa !63
  %704 = icmp eq i32 %700, %703
  br i1 %704, label %705, label %719

705:                                              ; preds = %697
  %706 = and i32 %668, 63
  %707 = and i32 %693, 63
  %708 = icmp eq i32 %706, %707
  br i1 %708, label %709, label %719

709:                                              ; preds = %705
  %710 = load i32, i32* %627, align 4, !tbaa !20
  %711 = icmp eq i32 %710, 0
  %712 = zext i1 %711 to i32
  %713 = lshr i32 %668, 12
  %714 = and i32 %713, 15
  %715 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %712, i32 %670, i32 %706, i32 %714)
  %716 = icmp sgt i32 %715, 0
  %717 = select i1 %716, i32 %628, i32 0
  %718 = add nsw i32 %717, %685
  br label %719

719:                                              ; preds = %709, %705, %697, %689, %684
  %720 = phi i32 [ %685, %705 ], [ %685, %697 ], [ %685, %689 ], [ %685, %684 ], [ %718, %709 ]
  %721 = load i32, i32* %14, align 4, !tbaa !5
  %722 = icmp eq i32 %721, 1
  %723 = icmp ne i32 %720, 0
  %724 = and i1 %723, %722
  %725 = and i1 %656, %724
  br i1 %725, label %726, label %727

726:                                              ; preds = %719
  store i32 1, i32* %15, align 4, !tbaa !5
  br label %732

727:                                              ; preds = %719
  %728 = icmp eq i32 %720, 0
  %729 = and i1 %728, %722
  %730 = and i1 %656, %729
  br i1 %730, label %731, label %732

731:                                              ; preds = %727
  store i32 0, i32* %15, align 4, !tbaa !5
  br label %735

732:                                              ; preds = %727, %726
  %733 = icmp slt i32 %720, 4
  %734 = select i1 %733, i32 %720, i32 4
  br label %735

735:                                              ; preds = %732, %731
  %736 = phi i32 [ %628, %731 ], [ %734, %732 ]
  %737 = load i32, i32* %9, align 4, !tbaa !5
  %738 = sext i32 %737 to i64
  %739 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %738, !intel-tbaa !60
  %740 = load i32, i32* %739, align 4, !tbaa !60
  %741 = lshr i32 %740, 19
  %742 = and i32 %741, 15
  switch i32 %742, label %743 [
    i32 13, label %746
    i32 1, label %746
    i32 2, label %746
  ]

743:                                              ; preds = %735
  %744 = add nsw i32 %736, 4
  %745 = select i1 %630, i32 %744, i32 %736
  br label %746

746:                                              ; preds = %743, %735, %735, %735, %660
  %747 = phi i64 [ %738, %743 ], [ %738, %735 ], [ %738, %735 ], [ %738, %735 ], [ %665, %660 ]
  %748 = phi i32 [ %745, %743 ], [ %736, %735 ], [ %736, %735 ], [ %736, %735 ], [ 0, %660 ]
  %749 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %747, !intel-tbaa !60
  %750 = load i32, i32* %749, align 4, !tbaa !60
  %751 = and i32 %750, 7864320
  %752 = icmp eq i32 %751, 6815744
  %753 = xor i1 %752, true
  %754 = xor i1 %657, true
  %755 = or i1 %753, %754
  %756 = select i1 %753, i1 false, i1 true
  %757 = select i1 %753, i32 %661, i32 %652
  br i1 %755, label %795, label %758

758:                                              ; preds = %746
  %759 = lshr i32 %750, 6
  %760 = and i32 %759, 63
  %761 = zext i32 %760 to i64
  %762 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %761, !intel-tbaa !48
  %763 = load i32, i32* %762, align 4, !tbaa !62
  %764 = add nsw i32 %763, -1
  %765 = and i32 %750, 63
  %766 = load i32, i32* %633, align 8, !tbaa !69
  %767 = sext i32 %766 to i64
  %768 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %767, !intel-tbaa !70
  %769 = sext i32 %764 to i64
  %770 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %768, i64 0, i64 %769, !intel-tbaa !73
  %771 = zext i32 %765 to i64
  %772 = getelementptr inbounds [64 x i32], [64 x i32]* %770, i64 0, i64 %771, !intel-tbaa !48
  %773 = load i32, i32* %772, align 4, !tbaa !74
  %774 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %767, !intel-tbaa !70
  %775 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %774, i64 0, i64 %769, !intel-tbaa !73
  %776 = getelementptr inbounds [64 x i32], [64 x i32]* %775, i64 0, i64 %771, !intel-tbaa !48
  %777 = load i32, i32* %776, align 4, !tbaa !74
  %778 = sub nsw i32 %777, %773
  %779 = mul nsw i32 %773, %632
  %780 = icmp sge i32 %779, %778
  %781 = or i1 %634, %780
  %782 = icmp ne i32 %748, 0
  %783 = or i1 %782, %781
  %784 = xor i1 %783, true
  %785 = and i1 %659, %784
  %786 = and i32 %750, 61440
  %787 = icmp eq i32 %786, 0
  %788 = and i1 %787, %785
  br i1 %788, label %789, label %795

789:                                              ; preds = %758
  %790 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  %791 = icmp eq i32 %790, 0
  br i1 %791, label %792, label %660

792:                                              ; preds = %789
  %793 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !54
  %794 = icmp eq i32 %793, 0
  br label %1106

795:                                              ; preds = %758, %746
  %796 = phi i1 [ true, %758 ], [ %756, %746 ]
  %797 = phi i32 [ %661, %758 ], [ %757, %746 ]
  br i1 %635, label %798, label %801

798:                                              ; preds = %795
  %799 = icmp slt i32 %639, %650
  %800 = icmp slt i32 %640, %650
  br label %806

801:                                              ; preds = %795
  %802 = icmp slt i32 %637, %650
  %803 = icmp slt i32 %638, %650
  %804 = and i1 %636, %802
  %805 = and i1 %636, %803
  br label %806

806:                                              ; preds = %801, %798
  %807 = phi i1 [ %799, %798 ], [ %804, %801 ]
  %808 = phi i1 [ %800, %798 ], [ %805, %801 ]
  br i1 %796, label %821, label %809

809:                                              ; preds = %806
  %810 = load i32, i32* %627, align 4, !tbaa !20
  %811 = icmp eq i32 %810, 0
  %812 = zext i1 %811 to i32
  %813 = lshr i32 %750, 6
  %814 = and i32 %813, 63
  %815 = and i32 %750, 63
  %816 = lshr i32 %750, 12
  %817 = and i32 %816, 15
  %818 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %812, i32 %814, i32 %815, i32 %817)
  %819 = load i32, i32* %9, align 4, !tbaa !5
  %820 = sext i32 %819 to i64
  br label %821

821:                                              ; preds = %809, %806
  %822 = phi i64 [ %747, %806 ], [ %820, %809 ]
  %823 = phi i32 [ -1000000, %806 ], [ %818, %809 ]
  %824 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %822, !intel-tbaa !60
  %825 = load i32, i32* %824, align 4, !tbaa !60
  call void @_Z4makeP7state_ti(%struct.state_t* nonnull %0, i32 %825)
  %826 = load i32, i32* %9, align 4, !tbaa !5
  %827 = sext i32 %826 to i64
  %828 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %827, !intel-tbaa !60
  %829 = load i32, i32* %828, align 4, !tbaa !60
  %830 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* nonnull %0, i32 %829)
  %831 = icmp eq i32 %830, 0
  br i1 %831, label %974, label %832

832:                                              ; preds = %821
  %833 = call i32 @_Z8in_checkP7state_t(%struct.state_t* nonnull %0)
  %834 = icmp ne i32 %833, 0
  %835 = select i1 %834, i32 %641, i32 0
  %836 = add nsw i32 %835, %748
  %837 = or i32 %833, %91
  %838 = icmp eq i32 %837, 0
  %839 = and i1 %659, %838
  br i1 %839, label %840, label %864

840:                                              ; preds = %832
  %841 = icmp slt i32 %823, 86
  %842 = and i1 %808, %841
  br i1 %842, label %843, label %852

843:                                              ; preds = %840
  %844 = load i32, i32* %9, align 4, !tbaa !5
  %845 = sext i32 %844 to i64
  %846 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %845, !intel-tbaa !60
  %847 = load i32, i32* %846, align 4, !tbaa !60
  %848 = and i32 %847, 61440
  %849 = icmp eq i32 %848, 0
  br i1 %849, label %850, label %852

850:                                              ; preds = %843
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %847)
  %851 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  br label %1078

852:                                              ; preds = %843, %840
  %853 = icmp slt i32 %823, -50
  %854 = and i1 %807, %853
  br i1 %854, label %855, label %864

855:                                              ; preds = %852
  %856 = load i32, i32* %9, align 4, !tbaa !5
  %857 = sext i32 %856 to i64
  %858 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %857, !intel-tbaa !60
  %859 = load i32, i32* %858, align 4, !tbaa !60
  %860 = and i32 %859, 61440
  %861 = icmp eq i32 %860, 0
  br i1 %861, label %862, label %864

862:                                              ; preds = %855
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %859)
  %863 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  br label %1078

864:                                              ; preds = %855, %852, %832
  %865 = add nsw i32 %836, %3
  %866 = add nsw i32 %865, -4
  %867 = sub nsw i32 0, %650
  %868 = sub i32 130, %650
  %869 = icmp sgt i32 %865, 4
  %870 = or i1 %834, %869
  %871 = zext i1 %870 to i32
  %872 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %642, i32 %868, i32 %871)
  %873 = load i32, i32* %31, align 8, !tbaa !38
  %874 = sext i32 %873 to i64
  %875 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %874, !intel-tbaa !48
  store i32 %833, i32* %875, align 4, !tbaa !49
  %876 = load i64, i64* %643, align 8, !tbaa !21
  %877 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !65
  %878 = add i32 %873, -1
  %879 = add i32 %878, %877
  %880 = sext i32 %879 to i64
  %881 = getelementptr inbounds [1000 x i64], [1000 x i64]* %644, i64 0, i64 %880, !intel-tbaa !66
  store i64 %876, i64* %881, align 8, !tbaa !67
  %882 = load i32, i32* %9, align 4, !tbaa !5
  %883 = sext i32 %882 to i64
  %884 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %883, !intel-tbaa !60
  %885 = load i32, i32* %884, align 4, !tbaa !60
  %886 = sext i32 %878 to i64
  %887 = getelementptr inbounds [64 x i32], [64 x i32]* %626, i64 0, i64 %886, !intel-tbaa !48
  store i32 %885, i32* %887, align 4, !tbaa !57
  %888 = icmp sgt i32 %655, 3
  %889 = and i1 %645, %888
  br i1 %889, label %890, label %924

890:                                              ; preds = %864
  %891 = or i32 %836, %833
  %892 = icmp eq i32 %891, 0
  %893 = and i1 %659, %892
  %894 = icmp slt i32 %823, -50
  %895 = and i1 %894, %893
  br i1 %895, label %896, label %924

896:                                              ; preds = %890
  %897 = and i32 %885, 63
  %898 = zext i32 %897 to i64
  %899 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %898, !intel-tbaa !48
  %900 = load i32, i32* %899, align 4, !tbaa !62
  %901 = add nsw i32 %900, -1
  %902 = load i32, i32* %633, align 8, !tbaa !69
  %903 = sext i32 %902 to i64
  %904 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %903, !intel-tbaa !70
  %905 = sext i32 %901 to i64
  %906 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %904, i64 0, i64 %905, !intel-tbaa !73
  %907 = getelementptr inbounds [64 x i32], [64 x i32]* %906, i64 0, i64 %898, !intel-tbaa !48
  %908 = load i32, i32* %907, align 4, !tbaa !74
  %909 = shl i32 %908, 7
  %910 = add i32 %909, 128
  %911 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %903, !intel-tbaa !70
  %912 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %911, i64 0, i64 %905, !intel-tbaa !73
  %913 = getelementptr inbounds [64 x i32], [64 x i32]* %912, i64 0, i64 %898, !intel-tbaa !48
  %914 = load i32, i32* %913, align 4, !tbaa !74
  %915 = add nsw i32 %914, 1
  %916 = sdiv i32 %910, %915
  %917 = icmp slt i32 %916, 80
  %918 = and i32 %885, 61440
  %919 = icmp eq i32 %918, 0
  %920 = and i1 %919, %917
  br i1 %920, label %921, label %924

921:                                              ; preds = %896
  %922 = add nsw i32 %836, -4
  %923 = add i32 %646, %922
  br label %924

924:                                              ; preds = %921, %896, %890, %864
  %925 = phi i32 [ 4, %921 ], [ 0, %896 ], [ 0, %890 ], [ 0, %864 ]
  %926 = phi i32 [ %922, %921 ], [ %836, %896 ], [ %836, %890 ], [ %836, %864 ]
  %927 = phi i32 [ %923, %921 ], [ %866, %896 ], [ %866, %890 ], [ %866, %864 ]
  %928 = icmp eq i32 %654, 1
  %929 = icmp slt i32 %927, 1
  br i1 %928, label %930, label %937

930:                                              ; preds = %924
  br i1 %929, label %931, label %934

931:                                              ; preds = %930
  %932 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %642, i32 %867, i32 0, i32 0)
  %933 = sub nsw i32 0, %932
  br label %970

934:                                              ; preds = %930
  %935 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %642, i32 %867, i32 %927, i32 0, i32 %648)
  %936 = sub nsw i32 0, %935
  br label %970

937:                                              ; preds = %924
  %938 = xor i32 %650, -1
  br i1 %929, label %939, label %941

939:                                              ; preds = %937
  %940 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %938, i32 %867, i32 0, i32 0)
  br label %943

941:                                              ; preds = %937
  %942 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %938, i32 %867, i32 %927, i32 0, i32 1)
  br label %943

943:                                              ; preds = %941, %939
  %944 = phi i32 [ %940, %939 ], [ %942, %941 ]
  %945 = sub nsw i32 0, %944
  %946 = icmp slt i32 %653, %945
  %947 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %948 = icmp eq i32 %947, 0
  %949 = and i1 %946, %948
  %950 = icmp slt i32 %650, %945
  %951 = and i1 %950, %949
  br i1 %951, label %952, label %970

952:                                              ; preds = %943
  %953 = icmp eq i32 %925, 0
  br i1 %953, label %956, label %954

954:                                              ; preds = %952
  %955 = add nsw i32 %926, %925
  br label %958

956:                                              ; preds = %952
  %957 = icmp sgt i32 %70, %945
  br i1 %957, label %958, label %970

958:                                              ; preds = %956, %954
  %959 = phi i32 [ %955, %954 ], [ %926, %956 ]
  %960 = add nsw i32 %959, %3
  %961 = icmp slt i32 %960, 5
  br i1 %961, label %962, label %965

962:                                              ; preds = %958
  %963 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %642, i32 %867, i32 0, i32 0)
  %964 = sub nsw i32 0, %963
  br label %970

965:                                              ; preds = %958
  %966 = add nsw i32 %960, -4
  %967 = lshr exact i32 %925, 2
  %968 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %642, i32 %867, i32 %966, i32 0, i32 %967)
  %969 = sub nsw i32 0, %968
  br label %970

970:                                              ; preds = %965, %962, %956, %943, %934, %931
  %971 = phi i32 [ %933, %931 ], [ %936, %934 ], [ %945, %943 ], [ %964, %962 ], [ %969, %965 ], [ %945, %956 ]
  %972 = icmp sgt i32 %971, %653
  %973 = select i1 %972, i32 %971, i32 %653
  br label %974

974:                                              ; preds = %970, %821
  %975 = phi i32 [ %973, %970 ], [ %653, %821 ]
  %976 = phi i32 [ 1, %970 ], [ 0, %821 ]
  %977 = phi i32 [ 0, %970 ], [ %797, %821 ]
  %978 = phi i32 [ %971, %970 ], [ %651, %821 ]
  %979 = load i32, i32* %9, align 4, !tbaa !5
  %980 = sext i32 %979 to i64
  %981 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %980, !intel-tbaa !60
  %982 = load i32, i32* %981, align 4, !tbaa !60
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %982)
  %983 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !54
  %984 = icmp eq i32 %983, 0
  br i1 %984, label %985, label %1119

985:                                              ; preds = %974
  %986 = icmp eq i32 %976, 0
  br i1 %986, label %1073, label %987

987:                                              ; preds = %985
  %988 = icmp sgt i32 %978, %650
  br i1 %988, label %989, label %1063

989:                                              ; preds = %987
  %990 = icmp slt i32 %978, %70
  %991 = load i32, i32* %9, align 4, !tbaa !5
  %992 = sext i32 %991 to i64
  %993 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %992
  %994 = load i32, i32* %993, align 4, !tbaa !60
  br i1 %990, label %1060, label %995

995:                                              ; preds = %989
  call fastcc void @_ZL12history_goodP7state_tii(%struct.state_t* nonnull %0, i32 %994, i32 %3)
  %996 = icmp sgt i32 %655, 1
  br i1 %996, label %997, label %1050

997:                                              ; preds = %995
  %998 = add nsw i32 %3, 3
  %999 = sdiv i32 %998, 4
  %1000 = add nsw i32 %655, -1
  %1001 = sext i32 %1000 to i64
  br label %1002

1002:                                             ; preds = %1047, %997
  %1003 = phi i64 [ 0, %997 ], [ %1048, %1047 ]
  %1004 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %1003, !intel-tbaa !60
  %1005 = load i32, i32* %1004, align 4, !tbaa !60
  %1006 = and i32 %1005, 7925760
  %1007 = icmp eq i32 %1006, 6815744
  br i1 %1007, label %1008, label %1047

1008:                                             ; preds = %1002
  %1009 = lshr i32 %1005, 6
  %1010 = and i32 %1009, 63
  %1011 = zext i32 %1010 to i64
  %1012 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %1011, !intel-tbaa !48
  %1013 = load i32, i32* %1012, align 4, !tbaa !62
  %1014 = add nsw i32 %1013, -1
  %1015 = and i32 %1005, 63
  %1016 = load i32, i32* %633, align 8, !tbaa !69
  %1017 = sext i32 %1016 to i64
  %1018 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %1017, !intel-tbaa !70
  %1019 = sext i32 %1014 to i64
  %1020 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1018, i64 0, i64 %1019, !intel-tbaa !73
  %1021 = zext i32 %1015 to i64
  %1022 = getelementptr inbounds [64 x i32], [64 x i32]* %1020, i64 0, i64 %1021, !intel-tbaa !48
  %1023 = load i32, i32* %1022, align 4, !tbaa !74
  %1024 = add nsw i32 %1023, %999
  store i32 %1024, i32* %1022, align 4, !tbaa !74
  %1025 = icmp sgt i32 %1024, 16384
  br i1 %1025, label %1026, label %1047

1026:                                             ; preds = %1008
  %1027 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %1017, !intel-tbaa !70
  br label %1028

1028:                                             ; preds = %1044, %1026
  %1029 = phi i64 [ 0, %1026 ], [ %1045, %1044 ]
  %1030 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1027, i64 0, i64 %1029, !intel-tbaa !73
  %1031 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1018, i64 0, i64 %1029, !intel-tbaa !73
  br label %1032

1032:                                             ; preds = %1032, %1028
  %1033 = phi i64 [ 0, %1028 ], [ %1042, %1032 ]
  %1034 = getelementptr inbounds [64 x i32], [64 x i32]* %1030, i64 0, i64 %1033, !intel-tbaa !48
  %1035 = load i32, i32* %1034, align 4, !tbaa !74
  %1036 = add nsw i32 %1035, 1
  %1037 = ashr i32 %1036, 1
  store i32 %1037, i32* %1034, align 4, !tbaa !74
  %1038 = getelementptr inbounds [64 x i32], [64 x i32]* %1031, i64 0, i64 %1033, !intel-tbaa !48
  %1039 = load i32, i32* %1038, align 4, !tbaa !74
  %1040 = add nsw i32 %1039, 1
  %1041 = ashr i32 %1040, 1
  store i32 %1041, i32* %1038, align 4, !tbaa !74
  %1042 = add nuw nsw i64 %1033, 1
  %1043 = icmp eq i64 %1042, 64
  br i1 %1043, label %1044, label %1032

1044:                                             ; preds = %1032
  %1045 = add nuw nsw i64 %1029, 1
  %1046 = icmp eq i64 %1045, 12
  br i1 %1046, label %1047, label %1028

1047:                                             ; preds = %1044, %1008, %1002
  %1048 = add nuw nsw i64 %1003, 1
  %1049 = icmp eq i64 %1048, %1001
  br i1 %1049, label %1050, label %1002

1050:                                             ; preds = %1047, %995
  %1051 = load i32, i32* %9, align 4, !tbaa !5
  %1052 = sext i32 %1051 to i64
  %1053 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %1052, !intel-tbaa !60
  %1054 = load i32, i32* %1053, align 4, !tbaa !60
  %1055 = call zeroext i16 @_Z12compact_movei(i32 %1054)
  %1056 = zext i16 %1055 to i32
  %1057 = load i32, i32* %11, align 4, !tbaa !5
  %1058 = load i32, i32* %14, align 4, !tbaa !5
  %1059 = load i32, i32* %15, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %978, i32 %64, i32 %70, i32 %1056, i32 %1057, i32 %1058, i32 %1059, i32 %3)
  br label %1119

1060:                                             ; preds = %989
  %1061 = call zeroext i16 @_Z12compact_movei(i32 %994)
  %1062 = zext i16 %1061 to i32
  store i32 %1062, i32* %13, align 4, !tbaa !5
  br label %1063

1063:                                             ; preds = %1060, %987
  %1064 = phi i32 [ %978, %1060 ], [ %650, %987 ]
  %1065 = load i32, i32* %9, align 4, !tbaa !5
  %1066 = sext i32 %1065 to i64
  %1067 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %1066, !intel-tbaa !60
  %1068 = load i32, i32* %1067, align 4, !tbaa !60
  %1069 = add nsw i32 %655, -1
  %1070 = sext i32 %1069 to i64
  %1071 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %1070, !intel-tbaa !60
  store i32 %1068, i32* %1071, align 4, !tbaa !60
  %1072 = add nsw i32 %655, 1
  br label %1073

1073:                                             ; preds = %1063, %985
  %1074 = phi i32 [ %1072, %1063 ], [ %655, %985 ]
  %1075 = phi i32 [ 0, %1063 ], [ %654, %985 ]
  %1076 = phi i32 [ %1064, %1063 ], [ %650, %985 ]
  %1077 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281)
  br label %1078

1078:                                             ; preds = %1073, %862, %850
  %1079 = phi i32 [ %1077, %1073 ], [ %863, %862 ], [ %851, %850 ]
  %1080 = phi i32 [ %1074, %1073 ], [ %655, %862 ], [ %655, %850 ]
  %1081 = phi i32 [ %1075, %1073 ], [ %654, %862 ], [ %654, %850 ]
  %1082 = phi i32 [ %975, %1073 ], [ %650, %862 ], [ %650, %850 ]
  %1083 = phi i32 [ %977, %1073 ], [ 0, %862 ], [ 0, %850 ]
  %1084 = phi i32 [ %978, %1073 ], [ %651, %862 ], [ %651, %850 ]
  %1085 = phi i32 [ %1076, %1073 ], [ %650, %862 ], [ %650, %850 ]
  %1086 = icmp eq i32 %1079, 0
  br i1 %1086, label %1087, label %649

1087:                                             ; preds = %1078, %615
  %1088 = phi i32 [ -32000, %615 ], [ %1082, %1078 ]
  %1089 = phi i32 [ 1, %615 ], [ %1083, %1078 ]
  %1090 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !54
  %1091 = icmp eq i32 %1090, 0
  %1092 = icmp ne i32 %1089, 0
  %1093 = and i1 %1092, %1091
  br i1 %1093, label %1094, label %1106

1094:                                             ; preds = %1087
  %1095 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %1096 = icmp eq i32 %1095, 0
  %1097 = load i32, i32* %11, align 4, !tbaa !5
  %1098 = load i32, i32* %14, align 4, !tbaa !5
  %1099 = load i32, i32* %15, align 4, !tbaa !5
  br i1 %1096, label %1105, label %1100

1100:                                             ; preds = %1094
  %1101 = load i32, i32* %31, align 8, !tbaa !38
  %1102 = add nsw i32 %1101, -32000
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %1102, i32 %64, i32 %70, i32 0, i32 %1097, i32 %1098, i32 %1099, i32 %3)
  %1103 = load i32, i32* %31, align 8, !tbaa !38
  %1104 = add nsw i32 %1103, -32000
  br label %1119

1105:                                             ; preds = %1094
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 0, i32 %64, i32 %70, i32 0, i32 %1097, i32 %1098, i32 %1099, i32 %3)
  br label %1119

1106:                                             ; preds = %1087, %792
  %1107 = phi i1 [ %794, %792 ], [ %1091, %1087 ]
  %1108 = phi i32 [ %653, %792 ], [ %1088, %1087 ]
  %1109 = load i32, i32* %46, align 4, !tbaa !42
  %1110 = icmp sgt i32 %1109, 98
  %1111 = xor i1 %1107, true
  %1112 = or i1 %1110, %1111
  %1113 = select i1 %1110, i32 0, i32 %1108
  br i1 %1112, label %1119, label %1114

1114:                                             ; preds = %1106
  %1115 = load i32, i32* %13, align 4, !tbaa !5
  %1116 = load i32, i32* %11, align 4, !tbaa !5
  %1117 = load i32, i32* %14, align 4, !tbaa !5
  %1118 = load i32, i32* %15, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %1108, i32 %64, i32 %70, i32 %1115, i32 %1116, i32 %1117, i32 %1118, i32 %3)
  br label %1119

1119:                                             ; preds = %1114, %1106, %1105, %1100, %1050, %974, %498, %243, %237, %224, %211, %172, %117, %110, %103, %77, %74, %72, %67, %61, %49, %36, %34
  %1120 = phi i32 [ %35, %34 ], [ %56, %49 ], [ %73, %72 ], [ 0, %36 ], [ %59, %61 ], [ %65, %67 ], [ %75, %74 ], [ %78, %77 ], [ %70, %498 ], [ %92, %103 ], [ %111, %110 ], [ %92, %117 ], [ 0, %172 ], [ %213, %224 ], [ 0, %211 ], [ %64, %243 ], [ 0, %237 ], [ %978, %1050 ], [ %1104, %1100 ], [ 0, %1105 ], [ %1113, %1106 ], [ %1108, %1114 ], [ 0, %974 ]
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
  ret i32 %1120
}
; Function Attrs: uwtable
define internal i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4) #1 {
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
  %18 = getelementptr %struct.state_t, %struct.state_t* %0, i64 0, i32 22, !intel-tbaa !41
  %19 = load i64, i64* %18, align 8, !tbaa !41
  %20 = add i64 %19, 1
  store i64 %20, i64* %18, align 8, !tbaa !41
  %21 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 23, !intel-tbaa !75
  %22 = load i64, i64* %21, align 8, !tbaa !75
  %23 = add i64 %22, 1
  store i64 %23, i64* %21, align 8, !tbaa !75
  %24 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !38
  %25 = load i32, i32* %24, align 8, !tbaa !38
  %26 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 24, !intel-tbaa !76
  %27 = load i32, i32* %26, align 8, !tbaa !76
  %28 = icmp sgt i32 %25, %27
  br i1 %28, label %29, label %30

29:                                               ; preds = %5
  store i32 %25, i32* %26, align 8, !tbaa !76
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
  %37 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15, !intel-tbaa !42
  %38 = load i32, i32* %37, align 4, !tbaa !42
  %39 = icmp sgt i32 %38, 99
  br i1 %39, label %40, label %48

40:                                               ; preds = %36, %33
  %41 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4, !tbaa !43
  %42 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !20
  %43 = load i32, i32* %42, align 4, !tbaa !20
  %44 = icmp eq i32 %41, %43
  %45 = load i32, i32* @contempt, align 4, !tbaa !5
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
  %51 = load i32, i32* %8, align 4, !tbaa !5
  br label %420

52:                                               ; preds = %48
  %53 = load i32, i32* %8, align 4, !tbaa !5
  %54 = icmp sgt i32 %53, %1
  br i1 %54, label %59, label %420

55:                                               ; preds = %48
  %56 = load i32, i32* %8, align 4, !tbaa !5
  %57 = icmp slt i32 %56, %2
  br i1 %57, label %59, label %420

58:                                               ; preds = %48
  store i32 65535, i32* %7, align 4, !tbaa !5
  br label %59

59:                                               ; preds = %58, %55, %52, %48
  %60 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4, !tbaa !68
  %61 = shl nsw i32 %60, 1
  %62 = icmp slt i32 %61, %4
  br i1 %62, label %66, label %63

63:                                               ; preds = %59
  %64 = load i32, i32* %24, align 8, !tbaa !38
  %65 = icmp sgt i32 %64, 60
  br i1 %65, label %66, label %68

66:                                               ; preds = %63, %59
  %67 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %1, i32 %2, i32 0)
  br label %420

68:                                               ; preds = %63
  %69 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25, !intel-tbaa !47
  %70 = sext i32 %64 to i64
  %71 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %70, !intel-tbaa !48
  %72 = load i32, i32* %71, align 4, !tbaa !49
  %73 = call i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nonnull %0)
  %74 = add nsw i32 %73, 50
  %75 = icmp ne i32 %72, 0
  br i1 %75, label %150, label %76

76:                                               ; preds = %68
  %77 = icmp slt i32 %73, %2
  br i1 %77, label %80, label %78

78:                                               ; preds = %76
  %79 = load i32, i32* %7, align 4, !tbaa !5
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
  %86 = load i32, i32* %7, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %83, i32 %1, i32 %2, i32 %86, i32 0, i32 0, i32 0, i32 0)
  br label %420

87:                                               ; preds = %82
  %88 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8, !intel-tbaa !50
  %89 = getelementptr inbounds [13 x i32], [13 x i32]* %88, i64 0, i64 0
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !20
  %91 = load i32, i32* %90, align 4, !tbaa !20
  %92 = icmp eq i32 %91, 0
  br i1 %92, label %120, label %93

93:                                               ; preds = %87
  %94 = getelementptr inbounds i32, i32* %89, i64 10
  %95 = load i32, i32* %94, align 4, !tbaa !5
  %96 = icmp eq i32 %95, 0
  br i1 %96, label %97, label %147

97:                                               ; preds = %93
  %98 = getelementptr inbounds i32, i32* %89, i64 8
  %99 = load i32, i32* %98, align 4, !tbaa !5
  %100 = icmp eq i32 %99, 0
  br i1 %100, label %101, label %115

101:                                              ; preds = %97
  %102 = getelementptr inbounds i32, i32* %89, i64 12
  %103 = load i32, i32* %102, align 4, !tbaa !5
  %104 = icmp eq i32 %103, 0
  br i1 %104, label %105, label %112

105:                                              ; preds = %101
  %106 = getelementptr inbounds i32, i32* %89, i64 4
  %107 = load i32, i32* %106, align 4, !tbaa !5
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
  %119 = load i32, i32* %7, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %116, i32 %1, i32 %2, i32 %119, i32 0, i32 0, i32 0, i32 0)
  br label %420

120:                                              ; preds = %87
  %121 = getelementptr inbounds i32, i32* %89, i64 9
  %122 = load i32, i32* %121, align 4, !tbaa !5
  %123 = icmp eq i32 %122, 0
  br i1 %123, label %124, label %147

124:                                              ; preds = %120
  %125 = getelementptr inbounds i32, i32* %89, i64 7
  %126 = load i32, i32* %125, align 4, !tbaa !5
  %127 = icmp eq i32 %126, 0
  br i1 %127, label %128, label %142

128:                                              ; preds = %124
  %129 = getelementptr inbounds i32, i32* %89, i64 11
  %130 = load i32, i32* %129, align 4, !tbaa !5
  %131 = icmp eq i32 %130, 0
  br i1 %131, label %132, label %139

132:                                              ; preds = %128
  %133 = getelementptr inbounds i32, i32* %89, i64 3
  %134 = load i32, i32* %133, align 4, !tbaa !5
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
  %146 = load i32, i32* %7, align 4, !tbaa !5
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %143, i32 %1, i32 %2, i32 %146, i32 0, i32 0, i32 0, i32 0)
  br label %420

147:                                              ; preds = %142, %139, %136, %120, %115, %112, %109, %93, %80
  %148 = phi i32 [ %73, %80 ], [ %1, %120 ], [ %1, %136 ], [ %1, %139 ], [ %1, %142 ], [ %1, %93 ], [ %1, %109 ], [ %1, %112 ], [ %1, %115 ]
  %149 = sub nsw i32 %148, %74
  br label %150

150:                                              ; preds = %147, %68
  %151 = phi i32 [ %148, %147 ], [ %1, %68 ]
  %152 = phi i32 [ %149, %147 ], [ 0, %68 ]
  %153 = load i32, i32* %7, align 4, !tbaa !5
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
  %195 = load i32, i32* %7, align 4, !tbaa !5
  call fastcc void @_ZL16fast_order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %155, i32* nonnull %169, i32 %194, i32 %195)
  store i32 -1, i32* %6, align 4, !tbaa !5
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
  %209 = load i32, i32* %6, align 4, !tbaa !5
  br label %336

210:                                              ; preds = %203
  br i1 %199, label %211, label %251

211:                                              ; preds = %248, %210
  %212 = load i32, i32* %6, align 4, !tbaa !5
  %213 = sext i32 %212 to i64
  %214 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %213
  %215 = load i32, i32* %214, align 4, !tbaa !60
  %216 = lshr i32 %215, 19
  %217 = and i32 %216, 15
  %218 = zext i32 %217 to i64
  %219 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %218
  %220 = load i32, i32* %219, align 4, !tbaa !63
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
  %232 = getelementptr inbounds [64 x i32], [64 x i32]* %170, i64 0, i64 %231, !intel-tbaa !48
  %233 = load i32, i32* %232, align 4, !tbaa !62
  %234 = sext i32 %233 to i64
  %235 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %234, !intel-tbaa !63
  %236 = load i32, i32* %235, align 4, !tbaa !63
  %237 = icmp slt i32 %236, 0
  %238 = sub nsw i32 0, %236
  %239 = select i1 %237, i32 %238, i32 %236
  %240 = icmp slt i32 %223, %239
  br i1 %240, label %241, label %336

241:                                              ; preds = %228
  %242 = load i32, i32* %172, align 4, !tbaa !20
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
  %253 = load i32, i32* %6, align 4, !tbaa !5
  %254 = sext i32 %253 to i64
  %255 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %254, !intel-tbaa !60
  %256 = load i32, i32* %255, align 4, !tbaa !60
  %257 = lshr i32 %256, 19
  %258 = and i32 %257, 15
  %259 = icmp eq i32 %258, 13
  br i1 %259, label %271, label %260

260:                                              ; preds = %252
  %261 = zext i32 %258 to i64
  %262 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %261, !intel-tbaa !63
  %263 = load i32, i32* %262, align 4, !tbaa !63
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
  %274 = load i32, i32* %6, align 4, !tbaa !5
  %275 = sext i32 %274 to i64
  %276 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %275, !intel-tbaa !60
  %277 = load i32, i32* %276, align 4, !tbaa !60
  %278 = lshr i32 %277, 6
  %279 = and i32 %278, 63
  %280 = zext i32 %279 to i64
  %281 = getelementptr inbounds [64 x i32], [64 x i32]* %170, i64 0, i64 %280, !intel-tbaa !48
  %282 = load i32, i32* %281, align 4, !tbaa !62
  %283 = add nsw i32 %282, -1
  %284 = and i32 %277, 63
  %285 = load i32, i32* %171, align 8, !tbaa !69
  %286 = sext i32 %285 to i64
  %287 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %286, !intel-tbaa !70
  %288 = sext i32 %283 to i64
  %289 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %287, i64 0, i64 %288, !intel-tbaa !73
  %290 = zext i32 %284 to i64
  %291 = getelementptr inbounds [64 x i32], [64 x i32]* %289, i64 0, i64 %290, !intel-tbaa !48
  %292 = load i32, i32* %291, align 4, !tbaa !74
  %293 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %286, !intel-tbaa !70
  %294 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %293, i64 0, i64 %288, !intel-tbaa !73
  %295 = getelementptr inbounds [64 x i32], [64 x i32]* %294, i64 0, i64 %290, !intel-tbaa !48
  %296 = load i32, i32* %295, align 4, !tbaa !74
  %297 = sub nsw i32 %296, %292
  %298 = icmp slt i32 %292, %297
  br i1 %298, label %268, label %299

299:                                              ; preds = %273, %271
  %300 = load i32, i32* %6, align 4, !tbaa !5
  %301 = sext i32 %300 to i64
  br i1 %272, label %325, label %302

302:                                              ; preds = %299
  %303 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %301, !intel-tbaa !60
  %304 = load i32, i32* %303, align 4, !tbaa !60
  %305 = lshr i32 %304, 19
  %306 = and i32 %305, 15
  %307 = zext i32 %306 to i64
  %308 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %307, !intel-tbaa !63
  %309 = load i32, i32* %308, align 4, !tbaa !63
  %310 = icmp slt i32 %309, 0
  %311 = sub nsw i32 0, %309
  %312 = select i1 %310, i32 %311, i32 %309
  %313 = lshr i32 %304, 6
  %314 = and i32 %313, 63
  %315 = zext i32 %314 to i64
  %316 = getelementptr inbounds [64 x i32], [64 x i32]* %170, i64 0, i64 %315, !intel-tbaa !48
  %317 = load i32, i32* %316, align 4, !tbaa !62
  %318 = sext i32 %317 to i64
  %319 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %318, !intel-tbaa !63
  %320 = load i32, i32* %319, align 4, !tbaa !63
  %321 = icmp slt i32 %320, 0
  %322 = sub nsw i32 0, %320
  %323 = select i1 %321, i32 %322, i32 %320
  %324 = icmp slt i32 %312, %323
  br i1 %324, label %325, label %336

325:                                              ; preds = %302, %299
  %326 = load i32, i32* %172, align 4, !tbaa !20
  %327 = icmp eq i32 %326, 0
  %328 = zext i1 %327 to i32
  %329 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %301, !intel-tbaa !60
  %330 = load i32, i32* %329, align 4, !tbaa !60
  %331 = lshr i32 %330, 6
  %332 = and i32 %331, 63
  %333 = and i32 %330, 63
  %334 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* %0, i32 %328, i32 %332, i32 %333, i32 0)
  %335 = icmp slt i32 %334, -50
  br i1 %335, label %268, label %336

336:                                              ; preds = %325, %302, %241, %228, %208
  %337 = phi i32 [ %209, %208 ], [ %212, %228 ], [ %212, %241 ], [ %300, %302 ], [ %300, %325 ]
  %338 = sext i32 %337 to i64
  %339 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %338, !intel-tbaa !60
  %340 = load i32, i32* %339, align 4, !tbaa !60
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %340)
  %341 = load i32, i32* %339, align 4, !tbaa !60
  %342 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %341)
  %343 = icmp eq i32 %342, 0
  br i1 %343, label %376, label %344

344:                                              ; preds = %336
  %345 = load i64, i64* %173, align 8, !tbaa !21
  %346 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !65
  %347 = load i32, i32* %24, align 8, !tbaa !38
  %348 = add i32 %347, -1
  %349 = add i32 %348, %346
  %350 = sext i32 %349 to i64
  %351 = getelementptr inbounds [1000 x i64], [1000 x i64]* %174, i64 0, i64 %350, !intel-tbaa !66
  store i64 %345, i64* %351, align 8, !tbaa !67
  %352 = load i32, i32* %339, align 4, !tbaa !60
  %353 = sext i32 %348 to i64
  %354 = getelementptr inbounds [64 x i32], [64 x i32]* %175, i64 0, i64 %353, !intel-tbaa !48
  store i32 %352, i32* %354, align 4, !tbaa !57
  %355 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %356 = load i32, i32* %24, align 8, !tbaa !38
  %357 = sext i32 %356 to i64
  %358 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %357, !intel-tbaa !48
  store i32 %355, i32* %358, align 4, !tbaa !49
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
  %380 = load i32, i32* %339, align 4, !tbaa !60
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %380)
  %381 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !54
  %382 = icmp eq i32 %381, 0
  br i1 %382, label %383, label %420

383:                                              ; preds = %376
  %384 = icmp sgt i32 %379, %204
  %385 = icmp ne i32 %378, 0
  %386 = and i1 %385, %384
  br i1 %386, label %387, label %393

387:                                              ; preds = %383
  %388 = load i32, i32* %339, align 4, !tbaa !60
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
  %416 = load i32, i32* %24, align 8, !tbaa !38
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

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { nounwind readnone speculatable willreturn }
attributes #5 = { argmemonly nounwind willreturn }
attributes #6 = { nofree nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #7 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #8 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{i32 1, !"LTOPostLink", i32 1}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!10, !6, i64 4356}
!10 = !{!"struct@_ZTS7state_t", !6, i64 0, !11, i64 4, !12, i64 264, !12, i64 272, !12, i64 280, !13, i64 288, !6, i64 392, !6, i64 396, !14, i64 400, !6, i64 452, !6, i64 456, !6, i64 460, !6, i64 464, !6, i64 468, !6, i64 472, !6, i64 476, !12, i64 480, !12, i64 488, !15, i64 496, !11, i64 2544, !11, i64 2800, !17, i64 3056, !12, i64 4080, !12, i64 4088, !6, i64 4096, !11, i64 4100, !6, i64 4356, !6, i64 4360, !6, i64 4364, !6, i64 4368, !6, i64 4372, !6, i64 4376, !6, i64 4380, !6, i64 4384, !6, i64 4388, !6, i64 4392, !19, i64 4400}
!11 = !{!"array@_ZTSA64_i", !6, i64 0}
!12 = !{!"long long", !7, i64 0}
!13 = !{!"array@_ZTSA13_y", !12, i64 0}
!14 = !{!"array@_ZTSA13_i", !6, i64 0}
!15 = !{!"array@_ZTSA64_6move_x", !16, i64 0}
!16 = !{!"struct@_ZTS6move_x", !6, i64 0, !6, i64 4, !6, i64 8, !6, i64 12, !12, i64 16, !12, i64 24}
!17 = !{!"array@_ZTSA64_N7state_tUt_E", !18, i64 0}
!18 = !{!"struct@_ZTSN7state_tUt_E", !6, i64 0, !6, i64 4, !6, i64 8, !6, i64 12}
!19 = !{!"array@_ZTSA1000_y", !12, i64 0}
!20 = !{!10, !6, i64 460}
!21 = !{!10, !12, i64 480}
!22 = !{!23, !23, i64 0}
!23 = !{!"pointer@_ZTSP9ttentry_t", !7, i64 0}
!24 = !{!25, !26, i64 0}
!25 = !{!"struct@_ZTS9ttentry_t", !26, i64 0}
!26 = !{!"array@_ZTSA4_10ttbucket_t", !27, i64 0}
!27 = !{!"struct@_ZTS10ttbucket_t", !6, i64 0, !28, i64 4, !28, i64 6, !7, i64 8, !7, i64 9, !7, i64 9, !7, i64 9, !7, i64 9, !7, i64 9}
!28 = !{!"short", !7, i64 0}
!29 = !{!26, !27, i64 0}
!30 = !{!27, !6, i64 0}
!31 = !{!25, !6, i64 0}
!32 = !{!10, !6, i64 4360}
!33 = !{!27, !7, i64 9}
!34 = !{!27, !7, i64 8}
!35 = !{!25, !7, i64 8}
!36 = !{!27, !28, i64 4}
!37 = !{!25, !28, i64 4}
!38 = !{!10, !6, i64 472}
!39 = !{!27, !28, i64 6}
!40 = !{!25, !28, i64 6}
!41 = !{!10, !12, i64 4080}
!42 = !{!10, !6, i64 476}
!43 = !{!44, !6, i64 12}
!44 = !{!"struct@_ZTS11gamestate_t", !6, i64 0, !6, i64 4, !6, i64 8, !6, i64 12, !6, i64 16, !6, i64 20, !6, i64 24, !6, i64 28, !6, i64 32, !6, i64 36, !6, i64 40, !6, i64 44, !6, i64 48, !6, i64 52, !6, i64 56, !6, i64 60, !45, i64 64, !46, i64 4064, !12, i64 36064, !6, i64 36072, !6, i64 36076, !6, i64 36080, !6, i64 36084, !6, i64 36088, !6, i64 36092, !6, i64 36096, !6, i64 36100}
!45 = !{!"array@_ZTSA1000_i", !6, i64 0}
!46 = !{!"array@_ZTSA1000_6move_x", !16, i64 0}
!47 = !{!10, !11, i64 4100}
!48 = !{!11, !6, i64 0}
!49 = !{!10, !6, i64 4100}
!50 = !{!10, !14, i64 400}
!51 = !{!14, !6, i64 0}
!52 = !{!10, !6, i64 400}
!53 = !{!44, !6, i64 4}
!54 = !{!44, !6, i64 36096}
!55 = !{!10, !6, i64 456}
!56 = !{!10, !11, i64 2544}
!57 = !{!10, !6, i64 2544}
!58 = !{!10, !11, i64 2800}
!59 = !{!10, !6, i64 2800}
!60 = !{!61, !6, i64 0}
!61 = !{!"array@_ZTSA240_i", !6, i64 0}
!62 = !{!10, !6, i64 4}
!63 = !{!64, !6, i64 0}
!64 = !{!"array@_ZTSA14_i", !6, i64 0}
!65 = !{!44, !6, i64 60}
!66 = !{!19, !12, i64 0}
!67 = !{!10, !12, i64 4400}
!68 = !{!44, !6, i64 20}
!69 = !{!10, !6, i64 0}
!70 = !{!71, !72, i64 0}
!71 = !{!"array@_ZTSA8_A12_A64_i", !72, i64 0}
!72 = !{!"array@_ZTSA12_A64_i", !11, i64 0}
!73 = !{!72, !11, i64 0}
!74 = !{!71, !6, i64 0}
!75 = !{!10, !12, i64 4088}
!76 = !{!10, !6, i64 4096}
