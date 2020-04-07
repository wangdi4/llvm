; Checks that IPO Prefetch pass can properly identify the prefetch opportunity, generate the prefetch
; function, and generate 2 calls to prefetch function in 2 host functions: 1 call to prefetch inside
; each host.
;
; [Note]
; The IPO prefetch pass is a module pass that specifically examines code patterns like those exhibited in
; cpu2017/531.deepsjeng (531), generate a prefetch function based on one of the existing functions in 531
; and insert multiple calls to this prefetch function at specific pattern-matched host locations.
;
; This LIT testcase (intel_ipo_prefetch4.ll) is similar to intel_ipo_prefetch0.ll lit case, except the
; LLVM IR instructions are completely re-generated. Recent LLVM community pulldowns introduced significant changes
; reflected on the LLVM IR level. This makes the previously captured LLVM IRs out of date. The introduction
; of this LIT testcase aims to do the needed refresh and brings LLVM IR patterns for IPO Prefetch upto date.
;
; The LIT testcase presented here is a vastly simplified version of 531.
; It takes the following 3 functions after all modules have been compiled individually and linked together:
; - _Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(): an existing function that the prefetch function is based upon;
; - _Z7qsearchP7state_tiiii(): host function1 where a call to prefetch function will be inserted;
; - _Z6searchP7state_tiiiii(): host function2 where a call to prefetch function will be inserted.
;
; Only the 3 functions listed above, plus any global data and metadata they use, will appear in this LIT test.
; All other relevant functions are reduced to declarations only. Their function bodies are purged, as is any global variable
; or metadata that is not used directly by any of the above 3 functions.
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

; ModuleID = '<stdin>'
source_filename = "<stdin>"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.state_t = type { i32, [64 x i32], i64, i64, i64, [13 x i64], i32, i32, [13 x i32], i32, i32, i32, i32, i32, i32, i32, i64, i64, [64 x %struct.move_x], [64 x i32], [64 x i32], [64 x %struct.anon], i64, i64, i32, [64 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i64] }
%struct.move_x = type { i32, i32, i32, i32, i64, i64 }
%struct.anon = type { i32, i32, i32, i32 }
%struct.gamestate_t = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i32], [1000 x %struct.move_x], i64, i32, i32, i32, i32, i32, i32, i32, i32 }
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

; Function Attrs: uwtable
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #0

; Function Attrs: nounwind uwtable
declare i32 @_Z11check_legalP7state_ti(%struct.state_t*, i32) #1

; Function Attrs: nounwind uwtable
declare i32 @_Z11is_attackedP7state_tii(%struct.state_t* nocapture readonly, i32, i32) #1

; Function Attrs: nofree norecurse nounwind uwtable
declare i64 @_Z11FileAttacksyj(i64, i32) #2

; Function Attrs: nofree norecurse nounwind uwtable
declare i64 @_Z11RankAttacksyj(i64, i32) #2

; Function Attrs: norecurse nounwind readonly uwtable
declare zeroext i16 @_Z12compact_movei(i32) #3

; Function Attrs: norecurse nounwind readonly uwtable
declare i32 @_Z4logLi(i32) #3

; Function Attrs: nounwind uwtable
declare i32 @_Z12gen_capturesP7state_tPi(%struct.state_t*, i32*) #1

; Function Attrs: nofree norecurse nounwind uwtable
declare i64 @_Z11RookAttacksP7state_ti(%struct.state_t* nocapture readonly, i32) #2

; Function Attrs: nounwind uwtable
declare i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t*, i32*, i32) #1

; Function Attrs: nofree norecurse nounwind uwtable
declare i64 @_Z13BishopAttacksP7state_ti(%struct.state_t* nocapture readonly, i32) #2

; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nocapture readonly) #2

; Function Attrs: nounwind uwtable
declare i32 @_Z15gen_good_checksP7state_tPi(%struct.state_t*, i32*) #1

; Function Attrs: nounwind uwtable
declare i32 @_Z3genP7state_tPi(%struct.state_t*, i32*) #1

; Function Attrs: nounwind uwtable
declare i32 @_Z3seeP7state_tiiii(%struct.state_t*, i32, i32, i32, i32) #1

; Function Attrs: nounwind uwtable
declare i32 @_Z4evalP7state_tiii(%struct.state_t*, i32, i32, i32) #1

; Function Attrs: norecurse nounwind readnone uwtable
declare void @_Z4makeP7state_ti(%struct.state_t*, i32) #4

; Function Attrs: norecurse nounwind readnone uwtable
declare void @_Z6unmakeP7state_ti(%struct.state_t*, i32) #4

; Function Attrs: norecurse nounwind readnone uwtable
declare void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nocapture, i32, i32, i32, i32, i32, i32, i32, i32) #4

; Function Attrs: nofree norecurse nounwind uwtable
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

; Function Attrs: nounwind readnone uwtable
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #6

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %puts = tail call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([20 x i8], [20 x i8]* @.str.4, i64 0, i64 0))
  ret i32 0
}

; Function Attrs: nofree norecurse nounwind uwtable
define internal i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nocapture %0, i32* nocapture %1, i32 %2, i32 %3, i32* nocapture %4, i32* nocapture %5, i32* nocapture %6, i32* nocapture %7, i32* nocapture %8, i32 %9) #2 {
  store i32 1, i32* %6, align 4, !tbaa !6
  %11 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 26, !intel-tbaa !10
  %12 = load i32, i32* %11, align 4, !tbaa !10
  %13 = add i32 %12, 1
  store i32 %13, i32* %11, align 4, !tbaa !10
  %14 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !21
  %15 = load i32, i32* %14, align 4, !tbaa !21
  %16 = icmp eq i32 %15, 0
  %17 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %18 = load i64, i64* %17, align 8, !tbaa !22
  %19 = zext i1 %16 to i64
  %20 = add i64 %18, %19
  %21 = trunc i64 %20 to i32
  %22 = load %struct.ttentry_t*, %struct.ttentry_t** @TTable, align 8, !tbaa !23
  %23 = load i32, i32* @TTSize, align 4, !tbaa !6
  %24 = urem i32 %21, %23
  %25 = zext i32 %24 to i64
  %26 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %22, i64 %25
  %27 = lshr i64 %20, 32
  %28 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %26, i64 0, i32 0, !intel-tbaa !25
  %29 = trunc i64 %27 to i32
  br label %33

30:                                               ; preds = %33
  %31 = add nuw nsw i64 %34, 1
  %32 = icmp eq i64 %31, 4
  br i1 %32, label %132, label %33

33:                                               ; preds = %30, %10
  %34 = phi i64 [ 0, %10 ], [ %31, %30 ]
  %35 = getelementptr inbounds [4 x %struct.ttbucket_t], [4 x %struct.ttbucket_t]* %28, i64 0, i64 %34, !intel-tbaa !30
  %36 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 0, !intel-tbaa !31
  %37 = load i32, i32* %36, align 4, !tbaa !32
  %38 = icmp eq i32 %37, %29
  br i1 %38, label %39, label %30

39:                                               ; preds = %33
  %40 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 27, !intel-tbaa !33
  %41 = load i32, i32* %40, align 8, !tbaa !33
  %42 = add i32 %41, 1
  store i32 %42, i32* %40, align 8, !tbaa !33
  %43 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 4
  %44 = load i8, i8* %43, align 1, !tbaa !34
  %45 = lshr i8 %44, 5
  %46 = and i8 %45, 3
  %47 = zext i8 %46 to i32
  %48 = load i32, i32* @TTAge, align 4, !tbaa !6
  %49 = icmp eq i32 %48, %47
  br i1 %49, label %56, label %50

50:                                               ; preds = %39
  %51 = trunc i32 %48 to i8
  %52 = shl i8 %51, 5
  %53 = and i8 %52, 96
  %54 = and i8 %44, -97
  %55 = or i8 %53, %54
  store i8 %55, i8* %43, align 1, !tbaa !34
  br label %56

56:                                               ; preds = %50, %39
  %57 = phi i8 [ %44, %39 ], [ %55, %50 ]
  %58 = and i8 %57, 6
  %59 = icmp eq i8 %58, 2
  br i1 %59, label %60, label %72

60:                                               ; preds = %56
  %61 = add nsw i32 %9, -16
  %62 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 3, !intel-tbaa !35
  %63 = load i8, i8* %62, align 4, !tbaa !36
  %64 = zext i8 %63 to i32
  %65 = icmp sgt i32 %61, %64
  br i1 %65, label %72, label %66

66:                                               ; preds = %60
  %67 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !intel-tbaa !37
  %68 = load i16, i16* %67, align 4, !tbaa !38
  %69 = sext i16 %68 to i32
  %70 = icmp slt i32 %69, %3
  br i1 %70, label %71, label %72

71:                                               ; preds = %66
  store i32 0, i32* %6, align 4, !tbaa !6
  br label %72

72:                                               ; preds = %71, %66, %60, %56
  %73 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 3, !intel-tbaa !35
  %74 = load i8, i8* %73, align 4, !tbaa !36
  %75 = zext i8 %74 to i32
  %76 = icmp slt i32 %75, %9
  br i1 %76, label %109, label %77

77:                                               ; preds = %72
  %78 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !intel-tbaa !37
  %79 = load i16, i16* %78, align 4, !tbaa !38
  %80 = sext i16 %79 to i32
  store i32 %80, i32* %1, align 4, !tbaa !6
  %81 = icmp sgt i16 %79, 31500
  br i1 %81, label %82, label %87

82:                                               ; preds = %77
  %83 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !39
  %84 = load i32, i32* %83, align 8, !tbaa !39
  %85 = add nsw i32 %80, 1
  %86 = sub i32 %85, %84
  store i32 %86, i32* %1, align 4, !tbaa !6
  br label %94

87:                                               ; preds = %77
  %88 = icmp slt i16 %79, -31500
  br i1 %88, label %89, label %94

89:                                               ; preds = %87
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !39
  %91 = load i32, i32* %90, align 8, !tbaa !39
  %92 = add nsw i32 %80, -1
  %93 = add i32 %92, %91
  store i32 %93, i32* %1, align 4, !tbaa !6
  br label %94

94:                                               ; preds = %89, %87, %82
  %95 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 2, !intel-tbaa !40
  %96 = load i16, i16* %95, align 2, !tbaa !41
  %97 = zext i16 %96 to i32
  store i32 %97, i32* %4, align 4, !tbaa !6
  %98 = and i8 %57, 1
  %99 = zext i8 %98 to i32
  store i32 %99, i32* %5, align 4, !tbaa !6
  %100 = lshr i8 %57, 3
  %101 = and i8 %100, 1
  %102 = zext i8 %101 to i32
  store i32 %102, i32* %7, align 4, !tbaa !6
  %103 = lshr i8 %57, 4
  %104 = and i8 %103, 1
  %105 = zext i8 %104 to i32
  store i32 %105, i32* %8, align 4, !tbaa !6
  %106 = lshr i8 %57, 1
  %107 = and i8 %106, 3
  %108 = zext i8 %107 to i32
  br label %132

109:                                              ; preds = %72
  %110 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 2, !intel-tbaa !40
  %111 = load i16, i16* %110, align 2, !tbaa !41
  %112 = zext i16 %111 to i32
  store i32 %112, i32* %4, align 4, !tbaa !6
  %113 = and i8 %57, 1
  %114 = zext i8 %113 to i32
  store i32 %114, i32* %5, align 4, !tbaa !6
  %115 = lshr i8 %57, 3
  %116 = and i8 %115, 1
  %117 = zext i8 %116 to i32
  store i32 %117, i32* %7, align 4, !tbaa !6
  %118 = lshr i8 %57, 4
  %119 = and i8 %118, 1
  %120 = zext i8 %119 to i32
  store i32 %120, i32* %8, align 4, !tbaa !6
  %121 = lshr i8 %57, 1
  %122 = and i8 %121, 3
  switch i8 %122, label %125 [
    i8 1, label %123
    i8 2, label %124
  ]

123:                                              ; preds = %109
  store i32 -1000000, i32* %1, align 4, !tbaa !6
  br label %129

124:                                              ; preds = %109
  store i32 1000000, i32* %1, align 4, !tbaa !6
  br label %129

125:                                              ; preds = %109
  %126 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !intel-tbaa !37
  %127 = load i16, i16* %126, align 4, !tbaa !38
  %128 = sext i16 %127 to i32
  store i32 %128, i32* %1, align 4, !tbaa !6
  br label %129

129:                                              ; preds = %125, %124, %123
  %130 = trunc i32 %9 to i8
  store i8 %130, i8* %73, align 4, !tbaa !36
  %131 = and i8 %57, -7
  store i8 %131, i8* %43, align 1, !tbaa !34
  br label %132

132:                                              ; preds = %129, %94, %30
  %133 = phi i32 [ %108, %94 ], [ 0, %129 ], [ 4, %30 ]
  ret i32 %133
}

; Function Attrs: uwtable
define internal i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5) #0 {
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
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %19) #7
  %20 = bitcast [240 x i32]* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %20) #7
  %21 = bitcast i32* %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %21) #7
  %22 = bitcast i32* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %22) #7
  %23 = bitcast i32* %11 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %23) #7
  %24 = bitcast i32* %12 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %24) #7
  %25 = bitcast i32* %13 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %25) #7
  %26 = bitcast i32* %14 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %26) #7
  %27 = bitcast i32* %15 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %27) #7
  %28 = bitcast [240 x i32]* %16 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %28) #7
  %29 = icmp slt i32 %3, 1
  br i1 %29, label %34, label %30

30:                                               ; preds = %6
  %31 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !39
  %32 = load i32, i32* %31, align 8, !tbaa !39
  %33 = icmp sgt i32 %32, 59
  br i1 %33, label %34, label %36

34:                                               ; preds = %30, %6
  %35 = tail call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 0, i32 0)
  br label %1119

36:                                               ; preds = %30
  %37 = getelementptr %struct.state_t, %struct.state_t* %0, i64 0, i32 22, !intel-tbaa !42
  %38 = load i64, i64* %37, align 8, !tbaa !42
  %39 = add i64 %38, 1
  store i64 %39, i64* %37, align 8, !tbaa !42
  %40 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %39)
  %41 = icmp eq i32 %40, 0
  br i1 %41, label %42, label %1119

42:                                               ; preds = %36
  %43 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nonnull @gamestate, %struct.state_t* nonnull %0)
  %44 = icmp eq i32 %43, 0
  br i1 %44, label %45, label %49

45:                                               ; preds = %42
  %46 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15, !intel-tbaa !43
  %47 = load i32, i32* %46, align 4, !tbaa !43
  %48 = icmp sgt i32 %47, 99
  br i1 %48, label %49, label %57

49:                                               ; preds = %45, %42
  %50 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4, !tbaa !44
  %51 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !21
  %52 = load i32, i32* %51, align 4, !tbaa !21
  %53 = icmp eq i32 %50, %52
  %54 = load i32, i32* @contempt, align 4, !tbaa !6
  %55 = sub nsw i32 0, %54
  %56 = select i1 %53, i32 %54, i32 %55
  br label %1119

57:                                               ; preds = %45
  %58 = load i32, i32* %31, align 8, !tbaa !39
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
  %73 = load i32, i32* %10, align 4, !tbaa !6
  br label %1119

74:                                               ; preds = %69
  %75 = load i32, i32* %10, align 4, !tbaa !6
  %76 = icmp sgt i32 %75, %64
  br i1 %76, label %85, label %1119

77:                                               ; preds = %69
  %78 = load i32, i32* %10, align 4, !tbaa !6
  %79 = icmp slt i32 %78, %70
  br i1 %79, label %85, label %1119

80:                                               ; preds = %69
  %81 = load i32, i32* %10, align 4, !tbaa !6
  %82 = icmp slt i32 %81, %70
  %83 = select i1 %82, i32 %5, i32 1
  br label %85

84:                                               ; preds = %69
  store i32 65535, i32* %13, align 4, !tbaa !6
  store i32 0, i32* %11, align 4, !tbaa !6
  store i32 0, i32* %14, align 4, !tbaa !6
  store i32 0, i32* %15, align 4, !tbaa !6
  br label %85

85:                                               ; preds = %84, %80, %77, %74, %69
  %86 = phi i32 [ %5, %69 ], [ %5, %84 ], [ 0, %74 ], [ 1, %77 ], [ %83, %80 ]
  %87 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25, !intel-tbaa !48
  %88 = load i32, i32* %31, align 8, !tbaa !39
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %89, !intel-tbaa !49
  %91 = load i32, i32* %90, align 4, !tbaa !50
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
  %104 = load i32, i32* %13, align 4, !tbaa !6
  %105 = load i32, i32* %11, align 4, !tbaa !6
  %106 = load i32, i32* %14, align 4, !tbaa !6
  %107 = load i32, i32* %15, align 4, !tbaa !6
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
  %118 = load i32, i32* %13, align 4, !tbaa !6
  %119 = load i32, i32* %11, align 4, !tbaa !6
  %120 = load i32, i32* %14, align 4, !tbaa !6
  %121 = load i32, i32* %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %115, i32 %64, i32 %70, i32 %118, i32 %119, i32 %120, i32 %121, i32 %3)
  br label %1119

122:                                              ; preds = %114, %112, %108, %85
  %123 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8, !intel-tbaa !51
  %124 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 9, !intel-tbaa !52
  %125 = load i32, i32* %124, align 4, !tbaa !53
  %126 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 7, !intel-tbaa !52
  %127 = load i32, i32* %126, align 4, !tbaa !53
  %128 = add nsw i32 %127, %125
  %129 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 11, !intel-tbaa !52
  %130 = load i32, i32* %129, align 4, !tbaa !53
  %131 = add nsw i32 %128, %130
  %132 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 3, !intel-tbaa !52
  %133 = load i32, i32* %132, align 4, !tbaa !53
  %134 = add nsw i32 %131, %133
  %135 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 10, !intel-tbaa !52
  %136 = load i32, i32* %135, align 8, !tbaa !53
  %137 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 8, !intel-tbaa !52
  %138 = load i32, i32* %137, align 8, !tbaa !53
  %139 = add nsw i32 %138, %136
  %140 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 12, !intel-tbaa !52
  %141 = load i32, i32* %140, align 8, !tbaa !53
  %142 = add nsw i32 %139, %141
  %143 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 4, !intel-tbaa !52
  %144 = load i32, i32* %143, align 8, !tbaa !53
  %145 = add nsw i32 %142, %144
  store i32 0, i32* %11, align 4, !tbaa !6
  %146 = icmp eq i32 %4, 0
  br i1 %146, label %147, label %231

147:                                              ; preds = %122
  %148 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !21
  %149 = load i32, i32* %148, align 4, !tbaa !21
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
  %162 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !tbaa !54
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
  %174 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %175 = icmp eq i32 %174, 0
  br i1 %175, label %176, label %1119

176:                                              ; preds = %172
  %177 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !tbaa !54
  %178 = icmp eq i32 %177, 2
  %179 = icmp slt i32 %173, %70
  %180 = and i1 %179, %178
  br i1 %180, label %248, label %181

181:                                              ; preds = %176
  %182 = load i32, i32* %148, align 4, !tbaa !21
  br label %183

183:                                              ; preds = %181, %161
  %184 = phi i32 [ %182, %181 ], [ %149, %161 ]
  %185 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 10, !intel-tbaa !56
  %186 = load i32, i32* %185, align 8, !tbaa !56
  store i32 0, i32* %185, align 8, !tbaa !56
  %187 = xor i32 %184, 1
  store i32 %187, i32* %148, align 4, !tbaa !21
  %188 = load i32, i32* %31, align 8, !tbaa !39
  %189 = add nsw i32 %188, 1
  store i32 %189, i32* %31, align 8, !tbaa !39
  %190 = load i32, i32* %46, align 4, !tbaa !43
  %191 = add nsw i32 %190, 1
  store i32 %191, i32* %46, align 4, !tbaa !43
  %192 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19, !intel-tbaa !57
  %193 = sext i32 %188 to i64
  %194 = getelementptr inbounds [64 x i32], [64 x i32]* %192, i64 0, i64 %193, !intel-tbaa !49
  store i32 0, i32* %194, align 4, !tbaa !58
  %195 = sext i32 %189 to i64
  %196 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %195, !intel-tbaa !49
  store i32 0, i32* %196, align 4, !tbaa !50
  %197 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 20, !intel-tbaa !59
  %198 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %193, !intel-tbaa !49
  %199 = load i32, i32* %198, align 4, !tbaa !60
  %200 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %195, !intel-tbaa !49
  store i32 %199, i32* %200, align 4, !tbaa !60
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
  %214 = load i32, i32* %46, align 4, !tbaa !43
  %215 = add nsw i32 %214, -1
  store i32 %215, i32* %46, align 4, !tbaa !43
  %216 = load i32, i32* %31, align 8, !tbaa !39
  %217 = add nsw i32 %216, -1
  store i32 %217, i32* %31, align 8, !tbaa !39
  %218 = load i32, i32* %148, align 4, !tbaa !21
  %219 = xor i32 %218, 1
  store i32 %219, i32* %148, align 4, !tbaa !21
  store i32 %186, i32* %185, align 8, !tbaa !56
  %220 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %221 = icmp eq i32 %220, 0
  br i1 %221, label %222, label %1119

222:                                              ; preds = %211
  %223 = icmp sgt i32 %70, %213
  br i1 %223, label %228, label %224

224:                                              ; preds = %222
  %225 = load i32, i32* %13, align 4, !tbaa !6
  %226 = load i32, i32* %11, align 4, !tbaa !6
  %227 = load i32, i32* %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %213, i32 %64, i32 %70, i32 %225, i32 %226, i32 0, i32 %227, i32 %3)
  br label %1119

228:                                              ; preds = %222
  %229 = icmp sgt i32 %212, 31400
  br i1 %229, label %230, label %248

230:                                              ; preds = %228
  store i32 1, i32* %11, align 4, !tbaa !6
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
  %239 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %240 = icmp eq i32 %239, 0
  br i1 %240, label %241, label %1119

241:                                              ; preds = %237
  %242 = icmp sgt i32 %238, %64
  br i1 %242, label %248, label %243

243:                                              ; preds = %241
  %244 = load i32, i32* %13, align 4, !tbaa !6
  %245 = load i32, i32* %11, align 4, !tbaa !6
  %246 = load i32, i32* %14, align 4, !tbaa !6
  %247 = load i32, i32* %15, align 4, !tbaa !6
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
  store i32 0, i32* %9, align 4, !tbaa !6
  %254 = icmp sgt i32 %251, 0
  br i1 %254, label %257, label %280

255:                                              ; preds = %248
  %256 = call i32 @_Z3genP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %249)
  br label %280

257:                                              ; preds = %257, %253
  %258 = phi i32 [ %270, %257 ], [ 0, %253 ]
  %259 = phi i32 [ %276, %257 ], [ 0, %253 ]
  %260 = sext i32 %259 to i64
  %261 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %260, !intel-tbaa !61
  %262 = load i32, i32* %261, align 4, !tbaa !61
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %262)
  %263 = load i32, i32* %9, align 4, !tbaa !6
  %264 = sext i32 %263 to i64
  %265 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %264, !intel-tbaa !61
  %266 = load i32, i32* %265, align 4, !tbaa !61
  %267 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %266)
  %268 = icmp ne i32 %267, 0
  %269 = zext i1 %268 to i32
  %270 = add nuw nsw i32 %258, %269
  %271 = load i32, i32* %9, align 4, !tbaa !6
  %272 = sext i32 %271 to i64
  %273 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %272, !intel-tbaa !61
  %274 = load i32, i32* %273, align 4, !tbaa !61
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %274)
  %275 = load i32, i32* %9, align 4, !tbaa !6
  %276 = add nsw i32 %275, 1
  store i32 %276, i32* %9, align 4, !tbaa !6
  %277 = icmp slt i32 %276, %251
  %278 = icmp ult i32 %270, 2
  %279 = and i1 %277, %278
  br i1 %279, label %257, label %280

280:                                              ; preds = %257, %255, %253, %250
  %281 = phi i32 [ 0, %250 ], [ %256, %255 ], [ %251, %253 ], [ %251, %257 ]
  %282 = phi i32 [ 0, %250 ], [ %256, %255 ], [ 0, %253 ], [ %270, %257 ]
  %283 = getelementptr inbounds [240 x i32], [240 x i32]* %8, i64 0, i64 0
  %284 = load i32, i32* %13, align 4, !tbaa !6
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
  store i32 0, i32* %9, align 4, !tbaa !6
  %292 = icmp sgt i32 %281, 0
  br i1 %292, label %293, label %324

293:                                              ; preds = %291
  %294 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %295 = zext i32 %281 to i64
  br label %296

296:                                              ; preds = %318, %293
  %297 = phi i64 [ 0, %293 ], [ %320, %318 ]
  %298 = phi i32 [ 0, %293 ], [ %319, %318 ]
  %299 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %297, !intel-tbaa !61
  %300 = load i32, i32* %299, align 4, !tbaa !61
  %301 = lshr i32 %300, 19
  %302 = and i32 %301, 15
  %303 = icmp eq i32 %302, 13
  br i1 %303, label %318, label %304

304:                                              ; preds = %296
  %305 = lshr i32 %300, 6
  %306 = and i32 %305, 63
  %307 = zext i32 %306 to i64
  %308 = getelementptr inbounds [64 x i32], [64 x i32]* %294, i64 0, i64 %307, !intel-tbaa !49
  %309 = load i32, i32* %308, align 4, !tbaa !63
  %310 = sext i32 %309 to i64
  %311 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %310, !intel-tbaa !64
  %312 = load i32, i32* %311, align 4, !tbaa !64
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
  store i32 %281, i32* %9, align 4, !tbaa !6
  %323 = icmp eq i32 %319, 0
  br i1 %323, label %324, label %336

324:                                              ; preds = %322, %291
  %325 = bitcast i32* %17 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %325) #7
  %326 = bitcast i32* %18 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %326) #7
  %327 = ashr i32 %3, 1
  %328 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %327, i32 0, i32 %86)
  %329 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* %0, i32* nonnull %17, i32 0, i32 0, i32* nonnull %18, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32 0)
  %330 = icmp eq i32 %329, 4
  br i1 %330, label %333, label %331

331:                                              ; preds = %324
  %332 = load i32, i32* %18, align 4, !tbaa !6
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %249, i32* nonnull %283, i32 %281, i32 %332)
  br label %335

333:                                              ; preds = %324
  %334 = load i32, i32* %13, align 4, !tbaa !6
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %249, i32* nonnull %283, i32 %281, i32 %334)
  br label %335

335:                                              ; preds = %333, %331
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %326) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %325) #7
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
  %345 = load i32, i32* %31, align 8, !tbaa !39
  %346 = add nsw i32 %345, -1
  %347 = sext i32 %346 to i64
  %348 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %347, !intel-tbaa !49
  %349 = load i32, i32* %348, align 4, !tbaa !50
  %350 = icmp eq i32 %349, 0
  br i1 %350, label %351, label %508

351:                                              ; preds = %344
  %352 = icmp slt i32 %345, 3
  br i1 %352, label %367, label %353

353:                                              ; preds = %351
  %354 = add nsw i32 %345, -2
  %355 = sext i32 %354 to i64
  %356 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %355, !intel-tbaa !49
  %357 = load i32, i32* %356, align 4, !tbaa !50
  %358 = icmp eq i32 %357, 0
  br i1 %358, label %359, label %508

359:                                              ; preds = %353
  %360 = icmp slt i32 %345, 4
  br i1 %360, label %367, label %361

361:                                              ; preds = %359
  %362 = add nsw i32 %345, -3
  %363 = sext i32 %362 to i64
  %364 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %363, !intel-tbaa !49
  %365 = load i32, i32* %364, align 4, !tbaa !50
  %366 = icmp eq i32 %365, 0
  br i1 %366, label %367, label %508

367:                                              ; preds = %361, %359, %351
  store i32 -1, i32* %9, align 4, !tbaa !6
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
  %387 = load i32, i32* %9, align 4, !tbaa !6
  %388 = sext i32 %387 to i64
  %389 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %388, !intel-tbaa !61
  %390 = load i32, i32* %389, align 4, !tbaa !61
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %390)
  %391 = load i32, i32* %9, align 4, !tbaa !6
  %392 = sext i32 %391 to i64
  %393 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %392, !intel-tbaa !61
  %394 = load i32, i32* %393, align 4, !tbaa !61
  %395 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %394)
  %396 = icmp eq i32 %395, 0
  br i1 %396, label %421, label %397

397:                                              ; preds = %382
  %398 = load i64, i64* %371, align 8, !tbaa !22
  %399 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !66
  %400 = load i32, i32* %31, align 8, !tbaa !39
  %401 = add i32 %400, -1
  %402 = add i32 %401, %399
  %403 = sext i32 %402 to i64
  %404 = getelementptr inbounds [1000 x i64], [1000 x i64]* %372, i64 0, i64 %403, !intel-tbaa !67
  store i64 %398, i64* %404, align 8, !tbaa !68
  %405 = load i32, i32* %9, align 4, !tbaa !6
  %406 = sext i32 %405 to i64
  %407 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %406, !intel-tbaa !61
  %408 = load i32, i32* %407, align 4, !tbaa !61
  %409 = sext i32 %401 to i64
  %410 = getelementptr inbounds [64 x i32], [64 x i32]* %373, i64 0, i64 %409, !intel-tbaa !49
  store i32 %408, i32* %410, align 4, !tbaa !58
  %411 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %412 = load i32, i32* %31, align 8, !tbaa !39
  %413 = sext i32 %412 to i64
  %414 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %413, !intel-tbaa !49
  store i32 %411, i32* %414, align 4, !tbaa !50
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
  %424 = load i32, i32* %9, align 4, !tbaa !6
  %425 = sext i32 %424 to i64
  %426 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %425, !intel-tbaa !61
  %427 = load i32, i32* %426, align 4, !tbaa !61
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %427)
  %428 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
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
  %448 = load i32, i32* %9, align 4, !tbaa !6
  %449 = sext i32 %448 to i64
  %450 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %449, !intel-tbaa !61
  %451 = load i32, i32* %450, align 4, !tbaa !61
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %451)
  %452 = load i32, i32* %9, align 4, !tbaa !6
  %453 = sext i32 %452 to i64
  %454 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %453, !intel-tbaa !61
  %455 = load i32, i32* %454, align 4, !tbaa !61
  %456 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %455)
  %457 = icmp eq i32 %456, 0
  br i1 %457, label %482, label %458

458:                                              ; preds = %443
  %459 = load i64, i64* %371, align 8, !tbaa !22
  %460 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !66
  %461 = load i32, i32* %31, align 8, !tbaa !39
  %462 = add i32 %461, -1
  %463 = add i32 %462, %460
  %464 = sext i32 %463 to i64
  %465 = getelementptr inbounds [1000 x i64], [1000 x i64]* %372, i64 0, i64 %464, !intel-tbaa !67
  store i64 %459, i64* %465, align 8, !tbaa !68
  %466 = load i32, i32* %9, align 4, !tbaa !6
  %467 = sext i32 %466 to i64
  %468 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %467, !intel-tbaa !61
  %469 = load i32, i32* %468, align 4, !tbaa !61
  %470 = sext i32 %462 to i64
  %471 = getelementptr inbounds [64 x i32], [64 x i32]* %373, i64 0, i64 %470, !intel-tbaa !49
  store i32 %469, i32* %471, align 4, !tbaa !58
  %472 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %473 = load i32, i32* %31, align 8, !tbaa !39
  %474 = sext i32 %473 to i64
  %475 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %474, !intel-tbaa !49
  store i32 %472, i32* %475, align 4, !tbaa !50
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
  %485 = load i32, i32* %9, align 4, !tbaa !6
  %486 = sext i32 %485 to i64
  %487 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %486, !intel-tbaa !61
  %488 = load i32, i32* %487, align 4, !tbaa !61
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %488)
  %489 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
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
  %499 = load i32, i32* %13, align 4, !tbaa !6
  %500 = load i32, i32* %11, align 4, !tbaa !6
  %501 = load i32, i32* %15, align 4, !tbaa !6
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
  %509 = phi i32 [ -32000, %344 ], [ -32000, %361 ], [ -32000, %353 ], [ -32000, %336 ], [ -32000, %367 ], [ %423, %437 ], [ %423, %421 ], [ %484, %502 ], [ %484, %482 ]
  %510 = load i32, i32* %14, align 4, !tbaa !6
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
  store i32 -1, i32* %9, align 4, !tbaa !6
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
  %549 = load i32, i32* %9, align 4, !tbaa !6
  %550 = sext i32 %549 to i64
  %551 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %550, !intel-tbaa !61
  %552 = load i32, i32* %551, align 4, !tbaa !61
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %552)
  %553 = load i32, i32* %9, align 4, !tbaa !6
  %554 = sext i32 %553 to i64
  %555 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %554, !intel-tbaa !61
  %556 = load i32, i32* %555, align 4, !tbaa !61
  %557 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %556)
  %558 = icmp eq i32 %557, 0
  br i1 %558, label %593, label %559

559:                                              ; preds = %545
  %560 = load i64, i64* %533, align 8, !tbaa !22
  %561 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !66
  %562 = load i32, i32* %31, align 8, !tbaa !39
  %563 = add i32 %562, -1
  %564 = add i32 %563, %561
  %565 = sext i32 %564 to i64
  %566 = getelementptr inbounds [1000 x i64], [1000 x i64]* %534, i64 0, i64 %565, !intel-tbaa !67
  store i64 %560, i64* %566, align 8, !tbaa !68
  %567 = load i32, i32* %9, align 4, !tbaa !6
  %568 = sext i32 %567 to i64
  %569 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %568, !intel-tbaa !61
  %570 = load i32, i32* %569, align 4, !tbaa !61
  %571 = sext i32 %563 to i64
  %572 = getelementptr inbounds [64 x i32], [64 x i32]* %535, i64 0, i64 %571, !intel-tbaa !49
  store i32 %570, i32* %572, align 4, !tbaa !58
  %573 = add nsw i32 %546, 1
  %574 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %575 = load i32, i32* %31, align 8, !tbaa !39
  %576 = sext i32 %575 to i64
  %577 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %576, !intel-tbaa !49
  store i32 %574, i32* %577, align 4, !tbaa !50
  %578 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %537, i32 %538, i32 1)
  %579 = icmp eq i32 %548, 0
  br i1 %579, label %587, label %580

580:                                              ; preds = %559
  %581 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %540, i32 %539, i32 %536, i32 0, i32 %542)
  %582 = sub nsw i32 0, %581
  %583 = icmp slt i32 %64, %582
  br i1 %583, label %584, label %585

584:                                              ; preds = %580
  store i32 1, i32* %14, align 4, !tbaa !6
  br label %593

585:                                              ; preds = %580
  store i32 0, i32* %14, align 4, !tbaa !6
  %586 = add nsw i32 %546, 11
  br label %593

587:                                              ; preds = %559
  %588 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %543, i32 %538, i32 %536, i32 0, i32 0)
  %589 = sub nsw i32 0, %588
  %590 = icmp slt i32 %544, %589
  br i1 %590, label %591, label %593

591:                                              ; preds = %587
  store i32 0, i32* %14, align 4, !tbaa !6
  %592 = add nsw i32 %546, 11
  br label %593

593:                                              ; preds = %591, %587, %585, %584, %545
  %594 = phi i32 [ %548, %545 ], [ 0, %587 ], [ 0, %591 ], [ 0, %584 ], [ 0, %585 ]
  %595 = phi i32 [ %547, %545 ], [ %589, %587 ], [ %589, %591 ], [ %582, %584 ], [ %582, %585 ]
  %596 = phi i32 [ %546, %545 ], [ %573, %587 ], [ %592, %591 ], [ %573, %584 ], [ %586, %585 ]
  %597 = load i32, i32* %9, align 4, !tbaa !6
  %598 = sext i32 %597 to i64
  %599 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %598, !intel-tbaa !61
  %600 = load i32, i32* %599, align 4, !tbaa !61
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
  %611 = load i32, i32* %31, align 8, !tbaa !39
  %612 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4, !tbaa !69
  %613 = shl nsw i32 %612, 1
  %614 = icmp sle i32 %611, %613
  br label %615

615:                                              ; preds = %610, %608
  %616 = phi i1 [ false, %608 ], [ %614, %610 ]
  store i32 -1, i32* %9, align 4, !tbaa !6
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
  %662 = load i32, i32* %31, align 8, !tbaa !39
  %663 = icmp slt i32 %662, 60
  %664 = load i32, i32* %9, align 4, !tbaa !6
  %665 = sext i32 %664 to i64
  br i1 %663, label %666, label %746

666:                                              ; preds = %660
  %667 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %665, !intel-tbaa !61
  %668 = load i32, i32* %667, align 4, !tbaa !61
  %669 = lshr i32 %668, 6
  %670 = and i32 %669, 63
  %671 = zext i32 %670 to i64
  %672 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %671, !intel-tbaa !49
  %673 = load i32, i32* %672, align 4, !tbaa !63
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
  %692 = getelementptr inbounds [64 x i32], [64 x i32]* %626, i64 0, i64 %691, !intel-tbaa !49
  %693 = load i32, i32* %692, align 4, !tbaa !58
  %694 = lshr i32 %693, 19
  %695 = and i32 %694, 15
  %696 = icmp eq i32 %695, 13
  br i1 %696, label %719, label %697

697:                                              ; preds = %689
  %698 = zext i32 %687 to i64
  %699 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %698, !intel-tbaa !64
  %700 = load i32, i32* %699, align 4, !tbaa !64
  %701 = zext i32 %695 to i64
  %702 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %701, !intel-tbaa !64
  %703 = load i32, i32* %702, align 4, !tbaa !64
  %704 = icmp eq i32 %700, %703
  br i1 %704, label %705, label %719

705:                                              ; preds = %697
  %706 = and i32 %668, 63
  %707 = and i32 %693, 63
  %708 = icmp eq i32 %706, %707
  br i1 %708, label %709, label %719

709:                                              ; preds = %705
  %710 = load i32, i32* %627, align 4, !tbaa !21
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
  %721 = load i32, i32* %14, align 4, !tbaa !6
  %722 = icmp eq i32 %721, 1
  %723 = icmp ne i32 %720, 0
  %724 = and i1 %723, %722
  %725 = and i1 %656, %724
  br i1 %725, label %726, label %727

726:                                              ; preds = %719
  store i32 1, i32* %15, align 4, !tbaa !6
  br label %732

727:                                              ; preds = %719
  %728 = icmp eq i32 %720, 0
  %729 = and i1 %728, %722
  %730 = and i1 %656, %729
  br i1 %730, label %731, label %732

731:                                              ; preds = %727
  store i32 0, i32* %15, align 4, !tbaa !6
  br label %735

732:                                              ; preds = %727, %726
  %733 = icmp slt i32 %720, 4
  %734 = select i1 %733, i32 %720, i32 4
  br label %735

735:                                              ; preds = %732, %731
  %736 = phi i32 [ %734, %732 ], [ %628, %731 ]
  %737 = load i32, i32* %9, align 4, !tbaa !6
  %738 = sext i32 %737 to i64
  %739 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %738, !intel-tbaa !61
  %740 = load i32, i32* %739, align 4, !tbaa !61
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
  %749 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %747, !intel-tbaa !61
  %750 = load i32, i32* %749, align 4, !tbaa !61
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
  %762 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %761, !intel-tbaa !49
  %763 = load i32, i32* %762, align 4, !tbaa !63
  %764 = add nsw i32 %763, -1
  %765 = and i32 %750, 63
  %766 = load i32, i32* %633, align 8, !tbaa !70
  %767 = sext i32 %766 to i64
  %768 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %767, !intel-tbaa !71
  %769 = sext i32 %764 to i64
  %770 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %768, i64 0, i64 %769, !intel-tbaa !74
  %771 = zext i32 %765 to i64
  %772 = getelementptr inbounds [64 x i32], [64 x i32]* %770, i64 0, i64 %771, !intel-tbaa !49
  %773 = load i32, i32* %772, align 4, !tbaa !75
  %774 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %767, !intel-tbaa !71
  %775 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %774, i64 0, i64 %769, !intel-tbaa !74
  %776 = getelementptr inbounds [64 x i32], [64 x i32]* %775, i64 0, i64 %771, !intel-tbaa !49
  %777 = load i32, i32* %776, align 4, !tbaa !75
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
  %793 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
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
  %810 = load i32, i32* %627, align 4, !tbaa !21
  %811 = icmp eq i32 %810, 0
  %812 = zext i1 %811 to i32
  %813 = lshr i32 %750, 6
  %814 = and i32 %813, 63
  %815 = and i32 %750, 63
  %816 = lshr i32 %750, 12
  %817 = and i32 %816, 15
  %818 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %812, i32 %814, i32 %815, i32 %817)
  %819 = load i32, i32* %9, align 4, !tbaa !6
  %820 = sext i32 %819 to i64
  br label %821

821:                                              ; preds = %809, %806
  %822 = phi i64 [ %747, %806 ], [ %820, %809 ]
  %823 = phi i32 [ -1000000, %806 ], [ %818, %809 ]
  %824 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %822, !intel-tbaa !61
  %825 = load i32, i32* %824, align 4, !tbaa !61
  call void @_Z4makeP7state_ti(%struct.state_t* nonnull %0, i32 %825)
  %826 = load i32, i32* %9, align 4, !tbaa !6
  %827 = sext i32 %826 to i64
  %828 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %827, !intel-tbaa !61
  %829 = load i32, i32* %828, align 4, !tbaa !61
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
  %844 = load i32, i32* %9, align 4, !tbaa !6
  %845 = sext i32 %844 to i64
  %846 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %845, !intel-tbaa !61
  %847 = load i32, i32* %846, align 4, !tbaa !61
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
  %856 = load i32, i32* %9, align 4, !tbaa !6
  %857 = sext i32 %856 to i64
  %858 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %857, !intel-tbaa !61
  %859 = load i32, i32* %858, align 4, !tbaa !61
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
  %873 = load i32, i32* %31, align 8, !tbaa !39
  %874 = sext i32 %873 to i64
  %875 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %874, !intel-tbaa !49
  store i32 %833, i32* %875, align 4, !tbaa !50
  %876 = load i64, i64* %643, align 8, !tbaa !22
  %877 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !66
  %878 = add i32 %873, -1
  %879 = add i32 %878, %877
  %880 = sext i32 %879 to i64
  %881 = getelementptr inbounds [1000 x i64], [1000 x i64]* %644, i64 0, i64 %880, !intel-tbaa !67
  store i64 %876, i64* %881, align 8, !tbaa !68
  %882 = load i32, i32* %9, align 4, !tbaa !6
  %883 = sext i32 %882 to i64
  %884 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %883, !intel-tbaa !61
  %885 = load i32, i32* %884, align 4, !tbaa !61
  %886 = sext i32 %878 to i64
  %887 = getelementptr inbounds [64 x i32], [64 x i32]* %626, i64 0, i64 %886, !intel-tbaa !49
  store i32 %885, i32* %887, align 4, !tbaa !58
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
  %899 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %898, !intel-tbaa !49
  %900 = load i32, i32* %899, align 4, !tbaa !63
  %901 = add nsw i32 %900, -1
  %902 = load i32, i32* %633, align 8, !tbaa !70
  %903 = sext i32 %902 to i64
  %904 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %903, !intel-tbaa !71
  %905 = sext i32 %901 to i64
  %906 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %904, i64 0, i64 %905, !intel-tbaa !74
  %907 = getelementptr inbounds [64 x i32], [64 x i32]* %906, i64 0, i64 %898, !intel-tbaa !49
  %908 = load i32, i32* %907, align 4, !tbaa !75
  %909 = shl i32 %908, 7
  %910 = add i32 %909, 128
  %911 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %903, !intel-tbaa !71
  %912 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %911, i64 0, i64 %905, !intel-tbaa !74
  %913 = getelementptr inbounds [64 x i32], [64 x i32]* %912, i64 0, i64 %898, !intel-tbaa !49
  %914 = load i32, i32* %913, align 4, !tbaa !75
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
  %959 = phi i32 [ %926, %956 ], [ %955, %954 ]
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
  %975 = phi i32 [ %653, %821 ], [ %973, %970 ]
  %976 = phi i32 [ 0, %821 ], [ 1, %970 ]
  %977 = phi i32 [ %797, %821 ], [ 0, %970 ]
  %978 = phi i32 [ %651, %821 ], [ %971, %970 ]
  %979 = load i32, i32* %9, align 4, !tbaa !6
  %980 = sext i32 %979 to i64
  %981 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %980, !intel-tbaa !61
  %982 = load i32, i32* %981, align 4, !tbaa !61
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %982)
  %983 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
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
  %991 = load i32, i32* %9, align 4, !tbaa !6
  %992 = sext i32 %991 to i64
  %993 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %992
  %994 = load i32, i32* %993, align 4, !tbaa !61
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
  %1004 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %1003, !intel-tbaa !61
  %1005 = load i32, i32* %1004, align 4, !tbaa !61
  %1006 = and i32 %1005, 7925760
  %1007 = icmp eq i32 %1006, 6815744
  br i1 %1007, label %1008, label %1047

1008:                                             ; preds = %1002
  %1009 = lshr i32 %1005, 6
  %1010 = and i32 %1009, 63
  %1011 = zext i32 %1010 to i64
  %1012 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %1011, !intel-tbaa !49
  %1013 = load i32, i32* %1012, align 4, !tbaa !63
  %1014 = add nsw i32 %1013, -1
  %1015 = and i32 %1005, 63
  %1016 = load i32, i32* %633, align 8, !tbaa !70
  %1017 = sext i32 %1016 to i64
  %1018 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %1017, !intel-tbaa !71
  %1019 = sext i32 %1014 to i64
  %1020 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1018, i64 0, i64 %1019, !intel-tbaa !74
  %1021 = zext i32 %1015 to i64
  %1022 = getelementptr inbounds [64 x i32], [64 x i32]* %1020, i64 0, i64 %1021, !intel-tbaa !49
  %1023 = load i32, i32* %1022, align 4, !tbaa !75
  %1024 = add nsw i32 %1023, %999
  store i32 %1024, i32* %1022, align 4, !tbaa !75
  %1025 = icmp sgt i32 %1024, 16384
  br i1 %1025, label %1026, label %1047

1026:                                             ; preds = %1008
  %1027 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %1017, !intel-tbaa !71
  br label %1028

1028:                                             ; preds = %1044, %1026
  %1029 = phi i64 [ 0, %1026 ], [ %1045, %1044 ]
  %1030 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1027, i64 0, i64 %1029, !intel-tbaa !74
  %1031 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1018, i64 0, i64 %1029, !intel-tbaa !74
  br label %1032

1032:                                             ; preds = %1032, %1028
  %1033 = phi i64 [ 0, %1028 ], [ %1042, %1032 ]
  %1034 = getelementptr inbounds [64 x i32], [64 x i32]* %1030, i64 0, i64 %1033, !intel-tbaa !49
  %1035 = load i32, i32* %1034, align 4, !tbaa !75
  %1036 = add nsw i32 %1035, 1
  %1037 = ashr i32 %1036, 1
  store i32 %1037, i32* %1034, align 4, !tbaa !75
  %1038 = getelementptr inbounds [64 x i32], [64 x i32]* %1031, i64 0, i64 %1033, !intel-tbaa !49
  %1039 = load i32, i32* %1038, align 4, !tbaa !75
  %1040 = add nsw i32 %1039, 1
  %1041 = ashr i32 %1040, 1
  store i32 %1041, i32* %1038, align 4, !tbaa !75
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
  %1051 = load i32, i32* %9, align 4, !tbaa !6
  %1052 = sext i32 %1051 to i64
  %1053 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %1052, !intel-tbaa !61
  %1054 = load i32, i32* %1053, align 4, !tbaa !61
  %1055 = call zeroext i16 @_Z12compact_movei(i32 %1054)
  %1056 = zext i16 %1055 to i32
  %1057 = load i32, i32* %11, align 4, !tbaa !6
  %1058 = load i32, i32* %14, align 4, !tbaa !6
  %1059 = load i32, i32* %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %978, i32 %64, i32 %70, i32 %1056, i32 %1057, i32 %1058, i32 %1059, i32 %3)
  br label %1119

1060:                                             ; preds = %989
  %1061 = call zeroext i16 @_Z12compact_movei(i32 %994)
  %1062 = zext i16 %1061 to i32
  store i32 %1062, i32* %13, align 4, !tbaa !6
  br label %1063

1063:                                             ; preds = %1060, %987
  %1064 = phi i32 [ %978, %1060 ], [ %650, %987 ]
  %1065 = load i32, i32* %9, align 4, !tbaa !6
  %1066 = sext i32 %1065 to i64
  %1067 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %1066, !intel-tbaa !61
  %1068 = load i32, i32* %1067, align 4, !tbaa !61
  %1069 = add nsw i32 %655, -1
  %1070 = sext i32 %1069 to i64
  %1071 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %1070, !intel-tbaa !61
  store i32 %1068, i32* %1071, align 4, !tbaa !61
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
  %1090 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %1091 = icmp eq i32 %1090, 0
  %1092 = icmp ne i32 %1089, 0
  %1093 = and i1 %1092, %1091
  br i1 %1093, label %1094, label %1106

1094:                                             ; preds = %1087
  %1095 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %1096 = icmp eq i32 %1095, 0
  %1097 = load i32, i32* %11, align 4, !tbaa !6
  %1098 = load i32, i32* %14, align 4, !tbaa !6
  %1099 = load i32, i32* %15, align 4, !tbaa !6
  br i1 %1096, label %1105, label %1100

1100:                                             ; preds = %1094
  %1101 = load i32, i32* %31, align 8, !tbaa !39
  %1102 = add nsw i32 %1101, -32000
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %1102, i32 %64, i32 %70, i32 0, i32 %1097, i32 %1098, i32 %1099, i32 %3)
  %1103 = load i32, i32* %31, align 8, !tbaa !39
  %1104 = add nsw i32 %1103, -32000
  br label %1119

1105:                                             ; preds = %1094
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 0, i32 %64, i32 %70, i32 0, i32 %1097, i32 %1098, i32 %1099, i32 %3)
  br label %1119

1106:                                             ; preds = %1087, %792
  %1107 = phi i1 [ %794, %792 ], [ %1091, %1087 ]
  %1108 = phi i32 [ %653, %792 ], [ %1088, %1087 ]
  %1109 = load i32, i32* %46, align 4, !tbaa !43
  %1110 = icmp sgt i32 %1109, 98
  %1111 = xor i1 %1107, true
  %1112 = or i1 %1110, %1111
  %1113 = select i1 %1110, i32 0, i32 %1108
  br i1 %1112, label %1119, label %1114

1114:                                             ; preds = %1106
  %1115 = load i32, i32* %13, align 4, !tbaa !6
  %1116 = load i32, i32* %11, align 4, !tbaa !6
  %1117 = load i32, i32* %14, align 4, !tbaa !6
  %1118 = load i32, i32* %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %1108, i32 %64, i32 %70, i32 %1115, i32 %1116, i32 %1117, i32 %1118, i32 %3)
  br label %1119

1119:                                             ; preds = %1114, %1106, %1105, %1100, %1050, %974, %498, %243, %237, %224, %211, %172, %117, %110, %103, %77, %74, %72, %67, %61, %49, %36, %34
  %1120 = phi i32 [ %35, %34 ], [ %56, %49 ], [ %73, %72 ], [ 0, %36 ], [ %59, %61 ], [ %65, %67 ], [ %75, %74 ], [ %78, %77 ], [ %92, %103 ], [ %111, %110 ], [ %92, %117 ], [ 0, %172 ], [ %213, %224 ], [ 0, %211 ], [ %70, %498 ], [ %1104, %1100 ], [ 0, %1105 ], [ %1113, %1106 ], [ %1108, %1114 ], [ 0, %237 ], [ %64, %243 ], [ %978, %1050 ], [ 0, %974 ]
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %28) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %27) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %26) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %25) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %24) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %23) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %22) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %21) #7
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %20) #7
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %19) #7
  ret i32 %1120
}

; Function Attrs: uwtable
define internal i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4) #0 {
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca [240 x i32], align 16
  %11 = alloca [240 x i32], align 16
  %12 = bitcast i32* %6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %12) #7
  %13 = bitcast i32* %7 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %13) #7
  %14 = bitcast i32* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %14) #7
  %15 = bitcast i32* %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %15) #7
  %16 = bitcast [240 x i32]* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %16) #7
  %17 = bitcast [240 x i32]* %11 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %17) #7
  %18 = getelementptr %struct.state_t, %struct.state_t* %0, i64 0, i32 22, !intel-tbaa !42
  %19 = load i64, i64* %18, align 8, !tbaa !42
  %20 = add i64 %19, 1
  store i64 %20, i64* %18, align 8, !tbaa !42
  %21 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 23, !intel-tbaa !76
  %22 = load i64, i64* %21, align 8, !tbaa !76
  %23 = add i64 %22, 1
  store i64 %23, i64* %21, align 8, !tbaa !76
  %24 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !39
  %25 = load i32, i32* %24, align 8, !tbaa !39
  %26 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 24, !intel-tbaa !77
  %27 = load i32, i32* %26, align 8, !tbaa !77
  %28 = icmp sgt i32 %25, %27
  br i1 %28, label %29, label %30

29:                                               ; preds = %5
  store i32 %25, i32* %26, align 8, !tbaa !77
  br label %30

30:                                               ; preds = %29, %5
  %31 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %20)
  %32 = icmp eq i32 %31, 0
  br i1 %32, label %33, label %397

33:                                               ; preds = %30
  %34 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nonnull @gamestate, %struct.state_t* nonnull %0)
  %35 = icmp eq i32 %34, 0
  br i1 %35, label %36, label %40

36:                                               ; preds = %33
  %37 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15, !intel-tbaa !43
  %38 = load i32, i32* %37, align 4, !tbaa !43
  %39 = icmp sgt i32 %38, 99
  br i1 %39, label %40, label %48

40:                                               ; preds = %36, %33
  %41 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4, !tbaa !44
  %42 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !21
  %43 = load i32, i32* %42, align 4, !tbaa !21
  %44 = icmp eq i32 %41, %43
  %45 = load i32, i32* @contempt, align 4, !tbaa !6
  %46 = sub nsw i32 0, %45
  %47 = select i1 %44, i32 %45, i32 %46
  br label %397

48:                                               ; preds = %36
  %49 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nonnull %0, i32* nonnull %8, i32 %1, i32 %2, i32* nonnull %7, i32* nonnull %9, i32* nonnull %9, i32* nonnull %9, i32* nonnull %9, i32 0)
  switch i32 %49, label %59 [
    i32 3, label %50
    i32 1, label %52
    i32 2, label %55
    i32 4, label %58
  ]

50:                                               ; preds = %48
  %51 = load i32, i32* %8, align 4, !tbaa !6
  br label %397

52:                                               ; preds = %48
  %53 = load i32, i32* %8, align 4, !tbaa !6
  %54 = icmp sgt i32 %53, %1
  br i1 %54, label %59, label %397

55:                                               ; preds = %48
  %56 = load i32, i32* %8, align 4, !tbaa !6
  %57 = icmp slt i32 %56, %2
  br i1 %57, label %59, label %397

58:                                               ; preds = %48
  store i32 65535, i32* %7, align 4, !tbaa !6
  br label %59

59:                                               ; preds = %58, %55, %52, %48
  %60 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4, !tbaa !69
  %61 = shl nsw i32 %60, 1
  %62 = icmp slt i32 %61, %4
  br i1 %62, label %66, label %63

63:                                               ; preds = %59
  %64 = load i32, i32* %24, align 8, !tbaa !39
  %65 = icmp sgt i32 %64, 60
  br i1 %65, label %66, label %68

66:                                               ; preds = %63, %59
  %67 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %1, i32 %2, i32 0)
  br label %397

68:                                               ; preds = %63
  %69 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25, !intel-tbaa !48
  %70 = sext i32 %64 to i64
  %71 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %70, !intel-tbaa !49
  %72 = load i32, i32* %71, align 4, !tbaa !50
  %73 = call i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nonnull %0)
  %74 = add nsw i32 %73, 50
  %75 = icmp ne i32 %72, 0
  br i1 %75, label %154, label %76

76:                                               ; preds = %68
  %77 = icmp slt i32 %73, %2
  br i1 %77, label %80, label %78

78:                                               ; preds = %76
  %79 = load i32, i32* %7, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %73, i32 %1, i32 %2, i32 %79, i32 0, i32 0, i32 0, i32 0)
  br label %397

80:                                               ; preds = %76
  %81 = icmp sgt i32 %73, %1
  br i1 %81, label %147, label %82

82:                                               ; preds = %80
  %83 = add nsw i32 %73, 985
  %84 = icmp sgt i32 %83, %1
  br i1 %84, label %87, label %85

85:                                               ; preds = %82
  %86 = load i32, i32* %7, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %83, i32 %1, i32 %2, i32 %86, i32 0, i32 0, i32 0, i32 0)
  br label %397

87:                                               ; preds = %82
  %88 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8, !intel-tbaa !51
  %89 = getelementptr inbounds [13 x i32], [13 x i32]* %88, i64 0, i64 0
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !21
  %91 = load i32, i32* %90, align 4, !tbaa !21
  %92 = icmp eq i32 %91, 0
  br i1 %92, label %120, label %93

93:                                               ; preds = %87
  %94 = getelementptr inbounds i32, i32* %89, i64 10
  %95 = load i32, i32* %94, align 4, !tbaa !6
  %96 = icmp eq i32 %95, 0
  br i1 %96, label %97, label %147

97:                                               ; preds = %93
  %98 = getelementptr inbounds i32, i32* %89, i64 8
  %99 = load i32, i32* %98, align 4, !tbaa !6
  %100 = icmp eq i32 %99, 0
  br i1 %100, label %101, label %115

101:                                              ; preds = %97
  %102 = getelementptr inbounds i32, i32* %89, i64 12
  %103 = load i32, i32* %102, align 4, !tbaa !6
  %104 = icmp eq i32 %103, 0
  br i1 %104, label %105, label %112

105:                                              ; preds = %101
  %106 = getelementptr inbounds i32, i32* %89, i64 4
  %107 = load i32, i32* %106, align 4, !tbaa !6
  %108 = icmp eq i32 %107, 0
  br i1 %108, label %109, label %112

109:                                              ; preds = %105
  %110 = add nsw i32 %73, 135
  %111 = icmp sgt i32 %110, %1
  br i1 %111, label %147, label %397

112:                                              ; preds = %105, %101
  %113 = add nsw i32 %73, 380
  %114 = icmp sgt i32 %113, %1
  br i1 %114, label %147, label %397

115:                                              ; preds = %97
  %116 = add nsw i32 %73, 540
  %117 = icmp sgt i32 %116, %1
  br i1 %117, label %147, label %118

118:                                              ; preds = %115
  %119 = load i32, i32* %7, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %116, i32 %1, i32 %2, i32 %119, i32 0, i32 0, i32 0, i32 0)
  br label %397

120:                                              ; preds = %87
  %121 = getelementptr inbounds i32, i32* %89, i64 9
  %122 = load i32, i32* %121, align 4, !tbaa !6
  %123 = icmp eq i32 %122, 0
  br i1 %123, label %124, label %147

124:                                              ; preds = %120
  %125 = getelementptr inbounds i32, i32* %89, i64 7
  %126 = load i32, i32* %125, align 4, !tbaa !6
  %127 = icmp eq i32 %126, 0
  br i1 %127, label %128, label %142

128:                                              ; preds = %124
  %129 = getelementptr inbounds i32, i32* %89, i64 11
  %130 = load i32, i32* %129, align 4, !tbaa !6
  %131 = icmp eq i32 %130, 0
  br i1 %131, label %132, label %139

132:                                              ; preds = %128
  %133 = getelementptr inbounds i32, i32* %89, i64 3
  %134 = load i32, i32* %133, align 4, !tbaa !6
  %135 = icmp eq i32 %134, 0
  br i1 %135, label %136, label %139

136:                                              ; preds = %132
  %137 = add nsw i32 %73, 135
  %138 = icmp sgt i32 %137, %1
  br i1 %138, label %147, label %397

139:                                              ; preds = %132, %128
  %140 = add nsw i32 %73, 380
  %141 = icmp sgt i32 %140, %1
  br i1 %141, label %147, label %397

142:                                              ; preds = %124
  %143 = add nsw i32 %73, 540
  %144 = icmp sgt i32 %143, %1
  br i1 %144, label %147, label %145

145:                                              ; preds = %142
  %146 = load i32, i32* %7, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %143, i32 %1, i32 %2, i32 %146, i32 0, i32 0, i32 0, i32 0)
  br label %397

147:                                              ; preds = %142, %139, %136, %120, %115, %112, %109, %93, %80
  %148 = phi i32 [ %73, %80 ], [ %1, %115 ], [ %1, %112 ], [ %1, %109 ], [ %1, %93 ], [ %1, %142 ], [ %1, %139 ], [ %1, %136 ], [ %1, %120 ]
  %149 = sub nsw i32 %148, %74
  %150 = load i32, i32* %7, align 4, !tbaa !6
  %151 = icmp sgt i32 %3, -6
  %152 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 0
  %153 = call i32 @_Z12gen_capturesP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %152)
  br label %158

154:                                              ; preds = %68
  %155 = load i32, i32* %7, align 4, !tbaa !6
  %156 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 0
  %157 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %156, i32 %72)
  br label %158

158:                                              ; preds = %154, %147
  %159 = phi i32* [ %152, %147 ], [ %156, %154 ]
  %160 = phi i32 [ %150, %147 ], [ %155, %154 ]
  %161 = phi i32 [ %149, %147 ], [ 0, %154 ]
  %162 = phi i32 [ %148, %147 ], [ %1, %154 ]
  %163 = phi i1 [ %151, %147 ], [ false, %154 ]
  %164 = phi i32 [ %153, %147 ], [ %157, %154 ]
  %165 = getelementptr inbounds [240 x i32], [240 x i32]* %11, i64 0, i64 0
  %166 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %167 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 0
  %168 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %169 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %170 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %171 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %172 = sub nsw i32 0, %2
  %173 = add nsw i32 %4, 1
  %174 = icmp sgt i32 %3, -1
  br label %175

175:                                              ; preds = %382, %158
  %176 = phi i32 [ %160, %158 ], [ %376, %382 ]
  %177 = phi i32 [ 1, %158 ], [ %377, %382 ]
  %178 = phi i32 [ -32000, %158 ], [ %378, %382 ]
  %179 = phi i32 [ 1, %158 ], [ %383, %382 ]
  %180 = phi i32 [ %164, %158 ], [ %190, %382 ]
  %181 = phi i32 [ %162, %158 ], [ %379, %382 ]
  %182 = icmp eq i32 %179, 2
  br i1 %182, label %183, label %185

183:                                              ; preds = %175
  %184 = call i32 @_Z15gen_good_checksP7state_tPi(%struct.state_t* %0, i32* nonnull %159)
  br label %189

185:                                              ; preds = %175
  %186 = icmp eq i32 %179, 3
  br i1 %186, label %187, label %189

187:                                              ; preds = %185
  %188 = call i32 @_Z9gen_quietP7state_tPi(%struct.state_t* %0, i32* nonnull %159)
  br label %189

189:                                              ; preds = %187, %185, %183
  %190 = phi i32 [ %184, %183 ], [ %188, %187 ], [ %180, %185 ]
  %191 = load i32, i32* %7, align 4, !tbaa !6
  call fastcc void @_ZL16fast_order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %159, i32* nonnull %165, i32 %190, i32 %191)
  store i32 -1, i32* %6, align 4, !tbaa !6
  %192 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %165, i32* nonnull %159, i32 %190)
  %193 = icmp eq i32 %192, 0
  br i1 %193, label %375, label %194

194:                                              ; preds = %189
  %195 = icmp eq i32 %179, 1
  %196 = icmp eq i32 %179, 3
  %197 = or i32 %179, 1
  %198 = icmp eq i32 %197, 3
  br label %199

199:                                              ; preds = %370, %194
  %200 = phi i32 [ %181, %194 ], [ %372, %370 ]
  %201 = phi i32 [ %178, %194 ], [ %356, %370 ]
  %202 = phi i32 [ %177, %194 ], [ %354, %370 ]
  %203 = phi i32 [ %176, %194 ], [ %371, %370 ]
  br i1 %75, label %204, label %206

204:                                              ; preds = %199
  %205 = load i32, i32* %6, align 4, !tbaa !6
  br label %313

206:                                              ; preds = %241, %199
  br i1 %195, label %207, label %224

207:                                              ; preds = %206
  %208 = load i32, i32* %6, align 4, !tbaa !6
  %209 = sext i32 %208 to i64
  %210 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %209, !intel-tbaa !61
  %211 = load i32, i32* %210, align 4, !tbaa !61
  %212 = lshr i32 %211, 19
  %213 = and i32 %212, 15
  %214 = zext i32 %213 to i64
  %215 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %214, !intel-tbaa !64
  %216 = load i32, i32* %215, align 4, !tbaa !64
  %217 = icmp slt i32 %216, 0
  %218 = sub nsw i32 0, %216
  %219 = select i1 %217, i32 %218, i32 %216
  %220 = icmp sle i32 %219, %161
  %221 = and i32 %211, 61440
  %222 = icmp eq i32 %221, 0
  %223 = and i1 %222, %220
  br i1 %223, label %375, label %275

224:                                              ; preds = %206
  br i1 %198, label %225, label %244

225:                                              ; preds = %224
  %226 = load i32, i32* %6, align 4, !tbaa !6
  %227 = sext i32 %226 to i64
  %228 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %227, !intel-tbaa !61
  %229 = load i32, i32* %228, align 4, !tbaa !61
  %230 = lshr i32 %229, 19
  %231 = and i32 %230, 15
  %232 = icmp eq i32 %231, 13
  br i1 %232, label %244, label %233

233:                                              ; preds = %225
  %234 = zext i32 %231 to i64
  %235 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %234, !intel-tbaa !64
  %236 = load i32, i32* %235, align 4, !tbaa !64
  %237 = icmp slt i32 %236, 0
  %238 = sub nsw i32 0, %236
  %239 = select i1 %237, i32 %238, i32 %236
  %240 = icmp sgt i32 %239, %161
  br i1 %240, label %241, label %244

241:                                              ; preds = %300, %246, %233
  %242 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %165, i32* nonnull %159, i32 %190)
  %243 = icmp eq i32 %242, 0
  br i1 %243, label %375, label %206

244:                                              ; preds = %233, %225, %224
  %245 = phi i1 [ true, %225 ], [ false, %224 ], [ true, %233 ]
  br i1 %196, label %246, label %272

246:                                              ; preds = %244
  %247 = load i32, i32* %6, align 4, !tbaa !6
  %248 = sext i32 %247 to i64
  %249 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %248, !intel-tbaa !61
  %250 = load i32, i32* %249, align 4, !tbaa !61
  %251 = lshr i32 %250, 6
  %252 = and i32 %251, 63
  %253 = zext i32 %252 to i64
  %254 = getelementptr inbounds [64 x i32], [64 x i32]* %166, i64 0, i64 %253, !intel-tbaa !49
  %255 = load i32, i32* %254, align 4, !tbaa !63
  %256 = add nsw i32 %255, -1
  %257 = and i32 %250, 63
  %258 = load i32, i32* %167, align 8, !tbaa !70
  %259 = sext i32 %258 to i64
  %260 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %259, !intel-tbaa !71
  %261 = sext i32 %256 to i64
  %262 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %260, i64 0, i64 %261, !intel-tbaa !74
  %263 = zext i32 %257 to i64
  %264 = getelementptr inbounds [64 x i32], [64 x i32]* %262, i64 0, i64 %263, !intel-tbaa !49
  %265 = load i32, i32* %264, align 4, !tbaa !75
  %266 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %259, !intel-tbaa !71
  %267 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %266, i64 0, i64 %261, !intel-tbaa !74
  %268 = getelementptr inbounds [64 x i32], [64 x i32]* %267, i64 0, i64 %263, !intel-tbaa !49
  %269 = load i32, i32* %268, align 4, !tbaa !75
  %270 = sub nsw i32 %269, %265
  %271 = icmp slt i32 %265, %270
  br i1 %271, label %241, label %272

272:                                              ; preds = %246, %244
  %273 = load i32, i32* %6, align 4, !tbaa !6
  %274 = sext i32 %273 to i64
  br i1 %245, label %300, label %275

275:                                              ; preds = %272, %207
  %276 = phi i64 [ %209, %207 ], [ %274, %272 ]
  %277 = phi i32 [ %208, %207 ], [ %273, %272 ]
  %278 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %276, !intel-tbaa !61
  %279 = load i32, i32* %278, align 4, !tbaa !61
  %280 = lshr i32 %279, 19
  %281 = and i32 %280, 15
  %282 = zext i32 %281 to i64
  %283 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %282, !intel-tbaa !64
  %284 = load i32, i32* %283, align 4, !tbaa !64
  %285 = icmp slt i32 %284, 0
  %286 = sub nsw i32 0, %284
  %287 = select i1 %285, i32 %286, i32 %284
  %288 = lshr i32 %279, 6
  %289 = and i32 %288, 63
  %290 = zext i32 %289 to i64
  %291 = getelementptr inbounds [64 x i32], [64 x i32]* %166, i64 0, i64 %290, !intel-tbaa !49
  %292 = load i32, i32* %291, align 4, !tbaa !63
  %293 = sext i32 %292 to i64
  %294 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %293, !intel-tbaa !64
  %295 = load i32, i32* %294, align 4, !tbaa !64
  %296 = icmp slt i32 %295, 0
  %297 = sub nsw i32 0, %295
  %298 = select i1 %296, i32 %297, i32 %295
  %299 = icmp slt i32 %287, %298
  br i1 %299, label %300, label %313

300:                                              ; preds = %275, %272
  %301 = phi i64 [ %276, %275 ], [ %274, %272 ]
  %302 = phi i32 [ %277, %275 ], [ %273, %272 ]
  %303 = load i32, i32* %168, align 4, !tbaa !21
  %304 = icmp eq i32 %303, 0
  %305 = zext i1 %304 to i32
  %306 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %301, !intel-tbaa !61
  %307 = load i32, i32* %306, align 4, !tbaa !61
  %308 = lshr i32 %307, 6
  %309 = and i32 %308, 63
  %310 = and i32 %307, 63
  %311 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* %0, i32 %305, i32 %309, i32 %310, i32 0)
  %312 = icmp slt i32 %311, -50
  br i1 %312, label %241, label %313

313:                                              ; preds = %300, %275, %204
  %314 = phi i32 [ %205, %204 ], [ %302, %300 ], [ %277, %275 ]
  %315 = sext i32 %314 to i64
  %316 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %315, !intel-tbaa !61
  %317 = load i32, i32* %316, align 4, !tbaa !61
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %317)
  %318 = load i32, i32* %316, align 4, !tbaa !61
  %319 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %318)
  %320 = icmp eq i32 %319, 0
  br i1 %320, label %353, label %321

321:                                              ; preds = %313
  %322 = load i64, i64* %169, align 8, !tbaa !22
  %323 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !66
  %324 = load i32, i32* %24, align 8, !tbaa !39
  %325 = add i32 %324, -1
  %326 = add i32 %325, %323
  %327 = sext i32 %326 to i64
  %328 = getelementptr inbounds [1000 x i64], [1000 x i64]* %170, i64 0, i64 %327, !intel-tbaa !67
  store i64 %322, i64* %328, align 8, !tbaa !68
  %329 = load i32, i32* %316, align 4, !tbaa !61
  %330 = sext i32 %325 to i64
  %331 = getelementptr inbounds [64 x i32], [64 x i32]* %171, i64 0, i64 %330, !intel-tbaa !49
  store i32 %329, i32* %331, align 4, !tbaa !58
  %332 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %333 = load i32, i32* %24, align 8, !tbaa !39
  %334 = sext i32 %333 to i64
  %335 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %334, !intel-tbaa !49
  store i32 %332, i32* %335, align 4, !tbaa !50
  %336 = sub nsw i32 0, %200
  %337 = sub i32 60, %200
  %338 = icmp ne i32 %332, 0
  %339 = zext i1 %338 to i32
  %340 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %172, i32 %337, i32 %339)
  br i1 %196, label %341, label %344

341:                                              ; preds = %321
  %342 = sub nsw i32 0, %340
  %343 = icmp slt i32 %200, %342
  br i1 %343, label %349, label %353

344:                                              ; preds = %321
  %345 = or i32 %332, %72
  %346 = icmp eq i32 %345, 0
  %347 = select i1 %346, i32 -8, i32 -1
  %348 = add nsw i32 %347, %3
  br label %349

349:                                              ; preds = %344, %341
  %350 = phi i32 [ %3, %341 ], [ %348, %344 ]
  %351 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %172, i32 %336, i32 %350, i32 %173)
  %352 = sub nsw i32 0, %351
  br label %353

353:                                              ; preds = %349, %341, %313
  %354 = phi i32 [ %202, %313 ], [ 0, %349 ], [ 0, %341 ]
  %355 = phi i32 [ 0, %313 ], [ 1, %349 ], [ 1, %341 ]
  %356 = phi i32 [ %201, %313 ], [ %352, %349 ], [ %201, %341 ]
  %357 = load i32, i32* %316, align 4, !tbaa !61
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %357)
  %358 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %359 = icmp eq i32 %358, 0
  br i1 %359, label %360, label %397

360:                                              ; preds = %353
  %361 = icmp sgt i32 %356, %200
  %362 = icmp ne i32 %355, 0
  %363 = and i1 %362, %361
  br i1 %363, label %364, label %370

364:                                              ; preds = %360
  %365 = load i32, i32* %316, align 4, !tbaa !61
  %366 = call zeroext i16 @_Z12compact_movei(i32 %365)
  %367 = zext i16 %366 to i32
  %368 = icmp slt i32 %356, %2
  br i1 %368, label %370, label %369

369:                                              ; preds = %364
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %356, i32 %1, i32 %2, i32 %367, i32 0, i32 0, i32 0, i32 0)
  br label %397

370:                                              ; preds = %364, %360
  %371 = phi i32 [ %203, %360 ], [ %367, %364 ]
  %372 = phi i32 [ %200, %360 ], [ %356, %364 ]
  %373 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %165, i32* nonnull %159, i32 %190)
  %374 = icmp eq i32 %373, 0
  br i1 %374, label %375, label %199

375:                                              ; preds = %370, %241, %207, %189
  %376 = phi i32 [ %176, %189 ], [ %203, %207 ], [ %203, %241 ], [ %371, %370 ]
  %377 = phi i32 [ %177, %189 ], [ %202, %207 ], [ %202, %241 ], [ %354, %370 ]
  %378 = phi i32 [ %178, %189 ], [ %201, %207 ], [ %201, %241 ], [ %356, %370 ]
  %379 = phi i32 [ %181, %189 ], [ %200, %207 ], [ %200, %241 ], [ %372, %370 ]
  %380 = icmp eq i32 %179, 1
  %381 = and i1 %163, %380
  br i1 %381, label %382, label %384

382:                                              ; preds = %384, %375
  %383 = phi i32 [ 2, %375 ], [ 3, %384 ]
  br label %175

384:                                              ; preds = %375
  %385 = and i1 %163, %182
  %386 = and i1 %174, %385
  %387 = icmp sgt i32 %74, %379
  %388 = and i1 %386, %387
  br i1 %388, label %382, label %389

389:                                              ; preds = %384
  %390 = icmp ne i32 %377, 0
  %391 = and i1 %75, %390
  br i1 %391, label %392, label %395

392:                                              ; preds = %389
  %393 = load i32, i32* %24, align 8, !tbaa !39
  %394 = add nsw i32 %393, -32000
  br label %395

395:                                              ; preds = %392, %389
  %396 = phi i32 [ %394, %392 ], [ %379, %389 ]
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %396, i32 %1, i32 %2, i32 %376, i32 0, i32 0, i32 0, i32 0)
  br label %397

397:                                              ; preds = %395, %369, %353, %145, %139, %136, %118, %112, %109, %85, %78, %66, %55, %52, %50, %40, %30
  %398 = phi i32 [ %47, %40 ], [ %67, %66 ], [ %356, %369 ], [ %396, %395 ], [ %73, %78 ], [ %83, %85 ], [ %51, %50 ], [ 0, %30 ], [ %53, %52 ], [ %56, %55 ], [ %140, %139 ], [ %137, %136 ], [ %113, %112 ], [ %110, %109 ], [ %143, %145 ], [ %116, %118 ], [ 0, %353 ]
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %17) #7
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %16) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %15) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %14) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %13) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %12) #7
  ret i32 %398
}

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #5 = { argmemonly nounwind willreturn }
attributes #6 = { nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #7 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"Virtual Function Elim", i32 0}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
!10 = !{!11, !7, i64 4356}
!11 = !{!"struct@_ZTS7state_t", !7, i64 0, !12, i64 4, !13, i64 264, !13, i64 272, !13, i64 280, !14, i64 288, !7, i64 392, !7, i64 396, !15, i64 400, !7, i64 452, !7, i64 456, !7, i64 460, !7, i64 464, !7, i64 468, !7, i64 472, !7, i64 476, !13, i64 480, !13, i64 488, !16, i64 496, !12, i64 2544, !12, i64 2800, !18, i64 3056, !13, i64 4080, !13, i64 4088, !7, i64 4096, !12, i64 4100, !7, i64 4356, !7, i64 4360, !7, i64 4364, !7, i64 4368, !7, i64 4372, !7, i64 4376, !7, i64 4380, !7, i64 4384, !7, i64 4388, !7, i64 4392, !20, i64 4400}
!12 = !{!"array@_ZTSA64_i", !7, i64 0}
!13 = !{!"long long", !8, i64 0}
!14 = !{!"array@_ZTSA13_y", !13, i64 0}
!15 = !{!"array@_ZTSA13_i", !7, i64 0}
!16 = !{!"array@_ZTSA64_6move_x", !17, i64 0}
!17 = !{!"struct@_ZTS6move_x", !7, i64 0, !7, i64 4, !7, i64 8, !7, i64 12, !13, i64 16, !13, i64 24}
!18 = !{!"array@_ZTSA64_N7state_tUt_E", !19, i64 0}
!19 = !{!"struct@_ZTSN7state_tUt_E", !7, i64 0, !7, i64 4, !7, i64 8, !7, i64 12}
!20 = !{!"array@_ZTSA1000_y", !13, i64 0}
!21 = !{!11, !7, i64 460}
!22 = !{!11, !13, i64 480}
!23 = !{!24, !24, i64 0}
!24 = !{!"pointer@_ZTSP9ttentry_t", !8, i64 0}
!25 = !{!26, !27, i64 0}
!26 = !{!"struct@_ZTS9ttentry_t", !27, i64 0}
!27 = !{!"array@_ZTSA4_10ttbucket_t", !28, i64 0}
!28 = !{!"struct@_ZTS10ttbucket_t", !7, i64 0, !29, i64 4, !29, i64 6, !8, i64 8, !8, i64 9, !8, i64 9, !8, i64 9, !8, i64 9, !8, i64 9}
!29 = !{!"short", !8, i64 0}
!30 = !{!27, !28, i64 0}
!31 = !{!28, !7, i64 0}
!32 = !{!26, !7, i64 0}
!33 = !{!11, !7, i64 4360}
!34 = !{!28, !8, i64 9}
!35 = !{!28, !8, i64 8}
!36 = !{!26, !8, i64 8}
!37 = !{!28, !29, i64 4}
!38 = !{!26, !29, i64 4}
!39 = !{!11, !7, i64 472}
!40 = !{!28, !29, i64 6}
!41 = !{!26, !29, i64 6}
!42 = !{!11, !13, i64 4080}
!43 = !{!11, !7, i64 476}
!44 = !{!45, !7, i64 12}
!45 = !{!"struct@_ZTS11gamestate_t", !7, i64 0, !7, i64 4, !7, i64 8, !7, i64 12, !7, i64 16, !7, i64 20, !7, i64 24, !7, i64 28, !7, i64 32, !7, i64 36, !7, i64 40, !7, i64 44, !7, i64 48, !7, i64 52, !7, i64 56, !7, i64 60, !46, i64 64, !47, i64 4064, !13, i64 36064, !7, i64 36072, !7, i64 36076, !7, i64 36080, !7, i64 36084, !7, i64 36088, !7, i64 36092, !7, i64 36096, !7, i64 36100}
!46 = !{!"array@_ZTSA1000_i", !7, i64 0}
!47 = !{!"array@_ZTSA1000_6move_x", !17, i64 0}
!48 = !{!11, !12, i64 4100}
!49 = !{!12, !7, i64 0}
!50 = !{!11, !7, i64 4100}
!51 = !{!11, !15, i64 400}
!52 = !{!15, !7, i64 0}
!53 = !{!11, !7, i64 400}
!54 = !{!45, !7, i64 4}
!55 = !{!45, !7, i64 36096}
!56 = !{!11, !7, i64 456}
!57 = !{!11, !12, i64 2544}
!58 = !{!11, !7, i64 2544}
!59 = !{!11, !12, i64 2800}
!60 = !{!11, !7, i64 2800}
!61 = !{!62, !7, i64 0}
!62 = !{!"array@_ZTSA240_i", !7, i64 0}
!63 = !{!11, !7, i64 4}
!64 = !{!65, !7, i64 0}
!65 = !{!"array@_ZTSA14_i", !7, i64 0}
!66 = !{!45, !7, i64 60}
!67 = !{!20, !13, i64 0}
!68 = !{!11, !13, i64 4400}
!69 = !{!45, !7, i64 20}
!70 = !{!11, !7, i64 0}
!71 = !{!72, !73, i64 0}
!72 = !{!"array@_ZTSA8_A12_A64_i", !73, i64 0}
!73 = !{!"array@_ZTSA12_A64_i", !12, i64 0}
!74 = !{!73, !12, i64 0}
!75 = !{!72, !7, i64 0}
!76 = !{!11, !13, i64 4088}
!77 = !{!11, !7, i64 4096}
