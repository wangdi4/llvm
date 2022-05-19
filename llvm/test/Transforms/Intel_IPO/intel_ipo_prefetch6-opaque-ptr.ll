; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; Checks that IPO Prefetch pass can properly identify the prefetch opportunity, generate the prefetch
; function, and generate 2 calls to the prefetch function in 2 DL host functions: 1 call to prefetch inside
; each host function. This test case is the same as intel_ipo_prefetch6.ll, but it checks for opaque
; pointers.
;
; [Note]
; The IPO prefetch pass is a module pass that specifically examines code patterns like those exhibited in
; cpu2017/531.deepsjeng (531), generate a prefetch function based on one of the existing functions in 531
; and insert multiple calls to this prefetch function at specific pattern-matched host locations.
;
; The LIT testcase presented here is a vastly simplified version of 531.
; It takes the following 4 functions after all modules have been compiled individually and linked together:
; - _Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(): an existing function that the prefetch function is based upon;
; - _Z7qsearchP7state_tiiii(): host function1 where a call to prefetch function will be inserted;
; - _Z6searchP7state_tiiiii(): host function2 where a call to prefetch function will be inserted;
; - @main(): the main function, identifying the input LLVM IR as a complete module.

; Only the 4 functions listed above, plus any global data and metadata they use, will appear in this LIT test.
; All other functions in the module are reduced to declarations only. Their function bodies are purged, as is any global
; variable or metadata that is not used directly by any of the above 4 functions.
;

; *** Run command section ***
; RUN: opt < %s -opaque-pointers -intel-ipoprefetch -ipo-prefetch-be-lit-friendly=1 -ipo-prefetch-suppress-inline-report=0 -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -S 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes='module(intel-ipoprefetch)' -ipo-prefetch-be-lit-friendly=1 -ipo-prefetch-suppress-inline-report=0 -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -S 2>&1 | FileCheck %s
;

; *** Check section 1 ***
; The LLVM-IR check below ensures that a call to the Prefetch.Backbone function is inserted inside host
; _Z6searchP7state_tiiiii.
; CHECK: define internal noundef i32 @_Z6searchP7state_tiiiii(ptr noundef %0, i32 noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4, i32 noundef %5) #{{.}} {
; CHECK: call void @Prefetch.Backbone(ptr %0)
;

; *** Check section 2 ***
; The LLVM-IR check below ensures that a call to the Prefetch.Backbone function is inserted inside host
; _Z7qsearchP7state_tiiii.
; CHECK: define internal noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef %0, i32 noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) #{{.}} {
; CHECK: call void @Prefetch.Backbone(ptr %0)
;

; *** Check section 3 ***
; The LLVM-IR check below ensures the prefetch function is generated.
; CHECK: define internal void @Prefetch.Backbone(ptr nocapture noundef %0) #{{.}} {
;

; ModuleID = 'intel_ipo_prefetch6.ll'
source_filename = "<stdin>"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS8pawntt_t.pawntt_t = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i32, i32 }
%struct._ZTS7state_t.state_t = type { i32, [64 x i32], i64, i64, i64, [13 x i64], i32, i32, [13 x i32], i32, i32, i32, i32, i32, i32, i32, i64, i64, [64 x %struct._ZTS6move_x.move_x], [64 x i32], [64 x i32], [64 x %struct._ZTSN7state_tUt_E.anon], i64, i64, i32, [64 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i64] }
%struct._ZTS6move_x.move_x = type { i32, i32, i32, i32, i64, i64 }
%struct._ZTSN7state_tUt_E.anon = type { i32, i32, i32, i32 }
%struct._ZTS12scoreboard_t.scoreboard_t = type { i32, i32, [8 x %struct._ZTSN12scoreboard_tUt_E.anon], [8 x i32], [8 x %struct._ZTS7state_t.state_t] }
%struct._ZTSN12scoreboard_tUt_E.anon = type { i32, i32, i32 }
%struct._ZTS11gamestate_t.gamestate_t = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i32], [1000 x %struct._ZTS6move_x.move_x], i64, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct._ZTS12t_eval_comps.t_eval_comps = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct._ZTS9ttentry_t.ttentry_t = type { [4 x %struct._ZTS10ttbucket_t.ttbucket_t] }
%struct._ZTS10ttbucket_t.ttbucket_t = type { i32, i16, i16, i8, i8 }
%struct._ZTS13__va_list_tag.__va_list_tag = type { i32, i32, ptr, ptr }
%struct._ZTS8_IO_FILE._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._ZTS10_IO_marker._IO_marker = type opaque
%struct._ZTS11_IO_codecvt._IO_codecvt = type opaque
%struct._ZTS13_IO_wide_data._IO_wide_data = type opaque
%struct._ZTS12attackinfo_t.attackinfo_t = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }

@PawnMovesBlack = internal global [64 x i64] zeroinitializer, align 16
@PawnMovesWhite = internal global [64 x i64] zeroinitializer, align 16
@fillUpAttacks = internal global [64 x [8 x i64]] zeroinitializer, align 16
@aFileAttacks = internal global [64 x [8 x i64]] zeroinitializer, align 16
@firstRankAttacks = internal global [64 x [8 x i8]] zeroinitializer, align 16
@DiagMaska1h8 = internal global [64 x i64] zeroinitializer, align 16
@DiagMaska8h1 = internal global [64 x i64] zeroinitializer, align 16
@LeftMask = internal global [8 x i64] zeroinitializer, align 16
@RightMask = internal global [8 x i64] zeroinitializer, align 16
@QueenMask = internal global [64 x i64] zeroinitializer, align 16
@WhiteKingSide = internal global i64 0, align 8
@WhiteQueenSide = internal global i64 0, align 8
@BlackKingSide = internal global i64 0, align 8
@BlackQueenSide = internal global i64 0, align 8
@KingSafetyMask1 = internal global [64 x i64] zeroinitializer, align 16
@CenterMask = internal global i64 0, align 8
@SpaceMask = internal global [2 x i64] zeroinitializer, align 16
@_ZL19DiagonalLength_a1h8 = internal unnamed_addr constant [64 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1], align 16
@_ZL19DiagonalLength_a8h1 = internal unnamed_addr constant [64 x i32] [i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8], align 16
@last_bit = internal global [65536 x i8] zeroinitializer, align 16
@KingSafetyMask = internal global [64 x i64] zeroinitializer, align 16
@_ZZ14setup_epd_lineP11gamestate_tP7state_tPKcE11rankoffsets = internal unnamed_addr constant [8 x i32] [i32 0, i32 8, i32 16, i32 24, i32 32, i32 40, i32 48, i32 56], align 16
@.str.3 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.4 = private unnamed_addr constant [20 x i8] c"Workload not found\0A\00", align 1
@.str.5 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.6 = private unnamed_addr constant [23 x i8] c"Analyzing %d plies...\0A\00", align 1
@.str.7 = private unnamed_addr constant [31 x i8] c"\0ANodes: %llu (%0.2f%% qnodes)\0A\00", align 1
@.str.8 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@KingMoves = internal global [64 x i64] zeroinitializer, align 16
@PawnAttacksBlack = internal global [64 x i64] zeroinitializer, align 16
@PawnAttacksWhite = internal global [64 x i64] zeroinitializer, align 16
@.str = private unnamed_addr constant [10 x i8] c"sjeng.log\00", align 1
@InvMask = internal global [64 x i64] zeroinitializer, align 16
@CastleMask = internal global [4 x i64] zeroinitializer, align 16
@KnightMoves = internal global [64 x i64] zeroinitializer, align 16
@KingFilesMask = internal global [8 x i64] zeroinitializer, align 16
@KingPressureMask = internal global [64 x i64] zeroinitializer, align 16
@KingPressureMask1 = internal global [64 x i64] zeroinitializer, align 16
@_ZL8w_passer = internal unnamed_addr constant [6 x i32] [i32 185, i32 120, i32 70, i32 40, i32 20, i32 15], align 16
@_ZL23w_passer_pawn_supported = internal unnamed_addr constant [6 x i32] [i32 65, i32 25, i32 8, i32 -3, i32 -5, i32 -5], align 16
@_ZL23w_passer_king_supported = internal unnamed_addr constant [6 x i32] [i32 -25, i32 25, i32 7, i32 5, i32 5, i32 4], align 16
@_ZL13w_passer_free = internal unnamed_addr constant [6 x i32] [i32 185, i32 15, i32 10, i32 8, i32 3, i32 1], align 16
@_ZL18w_passer_very_free = internal unnamed_addr constant [6 x i32] [i32 0, i32 80, i32 30, i32 15, i32 10, i32 10], align 16
@_ZL16w_passer_blocked = internal unnamed_addr constant [6 x i32] [i32 -25, i32 -10, i32 -4, i32 0, i32 0, i32 0], align 16
@WhiteSqMask = internal global i64 0, align 8
@BlackSqMask = internal global i64 0, align 8
@_ZL6PawnTT = internal global [8 x [16384 x %struct._ZTS8pawntt_t.pawntt_t]] zeroinitializer, align 16
@FileMask = internal global [8 x i64] zeroinitializer, align 16
@FileUpMask = internal global [64 x i64] zeroinitializer, align 16
@AboveMask = internal global [8 x i64] zeroinitializer, align 16
@BelowMask = internal global [8 x i64] zeroinitializer, align 16
@RankMask = internal global [8 x i64] zeroinitializer, align 16
@_ZL11w_candidate = internal unnamed_addr constant [6 x i32] [i32 0, i32 44, i32 12, i32 10, i32 3, i32 3], align 16
@FileDownMask = internal global [64 x i64] zeroinitializer, align 16
@WhiteStrongSquareMask = internal global i64 0, align 8
@BlackStrongSquareMask = internal global i64 0, align 8
@psq_table = internal global [12 x [64 x i8]] zeroinitializer, align 16
@flip = internal constant [64 x i32] [i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7], align 16
@KSMask = internal global i64 0, align 8
@QSMask = internal global i64 0, align 8
@_ZL13wking_psq_end = internal unnamed_addr constant [64 x i32] [i32 0, i32 30, i32 35, i32 20, i32 20, i32 35, i32 30, i32 0, i32 -4, i32 25, i32 20, i32 24, i32 24, i32 20, i32 25, i32 -4, i32 -4, i32 14, i32 16, i32 6, i32 6, i32 16, i32 14, i32 -4, i32 -20, i32 0, i32 5, i32 -6, i32 -6, i32 5, i32 0, i32 -20, i32 -16, i32 -4, i32 0, i32 2, i32 2, i32 0, i32 -4, i32 -16, i32 -22, i32 -8, i32 -4, i32 0, i32 0, i32 -4, i32 -8, i32 -22, i32 -26, i32 -15, i32 -10, i32 -8, i32 -8, i32 -10, i32 -15, i32 -26, i32 -30, i32 -25, i32 -25, i32 -25, i32 -25, i32 -25, i32 -25, i32 -30], align 16
@_ZL17psq_king_kingside = internal unnamed_addr constant [64 x i32] [i32 -75, i32 -55, i32 -40, i32 -30, i32 -20, i32 -20, i32 -30, i32 -40, i32 -70, i32 -50, i32 -30, i32 -15, i32 -5, i32 0, i32 0, i32 -25, i32 -70, i32 -45, i32 -25, i32 -10, i32 20, i32 15, i32 10, i32 -20, i32 -60, i32 -35, i32 -15, i32 -5, i32 30, i32 30, i32 15, i32 -15, i32 -60, i32 -35, i32 -25, i32 -10, i32 25, i32 20, i32 10, i32 -15, i32 -75, i32 -45, i32 -25, i32 -15, i32 15, i32 15, i32 5, i32 -25, i32 -75, i32 -50, i32 -30, i32 -20, i32 -10, i32 -10, i32 -10, i32 -30, i32 -80, i32 -60, i32 -50, i32 -40, i32 -30, i32 -15, i32 -20, i32 -50], align 16
@_ZL18psq_king_queenside = internal unnamed_addr constant [64 x i32] [i32 -40, i32 -30, i32 -20, i32 -20, i32 -30, i32 -40, i32 -55, i32 -75, i32 -25, i32 0, i32 0, i32 -5, i32 -15, i32 -30, i32 -50, i32 -70, i32 -20, i32 10, i32 15, i32 20, i32 -10, i32 -25, i32 -45, i32 -70, i32 -15, i32 15, i32 30, i32 30, i32 -5, i32 -15, i32 -35, i32 -60, i32 -15, i32 10, i32 20, i32 25, i32 -10, i32 -25, i32 -35, i32 -60, i32 -25, i32 5, i32 15, i32 15, i32 -15, i32 -25, i32 -45, i32 -75, i32 -30, i32 -10, i32 -10, i32 -10, i32 -20, i32 -30, i32 -50, i32 -75, i32 -50, i32 -20, i32 -15, i32 -30, i32 -40, i32 -50, i32 -60, i32 -80], align 16
@_ZL15psq_king_nopawn = internal unnamed_addr constant [64 x i32] [i32 -40, i32 -30, i32 -22, i32 -20, i32 -20, i32 -22, i32 -30, i32 -40, i32 -30, i32 -15, i32 -10, i32 -5, i32 -5, i32 -10, i32 -15, i32 -30, i32 -22, i32 -10, i32 5, i32 10, i32 10, i32 5, i32 -10, i32 -22, i32 -20, i32 -5, i32 10, i32 20, i32 20, i32 10, i32 -5, i32 -20, i32 -20, i32 -5, i32 10, i32 20, i32 20, i32 10, i32 -5, i32 -20, i32 -22, i32 -10, i32 5, i32 10, i32 10, i32 5, i32 -10, i32 -22, i32 -30, i32 -15, i32 -10, i32 -5, i32 -5, i32 -10, i32 -15, i32 -30, i32 -40, i32 -30, i32 -22, i32 -20, i32 -20, i32 -22, i32 -30, i32 -40], align 16
@_ZL15wknight_psq_end = internal unnamed_addr constant [64 x i32] [i32 -25, i32 -5, i32 0, i32 8, i32 8, i32 0, i32 -5, i32 -25, i32 -16, i32 4, i32 10, i32 16, i32 16, i32 10, i32 4, i32 7, i32 -1, i32 15, i32 20, i32 22, i32 22, i32 20, i32 15, i32 -7, i32 -5, i32 10, i32 16, i32 16, i32 16, i32 16, i32 10, i32 6, i32 -6, i32 5, i32 14, i32 13, i32 13, i32 14, i32 5, i32 -2, i32 -14, i32 -3, i32 4, i32 7, i32 7, i32 4, i32 -3, i32 -14, i32 -20, i32 -12, i32 -4, i32 -5, i32 -5, i32 -4, i32 -12, i32 -20, i32 -25, i32 -24, i32 -16, i32 -14, i32 -14, i32 -16, i32 -24, i32 -25], align 16
@_ZL15wbishop_psq_end = internal unnamed_addr constant [64 x i32] [i32 -8, i32 -10, i32 -6, i32 -1, i32 -1, i32 -6, i32 -10, i32 -8, i32 -8, i32 -1, i32 -1, i32 0, i32 0, i32 -1, i32 -1, i32 -8, i32 -1, i32 5, i32 7, i32 8, i32 8, i32 7, i32 5, i32 -1, i32 -1, i32 4, i32 5, i32 10, i32 10, i32 5, i32 4, i32 -1, i32 2, i32 2, i32 3, i32 9, i32 9, i32 7, i32 3, i32 -5, i32 -2, i32 0, i32 6, i32 4, i32 4, i32 6, i32 0, i32 -2, i32 -5, i32 3, i32 1, i32 2, i32 2, i32 1, i32 3, i32 -5, i32 -10, i32 -6, i32 -8, i32 -8, i32 -8, i32 -8, i32 -6, i32 -10], align 16
@_ZL13wrook_psq_end = internal unnamed_addr constant [64 x i32] [i32 5, i32 5, i32 7, i32 10, i32 10, i32 7, i32 5, i32 5, i32 8, i32 10, i32 14, i32 14, i32 14, i32 14, i32 10, i32 8, i32 1, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 1, i32 -1, i32 4, i32 3, i32 0, i32 0, i32 3, i32 4, i32 -1, i32 -6, i32 -1, i32 -3, i32 -2, i32 -2, i32 -3, i32 -1, i32 -6, i32 -10, i32 -4, i32 -8, i32 -8, i32 -8, i32 -8, i32 -4, i32 -10, i32 -15, i32 -12, i32 -9, i32 -8, i32 -8, i32 -9, i32 -12, i32 -15, i32 -15, i32 -10, i32 -8, i32 -8, i32 -8, i32 -8, i32 -10, i32 -15], align 16
@_ZL14wqueen_psq_end = internal unnamed_addr constant [64 x i32] [i32 5, i32 12, i32 16, i32 16, i32 16, i32 16, i32 12, i32 5, i32 -10, i32 12, i32 20, i32 26, i32 26, i32 20, i32 12, i32 -10, i32 -5, i32 10, i32 15, i32 18, i32 18, i32 15, i32 10, i32 -5, i32 -15, i32 1, i32 10, i32 14, i32 14, i32 10, i32 1, i32 -15, i32 -7, i32 -4, i32 6, i32 9, i32 9, i32 6, i32 -4, i32 -7, i32 -12, i32 -8, i32 -2, i32 -4, i32 -4, i32 -2, i32 -8, i32 -12, i32 -12, i32 -12, i32 -13, i32 -10, i32 -10, i32 -13, i32 -12, i32 -12, i32 -20, i32 -25, i32 -25, i32 -10, i32 -10, i32 -25, i32 -25, i32 -20], align 16
@_ZL9wpawn_psq = internal unnamed_addr constant <{ [38 x i32], [26 x i32] }> <{ [38 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 -10, i32 0, i32 0, i32 0, i32 0, i32 -10, i32 0, i32 0, i32 -5, i32 8, i32 12, i32 12, i32 8, i32 -5, i32 0, i32 0, i32 0, i32 6, i32 10, i32 10, i32 6, i32 0, i32 0, i32 0, i32 0, i32 4, i32 8, i32 8, i32 4], [26 x i32] zeroinitializer }>, align 16
@history_h = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16
@history_hit = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16
@history_tot = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16
@_ZL8rc_index = internal unnamed_addr constant [14 x i32] [i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3, i32 4, i32 4, i32 5, i32 5, i32 2, i32 2, i32 0], align 16
@_ZZ11search_rootP7state_tiiiE7changes = internal unnamed_addr global i32 0, align 4
@_ZZ11search_rootP7state_tiiiE5bmove = internal unnamed_addr global i32 0, align 4
@.str.37 = private unnamed_addr constant [20 x i8] c"info refutation %s \00", align 1
@_ZZ5thinkP11gamestate_tP7state_tE15lastsearchscore = internal unnamed_addr global i32 0, align 4
@.str.1 = private unnamed_addr constant [41 x i8] c"info depth 1 time 0 nodes 1 score cp %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [13 x i8] c"bestmove %s\0A\00", align 1
@.str.3.40 = private unnamed_addr constant [16 x i8] c"Opening phase.\0A\00", align 1
@.str.4.41 = private unnamed_addr constant [19 x i8] c"Middlegame phase.\0A\00", align 1
@.str.5.42 = private unnamed_addr constant [16 x i8] c"Endgame phase.\0A\00", align 1
@.str.6.43 = private unnamed_addr constant [20 x i8] c"Time for move : %d\0A\00", align 1
@.str.7.44 = private unnamed_addr constant [50 x i8] c"info string Time for move: %ds, early break: %ds\0A\00", align 1
@.str.8.45 = private unnamed_addr constant [85 x i8] c"info string Nonsense in temp_move, time_failure %d failed %d time_exit %d result %d\0A\00", align 1
@.str.9 = private unnamed_addr constant [15 x i8] c"bestmove 0000\0A\00", align 1
@.str.10 = private unnamed_addr constant [23 x i8] c"bestmove %s ponder %s\0A\00", align 1
@RookMask = internal global [64 x i64] zeroinitializer, align 16
@BishopMask = internal global [64 x i64] zeroinitializer, align 16
@is_analyzing = internal global i32 0, align 4
@uci_showcurrline = internal global i32 0, align 4
@uci_showrefutations = internal global i32 0, align 4
@uci_limitstrength = internal global i32 0, align 4
@uci_elo = internal global i32 0, align 4
@contempt = internal global i32 0, align 4
@time_check_log = internal global i32 0, align 4
@.str.58 = private unnamed_addr constant [30 x i8] c"Please specify the workfile.\0A\00", align 1
@state = internal global %struct._ZTS7state_t.state_t zeroinitializer, align 8
@scoreboard = internal global %struct._ZTS12scoreboard_t.scoreboard_t zeroinitializer, align 8
@zobrist = internal global [14 x [64 x i64]] zeroinitializer, align 16
@TTable = internal global ptr null, align 8
@TTAge = internal global i32 0, align 4
@TTSize = internal global i32 0, align 4
@.str.101 = private unnamed_addr constant [38 x i8] c"Out of memory allocating hashtables.\0A\00", align 1
@.str.1.102 = private unnamed_addr constant [50 x i8] c"Allocated %d hash entries, totalling %llu bytes.\0A\00", align 1
@root_scores = internal global [240 x i32] zeroinitializer, align 16
@multipv_strings = internal global [240 x [512 x i8]] zeroinitializer, align 16
@multipv_scores = internal global [240 x i32] zeroinitializer, align 16
@uci_mode = internal global i32 0, align 4
@gamestate = internal global %struct._ZTS11gamestate_t.gamestate_t zeroinitializer, align 8
@allow_pondering = internal global i32 0, align 4
@__const._Z11comp_to_sanP7state_tiPc.type_to_char = private unnamed_addr constant [14 x i8] c"FPPNNKKRRQQBBE", align 1
@.str.117 = private unnamed_addr constant [5 x i8] c"%c%d\00", align 1
@.str.1.118 = private unnamed_addr constant [8 x i8] c"%c%d=%c\00", align 1
@.str.2.119 = private unnamed_addr constant [8 x i8] c"%cx%c%d\00", align 1
@.str.3.120 = private unnamed_addr constant [11 x i8] c"%cx%c%d=%c\00", align 1
@.str.5.121 = private unnamed_addr constant [6 x i8] c"O-O-O\00", align 1
@.str.6.122 = private unnamed_addr constant [9 x i8] c"%c%c%c%d\00", align 1
@.str.7.123 = private unnamed_addr constant [9 x i8] c"%c%d%c%d\00", align 1
@.str.8.124 = private unnamed_addr constant [10 x i8] c"%c%cx%c%d\00", align 1
@.str.9.125 = private unnamed_addr constant [10 x i8] c"%c%dx%c%d\00", align 1
@.str.10.126 = private unnamed_addr constant [7 x i8] c"%c%c%d\00", align 1
@.str.11 = private unnamed_addr constant [5 x i8] c"illg\00", align 1
@.str.14 = private unnamed_addr constant [5 x i8] c"0000\00", align 1
@uci_chess960_mode = internal global i32 0, align 4
@.str.15 = private unnamed_addr constant [10 x i8] c"%c%d%c%dn\00", align 1
@.str.16 = private unnamed_addr constant [10 x i8] c"%c%d%c%dr\00", align 1
@.str.17 = private unnamed_addr constant [10 x i8] c"%c%d%c%db\00", align 1
@.str.18 = private unnamed_addr constant [10 x i8] c"%c%d%c%dq\00", align 1
@.str.19 = private unnamed_addr constant [42 x i8] c"+----+----+----+----+----+----+----+----+\00", align 1
@.str.20 = private unnamed_addr constant [3 x i8] c"!!\00", align 1
@.str.21 = private unnamed_addr constant [3 x i8] c" P\00", align 1
@.str.22 = private unnamed_addr constant [3 x i8] c"*P\00", align 1
@.str.23 = private unnamed_addr constant [3 x i8] c" N\00", align 1
@.str.24 = private unnamed_addr constant [3 x i8] c"*N\00", align 1
@.str.25 = private unnamed_addr constant [3 x i8] c" K\00", align 1
@.str.26 = private unnamed_addr constant [3 x i8] c"*K\00", align 1
@.str.27 = private unnamed_addr constant [3 x i8] c" R\00", align 1
@.str.28 = private unnamed_addr constant [3 x i8] c"*R\00", align 1
@.str.29 = private unnamed_addr constant [3 x i8] c" Q\00", align 1
@.str.30 = private unnamed_addr constant [3 x i8] c"*Q\00", align 1
@.str.31 = private unnamed_addr constant [3 x i8] c" B\00", align 1
@.str.32 = private unnamed_addr constant [3 x i8] c"*B\00", align 1
@.str.33 = private unnamed_addr constant [3 x i8] c"  \00", align 1
@__const._Z13display_boardP7state_ti.piece_rep = private unnamed_addr constant [14 x ptr] [ptr @.str.20, ptr @.str.21, ptr @.str.22, ptr @.str.23, ptr @.str.24, ptr @.str.25, ptr @.str.26, ptr @.str.27, ptr @.str.28, ptr @.str.29, ptr @.str.30, ptr @.str.31, ptr @.str.32, ptr @.str.33], align 16
@.str.34 = private unnamed_addr constant [6 x i8] c"  %s\0A\00", align 1
@.str.35 = private unnamed_addr constant [5 x i8] c"%d |\00", align 1
@.str.36 = private unnamed_addr constant [6 x i8] c" %s |\00", align 1
@.str.37.133 = private unnamed_addr constant [7 x i8] c"\0A  %s\0A\00", align 1
@.str.38 = private unnamed_addr constant [45 x i8] c"\0A     a    b    c    d    e    f    g    h\0A\0A\00", align 1
@.str.39 = private unnamed_addr constant [45 x i8] c"\0A     h    g    f    e    d    c    b    a\0A\0A\00", align 1
@_ZZ9init_gameP11gamestate_tP7state_tE10init_board = internal unnamed_addr constant [64 x i32] [i32 8, i32 4, i32 12, i32 10, i32 6, i32 12, i32 4, i32 8, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 7, i32 3, i32 11, i32 9, i32 5, i32 11, i32 3, i32 7], align 16
@.str.41 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.str.42 = private unnamed_addr constant [20 x i8] c"%2d %7d %5d %8llu  \00", align 1
@.str.43 = private unnamed_addr constant [36 x i8] c"info currmove %s currmovenumber %d\0A\00", align 1
@.str.44 = private unnamed_addr constant [81 x i8] c"info depth %d seldepth %d time %d nodes %llu tbhits %d score cp %d multipv 1 pv \00", align 1
@EGTBHits = internal global i32 0, align 4
@.str.45 = private unnamed_addr constant [83 x i8] c"info depth %d seldepth %d time %d nodes %llu tbhits %d score mate %d multipv 1 pv \00", align 1
@.str.46 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@.str.47 = private unnamed_addr constant [92 x i8] c"info depth %d seldepth %d time %d nodes %llu tbhits %d score cp %d lowerbound multipv 1 pv \00", align 1
@.str.48 = private unnamed_addr constant [6 x i8] c"%s !!\00", align 1
@.str.49 = private unnamed_addr constant [35 x i8] c"info currmove %s currmovenumber %d\00", align 1
@.str.50 = private unnamed_addr constant [92 x i8] c"info depth %d seldepth %d time %d nodes %llu tbhits %d score cp %d upperbound multipv 1 pv \00", align 1
@.str.51 = private unnamed_addr constant [6 x i8] c"%s ??\00", align 1
@.str.52 = private unnamed_addr constant [66 x i8] c"depth %d seldepth %d time %d nodes %llu tbhits %d score cp %d pv \00", align 1
@.str.53 = private unnamed_addr constant [68 x i8] c"depth %d seldepth %d time %d nodes %llu tbhits %d score mate %d pv \00", align 1
@uci_multipv = internal global i32 0, align 4
@.str.54 = private unnamed_addr constant [17 x i8] c"info multipv %d \00", align 1
@material = internal constant [14 x i32] [i32 0, i32 85, i32 -85, i32 305, i32 -305, i32 40000, i32 -40000, i32 490, i32 -490, i32 935, i32 -935, i32 330, i32 -330, i32 0], align 16
@Mask = internal global [64 x i64] zeroinitializer, align 16
@cfg_logging = internal global i32 0, align 4
@cfg_logfile = internal global [512 x i8] zeroinitializer, align 16
@.str.55 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@buffered_count = internal global i32 0, align 4
@is_pondering = internal global i32 0, align 4
@buffered_command = internal global [20 x [8192 x i8]] zeroinitializer, align 16
@.str.58.158 = private unnamed_addr constant [74 x i8] c"Deep Sjeng version 3.2 SPEC, Copyright (C) 2000-2009 Gian-Carlo Pascutto\0A\00", align 1
@stdin = external dso_local local_unnamed_addr global ptr, align 8
@_ZL2s1 = internal unnamed_addr global i32 0, align 4
@_ZL2s2 = internal unnamed_addr global i32 0, align 4
@_ZL2s3 = internal unnamed_addr global i32 0, align 4
@_ZZL15hash_extract_pvP7state_tiPcE10levelstack = internal unnamed_addr global [65 x i64] zeroinitializer, align 16
@stdout = external dso_local local_unnamed_addr global ptr, align 8

; Function Attrs: mustprogress uwtable
declare dso_local i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #0

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
declare i32 @_Z11check_legalP7state_ti(ptr, i32) #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
declare i32 @_Z11is_attackedP7state_tii(ptr nocapture readonly, i32, i32) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @_Z11FileAttacksyj(i64, i32) #2

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @_Z11RankAttacksyj(i64, i32) #2

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable willreturn
declare zeroext i16 @_Z12compact_movei(i32) #3

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable willreturn
declare i32 @_Z4logLi(i32) #3

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
declare i32 @_Z12gen_capturesP7state_tPi(ptr, ptr) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @_Z11RookAttacksP7state_ti(ptr nocapture readonly, i32) #2

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
declare i32 @_Z12gen_evasionsP7state_tPii(ptr, ptr, i32) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @_Z13BishopAttacksP7state_ti(ptr nocapture readonly, i32) #2

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @_Z13retrieve_evalP7state_t(ptr nocapture readonly) #2

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
declare i32 @_Z15gen_good_checksP7state_tPi(ptr, ptr) #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
declare i32 @_Z3genP7state_tPi(ptr, ptr) #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
declare i32 @_Z3seeP7state_tiiii(ptr, i32, i32, i32, i32) #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
declare i32 @_Z4evalP7state_tiii(ptr, i32, i32, i32) #1

; Function Attrs: mustprogress norecurse nosync nounwind readonly uwtable willreturn
declare void @_Z4makeP7state_ti(ptr, i32) #4

; Function Attrs: mustprogress norecurse nosync nounwind readonly uwtable willreturn
declare void @_Z6unmakeP7state_ti(ptr, i32) #4

; Function Attrs: mustprogress norecurse nosync nounwind readonly uwtable willreturn
declare void @_Z7StoreTTP7state_tiiijiiii(ptr nocapture, i32, i32, i32, i32, i32, i32, i32, i32) #4

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @_Z7is_drawP11gamestate_tP7state_t(ptr nocapture readonly, ptr nocapture readonly) #2

declare i32 @_Z8in_checkP7state_t(ptr)

declare i32 @_Z9gen_quietP7state_tPi(ptr, ptr nonnull)

declare void @_ZL10PrefetchL3P7state_t(i32, i64)

declare void @_ZL11order_movesP7state_tPiS1_ij(ptr, ptr nonnull, ptr nonnull, i32, i32)

declare void @_ZL12history_goodP7state_tii(ptr nonnull, i32, i32)

declare i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull, ptr nonnull, ptr nonnull, i32)

declare void @_ZL16fast_order_movesP7state_tPiS1_ij(ptr, ptr nonnull, ptr nonnull, i32, i32)

declare i32 @_ZL17search_time_checkP7state_t(i64)

declare void @_ZL11history_badP7state_tii(ptr, i32, i32)

; Function Attrs: mustprogress nosync nounwind readnone uwtable willreturn
declare i32 @puts(ptr nocapture readonly) local_unnamed_addr #5

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.abs.i32(i32, i1 immarg) #2

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable willreturn
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %puts = tail call i32 @puts(ptr nonnull dereferenceable(1) getelementptr inbounds ([20 x i8], ptr @.str.4, i64 0, i64 0))
  ret i32 0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define internal noundef i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(ptr nocapture noundef %0, ptr nocapture noundef writeonly %1, i32 noundef %2, i32 noundef %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6, ptr nocapture noundef writeonly %7, ptr nocapture noundef writeonly %8, i32 noundef %9) #1 {
  store i32 1, ptr %6, align 4
  %11 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 26
  %12 = load i32, ptr %11, align 4
  %13 = add i32 %12, 1
  store i32 %13, ptr %11, align 4
  %14 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 11
  %15 = load i32, ptr %14, align 4
  %16 = icmp eq i32 %15, 0
  %17 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 16
  %18 = load i64, ptr %17, align 8
  %19 = zext i1 %16 to i64
  %20 = add i64 %18, %19
  %21 = trunc i64 %20 to i32
  %22 = load ptr, ptr @TTable, align 8
  %23 = load i32, ptr @TTSize, align 4
  %24 = urem i32 %21, %23
  %25 = zext i32 %24 to i64
  %26 = getelementptr inbounds %struct._ZTS9ttentry_t.ttentry_t, ptr %22, i64 %25
  %27 = lshr i64 %20, 32
  %28 = getelementptr inbounds %struct._ZTS9ttentry_t.ttentry_t, ptr %26, i64 0, i32 0
  %29 = trunc i64 %27 to i32
  br label %33

30:                                               ; preds = %33
  %31 = add nuw nsw i64 %34, 1
  %32 = icmp eq i64 %31, 4
  br i1 %32, label %131, label %33, !llvm.loop !53

33:                                               ; preds = %30, %10
  %34 = phi i64 [ 0, %10 ], [ %31, %30 ]
  %35 = getelementptr inbounds [4 x %struct._ZTS10ttbucket_t.ttbucket_t], ptr %28, i64 0, i64 %34
  %36 = load i32, ptr %35, align 4
  %37 = icmp eq i32 %36, %29
  br i1 %37, label %38, label %30

38:                                               ; preds = %33
  %39 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 27
  %40 = load i32, ptr %39, align 8
  %41 = add i32 %40, 1
  store i32 %41, ptr %39, align 8
  %42 = getelementptr inbounds %struct._ZTS10ttbucket_t.ttbucket_t, ptr %35, i64 0, i32 4
  %43 = load i8, ptr %42, align 1
  %44 = lshr i8 %43, 5
  %45 = and i8 %44, 3
  %46 = zext i8 %45 to i32
  %47 = load i32, ptr @TTAge, align 4
  %48 = icmp eq i32 %47, %46
  br i1 %48, label %55, label %49

49:                                               ; preds = %38
  %50 = trunc i32 %47 to i8
  %51 = shl i8 %50, 5
  %52 = and i8 %51, 96
  %53 = and i8 %43, -97
  %54 = or i8 %52, %53
  store i8 %54, ptr %42, align 1
  br label %55

55:                                               ; preds = %49, %38
  %56 = phi i8 [ %54, %49 ], [ %43, %38 ]
  %57 = and i8 %56, 6
  %58 = icmp eq i8 %57, 2
  br i1 %58, label %59, label %71

59:                                               ; preds = %55
  %60 = add nsw i32 %9, -16
  %61 = getelementptr inbounds %struct._ZTS10ttbucket_t.ttbucket_t, ptr %35, i64 0, i32 3
  %62 = load i8, ptr %61, align 4
  %63 = zext i8 %62 to i32
  %64 = icmp sgt i32 %60, %63
  br i1 %64, label %71, label %65

65:                                               ; preds = %59
  %66 = getelementptr inbounds %struct._ZTS10ttbucket_t.ttbucket_t, ptr %35, i64 0, i32 1
  %67 = load i16, ptr %66, align 4
  %68 = sext i16 %67 to i32
  %69 = icmp slt i32 %68, %3
  br i1 %69, label %70, label %71

70:                                               ; preds = %65
  store i32 0, ptr %6, align 4
  br label %71

71:                                               ; preds = %70, %65, %59, %55
  %72 = getelementptr inbounds %struct._ZTS10ttbucket_t.ttbucket_t, ptr %35, i64 0, i32 3
  %73 = load i8, ptr %72, align 4
  %74 = zext i8 %73 to i32
  %75 = icmp slt i32 %74, %9
  br i1 %75, label %108, label %76

76:                                               ; preds = %71
  %77 = getelementptr inbounds %struct._ZTS10ttbucket_t.ttbucket_t, ptr %35, i64 0, i32 1
  %78 = load i16, ptr %77, align 4
  %79 = sext i16 %78 to i32
  store i32 %79, ptr %1, align 4
  %80 = icmp sgt i16 %78, 31500
  br i1 %80, label %81, label %86

81:                                               ; preds = %76
  %82 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 14
  %83 = load i32, ptr %82, align 8
  %84 = add nsw i32 %79, 1
  %85 = sub i32 %84, %83
  store i32 %85, ptr %1, align 4
  br label %93

86:                                               ; preds = %76
  %87 = icmp slt i16 %78, -31500
  br i1 %87, label %88, label %93

88:                                               ; preds = %86
  %89 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 14
  %90 = load i32, ptr %89, align 8
  %91 = add nsw i32 %79, -1
  %92 = add i32 %91, %90
  store i32 %92, ptr %1, align 4
  br label %93

93:                                               ; preds = %88, %86, %81
  %94 = getelementptr inbounds %struct._ZTS10ttbucket_t.ttbucket_t, ptr %35, i64 0, i32 2
  %95 = load i16, ptr %94, align 2
  %96 = zext i16 %95 to i32
  store i32 %96, ptr %4, align 4
  %97 = and i8 %56, 1
  %98 = zext i8 %97 to i32
  store i32 %98, ptr %5, align 4
  %99 = lshr i8 %56, 3
  %100 = and i8 %99, 1
  %101 = zext i8 %100 to i32
  store i32 %101, ptr %7, align 4
  %102 = lshr i8 %56, 4
  %103 = and i8 %102, 1
  %104 = zext i8 %103 to i32
  store i32 %104, ptr %8, align 4
  %105 = lshr i8 %56, 1
  %106 = and i8 %105, 3
  %107 = zext i8 %106 to i32
  br label %131

108:                                              ; preds = %71
  %109 = getelementptr inbounds %struct._ZTS10ttbucket_t.ttbucket_t, ptr %35, i64 0, i32 2
  %110 = load i16, ptr %109, align 2
  %111 = zext i16 %110 to i32
  store i32 %111, ptr %4, align 4
  %112 = and i8 %56, 1
  %113 = zext i8 %112 to i32
  store i32 %113, ptr %5, align 4
  %114 = lshr i8 %56, 3
  %115 = and i8 %114, 1
  %116 = zext i8 %115 to i32
  store i32 %116, ptr %7, align 4
  %117 = lshr i8 %56, 4
  %118 = and i8 %117, 1
  %119 = zext i8 %118 to i32
  store i32 %119, ptr %8, align 4
  %120 = lshr i8 %56, 1
  %121 = and i8 %120, 3
  switch i8 %121, label %124 [
    i8 1, label %122
    i8 2, label %123
  ]

122:                                              ; preds = %108
  store i32 -1000000, ptr %1, align 4
  br label %128

123:                                              ; preds = %108
  store i32 1000000, ptr %1, align 4
  br label %128

124:                                              ; preds = %108
  %125 = getelementptr inbounds %struct._ZTS10ttbucket_t.ttbucket_t, ptr %35, i64 0, i32 1
  %126 = load i16, ptr %125, align 4
  %127 = sext i16 %126 to i32
  store i32 %127, ptr %1, align 4
  br label %128

128:                                              ; preds = %124, %123, %122
  %129 = trunc i32 %9 to i8
  store i8 %129, ptr %72, align 4
  %130 = and i8 %56, -7
  store i8 %130, ptr %42, align 1
  br label %131

131:                                              ; preds = %128, %93, %30
  %132 = phi i32 [ %107, %93 ], [ 0, %128 ], [ 4, %30 ]
  ret i32 %132
}

; Function Attrs: mustprogress uwtable
define internal noundef i32 @_Z6searchP7state_tiiiii(ptr noundef %0, i32 noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4, i32 noundef %5) #0 {
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
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %7) #28
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %8) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %9) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %10) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %11) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %12) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %13) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %14) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %15) #28
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %16) #28
  %19 = icmp slt i32 %3, 1
  br i1 %19, label %24, label %20

20:                                               ; preds = %6
  %21 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 14
  %22 = load i32, ptr %21, align 8
  %23 = icmp sgt i32 %22, 59
  br i1 %23, label %24, label %26

24:                                               ; preds = %20, %6
  %25 = tail call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef %0, i32 noundef %1, i32 noundef %2, i32 noundef 0, i32 noundef 0)
  br label %1016

26:                                               ; preds = %20
  %27 = getelementptr %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 22
  %28 = load i64, ptr %27, align 8
  %29 = add i64 %28, 1
  store i64 %29, ptr %27, align 8
  %30 = tail call fastcc noundef i32 @_ZL17search_time_checkP7state_t(i64 %29)
  %31 = icmp eq i32 %30, 0
  br i1 %31, label %32, label %1016

32:                                               ; preds = %26
  %33 = tail call noundef i32 @_Z7is_drawP11gamestate_tP7state_t(ptr noundef nonnull @gamestate, ptr noundef nonnull %0)
  %34 = icmp eq i32 %33, 0
  br i1 %34, label %35, label %39

35:                                               ; preds = %32
  %36 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 15
  %37 = load i32, ptr %36, align 4
  %38 = icmp sgt i32 %37, 99
  br i1 %38, label %39, label %47

39:                                               ; preds = %35, %32
  %40 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 3), align 4
  %41 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 11
  %42 = load i32, ptr %41, align 4
  %43 = icmp eq i32 %40, %42
  %44 = load i32, ptr @contempt, align 4
  %45 = sub nsw i32 0, %44
  %46 = select i1 %43, i32 %44, i32 %45
  br label %1016

47:                                               ; preds = %35
  %48 = load i32, ptr %21, align 8
  %49 = add nsw i32 %48, -32000
  %50 = icmp sgt i32 %49, %1
  br i1 %50, label %51, label %53

51:                                               ; preds = %47
  %52 = icmp slt i32 %49, %2
  br i1 %52, label %53, label %1016

53:                                               ; preds = %51, %47
  %54 = phi i32 [ %49, %51 ], [ %1, %47 ]
  %55 = sub i32 31999, %48
  %56 = icmp slt i32 %55, %2
  br i1 %56, label %57, label %59

57:                                               ; preds = %53
  %58 = icmp sgt i32 %55, %54
  br i1 %58, label %59, label %1016

59:                                               ; preds = %57, %53
  %60 = phi i32 [ %55, %57 ], [ %2, %53 ]
  %61 = call noundef i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(ptr noundef nonnull %0, ptr noundef nonnull %10, i32 noundef %54, i32 noundef %60, ptr noundef nonnull %13, ptr noundef nonnull %11, ptr noundef nonnull %12, ptr noundef nonnull %14, ptr noundef nonnull %15, i32 noundef %3)
  switch i32 %61, label %75 [
    i32 3, label %62
    i32 1, label %64
    i32 2, label %67
    i32 0, label %70
    i32 4, label %74
  ]

62:                                               ; preds = %59
  %63 = load i32, ptr %10, align 4
  br label %1016

64:                                               ; preds = %59
  %65 = load i32, ptr %10, align 4
  %66 = icmp sgt i32 %65, %54
  br i1 %66, label %75, label %1016

67:                                               ; preds = %59
  %68 = load i32, ptr %10, align 4
  %69 = icmp slt i32 %68, %60
  br i1 %69, label %75, label %1016

70:                                               ; preds = %59
  %71 = load i32, ptr %10, align 4
  %72 = icmp slt i32 %71, %60
  %73 = select i1 %72, i32 %5, i32 1
  br label %75

74:                                               ; preds = %59
  store i32 65535, ptr %13, align 4
  store i32 0, ptr %11, align 4
  store i32 0, ptr %14, align 4
  store i32 0, ptr %15, align 4
  br label %75

75:                                               ; preds = %74, %70, %67, %64, %59
  %76 = phi i32 [ %5, %59 ], [ %5, %74 ], [ 0, %64 ], [ 1, %67 ], [ %73, %70 ]
  %77 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 25
  %78 = load i32, ptr %21, align 8
  %79 = sext i32 %78 to i64
  %80 = getelementptr inbounds [64 x i32], ptr %77, i64 0, i64 %79
  %81 = load i32, ptr %80, align 4
  %82 = call noundef i32 @_Z13retrieve_evalP7state_t(ptr noundef nonnull %0)
  %83 = icmp ne i32 %81, 0
  %84 = xor i1 %83, true
  %85 = add nsw i32 %54, 1
  %86 = icmp eq i32 %60, %85
  %87 = select i1 %84, i1 %86, i1 false
  br i1 %87, label %88, label %112

88:                                               ; preds = %75
  %89 = icmp ult i32 %3, 5
  br i1 %89, label %90, label %102

90:                                               ; preds = %88
  %91 = add nsw i32 %82, -75
  %92 = icmp slt i32 %91, %60
  br i1 %92, label %98, label %93

93:                                               ; preds = %90
  %94 = load i32, ptr %13, align 4
  %95 = load i32, ptr %11, align 4
  %96 = load i32, ptr %14, align 4
  %97 = load i32, ptr %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef nonnull %0, i32 noundef %91, i32 noundef %54, i32 noundef %60, i32 noundef %94, i32 noundef %95, i32 noundef %96, i32 noundef %97, i32 noundef %3)
  br label %1016

98:                                               ; preds = %90
  %99 = icmp slt i32 %82, %60
  br i1 %99, label %100, label %112

100:                                              ; preds = %98
  %101 = call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef nonnull %0, i32 noundef %54, i32 noundef %60, i32 noundef 0, i32 noundef 0)
  br label %1016

102:                                              ; preds = %88
  %103 = icmp ult i32 %3, 9
  br i1 %103, label %104, label %112

104:                                              ; preds = %102
  %105 = add nsw i32 %82, -125
  %106 = icmp slt i32 %105, %60
  br i1 %106, label %112, label %107

107:                                              ; preds = %104
  %108 = load i32, ptr %13, align 4
  %109 = load i32, ptr %11, align 4
  %110 = load i32, ptr %14, align 4
  %111 = load i32, ptr %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef nonnull %0, i32 noundef %105, i32 noundef %54, i32 noundef %60, i32 noundef %108, i32 noundef %109, i32 noundef %110, i32 noundef %111, i32 noundef %3)
  br label %1016

112:                                              ; preds = %104, %102, %98, %75
  %113 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 8
  %114 = getelementptr inbounds [13 x i32], ptr %113, i64 0, i64 9
  %115 = load i32, ptr %114, align 4
  %116 = getelementptr inbounds [13 x i32], ptr %113, i64 0, i64 7
  %117 = load i32, ptr %116, align 4
  %118 = add nsw i32 %117, %115
  %119 = getelementptr inbounds [13 x i32], ptr %113, i64 0, i64 11
  %120 = load i32, ptr %119, align 4
  %121 = add nsw i32 %118, %120
  %122 = getelementptr inbounds [13 x i32], ptr %113, i64 0, i64 3
  %123 = load i32, ptr %122, align 4
  %124 = add nsw i32 %121, %123
  %125 = getelementptr inbounds [13 x i32], ptr %113, i64 0, i64 10
  %126 = load i32, ptr %125, align 8
  %127 = getelementptr inbounds [13 x i32], ptr %113, i64 0, i64 8
  %128 = load i32, ptr %127, align 8
  %129 = add nsw i32 %128, %126
  %130 = getelementptr inbounds [13 x i32], ptr %113, i64 0, i64 12
  %131 = load i32, ptr %130, align 8
  %132 = add nsw i32 %129, %131
  %133 = getelementptr inbounds [13 x i32], ptr %113, i64 0, i64 4
  %134 = load i32, ptr %133, align 8
  %135 = add nsw i32 %132, %134
  store i32 0, ptr %11, align 4
  %136 = icmp eq i32 %4, 0
  br i1 %136, label %137, label %221

137:                                              ; preds = %112
  %138 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 11
  %139 = load i32, ptr %138, align 4
  %140 = icmp eq i32 %139, 0
  %141 = select i1 %140, i32 %135, i32 %124
  %142 = icmp eq i32 %141, 0
  %143 = select i1 %142, i1 true, i1 %83
  %144 = xor i1 %143, true
  %145 = load i32, ptr %12, align 4
  %146 = icmp ne i32 %145, 0
  %147 = select i1 %144, i1 %146, i1 false
  %148 = icmp ugt i32 %3, 4
  %149 = and i1 %86, %148
  %150 = select i1 %147, i1 %149, i1 false
  br i1 %150, label %151, label %221

151:                                              ; preds = %137
  %152 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 1), align 4
  %153 = icmp eq i32 %152, 2
  br i1 %153, label %154, label %173

154:                                              ; preds = %151
  %155 = icmp ult i32 %3, 25
  %156 = add nsw i32 %60, -1
  br i1 %155, label %157, label %159

157:                                              ; preds = %154
  %158 = call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef nonnull %0, i32 noundef %156, i32 noundef %60, i32 noundef 0, i32 noundef 0)
  br label %162

159:                                              ; preds = %154
  %160 = add nsw i32 %3, -24
  %161 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef nonnull %0, i32 noundef %156, i32 noundef %60, i32 noundef %160, i32 noundef 1, i32 noundef %76)
  br label %162

162:                                              ; preds = %159, %157
  %163 = phi i32 [ %158, %157 ], [ %161, %159 ]
  %164 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %165 = icmp eq i32 %164, 0
  br i1 %165, label %166, label %1016

166:                                              ; preds = %162
  %167 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 1), align 4
  %168 = icmp eq i32 %167, 2
  %169 = icmp slt i32 %163, %60
  %170 = and i1 %168, %169
  br i1 %170, label %238, label %171

171:                                              ; preds = %166
  %172 = load i32, ptr %138, align 4
  br label %173

173:                                              ; preds = %171, %151
  %174 = phi i32 [ %172, %171 ], [ %139, %151 ]
  %175 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 10
  %176 = load i32, ptr %175, align 8
  store i32 0, ptr %175, align 8
  %177 = xor i32 %174, 1
  store i32 %177, ptr %138, align 4
  %178 = load i32, ptr %21, align 8
  %179 = add nsw i32 %178, 1
  store i32 %179, ptr %21, align 8
  %180 = load i32, ptr %36, align 4
  %181 = add nsw i32 %180, 1
  store i32 %181, ptr %36, align 4
  %182 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 19
  %183 = sext i32 %178 to i64
  %184 = getelementptr inbounds [64 x i32], ptr %182, i64 0, i64 %183
  store i32 0, ptr %184, align 4
  %185 = sext i32 %179 to i64
  %186 = getelementptr inbounds [64 x i32], ptr %77, i64 0, i64 %185
  store i32 0, ptr %186, align 4
  %187 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 20
  %188 = getelementptr inbounds [64 x i32], ptr %187, i64 0, i64 %183
  %189 = load i32, ptr %188, align 4
  %190 = getelementptr inbounds [64 x i32], ptr %187, i64 0, i64 %185
  store i32 %189, ptr %190, align 4
  %191 = icmp ult i32 %3, 17
  %192 = sub nsw i32 0, %60
  %193 = sub i32 1, %60
  br i1 %191, label %194, label %196

194:                                              ; preds = %173
  %195 = call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef nonnull %0, i32 noundef %192, i32 noundef %193, i32 noundef 0, i32 noundef 0)
  br label %201

196:                                              ; preds = %173
  %197 = add nsw i32 %3, -16
  %198 = icmp eq i32 %76, 0
  %199 = zext i1 %198 to i32
  %200 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef nonnull %0, i32 noundef %192, i32 noundef %193, i32 noundef %197, i32 noundef 1, i32 noundef %199)
  br label %201

201:                                              ; preds = %196, %194
  %202 = phi i32 [ %195, %194 ], [ %200, %196 ]
  %203 = sub nsw i32 0, %202
  %204 = load i32, ptr %36, align 4
  %205 = add nsw i32 %204, -1
  store i32 %205, ptr %36, align 4
  %206 = load i32, ptr %21, align 8
  %207 = add nsw i32 %206, -1
  store i32 %207, ptr %21, align 8
  %208 = load i32, ptr %138, align 4
  %209 = xor i32 %208, 1
  store i32 %209, ptr %138, align 4
  store i32 %176, ptr %175, align 8
  %210 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %211 = icmp eq i32 %210, 0
  br i1 %211, label %212, label %1016

212:                                              ; preds = %201
  %213 = icmp sgt i32 %60, %203
  br i1 %213, label %218, label %214

214:                                              ; preds = %212
  %215 = load i32, ptr %13, align 4
  %216 = load i32, ptr %11, align 4
  %217 = load i32, ptr %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef nonnull %0, i32 noundef %203, i32 noundef %54, i32 noundef %60, i32 noundef %215, i32 noundef %216, i32 noundef 0, i32 noundef %217, i32 noundef %3)
  br label %1016

218:                                              ; preds = %212
  %219 = icmp sgt i32 %202, 31400
  br i1 %219, label %220, label %238

220:                                              ; preds = %218
  store i32 1, ptr %11, align 4
  br label %238

221:                                              ; preds = %137, %112
  %222 = icmp ult i32 %3, 13
  %223 = and i1 %86, %222
  %224 = add nsw i32 %60, -300
  %225 = icmp slt i32 %82, %224
  %226 = select i1 %223, i1 %225, i1 false
  br i1 %226, label %227, label %238

227:                                              ; preds = %221
  %228 = call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef nonnull %0, i32 noundef %54, i32 noundef %60, i32 noundef 0, i32 noundef 0)
  %229 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %230 = icmp eq i32 %229, 0
  br i1 %230, label %231, label %1016

231:                                              ; preds = %227
  %232 = icmp sgt i32 %228, %54
  br i1 %232, label %238, label %233

233:                                              ; preds = %231
  %234 = load i32, ptr %13, align 4
  %235 = load i32, ptr %11, align 4
  %236 = load i32, ptr %14, align 4
  %237 = load i32, ptr %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef nonnull %0, i32 noundef %54, i32 noundef %54, i32 noundef %60, i32 noundef %234, i32 noundef %235, i32 noundef %236, i32 noundef %237, i32 noundef %3)
  br label %1016

238:                                              ; preds = %231, %221, %220, %218, %166
  br i1 %83, label %239, label %262

239:                                              ; preds = %238
  %240 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 0
  %241 = call noundef i32 @_Z12gen_evasionsP7state_tPii(ptr noundef nonnull %0, ptr noundef nonnull %240, i32 noundef %81)
  %242 = icmp eq i32 %241, 0
  br i1 %242, label %265, label %243

243:                                              ; preds = %239
  %244 = icmp sgt i32 %241, 0
  br i1 %244, label %245, label %265

245:                                              ; preds = %243
  %246 = zext i32 %241 to i64
  br label %247

247:                                              ; preds = %247, %245
  %248 = phi i64 [ 0, %245 ], [ %258, %247 ]
  %249 = phi i32 [ 0, %245 ], [ %256, %247 ]
  %250 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %248
  %251 = load i32, ptr %250, align 4
  call void @_Z4makeP7state_ti(ptr noundef %0, i32 noundef %251)
  %252 = load i32, ptr %250, align 4
  %253 = call noundef i32 @_Z11check_legalP7state_ti(ptr noundef %0, i32 noundef %252)
  %254 = icmp ne i32 %253, 0
  %255 = zext i1 %254 to i32
  %256 = add nuw nsw i32 %249, %255
  %257 = load i32, ptr %250, align 4
  call void @_Z6unmakeP7state_ti(ptr noundef %0, i32 noundef %257)
  %258 = add nuw nsw i64 %248, 1
  %259 = icmp ult i64 %258, %246
  %260 = icmp ult i32 %256, 2
  %261 = select i1 %259, i1 %260, i1 false
  br i1 %261, label %247, label %265, !llvm.loop !45

262:                                              ; preds = %238
  %263 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 0
  %264 = call noundef i32 @_Z3genP7state_tPi(ptr noundef nonnull %0, ptr noundef nonnull %263)
  br label %265

265:                                              ; preds = %262, %247, %243, %239
  %266 = phi i32 [ 0, %239 ], [ %264, %262 ], [ %241, %243 ], [ %241, %247 ]
  %267 = phi i32 [ 0, %239 ], [ %264, %262 ], [ 0, %243 ], [ %256, %247 ]
  %268 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 0
  %269 = getelementptr inbounds [240 x i32], ptr %8, i64 0, i64 0
  %270 = load i32, ptr %13, align 4
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(ptr noundef %0, ptr noundef nonnull %268, ptr noundef nonnull %269, i32 noundef %266, i32 noundef %270)
  %271 = icmp sgt i32 %3, 19
  br i1 %271, label %272, label %318

272:                                              ; preds = %265
  %273 = icmp ne i32 %60, %85
  %274 = load i32, ptr %13, align 4
  %275 = icmp eq i32 %274, 65535
  %276 = select i1 %273, i1 %275, i1 false
  br i1 %276, label %277, label %318

277:                                              ; preds = %272
  %278 = icmp sgt i32 %266, 0
  br i1 %278, label %279, label %308

279:                                              ; preds = %277
  %280 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 1
  %281 = zext i32 %266 to i64
  br label %282

282:                                              ; preds = %302, %279
  %283 = phi i64 [ 0, %279 ], [ %304, %302 ]
  %284 = phi i32 [ 0, %279 ], [ %303, %302 ]
  %285 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %283
  %286 = load i32, ptr %285, align 4
  %287 = lshr i32 %286, 19
  %288 = and i32 %287, 15
  %289 = icmp eq i32 %288, 13
  br i1 %289, label %302, label %290

290:                                              ; preds = %282
  %291 = lshr i32 %286, 6
  %292 = and i32 %291, 63
  %293 = zext i32 %292 to i64
  %294 = getelementptr inbounds [64 x i32], ptr %280, i64 0, i64 %293
  %295 = load i32, ptr %294, align 4
  %296 = sext i32 %295 to i64
  %297 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %296
  %298 = load i32, ptr %297, align 4
  %299 = call i32 @llvm.abs.i32(i32 %298, i1 true)
  %300 = icmp ugt i32 %288, %299
  %301 = select i1 %300, i32 1, i32 %284
  br label %302

302:                                              ; preds = %290, %282
  %303 = phi i32 [ %284, %282 ], [ %301, %290 ]
  %304 = add nuw nsw i64 %283, 1
  %305 = icmp eq i64 %304, %281
  br i1 %305, label %306, label %282, !llvm.loop !46

306:                                              ; preds = %302
  %307 = icmp eq i32 %303, 0
  br i1 %307, label %308, label %318

308:                                              ; preds = %306, %277
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %17) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %18) #28
  %309 = ashr i32 %3, 1
  %310 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef %0, i32 noundef %54, i32 noundef %60, i32 noundef %309, i32 noundef 0, i32 noundef %76)
  %311 = call noundef i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(ptr noundef %0, ptr noundef nonnull %17, i32 noundef 0, i32 noundef 0, ptr noundef nonnull %18, ptr noundef nonnull %17, ptr noundef nonnull %17, ptr noundef nonnull %17, ptr noundef nonnull %17, i32 noundef 0)
  %312 = icmp eq i32 %311, 4
  br i1 %312, label %315, label %313

313:                                              ; preds = %308
  %314 = load i32, ptr %18, align 4
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(ptr noundef %0, ptr noundef nonnull %268, ptr noundef nonnull %269, i32 noundef %266, i32 noundef %314)
  br label %317

315:                                              ; preds = %308
  %316 = load i32, ptr %13, align 4
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(ptr noundef %0, ptr noundef nonnull %268, ptr noundef nonnull %269, i32 noundef %266, i32 noundef %316)
  br label %317

317:                                              ; preds = %315, %313
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %18) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %17) #28
  br label %318

318:                                              ; preds = %317, %306, %272, %265
  %319 = load i32, ptr %11, align 4
  %320 = icmp eq i32 %319, 0
  %321 = select i1 %84, i1 %320, i1 false
  %322 = icmp sgt i32 %3, 15
  %323 = and i1 %321, %322
  %324 = icmp sgt i32 %267, 8
  %325 = select i1 %323, i1 %324, i1 false
  br i1 %325, label %326, label %424

326:                                              ; preds = %318
  %327 = load i32, ptr %21, align 8
  %328 = add nsw i32 %327, -1
  %329 = sext i32 %328 to i64
  %330 = getelementptr inbounds [64 x i32], ptr %77, i64 0, i64 %329
  %331 = load i32, ptr %330, align 4
  %332 = icmp eq i32 %331, 0
  br i1 %332, label %333, label %424

333:                                              ; preds = %326
  %334 = icmp slt i32 %327, 3
  br i1 %334, label %349, label %335

335:                                              ; preds = %333
  %336 = add nsw i32 %327, -2
  %337 = zext i32 %336 to i64
  %338 = getelementptr inbounds [64 x i32], ptr %77, i64 0, i64 %337
  %339 = load i32, ptr %338, align 4
  %340 = icmp eq i32 %339, 0
  br i1 %340, label %341, label %424

341:                                              ; preds = %335
  %342 = icmp ult i32 %327, 4
  br i1 %342, label %349, label %343

343:                                              ; preds = %341
  %344 = add nsw i32 %327, -3
  %345 = zext i32 %344 to i64
  %346 = getelementptr inbounds [64 x i32], ptr %77, i64 0, i64 %345
  %347 = load i32, ptr %346, align 4
  %348 = icmp eq i32 %347, 0
  br i1 %348, label %349, label %424

349:                                              ; preds = %343, %341, %333
  store i32 -1, ptr %9, align 4
  %350 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  %351 = icmp eq i32 %350, 0
  br i1 %351, label %424, label %352

352:                                              ; preds = %349
  %353 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 16
  %354 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 36
  %355 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 19
  %356 = sub nsw i32 0, %60
  %357 = sub i32 50, %54
  %358 = icmp ugt i32 %3, 16
  %359 = icmp ult i32 %3, 17
  %360 = sub i32 1, %60
  %361 = add nsw i32 %3, -16
  %362 = icmp eq i32 %76, 0
  %363 = zext i1 %362 to i32
  br label %364

364:                                              ; preds = %412, %352
  %365 = phi i32 [ 0, %352 ], [ %413, %412 ]
  %366 = phi i32 [ 0, %352 ], [ %368, %412 ]
  %367 = phi i32 [ -32000, %352 ], [ %403, %412 ]
  %368 = add nuw nsw i32 %366, 1
  %369 = load i32, ptr %9, align 4
  %370 = sext i32 %369 to i64
  %371 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %370
  %372 = load i32, ptr %371, align 4
  call void @_Z4makeP7state_ti(ptr noundef %0, i32 noundef %372)
  %373 = load i32, ptr %371, align 4
  %374 = call noundef i32 @_Z11check_legalP7state_ti(ptr noundef %0, i32 noundef %373)
  %375 = icmp eq i32 %374, 0
  br i1 %375, label %402, label %376

376:                                              ; preds = %364
  %377 = load i64, ptr %353, align 8
  %378 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 15), align 4
  %379 = load i32, ptr %21, align 8
  %380 = add i32 %379, -1
  %381 = add i32 %380, %378
  %382 = sext i32 %381 to i64
  %383 = getelementptr inbounds [1000 x i64], ptr %354, i64 0, i64 %382
  store i64 %377, ptr %383, align 8
  %384 = load i32, ptr %371, align 4
  %385 = sext i32 %380 to i64
  %386 = getelementptr inbounds [64 x i32], ptr %355, i64 0, i64 %385
  store i32 %384, ptr %386, align 4
  %387 = call noundef i32 @_Z8in_checkP7state_t(ptr noundef %0)
  %388 = load i32, ptr %21, align 8
  %389 = sext i32 %388 to i64
  %390 = getelementptr inbounds [64 x i32], ptr %77, i64 0, i64 %389
  store i32 %387, ptr %390, align 4
  %391 = icmp ne i32 %387, 0
  %392 = or i1 %358, %391
  %393 = zext i1 %392 to i32
  %394 = call noundef i32 @_Z4evalP7state_tiii(ptr noundef %0, i32 noundef %356, i32 noundef %357, i32 noundef %393)
  br i1 %359, label %395, label %397

395:                                              ; preds = %376
  %396 = call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef nonnull %0, i32 noundef %356, i32 noundef %360, i32 noundef 0, i32 noundef 0)
  br label %399

397:                                              ; preds = %376
  %398 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef nonnull %0, i32 noundef %356, i32 noundef %360, i32 noundef %361, i32 noundef 0, i32 noundef %363)
  br label %399

399:                                              ; preds = %397, %395
  %400 = phi i32 [ %396, %395 ], [ %398, %397 ]
  %401 = sub nsw i32 0, %400
  br label %402

402:                                              ; preds = %399, %364
  %403 = phi i32 [ %401, %399 ], [ %367, %364 ]
  %404 = load i32, ptr %371, align 4
  call void @_Z6unmakeP7state_ti(ptr noundef %0, i32 noundef %404)
  %405 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %406 = icmp eq i32 %405, 0
  br i1 %406, label %407, label %422

407:                                              ; preds = %402
  %408 = icmp slt i32 %403, %60
  %409 = or i1 %375, %408
  br i1 %409, label %412, label %410

410:                                              ; preds = %407
  %411 = icmp sgt i32 %365, 0
  br i1 %411, label %418, label %412

412:                                              ; preds = %410, %407
  %413 = phi i32 [ 1, %410 ], [ %365, %407 ]
  %414 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  %415 = icmp ne i32 %414, 0
  %416 = icmp ult i32 %366, 2
  %417 = select i1 %415, i1 %416, i1 false
  br i1 %417, label %364, label %422, !llvm.loop !47

418:                                              ; preds = %410
  %419 = load i32, ptr %13, align 4
  %420 = load i32, ptr %11, align 4
  %421 = load i32, ptr %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef %0, i32 noundef %60, i32 noundef %54, i32 noundef %60, i32 noundef %419, i32 noundef %420, i32 noundef 0, i32 noundef %421, i32 noundef %3)
  br label %1016

422:                                              ; preds = %412, %402
  %423 = load i32, ptr %11, align 4
  br label %424

424:                                              ; preds = %422, %349, %343, %335, %326, %318
  %425 = phi i32 [ 0, %326 ], [ 0, %343 ], [ 0, %335 ], [ %319, %318 ], [ 0, %349 ], [ %423, %422 ]
  %426 = phi i32 [ -32000, %326 ], [ -32000, %343 ], [ -32000, %335 ], [ -32000, %318 ], [ -32000, %349 ], [ %403, %422 ]
  %427 = load i32, ptr %14, align 4
  %428 = icmp eq i32 %427, 0
  %429 = load i32, ptr %15, align 4
  %430 = icmp eq i32 %429, 0
  %431 = select i1 %428, i1 %430, i1 false
  %432 = icmp eq i32 %425, 0
  %433 = select i1 %431, i1 %432, i1 false
  %434 = and i1 %433, %271
  %435 = icmp sgt i32 %267, 1
  %436 = select i1 %434, i1 %435, i1 false
  %437 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 1), align 4
  %438 = icmp ne i32 %437, 2
  %439 = select i1 %436, i1 %438, i1 false
  br i1 %439, label %440, label %517

440:                                              ; preds = %424
  %441 = add nsw i32 %3, -24
  %442 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef %0, i32 noundef %54, i32 noundef %60, i32 noundef %441, i32 noundef 0, i32 noundef %76)
  %443 = icmp sgt i32 %442, %54
  br i1 %443, label %444, label %517

444:                                              ; preds = %440
  store i32 -1, ptr %9, align 4
  %445 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  %446 = icmp ne i32 %445, 0
  %447 = load i32, ptr %14, align 4
  %448 = icmp slt i32 %447, 2
  %449 = select i1 %446, i1 %448, i1 false
  br i1 %449, label %450, label %517

450:                                              ; preds = %444
  %451 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 16
  %452 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 36
  %453 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 19
  %454 = add nsw i32 %3, -16
  %455 = sub nsw i32 0, %60
  %456 = sub i32 50, %54
  %457 = sub nsw i32 0, %54
  %458 = xor i32 %54, -1
  %459 = icmp eq i32 %76, 0
  %460 = zext i1 %459 to i32
  %461 = sub i32 49, %54
  %462 = add nsw i32 %54, -50
  br label %463

463:                                              ; preds = %505, %450
  %464 = phi i32 [ 0, %450 ], [ %508, %505 ]
  %465 = phi i32 [ %426, %450 ], [ %507, %505 ]
  %466 = phi i32 [ 1, %450 ], [ %506, %505 ]
  %467 = load i32, ptr %9, align 4
  %468 = sext i32 %467 to i64
  %469 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %468
  %470 = load i32, ptr %469, align 4
  call void @_Z4makeP7state_ti(ptr noundef %0, i32 noundef %470)
  %471 = load i32, ptr %469, align 4
  %472 = call noundef i32 @_Z11check_legalP7state_ti(ptr noundef %0, i32 noundef %471)
  %473 = icmp eq i32 %472, 0
  br i1 %473, label %505, label %474

474:                                              ; preds = %463
  %475 = load i64, ptr %451, align 8
  %476 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 15), align 4
  %477 = load i32, ptr %21, align 8
  %478 = add i32 %477, -1
  %479 = add i32 %478, %476
  %480 = sext i32 %479 to i64
  %481 = getelementptr inbounds [1000 x i64], ptr %452, i64 0, i64 %480
  store i64 %475, ptr %481, align 8
  %482 = load i32, ptr %469, align 4
  %483 = sext i32 %478 to i64
  %484 = getelementptr inbounds [64 x i32], ptr %453, i64 0, i64 %483
  store i32 %482, ptr %484, align 4
  %485 = add nsw i32 %464, 1
  %486 = call noundef i32 @_Z8in_checkP7state_t(ptr noundef %0)
  %487 = load i32, ptr %21, align 8
  %488 = sext i32 %487 to i64
  %489 = getelementptr inbounds [64 x i32], ptr %77, i64 0, i64 %488
  store i32 %486, ptr %489, align 4
  %490 = call noundef i32 @_Z4evalP7state_tiii(ptr noundef %0, i32 noundef %455, i32 noundef %456, i32 noundef 1)
  %491 = icmp eq i32 %466, 0
  br i1 %491, label %499, label %492

492:                                              ; preds = %474
  %493 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef nonnull %0, i32 noundef %458, i32 noundef %457, i32 noundef %454, i32 noundef 0, i32 noundef %460)
  %494 = sub nsw i32 0, %493
  %495 = icmp slt i32 %54, %494
  br i1 %495, label %496, label %497

496:                                              ; preds = %492
  store i32 1, ptr %14, align 4
  br label %505

497:                                              ; preds = %492
  store i32 0, ptr %14, align 4
  %498 = add nsw i32 %464, 11
  br label %505

499:                                              ; preds = %474
  %500 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef nonnull %0, i32 noundef %461, i32 noundef %456, i32 noundef %454, i32 noundef 0, i32 noundef 0)
  %501 = sub nsw i32 0, %500
  %502 = icmp slt i32 %462, %501
  br i1 %502, label %503, label %505

503:                                              ; preds = %499
  store i32 0, ptr %14, align 4
  %504 = add nsw i32 %464, 11
  br label %505

505:                                              ; preds = %503, %499, %497, %496, %463
  %506 = phi i32 [ %466, %463 ], [ 0, %499 ], [ 0, %503 ], [ 0, %496 ], [ 0, %497 ]
  %507 = phi i32 [ %465, %463 ], [ %501, %499 ], [ %501, %503 ], [ %494, %496 ], [ %494, %497 ]
  %508 = phi i32 [ %464, %463 ], [ %485, %499 ], [ %504, %503 ], [ %485, %496 ], [ %498, %497 ]
  %509 = load i32, ptr %469, align 4
  call void @_Z6unmakeP7state_ti(ptr noundef %0, i32 noundef %509)
  %510 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  %511 = icmp ne i32 %510, 0
  %512 = load i32, ptr %14, align 4
  %513 = icmp slt i32 %512, 2
  %514 = select i1 %511, i1 %513, i1 false
  %515 = icmp slt i32 %508, 3
  %516 = select i1 %514, i1 %515, i1 false
  br i1 %516, label %463, label %517, !llvm.loop !48

517:                                              ; preds = %505, %444, %440, %424
  %518 = phi i32 [ %426, %424 ], [ %426, %440 ], [ %426, %444 ], [ %507, %505 ]
  br i1 %86, label %524, label %519

519:                                              ; preds = %517
  %520 = load i32, ptr %21, align 8
  %521 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 5), align 4
  %522 = shl nsw i32 %521, 1
  %523 = icmp sle i32 %520, %522
  br label %524

524:                                              ; preds = %519, %517
  %525 = phi i1 [ false, %517 ], [ %523, %519 ]
  store i32 -1, ptr %9, align 4
  %526 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  %527 = icmp eq i32 %526, 0
  br i1 %527, label %984, label %528

528:                                              ; preds = %524
  %529 = icmp eq i32 %267, 1
  %530 = select i1 %83, i1 %529, i1 false
  %531 = select i1 %530, i32 4, i32 0
  %532 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 1
  %533 = or i32 %531, 2
  %534 = select i1 %525, i32 %533, i32 %531
  %535 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 19
  %536 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 11
  %537 = select i1 %525, i32 3, i32 1
  %538 = add nsw i32 %135, %124
  %539 = icmp eq i32 %538, 1
  %540 = sdiv i32 %3, 4
  %541 = add nsw i32 %540, 1
  %542 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 0
  %543 = icmp slt i32 %3, 25
  %544 = icmp slt i32 %3, 9
  %545 = icmp ult i32 %3, 13
  %546 = add nsw i32 %82, 100
  %547 = add nsw i32 %82, 300
  %548 = add nsw i32 %82, 75
  %549 = add nsw i32 %82, 200
  %550 = select i1 %525, i32 4, i32 2
  %551 = sub nsw i32 0, %60
  %552 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 16
  %553 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 36
  %554 = icmp sgt i32 %3, 4
  %555 = icmp eq i32 %76, 0
  %556 = zext i1 %555 to i32
  %557 = add nsw i32 %3, -4
  br label %558

558:                                              ; preds = %972, %528
  %559 = phi i32 [ %54, %528 ], [ %979, %972 ]
  %560 = phi i32 [ %518, %528 ], [ %978, %972 ]
  %561 = phi i32 [ 1, %528 ], [ %977, %972 ]
  %562 = phi i32 [ -32000, %528 ], [ %976, %972 ]
  %563 = phi i32 [ 1, %528 ], [ %975, %972 ]
  %564 = phi i32 [ 1, %528 ], [ %974, %972 ]
  %565 = icmp ne i32 %563, 0
  %566 = icmp sgt i32 %564, %541
  %567 = add nsw i32 %559, 1
  %568 = icmp eq i32 %60, %567
  %569 = load i32, ptr %21, align 8
  %570 = icmp slt i32 %569, 60
  br i1 %570, label %622, label %571

571:                                              ; preds = %558
  %572 = load i32, ptr %9, align 4
  %573 = sext i32 %572 to i64
  %574 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %573
  %575 = load i32, ptr %574, align 4
  %576 = and i32 %575, 7864320
  %577 = icmp eq i32 %576, 6815744
  br i1 %577, label %578, label %758

578:                                              ; preds = %571
  br i1 %566, label %579, label %758

579:                                              ; preds = %578
  %580 = load i32, ptr %542, align 8
  %581 = sext i32 %580 to i64
  %582 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_hit, i64 0, i64 %581
  %583 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_tot, i64 0, i64 %581
  %584 = and i1 %568, %543
  br i1 %584, label %585, label %758

585:                                              ; preds = %610, %579
  %586 = phi i32 [ %614, %610 ], [ %575, %579 ]
  %587 = phi i32 [ %611, %610 ], [ %572, %579 ]
  %588 = phi i32 [ 0, %610 ], [ %561, %579 ]
  %589 = lshr i32 %586, 6
  %590 = and i32 %589, 63
  %591 = zext i32 %590 to i64
  %592 = getelementptr inbounds [64 x i32], ptr %532, i64 0, i64 %591
  %593 = load i32, ptr %592, align 4
  %594 = add nsw i32 %593, -1
  %595 = and i32 %586, 63
  %596 = sext i32 %594 to i64
  %597 = getelementptr inbounds [12 x [64 x i32]], ptr %582, i64 0, i64 %596
  %598 = zext i32 %595 to i64
  %599 = getelementptr inbounds [64 x i32], ptr %597, i64 0, i64 %598
  %600 = load i32, ptr %599, align 4
  %601 = getelementptr inbounds [12 x [64 x i32]], ptr %583, i64 0, i64 %596
  %602 = getelementptr inbounds [64 x i32], ptr %601, i64 0, i64 %598
  %603 = load i32, ptr %602, align 4
  %604 = sub nsw i32 %603, %600
  %605 = mul nsw i32 %600, %541
  %606 = icmp slt i32 %605, %604
  %607 = and i32 %586, 61440
  %608 = icmp eq i32 %607, 0
  %609 = select i1 %606, i1 %608, i1 false
  br i1 %609, label %617, label %752

610:                                              ; preds = %617
  %611 = load i32, ptr %9, align 4
  %612 = sext i32 %611 to i64
  %613 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %612
  %614 = load i32, ptr %613, align 4
  %615 = and i32 %614, 7864320
  %616 = icmp eq i32 %615, 6815744
  br i1 %616, label %585, label %752, !llvm.loop !49

617:                                              ; preds = %585
  %618 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  %619 = icmp eq i32 %618, 0
  br i1 %619, label %981, label %610, !llvm.loop !49

620:                                              ; preds = %749
  %621 = load i32, ptr %21, align 8
  br label %622

622:                                              ; preds = %620, %558
  %623 = phi i32 [ %621, %620 ], [ %569, %558 ]
  %624 = phi i32 [ 0, %620 ], [ %561, %558 ]
  %625 = icmp slt i32 %623, 60
  br i1 %625, label %629, label %626

626:                                              ; preds = %622
  %627 = load i32, ptr %9, align 4
  %628 = sext i32 %627 to i64
  br label %708

629:                                              ; preds = %622
  %630 = load i32, ptr %9, align 4
  %631 = sext i32 %630 to i64
  %632 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %631
  %633 = load i32, ptr %632, align 4
  %634 = lshr i32 %633, 6
  %635 = and i32 %634, 63
  %636 = zext i32 %635 to i64
  %637 = getelementptr inbounds [64 x i32], ptr %532, i64 0, i64 %636
  %638 = load i32, ptr %637, align 4
  %639 = add nsw i32 %638, 1
  %640 = and i32 %639, -2
  %641 = icmp eq i32 %640, 2
  br i1 %641, label %642, label %649

642:                                              ; preds = %629
  %643 = lshr i32 %633, 3
  %644 = and i32 %643, 7
  switch i32 %644, label %645 [
    i32 1, label %648
    i32 6, label %648
  ]

645:                                              ; preds = %642
  %646 = and i32 %633, 61440
  %647 = icmp eq i32 %646, 0
  br i1 %647, label %649, label %648

648:                                              ; preds = %645, %642, %642
  br label %649

649:                                              ; preds = %648, %645, %629
  %650 = phi i32 [ %531, %645 ], [ %531, %629 ], [ %534, %648 ]
  %651 = lshr i32 %633, 19
  %652 = and i32 %651, 15
  %653 = icmp eq i32 %652, 13
  br i1 %653, label %684, label %654

654:                                              ; preds = %649
  %655 = add nsw i32 %623, -1
  %656 = sext i32 %655 to i64
  %657 = getelementptr inbounds [64 x i32], ptr %535, i64 0, i64 %656
  %658 = load i32, ptr %657, align 4
  %659 = lshr i32 %658, 19
  %660 = and i32 %659, 15
  %661 = icmp eq i32 %660, 13
  br i1 %661, label %684, label %662

662:                                              ; preds = %654
  %663 = zext i32 %652 to i64
  %664 = getelementptr inbounds [14 x i32], ptr @_ZL8rc_index, i64 0, i64 %663
  %665 = load i32, ptr %664, align 4
  %666 = zext i32 %660 to i64
  %667 = getelementptr inbounds [14 x i32], ptr @_ZL8rc_index, i64 0, i64 %666
  %668 = load i32, ptr %667, align 4
  %669 = icmp eq i32 %665, %668
  br i1 %669, label %670, label %684

670:                                              ; preds = %662
  %671 = and i32 %633, 63
  %672 = and i32 %658, 63
  %673 = icmp eq i32 %671, %672
  br i1 %673, label %674, label %684

674:                                              ; preds = %670
  %675 = load i32, ptr %536, align 4
  %676 = icmp eq i32 %675, 0
  %677 = zext i1 %676 to i32
  %678 = lshr i32 %633, 12
  %679 = and i32 %678, 15
  %680 = call noundef i32 @_Z3seeP7state_tiiii(ptr noundef nonnull %0, i32 noundef %677, i32 noundef %635, i32 noundef %671, i32 noundef %679)
  %681 = icmp sgt i32 %680, 0
  %682 = select i1 %681, i32 %537, i32 0
  %683 = add nsw i32 %682, %650
  br label %684

684:                                              ; preds = %674, %670, %662, %654, %649
  %685 = phi i32 [ %650, %670 ], [ %650, %662 ], [ %650, %654 ], [ %650, %649 ], [ %683, %674 ]
  %686 = load i32, ptr %14, align 4
  %687 = icmp eq i32 %686, 1
  %688 = icmp ne i32 %685, 0
  %689 = select i1 %687, i1 %688, i1 false
  %690 = select i1 %689, i1 %565, i1 false
  br i1 %690, label %691, label %692

691:                                              ; preds = %684
  store i32 1, ptr %15, align 4
  br label %698

692:                                              ; preds = %684
  %693 = xor i1 %688, true
  %694 = select i1 %693, i1 %687, i1 false
  %695 = select i1 %694, i1 %565, i1 false
  br i1 %695, label %696, label %698

696:                                              ; preds = %692
  store i32 0, ptr %15, align 4
  %697 = add nsw i32 %685, %537
  br label %698

698:                                              ; preds = %696, %692, %691
  %699 = phi i32 [ %685, %691 ], [ %685, %692 ], [ %697, %696 ]
  %700 = icmp sgt i32 %699, 4
  %701 = select i1 %700, i32 4, i32 %699
  %702 = load i32, ptr %632, align 4
  %703 = lshr i32 %702, 19
  %704 = and i32 %703, 15
  switch i32 %704, label %705 [
    i32 13, label %708
    i32 1, label %708
    i32 2, label %708
  ]

705:                                              ; preds = %698
  %706 = add nsw i32 %701, 4
  %707 = select i1 %539, i32 %706, i32 %701
  br label %708

708:                                              ; preds = %705, %698, %698, %698, %626
  %709 = phi i64 [ %628, %626 ], [ %631, %705 ], [ %631, %698 ], [ %631, %698 ], [ %631, %698 ]
  %710 = phi i32 [ 0, %626 ], [ %707, %705 ], [ %701, %698 ], [ %701, %698 ], [ %701, %698 ]
  %711 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %709
  %712 = load i32, ptr %711, align 4
  %713 = and i32 %712, 7864320
  %714 = icmp eq i32 %713, 6815744
  %715 = xor i1 %714, true
  %716 = xor i1 %566, true
  %717 = select i1 %715, i1 true, i1 %716
  %718 = select i1 %715, i32 %624, i32 %561
  br i1 %717, label %758, label %719

719:                                              ; preds = %708
  %720 = lshr i32 %712, 6
  %721 = and i32 %720, 63
  %722 = zext i32 %721 to i64
  %723 = getelementptr inbounds [64 x i32], ptr %532, i64 0, i64 %722
  %724 = load i32, ptr %723, align 4
  %725 = add nsw i32 %724, -1
  %726 = and i32 %712, 63
  %727 = load i32, ptr %542, align 8
  %728 = sext i32 %727 to i64
  %729 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_hit, i64 0, i64 %728
  %730 = sext i32 %725 to i64
  %731 = getelementptr inbounds [12 x [64 x i32]], ptr %729, i64 0, i64 %730
  %732 = zext i32 %726 to i64
  %733 = getelementptr inbounds [64 x i32], ptr %731, i64 0, i64 %732
  %734 = load i32, ptr %733, align 4
  %735 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_tot, i64 0, i64 %728
  %736 = getelementptr inbounds [12 x [64 x i32]], ptr %735, i64 0, i64 %730
  %737 = getelementptr inbounds [64 x i32], ptr %736, i64 0, i64 %732
  %738 = load i32, ptr %737, align 4
  %739 = sub nsw i32 %738, %734
  %740 = mul nsw i32 %734, %541
  %741 = icmp slt i32 %740, %739
  %742 = icmp eq i32 %710, 0
  %743 = and i1 %741, %543
  %744 = select i1 %743, i1 %742, i1 false
  %745 = select i1 %744, i1 %568, i1 false
  %746 = and i32 %712, 61440
  %747 = icmp eq i32 %746, 0
  %748 = select i1 %745, i1 %747, i1 false
  br i1 %748, label %749, label %758

749:                                              ; preds = %719
  %750 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  %751 = icmp eq i32 %750, 0
  br i1 %751, label %981, label %620, !llvm.loop !50

752:                                              ; preds = %610, %585
  %753 = phi i32 [ %587, %585 ], [ %611, %610 ]
  %754 = phi i32 [ %586, %585 ], [ %614, %610 ]
  %755 = phi i32 [ %588, %585 ], [ 0, %610 ]
  %756 = sext i32 %753 to i64
  %757 = xor i1 %609, true
  br label %758

758:                                              ; preds = %752, %719, %708, %579, %578, %571
  %759 = phi i64 [ %756, %752 ], [ %573, %571 ], [ %573, %579 ], [ %573, %578 ], [ %709, %708 ], [ %709, %719 ]
  %760 = phi i32 [ 0, %752 ], [ 0, %571 ], [ 0, %579 ], [ 0, %578 ], [ %710, %708 ], [ %710, %719 ]
  %761 = phi i32 [ %754, %752 ], [ %575, %571 ], [ %575, %579 ], [ %575, %578 ], [ %712, %708 ], [ %712, %719 ]
  %762 = phi i1 [ %757, %752 ], [ false, %571 ], [ true, %579 ], [ true, %578 ], [ %714, %708 ], [ %714, %719 ]
  %763 = phi i32 [ %755, %752 ], [ %561, %571 ], [ %561, %579 ], [ %561, %578 ], [ %718, %708 ], [ %624, %719 ]
  %764 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %759
  br i1 %544, label %765, label %768

765:                                              ; preds = %758
  %766 = icmp slt i32 %548, %559
  %767 = icmp slt i32 %549, %559
  br label %773

768:                                              ; preds = %758
  %769 = icmp slt i32 %546, %559
  %770 = icmp slt i32 %547, %559
  %771 = select i1 %545, i1 %769, i1 false
  %772 = select i1 %545, i1 %770, i1 false
  br label %773

773:                                              ; preds = %768, %765
  %774 = phi i1 [ %766, %765 ], [ %771, %768 ]
  %775 = phi i1 [ %767, %765 ], [ %772, %768 ]
  br i1 %762, label %786, label %776

776:                                              ; preds = %773
  %777 = load i32, ptr %536, align 4
  %778 = icmp eq i32 %777, 0
  %779 = zext i1 %778 to i32
  %780 = lshr i32 %761, 6
  %781 = and i32 %780, 63
  %782 = and i32 %761, 63
  %783 = lshr i32 %761, 12
  %784 = and i32 %783, 15
  %785 = call noundef i32 @_Z3seeP7state_tiiii(ptr noundef nonnull %0, i32 noundef %779, i32 noundef %781, i32 noundef %782, i32 noundef %784)
  br label %786

786:                                              ; preds = %776, %773
  %787 = phi i32 [ %785, %776 ], [ -1000000, %773 ]
  %788 = load i32, ptr %764, align 4
  call void @_Z4makeP7state_ti(ptr noundef nonnull %0, i32 noundef %788)
  %789 = load i32, ptr %764, align 4
  %790 = call noundef i32 @_Z11check_legalP7state_ti(ptr noundef nonnull %0, i32 noundef %789)
  %791 = icmp eq i32 %790, 0
  br i1 %791, label %926, label %792

792:                                              ; preds = %786
  %793 = call noundef i32 @_Z8in_checkP7state_t(ptr noundef nonnull %0)
  %794 = icmp ne i32 %793, 0
  %795 = select i1 %794, i32 %550, i32 0
  %796 = add nsw i32 %795, %760
  %797 = or i32 %793, %81
  %798 = icmp eq i32 %797, 0
  %799 = select i1 %798, i1 %568, i1 false
  br i1 %799, label %800, label %818

800:                                              ; preds = %792
  %801 = icmp slt i32 %787, 86
  %802 = and i1 %775, %801
  br i1 %802, label %803, label %809

803:                                              ; preds = %800
  %804 = load i32, ptr %764, align 4
  %805 = and i32 %804, 61440
  %806 = icmp eq i32 %805, 0
  br i1 %806, label %807, label %809

807:                                              ; preds = %803
  call void @_Z6unmakeP7state_ti(ptr noundef nonnull %0, i32 noundef %804)
  %808 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  br label %972, !llvm.loop !49

809:                                              ; preds = %803, %800
  %810 = icmp slt i32 %787, -50
  %811 = and i1 %774, %810
  br i1 %811, label %812, label %818

812:                                              ; preds = %809
  %813 = load i32, ptr %764, align 4
  %814 = and i32 %813, 61440
  %815 = icmp eq i32 %814, 0
  br i1 %815, label %816, label %818

816:                                              ; preds = %812
  call void @_Z6unmakeP7state_ti(ptr noundef nonnull %0, i32 noundef %813)
  %817 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  br label %972, !llvm.loop !49

818:                                              ; preds = %812, %809, %792
  %819 = add nsw i32 %796, %3
  %820 = sub nsw i32 0, %559
  %821 = sub i32 130, %559
  %822 = icmp sgt i32 %819, 4
  %823 = or i1 %822, %794
  %824 = zext i1 %823 to i32
  %825 = call noundef i32 @_Z4evalP7state_tiii(ptr noundef nonnull %0, i32 noundef %551, i32 noundef %821, i32 noundef %824)
  %826 = load i32, ptr %21, align 8
  %827 = sext i32 %826 to i64
  %828 = getelementptr inbounds [64 x i32], ptr %77, i64 0, i64 %827
  store i32 %793, ptr %828, align 4
  %829 = load i64, ptr %552, align 8
  %830 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 15), align 4
  %831 = add i32 %826, -1
  %832 = add i32 %831, %830
  %833 = sext i32 %832 to i64
  %834 = getelementptr inbounds [1000 x i64], ptr %553, i64 0, i64 %833
  store i64 %829, ptr %834, align 8
  %835 = load i32, ptr %764, align 4
  %836 = sext i32 %831 to i64
  %837 = getelementptr inbounds [64 x i32], ptr %535, i64 0, i64 %836
  store i32 %835, ptr %837, align 4
  %838 = icmp sgt i32 %564, 3
  %839 = select i1 %554, i1 %838, i1 false
  br i1 %839, label %840, label %875

840:                                              ; preds = %818
  %841 = icmp ne i32 %60, %567
  %842 = icmp ne i32 %796, 0
  %843 = select i1 %841, i1 true, i1 %842
  %844 = or i1 %843, %794
  %845 = icmp sgt i32 %787, -51
  %846 = or i1 %844, %845
  br i1 %846, label %875, label %847

847:                                              ; preds = %840
  %848 = and i32 %835, 63
  %849 = zext i32 %848 to i64
  %850 = getelementptr inbounds [64 x i32], ptr %532, i64 0, i64 %849
  %851 = load i32, ptr %850, align 4
  %852 = add nsw i32 %851, -1
  %853 = load i32, ptr %542, align 8
  %854 = sext i32 %853 to i64
  %855 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_hit, i64 0, i64 %854
  %856 = sext i32 %852 to i64
  %857 = getelementptr inbounds [12 x [64 x i32]], ptr %855, i64 0, i64 %856
  %858 = getelementptr inbounds [64 x i32], ptr %857, i64 0, i64 %849
  %859 = load i32, ptr %858, align 4
  %860 = shl i32 %859, 7
  %861 = add i32 %860, 128
  %862 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_tot, i64 0, i64 %854
  %863 = getelementptr inbounds [12 x [64 x i32]], ptr %862, i64 0, i64 %856
  %864 = getelementptr inbounds [64 x i32], ptr %863, i64 0, i64 %849
  %865 = load i32, ptr %864, align 4
  %866 = add nsw i32 %865, 1
  %867 = sdiv i32 %861, %866
  %868 = icmp slt i32 %867, 80
  %869 = and i32 %835, 61440
  %870 = icmp eq i32 %869, 0
  %871 = select i1 %868, i1 %870, i1 false
  %872 = select i1 %871, i32 4, i32 0
  %873 = select i1 %871, i32 -4, i32 0
  %874 = select i1 %871, i32 %557, i32 %819
  br label %875

875:                                              ; preds = %847, %840, %818
  %876 = phi i1 [ false, %840 ], [ false, %818 ], [ %871, %847 ]
  %877 = phi i32 [ 0, %840 ], [ 0, %818 ], [ %872, %847 ]
  %878 = phi i32 [ %796, %840 ], [ %796, %818 ], [ %873, %847 ]
  %879 = phi i32 [ %819, %840 ], [ %819, %818 ], [ %874, %847 ]
  %880 = add nsw i32 %879, -4
  %881 = icmp eq i32 %563, 1
  %882 = icmp slt i32 %879, 5
  br i1 %881, label %883, label %890

883:                                              ; preds = %875
  br i1 %882, label %884, label %887

884:                                              ; preds = %883
  %885 = call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef nonnull %0, i32 noundef %551, i32 noundef %820, i32 noundef 0, i32 noundef 0)
  %886 = sub nsw i32 0, %885
  br label %922

887:                                              ; preds = %883
  %888 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef nonnull %0, i32 noundef %551, i32 noundef %820, i32 noundef %880, i32 noundef 0, i32 noundef %556)
  %889 = sub nsw i32 0, %888
  br label %922

890:                                              ; preds = %875
  %891 = xor i32 %559, -1
  br i1 %882, label %892, label %894

892:                                              ; preds = %890
  %893 = call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef nonnull %0, i32 noundef %891, i32 noundef %820, i32 noundef 0, i32 noundef 0)
  br label %896

894:                                              ; preds = %890
  %895 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef nonnull %0, i32 noundef %891, i32 noundef %820, i32 noundef %880, i32 noundef 0, i32 noundef 1)
  br label %896

896:                                              ; preds = %894, %892
  %897 = phi i32 [ %893, %892 ], [ %895, %894 ]
  %898 = sub nsw i32 0, %897
  %899 = icmp slt i32 %562, %898
  %900 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %901 = icmp eq i32 %900, 0
  %902 = select i1 %899, i1 %901, i1 false
  %903 = icmp slt i32 %559, %898
  %904 = select i1 %902, i1 %903, i1 false
  br i1 %904, label %905, label %922

905:                                              ; preds = %896
  br i1 %876, label %906, label %908

906:                                              ; preds = %905
  %907 = add nsw i32 %878, %877
  br label %910

908:                                              ; preds = %905
  %909 = icmp sgt i32 %60, %898
  br i1 %909, label %910, label %922

910:                                              ; preds = %908, %906
  %911 = phi i32 [ %907, %906 ], [ %878, %908 ]
  %912 = add nsw i32 %911, %3
  %913 = icmp slt i32 %912, 5
  br i1 %913, label %914, label %917

914:                                              ; preds = %910
  %915 = call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef nonnull %0, i32 noundef %551, i32 noundef %820, i32 noundef 0, i32 noundef 0)
  %916 = sub nsw i32 0, %915
  br label %922

917:                                              ; preds = %910
  %918 = add nsw i32 %912, -4
  %919 = zext i1 %876 to i32
  %920 = call noundef i32 @_Z6searchP7state_tiiiii(ptr noundef nonnull %0, i32 noundef %551, i32 noundef %820, i32 noundef %918, i32 noundef 0, i32 noundef %919)
  %921 = sub nsw i32 0, %920
  br label %922

922:                                              ; preds = %917, %914, %908, %896, %887, %884
  %923 = phi i32 [ %886, %884 ], [ %889, %887 ], [ %898, %896 ], [ %916, %914 ], [ %921, %917 ], [ %898, %908 ]
  %924 = icmp sgt i32 %923, %562
  %925 = select i1 %924, i32 %923, i32 %562
  br label %926

926:                                              ; preds = %922, %786
  %927 = phi i32 [ %562, %786 ], [ %925, %922 ]
  %928 = phi i32 [ %763, %786 ], [ 0, %922 ]
  %929 = phi i32 [ %560, %786 ], [ %923, %922 ]
  %930 = load i32, ptr %764, align 4
  call void @_Z6unmakeP7state_ti(ptr noundef nonnull %0, i32 noundef %930)
  %931 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %932 = icmp eq i32 %931, 0
  br i1 %932, label %933, label %1016

933:                                              ; preds = %926
  br i1 %791, label %967, label %934

934:                                              ; preds = %933
  %935 = icmp sgt i32 %929, %559
  br i1 %935, label %936, label %960

936:                                              ; preds = %934
  %937 = icmp slt i32 %929, %60
  %938 = load i32, ptr %764, align 4
  br i1 %937, label %957, label %939

939:                                              ; preds = %936
  call fastcc void @_ZL12history_goodP7state_tii(ptr noundef nonnull %0, i32 noundef %938, i32 noundef %3)
  %940 = icmp sgt i32 %564, 1
  br i1 %940, label %941, label %950

941:                                              ; preds = %939
  %942 = add nsw i32 %564, -1
  %943 = zext i32 %942 to i64
  br label %944

944:                                              ; preds = %944, %941
  %945 = phi i64 [ 0, %941 ], [ %948, %944 ]
  %946 = getelementptr inbounds [240 x i32], ptr %16, i64 0, i64 %945
  %947 = load i32, ptr %946, align 4
  call fastcc void @_ZL11history_badP7state_tii(ptr noundef %0, i32 noundef %947, i32 noundef %3)
  %948 = add nuw nsw i64 %945, 1
  %949 = icmp eq i64 %948, %943
  br i1 %949, label %950, label %944, !llvm.loop !52

950:                                              ; preds = %944, %939
  %951 = load i32, ptr %764, align 4
  %952 = call noundef zeroext i16 @_Z12compact_movei(i32 noundef %951)
  %953 = zext i16 %952 to i32
  %954 = load i32, ptr %11, align 4
  %955 = load i32, ptr %14, align 4
  %956 = load i32, ptr %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef %0, i32 noundef %929, i32 noundef %54, i32 noundef %60, i32 noundef %953, i32 noundef %954, i32 noundef %955, i32 noundef %956, i32 noundef %3)
  br label %1016

957:                                              ; preds = %936
  %958 = call noundef zeroext i16 @_Z12compact_movei(i32 noundef %938)
  %959 = zext i16 %958 to i32
  store i32 %959, ptr %13, align 4
  br label %960

960:                                              ; preds = %957, %934
  %961 = phi i32 [ %929, %957 ], [ %559, %934 ]
  %962 = load i32, ptr %764, align 4
  %963 = add nsw i32 %564, -1
  %964 = sext i32 %963 to i64
  %965 = getelementptr inbounds [240 x i32], ptr %16, i64 0, i64 %964
  store i32 %962, ptr %965, align 4
  %966 = add nsw i32 %564, 1
  br label %967

967:                                              ; preds = %960, %933
  %968 = phi i32 [ %966, %960 ], [ %564, %933 ]
  %969 = phi i32 [ 0, %960 ], [ %563, %933 ]
  %970 = phi i32 [ %961, %960 ], [ %559, %933 ]
  %971 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %9, ptr noundef nonnull %269, ptr noundef nonnull %268, i32 noundef %266)
  br label %972

972:                                              ; preds = %967, %816, %807
  %973 = phi i32 [ %971, %967 ], [ %817, %816 ], [ %808, %807 ]
  %974 = phi i32 [ %968, %967 ], [ %564, %816 ], [ %564, %807 ]
  %975 = phi i32 [ %969, %967 ], [ %563, %816 ], [ %563, %807 ]
  %976 = phi i32 [ %927, %967 ], [ %559, %816 ], [ %559, %807 ]
  %977 = phi i32 [ %928, %967 ], [ 0, %816 ], [ 0, %807 ]
  %978 = phi i32 [ %929, %967 ], [ %560, %816 ], [ %560, %807 ]
  %979 = phi i32 [ %970, %967 ], [ %559, %816 ], [ %559, %807 ]
  %980 = icmp eq i32 %973, 0
  br i1 %980, label %984, label %558

981:                                              ; preds = %749, %617
  %982 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %983 = icmp eq i32 %982, 0
  br label %1003

984:                                              ; preds = %972, %524
  %985 = phi i32 [ -32000, %524 ], [ %976, %972 ]
  %986 = phi i32 [ 1, %524 ], [ %977, %972 ]
  %987 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %988 = icmp eq i32 %987, 0
  %989 = icmp ne i32 %986, 0
  %990 = select i1 %989, i1 %988, i1 false
  br i1 %990, label %991, label %1003

991:                                              ; preds = %984
  %992 = call noundef i32 @_Z8in_checkP7state_t(ptr noundef %0)
  %993 = icmp eq i32 %992, 0
  %994 = load i32, ptr %11, align 4
  %995 = load i32, ptr %14, align 4
  %996 = load i32, ptr %15, align 4
  br i1 %993, label %1002, label %997

997:                                              ; preds = %991
  %998 = load i32, ptr %21, align 8
  %999 = add nsw i32 %998, -32000
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef %0, i32 noundef %999, i32 noundef %54, i32 noundef %60, i32 noundef 0, i32 noundef %994, i32 noundef %995, i32 noundef %996, i32 noundef %3)
  %1000 = load i32, ptr %21, align 8
  %1001 = add nsw i32 %1000, -32000
  br label %1016

1002:                                             ; preds = %991
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef %0, i32 noundef 0, i32 noundef %54, i32 noundef %60, i32 noundef 0, i32 noundef %994, i32 noundef %995, i32 noundef %996, i32 noundef %3)
  br label %1016

1003:                                             ; preds = %984, %981
  %1004 = phi i1 [ %983, %981 ], [ %988, %984 ]
  %1005 = phi i32 [ %562, %981 ], [ %985, %984 ]
  %1006 = load i32, ptr %36, align 4
  %1007 = icmp sgt i32 %1006, 98
  %1008 = xor i1 %1004, true
  %1009 = select i1 %1007, i1 true, i1 %1008
  %1010 = select i1 %1007, i32 0, i32 %1005
  br i1 %1009, label %1016, label %1011

1011:                                             ; preds = %1003
  %1012 = load i32, ptr %13, align 4
  %1013 = load i32, ptr %11, align 4
  %1014 = load i32, ptr %14, align 4
  %1015 = load i32, ptr %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef nonnull %0, i32 noundef %1005, i32 noundef %54, i32 noundef %60, i32 noundef %1012, i32 noundef %1013, i32 noundef %1014, i32 noundef %1015, i32 noundef %3)
  br label %1016

1016:                                             ; preds = %1011, %1003, %1002, %997, %950, %926, %418, %233, %227, %214, %201, %162, %107, %100, %93, %67, %64, %62, %57, %51, %39, %26, %24
  %1017 = phi i32 [ %25, %24 ], [ %46, %39 ], [ %63, %62 ], [ 0, %26 ], [ %49, %51 ], [ %55, %57 ], [ %65, %64 ], [ %68, %67 ], [ %60, %418 ], [ 0, %162 ], [ %82, %93 ], [ %101, %100 ], [ %82, %107 ], [ %203, %214 ], [ 0, %201 ], [ %1001, %997 ], [ 0, %1002 ], [ %1010, %1003 ], [ %1005, %1011 ], [ 0, %227 ], [ %54, %233 ], [ %929, %950 ], [ 0, %926 ]
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %16) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %15) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %14) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %13) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %12) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %11) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %10) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %9) #28
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %8) #28
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %7) #28
  ret i32 %1017
}

; Function Attrs: mustprogress uwtable
define internal noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef %0, i32 noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) #0 {
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca [240 x i32], align 16
  %11 = alloca [240 x i32], align 16
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %6) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %7) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %8) #28
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %9) #28
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %10) #28
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %11) #28
  %12 = getelementptr %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 22
  %13 = load i64, ptr %12, align 8
  %14 = add i64 %13, 1
  store i64 %14, ptr %12, align 8
  %15 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 23
  %16 = load i64, ptr %15, align 8
  %17 = add i64 %16, 1
  store i64 %17, ptr %15, align 8
  %18 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 14
  %19 = load i32, ptr %18, align 8
  %20 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 24
  %21 = load i32, ptr %20, align 8
  %22 = icmp sgt i32 %19, %21
  br i1 %22, label %23, label %24

23:                                               ; preds = %5
  store i32 %19, ptr %20, align 8
  br label %24

24:                                               ; preds = %23, %5
  %25 = tail call fastcc noundef i32 @_ZL17search_time_checkP7state_t(i64 %14)
  %26 = icmp eq i32 %25, 0
  br i1 %26, label %27, label %372

27:                                               ; preds = %24
  %28 = tail call noundef i32 @_Z7is_drawP11gamestate_tP7state_t(ptr noundef nonnull @gamestate, ptr noundef nonnull %0)
  %29 = icmp eq i32 %28, 0
  br i1 %29, label %30, label %34

30:                                               ; preds = %27
  %31 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 15
  %32 = load i32, ptr %31, align 4
  %33 = icmp sgt i32 %32, 99
  br i1 %33, label %34, label %42

34:                                               ; preds = %30, %27
  %35 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 3), align 4
  %36 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 11
  %37 = load i32, ptr %36, align 4
  %38 = icmp eq i32 %35, %37
  %39 = load i32, ptr @contempt, align 4
  %40 = sub nsw i32 0, %39
  %41 = select i1 %38, i32 %39, i32 %40
  br label %372

42:                                               ; preds = %30
  %43 = call noundef i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(ptr noundef nonnull %0, ptr noundef nonnull %8, i32 noundef %1, i32 noundef %2, ptr noundef nonnull %7, ptr noundef nonnull %9, ptr noundef nonnull %9, ptr noundef nonnull %9, ptr noundef nonnull %9, i32 noundef 0)
  switch i32 %43, label %53 [
    i32 3, label %44
    i32 1, label %46
    i32 2, label %49
    i32 4, label %52
  ]

44:                                               ; preds = %42
  %45 = load i32, ptr %8, align 4
  br label %372

46:                                               ; preds = %42
  %47 = load i32, ptr %8, align 4
  %48 = icmp sgt i32 %47, %1
  br i1 %48, label %53, label %372

49:                                               ; preds = %42
  %50 = load i32, ptr %8, align 4
  %51 = icmp slt i32 %50, %2
  br i1 %51, label %53, label %372

52:                                               ; preds = %42
  store i32 65535, ptr %7, align 4
  br label %53

53:                                               ; preds = %52, %49, %46, %42
  %54 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 5), align 4
  %55 = shl nsw i32 %54, 1
  %56 = icmp slt i32 %55, %4
  br i1 %56, label %60, label %57

57:                                               ; preds = %53
  %58 = load i32, ptr %18, align 8
  %59 = icmp sgt i32 %58, 60
  br i1 %59, label %60, label %62

60:                                               ; preds = %57, %53
  %61 = call noundef i32 @_Z4evalP7state_tiii(ptr noundef nonnull %0, i32 noundef %1, i32 noundef %2, i32 noundef 0)
  br label %372

62:                                               ; preds = %57
  %63 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 25
  %64 = sext i32 %58 to i64
  %65 = getelementptr inbounds [64 x i32], ptr %63, i64 0, i64 %64
  %66 = load i32, ptr %65, align 4
  %67 = call noundef i32 @_Z13retrieve_evalP7state_t(ptr noundef nonnull %0)
  %68 = add nsw i32 %67, 50
  %69 = icmp ne i32 %66, 0
  br i1 %69, label %141, label %70

70:                                               ; preds = %62
  %71 = icmp slt i32 %67, %2
  br i1 %71, label %74, label %72

72:                                               ; preds = %70
  %73 = load i32, ptr %7, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef nonnull %0, i32 noundef %67, i32 noundef %1, i32 noundef %2, i32 noundef %73, i32 noundef 0, i32 noundef 0, i32 noundef 0, i32 noundef 0)
  br label %372

74:                                               ; preds = %70
  %75 = icmp sgt i32 %67, %1
  br i1 %75, label %144, label %76

76:                                               ; preds = %74
  %77 = add nsw i32 %67, 985
  %78 = icmp sgt i32 %77, %1
  br i1 %78, label %81, label %79

79:                                               ; preds = %76
  %80 = load i32, ptr %7, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef nonnull %0, i32 noundef %77, i32 noundef %1, i32 noundef %2, i32 noundef %80, i32 noundef 0, i32 noundef 0, i32 noundef 0, i32 noundef 0)
  br label %372

81:                                               ; preds = %76
  %82 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 8
  %83 = getelementptr inbounds [13 x i32], ptr %82, i64 0, i64 0
  %84 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 11
  %85 = load i32, ptr %84, align 4
  %86 = icmp eq i32 %85, 0
  br i1 %86, label %114, label %87

87:                                               ; preds = %81
  %88 = getelementptr inbounds i32, ptr %83, i64 10
  %89 = load i32, ptr %88, align 4
  %90 = icmp eq i32 %89, 0
  br i1 %90, label %91, label %144

91:                                               ; preds = %87
  %92 = getelementptr inbounds i32, ptr %83, i64 8
  %93 = load i32, ptr %92, align 4
  %94 = icmp eq i32 %93, 0
  br i1 %94, label %95, label %109

95:                                               ; preds = %91
  %96 = getelementptr inbounds i32, ptr %83, i64 12
  %97 = load i32, ptr %96, align 4
  %98 = icmp eq i32 %97, 0
  br i1 %98, label %99, label %106

99:                                               ; preds = %95
  %100 = getelementptr inbounds i32, ptr %83, i64 4
  %101 = load i32, ptr %100, align 4
  %102 = icmp eq i32 %101, 0
  br i1 %102, label %103, label %106

103:                                              ; preds = %99
  %104 = add nsw i32 %67, 135
  %105 = icmp sgt i32 %104, %1
  br i1 %105, label %144, label %372

106:                                              ; preds = %99, %95
  %107 = add nsw i32 %67, 380
  %108 = icmp sgt i32 %107, %1
  br i1 %108, label %144, label %372

109:                                              ; preds = %91
  %110 = add nsw i32 %67, 540
  %111 = icmp sgt i32 %110, %1
  br i1 %111, label %144, label %112

112:                                              ; preds = %109
  %113 = load i32, ptr %7, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef nonnull %0, i32 noundef %110, i32 noundef %1, i32 noundef %2, i32 noundef %113, i32 noundef 0, i32 noundef 0, i32 noundef 0, i32 noundef 0)
  br label %372

114:                                              ; preds = %81
  %115 = getelementptr inbounds i32, ptr %83, i64 9
  %116 = load i32, ptr %115, align 4
  %117 = icmp eq i32 %116, 0
  br i1 %117, label %118, label %144

118:                                              ; preds = %114
  %119 = getelementptr inbounds i32, ptr %83, i64 7
  %120 = load i32, ptr %119, align 4
  %121 = icmp eq i32 %120, 0
  br i1 %121, label %122, label %136

122:                                              ; preds = %118
  %123 = getelementptr inbounds i32, ptr %83, i64 11
  %124 = load i32, ptr %123, align 4
  %125 = icmp eq i32 %124, 0
  br i1 %125, label %126, label %133

126:                                              ; preds = %122
  %127 = getelementptr inbounds i32, ptr %83, i64 3
  %128 = load i32, ptr %127, align 4
  %129 = icmp eq i32 %128, 0
  br i1 %129, label %130, label %133

130:                                              ; preds = %126
  %131 = add nsw i32 %67, 135
  %132 = icmp sgt i32 %131, %1
  br i1 %132, label %144, label %372

133:                                              ; preds = %126, %122
  %134 = add nsw i32 %67, 380
  %135 = icmp sgt i32 %134, %1
  br i1 %135, label %144, label %372

136:                                              ; preds = %118
  %137 = add nsw i32 %67, 540
  %138 = icmp sgt i32 %137, %1
  br i1 %138, label %144, label %139

139:                                              ; preds = %136
  %140 = load i32, ptr %7, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef nonnull %0, i32 noundef %137, i32 noundef %1, i32 noundef %2, i32 noundef %140, i32 noundef 0, i32 noundef 0, i32 noundef 0, i32 noundef 0)
  br label %372

141:                                              ; preds = %62
  %142 = load i32, ptr %7, align 4
  %143 = icmp sgt i32 %3, -6
  br i1 %143, label %149, label %158

144:                                              ; preds = %136, %133, %130, %114, %109, %106, %103, %87, %74
  %145 = phi i32 [ %1, %114 ], [ %1, %136 ], [ %1, %130 ], [ %1, %133 ], [ %1, %87 ], [ %1, %109 ], [ %1, %103 ], [ %1, %106 ], [ %67, %74 ]
  %146 = sub nsw i32 %145, %68
  %147 = load i32, ptr %7, align 4
  %148 = icmp sgt i32 %3, -6
  br i1 %148, label %152, label %155

149:                                              ; preds = %141
  %150 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %151 = call noundef i32 @_Z12gen_evasionsP7state_tPii(ptr noundef nonnull %0, ptr noundef nonnull %150, i32 noundef %66)
  br label %161

152:                                              ; preds = %144
  %153 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %154 = call noundef i32 @_Z12gen_capturesP7state_tPi(ptr noundef nonnull %0, ptr noundef nonnull %153)
  br label %161

155:                                              ; preds = %144
  %156 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %157 = call noundef i32 @_Z12gen_capturesP7state_tPi(ptr noundef nonnull %0, ptr noundef nonnull %156)
  br label %161

158:                                              ; preds = %141
  %159 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %160 = call noundef i32 @_Z12gen_evasionsP7state_tPii(ptr noundef nonnull %0, ptr noundef nonnull %159, i32 noundef %66)
  br label %161

161:                                              ; preds = %158, %155, %152, %149
  %162 = phi i32 [ %147, %155 ], [ %142, %158 ], [ %142, %149 ], [ %147, %152 ]
  %163 = phi i32 [ %146, %155 ], [ 0, %158 ], [ 0, %149 ], [ %146, %152 ]
  %164 = phi i32 [ %145, %155 ], [ %1, %158 ], [ %1, %149 ], [ %145, %152 ]
  %165 = phi i1 [ false, %155 ], [ false, %158 ], [ false, %149 ], [ true, %152 ]
  %166 = phi i32 [ %157, %155 ], [ %160, %158 ], [ %151, %149 ], [ %154, %152 ]
  %167 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %168 = getelementptr inbounds [240 x i32], ptr %11, i64 0, i64 0
  %169 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 1
  %170 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 0
  %171 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 11
  %172 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 16
  %173 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 36
  %174 = getelementptr inbounds %struct._ZTS7state_t.state_t, ptr %0, i64 0, i32 19
  %175 = sub nsw i32 0, %2
  %176 = add nsw i32 %4, 1
  %177 = icmp sgt i32 %3, -1
  br label %178

178:                                              ; preds = %357, %161
  %179 = phi i32 [ %162, %161 ], [ %197, %357 ]
  %180 = phi i32 [ 1, %161 ], [ %198, %357 ]
  %181 = phi i32 [ -32000, %161 ], [ %199, %357 ]
  %182 = phi i1 [ false, %161 ], [ %356, %357 ]
  %183 = phi i1 [ false, %161 ], [ %358, %357 ]
  %184 = phi i1 [ true, %161 ], [ false, %357 ]
  %185 = phi i32 [ %166, %161 ], [ %193, %357 ]
  %186 = phi i32 [ %164, %161 ], [ %200, %357 ]
  br i1 %182, label %187, label %189

187:                                              ; preds = %178
  %188 = call noundef i32 @_Z15gen_good_checksP7state_tPi(ptr noundef %0, ptr noundef nonnull %167)
  br label %192

189:                                              ; preds = %178
  br i1 %183, label %190, label %192

190:                                              ; preds = %189
  %191 = call noundef i32 @_Z9gen_quietP7state_tPi(ptr noundef %0, ptr noundef nonnull %167)
  br label %192

192:                                              ; preds = %190, %189, %187
  %193 = phi i32 [ %188, %187 ], [ %191, %190 ], [ %185, %189 ]
  %194 = load i32, ptr %7, align 4
  call fastcc void @_ZL16fast_order_movesP7state_tPiS1_ij(ptr noundef %0, ptr noundef nonnull %167, ptr noundef nonnull %168, i32 noundef %193, i32 noundef %194)
  store i32 -1, ptr %6, align 4
  %195 = or i1 %182, %183
  br label %196

196:                                              ; preds = %352, %192
  %197 = phi i32 [ %353, %352 ], [ %179, %192 ]
  %198 = phi i32 [ %338, %352 ], [ %180, %192 ]
  %199 = phi i32 [ %339, %352 ], [ %181, %192 ]
  %200 = phi i32 [ %354, %352 ], [ %186, %192 ]
  br i1 %69, label %201, label %206

201:                                              ; preds = %196
  %202 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %6, ptr noundef nonnull %168, ptr noundef nonnull %167, i32 noundef %193)
  %203 = icmp eq i32 %202, 0
  br i1 %203, label %355, label %204

204:                                              ; preds = %201
  %205 = load i32, ptr %6, align 4
  br label %298

206:                                              ; preds = %240, %196
  %207 = call fastcc noundef i32 @_ZL15remove_one_fastPiS_S_i(ptr noundef nonnull %6, ptr noundef nonnull %168, ptr noundef nonnull %167, i32 noundef %193)
  %208 = icmp eq i32 %207, 0
  br i1 %208, label %355, label %209

209:                                              ; preds = %206
  br i1 %184, label %210, label %225

210:                                              ; preds = %209
  %211 = load i32, ptr %6, align 4
  %212 = sext i32 %211 to i64
  %213 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %212
  %214 = load i32, ptr %213, align 4
  %215 = lshr i32 %214, 19
  %216 = and i32 %215, 15
  %217 = zext i32 %216 to i64
  %218 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %217
  %219 = load i32, ptr %218, align 4
  %220 = call i32 @llvm.abs.i32(i32 %219, i1 true)
  %221 = icmp sle i32 %220, %163
  %222 = and i32 %214, 61440
  %223 = icmp eq i32 %222, 0
  %224 = select i1 %221, i1 %223, i1 false
  br i1 %224, label %355, label %225

225:                                              ; preds = %210, %209
  br i1 %195, label %226, label %264

226:                                              ; preds = %225
  %227 = load i32, ptr %6, align 4
  %228 = sext i32 %227 to i64
  %229 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %228
  %230 = load i32, ptr %229, align 4
  %231 = lshr i32 %230, 19
  %232 = and i32 %231, 15
  %233 = icmp eq i32 %232, 13
  br i1 %233, label %241, label %234

234:                                              ; preds = %226
  %235 = zext i32 %232 to i64
  %236 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %235
  %237 = load i32, ptr %236, align 4
  %238 = call i32 @llvm.abs.i32(i32 %237, i1 true)
  %239 = icmp sgt i32 %238, %163
  br i1 %239, label %240, label %241

240:                                              ; preds = %285, %242, %234
  br label %206, !llvm.loop !44

241:                                              ; preds = %234, %226
  br i1 %183, label %242, label %285

242:                                              ; preds = %241
  %243 = lshr i32 %230, 6
  %244 = and i32 %243, 63
  %245 = zext i32 %244 to i64
  %246 = getelementptr inbounds [64 x i32], ptr %169, i64 0, i64 %245
  %247 = load i32, ptr %246, align 4
  %248 = add nsw i32 %247, -1
  %249 = and i32 %230, 63
  %250 = load i32, ptr %170, align 8
  %251 = sext i32 %250 to i64
  %252 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_hit, i64 0, i64 %251
  %253 = sext i32 %248 to i64
  %254 = getelementptr inbounds [12 x [64 x i32]], ptr %252, i64 0, i64 %253
  %255 = zext i32 %249 to i64
  %256 = getelementptr inbounds [64 x i32], ptr %254, i64 0, i64 %255
  %257 = load i32, ptr %256, align 4
  %258 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_tot, i64 0, i64 %251
  %259 = getelementptr inbounds [12 x [64 x i32]], ptr %258, i64 0, i64 %253
  %260 = getelementptr inbounds [64 x i32], ptr %259, i64 0, i64 %255
  %261 = load i32, ptr %260, align 4
  %262 = sub nsw i32 %261, %257
  %263 = icmp slt i32 %257, %262
  br i1 %263, label %240, label %285

264:                                              ; preds = %225
  %265 = load i32, ptr %6, align 4
  %266 = sext i32 %265 to i64
  %267 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %266
  %268 = load i32, ptr %267, align 4
  %269 = lshr i32 %268, 19
  %270 = and i32 %269, 15
  %271 = zext i32 %270 to i64
  %272 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %271
  %273 = load i32, ptr %272, align 4
  %274 = call i32 @llvm.abs.i32(i32 %273, i1 true)
  %275 = lshr i32 %268, 6
  %276 = and i32 %275, 63
  %277 = zext i32 %276 to i64
  %278 = getelementptr inbounds [64 x i32], ptr %169, i64 0, i64 %277
  %279 = load i32, ptr %278, align 4
  %280 = sext i32 %279 to i64
  %281 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %280
  %282 = load i32, ptr %281, align 4
  %283 = call i32 @llvm.abs.i32(i32 %282, i1 true)
  %284 = icmp ult i32 %274, %283
  br i1 %284, label %285, label %298

285:                                              ; preds = %264, %242, %241
  %286 = phi i64 [ %228, %241 ], [ %266, %264 ], [ %228, %242 ]
  %287 = phi i32 [ %227, %241 ], [ %265, %264 ], [ %227, %242 ]
  %288 = load i32, ptr %171, align 4
  %289 = icmp eq i32 %288, 0
  %290 = zext i1 %289 to i32
  %291 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %286
  %292 = load i32, ptr %291, align 4
  %293 = lshr i32 %292, 6
  %294 = and i32 %293, 63
  %295 = and i32 %292, 63
  %296 = call noundef i32 @_Z3seeP7state_tiiii(ptr noundef %0, i32 noundef %290, i32 noundef %294, i32 noundef %295, i32 noundef 0)
  %297 = icmp slt i32 %296, -50
  br i1 %297, label %240, label %298

298:                                              ; preds = %285, %264, %204
  %299 = phi i32 [ %205, %204 ], [ %265, %264 ], [ %287, %285 ]
  %300 = sext i32 %299 to i64
  %301 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %300
  %302 = load i32, ptr %301, align 4
  call void @_Z4makeP7state_ti(ptr noundef %0, i32 noundef %302)
  %303 = load i32, ptr %301, align 4
  %304 = call noundef i32 @_Z11check_legalP7state_ti(ptr noundef %0, i32 noundef %303)
  %305 = icmp eq i32 %304, 0
  br i1 %305, label %337, label %306

306:                                              ; preds = %298
  %307 = load i64, ptr %172, align 8
  %308 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 15), align 4
  %309 = load i32, ptr %18, align 8
  %310 = add i32 %309, -1
  %311 = add i32 %310, %308
  %312 = sext i32 %311 to i64
  %313 = getelementptr inbounds [1000 x i64], ptr %173, i64 0, i64 %312
  store i64 %307, ptr %313, align 8
  %314 = load i32, ptr %301, align 4
  %315 = sext i32 %310 to i64
  %316 = getelementptr inbounds [64 x i32], ptr %174, i64 0, i64 %315
  store i32 %314, ptr %316, align 4
  %317 = call noundef i32 @_Z8in_checkP7state_t(ptr noundef %0)
  %318 = load i32, ptr %18, align 8
  %319 = sext i32 %318 to i64
  %320 = getelementptr inbounds [64 x i32], ptr %63, i64 0, i64 %319
  store i32 %317, ptr %320, align 4
  %321 = sub nsw i32 0, %200
  %322 = sub i32 60, %200
  %323 = icmp ne i32 %317, 0
  %324 = zext i1 %323 to i32
  %325 = call noundef i32 @_Z4evalP7state_tiii(ptr noundef %0, i32 noundef %175, i32 noundef %322, i32 noundef %324)
  br i1 %183, label %326, label %329

326:                                              ; preds = %306
  %327 = sub nsw i32 0, %325
  %328 = icmp slt i32 %200, %327
  br i1 %328, label %333, label %337

329:                                              ; preds = %306
  %330 = select i1 %323, i1 true, i1 %69
  %331 = select i1 %330, i32 -1, i32 -8
  %332 = add nsw i32 %331, %3
  br label %333

333:                                              ; preds = %329, %326
  %334 = phi i32 [ %3, %326 ], [ %332, %329 ]
  %335 = call noundef i32 @_Z7qsearchP7state_tiiii(ptr noundef nonnull %0, i32 noundef %175, i32 noundef %321, i32 noundef %334, i32 noundef %176)
  %336 = sub nsw i32 0, %335
  br label %337

337:                                              ; preds = %333, %326, %298
  %338 = phi i32 [ %198, %298 ], [ 0, %333 ], [ 0, %326 ]
  %339 = phi i32 [ %199, %298 ], [ %336, %333 ], [ %199, %326 ]
  %340 = load i32, ptr %301, align 4
  call void @_Z6unmakeP7state_ti(ptr noundef %0, i32 noundef %340)
  %341 = load i32, ptr getelementptr inbounds (%struct._ZTS11gamestate_t.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %342 = icmp eq i32 %341, 0
  br i1 %342, label %343, label %372

343:                                              ; preds = %337
  %344 = icmp sle i32 %339, %200
  %345 = or i1 %305, %344
  br i1 %345, label %352, label %346

346:                                              ; preds = %343
  %347 = load i32, ptr %301, align 4
  %348 = call noundef zeroext i16 @_Z12compact_movei(i32 noundef %347)
  %349 = zext i16 %348 to i32
  %350 = icmp slt i32 %339, %2
  br i1 %350, label %352, label %351

351:                                              ; preds = %346
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef %0, i32 noundef %339, i32 noundef %1, i32 noundef %2, i32 noundef %349, i32 noundef 0, i32 noundef 0, i32 noundef 0, i32 noundef 0)
  br label %372

352:                                              ; preds = %346, %343
  %353 = phi i32 [ %197, %343 ], [ %349, %346 ]
  %354 = phi i32 [ %200, %343 ], [ %339, %346 ]
  br label %196, !llvm.loop !44

355:                                              ; preds = %210, %206, %201
  %356 = and i1 %165, %184
  br i1 %356, label %357, label %359

357:                                              ; preds = %359, %355
  %358 = xor i1 %356, true
  br label %178

359:                                              ; preds = %355
  %360 = and i1 %165, %182
  %361 = and i1 %360, %177
  %362 = icmp sgt i32 %68, %200
  %363 = select i1 %361, i1 %362, i1 false
  br i1 %363, label %357, label %364

364:                                              ; preds = %359
  %365 = icmp ne i32 %198, 0
  %366 = select i1 %365, i1 %69, i1 false
  br i1 %366, label %367, label %370

367:                                              ; preds = %364
  %368 = load i32, ptr %18, align 8
  %369 = add nsw i32 %368, -32000
  br label %370

370:                                              ; preds = %367, %364
  %371 = phi i32 [ %369, %367 ], [ %200, %364 ]
  call void @_Z7StoreTTP7state_tiiijiiii(ptr noundef %0, i32 noundef %371, i32 noundef %1, i32 noundef %2, i32 noundef %197, i32 noundef 0, i32 noundef 0, i32 noundef 0, i32 noundef 0)
  br label %372

372:                                              ; preds = %370, %351, %337, %139, %133, %130, %112, %106, %103, %79, %72, %60, %49, %46, %44, %34, %24
  %373 = phi i32 [ %41, %34 ], [ %61, %60 ], [ %339, %351 ], [ %371, %370 ], [ %67, %72 ], [ %77, %79 ], [ %45, %44 ], [ 0, %24 ], [ %47, %46 ], [ %50, %49 ], [ %134, %133 ], [ %131, %130 ], [ %107, %106 ], [ %104, %103 ], [ %110, %112 ], [ %137, %139 ], [ 0, %337 ]
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %11) #28
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %10) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %9) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %8) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %7) #28
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %6) #28
  ret i32 %373
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #6

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #6

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #2 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { mustprogress nofree norecurse nosync nounwind uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #5 = { mustprogress nosync nounwind readnone uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #6 = { argmemonly nofree nosync nounwind willreturn }
attributes #7 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
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
!30 = distinct !{!30, !31}
!31 = !{!"llvm.loop.mustprogress"}
!32 = !{!27, !28, i64 0}
!33 = !{!28, !7, i64 0}
!34 = !{!26, !7, i64 0}
!35 = !{!11, !7, i64 4360}
!36 = !{!28, !8, i64 9}
!37 = !{!28, !8, i64 8}
!38 = !{!26, !8, i64 8}
!39 = !{!28, !29, i64 4}
!40 = !{!26, !29, i64 4}
!41 = !{!11, !7, i64 472}
!42 = !{!28, !29, i64 6}
!43 = !{!26, !29, i64 6}
!44 = distinct !{!44, !31}
!45 = distinct !{!45, !31}
!46 = distinct !{!46, !31}
!47 = distinct !{!47, !31}
!48 = distinct !{!48, !31}
!49 = distinct !{!49, !31}
!50 = distinct !{!50, !31, !51}
!51 = !{!"llvm.loop.unswitch.partial.disable"}
!52 = distinct !{!52, !31}
!53 = distinct !{!53, !31}

; end INTEL_FEATURE_SW_ADVANCED