; Checks that IPO Prefetch pass can properly identify the prefetch opportunity, generate the prefetch
; function, and generate 2 calls to the prefetch function in 2 host functions: 1 call to prefetch inside
; each host function.
;
; [Note]
; The IPO prefetch pass is a module pass that specifically examines code patterns like those exhibited in
; cpu2017/531.deepsjeng (531), generate a prefetch function based on one of the existing functions in 531
; and insert multiple calls to this prefetch function at specific pattern-matched host locations.
;
; This LIT testcase (intel_ipo_prefetch5.ll) is similar to intel_ipo_prefetch4.ll lit case, except the
; LLVM IR instructions are partially re-generated. Recent LLVM community pulldowns introduced significant changes
; reflected on the LLVM IR level. This makes the previously captured LLVM IRs out of date. The introduction
; of this LIT testcase aims to do the needed refresh and brings LLVM IR patterns for IPO Prefetch up to date.
;
; The LIT testcase presented here is a vastly simplified version of 531.
; It takes the following 4 functions after all modules have been compiled individually and linked together:
; - _Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(): an existing function that the prefetch function is based upon;
; - _Z7qsearchP7state_tiiii(): host function1 where a call to prefetch function will be inserted;
; - _Z6searchP7state_tiiiii(): host function2 where a call to prefetch function will be inserted;
; - @main(): the main function, identifying the input LLVM IR as a complete module.

; Only the 4 functions listed above, plus any global data and metadata they use, will appear in this LIT test.
; All other relevant functions are reduced to declarations only. Their function bodies are purged, as is any global variable
; or metadata that is not used directly by any of the above 4 functions.
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
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %19) #10
  %20 = bitcast [240 x i32]* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %20) #10
  %21 = bitcast i32* %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %21) #10
  %22 = bitcast i32* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %22) #10
  %23 = bitcast i32* %11 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %23) #10
  %24 = bitcast i32* %12 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %24) #10
  %25 = bitcast i32* %13 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %25) #10
  %26 = bitcast i32* %14 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %26) #10
  %27 = bitcast i32* %15 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %27) #10
  %28 = bitcast [240 x i32]* %16 to i8*
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %28) #10
  %29 = icmp slt i32 %3, 1
  br i1 %29, label %34, label %30

30:                                               ; preds = %6
  %31 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !39
  %32 = load i32, i32* %31, align 8, !tbaa !39
  %33 = icmp sgt i32 %32, 59
  br i1 %33, label %34, label %36

34:                                               ; preds = %30, %6
  %35 = tail call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 0, i32 0)
  br label %1035

36:                                               ; preds = %30
  %37 = getelementptr %struct.state_t, %struct.state_t* %0, i64 0, i32 22, !intel-tbaa !42
  %38 = load i64, i64* %37, align 8, !tbaa !42
  %39 = add i64 %38, 1
  store i64 %39, i64* %37, align 8, !tbaa !42
  %40 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %39)
  %41 = icmp eq i32 %40, 0
  br i1 %41, label %42, label %1035

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
  br label %1035

57:                                               ; preds = %45
  %58 = load i32, i32* %31, align 8, !tbaa !39
  %59 = add nsw i32 %58, -32000
  %60 = icmp sgt i32 %59, %1
  br i1 %60, label %61, label %63

61:                                               ; preds = %57
  %62 = icmp slt i32 %59, %2
  br i1 %62, label %63, label %1035

63:                                               ; preds = %61, %57
  %64 = phi i32 [ %59, %61 ], [ %1, %57 ]
  %65 = sub i32 31999, %58
  %66 = icmp slt i32 %65, %2
  br i1 %66, label %67, label %69

67:                                               ; preds = %63
  %68 = icmp sgt i32 %65, %64
  br i1 %68, label %69, label %1035

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
  br label %1035

74:                                               ; preds = %69
  %75 = load i32, i32* %10, align 4, !tbaa !6
  %76 = icmp sgt i32 %75, %64
  br i1 %76, label %85, label %1035

77:                                               ; preds = %69
  %78 = load i32, i32* %10, align 4, !tbaa !6
  %79 = icmp slt i32 %78, %70
  br i1 %79, label %85, label %1035

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
  br i1 %97, label %98, label %114

98:                                               ; preds = %85
  %99 = icmp slt i32 %3, 5
  br i1 %99, label %100, label %108

100:                                              ; preds = %98
  %101 = add nsw i32 %92, -75
  %102 = icmp slt i32 %101, %70
  br i1 %102, label %104, label %103

103:                                              ; preds = %100
  br label %1035

104:                                              ; preds = %100
  %105 = icmp slt i32 %92, %70
  br i1 %105, label %106, label %114

106:                                              ; preds = %104
  %107 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %64, i32 %70, i32 0, i32 0)
  br label %1035

108:                                              ; preds = %98
  %109 = icmp slt i32 %3, 9
  br i1 %109, label %110, label %114

110:                                              ; preds = %108
  %111 = add nsw i32 %92, -125
  %112 = icmp slt i32 %111, %70
  br i1 %112, label %114, label %113

113:                                              ; preds = %110
  br label %1035

114:                                              ; preds = %110, %108, %104, %85
  %115 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8, !intel-tbaa !51
  %116 = getelementptr inbounds [13 x i32], [13 x i32]* %115, i64 0, i64 9, !intel-tbaa !52
  %117 = load i32, i32* %116, align 4, !tbaa !53
  %118 = getelementptr inbounds [13 x i32], [13 x i32]* %115, i64 0, i64 7, !intel-tbaa !52
  %119 = load i32, i32* %118, align 4, !tbaa !53
  %120 = add nsw i32 %119, %117
  %121 = getelementptr inbounds [13 x i32], [13 x i32]* %115, i64 0, i64 11, !intel-tbaa !52
  %122 = load i32, i32* %121, align 4, !tbaa !53
  %123 = add nsw i32 %120, %122
  %124 = getelementptr inbounds [13 x i32], [13 x i32]* %115, i64 0, i64 3, !intel-tbaa !52
  %125 = load i32, i32* %124, align 4, !tbaa !53
  %126 = add nsw i32 %123, %125
  %127 = getelementptr inbounds [13 x i32], [13 x i32]* %115, i64 0, i64 10, !intel-tbaa !52
  %128 = load i32, i32* %127, align 8, !tbaa !53
  %129 = getelementptr inbounds [13 x i32], [13 x i32]* %115, i64 0, i64 8, !intel-tbaa !52
  %130 = load i32, i32* %129, align 8, !tbaa !53
  %131 = add nsw i32 %130, %128
  %132 = getelementptr inbounds [13 x i32], [13 x i32]* %115, i64 0, i64 12, !intel-tbaa !52
  %133 = load i32, i32* %132, align 8, !tbaa !53
  %134 = add nsw i32 %131, %133
  %135 = getelementptr inbounds [13 x i32], [13 x i32]* %115, i64 0, i64 4, !intel-tbaa !52
  %136 = load i32, i32* %135, align 8, !tbaa !53
  %137 = add nsw i32 %134, %136
  store i32 0, i32* %11, align 4, !tbaa !6
  %138 = icmp eq i32 %4, 0
  br i1 %138, label %139, label %220

139:                                              ; preds = %114
  %140 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !21
  %141 = load i32, i32* %140, align 4, !tbaa !21
  %142 = icmp eq i32 %141, 0
  %143 = select i1 %142, i32 %137, i32 %126
  %144 = icmp eq i32 %143, 0
  %145 = or i1 %93, %144
  %146 = xor i1 %145, true
  %147 = load i32, i32* %12, align 4
  %148 = icmp ne i32 %147, 0
  %149 = and i1 %148, %146
  %150 = icmp sgt i32 %3, 4
  %151 = and i1 %150, %96
  %152 = and i1 %151, %149
  br i1 %152, label %153, label %220

153:                                              ; preds = %139
  %154 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !tbaa !54
  %155 = icmp eq i32 %154, 2
  br i1 %155, label %156, label %175

156:                                              ; preds = %153
  %157 = icmp slt i32 %3, 25
  %158 = add nsw i32 %70, -1
  br i1 %157, label %159, label %161

159:                                              ; preds = %156
  %160 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %158, i32 %70, i32 0, i32 0)
  br label %164

161:                                              ; preds = %156
  %162 = add nsw i32 %3, -24
  %163 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %158, i32 %70, i32 %162, i32 1, i32 %86)
  br label %164

164:                                              ; preds = %161, %159
  %165 = phi i32 [ %160, %159 ], [ %163, %161 ]
  %166 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %167 = icmp eq i32 %166, 0
  br i1 %167, label %168, label %1035

168:                                              ; preds = %164
  %169 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !tbaa !54
  %170 = icmp eq i32 %169, 2
  %171 = icmp slt i32 %165, %70
  %172 = and i1 %171, %170
  br i1 %172, label %233, label %173

173:                                              ; preds = %168
  %174 = load i32, i32* %140, align 4, !tbaa !21
  br label %175

175:                                              ; preds = %173, %153
  %176 = phi i32 [ %174, %173 ], [ %141, %153 ]
  %177 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 10, !intel-tbaa !56
  %178 = load i32, i32* %177, align 8, !tbaa !56
  store i32 0, i32* %177, align 8, !tbaa !56
  %179 = xor i32 %176, 1
  store i32 %179, i32* %140, align 4, !tbaa !21
  %180 = load i32, i32* %31, align 8, !tbaa !39
  %181 = add nsw i32 %180, 1
  store i32 %181, i32* %31, align 8, !tbaa !39
  %182 = load i32, i32* %46, align 4, !tbaa !43
  %183 = add nsw i32 %182, 1
  store i32 %183, i32* %46, align 4, !tbaa !43
  %184 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19, !intel-tbaa !57
  %185 = sext i32 %180 to i64
  %186 = getelementptr inbounds [64 x i32], [64 x i32]* %184, i64 0, i64 %185, !intel-tbaa !49
  store i32 0, i32* %186, align 4, !tbaa !58
  %187 = sext i32 %181 to i64
  %188 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %187, !intel-tbaa !49
  store i32 0, i32* %188, align 4, !tbaa !50
  %189 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 20, !intel-tbaa !59
  %190 = getelementptr inbounds [64 x i32], [64 x i32]* %189, i64 0, i64 %185, !intel-tbaa !49
  %191 = load i32, i32* %190, align 4, !tbaa !60
  %192 = getelementptr inbounds [64 x i32], [64 x i32]* %189, i64 0, i64 %187, !intel-tbaa !49
  store i32 %191, i32* %192, align 4, !tbaa !60
  %193 = icmp slt i32 %3, 17
  %194 = sub nsw i32 0, %70
  %195 = sub i32 1, %70
  br i1 %193, label %196, label %198

196:                                              ; preds = %175
  %197 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %194, i32 %195, i32 0, i32 0)
  br label %203

198:                                              ; preds = %175
  %199 = add nsw i32 %3, -16
  %200 = icmp eq i32 %86, 0
  %201 = zext i1 %200 to i32
  %202 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %194, i32 %195, i32 %199, i32 1, i32 %201)
  br label %203

203:                                              ; preds = %198, %196
  %204 = phi i32 [ %197, %196 ], [ %202, %198 ]
  %205 = sub nsw i32 0, %204
  %206 = load i32, i32* %46, align 4, !tbaa !43
  %207 = add nsw i32 %206, -1
  store i32 %207, i32* %46, align 4, !tbaa !43
  %208 = load i32, i32* %31, align 8, !tbaa !39
  %209 = add nsw i32 %208, -1
  store i32 %209, i32* %31, align 8, !tbaa !39
  %210 = load i32, i32* %140, align 4, !tbaa !21
  %211 = xor i32 %210, 1
  store i32 %211, i32* %140, align 4, !tbaa !21
  store i32 %178, i32* %177, align 8, !tbaa !56
  %212 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %213 = icmp eq i32 %212, 0
  br i1 %213, label %214, label %1035

214:                                              ; preds = %203
  %215 = icmp sgt i32 %70, %205
  br i1 %215, label %217, label %216

216:                                              ; preds = %214
  br label %1035

217:                                              ; preds = %214
  %218 = icmp sgt i32 %204, 31400
  br i1 %218, label %219, label %233

219:                                              ; preds = %217
  store i32 1, i32* %11, align 4, !tbaa !6
  br label %233

220:                                              ; preds = %139, %114
  %221 = icmp slt i32 %3, 13
  %222 = and i1 %221, %96
  %223 = add nsw i32 %70, -300
  %224 = icmp slt i32 %92, %223
  %225 = and i1 %222, %224
  br i1 %225, label %226, label %233

226:                                              ; preds = %220
  %227 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %64, i32 %70, i32 0, i32 0)
  %228 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %229 = icmp eq i32 %228, 0
  br i1 %229, label %230, label %1035

230:                                              ; preds = %226
  %231 = icmp sgt i32 %227, %64
  br i1 %231, label %233, label %232

232:                                              ; preds = %230
  br label %1035

233:                                              ; preds = %230, %220, %219, %217, %168
  %234 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 0
  br i1 %93, label %235, label %240

235:                                              ; preds = %233
  %236 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %234, i32 %91)
  %237 = icmp eq i32 %236, 0
  br i1 %237, label %257, label %238

238:                                              ; preds = %235
  store i32 0, i32* %9, align 4, !tbaa !6
  %239 = icmp sgt i32 %236, 0
  br i1 %239, label %242, label %257

240:                                              ; preds = %233
  %241 = call i32 @_Z3genP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %234)
  br label %257

242:                                              ; preds = %242, %238
  %243 = phi i32 [ %251, %242 ], [ 0, %238 ]
  %244 = load i32, i32* %9, align 4, !tbaa !6
  %245 = sext i32 %244 to i64
  %246 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %245, !intel-tbaa !61
  %247 = load i32, i32* %246, align 4, !tbaa !61
  %248 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %247)
  %249 = icmp ne i32 %248, 0
  %250 = zext i1 %249 to i32
  %251 = add nuw nsw i32 %243, %250
  %252 = load i32, i32* %9, align 4, !tbaa !6
  %253 = add nsw i32 %252, 1
  store i32 %253, i32* %9, align 4, !tbaa !6
  %254 = icmp slt i32 %253, %236
  %255 = icmp ult i32 %251, 2
  %256 = and i1 %254, %255
  br i1 %256, label %242, label %257

257:                                              ; preds = %242, %240, %238, %235
  %258 = phi i32 [ 0, %235 ], [ %241, %240 ], [ %236, %238 ], [ %236, %242 ]
  %259 = phi i32 [ 0, %235 ], [ %241, %240 ], [ 0, %238 ], [ %251, %242 ]
  %260 = getelementptr inbounds [240 x i32], [240 x i32]* %8, i64 0, i64 0
  %261 = load i32, i32* %13, align 4, !tbaa !6
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %234, i32* nonnull %260, i32 %258, i32 %261)
  %262 = icmp sgt i32 %3, 19
  br i1 %262, label %263, label %313

263:                                              ; preds = %257
  %264 = icmp ne i32 %70, %95
  %265 = load i32, i32* %13, align 4
  %266 = icmp eq i32 %265, 65535
  %267 = and i1 %264, %266
  br i1 %267, label %268, label %313

268:                                              ; preds = %263
  store i32 0, i32* %9, align 4, !tbaa !6
  %269 = icmp sgt i32 %258, 0
  br i1 %269, label %270, label %301

270:                                              ; preds = %268
  %271 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %272 = zext i32 %258 to i64
  br label %273

273:                                              ; preds = %295, %270
  %274 = phi i64 [ 0, %270 ], [ %297, %295 ]
  %275 = phi i32 [ 0, %270 ], [ %296, %295 ]
  %276 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %274, !intel-tbaa !61
  %277 = load i32, i32* %276, align 4, !tbaa !61
  %278 = lshr i32 %277, 19
  %279 = and i32 %278, 15
  %280 = icmp eq i32 %279, 13
  br i1 %280, label %295, label %281

281:                                              ; preds = %273
  %282 = lshr i32 %277, 6
  %283 = and i32 %282, 63
  %284 = zext i32 %283 to i64
  %285 = getelementptr inbounds [64 x i32], [64 x i32]* %271, i64 0, i64 %284, !intel-tbaa !49
  %286 = load i32, i32* %285, align 4, !tbaa !63
  %287 = sext i32 %286 to i64
  %288 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %287, !intel-tbaa !64
  %289 = load i32, i32* %288, align 4, !tbaa !64
  %290 = icmp slt i32 %289, 0
  %291 = sub nsw i32 0, %289
  %292 = select i1 %290, i32 %291, i32 %289
  %293 = icmp sgt i32 %279, %292
  %294 = select i1 %293, i32 1, i32 %275
  br label %295

295:                                              ; preds = %281, %273
  %296 = phi i32 [ %275, %273 ], [ %294, %281 ]
  %297 = add nuw nsw i64 %274, 1
  %298 = icmp eq i64 %297, %272
  br i1 %298, label %299, label %273

299:                                              ; preds = %295
  store i32 %258, i32* %9, align 4, !tbaa !6
  %300 = icmp eq i32 %296, 0
  br i1 %300, label %301, label %313

301:                                              ; preds = %299, %268
  %302 = bitcast i32* %17 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %302) #10
  %303 = bitcast i32* %18 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %303) #10
  %304 = ashr i32 %3, 1
  %305 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %304, i32 0, i32 %86)
  %306 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* %0, i32* nonnull %17, i32 0, i32 0, i32* nonnull %18, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32 0)
  %307 = icmp eq i32 %306, 4
  br i1 %307, label %310, label %308

308:                                              ; preds = %301
  %309 = load i32, i32* %18, align 4, !tbaa !6
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %234, i32* nonnull %260, i32 %258, i32 %309)
  br label %312

310:                                              ; preds = %301
  %311 = load i32, i32* %13, align 4, !tbaa !6
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %234, i32* nonnull %260, i32 %258, i32 %311)
  br label %312

312:                                              ; preds = %310, %308
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %303) #10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %302) #10
  br label %313

313:                                              ; preds = %312, %299, %263, %257
  %314 = load i32, i32* %11, align 4
  %315 = or i32 %314, %91
  %316 = icmp eq i32 %315, 0
  %317 = icmp sgt i32 %3, 15
  %318 = and i1 %317, %316
  %319 = icmp sgt i32 %259, 8
  %320 = and i1 %319, %318
  br i1 %320, label %321, label %466

321:                                              ; preds = %313
  %322 = load i32, i32* %31, align 8, !tbaa !39
  %323 = add nsw i32 %322, -1
  %324 = sext i32 %323 to i64
  %325 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %324, !intel-tbaa !49
  %326 = load i32, i32* %325, align 4, !tbaa !50
  %327 = icmp eq i32 %326, 0
  br i1 %327, label %328, label %466

328:                                              ; preds = %321
  %329 = icmp slt i32 %322, 3
  br i1 %329, label %344, label %330

330:                                              ; preds = %328
  %331 = add nsw i32 %322, -2
  %332 = sext i32 %331 to i64
  %333 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %332, !intel-tbaa !49
  %334 = load i32, i32* %333, align 4, !tbaa !50
  %335 = icmp eq i32 %334, 0
  br i1 %335, label %336, label %466

336:                                              ; preds = %330
  %337 = icmp slt i32 %322, 4
  br i1 %337, label %344, label %338

338:                                              ; preds = %336
  %339 = add nsw i32 %322, -3
  %340 = sext i32 %339 to i64
  %341 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %340, !intel-tbaa !49
  %342 = load i32, i32* %341, align 4, !tbaa !50
  %343 = icmp eq i32 %342, 0
  br i1 %343, label %344, label %466

344:                                              ; preds = %338, %336, %328
  store i32 -1, i32* %9, align 4, !tbaa !6
  %345 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  %346 = icmp eq i32 %345, 0
  br i1 %346, label %466, label %347

347:                                              ; preds = %344
  %348 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %349 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %350 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %351 = sub nsw i32 0, %70
  %352 = sub i32 50, %64
  %353 = icmp sgt i32 %3, 16
  %354 = icmp slt i32 %3, 17
  %355 = sub i32 1, %70
  %356 = add nsw i32 %3, -16
  %357 = icmp eq i32 %86, 0
  %358 = zext i1 %357 to i32
  br i1 %354, label %359, label %412

359:                                              ; preds = %406, %347
  %360 = phi i32 [ %407, %406 ], [ 0, %347 ]
  %361 = phi i32 [ %363, %406 ], [ 0, %347 ]
  %362 = phi i32 [ %396, %406 ], [ -32000, %347 ]
  %363 = add nuw nsw i32 %361, 1
  %364 = load i32, i32* %9, align 4, !tbaa !6
  %365 = sext i32 %364 to i64
  %366 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %365, !intel-tbaa !61
  %367 = load i32, i32* %366, align 4, !tbaa !61
  %368 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %367)
  %369 = icmp eq i32 %368, 0
  br i1 %369, label %394, label %370

370:                                              ; preds = %359
  %371 = load i64, i64* %348, align 8, !tbaa !22
  %372 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !66
  %373 = load i32, i32* %31, align 8, !tbaa !39
  %374 = add i32 %373, -1
  %375 = add i32 %374, %372
  %376 = sext i32 %375 to i64
  %377 = getelementptr inbounds [1000 x i64], [1000 x i64]* %349, i64 0, i64 %376, !intel-tbaa !67
  store i64 %371, i64* %377, align 8, !tbaa !68
  %378 = load i32, i32* %9, align 4, !tbaa !6
  %379 = sext i32 %378 to i64
  %380 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %379, !intel-tbaa !61
  %381 = load i32, i32* %380, align 4, !tbaa !61
  %382 = sext i32 %374 to i64
  %383 = getelementptr inbounds [64 x i32], [64 x i32]* %350, i64 0, i64 %382, !intel-tbaa !49
  store i32 %381, i32* %383, align 4, !tbaa !58
  %384 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %385 = load i32, i32* %31, align 8, !tbaa !39
  %386 = sext i32 %385 to i64
  %387 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %386, !intel-tbaa !49
  store i32 %384, i32* %387, align 4, !tbaa !50
  %388 = icmp ne i32 %384, 0
  %389 = or i1 %353, %388
  %390 = zext i1 %389 to i32
  %391 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %351, i32 %352, i32 %390)
  %392 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %351, i32 %355, i32 0, i32 0)
  %393 = sub nsw i32 0, %392
  br label %394

394:                                              ; preds = %370, %359
  %395 = phi i32 [ 1, %370 ], [ 0, %359 ]
  %396 = phi i32 [ %393, %370 ], [ %362, %359 ]
  %397 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %398 = icmp eq i32 %397, 0
  br i1 %398, label %399, label %466

399:                                              ; preds = %394
  %400 = icmp sge i32 %396, %70
  %401 = icmp ne i32 %395, 0
  %402 = and i1 %401, %400
  br i1 %402, label %403, label %406

403:                                              ; preds = %399
  %404 = add nsw i32 %360, 1
  %405 = icmp sgt i32 %360, 0
  br i1 %405, label %459, label %406

406:                                              ; preds = %403, %399
  %407 = phi i32 [ %404, %403 ], [ %360, %399 ]
  %408 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  %409 = icmp ne i32 %408, 0
  %410 = icmp ult i32 %361, 2
  %411 = and i1 %409, %410
  br i1 %411, label %359, label %466

412:                                              ; preds = %460, %347
  %413 = phi i32 [ %461, %460 ], [ 0, %347 ]
  %414 = phi i32 [ %416, %460 ], [ 0, %347 ]
  %415 = phi i32 [ %449, %460 ], [ -32000, %347 ]
  %416 = add nuw nsw i32 %414, 1
  %417 = load i32, i32* %9, align 4, !tbaa !6
  %418 = sext i32 %417 to i64
  %419 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %418, !intel-tbaa !61
  %420 = load i32, i32* %419, align 4, !tbaa !61
  %421 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %420)
  %422 = icmp eq i32 %421, 0
  br i1 %422, label %447, label %423

423:                                              ; preds = %412
  %424 = load i64, i64* %348, align 8, !tbaa !22
  %425 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !66
  %426 = load i32, i32* %31, align 8, !tbaa !39
  %427 = add i32 %426, -1
  %428 = add i32 %427, %425
  %429 = sext i32 %428 to i64
  %430 = getelementptr inbounds [1000 x i64], [1000 x i64]* %349, i64 0, i64 %429, !intel-tbaa !67
  store i64 %424, i64* %430, align 8, !tbaa !68
  %431 = load i32, i32* %9, align 4, !tbaa !6
  %432 = sext i32 %431 to i64
  %433 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %432, !intel-tbaa !61
  %434 = load i32, i32* %433, align 4, !tbaa !61
  %435 = sext i32 %427 to i64
  %436 = getelementptr inbounds [64 x i32], [64 x i32]* %350, i64 0, i64 %435, !intel-tbaa !49
  store i32 %434, i32* %436, align 4, !tbaa !58
  %437 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %438 = load i32, i32* %31, align 8, !tbaa !39
  %439 = sext i32 %438 to i64
  %440 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %439, !intel-tbaa !49
  store i32 %437, i32* %440, align 4, !tbaa !50
  %441 = icmp ne i32 %437, 0
  %442 = or i1 %353, %441
  %443 = zext i1 %442 to i32
  %444 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %351, i32 %352, i32 %443)
  %445 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %351, i32 %355, i32 %356, i32 0, i32 %358)
  %446 = sub nsw i32 0, %445
  br label %447

447:                                              ; preds = %423, %412
  %448 = phi i32 [ 1, %423 ], [ 0, %412 ]
  %449 = phi i32 [ %446, %423 ], [ %415, %412 ]
  %450 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %451 = icmp eq i32 %450, 0
  br i1 %451, label %452, label %466

452:                                              ; preds = %447
  %453 = icmp sge i32 %449, %70
  %454 = icmp ne i32 %448, 0
  %455 = and i1 %454, %453
  br i1 %455, label %456, label %460

456:                                              ; preds = %452
  %457 = add nsw i32 %413, 1
  %458 = icmp sgt i32 %413, 0
  br i1 %458, label %459, label %460

459:                                              ; preds = %456, %403
  br label %1035

460:                                              ; preds = %456, %452
  %461 = phi i32 [ %457, %456 ], [ %413, %452 ]
  %462 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  %463 = icmp ne i32 %462, 0
  %464 = icmp ult i32 %414, 2
  %465 = and i1 %463, %464
  br i1 %465, label %412, label %466

466:                                              ; preds = %460, %447, %406, %394, %344, %338, %330, %321, %313
  %467 = phi i32 [ -32000, %321 ], [ -32000, %338 ], [ -32000, %330 ], [ -32000, %313 ], [ -32000, %344 ], [ %396, %406 ], [ %396, %394 ], [ %449, %460 ], [ %449, %447 ]
  %468 = load i32, i32* %14, align 4, !tbaa !6
  %469 = load i32, i32* %15, align 4
  %470 = or i32 %469, %468
  %471 = load i32, i32* %11, align 4
  %472 = or i32 %470, %471
  %473 = icmp eq i32 %472, 0
  %474 = and i1 %262, %473
  %475 = icmp sgt i32 %259, 1
  %476 = and i1 %475, %474
  %477 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4
  %478 = icmp ne i32 %477, 2
  %479 = and i1 %478, %476
  br i1 %479, label %480, label %558

480:                                              ; preds = %466
  %481 = add nsw i32 %3, -24
  %482 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %481, i32 0, i32 %86)
  %483 = icmp sgt i32 %482, %64
  br i1 %483, label %484, label %558

484:                                              ; preds = %480
  store i32 -1, i32* %9, align 4, !tbaa !6
  %485 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  %486 = icmp ne i32 %485, 0
  %487 = load i32, i32* %14, align 4
  %488 = icmp slt i32 %487, 2
  %489 = and i1 %486, %488
  br i1 %489, label %490, label %558

490:                                              ; preds = %484
  %491 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %492 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %493 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %494 = add nsw i32 %3, -16
  %495 = sub nsw i32 0, %70
  %496 = sub i32 50, %64
  %497 = sub nsw i32 0, %64
  %498 = xor i32 %64, -1
  %499 = icmp eq i32 %86, 0
  %500 = zext i1 %499 to i32
  %501 = sub i32 49, %64
  %502 = add nsw i32 %64, -50
  br label %503

503:                                              ; preds = %547, %490
  %504 = phi i32 [ 0, %490 ], [ %550, %547 ]
  %505 = phi i32 [ %467, %490 ], [ %549, %547 ]
  %506 = phi i32 [ 1, %490 ], [ %548, %547 ]
  %507 = load i32, i32* %9, align 4, !tbaa !6
  %508 = sext i32 %507 to i64
  %509 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %508, !intel-tbaa !61
  %510 = load i32, i32* %509, align 4, !tbaa !61
  %511 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %510)
  %512 = icmp eq i32 %511, 0
  br i1 %512, label %547, label %513

513:                                              ; preds = %503
  %514 = load i64, i64* %491, align 8, !tbaa !22
  %515 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !66
  %516 = load i32, i32* %31, align 8, !tbaa !39
  %517 = add i32 %516, -1
  %518 = add i32 %517, %515
  %519 = sext i32 %518 to i64
  %520 = getelementptr inbounds [1000 x i64], [1000 x i64]* %492, i64 0, i64 %519, !intel-tbaa !67
  store i64 %514, i64* %520, align 8, !tbaa !68
  %521 = load i32, i32* %9, align 4, !tbaa !6
  %522 = sext i32 %521 to i64
  %523 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %522, !intel-tbaa !61
  %524 = load i32, i32* %523, align 4, !tbaa !61
  %525 = sext i32 %517 to i64
  %526 = getelementptr inbounds [64 x i32], [64 x i32]* %493, i64 0, i64 %525, !intel-tbaa !49
  store i32 %524, i32* %526, align 4, !tbaa !58
  %527 = add nsw i32 %504, 1
  %528 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %529 = load i32, i32* %31, align 8, !tbaa !39
  %530 = sext i32 %529 to i64
  %531 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %530, !intel-tbaa !49
  store i32 %528, i32* %531, align 4, !tbaa !50
  %532 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %495, i32 %496, i32 1)
  %533 = icmp eq i32 %506, 0
  br i1 %533, label %541, label %534

534:                                              ; preds = %513
  %535 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %498, i32 %497, i32 %494, i32 0, i32 %500)
  %536 = sub nsw i32 0, %535
  %537 = icmp slt i32 %64, %536
  br i1 %537, label %538, label %539

538:                                              ; preds = %534
  store i32 1, i32* %14, align 4, !tbaa !6
  br label %547

539:                                              ; preds = %534
  store i32 0, i32* %14, align 4, !tbaa !6
  %540 = add nsw i32 %504, 11
  br label %547

541:                                              ; preds = %513
  %542 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %501, i32 %496, i32 %494, i32 0, i32 0)
  %543 = sub nsw i32 0, %542
  %544 = icmp slt i32 %502, %543
  br i1 %544, label %545, label %547

545:                                              ; preds = %541
  store i32 0, i32* %14, align 4, !tbaa !6
  %546 = add nsw i32 %504, 11
  br label %547

547:                                              ; preds = %545, %541, %539, %538, %503
  %548 = phi i32 [ %506, %503 ], [ 0, %541 ], [ 0, %545 ], [ 0, %538 ], [ 0, %539 ]
  %549 = phi i32 [ %505, %503 ], [ %543, %541 ], [ %543, %545 ], [ %536, %538 ], [ %536, %539 ]
  %550 = phi i32 [ %504, %503 ], [ %527, %541 ], [ %546, %545 ], [ %527, %538 ], [ %540, %539 ]
  %551 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  %552 = icmp ne i32 %551, 0
  %553 = load i32, i32* %14, align 4
  %554 = icmp slt i32 %553, 2
  %555 = and i1 %552, %554
  %556 = icmp slt i32 %550, 3
  %557 = and i1 %556, %555
  br i1 %557, label %503, label %558

558:                                              ; preds = %547, %484, %480, %466
  %559 = phi i32 [ %467, %466 ], [ %467, %480 ], [ %467, %484 ], [ %549, %547 ]
  br i1 %96, label %565, label %560

560:                                              ; preds = %558
  %561 = load i32, i32* %31, align 8, !tbaa !39
  %562 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4, !tbaa !69
  %563 = shl nsw i32 %562, 1
  %564 = icmp sle i32 %561, %563
  br label %565

565:                                              ; preds = %560, %558
  %566 = phi i1 [ false, %558 ], [ %564, %560 ]
  store i32 -1, i32* %9, align 4, !tbaa !6
  %567 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  %568 = icmp eq i32 %567, 0
  br i1 %568, label %1012, label %569

569:                                              ; preds = %565
  %570 = icmp eq i32 %259, 1
  %571 = and i1 %93, %570
  %572 = select i1 %571, i32 4, i32 0
  %573 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %574 = or i32 %572, 2
  %575 = select i1 %566, i32 %574, i32 %572
  %576 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %577 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %578 = select i1 %566, i32 3, i32 1
  %579 = add nsw i32 %137, %126
  %580 = icmp eq i32 %579, 1
  %581 = sdiv i32 %3, 4
  %582 = add nsw i32 %581, 1
  %583 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 0
  %584 = icmp sgt i32 %3, 24
  %585 = icmp slt i32 %3, 9
  %586 = icmp slt i32 %3, 13
  %587 = add nsw i32 %92, 100
  %588 = add nsw i32 %92, 300
  %589 = add nsw i32 %92, 75
  %590 = add nsw i32 %92, 200
  %591 = select i1 %566, i32 4, i32 2
  %592 = sub nsw i32 0, %70
  %593 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %594 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %595 = add i32 %3, -4
  %596 = icmp eq i32 %86, 0
  %597 = zext i1 %596 to i32
  br label %598

598:                                              ; preds = %1003, %569
  %599 = phi i32 [ %64, %569 ], [ %1010, %1003 ]
  %600 = phi i32 [ %559, %569 ], [ %1009, %1003 ]
  %601 = phi i32 [ 1, %569 ], [ %1008, %1003 ]
  %602 = phi i32 [ -32000, %569 ], [ %1007, %1003 ]
  %603 = phi i32 [ 1, %569 ], [ %1006, %1003 ]
  %604 = phi i32 [ 1, %569 ], [ %1005, %1003 ]
  %605 = icmp ne i32 %603, 0
  %606 = icmp sgt i32 %604, %582
  %607 = add nsw i32 %599, 1
  %608 = icmp eq i32 %70, %607
  br label %609

609:                                              ; preds = %738, %598
  %610 = phi i32 [ %601, %598 ], [ 0, %738 ]
  %611 = load i32, i32* %31, align 8, !tbaa !39
  %612 = icmp slt i32 %611, 60
  %613 = load i32, i32* %9, align 4, !tbaa !6
  %614 = sext i32 %613 to i64
  br i1 %612, label %615, label %695

615:                                              ; preds = %609
  %616 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %614, !intel-tbaa !61
  %617 = load i32, i32* %616, align 4, !tbaa !61
  %618 = lshr i32 %617, 6
  %619 = and i32 %618, 63
  %620 = zext i32 %619 to i64
  %621 = getelementptr inbounds [64 x i32], [64 x i32]* %573, i64 0, i64 %620, !intel-tbaa !49
  %622 = load i32, i32* %621, align 4, !tbaa !63
  %623 = add nsw i32 %622, 1
  %624 = and i32 %623, -2
  %625 = icmp eq i32 %624, 2
  br i1 %625, label %626, label %633

626:                                              ; preds = %615
  %627 = lshr i32 %617, 3
  %628 = and i32 %627, 7
  switch i32 %628, label %629 [
    i32 1, label %632
    i32 6, label %632
  ]

629:                                              ; preds = %626
  %630 = and i32 %617, 61440
  %631 = icmp eq i32 %630, 0
  br i1 %631, label %633, label %632

632:                                              ; preds = %629, %626, %626
  br label %633

633:                                              ; preds = %632, %629, %615
  %634 = phi i32 [ %572, %629 ], [ %572, %615 ], [ %575, %632 ]
  %635 = lshr i32 %617, 19
  %636 = and i32 %635, 15
  %637 = icmp eq i32 %636, 13
  br i1 %637, label %668, label %638

638:                                              ; preds = %633
  %639 = add nsw i32 %611, -1
  %640 = sext i32 %639 to i64
  %641 = getelementptr inbounds [64 x i32], [64 x i32]* %576, i64 0, i64 %640, !intel-tbaa !49
  %642 = load i32, i32* %641, align 4, !tbaa !58
  %643 = lshr i32 %642, 19
  %644 = and i32 %643, 15
  %645 = icmp eq i32 %644, 13
  br i1 %645, label %668, label %646

646:                                              ; preds = %638
  %647 = zext i32 %636 to i64
  %648 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %647, !intel-tbaa !64
  %649 = load i32, i32* %648, align 4, !tbaa !64
  %650 = zext i32 %644 to i64
  %651 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %650, !intel-tbaa !64
  %652 = load i32, i32* %651, align 4, !tbaa !64
  %653 = icmp eq i32 %649, %652
  br i1 %653, label %654, label %668

654:                                              ; preds = %646
  %655 = and i32 %617, 63
  %656 = and i32 %642, 63
  %657 = icmp eq i32 %655, %656
  br i1 %657, label %658, label %668

658:                                              ; preds = %654
  %659 = load i32, i32* %577, align 4, !tbaa !21
  %660 = icmp eq i32 %659, 0
  %661 = zext i1 %660 to i32
  %662 = lshr i32 %617, 12
  %663 = and i32 %662, 15
  %664 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %661, i32 %619, i32 %655, i32 %663)
  %665 = icmp sgt i32 %664, 0
  %666 = select i1 %665, i32 %578, i32 0
  %667 = add nsw i32 %666, %634
  br label %668

668:                                              ; preds = %658, %654, %646, %638, %633
  %669 = phi i32 [ %634, %654 ], [ %634, %646 ], [ %634, %638 ], [ %634, %633 ], [ %667, %658 ]
  %670 = load i32, i32* %14, align 4, !tbaa !6
  %671 = icmp eq i32 %670, 1
  %672 = icmp ne i32 %669, 0
  %673 = and i1 %672, %671
  %674 = and i1 %605, %673
  br i1 %674, label %675, label %676

675:                                              ; preds = %668
  store i32 1, i32* %15, align 4, !tbaa !6
  br label %681

676:                                              ; preds = %668
  %677 = icmp eq i32 %669, 0
  %678 = and i1 %677, %671
  %679 = and i1 %605, %678
  br i1 %679, label %680, label %681

680:                                              ; preds = %676
  store i32 0, i32* %15, align 4, !tbaa !6
  br label %684

681:                                              ; preds = %676, %675
  %682 = icmp slt i32 %669, 4
  %683 = select i1 %682, i32 %669, i32 4
  br label %684

684:                                              ; preds = %681, %680
  %685 = phi i32 [ %683, %681 ], [ %578, %680 ]
  %686 = load i32, i32* %9, align 4, !tbaa !6
  %687 = sext i32 %686 to i64
  %688 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %687, !intel-tbaa !61
  %689 = load i32, i32* %688, align 4, !tbaa !61
  %690 = lshr i32 %689, 19
  %691 = and i32 %690, 15
  switch i32 %691, label %692 [
    i32 13, label %695
    i32 1, label %695
    i32 2, label %695
  ]

692:                                              ; preds = %684
  %693 = add nsw i32 %685, 4
  %694 = select i1 %580, i32 %693, i32 %685
  br label %695

695:                                              ; preds = %692, %684, %684, %684, %609
  %696 = phi i64 [ %687, %692 ], [ %687, %684 ], [ %687, %684 ], [ %687, %684 ], [ %614, %609 ]
  %697 = phi i32 [ %694, %692 ], [ %685, %684 ], [ %685, %684 ], [ %685, %684 ], [ 0, %609 ]
  %698 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %696, !intel-tbaa !61
  %699 = load i32, i32* %698, align 4, !tbaa !61
  %700 = and i32 %699, 7864320
  %701 = icmp eq i32 %700, 6815744
  %702 = xor i1 %701, true
  %703 = xor i1 %606, true
  %704 = or i1 %702, %703
  %705 = select i1 %702, i1 false, i1 true
  %706 = select i1 %702, i32 %610, i32 %601
  br i1 %704, label %744, label %707

707:                                              ; preds = %695
  %708 = lshr i32 %699, 6
  %709 = and i32 %708, 63
  %710 = zext i32 %709 to i64
  %711 = getelementptr inbounds [64 x i32], [64 x i32]* %573, i64 0, i64 %710, !intel-tbaa !49
  %712 = load i32, i32* %711, align 4, !tbaa !63
  %713 = add nsw i32 %712, -1
  %714 = and i32 %699, 63
  %715 = load i32, i32* %583, align 8, !tbaa !70
  %716 = sext i32 %715 to i64
  %717 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %716, !intel-tbaa !71
  %718 = sext i32 %713 to i64
  %719 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %717, i64 0, i64 %718, !intel-tbaa !74
  %720 = zext i32 %714 to i64
  %721 = getelementptr inbounds [64 x i32], [64 x i32]* %719, i64 0, i64 %720, !intel-tbaa !49
  %722 = load i32, i32* %721, align 4, !tbaa !75
  %723 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %716, !intel-tbaa !71
  %724 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %723, i64 0, i64 %718, !intel-tbaa !74
  %725 = getelementptr inbounds [64 x i32], [64 x i32]* %724, i64 0, i64 %720, !intel-tbaa !49
  %726 = load i32, i32* %725, align 4, !tbaa !75
  %727 = sub nsw i32 %726, %722
  %728 = mul nsw i32 %722, %582
  %729 = icmp sge i32 %728, %727
  %730 = or i1 %584, %729
  %731 = icmp ne i32 %697, 0
  %732 = or i1 %731, %730
  %733 = xor i1 %732, true
  %734 = and i1 %608, %733
  %735 = and i32 %699, 61440
  %736 = icmp eq i32 %735, 0
  %737 = and i1 %736, %734
  br i1 %737, label %738, label %744

738:                                              ; preds = %707
  %739 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  %740 = icmp eq i32 %739, 0
  br i1 %740, label %741, label %609

741:                                              ; preds = %738
  %742 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %743 = icmp eq i32 %742, 0
  br label %1026

744:                                              ; preds = %707, %695
  %745 = phi i1 [ true, %707 ], [ %705, %695 ]
  %746 = phi i32 [ %610, %707 ], [ %706, %695 ]
  br i1 %585, label %747, label %750

747:                                              ; preds = %744
  %748 = icmp slt i32 %589, %599
  %749 = icmp slt i32 %590, %599
  br label %755

750:                                              ; preds = %744
  %751 = icmp slt i32 %587, %599
  %752 = icmp slt i32 %588, %599
  %753 = and i1 %586, %751
  %754 = and i1 %586, %752
  br label %755

755:                                              ; preds = %750, %747
  %756 = phi i1 [ %748, %747 ], [ %753, %750 ]
  %757 = phi i1 [ %749, %747 ], [ %754, %750 ]
  br i1 %745, label %768, label %758

758:                                              ; preds = %755
  %759 = load i32, i32* %577, align 4, !tbaa !21
  %760 = icmp eq i32 %759, 0
  %761 = zext i1 %760 to i32
  %762 = lshr i32 %699, 6
  %763 = and i32 %762, 63
  %764 = and i32 %699, 63
  %765 = lshr i32 %699, 12
  %766 = and i32 %765, 15
  %767 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %761, i32 %763, i32 %764, i32 %766)
  br label %768

768:                                              ; preds = %758, %755
  %769 = phi i32 [ -1000000, %755 ], [ %767, %758 ]
  %770 = load i32, i32* %9, align 4, !tbaa !6
  %771 = sext i32 %770 to i64
  %772 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %771, !intel-tbaa !61
  %773 = load i32, i32* %772, align 4, !tbaa !61
  %774 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* nonnull %0, i32 %773)
  %775 = icmp eq i32 %774, 0
  br i1 %775, label %912, label %776

776:                                              ; preds = %768
  %777 = call i32 @_Z8in_checkP7state_t(%struct.state_t* nonnull %0)
  %778 = icmp ne i32 %777, 0
  %779 = select i1 %778, i32 %591, i32 0
  %780 = add nsw i32 %779, %697
  %781 = or i32 %777, %91
  %782 = icmp eq i32 %781, 0
  %783 = and i1 %608, %782
  br i1 %783, label %784, label %808

784:                                              ; preds = %776
  %785 = icmp slt i32 %769, 86
  %786 = and i1 %757, %785
  br i1 %786, label %787, label %796

787:                                              ; preds = %784
  %788 = load i32, i32* %9, align 4, !tbaa !6
  %789 = sext i32 %788 to i64
  %790 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %789, !intel-tbaa !61
  %791 = load i32, i32* %790, align 4, !tbaa !61
  %792 = and i32 %791, 61440
  %793 = icmp eq i32 %792, 0
  br i1 %793, label %794, label %796

794:                                              ; preds = %787
  %795 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  br label %1003

796:                                              ; preds = %787, %784
  %797 = icmp slt i32 %769, -50
  %798 = and i1 %756, %797
  br i1 %798, label %799, label %808

799:                                              ; preds = %796
  %800 = load i32, i32* %9, align 4, !tbaa !6
  %801 = sext i32 %800 to i64
  %802 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %801, !intel-tbaa !61
  %803 = load i32, i32* %802, align 4, !tbaa !61
  %804 = and i32 %803, 61440
  %805 = icmp eq i32 %804, 0
  br i1 %805, label %806, label %808

806:                                              ; preds = %799
  %807 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  br label %1003

808:                                              ; preds = %799, %796, %776
  %809 = add nsw i32 %780, %3
  %810 = add nsw i32 %809, -4
  %811 = sub nsw i32 0, %599
  %812 = sub i32 130, %599
  %813 = icmp sgt i32 %809, 4
  %814 = or i1 %778, %813
  %815 = zext i1 %814 to i32
  %816 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %592, i32 %812, i32 %815)
  %817 = load i32, i32* %31, align 8, !tbaa !39
  %818 = sext i32 %817 to i64
  %819 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %818, !intel-tbaa !49
  store i32 %777, i32* %819, align 4, !tbaa !50
  %820 = load i64, i64* %593, align 8, !tbaa !22
  %821 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !tbaa !66
  %822 = add i32 %817, -1
  %823 = add i32 %822, %821
  %824 = sext i32 %823 to i64
  %825 = getelementptr inbounds [1000 x i64], [1000 x i64]* %594, i64 0, i64 %824, !intel-tbaa !67
  store i64 %820, i64* %825, align 8, !tbaa !68
  %826 = icmp sgt i32 %822, 3
  %827 = add i1 false, false
  br i1 %827, label %828, label %862

828:                                              ; preds = %808
  %829 = or i32 %780, %777
  %830 = icmp eq i32 %829, 0
  %831 = and i1 %608, %830
  %832 = icmp slt i32 %769, -50
  %833 = and i1 %832, %831
  br i1 %833, label %834, label %862

834:                                              ; preds = %828
  %835 = and i1 %826, true
  %836 = zext i1 %835 to i64
  %837 = getelementptr inbounds [64 x i32], [64 x i32]* %573, i64 0, i64 %836, !intel-tbaa !49
  %838 = load i32, i32* %837, align 4, !tbaa !63
  %839 = add nsw i32 %838, -1
  %840 = load i32, i32* %583, align 8, !tbaa !70
  %841 = sext i32 %840 to i64
  %842 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %841, !intel-tbaa !71
  %843 = sext i32 %839 to i64
  %844 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %842, i64 0, i64 %843, !intel-tbaa !74
  %845 = getelementptr inbounds [64 x i32], [64 x i32]* %844, i64 0, i64 %836, !intel-tbaa !49
  %846 = load i32, i32* %845, align 4, !tbaa !75
  %847 = shl i32 %846, 7
  %848 = add i32 %847, 128
  %849 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %841, !intel-tbaa !71
  %850 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %849, i64 0, i64 %843, !intel-tbaa !74
  %851 = getelementptr inbounds [64 x i32], [64 x i32]* %850, i64 0, i64 %836, !intel-tbaa !49
  %852 = load i32, i32* %851, align 4, !tbaa !75
  %853 = add nsw i32 %852, 1
  %854 = sdiv i32 %848, %853
  %855 = icmp slt i32 %854, 80
  %856 = and i1 %826, false
  %857 = icmp eq i1 %856, false
  %858 = and i1 %857, %855
  br i1 %858, label %859, label %862

859:                                              ; preds = %834
  %860 = add nsw i32 %780, -4
  %861 = add i32 %595, %860
  br label %862

862:                                              ; preds = %859, %834, %828, %808
  %863 = phi i32 [ 4, %859 ], [ 0, %834 ], [ 0, %828 ], [ 0, %808 ]
  %864 = phi i32 [ %860, %859 ], [ %780, %834 ], [ %780, %828 ], [ %780, %808 ]
  %865 = phi i32 [ %861, %859 ], [ %810, %834 ], [ %810, %828 ], [ %810, %808 ]
  %866 = icmp eq i32 %603, 1
  %867 = icmp slt i32 %865, 1
  br i1 %866, label %868, label %875

868:                                              ; preds = %862
  br i1 %867, label %869, label %872

869:                                              ; preds = %868
  %870 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %592, i32 %811, i32 0, i32 0)
  %871 = sub nsw i32 0, %870
  br label %908

872:                                              ; preds = %868
  %873 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %592, i32 %811, i32 %865, i32 0, i32 %597)
  %874 = sub nsw i32 0, %873
  br label %908

875:                                              ; preds = %862
  %876 = xor i32 %599, -1
  br i1 %867, label %877, label %879

877:                                              ; preds = %875
  %878 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %876, i32 %811, i32 0, i32 0)
  br label %881

879:                                              ; preds = %875
  %880 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %876, i32 %811, i32 %865, i32 0, i32 1)
  br label %881

881:                                              ; preds = %879, %877
  %882 = phi i32 [ %878, %877 ], [ %880, %879 ]
  %883 = sub nsw i32 0, %882
  %884 = icmp slt i32 %602, %883
  %885 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %886 = icmp eq i32 %885, 0
  %887 = and i1 %884, %886
  %888 = icmp slt i32 %599, %883
  %889 = and i1 %888, %887
  br i1 %889, label %890, label %908

890:                                              ; preds = %881
  %891 = icmp eq i32 %863, 0
  br i1 %891, label %894, label %892

892:                                              ; preds = %890
  %893 = add nsw i32 %864, %863
  br label %896

894:                                              ; preds = %890
  %895 = icmp sgt i32 %70, %883
  br i1 %895, label %896, label %908

896:                                              ; preds = %894, %892
  %897 = phi i32 [ %864, %894 ], [ %893, %892 ]
  %898 = add nsw i32 %897, %3
  %899 = icmp slt i32 %898, 5
  br i1 %899, label %900, label %903

900:                                              ; preds = %896
  %901 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %592, i32 %811, i32 0, i32 0)
  %902 = sub nsw i32 0, %901
  br label %908

903:                                              ; preds = %896
  %904 = add nsw i32 %898, -4
  %905 = lshr exact i32 %863, 2
  %906 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %592, i32 %811, i32 %904, i32 0, i32 %905)
  %907 = sub nsw i32 0, %906
  br label %908

908:                                              ; preds = %903, %900, %894, %881, %872, %869
  %909 = phi i32 [ %871, %869 ], [ %874, %872 ], [ %883, %881 ], [ %902, %900 ], [ %907, %903 ], [ %883, %894 ]
  %910 = icmp sgt i32 %909, %602
  %911 = select i1 %910, i32 %909, i32 %602
  br label %912

912:                                              ; preds = %908, %768
  %913 = phi i32 [ %602, %768 ], [ %911, %908 ]
  %914 = phi i32 [ 0, %768 ], [ 1, %908 ]
  %915 = phi i32 [ %746, %768 ], [ 0, %908 ]
  %916 = phi i32 [ %600, %768 ], [ %909, %908 ]
  %917 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %918 = icmp eq i32 %917, 0
  br i1 %918, label %919, label %1035

919:                                              ; preds = %912
  %920 = icmp eq i32 %914, 0
  br i1 %920, label %998, label %921

921:                                              ; preds = %919
  %922 = icmp sgt i32 %916, %599
  br i1 %922, label %923, label %988

923:                                              ; preds = %921
  %924 = icmp slt i32 %916, %70
  %925 = load i32, i32* %9, align 4, !tbaa !6
  %926 = sext i32 %925 to i64
  %927 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %926
  %928 = load i32, i32* %927, align 4, !tbaa !61
  br i1 %924, label %985, label %929

929:                                              ; preds = %923
  call fastcc void @_ZL12history_goodP7state_tii(%struct.state_t* nonnull %0, i32 %928, i32 %3)
  %930 = icmp sgt i32 %604, 1
  br i1 %930, label %931, label %984

931:                                              ; preds = %929
  %932 = add nsw i32 %3, 3
  %933 = sdiv i32 %932, 4
  %934 = add nsw i32 %604, -1
  %935 = sext i32 %934 to i64
  br label %936

936:                                              ; preds = %981, %931
  %937 = phi i64 [ 0, %931 ], [ %982, %981 ]
  %938 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %937, !intel-tbaa !61
  %939 = load i32, i32* %938, align 4, !tbaa !61
  %940 = and i32 %939, 7925760
  %941 = icmp eq i32 %940, 6815744
  br i1 %941, label %942, label %981

942:                                              ; preds = %936
  %943 = lshr i32 %939, 6
  %944 = and i32 %943, 63
  %945 = zext i32 %944 to i64
  %946 = getelementptr inbounds [64 x i32], [64 x i32]* %573, i64 0, i64 %945, !intel-tbaa !49
  %947 = load i32, i32* %946, align 4, !tbaa !63
  %948 = add nsw i32 %947, -1
  %949 = and i32 %939, 63
  %950 = load i32, i32* %583, align 8, !tbaa !70
  %951 = sext i32 %950 to i64
  %952 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %951, !intel-tbaa !71
  %953 = sext i32 %948 to i64
  %954 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %952, i64 0, i64 %953, !intel-tbaa !74
  %955 = zext i32 %949 to i64
  %956 = getelementptr inbounds [64 x i32], [64 x i32]* %954, i64 0, i64 %955, !intel-tbaa !49
  %957 = load i32, i32* %956, align 4, !tbaa !75
  %958 = add nsw i32 %957, %933
  store i32 %958, i32* %956, align 4, !tbaa !75
  %959 = icmp sgt i32 %958, 16384
  br i1 %959, label %960, label %981

960:                                              ; preds = %942
  %961 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %951, !intel-tbaa !71
  br label %962

962:                                              ; preds = %978, %960
  %963 = phi i64 [ 0, %960 ], [ %979, %978 ]
  %964 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %961, i64 0, i64 %963, !intel-tbaa !74
  %965 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %952, i64 0, i64 %963, !intel-tbaa !74
  br label %966

966:                                              ; preds = %966, %962
  %967 = phi i64 [ 0, %962 ], [ %976, %966 ]
  %968 = getelementptr inbounds [64 x i32], [64 x i32]* %964, i64 0, i64 %967, !intel-tbaa !49
  %969 = load i32, i32* %968, align 4, !tbaa !75
  %970 = add nsw i32 %969, 1
  %971 = ashr i32 %970, 1
  store i32 %971, i32* %968, align 4, !tbaa !75
  %972 = getelementptr inbounds [64 x i32], [64 x i32]* %965, i64 0, i64 %967, !intel-tbaa !49
  %973 = load i32, i32* %972, align 4, !tbaa !75
  %974 = add nsw i32 %973, 1
  %975 = ashr i32 %974, 1
  store i32 %975, i32* %972, align 4, !tbaa !75
  %976 = add nuw nsw i64 %967, 1
  %977 = icmp eq i64 %976, 64
  br i1 %977, label %978, label %966

978:                                              ; preds = %966
  %979 = add nuw nsw i64 %963, 1
  %980 = icmp eq i64 %979, 12
  br i1 %980, label %981, label %962

981:                                              ; preds = %978, %942, %936
  %982 = add nuw nsw i64 %937, 1
  %983 = icmp eq i64 %982, %935
  br i1 %983, label %984, label %936

984:                                              ; preds = %981, %929
  br label %1035

985:                                              ; preds = %923
  %986 = call zeroext i16 @_Z12compact_movei(i32 %928)
  %987 = zext i16 %986 to i32
  store i32 %987, i32* %13, align 4, !tbaa !6
  br label %988

988:                                              ; preds = %985, %921
  %989 = phi i32 [ %916, %985 ], [ %599, %921 ]
  %990 = load i32, i32* %9, align 4, !tbaa !6
  %991 = sext i32 %990 to i64
  %992 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %991, !intel-tbaa !61
  %993 = load i32, i32* %992, align 4, !tbaa !61
  %994 = add nsw i32 %604, -1
  %995 = sext i32 %994 to i64
  %996 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %995, !intel-tbaa !61
  store i32 %993, i32* %996, align 4, !tbaa !61
  %997 = add nsw i32 %604, 1
  br label %998

998:                                              ; preds = %988, %919
  %999 = phi i32 [ %997, %988 ], [ %604, %919 ]
  %1000 = phi i32 [ 0, %988 ], [ %603, %919 ]
  %1001 = phi i32 [ %989, %988 ], [ %599, %919 ]
  %1002 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %260, i32* nonnull %234, i32 %258)
  br label %1003

1003:                                             ; preds = %998, %806, %794
  %1004 = phi i32 [ %1002, %998 ], [ %807, %806 ], [ %795, %794 ]
  %1005 = phi i32 [ %999, %998 ], [ %604, %806 ], [ %604, %794 ]
  %1006 = phi i32 [ %1000, %998 ], [ %603, %806 ], [ %603, %794 ]
  %1007 = phi i32 [ %913, %998 ], [ %599, %806 ], [ %599, %794 ]
  %1008 = phi i32 [ %915, %998 ], [ 0, %806 ], [ 0, %794 ]
  %1009 = phi i32 [ %916, %998 ], [ %600, %806 ], [ %600, %794 ]
  %1010 = phi i32 [ %1001, %998 ], [ %599, %806 ], [ %599, %794 ]
  %1011 = icmp eq i32 %1004, 0
  br i1 %1011, label %1012, label %598

1012:                                             ; preds = %1003, %565
  %1013 = phi i32 [ -32000, %565 ], [ %1007, %1003 ]
  %1014 = phi i32 [ 1, %565 ], [ %1008, %1003 ]
  %1015 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !tbaa !55
  %1016 = icmp eq i32 %1015, 0
  %1017 = icmp ne i32 %1014, 0
  %1018 = and i1 %1017, %1016
  br i1 %1018, label %1019, label %1026

1019:                                             ; preds = %1012
  %1020 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %1021 = icmp eq i32 %1020, 0
  br i1 %1021, label %1025, label %1022

1022:                                             ; preds = %1019
  %1023 = load i32, i32* %31, align 8, !tbaa !39
  %1024 = add nsw i32 %1023, -32000
  br label %1035

1025:                                             ; preds = %1019
  br label %1035

1026:                                             ; preds = %1012, %741
  %1027 = phi i1 [ %743, %741 ], [ %1016, %1012 ]
  %1028 = phi i32 [ %602, %741 ], [ %1013, %1012 ]
  %1029 = load i32, i32* %46, align 4, !tbaa !43
  %1030 = icmp sgt i32 %1029, 98
  %1031 = xor i1 %1027, true
  %1032 = or i1 %1030, %1031
  %1033 = select i1 %1030, i32 0, i32 %1028
  br i1 %1032, label %1035, label %1034

1034:                                             ; preds = %1026
  br label %1035

1035:                                             ; preds = %1034, %1026, %1025, %1022, %984, %912, %459, %232, %226, %216, %203, %164, %113, %106, %103, %77, %74, %72, %67, %61, %49, %36, %34
  %1036 = phi i32 [ %35, %34 ], [ %56, %49 ], [ %73, %72 ], [ 0, %36 ], [ %59, %61 ], [ %65, %67 ], [ %75, %74 ], [ %78, %77 ], [ %92, %103 ], [ %107, %106 ], [ %92, %113 ], [ 0, %164 ], [ %205, %216 ], [ 0, %203 ], [ %70, %459 ], [ %1024, %1022 ], [ 0, %1025 ], [ %1033, %1026 ], [ %1028, %1034 ], [ 0, %226 ], [ %64, %232 ], [ %916, %984 ], [ 0, %912 ]
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %28) #10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %27) #10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %26) #10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %25) #10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %24) #10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %23) #10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %22) #10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %21) #10
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %20) #10
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %19) #10
  ret i32 %1036
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
