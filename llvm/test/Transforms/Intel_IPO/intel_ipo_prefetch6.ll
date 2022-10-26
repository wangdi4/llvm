; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; Checks that IPO Prefetch pass can properly identify the prefetch opportunity, generate the prefetch
; function, and generate 2 calls to the prefetch function in 2 DL host functions: 1 call to prefetch inside
; each host function.
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
; RUN: opt < %s -passes='module(intel-ipoprefetch)' -ipo-prefetch-be-lit-friendly=1 -ipo-prefetch-suppress-inline-report=0 -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -S 2>&1 | FileCheck %s
;

; *** Check section 1 ***
; The LLVM-IR check below ensures that a call to the Prefetch.Backbone function is inserted inside host
; _Z6searchP7state_tiiiii.
; CHECK: define internal i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5) #{{.}} {
; CHECK: call void @Prefetch.Backbone(%struct.state_t* %0)
;

; *** Check section 2 ***
; The LLVM-IR check below ensures that a call to the Prefetch.Backbone function is inserted inside host
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
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.pawntt_t = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i32, i32 }
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque
%struct.state_t = type { i32, [64 x i32], i64, i64, i64, [13 x i64], i32, i32, [13 x i32], i32, i32, i32, i32, i32, i32, i32, i64, i64, [64 x %struct.move_x], [64 x i32], [64 x i32], [64 x %struct.anon], i64, i64, i32, [64 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i64] }
%struct.move_x = type { i32, i32, i32, i32, i64, i64 }
%struct.anon = type { i32, i32, i32, i32 }
%struct.gamestate_t = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i32], [1000 x %struct.move_x], i64, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.scoreboard_t = type { i32, i32, [8 x %struct.anon.0], [8 x i32], [8 x %struct.state_t] }
%struct.anon.0 = type { i32, i32, i32 }
%struct.ttentry_t = type { [4 x %struct.ttbucket_t] }
%struct.ttbucket_t = type { i32, i16, i16, i8, i8 }

@Mask = internal global [64 x i64] zeroinitializer, align 16
@InvMask = internal global [64 x i64] zeroinitializer, align 16
@FileMask = internal global [8 x i64] zeroinitializer, align 16
@RankMask = internal global [8 x i64] zeroinitializer, align 16
@FileUpMask = internal global [64 x i64] zeroinitializer, align 16
@FileDownMask = internal global [64 x i64] zeroinitializer, align 16
@DiagMaska1h8 = internal global [64 x i64] zeroinitializer, align 16
@DiagMaska8h1 = internal global [64 x i64] zeroinitializer, align 16
@_ZL19DiagonalLength_a1h8 = internal unnamed_addr constant [64 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1], align 16
@_ZL19DiagonalLength_a8h1 = internal unnamed_addr constant [64 x i32] [i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8], align 16
@KingSafetyMask = internal global [64 x i64] zeroinitializer, align 16
@KingSafetyMask1 = internal global [64 x i64] zeroinitializer, align 16
@KnightMoves = internal global [64 x i64] zeroinitializer, align 16
@PawnAttacksBlack = internal global [64 x i64] zeroinitializer, align 16
@PawnAttacksWhite = internal global [64 x i64] zeroinitializer, align 16
@PawnMovesBlack = internal global [64 x i64] zeroinitializer, align 16
@PawnMovesWhite = internal global [64 x i64] zeroinitializer, align 16
@KingMoves = internal global [64 x i64] zeroinitializer, align 16
@firstRankAttacks = internal global [64 x [8 x i8]] zeroinitializer, align 16
@fillUpAttacks = internal global [64 x [8 x i64]] zeroinitializer, align 16
@aFileAttacks = internal global [64 x [8 x i64]] zeroinitializer, align 16
@RookMask = internal global [64 x i64] zeroinitializer, align 16
@BishopMask = internal global [64 x i64] zeroinitializer, align 16
@QueenMask = internal global [64 x i64] zeroinitializer, align 16
@AboveMask = internal global [8 x i64] zeroinitializer, align 16
@BelowMask = internal global [8 x i64] zeroinitializer, align 16
@LeftMask = internal global [8 x i64] zeroinitializer, align 16
@RightMask = internal global [8 x i64] zeroinitializer, align 16
@WhiteSqMask = internal global i64 0, align 8
@BlackSqMask = internal global i64 0, align 8
@WhiteKingSide = internal global i64 0, align 8
@WhiteQueenSide = internal global i64 0, align 8
@BlackKingSide = internal global i64 0, align 8
@BlackQueenSide = internal global i64 0, align 8
@QSMask = internal global i64 0, align 8
@KSMask = internal global i64 0, align 8
@WhiteStrongSquareMask = internal global i64 0, align 8
@BlackStrongSquareMask = internal global i64 0, align 8
@CenterMask = internal global i64 0, align 8
@KingFilesMask = internal global [8 x i64] zeroinitializer, align 16
@KingPressureMask = internal global [64 x i64] zeroinitializer, align 16
@KingPressureMask1 = internal global [64 x i64] zeroinitializer, align 16
@SpaceMask = internal global [2 x i64] zeroinitializer, align 16
@CastleMask = internal global [4 x i64] zeroinitializer, align 16
@last_bit = internal global [65536 x i8] zeroinitializer, align 16
@_ZZ14setup_epd_lineP11gamestate_tP7state_tPKcE11rankoffsets = internal unnamed_addr constant [8 x i32] [i32 0, i32 8, i32 16, i32 24, i32 32, i32 40, i32 48, i32 56], align 16
@.str.3 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.4 = private unnamed_addr constant [20 x i8] c"Workload not found\0A\00", align 1
@.str.5 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.6 = private unnamed_addr constant [23 x i8] c"Analyzing %d plies...\0A\00", align 1
@.str.7 = private unnamed_addr constant [31 x i8] c"\0ANodes: %llu (%0.2f%% qnodes)\0A\00", align 1
@.str.8 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.str = private unnamed_addr constant [10 x i8] c"sjeng.log\00", align 1
@_ZL8w_passer = internal unnamed_addr constant [6 x i32] [i32 185, i32 120, i32 70, i32 40, i32 20, i32 15], align 16
@_ZL23w_passer_pawn_supported = internal unnamed_addr constant [6 x i32] [i32 65, i32 25, i32 8, i32 -3, i32 -5, i32 -5], align 16
@_ZL23w_passer_king_supported = internal unnamed_addr constant [6 x i32] [i32 -25, i32 25, i32 7, i32 5, i32 5, i32 4], align 16
@_ZL13w_passer_free = internal unnamed_addr constant [6 x i32] [i32 185, i32 15, i32 10, i32 8, i32 3, i32 1], align 16
@_ZL18w_passer_very_free = internal unnamed_addr constant [6 x i32] [i32 0, i32 80, i32 30, i32 15, i32 10, i32 10], align 16
@_ZL16w_passer_blocked = internal unnamed_addr constant [6 x i32] [i32 -25, i32 -10, i32 -4, i32 0, i32 0, i32 0], align 16
@material = internal constant [14 x i32] [i32 0, i32 85, i32 -85, i32 305, i32 -305, i32 40000, i32 -40000, i32 490, i32 -490, i32 935, i32 -935, i32 330, i32 -330, i32 0], align 16
@_ZL6PawnTT = internal global [8 x [16384 x %struct.pawntt_t]] zeroinitializer, align 16
@_ZL11w_candidate = internal unnamed_addr constant [6 x i32] [i32 0, i32 44, i32 12, i32 10, i32 3, i32 3], align 16
@_ZL15psq_king_nopawn = internal unnamed_addr constant [64 x i32] [i32 -40, i32 -30, i32 -22, i32 -20, i32 -20, i32 -22, i32 -30, i32 -40, i32 -30, i32 -15, i32 -10, i32 -5, i32 -5, i32 -10, i32 -15, i32 -30, i32 -22, i32 -10, i32 5, i32 10, i32 10, i32 5, i32 -10, i32 -22, i32 -20, i32 -5, i32 10, i32 20, i32 20, i32 10, i32 -5, i32 -20, i32 -20, i32 -5, i32 10, i32 20, i32 20, i32 10, i32 -5, i32 -20, i32 -22, i32 -10, i32 5, i32 10, i32 10, i32 5, i32 -10, i32 -22, i32 -30, i32 -15, i32 -10, i32 -5, i32 -5, i32 -10, i32 -15, i32 -30, i32 -40, i32 -30, i32 -22, i32 -20, i32 -20, i32 -22, i32 -30, i32 -40], align 16
@_ZL18psq_king_queenside = internal unnamed_addr constant [64 x i32] [i32 -40, i32 -30, i32 -20, i32 -20, i32 -30, i32 -40, i32 -55, i32 -75, i32 -25, i32 0, i32 0, i32 -5, i32 -15, i32 -30, i32 -50, i32 -70, i32 -20, i32 10, i32 15, i32 20, i32 -10, i32 -25, i32 -45, i32 -70, i32 -15, i32 15, i32 30, i32 30, i32 -5, i32 -15, i32 -35, i32 -60, i32 -15, i32 10, i32 20, i32 25, i32 -10, i32 -25, i32 -35, i32 -60, i32 -25, i32 5, i32 15, i32 15, i32 -15, i32 -25, i32 -45, i32 -75, i32 -30, i32 -10, i32 -10, i32 -10, i32 -20, i32 -30, i32 -50, i32 -75, i32 -50, i32 -20, i32 -15, i32 -30, i32 -40, i32 -50, i32 -60, i32 -80], align 16
@_ZL17psq_king_kingside = internal unnamed_addr constant [64 x i32] [i32 -75, i32 -55, i32 -40, i32 -30, i32 -20, i32 -20, i32 -30, i32 -40, i32 -70, i32 -50, i32 -30, i32 -15, i32 -5, i32 0, i32 0, i32 -25, i32 -70, i32 -45, i32 -25, i32 -10, i32 20, i32 15, i32 10, i32 -20, i32 -60, i32 -35, i32 -15, i32 -5, i32 30, i32 30, i32 15, i32 -15, i32 -60, i32 -35, i32 -25, i32 -10, i32 25, i32 20, i32 10, i32 -15, i32 -75, i32 -45, i32 -25, i32 -15, i32 15, i32 15, i32 5, i32 -25, i32 -75, i32 -50, i32 -30, i32 -20, i32 -10, i32 -10, i32 -10, i32 -30, i32 -80, i32 -60, i32 -50, i32 -40, i32 -30, i32 -15, i32 -20, i32 -50], align 16
@_ZL13wking_psq_end = internal unnamed_addr constant [64 x i32] [i32 0, i32 30, i32 35, i32 20, i32 20, i32 35, i32 30, i32 0, i32 -4, i32 25, i32 20, i32 24, i32 24, i32 20, i32 25, i32 -4, i32 -4, i32 14, i32 16, i32 6, i32 6, i32 16, i32 14, i32 -4, i32 -20, i32 0, i32 5, i32 -6, i32 -6, i32 5, i32 0, i32 -20, i32 -16, i32 -4, i32 0, i32 2, i32 2, i32 0, i32 -4, i32 -16, i32 -22, i32 -8, i32 -4, i32 0, i32 0, i32 -4, i32 -8, i32 -22, i32 -26, i32 -15, i32 -10, i32 -8, i32 -8, i32 -10, i32 -15, i32 -26, i32 -30, i32 -25, i32 -25, i32 -25, i32 -25, i32 -25, i32 -25, i32 -30], align 16
@flip = internal constant [64 x i32] [i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7], align 16
@psq_table = internal global [12 x [64 x i8]] zeroinitializer, align 16
@_ZL9wpawn_psq = internal unnamed_addr constant <{ [38 x i32], [26 x i32] }> <{ [38 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 -10, i32 0, i32 0, i32 0, i32 0, i32 -10, i32 0, i32 0, i32 -5, i32 8, i32 12, i32 12, i32 8, i32 -5, i32 0, i32 0, i32 0, i32 6, i32 10, i32 10, i32 6, i32 0, i32 0, i32 0, i32 0, i32 4, i32 8, i32 8, i32 4], [26 x i32] zeroinitializer }>, align 16
@_ZL15wknight_psq_end = internal unnamed_addr constant [64 x i32] [i32 -25, i32 -5, i32 0, i32 8, i32 8, i32 0, i32 -5, i32 -25, i32 -16, i32 4, i32 10, i32 16, i32 16, i32 10, i32 4, i32 7, i32 -1, i32 15, i32 20, i32 22, i32 22, i32 20, i32 15, i32 -7, i32 -5, i32 10, i32 16, i32 16, i32 16, i32 16, i32 10, i32 6, i32 -6, i32 5, i32 14, i32 13, i32 13, i32 14, i32 5, i32 -2, i32 -14, i32 -3, i32 4, i32 7, i32 7, i32 4, i32 -3, i32 -14, i32 -20, i32 -12, i32 -4, i32 -5, i32 -5, i32 -4, i32 -12, i32 -20, i32 -25, i32 -24, i32 -16, i32 -14, i32 -14, i32 -16, i32 -24, i32 -25], align 16
@_ZL15wbishop_psq_end = internal unnamed_addr constant [64 x i32] [i32 -8, i32 -10, i32 -6, i32 -1, i32 -1, i32 -6, i32 -10, i32 -8, i32 -8, i32 -1, i32 -1, i32 0, i32 0, i32 -1, i32 -1, i32 -8, i32 -1, i32 5, i32 7, i32 8, i32 8, i32 7, i32 5, i32 -1, i32 -1, i32 4, i32 5, i32 10, i32 10, i32 5, i32 4, i32 -1, i32 2, i32 2, i32 3, i32 9, i32 9, i32 7, i32 3, i32 -5, i32 -2, i32 0, i32 6, i32 4, i32 4, i32 6, i32 0, i32 -2, i32 -5, i32 3, i32 1, i32 2, i32 2, i32 1, i32 3, i32 -5, i32 -10, i32 -6, i32 -8, i32 -8, i32 -8, i32 -8, i32 -6, i32 -10], align 16
@_ZL13wrook_psq_end = internal unnamed_addr constant [64 x i32] [i32 5, i32 5, i32 7, i32 10, i32 10, i32 7, i32 5, i32 5, i32 8, i32 10, i32 14, i32 14, i32 14, i32 14, i32 10, i32 8, i32 1, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 1, i32 -1, i32 4, i32 3, i32 0, i32 0, i32 3, i32 4, i32 -1, i32 -6, i32 -1, i32 -3, i32 -2, i32 -2, i32 -3, i32 -1, i32 -6, i32 -10, i32 -4, i32 -8, i32 -8, i32 -8, i32 -8, i32 -4, i32 -10, i32 -15, i32 -12, i32 -9, i32 -8, i32 -8, i32 -9, i32 -12, i32 -15, i32 -15, i32 -10, i32 -8, i32 -8, i32 -8, i32 -8, i32 -10, i32 -15], align 16
@_ZL14wqueen_psq_end = internal unnamed_addr constant [64 x i32] [i32 5, i32 12, i32 16, i32 16, i32 16, i32 16, i32 12, i32 5, i32 -10, i32 12, i32 20, i32 26, i32 26, i32 20, i32 12, i32 -10, i32 -5, i32 10, i32 15, i32 18, i32 18, i32 15, i32 10, i32 -5, i32 -15, i32 1, i32 10, i32 14, i32 14, i32 10, i32 1, i32 -15, i32 -7, i32 -4, i32 6, i32 9, i32 9, i32 6, i32 -4, i32 -7, i32 -12, i32 -8, i32 -2, i32 -4, i32 -4, i32 -2, i32 -8, i32 -12, i32 -12, i32 -12, i32 -13, i32 -10, i32 -10, i32 -13, i32 -12, i32 -12, i32 -20, i32 -25, i32 -25, i32 -10, i32 -10, i32 -25, i32 -25, i32 -20], align 16
@history_hit = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16
@history_tot = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16
@history_h = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16
@_ZL8rc_index = internal unnamed_addr constant [14 x i32] [i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3, i32 4, i32 4, i32 5, i32 5, i32 2, i32 2, i32 0], align 16
@_ZZ11search_rootP7state_tiiiE5bmove = internal unnamed_addr global i32 0, align 4
@.str.37 = private unnamed_addr constant [20 x i8] c"info refutation %s \00", align 1
@_ZZ11search_rootP7state_tiiiE7changes = internal unnamed_addr global i32 0, align 4
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
@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@stdin = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@contempt = internal global i32 0, align 4
@is_pondering = internal global i32 0, align 4
@allow_pondering = internal global i32 0, align 4
@is_analyzing = internal global i32 0, align 4
@time_check_log = internal global i32 0, align 4
@buffered_command = internal global [20 x [8192 x i8]] zeroinitializer, align 16
@buffered_count = internal global i32 0, align 4
@.str.58 = private unnamed_addr constant [30 x i8] c"Please specify the workfile.\0A\00", align 1
@TTSize = internal global i32 0, align 4
@uci_mode = internal global i32 0, align 4
@uci_chess960_mode = internal global i32 0, align 4
@uci_showcurrline = internal global i32 0, align 4
@uci_showrefutations = internal global i32 0, align 4
@uci_limitstrength = internal global i32 0, align 4
@uci_elo = internal global i32 0, align 4
@uci_multipv = internal global i32 0, align 4
@cfg_logging = internal global i32 0, align 4
@cfg_logfile = internal global [512 x i8] zeroinitializer, align 16
@EGTBHits = internal global i32 0, align 4
@state = internal global %struct.state_t zeroinitializer, align 8
@gamestate = internal global %struct.gamestate_t zeroinitializer, align 8
@scoreboard = internal global %struct.scoreboard_t zeroinitializer, align 8
@TTable = internal global %struct.ttentry_t* null, align 8
@TTAge = internal global i32 0, align 4
@zobrist = internal global [14 x [64 x i64]] zeroinitializer, align 16
@.str.101 = private unnamed_addr constant [38 x i8] c"Out of memory allocating hashtables.\0A\00", align 1
@.str.1.102 = private unnamed_addr constant [50 x i8] c"Allocated %d hash entries, totalling %llu bytes.\0A\00", align 1
@.str.117 = private unnamed_addr constant [5 x i8] c"%c%d\00", align 1
@__const._Z11comp_to_sanP7state_tiPc.type_to_char = private unnamed_addr constant [14 x i8] c"FPPNNKKRRQQBBE", align 1
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
@.str.15 = private unnamed_addr constant [10 x i8] c"%c%d%c%dn\00", align 1
@.str.16 = private unnamed_addr constant [10 x i8] c"%c%d%c%dr\00", align 1
@.str.17 = private unnamed_addr constant [10 x i8] c"%c%d%c%db\00", align 1
@.str.18 = private unnamed_addr constant [10 x i8] c"%c%d%c%dq\00", align 1
@.str.34 = private unnamed_addr constant [6 x i8] c"  %s\0A\00", align 1
@.str.19 = private unnamed_addr constant [42 x i8] c"+----+----+----+----+----+----+----+----+\00", align 1
@.str.35 = private unnamed_addr constant [5 x i8] c"%d |\00", align 1
@__const._Z13display_boardP7state_ti.piece_rep = private unnamed_addr constant [14 x i8*] [i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.20, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.21, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.22, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.23, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.24, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.25, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.26, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.27, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.28, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.29, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.30, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.31, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.32, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.33, i32 0, i32 0)], align 16
@.str.36 = private unnamed_addr constant [6 x i8] c" %s |\00", align 1
@.str.37.133 = private unnamed_addr constant [7 x i8] c"\0A  %s\0A\00", align 1
@.str.38 = private unnamed_addr constant [45 x i8] c"\0A     a    b    c    d    e    f    g    h\0A\0A\00", align 1
@.str.39 = private unnamed_addr constant [45 x i8] c"\0A     h    g    f    e    d    c    b    a\0A\0A\00", align 1
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
@.str.55 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@_ZZ9init_gameP11gamestate_tP7state_tE10init_board = internal unnamed_addr constant [64 x i32] [i32 8, i32 4, i32 12, i32 10, i32 6, i32 12, i32 4, i32 8, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 7, i32 3, i32 11, i32 9, i32 5, i32 11, i32 3, i32 7], align 16
@_ZZL15hash_extract_pvP7state_tiPcE10levelstack = internal unnamed_addr global [65 x i64] zeroinitializer, align 16
@.str.42 = private unnamed_addr constant [20 x i8] c"%2d %7d %5d %8llu  \00", align 1
@.str.43 = private unnamed_addr constant [36 x i8] c"info currmove %s currmovenumber %d\0A\00", align 1
@.str.44 = private unnamed_addr constant [81 x i8] c"info depth %d seldepth %d time %d nodes %llu tbhits %d score cp %d multipv 1 pv \00", align 1
@.str.45 = private unnamed_addr constant [83 x i8] c"info depth %d seldepth %d time %d nodes %llu tbhits %d score mate %d multipv 1 pv \00", align 1
@.str.46 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@.str.41 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.str.47 = private unnamed_addr constant [92 x i8] c"info depth %d seldepth %d time %d nodes %llu tbhits %d score cp %d lowerbound multipv 1 pv \00", align 1
@.str.48 = private unnamed_addr constant [6 x i8] c"%s !!\00", align 1
@.str.49 = private unnamed_addr constant [35 x i8] c"info currmove %s currmovenumber %d\00", align 1
@.str.50 = private unnamed_addr constant [92 x i8] c"info depth %d seldepth %d time %d nodes %llu tbhits %d score cp %d upperbound multipv 1 pv \00", align 1
@.str.51 = private unnamed_addr constant [6 x i8] c"%s ??\00", align 1
@multipv_strings = internal global [240 x [512 x i8]] zeroinitializer, align 16
@multipv_scores = internal global [240 x i32] zeroinitializer, align 16
@.str.52 = private unnamed_addr constant [66 x i8] c"depth %d seldepth %d time %d nodes %llu tbhits %d score cp %d pv \00", align 1
@.str.53 = private unnamed_addr constant [68 x i8] c"depth %d seldepth %d time %d nodes %llu tbhits %d score mate %d pv \00", align 1
@.str.54 = private unnamed_addr constant [17 x i8] c"info multipv %d \00", align 1
@.str.58.158 = private unnamed_addr constant [74 x i8] c"Deep Sjeng version 3.2 SPEC, Copyright (C) 2000-2009 Gian-Carlo Pascutto\0A\00", align 1
@_ZL2s1 = internal unnamed_addr global i32 0, align 4
@_ZL2s2 = internal unnamed_addr global i32 0, align 4
@_ZL2s3 = internal unnamed_addr global i32 0, align 4
@root_scores = internal global [240 x i32] zeroinitializer, align 16

; Function Attrs: uwtable mustprogress
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #0

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
declare i32 @_Z11check_legalP7state_ti(%struct.state_t*, i32) #1

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
declare i32 @_Z11is_attackedP7state_tii(%struct.state_t* nocapture readonly, i32, i32) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @_Z11FileAttacksyj(i64, i32) #2

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @_Z11RankAttacksyj(i64, i32) #2

; Function Attrs: nofree norecurse nosync nounwind uwtable willreturn mustprogress
declare zeroext i16 @_Z12compact_movei(i32) #3

; Function Attrs: nofree norecurse nosync nounwind uwtable willreturn mustprogress
declare i32 @_Z4logLi(i32) #3

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
declare i32 @_Z12gen_capturesP7state_tPi(%struct.state_t*, i32*) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @_Z11RookAttacksP7state_ti(%struct.state_t* nocapture readonly, i32) #2

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
declare i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t*, i32*, i32) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @_Z13BishopAttacksP7state_ti(%struct.state_t* nocapture readonly, i32) #2

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nocapture readonly) #2

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
declare i32 @_Z15gen_good_checksP7state_tPi(%struct.state_t*, i32*) #1

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
declare i32 @_Z3genP7state_tPi(%struct.state_t*, i32*) #1

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
declare i32 @_Z3seeP7state_tiiii(%struct.state_t*, i32, i32, i32, i32) #1

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
declare i32 @_Z4evalP7state_tiii(%struct.state_t*, i32, i32, i32) #1

; Function Attrs: norecurse nosync nounwind readonly uwtable willreturn mustprogress
declare void @_Z4makeP7state_ti(%struct.state_t*, i32) #4

; Function Attrs: norecurse nosync nounwind readonly uwtable willreturn mustprogress
declare void @_Z6unmakeP7state_ti(%struct.state_t*, i32) #4

; Function Attrs: norecurse nosync nounwind readonly uwtable willreturn mustprogress
declare void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nocapture, i32, i32, i32, i32, i32, i32, i32, i32) #4

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nocapture readonly, %struct.state_t* nocapture readonly) #2

declare i32 @_Z8in_checkP7state_t(%struct.state_t*)

declare i32 @_Z9gen_quietP7state_tPi(%struct.state_t*, i32* nonnull)

declare void @_ZL10PrefetchL3P7state_t(i32, i64)

declare void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t*, i32* nonnull, i32* nonnull, i32, i32)

declare void @_ZL12history_goodP7state_tii(%struct.state_t* nonnull, i32, i32)

declare i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull, i32* nonnull, i32* nonnull, i32)

declare void @_ZL16fast_order_movesP7state_tPiS1_ij(%struct.state_t*, i32* nonnull, i32* nonnull, i32, i32)

declare i32 @_ZL17search_time_checkP7state_t(i64)

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #5

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #5

; Function Attrs: nosync nounwind readnone uwtable willreturn mustprogress
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #6

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.abs.i32(i32, i1 immarg) #2

; Function Attrs: nofree norecurse nosync nounwind uwtable willreturn mustprogress
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %puts = tail call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([20 x i8], [20 x i8]* @.str.4, i64 0, i64 0))
  ret i32 0
}

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
define internal i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nocapture %0, i32* nocapture %1, i32 %2, i32 %3, i32* nocapture %4, i32* nocapture %5, i32* nocapture %6, i32* nocapture %7, i32* nocapture %8, i32 %9) #1 {
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
  br i1 %32, label %132, label %33, !llvm.loop !30

33:                                               ; preds = %30, %10
  %34 = phi i64 [ 0, %10 ], [ %31, %30 ]
  %35 = getelementptr inbounds [4 x %struct.ttbucket_t], [4 x %struct.ttbucket_t]* %28, i64 0, i64 %34, !intel-tbaa !32
  %36 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 0, !intel-tbaa !33
  %37 = load i32, i32* %36, align 4, !tbaa !34
  %38 = icmp eq i32 %37, %29
  br i1 %38, label %39, label %30

39:                                               ; preds = %33
  %40 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 27, !intel-tbaa !35
  %41 = load i32, i32* %40, align 8, !tbaa !35
  %42 = add i32 %41, 1
  store i32 %42, i32* %40, align 8, !tbaa !35
  %43 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 4
  %44 = load i8, i8* %43, align 1, !tbaa !36
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
  store i8 %55, i8* %43, align 1, !tbaa !36
  br label %56

56:                                               ; preds = %50, %39
  %57 = phi i8 [ %55, %50 ], [ %44, %39 ]
  %58 = and i8 %57, 6
  %59 = icmp eq i8 %58, 2
  br i1 %59, label %60, label %72

60:                                               ; preds = %56
  %61 = add nsw i32 %9, -16
  %62 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 3, !intel-tbaa !37
  %63 = load i8, i8* %62, align 4, !tbaa !38
  %64 = zext i8 %63 to i32
  %65 = icmp sgt i32 %61, %64
  br i1 %65, label %72, label %66

66:                                               ; preds = %60
  %67 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !intel-tbaa !39
  %68 = load i16, i16* %67, align 4, !tbaa !40
  %69 = sext i16 %68 to i32
  %70 = icmp slt i32 %69, %3
  br i1 %70, label %71, label %72

71:                                               ; preds = %66
  store i32 0, i32* %6, align 4, !tbaa !6
  br label %72

72:                                               ; preds = %71, %66, %60, %56
  %73 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 3, !intel-tbaa !37
  %74 = load i8, i8* %73, align 4, !tbaa !38
  %75 = zext i8 %74 to i32
  %76 = icmp slt i32 %75, %9
  br i1 %76, label %109, label %77

77:                                               ; preds = %72
  %78 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !intel-tbaa !39
  %79 = load i16, i16* %78, align 4, !tbaa !40
  %80 = sext i16 %79 to i32
  store i32 %80, i32* %1, align 4, !tbaa !6
  %81 = icmp sgt i16 %79, 31500
  br i1 %81, label %82, label %87

82:                                               ; preds = %77
  %83 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !41
  %84 = load i32, i32* %83, align 8, !tbaa !41
  %85 = add nsw i32 %80, 1
  %86 = sub i32 %85, %84
  store i32 %86, i32* %1, align 4, !tbaa !6
  br label %94

87:                                               ; preds = %77
  %88 = icmp slt i16 %79, -31500
  br i1 %88, label %89, label %94

89:                                               ; preds = %87
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !intel-tbaa !41
  %91 = load i32, i32* %90, align 8, !tbaa !41
  %92 = add nsw i32 %80, -1
  %93 = add i32 %92, %91
  store i32 %93, i32* %1, align 4, !tbaa !6
  br label %94

94:                                               ; preds = %89, %87, %82
  %95 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 2, !intel-tbaa !42
  %96 = load i16, i16* %95, align 2, !tbaa !43
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
  %110 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 2, !intel-tbaa !42
  %111 = load i16, i16* %110, align 2, !tbaa !43
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
  %126 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !intel-tbaa !39
  %127 = load i16, i16* %126, align 4, !tbaa !40
  %128 = sext i16 %127 to i32
  store i32 %128, i32* %1, align 4, !tbaa !6
  br label %129

129:                                              ; preds = %125, %124, %123
  %130 = trunc i32 %9 to i8
  store i8 %130, i8* %73, align 4, !tbaa !38
  %131 = and i8 %57, -7
  store i8 %131, i8* %43, align 1, !tbaa !36
  br label %132

132:                                              ; preds = %129, %94, %30
  %133 = phi i32 [ %108, %94 ], [ 0, %129 ], [ 4, %30 ]
  ret i32 %133
}

; Function Attrs: uwtable mustprogress
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
  %31 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14
  %32 = load i32, i32* %31, align 8
  %33 = icmp sgt i32 %32, 59
  br i1 %33, label %34, label %36

34:                                               ; preds = %30, %6
  %35 = tail call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 0, i32 0)
  br label %1052

36:                                               ; preds = %30
  %37 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 22
  %38 = load i64, i64* %37, align 8
  %39 = add i64 %38, 1
  store i64 %39, i64* %37, align 8
  %40 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %39)
  %41 = icmp eq i32 %40, 0
  br i1 %41, label %42, label %1052

42:                                               ; preds = %36
  %43 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nonnull @gamestate, %struct.state_t* nonnull %0)
  %44 = icmp eq i32 %43, 0
  br i1 %44, label %45, label %49

45:                                               ; preds = %42
  %46 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15
  %47 = load i32, i32* %46, align 4
  %48 = icmp sgt i32 %47, 99
  br i1 %48, label %49, label %57

49:                                               ; preds = %45, %42
  %50 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4
  %51 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %52 = load i32, i32* %51, align 4
  %53 = icmp eq i32 %50, %52
  %54 = load i32, i32* @contempt, align 4
  %55 = sub nsw i32 0, %54
  %56 = select i1 %53, i32 %54, i32 %55
  br label %1052

57:                                               ; preds = %45
  %58 = load i32, i32* %31, align 8
  %59 = add nsw i32 %58, -32000
  %60 = icmp sgt i32 %59, %1
  br i1 %60, label %61, label %63

61:                                               ; preds = %57
  %62 = icmp slt i32 %59, %2
  br i1 %62, label %63, label %1052

63:                                               ; preds = %61, %57
  %64 = phi i32 [ %59, %61 ], [ %1, %57 ]
  %65 = sub i32 31999, %58
  %66 = icmp slt i32 %65, %2
  br i1 %66, label %67, label %69

67:                                               ; preds = %63
  %68 = icmp sgt i32 %65, %64
  br i1 %68, label %69, label %1052

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
  %73 = load i32, i32* %10, align 4
  br label %1052

74:                                               ; preds = %69
  %75 = load i32, i32* %10, align 4
  %76 = icmp sgt i32 %75, %64
  br i1 %76, label %85, label %1052

77:                                               ; preds = %69
  %78 = load i32, i32* %10, align 4
  %79 = icmp slt i32 %78, %70
  br i1 %79, label %85, label %1052

80:                                               ; preds = %69
  %81 = load i32, i32* %10, align 4
  %82 = icmp slt i32 %81, %70
  %83 = select i1 %82, i32 %5, i32 1
  br label %85

84:                                               ; preds = %69
  store i32 65535, i32* %13, align 4
  store i32 0, i32* %11, align 4
  store i32 0, i32* %14, align 4
  store i32 0, i32* %15, align 4
  br label %85

85:                                               ; preds = %84, %80, %77, %74, %69
  %86 = phi i32 [ %5, %69 ], [ %5, %84 ], [ 0, %74 ], [ 1, %77 ], [ %83, %80 ]
  %87 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25
  %88 = load i32, i32* %31, align 8
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %89
  %91 = load i32, i32* %90, align 4
  %92 = call i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nonnull %0)
  %93 = icmp ne i32 %91, 0
  %94 = xor i1 %93, true
  %95 = add nsw i32 %64, 1
  %96 = icmp eq i32 %70, %95
  %97 = select i1 %94, i1 %96, i1 false
  br i1 %97, label %98, label %122

98:                                               ; preds = %85
  %99 = icmp ult i32 %3, 5
  br i1 %99, label %100, label %112

100:                                              ; preds = %98
  %101 = add nsw i32 %92, -75
  %102 = icmp slt i32 %101, %70
  br i1 %102, label %108, label %103

103:                                              ; preds = %100
  %104 = load i32, i32* %13, align 4
  %105 = load i32, i32* %11, align 4
  %106 = load i32, i32* %14, align 4
  %107 = load i32, i32* %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %101, i32 %64, i32 %70, i32 %104, i32 %105, i32 %106, i32 %107, i32 %3)
  br label %1052

108:                                              ; preds = %100
  %109 = icmp slt i32 %92, %70
  br i1 %109, label %110, label %122

110:                                              ; preds = %108
  %111 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %64, i32 %70, i32 0, i32 0)
  br label %1052

112:                                              ; preds = %98
  %113 = icmp ult i32 %3, 9
  br i1 %113, label %114, label %122

114:                                              ; preds = %112
  %115 = add nsw i32 %92, -125
  %116 = icmp slt i32 %115, %70
  br i1 %116, label %122, label %117

117:                                              ; preds = %114
  %118 = load i32, i32* %13, align 4
  %119 = load i32, i32* %11, align 4
  %120 = load i32, i32* %14, align 4
  %121 = load i32, i32* %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %115, i32 %64, i32 %70, i32 %118, i32 %119, i32 %120, i32 %121, i32 %3)
  br label %1052

122:                                              ; preds = %114, %112, %108, %85
  %123 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8
  %124 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 9
  %125 = load i32, i32* %124, align 4
  %126 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 7
  %127 = load i32, i32* %126, align 4
  %128 = add nsw i32 %127, %125
  %129 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 11
  %130 = load i32, i32* %129, align 4
  %131 = add nsw i32 %128, %130
  %132 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 3
  %133 = load i32, i32* %132, align 4
  %134 = add nsw i32 %131, %133
  %135 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 10
  %136 = load i32, i32* %135, align 8
  %137 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 8
  %138 = load i32, i32* %137, align 8
  %139 = add nsw i32 %138, %136
  %140 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 12
  %141 = load i32, i32* %140, align 8
  %142 = add nsw i32 %139, %141
  %143 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 4
  %144 = load i32, i32* %143, align 8
  %145 = add nsw i32 %142, %144
  store i32 0, i32* %11, align 4
  %146 = icmp eq i32 %4, 0
  br i1 %146, label %147, label %231

147:                                              ; preds = %122
  %148 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %149 = load i32, i32* %148, align 4
  %150 = icmp eq i32 %149, 0
  %151 = select i1 %150, i32 %145, i32 %134
  %152 = icmp eq i32 %151, 0
  %153 = select i1 %152, i1 true, i1 %93
  %154 = xor i1 %153, true
  %155 = load i32, i32* %12, align 4
  %156 = icmp ne i32 %155, 0
  %157 = select i1 %154, i1 %156, i1 false
  %158 = icmp ugt i32 %3, 4
  %159 = and i1 %96, %158
  %160 = select i1 %157, i1 %159, i1 false
  br i1 %160, label %161, label %231

161:                                              ; preds = %147
  %162 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4
  %163 = icmp eq i32 %162, 2
  br i1 %163, label %164, label %183

164:                                              ; preds = %161
  %165 = icmp ult i32 %3, 25
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
  %174 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %175 = icmp eq i32 %174, 0
  br i1 %175, label %176, label %1052

176:                                              ; preds = %172
  %177 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4
  %178 = icmp eq i32 %177, 2
  %179 = icmp slt i32 %173, %70
  %180 = and i1 %178, %179
  br i1 %180, label %248, label %181

181:                                              ; preds = %176
  %182 = load i32, i32* %148, align 4
  br label %183

183:                                              ; preds = %181, %161
  %184 = phi i32 [ %182, %181 ], [ %149, %161 ]
  %185 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 10
  %186 = load i32, i32* %185, align 8
  store i32 0, i32* %185, align 8
  %187 = xor i32 %184, 1
  store i32 %187, i32* %148, align 4
  %188 = load i32, i32* %31, align 8
  %189 = add nsw i32 %188, 1
  store i32 %189, i32* %31, align 8
  %190 = load i32, i32* %46, align 4
  %191 = add nsw i32 %190, 1
  store i32 %191, i32* %46, align 4
  %192 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %193 = sext i32 %188 to i64
  %194 = getelementptr inbounds [64 x i32], [64 x i32]* %192, i64 0, i64 %193
  store i32 0, i32* %194, align 4
  %195 = sext i32 %189 to i64
  %196 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %195
  store i32 0, i32* %196, align 4
  %197 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 20
  %198 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %193
  %199 = load i32, i32* %198, align 4
  %200 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %195
  store i32 %199, i32* %200, align 4
  %201 = icmp ult i32 %3, 17
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
  %214 = load i32, i32* %46, align 4
  %215 = add nsw i32 %214, -1
  store i32 %215, i32* %46, align 4
  %216 = load i32, i32* %31, align 8
  %217 = add nsw i32 %216, -1
  store i32 %217, i32* %31, align 8
  %218 = load i32, i32* %148, align 4
  %219 = xor i32 %218, 1
  store i32 %219, i32* %148, align 4
  store i32 %186, i32* %185, align 8
  %220 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %221 = icmp eq i32 %220, 0
  br i1 %221, label %222, label %1052

222:                                              ; preds = %211
  %223 = icmp sgt i32 %70, %213
  br i1 %223, label %228, label %224

224:                                              ; preds = %222
  %225 = load i32, i32* %13, align 4
  %226 = load i32, i32* %11, align 4
  %227 = load i32, i32* %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %213, i32 %64, i32 %70, i32 %225, i32 %226, i32 0, i32 %227, i32 %3)
  br label %1052

228:                                              ; preds = %222
  %229 = icmp sgt i32 %212, 31400
  br i1 %229, label %230, label %248

230:                                              ; preds = %228
  store i32 1, i32* %11, align 4
  br label %248

231:                                              ; preds = %147, %122
  %232 = icmp ult i32 %3, 13
  %233 = and i1 %96, %232
  %234 = add nsw i32 %70, -300
  %235 = icmp slt i32 %92, %234
  %236 = select i1 %233, i1 %235, i1 false
  br i1 %236, label %237, label %248

237:                                              ; preds = %231
  %238 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %64, i32 %70, i32 0, i32 0)
  %239 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %240 = icmp eq i32 %239, 0
  br i1 %240, label %241, label %1052

241:                                              ; preds = %237
  %242 = icmp sgt i32 %238, %64
  br i1 %242, label %248, label %243

243:                                              ; preds = %241
  %244 = load i32, i32* %13, align 4
  %245 = load i32, i32* %11, align 4
  %246 = load i32, i32* %14, align 4
  %247 = load i32, i32* %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %64, i32 %64, i32 %70, i32 %244, i32 %245, i32 %246, i32 %247, i32 %3)
  br label %1052

248:                                              ; preds = %241, %231, %230, %228, %176
  br i1 %93, label %249, label %272

249:                                              ; preds = %248
  %250 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 0
  %251 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %250, i32 %91)
  %252 = icmp eq i32 %251, 0
  br i1 %252, label %275, label %253

253:                                              ; preds = %249
  %254 = icmp sgt i32 %251, 0
  br i1 %254, label %255, label %275

255:                                              ; preds = %253
  %256 = zext i32 %251 to i64
  br label %257

257:                                              ; preds = %257, %255
  %258 = phi i64 [ 0, %255 ], [ %268, %257 ]
  %259 = phi i32 [ 0, %255 ], [ %266, %257 ]
  %260 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %258
  %261 = load i32, i32* %260, align 4
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %261)
  %262 = load i32, i32* %260, align 4
  %263 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %262)
  %264 = icmp ne i32 %263, 0
  %265 = zext i1 %264 to i32
  %266 = add nuw nsw i32 %259, %265
  %267 = load i32, i32* %260, align 4
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %267)
  %268 = add nuw nsw i64 %258, 1
  %269 = icmp ult i64 %268, %256
  %270 = icmp ult i32 %266, 2
  %271 = select i1 %269, i1 %270, i1 false
  br i1 %271, label %257, label %275, !llvm.loop !44

272:                                              ; preds = %248
  %273 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 0
  %274 = call i32 @_Z3genP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %273)
  br label %275

275:                                              ; preds = %272, %257, %253, %249
  %276 = phi i32 [ 0, %249 ], [ %274, %272 ], [ %251, %253 ], [ %251, %257 ]
  %277 = phi i32 [ 0, %249 ], [ %274, %272 ], [ 0, %253 ], [ %266, %257 ]
  %278 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 0
  %279 = getelementptr inbounds [240 x i32], [240 x i32]* %8, i64 0, i64 0
  %280 = load i32, i32* %13, align 4
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %278, i32* nonnull %279, i32 %276, i32 %280)
  %281 = icmp sgt i32 %3, 19
  br i1 %281, label %282, label %330

282:                                              ; preds = %275
  %283 = icmp ne i32 %70, %95
  %284 = load i32, i32* %13, align 4
  %285 = icmp eq i32 %284, 65535
  %286 = select i1 %283, i1 %285, i1 false
  br i1 %286, label %287, label %330

287:                                              ; preds = %282
  %288 = icmp sgt i32 %276, 0
  br i1 %288, label %289, label %318

289:                                              ; preds = %287
  %290 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %291 = zext i32 %276 to i64
  br label %292

292:                                              ; preds = %312, %289
  %293 = phi i64 [ 0, %289 ], [ %314, %312 ]
  %294 = phi i32 [ 0, %289 ], [ %313, %312 ]
  %295 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %293
  %296 = load i32, i32* %295, align 4
  %297 = lshr i32 %296, 19
  %298 = and i32 %297, 15
  %299 = icmp eq i32 %298, 13
  br i1 %299, label %312, label %300

300:                                              ; preds = %292
  %301 = lshr i32 %296, 6
  %302 = and i32 %301, 63
  %303 = zext i32 %302 to i64
  %304 = getelementptr inbounds [64 x i32], [64 x i32]* %290, i64 0, i64 %303
  %305 = load i32, i32* %304, align 4
  %306 = sext i32 %305 to i64
  %307 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %306
  %308 = load i32, i32* %307, align 4
  %309 = call i32 @llvm.abs.i32(i32 %308, i1 true)
  %310 = icmp ugt i32 %298, %309
  %311 = select i1 %310, i32 1, i32 %294
  br label %312

312:                                              ; preds = %300, %292
  %313 = phi i32 [ %294, %292 ], [ %311, %300 ]
  %314 = add nuw nsw i64 %293, 1
  %315 = icmp eq i64 %314, %291
  br i1 %315, label %316, label %292, !llvm\.loop !45

316:                                              ; preds = %312
  %317 = icmp eq i32 %313, 0
  br i1 %317, label %318, label %330

318:                                              ; preds = %316, %287
  %319 = bitcast i32* %17 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %319) #7
  %320 = bitcast i32* %18 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %320) #7
  %321 = ashr i32 %3, 1
  %322 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %321, i32 0, i32 %86)
  %323 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* %0, i32* nonnull %17, i32 0, i32 0, i32* nonnull %18, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32 0)
  %324 = icmp eq i32 %323, 4
  br i1 %324, label %327, label %325

325:                                              ; preds = %318
  %326 = load i32, i32* %18, align 4
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %278, i32* nonnull %279, i32 %276, i32 %326)
  br label %329

327:                                              ; preds = %318
  %328 = load i32, i32* %13, align 4
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %278, i32* nonnull %279, i32 %276, i32 %328)
  br label %329

329:                                              ; preds = %327, %325
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %320) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %319) #7
  br label %330

330:                                              ; preds = %329, %316, %282, %275
  %331 = load i32, i32* %11, align 4
  %332 = icmp eq i32 %331, 0
  %333 = select i1 %94, i1 %332, i1 false
  %334 = icmp sgt i32 %3, 15
  %335 = and i1 %333, %334
  %336 = icmp sgt i32 %277, 8
  %337 = select i1 %335, i1 %336, i1 false
  br i1 %337, label %338, label %478

338:                                              ; preds = %330
  %339 = load i32, i32* %31, align 8
  %340 = add nsw i32 %339, -1
  %341 = sext i32 %340 to i64
  %342 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %341
  %343 = load i32, i32* %342, align 4
  %344 = icmp eq i32 %343, 0
  br i1 %344, label %345, label %478

345:                                              ; preds = %338
  %346 = icmp slt i32 %339, 3
  br i1 %346, label %361, label %347

347:                                              ; preds = %345
  %348 = add nsw i32 %339, -2
  %349 = zext i32 %348 to i64
  %350 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %349
  %351 = load i32, i32* %350, align 4
  %352 = icmp eq i32 %351, 0
  br i1 %352, label %353, label %478

353:                                              ; preds = %347
  %354 = icmp ult i32 %339, 4
  br i1 %354, label %361, label %355

355:                                              ; preds = %353
  %356 = add nsw i32 %339, -3
  %357 = zext i32 %356 to i64
  %358 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %357
  %359 = load i32, i32* %358, align 4
  %360 = icmp eq i32 %359, 0
  br i1 %360, label %361, label %478

361:                                              ; preds = %355, %353, %345
  store i32 -1, i32* %9, align 4
  %362 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  %363 = icmp eq i32 %362, 0
  br i1 %363, label %478, label %364

364:                                              ; preds = %361
  %365 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %366 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %367 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %368 = sub nsw i32 0, %70
  %369 = sub i32 50, %64
  %370 = icmp ugt i32 %3, 16
  %371 = icmp ult i32 %3, 17
  %372 = sub i32 1, %70
  %373 = add nsw i32 %3, -16
  %374 = icmp eq i32 %86, 0
  %375 = zext i1 %374 to i32
  br i1 %371, label %376, label %425

376:                                              ; preds = %419, %364
  %377 = phi i32 [ %420, %419 ], [ 0, %364 ]
  %378 = phi i32 [ %380, %419 ], [ 0, %364 ]
  %379 = phi i32 [ %410, %419 ], [ -32000, %364 ]
  %380 = add nuw nsw i32 %378, 1
  %381 = load i32, i32* %9, align 4
  %382 = sext i32 %381 to i64
  %383 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %382
  %384 = load i32, i32* %383, align 4
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %384)
  %385 = load i32, i32* %383, align 4
  %386 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %385)
  %387 = icmp eq i32 %386, 0
  br i1 %387, label %409, label %388

388:                                              ; preds = %376
  %389 = load i64, i64* %365, align 8
  %390 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4
  %391 = load i32, i32* %31, align 8
  %392 = add i32 %391, -1
  %393 = add i32 %392, %390
  %394 = sext i32 %393 to i64
  %395 = getelementptr inbounds [1000 x i64], [1000 x i64]* %366, i64 0, i64 %394
  store i64 %389, i64* %395, align 8
  %396 = load i32, i32* %383, align 4
  %397 = sext i32 %392 to i64
  %398 = getelementptr inbounds [64 x i32], [64 x i32]* %367, i64 0, i64 %397
  store i32 %396, i32* %398, align 4
  %399 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %400 = load i32, i32* %31, align 8
  %401 = sext i32 %400 to i64
  %402 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %401
  store i32 %399, i32* %402, align 4
  %403 = icmp ne i32 %399, 0
  %404 = or i1 %370, %403
  %405 = zext i1 %404 to i32
  %406 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %368, i32 %369, i32 %405)
  %407 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %368, i32 %372, i32 0, i32 0)
  %408 = sub nsw i32 0, %407
  br label %409

409:                                              ; preds = %388, %376
  %410 = phi i32 [ %408, %388 ], [ %379, %376 ]
  %411 = load i32, i32* %383, align 4
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %411)
  %412 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %413 = icmp eq i32 %412, 0
  br i1 %413, label %414, label %478

414:                                              ; preds = %409
  %415 = icmp slt i32 %410, %70
  %416 = or i1 %387, %415
  br i1 %416, label %419, label %417

417:                                              ; preds = %414
  %418 = icmp sgt i32 %377, 0
  br i1 %418, label %474, label %419

419:                                              ; preds = %417, %414
  %420 = phi i32 [ 1, %417 ], [ %377, %414 ]
  %421 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  %422 = icmp ne i32 %421, 0
  %423 = icmp ult i32 %378, 2
  %424 = select i1 %422, i1 %423, i1 false
  br i1 %424, label %376, label %478, !llvm\.loop !46

425:                                              ; preds = %468, %364
  %426 = phi i32 [ %469, %468 ], [ 0, %364 ]
  %427 = phi i32 [ %429, %468 ], [ 0, %364 ]
  %428 = phi i32 [ %459, %468 ], [ -32000, %364 ]
  %429 = add nuw nsw i32 %427, 1
  %430 = load i32, i32* %9, align 4
  %431 = sext i32 %430 to i64
  %432 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %431
  %433 = load i32, i32* %432, align 4
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %433)
  %434 = load i32, i32* %432, align 4
  %435 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %434)
  %436 = icmp eq i32 %435, 0
  br i1 %436, label %458, label %437

437:                                              ; preds = %425
  %438 = load i64, i64* %365, align 8
  %439 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4
  %440 = load i32, i32* %31, align 8
  %441 = add i32 %440, -1
  %442 = add i32 %441, %439
  %443 = sext i32 %442 to i64
  %444 = getelementptr inbounds [1000 x i64], [1000 x i64]* %366, i64 0, i64 %443
  store i64 %438, i64* %444, align 8
  %445 = load i32, i32* %432, align 4
  %446 = sext i32 %441 to i64
  %447 = getelementptr inbounds [64 x i32], [64 x i32]* %367, i64 0, i64 %446
  store i32 %445, i32* %447, align 4
  %448 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %449 = load i32, i32* %31, align 8
  %450 = sext i32 %449 to i64
  %451 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %450
  store i32 %448, i32* %451, align 4
  %452 = icmp ne i32 %448, 0
  %453 = or i1 %370, %452
  %454 = zext i1 %453 to i32
  %455 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %368, i32 %369, i32 %454)
  %456 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %368, i32 %372, i32 %373, i32 0, i32 %375)
  %457 = sub nsw i32 0, %456
  br label %458

458:                                              ; preds = %437, %425
  %459 = phi i32 [ %457, %437 ], [ %428, %425 ]
  %460 = load i32, i32* %432, align 4
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %460)
  %461 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %462 = icmp eq i32 %461, 0
  br i1 %462, label %463, label %478

463:                                              ; preds = %458
  %464 = icmp slt i32 %459, %70
  %465 = or i1 %436, %464
  br i1 %465, label %468, label %466

466:                                              ; preds = %463
  %467 = icmp sgt i32 %426, 0
  br i1 %467, label %474, label %468

468:                                              ; preds = %466, %463
  %469 = phi i32 [ 1, %466 ], [ %426, %463 ]
  %470 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  %471 = icmp ne i32 %470, 0
  %472 = icmp ult i32 %427, 2
  %473 = select i1 %471, i1 %472, i1 false
  br i1 %473, label %425, label %478, !llvm\.loop !46

474:                                              ; preds = %466, %417
  %475 = load i32, i32* %13, align 4
  %476 = load i32, i32* %11, align 4
  %477 = load i32, i32* %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %70, i32 %64, i32 %70, i32 %475, i32 %476, i32 0, i32 %477, i32 %3)
  br label %1052

478:                                              ; preds = %468, %458, %419, %409, %361, %355, %347, %338, %330
  %479 = phi i32 [ -32000, %338 ], [ -32000, %355 ], [ -32000, %347 ], [ -32000, %330 ], [ -32000, %361 ], [ %410, %419 ], [ %410, %409 ], [ %459, %468 ], [ %459, %458 ]
  %480 = load i32, i32* %14, align 4
  %481 = icmp eq i32 %480, 0
  %482 = load i32, i32* %15, align 4
  %483 = icmp eq i32 %482, 0
  %484 = select i1 %481, i1 %483, i1 false
  %485 = load i32, i32* %11, align 4
  %486 = icmp eq i32 %485, 0
  %487 = select i1 %484, i1 %486, i1 false
  %488 = and i1 %487, %281
  %489 = icmp sgt i32 %277, 1
  %490 = select i1 %488, i1 %489, i1 false
  %491 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4
  %492 = icmp ne i32 %491, 2
  %493 = select i1 %490, i1 %492, i1 false
  br i1 %493, label %494, label %571

494:                                              ; preds = %478
  %495 = add nsw i32 %3, -24
  %496 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %495, i32 0, i32 %86)
  %497 = icmp sgt i32 %496, %64
  br i1 %497, label %498, label %571

498:                                              ; preds = %494
  store i32 -1, i32* %9, align 4
  %499 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  %500 = icmp ne i32 %499, 0
  %501 = load i32, i32* %14, align 4
  %502 = icmp slt i32 %501, 2
  %503 = select i1 %500, i1 %502, i1 false
  br i1 %503, label %504, label %571

504:                                              ; preds = %498
  %505 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %506 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %507 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %508 = add nsw i32 %3, -16
  %509 = sub nsw i32 0, %70
  %510 = sub i32 50, %64
  %511 = sub nsw i32 0, %64
  %512 = xor i32 %64, -1
  %513 = icmp eq i32 %86, 0
  %514 = zext i1 %513 to i32
  %515 = sub i32 49, %64
  %516 = add nsw i32 %64, -50
  br label %517

517:                                              ; preds = %559, %504
  %518 = phi i32 [ 0, %504 ], [ %562, %559 ]
  %519 = phi i32 [ %479, %504 ], [ %561, %559 ]
  %520 = phi i32 [ 1, %504 ], [ %560, %559 ]
  %521 = load i32, i32* %9, align 4
  %522 = sext i32 %521 to i64
  %523 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %522
  %524 = load i32, i32* %523, align 4
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %524)
  %525 = load i32, i32* %523, align 4
  %526 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %525)
  %527 = icmp eq i32 %526, 0
  br i1 %527, label %559, label %528

528:                                              ; preds = %517
  %529 = load i64, i64* %505, align 8
  %530 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4
  %531 = load i32, i32* %31, align 8
  %532 = add i32 %531, -1
  %533 = add i32 %532, %530
  %534 = sext i32 %533 to i64
  %535 = getelementptr inbounds [1000 x i64], [1000 x i64]* %506, i64 0, i64 %534
  store i64 %529, i64* %535, align 8
  %536 = load i32, i32* %523, align 4
  %537 = sext i32 %532 to i64
  %538 = getelementptr inbounds [64 x i32], [64 x i32]* %507, i64 0, i64 %537
  store i32 %536, i32* %538, align 4
  %539 = add nsw i32 %518, 1
  %540 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %541 = load i32, i32* %31, align 8
  %542 = sext i32 %541 to i64
  %543 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %542
  store i32 %540, i32* %543, align 4
  %544 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %509, i32 %510, i32 1)
  %545 = icmp eq i32 %520, 0
  br i1 %545, label %553, label %546

546:                                              ; preds = %528
  %547 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %512, i32 %511, i32 %508, i32 0, i32 %514)
  %548 = sub nsw i32 0, %547
  %549 = icmp slt i32 %64, %548
  br i1 %549, label %550, label %551

550:                                              ; preds = %546
  store i32 1, i32* %14, align 4
  br label %559

551:                                              ; preds = %546
  store i32 0, i32* %14, align 4
  %552 = add nsw i32 %518, 11
  br label %559

553:                                              ; preds = %528
  %554 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %515, i32 %510, i32 %508, i32 0, i32 0)
  %555 = sub nsw i32 0, %554
  %556 = icmp slt i32 %516, %555
  br i1 %556, label %557, label %559

557:                                              ; preds = %553
  store i32 0, i32* %14, align 4
  %558 = add nsw i32 %518, 11
  br label %559

559:                                              ; preds = %557, %553, %551, %550, %517
  %560 = phi i32 [ %520, %517 ], [ 0, %553 ], [ 0, %557 ], [ 0, %550 ], [ 0, %551 ]
  %561 = phi i32 [ %519, %517 ], [ %555, %553 ], [ %555, %557 ], [ %548, %550 ], [ %548, %551 ]
  %562 = phi i32 [ %518, %517 ], [ %539, %553 ], [ %558, %557 ], [ %539, %550 ], [ %552, %551 ]
  %563 = load i32, i32* %523, align 4
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %563)
  %564 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  %565 = icmp ne i32 %564, 0
  %566 = load i32, i32* %14, align 4
  %567 = icmp slt i32 %566, 2
  %568 = select i1 %565, i1 %567, i1 false
  %569 = icmp slt i32 %562, 3
  %570 = select i1 %568, i1 %569, i1 false
  br i1 %570, label %517, label %571, !llvm\.loop !47

571:                                              ; preds = %559, %498, %494, %478
  %572 = phi i32 [ %479, %478 ], [ %479, %494 ], [ %479, %498 ], [ %561, %559 ]
  br i1 %96, label %578, label %573

573:                                              ; preds = %571
  %574 = load i32, i32* %31, align 8
  %575 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4
  %576 = shl nsw i32 %575, 1
  %577 = icmp sle i32 %574, %576
  br label %578

578:                                              ; preds = %573, %571
  %579 = phi i1 [ false, %571 ], [ %577, %573 ]
  store i32 -1, i32* %9, align 4
  %580 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  %581 = icmp eq i32 %580, 0
  br i1 %581, label %1020, label %582

582:                                              ; preds = %578
  %583 = icmp eq i32 %277, 1
  %584 = select i1 %93, i1 %583, i1 false
  %585 = select i1 %584, i32 4, i32 0
  %586 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %587 = or i32 %585, 2
  %588 = select i1 %579, i32 %587, i32 %585
  %589 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %590 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %591 = select i1 %579, i32 3, i32 1
  %592 = add nsw i32 %145, %134
  %593 = icmp eq i32 %592, 1
  %594 = sdiv i32 %3, 4
  %595 = add nsw i32 %594, 1
  %596 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 0
  %597 = icmp slt i32 %3, 25
  %598 = icmp slt i32 %3, 9
  %599 = icmp ult i32 %3, 13
  %600 = add nsw i32 %92, 100
  %601 = add nsw i32 %92, 300
  %602 = add nsw i32 %92, 75
  %603 = add nsw i32 %92, 200
  %604 = select i1 %579, i32 4, i32 2
  %605 = sub nsw i32 0, %70
  %606 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %607 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %608 = icmp sgt i32 %3, 4
  %609 = icmp eq i32 %86, 0
  %610 = zext i1 %609 to i32
  %611 = add nsw i32 %3, -4
  br label %612

612:                                              ; preds = %1011, %582
  %613 = phi i32 [ %64, %582 ], [ %1018, %1011 ]
  %614 = phi i32 [ %572, %582 ], [ %1017, %1011 ]
  %615 = phi i32 [ 1, %582 ], [ %1016, %1011 ]
  %616 = phi i32 [ -32000, %582 ], [ %1015, %1011 ]
  %617 = phi i32 [ 1, %582 ], [ %1014, %1011 ]
  %618 = phi i32 [ 1, %582 ], [ %1013, %1011 ]
  %619 = icmp ne i32 %617, 0
  %620 = icmp sgt i32 %618, %595
  %621 = add nsw i32 %613, 1
  %622 = icmp eq i32 %70, %621
  br label %623

623:                                              ; preds = %751, %612
  %624 = phi i32 [ %615, %612 ], [ 0, %751 ]
  %625 = load i32, i32* %31, align 8
  %626 = icmp slt i32 %625, 60
  br i1 %626, label %630, label %627

627:                                              ; preds = %623
  %628 = load i32, i32* %9, align 4
  %629 = sext i32 %628 to i64
  br label %709

630:                                              ; preds = %623
  %631 = load i32, i32* %9, align 4
  %632 = sext i32 %631 to i64
  %633 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %632
  %634 = load i32, i32* %633, align 4
  %635 = lshr i32 %634, 6
  %636 = and i32 %635, 63
  %637 = zext i32 %636 to i64
  %638 = getelementptr inbounds [64 x i32], [64 x i32]* %586, i64 0, i64 %637
  %639 = load i32, i32* %638, align 4
  %640 = add nsw i32 %639, 1
  %641 = and i32 %640, -2
  %642 = icmp eq i32 %641, 2
  br i1 %642, label %643, label %650

643:                                              ; preds = %630
  %644 = lshr i32 %634, 3
  %645 = and i32 %644, 7
  switch i32 %645, label %646 [
    i32 1, label %649
    i32 6, label %649
  ]

646:                                              ; preds = %643
  %647 = and i32 %634, 61440
  %648 = icmp eq i32 %647, 0
  br i1 %648, label %650, label %649

649:                                              ; preds = %646, %643, %643
  br label %650

650:                                              ; preds = %649, %646, %630
  %651 = phi i32 [ %585, %646 ], [ %585, %630 ], [ %588, %649 ]
  %652 = lshr i32 %634, 19
  %653 = and i32 %652, 15
  %654 = icmp eq i32 %653, 13
  br i1 %654, label %685, label %655

655:                                              ; preds = %650
  %656 = add nsw i32 %625, -1
  %657 = sext i32 %656 to i64
  %658 = getelementptr inbounds [64 x i32], [64 x i32]* %589, i64 0, i64 %657
  %659 = load i32, i32* %658, align 4
  %660 = lshr i32 %659, 19
  %661 = and i32 %660, 15
  %662 = icmp eq i32 %661, 13
  br i1 %662, label %685, label %663

663:                                              ; preds = %655
  %664 = zext i32 %653 to i64
  %665 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %664
  %666 = load i32, i32* %665, align 4
  %667 = zext i32 %661 to i64
  %668 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %667
  %669 = load i32, i32* %668, align 4
  %670 = icmp eq i32 %666, %669
  br i1 %670, label %671, label %685

671:                                              ; preds = %663
  %672 = and i32 %634, 63
  %673 = and i32 %659, 63
  %674 = icmp eq i32 %672, %673
  br i1 %674, label %675, label %685

675:                                              ; preds = %671
  %676 = load i32, i32* %590, align 4
  %677 = icmp eq i32 %676, 0
  %678 = zext i1 %677 to i32
  %679 = lshr i32 %634, 12
  %680 = and i32 %679, 15
  %681 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %678, i32 %636, i32 %672, i32 %680)
  %682 = icmp sgt i32 %681, 0
  %683 = select i1 %682, i32 %591, i32 0
  %684 = add nsw i32 %683, %651
  br label %685

685:                                              ; preds = %675, %671, %663, %655, %650
  %686 = phi i32 [ %651, %671 ], [ %651, %663 ], [ %651, %655 ], [ %651, %650 ], [ %684, %675 ]
  %687 = load i32, i32* %14, align 4
  %688 = icmp eq i32 %687, 1
  %689 = icmp ne i32 %686, 0
  %690 = select i1 %688, i1 %689, i1 false
  %691 = select i1 %690, i1 %619, i1 false
  br i1 %691, label %692, label %693

692:                                              ; preds = %685
  store i32 1, i32* %15, align 4
  br label %699

693:                                              ; preds = %685
  %694 = xor i1 %689, true
  %695 = select i1 %694, i1 %688, i1 false
  %696 = select i1 %695, i1 %619, i1 false
  br i1 %696, label %697, label %699

697:                                              ; preds = %693
  store i32 0, i32* %15, align 4
  %698 = add nsw i32 %686, %591
  br label %699

699:                                              ; preds = %697, %693, %692
  %700 = phi i32 [ %686, %692 ], [ %686, %693 ], [ %698, %697 ]
  %701 = icmp slt i32 %700, 4
  %702 = select i1 %701, i32 %700, i32 4
  %703 = load i32, i32* %633, align 4
  %704 = lshr i32 %703, 19
  %705 = and i32 %704, 15
  switch i32 %705, label %706 [
    i32 13, label %709
    i32 1, label %709
    i32 2, label %709
  ]

706:                                              ; preds = %699
  %707 = add nsw i32 %702, 4
  %708 = select i1 %593, i32 %707, i32 %702
  br label %709

709:                                              ; preds = %706, %699, %699, %699, %627
  %710 = phi i64 [ %629, %627 ], [ %632, %706 ], [ %632, %699 ], [ %632, %699 ], [ %632, %699 ]
  %711 = phi i32 [ 0, %627 ], [ %708, %706 ], [ %702, %699 ], [ %702, %699 ], [ %702, %699 ]
  %712 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %710
  %713 = load i32, i32* %712, align 4
  %714 = and i32 %713, 7864320
  %715 = icmp eq i32 %714, 6815744
  %716 = xor i1 %715, true
  %717 = xor i1 %620, true
  %718 = select i1 %716, i1 true, i1 %717
  %719 = select i1 %716, i1 false, i1 true
  %720 = select i1 %716, i32 %624, i32 %615
  br i1 %718, label %757, label %721

721:                                              ; preds = %709
  %722 = lshr i32 %713, 6
  %723 = and i32 %722, 63
  %724 = zext i32 %723 to i64
  %725 = getelementptr inbounds [64 x i32], [64 x i32]* %586, i64 0, i64 %724
  %726 = load i32, i32* %725, align 4
  %727 = add nsw i32 %726, -1
  %728 = and i32 %713, 63
  %729 = load i32, i32* %596, align 8
  %730 = sext i32 %729 to i64
  %731 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %730
  %732 = sext i32 %727 to i64
  %733 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %731, i64 0, i64 %732
  %734 = zext i32 %728 to i64
  %735 = getelementptr inbounds [64 x i32], [64 x i32]* %733, i64 0, i64 %734
  %736 = load i32, i32* %735, align 4
  %737 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %730
  %738 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %737, i64 0, i64 %732
  %739 = getelementptr inbounds [64 x i32], [64 x i32]* %738, i64 0, i64 %734
  %740 = load i32, i32* %739, align 4
  %741 = sub nsw i32 %740, %736
  %742 = mul nsw i32 %736, %595
  %743 = icmp slt i32 %742, %741
  %744 = icmp eq i32 %711, 0
  %745 = and i1 %743, %597
  %746 = select i1 %745, i1 %744, i1 false
  %747 = select i1 %746, i1 %622, i1 false
  %748 = and i32 %713, 61440
  %749 = icmp eq i32 %748, 0
  %750 = select i1 %747, i1 %749, i1 false
  br i1 %750, label %751, label %757

751:                                              ; preds = %721
  %752 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  %753 = icmp eq i32 %752, 0
  br i1 %753, label %754, label %623, !llvm\.loop !48

754:                                              ; preds = %751
  %755 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %756 = icmp eq i32 %755, 0
  br label %1039

757:                                              ; preds = %721, %709
  %758 = phi i1 [ true, %721 ], [ %719, %709 ]
  %759 = phi i32 [ %624, %721 ], [ %720, %709 ]
  br i1 %598, label %760, label %763

760:                                              ; preds = %757
  %761 = icmp slt i32 %602, %613
  %762 = icmp slt i32 %603, %613
  br label %768

763:                                              ; preds = %757
  %764 = icmp slt i32 %600, %613
  %765 = icmp slt i32 %601, %613
  %766 = select i1 %599, i1 %764, i1 false
  %767 = select i1 %599, i1 %765, i1 false
  br label %768

768:                                              ; preds = %763, %760
  %769 = phi i1 [ %761, %760 ], [ %766, %763 ]
  %770 = phi i1 [ %762, %760 ], [ %767, %763 ]
  br i1 %758, label %781, label %771

771:                                              ; preds = %768
  %772 = load i32, i32* %590, align 4
  %773 = icmp eq i32 %772, 0
  %774 = zext i1 %773 to i32
  %775 = lshr i32 %713, 6
  %776 = and i32 %775, 63
  %777 = and i32 %713, 63
  %778 = lshr i32 %713, 12
  %779 = and i32 %778, 15
  %780 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %774, i32 %776, i32 %777, i32 %779)
  br label %781

781:                                              ; preds = %771, %768
  %782 = phi i32 [ %780, %771 ], [ -1000000, %768 ]
  %783 = load i32, i32* %712, align 4
  call void @_Z4makeP7state_ti(%struct.state_t* nonnull %0, i32 %783)
  %784 = load i32, i32* %712, align 4
  %785 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* nonnull %0, i32 %784)
  %786 = icmp eq i32 %785, 0
  br i1 %786, label %921, label %787

787:                                              ; preds = %781
  %788 = call i32 @_Z8in_checkP7state_t(%struct.state_t* nonnull %0)
  %789 = icmp ne i32 %788, 0
  %790 = select i1 %789, i32 %604, i32 0
  %791 = add nsw i32 %790, %711
  %792 = or i32 %788, %91
  %793 = icmp eq i32 %792, 0
  %794 = select i1 %793, i1 %622, i1 false
  br i1 %794, label %795, label %813

795:                                              ; preds = %787
  %796 = icmp slt i32 %782, 86
  %797 = and i1 %770, %796
  br i1 %797, label %798, label %804

798:                                              ; preds = %795
  %799 = load i32, i32* %712, align 4
  %800 = and i32 %799, 61440
  %801 = icmp eq i32 %800, 0
  br i1 %801, label %802, label %804

802:                                              ; preds = %798
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %799)
  %803 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  br label %1011, !llvm\.loop !48

804:                                              ; preds = %798, %795
  %805 = icmp slt i32 %782, -50
  %806 = and i1 %769, %805
  br i1 %806, label %807, label %813

807:                                              ; preds = %804
  %808 = load i32, i32* %712, align 4
  %809 = and i32 %808, 61440
  %810 = icmp eq i32 %809, 0
  br i1 %810, label %811, label %813

811:                                              ; preds = %807
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %808)
  %812 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  br label %1011, !llvm\.loop !48

813:                                              ; preds = %807, %804, %787
  %814 = add nsw i32 %791, %3
  %815 = sub nsw i32 0, %613
  %816 = sub i32 130, %613
  %817 = icmp sgt i32 %814, 4
  %818 = or i1 %817, %789
  %819 = zext i1 %818 to i32
  %820 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %605, i32 %816, i32 %819)
  %821 = load i32, i32* %31, align 8
  %822 = sext i32 %821 to i64
  %823 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %822
  store i32 %788, i32* %823, align 4
  %824 = load i64, i64* %606, align 8
  %825 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4
  %826 = add i32 %821, -1
  %827 = add i32 %826, %825
  %828 = sext i32 %827 to i64
  %829 = getelementptr inbounds [1000 x i64], [1000 x i64]* %607, i64 0, i64 %828
  store i64 %824, i64* %829, align 8
  %830 = load i32, i32* %712, align 4
  %831 = sext i32 %826 to i64
  %832 = getelementptr inbounds [64 x i32], [64 x i32]* %589, i64 0, i64 %831
  store i32 %830, i32* %832, align 4
  %833 = icmp sgt i32 %618, 3
  %834 = select i1 %608, i1 %833, i1 false
  br i1 %834, label %835, label %870

835:                                              ; preds = %813
  %836 = icmp ne i32 %70, %621
  %837 = icmp ne i32 %791, 0
  %838 = select i1 %836, i1 true, i1 %837
  %839 = or i1 %838, %789
  %840 = icmp sgt i32 %782, -51
  %841 = or i1 %839, %840
  br i1 %841, label %870, label %842

842:                                              ; preds = %835
  %843 = and i32 %830, 63
  %844 = zext i32 %843 to i64
  %845 = getelementptr inbounds [64 x i32], [64 x i32]* %586, i64 0, i64 %844
  %846 = load i32, i32* %845, align 4
  %847 = add nsw i32 %846, -1
  %848 = load i32, i32* %596, align 8
  %849 = sext i32 %848 to i64
  %850 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %849
  %851 = sext i32 %847 to i64
  %852 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %850, i64 0, i64 %851
  %853 = getelementptr inbounds [64 x i32], [64 x i32]* %852, i64 0, i64 %844
  %854 = load i32, i32* %853, align 4
  %855 = shl i32 %854, 7
  %856 = add i32 %855, 128
  %857 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %849
  %858 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %857, i64 0, i64 %851
  %859 = getelementptr inbounds [64 x i32], [64 x i32]* %858, i64 0, i64 %844
  %860 = load i32, i32* %859, align 4
  %861 = add nsw i32 %860, 1
  %862 = sdiv i32 %856, %861
  %863 = icmp slt i32 %862, 80
  %864 = and i32 %830, 61440
  %865 = icmp eq i32 %864, 0
  %866 = select i1 %863, i1 %865, i1 false
  %867 = select i1 %866, i32 4, i32 0
  %868 = select i1 %866, i32 -4, i32 0
  %869 = select i1 %866, i32 %611, i32 %814
  br label %870

870:                                              ; preds = %842, %835, %813
  %871 = phi i1 [ false, %835 ], [ false, %813 ], [ %866, %842 ]
  %872 = phi i32 [ 0, %835 ], [ 0, %813 ], [ %867, %842 ]
  %873 = phi i32 [ %791, %835 ], [ %791, %813 ], [ %868, %842 ]
  %874 = phi i32 [ %814, %835 ], [ %814, %813 ], [ %869, %842 ]
  %875 = add nsw i32 %874, -4
  %876 = icmp eq i32 %617, 1
  %877 = icmp slt i32 %874, 5
  br i1 %876, label %878, label %885

878:                                              ; preds = %870
  br i1 %877, label %879, label %882

879:                                              ; preds = %878
  %880 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %605, i32 %815, i32 0, i32 0)
  %881 = sub nsw i32 0, %880
  br label %917

882:                                              ; preds = %878
  %883 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %605, i32 %815, i32 %875, i32 0, i32 %610)
  %884 = sub nsw i32 0, %883
  br label %917

885:                                              ; preds = %870
  %886 = xor i32 %613, -1
  br i1 %877, label %887, label %889

887:                                              ; preds = %885
  %888 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %886, i32 %815, i32 0, i32 0)
  br label %891

889:                                              ; preds = %885
  %890 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %886, i32 %815, i32 %875, i32 0, i32 1)
  br label %891

891:                                              ; preds = %889, %887
  %892 = phi i32 [ %888, %887 ], [ %890, %889 ]
  %893 = sub nsw i32 0, %892
  %894 = icmp slt i32 %616, %893
  %895 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %896 = icmp eq i32 %895, 0
  %897 = select i1 %894, i1 %896, i1 false
  %898 = icmp slt i32 %613, %893
  %899 = select i1 %897, i1 %898, i1 false
  br i1 %899, label %900, label %917

900:                                              ; preds = %891
  br i1 %871, label %901, label %903

901:                                              ; preds = %900
  %902 = add nsw i32 %873, %872
  br label %905

903:                                              ; preds = %900
  %904 = icmp sgt i32 %70, %893
  br i1 %904, label %905, label %917

905:                                              ; preds = %903, %901
  %906 = phi i32 [ %902, %901 ], [ %873, %903 ]
  %907 = add nsw i32 %906, %3
  %908 = icmp slt i32 %907, 5
  br i1 %908, label %909, label %912

909:                                              ; preds = %905
  %910 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %605, i32 %815, i32 0, i32 0)
  %911 = sub nsw i32 0, %910
  br label %917

912:                                              ; preds = %905
  %913 = add nsw i32 %907, -4
  %914 = zext i1 %871 to i32
  %915 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %605, i32 %815, i32 %913, i32 0, i32 %914)
  %916 = sub nsw i32 0, %915
  br label %917

917:                                              ; preds = %912, %909, %903, %891, %882, %879
  %918 = phi i32 [ %881, %879 ], [ %884, %882 ], [ %893, %891 ], [ %911, %909 ], [ %916, %912 ], [ %893, %903 ]
  %919 = icmp sgt i32 %918, %616
  %920 = select i1 %919, i32 %918, i32 %616
  br label %921

921:                                              ; preds = %917, %781
  %922 = phi i32 [ %616, %781 ], [ %920, %917 ]
  %923 = phi i32 [ %759, %781 ], [ 0, %917 ]
  %924 = phi i32 [ %614, %781 ], [ %918, %917 ]
  %925 = load i32, i32* %712, align 4
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %925)
  %926 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %927 = icmp eq i32 %926, 0
  br i1 %927, label %928, label %1052

928:                                              ; preds = %921
  br i1 %786, label %1006, label %929

929:                                              ; preds = %928
  %930 = icmp sgt i32 %924, %613
  br i1 %930, label %931, label %999

931:                                              ; preds = %929
  %932 = icmp slt i32 %924, %70
  %933 = load i32, i32* %712, align 4
  br i1 %932, label %996, label %934

934:                                              ; preds = %931
  call fastcc void @_ZL12history_goodP7state_tii(%struct.state_t* nonnull %0, i32 %933, i32 %3)
  %935 = icmp sgt i32 %618, 1
  br i1 %935, label %936, label %989

936:                                              ; preds = %934
  %937 = add nsw i32 %618, -1
  %938 = add nsw i32 %3, 3
  %939 = sdiv i32 %938, 4
  %940 = zext i32 %937 to i64
  br label %941

941:                                              ; preds = %986, %936
  %942 = phi i64 [ 0, %936 ], [ %987, %986 ]
  %943 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %942
  %944 = load i32, i32* %943, align 4
  %945 = and i32 %944, 7925760
  %946 = icmp eq i32 %945, 6815744
  br i1 %946, label %947, label %986

947:                                              ; preds = %941
  %948 = lshr i32 %944, 6
  %949 = and i32 %948, 63
  %950 = zext i32 %949 to i64
  %951 = getelementptr inbounds [64 x i32], [64 x i32]* %586, i64 0, i64 %950
  %952 = load i32, i32* %951, align 4
  %953 = add nsw i32 %952, -1
  %954 = and i32 %944, 63
  %955 = load i32, i32* %596, align 8
  %956 = sext i32 %955 to i64
  %957 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %956
  %958 = sext i32 %953 to i64
  %959 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %957, i64 0, i64 %958
  %960 = zext i32 %954 to i64
  %961 = getelementptr inbounds [64 x i32], [64 x i32]* %959, i64 0, i64 %960
  %962 = load i32, i32* %961, align 4
  %963 = add nsw i32 %962, %939
  store i32 %963, i32* %961, align 4
  %964 = icmp sgt i32 %963, 16384
  br i1 %964, label %965, label %986

965:                                              ; preds = %947
  %966 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %956
  br label %967

967:                                              ; preds = %983, %965
  %968 = phi i64 [ 0, %965 ], [ %984, %983 ]
  %969 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %966, i64 0, i64 %968
  %970 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %957, i64 0, i64 %968
  br label %971

971:                                              ; preds = %971, %967
  %972 = phi i64 [ 0, %967 ], [ %981, %971 ]
  %973 = getelementptr inbounds [64 x i32], [64 x i32]* %969, i64 0, i64 %972
  %974 = load i32, i32* %973, align 4
  %975 = add nsw i32 %974, 1
  %976 = ashr i32 %975, 1
  store i32 %976, i32* %973, align 4
  %977 = getelementptr inbounds [64 x i32], [64 x i32]* %970, i64 0, i64 %972
  %978 = load i32, i32* %977, align 4
  %979 = add nsw i32 %978, 1
  %980 = ashr i32 %979, 1
  store i32 %980, i32* %977, align 4
  %981 = add nuw nsw i64 %972, 1
  %982 = icmp eq i64 %981, 64
  br i1 %982, label %983, label %971, !llvm\.loop !49

983:                                              ; preds = %971
  %984 = add nuw nsw i64 %968, 1
  %985 = icmp eq i64 %984, 12
  br i1 %985, label %986, label %967, !llvm.loop !50

986:                                              ; preds = %983, %947, %941
  %987 = add nuw nsw i64 %942, 1
  %988 = icmp eq i64 %987, %940
  br i1 %988, label %989, label %941, !llvm.loop !51

989:                                              ; preds = %986, %934
  %990 = load i32, i32* %712, align 4
  %991 = call zeroext i16 @_Z12compact_movei(i32 %990)
  %992 = zext i16 %991 to i32
  %993 = load i32, i32* %11, align 4
  %994 = load i32, i32* %14, align 4
  %995 = load i32, i32* %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %924, i32 %64, i32 %70, i32 %992, i32 %993, i32 %994, i32 %995, i32 %3)
  br label %1052

996:                                              ; preds = %931
  %997 = call zeroext i16 @_Z12compact_movei(i32 %933)
  %998 = zext i16 %997 to i32
  store i32 %998, i32* %13, align 4
  br label %999

999:                                              ; preds = %996, %929
  %1000 = phi i32 [ %924, %996 ], [ %613, %929 ]
  %1001 = load i32, i32* %712, align 4
  %1002 = add nsw i32 %618, -1
  %1003 = sext i32 %1002 to i64
  %1004 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %1003
  store i32 %1001, i32* %1004, align 4
  %1005 = add nsw i32 %618, 1
  br label %1006

1006:                                             ; preds = %999, %928
  %1007 = phi i32 [ %1005, %999 ], [ %618, %928 ]
  %1008 = phi i32 [ 0, %999 ], [ %617, %928 ]
  %1009 = phi i32 [ %1000, %999 ], [ %613, %928 ]
  %1010 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %279, i32* nonnull %278, i32 %276)
  br label %1011

1011:                                             ; preds = %1006, %811, %802
  %1012 = phi i32 [ %1010, %1006 ], [ %812, %811 ], [ %803, %802 ]
  %1013 = phi i32 [ %1007, %1006 ], [ %618, %811 ], [ %618, %802 ]
  %1014 = phi i32 [ %1008, %1006 ], [ %617, %811 ], [ %617, %802 ]
  %1015 = phi i32 [ %922, %1006 ], [ %613, %811 ], [ %613, %802 ]
  %1016 = phi i32 [ %923, %1006 ], [ 0, %811 ], [ 0, %802 ]
  %1017 = phi i32 [ %924, %1006 ], [ %614, %811 ], [ %614, %802 ]
  %1018 = phi i32 [ %1009, %1006 ], [ %613, %811 ], [ %613, %802 ]
  %1019 = icmp eq i32 %1012, 0
  br i1 %1019, label %1020, label %612

1020:                                             ; preds = %1011, %578
  %1021 = phi i32 [ -32000, %578 ], [ %1015, %1011 ]
  %1022 = phi i32 [ 1, %578 ], [ %1016, %1011 ]
  %1023 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %1024 = icmp eq i32 %1023, 0
  %1025 = icmp ne i32 %1022, 0
  %1026 = select i1 %1025, i1 %1024, i1 false
  br i1 %1026, label %1027, label %1039

1027:                                             ; preds = %1020
  %1028 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %1029 = icmp eq i32 %1028, 0
  %1030 = load i32, i32* %11, align 4
  %1031 = load i32, i32* %14, align 4
  %1032 = load i32, i32* %15, align 4
  br i1 %1029, label %1038, label %1033

1033:                                             ; preds = %1027
  %1034 = load i32, i32* %31, align 8
  %1035 = add nsw i32 %1034, -32000
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %1035, i32 %64, i32 %70, i32 0, i32 %1030, i32 %1031, i32 %1032, i32 %3)
  %1036 = load i32, i32* %31, align 8
  %1037 = add nsw i32 %1036, -32000
  br label %1052

1038:                                             ; preds = %1027
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 0, i32 %64, i32 %70, i32 0, i32 %1030, i32 %1031, i32 %1032, i32 %3)
  br label %1052

1039:                                             ; preds = %1020, %754
  %1040 = phi i1 [ %756, %754 ], [ %1024, %1020 ]
  %1041 = phi i32 [ %616, %754 ], [ %1021, %1020 ]
  %1042 = load i32, i32* %46, align 4
  %1043 = icmp sgt i32 %1042, 98
  %1044 = xor i1 %1040, true
  %1045 = select i1 %1043, i1 true, i1 %1044
  %1046 = select i1 %1043, i32 0, i32 %1041
  br i1 %1045, label %1052, label %1047

1047:                                             ; preds = %1039
  %1048 = load i32, i32* %13, align 4
  %1049 = load i32, i32* %11, align 4
  %1050 = load i32, i32* %14, align 4
  %1051 = load i32, i32* %15, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %1041, i32 %64, i32 %70, i32 %1048, i32 %1049, i32 %1050, i32 %1051, i32 %3)
  br label %1052

1052:                                             ; preds = %1047, %1039, %1038, %1033, %989, %921, %474, %243, %237, %224, %211, %172, %117, %110, %103, %77, %74, %72, %67, %61, %49, %36, %34
  %1053 = phi i32 [ %35, %34 ], [ %56, %49 ], [ %73, %72 ], [ 0, %36 ], [ %59, %61 ], [ %65, %67 ], [ %75, %74 ], [ %78, %77 ], [ %70, %474 ], [ 0, %172 ], [ %92, %103 ], [ %111, %110 ], [ %92, %117 ], [ %213, %224 ], [ 0, %211 ], [ %1037, %1033 ], [ 0, %1038 ], [ %1046, %1039 ], [ %1041, %1047 ], [ 0, %237 ], [ %64, %243 ], [ %924, %989 ], [ 0, %921 ]
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
  ret i32 %1053
}

; Function Attrs: uwtable mustprogress
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
  %18 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 22
  %19 = load i64, i64* %18, align 8
  %20 = add i64 %19, 1
  store i64 %20, i64* %18, align 8
  %21 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 23
  %22 = load i64, i64* %21, align 8
  %23 = add i64 %22, 1
  store i64 %23, i64* %21, align 8
  %24 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14
  %25 = load i32, i32* %24, align 8
  %26 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 24
  %27 = load i32, i32* %26, align 8
  %28 = icmp sgt i32 %25, %27
  br i1 %28, label %29, label %30

29:                                               ; preds = %5
  store i32 %25, i32* %26, align 8
  br label %30

30:                                               ; preds = %29, %5
  %31 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %20)
  %32 = icmp eq i32 %31, 0
  br i1 %32, label %33, label %378

33:                                               ; preds = %30
  %34 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nonnull @gamestate, %struct.state_t* nonnull %0)
  %35 = icmp eq i32 %34, 0
  br i1 %35, label %36, label %40

36:                                               ; preds = %33
  %37 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15
  %38 = load i32, i32* %37, align 4
  %39 = icmp sgt i32 %38, 99
  br i1 %39, label %40, label %48

40:                                               ; preds = %36, %33
  %41 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4
  %42 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %43 = load i32, i32* %42, align 4
  %44 = icmp eq i32 %41, %43
  %45 = load i32, i32* @contempt, align 4
  %46 = sub nsw i32 0, %45
  %47 = select i1 %44, i32 %45, i32 %46
  br label %378

48:                                               ; preds = %36
  %49 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nonnull %0, i32* nonnull %8, i32 %1, i32 %2, i32* nonnull %7, i32* nonnull %9, i32* nonnull %9, i32* nonnull %9, i32* nonnull %9, i32 0)
  switch i32 %49, label %59 [
    i32 3, label %50
    i32 1, label %52
    i32 2, label %55
    i32 4, label %58
  ]

50:                                               ; preds = %48
  %51 = load i32, i32* %8, align 4
  br label %378

52:                                               ; preds = %48
  %53 = load i32, i32* %8, align 4
  %54 = icmp sgt i32 %53, %1
  br i1 %54, label %59, label %378

55:                                               ; preds = %48
  %56 = load i32, i32* %8, align 4
  %57 = icmp slt i32 %56, %2
  br i1 %57, label %59, label %378

58:                                               ; preds = %48
  store i32 65535, i32* %7, align 4
  br label %59

59:                                               ; preds = %58, %55, %52, %48
  %60 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4
  %61 = shl nsw i32 %60, 1
  %62 = icmp slt i32 %61, %4
  br i1 %62, label %66, label %63

63:                                               ; preds = %59
  %64 = load i32, i32* %24, align 8
  %65 = icmp sgt i32 %64, 60
  br i1 %65, label %66, label %68

66:                                               ; preds = %63, %59
  %67 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %1, i32 %2, i32 0)
  br label %378

68:                                               ; preds = %63
  %69 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25
  %70 = sext i32 %64 to i64
  %71 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %70
  %72 = load i32, i32* %71, align 4
  %73 = call i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nonnull %0)
  %74 = add nsw i32 %73, 50
  %75 = icmp ne i32 %72, 0
  br i1 %75, label %147, label %76

76:                                               ; preds = %68
  %77 = icmp slt i32 %73, %2
  br i1 %77, label %80, label %78

78:                                               ; preds = %76
  %79 = load i32, i32* %7, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %73, i32 %1, i32 %2, i32 %79, i32 0, i32 0, i32 0, i32 0)
  br label %378

80:                                               ; preds = %76
  %81 = icmp sgt i32 %73, %1
  br i1 %81, label %150, label %82

82:                                               ; preds = %80
  %83 = add nsw i32 %73, 985
  %84 = icmp sgt i32 %83, %1
  br i1 %84, label %87, label %85

85:                                               ; preds = %82
  %86 = load i32, i32* %7, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %83, i32 %1, i32 %2, i32 %86, i32 0, i32 0, i32 0, i32 0)
  br label %378

87:                                               ; preds = %82
  %88 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8
  %89 = getelementptr inbounds [13 x i32], [13 x i32]* %88, i64 0, i64 0
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %91 = load i32, i32* %90, align 4
  %92 = icmp eq i32 %91, 0
  br i1 %92, label %120, label %93

93:                                               ; preds = %87
  %94 = getelementptr inbounds i32, i32* %89, i64 10
  %95 = load i32, i32* %94, align 4
  %96 = icmp eq i32 %95, 0
  br i1 %96, label %97, label %150

97:                                               ; preds = %93
  %98 = getelementptr inbounds i32, i32* %89, i64 8
  %99 = load i32, i32* %98, align 4
  %100 = icmp eq i32 %99, 0
  br i1 %100, label %101, label %115

101:                                              ; preds = %97
  %102 = getelementptr inbounds i32, i32* %89, i64 12
  %103 = load i32, i32* %102, align 4
  %104 = icmp eq i32 %103, 0
  br i1 %104, label %105, label %112

105:                                              ; preds = %101
  %106 = getelementptr inbounds i32, i32* %89, i64 4
  %107 = load i32, i32* %106, align 4
  %108 = icmp eq i32 %107, 0
  br i1 %108, label %109, label %112

109:                                              ; preds = %105
  %110 = add nsw i32 %73, 135
  %111 = icmp sgt i32 %110, %1
  br i1 %111, label %150, label %378

112:                                              ; preds = %105, %101
  %113 = add nsw i32 %73, 380
  %114 = icmp sgt i32 %113, %1
  br i1 %114, label %150, label %378

115:                                              ; preds = %97
  %116 = add nsw i32 %73, 540
  %117 = icmp sgt i32 %116, %1
  br i1 %117, label %150, label %118

118:                                              ; preds = %115
  %119 = load i32, i32* %7, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %116, i32 %1, i32 %2, i32 %119, i32 0, i32 0, i32 0, i32 0)
  br label %378

120:                                              ; preds = %87
  %121 = getelementptr inbounds i32, i32* %89, i64 9
  %122 = load i32, i32* %121, align 4
  %123 = icmp eq i32 %122, 0
  br i1 %123, label %124, label %150

124:                                              ; preds = %120
  %125 = getelementptr inbounds i32, i32* %89, i64 7
  %126 = load i32, i32* %125, align 4
  %127 = icmp eq i32 %126, 0
  br i1 %127, label %128, label %142

128:                                              ; preds = %124
  %129 = getelementptr inbounds i32, i32* %89, i64 11
  %130 = load i32, i32* %129, align 4
  %131 = icmp eq i32 %130, 0
  br i1 %131, label %132, label %139

132:                                              ; preds = %128
  %133 = getelementptr inbounds i32, i32* %89, i64 3
  %134 = load i32, i32* %133, align 4
  %135 = icmp eq i32 %134, 0
  br i1 %135, label %136, label %139

136:                                              ; preds = %132
  %137 = add nsw i32 %73, 135
  %138 = icmp sgt i32 %137, %1
  br i1 %138, label %150, label %378

139:                                              ; preds = %132, %128
  %140 = add nsw i32 %73, 380
  %141 = icmp sgt i32 %140, %1
  br i1 %141, label %150, label %378

142:                                              ; preds = %124
  %143 = add nsw i32 %73, 540
  %144 = icmp sgt i32 %143, %1
  br i1 %144, label %150, label %145

145:                                              ; preds = %142
  %146 = load i32, i32* %7, align 4
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %143, i32 %1, i32 %2, i32 %146, i32 0, i32 0, i32 0, i32 0)
  br label %378

147:                                              ; preds = %68
  %148 = load i32, i32* %7, align 4
  %149 = icmp sgt i32 %3, -6
  br i1 %149, label %155, label %164

150:                                              ; preds = %142, %139, %136, %120, %115, %112, %109, %93, %80
  %151 = phi i32 [ %1, %120 ], [ %1, %142 ], [ %1, %136 ], [ %1, %139 ], [ %1, %93 ], [ %1, %115 ], [ %1, %109 ], [ %1, %112 ], [ %73, %80 ]
  %152 = sub nsw i32 %151, %74
  %153 = load i32, i32* %7, align 4
  %154 = icmp sgt i32 %3, -6
  br i1 %154, label %158, label %161

155:                                              ; preds = %147
  %156 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 0
  %157 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %156, i32 %72)
  br label %167

158:                                              ; preds = %150
  %159 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 0
  %160 = call i32 @_Z12gen_capturesP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %159)
  br label %167

161:                                              ; preds = %150
  %162 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 0
  %163 = call i32 @_Z12gen_capturesP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %162)
  br label %167

164:                                              ; preds = %147
  %165 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 0
  %166 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %165, i32 %72)
  br label %167

167:                                              ; preds = %164, %161, %158, %155
  %168 = phi i32 [ %153, %161 ], [ %148, %164 ], [ %148, %155 ], [ %153, %158 ]
  %169 = phi i32 [ %152, %161 ], [ 0, %164 ], [ 0, %155 ], [ %152, %158 ]
  %170 = phi i32 [ %151, %161 ], [ %1, %164 ], [ %1, %155 ], [ %151, %158 ]
  %171 = phi i1 [ false, %161 ], [ false, %164 ], [ false, %155 ], [ true, %158 ]
  %172 = phi i32 [ %163, %161 ], [ %166, %164 ], [ %157, %155 ], [ %160, %158 ]
  %173 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 0
  %174 = getelementptr inbounds [240 x i32], [240 x i32]* %11, i64 0, i64 0
  %175 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1
  %176 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 0
  %177 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11
  %178 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  %179 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36
  %180 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19
  %181 = sub nsw i32 0, %2
  %182 = add nsw i32 %4, 1
  %183 = icmp sgt i32 %3, -1
  br label %184

184:                                              ; preds = %363, %167
  %185 = phi i32 [ %168, %167 ], [ %203, %363 ]
  %186 = phi i32 [ 1, %167 ], [ %204, %363 ]
  %187 = phi i32 [ -32000, %167 ], [ %205, %363 ]
  %188 = phi i1 [ false, %167 ], [ %362, %363 ]
  %189 = phi i1 [ false, %167 ], [ %364, %363 ]
  %190 = phi i1 [ true, %167 ], [ false, %363 ]
  %191 = phi i32 [ %172, %167 ], [ %199, %363 ]
  %192 = phi i32 [ %170, %167 ], [ %206, %363 ]
  br i1 %188, label %193, label %195

193:                                              ; preds = %184
  %194 = call i32 @_Z15gen_good_checksP7state_tPi(%struct.state_t* %0, i32* nonnull %173)
  br label %198

195:                                              ; preds = %184
  br i1 %189, label %196, label %198

196:                                              ; preds = %195
  %197 = call i32 @_Z9gen_quietP7state_tPi(%struct.state_t* %0, i32* nonnull %173)
  br label %198

198:                                              ; preds = %196, %195, %193
  %199 = phi i32 [ %194, %193 ], [ %197, %196 ], [ %191, %195 ]
  %200 = load i32, i32* %7, align 4
  call fastcc void @_ZL16fast_order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %173, i32* nonnull %174, i32 %199, i32 %200)
  store i32 -1, i32* %6, align 4
  %201 = or i1 %188, %189
  br label %202

202:                                              ; preds = %358, %198
  %203 = phi i32 [ %359, %358 ], [ %185, %198 ]
  %204 = phi i32 [ %344, %358 ], [ %186, %198 ]
  %205 = phi i32 [ %345, %358 ], [ %187, %198 ]
  %206 = phi i32 [ %360, %358 ], [ %192, %198 ]
  br i1 %75, label %207, label %212

207:                                              ; preds = %202
  %208 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %174, i32* nonnull %173, i32 %199)
  %209 = icmp eq i32 %208, 0
  br i1 %209, label %361, label %210

210:                                              ; preds = %207
  %211 = load i32, i32* %6, align 4
  br label %304

212:                                              ; preds = %246, %202
  %213 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %174, i32* nonnull %173, i32 %199)
  %214 = icmp eq i32 %213, 0
  br i1 %214, label %361, label %215

215:                                              ; preds = %212
  br i1 %190, label %216, label %231

216:                                              ; preds = %215
  %217 = load i32, i32* %6, align 4
  %218 = sext i32 %217 to i64
  %219 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %218
  %220 = load i32, i32* %219, align 4
  %221 = lshr i32 %220, 19
  %222 = and i32 %221, 15
  %223 = zext i32 %222 to i64
  %224 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %223
  %225 = load i32, i32* %224, align 4
  %226 = call i32 @llvm.abs.i32(i32 %225, i1 true)
  %227 = icmp sle i32 %226, %169
  %228 = and i32 %220, 61440
  %229 = icmp eq i32 %228, 0
  %230 = select i1 %227, i1 %229, i1 false
  br i1 %230, label %361, label %231

231:                                              ; preds = %216, %215
  br i1 %201, label %232, label %270

232:                                              ; preds = %231
  %233 = load i32, i32* %6, align 4
  %234 = sext i32 %233 to i64
  %235 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %234
  %236 = load i32, i32* %235, align 4
  %237 = lshr i32 %236, 19
  %238 = and i32 %237, 15
  %239 = icmp eq i32 %238, 13
  br i1 %239, label %247, label %240

240:                                              ; preds = %232
  %241 = zext i32 %238 to i64
  %242 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %241
  %243 = load i32, i32* %242, align 4
  %244 = call i32 @llvm.abs.i32(i32 %243, i1 true)
  %245 = icmp sgt i32 %244, %169
  br i1 %245, label %246, label %247

246:                                              ; preds = %291, %248, %240
  br label %212, !llvm.loop !52

247:                                              ; preds = %240, %232
  br i1 %189, label %248, label %291

248:                                              ; preds = %247
  %249 = lshr i32 %236, 6
  %250 = and i32 %249, 63
  %251 = zext i32 %250 to i64
  %252 = getelementptr inbounds [64 x i32], [64 x i32]* %175, i64 0, i64 %251
  %253 = load i32, i32* %252, align 4
  %254 = add nsw i32 %253, -1
  %255 = and i32 %236, 63
  %256 = load i32, i32* %176, align 8
  %257 = sext i32 %256 to i64
  %258 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %257
  %259 = sext i32 %254 to i64
  %260 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %258, i64 0, i64 %259
  %261 = zext i32 %255 to i64
  %262 = getelementptr inbounds [64 x i32], [64 x i32]* %260, i64 0, i64 %261
  %263 = load i32, i32* %262, align 4
  %264 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %257
  %265 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %264, i64 0, i64 %259
  %266 = getelementptr inbounds [64 x i32], [64 x i32]* %265, i64 0, i64 %261
  %267 = load i32, i32* %266, align 4
  %268 = sub nsw i32 %267, %263
  %269 = icmp slt i32 %263, %268
  br i1 %269, label %246, label %291

270:                                              ; preds = %231
  %271 = load i32, i32* %6, align 4
  %272 = sext i32 %271 to i64
  %273 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %272
  %274 = load i32, i32* %273, align 4
  %275 = lshr i32 %274, 19
  %276 = and i32 %275, 15
  %277 = zext i32 %276 to i64
  %278 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %277
  %279 = load i32, i32* %278, align 4
  %280 = call i32 @llvm.abs.i32(i32 %279, i1 true)
  %281 = lshr i32 %274, 6
  %282 = and i32 %281, 63
  %283 = zext i32 %282 to i64
  %284 = getelementptr inbounds [64 x i32], [64 x i32]* %175, i64 0, i64 %283
  %285 = load i32, i32* %284, align 4
  %286 = sext i32 %285 to i64
  %287 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %286
  %288 = load i32, i32* %287, align 4
  %289 = call i32 @llvm.abs.i32(i32 %288, i1 true)
  %290 = icmp ult i32 %280, %289
  br i1 %290, label %291, label %304

291:                                              ; preds = %270, %248, %247
  %292 = phi i64 [ %234, %247 ], [ %272, %270 ], [ %234, %248 ]
  %293 = phi i32 [ %233, %247 ], [ %271, %270 ], [ %233, %248 ]
  %294 = load i32, i32* %177, align 4
  %295 = icmp eq i32 %294, 0
  %296 = zext i1 %295 to i32
  %297 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %292
  %298 = load i32, i32* %297, align 4
  %299 = lshr i32 %298, 6
  %300 = and i32 %299, 63
  %301 = and i32 %298, 63
  %302 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* %0, i32 %296, i32 %300, i32 %301, i32 0)
  %303 = icmp slt i32 %302, -50
  br i1 %303, label %246, label %304

304:                                              ; preds = %291, %270, %210
  %305 = phi i32 [ %211, %210 ], [ %293, %291 ], [ %271, %270 ]
  %306 = sext i32 %305 to i64
  %307 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %306
  %308 = load i32, i32* %307, align 4
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %308)
  %309 = load i32, i32* %307, align 4
  %310 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %309)
  %311 = icmp eq i32 %310, 0
  br i1 %311, label %343, label %312
; *** insert position BB ***
312:                                              ; preds = %304
  %313 = load i64, i64* %178, align 8
  %314 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4
  %315 = load i32, i32* %24, align 8
  %316 = add i32 %315, -1
  %317 = add i32 %316, %314
  %318 = sext i32 %317 to i64
  %319 = getelementptr inbounds [1000 x i64], [1000 x i64]* %179, i64 0, i64 %318
  store i64 %313, i64* %319, align 8
  %320 = load i32, i32* %307, align 4
  %321 = sext i32 %316 to i64
  %322 = getelementptr inbounds [64 x i32], [64 x i32]* %180, i64 0, i64 %321
  store i32 %320, i32* %322, align 4
  %323 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0)
  %324 = load i32, i32* %24, align 8
  %325 = sext i32 %324 to i64
  %326 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %325
  store i32 %323, i32* %326, align 4
  %327 = sub nsw i32 0, %206
  %328 = sub i32 60, %206
  %329 = icmp ne i32 %323, 0
  %330 = zext i1 %329 to i32
  %331 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %181, i32 %328, i32 %330)
  br i1 %189, label %332, label %335

332:                                              ; preds = %312
  %333 = sub nsw i32 0, %331
  %334 = icmp slt i32 %206, %333
  br i1 %334, label %339, label %343

335:                                              ; preds = %312
  %336 = select i1 %329, i1 true, i1 %75
  %337 = select i1 %336, i32 -1, i32 -8
  %338 = add nsw i32 %337, %3
  br label %339

339:                                              ; preds = %335, %332
  %340 = phi i32 [ %3, %332 ], [ %338, %335 ]
  %341 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %181, i32 %327, i32 %340, i32 %182)
  %342 = sub nsw i32 0, %341
  br label %343

343:                                              ; preds = %339, %332, %304
  %344 = phi i32 [ %204, %304 ], [ 0, %339 ], [ 0, %332 ]
  %345 = phi i32 [ %205, %304 ], [ %342, %339 ], [ %205, %332 ]
  %346 = load i32, i32* %307, align 4
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %346)
  %347 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8
  %348 = icmp eq i32 %347, 0
  br i1 %348, label %349, label %378

349:                                              ; preds = %343
  %350 = icmp sle i32 %345, %206
  %351 = or i1 %311, %350
  br i1 %351, label %358, label %352

352:                                              ; preds = %349
  %353 = load i32, i32* %307, align 4
  %354 = call zeroext i16 @_Z12compact_movei(i32 %353)
  %355 = zext i16 %354 to i32
  %356 = icmp slt i32 %345, %2
  br i1 %356, label %358, label %357

357:                                              ; preds = %352
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %345, i32 %1, i32 %2, i32 %355, i32 0, i32 0, i32 0, i32 0)
  br label %378

358:                                              ; preds = %352, %349
  %359 = phi i32 [ %203, %349 ], [ %355, %352 ]
  %360 = phi i32 [ %206, %349 ], [ %345, %352 ]
  br label %202, !llvm.loop !52

361:                                              ; preds = %216, %212, %207
  %362 = and i1 %171, %190
  br i1 %362, label %363, label %365

363:                                              ; preds = %365, %361
  %364 = xor i1 %362, true
  br label %184

365:                                              ; preds = %361
  %366 = and i1 %171, %188
  %367 = and i1 %366, %183
  %368 = icmp sgt i32 %74, %206
  %369 = select i1 %367, i1 %368, i1 false
  br i1 %369, label %363, label %370

370:                                              ; preds = %365
  %371 = icmp ne i32 %204, 0
  %372 = select i1 %371, i1 %75, i1 false
  br i1 %372, label %373, label %376

373:                                              ; preds = %370
  %374 = load i32, i32* %24, align 8
  %375 = add nsw i32 %374, -32000
  br label %376

376:                                              ; preds = %373, %370
  %377 = phi i32 [ %375, %373 ], [ %206, %370 ]
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %377, i32 %1, i32 %2, i32 %203, i32 0, i32 0, i32 0, i32 0)
  br label %378

378:                                              ; preds = %376, %357, %343, %145, %139, %136, %118, %112, %109, %85, %78, %66, %55, %52, %50, %40, %30
  %379 = phi i32 [ %47, %40 ], [ %67, %66 ], [ %345, %357 ], [ %377, %376 ], [ %73, %78 ], [ %83, %85 ], [ %51, %50 ], [ 0, %30 ], [ %53, %52 ], [ %56, %55 ], [ %140, %139 ], [ %137, %136 ], [ %113, %112 ], [ %110, %109 ], [ %116, %118 ], [ %143, %145 ], [ 0, %343 ]
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %17) #7
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %16) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %15) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %14) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %13) #7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %12) #7
  ret i32 %379
}

attributes #0 = { uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { nofree norecurse nosync nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #2 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { nofree norecurse nosync nounwind uwtable willreturn mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #4 = { norecurse nosync nounwind readonly uwtable willreturn mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #5 = { argmemonly nofree nosync nounwind willreturn }
attributes #6 = { nosync nounwind readnone uwtable willreturn mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
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
!50 = distinct !{!50, !31}
!51 = distinct !{!51, !31}
!52 = distinct !{!52, !31}

; end INTEL_FEATURE_SW_ADVANCED
