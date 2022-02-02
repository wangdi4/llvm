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
; CHECK: define internal i32 @_Z6searchP7state_tiiiii(ptr %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5) #{{.}} {
; CHECK: call void @Prefetch.Backbone(ptr %0)
;

; *** Check section 2 ***
; The LLVM-IR check below ensures that a call to the Prefetch.Backbone function is inserted inside host
; _Z7qsearchP7state_tiiii.
; CHECK:define internal i32 @_Z7qsearchP7state_tiiii(ptr %0, i32 %1, i32 %2, i32 %3, i32 %4) #{{.}} {
; CHECK: call void @Prefetch.Backbone(ptr %0)
;

; *** Check section 3 ***
; The LLVM-IR check below ensures the prefetch function is generated.
; CHECK: define internal void @Prefetch.Backbone(ptr nocapture %0) #{{.}} {
;


; ModuleID = 'intel_ipo_prefetch6.ll'
source_filename = "<stdin>"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.pawntt_t = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i32, i32 }
%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
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
@stdout = external dso_local local_unnamed_addr global ptr, align 8
@stdin = external dso_local local_unnamed_addr global ptr, align 8
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
@TTable = internal global ptr null, align 8
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
@__const._Z13display_boardP7state_ti.piece_rep = private unnamed_addr constant [14 x ptr] [ptr getelementptr inbounds ([3 x i8], ptr @.str.20, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.21, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.22, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.23, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.24, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.25, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.26, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.27, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.28, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.29, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.30, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.31, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.32, i32 0, i32 0), ptr getelementptr inbounds ([3 x i8], ptr @.str.33, i32 0, i32 0)], align 16
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
define internal i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(ptr nocapture %0, ptr nocapture %1, i32 %2, i32 %3, ptr nocapture %4, ptr nocapture %5, ptr nocapture %6, ptr nocapture %7, ptr nocapture %8, i32 %9) #1 {
  store i32 1, ptr %6, align 4, !tbaa !6
  %11 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 26, !intel-tbaa !10
  %12 = load i32, ptr %11, align 4, !tbaa !10
  %13 = add i32 %12, 1
  store i32 %13, ptr %11, align 4, !tbaa !10
  %14 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 11, !intel-tbaa !21
  %15 = load i32, ptr %14, align 4, !tbaa !21
  %16 = icmp eq i32 %15, 0
  %17 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 16
  %18 = load i64, ptr %17, align 8, !tbaa !22
  %19 = zext i1 %16 to i64
  %20 = add i64 %18, %19
  %21 = trunc i64 %20 to i32
  %22 = load ptr, ptr @TTable, align 8, !tbaa !23
  %23 = load i32, ptr @TTSize, align 4, !tbaa !6
  %24 = urem i32 %21, %23
  %25 = zext i32 %24 to i64
  %26 = getelementptr inbounds %struct.ttentry_t, ptr %22, i64 %25
  %27 = lshr i64 %20, 32
  %28 = getelementptr inbounds %struct.ttentry_t, ptr %26, i64 0, i32 0, !intel-tbaa !25
  %29 = trunc i64 %27 to i32
  br label %33

30:                                               ; preds = %33
  %31 = add nuw nsw i64 %34, 1
  %32 = icmp eq i64 %31, 4
  br i1 %32, label %132, label %33, !llvm.loop !30

33:                                               ; preds = %30, %10
  %34 = phi i64 [ 0, %10 ], [ %31, %30 ]
  %35 = getelementptr inbounds [4 x %struct.ttbucket_t], ptr %28, i64 0, i64 %34, !intel-tbaa !32
  %36 = getelementptr inbounds %struct.ttbucket_t, ptr %35, i64 0, i32 0, !intel-tbaa !33
  %37 = load i32, ptr %36, align 4, !tbaa !34
  %38 = icmp eq i32 %37, %29
  br i1 %38, label %39, label %30

39:                                               ; preds = %33
  %40 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 27, !intel-tbaa !35
  %41 = load i32, ptr %40, align 8, !tbaa !35
  %42 = add i32 %41, 1
  store i32 %42, ptr %40, align 8, !tbaa !35
  %43 = getelementptr inbounds %struct.ttbucket_t, ptr %35, i64 0, i32 4
  %44 = load i8, ptr %43, align 1, !tbaa !36
  %45 = lshr i8 %44, 5
  %46 = and i8 %45, 3
  %47 = zext i8 %46 to i32
  %48 = load i32, ptr @TTAge, align 4, !tbaa !6
  %49 = icmp eq i32 %48, %47
  br i1 %49, label %56, label %50

50:                                               ; preds = %39
  %51 = trunc i32 %48 to i8
  %52 = shl i8 %51, 5
  %53 = and i8 %52, 96
  %54 = and i8 %44, -97
  %55 = or i8 %53, %54
  store i8 %55, ptr %43, align 1, !tbaa !36
  br label %56

56:                                               ; preds = %50, %39
  %57 = phi i8 [ %55, %50 ], [ %44, %39 ]
  %58 = and i8 %57, 6
  %59 = icmp eq i8 %58, 2
  br i1 %59, label %60, label %72

60:                                               ; preds = %56
  %61 = add nsw i32 %9, -16
  %62 = getelementptr inbounds %struct.ttbucket_t, ptr %35, i64 0, i32 3, !intel-tbaa !37
  %63 = load i8, ptr %62, align 4, !tbaa !38
  %64 = zext i8 %63 to i32
  %65 = icmp sgt i32 %61, %64
  br i1 %65, label %72, label %66

66:                                               ; preds = %60
  %67 = getelementptr inbounds %struct.ttbucket_t, ptr %35, i64 0, i32 1, !intel-tbaa !39
  %68 = load i16, ptr %67, align 4, !tbaa !40
  %69 = sext i16 %68 to i32
  %70 = icmp slt i32 %69, %3
  br i1 %70, label %71, label %72

71:                                               ; preds = %66
  store i32 0, ptr %6, align 4, !tbaa !6
  br label %72

72:                                               ; preds = %71, %66, %60, %56
  %73 = getelementptr inbounds %struct.ttbucket_t, ptr %35, i64 0, i32 3, !intel-tbaa !37
  %74 = load i8, ptr %73, align 4, !tbaa !38
  %75 = zext i8 %74 to i32
  %76 = icmp slt i32 %75, %9
  br i1 %76, label %109, label %77

77:                                               ; preds = %72
  %78 = getelementptr inbounds %struct.ttbucket_t, ptr %35, i64 0, i32 1, !intel-tbaa !39
  %79 = load i16, ptr %78, align 4, !tbaa !40
  %80 = sext i16 %79 to i32
  store i32 %80, ptr %1, align 4, !tbaa !6
  %81 = icmp sgt i16 %79, 31500
  br i1 %81, label %82, label %87

82:                                               ; preds = %77
  %83 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 14, !intel-tbaa !41
  %84 = load i32, ptr %83, align 8, !tbaa !41
  %85 = add nsw i32 %80, 1
  %86 = sub i32 %85, %84
  store i32 %86, ptr %1, align 4, !tbaa !6
  br label %94

87:                                               ; preds = %77
  %88 = icmp slt i16 %79, -31500
  br i1 %88, label %89, label %94

89:                                               ; preds = %87
  %90 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 14, !intel-tbaa !41
  %91 = load i32, ptr %90, align 8, !tbaa !41
  %92 = add nsw i32 %80, -1
  %93 = add i32 %92, %91
  store i32 %93, ptr %1, align 4, !tbaa !6
  br label %94

94:                                               ; preds = %89, %87, %82
  %95 = getelementptr inbounds %struct.ttbucket_t, ptr %35, i64 0, i32 2, !intel-tbaa !42
  %96 = load i16, ptr %95, align 2, !tbaa !43
  %97 = zext i16 %96 to i32
  store i32 %97, ptr %4, align 4, !tbaa !6
  %98 = and i8 %57, 1
  %99 = zext i8 %98 to i32
  store i32 %99, ptr %5, align 4, !tbaa !6
  %100 = lshr i8 %57, 3
  %101 = and i8 %100, 1
  %102 = zext i8 %101 to i32
  store i32 %102, ptr %7, align 4, !tbaa !6
  %103 = lshr i8 %57, 4
  %104 = and i8 %103, 1
  %105 = zext i8 %104 to i32
  store i32 %105, ptr %8, align 4, !tbaa !6
  %106 = lshr i8 %57, 1
  %107 = and i8 %106, 3
  %108 = zext i8 %107 to i32
  br label %132

109:                                              ; preds = %72
  %110 = getelementptr inbounds %struct.ttbucket_t, ptr %35, i64 0, i32 2, !intel-tbaa !42
  %111 = load i16, ptr %110, align 2, !tbaa !43
  %112 = zext i16 %111 to i32
  store i32 %112, ptr %4, align 4, !tbaa !6
  %113 = and i8 %57, 1
  %114 = zext i8 %113 to i32
  store i32 %114, ptr %5, align 4, !tbaa !6
  %115 = lshr i8 %57, 3
  %116 = and i8 %115, 1
  %117 = zext i8 %116 to i32
  store i32 %117, ptr %7, align 4, !tbaa !6
  %118 = lshr i8 %57, 4
  %119 = and i8 %118, 1
  %120 = zext i8 %119 to i32
  store i32 %120, ptr %8, align 4, !tbaa !6
  %121 = lshr i8 %57, 1
  %122 = and i8 %121, 3
  switch i8 %122, label %125 [
    i8 1, label %123
    i8 2, label %124
  ]

123:                                              ; preds = %109
  store i32 -1000000, ptr %1, align 4, !tbaa !6
  br label %129

124:                                              ; preds = %109
  store i32 1000000, ptr %1, align 4, !tbaa !6
  br label %129

125:                                              ; preds = %109
  %126 = getelementptr inbounds %struct.ttbucket_t, ptr %35, i64 0, i32 1, !intel-tbaa !39
  %127 = load i16, ptr %126, align 4, !tbaa !40
  %128 = sext i16 %127 to i32
  store i32 %128, ptr %1, align 4, !tbaa !6
  br label %129

129:                                              ; preds = %125, %124, %123
  %130 = trunc i32 %9 to i8
  store i8 %130, ptr %73, align 4, !tbaa !38
  %131 = and i8 %57, -7
  store i8 %131, ptr %43, align 1, !tbaa !36
  br label %132

132:                                              ; preds = %129, %94, %30
  %133 = phi i32 [ %108, %94 ], [ 0, %129 ], [ 4, %30 ]
  ret i32 %133
}

; Function Attrs: mustprogress uwtable
define internal i32 @_Z6searchP7state_tiiiii(ptr %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5) #0 {
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
  %19 = bitcast ptr %7 to ptr
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %19) #7
  %20 = bitcast ptr %8 to ptr
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %20) #7
  %21 = bitcast ptr %9 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %21) #7
  %22 = bitcast ptr %10 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %22) #7
  %23 = bitcast ptr %11 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %23) #7
  %24 = bitcast ptr %12 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %24) #7
  %25 = bitcast ptr %13 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %25) #7
  %26 = bitcast ptr %14 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %26) #7
  %27 = bitcast ptr %15 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %27) #7
  %28 = bitcast ptr %16 to ptr
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %28) #7
  %29 = icmp slt i32 %3, 1
  br i1 %29, label %34, label %30

30:                                               ; preds = %6
  %31 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 14, !intel-tbaa !41
  %32 = load i32, ptr %31, align 8, !tbaa !41
  %33 = icmp sgt i32 %32, 59
  br i1 %33, label %34, label %36

34:                                               ; preds = %30, %6
  %35 = tail call i32 @_Z7qsearchP7state_tiiii(ptr %0, i32 %1, i32 %2, i32 0, i32 0)
  br label %1056

36:                                               ; preds = %30
  %37 = getelementptr %struct.state_t, ptr %0, i64 0, i32 22, !intel-tbaa !44
  %38 = load i64, ptr %37, align 8, !tbaa !44
  %39 = add i64 %38, 1
  store i64 %39, ptr %37, align 8, !tbaa !44
  %40 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %39)
  %41 = icmp eq i32 %40, 0
  br i1 %41, label %42, label %1056

42:                                               ; preds = %36
  %43 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(ptr nonnull @gamestate, ptr nonnull %0)
  %44 = icmp eq i32 %43, 0
  br i1 %44, label %45, label %49

45:                                               ; preds = %42
  %46 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 15, !intel-tbaa !45
  %47 = load i32, ptr %46, align 4, !tbaa !45
  %48 = icmp sgt i32 %47, 99
  br i1 %48, label %49, label %57

49:                                               ; preds = %45, %42
  %50 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 3), align 4, !tbaa !46
  %51 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 11, !intel-tbaa !21
  %52 = load i32, ptr %51, align 4, !tbaa !21
  %53 = icmp eq i32 %50, %52
  %54 = load i32, ptr @contempt, align 4
  %55 = sub nsw i32 0, %54
  %56 = select i1 %53, i32 %54, i32 %55
  br label %1056

57:                                               ; preds = %45
  %58 = load i32, ptr %31, align 8, !tbaa !41
  %59 = add nsw i32 %58, -32000
  %60 = icmp sgt i32 %59, %1
  br i1 %60, label %61, label %63

61:                                               ; preds = %57
  %62 = icmp slt i32 %59, %2
  br i1 %62, label %63, label %1056

63:                                               ; preds = %61, %57
  %64 = phi i32 [ %59, %61 ], [ %1, %57 ]
  %65 = sub i32 31999, %58
  %66 = icmp slt i32 %65, %2
  br i1 %66, label %67, label %69

67:                                               ; preds = %63
  %68 = icmp sgt i32 %65, %64
  br i1 %68, label %69, label %1056

69:                                               ; preds = %67, %63
  %70 = phi i32 [ %65, %67 ], [ %2, %63 ]
  %71 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(ptr nonnull %0, ptr nonnull %10, i32 %64, i32 %70, ptr nonnull %13, ptr nonnull %11, ptr nonnull %12, ptr nonnull %14, ptr nonnull %15, i32 %3)
  switch i32 %71, label %85 [
    i32 3, label %72
    i32 1, label %74
    i32 2, label %77
    i32 0, label %80
    i32 4, label %84
  ]

72:                                               ; preds = %69
  %73 = load i32, ptr %10, align 4, !tbaa !6
  br label %1056

74:                                               ; preds = %69
  %75 = load i32, ptr %10, align 4, !tbaa !6
  %76 = icmp sgt i32 %75, %64
  br i1 %76, label %85, label %1056

77:                                               ; preds = %69
  %78 = load i32, ptr %10, align 4, !tbaa !6
  %79 = icmp slt i32 %78, %70
  br i1 %79, label %85, label %1056

80:                                               ; preds = %69
  %81 = load i32, ptr %10, align 4, !tbaa !6
  %82 = icmp slt i32 %81, %70
  %83 = select i1 %82, i32 %5, i32 1
  br label %85

84:                                               ; preds = %69
  store i32 65535, ptr %13, align 4, !tbaa !6
  store i32 0, ptr %11, align 4, !tbaa !6
  store i32 0, ptr %14, align 4, !tbaa !6
  store i32 0, ptr %15, align 4, !tbaa !6
  br label %85

85:                                               ; preds = %84, %80, %77, %74, %69
  %86 = phi i32 [ %5, %69 ], [ %5, %84 ], [ 0, %74 ], [ 1, %77 ], [ %83, %80 ]
  %87 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 25, !intel-tbaa !50
  %88 = load i32, ptr %31, align 8, !tbaa !41
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds [64 x i32], ptr %87, i64 0, i64 %89, !intel-tbaa !51
  %91 = load i32, ptr %90, align 4, !tbaa !52
  %92 = call i32 @_Z13retrieve_evalP7state_t(ptr nonnull %0)
  %93 = icmp ne i32 %91, 0
  %94 = xor i1 %93, true
  %95 = add nsw i32 %64, 1
  %96 = icmp eq i32 %70, %95
  %97 = select i1 %94, i1 %96, i1 false
  br i1 %97, label %98, label %122

98:                                               ; preds = %85
  %99 = icmp slt i32 %3, 5
  br i1 %99, label %100, label %112

100:                                              ; preds = %98
  %101 = add nsw i32 %92, -75
  %102 = icmp slt i32 %101, %70
  br i1 %102, label %108, label %103

103:                                              ; preds = %100
  %104 = load i32, ptr %13, align 4, !tbaa !6
  %105 = load i32, ptr %11, align 4, !tbaa !6
  %106 = load i32, ptr %14, align 4, !tbaa !6
  %107 = load i32, ptr %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr nonnull %0, i32 %101, i32 %64, i32 %70, i32 %104, i32 %105, i32 %106, i32 %107, i32 %3)
  br label %1056

108:                                              ; preds = %100
  %109 = icmp slt i32 %92, %70
  br i1 %109, label %110, label %122

110:                                              ; preds = %108
  %111 = call i32 @_Z7qsearchP7state_tiiii(ptr nonnull %0, i32 %64, i32 %70, i32 0, i32 0)
  br label %1056

112:                                              ; preds = %98
  %113 = icmp sgt i32 %3, 8
  %114 = add nsw i32 %92, -125
  %115 = icmp slt i32 %114, %70
  %116 = select i1 %113, i1 true, i1 %115
  br i1 %116, label %122, label %117

117:                                              ; preds = %112
  %118 = load i32, ptr %13, align 4, !tbaa !6
  %119 = load i32, ptr %11, align 4, !tbaa !6
  %120 = load i32, ptr %14, align 4, !tbaa !6
  %121 = load i32, ptr %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr nonnull %0, i32 %114, i32 %64, i32 %70, i32 %118, i32 %119, i32 %120, i32 %121, i32 %3)
  br label %1056

122:                                              ; preds = %112, %108, %85
  %123 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 8, !intel-tbaa !53
  %124 = getelementptr inbounds [13 x i32], ptr %123, i64 0, i64 9, !intel-tbaa !54
  %125 = load i32, ptr %124, align 4, !tbaa !55
  %126 = getelementptr inbounds [13 x i32], ptr %123, i64 0, i64 7, !intel-tbaa !54
  %127 = load i32, ptr %126, align 4, !tbaa !55
  %128 = add nsw i32 %127, %125
  %129 = getelementptr inbounds [13 x i32], ptr %123, i64 0, i64 11, !intel-tbaa !54
  %130 = load i32, ptr %129, align 4, !tbaa !55
  %131 = add nsw i32 %128, %130
  %132 = getelementptr inbounds [13 x i32], ptr %123, i64 0, i64 3, !intel-tbaa !54
  %133 = load i32, ptr %132, align 4, !tbaa !55
  %134 = add nsw i32 %131, %133
  %135 = getelementptr inbounds [13 x i32], ptr %123, i64 0, i64 10, !intel-tbaa !54
  %136 = load i32, ptr %135, align 8, !tbaa !55
  %137 = getelementptr inbounds [13 x i32], ptr %123, i64 0, i64 8, !intel-tbaa !54
  %138 = load i32, ptr %137, align 8, !tbaa !55
  %139 = add nsw i32 %138, %136
  %140 = getelementptr inbounds [13 x i32], ptr %123, i64 0, i64 12, !intel-tbaa !54
  %141 = load i32, ptr %140, align 8, !tbaa !55
  %142 = add nsw i32 %139, %141
  %143 = getelementptr inbounds [13 x i32], ptr %123, i64 0, i64 4, !intel-tbaa !54
  %144 = load i32, ptr %143, align 8, !tbaa !55
  %145 = add nsw i32 %142, %144
  store i32 0, ptr %11, align 4, !tbaa !6
  %146 = icmp eq i32 %4, 0
  br i1 %146, label %147, label %231

147:                                              ; preds = %122
  %148 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 11, !intel-tbaa !21
  %149 = load i32, ptr %148, align 4, !tbaa !21
  %150 = icmp eq i32 %149, 0
  %151 = select i1 %150, i32 %145, i32 %134
  %152 = icmp eq i32 %151, 0
  %153 = select i1 %152, i1 true, i1 %93
  %154 = xor i1 %153, true
  %155 = load i32, ptr %12, align 4
  %156 = icmp ne i32 %155, 0
  %157 = select i1 %154, i1 %156, i1 false
  %158 = icmp sgt i32 %3, 4
  %159 = and i1 %96, %158
  %160 = and i1 %157, %159
  br i1 %160, label %161, label %231

161:                                              ; preds = %147
  %162 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 1), align 4, !tbaa !56
  %163 = icmp eq i32 %162, 2
  br i1 %163, label %164, label %183

164:                                              ; preds = %161
  %165 = icmp slt i32 %3, 25
  %166 = add nsw i32 %70, -1
  br i1 %165, label %167, label %169

167:                                              ; preds = %164
  %168 = call i32 @_Z7qsearchP7state_tiiii(ptr nonnull %0, i32 %166, i32 %70, i32 0, i32 0)
  br label %172

169:                                              ; preds = %164
  %170 = add nsw i32 %3, -24
  %171 = call i32 @_Z6searchP7state_tiiiii(ptr nonnull %0, i32 %166, i32 %70, i32 %170, i32 1, i32 %86)
  br label %172

172:                                              ; preds = %169, %167
  %173 = phi i32 [ %168, %167 ], [ %171, %169 ]
  %174 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8, !tbaa !57
  %175 = icmp eq i32 %174, 0
  br i1 %175, label %176, label %1056

176:                                              ; preds = %172
  %177 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 1), align 4, !tbaa !56
  %178 = icmp eq i32 %177, 2
  %179 = icmp slt i32 %173, %70
  %180 = and i1 %178, %179
  br i1 %180, label %248, label %181

181:                                              ; preds = %176
  %182 = load i32, ptr %148, align 4, !tbaa !21
  br label %183

183:                                              ; preds = %181, %161
  %184 = phi i32 [ %182, %181 ], [ %149, %161 ]
  %185 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 10, !intel-tbaa !58
  %186 = load i32, ptr %185, align 8, !tbaa !58
  store i32 0, ptr %185, align 8, !tbaa !58
  %187 = xor i32 %184, 1
  store i32 %187, ptr %148, align 4, !tbaa !21
  %188 = load i32, ptr %31, align 8, !tbaa !41
  %189 = add nsw i32 %188, 1
  store i32 %189, ptr %31, align 8, !tbaa !41
  %190 = load i32, ptr %46, align 4, !tbaa !45
  %191 = add nsw i32 %190, 1
  store i32 %191, ptr %46, align 4, !tbaa !45
  %192 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 19, !intel-tbaa !59
  %193 = sext i32 %188 to i64
  %194 = getelementptr inbounds [64 x i32], ptr %192, i64 0, i64 %193, !intel-tbaa !51
  store i32 0, ptr %194, align 4, !tbaa !60
  %195 = sext i32 %189 to i64
  %196 = getelementptr inbounds [64 x i32], ptr %87, i64 0, i64 %195, !intel-tbaa !51
  store i32 0, ptr %196, align 4, !tbaa !52
  %197 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 20, !intel-tbaa !61
  %198 = getelementptr inbounds [64 x i32], ptr %197, i64 0, i64 %193, !intel-tbaa !51
  %199 = load i32, ptr %198, align 4, !tbaa !62
  %200 = getelementptr inbounds [64 x i32], ptr %197, i64 0, i64 %195, !intel-tbaa !51
  store i32 %199, ptr %200, align 4, !tbaa !62
  %201 = icmp slt i32 %3, 17
  %202 = sub nsw i32 0, %70
  %203 = sub i32 1, %70
  br i1 %201, label %204, label %206

204:                                              ; preds = %183
  %205 = call i32 @_Z7qsearchP7state_tiiii(ptr nonnull %0, i32 %202, i32 %203, i32 0, i32 0)
  br label %211

206:                                              ; preds = %183
  %207 = add nsw i32 %3, -16
  %208 = icmp eq i32 %86, 0
  %209 = zext i1 %208 to i32
  %210 = call i32 @_Z6searchP7state_tiiiii(ptr nonnull %0, i32 %202, i32 %203, i32 %207, i32 1, i32 %209)
  br label %211

211:                                              ; preds = %206, %204
  %212 = phi i32 [ %205, %204 ], [ %210, %206 ]
  %213 = sub nsw i32 0, %212
  %214 = load i32, ptr %46, align 4, !tbaa !45
  %215 = add nsw i32 %214, -1
  store i32 %215, ptr %46, align 4, !tbaa !45
  %216 = load i32, ptr %31, align 8, !tbaa !41
  %217 = add nsw i32 %216, -1
  store i32 %217, ptr %31, align 8, !tbaa !41
  %218 = load i32, ptr %148, align 4, !tbaa !21
  %219 = xor i32 %218, 1
  store i32 %219, ptr %148, align 4, !tbaa !21
  store i32 %186, ptr %185, align 8, !tbaa !58
  %220 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8, !tbaa !57
  %221 = icmp eq i32 %220, 0
  br i1 %221, label %222, label %1056

222:                                              ; preds = %211
  %223 = icmp sgt i32 %70, %213
  br i1 %223, label %228, label %224

224:                                              ; preds = %222
  %225 = load i32, ptr %13, align 4, !tbaa !6
  %226 = load i32, ptr %11, align 4, !tbaa !6
  %227 = load i32, ptr %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr nonnull %0, i32 %213, i32 %64, i32 %70, i32 %225, i32 %226, i32 0, i32 %227, i32 %3)
  br label %1056

228:                                              ; preds = %222
  %229 = icmp sgt i32 %212, 31400
  br i1 %229, label %230, label %248

230:                                              ; preds = %228
  store i32 1, ptr %11, align 4, !tbaa !6
  br label %248

231:                                              ; preds = %147, %122
  %232 = icmp slt i32 %3, 13
  %233 = and i1 %96, %232
  %234 = add nsw i32 %70, -300
  %235 = icmp slt i32 %92, %234
  %236 = select i1 %233, i1 %235, i1 false
  br i1 %236, label %237, label %248

237:                                              ; preds = %231
  %238 = call i32 @_Z7qsearchP7state_tiiii(ptr nonnull %0, i32 %64, i32 %70, i32 0, i32 0)
  %239 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8, !tbaa !57
  %240 = icmp eq i32 %239, 0
  br i1 %240, label %241, label %1056

241:                                              ; preds = %237
  %242 = icmp sgt i32 %238, %64
  br i1 %242, label %248, label %243

243:                                              ; preds = %241
  %244 = load i32, ptr %13, align 4, !tbaa !6
  %245 = load i32, ptr %11, align 4, !tbaa !6
  %246 = load i32, ptr %14, align 4, !tbaa !6
  %247 = load i32, ptr %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr nonnull %0, i32 %64, i32 %64, i32 %70, i32 %244, i32 %245, i32 %246, i32 %247, i32 %3)
  br label %1056

248:                                              ; preds = %241, %231, %230, %228, %176
  br i1 %93, label %249, label %272

249:                                              ; preds = %248
  %250 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 0
  %251 = call i32 @_Z12gen_evasionsP7state_tPii(ptr nonnull %0, ptr nonnull %250, i32 %91)
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
  %260 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %258, !intel-tbaa !63
  %261 = load i32, ptr %260, align 4, !tbaa !63
  call void @_Z4makeP7state_ti(ptr %0, i32 %261)
  %262 = load i32, ptr %260, align 4, !tbaa !63
  %263 = call i32 @_Z11check_legalP7state_ti(ptr %0, i32 %262)
  %264 = icmp ne i32 %263, 0
  %265 = zext i1 %264 to i32
  %266 = add nuw nsw i32 %259, %265
  %267 = load i32, ptr %260, align 4, !tbaa !63
  call void @_Z6unmakeP7state_ti(ptr %0, i32 %267)
  %268 = add nuw nsw i64 %258, 1
  %269 = icmp ult i64 %268, %256
  %270 = icmp ult i32 %266, 2
  %271 = and i1 %269, %270
  br i1 %271, label %257, label %275, !llvm.loop !65

272:                                              ; preds = %248
  %273 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 0
  %274 = call i32 @_Z3genP7state_tPi(ptr nonnull %0, ptr nonnull %273)
  br label %275

275:                                              ; preds = %272, %257, %253, %249
  %276 = phi i32 [ 0, %249 ], [ %274, %272 ], [ %251, %253 ], [ %251, %257 ]
  %277 = phi i32 [ 0, %249 ], [ %274, %272 ], [ 0, %253 ], [ %266, %257 ]
  %278 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 0
  %279 = getelementptr inbounds [240 x i32], ptr %8, i64 0, i64 0
  %280 = load i32, ptr %13, align 4, !tbaa !6
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(ptr %0, ptr nonnull %278, ptr nonnull %279, i32 %276, i32 %280)
  %281 = icmp sgt i32 %3, 19
  br i1 %281, label %282, label %331

282:                                              ; preds = %275
  %283 = icmp ne i32 %70, %95
  %284 = load i32, ptr %13, align 4
  %285 = icmp eq i32 %284, 65535
  %286 = select i1 %283, i1 %285, i1 false
  br i1 %286, label %287, label %331

287:                                              ; preds = %282
  %288 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 1
  %289 = icmp sgt i32 %276, 0
  br i1 %289, label %291, label %290

290:                                              ; preds = %287
  store i32 0, ptr %9, align 4, !tbaa !6
  br label %319

291:                                              ; preds = %287
  %292 = zext i32 %276 to i64
  br label %293

293:                                              ; preds = %313, %291
  %294 = phi i64 [ 0, %291 ], [ %315, %313 ]
  %295 = phi i32 [ 0, %291 ], [ %314, %313 ]
  %296 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %294, !intel-tbaa !63
  %297 = load i32, ptr %296, align 4, !tbaa !63
  %298 = lshr i32 %297, 19
  %299 = and i32 %298, 15
  %300 = icmp eq i32 %299, 13
  br i1 %300, label %313, label %301

301:                                              ; preds = %293
  %302 = lshr i32 %297, 6
  %303 = and i32 %302, 63
  %304 = zext i32 %303 to i64
  %305 = getelementptr inbounds [64 x i32], ptr %288, i64 0, i64 %304, !intel-tbaa !51
  %306 = load i32, ptr %305, align 4, !tbaa !66
  %307 = sext i32 %306 to i64
  %308 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %307, !intel-tbaa !67
  %309 = load i32, ptr %308, align 4, !tbaa !67
  %310 = call i32 @llvm.abs.i32(i32 %309, i1 true)
  %311 = icmp ugt i32 %299, %310
  %312 = select i1 %311, i32 1, i32 %295
  br label %313

313:                                              ; preds = %301, %293
  %314 = phi i32 [ %295, %293 ], [ %312, %301 ]
  %315 = add nuw nsw i64 %294, 1
  %316 = icmp eq i64 %315, %292
  br i1 %316, label %317, label %293, !llvm.loop !69

317:                                              ; preds = %313
  %318 = icmp eq i32 %314, 0
  br i1 %318, label %319, label %331

319:                                              ; preds = %317, %290
  %320 = bitcast ptr %17 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %320) #7
  %321 = bitcast ptr %18 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %321) #7
  %322 = ashr i32 %3, 1
  %323 = call i32 @_Z6searchP7state_tiiiii(ptr %0, i32 %64, i32 %70, i32 %322, i32 0, i32 %86)
  %324 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(ptr %0, ptr nonnull %17, i32 0, i32 0, ptr nonnull %18, ptr nonnull %17, ptr nonnull %17, ptr nonnull %17, ptr nonnull %17, i32 0)
  %325 = icmp eq i32 %324, 4
  br i1 %325, label %328, label %326

326:                                              ; preds = %319
  %327 = load i32, ptr %18, align 4, !tbaa !6
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(ptr %0, ptr nonnull %278, ptr nonnull %279, i32 %276, i32 %327)
  br label %330

328:                                              ; preds = %319
  %329 = load i32, ptr %13, align 4, !tbaa !6
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(ptr %0, ptr nonnull %278, ptr nonnull %279, i32 %276, i32 %329)
  br label %330

330:                                              ; preds = %328, %326
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %321) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %320) #7
  br label %331

331:                                              ; preds = %330, %317, %282, %275
  %332 = load i32, ptr %11, align 4
  %333 = icmp eq i32 %332, 0
  %334 = select i1 %94, i1 %333, i1 false
  %335 = icmp sgt i32 %3, 15
  %336 = and i1 %334, %335
  %337 = icmp sgt i32 %277, 8
  %338 = and i1 %336, %337
  br i1 %338, label %339, label %479

339:                                              ; preds = %331
  %340 = load i32, ptr %31, align 8, !tbaa !41
  %341 = add nsw i32 %340, -1
  %342 = sext i32 %341 to i64
  %343 = getelementptr inbounds [64 x i32], ptr %87, i64 0, i64 %342, !intel-tbaa !51
  %344 = load i32, ptr %343, align 4, !tbaa !52
  %345 = icmp eq i32 %344, 0
  br i1 %345, label %346, label %479

346:                                              ; preds = %339
  %347 = icmp slt i32 %340, 3
  br i1 %347, label %362, label %348

348:                                              ; preds = %346
  %349 = add nsw i32 %340, -2
  %350 = zext i32 %349 to i64
  %351 = getelementptr inbounds [64 x i32], ptr %87, i64 0, i64 %350, !intel-tbaa !51
  %352 = load i32, ptr %351, align 4, !tbaa !52
  %353 = icmp eq i32 %352, 0
  br i1 %353, label %354, label %479

354:                                              ; preds = %348
  %355 = icmp slt i32 %340, 4
  br i1 %355, label %362, label %356

356:                                              ; preds = %354
  %357 = add nsw i32 %340, -3
  %358 = zext i32 %357 to i64
  %359 = getelementptr inbounds [64 x i32], ptr %87, i64 0, i64 %358, !intel-tbaa !51
  %360 = load i32, ptr %359, align 4, !tbaa !52
  %361 = icmp eq i32 %360, 0
  br i1 %361, label %362, label %479

362:                                              ; preds = %356, %354, %346
  store i32 -1, ptr %9, align 4, !tbaa !6
  %363 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  %364 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 16
  %365 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 36
  %366 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 19
  %367 = sub nsw i32 0, %70
  %368 = sub i32 50, %64
  %369 = icmp sgt i32 %3, 16
  %370 = sub i32 1, %70
  %371 = add nsw i32 %3, -16
  %372 = icmp eq i32 %86, 0
  %373 = zext i1 %372 to i32
  %374 = icmp eq i32 %363, 0
  br i1 %374, label %479, label %375

375:                                              ; preds = %362
  %376 = icmp slt i32 %3, 17
  br i1 %376, label %377, label %426

377:                                              ; preds = %420, %375
  %378 = phi i32 [ %421, %420 ], [ 0, %375 ]
  %379 = phi i32 [ %381, %420 ], [ 0, %375 ]
  %380 = phi i32 [ %411, %420 ], [ -32000, %375 ]
  %381 = add nuw nsw i32 %379, 1
  %382 = load i32, ptr %9, align 4, !tbaa !6
  %383 = sext i32 %382 to i64
  %384 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %383, !intel-tbaa !63
  %385 = load i32, ptr %384, align 4, !tbaa !63
  call void @_Z4makeP7state_ti(ptr %0, i32 %385)
  %386 = load i32, ptr %384, align 4, !tbaa !63
  %387 = call i32 @_Z11check_legalP7state_ti(ptr %0, i32 %386)
  %388 = icmp eq i32 %387, 0
  br i1 %388, label %410, label %389

389:                                              ; preds = %377
  %390 = load i64, ptr %364, align 8, !tbaa !22
  %391 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 15), align 4, !tbaa !70
  %392 = load i32, ptr %31, align 8, !tbaa !41
  %393 = add i32 %392, -1
  %394 = add i32 %393, %391
  %395 = sext i32 %394 to i64
  %396 = getelementptr inbounds [1000 x i64], ptr %365, i64 0, i64 %395, !intel-tbaa !71
  store i64 %390, ptr %396, align 8, !tbaa !72
  %397 = load i32, ptr %384, align 4, !tbaa !63
  %398 = sext i32 %393 to i64
  %399 = getelementptr inbounds [64 x i32], ptr %366, i64 0, i64 %398, !intel-tbaa !51
  store i32 %397, ptr %399, align 4, !tbaa !60
  %400 = call i32 @_Z8in_checkP7state_t(ptr %0)
  %401 = load i32, ptr %31, align 8, !tbaa !41
  %402 = sext i32 %401 to i64
  %403 = getelementptr inbounds [64 x i32], ptr %87, i64 0, i64 %402, !intel-tbaa !51
  store i32 %400, ptr %403, align 4, !tbaa !52
  %404 = icmp ne i32 %400, 0
  %405 = select i1 %369, i1 true, i1 %404
  %406 = zext i1 %405 to i32
  %407 = call i32 @_Z4evalP7state_tiii(ptr %0, i32 %367, i32 %368, i32 %406)
  %408 = call i32 @_Z7qsearchP7state_tiiii(ptr nonnull %0, i32 %367, i32 %370, i32 0, i32 0)
  %409 = sub nsw i32 0, %408
  br label %410

410:                                              ; preds = %389, %377
  %411 = phi i32 [ %409, %389 ], [ %380, %377 ]
  %412 = load i32, ptr %384, align 4, !tbaa !63
  call void @_Z6unmakeP7state_ti(ptr %0, i32 %412)
  %413 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8, !tbaa !57
  %414 = icmp eq i32 %413, 0
  br i1 %414, label %415, label %479

415:                                              ; preds = %410
  %416 = icmp slt i32 %411, %70
  %417 = or i1 %388, %416
  br i1 %417, label %420, label %418

418:                                              ; preds = %415
  %419 = icmp sgt i32 %378, 0
  br i1 %419, label %475, label %420

420:                                              ; preds = %418, %415
  %421 = phi i32 [ 1, %418 ], [ %378, %415 ]
  %422 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  %423 = icmp ne i32 %422, 0
  %424 = icmp ult i32 %379, 2
  %425 = and i1 %423, %424
  br i1 %425, label %377, label %479, !llvm.loop !73

426:                                              ; preds = %469, %375
  %427 = phi i32 [ %470, %469 ], [ 0, %375 ]
  %428 = phi i32 [ %430, %469 ], [ 0, %375 ]
  %429 = phi i32 [ %460, %469 ], [ -32000, %375 ]
  %430 = add nuw nsw i32 %428, 1
  %431 = load i32, ptr %9, align 4, !tbaa !6
  %432 = sext i32 %431 to i64
  %433 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %432, !intel-tbaa !63
  %434 = load i32, ptr %433, align 4, !tbaa !63
  call void @_Z4makeP7state_ti(ptr %0, i32 %434)
  %435 = load i32, ptr %433, align 4, !tbaa !63
  %436 = call i32 @_Z11check_legalP7state_ti(ptr %0, i32 %435)
  %437 = icmp eq i32 %436, 0
  br i1 %437, label %459, label %438

438:                                              ; preds = %426
  %439 = load i64, ptr %364, align 8, !tbaa !22
  %440 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 15), align 4, !tbaa !70
  %441 = load i32, ptr %31, align 8, !tbaa !41
  %442 = add i32 %441, -1
  %443 = add i32 %442, %440
  %444 = sext i32 %443 to i64
  %445 = getelementptr inbounds [1000 x i64], ptr %365, i64 0, i64 %444, !intel-tbaa !71
  store i64 %439, ptr %445, align 8, !tbaa !72
  %446 = load i32, ptr %433, align 4, !tbaa !63
  %447 = sext i32 %442 to i64
  %448 = getelementptr inbounds [64 x i32], ptr %366, i64 0, i64 %447, !intel-tbaa !51
  store i32 %446, ptr %448, align 4, !tbaa !60
  %449 = call i32 @_Z8in_checkP7state_t(ptr %0)
  %450 = load i32, ptr %31, align 8, !tbaa !41
  %451 = sext i32 %450 to i64
  %452 = getelementptr inbounds [64 x i32], ptr %87, i64 0, i64 %451, !intel-tbaa !51
  store i32 %449, ptr %452, align 4, !tbaa !52
  %453 = icmp ne i32 %449, 0
  %454 = select i1 %369, i1 true, i1 %453
  %455 = zext i1 %454 to i32
  %456 = call i32 @_Z4evalP7state_tiii(ptr %0, i32 %367, i32 %368, i32 %455)
  %457 = call i32 @_Z6searchP7state_tiiiii(ptr nonnull %0, i32 %367, i32 %370, i32 %371, i32 0, i32 %373)
  %458 = sub nsw i32 0, %457
  br label %459

459:                                              ; preds = %438, %426
  %460 = phi i32 [ %458, %438 ], [ %429, %426 ]
  %461 = load i32, ptr %433, align 4, !tbaa !63
  call void @_Z6unmakeP7state_ti(ptr %0, i32 %461)
  %462 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8, !tbaa !57
  %463 = icmp eq i32 %462, 0
  br i1 %463, label %464, label %479

464:                                              ; preds = %459
  %465 = icmp slt i32 %460, %70
  %466 = or i1 %437, %465
  br i1 %466, label %469, label %467

467:                                              ; preds = %464
  %468 = icmp sgt i32 %427, 0
  br i1 %468, label %475, label %469

469:                                              ; preds = %467, %464
  %470 = phi i32 [ 1, %467 ], [ %427, %464 ]
  %471 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  %472 = icmp ne i32 %471, 0
  %473 = icmp ult i32 %428, 2
  %474 = and i1 %472, %473
  br i1 %474, label %426, label %479, !llvm.loop !73

475:                                              ; preds = %467, %418
  %476 = load i32, ptr %13, align 4, !tbaa !6
  %477 = load i32, ptr %11, align 4, !tbaa !6
  %478 = load i32, ptr %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr %0, i32 %70, i32 %64, i32 %70, i32 %476, i32 %477, i32 0, i32 %478, i32 %3)
  br label %1056

479:                                              ; preds = %469, %459, %420, %410, %362, %356, %348, %339, %331
  %480 = phi i32 [ -32000, %339 ], [ -32000, %356 ], [ -32000, %348 ], [ -32000, %331 ], [ -32000, %362 ], [ %411, %420 ], [ %411, %410 ], [ %460, %469 ], [ %460, %459 ]
  %481 = load i32, ptr %14, align 4, !tbaa !6
  %482 = icmp eq i32 %481, 0
  %483 = load i32, ptr %15, align 4
  %484 = icmp eq i32 %483, 0
  %485 = select i1 %482, i1 %484, i1 false
  %486 = load i32, ptr %11, align 4
  %487 = icmp eq i32 %486, 0
  %488 = select i1 %485, i1 %487, i1 false
  %489 = and i1 %488, %281
  %490 = icmp sgt i32 %277, 1
  %491 = and i1 %489, %490
  %492 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 1), align 4
  %493 = icmp ne i32 %492, 2
  %494 = select i1 %491, i1 %493, i1 false
  br i1 %494, label %495, label %571

495:                                              ; preds = %479
  %496 = add nsw i32 %3, -24
  %497 = call i32 @_Z6searchP7state_tiiiii(ptr %0, i32 %64, i32 %70, i32 %496, i32 0, i32 %86)
  %498 = icmp sgt i32 %497, %64
  br i1 %498, label %499, label %571

499:                                              ; preds = %495
  store i32 -1, ptr %9, align 4, !tbaa !6
  %500 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  %501 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 16
  %502 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 36
  %503 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 19
  %504 = add nsw i32 %3, -16
  %505 = sub nsw i32 0, %70
  %506 = sub i32 50, %64
  %507 = sub nsw i32 0, %64
  %508 = xor i32 %64, -1
  %509 = icmp eq i32 %86, 0
  %510 = zext i1 %509 to i32
  %511 = sub i32 49, %64
  %512 = add nsw i32 %64, -50
  %513 = icmp ne i32 %500, 0
  %514 = load i32, ptr %14, align 4
  %515 = icmp slt i32 %514, 2
  %516 = select i1 %513, i1 %515, i1 false
  br i1 %516, label %517, label %571

517:                                              ; preds = %559, %499
  %518 = phi i32 [ %562, %559 ], [ 0, %499 ]
  %519 = phi i32 [ %561, %559 ], [ %480, %499 ]
  %520 = phi i32 [ %560, %559 ], [ 1, %499 ]
  %521 = load i32, ptr %9, align 4, !tbaa !6
  %522 = sext i32 %521 to i64
  %523 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %522, !intel-tbaa !63
  %524 = load i32, ptr %523, align 4, !tbaa !63
  call void @_Z4makeP7state_ti(ptr %0, i32 %524)
  %525 = load i32, ptr %523, align 4, !tbaa !63
  %526 = call i32 @_Z11check_legalP7state_ti(ptr %0, i32 %525)
  %527 = icmp eq i32 %526, 0
  br i1 %527, label %559, label %528

528:                                              ; preds = %517
  %529 = load i64, ptr %501, align 8, !tbaa !22
  %530 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 15), align 4, !tbaa !70
  %531 = load i32, ptr %31, align 8, !tbaa !41
  %532 = add i32 %531, -1
  %533 = add i32 %532, %530
  %534 = sext i32 %533 to i64
  %535 = getelementptr inbounds [1000 x i64], ptr %502, i64 0, i64 %534, !intel-tbaa !71
  store i64 %529, ptr %535, align 8, !tbaa !72
  %536 = load i32, ptr %523, align 4, !tbaa !63
  %537 = sext i32 %532 to i64
  %538 = getelementptr inbounds [64 x i32], ptr %503, i64 0, i64 %537, !intel-tbaa !51
  store i32 %536, ptr %538, align 4, !tbaa !60
  %539 = add nsw i32 %518, 1
  %540 = call i32 @_Z8in_checkP7state_t(ptr %0)
  %541 = load i32, ptr %31, align 8, !tbaa !41
  %542 = sext i32 %541 to i64
  %543 = getelementptr inbounds [64 x i32], ptr %87, i64 0, i64 %542, !intel-tbaa !51
  store i32 %540, ptr %543, align 4, !tbaa !52
  %544 = call i32 @_Z4evalP7state_tiii(ptr %0, i32 %505, i32 %506, i32 1)
  %545 = icmp eq i32 %520, 0
  br i1 %545, label %553, label %546

546:                                              ; preds = %528
  %547 = call i32 @_Z6searchP7state_tiiiii(ptr nonnull %0, i32 %508, i32 %507, i32 %504, i32 0, i32 %510)
  %548 = sub nsw i32 0, %547
  %549 = icmp slt i32 %64, %548
  br i1 %549, label %550, label %551

550:                                              ; preds = %546
  store i32 1, ptr %14, align 4, !tbaa !6
  br label %559

551:                                              ; preds = %546
  store i32 0, ptr %14, align 4, !tbaa !6
  %552 = add nsw i32 %518, 11
  br label %559

553:                                              ; preds = %528
  %554 = call i32 @_Z6searchP7state_tiiiii(ptr nonnull %0, i32 %511, i32 %506, i32 %504, i32 0, i32 0)
  %555 = sub nsw i32 0, %554
  %556 = icmp slt i32 %512, %555
  br i1 %556, label %557, label %559

557:                                              ; preds = %553
  store i32 0, ptr %14, align 4, !tbaa !6
  %558 = add nsw i32 %518, 11
  br label %559

559:                                              ; preds = %557, %553, %551, %550, %517
  %560 = phi i32 [ %520, %517 ], [ 0, %553 ], [ 0, %557 ], [ 0, %550 ], [ 0, %551 ]
  %561 = phi i32 [ %519, %517 ], [ %555, %553 ], [ %555, %557 ], [ %548, %550 ], [ %548, %551 ]
  %562 = phi i32 [ %518, %517 ], [ %539, %553 ], [ %558, %557 ], [ %539, %550 ], [ %552, %551 ]
  %563 = load i32, ptr %523, align 4, !tbaa !63
  call void @_Z6unmakeP7state_ti(ptr %0, i32 %563)
  %564 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  %565 = icmp ne i32 %564, 0
  %566 = load i32, ptr %14, align 4
  %567 = icmp slt i32 %566, 2
  %568 = select i1 %565, i1 %567, i1 false
  %569 = icmp slt i32 %562, 3
  %570 = and i1 %568, %569
  br i1 %570, label %517, label %571, !llvm.loop !74

571:                                              ; preds = %559, %499, %495, %479
  %572 = phi i32 [ %480, %479 ], [ %480, %495 ], [ %480, %499 ], [ %561, %559 ]
  br i1 %96, label %578, label %573

573:                                              ; preds = %571
  %574 = load i32, ptr %31, align 8, !tbaa !41
  %575 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 5), align 4, !tbaa !75
  %576 = shl nsw i32 %575, 1
  %577 = icmp sle i32 %574, %576
  br label %578

578:                                              ; preds = %573, %571
  %579 = phi i1 [ false, %571 ], [ %577, %573 ]
  store i32 -1, ptr %9, align 4, !tbaa !6
  %580 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  %581 = icmp eq i32 %277, 1
  %582 = and i1 %93, %581
  %583 = select i1 %582, i32 4, i32 0
  %584 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 1
  %585 = or i32 %583, 2
  %586 = select i1 %579, i32 4, i32 2
  %587 = select i1 %579, i32 %585, i32 %583
  %588 = select i1 %579, i32 3, i32 1
  %589 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 19
  %590 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 11
  %591 = add nsw i32 %145, %134
  %592 = icmp eq i32 %591, 1
  %593 = sdiv i32 %3, 4
  %594 = add nsw i32 %593, 1
  %595 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 0
  %596 = icmp sgt i32 %3, 24
  %597 = icmp slt i32 %3, 9
  %598 = icmp slt i32 %3, 13
  %599 = add nsw i32 %92, 100
  %600 = add nsw i32 %92, 300
  %601 = add nsw i32 %92, 75
  %602 = add nsw i32 %92, 200
  %603 = sub nsw i32 0, %70
  %604 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 16
  %605 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 36
  %606 = icmp sgt i32 %3, 4
  %607 = icmp eq i32 %86, 0
  %608 = zext i1 %607 to i32
  %609 = icmp eq i32 %580, 0
  br i1 %609, label %1024, label %610

610:                                              ; preds = %578
  %611 = icmp slt i32 %593, 0
  %612 = add nsw i32 %3, -4
  br label %613

613:                                              ; preds = %1013, %610
  %614 = phi i1 [ %1022, %1013 ], [ %611, %610 ]
  %615 = phi i1 [ %1021, %1013 ], [ true, %610 ]
  %616 = phi i32 [ %1020, %1013 ], [ %64, %610 ]
  %617 = phi i32 [ %1019, %1013 ], [ %572, %610 ]
  %618 = phi i32 [ %1018, %1013 ], [ 1, %610 ]
  %619 = phi i32 [ %1017, %1013 ], [ -32000, %610 ]
  %620 = phi i32 [ %1016, %1013 ], [ 1, %610 ]
  %621 = phi i32 [ %1015, %1013 ], [ 1, %610 ]
  %622 = add nsw i32 %616, 1
  %623 = icmp eq i32 %70, %622
  br label %624

624:                                              ; preds = %753, %613
  %625 = phi i32 [ %618, %613 ], [ 0, %753 ]
  %626 = load i32, ptr %31, align 8, !tbaa !41
  %627 = icmp slt i32 %626, 60
  br i1 %627, label %631, label %628

628:                                              ; preds = %624
  %629 = load i32, ptr %9, align 4, !tbaa !6
  %630 = sext i32 %629 to i64
  br label %710

631:                                              ; preds = %624
  %632 = load i32, ptr %9, align 4, !tbaa !6
  %633 = sext i32 %632 to i64
  %634 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %633, !intel-tbaa !63
  %635 = load i32, ptr %634, align 4, !tbaa !63
  %636 = lshr i32 %635, 6
  %637 = and i32 %636, 63
  %638 = zext i32 %637 to i64
  %639 = getelementptr inbounds [64 x i32], ptr %584, i64 0, i64 %638, !intel-tbaa !51
  %640 = load i32, ptr %639, align 4, !tbaa !66
  %641 = add nsw i32 %640, 1
  %642 = and i32 %641, -2
  %643 = icmp eq i32 %642, 2
  br i1 %643, label %644, label %651

644:                                              ; preds = %631
  %645 = lshr i32 %635, 3
  %646 = and i32 %645, 7
  switch i32 %646, label %647 [
    i32 1, label %650
    i32 6, label %650
  ]

647:                                              ; preds = %644
  %648 = and i32 %635, 61440
  %649 = icmp eq i32 %648, 0
  br i1 %649, label %651, label %650

650:                                              ; preds = %647, %644, %644
  br label %651

651:                                              ; preds = %650, %647, %631
  %652 = phi i32 [ %583, %647 ], [ %583, %631 ], [ %587, %650 ]
  %653 = lshr i32 %635, 19
  %654 = and i32 %653, 15
  %655 = icmp eq i32 %654, 13
  br i1 %655, label %686, label %656

656:                                              ; preds = %651
  %657 = add nsw i32 %626, -1
  %658 = sext i32 %657 to i64
  %659 = getelementptr inbounds [64 x i32], ptr %589, i64 0, i64 %658, !intel-tbaa !51
  %660 = load i32, ptr %659, align 4, !tbaa !60
  %661 = lshr i32 %660, 19
  %662 = and i32 %661, 15
  %663 = icmp eq i32 %662, 13
  br i1 %663, label %686, label %664

664:                                              ; preds = %656
  %665 = zext i32 %654 to i64
  %666 = getelementptr inbounds [14 x i32], ptr @_ZL8rc_index, i64 0, i64 %665, !intel-tbaa !67
  %667 = load i32, ptr %666, align 4, !tbaa !67
  %668 = zext i32 %662 to i64
  %669 = getelementptr inbounds [14 x i32], ptr @_ZL8rc_index, i64 0, i64 %668, !intel-tbaa !67
  %670 = load i32, ptr %669, align 4, !tbaa !67
  %671 = icmp eq i32 %667, %670
  br i1 %671, label %672, label %686

672:                                              ; preds = %664
  %673 = and i32 %635, 63
  %674 = and i32 %660, 63
  %675 = icmp eq i32 %673, %674
  br i1 %675, label %676, label %686

676:                                              ; preds = %672
  %677 = load i32, ptr %590, align 4, !tbaa !21
  %678 = icmp eq i32 %677, 0
  %679 = zext i1 %678 to i32
  %680 = lshr i32 %635, 12
  %681 = and i32 %680, 15
  %682 = call i32 @_Z3seeP7state_tiiii(ptr nonnull %0, i32 %679, i32 %637, i32 %673, i32 %681)
  %683 = icmp sgt i32 %682, 0
  %684 = select i1 %683, i32 %588, i32 0
  %685 = add nsw i32 %684, %652
  br label %686

686:                                              ; preds = %676, %672, %664, %656, %651
  %687 = phi i32 [ %652, %672 ], [ %652, %664 ], [ %652, %656 ], [ %652, %651 ], [ %685, %676 ]
  %688 = load i32, ptr %14, align 4, !tbaa !6
  %689 = icmp eq i32 %688, 1
  %690 = icmp ne i32 %687, 0
  %691 = and i1 %689, %690
  %692 = and i1 %691, %615
  br i1 %692, label %693, label %694

693:                                              ; preds = %686
  store i32 1, ptr %15, align 4, !tbaa !6
  br label %700

694:                                              ; preds = %686
  %695 = xor i1 %690, true
  %696 = select i1 %695, i1 %689, i1 false
  %697 = and i1 %696, %615
  br i1 %697, label %698, label %700

698:                                              ; preds = %694
  store i32 0, ptr %15, align 4, !tbaa !6
  %699 = add nsw i32 %687, %588
  br label %700

700:                                              ; preds = %698, %694, %693
  %701 = phi i32 [ %687, %693 ], [ %687, %694 ], [ %699, %698 ]
  %702 = icmp slt i32 %701, 4
  %703 = select i1 %702, i32 %701, i32 4
  %704 = load i32, ptr %634, align 4, !tbaa !63
  %705 = lshr i32 %704, 19
  %706 = and i32 %705, 15
  switch i32 %706, label %707 [
    i32 13, label %710
    i32 1, label %710
    i32 2, label %710
  ]

707:                                              ; preds = %700
  %708 = add nsw i32 %703, 4
  %709 = select i1 %592, i32 %708, i32 %703
  br label %710

710:                                              ; preds = %707, %700, %700, %700, %628
  %711 = phi i64 [ %630, %628 ], [ %633, %707 ], [ %633, %700 ], [ %633, %700 ], [ %633, %700 ]
  %712 = phi i32 [ 0, %628 ], [ %709, %707 ], [ %703, %700 ], [ %703, %700 ], [ %703, %700 ]
  %713 = getelementptr inbounds [240 x i32], ptr %7, i64 0, i64 %711
  %714 = load i32, ptr %713, align 4, !tbaa !63
  %715 = and i32 %714, 7864320
  %716 = icmp eq i32 %715, 6815744
  %717 = xor i1 %716, true
  %718 = xor i1 %614, true
  %719 = or i1 %717, %718
  %720 = select i1 %717, i1 false, i1 true
  %721 = select i1 %717, i32 %625, i32 %618
  br i1 %719, label %759, label %722

722:                                              ; preds = %710
  %723 = lshr i32 %714, 6
  %724 = and i32 %723, 63
  %725 = zext i32 %724 to i64
  %726 = getelementptr inbounds [64 x i32], ptr %584, i64 0, i64 %725, !intel-tbaa !51
  %727 = load i32, ptr %726, align 4, !tbaa !66
  %728 = add nsw i32 %727, -1
  %729 = and i32 %714, 63
  %730 = load i32, ptr %595, align 8, !tbaa !76
  %731 = sext i32 %730 to i64
  %732 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_hit, i64 0, i64 %731, !intel-tbaa !77
  %733 = sext i32 %728 to i64
  %734 = getelementptr inbounds [12 x [64 x i32]], ptr %732, i64 0, i64 %733, !intel-tbaa !80
  %735 = zext i32 %729 to i64
  %736 = getelementptr inbounds [64 x i32], ptr %734, i64 0, i64 %735, !intel-tbaa !51
  %737 = load i32, ptr %736, align 4, !tbaa !81
  %738 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_tot, i64 0, i64 %731, !intel-tbaa !77
  %739 = getelementptr inbounds [12 x [64 x i32]], ptr %738, i64 0, i64 %733, !intel-tbaa !80
  %740 = getelementptr inbounds [64 x i32], ptr %739, i64 0, i64 %735, !intel-tbaa !51
  %741 = load i32, ptr %740, align 4, !tbaa !81
  %742 = sub nsw i32 %741, %737
  %743 = mul nsw i32 %737, %594
  %744 = icmp sge i32 %743, %742
  %745 = or i1 %744, %596
  %746 = icmp ne i32 %712, 0
  %747 = or i1 %745, %746
  %748 = xor i1 %747, true
  %749 = select i1 %748, i1 %623, i1 false
  %750 = and i32 %714, 61440
  %751 = icmp eq i32 %750, 0
  %752 = and i1 %749, %751
  br i1 %752, label %753, label %759

753:                                              ; preds = %722
  %754 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  %755 = icmp eq i32 %754, 0
  br i1 %755, label %756, label %624, !llvm.loop !82

756:                                              ; preds = %753
  %757 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8, !tbaa !57
  %758 = icmp eq i32 %757, 0
  br label %1043

759:                                              ; preds = %722, %710
  %760 = phi i1 [ true, %722 ], [ %720, %710 ]
  %761 = phi i32 [ %625, %722 ], [ %721, %710 ]
  br i1 %597, label %762, label %765

762:                                              ; preds = %759
  %763 = icmp slt i32 %601, %616
  %764 = icmp slt i32 %602, %616
  br label %770

765:                                              ; preds = %759
  %766 = icmp slt i32 %599, %616
  %767 = icmp slt i32 %600, %616
  %768 = select i1 %598, i1 %766, i1 false
  %769 = select i1 %598, i1 %767, i1 false
  br label %770

770:                                              ; preds = %765, %762
  %771 = phi i1 [ %763, %762 ], [ %768, %765 ]
  %772 = phi i1 [ %764, %762 ], [ %769, %765 ]
  br i1 %760, label %783, label %773

773:                                              ; preds = %770
  %774 = load i32, ptr %590, align 4, !tbaa !21
  %775 = icmp eq i32 %774, 0
  %776 = zext i1 %775 to i32
  %777 = lshr i32 %714, 6
  %778 = and i32 %777, 63
  %779 = and i32 %714, 63
  %780 = lshr i32 %714, 12
  %781 = and i32 %780, 15
  %782 = call i32 @_Z3seeP7state_tiiii(ptr nonnull %0, i32 %776, i32 %778, i32 %779, i32 %781)
  br label %783

783:                                              ; preds = %773, %770
  %784 = phi i32 [ %782, %773 ], [ -1000000, %770 ]
  %785 = load i32, ptr %713, align 4, !tbaa !63
  call void @_Z4makeP7state_ti(ptr nonnull %0, i32 %785)
  %786 = load i32, ptr %713, align 4, !tbaa !63
  %787 = call i32 @_Z11check_legalP7state_ti(ptr nonnull %0, i32 %786)
  %788 = icmp eq i32 %787, 0
  br i1 %788, label %923, label %789

789:                                              ; preds = %783
  %790 = call i32 @_Z8in_checkP7state_t(ptr nonnull %0)
  %791 = icmp ne i32 %790, 0
  br i1 %791, label %792, label %794

792:                                              ; preds = %789
  %793 = add nsw i32 %712, %586
  br label %814

794:                                              ; preds = %789
  %795 = select i1 %94, i1 %623, i1 false
  br i1 %795, label %796, label %814

796:                                              ; preds = %794
  %797 = icmp slt i32 %784, 86
  %798 = and i1 %772, %797
  br i1 %798, label %799, label %805

799:                                              ; preds = %796
  %800 = load i32, ptr %713, align 4, !tbaa !63
  %801 = and i32 %800, 61440
  %802 = icmp eq i32 %801, 0
  br i1 %802, label %803, label %805

803:                                              ; preds = %799
  call void @_Z6unmakeP7state_ti(ptr nonnull %0, i32 %800)
  %804 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  br label %1013, !llvm.loop !82

805:                                              ; preds = %799, %796
  %806 = icmp slt i32 %784, -50
  %807 = and i1 %771, %806
  br i1 %807, label %808, label %814

808:                                              ; preds = %805
  %809 = load i32, ptr %713, align 4, !tbaa !63
  %810 = and i32 %809, 61440
  %811 = icmp eq i32 %810, 0
  br i1 %811, label %812, label %814

812:                                              ; preds = %808
  call void @_Z6unmakeP7state_ti(ptr nonnull %0, i32 %809)
  %813 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  br label %1013, !llvm.loop !82

814:                                              ; preds = %808, %805, %794, %792
  %815 = phi i32 [ %712, %805 ], [ %712, %808 ], [ %712, %794 ], [ %793, %792 ]
  %816 = add nsw i32 %815, %3
  %817 = sub nsw i32 0, %616
  %818 = sub i32 130, %616
  %819 = icmp sgt i32 %816, 4
  %820 = select i1 %819, i1 true, i1 %791
  %821 = zext i1 %820 to i32
  %822 = call i32 @_Z4evalP7state_tiii(ptr nonnull %0, i32 %603, i32 %818, i32 %821)
  %823 = load i32, ptr %31, align 8, !tbaa !41
  %824 = sext i32 %823 to i64
  %825 = getelementptr inbounds [64 x i32], ptr %87, i64 0, i64 %824, !intel-tbaa !51
  store i32 %790, ptr %825, align 4, !tbaa !52
  %826 = load i64, ptr %604, align 8, !tbaa !22
  %827 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 15), align 4, !tbaa !70
  %828 = add i32 %823, -1
  %829 = add i32 %828, %827
  %830 = sext i32 %829 to i64
  %831 = getelementptr inbounds [1000 x i64], ptr %605, i64 0, i64 %830, !intel-tbaa !71
  store i64 %826, ptr %831, align 8, !tbaa !72
  %832 = load i32, ptr %713, align 4, !tbaa !63
  %833 = sext i32 %828 to i64
  %834 = getelementptr inbounds [64 x i32], ptr %589, i64 0, i64 %833, !intel-tbaa !51
  store i32 %832, ptr %834, align 4, !tbaa !60
  %835 = icmp sgt i32 %621, 3
  %836 = and i1 %606, %835
  br i1 %836, label %837, label %872

837:                                              ; preds = %814
  %838 = icmp ne i32 %70, %622
  %839 = icmp ne i32 %815, 0
  %840 = or i1 %838, %839
  %841 = select i1 %840, i1 true, i1 %791
  %842 = icmp sgt i32 %784, -51
  %843 = or i1 %841, %842
  br i1 %843, label %872, label %844

844:                                              ; preds = %837
  %845 = and i32 %832, 63
  %846 = zext i32 %845 to i64
  %847 = getelementptr inbounds [64 x i32], ptr %584, i64 0, i64 %846, !intel-tbaa !51
  %848 = load i32, ptr %847, align 4, !tbaa !66
  %849 = add nsw i32 %848, -1
  %850 = load i32, ptr %595, align 8, !tbaa !76
  %851 = sext i32 %850 to i64
  %852 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_hit, i64 0, i64 %851, !intel-tbaa !77
  %853 = sext i32 %849 to i64
  %854 = getelementptr inbounds [12 x [64 x i32]], ptr %852, i64 0, i64 %853, !intel-tbaa !80
  %855 = getelementptr inbounds [64 x i32], ptr %854, i64 0, i64 %846, !intel-tbaa !51
  %856 = load i32, ptr %855, align 4, !tbaa !81
  %857 = shl i32 %856, 7
  %858 = add i32 %857, 128
  %859 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_tot, i64 0, i64 %851, !intel-tbaa !77
  %860 = getelementptr inbounds [12 x [64 x i32]], ptr %859, i64 0, i64 %853, !intel-tbaa !80
  %861 = getelementptr inbounds [64 x i32], ptr %860, i64 0, i64 %846, !intel-tbaa !51
  %862 = load i32, ptr %861, align 4, !tbaa !81
  %863 = add nsw i32 %862, 1
  %864 = sdiv i32 %858, %863
  %865 = icmp slt i32 %864, 80
  %866 = and i32 %832, 61440
  %867 = icmp eq i32 %866, 0
  %868 = and i1 %865, %867
  %869 = select i1 %868, i32 4, i32 0
  %870 = select i1 %868, i32 -4, i32 0
  %871 = select i1 %868, i32 %612, i32 %816
  br label %872

872:                                              ; preds = %844, %837, %814
  %873 = phi i1 [ false, %837 ], [ false, %814 ], [ %868, %844 ]
  %874 = phi i32 [ 0, %837 ], [ 0, %814 ], [ %869, %844 ]
  %875 = phi i32 [ %815, %837 ], [ %815, %814 ], [ %870, %844 ]
  %876 = phi i32 [ %816, %837 ], [ %816, %814 ], [ %871, %844 ]
  %877 = add nsw i32 %876, -4
  %878 = icmp eq i32 %620, 1
  %879 = icmp slt i32 %876, 5
  br i1 %878, label %880, label %887

880:                                              ; preds = %872
  br i1 %879, label %881, label %884

881:                                              ; preds = %880
  %882 = call i32 @_Z7qsearchP7state_tiiii(ptr nonnull %0, i32 %603, i32 %817, i32 0, i32 0)
  %883 = sub nsw i32 0, %882
  br label %919

884:                                              ; preds = %880
  %885 = call i32 @_Z6searchP7state_tiiiii(ptr nonnull %0, i32 %603, i32 %817, i32 %877, i32 0, i32 %608)
  %886 = sub nsw i32 0, %885
  br label %919

887:                                              ; preds = %872
  %888 = xor i32 %616, -1
  br i1 %879, label %889, label %891

889:                                              ; preds = %887
  %890 = call i32 @_Z7qsearchP7state_tiiii(ptr nonnull %0, i32 %888, i32 %817, i32 0, i32 0)
  br label %893

891:                                              ; preds = %887
  %892 = call i32 @_Z6searchP7state_tiiiii(ptr nonnull %0, i32 %888, i32 %817, i32 %877, i32 0, i32 1)
  br label %893

893:                                              ; preds = %891, %889
  %894 = phi i32 [ %890, %889 ], [ %892, %891 ]
  %895 = sub nsw i32 0, %894
  %896 = icmp slt i32 %619, %895
  %897 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8
  %898 = icmp eq i32 %897, 0
  %899 = select i1 %896, i1 %898, i1 false
  %900 = icmp slt i32 %616, %895
  %901 = select i1 %899, i1 %900, i1 false
  br i1 %901, label %902, label %919

902:                                              ; preds = %893
  br i1 %873, label %903, label %905

903:                                              ; preds = %902
  %904 = add nsw i32 %875, %874
  br label %907

905:                                              ; preds = %902
  %906 = icmp sgt i32 %70, %895
  br i1 %906, label %907, label %919

907:                                              ; preds = %905, %903
  %908 = phi i32 [ %904, %903 ], [ %875, %905 ]
  %909 = add nsw i32 %908, %3
  %910 = icmp slt i32 %909, 5
  br i1 %910, label %911, label %914

911:                                              ; preds = %907
  %912 = call i32 @_Z7qsearchP7state_tiiii(ptr nonnull %0, i32 %603, i32 %817, i32 0, i32 0)
  %913 = sub nsw i32 0, %912
  br label %919

914:                                              ; preds = %907
  %915 = add nsw i32 %909, -4
  %916 = zext i1 %873 to i32
  %917 = call i32 @_Z6searchP7state_tiiiii(ptr nonnull %0, i32 %603, i32 %817, i32 %915, i32 0, i32 %916)
  %918 = sub nsw i32 0, %917
  br label %919

919:                                              ; preds = %914, %911, %905, %893, %884, %881
  %920 = phi i32 [ %883, %881 ], [ %886, %884 ], [ %895, %893 ], [ %913, %911 ], [ %918, %914 ], [ %895, %905 ]
  %921 = icmp sgt i32 %920, %619
  %922 = select i1 %921, i32 %920, i32 %619
  br label %923

923:                                              ; preds = %919, %783
  %924 = phi i32 [ %619, %783 ], [ %922, %919 ]
  %925 = phi i32 [ %761, %783 ], [ 0, %919 ]
  %926 = phi i32 [ %617, %783 ], [ %920, %919 ]
  %927 = load i32, ptr %713, align 4, !tbaa !63
  call void @_Z6unmakeP7state_ti(ptr nonnull %0, i32 %927)
  %928 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8, !tbaa !57
  %929 = icmp eq i32 %928, 0
  br i1 %929, label %930, label %1056

930:                                              ; preds = %923
  br i1 %788, label %1008, label %931

931:                                              ; preds = %930
  %932 = icmp sgt i32 %926, %616
  br i1 %932, label %933, label %1001

933:                                              ; preds = %931
  %934 = icmp slt i32 %926, %70
  %935 = load i32, ptr %713, align 4, !tbaa !63
  br i1 %934, label %998, label %936

936:                                              ; preds = %933
  call fastcc void @_ZL12history_goodP7state_tii(ptr nonnull %0, i32 %935, i32 %3)
  %937 = add nsw i32 %3, 3
  %938 = sdiv i32 %937, 4
  %939 = icmp sgt i32 %621, 1
  br i1 %939, label %940, label %991

940:                                              ; preds = %936
  %941 = add nsw i32 %621, -1
  %942 = zext i32 %941 to i64
  br label %943

943:                                              ; preds = %988, %940
  %944 = phi i64 [ 0, %940 ], [ %989, %988 ]
  %945 = getelementptr inbounds [240 x i32], ptr %16, i64 0, i64 %944, !intel-tbaa !63
  %946 = load i32, ptr %945, align 4, !tbaa !63
  %947 = and i32 %946, 7925760
  %948 = icmp eq i32 %947, 6815744
  br i1 %948, label %949, label %988

949:                                              ; preds = %943
  %950 = lshr i32 %946, 6
  %951 = and i32 %950, 63
  %952 = zext i32 %951 to i64
  %953 = getelementptr inbounds [64 x i32], ptr %584, i64 0, i64 %952, !intel-tbaa !51
  %954 = load i32, ptr %953, align 4, !tbaa !66
  %955 = add nsw i32 %954, -1
  %956 = and i32 %946, 63
  %957 = load i32, ptr %595, align 8, !tbaa !76
  %958 = sext i32 %957 to i64
  %959 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_tot, i64 0, i64 %958, !intel-tbaa !77
  %960 = sext i32 %955 to i64
  %961 = getelementptr inbounds [12 x [64 x i32]], ptr %959, i64 0, i64 %960, !intel-tbaa !80
  %962 = zext i32 %956 to i64
  %963 = getelementptr inbounds [64 x i32], ptr %961, i64 0, i64 %962, !intel-tbaa !51
  %964 = load i32, ptr %963, align 4, !tbaa !81
  %965 = add nsw i32 %964, %938
  store i32 %965, ptr %963, align 4, !tbaa !81
  %966 = icmp sgt i32 %965, 16384
  br i1 %966, label %967, label %988

967:                                              ; preds = %949
  %968 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_hit, i64 0, i64 %958, !intel-tbaa !77
  br label %969

969:                                              ; preds = %985, %967
  %970 = phi i64 [ 0, %967 ], [ %986, %985 ]
  %971 = getelementptr inbounds [12 x [64 x i32]], ptr %968, i64 0, i64 %970, !intel-tbaa !80
  %972 = getelementptr inbounds [12 x [64 x i32]], ptr %959, i64 0, i64 %970, !intel-tbaa !80
  br label %973

973:                                              ; preds = %973, %969
  %974 = phi i64 [ 0, %969 ], [ %983, %973 ]
  %975 = getelementptr inbounds [64 x i32], ptr %971, i64 0, i64 %974, !intel-tbaa !51
  %976 = load i32, ptr %975, align 4, !tbaa !81
  %977 = add nsw i32 %976, 1
  %978 = ashr i32 %977, 1
  store i32 %978, ptr %975, align 4, !tbaa !81
  %979 = getelementptr inbounds [64 x i32], ptr %972, i64 0, i64 %974, !intel-tbaa !51
  %980 = load i32, ptr %979, align 4, !tbaa !81
  %981 = add nsw i32 %980, 1
  %982 = ashr i32 %981, 1
  store i32 %982, ptr %979, align 4, !tbaa !81
  %983 = add nuw nsw i64 %974, 1
  %984 = icmp eq i64 %983, 64
  br i1 %984, label %985, label %973, !llvm.loop !83

985:                                              ; preds = %973
  %986 = add nuw nsw i64 %970, 1
  %987 = icmp eq i64 %986, 12
  br i1 %987, label %988, label %969, !llvm.loop !84

988:                                              ; preds = %985, %949, %943
  %989 = add nuw nsw i64 %944, 1
  %990 = icmp eq i64 %989, %942
  br i1 %990, label %991, label %943, !llvm.loop !85

991:                                              ; preds = %988, %936
  %992 = load i32, ptr %713, align 4, !tbaa !63
  %993 = call zeroext i16 @_Z12compact_movei(i32 %992)
  %994 = zext i16 %993 to i32
  %995 = load i32, ptr %11, align 4, !tbaa !6
  %996 = load i32, ptr %14, align 4, !tbaa !6
  %997 = load i32, ptr %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr %0, i32 %926, i32 %64, i32 %70, i32 %994, i32 %995, i32 %996, i32 %997, i32 %3)
  br label %1056

998:                                              ; preds = %933
  %999 = call zeroext i16 @_Z12compact_movei(i32 %935)
  %1000 = zext i16 %999 to i32
  store i32 %1000, ptr %13, align 4, !tbaa !6
  br label %1001

1001:                                             ; preds = %998, %931
  %1002 = phi i32 [ %926, %998 ], [ %616, %931 ]
  %1003 = load i32, ptr %713, align 4, !tbaa !63
  %1004 = add nsw i32 %621, -1
  %1005 = sext i32 %1004 to i64
  %1006 = getelementptr inbounds [240 x i32], ptr %16, i64 0, i64 %1005, !intel-tbaa !63
  store i32 %1003, ptr %1006, align 4, !tbaa !63
  %1007 = add nsw i32 %621, 1
  br label %1008

1008:                                             ; preds = %1001, %930
  %1009 = phi i32 [ %1007, %1001 ], [ %621, %930 ]
  %1010 = phi i32 [ 0, %1001 ], [ %620, %930 ]
  %1011 = phi i32 [ %1002, %1001 ], [ %616, %930 ]
  %1012 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %9, ptr nonnull %279, ptr nonnull %278, i32 %276)
  br label %1013

1013:                                             ; preds = %1008, %812, %803
  %1014 = phi i32 [ %1012, %1008 ], [ %813, %812 ], [ %804, %803 ]
  %1015 = phi i32 [ %1009, %1008 ], [ %621, %812 ], [ %621, %803 ]
  %1016 = phi i32 [ %1010, %1008 ], [ %620, %812 ], [ %620, %803 ]
  %1017 = phi i32 [ %924, %1008 ], [ %616, %812 ], [ %616, %803 ]
  %1018 = phi i32 [ %925, %1008 ], [ 0, %812 ], [ 0, %803 ]
  %1019 = phi i32 [ %926, %1008 ], [ %617, %812 ], [ %617, %803 ]
  %1020 = phi i32 [ %1011, %1008 ], [ %616, %812 ], [ %616, %803 ]
  %1021 = icmp ne i32 %1016, 0
  %1022 = icmp sgt i32 %1015, %594
  %1023 = icmp eq i32 %1014, 0
  br i1 %1023, label %1024, label %613

1024:                                             ; preds = %1013, %578
  %1025 = phi i32 [ -32000, %578 ], [ %1017, %1013 ]
  %1026 = phi i32 [ 1, %578 ], [ %1018, %1013 ]
  %1027 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8, !tbaa !57
  %1028 = icmp eq i32 %1027, 0
  %1029 = icmp ne i32 %1026, 0
  %1030 = select i1 %1029, i1 %1028, i1 false
  br i1 %1030, label %1031, label %1043

1031:                                             ; preds = %1024
  %1032 = call i32 @_Z8in_checkP7state_t(ptr %0)
  %1033 = icmp eq i32 %1032, 0
  %1034 = load i32, ptr %11, align 4, !tbaa !6
  %1035 = load i32, ptr %14, align 4, !tbaa !6
  %1036 = load i32, ptr %15, align 4, !tbaa !6
  br i1 %1033, label %1042, label %1037

1037:                                             ; preds = %1031
  %1038 = load i32, ptr %31, align 8, !tbaa !41
  %1039 = add nsw i32 %1038, -32000
  call void @_Z7StoreTTP7state_tiiijiiii(ptr %0, i32 %1039, i32 %64, i32 %70, i32 0, i32 %1034, i32 %1035, i32 %1036, i32 %3)
  %1040 = load i32, ptr %31, align 8, !tbaa !41
  %1041 = add nsw i32 %1040, -32000
  br label %1056

1042:                                             ; preds = %1031
  call void @_Z7StoreTTP7state_tiiijiiii(ptr %0, i32 0, i32 %64, i32 %70, i32 0, i32 %1034, i32 %1035, i32 %1036, i32 %3)
  br label %1056

1043:                                             ; preds = %1024, %756
  %1044 = phi i1 [ %758, %756 ], [ %1028, %1024 ]
  %1045 = phi i32 [ %619, %756 ], [ %1025, %1024 ]
  %1046 = load i32, ptr %46, align 4, !tbaa !45
  %1047 = icmp sgt i32 %1046, 98
  %1048 = xor i1 %1044, true
  %1049 = or i1 %1047, %1048
  %1050 = select i1 %1047, i32 0, i32 %1045
  br i1 %1049, label %1056, label %1051

1051:                                             ; preds = %1043
  %1052 = load i32, ptr %13, align 4, !tbaa !6
  %1053 = load i32, ptr %11, align 4, !tbaa !6
  %1054 = load i32, ptr %14, align 4, !tbaa !6
  %1055 = load i32, ptr %15, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr nonnull %0, i32 %1045, i32 %64, i32 %70, i32 %1052, i32 %1053, i32 %1054, i32 %1055, i32 %3)
  br label %1056

1056:                                             ; preds = %1051, %1043, %1042, %1037, %991, %923, %475, %243, %237, %224, %211, %172, %117, %110, %103, %77, %74, %72, %67, %61, %49, %36, %34
  %1057 = phi i32 [ %35, %34 ], [ %56, %49 ], [ %73, %72 ], [ 0, %36 ], [ %59, %61 ], [ %65, %67 ], [ %75, %74 ], [ %78, %77 ], [ %70, %475 ], [ 0, %172 ], [ %92, %103 ], [ %111, %110 ], [ %92, %117 ], [ %213, %224 ], [ 0, %211 ], [ %1041, %1037 ], [ 0, %1042 ], [ %1050, %1043 ], [ %1045, %1051 ], [ 0, %237 ], [ %64, %243 ], [ %926, %991 ], [ 0, %923 ]
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %28) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %27) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %26) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %25) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %24) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %23) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %22) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %21) #7
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %20) #7
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %19) #7
  ret i32 %1057
}

; Function Attrs: mustprogress uwtable
define internal i32 @_Z7qsearchP7state_tiiii(ptr %0, i32 %1, i32 %2, i32 %3, i32 %4) #0 {
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca [240 x i32], align 16
  %11 = alloca [240 x i32], align 16
  %12 = bitcast ptr %6 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %12) #7
  %13 = bitcast ptr %7 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %13) #7
  %14 = bitcast ptr %8 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %14) #7
  %15 = bitcast ptr %9 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %15) #7
  %16 = bitcast ptr %10 to ptr
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %16) #7
  %17 = bitcast ptr %11 to ptr
  call void @llvm.lifetime.start.p0(i64 960, ptr nonnull %17) #7
  %18 = getelementptr %struct.state_t, ptr %0, i64 0, i32 22, !intel-tbaa !44
  %19 = load i64, ptr %18, align 8, !tbaa !44
  %20 = add i64 %19, 1
  store i64 %20, ptr %18, align 8, !tbaa !44
  %21 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 23, !intel-tbaa !86
  %22 = load i64, ptr %21, align 8, !tbaa !86
  %23 = add i64 %22, 1
  store i64 %23, ptr %21, align 8, !tbaa !86
  %24 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 14, !intel-tbaa !41
  %25 = load i32, ptr %24, align 8, !tbaa !41
  %26 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 24, !intel-tbaa !87
  %27 = load i32, ptr %26, align 8, !tbaa !87
  %28 = icmp sgt i32 %25, %27
  br i1 %28, label %29, label %30

29:                                               ; preds = %5
  store i32 %25, ptr %26, align 8, !tbaa !87
  br label %30

30:                                               ; preds = %29, %5
  %31 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %20)
  %32 = icmp eq i32 %31, 0
  br i1 %32, label %33, label %378

33:                                               ; preds = %30
  %34 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(ptr nonnull @gamestate, ptr nonnull %0)
  %35 = icmp eq i32 %34, 0
  br i1 %35, label %36, label %40

36:                                               ; preds = %33
  %37 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 15, !intel-tbaa !45
  %38 = load i32, ptr %37, align 4, !tbaa !45
  %39 = icmp sgt i32 %38, 99
  br i1 %39, label %40, label %48

40:                                               ; preds = %36, %33
  %41 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 3), align 4, !tbaa !46
  %42 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 11, !intel-tbaa !21
  %43 = load i32, ptr %42, align 4, !tbaa !21
  %44 = icmp eq i32 %41, %43
  %45 = load i32, ptr @contempt, align 4
  %46 = sub nsw i32 0, %45
  %47 = select i1 %44, i32 %45, i32 %46
  br label %378

48:                                               ; preds = %36
  %49 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(ptr nonnull %0, ptr nonnull %8, i32 %1, i32 %2, ptr nonnull %7, ptr nonnull %9, ptr nonnull %9, ptr nonnull %9, ptr nonnull %9, i32 0)
  switch i32 %49, label %59 [
    i32 3, label %50
    i32 1, label %52
    i32 2, label %55
    i32 4, label %58
  ]

50:                                               ; preds = %48
  %51 = load i32, ptr %8, align 4, !tbaa !6
  br label %378

52:                                               ; preds = %48
  %53 = load i32, ptr %8, align 4, !tbaa !6
  %54 = icmp sgt i32 %53, %1
  br i1 %54, label %59, label %378

55:                                               ; preds = %48
  %56 = load i32, ptr %8, align 4, !tbaa !6
  %57 = icmp slt i32 %56, %2
  br i1 %57, label %59, label %378

58:                                               ; preds = %48
  store i32 65535, ptr %7, align 4, !tbaa !6
  br label %59

59:                                               ; preds = %58, %55, %52, %48
  %60 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 5), align 4, !tbaa !75
  %61 = shl nsw i32 %60, 1
  %62 = icmp slt i32 %61, %4
  br i1 %62, label %66, label %63

63:                                               ; preds = %59
  %64 = load i32, ptr %24, align 8, !tbaa !41
  %65 = icmp sgt i32 %64, 60
  br i1 %65, label %66, label %68

66:                                               ; preds = %63, %59
  %67 = call i32 @_Z4evalP7state_tiii(ptr nonnull %0, i32 %1, i32 %2, i32 0)
  br label %378

68:                                               ; preds = %63
  %69 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 25, !intel-tbaa !50
  %70 = sext i32 %64 to i64
  %71 = getelementptr inbounds [64 x i32], ptr %69, i64 0, i64 %70, !intel-tbaa !51
  %72 = load i32, ptr %71, align 4, !tbaa !52
  %73 = call i32 @_Z13retrieve_evalP7state_t(ptr nonnull %0)
  %74 = add nsw i32 %73, 50
  %75 = icmp ne i32 %72, 0
  br i1 %75, label %147, label %76

76:                                               ; preds = %68
  %77 = icmp slt i32 %73, %2
  br i1 %77, label %80, label %78

78:                                               ; preds = %76
  %79 = load i32, ptr %7, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr nonnull %0, i32 %73, i32 %1, i32 %2, i32 %79, i32 0, i32 0, i32 0, i32 0)
  br label %378

80:                                               ; preds = %76
  %81 = icmp sgt i32 %73, %1
  br i1 %81, label %150, label %82

82:                                               ; preds = %80
  %83 = add nsw i32 %73, 985
  %84 = icmp sgt i32 %83, %1
  br i1 %84, label %87, label %85

85:                                               ; preds = %82
  %86 = load i32, ptr %7, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr nonnull %0, i32 %83, i32 %1, i32 %2, i32 %86, i32 0, i32 0, i32 0, i32 0)
  br label %378

87:                                               ; preds = %82
  %88 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 8, !intel-tbaa !53
  %89 = getelementptr inbounds [13 x i32], ptr %88, i64 0, i64 0
  %90 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 11, !intel-tbaa !21
  %91 = load i32, ptr %90, align 4, !tbaa !21
  %92 = icmp eq i32 %91, 0
  br i1 %92, label %120, label %93

93:                                               ; preds = %87
  %94 = getelementptr inbounds i32, ptr %89, i64 10
  %95 = load i32, ptr %94, align 4, !tbaa !6
  %96 = icmp eq i32 %95, 0
  br i1 %96, label %97, label %150

97:                                               ; preds = %93
  %98 = getelementptr inbounds i32, ptr %89, i64 8
  %99 = load i32, ptr %98, align 4, !tbaa !6
  %100 = icmp eq i32 %99, 0
  br i1 %100, label %101, label %115

101:                                              ; preds = %97
  %102 = getelementptr inbounds i32, ptr %89, i64 12
  %103 = load i32, ptr %102, align 4, !tbaa !6
  %104 = icmp eq i32 %103, 0
  br i1 %104, label %105, label %112

105:                                              ; preds = %101
  %106 = getelementptr inbounds i32, ptr %89, i64 4
  %107 = load i32, ptr %106, align 4, !tbaa !6
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
  %119 = load i32, ptr %7, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr nonnull %0, i32 %116, i32 %1, i32 %2, i32 %119, i32 0, i32 0, i32 0, i32 0)
  br label %378

120:                                              ; preds = %87
  %121 = getelementptr inbounds i32, ptr %89, i64 9
  %122 = load i32, ptr %121, align 4, !tbaa !6
  %123 = icmp eq i32 %122, 0
  br i1 %123, label %124, label %150

124:                                              ; preds = %120
  %125 = getelementptr inbounds i32, ptr %89, i64 7
  %126 = load i32, ptr %125, align 4, !tbaa !6
  %127 = icmp eq i32 %126, 0
  br i1 %127, label %128, label %142

128:                                              ; preds = %124
  %129 = getelementptr inbounds i32, ptr %89, i64 11
  %130 = load i32, ptr %129, align 4, !tbaa !6
  %131 = icmp eq i32 %130, 0
  br i1 %131, label %132, label %139

132:                                              ; preds = %128
  %133 = getelementptr inbounds i32, ptr %89, i64 3
  %134 = load i32, ptr %133, align 4, !tbaa !6
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
  %146 = load i32, ptr %7, align 4, !tbaa !6
  call void @_Z7StoreTTP7state_tiiijiiii(ptr nonnull %0, i32 %143, i32 %1, i32 %2, i32 %146, i32 0, i32 0, i32 0, i32 0)
  br label %378

147:                                              ; preds = %68
  %148 = load i32, ptr %7, align 4, !tbaa !6
  %149 = icmp sgt i32 %3, -6
  br i1 %149, label %155, label %164

150:                                              ; preds = %142, %139, %136, %120, %115, %112, %109, %93, %80
  %151 = phi i32 [ %1, %120 ], [ %1, %142 ], [ %1, %136 ], [ %1, %139 ], [ %1, %93 ], [ %1, %115 ], [ %1, %109 ], [ %1, %112 ], [ %73, %80 ]
  %152 = sub nsw i32 %151, %74
  %153 = load i32, ptr %7, align 4, !tbaa !6
  %154 = icmp sgt i32 %3, -6
  br i1 %154, label %158, label %161

155:                                              ; preds = %147
  %156 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %157 = call i32 @_Z12gen_evasionsP7state_tPii(ptr nonnull %0, ptr nonnull %156, i32 %72)
  br label %167

158:                                              ; preds = %150
  %159 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %160 = call i32 @_Z12gen_capturesP7state_tPi(ptr nonnull %0, ptr nonnull %159)
  br label %167

161:                                              ; preds = %150
  %162 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %163 = call i32 @_Z12gen_capturesP7state_tPi(ptr nonnull %0, ptr nonnull %162)
  br label %167

164:                                              ; preds = %147
  %165 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %166 = call i32 @_Z12gen_evasionsP7state_tPii(ptr nonnull %0, ptr nonnull %165, i32 %72)
  br label %167

167:                                              ; preds = %164, %161, %158, %155
  %168 = phi i32 [ %153, %161 ], [ %148, %164 ], [ %148, %155 ], [ %153, %158 ]
  %169 = phi i32 [ %152, %161 ], [ 0, %164 ], [ 0, %155 ], [ %152, %158 ]
  %170 = phi i32 [ %151, %161 ], [ %1, %164 ], [ %1, %155 ], [ %151, %158 ]
  %171 = phi i1 [ false, %161 ], [ false, %164 ], [ false, %155 ], [ true, %158 ]
  %172 = phi i32 [ %163, %161 ], [ %166, %164 ], [ %157, %155 ], [ %160, %158 ]
  %173 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 0
  %174 = getelementptr inbounds [240 x i32], ptr %11, i64 0, i64 0
  %175 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 1
  %176 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 0
  %177 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 11
  %178 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 16
  %179 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 36
  %180 = getelementptr inbounds %struct.state_t, ptr %0, i64 0, i32 19
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
  %194 = call i32 @_Z15gen_good_checksP7state_tPi(ptr %0, ptr nonnull %173)
  br label %198

195:                                              ; preds = %184
  br i1 %189, label %196, label %198

196:                                              ; preds = %195
  %197 = call i32 @_Z9gen_quietP7state_tPi(ptr %0, ptr nonnull %173)
  br label %198

198:                                              ; preds = %196, %195, %193
  %199 = phi i32 [ %194, %193 ], [ %197, %196 ], [ %191, %195 ]
  %200 = load i32, ptr %7, align 4, !tbaa !6
  call fastcc void @_ZL16fast_order_movesP7state_tPiS1_ij(ptr %0, ptr nonnull %173, ptr nonnull %174, i32 %199, i32 %200)
  store i32 -1, ptr %6, align 4, !tbaa !6
  %201 = or i1 %188, %189
  br label %202

202:                                              ; preds = %358, %198
  %203 = phi i32 [ %359, %358 ], [ %185, %198 ]
  %204 = phi i32 [ %344, %358 ], [ %186, %198 ]
  %205 = phi i32 [ %345, %358 ], [ %187, %198 ]
  %206 = phi i32 [ %360, %358 ], [ %192, %198 ]
  br i1 %75, label %207, label %212

207:                                              ; preds = %202
  %208 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %6, ptr nonnull %174, ptr nonnull %173, i32 %199)
  %209 = icmp eq i32 %208, 0
  br i1 %209, label %361, label %210

210:                                              ; preds = %207
  %211 = load i32, ptr %6, align 4, !tbaa !6
  br label %304

212:                                              ; preds = %246, %202
  %213 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(ptr nonnull %6, ptr nonnull %174, ptr nonnull %173, i32 %199)
  %214 = icmp eq i32 %213, 0
  br i1 %214, label %361, label %215

215:                                              ; preds = %212
  br i1 %190, label %216, label %231

216:                                              ; preds = %215
  %217 = load i32, ptr %6, align 4, !tbaa !6
  %218 = sext i32 %217 to i64
  %219 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %218, !intel-tbaa !63
  %220 = load i32, ptr %219, align 4, !tbaa !63
  %221 = lshr i32 %220, 19
  %222 = and i32 %221, 15
  %223 = zext i32 %222 to i64
  %224 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %223, !intel-tbaa !67
  %225 = load i32, ptr %224, align 4, !tbaa !67
  %226 = call i32 @llvm.abs.i32(i32 %225, i1 true)
  %227 = icmp sle i32 %226, %169
  %228 = and i32 %220, 61440
  %229 = icmp eq i32 %228, 0
  %230 = and i1 %227, %229
  br i1 %230, label %361, label %231

231:                                              ; preds = %216, %215
  br i1 %201, label %232, label %270

232:                                              ; preds = %231
  %233 = load i32, ptr %6, align 4, !tbaa !6
  %234 = sext i32 %233 to i64
  %235 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %234, !intel-tbaa !63
  %236 = load i32, ptr %235, align 4, !tbaa !63
  %237 = lshr i32 %236, 19
  %238 = and i32 %237, 15
  %239 = icmp eq i32 %238, 13
  br i1 %239, label %247, label %240

240:                                              ; preds = %232
  %241 = zext i32 %238 to i64
  %242 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %241, !intel-tbaa !67
  %243 = load i32, ptr %242, align 4, !tbaa !67
  %244 = call i32 @llvm.abs.i32(i32 %243, i1 true)
  %245 = icmp sgt i32 %244, %169
  br i1 %245, label %246, label %247

246:                                              ; preds = %291, %248, %240
  br label %212, !llvm.loop !88

247:                                              ; preds = %240, %232
  br i1 %189, label %248, label %291

248:                                              ; preds = %247
  %249 = lshr i32 %236, 6
  %250 = and i32 %249, 63
  %251 = zext i32 %250 to i64
  %252 = getelementptr inbounds [64 x i32], ptr %175, i64 0, i64 %251, !intel-tbaa !51
  %253 = load i32, ptr %252, align 4, !tbaa !66
  %254 = add nsw i32 %253, -1
  %255 = and i32 %236, 63
  %256 = load i32, ptr %176, align 8, !tbaa !76
  %257 = sext i32 %256 to i64
  %258 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_hit, i64 0, i64 %257, !intel-tbaa !77
  %259 = sext i32 %254 to i64
  %260 = getelementptr inbounds [12 x [64 x i32]], ptr %258, i64 0, i64 %259, !intel-tbaa !80
  %261 = zext i32 %255 to i64
  %262 = getelementptr inbounds [64 x i32], ptr %260, i64 0, i64 %261, !intel-tbaa !51
  %263 = load i32, ptr %262, align 4, !tbaa !81
  %264 = getelementptr inbounds [8 x [12 x [64 x i32]]], ptr @history_tot, i64 0, i64 %257, !intel-tbaa !77
  %265 = getelementptr inbounds [12 x [64 x i32]], ptr %264, i64 0, i64 %259, !intel-tbaa !80
  %266 = getelementptr inbounds [64 x i32], ptr %265, i64 0, i64 %261, !intel-tbaa !51
  %267 = load i32, ptr %266, align 4, !tbaa !81
  %268 = sub nsw i32 %267, %263
  %269 = icmp slt i32 %263, %268
  br i1 %269, label %246, label %291

270:                                              ; preds = %231
  %271 = load i32, ptr %6, align 4, !tbaa !6
  %272 = sext i32 %271 to i64
  %273 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %272, !intel-tbaa !63
  %274 = load i32, ptr %273, align 4, !tbaa !63
  %275 = lshr i32 %274, 19
  %276 = and i32 %275, 15
  %277 = zext i32 %276 to i64
  %278 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %277, !intel-tbaa !67
  %279 = load i32, ptr %278, align 4, !tbaa !67
  %280 = call i32 @llvm.abs.i32(i32 %279, i1 true)
  %281 = lshr i32 %274, 6
  %282 = and i32 %281, 63
  %283 = zext i32 %282 to i64
  %284 = getelementptr inbounds [64 x i32], ptr %175, i64 0, i64 %283, !intel-tbaa !51
  %285 = load i32, ptr %284, align 4, !tbaa !66
  %286 = sext i32 %285 to i64
  %287 = getelementptr inbounds [14 x i32], ptr @material, i64 0, i64 %286, !intel-tbaa !67
  %288 = load i32, ptr %287, align 4, !tbaa !67
  %289 = call i32 @llvm.abs.i32(i32 %288, i1 true)
  %290 = icmp ult i32 %280, %289
  br i1 %290, label %291, label %304

291:                                              ; preds = %270, %248, %247
  %292 = phi i64 [ %234, %247 ], [ %272, %270 ], [ %234, %248 ]
  %293 = phi i32 [ %233, %247 ], [ %271, %270 ], [ %233, %248 ]
  %294 = load i32, ptr %177, align 4, !tbaa !21
  %295 = icmp eq i32 %294, 0
  %296 = zext i1 %295 to i32
  %297 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %292, !intel-tbaa !63
  %298 = load i32, ptr %297, align 4, !tbaa !63
  %299 = lshr i32 %298, 6
  %300 = and i32 %299, 63
  %301 = and i32 %298, 63
  %302 = call i32 @_Z3seeP7state_tiiii(ptr %0, i32 %296, i32 %300, i32 %301, i32 0)
  %303 = icmp slt i32 %302, -50
  br i1 %303, label %246, label %304

304:                                              ; preds = %291, %270, %210
  %305 = phi i32 [ %211, %210 ], [ %293, %291 ], [ %271, %270 ]
  %306 = sext i32 %305 to i64
  %307 = getelementptr inbounds [240 x i32], ptr %10, i64 0, i64 %306, !intel-tbaa !63
  %308 = load i32, ptr %307, align 4, !tbaa !63
  call void @_Z4makeP7state_ti(ptr %0, i32 %308)
  %309 = load i32, ptr %307, align 4, !tbaa !63
  %310 = call i32 @_Z11check_legalP7state_ti(ptr %0, i32 %309)
  %311 = icmp eq i32 %310, 0
  br i1 %311, label %343, label %312

312:                                              ; preds = %304
  %313 = load i64, ptr %178, align 8, !tbaa !22
  %314 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 15), align 4, !tbaa !70
  %315 = load i32, ptr %24, align 8, !tbaa !41
  %316 = add i32 %315, -1
  %317 = add i32 %316, %314
  %318 = sext i32 %317 to i64
  %319 = getelementptr inbounds [1000 x i64], ptr %179, i64 0, i64 %318, !intel-tbaa !71
  store i64 %313, ptr %319, align 8, !tbaa !72
  %320 = load i32, ptr %307, align 4, !tbaa !63
  %321 = sext i32 %316 to i64
  %322 = getelementptr inbounds [64 x i32], ptr %180, i64 0, i64 %321, !intel-tbaa !51
  store i32 %320, ptr %322, align 4, !tbaa !60
  %323 = call i32 @_Z8in_checkP7state_t(ptr %0)
  %324 = load i32, ptr %24, align 8, !tbaa !41
  %325 = sext i32 %324 to i64
  %326 = getelementptr inbounds [64 x i32], ptr %69, i64 0, i64 %325, !intel-tbaa !51
  store i32 %323, ptr %326, align 4, !tbaa !52
  %327 = sub nsw i32 0, %206
  %328 = sub i32 60, %206
  %329 = icmp ne i32 %323, 0
  %330 = zext i1 %329 to i32
  %331 = call i32 @_Z4evalP7state_tiii(ptr %0, i32 %181, i32 %328, i32 %330)
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
  %341 = call i32 @_Z7qsearchP7state_tiiii(ptr nonnull %0, i32 %181, i32 %327, i32 %340, i32 %182)
  %342 = sub nsw i32 0, %341
  br label %343

343:                                              ; preds = %339, %332, %304
  %344 = phi i32 [ %204, %304 ], [ 0, %339 ], [ 0, %332 ]
  %345 = phi i32 [ %205, %304 ], [ %342, %339 ], [ %205, %332 ]
  %346 = load i32, ptr %307, align 4, !tbaa !63
  call void @_Z6unmakeP7state_ti(ptr %0, i32 %346)
  %347 = load i32, ptr getelementptr inbounds (%struct.gamestate_t, ptr @gamestate, i64 0, i32 25), align 8, !tbaa !57
  %348 = icmp eq i32 %347, 0
  br i1 %348, label %349, label %378

349:                                              ; preds = %343
  %350 = icmp sle i32 %345, %206
  %351 = or i1 %311, %350
  br i1 %351, label %358, label %352

352:                                              ; preds = %349
  %353 = load i32, ptr %307, align 4, !tbaa !63
  %354 = call zeroext i16 @_Z12compact_movei(i32 %353)
  %355 = zext i16 %354 to i32
  %356 = icmp slt i32 %345, %2
  br i1 %356, label %358, label %357

357:                                              ; preds = %352
  call void @_Z7StoreTTP7state_tiiijiiii(ptr %0, i32 %345, i32 %1, i32 %2, i32 %355, i32 0, i32 0, i32 0, i32 0)
  br label %378

358:                                              ; preds = %352, %349
  %359 = phi i32 [ %203, %349 ], [ %355, %352 ]
  %360 = phi i32 [ %206, %349 ], [ %345, %352 ]
  br label %202, !llvm.loop !88

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
  %374 = load i32, ptr %24, align 8, !tbaa !41
  %375 = add nsw i32 %374, -32000
  br label %376

376:                                              ; preds = %373, %370
  %377 = phi i32 [ %375, %373 ], [ %206, %370 ]
  call void @_Z7StoreTTP7state_tiiijiiii(ptr %0, i32 %377, i32 %1, i32 %2, i32 %203, i32 0, i32 0, i32 0, i32 0)
  br label %378

378:                                              ; preds = %376, %357, %343, %145, %139, %136, %118, %112, %109, %85, %78, %66, %55, %52, %50, %40, %30
  %379 = phi i32 [ %47, %40 ], [ %67, %66 ], [ %345, %357 ], [ %377, %376 ], [ %73, %78 ], [ %83, %85 ], [ %51, %50 ], [ 0, %30 ], [ %53, %52 ], [ %56, %55 ], [ %140, %139 ], [ %137, %136 ], [ %113, %112 ], [ %110, %109 ], [ %116, %118 ], [ %143, %145 ], [ 0, %343 ]
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %17) #7
  call void @llvm.lifetime.end.p0(i64 960, ptr nonnull %16) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %15) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %14) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %13) #7
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %12) #7
  ret i32 %379
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
!44 = !{!11, !13, i64 4080}
!45 = !{!11, !7, i64 476}
!46 = !{!47, !7, i64 12}
!47 = !{!"struct@_ZTS11gamestate_t", !7, i64 0, !7, i64 4, !7, i64 8, !7, i64 12, !7, i64 16, !7, i64 20, !7, i64 24, !7, i64 28, !7, i64 32, !7, i64 36, !7, i64 40, !7, i64 44, !7, i64 48, !7, i64 52, !7, i64 56, !7, i64 60, !48, i64 64, !49, i64 4064, !13, i64 36064, !7, i64 36072, !7, i64 36076, !7, i64 36080, !7, i64 36084, !7, i64 36088, !7, i64 36092, !7, i64 36096, !7, i64 36100}
!48 = !{!"array@_ZTSA1000_i", !7, i64 0}
!49 = !{!"array@_ZTSA1000_6move_x", !17, i64 0}
!50 = !{!11, !12, i64 4100}
!51 = !{!12, !7, i64 0}
!52 = !{!11, !7, i64 4100}
!53 = !{!11, !15, i64 400}
!54 = !{!15, !7, i64 0}
!55 = !{!11, !7, i64 400}
!56 = !{!47, !7, i64 4}
!57 = !{!47, !7, i64 36096}
!58 = !{!11, !7, i64 456}
!59 = !{!11, !12, i64 2544}
!60 = !{!11, !7, i64 2544}
!61 = !{!11, !12, i64 2800}
!62 = !{!11, !7, i64 2800}
!63 = !{!64, !7, i64 0}
!64 = !{!"array@_ZTSA240_i", !7, i64 0}
!65 = distinct !{!65, !31}
!66 = !{!11, !7, i64 4}
!67 = !{!68, !7, i64 0}
!68 = !{!"array@_ZTSA14_i", !7, i64 0}
!69 = distinct !{!69, !31}
!70 = !{!47, !7, i64 60}
!71 = !{!20, !13, i64 0}
!72 = !{!11, !13, i64 4400}
!73 = distinct !{!73, !31}
!74 = distinct !{!74, !31}
!75 = !{!47, !7, i64 20}
!76 = !{!11, !7, i64 0}
!77 = !{!78, !79, i64 0}
!78 = !{!"array@_ZTSA8_A12_A64_i", !79, i64 0}
!79 = !{!"array@_ZTSA12_A64_i", !12, i64 0}
!80 = !{!79, !12, i64 0}
!81 = !{!78, !7, i64 0}
!82 = distinct !{!82, !31}
!83 = distinct !{!83, !31}
!84 = distinct !{!84, !31}
!85 = distinct !{!85, !31}
!86 = !{!11, !13, i64 4088}
!87 = !{!11, !7, i64 4096}
!88 = distinct !{!88, !31}

; end INTEL_FEATURE_SW_ADVANCED