; Checks that IPO Prefetch pass can work properly even under the conditions that debug information is now available.
;
; [Note]
; There are 3 functions in this LIT test. Each function has multiple debug intrinsics (in form of llvm.dbg.value).
; When a program is compiled with "-g" or -qopt-report, debug info is generated.
; The IPO Prefetch pass is now working properly in presence of debug info.
;
;

; REQUIRES: asserts


; *** Run command section ***
; RUN: opt < %s -intel-ipoprefetch -ipo-prefetch-be-lit-friendly=1 -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(intel-ipoprefetch)' -ipo-prefetch-be-lit-friendly=1 -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -S 2>&1 | FileCheck %s
;

; *** Check section 1 ***
; The LLVM-IR check below ensure that a call to the Prefetch.Backbone function is inserted inside host
; _Z6searchP7state_tiiiii.
; CHECK: define internal i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5) #5 !dbg [[DBG_0:![0-9]+]] {
; CHECK: call void @Prefetch.Backbone(%struct.state_t* %0), !dbg !1847
;

; *** Check section 2 ***
; The LLVM-IR check below ensure that a call to the Prefetch.Backbone function is inserted inside host
; _Z7qsearchP7state_tiiii.
; CHECK: define internal i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4) #5 !dbg [[DBG_0:![0-9]+]] {
; CHECK: call void @Prefetch.Backbone(%struct.state_t* %0), !dbg [[SUPPRESS_PRINT:![0-9]+]]

; *** Check section 3 ***
; The LLVM-IR check below ensures the prefetch function is generated.
;
; CHECK: define internal void @Prefetch.Backbone(%struct.state_t* nocapture %0) #7 !dbg [[DBG_0:![0-9]+]] !InlRpt.Suppress [[SUPPRESS_PRINT:![0-9]+]] {
;
;

; ModuleID = '<stdin>'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.pawntt_t = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i32, i32 }
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.state_t = type { i32, [64 x i32], i64, i64, i64, [13 x i64], i32, i32, [13 x i32], i32, i32, i32, i32, i32, i32, i32, i64, i64, [64 x %struct.move_x], [64 x i32], [64 x i32], [64 x %struct.anon], i64, i64, i32, [64 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i64] }
%struct.move_x = type { i32, i32, i32, i32, i64, i64 }
%struct.anon = type { i32, i32, i32, i32 }
%struct.gamestate_t = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1000 x i32], [1000 x %struct.move_x], i64, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.scoreboard_t = type { i32, i32, [8 x %struct.anon.0], [8 x i32], [8 x %struct.state_t] }
%struct.anon.0 = type { i32, i32, i32 }
%struct.ttentry_t = type { [4 x %struct.ttbucket_t] }
%struct.ttbucket_t = type { i32, i16, i16, i8, i8 }

@Mask = internal global [64 x i64] zeroinitializer, align 16, !dbg !0
@InvMask = internal global [64 x i64] zeroinitializer, align 16, !dbg !37
@FileMask = internal global [8 x i64] zeroinitializer, align 16, !dbg !43
@RankMask = internal global [8 x i64] zeroinitializer, align 16, !dbg !47
@FileUpMask = internal global [64 x i64] zeroinitializer, align 16, !dbg !68
@FileDownMask = internal global [64 x i64] zeroinitializer, align 16, !dbg !70
@DiagMaska1h8 = internal global [64 x i64] zeroinitializer, align 16, !dbg !39
@DiagMaska8h1 = internal global [64 x i64] zeroinitializer, align 16, !dbg !41
@_ZL19DiagonalLength_a1h8 = internal unnamed_addr constant [64 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1], align 16, !dbg !109
@_ZL19DiagonalLength_a8h1 = internal unnamed_addr constant [64 x i32] [i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 4, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 5, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 6, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 7, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8], align 16, !dbg !114
@KingSafetyMask = internal global [64 x i64] zeroinitializer, align 16, !dbg !80
@KingSafetyMask1 = internal global [64 x i64] zeroinitializer, align 16, !dbg !82
@KnightMoves = internal global [64 x i64] zeroinitializer, align 16, !dbg !12
@PawnAttacksBlack = internal global [64 x i64] zeroinitializer, align 16, !dbg !19
@PawnAttacksWhite = internal global [64 x i64] zeroinitializer, align 16, !dbg !21
@PawnMovesBlack = internal global [64 x i64] zeroinitializer, align 16, !dbg !23
@PawnMovesWhite = internal global [64 x i64] zeroinitializer, align 16, !dbg !25
@KingMoves = internal global [64 x i64] zeroinitializer, align 16, !dbg !17
@firstRankAttacks = internal global [64 x [8 x i8]] zeroinitializer, align 16, !dbg !34
@fillUpAttacks = internal global [64 x [8 x i64]] zeroinitializer, align 16, !dbg !27
@aFileAttacks = internal global [64 x [8 x i64]] zeroinitializer, align 16, !dbg !32
@RookMask = internal global [64 x i64] zeroinitializer, align 16, !dbg !57
@BishopMask = internal global [64 x i64] zeroinitializer, align 16, !dbg !59
@QueenMask = internal global [64 x i64] zeroinitializer, align 16, !dbg !61
@AboveMask = internal global [8 x i64] zeroinitializer, align 16, !dbg !49
@BelowMask = internal global [8 x i64] zeroinitializer, align 16, !dbg !51
@LeftMask = internal global [8 x i64] zeroinitializer, align 16, !dbg !53
@RightMask = internal global [8 x i64] zeroinitializer, align 16, !dbg !55
@WhiteSqMask = internal global i64 0, align 8, !dbg !88
@BlackSqMask = internal global i64 0, align 8, !dbg !90
@WhiteKingSide = internal global i64 0, align 8, !dbg !72
@WhiteQueenSide = internal global i64 0, align 8, !dbg !74
@BlackKingSide = internal global i64 0, align 8, !dbg !76
@BlackQueenSide = internal global i64 0, align 8, !dbg !78
@QSMask = internal global i64 0, align 8, !dbg !94
@KSMask = internal global i64 0, align 8, !dbg !92
@WhiteStrongSquareMask = internal global i64 0, align 8, !dbg !84
@BlackStrongSquareMask = internal global i64 0, align 8, !dbg !86
@CenterMask = internal global i64 0, align 8, !dbg !102
@KingFilesMask = internal global [8 x i64] zeroinitializer, align 16, !dbg !96
@KingPressureMask = internal global [64 x i64] zeroinitializer, align 16, !dbg !98
@KingPressureMask1 = internal global [64 x i64] zeroinitializer, align 16, !dbg !100
@SpaceMask = internal global [2 x i64] zeroinitializer, align 16, !dbg !104
@CastleMask = internal global [4 x i64] zeroinitializer, align 16, !dbg !63
@last_bit = internal global [65536 x i8] zeroinitializer, align 16, !dbg !328
@_ZZ14setup_epd_lineP11gamestate_tP7state_tPKcE11rankoffsets = internal unnamed_addr constant [8 x i32] [i32 0, i32 8, i32 16, i32 24, i32 32, i32 40, i32 48, i32 56], align 16, !dbg !341
@.str.3 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.4 = private unnamed_addr constant [20 x i8] c"Workload not found\0A\00", align 1
@.str.5 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.6 = private unnamed_addr constant [23 x i8] c"Analyzing %d plies...\0A\00", align 1
@.str.7 = private unnamed_addr constant [31 x i8] c"\0ANodes: %llu (%0.2f%% qnodes)\0A\00", align 1
@.str.8 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.str = private unnamed_addr constant [10 x i8] c"sjeng.log\00", align 1
@_ZL8w_passer = internal unnamed_addr constant [6 x i32] [i32 185, i32 120, i32 70, i32 40, i32 20, i32 15], align 16, !dbg !510
@_ZL23w_passer_pawn_supported = internal unnamed_addr constant [6 x i32] [i32 65, i32 25, i32 8, i32 -3, i32 -5, i32 -5], align 16, !dbg !518
@_ZL23w_passer_king_supported = internal unnamed_addr constant [6 x i32] [i32 -25, i32 25, i32 7, i32 5, i32 5, i32 4], align 16, !dbg !523
@_ZL13w_passer_free = internal unnamed_addr constant [6 x i32] [i32 185, i32 15, i32 10, i32 8, i32 3, i32 1], align 16, !dbg !525
@_ZL18w_passer_very_free = internal unnamed_addr constant [6 x i32] [i32 0, i32 80, i32 30, i32 15, i32 10, i32 10], align 16, !dbg !527
@_ZL16w_passer_blocked = internal unnamed_addr constant [6 x i32] [i32 -25, i32 -10, i32 -4, i32 0, i32 0, i32 0], align 16, !dbg !529
@material = internal constant [14 x i32] [i32 0, i32 85, i32 -85, i32 305, i32 -305, i32 40000, i32 -40000, i32 490, i32 -490, i32 935, i32 -935, i32 330, i32 -330, i32 0], align 16, !dbg !515
@_ZL6PawnTT = internal global [8 x [16384 x %struct.pawntt_t]] zeroinitializer, align 16, !dbg !531
@_ZL11w_candidate = internal unnamed_addr constant [6 x i32] [i32 0, i32 44, i32 12, i32 10, i32 3, i32 3], align 16, !dbg !537
@_ZL15psq_king_nopawn = internal unnamed_addr constant [64 x i32] [i32 -40, i32 -30, i32 -22, i32 -20, i32 -20, i32 -22, i32 -30, i32 -40, i32 -30, i32 -15, i32 -10, i32 -5, i32 -5, i32 -10, i32 -15, i32 -30, i32 -22, i32 -10, i32 5, i32 10, i32 10, i32 5, i32 -10, i32 -22, i32 -20, i32 -5, i32 10, i32 20, i32 20, i32 10, i32 -5, i32 -20, i32 -20, i32 -5, i32 10, i32 20, i32 20, i32 10, i32 -5, i32 -20, i32 -22, i32 -10, i32 5, i32 10, i32 10, i32 5, i32 -10, i32 -22, i32 -30, i32 -15, i32 -10, i32 -5, i32 -5, i32 -10, i32 -15, i32 -30, i32 -40, i32 -30, i32 -22, i32 -20, i32 -20, i32 -22, i32 -30, i32 -40], align 16, !dbg !557
@_ZL18psq_king_queenside = internal unnamed_addr constant [64 x i32] [i32 -40, i32 -30, i32 -20, i32 -20, i32 -30, i32 -40, i32 -55, i32 -75, i32 -25, i32 0, i32 0, i32 -5, i32 -15, i32 -30, i32 -50, i32 -70, i32 -20, i32 10, i32 15, i32 20, i32 -10, i32 -25, i32 -45, i32 -70, i32 -15, i32 15, i32 30, i32 30, i32 -5, i32 -15, i32 -35, i32 -60, i32 -15, i32 10, i32 20, i32 25, i32 -10, i32 -25, i32 -35, i32 -60, i32 -25, i32 5, i32 15, i32 15, i32 -15, i32 -25, i32 -45, i32 -75, i32 -30, i32 -10, i32 -10, i32 -10, i32 -20, i32 -30, i32 -50, i32 -75, i32 -50, i32 -20, i32 -15, i32 -30, i32 -40, i32 -50, i32 -60, i32 -80], align 16, !dbg !574
@_ZL17psq_king_kingside = internal unnamed_addr constant [64 x i32] [i32 -75, i32 -55, i32 -40, i32 -30, i32 -20, i32 -20, i32 -30, i32 -40, i32 -70, i32 -50, i32 -30, i32 -15, i32 -5, i32 0, i32 0, i32 -25, i32 -70, i32 -45, i32 -25, i32 -10, i32 20, i32 15, i32 10, i32 -20, i32 -60, i32 -35, i32 -15, i32 -5, i32 30, i32 30, i32 15, i32 -15, i32 -60, i32 -35, i32 -25, i32 -10, i32 25, i32 20, i32 10, i32 -15, i32 -75, i32 -45, i32 -25, i32 -15, i32 15, i32 15, i32 5, i32 -25, i32 -75, i32 -50, i32 -30, i32 -20, i32 -10, i32 -10, i32 -10, i32 -30, i32 -80, i32 -60, i32 -50, i32 -40, i32 -30, i32 -15, i32 -20, i32 -50], align 16, !dbg !572
@_ZL13wking_psq_end = internal unnamed_addr constant [64 x i32] [i32 0, i32 30, i32 35, i32 20, i32 20, i32 35, i32 30, i32 0, i32 -4, i32 25, i32 20, i32 24, i32 24, i32 20, i32 25, i32 -4, i32 -4, i32 14, i32 16, i32 6, i32 6, i32 16, i32 14, i32 -4, i32 -20, i32 0, i32 5, i32 -6, i32 -6, i32 5, i32 0, i32 -20, i32 -16, i32 -4, i32 0, i32 2, i32 2, i32 0, i32 -4, i32 -16, i32 -22, i32 -8, i32 -4, i32 0, i32 0, i32 -4, i32 -8, i32 -22, i32 -26, i32 -15, i32 -10, i32 -8, i32 -8, i32 -10, i32 -15, i32 -26, i32 -30, i32 -25, i32 -25, i32 -25, i32 -25, i32 -25, i32 -25, i32 -30], align 16, !dbg !570
@flip = internal constant [64 x i32] [i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7], align 16, !dbg !568
@psq_table = internal global [12 x [64 x i8]] zeroinitializer, align 16, !dbg !562
@_ZL9wpawn_psq = internal unnamed_addr constant <{ [38 x i32], [26 x i32] }> <{ [38 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 -10, i32 0, i32 0, i32 0, i32 0, i32 -10, i32 0, i32 0, i32 -5, i32 8, i32 12, i32 12, i32 8, i32 -5, i32 0, i32 0, i32 0, i32 6, i32 10, i32 10, i32 6, i32 0, i32 0, i32 0, i32 0, i32 4, i32 8, i32 8, i32 4], [26 x i32] zeroinitializer }>, align 16, !dbg !576
@_ZL15wknight_psq_end = internal unnamed_addr constant [64 x i32] [i32 -25, i32 -5, i32 0, i32 8, i32 8, i32 0, i32 -5, i32 -25, i32 -16, i32 4, i32 10, i32 16, i32 16, i32 10, i32 4, i32 7, i32 -1, i32 15, i32 20, i32 22, i32 22, i32 20, i32 15, i32 -7, i32 -5, i32 10, i32 16, i32 16, i32 16, i32 16, i32 10, i32 6, i32 -6, i32 5, i32 14, i32 13, i32 13, i32 14, i32 5, i32 -2, i32 -14, i32 -3, i32 4, i32 7, i32 7, i32 4, i32 -3, i32 -14, i32 -20, i32 -12, i32 -4, i32 -5, i32 -5, i32 -4, i32 -12, i32 -20, i32 -25, i32 -24, i32 -16, i32 -14, i32 -14, i32 -16, i32 -24, i32 -25], align 16, !dbg !578
@_ZL15wbishop_psq_end = internal unnamed_addr constant [64 x i32] [i32 -8, i32 -10, i32 -6, i32 -1, i32 -1, i32 -6, i32 -10, i32 -8, i32 -8, i32 -1, i32 -1, i32 0, i32 0, i32 -1, i32 -1, i32 -8, i32 -1, i32 5, i32 7, i32 8, i32 8, i32 7, i32 5, i32 -1, i32 -1, i32 4, i32 5, i32 10, i32 10, i32 5, i32 4, i32 -1, i32 2, i32 2, i32 3, i32 9, i32 9, i32 7, i32 3, i32 -5, i32 -2, i32 0, i32 6, i32 4, i32 4, i32 6, i32 0, i32 -2, i32 -5, i32 3, i32 1, i32 2, i32 2, i32 1, i32 3, i32 -5, i32 -10, i32 -6, i32 -8, i32 -8, i32 -8, i32 -8, i32 -6, i32 -10], align 16, !dbg !580
@_ZL13wrook_psq_end = internal unnamed_addr constant [64 x i32] [i32 5, i32 5, i32 7, i32 10, i32 10, i32 7, i32 5, i32 5, i32 8, i32 10, i32 14, i32 14, i32 14, i32 14, i32 10, i32 8, i32 1, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 1, i32 -1, i32 4, i32 3, i32 0, i32 0, i32 3, i32 4, i32 -1, i32 -6, i32 -1, i32 -3, i32 -2, i32 -2, i32 -3, i32 -1, i32 -6, i32 -10, i32 -4, i32 -8, i32 -8, i32 -8, i32 -8, i32 -4, i32 -10, i32 -15, i32 -12, i32 -9, i32 -8, i32 -8, i32 -9, i32 -12, i32 -15, i32 -15, i32 -10, i32 -8, i32 -8, i32 -8, i32 -8, i32 -10, i32 -15], align 16, !dbg !582
@_ZL14wqueen_psq_end = internal unnamed_addr constant [64 x i32] [i32 5, i32 12, i32 16, i32 16, i32 16, i32 16, i32 12, i32 5, i32 -10, i32 12, i32 20, i32 26, i32 26, i32 20, i32 12, i32 -10, i32 -5, i32 10, i32 15, i32 18, i32 18, i32 15, i32 10, i32 -5, i32 -15, i32 1, i32 10, i32 14, i32 14, i32 10, i32 1, i32 -15, i32 -7, i32 -4, i32 6, i32 9, i32 9, i32 6, i32 -4, i32 -7, i32 -12, i32 -8, i32 -2, i32 -4, i32 -4, i32 -2, i32 -8, i32 -12, i32 -12, i32 -12, i32 -13, i32 -10, i32 -10, i32 -13, i32 -12, i32 -12, i32 -20, i32 -25, i32 -25, i32 -10, i32 -10, i32 -25, i32 -25, i32 -20], align 16, !dbg !584
@history_hit = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16, !dbg !586
@history_tot = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16, !dbg !596
@history_h = internal global [8 x [12 x [64 x i32]]] zeroinitializer, align 16, !dbg !592
@_ZL8rc_index = internal unnamed_addr constant [14 x i32] [i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3, i32 4, i32 4, i32 5, i32 5, i32 2, i32 2, i32 0], align 16, !dbg !666
@_ZZ11search_rootP7state_tiiiE5bmove = internal unnamed_addr global i32 0, align 4, !dbg !639
@.str.37 = private unnamed_addr constant [20 x i8] c"info refutation %s \00", align 1
@_ZZ11search_rootP7state_tiiiE7changes = internal unnamed_addr global i32 0, align 4, !dbg !598
@_ZZ5thinkP11gamestate_tP7state_tE15lastsearchscore = internal unnamed_addr global i32 0, align 4, !dbg !641
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
@contempt = internal global i32 0, align 4, !dbg !668
@is_pondering = internal global i32 0, align 4, !dbg !681
@allow_pondering = internal global i32 0, align 4, !dbg !683
@is_analyzing = internal global i32 0, align 4, !dbg !685
@time_check_log = internal global i32 0, align 4, !dbg !707
@buffered_command = internal global [20 x [8192 x i8]] zeroinitializer, align 16, !dbg !673
@buffered_count = internal global i32 0, align 4, !dbg !679
@.str.58 = private unnamed_addr constant [30 x i8] c"Please specify the workfile.\0A\00", align 1
@TTSize = internal global i32 0, align 4, !dbg !687
@uci_mode = internal global i32 0, align 4, !dbg !689
@uci_chess960_mode = internal global i32 0, align 4, !dbg !691
@uci_showcurrline = internal global i32 0, align 4, !dbg !693
@uci_showrefutations = internal global i32 0, align 4, !dbg !695
@uci_limitstrength = internal global i32 0, align 4, !dbg !697
@uci_elo = internal global i32 0, align 4, !dbg !699
@uci_multipv = internal global i32 0, align 4, !dbg !701
@cfg_logging = internal global i32 0, align 4, !dbg !703
@cfg_logfile = internal global [512 x i8] zeroinitializer, align 16, !dbg !705
@EGTBHits = internal global i32 0, align 4, !dbg !711
@state = internal global %struct.state_t zeroinitializer, align 8, !dbg !715
@gamestate = internal global %struct.gamestate_t zeroinitializer, align 8, !dbg !720
@scoreboard = internal global %struct.scoreboard_t zeroinitializer, align 8, !dbg !722
@TTable = internal global %struct.ttentry_t* null, align 8, !dbg !740
@TTAge = internal global i32 0, align 4, !dbg !768
@zobrist = internal global [14 x [64 x i64]] zeroinitializer, align 16, !dbg !764
@.str.101 = private unnamed_addr constant [38 x i8] c"Out of memory allocating hashtables.\0A\00", align 1
@.str.1.102 = private unnamed_addr constant [50 x i8] c"Allocated %d hash entries, totalling %llu bytes.\0A\00", align 1
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
@.str.15 = private unnamed_addr constant [10 x i8] c"%c%d%c%dn\00", align 1
@.str.16 = private unnamed_addr constant [10 x i8] c"%c%d%c%dr\00", align 1
@.str.17 = private unnamed_addr constant [10 x i8] c"%c%d%c%db\00", align 1
@.str.18 = private unnamed_addr constant [10 x i8] c"%c%d%c%dq\00", align 1
@.str.19 = private unnamed_addr constant [42 x i8] c"+----+----+----+----+----+----+----+----+\00", align 1
@__const._Z13display_boardP7state_ti.piece_rep = private unnamed_addr constant [14 x i8*] [i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.20, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.21, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.22, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.23, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.24, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.25, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.26, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.27, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.28, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.29, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.30, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.31, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.32, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.33, i32 0, i32 0)], align 16
@.str.34 = private unnamed_addr constant [6 x i8] c"  %s\0A\00", align 1
@.str.35 = private unnamed_addr constant [5 x i8] c"%d |\00", align 1
@.str.36 = private unnamed_addr constant [6 x i8] c" %s |\00", align 1
@.str.37.133 = private unnamed_addr constant [7 x i8] c"\0A  %s\0A\00", align 1
@.str.38 = private unnamed_addr constant [45 x i8] c"\0A     a    b    c    d    e    f    g    h\0A\0A\00", align 1
@.str.39 = private unnamed_addr constant [45 x i8] c"\0A     h    g    f    e    d    c    b    a\0A\0A\00", align 1
@.str.55 = private unnamed_addr constant [2 x i8] c"a\00", align 1
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
@_ZZ9init_gameP11gamestate_tP7state_tE10init_board = internal unnamed_addr constant [64 x i32] [i32 8, i32 4, i32 12, i32 10, i32 6, i32 12, i32 4, i32 8, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 7, i32 3, i32 11, i32 9, i32 5, i32 11, i32 3, i32 7], align 16, !dbg !770
@_ZZL15hash_extract_pvP7state_tiPcE10levelstack = internal unnamed_addr global [65 x i64] zeroinitializer, align 16, !dbg !787
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
@multipv_strings = internal global [240 x [512 x i8]] zeroinitializer, align 16, !dbg !781
@multipv_scores = internal global [240 x i32] zeroinitializer, align 16, !dbg !785
@.str.52 = private unnamed_addr constant [66 x i8] c"depth %d seldepth %d time %d nodes %llu tbhits %d score cp %d pv \00", align 1
@.str.53 = private unnamed_addr constant [68 x i8] c"depth %d seldepth %d time %d nodes %llu tbhits %d score mate %d pv \00", align 1
@.str.54 = private unnamed_addr constant [17 x i8] c"info multipv %d \00", align 1
@.str.58.158 = private unnamed_addr constant [74 x i8] c"Deep Sjeng version 3.2 SPEC, Copyright (C) 2000-2009 Gian-Carlo Pascutto\0A\00", align 1
@_ZL2s1 = internal unnamed_addr global i32 0, align 4, !dbg !807
@_ZL2s2 = internal unnamed_addr global i32 0, align 4, !dbg !809
@_ZL2s3 = internal unnamed_addr global i32 0, align 4, !dbg !811
@root_scores = internal global [240 x i32] zeroinitializer, align 16, !dbg !779
; Function Attrs: nofree norecurse nounwind uwtable
define internal i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nocapture %0, i32* nocapture %1, i32 %2, i32 %3, i32* nocapture %4, i32* nocapture %5, i32* nocapture %6, i32* nocapture %7, i32* nocapture %8, i32 %9) #0 !dbg !842 {
  call void @llvm.dbg.value(metadata %struct.state_t* %0, metadata !847, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32* %1, metadata !848, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32 %2, metadata !849, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32 %3, metadata !850, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32* %4, metadata !851, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32* %5, metadata !852, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32* %6, metadata !853, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32* %7, metadata !854, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32* %8, metadata !855, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32 %9, metadata !856, metadata !DIExpression()), !dbg !864
  store i32 1, i32* %6, align 4, !dbg !865, !tbaa !866
  %11 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 26, !dbg !870, !intel-tbaa !871
  %12 = load i32, i32* %11, align 4, !dbg !882, !tbaa !871
  %13 = add i32 %12, 1, !dbg !882
  store i32 %13, i32* %11, align 4, !dbg !882, !tbaa !871
  %14 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !dbg !883, !intel-tbaa !885
  %15 = load i32, i32* %14, align 4, !dbg !883, !tbaa !885
  %16 = icmp eq i32 %15, 0, !dbg !886
  %17 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16, !dbg !887
  %18 = load i64, i64* %17, align 8, !dbg !887, !tbaa !888
  %19 = zext i1 %16 to i64, !dbg !889
  %20 = add i64 %18, %19, !dbg !889
  call void @llvm.dbg.value(metadata i64 %20, metadata !859, metadata !DIExpression()), !dbg !864
  %21 = trunc i64 %20 to i32, !dbg !890
  call void @llvm.dbg.value(metadata i32 %21, metadata !860, metadata !DIExpression()), !dbg !864
  %22 = load %struct.ttentry_t*, %struct.ttentry_t** @TTable, align 8, !dbg !891, !tbaa !892
  %23 = load i32, i32* @TTSize, align 4, !dbg !894, !tbaa !866
  %24 = urem i32 %21, %23, !dbg !895
  %25 = zext i32 %24 to i64, !dbg !891
  %26 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %22, i64 %25, !dbg !891
  call void @llvm.dbg.value(metadata %struct.ttentry_t* %26, metadata !861, metadata !DIExpression()), !dbg !864
  %27 = lshr i64 %20, 32, !dbg !896
  call void @llvm.dbg.value(metadata i64 %27, metadata !859, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i32 0, metadata !858, metadata !DIExpression()), !dbg !864
  %28 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %26, i64 0, i32 0, !dbg !897, !intel-tbaa !902
  %29 = trunc i64 %27 to i32, !dbg !907
  br label %33, !dbg !908

30:                                               ; preds = %33
  %31 = add nuw nsw i64 %34, 1, !dbg !909
  call void @llvm.dbg.value(metadata i64 %31, metadata !858, metadata !DIExpression()), !dbg !864
  call void @llvm.dbg.value(metadata i64 %31, metadata !858, metadata !DIExpression()), !dbg !864
  %32 = icmp eq i64 %31, 4, !dbg !910
  br i1 %32, label %132, label %33, !dbg !908, !llvm.loop !911

33:                                               ; preds = %30, %10
  %34 = phi i64 [ 0, %10 ], [ %31, %30 ]
  call void @llvm.dbg.value(metadata i64 %34, metadata !858, metadata !DIExpression()), !dbg !864
  %35 = getelementptr inbounds [4 x %struct.ttbucket_t], [4 x %struct.ttbucket_t]* %28, i64 0, i64 %34, !dbg !913, !intel-tbaa !914
  %36 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 0, !dbg !915, !intel-tbaa !916
  %37 = load i32, i32* %36, align 4, !dbg !915, !tbaa !917
  %38 = icmp eq i32 %37, %29, !dbg !907
  call void @llvm.dbg.value(metadata i64 %34, metadata !858, metadata !DIExpression(DW_OP_plus_uconst, 1, DW_OP_stack_value)), !dbg !864
  br i1 %38, label %39, label %30, !dbg !918

39:                                               ; preds = %33
  %40 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 27, !dbg !919, !intel-tbaa !921
  %41 = load i32, i32* %40, align 8, !dbg !922, !tbaa !921
  %42 = add i32 %41, 1, !dbg !922
  store i32 %42, i32* %40, align 8, !dbg !922, !tbaa !921
  call void @llvm.dbg.value(metadata %struct.ttbucket_t* %35, metadata !862, metadata !DIExpression()), !dbg !864
  %43 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 4, !dbg !923
  %44 = load i8, i8* %43, align 1, !dbg !923, !tbaa !925
  %45 = lshr i8 %44, 5, !dbg !923
  %46 = and i8 %45, 3, !dbg !923
  %47 = zext i8 %46 to i32, !dbg !926
  %48 = load i32, i32* @TTAge, align 4, !dbg !927, !tbaa !866
  %49 = icmp eq i32 %48, %47, !dbg !928
  br i1 %49, label %56, label %50, !dbg !929

50:                                               ; preds = %39
  %51 = trunc i32 %48 to i8, !dbg !930
  %52 = shl i8 %51, 5, !dbg !932
  %53 = and i8 %52, 96, !dbg !932
  %54 = and i8 %44, -97, !dbg !932
  %55 = or i8 %53, %54, !dbg !932
  store i8 %55, i8* %43, align 1, !dbg !932, !tbaa !925
  br label %56, !dbg !933

56:                                               ; preds = %50, %39
  %57 = phi i8 [ %44, %39 ], [ %55, %50 ], !dbg !934
  %58 = and i8 %57, 6, !dbg !934
  %59 = icmp eq i8 %58, 2, !dbg !936
  br i1 %59, label %60, label %72, !dbg !937

60:                                               ; preds = %56
  %61 = add nsw i32 %9, -16, !dbg !938
  %62 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 3, !dbg !939, !intel-tbaa !940
  %63 = load i8, i8* %62, align 4, !dbg !939, !tbaa !941
  %64 = zext i8 %63 to i32, !dbg !942
  %65 = icmp sgt i32 %61, %64, !dbg !943
  br i1 %65, label %72, label %66, !dbg !944

66:                                               ; preds = %60
  %67 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !dbg !945, !intel-tbaa !946
  %68 = load i16, i16* %67, align 4, !dbg !945, !tbaa !947
  %69 = sext i16 %68 to i32, !dbg !948
  %70 = icmp slt i32 %69, %3, !dbg !949
  br i1 %70, label %71, label %72, !dbg !950

71:                                               ; preds = %66
  store i32 0, i32* %6, align 4, !dbg !951, !tbaa !866
  br label %72, !dbg !953

72:                                               ; preds = %71, %66, %60, %56
  %73 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 3, !dbg !954, !intel-tbaa !940
  %74 = load i8, i8* %73, align 4, !dbg !954, !tbaa !941
  %75 = zext i8 %74 to i32, !dbg !956
  %76 = icmp slt i32 %75, %9, !dbg !957
  br i1 %76, label %109, label %77, !dbg !958

77:                                               ; preds = %72
  %78 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !dbg !959, !intel-tbaa !946
  %79 = load i16, i16* %78, align 4, !dbg !959, !tbaa !947
  %80 = sext i16 %79 to i32, !dbg !961
  store i32 %80, i32* %1, align 4, !dbg !962, !tbaa !866
  %81 = icmp sgt i16 %79, 31500, !dbg !963
  br i1 %81, label %82, label %87, !dbg !965

82:                                               ; preds = %77
  %83 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !dbg !966, !intel-tbaa !968
  %84 = load i32, i32* %83, align 8, !dbg !966, !tbaa !968
  %85 = add nsw i32 %80, 1, !dbg !969
  %86 = sub i32 %85, %84, !dbg !970
  store i32 %86, i32* %1, align 4, !dbg !970, !tbaa !866
  br label %94, !dbg !971

87:                                               ; preds = %77
  %88 = icmp slt i16 %79, -31500, !dbg !972
  br i1 %88, label %89, label %94, !dbg !974

89:                                               ; preds = %87
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !dbg !975, !intel-tbaa !968
  %91 = load i32, i32* %90, align 8, !dbg !975, !tbaa !968
  %92 = add nsw i32 %80, -1, !dbg !977
  %93 = add i32 %92, %91, !dbg !978
  store i32 %93, i32* %1, align 4, !dbg !978, !tbaa !866
  br label %94, !dbg !979

94:                                               ; preds = %89, %87, %82
  %95 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 2, !dbg !980, !intel-tbaa !981
  %96 = load i16, i16* %95, align 2, !dbg !980, !tbaa !982
  %97 = zext i16 %96 to i32, !dbg !983
  store i32 %97, i32* %4, align 4, !dbg !984, !tbaa !866
  %98 = and i8 %57, 1, !dbg !985
  %99 = zext i8 %98 to i32, !dbg !986
  store i32 %99, i32* %5, align 4, !dbg !987, !tbaa !866
  %100 = lshr i8 %57, 3, !dbg !988
  %101 = and i8 %100, 1, !dbg !988
  %102 = zext i8 %101 to i32, !dbg !989
  store i32 %102, i32* %7, align 4, !dbg !990, !tbaa !866
  %103 = lshr i8 %57, 4, !dbg !991
  %104 = and i8 %103, 1, !dbg !991
  %105 = zext i8 %104 to i32, !dbg !992
  store i32 %105, i32* %8, align 4, !dbg !993, !tbaa !866
  %106 = lshr i8 %57, 1, !dbg !994
  %107 = and i8 %106, 3, !dbg !994
  %108 = zext i8 %107 to i32, !dbg !995
  call void @llvm.dbg.value(metadata i32 %108, metadata !857, metadata !DIExpression()), !dbg !864
  br label %132, !dbg !996

109:                                              ; preds = %72
  %110 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 2, !dbg !997, !intel-tbaa !981
  %111 = load i16, i16* %110, align 2, !dbg !997, !tbaa !982
  %112 = zext i16 %111 to i32, !dbg !999
  store i32 %112, i32* %4, align 4, !dbg !1000, !tbaa !866
  %113 = and i8 %57, 1, !dbg !1001
  %114 = zext i8 %113 to i32, !dbg !1002
  store i32 %114, i32* %5, align 4, !dbg !1003, !tbaa !866
  %115 = lshr i8 %57, 3, !dbg !1004
  %116 = and i8 %115, 1, !dbg !1004
  %117 = zext i8 %116 to i32, !dbg !1005
  store i32 %117, i32* %7, align 4, !dbg !1006, !tbaa !866
  %118 = lshr i8 %57, 4, !dbg !1007
  %119 = and i8 %118, 1, !dbg !1007
  %120 = zext i8 %119 to i32, !dbg !1008
  store i32 %120, i32* %8, align 4, !dbg !1009, !tbaa !866
  %121 = lshr i8 %57, 1, !dbg !1010
  %122 = and i8 %121, 3, !dbg !1010
  switch i8 %122, label %125 [
    i8 1, label %123
    i8 2, label %124
  ], !dbg !1012

123:                                              ; preds = %109
  store i32 -1000000, i32* %1, align 4, !dbg !1013, !tbaa !866
  br label %129, !dbg !1015

124:                                              ; preds = %109
  store i32 1000000, i32* %1, align 4, !dbg !1016, !tbaa !866
  br label %129, !dbg !1019

125:                                              ; preds = %109
  %126 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %35, i64 0, i32 1, !dbg !1020, !intel-tbaa !946
  %127 = load i16, i16* %126, align 4, !dbg !1020, !tbaa !947
  %128 = sext i16 %127 to i32, !dbg !1022
  store i32 %128, i32* %1, align 4, !dbg !1023, !tbaa !866
  br label %129

129:                                              ; preds = %125, %124, %123
  %130 = trunc i32 %9 to i8, !dbg !1024
  store i8 %130, i8* %73, align 4, !dbg !1025, !tbaa !941
  %131 = and i8 %57, -7, !dbg !1026
  store i8 %131, i8* %43, align 1, !dbg !1026, !tbaa !925
  br label %132, !dbg !1027

132:                                              ; preds = %129, %94, %30
  %133 = phi i32 [ %108, %94 ], [ 0, %129 ], [ 4, %30 ]
  ret i32 %133, !dbg !1028
}
; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1
; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.label(metadata) #1
; Function Attrs: nounwind uwtable
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #2
; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1
; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z11check_legalP7state_ti(%struct.state_t*, i32) #0
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
; Function Attrs: uwtable
declare void @_Z4makeP7state_ti(%struct.state_t*, i32) #5
; Function Attrs: uwtable
declare void @_Z6unmakeP7state_ti(%struct.state_t*, i32) #5
; Function Attrs: uwtable
declare void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nocapture, i32, i32, i32, i32, i32, i32, i32, i32) #5
; Function Attrs: norecurse nounwind readonly uwtable
declare i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nocapture readonly, %struct.state_t* nocapture readonly) #3
; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z8in_checkP7state_t(%struct.state_t* nocapture readonly) #0
; Function Attrs: nofree norecurse nounwind uwtable
declare i32 @_Z9gen_quietP7state_tPi(%struct.state_t*, i32*) #0
; Function Attrs: nofree norecurse nounwind uwtable
declare fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t*, i32* nocapture readonly, i32* nocapture, i32, i32) unnamed_addr #0
; Function Attrs: uwtable
declare fastcc void @_ZL12history_goodP7state_tii(%struct.state_t* nocapture, i32, i32) unnamed_addr #5
; Function Attrs: uwtable
declare fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nocapture, i32* nocapture, i32* nocapture, i32) unnamed_addr #5
; Function Attrs: nofree norecurse nounwind uwtable
declare fastcc void @_ZL16fast_order_movesP7state_tPiS1_ij(%struct.state_t* nocapture readonly, i32* nocapture readonly, i32* nocapture, i32, i32) unnamed_addr #0
; Function Attrs: nofree norecurse nounwind uwtable
declare fastcc i32 @_ZL17search_time_checkP7state_t(i64) unnamed_addr #0
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #6
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #6
; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %puts = tail call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([20 x i8], [20 x i8]* @.str.4, i64 0, i64 0))
  ret i32 0
}
; Function Attrs: uwtable
define internal i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5) #5 !dbg !1029 {
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
  call void @llvm.dbg.value(metadata %struct.state_t* %0, metadata !1033, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1, metadata !1034, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %2, metadata !1035, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %3, metadata !1036, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %4, metadata !1037, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %5, metadata !1038, metadata !DIExpression()), !dbg !1120
  %19 = bitcast [240 x i32]* %7 to i8*, !dbg !1121
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %19) #7, !dbg !1121
  call void @llvm.dbg.declare(metadata [240 x i32]* %7, metadata !1039, metadata !DIExpression()), !dbg !1122
  %20 = bitcast [240 x i32]* %8 to i8*, !dbg !1123
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %20) #7, !dbg !1123
  call void @llvm.dbg.declare(metadata [240 x i32]* %8, metadata !1040, metadata !DIExpression()), !dbg !1124
  %21 = bitcast i32* %9 to i8*, !dbg !1125
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %21) #7, !dbg !1125
  %22 = bitcast i32* %10 to i8*, !dbg !1126
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %22) #7, !dbg !1126
  %23 = bitcast i32* %11 to i8*, !dbg !1126
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %23) #7, !dbg !1126
  %24 = bitcast i32* %12 to i8*, !dbg !1126
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %24) #7, !dbg !1126
  %25 = bitcast i32* %13 to i8*, !dbg !1127
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %25) #7, !dbg !1127
  %26 = bitcast i32* %14 to i8*, !dbg !1128
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %26) #7, !dbg !1128
  %27 = bitcast i32* %15 to i8*, !dbg !1129
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %27) #7, !dbg !1129
  %28 = bitcast [240 x i32]* %16 to i8*, !dbg !1130
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %28) #7, !dbg !1130
  call void @llvm.dbg.declare(metadata [240 x i32]* %16, metadata !1069, metadata !DIExpression()), !dbg !1131
  %29 = icmp slt i32 %3, 1, !dbg !1132
  br i1 %29, label %34, label %30, !dbg !1134

30:                                               ; preds = %6
  %31 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !dbg !1135, !intel-tbaa !968
  %32 = load i32, i32* %31, align 8, !dbg !1135, !tbaa !968
  %33 = icmp sgt i32 %32, 59, !dbg !1136
  br i1 %33, label %34, label %36, !dbg !1137

34:                                               ; preds = %30, %6
  %35 = tail call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 0, i32 0), !dbg !1138
  br label %1119, !dbg !1140

36:                                               ; preds = %30
  %37 = getelementptr %struct.state_t, %struct.state_t* %0, i64 0, i32 22, !dbg !1141, !intel-tbaa !1142
  %38 = load i64, i64* %37, align 8, !dbg !1143, !tbaa !1142
  %39 = add i64 %38, 1, !dbg !1143
  store i64 %39, i64* %37, align 8, !dbg !1143, !tbaa !1142
  %40 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %39), !dbg !1144
  %41 = icmp eq i32 %40, 0, !dbg !1144
  br i1 %41, label %42, label %1119, !dbg !1146

42:                                               ; preds = %36
  %43 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nonnull @gamestate, %struct.state_t* nonnull %0), !dbg !1147
  %44 = icmp eq i32 %43, 0, !dbg !1147
  br i1 %44, label %45, label %49, !dbg !1149

45:                                               ; preds = %42
  %46 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15, !dbg !1150, !intel-tbaa !1151
  %47 = load i32, i32* %46, align 4, !dbg !1150, !tbaa !1151
  %48 = icmp sgt i32 %47, 99, !dbg !1152
  br i1 %48, label %49, label %57, !dbg !1153

49:                                               ; preds = %45, %42
  %50 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4, !dbg !1154, !tbaa !1156
  %51 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !dbg !1160, !intel-tbaa !885
  %52 = load i32, i32* %51, align 4, !dbg !1160, !tbaa !885
  %53 = icmp eq i32 %50, %52, !dbg !1161
  %54 = load i32, i32* @contempt, align 4, !dbg !1162, !tbaa !866
  %55 = sub nsw i32 0, %54, !dbg !1163
  %56 = select i1 %53, i32 %54, i32 %55, !dbg !1163
  br label %1119, !dbg !1164

57:                                               ; preds = %45
  %58 = load i32, i32* %31, align 8, !dbg !1165, !tbaa !968
  %59 = add nsw i32 %58, -32000, !dbg !1166
  call void @llvm.dbg.value(metadata i32 %59, metadata !1057, metadata !DIExpression()), !dbg !1120
  %60 = icmp sgt i32 %59, %1, !dbg !1167
  br i1 %60, label %61, label %63, !dbg !1169

61:                                               ; preds = %57
  call void @llvm.dbg.value(metadata i32 %59, metadata !1034, metadata !DIExpression()), !dbg !1120
  %62 = icmp slt i32 %59, %2, !dbg !1170
  br i1 %62, label %63, label %1119, !dbg !1173

63:                                               ; preds = %61, %57
  %64 = phi i32 [ %59, %61 ], [ %1, %57 ]
  call void @llvm.dbg.value(metadata i32 %64, metadata !1034, metadata !DIExpression()), !dbg !1120
  %65 = sub i32 31999, %58, !dbg !1174
  call void @llvm.dbg.value(metadata i32 %65, metadata !1057, metadata !DIExpression()), !dbg !1120
  %66 = icmp slt i32 %65, %2, !dbg !1175
  br i1 %66, label %67, label %69, !dbg !1177

67:                                               ; preds = %63
  call void @llvm.dbg.value(metadata i32 %65, metadata !1035, metadata !DIExpression()), !dbg !1120
  %68 = icmp sgt i32 %65, %64, !dbg !1178
  br i1 %68, label %69, label %1119, !dbg !1181

69:                                               ; preds = %67, %63
  %70 = phi i32 [ %65, %67 ], [ %2, %63 ]
  call void @llvm.dbg.value(metadata i32 %70, metadata !1035, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32* %10, metadata !1047, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  call void @llvm.dbg.value(metadata i32* %11, metadata !1048, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  call void @llvm.dbg.value(metadata i32* %12, metadata !1049, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  call void @llvm.dbg.value(metadata i32* %13, metadata !1052, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  call void @llvm.dbg.value(metadata i32* %14, metadata !1063, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  call void @llvm.dbg.value(metadata i32* %15, metadata !1065, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %71 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nonnull %0, i32* nonnull %10, i32 %64, i32 %70, i32* nonnull %13, i32* nonnull %11, i32* nonnull %12, i32* nonnull %14, i32* nonnull %15, i32 %3), !dbg !1182
  switch i32 %71, label %85 [
    i32 3, label %72
    i32 1, label %74
    i32 2, label %77
    i32 0, label %80
    i32 4, label %84
  ], !dbg !1183

72:                                               ; preds = %69
  %73 = load i32, i32* %10, align 4, !dbg !1184, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %73, metadata !1047, metadata !DIExpression()), !dbg !1120
  br label %1119, !dbg !1186

74:                                               ; preds = %69
  %75 = load i32, i32* %10, align 4, !dbg !1187, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %75, metadata !1047, metadata !DIExpression()), !dbg !1120
  %76 = icmp sgt i32 %75, %64, !dbg !1189
  br i1 %76, label %85, label %1119, !dbg !1190

77:                                               ; preds = %69
  %78 = load i32, i32* %10, align 4, !dbg !1191, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %78, metadata !1047, metadata !DIExpression()), !dbg !1120
  %79 = icmp slt i32 %78, %70, !dbg !1193
  br i1 %79, label %85, label %1119, !dbg !1194

80:                                               ; preds = %69
  %81 = load i32, i32* %10, align 4, !dbg !1195, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %81, metadata !1047, metadata !DIExpression()), !dbg !1120
  %82 = icmp slt i32 %81, %70, !dbg !1197
  %83 = select i1 %82, i32 %5, i32 1, !dbg !1198
  br label %85, !dbg !1198

84:                                               ; preds = %69
  call void @llvm.dbg.value(metadata i32 65535, metadata !1052, metadata !DIExpression()), !dbg !1120
  store i32 65535, i32* %13, align 4, !dbg !1199, !tbaa !866
  call void @llvm.dbg.value(metadata i32 0, metadata !1048, metadata !DIExpression()), !dbg !1120
  store i32 0, i32* %11, align 4, !dbg !1200, !tbaa !866
  call void @llvm.dbg.value(metadata i32 0, metadata !1063, metadata !DIExpression()), !dbg !1120
  store i32 0, i32* %14, align 4, !dbg !1201, !tbaa !866
  call void @llvm.dbg.value(metadata i32 0, metadata !1065, metadata !DIExpression()), !dbg !1120
  store i32 0, i32* %15, align 4, !dbg !1202, !tbaa !866
  br label %85, !dbg !1203

85:                                               ; preds = %84, %80, %77, %74, %69
  %86 = phi i32 [ %5, %69 ], [ %5, %84 ], [ 0, %74 ], [ 1, %77 ], [ %83, %80 ]
  call void @llvm.dbg.value(metadata i32 %86, metadata !1038, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %64, metadata !1056, metadata !DIExpression()), !dbg !1120
  %87 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25, !dbg !1204, !intel-tbaa !1205
  %88 = load i32, i32* %31, align 8, !dbg !1206, !tbaa !968
  %89 = sext i32 %88 to i64, !dbg !1207
  %90 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %89, !dbg !1207, !intel-tbaa !1208
  %91 = load i32, i32* %90, align 4, !dbg !1207, !tbaa !1209
  call void @llvm.dbg.value(metadata i32 %91, metadata !1053, metadata !DIExpression()), !dbg !1120
  %92 = call i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nonnull %0), !dbg !1210
  call void @llvm.dbg.value(metadata i32 %92, metadata !1070, metadata !DIExpression()), !dbg !1120
  %93 = icmp ne i32 %91, 0, !dbg !1211
  %94 = xor i1 %93, true, !dbg !1213
  %95 = add nsw i32 %64, 1, !dbg !1214
  %96 = icmp eq i32 %70, %95, !dbg !1215
  %97 = and i1 %96, %94, !dbg !1213
  br i1 %97, label %98, label %122, !dbg !1213

98:                                               ; preds = %85
  %99 = icmp slt i32 %3, 5, !dbg !1216
  br i1 %99, label %100, label %112, !dbg !1219

100:                                              ; preds = %98
  %101 = add nsw i32 %92, -75, !dbg !1220
  %102 = icmp slt i32 %101, %70, !dbg !1223
  br i1 %102, label %108, label %103, !dbg !1224

103:                                              ; preds = %100
  %104 = load i32, i32* %13, align 4, !dbg !1225, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %104, metadata !1052, metadata !DIExpression()), !dbg !1120
  %105 = load i32, i32* %11, align 4, !dbg !1227, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %105, metadata !1048, metadata !DIExpression()), !dbg !1120
  %106 = load i32, i32* %14, align 4, !dbg !1228, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %106, metadata !1063, metadata !DIExpression()), !dbg !1120
  %107 = load i32, i32* %15, align 4, !dbg !1229, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %107, metadata !1065, metadata !DIExpression()), !dbg !1120
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %101, i32 %64, i32 %70, i32 %104, i32 %105, i32 %106, i32 %107, i32 %3), !dbg !1230
  br label %1119, !dbg !1231

108:                                              ; preds = %100
  %109 = icmp slt i32 %92, %70, !dbg !1232
  br i1 %109, label %110, label %122, !dbg !1234

110:                                              ; preds = %108
  %111 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %64, i32 %70, i32 0, i32 0), !dbg !1235
  br label %1119, !dbg !1237

112:                                              ; preds = %98
  %113 = icmp slt i32 %3, 9, !dbg !1238
  br i1 %113, label %114, label %122, !dbg !1240

114:                                              ; preds = %112
  %115 = add nsw i32 %92, -125, !dbg !1241
  %116 = icmp slt i32 %115, %70, !dbg !1244
  br i1 %116, label %122, label %117, !dbg !1245

117:                                              ; preds = %114
  %118 = load i32, i32* %13, align 4, !dbg !1246, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %118, metadata !1052, metadata !DIExpression()), !dbg !1120
  %119 = load i32, i32* %11, align 4, !dbg !1248, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %119, metadata !1048, metadata !DIExpression()), !dbg !1120
  %120 = load i32, i32* %14, align 4, !dbg !1249, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %120, metadata !1063, metadata !DIExpression()), !dbg !1120
  %121 = load i32, i32* %15, align 4, !dbg !1250, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %121, metadata !1065, metadata !DIExpression()), !dbg !1120
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %115, i32 %64, i32 %70, i32 %118, i32 %119, i32 %120, i32 %121, i32 %3), !dbg !1251
  br label %1119, !dbg !1252

122:                                              ; preds = %114, %112, %108, %85
  call void @llvm.dbg.value(metadata i32 -32000, metadata !1050, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 -32000, metadata !1044, metadata !DIExpression()), !dbg !1120
  %123 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8, !dbg !1253, !intel-tbaa !1254
  %124 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 9, !dbg !1255, !intel-tbaa !1256
  %125 = load i32, i32* %124, align 4, !dbg !1255, !tbaa !1257
  %126 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 7, !dbg !1258, !intel-tbaa !1256
  %127 = load i32, i32* %126, align 4, !dbg !1258, !tbaa !1257
  %128 = add nsw i32 %127, %125, !dbg !1259
  %129 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 11, !dbg !1260, !intel-tbaa !1256
  %130 = load i32, i32* %129, align 4, !dbg !1260, !tbaa !1257
  %131 = add nsw i32 %128, %130, !dbg !1261
  %132 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 3, !dbg !1262, !intel-tbaa !1256
  %133 = load i32, i32* %132, align 4, !dbg !1262, !tbaa !1257
  %134 = add nsw i32 %131, %133, !dbg !1263
  call void @llvm.dbg.value(metadata i32 %134, metadata !1066, metadata !DIExpression()), !dbg !1120
  %135 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 10, !dbg !1264, !intel-tbaa !1256
  %136 = load i32, i32* %135, align 8, !dbg !1264, !tbaa !1257
  %137 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 8, !dbg !1265, !intel-tbaa !1256
  %138 = load i32, i32* %137, align 8, !dbg !1265, !tbaa !1257
  %139 = add nsw i32 %138, %136, !dbg !1266
  %140 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 12, !dbg !1267, !intel-tbaa !1256
  %141 = load i32, i32* %140, align 8, !dbg !1267, !tbaa !1257
  %142 = add nsw i32 %139, %141, !dbg !1268
  %143 = getelementptr inbounds [13 x i32], [13 x i32]* %123, i64 0, i64 4, !dbg !1269, !intel-tbaa !1256
  %144 = load i32, i32* %143, align 8, !dbg !1269, !tbaa !1257
  %145 = add nsw i32 %142, %144, !dbg !1270
  call void @llvm.dbg.value(metadata i32 %145, metadata !1067, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1048, metadata !DIExpression()), !dbg !1120
  store i32 0, i32* %11, align 4, !dbg !1271, !tbaa !866
  %146 = icmp eq i32 %4, 0, !dbg !1272
  br i1 %146, label %147, label %231, !dbg !1273

147:                                              ; preds = %122
  %148 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !dbg !1274, !intel-tbaa !885
  %149 = load i32, i32* %148, align 4, !dbg !1274, !tbaa !885
  %150 = icmp eq i32 %149, 0, !dbg !1275
  %151 = select i1 %150, i32 %145, i32 %134, !dbg !1275
  %152 = icmp eq i32 %151, 0, !dbg !1276
  %153 = or i1 %93, %152, !dbg !1277
  %154 = xor i1 %153, true, !dbg !1277
  %155 = load i32, i32* %12, align 4, !dbg !1278
  call void @llvm.dbg.value(metadata i32 %155, metadata !1049, metadata !DIExpression()), !dbg !1120
  %156 = icmp ne i32 %155, 0, !dbg !1278
  %157 = and i1 %156, %154, !dbg !1277
  %158 = icmp sgt i32 %3, 4, !dbg !1279
  %159 = and i1 %158, %96, !dbg !1280
  %160 = and i1 %159, %157, !dbg !1277
  br i1 %160, label %161, label %231, !dbg !1277

161:                                              ; preds = %147
  %162 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !dbg !1281, !tbaa !1282
  %163 = icmp eq i32 %162, 2, !dbg !1283
  br i1 %163, label %164, label %183, !dbg !1284

164:                                              ; preds = %161
  call void @llvm.dbg.value(metadata i32 %3, metadata !1071, metadata !DIExpression(DW_OP_constu, 24, DW_OP_minus, DW_OP_stack_value)), !dbg !1285
  %165 = icmp slt i32 %3, 25, !dbg !1286
  %166 = add nsw i32 %70, -1, !dbg !1288
  br i1 %165, label %167, label %169, !dbg !1289

167:                                              ; preds = %164
  %168 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %166, i32 %70, i32 0, i32 0), !dbg !1290
  call void @llvm.dbg.value(metadata i32 %168, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %172, !dbg !1292

169:                                              ; preds = %164
  %170 = add nsw i32 %3, -24, !dbg !1293
  call void @llvm.dbg.value(metadata i32 %170, metadata !1071, metadata !DIExpression()), !dbg !1285
  %171 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %166, i32 %70, i32 %170, i32 1, i32 %86), !dbg !1294
  call void @llvm.dbg.value(metadata i32 %171, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %172

172:                                              ; preds = %169, %167
  %173 = phi i32 [ %168, %167 ], [ %171, %169 ]
  call void @llvm.dbg.value(metadata i32 %173, metadata !1044, metadata !DIExpression()), !dbg !1120
  %174 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !1296, !tbaa !1298
  %175 = icmp eq i32 %174, 0, !dbg !1299
  br i1 %175, label %176, label %1119, !dbg !1300

176:                                              ; preds = %172
  %177 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !dbg !1301, !tbaa !1282
  call void @llvm.dbg.value(metadata i32 %173, metadata !1044, metadata !DIExpression()), !dbg !1120
  %178 = icmp eq i32 %177, 2, !dbg !1302
  %179 = icmp slt i32 %173, %70, !dbg !1303
  %180 = and i1 %179, %178, !dbg !1304
  br i1 %180, label %248, label %181, !dbg !1304

181:                                              ; preds = %176
  %182 = load i32, i32* %148, align 4, !dbg !1305, !tbaa !885
  br label %183, !dbg !1304

183:                                              ; preds = %181, %161
  %184 = phi i32 [ %182, %181 ], [ %149, %161 ], !dbg !1305
  %185 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 10, !dbg !1306, !intel-tbaa !1307
  %186 = load i32, i32* %185, align 8, !dbg !1306, !tbaa !1307
  call void @llvm.dbg.value(metadata i32 %186, metadata !1051, metadata !DIExpression()), !dbg !1120
  store i32 0, i32* %185, align 8, !dbg !1308, !tbaa !1307
  %187 = xor i32 %184, 1, !dbg !1305
  store i32 %187, i32* %148, align 4, !dbg !1305, !tbaa !885
  %188 = load i32, i32* %31, align 8, !dbg !1309, !tbaa !968
  %189 = add nsw i32 %188, 1, !dbg !1309
  store i32 %189, i32* %31, align 8, !dbg !1309, !tbaa !968
  %190 = load i32, i32* %46, align 4, !dbg !1310, !tbaa !1151
  %191 = add nsw i32 %190, 1, !dbg !1310
  store i32 %191, i32* %46, align 4, !dbg !1310, !tbaa !1151
  %192 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19, !dbg !1311, !intel-tbaa !1312
  %193 = sext i32 %188 to i64, !dbg !1313
  %194 = getelementptr inbounds [64 x i32], [64 x i32]* %192, i64 0, i64 %193, !dbg !1313, !intel-tbaa !1208
  store i32 0, i32* %194, align 4, !dbg !1314, !tbaa !1315
  %195 = sext i32 %189 to i64, !dbg !1316
  %196 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %195, !dbg !1316, !intel-tbaa !1208
  store i32 0, i32* %196, align 4, !dbg !1317, !tbaa !1209
  %197 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 20, !dbg !1318, !intel-tbaa !1319
  %198 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %193, !dbg !1320, !intel-tbaa !1208
  %199 = load i32, i32* %198, align 4, !dbg !1320, !tbaa !1321
  %200 = getelementptr inbounds [64 x i32], [64 x i32]* %197, i64 0, i64 %195, !dbg !1322, !intel-tbaa !1208
  store i32 %199, i32* %200, align 4, !dbg !1323, !tbaa !1321
  call void @llvm.dbg.value(metadata i32 16, metadata !1060, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %3, metadata !1076, metadata !DIExpression(DW_OP_constu, 16, DW_OP_minus, DW_OP_stack_value)), !dbg !1324
  %201 = icmp slt i32 %3, 17, !dbg !1325
  %202 = sub nsw i32 0, %70, !dbg !1327
  %203 = sub i32 1, %70, !dbg !1327
  br i1 %201, label %204, label %206, !dbg !1328

204:                                              ; preds = %183
  %205 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %202, i32 %203, i32 0, i32 0), !dbg !1329
  call void @llvm.dbg.value(metadata i32 undef, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %211, !dbg !1331

206:                                              ; preds = %183
  %207 = add nsw i32 %3, -16, !dbg !1332
  call void @llvm.dbg.value(metadata i32 %207, metadata !1076, metadata !DIExpression()), !dbg !1324
  %208 = icmp eq i32 %86, 0, !dbg !1333
  %209 = zext i1 %208 to i32, !dbg !1335
  %210 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %202, i32 %203, i32 %207, i32 1, i32 %209), !dbg !1336
  call void @llvm.dbg.value(metadata i32 undef, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %211

211:                                              ; preds = %206, %204
  %212 = phi i32 [ %205, %204 ], [ %210, %206 ]
  %213 = sub nsw i32 0, %212, !dbg !1327
  call void @llvm.dbg.value(metadata i32 %213, metadata !1044, metadata !DIExpression()), !dbg !1120
  %214 = load i32, i32* %46, align 4, !dbg !1337, !tbaa !1151
  %215 = add nsw i32 %214, -1, !dbg !1337
  store i32 %215, i32* %46, align 4, !dbg !1337, !tbaa !1151
  %216 = load i32, i32* %31, align 8, !dbg !1338, !tbaa !968
  %217 = add nsw i32 %216, -1, !dbg !1338
  store i32 %217, i32* %31, align 8, !dbg !1338, !tbaa !968
  %218 = load i32, i32* %148, align 4, !dbg !1339, !tbaa !885
  %219 = xor i32 %218, 1, !dbg !1339
  store i32 %219, i32* %148, align 4, !dbg !1339, !tbaa !885
  store i32 %186, i32* %185, align 8, !dbg !1340, !tbaa !1307
  %220 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !1341, !tbaa !1298
  %221 = icmp eq i32 %220, 0, !dbg !1343
  br i1 %221, label %222, label %1119, !dbg !1344

222:                                              ; preds = %211
  %223 = icmp sgt i32 %70, %213, !dbg !1345
  br i1 %223, label %228, label %224, !dbg !1347

224:                                              ; preds = %222
  %225 = load i32, i32* %13, align 4, !dbg !1348, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %225, metadata !1052, metadata !DIExpression()), !dbg !1120
  %226 = load i32, i32* %11, align 4, !dbg !1350, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %226, metadata !1048, metadata !DIExpression()), !dbg !1120
  %227 = load i32, i32* %15, align 4, !dbg !1351, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %227, metadata !1065, metadata !DIExpression()), !dbg !1120
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %213, i32 %64, i32 %70, i32 %225, i32 %226, i32 0, i32 %227, i32 %3), !dbg !1352
  br label %1119, !dbg !1353

228:                                              ; preds = %222
  %229 = icmp sgt i32 %212, 31400, !dbg !1354
  br i1 %229, label %230, label %248, !dbg !1356

230:                                              ; preds = %228
  call void @llvm.dbg.value(metadata i32 1, metadata !1048, metadata !DIExpression()), !dbg !1120
  store i32 1, i32* %11, align 4, !dbg !1357, !tbaa !866
  br label %248, !dbg !1359

231:                                              ; preds = %147, %122
  %232 = icmp slt i32 %3, 13, !dbg !1360
  %233 = and i1 %232, %96, !dbg !1361
  %234 = add nsw i32 %70, -300, !dbg !1362
  %235 = icmp slt i32 %92, %234, !dbg !1363
  %236 = and i1 %233, %235, !dbg !1361
  br i1 %236, label %237, label %248, !dbg !1361

237:                                              ; preds = %231
  %238 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %64, i32 %70, i32 0, i32 0), !dbg !1364
  call void @llvm.dbg.value(metadata i32 %238, metadata !1079, metadata !DIExpression()), !dbg !1365
  %239 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !1366, !tbaa !1298
  %240 = icmp eq i32 %239, 0, !dbg !1368
  br i1 %240, label %241, label %1119, !dbg !1369

241:                                              ; preds = %237
  %242 = icmp sgt i32 %238, %64, !dbg !1370
  br i1 %242, label %248, label %243, !dbg !1372

243:                                              ; preds = %241
  %244 = load i32, i32* %13, align 4, !dbg !1373, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %244, metadata !1052, metadata !DIExpression()), !dbg !1120
  %245 = load i32, i32* %11, align 4, !dbg !1375, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %245, metadata !1048, metadata !DIExpression()), !dbg !1120
  %246 = load i32, i32* %14, align 4, !dbg !1376, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %246, metadata !1063, metadata !DIExpression()), !dbg !1120
  %247 = load i32, i32* %15, align 4, !dbg !1377, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %247, metadata !1065, metadata !DIExpression()), !dbg !1120
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %64, i32 %64, i32 %70, i32 %244, i32 %245, i32 %246, i32 %247, i32 %3), !dbg !1378
  br label %1119, !dbg !1379

248:                                              ; preds = %241, %231, %230, %228, %176
  call void @llvm.dbg.value(metadata i32 0, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1041, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 1, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1059, metadata !DIExpression()), !dbg !1120
  %249 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 0, !dbg !1380
  br i1 %93, label %250, label %255, !dbg !1382

250:                                              ; preds = %248
  %251 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %249, i32 %91), !dbg !1383
  call void @llvm.dbg.value(metadata i32 %256, metadata !1041, metadata !DIExpression()), !dbg !1120
  %252 = icmp eq i32 %251, 0, !dbg !1385
  br i1 %252, label %280, label %253, !dbg !1389

253:                                              ; preds = %250
  store i32 0, i32* %9, align 4, !dbg !1390, !tbaa !866
  call void @llvm.dbg.value(metadata i32 0, metadata !1059, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1042, metadata !DIExpression()), !dbg !1120
  %254 = icmp sgt i32 %251, 0, !dbg !1393
  br i1 %254, label %257, label %280, !dbg !1395

255:                                              ; preds = %248
  %256 = call i32 @_Z3genP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %249), !dbg !1396
  call void @llvm.dbg.value(metadata i32 %256, metadata !1041, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %256, metadata !1059, metadata !DIExpression()), !dbg !1120
  br label %280

257:                                              ; preds = %257, %253
  %258 = phi i32 [ %270, %257 ], [ 0, %253 ]
  %259 = phi i32 [ %276, %257 ], [ 0, %253 ]
  call void @llvm.dbg.value(metadata i32 %258, metadata !1059, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %259, metadata !1042, metadata !DIExpression()), !dbg !1120
  %260 = sext i32 %259 to i64, !dbg !1398
  %261 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %260, !dbg !1398, !intel-tbaa !1400
  %262 = load i32, i32* %261, align 4, !dbg !1398, !tbaa !1400
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %262), !dbg !1402
  %263 = load i32, i32* %9, align 4, !dbg !1403, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %263, metadata !1042, metadata !DIExpression()), !dbg !1120
  %264 = sext i32 %263 to i64, !dbg !1405
  %265 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %264, !dbg !1405, !intel-tbaa !1400
  %266 = load i32, i32* %265, align 4, !dbg !1405, !tbaa !1400
  %267 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %266), !dbg !1406
  %268 = icmp ne i32 %267, 0, !dbg !1406
  %269 = zext i1 %268 to i32, !dbg !1407
  %270 = add nuw nsw i32 %258, %269, !dbg !1407
  call void @llvm.dbg.value(metadata i32 %270, metadata !1059, metadata !DIExpression()), !dbg !1120
  %271 = load i32, i32* %9, align 4, !dbg !1408, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %271, metadata !1042, metadata !DIExpression()), !dbg !1120
  %272 = sext i32 %271 to i64, !dbg !1409
  %273 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %272, !dbg !1409, !intel-tbaa !1400
  %274 = load i32, i32* %273, align 4, !dbg !1409, !tbaa !1400
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %274), !dbg !1410
  %275 = load i32, i32* %9, align 4, !dbg !1411, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %275, metadata !1042, metadata !DIExpression()), !dbg !1120
  %276 = add nsw i32 %275, 1, !dbg !1411
  call void @llvm.dbg.value(metadata i32 %276, metadata !1042, metadata !DIExpression()), !dbg !1120
  store i32 %276, i32* %9, align 4, !dbg !1390, !tbaa !866
  %277 = icmp slt i32 %276, %251, !dbg !1393
  %278 = icmp ult i32 %270, 2, !dbg !1412
  %279 = and i1 %277, %278, !dbg !1412
  br i1 %279, label %257, label %280, !dbg !1395, !llvm.loop !1413

280:                                              ; preds = %257, %255, %253, %250
  %281 = phi i32 [ 0, %250 ], [ %256, %255 ], [ %251, %253 ], [ %251, %257 ]
  %282 = phi i32 [ 0, %250 ], [ %256, %255 ], [ 0, %253 ], [ %270, %257 ]
  call void @llvm.dbg.value(metadata i32 %282, metadata !1059, metadata !DIExpression()), !dbg !1120
  %283 = getelementptr inbounds [240 x i32], [240 x i32]* %8, i64 0, i64 0, !dbg !1415
  %284 = load i32, i32* %13, align 4, !dbg !1416, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %284, metadata !1052, metadata !DIExpression()), !dbg !1120
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %249, i32* nonnull %283, i32 %281, i32 %284), !dbg !1417
  %285 = icmp sgt i32 %3, 19, !dbg !1418
  br i1 %285, label %286, label %336, !dbg !1419

286:                                              ; preds = %280
  %287 = icmp ne i32 %70, %95, !dbg !1420
  %288 = load i32, i32* %13, align 4, !dbg !1421
  call void @llvm.dbg.value(metadata i32 %288, metadata !1052, metadata !DIExpression()), !dbg !1120
  %289 = icmp eq i32 %288, 65535, !dbg !1422
  %290 = and i1 %287, %289, !dbg !1423
  br i1 %290, label %291, label %336, !dbg !1423

291:                                              ; preds = %286
  store i32 0, i32* %9, align 4, !dbg !1424, !tbaa !866
  call void @llvm.dbg.value(metadata i32 0, metadata !1082, metadata !DIExpression()), !dbg !1426
  call void @llvm.dbg.value(metadata i32 0, metadata !1042, metadata !DIExpression()), !dbg !1120
  %292 = icmp sgt i32 %281, 0, !dbg !1427
  br i1 %292, label %293, label %324, !dbg !1429

293:                                              ; preds = %291
  %294 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1, !dbg !1430
  %295 = zext i32 %281 to i64, !dbg !1427
  br label %296, !dbg !1429

296:                                              ; preds = %318, %293
  %297 = phi i64 [ 0, %293 ], [ %320, %318 ]
  %298 = phi i32 [ 0, %293 ], [ %319, %318 ]
  call void @llvm.dbg.value(metadata i32 %298, metadata !1082, metadata !DIExpression()), !dbg !1426
  call void @llvm.dbg.value(metadata i64 %297, metadata !1042, metadata !DIExpression()), !dbg !1120
  %299 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %297, !dbg !1433, !intel-tbaa !1400
  %300 = load i32, i32* %299, align 4, !dbg !1433, !tbaa !1400
  %301 = lshr i32 %300, 19, !dbg !1433
  %302 = and i32 %301, 15, !dbg !1433
  %303 = icmp eq i32 %302, 13, !dbg !1434
  br i1 %303, label %318, label %304, !dbg !1435

304:                                              ; preds = %296
  %305 = lshr i32 %300, 6, !dbg !1436
  %306 = and i32 %305, 63, !dbg !1436
  %307 = zext i32 %306 to i64, !dbg !1437
  %308 = getelementptr inbounds [64 x i32], [64 x i32]* %294, i64 0, i64 %307, !dbg !1437, !intel-tbaa !1208
  %309 = load i32, i32* %308, align 4, !dbg !1437, !tbaa !1438
  %310 = sext i32 %309 to i64, !dbg !1439
  %311 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %310, !dbg !1439, !intel-tbaa !1440
  %312 = load i32, i32* %311, align 4, !dbg !1439, !tbaa !1440
  %313 = icmp slt i32 %312, 0, !dbg !1442
  %314 = sub nsw i32 0, %312, !dbg !1442
  %315 = select i1 %313, i32 %314, i32 %312, !dbg !1442
  %316 = icmp sgt i32 %302, %315, !dbg !1443
  %317 = select i1 %316, i32 1, i32 %298, !dbg !1444
  br label %318, !dbg !1444

318:                                              ; preds = %304, %296
  %319 = phi i32 [ %298, %296 ], [ %317, %304 ]
  call void @llvm.dbg.value(metadata i32 %319, metadata !1082, metadata !DIExpression()), !dbg !1426
  call void @llvm.dbg.value(metadata i64 %297, metadata !1042, metadata !DIExpression()), !dbg !1120
  %320 = add nuw nsw i64 %297, 1, !dbg !1445
  call void @llvm.dbg.value(metadata i64 %320, metadata !1042, metadata !DIExpression()), !dbg !1120
  %321 = icmp eq i64 %320, %295, !dbg !1427
  br i1 %321, label %322, label %296, !dbg !1429, !llvm.loop !1446

322:                                              ; preds = %318
  call void @llvm.dbg.value(metadata i32 %319, metadata !1082, metadata !DIExpression()), !dbg !1426
  call void @llvm.dbg.value(metadata i32 %319, metadata !1082, metadata !DIExpression()), !dbg !1426
  call void @llvm.dbg.value(metadata i32 %319, metadata !1082, metadata !DIExpression()), !dbg !1426
  call void @llvm.dbg.value(metadata i32 %319, metadata !1082, metadata !DIExpression()), !dbg !1426
  store i32 %281, i32* %9, align 4, !dbg !1424, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %319, metadata !1082, metadata !DIExpression()), !dbg !1426
  %323 = icmp eq i32 %319, 0, !dbg !1448
  br i1 %323, label %324, label %336, !dbg !1449

324:                                              ; preds = %322, %291
  %325 = bitcast i32* %17 to i8*, !dbg !1450
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %325) #7, !dbg !1450
  %326 = bitcast i32* %18 to i8*, !dbg !1451
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %326) #7, !dbg !1451
  %327 = ashr i32 %3, 1, !dbg !1452
  %328 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %327, i32 0, i32 %86), !dbg !1453
  call void @llvm.dbg.value(metadata i32 %328, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32* %17, metadata !1085, metadata !DIExpression(DW_OP_deref)), !dbg !1454
  call void @llvm.dbg.value(metadata i32* %17, metadata !1085, metadata !DIExpression(DW_OP_deref)), !dbg !1454
  call void @llvm.dbg.value(metadata i32* %17, metadata !1085, metadata !DIExpression(DW_OP_deref)), !dbg !1454
  call void @llvm.dbg.value(metadata i32* %17, metadata !1085, metadata !DIExpression(DW_OP_deref)), !dbg !1454
  call void @llvm.dbg.value(metadata i32* %17, metadata !1085, metadata !DIExpression(DW_OP_deref)), !dbg !1454
  call void @llvm.dbg.value(metadata i32* %18, metadata !1088, metadata !DIExpression(DW_OP_deref)), !dbg !1454
  %329 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* %0, i32* nonnull %17, i32 0, i32 0, i32* nonnull %18, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32* nonnull %17, i32 0), !dbg !1455
  %330 = icmp eq i32 %329, 4, !dbg !1457
  br i1 %330, label %333, label %331, !dbg !1458

331:                                              ; preds = %324
  %332 = load i32, i32* %18, align 4, !dbg !1459, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %332, metadata !1088, metadata !DIExpression()), !dbg !1454
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %249, i32* nonnull %283, i32 %281, i32 %332), !dbg !1461
  br label %335, !dbg !1462

333:                                              ; preds = %324
  %334 = load i32, i32* %13, align 4, !dbg !1463, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %334, metadata !1052, metadata !DIExpression()), !dbg !1120
  call fastcc void @_ZL11order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %249, i32* nonnull %283, i32 %281, i32 %334), !dbg !1465
  br label %335

335:                                              ; preds = %333, %331
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %326) #7, !dbg !1466
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %325) #7, !dbg !1466
  br label %336, !dbg !1467

336:                                              ; preds = %335, %322, %286, %280
  call void @llvm.dbg.value(metadata i32 -32000, metadata !1044, metadata !DIExpression()), !dbg !1120
  %337 = load i32, i32* %11, align 4, !dbg !1468
  call void @llvm.dbg.value(metadata i32 %337, metadata !1048, metadata !DIExpression()), !dbg !1120
  %338 = or i32 %337, %91, !dbg !1469
  %339 = icmp eq i32 %338, 0, !dbg !1469
  %340 = icmp sgt i32 %3, 15, !dbg !1470
  %341 = and i1 %340, %339, !dbg !1469
  %342 = icmp sgt i32 %282, 8, !dbg !1471
  %343 = and i1 %342, %341, !dbg !1469
  br i1 %343, label %344, label %508, !dbg !1469

344:                                              ; preds = %336
  %345 = load i32, i32* %31, align 8, !dbg !1472, !tbaa !968
  %346 = add nsw i32 %345, -1, !dbg !1473
  %347 = sext i32 %346 to i64, !dbg !1474
  %348 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %347, !dbg !1474, !intel-tbaa !1208
  %349 = load i32, i32* %348, align 4, !dbg !1474, !tbaa !1209
  %350 = icmp eq i32 %349, 0, !dbg !1474
  br i1 %350, label %351, label %508, !dbg !1475

351:                                              ; preds = %344
  %352 = icmp slt i32 %345, 3, !dbg !1476
  br i1 %352, label %367, label %353, !dbg !1477

353:                                              ; preds = %351
  %354 = add nsw i32 %345, -2, !dbg !1478
  %355 = sext i32 %354 to i64, !dbg !1479
  %356 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %355, !dbg !1479, !intel-tbaa !1208
  %357 = load i32, i32* %356, align 4, !dbg !1479, !tbaa !1209
  %358 = icmp eq i32 %357, 0, !dbg !1479
  br i1 %358, label %359, label %508, !dbg !1480

359:                                              ; preds = %353
  %360 = icmp slt i32 %345, 4, !dbg !1481
  br i1 %360, label %367, label %361, !dbg !1482

361:                                              ; preds = %359
  %362 = add nsw i32 %345, -3, !dbg !1483
  %363 = sext i32 %362 to i64, !dbg !1484
  %364 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %363, !dbg !1484, !intel-tbaa !1208
  %365 = load i32, i32* %364, align 4, !dbg !1484, !tbaa !1209
  %366 = icmp eq i32 %365, 0, !dbg !1484
  br i1 %366, label %367, label %508, !dbg !1485

367:                                              ; preds = %361, %359, %351
  call void @llvm.dbg.value(metadata i32 0, metadata !1089, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 0, metadata !1092, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 -1, metadata !1042, metadata !DIExpression()), !dbg !1120
  store i32 -1, i32* %9, align 4, !dbg !1487, !tbaa !866
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %368 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !1488
  call void @llvm.dbg.value(metadata i32 %368, metadata !1061, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1092, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 0, metadata !1089, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 -32000, metadata !1044, metadata !DIExpression()), !dbg !1120
  %369 = icmp eq i32 %368, 0, !dbg !1489
  br i1 %369, label %508, label %370, !dbg !1490

370:                                              ; preds = %367
  %371 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16, !dbg !1491
  %372 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36, !dbg !1492
  %373 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19, !dbg !1493
  %374 = sub nsw i32 0, %70, !dbg !1494
  %375 = sub i32 50, %64, !dbg !1495
  %376 = icmp sgt i32 %3, 16, !dbg !1496
  %377 = icmp slt i32 %3, 17, !dbg !1497
  %378 = sub i32 1, %70, !dbg !1499
  %379 = add nsw i32 %3, -16, !dbg !1500
  %380 = icmp eq i32 %86, 0, !dbg !1501
  %381 = zext i1 %380 to i32, !dbg !1503
  br i1 %377, label %382, label %443, !dbg !1490

382:                                              ; preds = %437, %370
  %383 = phi i32 [ %438, %437 ], [ 0, %370 ]
  %384 = phi i32 [ %386, %437 ], [ 0, %370 ]
  %385 = phi i32 [ %423, %437 ], [ -32000, %370 ]
  call void @llvm.dbg.value(metadata i32 %383, metadata !1092, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 %384, metadata !1089, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 %385, metadata !1044, metadata !DIExpression()), !dbg !1120
  %386 = add nuw nsw i32 %384, 1, !dbg !1504
  call void @llvm.dbg.value(metadata i32 %386, metadata !1089, metadata !DIExpression()), !dbg !1486
  %387 = load i32, i32* %9, align 4, !dbg !1505, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %387, metadata !1042, metadata !DIExpression()), !dbg !1120
  %388 = sext i32 %387 to i64, !dbg !1506
  %389 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %388, !dbg !1506, !intel-tbaa !1400
  %390 = load i32, i32* %389, align 4, !dbg !1506, !tbaa !1400
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %390), !dbg !1507
  call void @llvm.dbg.value(metadata i32 0, metadata !1046, metadata !DIExpression()), !dbg !1120
  %391 = load i32, i32* %9, align 4, !dbg !1508, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %391, metadata !1042, metadata !DIExpression()), !dbg !1120
  %392 = sext i32 %391 to i64, !dbg !1509
  %393 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %392, !dbg !1509, !intel-tbaa !1400
  %394 = load i32, i32* %393, align 4, !dbg !1509, !tbaa !1400
  %395 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %394), !dbg !1510
  %396 = icmp eq i32 %395, 0, !dbg !1510
  br i1 %396, label %421, label %397, !dbg !1511

397:                                              ; preds = %382
  %398 = load i64, i64* %371, align 8, !dbg !1491, !tbaa !888
  %399 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !dbg !1512, !tbaa !1513
  %400 = load i32, i32* %31, align 8, !dbg !1514, !tbaa !968
  %401 = add i32 %400, -1, !dbg !1515
  %402 = add i32 %401, %399, !dbg !1516
  %403 = sext i32 %402 to i64, !dbg !1517
  %404 = getelementptr inbounds [1000 x i64], [1000 x i64]* %372, i64 0, i64 %403, !dbg !1517, !intel-tbaa !1518
  store i64 %398, i64* %404, align 8, !dbg !1519, !tbaa !1520
  %405 = load i32, i32* %9, align 4, !dbg !1521, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %405, metadata !1042, metadata !DIExpression()), !dbg !1120
  %406 = sext i32 %405 to i64, !dbg !1522
  %407 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %406, !dbg !1522, !intel-tbaa !1400
  %408 = load i32, i32* %407, align 4, !dbg !1522, !tbaa !1400
  %409 = sext i32 %401 to i64, !dbg !1523
  %410 = getelementptr inbounds [64 x i32], [64 x i32]* %373, i64 0, i64 %409, !dbg !1523, !intel-tbaa !1208
  store i32 %408, i32* %410, align 4, !dbg !1524, !tbaa !1315
  call void @llvm.dbg.value(metadata i32 1, metadata !1046, metadata !DIExpression()), !dbg !1120
  %411 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0), !dbg !1525
  call void @llvm.dbg.value(metadata i32 %411, metadata !1058, metadata !DIExpression()), !dbg !1120
  %412 = load i32, i32* %31, align 8, !dbg !1526, !tbaa !968
  %413 = sext i32 %412 to i64, !dbg !1527
  %414 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %413, !dbg !1527, !intel-tbaa !1208
  store i32 %411, i32* %414, align 4, !dbg !1528, !tbaa !1209
  call void @llvm.dbg.value(metadata i32 %3, metadata !1093, metadata !DIExpression(DW_OP_constu, 16, DW_OP_minus, DW_OP_stack_value)), !dbg !1529
  %415 = icmp ne i32 %411, 0, !dbg !1530
  %416 = or i1 %376, %415, !dbg !1530
  %417 = zext i1 %416 to i32, !dbg !1531
  %418 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %374, i32 %375, i32 %417), !dbg !1532
  %419 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %374, i32 %378, i32 0, i32 0), !dbg !1533
  call void @llvm.dbg.value(metadata i32 undef, metadata !1044, metadata !DIExpression()), !dbg !1120
  %420 = sub nsw i32 0, %419, !dbg !1499
  call void @llvm.dbg.value(metadata i32 %420, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %421, !dbg !1535

421:                                              ; preds = %397, %382
  %422 = phi i32 [ 1, %397 ], [ 0, %382 ]
  %423 = phi i32 [ %420, %397 ], [ %385, %382 ]
  call void @llvm.dbg.value(metadata i32 %423, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %422, metadata !1046, metadata !DIExpression()), !dbg !1120
  %424 = load i32, i32* %9, align 4, !dbg !1536, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %424, metadata !1042, metadata !DIExpression()), !dbg !1120
  %425 = sext i32 %424 to i64, !dbg !1537
  %426 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %425, !dbg !1537, !intel-tbaa !1400
  %427 = load i32, i32* %426, align 4, !dbg !1537, !tbaa !1400
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %427), !dbg !1538
  %428 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !1539, !tbaa !1298
  %429 = icmp eq i32 %428, 0, !dbg !1541
  br i1 %429, label %430, label %508, !dbg !1542

430:                                              ; preds = %421
  %431 = icmp sge i32 %423, %70, !dbg !1543
  %432 = icmp ne i32 %422, 0, !dbg !1546
  %433 = and i1 %432, %431, !dbg !1547
  br i1 %433, label %434, label %437, !dbg !1547

434:                                              ; preds = %430
  %435 = add nsw i32 %383, 1, !dbg !1548
  call void @llvm.dbg.value(metadata i32 %435, metadata !1092, metadata !DIExpression()), !dbg !1486
  %436 = icmp sgt i32 %383, 0, !dbg !1550
  br i1 %436, label %498, label %437, !dbg !1552

437:                                              ; preds = %434, %430
  %438 = phi i32 [ %435, %434 ], [ %383, %430 ]
  call void @llvm.dbg.value(metadata i32 %438, metadata !1092, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %439 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !1553
  call void @llvm.dbg.value(metadata i32 %386, metadata !1089, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 %423, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %439, metadata !1061, metadata !DIExpression()), !dbg !1120
  %440 = icmp ne i32 %439, 0, !dbg !1489
  %441 = icmp ult i32 %384, 2, !dbg !1554
  %442 = and i1 %440, %441, !dbg !1554
  br i1 %442, label %382, label %508, !dbg !1490, !llvm.loop !1555

443:                                              ; preds = %502, %370
  %444 = phi i32 [ %503, %502 ], [ 0, %370 ]
  %445 = phi i32 [ %447, %502 ], [ 0, %370 ]
  %446 = phi i32 [ %484, %502 ], [ -32000, %370 ]
  call void @llvm.dbg.value(metadata i32 %444, metadata !1092, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 %445, metadata !1089, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 %446, metadata !1044, metadata !DIExpression()), !dbg !1120
  %447 = add nuw nsw i32 %445, 1, !dbg !1504
  call void @llvm.dbg.value(metadata i32 %447, metadata !1089, metadata !DIExpression()), !dbg !1486
  %448 = load i32, i32* %9, align 4, !dbg !1505, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %448, metadata !1042, metadata !DIExpression()), !dbg !1120
  %449 = sext i32 %448 to i64, !dbg !1506
  %450 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %449, !dbg !1506, !intel-tbaa !1400
  %451 = load i32, i32* %450, align 4, !dbg !1506, !tbaa !1400
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %451), !dbg !1507
  call void @llvm.dbg.value(metadata i32 0, metadata !1046, metadata !DIExpression()), !dbg !1120
  %452 = load i32, i32* %9, align 4, !dbg !1508, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %452, metadata !1042, metadata !DIExpression()), !dbg !1120
  %453 = sext i32 %452 to i64, !dbg !1509
  %454 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %453, !dbg !1509, !intel-tbaa !1400
  %455 = load i32, i32* %454, align 4, !dbg !1509, !tbaa !1400
  %456 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %455), !dbg !1510
  %457 = icmp eq i32 %456, 0, !dbg !1510
  br i1 %457, label %482, label %458, !dbg !1511

458:                                              ; preds = %443
  %459 = load i64, i64* %371, align 8, !dbg !1491, !tbaa !888
  %460 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !dbg !1512, !tbaa !1513
  %461 = load i32, i32* %31, align 8, !dbg !1514, !tbaa !968
  %462 = add i32 %461, -1, !dbg !1515
  %463 = add i32 %462, %460, !dbg !1516
  %464 = sext i32 %463 to i64, !dbg !1517
  %465 = getelementptr inbounds [1000 x i64], [1000 x i64]* %372, i64 0, i64 %464, !dbg !1517, !intel-tbaa !1518
  store i64 %459, i64* %465, align 8, !dbg !1519, !tbaa !1520
  %466 = load i32, i32* %9, align 4, !dbg !1521, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %466, metadata !1042, metadata !DIExpression()), !dbg !1120
  %467 = sext i32 %466 to i64, !dbg !1522
  %468 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %467, !dbg !1522, !intel-tbaa !1400
  %469 = load i32, i32* %468, align 4, !dbg !1522, !tbaa !1400
  %470 = sext i32 %462 to i64, !dbg !1523
  %471 = getelementptr inbounds [64 x i32], [64 x i32]* %373, i64 0, i64 %470, !dbg !1523, !intel-tbaa !1208
  store i32 %469, i32* %471, align 4, !dbg !1524, !tbaa !1315
  call void @llvm.dbg.value(metadata i32 1, metadata !1046, metadata !DIExpression()), !dbg !1120
  %472 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0), !dbg !1525
  call void @llvm.dbg.value(metadata i32 %472, metadata !1058, metadata !DIExpression()), !dbg !1120
  %473 = load i32, i32* %31, align 8, !dbg !1526, !tbaa !968
  %474 = sext i32 %473 to i64, !dbg !1527
  %475 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %474, !dbg !1527, !intel-tbaa !1208
  store i32 %472, i32* %475, align 4, !dbg !1528, !tbaa !1209
  call void @llvm.dbg.value(metadata i32 %3, metadata !1093, metadata !DIExpression(DW_OP_constu, 16, DW_OP_minus, DW_OP_stack_value)), !dbg !1529
  %476 = icmp ne i32 %472, 0, !dbg !1530
  %477 = or i1 %376, %476, !dbg !1530
  %478 = zext i1 %477 to i32, !dbg !1531
  %479 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %374, i32 %375, i32 %478), !dbg !1532
  call void @llvm.dbg.value(metadata i32 %379, metadata !1093, metadata !DIExpression()), !dbg !1529
  %480 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %374, i32 %378, i32 %379, i32 0, i32 %381), !dbg !1557
  call void @llvm.dbg.value(metadata i32 undef, metadata !1044, metadata !DIExpression()), !dbg !1120
  %481 = sub nsw i32 0, %480, !dbg !1499
  call void @llvm.dbg.value(metadata i32 %481, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %482, !dbg !1535

482:                                              ; preds = %458, %443
  %483 = phi i32 [ 1, %458 ], [ 0, %443 ]
  %484 = phi i32 [ %481, %458 ], [ %446, %443 ]
  call void @llvm.dbg.value(metadata i32 %484, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %483, metadata !1046, metadata !DIExpression()), !dbg !1120
  %485 = load i32, i32* %9, align 4, !dbg !1536, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %485, metadata !1042, metadata !DIExpression()), !dbg !1120
  %486 = sext i32 %485 to i64, !dbg !1537
  %487 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %486, !dbg !1537, !intel-tbaa !1400
  %488 = load i32, i32* %487, align 4, !dbg !1537, !tbaa !1400
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %488), !dbg !1538
  %489 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !1539, !tbaa !1298
  %490 = icmp eq i32 %489, 0, !dbg !1541
  br i1 %490, label %491, label %508, !dbg !1542

491:                                              ; preds = %482
  %492 = icmp sge i32 %484, %70, !dbg !1543
  %493 = icmp ne i32 %483, 0, !dbg !1546
  %494 = and i1 %493, %492, !dbg !1547
  br i1 %494, label %495, label %502, !dbg !1547

495:                                              ; preds = %491
  %496 = add nsw i32 %444, 1, !dbg !1548
  call void @llvm.dbg.value(metadata i32 %496, metadata !1092, metadata !DIExpression()), !dbg !1486
  %497 = icmp sgt i32 %444, 0, !dbg !1550
  br i1 %497, label %498, label %502, !dbg !1552

498:                                              ; preds = %495, %434
  %499 = load i32, i32* %13, align 4, !dbg !1558, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %499, metadata !1052, metadata !DIExpression()), !dbg !1120
  %500 = load i32, i32* %11, align 4, !dbg !1560, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %500, metadata !1048, metadata !DIExpression()), !dbg !1120
  %501 = load i32, i32* %15, align 4, !dbg !1561, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %501, metadata !1065, metadata !DIExpression()), !dbg !1120
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %70, i32 %64, i32 %70, i32 %499, i32 %500, i32 0, i32 %501, i32 %3), !dbg !1562
  call void @llvm.dbg.value(metadata i32 %484, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %1119

502:                                              ; preds = %495, %491
  %503 = phi i32 [ %496, %495 ], [ %444, %491 ]
  call void @llvm.dbg.value(metadata i32 %503, metadata !1092, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %504 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !1553
  call void @llvm.dbg.value(metadata i32 %447, metadata !1089, metadata !DIExpression()), !dbg !1486
  call void @llvm.dbg.value(metadata i32 %484, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %504, metadata !1061, metadata !DIExpression()), !dbg !1120
  %505 = icmp ne i32 %504, 0, !dbg !1489
  %506 = icmp ult i32 %445, 2, !dbg !1554
  %507 = and i1 %505, %506, !dbg !1554
  br i1 %507, label %443, label %508, !dbg !1490, !llvm.loop !1555

508:                                              ; preds = %502, %482, %437, %421, %367, %361, %353, %344, %336
  %509 = phi i32 [ -32000, %344 ], [ -32000, %361 ], [ -32000, %353 ], [ -32000, %336 ], [ -32000, %367 ], [ %423, %421 ], [ %423, %437 ], [ %484, %482 ], [ %484, %502 ]
  call void @llvm.dbg.value(metadata i32 %509, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 1, metadata !1054, metadata !DIExpression()), !dbg !1120
  %510 = load i32, i32* %14, align 4, !dbg !1563, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %510, metadata !1063, metadata !DIExpression()), !dbg !1120
  %511 = load i32, i32* %15, align 4, !dbg !1564
  call void @llvm.dbg.value(metadata i32 %511, metadata !1065, metadata !DIExpression()), !dbg !1120
  %512 = or i32 %511, %510, !dbg !1565
  %513 = load i32, i32* %11, align 4, !dbg !1566
  call void @llvm.dbg.value(metadata i32 %513, metadata !1048, metadata !DIExpression()), !dbg !1120
  %514 = or i32 %512, %513, !dbg !1565
  %515 = icmp eq i32 %514, 0, !dbg !1565
  %516 = and i1 %285, %515, !dbg !1565
  %517 = icmp sgt i32 %282, 1, !dbg !1567
  %518 = and i1 %517, %516, !dbg !1565
  %519 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 1), align 4, !dbg !1568
  %520 = icmp ne i32 %519, 2, !dbg !1569
  %521 = and i1 %520, %518, !dbg !1565
  br i1 %521, label %522, label %608, !dbg !1565

522:                                              ; preds = %508
  %523 = add nsw i32 %3, -24, !dbg !1570
  %524 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* %0, i32 %64, i32 %70, i32 %523, i32 0, i32 %86), !dbg !1571
  call void @llvm.dbg.value(metadata i32 %524, metadata !1098, metadata !DIExpression()), !dbg !1572
  %525 = icmp sgt i32 %524, %64, !dbg !1573
  br i1 %525, label %526, label %608, !dbg !1574

526:                                              ; preds = %522
  call void @llvm.dbg.value(metadata i32 -1, metadata !1042, metadata !DIExpression()), !dbg !1120
  store i32 -1, i32* %9, align 4, !dbg !1575, !tbaa !866
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %527 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !1576
  call void @llvm.dbg.value(metadata i32 %527, metadata !1061, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1097, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %509, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 1, metadata !1054, metadata !DIExpression()), !dbg !1120
  %528 = icmp ne i32 %527, 0, !dbg !1577
  %529 = load i32, i32* %14, align 4, !dbg !1578
  call void @llvm.dbg.value(metadata i32 %529, metadata !1063, metadata !DIExpression()), !dbg !1120
  %530 = icmp slt i32 %529, 2, !dbg !1579
  %531 = and i1 %528, %530, !dbg !1580
  br i1 %531, label %532, label %608, !dbg !1580

532:                                              ; preds = %526
  %533 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16, !dbg !1581
  %534 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36, !dbg !1582
  %535 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19, !dbg !1583
  %536 = add nsw i32 %3, -16, !dbg !1584
  %537 = sub nsw i32 0, %70, !dbg !1585
  %538 = sub i32 50, %64, !dbg !1586
  %539 = sub nsw i32 0, %64, !dbg !1587
  %540 = xor i32 %64, -1, !dbg !1588
  %541 = icmp eq i32 %86, 0, !dbg !1591
  %542 = zext i1 %541 to i32, !dbg !1592
  %543 = sub i32 49, %64, !dbg !1593
  %544 = add nsw i32 %64, -50, !dbg !1595
  br label %545, !dbg !1580

545:                                              ; preds = %593, %532
  %546 = phi i32 [ 0, %532 ], [ %596, %593 ]
  %547 = phi i32 [ %509, %532 ], [ %595, %593 ]
  %548 = phi i32 [ 1, %532 ], [ %594, %593 ]
  call void @llvm.dbg.value(metadata i32 %546, metadata !1097, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %547, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %548, metadata !1054, metadata !DIExpression()), !dbg !1120
  %549 = load i32, i32* %9, align 4, !dbg !1596, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %549, metadata !1042, metadata !DIExpression()), !dbg !1120
  %550 = sext i32 %549 to i64, !dbg !1597
  %551 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %550, !dbg !1597, !intel-tbaa !1400
  %552 = load i32, i32* %551, align 4, !dbg !1597, !tbaa !1400
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %552), !dbg !1598
  call void @llvm.dbg.value(metadata i32 0, metadata !1046, metadata !DIExpression()), !dbg !1120
  %553 = load i32, i32* %9, align 4, !dbg !1599, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %553, metadata !1042, metadata !DIExpression()), !dbg !1120
  %554 = sext i32 %553 to i64, !dbg !1600
  %555 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %554, !dbg !1600, !intel-tbaa !1400
  %556 = load i32, i32* %555, align 4, !dbg !1600, !tbaa !1400
  %557 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %556), !dbg !1601
  %558 = icmp eq i32 %557, 0, !dbg !1601
  br i1 %558, label %593, label %559, !dbg !1602

559:                                              ; preds = %545
  %560 = load i64, i64* %533, align 8, !dbg !1581, !tbaa !888
  %561 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !dbg !1603, !tbaa !1513
  %562 = load i32, i32* %31, align 8, !dbg !1604, !tbaa !968
  %563 = add i32 %562, -1, !dbg !1605
  %564 = add i32 %563, %561, !dbg !1606
  %565 = sext i32 %564 to i64, !dbg !1607
  %566 = getelementptr inbounds [1000 x i64], [1000 x i64]* %534, i64 0, i64 %565, !dbg !1607, !intel-tbaa !1518
  store i64 %560, i64* %566, align 8, !dbg !1608, !tbaa !1520
  %567 = load i32, i32* %9, align 4, !dbg !1609, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %567, metadata !1042, metadata !DIExpression()), !dbg !1120
  %568 = sext i32 %567 to i64, !dbg !1610
  %569 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %568, !dbg !1610, !intel-tbaa !1400
  %570 = load i32, i32* %569, align 4, !dbg !1610, !tbaa !1400
  %571 = sext i32 %563 to i64, !dbg !1611
  %572 = getelementptr inbounds [64 x i32], [64 x i32]* %535, i64 0, i64 %571, !dbg !1611, !intel-tbaa !1208
  store i32 %570, i32* %572, align 4, !dbg !1612, !tbaa !1315
  %573 = add nsw i32 %546, 1, !dbg !1613
  call void @llvm.dbg.value(metadata i32 %573, metadata !1097, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 1, metadata !1046, metadata !DIExpression()), !dbg !1120
  %574 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0), !dbg !1614
  call void @llvm.dbg.value(metadata i32 %574, metadata !1058, metadata !DIExpression()), !dbg !1120
  %575 = load i32, i32* %31, align 8, !dbg !1615, !tbaa !968
  %576 = sext i32 %575 to i64, !dbg !1616
  %577 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %576, !dbg !1616, !intel-tbaa !1208
  store i32 %574, i32* %577, align 4, !dbg !1617, !tbaa !1209
  call void @llvm.dbg.value(metadata i32 %536, metadata !1101, metadata !DIExpression()), !dbg !1618
  %578 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %537, i32 %538, i32 1), !dbg !1619
  %579 = icmp eq i32 %548, 0, !dbg !1620
  br i1 %579, label %587, label %580, !dbg !1621

580:                                              ; preds = %559
  %581 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %540, i32 %539, i32 %536, i32 0, i32 %542), !dbg !1622
  %582 = sub nsw i32 0, %581, !dbg !1623
  call void @llvm.dbg.value(metadata i32 %582, metadata !1044, metadata !DIExpression()), !dbg !1120
  %583 = icmp slt i32 %64, %582, !dbg !1624
  br i1 %583, label %584, label %585, !dbg !1626

584:                                              ; preds = %580
  call void @llvm.dbg.value(metadata i32 1, metadata !1063, metadata !DIExpression()), !dbg !1120
  store i32 1, i32* %14, align 4, !dbg !1627, !tbaa !866
  br label %593, !dbg !1629

585:                                              ; preds = %580
  call void @llvm.dbg.value(metadata i32 0, metadata !1063, metadata !DIExpression()), !dbg !1120
  store i32 0, i32* %14, align 4, !dbg !1630, !tbaa !866
  %586 = add nsw i32 %546, 11, !dbg !1632
  call void @llvm.dbg.value(metadata i32 %586, metadata !1097, metadata !DIExpression()), !dbg !1120
  br label %593

587:                                              ; preds = %559
  %588 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %543, i32 %538, i32 %536, i32 0, i32 0), !dbg !1633
  %589 = sub nsw i32 0, %588, !dbg !1634
  call void @llvm.dbg.value(metadata i32 %589, metadata !1044, metadata !DIExpression()), !dbg !1120
  %590 = icmp slt i32 %544, %589, !dbg !1635
  br i1 %590, label %591, label %593, !dbg !1637

591:                                              ; preds = %587
  call void @llvm.dbg.value(metadata i32 0, metadata !1063, metadata !DIExpression()), !dbg !1120
  store i32 0, i32* %14, align 4, !dbg !1638, !tbaa !866
  %592 = add nsw i32 %546, 11, !dbg !1640
  call void @llvm.dbg.value(metadata i32 %592, metadata !1097, metadata !DIExpression()), !dbg !1120
  br label %593, !dbg !1641

593:                                              ; preds = %591, %587, %585, %584, %545
  %594 = phi i32 [ %548, %545 ], [ 0, %587 ], [ 0, %591 ], [ 0, %584 ], [ 0, %585 ]
  %595 = phi i32 [ %547, %545 ], [ %589, %587 ], [ %589, %591 ], [ %582, %584 ], [ %582, %585 ]
  %596 = phi i32 [ %546, %545 ], [ %573, %587 ], [ %592, %591 ], [ %573, %584 ], [ %586, %585 ]
  call void @llvm.dbg.value(metadata i32 %596, metadata !1097, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %595, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %594, metadata !1054, metadata !DIExpression()), !dbg !1120
  %597 = load i32, i32* %9, align 4, !dbg !1642, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %597, metadata !1042, metadata !DIExpression()), !dbg !1120
  %598 = sext i32 %597 to i64, !dbg !1643
  %599 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %598, !dbg !1643, !intel-tbaa !1400
  %600 = load i32, i32* %599, align 4, !dbg !1643, !tbaa !1400
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %600), !dbg !1644
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %601 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !1645
  call void @llvm.dbg.value(metadata i32 %601, metadata !1061, metadata !DIExpression()), !dbg !1120
  %602 = icmp ne i32 %601, 0, !dbg !1577
  %603 = load i32, i32* %14, align 4, !dbg !1578
  call void @llvm.dbg.value(metadata i32 %603, metadata !1063, metadata !DIExpression()), !dbg !1120
  %604 = icmp slt i32 %603, 2, !dbg !1579
  %605 = and i1 %602, %604, !dbg !1580
  %606 = icmp slt i32 %596, 3, !dbg !1646
  %607 = and i1 %606, %605, !dbg !1580
  br i1 %607, label %545, label %608, !dbg !1580, !llvm.loop !1647

608:                                              ; preds = %593, %526, %522, %508
  %609 = phi i32 [ %509, %508 ], [ %509, %522 ], [ %509, %526 ], [ %595, %593 ]
  call void @llvm.dbg.value(metadata i32 %609, metadata !1044, metadata !DIExpression()), !dbg !1120
  br i1 %96, label %615, label %610, !dbg !1650

610:                                              ; preds = %608
  %611 = load i32, i32* %31, align 8, !dbg !1651, !tbaa !968
  %612 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4, !dbg !1652, !tbaa !1653
  %613 = shl nsw i32 %612, 1, !dbg !1654
  %614 = icmp sle i32 %611, %613, !dbg !1655
  br label %615

615:                                              ; preds = %610, %608
  %616 = phi i1 [ false, %608 ], [ %614, %610 ], !dbg !1120
  call void @llvm.dbg.value(metadata i1 %616, metadata !1068, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 1, metadata !1054, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 1, metadata !1062, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 -1, metadata !1042, metadata !DIExpression()), !dbg !1120
  store i32 -1, i32* %9, align 4, !dbg !1656, !tbaa !866
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %617 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !1657
  call void @llvm.dbg.value(metadata i32 %617, metadata !1061, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %64, metadata !1034, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %609, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 1, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 -32000, metadata !1050, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 1, metadata !1054, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 1, metadata !1062, metadata !DIExpression()), !dbg !1120
  %618 = icmp eq i32 %617, 0, !dbg !1658
  br i1 %618, label %1087, label %619, !dbg !1659

619:                                              ; preds = %615
  %620 = icmp eq i32 %282, 1, !dbg !1660
  %621 = and i1 %93, %620, !dbg !1662
  %622 = select i1 %621, i32 4, i32 0, !dbg !1662
  %623 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1, !dbg !1663
  %624 = or i32 %622, 2, !dbg !1665
  %625 = select i1 %616, i32 %624, i32 %622, !dbg !1665
  %626 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19, !dbg !1667
  %627 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !dbg !1668
  %628 = select i1 %616, i32 3, i32 1, !dbg !1669
  %629 = add nsw i32 %145, %134, !dbg !1670
  %630 = icmp eq i32 %629, 1, !dbg !1674
  %631 = sdiv i32 %3, 4, !dbg !1675
  %632 = add nsw i32 %631, 1, !dbg !1677
  %633 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 0, !dbg !1678
  %634 = icmp sgt i32 %3, 24, !dbg !1692
  %635 = icmp slt i32 %3, 9, !dbg !1693
  %636 = icmp slt i32 %3, 13, !dbg !1695
  %637 = add nsw i32 %92, 100, !dbg !1697
  %638 = add nsw i32 %92, 300, !dbg !1697
  %639 = add nsw i32 %92, 75, !dbg !1698
  %640 = add nsw i32 %92, 200, !dbg !1701
  %641 = select i1 %616, i32 4, i32 2, !dbg !1703
  %642 = sub nsw i32 0, %70, !dbg !1704
  %643 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16, !dbg !1705
  %644 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36, !dbg !1706
  %645 = icmp sgt i32 %3, 4, !dbg !1707
  %646 = add i32 %3, -4, !dbg !1709
  %647 = icmp eq i32 %86, 0, !dbg !1711
  %648 = zext i1 %647 to i32, !dbg !1716
  br label %649, !dbg !1659

649:                                              ; preds = %1078, %619
  %650 = phi i32 [ %64, %619 ], [ %1085, %1078 ]
  %651 = phi i32 [ %609, %619 ], [ %1084, %1078 ]
  %652 = phi i32 [ 1, %619 ], [ %1083, %1078 ]
  %653 = phi i32 [ -32000, %619 ], [ %1082, %1078 ]
  %654 = phi i32 [ 1, %619 ], [ %1081, %1078 ]
  %655 = phi i32 [ 1, %619 ], [ %1080, %1078 ]
  call void @llvm.dbg.value(metadata i32 %650, metadata !1034, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %651, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %652, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %653, metadata !1050, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %654, metadata !1054, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %655, metadata !1062, metadata !DIExpression()), !dbg !1120
  %656 = icmp ne i32 %654, 0, !dbg !1717
  %657 = icmp sgt i32 %655, %632, !dbg !1719
  %658 = add nsw i32 %650, 1, !dbg !1720
  %659 = icmp eq i32 %70, %658, !dbg !1721
  br label %660, !dbg !1659

660:                                              ; preds = %789, %649
  %661 = phi i32 [ %652, %649 ], [ 0, %789 ]
  call void @llvm.dbg.value(metadata i32 %661, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1055, metadata !DIExpression()), !dbg !1120
  %662 = load i32, i32* %31, align 8, !dbg !1722, !tbaa !968
  %663 = icmp slt i32 %662, 60, !dbg !1723
  %664 = load i32, i32* %9, align 4, !dbg !1724, !tbaa !866
  %665 = sext i32 %664 to i64, !dbg !1724
  br i1 %663, label %666, label %746, !dbg !1725

666:                                              ; preds = %660
  call void @llvm.dbg.value(metadata i32 %622, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %664, metadata !1042, metadata !DIExpression()), !dbg !1120
  %667 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %665, !dbg !1663, !intel-tbaa !1400
  %668 = load i32, i32* %667, align 4, !dbg !1663, !tbaa !1400
  %669 = lshr i32 %668, 6, !dbg !1663
  %670 = and i32 %669, 63, !dbg !1663
  %671 = zext i32 %670 to i64, !dbg !1663
  %672 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %671, !dbg !1663, !intel-tbaa !1208
  %673 = load i32, i32* %672, align 4, !dbg !1663, !tbaa !1438
  %674 = add nsw i32 %673, 1, !dbg !1663
  %675 = and i32 %674, -2, !dbg !1726
  %676 = icmp eq i32 %675, 2, !dbg !1726
  br i1 %676, label %677, label %684, !dbg !1727

677:                                              ; preds = %666
  %678 = lshr i32 %668, 3, !dbg !1728
  %679 = and i32 %678, 7, !dbg !1728
  switch i32 %679, label %680 [
    i32 1, label %683
    i32 6, label %683
  ], !dbg !1729

680:                                              ; preds = %677
  %681 = and i32 %668, 61440, !dbg !1730
  %682 = icmp eq i32 %681, 0, !dbg !1730
  br i1 %682, label %684, label %683, !dbg !1731

683:                                              ; preds = %680, %677, %677
  br label %684, !dbg !1665

684:                                              ; preds = %683, %680, %666
  %685 = phi i32 [ %622, %680 ], [ %622, %666 ], [ %625, %683 ]
  call void @llvm.dbg.value(metadata i32 %685, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %664, metadata !1042, metadata !DIExpression()), !dbg !1120
  %686 = lshr i32 %668, 19, !dbg !1732
  %687 = and i32 %686, 15, !dbg !1732
  %688 = icmp eq i32 %687, 13, !dbg !1733
  br i1 %688, label %719, label %689, !dbg !1734

689:                                              ; preds = %684
  %690 = add nsw i32 %662, -1, !dbg !1667
  %691 = sext i32 %690 to i64, !dbg !1667
  %692 = getelementptr inbounds [64 x i32], [64 x i32]* %626, i64 0, i64 %691, !dbg !1667, !intel-tbaa !1208
  %693 = load i32, i32* %692, align 4, !dbg !1667, !tbaa !1315
  %694 = lshr i32 %693, 19, !dbg !1667
  %695 = and i32 %694, 15, !dbg !1667
  %696 = icmp eq i32 %695, 13, !dbg !1735
  br i1 %696, label %719, label %697, !dbg !1736

697:                                              ; preds = %689
  %698 = zext i32 %687 to i64, !dbg !1737
  %699 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %698, !dbg !1737, !intel-tbaa !1440
  %700 = load i32, i32* %699, align 4, !dbg !1737, !tbaa !1440
  %701 = zext i32 %695 to i64, !dbg !1738
  %702 = getelementptr inbounds [14 x i32], [14 x i32]* @_ZL8rc_index, i64 0, i64 %701, !dbg !1738, !intel-tbaa !1440
  %703 = load i32, i32* %702, align 4, !dbg !1738, !tbaa !1440
  %704 = icmp eq i32 %700, %703, !dbg !1739
  br i1 %704, label %705, label %719, !dbg !1740

705:                                              ; preds = %697
  %706 = and i32 %668, 63, !dbg !1741
  %707 = and i32 %693, 63, !dbg !1742
  %708 = icmp eq i32 %706, %707, !dbg !1743
  br i1 %708, label %709, label %719, !dbg !1744

709:                                              ; preds = %705
  %710 = load i32, i32* %627, align 4, !dbg !1668, !tbaa !885
  %711 = icmp eq i32 %710, 0, !dbg !1668
  %712 = zext i1 %711 to i32, !dbg !1668
  %713 = lshr i32 %668, 12, !dbg !1745
  %714 = and i32 %713, 15, !dbg !1745
  %715 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %712, i32 %670, i32 %706, i32 %714), !dbg !1746
  call void @llvm.dbg.value(metadata i32 %715, metadata !1107, metadata !DIExpression()), !dbg !1747
  %716 = icmp sgt i32 %715, 0, !dbg !1748
  %717 = select i1 %716, i32 %628, i32 0, !dbg !1669
  %718 = add nsw i32 %717, %685, !dbg !1669
  br label %719, !dbg !1669

719:                                              ; preds = %709, %705, %697, %689, %684
  %720 = phi i32 [ %685, %705 ], [ %685, %697 ], [ %685, %689 ], [ %685, %684 ], [ %718, %709 ]
  call void @llvm.dbg.value(metadata i32 %720, metadata !1055, metadata !DIExpression()), !dbg !1120
  %721 = load i32, i32* %14, align 4, !dbg !1750, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %721, metadata !1063, metadata !DIExpression()), !dbg !1120
  %722 = icmp eq i32 %721, 1, !dbg !1751
  %723 = icmp ne i32 %720, 0, !dbg !1752
  %724 = and i1 %723, %722, !dbg !1753
  %725 = and i1 %656, %724, !dbg !1753
  br i1 %725, label %726, label %727, !dbg !1753

726:                                              ; preds = %719
  call void @llvm.dbg.value(metadata i32 1, metadata !1065, metadata !DIExpression()), !dbg !1120
  store i32 1, i32* %15, align 4, !dbg !1754, !tbaa !866
  br label %732, !dbg !1756

727:                                              ; preds = %719
  %728 = icmp eq i32 %720, 0, !dbg !1757
  %729 = and i1 %728, %722, !dbg !1759
  %730 = and i1 %656, %729, !dbg !1759
  br i1 %730, label %731, label %732, !dbg !1759

731:                                              ; preds = %727
  call void @llvm.dbg.value(metadata i32 0, metadata !1065, metadata !DIExpression()), !dbg !1120
  store i32 0, i32* %15, align 4, !dbg !1760, !tbaa !866
  br label %735, !dbg !1762

732:                                              ; preds = %727, %726
  call void @llvm.dbg.value(metadata i32 %720, metadata !1055, metadata !DIExpression()), !dbg !1120
  %733 = icmp slt i32 %720, 4, !dbg !1763
  %734 = select i1 %733, i32 %720, i32 4, !dbg !1763
  br label %735, !dbg !1763

735:                                              ; preds = %732, %731
  %736 = phi i32 [ %628, %731 ], [ %734, %732 ]
  call void @llvm.dbg.value(metadata i32 %736, metadata !1055, metadata !DIExpression()), !dbg !1120
  %737 = load i32, i32* %9, align 4, !dbg !1764, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %737, metadata !1042, metadata !DIExpression()), !dbg !1120
  %738 = sext i32 %737 to i64, !dbg !1764
  %739 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %738, !dbg !1764, !intel-tbaa !1400
  %740 = load i32, i32* %739, align 4, !dbg !1764, !tbaa !1400
  %741 = lshr i32 %740, 19, !dbg !1764
  %742 = and i32 %741, 15, !dbg !1764
  switch i32 %742, label %743 [
    i32 13, label %746
    i32 1, label %746
    i32 2, label %746
  ], !dbg !1765

743:                                              ; preds = %735
  %744 = add nsw i32 %736, 4, !dbg !1766
  %745 = select i1 %630, i32 %744, i32 %736, !dbg !1768
  br label %746, !dbg !1768

746:                                              ; preds = %743, %735, %735, %735, %660
  %747 = phi i64 [ %738, %743 ], [ %738, %735 ], [ %738, %735 ], [ %738, %735 ], [ %665, %660 ], !dbg !1769
  %748 = phi i32 [ %745, %743 ], [ %736, %735 ], [ %736, %735 ], [ %736, %735 ], [ 0, %660 ]
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 undef, metadata !1042, metadata !DIExpression()), !dbg !1120
  %749 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %747, !dbg !1769, !intel-tbaa !1400
  %750 = load i32, i32* %749, align 4, !dbg !1769, !tbaa !1400
  %751 = and i32 %750, 7864320, !dbg !1769
  %752 = icmp eq i32 %751, 6815744, !dbg !1770
  %753 = xor i1 %752, true, !dbg !1771
  %754 = xor i1 %657, true, !dbg !1771
  %755 = or i1 %753, %754, !dbg !1771
  %756 = select i1 %753, i1 false, i1 true, !dbg !1771
  %757 = select i1 %753, i32 %661, i32 %652, !dbg !1771
  br i1 %755, label %795, label %758, !dbg !1771

758:                                              ; preds = %746
  call void @llvm.dbg.value(metadata %struct.state_t* %0, metadata !1684, metadata !DIExpression()), !dbg !1772
  call void @llvm.dbg.value(metadata i32 %750, metadata !1685, metadata !DIExpression()), !dbg !1772
  call void @llvm.dbg.value(metadata i32 %632, metadata !1686, metadata !DIExpression()), !dbg !1772
  %759 = lshr i32 %750, 6, !dbg !1773
  %760 = and i32 %759, 63, !dbg !1773
  %761 = zext i32 %760 to i64, !dbg !1774
  %762 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %761, !dbg !1774, !intel-tbaa !1208
  %763 = load i32, i32* %762, align 4, !dbg !1774, !tbaa !1438
  %764 = add nsw i32 %763, -1, !dbg !1775
  call void @llvm.dbg.value(metadata i32 %764, metadata !1687, metadata !DIExpression()), !dbg !1772
  %765 = and i32 %750, 63, !dbg !1776
  call void @llvm.dbg.value(metadata i32 %765, metadata !1688, metadata !DIExpression()), !dbg !1772
  %766 = load i32, i32* %633, align 8, !dbg !1678, !tbaa !1777
  %767 = sext i32 %766 to i64, !dbg !1778
  %768 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %767, !dbg !1778, !intel-tbaa !1779
  %769 = sext i32 %764 to i64, !dbg !1778
  %770 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %768, i64 0, i64 %769, !dbg !1778, !intel-tbaa !1782
  %771 = zext i32 %765 to i64, !dbg !1778
  %772 = getelementptr inbounds [64 x i32], [64 x i32]* %770, i64 0, i64 %771, !dbg !1778, !intel-tbaa !1208
  %773 = load i32, i32* %772, align 4, !dbg !1778, !tbaa !1783
  call void @llvm.dbg.value(metadata i32 %773, metadata !1690, metadata !DIExpression()), !dbg !1772
  %774 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %767, !dbg !1784, !intel-tbaa !1779
  %775 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %774, i64 0, i64 %769, !dbg !1784, !intel-tbaa !1782
  %776 = getelementptr inbounds [64 x i32], [64 x i32]* %775, i64 0, i64 %771, !dbg !1784, !intel-tbaa !1208
  %777 = load i32, i32* %776, align 4, !dbg !1784, !tbaa !1783
  %778 = sub nsw i32 %777, %773, !dbg !1785
  call void @llvm.dbg.value(metadata i32 %778, metadata !1689, metadata !DIExpression()), !dbg !1772
  %779 = mul nsw i32 %773, %632, !dbg !1786
  %780 = icmp sge i32 %779, %778, !dbg !1787
  %781 = or i1 %634, %780, !dbg !1788
  %782 = icmp ne i32 %748, 0, !dbg !1789
  %783 = or i1 %782, %781, !dbg !1788
  %784 = xor i1 %783, true, !dbg !1788
  %785 = and i1 %659, %784, !dbg !1788
  %786 = and i32 %750, 61440, !dbg !1790
  %787 = icmp eq i32 %786, 0, !dbg !1790
  %788 = and i1 %787, %785, !dbg !1788
  call void @llvm.dbg.value(metadata i32 undef, metadata !1042, metadata !DIExpression()), !dbg !1120
  br i1 %788, label %789, label %795, !dbg !1788

789:                                              ; preds = %758
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %790 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !1791
  call void @llvm.dbg.value(metadata i32 %650, metadata !1034, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %651, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %653, metadata !1050, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %654, metadata !1054, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %655, metadata !1062, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %790, metadata !1061, metadata !DIExpression()), !dbg !1120
  %791 = icmp eq i32 %790, 0, !dbg !1658
  br i1 %791, label %792, label %660, !dbg !1659, !llvm.loop !1793

792:                                              ; preds = %789
  call void @llvm.dbg.value(metadata i32 %653, metadata !1050, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %653, metadata !1050, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %653, metadata !1050, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1089, metadata !1045, metadata !DIExpression()), !dbg !1120
  %793 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !1795, !tbaa !1298
  %794 = icmp eq i32 %793, 0, !dbg !1796
  call void @llvm.dbg.value(metadata i1 %1091, metadata !1119, metadata !DIExpression()), !dbg !1120
  br label %1106, !dbg !1797

795:                                              ; preds = %758, %746
  %796 = phi i1 [ true, %758 ], [ %756, %746 ], !dbg !1770
  %797 = phi i32 [ %661, %758 ], [ %757, %746 ]
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %797, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %797, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %797, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %748, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %797, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %797, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %797, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1113, metadata !DIExpression()), !dbg !1724
  call void @llvm.dbg.value(metadata i32 0, metadata !1114, metadata !DIExpression()), !dbg !1724
  br i1 %635, label %798, label %801, !dbg !1799

798:                                              ; preds = %795
  %799 = icmp slt i32 %639, %650, !dbg !1800
  call void @llvm.dbg.value(metadata i1 %799, metadata !1113, metadata !DIExpression()), !dbg !1724
  %800 = icmp slt i32 %640, %650, !dbg !1801
  br label %806, !dbg !1802

801:                                              ; preds = %795
  %802 = icmp slt i32 %637, %650, !dbg !1697
  %803 = icmp slt i32 %638, %650, !dbg !1697
  %804 = and i1 %636, %802, !dbg !1697
  %805 = and i1 %636, %803, !dbg !1697
  br label %806, !dbg !1697

806:                                              ; preds = %801, %798
  %807 = phi i1 [ %799, %798 ], [ %804, %801 ]
  %808 = phi i1 [ %800, %798 ], [ %805, %801 ]
  call void @llvm.dbg.value(metadata i1 %808, metadata !1114, metadata !DIExpression()), !dbg !1724
  call void @llvm.dbg.value(metadata i1 %807, metadata !1113, metadata !DIExpression()), !dbg !1724
  call void @llvm.dbg.value(metadata i32 undef, metadata !1042, metadata !DIExpression()), !dbg !1120
  br i1 %796, label %821, label %809, !dbg !1803

809:                                              ; preds = %806
  %810 = load i32, i32* %627, align 4, !dbg !1804, !tbaa !885
  %811 = icmp eq i32 %810, 0, !dbg !1804
  %812 = zext i1 %811 to i32, !dbg !1804
  %813 = lshr i32 %750, 6, !dbg !1807
  %814 = and i32 %813, 63, !dbg !1807
  %815 = and i32 %750, 63, !dbg !1808
  %816 = lshr i32 %750, 12, !dbg !1809
  %817 = and i32 %816, 15, !dbg !1809
  %818 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %812, i32 %814, i32 %815, i32 %817), !dbg !1810
  call void @llvm.dbg.value(metadata i32 %818, metadata !1115, metadata !DIExpression()), !dbg !1724
  %819 = load i32, i32* %9, align 4, !dbg !1811, !tbaa !866
  %820 = sext i32 %819 to i64, !dbg !1812
  br label %821, !dbg !1813

821:                                              ; preds = %809, %806
  %822 = phi i64 [ %747, %806 ], [ %820, %809 ], !dbg !1812
  %823 = phi i32 [ -1000000, %806 ], [ %818, %809 ]
  call void @llvm.dbg.value(metadata i32 %823, metadata !1115, metadata !DIExpression()), !dbg !1724
  call void @llvm.dbg.value(metadata i32 undef, metadata !1042, metadata !DIExpression()), !dbg !1120
  %824 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %822, !dbg !1812, !intel-tbaa !1400
  %825 = load i32, i32* %824, align 4, !dbg !1812, !tbaa !1400
  call void @_Z4makeP7state_ti(%struct.state_t* nonnull %0, i32 %825), !dbg !1814
  call void @llvm.dbg.value(metadata i32 0, metadata !1046, metadata !DIExpression()), !dbg !1120
  %826 = load i32, i32* %9, align 4, !dbg !1815, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %826, metadata !1042, metadata !DIExpression()), !dbg !1120
  %827 = sext i32 %826 to i64, !dbg !1816
  %828 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %827, !dbg !1816, !intel-tbaa !1400
  %829 = load i32, i32* %828, align 4, !dbg !1816, !tbaa !1400
  %830 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* nonnull %0, i32 %829), !dbg !1817
  %831 = icmp eq i32 %830, 0, !dbg !1817
  br i1 %831, label %974, label %832, !dbg !1818

832:                                              ; preds = %821
  call void @llvm.dbg.value(metadata i32 1, metadata !1046, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1045, metadata !DIExpression()), !dbg !1120
  %833 = call i32 @_Z8in_checkP7state_t(%struct.state_t* nonnull %0), !dbg !1819
  call void @llvm.dbg.value(metadata i32 %833, metadata !1058, metadata !DIExpression()), !dbg !1120
  %834 = icmp ne i32 %833, 0, !dbg !1820
  %835 = select i1 %834, i32 %641, i32 0, !dbg !1703
  %836 = add nsw i32 %835, %748, !dbg !1703
  call void @llvm.dbg.value(metadata i32 %836, metadata !1055, metadata !DIExpression()), !dbg !1120
  %837 = or i32 %833, %91, !dbg !1822
  %838 = icmp eq i32 %837, 0, !dbg !1822
  %839 = and i1 %659, %838, !dbg !1822
  br i1 %839, label %840, label %864, !dbg !1822

840:                                              ; preds = %832
  %841 = icmp slt i32 %823, 86, !dbg !1824
  %842 = and i1 %808, %841, !dbg !1829
  br i1 %842, label %843, label %852, !dbg !1829

843:                                              ; preds = %840
  %844 = load i32, i32* %9, align 4, !dbg !1830, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %844, metadata !1042, metadata !DIExpression()), !dbg !1120
  %845 = sext i32 %844 to i64, !dbg !1830
  %846 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %845, !dbg !1830, !intel-tbaa !1400
  %847 = load i32, i32* %846, align 4, !dbg !1830, !tbaa !1400
  %848 = and i32 %847, 61440, !dbg !1830
  %849 = icmp eq i32 %848, 0, !dbg !1830
  br i1 %849, label %850, label %852, !dbg !1831

850:                                              ; preds = %843
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %847), !dbg !1832
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %851 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !1834
  call void @llvm.dbg.value(metadata i32 %851, metadata !1061, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %650, metadata !1050, metadata !DIExpression()), !dbg !1120
  br label %1078, !dbg !1835, !llvm.loop !1793

852:                                              ; preds = %843, %840
  %853 = icmp slt i32 %823, -50, !dbg !1836
  %854 = and i1 %807, %853, !dbg !1840
  br i1 %854, label %855, label %864, !dbg !1840

855:                                              ; preds = %852
  %856 = load i32, i32* %9, align 4, !dbg !1841, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %856, metadata !1042, metadata !DIExpression()), !dbg !1120
  %857 = sext i32 %856 to i64, !dbg !1841
  %858 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %857, !dbg !1841, !intel-tbaa !1400
  %859 = load i32, i32* %858, align 4, !dbg !1841, !tbaa !1400
  %860 = and i32 %859, 61440, !dbg !1841
  %861 = icmp eq i32 %860, 0, !dbg !1841
  br i1 %861, label %862, label %864, !dbg !1842

862:                                              ; preds = %855
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %859), !dbg !1843
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %863 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !1845
  call void @llvm.dbg.value(metadata i32 %863, metadata !1061, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %650, metadata !1050, metadata !DIExpression()), !dbg !1120
  br label %1078, !dbg !1846, !llvm.loop !1793

864:                                              ; preds = %855, %852, %832
  %865 = add nsw i32 %836, %3, !dbg !1847
  %866 = add nsw i32 %865, -4, !dbg !1848
  call void @llvm.dbg.value(metadata i32 %866, metadata !1116, metadata !DIExpression()), !dbg !1849
  %867 = sub nsw i32 0, %650, !dbg !1850
  %868 = sub i32 130, %650, !dbg !1851
  %869 = icmp sgt i32 %865, 4, !dbg !1852
  %870 = or i1 %834, %869, !dbg !1853
  %871 = zext i1 %870 to i32, !dbg !1854
  %872 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %642, i32 %868, i32 %871), !dbg !1855
  %873 = load i32, i32* %31, align 8, !dbg !1856, !tbaa !968
  %874 = sext i32 %873 to i64, !dbg !1857
  %875 = getelementptr inbounds [64 x i32], [64 x i32]* %87, i64 0, i64 %874, !dbg !1857, !intel-tbaa !1208
  store i32 %833, i32* %875, align 4, !dbg !1858, !tbaa !1209
  %876 = load i64, i64* %643, align 8, !dbg !1705, !tbaa !888
  %877 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !dbg !1859, !tbaa !1513
  %878 = add i32 %873, -1, !dbg !1860
  %879 = add i32 %878, %877, !dbg !1861
  %880 = sext i32 %879 to i64, !dbg !1862
  %881 = getelementptr inbounds [1000 x i64], [1000 x i64]* %644, i64 0, i64 %880, !dbg !1862, !intel-tbaa !1518
  store i64 %876, i64* %881, align 8, !dbg !1863, !tbaa !1520
  %882 = load i32, i32* %9, align 4, !dbg !1864, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %882, metadata !1042, metadata !DIExpression()), !dbg !1120
  %883 = sext i32 %882 to i64, !dbg !1865
  %884 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %883, !dbg !1865, !intel-tbaa !1400
  %885 = load i32, i32* %884, align 4, !dbg !1865, !tbaa !1400
  %886 = sext i32 %878 to i64, !dbg !1866
  %887 = getelementptr inbounds [64 x i32], [64 x i32]* %626, i64 0, i64 %886, !dbg !1866, !intel-tbaa !1208
  store i32 %885, i32* %887, align 4, !dbg !1867, !tbaa !1315
  call void @llvm.dbg.value(metadata i32 0, metadata !1064, metadata !DIExpression()), !dbg !1120
  %888 = icmp sgt i32 %655, 3, !dbg !1868
  %889 = and i1 %645, %888, !dbg !1869
  br i1 %889, label %890, label %924, !dbg !1869

890:                                              ; preds = %864
  %891 = or i32 %836, %833, !dbg !1870
  %892 = icmp eq i32 %891, 0, !dbg !1870
  %893 = and i1 %659, %892, !dbg !1870
  %894 = icmp slt i32 %823, -50, !dbg !1871
  %895 = and i1 %894, %893, !dbg !1870
  br i1 %895, label %896, label %924, !dbg !1870

896:                                              ; preds = %890
  call void @llvm.dbg.value(metadata i32 %882, metadata !1042, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata %struct.state_t* %0, metadata !1872, metadata !DIExpression()), !dbg !1881
  call void @llvm.dbg.value(metadata i32 %885, metadata !1877, metadata !DIExpression()), !dbg !1881
  %897 = and i32 %885, 63, !dbg !1883
  %898 = zext i32 %897 to i64, !dbg !1884
  %899 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %898, !dbg !1884, !intel-tbaa !1208
  %900 = load i32, i32* %899, align 4, !dbg !1884, !tbaa !1438
  %901 = add nsw i32 %900, -1, !dbg !1885
  call void @llvm.dbg.value(metadata i32 %901, metadata !1878, metadata !DIExpression()), !dbg !1881
  call void @llvm.dbg.value(metadata i32 %897, metadata !1879, metadata !DIExpression()), !dbg !1881
  %902 = load i32, i32* %633, align 8, !dbg !1886, !tbaa !1777
  %903 = sext i32 %902 to i64, !dbg !1887
  %904 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %903, !dbg !1887, !intel-tbaa !1779
  %905 = sext i32 %901 to i64, !dbg !1887
  %906 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %904, i64 0, i64 %905, !dbg !1887, !intel-tbaa !1782
  %907 = getelementptr inbounds [64 x i32], [64 x i32]* %906, i64 0, i64 %898, !dbg !1887, !intel-tbaa !1208
  %908 = load i32, i32* %907, align 4, !dbg !1887, !tbaa !1783
  %909 = shl i32 %908, 7, !dbg !1888
  %910 = add i32 %909, 128, !dbg !1888
  %911 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %903, !dbg !1889, !intel-tbaa !1779
  %912 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %911, i64 0, i64 %905, !dbg !1889, !intel-tbaa !1782
  %913 = getelementptr inbounds [64 x i32], [64 x i32]* %912, i64 0, i64 %898, !dbg !1889, !intel-tbaa !1208
  %914 = load i32, i32* %913, align 4, !dbg !1889, !tbaa !1783
  %915 = add nsw i32 %914, 1, !dbg !1890
  %916 = sdiv i32 %910, %915, !dbg !1891
  call void @llvm.dbg.value(metadata i32 %916, metadata !1880, metadata !DIExpression()), !dbg !1881
  %917 = icmp slt i32 %916, 80, !dbg !1892
  %918 = and i32 %885, 61440, !dbg !1893
  %919 = icmp eq i32 %918, 0, !dbg !1893
  %920 = and i1 %919, %917, !dbg !1894
  call void @llvm.dbg.value(metadata i32 %882, metadata !1042, metadata !DIExpression()), !dbg !1120
  br i1 %920, label %921, label %924, !dbg !1894

921:                                              ; preds = %896
  %922 = add nsw i32 %836, -4, !dbg !1895
  call void @llvm.dbg.value(metadata i32 %922, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 4, metadata !1064, metadata !DIExpression()), !dbg !1120
  %923 = add i32 %646, %922, !dbg !1896
  call void @llvm.dbg.value(metadata i32 %923, metadata !1116, metadata !DIExpression()), !dbg !1849
  br label %924, !dbg !1897

924:                                              ; preds = %921, %896, %890, %864
  %925 = phi i32 [ 4, %921 ], [ 0, %896 ], [ 0, %890 ], [ 0, %864 ]
  %926 = phi i32 [ %922, %921 ], [ %836, %896 ], [ %836, %890 ], [ %836, %864 ]
  %927 = phi i32 [ %923, %921 ], [ %866, %896 ], [ %866, %890 ], [ %866, %864 ]
  call void @llvm.dbg.value(metadata i32 %927, metadata !1116, metadata !DIExpression()), !dbg !1849
  call void @llvm.dbg.value(metadata i32 %926, metadata !1055, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %925, metadata !1064, metadata !DIExpression()), !dbg !1120
  %928 = icmp eq i32 %654, 1, !dbg !1898
  %929 = icmp slt i32 %927, 1, !dbg !1899
  br i1 %928, label %930, label %937, !dbg !1900

930:                                              ; preds = %924
  br i1 %929, label %931, label %934, !dbg !1901

931:                                              ; preds = %930
  %932 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %642, i32 %867, i32 0, i32 0), !dbg !1902
  %933 = sub nsw i32 0, %932, !dbg !1904
  call void @llvm.dbg.value(metadata i32 %933, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %970, !dbg !1905

934:                                              ; preds = %930
  %935 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %642, i32 %867, i32 %927, i32 0, i32 %648), !dbg !1906
  %936 = sub nsw i32 0, %935, !dbg !1907
  call void @llvm.dbg.value(metadata i32 %936, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %970

937:                                              ; preds = %924
  %938 = xor i32 %650, -1, !dbg !1908
  br i1 %929, label %939, label %941, !dbg !1911

939:                                              ; preds = %937
  %940 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %938, i32 %867, i32 0, i32 0), !dbg !1912
  call void @llvm.dbg.value(metadata i32 undef, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %943, !dbg !1914

941:                                              ; preds = %937
  %942 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %938, i32 %867, i32 %927, i32 0, i32 1), !dbg !1915
  call void @llvm.dbg.value(metadata i32 undef, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %943

943:                                              ; preds = %941, %939
  %944 = phi i32 [ %940, %939 ], [ %942, %941 ]
  %945 = sub nsw i32 0, %944, !dbg !1908
  call void @llvm.dbg.value(metadata i32 %945, metadata !1044, metadata !DIExpression()), !dbg !1120
  %946 = icmp slt i32 %653, %945, !dbg !1917
  %947 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !1919
  %948 = icmp eq i32 %947, 0, !dbg !1920
  %949 = and i1 %946, %948, !dbg !1921
  %950 = icmp slt i32 %650, %945, !dbg !1922
  %951 = and i1 %950, %949, !dbg !1921
  br i1 %951, label %952, label %970, !dbg !1921

952:                                              ; preds = %943
  %953 = icmp eq i32 %925, 0, !dbg !1925
  br i1 %953, label %956, label %954, !dbg !1928

954:                                              ; preds = %952
  %955 = add nsw i32 %926, %925, !dbg !1929
  call void @llvm.dbg.value(metadata i32 %926, metadata !1055, metadata !DIExpression()), !dbg !1120
  br label %958, !dbg !1931

956:                                              ; preds = %952
  call void @llvm.dbg.value(metadata i32 %926, metadata !1055, metadata !DIExpression()), !dbg !1120
  %957 = icmp sgt i32 %70, %945, !dbg !1933
  br i1 %957, label %958, label %970, !dbg !1931

958:                                              ; preds = %956, %954
  %959 = phi i32 [ %955, %954 ], [ %926, %956 ]
  %960 = add nsw i32 %959, %3, !dbg !1934
  call void @llvm.dbg.value(metadata i32 %960, metadata !1116, metadata !DIExpression(DW_OP_constu, 4, DW_OP_minus, DW_OP_stack_value)), !dbg !1849
  %961 = icmp slt i32 %960, 5, !dbg !1936
  br i1 %961, label %962, label %965, !dbg !1938

962:                                              ; preds = %958
  %963 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %642, i32 %867, i32 0, i32 0), !dbg !1939
  %964 = sub nsw i32 0, %963, !dbg !1941
  call void @llvm.dbg.value(metadata i32 %964, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %970, !dbg !1942

965:                                              ; preds = %958
  %966 = add nsw i32 %960, -4, !dbg !1943
  call void @llvm.dbg.value(metadata i32 %966, metadata !1116, metadata !DIExpression()), !dbg !1849
  %967 = lshr exact i32 %925, 2, !dbg !1944
  %968 = call i32 @_Z6searchP7state_tiiiii(%struct.state_t* nonnull %0, i32 %642, i32 %867, i32 %966, i32 0, i32 %967), !dbg !1946
  %969 = sub nsw i32 0, %968, !dbg !1947
  call void @llvm.dbg.value(metadata i32 %969, metadata !1044, metadata !DIExpression()), !dbg !1120
  br label %970

970:                                              ; preds = %965, %962, %956, %943, %934, %931
  %971 = phi i32 [ %933, %931 ], [ %936, %934 ], [ %945, %943 ], [ %964, %962 ], [ %969, %965 ], [ %945, %956 ]
  call void @llvm.dbg.value(metadata i32 %971, metadata !1044, metadata !DIExpression()), !dbg !1120
  %972 = icmp sgt i32 %971, %653, !dbg !1948
  %973 = select i1 %972, i32 %971, i32 %653, !dbg !1950
  call void @llvm.dbg.value(metadata i32 %973, metadata !1050, metadata !DIExpression()), !dbg !1120
  br label %974, !dbg !1951

974:                                              ; preds = %970, %821
  %975 = phi i32 [ %973, %970 ], [ %653, %821 ]
  %976 = phi i32 [ 1, %970 ], [ 0, %821 ]
  %977 = phi i32 [ 0, %970 ], [ %797, %821 ]
  %978 = phi i32 [ %971, %970 ], [ %651, %821 ]
  call void @llvm.dbg.value(metadata i32 %978, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %977, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %976, metadata !1046, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %975, metadata !1050, metadata !DIExpression()), !dbg !1120
  %979 = load i32, i32* %9, align 4, !dbg !1952, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %979, metadata !1042, metadata !DIExpression()), !dbg !1120
  %980 = sext i32 %979 to i64, !dbg !1953
  %981 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %980, !dbg !1953, !intel-tbaa !1400
  %982 = load i32, i32* %981, align 4, !dbg !1953, !tbaa !1400
  call void @_Z6unmakeP7state_ti(%struct.state_t* nonnull %0, i32 %982), !dbg !1954
  %983 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !1955, !tbaa !1298
  %984 = icmp eq i32 %983, 0, !dbg !1957
  br i1 %984, label %985, label %1119, !dbg !1958

985:                                              ; preds = %974
  %986 = icmp eq i32 %976, 0, !dbg !1959
  br i1 %986, label %1073, label %987, !dbg !1961

987:                                              ; preds = %985
  %988 = icmp sgt i32 %978, %650, !dbg !1962
  br i1 %988, label %989, label %1063, !dbg !1965

989:                                              ; preds = %987
  %990 = icmp slt i32 %978, %70, !dbg !1966
  %991 = load i32, i32* %9, align 4, !dbg !1969, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %991, metadata !1042, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %991, metadata !1042, metadata !DIExpression()), !dbg !1120
  %992 = sext i32 %991 to i64, !dbg !1969
  %993 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %992, !dbg !1969
  %994 = load i32, i32* %993, align 4, !dbg !1969, !tbaa !1400
  br i1 %990, label %1060, label %995, !dbg !1970

995:                                              ; preds = %989
  call void @llvm.dbg.value(metadata i32 %655, metadata !1062, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %978, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %655, metadata !1062, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %978, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %655, metadata !1062, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %978, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %655, metadata !1062, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %978, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %978, metadata !1044, metadata !DIExpression()), !dbg !1120
  call fastcc void @_ZL12history_goodP7state_tii(%struct.state_t* nonnull %0, i32 %994, i32 %3), !dbg !1971
  call void @llvm.dbg.value(metadata i32 0, metadata !1043, metadata !DIExpression()), !dbg !1120
  %996 = icmp sgt i32 %655, 1, !dbg !1973
  br i1 %996, label %997, label %1050, !dbg !1976

997:                                              ; preds = %995
  %998 = add nsw i32 %3, 3, !dbg !1977
  %999 = sdiv i32 %998, 4, !dbg !1993
  %1000 = add nsw i32 %655, -1, !dbg !1976
  %1001 = sext i32 %1000 to i64, !dbg !1973
  br label %1002, !dbg !1976

1002:                                             ; preds = %1047, %997
  %1003 = phi i64 [ 0, %997 ], [ %1048, %1047 ]
  call void @llvm.dbg.value(metadata i64 %1003, metadata !1043, metadata !DIExpression()), !dbg !1120
  %1004 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %1003, !dbg !1994, !intel-tbaa !1400
  %1005 = load i32, i32* %1004, align 4, !dbg !1994, !tbaa !1400
  call void @llvm.dbg.value(metadata %struct.state_t* %0, metadata !1984, metadata !DIExpression()), !dbg !1995
  call void @llvm.dbg.value(metadata i32 %1005, metadata !1985, metadata !DIExpression()), !dbg !1995
  call void @llvm.dbg.value(metadata i32 %3, metadata !1986, metadata !DIExpression()), !dbg !1995
  %1006 = and i32 %1005, 7925760, !dbg !1996
  %1007 = icmp eq i32 %1006, 6815744, !dbg !1996
  br i1 %1007, label %1008, label %1047, !dbg !1996

1008:                                             ; preds = %1002
  %1009 = lshr i32 %1005, 6, !dbg !1997
  %1010 = and i32 %1009, 63, !dbg !1997
  %1011 = zext i32 %1010 to i64, !dbg !1998
  %1012 = getelementptr inbounds [64 x i32], [64 x i32]* %623, i64 0, i64 %1011, !dbg !1998, !intel-tbaa !1208
  %1013 = load i32, i32* %1012, align 4, !dbg !1998, !tbaa !1438
  %1014 = add nsw i32 %1013, -1, !dbg !1999
  call void @llvm.dbg.value(metadata i32 %1014, metadata !1989, metadata !DIExpression()), !dbg !1995
  %1015 = and i32 %1005, 63, !dbg !2000
  call void @llvm.dbg.value(metadata i32 %1015, metadata !1990, metadata !DIExpression()), !dbg !1995
  %1016 = load i32, i32* %633, align 8, !dbg !2001, !tbaa !1777
  %1017 = sext i32 %1016 to i64, !dbg !2002
  %1018 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %1017, !dbg !2002, !intel-tbaa !1779
  %1019 = sext i32 %1014 to i64, !dbg !2002
  %1020 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1018, i64 0, i64 %1019, !dbg !2002, !intel-tbaa !1782
  %1021 = zext i32 %1015 to i64, !dbg !2002
  %1022 = getelementptr inbounds [64 x i32], [64 x i32]* %1020, i64 0, i64 %1021, !dbg !2002, !intel-tbaa !1208
  %1023 = load i32, i32* %1022, align 4, !dbg !2003, !tbaa !1783
  %1024 = add nsw i32 %1023, %999, !dbg !2003
  store i32 %1024, i32* %1022, align 4, !dbg !2003, !tbaa !1783
  %1025 = icmp sgt i32 %1024, 16384, !dbg !2004
  br i1 %1025, label %1026, label %1047, !dbg !2006

1026:                                             ; preds = %1008
  call void @llvm.dbg.value(metadata i32 0, metadata !1987, metadata !DIExpression()), !dbg !1995
  %1027 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %1017, !dbg !2007, !intel-tbaa !1779
  br label %1028, !dbg !2015

1028:                                             ; preds = %1044, %1026
  %1029 = phi i64 [ 0, %1026 ], [ %1045, %1044 ]
  call void @llvm.dbg.value(metadata i64 %1029, metadata !1987, metadata !DIExpression()), !dbg !1995
  call void @llvm.dbg.value(metadata i32 0, metadata !1988, metadata !DIExpression()), !dbg !1995
  %1030 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1027, i64 0, i64 %1029, !dbg !2007, !intel-tbaa !1782
  %1031 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %1018, i64 0, i64 %1029, !dbg !2016, !intel-tbaa !1782
  br label %1032, !dbg !2017

1032:                                             ; preds = %1032, %1028
  %1033 = phi i64 [ 0, %1028 ], [ %1042, %1032 ]
  call void @llvm.dbg.value(metadata i64 %1033, metadata !1988, metadata !DIExpression()), !dbg !1995
  %1034 = getelementptr inbounds [64 x i32], [64 x i32]* %1030, i64 0, i64 %1033, !dbg !2007, !intel-tbaa !1208
  %1035 = load i32, i32* %1034, align 4, !dbg !2007, !tbaa !1783
  %1036 = add nsw i32 %1035, 1, !dbg !2018
  %1037 = ashr i32 %1036, 1, !dbg !2019
  store i32 %1037, i32* %1034, align 4, !dbg !2020, !tbaa !1783
  %1038 = getelementptr inbounds [64 x i32], [64 x i32]* %1031, i64 0, i64 %1033, !dbg !2016, !intel-tbaa !1208
  %1039 = load i32, i32* %1038, align 4, !dbg !2016, !tbaa !1783
  %1040 = add nsw i32 %1039, 1, !dbg !2021
  %1041 = ashr i32 %1040, 1, !dbg !2022
  store i32 %1041, i32* %1038, align 4, !dbg !2023, !tbaa !1783
  %1042 = add nuw nsw i64 %1033, 1, !dbg !2024
  call void @llvm.dbg.value(metadata i64 %1042, metadata !1988, metadata !DIExpression()), !dbg !1995
  %1043 = icmp eq i64 %1042, 64, !dbg !2025
  br i1 %1043, label %1044, label %1032, !dbg !2017, !llvm.loop !2026

1044:                                             ; preds = %1032
  %1045 = add nuw nsw i64 %1029, 1, !dbg !2028
  call void @llvm.dbg.value(metadata i64 %1045, metadata !1987, metadata !DIExpression()), !dbg !1995
  %1046 = icmp eq i64 %1045, 12, !dbg !2029
  br i1 %1046, label %1047, label %1028, !dbg !2015, !llvm.loop !2030

1047:                                             ; preds = %1044, %1008, %1002
  %1048 = add nuw nsw i64 %1003, 1, !dbg !2032
  call void @llvm.dbg.value(metadata i64 %1048, metadata !1043, metadata !DIExpression()), !dbg !1120
  %1049 = icmp eq i64 %1048, %1001, !dbg !1973
  br i1 %1049, label %1050, label %1002, !dbg !1976, !llvm.loop !2033

1050:                                             ; preds = %1047, %995
  %1051 = load i32, i32* %9, align 4, !dbg !2035, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %1051, metadata !1042, metadata !DIExpression()), !dbg !1120
  %1052 = sext i32 %1051 to i64, !dbg !2036
  %1053 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %1052, !dbg !2036, !intel-tbaa !1400
  %1054 = load i32, i32* %1053, align 4, !dbg !2036, !tbaa !1400
  %1055 = call zeroext i16 @_Z12compact_movei(i32 %1054), !dbg !2037
  %1056 = zext i16 %1055 to i32, !dbg !2037
  %1057 = load i32, i32* %11, align 4, !dbg !2038, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %1057, metadata !1048, metadata !DIExpression()), !dbg !1120
  %1058 = load i32, i32* %14, align 4, !dbg !2039, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %1058, metadata !1063, metadata !DIExpression()), !dbg !1120
  %1059 = load i32, i32* %15, align 4, !dbg !2040, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %1059, metadata !1065, metadata !DIExpression()), !dbg !1120
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %978, i32 %64, i32 %70, i32 %1056, i32 %1057, i32 %1058, i32 %1059, i32 %3), !dbg !2041
  br label %1119, !dbg !2042

1060:                                             ; preds = %989
  call void @llvm.dbg.value(metadata i32 %978, metadata !1034, metadata !DIExpression()), !dbg !1120
  %1061 = call zeroext i16 @_Z12compact_movei(i32 %994), !dbg !2043
  %1062 = zext i16 %1061 to i32, !dbg !2043
  call void @llvm.dbg.value(metadata i32 %1062, metadata !1052, metadata !DIExpression()), !dbg !1120
  store i32 %1062, i32* %13, align 4, !dbg !2044, !tbaa !866
  br label %1063, !dbg !2045

1063:                                             ; preds = %1060, %987
  %1064 = phi i32 [ %978, %1060 ], [ %650, %987 ]
  call void @llvm.dbg.value(metadata i32 %1064, metadata !1034, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 0, metadata !1054, metadata !DIExpression()), !dbg !1120
  %1065 = load i32, i32* %9, align 4, !dbg !2046, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %1065, metadata !1042, metadata !DIExpression()), !dbg !1120
  %1066 = sext i32 %1065 to i64, !dbg !2047
  %1067 = getelementptr inbounds [240 x i32], [240 x i32]* %7, i64 0, i64 %1066, !dbg !2047, !intel-tbaa !1400
  %1068 = load i32, i32* %1067, align 4, !dbg !2047, !tbaa !1400
  %1069 = add nsw i32 %655, -1, !dbg !2048
  %1070 = sext i32 %1069 to i64, !dbg !2049
  %1071 = getelementptr inbounds [240 x i32], [240 x i32]* %16, i64 0, i64 %1070, !dbg !2049, !intel-tbaa !1400
  store i32 %1068, i32* %1071, align 4, !dbg !2050, !tbaa !1400
  %1072 = add nsw i32 %655, 1, !dbg !2051
  call void @llvm.dbg.value(metadata i32 %1072, metadata !1062, metadata !DIExpression()), !dbg !1120
  br label %1073, !dbg !2052

1073:                                             ; preds = %1063, %985
  %1074 = phi i32 [ %1072, %1063 ], [ %655, %985 ]
  %1075 = phi i32 [ 0, %1063 ], [ %654, %985 ]
  %1076 = phi i32 [ %1064, %1063 ], [ %650, %985 ]
  call void @llvm.dbg.value(metadata i32 %1076, metadata !1034, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1075, metadata !1054, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1074, metadata !1062, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32* %9, metadata !1042, metadata !DIExpression(DW_OP_deref)), !dbg !1120
  %1077 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %9, i32* nonnull %283, i32* nonnull %249, i32 %281), !dbg !2053
  call void @llvm.dbg.value(metadata i32 %1077, metadata !1061, metadata !DIExpression()), !dbg !1120
  br label %1078, !dbg !1794

1078:                                             ; preds = %1073, %862, %850
  %1079 = phi i32 [ %1077, %1073 ], [ %863, %862 ], [ %851, %850 ]
  %1080 = phi i32 [ %1074, %1073 ], [ %655, %862 ], [ %655, %850 ]
  %1081 = phi i32 [ %1075, %1073 ], [ %654, %862 ], [ %654, %850 ]
  %1082 = phi i32 [ %975, %1073 ], [ %650, %862 ], [ %650, %850 ]
  %1083 = phi i32 [ %977, %1073 ], [ 0, %862 ], [ 0, %850 ]
  %1084 = phi i32 [ %978, %1073 ], [ %651, %862 ], [ %651, %850 ]
  %1085 = phi i32 [ %1076, %1073 ], [ %650, %862 ], [ %650, %850 ]
  call void @llvm.dbg.value(metadata i32 %1085, metadata !1034, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1084, metadata !1044, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1083, metadata !1045, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1082, metadata !1050, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1081, metadata !1054, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1080, metadata !1062, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1079, metadata !1061, metadata !DIExpression()), !dbg !1120
  %1086 = icmp eq i32 %1079, 0, !dbg !1658
  br i1 %1086, label %1087, label %649, !dbg !1659

1087:                                             ; preds = %1078, %615
  %1088 = phi i32 [ -32000, %615 ], [ %1082, %1078 ]
  %1089 = phi i32 [ 1, %615 ], [ %1083, %1078 ]
  call void @llvm.dbg.value(metadata i32 %1089, metadata !1045, metadata !DIExpression()), !dbg !1120
  %1090 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !1795, !tbaa !1298
  %1091 = icmp eq i32 %1090, 0, !dbg !1796
  call void @llvm.dbg.value(metadata i1 %1091, metadata !1119, metadata !DIExpression()), !dbg !1120
  %1092 = icmp ne i32 %1089, 0, !dbg !2054
  %1093 = and i1 %1092, %1091, !dbg !1797
  br i1 %1093, label %1094, label %1106, !dbg !1797

1094:                                             ; preds = %1087
  %1095 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0), !dbg !2055
  %1096 = icmp eq i32 %1095, 0, !dbg !2055
  %1097 = load i32, i32* %11, align 4, !dbg !2058, !tbaa !866
  %1098 = load i32, i32* %14, align 4, !dbg !2060, !tbaa !866
  %1099 = load i32, i32* %15, align 4, !dbg !2061, !tbaa !866
  br i1 %1096, label %1105, label %1100, !dbg !2062

1100:                                             ; preds = %1094
  %1101 = load i32, i32* %31, align 8, !dbg !2063, !tbaa !968
  %1102 = add nsw i32 %1101, -32000, !dbg !2065
  call void @llvm.dbg.value(metadata i32 %1097, metadata !1048, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1098, metadata !1063, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1099, metadata !1065, metadata !DIExpression()), !dbg !1120
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %1102, i32 %64, i32 %70, i32 0, i32 %1097, i32 %1098, i32 %1099, i32 %3), !dbg !2066
  %1103 = load i32, i32* %31, align 8, !dbg !2067, !tbaa !968
  %1104 = add nsw i32 %1103, -32000, !dbg !2068
  br label %1119, !dbg !2069

1105:                                             ; preds = %1094
  call void @llvm.dbg.value(metadata i32 %1097, metadata !1048, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1098, metadata !1063, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.value(metadata i32 %1099, metadata !1065, metadata !DIExpression()), !dbg !1120
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 0, i32 %64, i32 %70, i32 0, i32 %1097, i32 %1098, i32 %1099, i32 %3), !dbg !2070
  br label %1119, !dbg !2071

1106:                                             ; preds = %1087, %792
  %1107 = phi i1 [ %794, %792 ], [ %1091, %1087 ]
  %1108 = phi i32 [ %653, %792 ], [ %1088, %1087 ]
  %1109 = load i32, i32* %46, align 4, !dbg !2072, !tbaa !1151
  %1110 = icmp sgt i32 %1109, 98, !dbg !2075
  %1111 = xor i1 %1107, true, !dbg !2076
  %1112 = or i1 %1110, %1111, !dbg !2076
  %1113 = select i1 %1110, i32 0, i32 %1108, !dbg !2076
  br i1 %1112, label %1119, label %1114, !dbg !2076

1114:                                             ; preds = %1106
  %1115 = load i32, i32* %13, align 4, !dbg !2077, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %1115, metadata !1052, metadata !DIExpression()), !dbg !1120
  %1116 = load i32, i32* %11, align 4, !dbg !2080, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %1116, metadata !1048, metadata !DIExpression()), !dbg !1120
  %1117 = load i32, i32* %14, align 4, !dbg !2081, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %1117, metadata !1063, metadata !DIExpression()), !dbg !1120
  %1118 = load i32, i32* %15, align 4, !dbg !2082, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %1118, metadata !1065, metadata !DIExpression()), !dbg !1120
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %1108, i32 %64, i32 %70, i32 %1115, i32 %1116, i32 %1117, i32 %1118, i32 %3), !dbg !2083
  br label %1119, !dbg !2084

1119:                                             ; preds = %1114, %1106, %1105, %1100, %1050, %974, %498, %243, %237, %224, %211, %172, %117, %110, %103, %77, %74, %72, %67, %61, %49, %36, %34
  %1120 = phi i32 [ %35, %34 ], [ %56, %49 ], [ %73, %72 ], [ 0, %36 ], [ %59, %61 ], [ %65, %67 ], [ %75, %74 ], [ %78, %77 ], [ %70, %498 ], [ %92, %103 ], [ %111, %110 ], [ %92, %117 ], [ 0, %172 ], [ %213, %224 ], [ 0, %211 ], [ %64, %243 ], [ 0, %237 ], [ %978, %1050 ], [ %1104, %1100 ], [ 0, %1105 ], [ %1113, %1106 ], [ %1108, %1114 ], [ 0, %974 ]
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %28) #7, !dbg !2085
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %27) #7, !dbg !2085
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %26) #7, !dbg !2085
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %25) #7, !dbg !2085
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %24) #7, !dbg !2085
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %23) #7, !dbg !2085
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %22) #7, !dbg !2085
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %21) #7, !dbg !2085
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %20) #7, !dbg !2085
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %19) #7, !dbg !2085
  ret i32 %1120, !dbg !2085
}
; Function Attrs: uwtable
define internal i32 @_Z7qsearchP7state_tiiii(%struct.state_t* %0, i32 %1, i32 %2, i32 %3, i32 %4) #5 !dbg !2086 {
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca [240 x i32], align 16
  %11 = alloca [240 x i32], align 16
  call void @llvm.dbg.value(metadata %struct.state_t* %0, metadata !2090, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %1, metadata !2091, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %2, metadata !2092, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %3, metadata !2093, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %4, metadata !2094, metadata !DIExpression()), !dbg !2130
  %12 = bitcast i32* %6 to i8*, !dbg !2131
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %12) #7, !dbg !2131
  %13 = bitcast i32* %7 to i8*, !dbg !2132
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %13) #7, !dbg !2132
  %14 = bitcast i32* %8 to i8*, !dbg !2133
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %14) #7, !dbg !2133
  %15 = bitcast i32* %9 to i8*, !dbg !2133
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %15) #7, !dbg !2133
  %16 = bitcast [240 x i32]* %10 to i8*, !dbg !2134
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %16) #7, !dbg !2134
  call void @llvm.dbg.declare(metadata [240 x i32]* %10, metadata !2112, metadata !DIExpression()), !dbg !2135
  %17 = bitcast [240 x i32]* %11 to i8*, !dbg !2136
  call void @llvm.lifetime.start.p0i8(i64 960, i8* nonnull %17) #7, !dbg !2136
  call void @llvm.dbg.declare(metadata [240 x i32]* %11, metadata !2113, metadata !DIExpression()), !dbg !2137
  %18 = getelementptr %struct.state_t, %struct.state_t* %0, i64 0, i32 22, !dbg !2138, !intel-tbaa !1142
  %19 = load i64, i64* %18, align 8, !dbg !2139, !tbaa !1142
  %20 = add i64 %19, 1, !dbg !2139
  store i64 %20, i64* %18, align 8, !dbg !2139, !tbaa !1142
  %21 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 23, !dbg !2140, !intel-tbaa !2141
  %22 = load i64, i64* %21, align 8, !dbg !2142, !tbaa !2141
  %23 = add i64 %22, 1, !dbg !2142
  store i64 %23, i64* %21, align 8, !dbg !2142, !tbaa !2141
  %24 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 14, !dbg !2143, !intel-tbaa !968
  %25 = load i32, i32* %24, align 8, !dbg !2143, !tbaa !968
  %26 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 24, !dbg !2145, !intel-tbaa !2146
  %27 = load i32, i32* %26, align 8, !dbg !2145, !tbaa !2146
  %28 = icmp sgt i32 %25, %27, !dbg !2147
  br i1 %28, label %29, label %30, !dbg !2148

29:                                               ; preds = %5
  store i32 %25, i32* %26, align 8, !dbg !2149, !tbaa !2146
  br label %30, !dbg !2151

30:                                               ; preds = %29, %5
  %31 = tail call fastcc i32 @_ZL17search_time_checkP7state_t(i64 %20), !dbg !2152
  %32 = icmp eq i32 %31, 0, !dbg !2152
  br i1 %32, label %33, label %420, !dbg !2154

33:                                               ; preds = %30
  %34 = tail call i32 @_Z7is_drawP11gamestate_tP7state_t(%struct.gamestate_t* nonnull @gamestate, %struct.state_t* nonnull %0), !dbg !2155
  %35 = icmp eq i32 %34, 0, !dbg !2155
  br i1 %35, label %36, label %40, !dbg !2157

36:                                               ; preds = %33
  %37 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 15, !dbg !2158, !intel-tbaa !1151
  %38 = load i32, i32* %37, align 4, !dbg !2158, !tbaa !1151
  %39 = icmp sgt i32 %38, 99, !dbg !2159
  br i1 %39, label %40, label %48, !dbg !2160

40:                                               ; preds = %36, %33
  %41 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 3), align 4, !dbg !2161, !tbaa !1156
  %42 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !dbg !2163, !intel-tbaa !885
  %43 = load i32, i32* %42, align 4, !dbg !2163, !tbaa !885
  %44 = icmp eq i32 %41, %43, !dbg !2164
  %45 = load i32, i32* @contempt, align 4, !dbg !2165, !tbaa !866
  %46 = sub nsw i32 0, %45, !dbg !2166
  %47 = select i1 %44, i32 %45, i32 %46, !dbg !2166
  br label %420, !dbg !2167

48:                                               ; preds = %36
  call void @llvm.dbg.value(metadata i32* %7, metadata !2102, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  call void @llvm.dbg.value(metadata i32* %8, metadata !2105, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  call void @llvm.dbg.value(metadata i32* %9, metadata !2106, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  call void @llvm.dbg.value(metadata i32* %9, metadata !2106, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  call void @llvm.dbg.value(metadata i32* %9, metadata !2106, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  call void @llvm.dbg.value(metadata i32* %9, metadata !2106, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  %49 = call i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(%struct.state_t* nonnull %0, i32* nonnull %8, i32 %1, i32 %2, i32* nonnull %7, i32* nonnull %9, i32* nonnull %9, i32* nonnull %9, i32* nonnull %9, i32 0), !dbg !2168
  switch i32 %49, label %59 [
    i32 3, label %50
    i32 1, label %52
    i32 2, label %55
    i32 4, label %58
  ], !dbg !2169

50:                                               ; preds = %48
  %51 = load i32, i32* %8, align 4, !dbg !2170, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %51, metadata !2105, metadata !DIExpression()), !dbg !2130
  br label %420, !dbg !2172

52:                                               ; preds = %48
  %53 = load i32, i32* %8, align 4, !dbg !2173, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %53, metadata !2105, metadata !DIExpression()), !dbg !2130
  %54 = icmp sgt i32 %53, %1, !dbg !2175
  br i1 %54, label %59, label %420, !dbg !2176

55:                                               ; preds = %48
  %56 = load i32, i32* %8, align 4, !dbg !2177, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %56, metadata !2105, metadata !DIExpression()), !dbg !2130
  %57 = icmp slt i32 %56, %2, !dbg !2179
  br i1 %57, label %59, label %420, !dbg !2180

58:                                               ; preds = %48
  call void @llvm.dbg.value(metadata i32 65535, metadata !2102, metadata !DIExpression()), !dbg !2130
  store i32 65535, i32* %7, align 4, !dbg !2181, !tbaa !866
  br label %59, !dbg !2182

59:                                               ; preds = %58, %55, %52, %48
  %60 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 5), align 4, !dbg !2183, !tbaa !1653
  %61 = shl nsw i32 %60, 1, !dbg !2185
  %62 = icmp slt i32 %61, %4, !dbg !2186
  br i1 %62, label %66, label %63, !dbg !2187

63:                                               ; preds = %59
  %64 = load i32, i32* %24, align 8, !dbg !2188, !tbaa !968
  %65 = icmp sgt i32 %64, 60, !dbg !2189
  br i1 %65, label %66, label %68, !dbg !2190

66:                                               ; preds = %63, %59
  %67 = call i32 @_Z4evalP7state_tiii(%struct.state_t* nonnull %0, i32 %1, i32 %2, i32 0), !dbg !2191
  br label %420, !dbg !2193

68:                                               ; preds = %63
  %69 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 25, !dbg !2194, !intel-tbaa !1205
  %70 = sext i32 %64 to i64, !dbg !2195
  %71 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %70, !dbg !2195, !intel-tbaa !1208
  %72 = load i32, i32* %71, align 4, !dbg !2195, !tbaa !1209
  call void @llvm.dbg.value(metadata i32 %72, metadata !2107, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %1, metadata !2104, metadata !DIExpression()), !dbg !2130
  %73 = call i32 @_Z13retrieve_evalP7state_t(%struct.state_t* nonnull %0), !dbg !2196
  call void @llvm.dbg.value(metadata i32 %73, metadata !2098, metadata !DIExpression()), !dbg !2130
  %74 = add nsw i32 %73, 50, !dbg !2197
  call void @llvm.dbg.value(metadata i32 %74, metadata !2111, metadata !DIExpression()), !dbg !2130
  %75 = icmp ne i32 %72, 0, !dbg !2198
  br i1 %75, label %150, label %76, !dbg !2199

76:                                               ; preds = %68
  %77 = icmp slt i32 %73, %2, !dbg !2200
  br i1 %77, label %80, label %78, !dbg !2201

78:                                               ; preds = %76
  %79 = load i32, i32* %7, align 4, !dbg !2202, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %79, metadata !2102, metadata !DIExpression()), !dbg !2130
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %73, i32 %1, i32 %2, i32 %79, i32 0, i32 0, i32 0, i32 0), !dbg !2204
  br label %420, !dbg !2205

80:                                               ; preds = %76
  %81 = icmp sgt i32 %73, %1, !dbg !2206
  br i1 %81, label %147, label %82, !dbg !2207

82:                                               ; preds = %80
  %83 = add nsw i32 %73, 985, !dbg !2208
  %84 = icmp sgt i32 %83, %1, !dbg !2209
  br i1 %84, label %87, label %85, !dbg !2210

85:                                               ; preds = %82
  %86 = load i32, i32* %7, align 4, !dbg !2211, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %86, metadata !2102, metadata !DIExpression()), !dbg !2130
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %83, i32 %1, i32 %2, i32 %86, i32 0, i32 0, i32 0, i32 0), !dbg !2213
  br label %420, !dbg !2214

87:                                               ; preds = %82
  %88 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 8, !dbg !2215, !intel-tbaa !1254
  %89 = getelementptr inbounds [13 x i32], [13 x i32]* %88, i64 0, i64 0, !dbg !2216
  call void @llvm.dbg.value(metadata i32* %89, metadata !2114, metadata !DIExpression()), !dbg !2217
  %90 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !dbg !2218, !intel-tbaa !885
  %91 = load i32, i32* %90, align 4, !dbg !2218, !tbaa !885
  %92 = icmp eq i32 %91, 0, !dbg !2220
  br i1 %92, label %120, label %93, !dbg !2221

93:                                               ; preds = %87
  %94 = getelementptr inbounds i32, i32* %89, i64 10, !dbg !2222
  %95 = load i32, i32* %94, align 4, !dbg !2222, !tbaa !866
  %96 = icmp eq i32 %95, 0, !dbg !2222
  br i1 %96, label %97, label %147, !dbg !2225

97:                                               ; preds = %93
  %98 = getelementptr inbounds i32, i32* %89, i64 8, !dbg !2226
  %99 = load i32, i32* %98, align 4, !dbg !2226, !tbaa !866
  %100 = icmp eq i32 %99, 0, !dbg !2226
  br i1 %100, label %101, label %115, !dbg !2229

101:                                              ; preds = %97
  %102 = getelementptr inbounds i32, i32* %89, i64 12, !dbg !2230
  %103 = load i32, i32* %102, align 4, !dbg !2230, !tbaa !866
  %104 = icmp eq i32 %103, 0, !dbg !2230
  br i1 %104, label %105, label %112, !dbg !2233

105:                                              ; preds = %101
  %106 = getelementptr inbounds i32, i32* %89, i64 4, !dbg !2234
  %107 = load i32, i32* %106, align 4, !dbg !2234, !tbaa !866
  %108 = icmp eq i32 %107, 0, !dbg !2234
  br i1 %108, label %109, label %112, !dbg !2235

109:                                              ; preds = %105
  %110 = add nsw i32 %73, 135, !dbg !2236
  %111 = icmp sgt i32 %110, %1, !dbg !2239
  br i1 %111, label %147, label %420, !dbg !2240

112:                                              ; preds = %105, %101
  %113 = add nsw i32 %73, 380, !dbg !2241
  %114 = icmp sgt i32 %113, %1, !dbg !2244
  br i1 %114, label %147, label %420, !dbg !2245

115:                                              ; preds = %97
  %116 = add nsw i32 %73, 540, !dbg !2246
  %117 = icmp sgt i32 %116, %1, !dbg !2249
  br i1 %117, label %147, label %118, !dbg !2250

118:                                              ; preds = %115
  %119 = load i32, i32* %7, align 4, !dbg !2251, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %119, metadata !2102, metadata !DIExpression()), !dbg !2130
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %116, i32 %1, i32 %2, i32 %119, i32 0, i32 0, i32 0, i32 0), !dbg !2253
  br label %420, !dbg !2254

120:                                              ; preds = %87
  %121 = getelementptr inbounds i32, i32* %89, i64 9, !dbg !2255
  %122 = load i32, i32* %121, align 4, !dbg !2255, !tbaa !866
  %123 = icmp eq i32 %122, 0, !dbg !2255
  br i1 %123, label %124, label %147, !dbg !2258

124:                                              ; preds = %120
  %125 = getelementptr inbounds i32, i32* %89, i64 7, !dbg !2259
  %126 = load i32, i32* %125, align 4, !dbg !2259, !tbaa !866
  %127 = icmp eq i32 %126, 0, !dbg !2259
  br i1 %127, label %128, label %142, !dbg !2262

128:                                              ; preds = %124
  %129 = getelementptr inbounds i32, i32* %89, i64 11, !dbg !2263
  %130 = load i32, i32* %129, align 4, !dbg !2263, !tbaa !866
  %131 = icmp eq i32 %130, 0, !dbg !2263
  br i1 %131, label %132, label %139, !dbg !2266

132:                                              ; preds = %128
  %133 = getelementptr inbounds i32, i32* %89, i64 3, !dbg !2267
  %134 = load i32, i32* %133, align 4, !dbg !2267, !tbaa !866
  %135 = icmp eq i32 %134, 0, !dbg !2267
  br i1 %135, label %136, label %139, !dbg !2268

136:                                              ; preds = %132
  %137 = add nsw i32 %73, 135, !dbg !2269
  %138 = icmp sgt i32 %137, %1, !dbg !2272
  br i1 %138, label %147, label %420, !dbg !2273

139:                                              ; preds = %132, %128
  %140 = add nsw i32 %73, 380, !dbg !2274
  %141 = icmp sgt i32 %140, %1, !dbg !2277
  br i1 %141, label %147, label %420, !dbg !2278

142:                                              ; preds = %124
  %143 = add nsw i32 %73, 540, !dbg !2279
  %144 = icmp sgt i32 %143, %1, !dbg !2282
  br i1 %144, label %147, label %145, !dbg !2283

145:                                              ; preds = %142
  %146 = load i32, i32* %7, align 4, !dbg !2284, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %146, metadata !2102, metadata !DIExpression()), !dbg !2130
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* nonnull %0, i32 %143, i32 %1, i32 %2, i32 %146, i32 0, i32 0, i32 0, i32 0), !dbg !2286
  br label %420, !dbg !2287

147:                                              ; preds = %142, %139, %136, %120, %115, %112, %109, %93, %80
  %148 = phi i32 [ %73, %80 ], [ %1, %120 ], [ %1, %136 ], [ %1, %139 ], [ %1, %142 ], [ %1, %93 ], [ %1, %109 ], [ %1, %112 ], [ %1, %115 ]
  call void @llvm.dbg.value(metadata i32 0, metadata !2095, metadata !DIExpression()), !dbg !2130
  %149 = sub nsw i32 %148, %74, !dbg !2288
  call void @llvm.dbg.value(metadata i32 %149, metadata !2101, metadata !DIExpression()), !dbg !2130
  br label %150, !dbg !2291

150:                                              ; preds = %147, %68
  %151 = phi i32 [ %148, %147 ], [ %1, %68 ]
  %152 = phi i32 [ %149, %147 ], [ 0, %68 ]
  call void @llvm.dbg.value(metadata i32 %152, metadata !2101, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 0, metadata !2110, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 1, metadata !2109, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 -32000, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 1, metadata !2100, metadata !DIExpression()), !dbg !2130
  %153 = load i32, i32* %7, align 4, !dbg !2292, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %153, metadata !2102, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %153, metadata !2103, metadata !DIExpression()), !dbg !2130
  %154 = icmp sgt i32 %3, -6, !dbg !2293
  %155 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 0, !dbg !2295
  br i1 %154, label %156, label %161, !dbg !2296

156:                                              ; preds = %150
  br i1 %75, label %157, label %159, !dbg !2297

157:                                              ; preds = %156
  %158 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %155, i32 %72), !dbg !2299
  call void @llvm.dbg.value(metadata i32 %158, metadata !2095, metadata !DIExpression()), !dbg !2130
  br label %166, !dbg !2302

159:                                              ; preds = %156
  %160 = call i32 @_Z12gen_capturesP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %155), !dbg !2303
  call void @llvm.dbg.value(metadata i32 %160, metadata !2095, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 1, metadata !2110, metadata !DIExpression()), !dbg !2130
  br label %166

161:                                              ; preds = %150
  br i1 %75, label %164, label %162, !dbg !2305

162:                                              ; preds = %161
  %163 = call i32 @_Z12gen_capturesP7state_tPi(%struct.state_t* nonnull %0, i32* nonnull %155), !dbg !2307
  call void @llvm.dbg.value(metadata i32 %163, metadata !2095, metadata !DIExpression()), !dbg !2130
  br label %166, !dbg !2310

164:                                              ; preds = %161
  %165 = call i32 @_Z12gen_evasionsP7state_tPii(%struct.state_t* nonnull %0, i32* nonnull %155, i32 %72), !dbg !2311
  call void @llvm.dbg.value(metadata i32 %165, metadata !2095, metadata !DIExpression()), !dbg !2130
  br label %166

166:                                              ; preds = %164, %162, %159, %157
  %167 = phi i1 [ false, %157 ], [ true, %159 ], [ false, %164 ], [ false, %162 ]
  %168 = phi i32 [ %158, %157 ], [ %160, %159 ], [ %165, %164 ], [ %163, %162 ]
  call void @llvm.dbg.value(metadata i32 %168, metadata !2095, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 undef, metadata !2110, metadata !DIExpression()), !dbg !2130
  %169 = getelementptr inbounds [240 x i32], [240 x i32]* %11, i64 0, i64 0, !dbg !2313
  %170 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 1, !dbg !2314
  %171 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 0, !dbg !2321
  %172 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !dbg !2322
  %173 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16, !dbg !2326
  %174 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 36, !dbg !2327
  %175 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 19, !dbg !2328
  %176 = sub nsw i32 0, %2, !dbg !2329
  %177 = add nsw i32 %4, 1, !dbg !2330
  %178 = icmp sgt i32 %3, -1, !dbg !2331
  br label %179, !dbg !2336

179:                                              ; preds = %405, %166
  %180 = phi i32 [ %153, %166 ], [ %399, %405 ]
  %181 = phi i32 [ 1, %166 ], [ %400, %405 ]
  %182 = phi i32 [ -32000, %166 ], [ %401, %405 ]
  %183 = phi i32 [ 1, %166 ], [ %406, %405 ]
  %184 = phi i32 [ %168, %166 ], [ %194, %405 ]
  %185 = phi i32 [ %151, %166 ], [ %402, %405 ]
  call void @llvm.dbg.value(metadata i32 %185, metadata !2091, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %184, metadata !2095, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %183, metadata !2109, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %182, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %181, metadata !2100, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %180, metadata !2103, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.label(metadata !2128), !dbg !2337
  %186 = icmp eq i32 %183, 2, !dbg !2338
  br i1 %186, label %187, label %189, !dbg !2340

187:                                              ; preds = %179
  %188 = call i32 @_Z15gen_good_checksP7state_tPi(%struct.state_t* %0, i32* nonnull %155), !dbg !2341
  call void @llvm.dbg.value(metadata i32 %188, metadata !2095, metadata !DIExpression()), !dbg !2130
  br label %193, !dbg !2343

189:                                              ; preds = %179
  %190 = icmp eq i32 %183, 3, !dbg !2344
  br i1 %190, label %191, label %193, !dbg !2346

191:                                              ; preds = %189
  %192 = call i32 @_Z9gen_quietP7state_tPi(%struct.state_t* %0, i32* nonnull %155), !dbg !2347
  call void @llvm.dbg.value(metadata i32 %192, metadata !2095, metadata !DIExpression()), !dbg !2130
  br label %193, !dbg !2349

193:                                              ; preds = %191, %189, %187
  %194 = phi i32 [ %188, %187 ], [ %192, %191 ], [ %184, %189 ]
  call void @llvm.dbg.value(metadata i32 %194, metadata !2095, metadata !DIExpression()), !dbg !2130
  %195 = load i32, i32* %7, align 4, !dbg !2350, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %195, metadata !2102, metadata !DIExpression()), !dbg !2130
  call fastcc void @_ZL16fast_order_movesP7state_tPiS1_ij(%struct.state_t* %0, i32* nonnull %155, i32* nonnull %169, i32 %194, i32 %195), !dbg !2351
  call void @llvm.dbg.value(metadata i32 -1, metadata !2096, metadata !DIExpression()), !dbg !2130
  store i32 -1, i32* %6, align 4, !dbg !2352, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %185, metadata !2091, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %182, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %181, metadata !2100, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %180, metadata !2103, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32* %6, metadata !2096, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  %196 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %169, i32* nonnull %155, i32 %194), !dbg !2353
  %197 = icmp eq i32 %196, 0, !dbg !2353
  br i1 %197, label %398, label %198, !dbg !2354

198:                                              ; preds = %193
  %199 = icmp eq i32 %183, 1, !dbg !2355
  %200 = icmp eq i32 %183, 3, !dbg !2357
  %201 = or i32 %183, 1, !dbg !2359
  %202 = icmp eq i32 %201, 3, !dbg !2359
  br label %203, !dbg !2354

203:                                              ; preds = %393, %198
  %204 = phi i32 [ %185, %198 ], [ %395, %393 ]
  %205 = phi i32 [ %182, %198 ], [ %379, %393 ]
  %206 = phi i32 [ %181, %198 ], [ %377, %393 ]
  %207 = phi i32 [ %180, %198 ], [ %394, %393 ]
  call void @llvm.dbg.value(metadata i32 %204, metadata !2091, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %205, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %206, metadata !2100, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %207, metadata !2103, metadata !DIExpression()), !dbg !2130
  br i1 %75, label %208, label %210, !dbg !2354

208:                                              ; preds = %203
  %209 = load i32, i32* %6, align 4, !dbg !2360, !tbaa !866
  br label %336, !dbg !2354

210:                                              ; preds = %203
  br i1 %199, label %211, label %251, !dbg !2354

211:                                              ; preds = %248, %210
  %212 = load i32, i32* %6, align 4, !dbg !2361, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %212, metadata !2096, metadata !DIExpression()), !dbg !2130
  %213 = sext i32 %212 to i64, !dbg !2361
  %214 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %213, !dbg !2361
  %215 = load i32, i32* %214, align 4, !dbg !2361, !tbaa !1400
  %216 = lshr i32 %215, 19, !dbg !2361
  %217 = and i32 %216, 15, !dbg !2361
  %218 = zext i32 %217 to i64, !dbg !2364
  %219 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %218, !dbg !2364
  %220 = load i32, i32* %219, align 4, !dbg !2364, !tbaa !1440
  %221 = icmp slt i32 %220, 0, !dbg !2365
  %222 = sub nsw i32 0, %220, !dbg !2365
  %223 = select i1 %221, i32 %222, i32 %220, !dbg !2365
  %224 = icmp sle i32 %223, %152, !dbg !2366
  %225 = and i32 %215, 61440, !dbg !2367
  %226 = icmp eq i32 %225, 0, !dbg !2367
  %227 = and i1 %226, %224, !dbg !2368
  br i1 %227, label %398, label %228, !dbg !2368

228:                                              ; preds = %211
  call void @llvm.dbg.value(metadata i32 %212, metadata !2096, metadata !DIExpression()), !dbg !2130
  %229 = lshr i32 %215, 6, !dbg !2369
  %230 = and i32 %229, 63, !dbg !2369
  %231 = zext i32 %230 to i64, !dbg !2370
  %232 = getelementptr inbounds [64 x i32], [64 x i32]* %170, i64 0, i64 %231, !dbg !2370, !intel-tbaa !1208
  %233 = load i32, i32* %232, align 4, !dbg !2370, !tbaa !1438
  %234 = sext i32 %233 to i64, !dbg !2371
  %235 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %234, !dbg !2371, !intel-tbaa !1440
  %236 = load i32, i32* %235, align 4, !dbg !2371, !tbaa !1440
  %237 = icmp slt i32 %236, 0, !dbg !2372
  %238 = sub nsw i32 0, %236, !dbg !2372
  %239 = select i1 %237, i32 %238, i32 %236, !dbg !2372
  %240 = icmp slt i32 %223, %239, !dbg !2373
  br i1 %240, label %241, label %336, !dbg !2374

241:                                              ; preds = %228
  %242 = load i32, i32* %172, align 4, !dbg !2322, !tbaa !885
  %243 = icmp eq i32 %242, 0, !dbg !2322
  %244 = zext i1 %243 to i32, !dbg !2322
  call void @llvm.dbg.value(metadata i32 %212, metadata !2096, metadata !DIExpression()), !dbg !2130
  %245 = and i32 %215, 63, !dbg !2375
  %246 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* nonnull %0, i32 %244, i32 %230, i32 %245, i32 0), !dbg !2376
  %247 = icmp slt i32 %246, -50, !dbg !2377
  br i1 %247, label %248, label %336, !dbg !2378

248:                                              ; preds = %241
  call void @llvm.dbg.value(metadata i32 %204, metadata !2091, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %205, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %206, metadata !2100, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %207, metadata !2103, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32* %6, metadata !2096, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  %249 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %169, i32* nonnull %155, i32 %194), !dbg !2353
  %250 = icmp eq i32 %249, 0, !dbg !2353
  br i1 %250, label %398, label %211, !dbg !2354, !llvm.loop !2379

251:                                              ; preds = %268, %210
  br i1 %202, label %252, label %271, !dbg !2359

252:                                              ; preds = %251
  %253 = load i32, i32* %6, align 4, !dbg !2381, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %253, metadata !2096, metadata !DIExpression()), !dbg !2130
  %254 = sext i32 %253 to i64, !dbg !2381
  %255 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %254, !dbg !2381, !intel-tbaa !1400
  %256 = load i32, i32* %255, align 4, !dbg !2381, !tbaa !1400
  %257 = lshr i32 %256, 19, !dbg !2381
  %258 = and i32 %257, 15, !dbg !2381
  %259 = icmp eq i32 %258, 13, !dbg !2382
  br i1 %259, label %271, label %260, !dbg !2383

260:                                              ; preds = %252
  %261 = zext i32 %258 to i64, !dbg !2384
  %262 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %261, !dbg !2384, !intel-tbaa !1440
  %263 = load i32, i32* %262, align 4, !dbg !2384, !tbaa !1440
  %264 = icmp slt i32 %263, 0, !dbg !2385
  %265 = sub nsw i32 0, %263, !dbg !2385
  %266 = select i1 %264, i32 %265, i32 %263, !dbg !2385
  %267 = icmp sgt i32 %266, %152, !dbg !2386
  br i1 %267, label %268, label %271, !dbg !2387

268:                                              ; preds = %325, %273, %260
  call void @llvm.dbg.value(metadata i32 %204, metadata !2091, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %205, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %206, metadata !2100, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %207, metadata !2103, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32* %6, metadata !2096, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  %269 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %169, i32* nonnull %155, i32 %194), !dbg !2353
  %270 = icmp eq i32 %269, 0, !dbg !2353
  br i1 %270, label %398, label %251, !dbg !2354, !llvm.loop !2379

271:                                              ; preds = %260, %252, %251
  %272 = phi i1 [ true, %252 ], [ false, %251 ], [ true, %260 ]
  br i1 %200, label %273, label %299, !dbg !2388

273:                                              ; preds = %271
  %274 = load i32, i32* %6, align 4, !dbg !2389, !tbaa !866
  call void @llvm.dbg.value(metadata i32 %274, metadata !2096, metadata !DIExpression()), !dbg !2130
  %275 = sext i32 %274 to i64, !dbg !2390
  %276 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %275, !dbg !2390, !intel-tbaa !1400
  %277 = load i32, i32* %276, align 4, !dbg !2390, !tbaa !1400
  call void @llvm.dbg.value(metadata %struct.state_t* %0, metadata !1684, metadata !DIExpression()), !dbg !2391
  call void @llvm.dbg.value(metadata i32 %277, metadata !1685, metadata !DIExpression()), !dbg !2391
  call void @llvm.dbg.value(metadata i32 1, metadata !1686, metadata !DIExpression()), !dbg !2391
  %278 = lshr i32 %277, 6, !dbg !2392
  %279 = and i32 %278, 63, !dbg !2392
  %280 = zext i32 %279 to i64, !dbg !2393
  %281 = getelementptr inbounds [64 x i32], [64 x i32]* %170, i64 0, i64 %280, !dbg !2393, !intel-tbaa !1208
  %282 = load i32, i32* %281, align 4, !dbg !2393, !tbaa !1438
  %283 = add nsw i32 %282, -1, !dbg !2394
  call void @llvm.dbg.value(metadata i32 %283, metadata !1687, metadata !DIExpression()), !dbg !2391
  %284 = and i32 %277, 63, !dbg !2395
  call void @llvm.dbg.value(metadata i32 %284, metadata !1688, metadata !DIExpression()), !dbg !2391
  %285 = load i32, i32* %171, align 8, !dbg !2321, !tbaa !1777
  %286 = sext i32 %285 to i64, !dbg !2396
  %287 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_hit, i64 0, i64 %286, !dbg !2396, !intel-tbaa !1779
  %288 = sext i32 %283 to i64, !dbg !2396
  %289 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %287, i64 0, i64 %288, !dbg !2396, !intel-tbaa !1782
  %290 = zext i32 %284 to i64, !dbg !2396
  %291 = getelementptr inbounds [64 x i32], [64 x i32]* %289, i64 0, i64 %290, !dbg !2396, !intel-tbaa !1208
  %292 = load i32, i32* %291, align 4, !dbg !2396, !tbaa !1783
  call void @llvm.dbg.value(metadata i32 %292, metadata !1690, metadata !DIExpression()), !dbg !2391
  %293 = getelementptr inbounds [8 x [12 x [64 x i32]]], [8 x [12 x [64 x i32]]]* @history_tot, i64 0, i64 %286, !dbg !2397, !intel-tbaa !1779
  %294 = getelementptr inbounds [12 x [64 x i32]], [12 x [64 x i32]]* %293, i64 0, i64 %288, !dbg !2397, !intel-tbaa !1782
  %295 = getelementptr inbounds [64 x i32], [64 x i32]* %294, i64 0, i64 %290, !dbg !2397, !intel-tbaa !1208
  %296 = load i32, i32* %295, align 4, !dbg !2397, !tbaa !1783
  %297 = sub nsw i32 %296, %292, !dbg !2398
  call void @llvm.dbg.value(metadata i32 %297, metadata !1689, metadata !DIExpression()), !dbg !2391
  %298 = icmp slt i32 %292, %297, !dbg !2399
  br i1 %298, label %268, label %299, !dbg !2400

299:                                              ; preds = %273, %271
  %300 = load i32, i32* %6, align 4, !dbg !2401, !tbaa !866
  %301 = sext i32 %300 to i64, !dbg !2401
  br i1 %272, label %325, label %302, !dbg !2402

302:                                              ; preds = %299
  call void @llvm.dbg.value(metadata i32 %300, metadata !2096, metadata !DIExpression()), !dbg !2130
  %303 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %301, !dbg !2403, !intel-tbaa !1400
  %304 = load i32, i32* %303, align 4, !dbg !2403, !tbaa !1400
  %305 = lshr i32 %304, 19, !dbg !2403
  %306 = and i32 %305, 15, !dbg !2403
  %307 = zext i32 %306 to i64, !dbg !2404
  %308 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %307, !dbg !2404, !intel-tbaa !1440
  %309 = load i32, i32* %308, align 4, !dbg !2404, !tbaa !1440
  %310 = icmp slt i32 %309, 0, !dbg !2405
  %311 = sub nsw i32 0, %309, !dbg !2405
  %312 = select i1 %310, i32 %311, i32 %309, !dbg !2405
  %313 = lshr i32 %304, 6, !dbg !2369
  %314 = and i32 %313, 63, !dbg !2369
  %315 = zext i32 %314 to i64, !dbg !2370
  %316 = getelementptr inbounds [64 x i32], [64 x i32]* %170, i64 0, i64 %315, !dbg !2370, !intel-tbaa !1208
  %317 = load i32, i32* %316, align 4, !dbg !2370, !tbaa !1438
  %318 = sext i32 %317 to i64, !dbg !2371
  %319 = getelementptr inbounds [14 x i32], [14 x i32]* @material, i64 0, i64 %318, !dbg !2371, !intel-tbaa !1440
  %320 = load i32, i32* %319, align 4, !dbg !2371, !tbaa !1440
  %321 = icmp slt i32 %320, 0, !dbg !2372
  %322 = sub nsw i32 0, %320, !dbg !2372
  %323 = select i1 %321, i32 %322, i32 %320, !dbg !2372
  %324 = icmp slt i32 %312, %323, !dbg !2373
  br i1 %324, label %325, label %336, !dbg !2374

325:                                              ; preds = %302, %299
  %326 = load i32, i32* %172, align 4, !dbg !2322, !tbaa !885
  %327 = icmp eq i32 %326, 0, !dbg !2322
  %328 = zext i1 %327 to i32, !dbg !2322
  call void @llvm.dbg.value(metadata i32 %300, metadata !2096, metadata !DIExpression()), !dbg !2130
  %329 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %301, !dbg !2406, !intel-tbaa !1400
  %330 = load i32, i32* %329, align 4, !dbg !2406, !tbaa !1400
  %331 = lshr i32 %330, 6, !dbg !2406
  %332 = and i32 %331, 63, !dbg !2406
  %333 = and i32 %330, 63, !dbg !2375
  %334 = call i32 @_Z3seeP7state_tiiii(%struct.state_t* %0, i32 %328, i32 %332, i32 %333, i32 0), !dbg !2376
  %335 = icmp slt i32 %334, -50, !dbg !2377
  br i1 %335, label %268, label %336, !dbg !2378

336:                                              ; preds = %325, %302, %241, %228, %208
  %337 = phi i32 [ %209, %208 ], [ %212, %228 ], [ %212, %241 ], [ %300, %302 ], [ %300, %325 ], !dbg !2360
  call void @llvm.dbg.value(metadata i32 0, metadata !2099, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %337, metadata !2096, metadata !DIExpression()), !dbg !2130
  %338 = sext i32 %337 to i64, !dbg !2407
  %339 = getelementptr inbounds [240 x i32], [240 x i32]* %10, i64 0, i64 %338, !dbg !2407, !intel-tbaa !1400
  %340 = load i32, i32* %339, align 4, !dbg !2407, !tbaa !1400
  call void @_Z4makeP7state_ti(%struct.state_t* %0, i32 %340), !dbg !2408
  call void @llvm.dbg.value(metadata i32 %337, metadata !2096, metadata !DIExpression()), !dbg !2130
  %341 = load i32, i32* %339, align 4, !dbg !2409, !tbaa !1400
  %342 = call i32 @_Z11check_legalP7state_ti(%struct.state_t* %0, i32 %341), !dbg !2410
  %343 = icmp eq i32 %342, 0, !dbg !2410
  br i1 %343, label %376, label %344, !dbg !2411

344:                                              ; preds = %336
  %345 = load i64, i64* %173, align 8, !dbg !2326, !tbaa !888
  %346 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 15), align 4, !dbg !2412, !tbaa !1513
  %347 = load i32, i32* %24, align 8, !dbg !2413, !tbaa !968
  %348 = add i32 %347, -1, !dbg !2414
  %349 = add i32 %348, %346, !dbg !2415
  %350 = sext i32 %349 to i64, !dbg !2416
  %351 = getelementptr inbounds [1000 x i64], [1000 x i64]* %174, i64 0, i64 %350, !dbg !2416, !intel-tbaa !1518
  store i64 %345, i64* %351, align 8, !dbg !2417, !tbaa !1520
  call void @llvm.dbg.value(metadata i32 %337, metadata !2096, metadata !DIExpression()), !dbg !2130
  %352 = load i32, i32* %339, align 4, !dbg !2418, !tbaa !1400
  %353 = sext i32 %348 to i64, !dbg !2419
  %354 = getelementptr inbounds [64 x i32], [64 x i32]* %175, i64 0, i64 %353, !dbg !2419, !intel-tbaa !1208
  store i32 %352, i32* %354, align 4, !dbg !2420, !tbaa !1315
  %355 = call i32 @_Z8in_checkP7state_t(%struct.state_t* %0), !dbg !2421
  call void @llvm.dbg.value(metadata i32 %355, metadata !2108, metadata !DIExpression()), !dbg !2130
  %356 = load i32, i32* %24, align 8, !dbg !2422, !tbaa !968
  %357 = sext i32 %356 to i64, !dbg !2423
  %358 = getelementptr inbounds [64 x i32], [64 x i32]* %69, i64 0, i64 %357, !dbg !2423, !intel-tbaa !1208
  store i32 %355, i32* %358, align 4, !dbg !2424, !tbaa !1209
  %359 = sub nsw i32 0, %204, !dbg !2425
  %360 = sub i32 60, %204, !dbg !2426
  %361 = icmp ne i32 %355, 0, !dbg !2427
  %362 = zext i1 %361 to i32, !dbg !2428
  %363 = call i32 @_Z4evalP7state_tiii(%struct.state_t* %0, i32 %176, i32 %360, i32 %362), !dbg !2429
  call void @llvm.dbg.value(metadata i32 undef, metadata !2121, metadata !DIExpression()), !dbg !2430
  br i1 %200, label %364, label %367, !dbg !2431

364:                                              ; preds = %344
  %365 = sub nsw i32 0, %363, !dbg !2432
  call void @llvm.dbg.value(metadata i32 %365, metadata !2121, metadata !DIExpression()), !dbg !2430
  %366 = icmp slt i32 %204, %365, !dbg !2433
  br i1 %366, label %372, label %376, !dbg !2434

367:                                              ; preds = %344
  %368 = or i32 %355, %72, !dbg !2435
  %369 = icmp eq i32 %368, 0, !dbg !2435
  %370 = select i1 %369, i32 -8, i32 -1, !dbg !2435
  %371 = add nsw i32 %370, %3, !dbg !2435
  br label %372, !dbg !2435

372:                                              ; preds = %367, %364
  %373 = phi i32 [ %3, %364 ], [ %371, %367 ]
  call void @llvm.dbg.value(metadata i32 %373, metadata !2125, metadata !DIExpression()), !dbg !2438
  %374 = call i32 @_Z7qsearchP7state_tiiii(%struct.state_t* nonnull %0, i32 %176, i32 %359, i32 %373, i32 %177), !dbg !2439
  %375 = sub nsw i32 0, %374, !dbg !2440
  call void @llvm.dbg.value(metadata i32 %375, metadata !2097, metadata !DIExpression()), !dbg !2130
  br label %376, !dbg !2441

376:                                              ; preds = %372, %364, %336
  %377 = phi i32 [ %206, %336 ], [ 0, %372 ], [ 0, %364 ]
  %378 = phi i32 [ 0, %336 ], [ 1, %372 ], [ 1, %364 ]
  %379 = phi i32 [ %205, %336 ], [ %375, %372 ], [ %205, %364 ]
  call void @llvm.dbg.value(metadata i32 %379, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %378, metadata !2099, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %377, metadata !2100, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %337, metadata !2096, metadata !DIExpression()), !dbg !2130
  %380 = load i32, i32* %339, align 4, !dbg !2442, !tbaa !1400
  call void @_Z6unmakeP7state_ti(%struct.state_t* %0, i32 %380), !dbg !2443
  %381 = load i32, i32* getelementptr inbounds (%struct.gamestate_t, %struct.gamestate_t* @gamestate, i64 0, i32 25), align 8, !dbg !2444, !tbaa !1298
  %382 = icmp eq i32 %381, 0, !dbg !2446
  br i1 %382, label %383, label %420, !dbg !2447

383:                                              ; preds = %376
  %384 = icmp sgt i32 %379, %204, !dbg !2448
  %385 = icmp ne i32 %378, 0, !dbg !2450
  %386 = and i1 %385, %384, !dbg !2451
  br i1 %386, label %387, label %393, !dbg !2451

387:                                              ; preds = %383
  call void @llvm.dbg.value(metadata i32 %337, metadata !2096, metadata !DIExpression()), !dbg !2130
  %388 = load i32, i32* %339, align 4, !dbg !2452, !tbaa !1400
  %389 = call zeroext i16 @_Z12compact_movei(i32 %388), !dbg !2454
  %390 = zext i16 %389 to i32, !dbg !2454
  call void @llvm.dbg.value(metadata i32 %390, metadata !2103, metadata !DIExpression()), !dbg !2130
  %391 = icmp slt i32 %379, %2, !dbg !2455
  br i1 %391, label %393, label %392, !dbg !2457

392:                                              ; preds = %387
  call void @llvm.dbg.value(metadata i32 %379, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %379, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %379, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %379, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %379, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %379, i32 %1, i32 %2, i32 %390, i32 0, i32 0, i32 0, i32 0), !dbg !2458
  br label %420, !dbg !2460

393:                                              ; preds = %387, %383
  %394 = phi i32 [ %207, %383 ], [ %390, %387 ]
  %395 = phi i32 [ %204, %383 ], [ %379, %387 ]
  call void @llvm.dbg.value(metadata i32 %395, metadata !2091, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %379, metadata !2097, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %377, metadata !2100, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32 %394, metadata !2103, metadata !DIExpression()), !dbg !2130
  call void @llvm.dbg.value(metadata i32* %6, metadata !2096, metadata !DIExpression(DW_OP_deref)), !dbg !2130
  %396 = call fastcc i32 @_ZL15remove_one_fastPiS_S_i(i32* nonnull %6, i32* nonnull %169, i32* nonnull %155, i32 %194), !dbg !2353
  %397 = icmp eq i32 %396, 0, !dbg !2353
  br i1 %397, label %398, label %203, !dbg !2354, !llvm.loop !2379

398:                                              ; preds = %393, %268, %248, %211, %193
  %399 = phi i32 [ %180, %193 ], [ %207, %211 ], [ %207, %248 ], [ %207, %268 ], [ %394, %393 ]
  %400 = phi i32 [ %181, %193 ], [ %206, %211 ], [ %206, %248 ], [ %206, %268 ], [ %377, %393 ]
  %401 = phi i32 [ %182, %193 ], [ %205, %211 ], [ %205, %248 ], [ %205, %268 ], [ %379, %393 ]
  %402 = phi i32 [ %185, %193 ], [ %204, %211 ], [ %204, %248 ], [ %204, %268 ], [ %395, %393 ]
  call void @llvm.dbg.label(metadata !2129), !dbg !2461
  %403 = icmp eq i32 %183, 1, !dbg !2462
  %404 = and i1 %167, %403, !dbg !2463
  br i1 %404, label %405, label %407, !dbg !2463

405:                                              ; preds = %407, %398
  %406 = phi i32 [ 2, %398 ], [ 3, %407 ]
  br label %179, !dbg !2130

407:                                              ; preds = %398
  %408 = and i1 %167, %186, !dbg !2464
  %409 = and i1 %178, %408, !dbg !2464
  %410 = icmp sgt i32 %74, %402, !dbg !2465
  %411 = and i1 %409, %410, !dbg !2464
  br i1 %411, label %405, label %412, !dbg !2464

412:                                              ; preds = %407
  %413 = icmp ne i32 %400, 0, !dbg !2468
  %414 = and i1 %75, %413, !dbg !2470
  br i1 %414, label %415, label %418, !dbg !2470

415:                                              ; preds = %412
  %416 = load i32, i32* %24, align 8, !dbg !2471, !tbaa !968
  %417 = add nsw i32 %416, -32000, !dbg !2473
  call void @llvm.dbg.value(metadata i32 %417, metadata !2091, metadata !DIExpression()), !dbg !2130
  br label %418, !dbg !2474

418:                                              ; preds = %415, %412
  %419 = phi i32 [ %417, %415 ], [ %402, %412 ]
  call void @llvm.dbg.value(metadata i32 %419, metadata !2091, metadata !DIExpression()), !dbg !2130
  call void @_Z7StoreTTP7state_tiiijiiii(%struct.state_t* %0, i32 %419, i32 %1, i32 %2, i32 %399, i32 0, i32 0, i32 0, i32 0), !dbg !2475
  br label %420, !dbg !2476

420:                                              ; preds = %418, %392, %376, %145, %139, %136, %118, %112, %109, %85, %78, %66, %55, %52, %50, %40, %30
  %421 = phi i32 [ %47, %40 ], [ %67, %66 ], [ %379, %392 ], [ %419, %418 ], [ %73, %78 ], [ %83, %85 ], [ %51, %50 ], [ 0, %30 ], [ %53, %52 ], [ %56, %55 ], [ %143, %145 ], [ %116, %118 ], [ %110, %109 ], [ %113, %112 ], [ %137, %136 ], [ %140, %139 ], [ 0, %376 ]
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %17) #7, !dbg !2477
  call void @llvm.lifetime.end.p0i8(i64 960, i8* nonnull %16) #7, !dbg !2477
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %15) #7, !dbg !2477
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %14) #7, !dbg !2477
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %13) #7, !dbg !2477
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %12) #7, !dbg !2477
  ret i32 %421, !dbg !2477
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #5 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { argmemonly nounwind willreturn }
attributes #7 = { nounwind }

!llvm.dbg.cu = !{!816, !2, !330, !818, !820, !822, !446, !824, !827, !829, !831, !512, !533, !559, !588, !833, !670, !717, !742, !776}
!llvm.ident = !{!835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835, !835}
!llvm.module.flags = !{!836, !837, !838, !839, !840, !841}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "Mask", scope: !2, file: !3, line: 64, type: !14, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !3, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !5, globals: !11, imports: !116, nameTableKind: None)
!3 = !DIFile(filename: "bitboard.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!4 = !{}
!5 = !{!6, !9, !10}
!6 = !DIDerivedType(tag: DW_TAG_typedef, name: "BITBOARD", file: !7, line: 33, baseType: !8)
!7 = !DIFile(filename: "./sjeng.h", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!8 = !DIBasicType(name: "long long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!9 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!10 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!11 = !{!12, !17, !19, !21, !23, !25, !27, !32, !34, !0, !37, !39, !41, !43, !47, !49, !51, !53, !55, !57, !59, !61, !63, !68, !70, !72, !74, !76, !78, !80, !82, !84, !86, !88, !90, !92, !94, !96, !98, !100, !102, !104, !109, !114}
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(name: "KnightMoves", scope: !2, file: !3, line: 49, type: !14, isLocal: false, isDefinition: true)
!14 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 4096, elements: !15)
!15 = !{!16}
!16 = !DISubrange(count: 64)
!17 = !DIGlobalVariableExpression(var: !18, expr: !DIExpression())
!18 = distinct !DIGlobalVariable(name: "KingMoves", scope: !2, file: !3, line: 50, type: !14, isLocal: false, isDefinition: true)
!19 = !DIGlobalVariableExpression(var: !20, expr: !DIExpression())
!20 = distinct !DIGlobalVariable(name: "PawnAttacksBlack", scope: !2, file: !3, line: 51, type: !14, isLocal: false, isDefinition: true)
!21 = !DIGlobalVariableExpression(var: !22, expr: !DIExpression())
!22 = distinct !DIGlobalVariable(name: "PawnAttacksWhite", scope: !2, file: !3, line: 52, type: !14, isLocal: false, isDefinition: true)
!23 = !DIGlobalVariableExpression(var: !24, expr: !DIExpression())
!24 = distinct !DIGlobalVariable(name: "PawnMovesBlack", scope: !2, file: !3, line: 53, type: !14, isLocal: false, isDefinition: true)
!25 = !DIGlobalVariableExpression(var: !26, expr: !DIExpression())
!26 = distinct !DIGlobalVariable(name: "PawnMovesWhite", scope: !2, file: !3, line: 54, type: !14, isLocal: false, isDefinition: true)
!27 = !DIGlobalVariableExpression(var: !28, expr: !DIExpression())
!28 = distinct !DIGlobalVariable(name: "fillUpAttacks", scope: !2, file: !3, line: 56, type: !29, isLocal: false, isDefinition: true)
!29 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 32768, elements: !30)
!30 = !{!16, !31}
!31 = !DISubrange(count: 8)
!32 = !DIGlobalVariableExpression(var: !33, expr: !DIExpression())
!33 = distinct !DIGlobalVariable(name: "aFileAttacks", scope: !2, file: !3, line: 57, type: !29, isLocal: false, isDefinition: true)
!34 = !DIGlobalVariableExpression(var: !35, expr: !DIExpression())
!35 = distinct !DIGlobalVariable(name: "firstRankAttacks", scope: !2, file: !3, line: 59, type: !36, isLocal: false, isDefinition: true)
!36 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 4096, elements: !30)
!37 = !DIGlobalVariableExpression(var: !38, expr: !DIExpression())
!38 = distinct !DIGlobalVariable(name: "InvMask", scope: !2, file: !3, line: 65, type: !14, isLocal: false, isDefinition: true)
!39 = !DIGlobalVariableExpression(var: !40, expr: !DIExpression())
!40 = distinct !DIGlobalVariable(name: "DiagMaska1h8", scope: !2, file: !3, line: 67, type: !14, isLocal: false, isDefinition: true)
!41 = !DIGlobalVariableExpression(var: !42, expr: !DIExpression())
!42 = distinct !DIGlobalVariable(name: "DiagMaska8h1", scope: !2, file: !3, line: 68, type: !14, isLocal: false, isDefinition: true)
!43 = !DIGlobalVariableExpression(var: !44, expr: !DIExpression())
!44 = distinct !DIGlobalVariable(name: "FileMask", scope: !2, file: !3, line: 70, type: !45, isLocal: false, isDefinition: true)
!45 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 512, elements: !46)
!46 = !{!31}
!47 = !DIGlobalVariableExpression(var: !48, expr: !DIExpression())
!48 = distinct !DIGlobalVariable(name: "RankMask", scope: !2, file: !3, line: 71, type: !45, isLocal: false, isDefinition: true)
!49 = !DIGlobalVariableExpression(var: !50, expr: !DIExpression())
!50 = distinct !DIGlobalVariable(name: "AboveMask", scope: !2, file: !3, line: 72, type: !45, isLocal: false, isDefinition: true)
!51 = !DIGlobalVariableExpression(var: !52, expr: !DIExpression())
!52 = distinct !DIGlobalVariable(name: "BelowMask", scope: !2, file: !3, line: 73, type: !45, isLocal: false, isDefinition: true)
!53 = !DIGlobalVariableExpression(var: !54, expr: !DIExpression())
!54 = distinct !DIGlobalVariable(name: "LeftMask", scope: !2, file: !3, line: 74, type: !45, isLocal: false, isDefinition: true)
!55 = !DIGlobalVariableExpression(var: !56, expr: !DIExpression())
!56 = distinct !DIGlobalVariable(name: "RightMask", scope: !2, file: !3, line: 75, type: !45, isLocal: false, isDefinition: true)
!57 = !DIGlobalVariableExpression(var: !58, expr: !DIExpression())
!58 = distinct !DIGlobalVariable(name: "RookMask", scope: !2, file: !3, line: 77, type: !14, isLocal: false, isDefinition: true)
!59 = !DIGlobalVariableExpression(var: !60, expr: !DIExpression())
!60 = distinct !DIGlobalVariable(name: "BishopMask", scope: !2, file: !3, line: 78, type: !14, isLocal: false, isDefinition: true)
!61 = !DIGlobalVariableExpression(var: !62, expr: !DIExpression())
!62 = distinct !DIGlobalVariable(name: "QueenMask", scope: !2, file: !3, line: 79, type: !14, isLocal: false, isDefinition: true)
!63 = !DIGlobalVariableExpression(var: !64, expr: !DIExpression())
!64 = distinct !DIGlobalVariable(name: "CastleMask", scope: !2, file: !3, line: 81, type: !65, isLocal: false, isDefinition: true)
!65 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 256, elements: !66)
!66 = !{!67}
!67 = !DISubrange(count: 4)
!68 = !DIGlobalVariableExpression(var: !69, expr: !DIExpression())
!69 = distinct !DIGlobalVariable(name: "FileUpMask", scope: !2, file: !3, line: 83, type: !14, isLocal: false, isDefinition: true)
!70 = !DIGlobalVariableExpression(var: !71, expr: !DIExpression())
!71 = distinct !DIGlobalVariable(name: "FileDownMask", scope: !2, file: !3, line: 84, type: !14, isLocal: false, isDefinition: true)
!72 = !DIGlobalVariableExpression(var: !73, expr: !DIExpression())
!73 = distinct !DIGlobalVariable(name: "WhiteKingSide", scope: !2, file: !3, line: 86, type: !6, isLocal: false, isDefinition: true)
!74 = !DIGlobalVariableExpression(var: !75, expr: !DIExpression())
!75 = distinct !DIGlobalVariable(name: "WhiteQueenSide", scope: !2, file: !3, line: 87, type: !6, isLocal: false, isDefinition: true)
!76 = !DIGlobalVariableExpression(var: !77, expr: !DIExpression())
!77 = distinct !DIGlobalVariable(name: "BlackKingSide", scope: !2, file: !3, line: 88, type: !6, isLocal: false, isDefinition: true)
!78 = !DIGlobalVariableExpression(var: !79, expr: !DIExpression())
!79 = distinct !DIGlobalVariable(name: "BlackQueenSide", scope: !2, file: !3, line: 89, type: !6, isLocal: false, isDefinition: true)
!80 = !DIGlobalVariableExpression(var: !81, expr: !DIExpression())
!81 = distinct !DIGlobalVariable(name: "KingSafetyMask", scope: !2, file: !3, line: 90, type: !14, isLocal: false, isDefinition: true)
!82 = !DIGlobalVariableExpression(var: !83, expr: !DIExpression())
!83 = distinct !DIGlobalVariable(name: "KingSafetyMask1", scope: !2, file: !3, line: 91, type: !14, isLocal: false, isDefinition: true)
!84 = !DIGlobalVariableExpression(var: !85, expr: !DIExpression())
!85 = distinct !DIGlobalVariable(name: "WhiteStrongSquareMask", scope: !2, file: !3, line: 93, type: !6, isLocal: false, isDefinition: true)
!86 = !DIGlobalVariableExpression(var: !87, expr: !DIExpression())
!87 = distinct !DIGlobalVariable(name: "BlackStrongSquareMask", scope: !2, file: !3, line: 94, type: !6, isLocal: false, isDefinition: true)
!88 = !DIGlobalVariableExpression(var: !89, expr: !DIExpression())
!89 = distinct !DIGlobalVariable(name: "WhiteSqMask", scope: !2, file: !3, line: 96, type: !6, isLocal: false, isDefinition: true)
!90 = !DIGlobalVariableExpression(var: !91, expr: !DIExpression())
!91 = distinct !DIGlobalVariable(name: "BlackSqMask", scope: !2, file: !3, line: 97, type: !6, isLocal: false, isDefinition: true)
!92 = !DIGlobalVariableExpression(var: !93, expr: !DIExpression())
!93 = distinct !DIGlobalVariable(name: "KSMask", scope: !2, file: !3, line: 99, type: !6, isLocal: false, isDefinition: true)
!94 = !DIGlobalVariableExpression(var: !95, expr: !DIExpression())
!95 = distinct !DIGlobalVariable(name: "QSMask", scope: !2, file: !3, line: 100, type: !6, isLocal: false, isDefinition: true)
!96 = !DIGlobalVariableExpression(var: !97, expr: !DIExpression())
!97 = distinct !DIGlobalVariable(name: "KingFilesMask", scope: !2, file: !3, line: 102, type: !45, isLocal: false, isDefinition: true)
!98 = !DIGlobalVariableExpression(var: !99, expr: !DIExpression())
!99 = distinct !DIGlobalVariable(name: "KingPressureMask", scope: !2, file: !3, line: 103, type: !14, isLocal: false, isDefinition: true)
!100 = !DIGlobalVariableExpression(var: !101, expr: !DIExpression())
!101 = distinct !DIGlobalVariable(name: "KingPressureMask1", scope: !2, file: !3, line: 104, type: !14, isLocal: false, isDefinition: true)
!102 = !DIGlobalVariableExpression(var: !103, expr: !DIExpression())
!103 = distinct !DIGlobalVariable(name: "CenterMask", scope: !2, file: !3, line: 105, type: !6, isLocal: false, isDefinition: true)
!104 = !DIGlobalVariableExpression(var: !105, expr: !DIExpression())
!105 = distinct !DIGlobalVariable(name: "SpaceMask", scope: !2, file: !3, line: 106, type: !106, isLocal: false, isDefinition: true)
!106 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 128, elements: !107)
!107 = !{!108}
!108 = !DISubrange(count: 2)
!109 = !DIGlobalVariableExpression(var: !110, expr: !DIExpression())
!110 = distinct !DIGlobalVariable(name: "DiagonalLength_a1h8", linkageName: "_ZL19DiagonalLength_a1h8", scope: !2, file: !3, line: 27, type: !111, isLocal: true, isDefinition: true)
!111 = !DICompositeType(tag: DW_TAG_array_type, baseType: !112, size: 2048, elements: !15)
!112 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !113)
!113 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!114 = !DIGlobalVariableExpression(var: !115, expr: !DIExpression())
!115 = distinct !DIGlobalVariable(name: "DiagonalLength_a8h1", linkageName: "_ZL19DiagonalLength_a8h1", scope: !2, file: !3, line: 38, type: !111, isLocal: true, isDefinition: true)
!116 = !{!117, !122, !127, !134, !138, !142, !147, !156, !160, !164, !178, !182, !186, !190, !194, !199, !203, !207, !211, !215, !223, !227, !231, !235, !239, !243, !249, !253, !257, !259, !267, !271, !279, !281, !285, !289, !293, !297, !301, !306, !311, !312, !313, !314, !316, !317, !318, !319, !320, !321, !322, !326}
!117 = !DIImportedEntity(tag: DW_TAG_imported_module, scope: !118, entity: !119, file: !121, line: 56)
!118 = !DINamespace(name: "__gnu_debug", scope: null)
!119 = !DINamespace(name: "__debug", scope: !120)
!120 = !DINamespace(name: "std", scope: null)
!121 = !DIFile(filename: "/nfs/sc/proj/icl/rdrive/ref/gcc/5.5.0/rhel70/efi2/bin/../include/c++/5.5.0/debug/debug.h", directory: "")
!122 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !123, file: !126, line: 118)
!123 = !DIDerivedType(tag: DW_TAG_typedef, name: "div_t", file: !124, line: 101, baseType: !125)
!124 = !DIFile(filename: "/usr/include/stdlib.h", directory: "")
!125 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !124, line: 97, flags: DIFlagFwdDecl, identifier: "_ZTS5div_t")
!126 = !DIFile(filename: "/nfs/sc/proj/icl/rdrive/ref/gcc/5.5.0/rhel70/efi2/bin/../include/c++/5.5.0/cstdlib", directory: "")
!127 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !128, file: !126, line: 119)
!128 = !DIDerivedType(tag: DW_TAG_typedef, name: "ldiv_t", file: !124, line: 109, baseType: !129)
!129 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !124, line: 105, size: 128, flags: DIFlagTypePassByValue, elements: !130, identifier: "_ZTS6ldiv_t")
!130 = !{!131, !133}
!131 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !129, file: !124, line: 107, baseType: !132, size: 64)
!132 = !DIBasicType(name: "long int", size: 64, encoding: DW_ATE_signed)
!133 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !129, file: !124, line: 108, baseType: !132, size: 64, offset: 64)
!134 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !135, file: !126, line: 121)
!135 = !DISubprogram(name: "abort", scope: !124, file: !124, line: 514, type: !136, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!136 = !DISubroutineType(types: !137)
!137 = !{null}
!138 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !139, file: !126, line: 122)
!139 = !DISubprogram(name: "abs", scope: !124, file: !124, line: 770, type: !140, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!140 = !DISubroutineType(types: !141)
!141 = !{!113, !113}
!142 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !143, file: !126, line: 123)
!143 = !DISubprogram(name: "atexit", scope: !124, file: !124, line: 518, type: !144, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!144 = !DISubroutineType(types: !145)
!145 = !{!113, !146}
!146 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !136, size: 64)
!147 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !148, file: !126, line: 129)
!148 = !DISubprogram(name: "atof", scope: !149, file: !149, line: 26, type: !150, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!149 = !DIFile(filename: "/usr/include/bits/stdlib-float.h", directory: "")
!150 = !DISubroutineType(types: !151)
!151 = !{!152, !153}
!152 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!153 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !154, size: 64)
!154 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !155)
!155 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!156 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !157, file: !126, line: 130)
!157 = !DISubprogram(name: "atoi", scope: !124, file: !124, line: 278, type: !158, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!158 = !DISubroutineType(types: !159)
!159 = !{!113, !153}
!160 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !161, file: !126, line: 131)
!161 = !DISubprogram(name: "atol", scope: !124, file: !124, line: 283, type: !162, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!162 = !DISubroutineType(types: !163)
!163 = !{!132, !153}
!164 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !165, file: !126, line: 132)
!165 = !DISubprogram(name: "bsearch", scope: !124, file: !124, line: 754, type: !166, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!166 = !DISubroutineType(types: !167)
!167 = !{!168, !169, !169, !171, !171, !174}
!168 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!169 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !170, size: 64)
!170 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!171 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !172, line: 51, baseType: !173)
!172 = !DIFile(filename: "XMAIN/deploy/linux_debug/lib/clang/10.0.0/include/stddef.h", directory: "/export/iusers/cczhao/Workspaces")
!173 = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!174 = !DIDerivedType(tag: DW_TAG_typedef, name: "__compar_fn_t", file: !124, line: 741, baseType: !175)
!175 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !176, size: 64)
!176 = !DISubroutineType(types: !177)
!177 = !{!113, !169, !169}
!178 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !179, file: !126, line: 133)
!179 = !DISubprogram(name: "calloc", scope: !124, file: !124, line: 467, type: !180, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!180 = !DISubroutineType(types: !181)
!181 = !{!168, !171, !171}
!182 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !183, file: !126, line: 134)
!183 = !DISubprogram(name: "div", scope: !124, file: !124, line: 784, type: !184, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!184 = !DISubroutineType(types: !185)
!185 = !{!123, !113, !113}
!186 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !187, file: !126, line: 135)
!187 = !DISubprogram(name: "exit", scope: !124, file: !124, line: 542, type: !188, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!188 = !DISubroutineType(types: !189)
!189 = !{null, !113}
!190 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !191, file: !126, line: 136)
!191 = !DISubprogram(name: "free", scope: !124, file: !124, line: 482, type: !192, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!192 = !DISubroutineType(types: !193)
!193 = !{null, !168}
!194 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !195, file: !126, line: 137)
!195 = !DISubprogram(name: "getenv", scope: !124, file: !124, line: 563, type: !196, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!196 = !DISubroutineType(types: !197)
!197 = !{!198, !153}
!198 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !155, size: 64)
!199 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !200, file: !126, line: 138)
!200 = !DISubprogram(name: "labs", scope: !124, file: !124, line: 771, type: !201, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!201 = !DISubroutineType(types: !202)
!202 = !{!132, !132}
!203 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !204, file: !126, line: 139)
!204 = !DISubprogram(name: "ldiv", scope: !124, file: !124, line: 786, type: !205, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!205 = !DISubroutineType(types: !206)
!206 = !{!128, !132, !132}
!207 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !208, file: !126, line: 140)
!208 = !DISubprogram(name: "malloc", scope: !124, file: !124, line: 465, type: !209, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!209 = !DISubroutineType(types: !210)
!210 = !{!168, !171}
!211 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !212, file: !126, line: 142)
!212 = !DISubprogram(name: "mblen", scope: !124, file: !124, line: 859, type: !213, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!213 = !DISubroutineType(types: !214)
!214 = !{!113, !153, !171}
!215 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !216, file: !126, line: 143)
!216 = !DISubprogram(name: "mbstowcs", scope: !124, file: !124, line: 870, type: !217, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!217 = !DISubroutineType(types: !218)
!218 = !{!171, !219, !222, !171}
!219 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !220)
!220 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !221, size: 64)
!221 = !DIBasicType(name: "wchar_t", size: 32, encoding: DW_ATE_signed)
!222 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !153)
!223 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !224, file: !126, line: 144)
!224 = !DISubprogram(name: "mbtowc", scope: !124, file: !124, line: 862, type: !225, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!225 = !DISubroutineType(types: !226)
!226 = !{!113, !219, !222, !171}
!227 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !228, file: !126, line: 146)
!228 = !DISubprogram(name: "qsort", scope: !124, file: !124, line: 760, type: !229, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!229 = !DISubroutineType(types: !230)
!230 = !{null, !168, !171, !171, !174}
!231 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !232, file: !126, line: 152)
!232 = !DISubprogram(name: "rand", scope: !124, file: !124, line: 374, type: !233, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!233 = !DISubroutineType(types: !234)
!234 = !{!113}
!235 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !236, file: !126, line: 153)
!236 = !DISubprogram(name: "realloc", scope: !124, file: !124, line: 479, type: !237, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!237 = !DISubroutineType(types: !238)
!238 = !{!168, !168, !171}
!239 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !240, file: !126, line: 154)
!240 = !DISubprogram(name: "srand", scope: !124, file: !124, line: 376, type: !241, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!241 = !DISubroutineType(types: !242)
!242 = !{null, !10}
!243 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !244, file: !126, line: 155)
!244 = !DISubprogram(name: "strtod", scope: !124, file: !124, line: 164, type: !245, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!245 = !DISubroutineType(types: !246)
!246 = !{!152, !222, !247}
!247 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !248)
!248 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !198, size: 64)
!249 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !250, file: !126, line: 156)
!250 = !DISubprogram(name: "strtol", scope: !124, file: !124, line: 183, type: !251, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!251 = !DISubroutineType(types: !252)
!252 = !{!132, !222, !247, !113}
!253 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !254, file: !126, line: 157)
!254 = !DISubprogram(name: "strtoul", scope: !124, file: !124, line: 187, type: !255, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!255 = !DISubroutineType(types: !256)
!256 = !{!173, !222, !247, !113}
!257 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !258, file: !126, line: 158)
!258 = !DISubprogram(name: "system", scope: !124, file: !124, line: 716, type: !158, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!259 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !260, file: !126, line: 160)
!260 = !DISubprogram(name: "wcstombs", scope: !124, file: !124, line: 873, type: !261, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!261 = !DISubroutineType(types: !262)
!262 = !{!171, !263, !264, !171}
!263 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !198)
!264 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !265)
!265 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !266, size: 64)
!266 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !221)
!267 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !268, file: !126, line: 161)
!268 = !DISubprogram(name: "wctomb", scope: !124, file: !124, line: 866, type: !269, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!269 = !DISubroutineType(types: !270)
!270 = !{!113, !198, !221}
!271 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !273, file: !126, line: 214)
!272 = !DINamespace(name: "__gnu_cxx", scope: null)
!273 = !DIDerivedType(tag: DW_TAG_typedef, name: "lldiv_t", file: !124, line: 121, baseType: !274)
!274 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !124, line: 117, size: 128, flags: DIFlagTypePassByValue, elements: !275, identifier: "_ZTS7lldiv_t")
!275 = !{!276, !278}
!276 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !274, file: !124, line: 119, baseType: !277, size: 64)
!277 = !DIBasicType(name: "long long int", size: 64, encoding: DW_ATE_signed)
!278 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !274, file: !124, line: 120, baseType: !277, size: 64, offset: 64)
!279 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !280, file: !126, line: 220)
!280 = !DISubprogram(name: "_Exit", scope: !124, file: !124, line: 556, type: !188, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!281 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !282, file: !126, line: 224)
!282 = !DISubprogram(name: "llabs", scope: !124, file: !124, line: 775, type: !283, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!283 = !DISubroutineType(types: !284)
!284 = !{!277, !277}
!285 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !286, file: !126, line: 230)
!286 = !DISubprogram(name: "lldiv", scope: !124, file: !124, line: 792, type: !287, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!287 = !DISubroutineType(types: !288)
!288 = !{!273, !277, !277}
!289 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !290, file: !126, line: 241)
!290 = !DISubprogram(name: "atoll", scope: !124, file: !124, line: 292, type: !291, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!291 = !DISubroutineType(types: !292)
!292 = !{!277, !153}
!293 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !294, file: !126, line: 242)
!294 = !DISubprogram(name: "strtoll", scope: !124, file: !124, line: 209, type: !295, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!295 = !DISubroutineType(types: !296)
!296 = !{!277, !222, !247, !113}
!297 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !298, file: !126, line: 243)
!298 = !DISubprogram(name: "strtoull", scope: !124, file: !124, line: 214, type: !299, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!299 = !DISubroutineType(types: !300)
!300 = !{!8, !222, !247, !113}
!301 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !302, file: !126, line: 245)
!302 = !DISubprogram(name: "strtof", scope: !124, file: !124, line: 172, type: !303, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!303 = !DISubroutineType(types: !304)
!304 = !{!305, !222, !247}
!305 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!306 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !307, file: !126, line: 246)
!307 = !DISubprogram(name: "strtold", scope: !124, file: !124, line: 175, type: !308, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!308 = !DISubroutineType(types: !309)
!309 = !{!310, !222, !247}
!310 = !DIBasicType(name: "long double", size: 128, encoding: DW_ATE_float)
!311 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !273, file: !126, line: 254)
!312 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !280, file: !126, line: 256)
!313 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !282, file: !126, line: 258)
!314 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !315, file: !126, line: 259)
!315 = !DISubprogram(name: "div", linkageName: "_ZN9__gnu_cxx3divExx", scope: !272, file: !126, line: 227, type: !287, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!316 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !286, file: !126, line: 260)
!317 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !290, file: !126, line: 262)
!318 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !302, file: !126, line: 263)
!319 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !294, file: !126, line: 264)
!320 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !298, file: !126, line: 265)
!321 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !120, entity: !307, file: !126, line: 266)
!322 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !323, file: !325, line: 44)
!323 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", scope: !120, file: !324, line: 199, baseType: !173)
!324 = !DIFile(filename: "/nfs/sc/proj/icl/rdrive/ref/gcc/5.5.0/rhel70/efi2/bin/../include/c++/5.5.0/x86_64-linux-gnu/bits/c++config.h", directory: "")
!325 = !DIFile(filename: "/nfs/sc/proj/icl/rdrive/ref/gcc/5.5.0/rhel70/efi2/bin/../include/c++/5.5.0/ext/new_allocator.h", directory: "")
!326 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !272, entity: !327, file: !325, line: 45)
!327 = !DIDerivedType(tag: DW_TAG_typedef, name: "ptrdiff_t", scope: !120, file: !324, line: 200, baseType: !132)
!328 = !DIGlobalVariableExpression(var: !329, expr: !DIExpression())
!329 = distinct !DIGlobalVariable(name: "last_bit", scope: !330, file: !331, line: 55, type: !338, isLocal: false, isDefinition: true)
!330 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !331, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !234, globals: !332, nameTableKind: None)
!331 = !DIFile(filename: "bits.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!332 = !{!333, !328, !336}
!333 = !DIGlobalVariableExpression(var: !334, expr: !DIExpression(DW_OP_constu, 283881067100198605, DW_OP_stack_value))
!334 = distinct !DIGlobalVariable(name: "magic", scope: !330, file: !331, line: 29, type: !335, isLocal: true, isDefinition: true)
!335 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !6)
!336 = !DIGlobalVariableExpression(var: !337, expr: !DIExpression())
!337 = distinct !DIGlobalVariable(name: "magictable", linkageName: "_ZL10magictable", scope: !330, file: !331, line: 30, type: !111, isLocal: true, isDefinition: true)
!338 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 524288, elements: !339)
!339 = !{!340}
!340 = !DISubrange(count: 65536)
!341 = !DIGlobalVariableExpression(var: !342, expr: !DIExpression())
!342 = distinct !DIGlobalVariable(name: "rankoffsets", scope: !343, file: !344, line: 24, type: !492, isLocal: true, isDefinition: true)
!343 = distinct !DISubprogram(name: "setup_epd_line", linkageName: "_Z14setup_epd_lineP11gamestate_tP7state_tPKc", scope: !344, file: !344, line: 23, type: !345, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !446, retainedNodes: !493)
!344 = !DIFile(filename: "epd.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!345 = !DISubroutineType(types: !346)
!346 = !{null, !347, !391, !153}
!347 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !348, size: 64)
!348 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "gamestate_t", file: !349, line: 72, size: 288832, flags: DIFlagTypePassByValue, elements: !350, identifier: "_ZTS11gamestate_t")
!349 = !DIFile(filename: "./state.h", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!350 = !{!351, !352, !353, !354, !355, !356, !357, !358, !359, !360, !361, !362, !363, !364, !365, !366, !367, !372, !382, !383, !384, !385, !386, !387, !388, !389, !390}
!351 = !DIDerivedType(tag: DW_TAG_member, name: "cur_score", scope: !348, file: !349, line: 73, baseType: !113, size: 32)
!352 = !DIDerivedType(tag: DW_TAG_member, name: "phase", scope: !348, file: !349, line: 74, baseType: !113, size: 32, offset: 32)
!353 = !DIDerivedType(tag: DW_TAG_member, name: "root_to_move", scope: !348, file: !349, line: 75, baseType: !113, size: 32, offset: 64)
!354 = !DIDerivedType(tag: DW_TAG_member, name: "comp_color", scope: !348, file: !349, line: 76, baseType: !113, size: 32, offset: 96)
!355 = !DIDerivedType(tag: DW_TAG_member, name: "result", scope: !348, file: !349, line: 76, baseType: !113, size: 32, offset: 128)
!356 = !DIDerivedType(tag: DW_TAG_member, name: "i_depth", scope: !348, file: !349, line: 76, baseType: !113, size: 32, offset: 160)
!357 = !DIDerivedType(tag: DW_TAG_member, name: "moves_to_tc", scope: !348, file: !349, line: 77, baseType: !113, size: 32, offset: 192)
!358 = !DIDerivedType(tag: DW_TAG_member, name: "min_per_game", scope: !348, file: !349, line: 77, baseType: !113, size: 32, offset: 224)
!359 = !DIDerivedType(tag: DW_TAG_member, name: "sec_per_game", scope: !348, file: !349, line: 77, baseType: !113, size: 32, offset: 256)
!360 = !DIDerivedType(tag: DW_TAG_member, name: "inc", scope: !348, file: !349, line: 77, baseType: !113, size: 32, offset: 288)
!361 = !DIDerivedType(tag: DW_TAG_member, name: "time_left", scope: !348, file: !349, line: 78, baseType: !113, size: 32, offset: 320)
!362 = !DIDerivedType(tag: DW_TAG_member, name: "opp_time", scope: !348, file: !349, line: 78, baseType: !113, size: 32, offset: 352)
!363 = !DIDerivedType(tag: DW_TAG_member, name: "time_for_move", scope: !348, file: !349, line: 78, baseType: !113, size: 32, offset: 384)
!364 = !DIDerivedType(tag: DW_TAG_member, name: "fixed_time", scope: !348, file: !349, line: 79, baseType: !113, size: 32, offset: 416)
!365 = !DIDerivedType(tag: DW_TAG_member, name: "maxdepth", scope: !348, file: !349, line: 80, baseType: !113, size: 32, offset: 448)
!366 = !DIDerivedType(tag: DW_TAG_member, name: "move_number", scope: !348, file: !349, line: 82, baseType: !113, size: 32, offset: 480)
!367 = !DIDerivedType(tag: DW_TAG_member, name: "game_history", scope: !348, file: !349, line: 83, baseType: !368, size: 32000, offset: 512)
!368 = !DICompositeType(tag: DW_TAG_array_type, baseType: !369, size: 32000, elements: !370)
!369 = !DIDerivedType(tag: DW_TAG_typedef, name: "move_s", file: !7, line: 121, baseType: !113)
!370 = !{!371}
!371 = !DISubrange(count: 1000)
!372 = !DIDerivedType(tag: DW_TAG_member, name: "game_history_x", scope: !348, file: !349, line: 84, baseType: !373, size: 256000, offset: 32512)
!373 = !DICompositeType(tag: DW_TAG_array_type, baseType: !374, size: 256000, elements: !370)
!374 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "move_x", file: !7, line: 130, size: 256, flags: DIFlagTypePassByValue, elements: !375, identifier: "_ZTS6move_x")
!375 = !{!376, !377, !378, !379, !380, !381}
!376 = !DIDerivedType(tag: DW_TAG_member, name: "epsq", scope: !374, file: !7, line: 131, baseType: !113, size: 32)
!377 = !DIDerivedType(tag: DW_TAG_member, name: "fifty", scope: !374, file: !7, line: 132, baseType: !113, size: 32, offset: 32)
!378 = !DIDerivedType(tag: DW_TAG_member, name: "castleflag", scope: !374, file: !7, line: 133, baseType: !113, size: 32, offset: 64)
!379 = !DIDerivedType(tag: DW_TAG_member, name: "psq_score", scope: !374, file: !7, line: 134, baseType: !113, size: 32, offset: 96)
!380 = !DIDerivedType(tag: DW_TAG_member, name: "hash", scope: !374, file: !7, line: 135, baseType: !6, size: 64, offset: 128)
!381 = !DIDerivedType(tag: DW_TAG_member, name: "pawnhash", scope: !374, file: !7, line: 136, baseType: !6, size: 64, offset: 192)
!382 = !DIDerivedType(tag: DW_TAG_member, name: "pvnodecount", scope: !348, file: !349, line: 85, baseType: !6, size: 64, offset: 288512)
!383 = !DIDerivedType(tag: DW_TAG_member, name: "start_time", scope: !348, file: !349, line: 86, baseType: !113, size: 32, offset: 288576)
!384 = !DIDerivedType(tag: DW_TAG_member, name: "pv_best", scope: !348, file: !349, line: 88, baseType: !10, size: 32, offset: 288608)
!385 = !DIDerivedType(tag: DW_TAG_member, name: "legals", scope: !348, file: !349, line: 89, baseType: !113, size: 32, offset: 288640)
!386 = !DIDerivedType(tag: DW_TAG_member, name: "failed", scope: !348, file: !349, line: 90, baseType: !113, size: 32, offset: 288672)
!387 = !DIDerivedType(tag: DW_TAG_member, name: "failedhigh", scope: !348, file: !349, line: 91, baseType: !113, size: 32, offset: 288704)
!388 = !DIDerivedType(tag: DW_TAG_member, name: "extendedtime", scope: !348, file: !349, line: 92, baseType: !113, size: 32, offset: 288736)
!389 = !DIDerivedType(tag: DW_TAG_member, name: "time_exit", scope: !348, file: !349, line: 93, baseType: !113, size: 32, offset: 288768)
!390 = !DIDerivedType(tag: DW_TAG_member, name: "time_failure", scope: !348, file: !349, line: 93, baseType: !113, size: 32, offset: 288800)
!391 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !392, size: 64)
!392 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "state_t", file: !349, line: 11, size: 99200, flags: DIFlagTypePassByValue, elements: !393, identifier: "_ZTS7state_t")
!393 = !{!394, !395, !397, !398, !399, !400, !404, !405, !406, !408, !409, !410, !411, !412, !413, !414, !415, !416, !417, !419, !421, !422, !430, !431, !432, !433, !434, !435, !436, !437, !438, !439, !440, !441, !442, !443, !444}
!394 = !DIDerivedType(tag: DW_TAG_member, name: "threadid", scope: !392, file: !349, line: 13, baseType: !113, size: 32)
!395 = !DIDerivedType(tag: DW_TAG_member, name: "sboard", scope: !392, file: !349, line: 15, baseType: !396, size: 2048, offset: 32)
!396 = !DICompositeType(tag: DW_TAG_array_type, baseType: !113, size: 2048, elements: !15)
!397 = !DIDerivedType(tag: DW_TAG_member, name: "All", scope: !392, file: !349, line: 16, baseType: !6, size: 64, offset: 2112)
!398 = !DIDerivedType(tag: DW_TAG_member, name: "BlackPieces", scope: !392, file: !349, line: 17, baseType: !6, size: 64, offset: 2176)
!399 = !DIDerivedType(tag: DW_TAG_member, name: "WhitePieces", scope: !392, file: !349, line: 17, baseType: !6, size: 64, offset: 2240)
!400 = !DIDerivedType(tag: DW_TAG_member, name: "BitBoard", scope: !392, file: !349, line: 18, baseType: !401, size: 832, offset: 2304)
!401 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 832, elements: !402)
!402 = !{!403}
!403 = !DISubrange(count: 13)
!404 = !DIDerivedType(tag: DW_TAG_member, name: "Material", scope: !392, file: !349, line: 20, baseType: !113, size: 32, offset: 3136)
!405 = !DIDerivedType(tag: DW_TAG_member, name: "psq_score", scope: !392, file: !349, line: 21, baseType: !113, size: 32, offset: 3168)
!406 = !DIDerivedType(tag: DW_TAG_member, name: "npieces", scope: !392, file: !349, line: 22, baseType: !407, size: 416, offset: 3200)
!407 = !DICompositeType(tag: DW_TAG_array_type, baseType: !113, size: 416, elements: !402)
!408 = !DIDerivedType(tag: DW_TAG_member, name: "castleflag", scope: !392, file: !349, line: 23, baseType: !113, size: 32, offset: 3616)
!409 = !DIDerivedType(tag: DW_TAG_member, name: "ep_square", scope: !392, file: !349, line: 25, baseType: !113, size: 32, offset: 3648)
!410 = !DIDerivedType(tag: DW_TAG_member, name: "white_to_move", scope: !392, file: !349, line: 25, baseType: !113, size: 32, offset: 3680)
!411 = !DIDerivedType(tag: DW_TAG_member, name: "wking_loc", scope: !392, file: !349, line: 25, baseType: !113, size: 32, offset: 3712)
!412 = !DIDerivedType(tag: DW_TAG_member, name: "bking_loc", scope: !392, file: !349, line: 25, baseType: !113, size: 32, offset: 3744)
!413 = !DIDerivedType(tag: DW_TAG_member, name: "ply", scope: !392, file: !349, line: 26, baseType: !113, size: 32, offset: 3776)
!414 = !DIDerivedType(tag: DW_TAG_member, name: "fifty", scope: !392, file: !349, line: 26, baseType: !113, size: 32, offset: 3808)
!415 = !DIDerivedType(tag: DW_TAG_member, name: "hash", scope: !392, file: !349, line: 28, baseType: !6, size: 64, offset: 3840)
!416 = !DIDerivedType(tag: DW_TAG_member, name: "pawnhash", scope: !392, file: !349, line: 29, baseType: !6, size: 64, offset: 3904)
!417 = !DIDerivedType(tag: DW_TAG_member, name: "path_x", scope: !392, file: !349, line: 31, baseType: !418, size: 16384, offset: 3968)
!418 = !DICompositeType(tag: DW_TAG_array_type, baseType: !374, size: 16384, elements: !15)
!419 = !DIDerivedType(tag: DW_TAG_member, name: "path", scope: !392, file: !349, line: 32, baseType: !420, size: 2048, offset: 20352)
!420 = !DICompositeType(tag: DW_TAG_array_type, baseType: !369, size: 2048, elements: !15)
!421 = !DIDerivedType(tag: DW_TAG_member, name: "plyeval", scope: !392, file: !349, line: 33, baseType: !396, size: 2048, offset: 22400)
!422 = !DIDerivedType(tag: DW_TAG_member, name: "killerstack", scope: !392, file: !349, line: 41, baseType: !423, size: 8192, offset: 24448)
!423 = !DICompositeType(tag: DW_TAG_array_type, baseType: !424, size: 8192, elements: !15)
!424 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !392, file: !349, line: 36, size: 128, flags: DIFlagTypePassByValue, elements: !425, identifier: "_ZTSN7state_tUt_E")
!425 = !{!426, !427, !428, !429}
!426 = !DIDerivedType(tag: DW_TAG_member, name: "killer1", scope: !424, file: !349, line: 37, baseType: !369, size: 32)
!427 = !DIDerivedType(tag: DW_TAG_member, name: "killer2", scope: !424, file: !349, line: 38, baseType: !369, size: 32, offset: 32)
!428 = !DIDerivedType(tag: DW_TAG_member, name: "killer3", scope: !424, file: !349, line: 39, baseType: !369, size: 32, offset: 64)
!429 = !DIDerivedType(tag: DW_TAG_member, name: "killer4", scope: !424, file: !349, line: 40, baseType: !369, size: 32, offset: 96)
!430 = !DIDerivedType(tag: DW_TAG_member, name: "nodes", scope: !392, file: !349, line: 43, baseType: !6, size: 64, offset: 32640)
!431 = !DIDerivedType(tag: DW_TAG_member, name: "qnodes", scope: !392, file: !349, line: 43, baseType: !6, size: 64, offset: 32704)
!432 = !DIDerivedType(tag: DW_TAG_member, name: "maxply", scope: !392, file: !349, line: 44, baseType: !113, size: 32, offset: 32768)
!433 = !DIDerivedType(tag: DW_TAG_member, name: "checks", scope: !392, file: !349, line: 45, baseType: !396, size: 2048, offset: 32800)
!434 = !DIDerivedType(tag: DW_TAG_member, name: "TTProbes", scope: !392, file: !349, line: 51, baseType: !10, size: 32, offset: 34848)
!435 = !DIDerivedType(tag: DW_TAG_member, name: "TTHits", scope: !392, file: !349, line: 52, baseType: !10, size: 32, offset: 34880)
!436 = !DIDerivedType(tag: DW_TAG_member, name: "TTStores", scope: !392, file: !349, line: 53, baseType: !10, size: 32, offset: 34912)
!437 = !DIDerivedType(tag: DW_TAG_member, name: "TTColls", scope: !392, file: !349, line: 54, baseType: !10, size: 32, offset: 34944)
!438 = !DIDerivedType(tag: DW_TAG_member, name: "wking_start", scope: !392, file: !349, line: 60, baseType: !113, size: 32, offset: 34976)
!439 = !DIDerivedType(tag: DW_TAG_member, name: "bking_start", scope: !392, file: !349, line: 61, baseType: !113, size: 32, offset: 35008)
!440 = !DIDerivedType(tag: DW_TAG_member, name: "wlrook_start", scope: !392, file: !349, line: 62, baseType: !113, size: 32, offset: 35040)
!441 = !DIDerivedType(tag: DW_TAG_member, name: "wrrook_start", scope: !392, file: !349, line: 63, baseType: !113, size: 32, offset: 35072)
!442 = !DIDerivedType(tag: DW_TAG_member, name: "blrook_start", scope: !392, file: !349, line: 64, baseType: !113, size: 32, offset: 35104)
!443 = !DIDerivedType(tag: DW_TAG_member, name: "brrook_start", scope: !392, file: !349, line: 65, baseType: !113, size: 32, offset: 35136)
!444 = !DIDerivedType(tag: DW_TAG_member, name: "hash_history", scope: !392, file: !349, line: 69, baseType: !445, size: 64000, offset: 35200)
!445 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 64000, elements: !370)
!446 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !344, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !447, retainedTypes: !463, globals: !465, nameTableKind: None)
!447 = !{!448}
!448 = !DICompositeType(tag: DW_TAG_enumeration_type, file: !449, line: 47, baseType: !10, size: 32, elements: !450)
!449 = !DIFile(filename: "/usr/include/ctype.h", directory: "")
!450 = !{!451, !452, !453, !454, !455, !456, !457, !458, !459, !460, !461, !462}
!451 = !DIEnumerator(name: "_ISupper", value: 256, isUnsigned: true)
!452 = !DIEnumerator(name: "_ISlower", value: 512, isUnsigned: true)
!453 = !DIEnumerator(name: "_ISalpha", value: 1024, isUnsigned: true)
!454 = !DIEnumerator(name: "_ISdigit", value: 2048, isUnsigned: true)
!455 = !DIEnumerator(name: "_ISxdigit", value: 4096, isUnsigned: true)
!456 = !DIEnumerator(name: "_ISspace", value: 8192, isUnsigned: true)
!457 = !DIEnumerator(name: "_ISprint", value: 16384, isUnsigned: true)
!458 = !DIEnumerator(name: "_ISgraph", value: 32768, isUnsigned: true)
!459 = !DIEnumerator(name: "_ISblank", value: 1, isUnsigned: true)
!460 = !DIEnumerator(name: "_IScntrl", value: 2, isUnsigned: true)
!461 = !DIEnumerator(name: "_ISpunct", value: 4, isUnsigned: true)
!462 = !DIEnumerator(name: "_ISalnum", value: 8, isUnsigned: true)
!463 = !{!113, !305, !464, !248}
!464 = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)
!465 = !{!341, !466, !485, !490}
!466 = !DIGlobalVariableExpression(var: !467, expr: !DIExpression())
!467 = distinct !DIGlobalVariable(name: "xlate", scope: !468, file: !344, line: 384, type: !482, isLocal: true, isDefinition: true)
!468 = distinct !DISubprogram(name: "position_to_fen", linkageName: "_Z15position_to_fenP7state_tPc", scope: !344, file: !344, line: 383, type: !469, scopeLine: 383, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !446, retainedNodes: !471)
!469 = !DISubroutineType(types: !470)
!470 = !{null, !391, !198}
!471 = !{!472, !473, !474, !475, !476, !477, !478, !479, !480}
!472 = !DILocalVariable(name: "s", arg: 1, scope: !468, file: !344, line: 383, type: !391)
!473 = !DILocalVariable(name: "fen", arg: 2, scope: !468, file: !344, line: 383, type: !198)
!474 = !DILocalVariable(name: "xrank", scope: !468, file: !344, line: 387, type: !113)
!475 = !DILocalVariable(name: "xfile", scope: !468, file: !344, line: 387, type: !113)
!476 = !DILocalVariable(name: "nempty", scope: !468, file: !344, line: 387, type: !113)
!477 = !DILocalVariable(name: "thissq", scope: !468, file: !344, line: 387, type: !113)
!478 = !DILocalVariable(name: "castflag", scope: !468, file: !344, line: 388, type: !113)
!479 = !DILocalVariable(name: "c", scope: !468, file: !344, line: 389, type: !198)
!480 = !DILocalVariable(name: "sboard", scope: !468, file: !344, line: 391, type: !481)
!481 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !113, size: 64)
!482 = !DICompositeType(tag: DW_TAG_array_type, baseType: !154, size: 112, elements: !483)
!483 = !{!484}
!484 = !DISubrange(count: 14)
!485 = !DIGlobalVariableExpression(var: !486, expr: !DIExpression())
!486 = distinct !DIGlobalVariable(name: "str_empty", scope: !468, file: !344, line: 385, type: !487, isLocal: true, isDefinition: true)
!487 = !DICompositeType(tag: DW_TAG_array_type, baseType: !154, size: 72, elements: !488)
!488 = !{!489}
!489 = !DISubrange(count: 9)
!490 = !DIGlobalVariableExpression(var: !491, expr: !DIExpression())
!491 = distinct !DIGlobalVariable(name: "rankoffsets", scope: !468, file: !344, line: 386, type: !492, isLocal: true, isDefinition: true)
!492 = !DICompositeType(tag: DW_TAG_array_type, baseType: !112, size: 256, elements: !46)
!493 = !{!494, !495, !496, !497, !498, !499, !500, !501, !502, !503, !504, !505, !506, !507, !508, !509}
!494 = !DILocalVariable(name: "g", arg: 1, scope: !343, file: !344, line: 23, type: !347)
!495 = !DILocalVariable(name: "s", arg: 2, scope: !343, file: !344, line: 23, type: !391)
!496 = !DILocalVariable(name: "inbuff", arg: 3, scope: !343, file: !344, line: 23, type: !153)
!497 = !DILocalVariable(name: "stage", scope: !343, file: !344, line: 37, type: !113)
!498 = !DILocalVariable(name: "i", scope: !343, file: !344, line: 39, type: !113)
!499 = !DILocalVariable(name: "j", scope: !343, file: !344, line: 40, type: !113)
!500 = !DILocalVariable(name: "curr_rank", scope: !343, file: !344, line: 41, type: !113)
!501 = !DILocalVariable(name: "fileoffset", scope: !343, file: !344, line: 42, type: !113)
!502 = !DILocalVariable(name: "rankoffset", scope: !343, file: !344, line: 43, type: !113)
!503 = !DILocalVariable(name: "castlefile", scope: !343, file: !344, line: 44, type: !113)
!504 = !DILocalVariable(name: "castlesq", scope: !343, file: !344, line: 45, type: !113)
!505 = !DILocalVariable(name: "ep_file", scope: !343, file: !344, line: 47, type: !113)
!506 = !DILocalVariable(name: "ep_rank", scope: !343, file: !344, line: 47, type: !113)
!507 = !DILocalVariable(name: "norm_file", scope: !343, file: !344, line: 47, type: !113)
!508 = !DILocalVariable(name: "norm_rank", scope: !343, file: !344, line: 47, type: !113)
!509 = !DILocalVariable(name: "foundflags", scope: !343, file: !344, line: 48, type: !113)
!510 = !DIGlobalVariableExpression(var: !511, expr: !DIExpression())
!511 = distinct !DIGlobalVariable(name: "w_passer", linkageName: "_ZL8w_passer", scope: !512, file: !513, line: 39, type: !520, isLocal: true, isDefinition: true)
!512 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !513, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !514, imports: !116, nameTableKind: None)
!513 = !DIFile(filename: "neval.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!514 = !{!515, !510, !518, !523, !525, !527, !529}
!515 = !DIGlobalVariableExpression(var: !516, expr: !DIExpression())
!516 = distinct !DIGlobalVariable(name: "material", scope: !512, file: !513, line: 30, type: !517, isLocal: false, isDefinition: true)
!517 = !DICompositeType(tag: DW_TAG_array_type, baseType: !112, size: 448, elements: !483)
!518 = !DIGlobalVariableExpression(var: !519, expr: !DIExpression())
!519 = distinct !DIGlobalVariable(name: "w_passer_pawn_supported", linkageName: "_ZL23w_passer_pawn_supported", scope: !512, file: !513, line: 59, type: !520, isLocal: true, isDefinition: true)
!520 = !DICompositeType(tag: DW_TAG_array_type, baseType: !112, size: 192, elements: !521)
!521 = !{!522}
!522 = !DISubrange(count: 6)
!523 = !DIGlobalVariableExpression(var: !524, expr: !DIExpression())
!524 = distinct !DIGlobalVariable(name: "w_passer_king_supported", linkageName: "_ZL23w_passer_king_supported", scope: !512, file: !513, line: 51, type: !520, isLocal: true, isDefinition: true)
!525 = !DIGlobalVariableExpression(var: !526, expr: !DIExpression())
!526 = distinct !DIGlobalVariable(name: "w_passer_free", linkageName: "_ZL13w_passer_free", scope: !512, file: !513, line: 43, type: !520, isLocal: true, isDefinition: true)
!527 = !DIGlobalVariableExpression(var: !528, expr: !DIExpression())
!528 = distinct !DIGlobalVariable(name: "w_passer_very_free", linkageName: "_ZL18w_passer_very_free", scope: !512, file: !513, line: 47, type: !520, isLocal: true, isDefinition: true)
!529 = !DIGlobalVariableExpression(var: !530, expr: !DIExpression())
!530 = distinct !DIGlobalVariable(name: "w_passer_blocked", linkageName: "_ZL16w_passer_blocked", scope: !512, file: !513, line: 55, type: !520, isLocal: true, isDefinition: true)
!531 = !DIGlobalVariableExpression(var: !532, expr: !DIExpression())
!532 = distinct !DIGlobalVariable(name: "PawnTT", linkageName: "_ZL6PawnTT", scope: !533, file: !534, line: 30, type: !539, isLocal: true, isDefinition: true)
!533 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !534, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !535, globals: !536, nameTableKind: None)
!534 = !DIFile(filename: "pawn.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!535 = !{!10}
!536 = !{!531, !537}
!537 = !DIGlobalVariableExpression(var: !538, expr: !DIExpression())
!538 = distinct !DIGlobalVariable(name: "w_candidate", linkageName: "_ZL11w_candidate", scope: !533, file: !534, line: 23, type: !520, isLocal: true, isDefinition: true)
!539 = !DICompositeType(tag: DW_TAG_array_type, baseType: !540, size: 92274688, elements: !555)
!540 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pawntt_t", file: !541, line: 7, size: 704, flags: DIFlagTypePassByValue, elements: !542, identifier: "_ZTS8pawntt_t")
!541 = !DIFile(filename: "./pawn.h", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!542 = !{!543, !544, !545, !546, !547, !548, !549, !550, !551, !552, !553, !554}
!543 = !DIDerivedType(tag: DW_TAG_member, name: "pawnhash", scope: !540, file: !541, line: 8, baseType: !6, size: 64)
!544 = !DIDerivedType(tag: DW_TAG_member, name: "open_files", scope: !540, file: !541, line: 9, baseType: !6, size: 64, offset: 64)
!545 = !DIDerivedType(tag: DW_TAG_member, name: "w_half_open_files", scope: !540, file: !541, line: 10, baseType: !6, size: 64, offset: 128)
!546 = !DIDerivedType(tag: DW_TAG_member, name: "b_half_open_files", scope: !540, file: !541, line: 11, baseType: !6, size: 64, offset: 192)
!547 = !DIDerivedType(tag: DW_TAG_member, name: "w_passed", scope: !540, file: !541, line: 12, baseType: !6, size: 64, offset: 256)
!548 = !DIDerivedType(tag: DW_TAG_member, name: "b_passed", scope: !540, file: !541, line: 13, baseType: !6, size: 64, offset: 320)
!549 = !DIDerivedType(tag: DW_TAG_member, name: "w_strong_square", scope: !540, file: !541, line: 14, baseType: !6, size: 64, offset: 384)
!550 = !DIDerivedType(tag: DW_TAG_member, name: "b_strong_square", scope: !540, file: !541, line: 15, baseType: !6, size: 64, offset: 448)
!551 = !DIDerivedType(tag: DW_TAG_member, name: "w_super_strong_square", scope: !540, file: !541, line: 16, baseType: !6, size: 64, offset: 512)
!552 = !DIDerivedType(tag: DW_TAG_member, name: "b_super_strong_square", scope: !540, file: !541, line: 17, baseType: !6, size: 64, offset: 576)
!553 = !DIDerivedType(tag: DW_TAG_member, name: "w_score", scope: !540, file: !541, line: 19, baseType: !113, size: 32, offset: 640)
!554 = !DIDerivedType(tag: DW_TAG_member, name: "b_score", scope: !540, file: !541, line: 20, baseType: !113, size: 32, offset: 672)
!555 = !{!31, !556}
!556 = !DISubrange(count: 16384)
!557 = !DIGlobalVariableExpression(var: !558, expr: !DIExpression())
!558 = distinct !DIGlobalVariable(name: "psq_king_nopawn", linkageName: "_ZL15psq_king_nopawn", scope: !559, file: !560, line: 39, type: !111, isLocal: true, isDefinition: true)
!559 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !560, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !561, nameTableKind: None)
!560 = !DIFile(filename: "preproc.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!561 = !{!562, !568, !570, !572, !574, !557, !576, !578, !580, !582, !584}
!562 = !DIGlobalVariableExpression(var: !563, expr: !DIExpression())
!563 = distinct !DIGlobalVariable(name: "psq_table", scope: !559, file: !560, line: 20, type: !564, isLocal: false, isDefinition: true)
!564 = !DICompositeType(tag: DW_TAG_array_type, baseType: !565, size: 6144, elements: !566)
!565 = !DIBasicType(name: "signed char", size: 8, encoding: DW_ATE_signed_char)
!566 = !{!567, !16}
!567 = !DISubrange(count: 12)
!568 = !DIGlobalVariableExpression(var: !569, expr: !DIExpression())
!569 = distinct !DIGlobalVariable(name: "flip", scope: !559, file: !560, line: 25, type: !111, isLocal: false, isDefinition: true)
!570 = !DIGlobalVariableExpression(var: !571, expr: !DIExpression())
!571 = distinct !DIGlobalVariable(name: "wking_psq_end", linkageName: "_ZL13wking_psq_end", scope: !559, file: !560, line: 83, type: !111, isLocal: true, isDefinition: true)
!572 = !DIGlobalVariableExpression(var: !573, expr: !DIExpression())
!573 = distinct !DIGlobalVariable(name: "psq_king_kingside", linkageName: "_ZL17psq_king_kingside", scope: !559, file: !560, line: 50, type: !111, isLocal: true, isDefinition: true)
!574 = !DIGlobalVariableExpression(var: !575, expr: !DIExpression())
!575 = distinct !DIGlobalVariable(name: "psq_king_queenside", linkageName: "_ZL18psq_king_queenside", scope: !559, file: !560, line: 61, type: !111, isLocal: true, isDefinition: true)
!576 = !DIGlobalVariableExpression(var: !577, expr: !DIExpression())
!577 = distinct !DIGlobalVariable(name: "wpawn_psq", linkageName: "_ZL9wpawn_psq", scope: !559, file: !560, line: 127, type: !111, isLocal: true, isDefinition: true)
!578 = !DIGlobalVariableExpression(var: !579, expr: !DIExpression())
!579 = distinct !DIGlobalVariable(name: "wknight_psq_end", linkageName: "_ZL15wknight_psq_end", scope: !559, file: !560, line: 72, type: !111, isLocal: true, isDefinition: true)
!580 = !DIGlobalVariableExpression(var: !581, expr: !DIExpression())
!581 = distinct !DIGlobalVariable(name: "wbishop_psq_end", linkageName: "_ZL15wbishop_psq_end", scope: !559, file: !560, line: 105, type: !111, isLocal: true, isDefinition: true)
!582 = !DIGlobalVariableExpression(var: !583, expr: !DIExpression())
!583 = distinct !DIGlobalVariable(name: "wrook_psq_end", linkageName: "_ZL13wrook_psq_end", scope: !559, file: !560, line: 94, type: !111, isLocal: true, isDefinition: true)
!584 = !DIGlobalVariableExpression(var: !585, expr: !DIExpression())
!585 = distinct !DIGlobalVariable(name: "wqueen_psq_end", linkageName: "_ZL14wqueen_psq_end", scope: !559, file: !560, line: 116, type: !111, isLocal: true, isDefinition: true)
!586 = !DIGlobalVariableExpression(var: !587, expr: !DIExpression())
!587 = distinct !DIGlobalVariable(name: "history_hit", scope: !588, file: !589, line: 41, type: !594, isLocal: false, isDefinition: true)
!588 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !589, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !590, globals: !591, imports: !116, nameTableKind: None)
!589 = !DIFile(filename: "search.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!590 = !{!6, !113, !305}
!591 = !{!592, !586, !596, !598, !639, !641, !666}
!592 = !DIGlobalVariableExpression(var: !593, expr: !DIExpression())
!593 = distinct !DIGlobalVariable(name: "history_h", scope: !588, file: !589, line: 40, type: !594, isLocal: false, isDefinition: true)
!594 = !DICompositeType(tag: DW_TAG_array_type, baseType: !113, size: 196608, elements: !595)
!595 = !{!31, !567, !16}
!596 = !DIGlobalVariableExpression(var: !597, expr: !DIExpression())
!597 = distinct !DIGlobalVariable(name: "history_tot", scope: !588, file: !589, line: 42, type: !594, isLocal: false, isDefinition: true)
!598 = !DIGlobalVariableExpression(var: !599, expr: !DIExpression())
!599 = distinct !DIGlobalVariable(name: "changes", scope: !600, file: !589, line: 1328, type: !113, isLocal: true, isDefinition: true)
!600 = distinct !DISubprogram(name: "search_root", linkageName: "_Z11search_rootP7state_tiii", scope: !589, file: !589, line: 1316, type: !601, scopeLine: 1316, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !588, retainedNodes: !603)
!601 = !DISubroutineType(types: !602)
!602 = !{!369, !391, !113, !113, !113}
!603 = !{!604, !605, !606, !607, !608, !612, !613, !614, !615, !616, !618, !619, !620, !621, !622, !623, !624, !625, !626, !627, !628, !629, !630, !634, !635}
!604 = !DILocalVariable(name: "s", arg: 1, scope: !600, file: !589, line: 1316, type: !391)
!605 = !DILocalVariable(name: "originalalpha", arg: 2, scope: !600, file: !589, line: 1316, type: !113)
!606 = !DILocalVariable(name: "originalbeta", arg: 3, scope: !600, file: !589, line: 1316, type: !113)
!607 = !DILocalVariable(name: "depth", arg: 4, scope: !600, file: !589, line: 1316, type: !113)
!608 = !DILocalVariable(name: "moves", scope: !600, file: !589, line: 1317, type: !609)
!609 = !DICompositeType(tag: DW_TAG_array_type, baseType: !369, size: 7680, elements: !610)
!610 = !{!611}
!611 = !DISubrange(count: 240)
!612 = !DILocalVariable(name: "best_move", scope: !600, file: !589, line: 1317, type: !369)
!613 = !DILocalVariable(name: "num_moves", scope: !600, file: !589, line: 1318, type: !113)
!614 = !DILocalVariable(name: "i", scope: !600, file: !589, line: 1318, type: !113)
!615 = !DILocalVariable(name: "root_score", scope: !600, file: !589, line: 1319, type: !113)
!616 = !DILocalVariable(name: "move_ordering", scope: !600, file: !589, line: 1319, type: !617)
!617 = !DICompositeType(tag: DW_TAG_array_type, baseType: !113, size: 7680, elements: !610)
!618 = !DILocalVariable(name: "no_moves", scope: !600, file: !589, line: 1320, type: !113)
!619 = !DILocalVariable(name: "legal_move", scope: !600, file: !589, line: 1320, type: !113)
!620 = !DILocalVariable(name: "first", scope: !600, file: !589, line: 1320, type: !113)
!621 = !DILocalVariable(name: "alpha", scope: !600, file: !589, line: 1321, type: !113)
!622 = !DILocalVariable(name: "beta", scope: !600, file: !589, line: 1321, type: !113)
!623 = !DILocalVariable(name: "dummy", scope: !600, file: !589, line: 1322, type: !113)
!624 = !DILocalVariable(name: "dummy2", scope: !600, file: !589, line: 1323, type: !10)
!625 = !DILocalVariable(name: "incheck", scope: !600, file: !589, line: 1324, type: !113)
!626 = !DILocalVariable(name: "mc", scope: !600, file: !589, line: 1325, type: !113)
!627 = !DILocalVariable(name: "oldnodecount", scope: !600, file: !589, line: 1326, type: !6)
!628 = !DILocalVariable(name: "extend", scope: !600, file: !589, line: 1327, type: !113)
!629 = !DILocalVariable(name: "huber", scope: !600, file: !589, line: 1327, type: !113)
!630 = !DILocalVariable(name: "searching_move", scope: !600, file: !589, line: 1330, type: !631)
!631 = !DICompositeType(tag: DW_TAG_array_type, baseType: !155, size: 4096, elements: !632)
!632 = !{!633}
!633 = !DISubrange(count: 512)
!634 = !DILocalVariable(name: "movetotal", scope: !600, file: !589, line: 1358, type: !113)
!635 = !DILocalVariable(name: "moveleft", scope: !636, file: !589, line: 1389, type: !113)
!636 = distinct !DILexicalBlock(scope: !637, file: !589, line: 1385, column: 39)
!637 = distinct !DILexicalBlock(scope: !638, file: !589, line: 1385, column: 13)
!638 = distinct !DILexicalBlock(scope: !600, file: !589, line: 1374, column: 66)
!639 = !DIGlobalVariableExpression(var: !640, expr: !DIExpression())
!640 = distinct !DIGlobalVariable(name: "bmove", scope: !600, file: !589, line: 1329, type: !369, isLocal: true, isDefinition: true)
!641 = !DIGlobalVariableExpression(var: !642, expr: !DIExpression())
!642 = distinct !DIGlobalVariable(name: "lastsearchscore", scope: !643, file: !589, line: 1638, type: !113, isLocal: true, isDefinition: true)
!643 = distinct !DISubprogram(name: "think", linkageName: "_Z5thinkP11gamestate_tP7state_t", scope: !589, file: !589, line: 1629, type: !644, scopeLine: 1629, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !588, retainedNodes: !646)
!644 = !DISubroutineType(types: !645)
!645 = !{!369, !347, !391}
!646 = !{!647, !648, !649, !650, !651, !652, !653, !654, !655, !656, !657, !658, !659, !660, !661, !662, !663, !664, !665}
!647 = !DILocalVariable(name: "g", arg: 1, scope: !643, file: !589, line: 1629, type: !347)
!648 = !DILocalVariable(name: "s", arg: 2, scope: !643, file: !589, line: 1629, type: !391)
!649 = !DILocalVariable(name: "comp_move", scope: !643, file: !589, line: 1630, type: !369)
!650 = !DILocalVariable(name: "temp_move", scope: !643, file: !589, line: 1630, type: !369)
!651 = !DILocalVariable(name: "elapsed", scope: !643, file: !589, line: 1631, type: !113)
!652 = !DILocalVariable(name: "temp_score", scope: !643, file: !589, line: 1631, type: !113)
!653 = !DILocalVariable(name: "output", scope: !643, file: !589, line: 1632, type: !631)
!654 = !DILocalVariable(name: "output2", scope: !643, file: !589, line: 1632, type: !631)
!655 = !DILocalVariable(name: "alpha", scope: !643, file: !589, line: 1633, type: !113)
!656 = !DILocalVariable(name: "beta", scope: !643, file: !589, line: 1633, type: !113)
!657 = !DILocalVariable(name: "rs", scope: !643, file: !589, line: 1634, type: !113)
!658 = !DILocalVariable(name: "moves", scope: !643, file: !589, line: 1635, type: !609)
!659 = !DILocalVariable(name: "l", scope: !643, file: !589, line: 1636, type: !113)
!660 = !DILocalVariable(name: "lastlegal", scope: !643, file: !589, line: 1636, type: !113)
!661 = !DILocalVariable(name: "ic", scope: !643, file: !589, line: 1636, type: !113)
!662 = !DILocalVariable(name: "num_moves", scope: !643, file: !589, line: 1637, type: !113)
!663 = !DILocalVariable(name: "true_i_depth", scope: !643, file: !589, line: 1639, type: !113)
!664 = !DILocalVariable(name: "pondermove", scope: !643, file: !589, line: 1640, type: !369)
!665 = !DILocalVariable(name: "legals", scope: !643, file: !589, line: 1651, type: !113)
!666 = !DIGlobalVariableExpression(var: !667, expr: !DIExpression())
!667 = distinct !DIGlobalVariable(name: "rc_index", linkageName: "_ZL8rc_index", scope: !588, file: !589, line: 36, type: !517, isLocal: true, isDefinition: true)
!668 = !DIGlobalVariableExpression(var: !669, expr: !DIExpression())
!669 = distinct !DIGlobalVariable(name: "contempt", scope: !670, file: !671, line: 38, type: !113, isLocal: false, isDefinition: true)
!670 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !671, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !672, nameTableKind: None)
!671 = !DIFile(filename: "sjeng.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!672 = !{!673, !679, !681, !683, !685, !687, !689, !691, !693, !695, !697, !699, !701, !703, !705, !668, !707, !709, !711, !713}
!673 = !DIGlobalVariableExpression(var: !674, expr: !DIExpression())
!674 = distinct !DIGlobalVariable(name: "buffered_command", scope: !670, file: !671, line: 25, type: !675, isLocal: false, isDefinition: true)
!675 = !DICompositeType(tag: DW_TAG_array_type, baseType: !155, size: 1310720, elements: !676)
!676 = !{!677, !678}
!677 = !DISubrange(count: 20)
!678 = !DISubrange(count: 8192)
!679 = !DIGlobalVariableExpression(var: !680, expr: !DIExpression())
!680 = distinct !DIGlobalVariable(name: "buffered_count", scope: !670, file: !671, line: 26, type: !113, isLocal: false, isDefinition: true)
!681 = !DIGlobalVariableExpression(var: !682, expr: !DIExpression())
!682 = distinct !DIGlobalVariable(name: "is_pondering", scope: !670, file: !671, line: 27, type: !113, isLocal: false, isDefinition: true)
!683 = !DIGlobalVariableExpression(var: !684, expr: !DIExpression())
!684 = distinct !DIGlobalVariable(name: "allow_pondering", scope: !670, file: !671, line: 27, type: !113, isLocal: false, isDefinition: true)
!685 = !DIGlobalVariableExpression(var: !686, expr: !DIExpression())
!686 = distinct !DIGlobalVariable(name: "is_analyzing", scope: !670, file: !671, line: 27, type: !113, isLocal: false, isDefinition: true)
!687 = !DIGlobalVariableExpression(var: !688, expr: !DIExpression())
!688 = distinct !DIGlobalVariable(name: "TTSize", scope: !670, file: !671, line: 28, type: !10, isLocal: false, isDefinition: true)
!689 = !DIGlobalVariableExpression(var: !690, expr: !DIExpression())
!690 = distinct !DIGlobalVariable(name: "uci_mode", scope: !670, file: !671, line: 29, type: !113, isLocal: false, isDefinition: true)
!691 = !DIGlobalVariableExpression(var: !692, expr: !DIExpression())
!692 = distinct !DIGlobalVariable(name: "uci_chess960_mode", scope: !670, file: !671, line: 30, type: !113, isLocal: false, isDefinition: true)
!693 = !DIGlobalVariableExpression(var: !694, expr: !DIExpression())
!694 = distinct !DIGlobalVariable(name: "uci_showcurrline", scope: !670, file: !671, line: 31, type: !113, isLocal: false, isDefinition: true)
!695 = !DIGlobalVariableExpression(var: !696, expr: !DIExpression())
!696 = distinct !DIGlobalVariable(name: "uci_showrefutations", scope: !670, file: !671, line: 32, type: !113, isLocal: false, isDefinition: true)
!697 = !DIGlobalVariableExpression(var: !698, expr: !DIExpression())
!698 = distinct !DIGlobalVariable(name: "uci_limitstrength", scope: !670, file: !671, line: 33, type: !113, isLocal: false, isDefinition: true)
!699 = !DIGlobalVariableExpression(var: !700, expr: !DIExpression())
!700 = distinct !DIGlobalVariable(name: "uci_elo", scope: !670, file: !671, line: 34, type: !113, isLocal: false, isDefinition: true)
!701 = !DIGlobalVariableExpression(var: !702, expr: !DIExpression())
!702 = distinct !DIGlobalVariable(name: "uci_multipv", scope: !670, file: !671, line: 35, type: !113, isLocal: false, isDefinition: true)
!703 = !DIGlobalVariableExpression(var: !704, expr: !DIExpression())
!704 = distinct !DIGlobalVariable(name: "cfg_logging", scope: !670, file: !671, line: 36, type: !113, isLocal: false, isDefinition: true)
!705 = !DIGlobalVariableExpression(var: !706, expr: !DIExpression())
!706 = distinct !DIGlobalVariable(name: "cfg_logfile", scope: !670, file: !671, line: 37, type: !631, isLocal: false, isDefinition: true)
!707 = !DIGlobalVariableExpression(var: !708, expr: !DIExpression())
!708 = distinct !DIGlobalVariable(name: "time_check_log", scope: !670, file: !671, line: 39, type: !113, isLocal: false, isDefinition: true)
!709 = !DIGlobalVariableExpression(var: !710, expr: !DIExpression())
!710 = distinct !DIGlobalVariable(name: "global_id", scope: !670, file: !671, line: 40, type: !113, isLocal: false, isDefinition: true)
!711 = !DIGlobalVariableExpression(var: !712, expr: !DIExpression())
!712 = distinct !DIGlobalVariable(name: "EGTBHits", scope: !670, file: !671, line: 41, type: !113, isLocal: false, isDefinition: true)
!713 = !DIGlobalVariableExpression(var: !714, expr: !DIExpression())
!714 = distinct !DIGlobalVariable(name: "EGTBProbes", scope: !670, file: !671, line: 42, type: !113, isLocal: false, isDefinition: true)
!715 = !DIGlobalVariableExpression(var: !716, expr: !DIExpression())
!716 = distinct !DIGlobalVariable(name: "state", scope: !717, file: !718, line: 16, type: !392, isLocal: false, isDefinition: true)
!717 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !718, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !719, nameTableKind: None)
!718 = !DIFile(filename: "state.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!719 = !{!715, !720, !722}
!720 = !DIGlobalVariableExpression(var: !721, expr: !DIExpression())
!721 = distinct !DIGlobalVariable(name: "gamestate", scope: !717, file: !718, line: 17, type: !348, isLocal: false, isDefinition: true)
!722 = !DIGlobalVariableExpression(var: !723, expr: !DIExpression())
!723 = distinct !DIGlobalVariable(name: "scoreboard", scope: !717, file: !718, line: 18, type: !724, isLocal: false, isDefinition: true)
!724 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "scoreboard_t", file: !349, line: 96, size: 794688, flags: DIFlagTypePassByValue, elements: !725, identifier: "_ZTS12scoreboard_t")
!725 = !{!726, !727, !729, !736, !738}
!726 = !DIDerivedType(tag: DW_TAG_member, name: "cpus", scope: !724, file: !349, line: 97, baseType: !113, size: 32)
!727 = !DIDerivedType(tag: DW_TAG_member, name: "running", scope: !724, file: !349, line: 98, baseType: !728, size: 32, offset: 32)
!728 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !113)
!729 = !DIDerivedType(tag: DW_TAG_member, name: "searchstart", scope: !724, file: !349, line: 103, baseType: !730, size: 768, offset: 64)
!730 = !DICompositeType(tag: DW_TAG_array_type, baseType: !731, size: 768, elements: !46)
!731 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !724, file: !349, line: 99, size: 96, flags: DIFlagTypePassByValue, elements: !732, identifier: "_ZTSN12scoreboard_tUt_E")
!732 = !{!733, !734, !735}
!733 = !DIDerivedType(tag: DW_TAG_member, name: "alpha", scope: !731, file: !349, line: 100, baseType: !113, size: 32)
!734 = !DIDerivedType(tag: DW_TAG_member, name: "beta", scope: !731, file: !349, line: 101, baseType: !113, size: 32, offset: 32)
!735 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !731, file: !349, line: 102, baseType: !113, size: 32, offset: 64)
!736 = !DIDerivedType(tag: DW_TAG_member, name: "score", scope: !724, file: !349, line: 104, baseType: !737, size: 256, offset: 832)
!737 = !DICompositeType(tag: DW_TAG_array_type, baseType: !728, size: 256, elements: !46)
!738 = !DIDerivedType(tag: DW_TAG_member, name: "threadstate", scope: !724, file: !349, line: 105, baseType: !739, size: 793600, offset: 1088)
!739 = !DICompositeType(tag: DW_TAG_array_type, baseType: !392, size: 793600, elements: !46)
!740 = !DIGlobalVariableExpression(var: !741, expr: !DIExpression())
!741 = distinct !DIGlobalVariable(name: "TTable", scope: !742, file: !743, line: 19, type: !745, isLocal: false, isDefinition: true)
!742 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !743, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !744, globals: !763, nameTableKind: None)
!743 = !DIFile(filename: "ttable.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!744 = !{!6, !10, !113, !745, !171}
!745 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !746, size: 64)
!746 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ttentry_t", file: !747, line: 22, size: 384, flags: DIFlagTypePassByValue, elements: !748, identifier: "_ZTS9ttentry_t")
!747 = !DIFile(filename: "./ttable.h", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!748 = !{!749}
!749 = !DIDerivedType(tag: DW_TAG_member, name: "buckets", scope: !746, file: !747, line: 23, baseType: !750, size: 384)
!750 = !DICompositeType(tag: DW_TAG_array_type, baseType: !751, size: 384, elements: !66)
!751 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ttbucket_t", file: !747, line: 10, size: 96, flags: DIFlagTypePassByValue, elements: !752, identifier: "_ZTS10ttbucket_t")
!752 = !{!753, !754, !756, !757, !758, !759, !760, !761, !762}
!753 = !DIDerivedType(tag: DW_TAG_member, name: "hash", scope: !751, file: !747, line: 11, baseType: !10, size: 32)
!754 = !DIDerivedType(tag: DW_TAG_member, name: "bound", scope: !751, file: !747, line: 12, baseType: !755, size: 16, offset: 32)
!755 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!756 = !DIDerivedType(tag: DW_TAG_member, name: "bestmove", scope: !751, file: !747, line: 13, baseType: !464, size: 16, offset: 48)
!757 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !751, file: !747, line: 14, baseType: !9, size: 8, offset: 64)
!758 = !DIDerivedType(tag: DW_TAG_member, name: "threat", scope: !751, file: !747, line: 15, baseType: !9, size: 1, offset: 72, flags: DIFlagBitField, extraData: i64 72)
!759 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !751, file: !747, line: 16, baseType: !9, size: 2, offset: 73, flags: DIFlagBitField, extraData: i64 72)
!760 = !DIDerivedType(tag: DW_TAG_member, name: "singular", scope: !751, file: !747, line: 17, baseType: !9, size: 1, offset: 75, flags: DIFlagBitField, extraData: i64 72)
!761 = !DIDerivedType(tag: DW_TAG_member, name: "nosingular", scope: !751, file: !747, line: 18, baseType: !9, size: 1, offset: 76, flags: DIFlagBitField, extraData: i64 72)
!762 = !DIDerivedType(tag: DW_TAG_member, name: "age", scope: !751, file: !747, line: 19, baseType: !9, size: 2, offset: 77, flags: DIFlagBitField, extraData: i64 72)
!763 = !{!764, !740, !768}
!764 = !DIGlobalVariableExpression(var: !765, expr: !DIExpression())
!765 = distinct !DIGlobalVariable(name: "zobrist", scope: !742, file: !743, line: 17, type: !766, isLocal: false, isDefinition: true)
!766 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 57344, elements: !767)
!767 = !{!484, !16}
!768 = !DIGlobalVariableExpression(var: !769, expr: !DIExpression())
!769 = distinct !DIGlobalVariable(name: "TTAge", scope: !742, file: !743, line: 20, type: !10, isLocal: false, isDefinition: true)
!770 = !DIGlobalVariableExpression(var: !771, expr: !DIExpression())
!771 = distinct !DIGlobalVariable(name: "init_board", scope: !772, file: !773, line: 401, type: !111, isLocal: true, isDefinition: true)
!772 = distinct !DISubprogram(name: "init_game", linkageName: "_Z9init_gameP11gamestate_tP7state_t", scope: !773, file: !773, line: 399, type: !774, scopeLine: 399, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !776, retainedNodes: !813)
!773 = !DIFile(filename: "utils.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!774 = !DISubroutineType(types: !775)
!775 = !{null, !347, !391}
!776 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !773, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !447, retainedTypes: !777, globals: !778, nameTableKind: None)
!777 = !{!152, !113, !464}
!778 = !{!779, !781, !785, !770, !787, !807, !809, !811}
!779 = !DIGlobalVariableExpression(var: !780, expr: !DIExpression())
!780 = distinct !DIGlobalVariable(name: "root_scores", scope: !776, file: !773, line: 31, type: !617, isLocal: false, isDefinition: true)
!781 = !DIGlobalVariableExpression(var: !782, expr: !DIExpression())
!782 = distinct !DIGlobalVariable(name: "multipv_strings", scope: !776, file: !773, line: 32, type: !783, isLocal: false, isDefinition: true)
!783 = !DICompositeType(tag: DW_TAG_array_type, baseType: !155, size: 983040, elements: !784)
!784 = !{!611, !633}
!785 = !DIGlobalVariableExpression(var: !786, expr: !DIExpression())
!786 = distinct !DIGlobalVariable(name: "multipv_scores", scope: !776, file: !773, line: 33, type: !617, isLocal: false, isDefinition: true)
!787 = !DIGlobalVariableExpression(var: !788, expr: !DIExpression())
!788 = distinct !DIGlobalVariable(name: "levelstack", scope: !789, file: !773, line: 477, type: !804, isLocal: true, isDefinition: true)
!789 = distinct !DISubprogram(name: "hash_extract_pv", linkageName: "_ZL15hash_extract_pvP7state_tiPc", scope: !773, file: !773, line: 471, type: !790, scopeLine: 471, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !776, retainedNodes: !792)
!790 = !DISubroutineType(types: !791)
!791 = !{null, !391, !113, !198}
!792 = !{!793, !794, !795, !796, !797, !798, !799, !800, !801, !802, !803}
!793 = !DILocalVariable(name: "s", arg: 1, scope: !789, file: !773, line: 471, type: !391)
!794 = !DILocalVariable(name: "level", arg: 2, scope: !789, file: !773, line: 471, type: !113)
!795 = !DILocalVariable(name: "str", arg: 3, scope: !789, file: !773, line: 471, type: !198)
!796 = !DILocalVariable(name: "xdummy", scope: !789, file: !773, line: 472, type: !113)
!797 = !DILocalVariable(name: "bm", scope: !789, file: !773, line: 472, type: !113)
!798 = !DILocalVariable(name: "j", scope: !789, file: !773, line: 472, type: !113)
!799 = !DILocalVariable(name: "mv", scope: !789, file: !773, line: 473, type: !10)
!800 = !DILocalVariable(name: "moves", scope: !789, file: !773, line: 474, type: !609)
!801 = !DILocalVariable(name: "num_moves", scope: !789, file: !773, line: 475, type: !113)
!802 = !DILocalVariable(name: "output", scope: !789, file: !773, line: 476, type: !631)
!803 = !DILocalVariable(name: "i", scope: !789, file: !773, line: 478, type: !113)
!804 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 4160, elements: !805)
!805 = !{!806}
!806 = !DISubrange(count: 65)
!807 = !DIGlobalVariableExpression(var: !808, expr: !DIExpression())
!808 = distinct !DIGlobalVariable(name: "s1", linkageName: "_ZL2s1", scope: !776, file: !773, line: 28, type: !10, isLocal: true, isDefinition: true)
!809 = !DIGlobalVariableExpression(var: !810, expr: !DIExpression())
!810 = distinct !DIGlobalVariable(name: "s2", linkageName: "_ZL2s2", scope: !776, file: !773, line: 28, type: !10, isLocal: true, isDefinition: true)
!811 = !DIGlobalVariableExpression(var: !812, expr: !DIExpression())
!812 = distinct !DIGlobalVariable(name: "s3", linkageName: "_ZL2s3", scope: !776, file: !773, line: 28, type: !10, isLocal: true, isDefinition: true)
!813 = !{!814, !815}
!814 = !DILocalVariable(name: "g", arg: 1, scope: !772, file: !773, line: 399, type: !347)
!815 = !DILocalVariable(name: "s", arg: 2, scope: !772, file: !773, line: 399, type: !391)
!816 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !817, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, nameTableKind: None)
!817 = !DIFile(filename: "attacks.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!818 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !819, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, nameTableKind: None)
!819 = !DIFile(filename: "board.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!820 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !821, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, nameTableKind: None)
!821 = !DIFile(filename: "draw.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!822 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !823, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, nameTableKind: None)
!823 = !DIFile(filename: "endgame.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!824 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !825, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !826, nameTableKind: None)
!825 = !DIFile(filename: "generate.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!826 = !{!113, !6}
!827 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !828, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, nameTableKind: None)
!828 = !DIFile(filename: "initp.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!829 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !830, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, nameTableKind: None)
!830 = !DIFile(filename: "make.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!831 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !832, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, nameTableKind: None)
!832 = !DIFile(filename: "moves.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!833 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !834, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, nameTableKind: None)
!834 = !DIFile(filename: "see.cpp", directory: "/export/iusers/cczhao/Workspaces/DebugTests-XMAIN/JIRA-11388/531.exp.g/benchspec/CPU/531.deepsjeng_r/build/build_base_core_avx512.0000")
!835 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!836 = !{i32 7, !"Dwarf Version", i32 4}
!837 = !{i32 2, !"Debug Info Version", i32 3}
!838 = !{i32 1, !"wchar_size", i32 4}
!839 = !{i32 1, !"ThinLTO", i32 0}
!840 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!841 = !{i32 1, !"LTOPostLink", i32 1}
!842 = distinct !DISubprogram(name: "ProbeTT", linkageName: "_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i", scope: !743, file: !743, line: 173, type: !843, scopeLine: 176, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !742, retainedNodes: !846)
!843 = !DISubroutineType(types: !844)
!844 = !{!113, !391, !481, !113, !113, !845, !481, !481, !481, !481, !112}
!845 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!846 = !{!847, !848, !849, !850, !851, !852, !853, !854, !855, !856, !857, !858, !859, !860, !861, !862}
!847 = !DILocalVariable(name: "s", arg: 1, scope: !842, file: !743, line: 173, type: !391)
!848 = !DILocalVariable(name: "score", arg: 2, scope: !842, file: !743, line: 174, type: !481)
!849 = !DILocalVariable(name: "alpha", arg: 3, scope: !842, file: !743, line: 174, type: !113)
!850 = !DILocalVariable(name: "beta", arg: 4, scope: !842, file: !743, line: 174, type: !113)
!851 = !DILocalVariable(name: "best", arg: 5, scope: !842, file: !743, line: 175, type: !845)
!852 = !DILocalVariable(name: "threat", arg: 6, scope: !842, file: !743, line: 175, type: !481)
!853 = !DILocalVariable(name: "donull", arg: 7, scope: !842, file: !743, line: 175, type: !481)
!854 = !DILocalVariable(name: "singular", arg: 8, scope: !842, file: !743, line: 175, type: !481)
!855 = !DILocalVariable(name: "nosingular", arg: 9, scope: !842, file: !743, line: 176, type: !481)
!856 = !DILocalVariable(name: "depth", arg: 10, scope: !842, file: !743, line: 176, type: !112)
!857 = !DILocalVariable(name: "type", scope: !842, file: !743, line: 177, type: !113)
!858 = !DILocalVariable(name: "i", scope: !842, file: !743, line: 178, type: !113)
!859 = !DILocalVariable(name: "nhash", scope: !842, file: !743, line: 179, type: !6)
!860 = !DILocalVariable(name: "index", scope: !842, file: !743, line: 180, type: !10)
!861 = !DILocalVariable(name: "temp", scope: !842, file: !743, line: 181, type: !745)
!862 = !DILocalVariable(name: "entry", scope: !842, file: !743, line: 182, type: !863)
!863 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !751, size: 64)
!864 = !DILocation(line: 0, scope: !842)
!865 = !DILocation(line: 184, column: 13, scope: !842)
!866 = !{!867, !867, i64 0}
!867 = !{!"int", !868, i64 0}
!868 = !{!"omnipotent char", !869, i64 0}
!869 = !{!"Simple C++ TBAA"}
!870 = !DILocation(line: 186, column: 8, scope: !842)
!871 = !{!872, !867, i64 4356}
!872 = !{!"struct@_ZTS7state_t", !867, i64 0, !873, i64 4, !874, i64 264, !874, i64 272, !874, i64 280, !875, i64 288, !867, i64 392, !867, i64 396, !876, i64 400, !867, i64 452, !867, i64 456, !867, i64 460, !867, i64 464, !867, i64 468, !867, i64 472, !867, i64 476, !874, i64 480, !874, i64 488, !877, i64 496, !873, i64 2544, !873, i64 2800, !879, i64 3056, !874, i64 4080, !874, i64 4088, !867, i64 4096, !873, i64 4100, !867, i64 4356, !867, i64 4360, !867, i64 4364, !867, i64 4368, !867, i64 4372, !867, i64 4376, !867, i64 4380, !867, i64 4384, !867, i64 4388, !867, i64 4392, !881, i64 4400}
!873 = !{!"array@_ZTSA64_i", !867, i64 0}
!874 = !{!"long long", !868, i64 0}
!875 = !{!"array@_ZTSA13_y", !874, i64 0}
!876 = !{!"array@_ZTSA13_i", !867, i64 0}
!877 = !{!"array@_ZTSA64_6move_x", !878, i64 0}
!878 = !{!"struct@_ZTS6move_x", !867, i64 0, !867, i64 4, !867, i64 8, !867, i64 12, !874, i64 16, !874, i64 24}
!879 = !{!"array@_ZTSA64_N7state_tUt_E", !880, i64 0}
!880 = !{!"struct@_ZTSN7state_tUt_E", !867, i64 0, !867, i64 4, !867, i64 8, !867, i64 12}
!881 = !{!"array@_ZTSA1000_y", !874, i64 0}
!882 = !DILocation(line: 186, column: 16, scope: !842)
!883 = !DILocation(line: 188, column: 13, scope: !884)
!884 = distinct !DILexicalBlock(scope: !842, file: !743, line: 188, column: 9)
!885 = !{!872, !867, i64 460}
!886 = !DILocation(line: 188, column: 10, scope: !884)
!887 = !DILocation(line: 0, scope: !884)
!888 = !{!872, !874, i64 480}
!889 = !DILocation(line: 188, column: 9, scope: !842)
!890 = !DILocation(line: 194, column: 27, scope: !842)
!891 = !DILocation(line: 195, column: 14, scope: !842)
!892 = !{!893, !893, i64 0}
!893 = !{!"pointer@_ZTSP9ttentry_t", !868, i64 0}
!894 = !DILocation(line: 195, column: 30, scope: !842)
!895 = !DILocation(line: 195, column: 27, scope: !842)
!896 = !DILocation(line: 197, column: 11, scope: !842)
!897 = !DILocation(line: 200, column: 19, scope: !898)
!898 = distinct !DILexicalBlock(scope: !899, file: !743, line: 200, column: 13)
!899 = distinct !DILexicalBlock(scope: !900, file: !743, line: 199, column: 35)
!900 = distinct !DILexicalBlock(scope: !901, file: !743, line: 199, column: 5)
!901 = distinct !DILexicalBlock(scope: !842, file: !743, line: 199, column: 5)
!902 = !{!903, !904, i64 0}
!903 = !{!"struct@_ZTS9ttentry_t", !904, i64 0}
!904 = !{!"array@_ZTSA4_10ttbucket_t", !905, i64 0}
!905 = !{!"struct@_ZTS10ttbucket_t", !867, i64 0, !906, i64 4, !906, i64 6, !868, i64 8, !868, i64 9, !868, i64 9, !868, i64 9, !868, i64 9, !868, i64 9}
!906 = !{!"short", !868, i64 0}
!907 = !DILocation(line: 200, column: 35, scope: !898)
!908 = !DILocation(line: 199, column: 5, scope: !901)
!909 = !DILocation(line: 199, column: 31, scope: !900)
!910 = !DILocation(line: 199, column: 19, scope: !900)
!911 = distinct !{!911, !908, !912}
!912 = !DILocation(line: 251, column: 5, scope: !901)
!913 = !DILocation(line: 200, column: 13, scope: !898)
!914 = !{!904, !905, i64 0}
!915 = !DILocation(line: 200, column: 30, scope: !898)
!916 = !{!905, !867, i64 0}
!917 = !{!903, !867, i64 0}
!918 = !DILocation(line: 200, column: 13, scope: !899)
!919 = !DILocation(line: 201, column: 16, scope: !920)
!920 = distinct !DILexicalBlock(scope: !898, file: !743, line: 200, column: 45)
!921 = !{!872, !867, i64 4360}
!922 = !DILocation(line: 201, column: 22, scope: !920)
!923 = !DILocation(line: 205, column: 24, scope: !924)
!924 = distinct !DILexicalBlock(scope: !920, file: !743, line: 205, column: 17)
!925 = !{!905, !868, i64 9}
!926 = !DILocation(line: 205, column: 17, scope: !924)
!927 = !DILocation(line: 205, column: 31, scope: !924)
!928 = !DILocation(line: 205, column: 28, scope: !924)
!929 = !DILocation(line: 205, column: 17, scope: !920)
!930 = !DILocation(line: 206, column: 30, scope: !931)
!931 = distinct !DILexicalBlock(scope: !924, file: !743, line: 205, column: 38)
!932 = !DILocation(line: 206, column: 28, scope: !931)
!933 = !DILocation(line: 207, column: 13, scope: !931)
!934 = !DILocation(line: 209, column: 24, scope: !935)
!935 = distinct !DILexicalBlock(scope: !920, file: !743, line: 209, column: 17)
!936 = !DILocation(line: 209, column: 29, scope: !935)
!937 = !DILocation(line: 210, column: 17, scope: !935)
!938 = !DILocation(line: 210, column: 26, scope: !935)
!939 = !DILocation(line: 210, column: 46, scope: !935)
!940 = !{!905, !868, i64 8}
!941 = !{!903, !868, i64 8}
!942 = !DILocation(line: 210, column: 39, scope: !935)
!943 = !DILocation(line: 210, column: 36, scope: !935)
!944 = !DILocation(line: 211, column: 17, scope: !935)
!945 = !DILocation(line: 211, column: 27, scope: !935)
!946 = !{!905, !906, i64 4}
!947 = !{!903, !906, i64 4}
!948 = !DILocation(line: 211, column: 20, scope: !935)
!949 = !DILocation(line: 211, column: 33, scope: !935)
!950 = !DILocation(line: 209, column: 17, scope: !920)
!951 = !DILocation(line: 212, column: 25, scope: !952)
!952 = distinct !DILexicalBlock(scope: !935, file: !743, line: 211, column: 41)
!953 = !DILocation(line: 213, column: 13, scope: !952)
!954 = !DILocation(line: 215, column: 24, scope: !955)
!955 = distinct !DILexicalBlock(scope: !920, file: !743, line: 215, column: 17)
!956 = !DILocation(line: 215, column: 17, scope: !955)
!957 = !DILocation(line: 215, column: 30, scope: !955)
!958 = !DILocation(line: 215, column: 17, scope: !920)
!959 = !DILocation(line: 216, column: 33, scope: !960)
!960 = distinct !DILexicalBlock(scope: !955, file: !743, line: 215, column: 40)
!961 = !DILocation(line: 216, column: 26, scope: !960)
!962 = !DILocation(line: 216, column: 24, scope: !960)
!963 = !DILocation(line: 218, column: 28, scope: !964)
!964 = distinct !DILexicalBlock(scope: !960, file: !743, line: 218, column: 21)
!965 = !DILocation(line: 218, column: 21, scope: !960)
!966 = !DILocation(line: 219, column: 35, scope: !967)
!967 = distinct !DILexicalBlock(scope: !964, file: !743, line: 218, column: 45)
!968 = !{!872, !867, i64 472}
!969 = !DILocation(line: 219, column: 39, scope: !967)
!970 = !DILocation(line: 219, column: 28, scope: !967)
!971 = !DILocation(line: 220, column: 17, scope: !967)
!972 = !DILocation(line: 220, column: 35, scope: !973)
!973 = distinct !DILexicalBlock(scope: !964, file: !743, line: 220, column: 28)
!974 = !DILocation(line: 220, column: 28, scope: !964)
!975 = !DILocation(line: 221, column: 35, scope: !976)
!976 = distinct !DILexicalBlock(scope: !973, file: !743, line: 220, column: 52)
!977 = !DILocation(line: 221, column: 39, scope: !976)
!978 = !DILocation(line: 221, column: 28, scope: !976)
!979 = !DILocation(line: 222, column: 17, scope: !976)
!980 = !DILocation(line: 224, column: 32, scope: !960)
!981 = !{!905, !906, i64 6}
!982 = !{!903, !906, i64 6}
!983 = !DILocation(line: 224, column: 25, scope: !960)
!984 = !DILocation(line: 224, column: 23, scope: !960)
!985 = !DILocation(line: 225, column: 34, scope: !960)
!986 = !DILocation(line: 225, column: 27, scope: !960)
!987 = !DILocation(line: 225, column: 25, scope: !960)
!988 = !DILocation(line: 226, column: 36, scope: !960)
!989 = !DILocation(line: 226, column: 29, scope: !960)
!990 = !DILocation(line: 226, column: 27, scope: !960)
!991 = !DILocation(line: 227, column: 38, scope: !960)
!992 = !DILocation(line: 227, column: 31, scope: !960)
!993 = !DILocation(line: 227, column: 29, scope: !960)
!994 = !DILocation(line: 228, column: 31, scope: !960)
!995 = !DILocation(line: 228, column: 24, scope: !960)
!996 = !DILocation(line: 230, column: 17, scope: !960)
!997 = !DILocation(line: 232, column: 32, scope: !998)
!998 = distinct !DILexicalBlock(scope: !955, file: !743, line: 231, column: 20)
!999 = !DILocation(line: 232, column: 25, scope: !998)
!1000 = !DILocation(line: 232, column: 23, scope: !998)
!1001 = !DILocation(line: 233, column: 34, scope: !998)
!1002 = !DILocation(line: 233, column: 27, scope: !998)
!1003 = !DILocation(line: 233, column: 25, scope: !998)
!1004 = !DILocation(line: 234, column: 36, scope: !998)
!1005 = !DILocation(line: 234, column: 29, scope: !998)
!1006 = !DILocation(line: 234, column: 27, scope: !998)
!1007 = !DILocation(line: 235, column: 38, scope: !998)
!1008 = !DILocation(line: 235, column: 31, scope: !998)
!1009 = !DILocation(line: 235, column: 29, scope: !998)
!1010 = !DILocation(line: 237, column: 28, scope: !1011)
!1011 = distinct !DILexicalBlock(scope: !998, file: !743, line: 237, column: 21)
!1012 = !DILocation(line: 237, column: 21, scope: !998)
!1013 = !DILocation(line: 238, column: 28, scope: !1014)
!1014 = distinct !DILexicalBlock(scope: !1011, file: !743, line: 237, column: 43)
!1015 = !DILocation(line: 239, column: 17, scope: !1014)
!1016 = !DILocation(line: 240, column: 28, scope: !1017)
!1017 = distinct !DILexicalBlock(scope: !1018, file: !743, line: 239, column: 50)
!1018 = distinct !DILexicalBlock(scope: !1011, file: !743, line: 239, column: 28)
!1019 = !DILocation(line: 241, column: 17, scope: !1017)
!1020 = !DILocation(line: 242, column: 37, scope: !1021)
!1021 = distinct !DILexicalBlock(scope: !1018, file: !743, line: 241, column: 24)
!1022 = !DILocation(line: 242, column: 30, scope: !1021)
!1023 = !DILocation(line: 242, column: 28, scope: !1021)
!1024 = !DILocation(line: 245, column: 32, scope: !998)
!1025 = !DILocation(line: 245, column: 30, scope: !998)
!1026 = !DILocation(line: 246, column: 29, scope: !998)
!1027 = !DILocation(line: 248, column: 17, scope: !998)
!1028 = !DILocation(line: 254, column: 1, scope: !842)
!1029 = distinct !DISubprogram(name: "search", linkageName: "_Z6searchP7state_tiiiii", scope: !589, file: !589, line: 666, type: !1030, scopeLine: 666, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !588, retainedNodes: !1032)
!1030 = !DISubroutineType(types: !1031)
!1031 = !{!113, !391, !113, !113, !113, !113, !113}
!1032 = !{!1033, !1034, !1035, !1036, !1037, !1038, !1039, !1040, !1041, !1042, !1043, !1044, !1045, !1046, !1047, !1048, !1049, !1050, !1051, !1052, !1053, !1054, !1055, !1056, !1057, !1058, !1059, !1060, !1061, !1062, !1063, !1064, !1065, !1066, !1067, !1068, !1069, !1070, !1071, !1076, !1079, !1082, !1085, !1088, !1089, !1092, !1093, !1097, !1098, !1101, !1107, !1113, !1114, !1115, !1116, !1119}
!1033 = !DILocalVariable(name: "s", arg: 1, scope: !1029, file: !589, line: 666, type: !391)
!1034 = !DILocalVariable(name: "alpha", arg: 2, scope: !1029, file: !589, line: 666, type: !113)
!1035 = !DILocalVariable(name: "beta", arg: 3, scope: !1029, file: !589, line: 666, type: !113)
!1036 = !DILocalVariable(name: "depth", arg: 4, scope: !1029, file: !589, line: 666, type: !113)
!1037 = !DILocalVariable(name: "is_null", arg: 5, scope: !1029, file: !589, line: 666, type: !113)
!1038 = !DILocalVariable(name: "cutnode", arg: 6, scope: !1029, file: !589, line: 666, type: !113)
!1039 = !DILocalVariable(name: "moves", scope: !1029, file: !589, line: 667, type: !609)
!1040 = !DILocalVariable(name: "move_ordering", scope: !1029, file: !589, line: 668, type: !617)
!1041 = !DILocalVariable(name: "num_moves", scope: !1029, file: !589, line: 669, type: !113)
!1042 = !DILocalVariable(name: "i", scope: !1029, file: !589, line: 669, type: !113)
!1043 = !DILocalVariable(name: "j", scope: !1029, file: !589, line: 669, type: !113)
!1044 = !DILocalVariable(name: "score", scope: !1029, file: !589, line: 670, type: !113)
!1045 = !DILocalVariable(name: "no_moves", scope: !1029, file: !589, line: 671, type: !113)
!1046 = !DILocalVariable(name: "legal_move", scope: !1029, file: !589, line: 671, type: !113)
!1047 = !DILocalVariable(name: "bound", scope: !1029, file: !589, line: 672, type: !113)
!1048 = !DILocalVariable(name: "threat", scope: !1029, file: !589, line: 672, type: !113)
!1049 = !DILocalVariable(name: "donull", scope: !1029, file: !589, line: 672, type: !113)
!1050 = !DILocalVariable(name: "best_score", scope: !1029, file: !589, line: 672, type: !113)
!1051 = !DILocalVariable(name: "old_ep", scope: !1029, file: !589, line: 672, type: !113)
!1052 = !DILocalVariable(name: "best", scope: !1029, file: !589, line: 673, type: !10)
!1053 = !DILocalVariable(name: "incheck", scope: !1029, file: !589, line: 674, type: !113)
!1054 = !DILocalVariable(name: "first", scope: !1029, file: !589, line: 674, type: !113)
!1055 = !DILocalVariable(name: "extend", scope: !1029, file: !589, line: 675, type: !113)
!1056 = !DILocalVariable(name: "originalalpha", scope: !1029, file: !589, line: 676, type: !113)
!1057 = !DILocalVariable(name: "mateprune", scope: !1029, file: !589, line: 677, type: !113)
!1058 = !DILocalVariable(name: "afterincheck", scope: !1029, file: !589, line: 678, type: !113)
!1059 = !DILocalVariable(name: "legalmoves", scope: !1029, file: !589, line: 679, type: !113)
!1060 = !DILocalVariable(name: "reduc", scope: !1029, file: !589, line: 680, type: !113)
!1061 = !DILocalVariable(name: "remoneflag", scope: !1029, file: !589, line: 681, type: !113)
!1062 = !DILocalVariable(name: "mn", scope: !1029, file: !589, line: 682, type: !113)
!1063 = !DILocalVariable(name: "singular", scope: !1029, file: !589, line: 683, type: !113)
!1064 = !DILocalVariable(name: "huber", scope: !1029, file: !589, line: 684, type: !113)
!1065 = !DILocalVariable(name: "nosingular", scope: !1029, file: !589, line: 685, type: !113)
!1066 = !DILocalVariable(name: "wpcs", scope: !1029, file: !589, line: 686, type: !113)
!1067 = !DILocalVariable(name: "bpcs", scope: !1029, file: !589, line: 686, type: !113)
!1068 = !DILocalVariable(name: "fullext", scope: !1029, file: !589, line: 687, type: !113)
!1069 = !DILocalVariable(name: "searched_moves", scope: !1029, file: !589, line: 688, type: !609)
!1070 = !DILocalVariable(name: "escore", scope: !1029, file: !589, line: 759, type: !113)
!1071 = !DILocalVariable(name: "newdepth", scope: !1072, file: !589, line: 797, type: !113)
!1072 = distinct !DILexicalBlock(scope: !1073, file: !589, line: 796, column: 41)
!1073 = distinct !DILexicalBlock(scope: !1074, file: !589, line: 796, column: 13)
!1074 = distinct !DILexicalBlock(scope: !1075, file: !589, line: 791, column: 25)
!1075 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 786, column: 9)
!1076 = !DILocalVariable(name: "newdepth", scope: !1077, file: !589, line: 823, type: !113)
!1077 = distinct !DILexicalBlock(scope: !1078, file: !589, line: 808, column: 58)
!1078 = distinct !DILexicalBlock(scope: !1074, file: !589, line: 808, column: 13)
!1079 = !DILocalVariable(name: "rscore", scope: !1080, file: !589, line: 849, type: !113)
!1080 = distinct !DILexicalBlock(scope: !1081, file: !589, line: 848, column: 33)
!1081 = distinct !DILexicalBlock(scope: !1075, file: !589, line: 847, column: 16)
!1082 = !DILocalVariable(name: "goodmove", scope: !1083, file: !589, line: 891, type: !113)
!1083 = distinct !DILexicalBlock(scope: !1084, file: !589, line: 890, column: 70)
!1084 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 890, column: 9)
!1085 = !DILocalVariable(name: "tmp", scope: !1086, file: !589, line: 901, type: !113)
!1086 = distinct !DILexicalBlock(scope: !1087, file: !589, line: 900, column: 24)
!1087 = distinct !DILexicalBlock(scope: !1083, file: !589, line: 900, column: 13)
!1088 = !DILocalVariable(name: "mv", scope: !1086, file: !589, line: 902, type: !10)
!1089 = !DILocalVariable(name: "m_s", scope: !1090, file: !589, line: 928, type: !113)
!1090 = distinct !DILexicalBlock(scope: !1091, file: !589, line: 926, column: 54)
!1091 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 919, column: 9)
!1092 = !DILocalVariable(name: "m_c", scope: !1090, file: !589, line: 928, type: !113)
!1093 = !DILocalVariable(name: "newdepth", scope: !1094, file: !589, line: 952, type: !113)
!1094 = distinct !DILexicalBlock(scope: !1095, file: !589, line: 943, column: 43)
!1095 = distinct !DILexicalBlock(scope: !1096, file: !589, line: 943, column: 17)
!1096 = distinct !DILexicalBlock(scope: !1090, file: !589, line: 936, column: 39)
!1097 = !DILocalVariable(name: "s_c", scope: !1029, file: !589, line: 984, type: !113)
!1098 = !DILocalVariable(name: "prescore", scope: !1099, file: !589, line: 996, type: !113)
!1099 = distinct !DILexicalBlock(scope: !1100, file: !589, line: 994, column: 42)
!1100 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 989, column: 9)
!1101 = !DILocalVariable(name: "newdepth", scope: !1102, file: !589, line: 1020, type: !113)
!1102 = distinct !DILexicalBlock(scope: !1103, file: !589, line: 1010, column: 47)
!1103 = distinct !DILexicalBlock(scope: !1104, file: !589, line: 1010, column: 21)
!1104 = distinct !DILexicalBlock(scope: !1105, file: !589, line: 1005, column: 60)
!1105 = distinct !DILexicalBlock(scope: !1106, file: !589, line: 998, column: 31)
!1106 = distinct !DILexicalBlock(scope: !1099, file: !589, line: 998, column: 13)
!1107 = !DILocalVariable(name: "capsee", scope: !1108, file: !589, line: 1083, type: !113)
!1108 = distinct !DILexicalBlock(scope: !1109, file: !589, line: 1082, column: 69)
!1109 = distinct !DILexicalBlock(scope: !1110, file: !589, line: 1079, column: 17)
!1110 = distinct !DILexicalBlock(scope: !1111, file: !589, line: 1063, column: 32)
!1111 = distinct !DILexicalBlock(scope: !1112, file: !589, line: 1063, column: 13)
!1112 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 1059, column: 24)
!1113 = !DILocalVariable(name: "afutprun", scope: !1112, file: !589, line: 1131, type: !113)
!1114 = !DILocalVariable(name: "pfutprun", scope: !1112, file: !589, line: 1132, type: !113)
!1115 = !DILocalVariable(name: "capval", scope: !1112, file: !589, line: 1150, type: !113)
!1116 = !DILocalVariable(name: "newdepth", scope: !1117, file: !589, line: 1194, type: !113)
!1117 = distinct !DILexicalBlock(scope: !1118, file: !589, line: 1161, column: 39)
!1118 = distinct !DILexicalBlock(scope: !1112, file: !589, line: 1161, column: 13)
!1119 = !DILocalVariable(name: "validresult", scope: !1029, file: !589, line: 1283, type: !113)
!1120 = !DILocation(line: 0, scope: !1029)
!1121 = !DILocation(line: 667, column: 5, scope: !1029)
!1122 = !DILocation(line: 667, column: 12, scope: !1029)
!1123 = !DILocation(line: 668, column: 5, scope: !1029)
!1124 = !DILocation(line: 668, column: 9, scope: !1029)
!1125 = !DILocation(line: 669, column: 5, scope: !1029)
!1126 = !DILocation(line: 672, column: 5, scope: !1029)
!1127 = !DILocation(line: 673, column: 5, scope: !1029)
!1128 = !DILocation(line: 683, column: 5, scope: !1029)
!1129 = !DILocation(line: 685, column: 5, scope: !1029)
!1130 = !DILocation(line: 688, column: 5, scope: !1029)
!1131 = !DILocation(line: 688, column: 12, scope: !1029)
!1132 = !DILocation(line: 690, column: 15, scope: !1133)
!1133 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 690, column: 9)
!1134 = !DILocation(line: 690, column: 20, scope: !1133)
!1135 = !DILocation(line: 690, column: 26, scope: !1133)
!1136 = !DILocation(line: 690, column: 30, scope: !1133)
!1137 = !DILocation(line: 690, column: 9, scope: !1029)
!1138 = !DILocation(line: 691, column: 16, scope: !1139)
!1139 = distinct !DILexicalBlock(scope: !1133, file: !589, line: 690, column: 43)
!1140 = !DILocation(line: 691, column: 9, scope: !1139)
!1141 = !DILocation(line: 694, column: 8, scope: !1029)
!1142 = !{!872, !874, i64 4080}
!1143 = !DILocation(line: 694, column: 13, scope: !1029)
!1144 = !DILocation(line: 696, column: 9, scope: !1145)
!1145 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 696, column: 9)
!1146 = !DILocation(line: 696, column: 9, scope: !1029)
!1147 = !DILocation(line: 700, column: 9, scope: !1148)
!1148 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 700, column: 9)
!1149 = !DILocation(line: 700, column: 32, scope: !1148)
!1150 = !DILocation(line: 700, column: 38, scope: !1148)
!1151 = !{!872, !867, i64 476}
!1152 = !DILocation(line: 700, column: 44, scope: !1148)
!1153 = !DILocation(line: 700, column: 9, scope: !1029)
!1154 = !DILocation(line: 701, column: 27, scope: !1155)
!1155 = distinct !DILexicalBlock(scope: !1148, file: !589, line: 700, column: 50)
!1156 = !{!1157, !867, i64 12}
!1157 = !{!"struct@_ZTS11gamestate_t", !867, i64 0, !867, i64 4, !867, i64 8, !867, i64 12, !867, i64 16, !867, i64 20, !867, i64 24, !867, i64 28, !867, i64 32, !867, i64 36, !867, i64 40, !867, i64 44, !867, i64 48, !867, i64 52, !867, i64 56, !867, i64 60, !1158, i64 64, !1159, i64 4064, !874, i64 36064, !867, i64 36072, !867, i64 36076, !867, i64 36080, !867, i64 36084, !867, i64 36088, !867, i64 36092, !867, i64 36096, !867, i64 36100}
!1158 = !{!"array@_ZTSA1000_i", !867, i64 0}
!1159 = !{!"array@_ZTSA1000_6move_x", !878, i64 0}
!1160 = !DILocation(line: 701, column: 44, scope: !1155)
!1161 = !DILocation(line: 701, column: 38, scope: !1155)
!1162 = !DILocation(line: 0, scope: !1155)
!1163 = !DILocation(line: 701, column: 17, scope: !1155)
!1164 = !DILocation(line: 701, column: 9, scope: !1155)
!1165 = !DILocation(line: 708, column: 28, scope: !1029)
!1166 = !DILocation(line: 708, column: 23, scope: !1029)
!1167 = !DILocation(line: 709, column: 19, scope: !1168)
!1168 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 709, column: 9)
!1169 = !DILocation(line: 709, column: 9, scope: !1029)
!1170 = !DILocation(line: 711, column: 23, scope: !1171)
!1171 = distinct !DILexicalBlock(scope: !1172, file: !589, line: 711, column: 13)
!1172 = distinct !DILexicalBlock(scope: !1168, file: !589, line: 709, column: 28)
!1173 = !DILocation(line: 711, column: 13, scope: !1172)
!1174 = !DILocation(line: 720, column: 31, scope: !1029)
!1175 = !DILocation(line: 721, column: 19, scope: !1176)
!1176 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 721, column: 9)
!1177 = !DILocation(line: 721, column: 9, scope: !1029)
!1178 = !DILocation(line: 723, column: 23, scope: !1179)
!1179 = distinct !DILexicalBlock(scope: !1180, file: !589, line: 723, column: 13)
!1180 = distinct !DILexicalBlock(scope: !1176, file: !589, line: 721, column: 27)
!1181 = !DILocation(line: 723, column: 13, scope: !1180)
!1182 = !DILocation(line: 728, column: 13, scope: !1029)
!1183 = !DILocation(line: 728, column: 5, scope: !1029)
!1184 = !DILocation(line: 730, column: 20, scope: !1185)
!1185 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 728, column: 93)
!1186 = !DILocation(line: 730, column: 13, scope: !1185)
!1187 = !DILocation(line: 733, column: 17, scope: !1188)
!1188 = distinct !DILexicalBlock(scope: !1185, file: !589, line: 733, column: 17)
!1189 = !DILocation(line: 733, column: 23, scope: !1188)
!1190 = !DILocation(line: 733, column: 17, scope: !1185)
!1191 = !DILocation(line: 739, column: 17, scope: !1192)
!1192 = distinct !DILexicalBlock(scope: !1185, file: !589, line: 739, column: 17)
!1193 = !DILocation(line: 739, column: 23, scope: !1192)
!1194 = !DILocation(line: 739, column: 17, scope: !1185)
!1195 = !DILocation(line: 745, column: 17, scope: !1196)
!1196 = distinct !DILexicalBlock(scope: !1185, file: !589, line: 745, column: 17)
!1197 = !DILocation(line: 745, column: 23, scope: !1196)
!1198 = !DILocation(line: 745, column: 17, scope: !1185)
!1199 = !DILocation(line: 750, column: 18, scope: !1185)
!1200 = !DILocation(line: 751, column: 20, scope: !1185)
!1201 = !DILocation(line: 752, column: 22, scope: !1185)
!1202 = !DILocation(line: 753, column: 24, scope: !1185)
!1203 = !DILocation(line: 754, column: 13, scope: !1185)
!1204 = !DILocation(line: 758, column: 18, scope: !1029)
!1205 = !{!872, !873, i64 4100}
!1206 = !DILocation(line: 758, column: 28, scope: !1029)
!1207 = !DILocation(line: 758, column: 15, scope: !1029)
!1208 = !{!873, !867, i64 0}
!1209 = !{!872, !867, i64 4100}
!1210 = !DILocation(line: 759, column: 18, scope: !1029)
!1211 = !DILocation(line: 761, column: 11, scope: !1212)
!1212 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 761, column: 10)
!1213 = !DILocation(line: 761, column: 19, scope: !1212)
!1214 = !DILocation(line: 761, column: 36, scope: !1212)
!1215 = !DILocation(line: 761, column: 27, scope: !1212)
!1216 = !DILocation(line: 762, column: 19, scope: !1217)
!1217 = distinct !DILexicalBlock(scope: !1218, file: !589, line: 762, column: 13)
!1218 = distinct !DILexicalBlock(scope: !1212, file: !589, line: 761, column: 41)
!1219 = !DILocation(line: 762, column: 13, scope: !1218)
!1220 = !DILocation(line: 763, column: 24, scope: !1221)
!1221 = distinct !DILexicalBlock(scope: !1222, file: !589, line: 763, column: 17)
!1222 = distinct !DILexicalBlock(scope: !1217, file: !589, line: 762, column: 27)
!1223 = !DILocation(line: 763, column: 29, scope: !1221)
!1224 = !DILocation(line: 763, column: 17, scope: !1222)
!1225 = !DILocation(line: 764, column: 59, scope: !1226)
!1226 = distinct !DILexicalBlock(scope: !1221, file: !589, line: 763, column: 38)
!1227 = !DILocation(line: 764, column: 64, scope: !1226)
!1228 = !DILocation(line: 764, column: 71, scope: !1226)
!1229 = !DILocation(line: 764, column: 80, scope: !1226)
!1230 = !DILocation(line: 764, column: 17, scope: !1226)
!1231 = !DILocation(line: 765, column: 17, scope: !1226)
!1232 = !DILocation(line: 767, column: 24, scope: !1233)
!1233 = distinct !DILexicalBlock(scope: !1222, file: !589, line: 767, column: 17)
!1234 = !DILocation(line: 767, column: 17, scope: !1222)
!1235 = !DILocation(line: 768, column: 24, scope: !1236)
!1236 = distinct !DILexicalBlock(scope: !1233, file: !589, line: 767, column: 32)
!1237 = !DILocation(line: 768, column: 17, scope: !1236)
!1238 = !DILocation(line: 770, column: 26, scope: !1239)
!1239 = distinct !DILexicalBlock(scope: !1217, file: !589, line: 770, column: 20)
!1240 = !DILocation(line: 770, column: 20, scope: !1217)
!1241 = !DILocation(line: 771, column: 24, scope: !1242)
!1242 = distinct !DILexicalBlock(scope: !1243, file: !589, line: 771, column: 17)
!1243 = distinct !DILexicalBlock(scope: !1239, file: !589, line: 770, column: 36)
!1244 = !DILocation(line: 771, column: 30, scope: !1242)
!1245 = !DILocation(line: 771, column: 17, scope: !1243)
!1246 = !DILocation(line: 772, column: 60, scope: !1247)
!1247 = distinct !DILexicalBlock(scope: !1242, file: !589, line: 771, column: 39)
!1248 = !DILocation(line: 772, column: 65, scope: !1247)
!1249 = !DILocation(line: 772, column: 72, scope: !1247)
!1250 = !DILocation(line: 772, column: 81, scope: !1247)
!1251 = !DILocation(line: 772, column: 17, scope: !1247)
!1252 = !DILocation(line: 773, column: 17, scope: !1247)
!1253 = !DILocation(line: 781, column: 15, scope: !1029)
!1254 = !{!872, !876, i64 400}
!1255 = !DILocation(line: 781, column: 12, scope: !1029)
!1256 = !{!876, !867, i64 0}
!1257 = !{!872, !867, i64 400}
!1258 = !DILocation(line: 781, column: 33, scope: !1029)
!1259 = !DILocation(line: 781, column: 31, scope: !1029)
!1260 = !DILocation(line: 781, column: 53, scope: !1029)
!1261 = !DILocation(line: 781, column: 51, scope: !1029)
!1262 = !DILocation(line: 781, column: 75, scope: !1029)
!1263 = !DILocation(line: 781, column: 73, scope: !1029)
!1264 = !DILocation(line: 782, column: 12, scope: !1029)
!1265 = !DILocation(line: 782, column: 33, scope: !1029)
!1266 = !DILocation(line: 782, column: 31, scope: !1029)
!1267 = !DILocation(line: 782, column: 53, scope: !1029)
!1268 = !DILocation(line: 782, column: 51, scope: !1029)
!1269 = !DILocation(line: 782, column: 75, scope: !1029)
!1270 = !DILocation(line: 782, column: 73, scope: !1029)
!1271 = !DILocation(line: 784, column: 12, scope: !1029)
!1272 = !DILocation(line: 786, column: 18, scope: !1075)
!1273 = !DILocation(line: 787, column: 9, scope: !1075)
!1274 = !DILocation(line: 787, column: 16, scope: !1075)
!1275 = !DILocation(line: 787, column: 13, scope: !1075)
!1276 = !DILocation(line: 787, column: 12, scope: !1075)
!1277 = !DILocation(line: 788, column: 9, scope: !1075)
!1278 = !DILocation(line: 788, column: 24, scope: !1075)
!1279 = !DILocation(line: 791, column: 18, scope: !1075)
!1280 = !DILocation(line: 790, column: 9, scope: !1075)
!1281 = !DILocation(line: 796, column: 23, scope: !1073)
!1282 = !{!1157, !867, i64 4}
!1283 = !DILocation(line: 796, column: 29, scope: !1073)
!1284 = !DILocation(line: 796, column: 13, scope: !1074)
!1285 = !DILocation(line: 0, scope: !1072)
!1286 = !DILocation(line: 798, column: 26, scope: !1287)
!1287 = distinct !DILexicalBlock(scope: !1072, file: !589, line: 798, column: 17)
!1288 = !DILocation(line: 0, scope: !1287)
!1289 = !DILocation(line: 798, column: 17, scope: !1072)
!1290 = !DILocation(line: 799, column: 25, scope: !1291)
!1291 = distinct !DILexicalBlock(scope: !1287, file: !589, line: 798, column: 32)
!1292 = !DILocation(line: 800, column: 13, scope: !1291)
!1293 = !DILocation(line: 797, column: 34, scope: !1072)
!1294 = !DILocation(line: 801, column: 25, scope: !1295)
!1295 = distinct !DILexicalBlock(scope: !1287, file: !589, line: 800, column: 20)
!1296 = !DILocation(line: 803, column: 27, scope: !1297)
!1297 = distinct !DILexicalBlock(scope: !1072, file: !589, line: 803, column: 17)
!1298 = !{!1157, !867, i64 36096}
!1299 = !DILocation(line: 803, column: 17, scope: !1297)
!1300 = !DILocation(line: 803, column: 17, scope: !1072)
!1301 = !DILocation(line: 808, column: 23, scope: !1078)
!1302 = !DILocation(line: 808, column: 29, scope: !1078)
!1303 = !DILocation(line: 808, column: 49, scope: !1078)
!1304 = !DILocation(line: 808, column: 40, scope: !1078)
!1305 = !DILocation(line: 811, column: 30, scope: !1077)
!1306 = !DILocation(line: 809, column: 25, scope: !1077)
!1307 = !{!872, !867, i64 456}
!1308 = !DILocation(line: 810, column: 26, scope: !1077)
!1309 = !DILocation(line: 812, column: 19, scope: !1077)
!1310 = !DILocation(line: 813, column: 21, scope: !1077)
!1311 = !DILocation(line: 815, column: 16, scope: !1077)
!1312 = !{!872, !873, i64 2544}
!1313 = !DILocation(line: 815, column: 13, scope: !1077)
!1314 = !DILocation(line: 815, column: 33, scope: !1077)
!1315 = !{!872, !867, i64 2544}
!1316 = !DILocation(line: 816, column: 13, scope: !1077)
!1317 = !DILocation(line: 816, column: 31, scope: !1077)
!1318 = !DILocation(line: 817, column: 37, scope: !1077)
!1319 = !{!872, !873, i64 2800}
!1320 = !DILocation(line: 817, column: 34, scope: !1077)
!1321 = !{!872, !867, i64 2800}
!1322 = !DILocation(line: 817, column: 13, scope: !1077)
!1323 = !DILocation(line: 817, column: 32, scope: !1077)
!1324 = !DILocation(line: 0, scope: !1077)
!1325 = !DILocation(line: 825, column: 26, scope: !1326)
!1326 = distinct !DILexicalBlock(scope: !1077, file: !589, line: 825, column: 17)
!1327 = !DILocation(line: 0, scope: !1326)
!1328 = !DILocation(line: 825, column: 17, scope: !1077)
!1329 = !DILocation(line: 826, column: 26, scope: !1330)
!1330 = distinct !DILexicalBlock(scope: !1326, file: !589, line: 825, column: 32)
!1331 = !DILocation(line: 827, column: 13, scope: !1330)
!1332 = !DILocation(line: 823, column: 34, scope: !1077)
!1333 = !DILocation(line: 828, column: 73, scope: !1334)
!1334 = distinct !DILexicalBlock(scope: !1326, file: !589, line: 827, column: 20)
!1335 = !DILocation(line: 828, column: 72, scope: !1334)
!1336 = !DILocation(line: 828, column: 26, scope: !1334)
!1337 = !DILocation(line: 831, column: 21, scope: !1077)
!1338 = !DILocation(line: 832, column: 19, scope: !1077)
!1339 = !DILocation(line: 833, column: 30, scope: !1077)
!1340 = !DILocation(line: 834, column: 26, scope: !1077)
!1341 = !DILocation(line: 836, column: 27, scope: !1342)
!1342 = distinct !DILexicalBlock(scope: !1077, file: !589, line: 836, column: 17)
!1343 = !DILocation(line: 836, column: 17, scope: !1342)
!1344 = !DILocation(line: 836, column: 17, scope: !1077)
!1345 = !DILocation(line: 840, column: 23, scope: !1346)
!1346 = distinct !DILexicalBlock(scope: !1077, file: !589, line: 840, column: 17)
!1347 = !DILocation(line: 840, column: 17, scope: !1077)
!1348 = !DILocation(line: 841, column: 56, scope: !1349)
!1349 = distinct !DILexicalBlock(scope: !1346, file: !589, line: 840, column: 32)
!1350 = !DILocation(line: 841, column: 62, scope: !1349)
!1351 = !DILocation(line: 841, column: 77, scope: !1349)
!1352 = !DILocation(line: 841, column: 17, scope: !1349)
!1353 = !DILocation(line: 842, column: 17, scope: !1349)
!1354 = !DILocation(line: 843, column: 30, scope: !1355)
!1355 = distinct !DILexicalBlock(scope: !1346, file: !589, line: 843, column: 24)
!1356 = !DILocation(line: 843, column: 24, scope: !1346)
!1357 = !DILocation(line: 844, column: 24, scope: !1358)
!1358 = distinct !DILexicalBlock(scope: !1355, file: !589, line: 843, column: 45)
!1359 = !DILocation(line: 845, column: 13, scope: !1358)
!1360 = !DILocation(line: 847, column: 43, scope: !1081)
!1361 = !DILocation(line: 847, column: 34, scope: !1081)
!1362 = !DILocation(line: 848, column: 26, scope: !1081)
!1363 = !DILocation(line: 848, column: 19, scope: !1081)
!1364 = !DILocation(line: 849, column: 22, scope: !1080)
!1365 = !DILocation(line: 0, scope: !1080)
!1366 = !DILocation(line: 850, column: 23, scope: !1367)
!1367 = distinct !DILexicalBlock(scope: !1080, file: !589, line: 850, column: 13)
!1368 = !DILocation(line: 850, column: 13, scope: !1367)
!1369 = !DILocation(line: 850, column: 13, scope: !1080)
!1370 = !DILocation(line: 854, column: 20, scope: !1371)
!1371 = distinct !DILexicalBlock(scope: !1080, file: !589, line: 854, column: 13)
!1372 = !DILocation(line: 854, column: 13, scope: !1080)
!1373 = !DILocation(line: 855, column: 48, scope: !1374)
!1374 = distinct !DILexicalBlock(scope: !1371, file: !589, line: 854, column: 30)
!1375 = !DILocation(line: 855, column: 53, scope: !1374)
!1376 = !DILocation(line: 855, column: 60, scope: !1374)
!1377 = !DILocation(line: 855, column: 69, scope: !1374)
!1378 = !DILocation(line: 855, column: 13, scope: !1374)
!1379 = !DILocation(line: 856, column: 13, scope: !1374)
!1380 = !DILocation(line: 0, scope: !1381)
!1381 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 866, column: 9)
!1382 = !DILocation(line: 866, column: 9, scope: !1029)
!1383 = !DILocation(line: 867, column: 21, scope: !1384)
!1384 = distinct !DILexicalBlock(scope: !1381, file: !589, line: 866, column: 18)
!1385 = !DILocation(line: 873, column: 13, scope: !1386)
!1386 = distinct !DILexicalBlock(scope: !1387, file: !589, line: 873, column: 13)
!1387 = distinct !DILexicalBlock(scope: !1388, file: !589, line: 872, column: 18)
!1388 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 872, column: 9)
!1389 = !DILocation(line: 873, column: 13, scope: !1387)
!1390 = !DILocation(line: 0, scope: !1391)
!1391 = distinct !DILexicalBlock(scope: !1392, file: !589, line: 874, column: 13)
!1392 = distinct !DILexicalBlock(scope: !1386, file: !589, line: 873, column: 24)
!1393 = !DILocation(line: 874, column: 28, scope: !1394)
!1394 = distinct !DILexicalBlock(scope: !1391, file: !589, line: 874, column: 13)
!1395 = !DILocation(line: 874, column: 13, scope: !1391)
!1396 = !DILocation(line: 869, column: 21, scope: !1397)
!1397 = distinct !DILexicalBlock(scope: !1381, file: !589, line: 868, column: 12)
!1398 = !DILocation(line: 875, column: 25, scope: !1399)
!1399 = distinct !DILexicalBlock(scope: !1394, file: !589, line: 874, column: 67)
!1400 = !{!1401, !867, i64 0}
!1401 = !{!"array@_ZTSA240_i", !867, i64 0}
!1402 = !DILocation(line: 875, column: 17, scope: !1399)
!1403 = !DILocation(line: 877, column: 42, scope: !1404)
!1404 = distinct !DILexicalBlock(scope: !1399, file: !589, line: 877, column: 21)
!1405 = !DILocation(line: 877, column: 36, scope: !1404)
!1406 = !DILocation(line: 877, column: 21, scope: !1404)
!1407 = !DILocation(line: 877, column: 21, scope: !1399)
!1408 = !DILocation(line: 881, column: 33, scope: !1399)
!1409 = !DILocation(line: 881, column: 27, scope: !1399)
!1410 = !DILocation(line: 881, column: 17, scope: !1399)
!1411 = !DILocation(line: 874, column: 63, scope: !1394)
!1412 = !DILocation(line: 874, column: 41, scope: !1394)
!1413 = distinct !{!1413, !1395, !1414}
!1414 = !DILocation(line: 882, column: 13, scope: !1391)
!1415 = !DILocation(line: 888, column: 27, scope: !1029)
!1416 = !DILocation(line: 888, column: 53, scope: !1029)
!1417 = !DILocation(line: 888, column: 5, scope: !1029)
!1418 = !DILocation(line: 890, column: 15, scope: !1084)
!1419 = !DILocation(line: 890, column: 26, scope: !1084)
!1420 = !DILocation(line: 890, column: 35, scope: !1084)
!1421 = !DILocation(line: 890, column: 53, scope: !1084)
!1422 = !DILocation(line: 890, column: 58, scope: !1084)
!1423 = !DILocation(line: 890, column: 49, scope: !1084)
!1424 = !DILocation(line: 0, scope: !1425)
!1425 = distinct !DILexicalBlock(scope: !1083, file: !589, line: 893, column: 9)
!1426 = !DILocation(line: 0, scope: !1083)
!1427 = !DILocation(line: 893, column: 23, scope: !1428)
!1428 = distinct !DILexicalBlock(scope: !1425, file: !589, line: 893, column: 9)
!1429 = !DILocation(line: 893, column: 9, scope: !1425)
!1430 = !DILocation(line: 895, column: 63, scope: !1431)
!1431 = distinct !DILexicalBlock(scope: !1432, file: !589, line: 894, column: 17)
!1432 = distinct !DILexicalBlock(scope: !1428, file: !589, line: 893, column: 41)
!1433 = !DILocation(line: 894, column: 17, scope: !1431)
!1434 = !DILocation(line: 894, column: 36, scope: !1431)
!1435 = !DILocation(line: 895, column: 17, scope: !1431)
!1436 = !DILocation(line: 895, column: 70, scope: !1431)
!1437 = !DILocation(line: 895, column: 60, scope: !1431)
!1438 = !{!872, !867, i64 4}
!1439 = !DILocation(line: 895, column: 51, scope: !1431)
!1440 = !{!1441, !867, i64 0}
!1441 = !{!"array@_ZTSA14_i", !867, i64 0}
!1442 = !DILocation(line: 895, column: 47, scope: !1431)
!1443 = !DILocation(line: 895, column: 45, scope: !1431)
!1444 = !DILocation(line: 894, column: 17, scope: !1432)
!1445 = !DILocation(line: 893, column: 37, scope: !1428)
!1446 = distinct !{!1446, !1429, !1447}
!1447 = !DILocation(line: 898, column: 9, scope: !1425)
!1448 = !DILocation(line: 900, column: 14, scope: !1087)
!1449 = !DILocation(line: 900, column: 13, scope: !1083)
!1450 = !DILocation(line: 901, column: 13, scope: !1086)
!1451 = !DILocation(line: 902, column: 13, scope: !1086)
!1452 = !DILocation(line: 904, column: 50, scope: !1086)
!1453 = !DILocation(line: 904, column: 21, scope: !1086)
!1454 = !DILocation(line: 0, scope: !1086)
!1455 = !DILocation(line: 906, column: 17, scope: !1456)
!1456 = distinct !DILexicalBlock(scope: !1086, file: !589, line: 906, column: 17)
!1457 = !DILocation(line: 906, column: 72, scope: !1456)
!1458 = !DILocation(line: 906, column: 17, scope: !1086)
!1459 = !DILocation(line: 907, column: 65, scope: !1460)
!1460 = distinct !DILexicalBlock(scope: !1456, file: !589, line: 906, column: 82)
!1461 = !DILocation(line: 907, column: 17, scope: !1460)
!1462 = !DILocation(line: 908, column: 13, scope: !1460)
!1463 = !DILocation(line: 909, column: 65, scope: !1464)
!1464 = distinct !DILexicalBlock(scope: !1456, file: !589, line: 908, column: 20)
!1465 = !DILocation(line: 909, column: 17, scope: !1464)
!1466 = !DILocation(line: 911, column: 9, scope: !1087)
!1467 = !DILocation(line: 911, column: 9, scope: !1086)
!1468 = !DILocation(line: 920, column: 13, scope: !1091)
!1469 = !DILocation(line: 920, column: 9, scope: !1091)
!1470 = !DILocation(line: 921, column: 18, scope: !1091)
!1471 = !DILocation(line: 922, column: 23, scope: !1091)
!1472 = !DILocation(line: 924, column: 26, scope: !1091)
!1473 = !DILocation(line: 924, column: 30, scope: !1091)
!1474 = !DILocation(line: 924, column: 13, scope: !1091)
!1475 = !DILocation(line: 925, column: 9, scope: !1091)
!1476 = !DILocation(line: 925, column: 20, scope: !1091)
!1477 = !DILocation(line: 925, column: 24, scope: !1091)
!1478 = !DILocation(line: 925, column: 46, scope: !1091)
!1479 = !DILocation(line: 925, column: 29, scope: !1091)
!1480 = !DILocation(line: 926, column: 9, scope: !1091)
!1481 = !DILocation(line: 926, column: 20, scope: !1091)
!1482 = !DILocation(line: 926, column: 24, scope: !1091)
!1483 = !DILocation(line: 926, column: 46, scope: !1091)
!1484 = !DILocation(line: 926, column: 29, scope: !1091)
!1485 = !DILocation(line: 919, column: 9, scope: !1029)
!1486 = !DILocation(line: 0, scope: !1090)
!1487 = !DILocation(line: 932, column: 11, scope: !1090)
!1488 = !DILocation(line: 934, column: 22, scope: !1090)
!1489 = !DILocation(line: 936, column: 16, scope: !1090)
!1490 = !DILocation(line: 936, column: 9, scope: !1090)
!1491 = !DILocation(line: 944, column: 74, scope: !1094)
!1492 = !DILocation(line: 944, column: 20, scope: !1094)
!1493 = !DILocation(line: 945, column: 20, scope: !1094)
!1494 = !DILocation(line: 954, column: 25, scope: !1094)
!1495 = !DILocation(line: 954, column: 38, scope: !1094)
!1496 = !DILocation(line: 954, column: 52, scope: !1094)
!1497 = !DILocation(line: 956, column: 30, scope: !1498)
!1498 = distinct !DILexicalBlock(scope: !1094, file: !589, line: 956, column: 21)
!1499 = !DILocation(line: 0, scope: !1498)
!1500 = !DILocation(line: 952, column: 38, scope: !1094)
!1501 = !DILocation(line: 959, column: 74, scope: !1502)
!1502 = distinct !DILexicalBlock(scope: !1498, file: !589, line: 958, column: 24)
!1503 = !DILocation(line: 959, column: 73, scope: !1502)
!1504 = !DILocation(line: 937, column: 16, scope: !1096)
!1505 = !DILocation(line: 939, column: 27, scope: !1096)
!1506 = !DILocation(line: 939, column: 21, scope: !1096)
!1507 = !DILocation(line: 939, column: 13, scope: !1096)
!1508 = !DILocation(line: 943, column: 38, scope: !1095)
!1509 = !DILocation(line: 943, column: 32, scope: !1095)
!1510 = !DILocation(line: 943, column: 17, scope: !1095)
!1511 = !DILocation(line: 943, column: 17, scope: !1096)
!1512 = !DILocation(line: 944, column: 43, scope: !1094)
!1513 = !{!1157, !867, i64 60}
!1514 = !DILocation(line: 944, column: 60, scope: !1094)
!1515 = !DILocation(line: 944, column: 55, scope: !1094)
!1516 = !DILocation(line: 944, column: 64, scope: !1094)
!1517 = !DILocation(line: 944, column: 17, scope: !1094)
!1518 = !{!881, !874, i64 0}
!1519 = !DILocation(line: 944, column: 69, scope: !1094)
!1520 = !{!872, !874, i64 4400}
!1521 = !DILocation(line: 945, column: 45, scope: !1094)
!1522 = !DILocation(line: 945, column: 39, scope: !1094)
!1523 = !DILocation(line: 945, column: 17, scope: !1094)
!1524 = !DILocation(line: 945, column: 37, scope: !1094)
!1525 = !DILocation(line: 949, column: 32, scope: !1094)
!1526 = !DILocation(line: 950, column: 30, scope: !1094)
!1527 = !DILocation(line: 950, column: 17, scope: !1094)
!1528 = !DILocation(line: 950, column: 35, scope: !1094)
!1529 = !DILocation(line: 0, scope: !1094)
!1530 = !DILocation(line: 954, column: 56, scope: !1094)
!1531 = !DILocation(line: 954, column: 43, scope: !1094)
!1532 = !DILocation(line: 954, column: 17, scope: !1094)
!1533 = !DILocation(line: 957, column: 30, scope: !1534)
!1534 = distinct !DILexicalBlock(scope: !1498, file: !589, line: 956, column: 36)
!1535 = !DILocation(line: 961, column: 13, scope: !1094)
!1536 = !DILocation(line: 963, column: 29, scope: !1096)
!1537 = !DILocation(line: 963, column: 23, scope: !1096)
!1538 = !DILocation(line: 963, column: 13, scope: !1096)
!1539 = !DILocation(line: 965, column: 28, scope: !1540)
!1540 = distinct !DILexicalBlock(scope: !1096, file: !589, line: 965, column: 17)
!1541 = !DILocation(line: 965, column: 18, scope: !1540)
!1542 = !DILocation(line: 965, column: 17, scope: !1096)
!1543 = !DILocation(line: 966, column: 27, scope: !1544)
!1544 = distinct !DILexicalBlock(scope: !1545, file: !589, line: 966, column: 21)
!1545 = distinct !DILexicalBlock(scope: !1540, file: !589, line: 965, column: 39)
!1546 = !DILocation(line: 966, column: 38, scope: !1544)
!1547 = !DILocation(line: 966, column: 35, scope: !1544)
!1548 = !DILocation(line: 967, column: 24, scope: !1549)
!1549 = distinct !DILexicalBlock(scope: !1544, file: !589, line: 966, column: 50)
!1550 = !DILocation(line: 969, column: 29, scope: !1551)
!1551 = distinct !DILexicalBlock(scope: !1549, file: !589, line: 969, column: 25)
!1552 = !DILocation(line: 969, column: 25, scope: !1549)
!1553 = !DILocation(line: 978, column: 26, scope: !1096)
!1554 = !DILocation(line: 936, column: 27, scope: !1090)
!1555 = distinct !{!1555, !1490, !1556}
!1556 = !DILocation(line: 979, column: 9, scope: !1090)
!1557 = !DILocation(line: 959, column: 30, scope: !1502)
!1558 = !DILocation(line: 970, column: 63, scope: !1559)
!1559 = distinct !DILexicalBlock(scope: !1551, file: !589, line: 969, column: 35)
!1560 = !DILocation(line: 970, column: 69, scope: !1559)
!1561 = !DILocation(line: 970, column: 84, scope: !1559)
!1562 = !DILocation(line: 970, column: 25, scope: !1559)
!1563 = !DILocation(line: 989, column: 10, scope: !1100)
!1564 = !DILocation(line: 990, column: 13, scope: !1100)
!1565 = !DILocation(line: 990, column: 9, scope: !1100)
!1566 = !DILocation(line: 991, column: 13, scope: !1100)
!1567 = !DILocation(line: 993, column: 23, scope: !1100)
!1568 = !DILocation(line: 994, column: 23, scope: !1100)
!1569 = !DILocation(line: 994, column: 29, scope: !1100)
!1570 = !DILocation(line: 996, column: 53, scope: !1099)
!1571 = !DILocation(line: 996, column: 24, scope: !1099)
!1572 = !DILocation(line: 0, scope: !1099)
!1573 = !DILocation(line: 998, column: 22, scope: !1106)
!1574 = !DILocation(line: 998, column: 13, scope: !1099)
!1575 = !DILocation(line: 999, column: 15, scope: !1105)
!1576 = !DILocation(line: 1001, column: 26, scope: !1105)
!1577 = !DILocation(line: 1005, column: 20, scope: !1105)
!1578 = !DILocation(line: 1005, column: 34, scope: !1105)
!1579 = !DILocation(line: 1005, column: 43, scope: !1105)
!1580 = !DILocation(line: 1005, column: 31, scope: !1105)
!1581 = !DILocation(line: 1011, column: 78, scope: !1102)
!1582 = !DILocation(line: 1011, column: 24, scope: !1102)
!1583 = !DILocation(line: 1012, column: 24, scope: !1102)
!1584 = !DILocation(line: 1020, column: 42, scope: !1102)
!1585 = !DILocation(line: 1021, column: 29, scope: !1102)
!1586 = !DILocation(line: 1021, column: 42, scope: !1102)
!1587 = !DILocation(line: 1021, column: 36, scope: !1102)
!1588 = !DILocation(line: 1024, column: 44, scope: !1589)
!1589 = distinct !DILexicalBlock(scope: !1590, file: !589, line: 1023, column: 32)
!1590 = distinct !DILexicalBlock(scope: !1102, file: !589, line: 1023, column: 25)
!1591 = !DILocation(line: 1024, column: 84, scope: !1589)
!1592 = !DILocation(line: 1024, column: 83, scope: !1589)
!1593 = !DILocation(line: 1032, column: 44, scope: !1594)
!1594 = distinct !DILexicalBlock(scope: !1590, file: !589, line: 1031, column: 28)
!1595 = !DILocation(line: 1032, column: 66, scope: !1594)
!1596 = !DILocation(line: 1006, column: 31, scope: !1104)
!1597 = !DILocation(line: 1006, column: 25, scope: !1104)
!1598 = !DILocation(line: 1006, column: 17, scope: !1104)
!1599 = !DILocation(line: 1010, column: 42, scope: !1103)
!1600 = !DILocation(line: 1010, column: 36, scope: !1103)
!1601 = !DILocation(line: 1010, column: 21, scope: !1103)
!1602 = !DILocation(line: 1010, column: 21, scope: !1104)
!1603 = !DILocation(line: 1011, column: 47, scope: !1102)
!1604 = !DILocation(line: 1011, column: 64, scope: !1102)
!1605 = !DILocation(line: 1011, column: 59, scope: !1102)
!1606 = !DILocation(line: 1011, column: 68, scope: !1102)
!1607 = !DILocation(line: 1011, column: 21, scope: !1102)
!1608 = !DILocation(line: 1011, column: 73, scope: !1102)
!1609 = !DILocation(line: 1012, column: 49, scope: !1102)
!1610 = !DILocation(line: 1012, column: 43, scope: !1102)
!1611 = !DILocation(line: 1012, column: 21, scope: !1102)
!1612 = !DILocation(line: 1012, column: 41, scope: !1102)
!1613 = !DILocation(line: 1014, column: 24, scope: !1102)
!1614 = !DILocation(line: 1018, column: 36, scope: !1102)
!1615 = !DILocation(line: 1019, column: 34, scope: !1102)
!1616 = !DILocation(line: 1019, column: 21, scope: !1102)
!1617 = !DILocation(line: 1019, column: 39, scope: !1102)
!1618 = !DILocation(line: 0, scope: !1102)
!1619 = !DILocation(line: 1021, column: 21, scope: !1102)
!1620 = !DILocation(line: 1023, column: 25, scope: !1590)
!1621 = !DILocation(line: 1023, column: 25, scope: !1102)
!1622 = !DILocation(line: 1024, column: 34, scope: !1589)
!1623 = !DILocation(line: 1024, column: 33, scope: !1589)
!1624 = !DILocation(line: 1025, column: 35, scope: !1625)
!1625 = distinct !DILexicalBlock(scope: !1589, file: !589, line: 1025, column: 29)
!1626 = !DILocation(line: 1025, column: 29, scope: !1589)
!1627 = !DILocation(line: 1026, column: 38, scope: !1628)
!1628 = distinct !DILexicalBlock(scope: !1625, file: !589, line: 1025, column: 44)
!1629 = !DILocation(line: 1027, column: 25, scope: !1628)
!1630 = !DILocation(line: 1028, column: 38, scope: !1631)
!1631 = distinct !DILexicalBlock(scope: !1625, file: !589, line: 1027, column: 32)
!1632 = !DILocation(line: 1029, column: 33, scope: !1631)
!1633 = !DILocation(line: 1032, column: 34, scope: !1594)
!1634 = !DILocation(line: 1032, column: 33, scope: !1594)
!1635 = !DILocation(line: 1033, column: 35, scope: !1636)
!1636 = distinct !DILexicalBlock(scope: !1594, file: !589, line: 1033, column: 29)
!1637 = !DILocation(line: 1033, column: 29, scope: !1594)
!1638 = !DILocation(line: 1034, column: 38, scope: !1639)
!1639 = distinct !DILexicalBlock(scope: !1636, file: !589, line: 1033, column: 49)
!1640 = !DILocation(line: 1035, column: 33, scope: !1639)
!1641 = !DILocation(line: 1036, column: 25, scope: !1639)
!1642 = !DILocation(line: 1042, column: 33, scope: !1104)
!1643 = !DILocation(line: 1042, column: 27, scope: !1104)
!1644 = !DILocation(line: 1042, column: 17, scope: !1104)
!1645 = !DILocation(line: 1043, column: 30, scope: !1104)
!1646 = !DILocation(line: 1005, column: 55, scope: !1105)
!1647 = distinct !{!1647, !1648, !1649}
!1648 = !DILocation(line: 1005, column: 13, scope: !1105)
!1649 = !DILocation(line: 1044, column: 13, scope: !1105)
!1650 = !DILocation(line: 1048, column: 35, scope: !1029)
!1651 = !DILocation(line: 1048, column: 43, scope: !1029)
!1652 = !DILocation(line: 1048, column: 61, scope: !1029)
!1653 = !{!1157, !867, i64 20}
!1654 = !DILocation(line: 1048, column: 50, scope: !1029)
!1655 = !DILocation(line: 1048, column: 47, scope: !1029)
!1656 = !DILocation(line: 1054, column: 7, scope: !1029)
!1657 = !DILocation(line: 1056, column: 18, scope: !1029)
!1658 = !DILocation(line: 1059, column: 12, scope: !1029)
!1659 = !DILocation(line: 1059, column: 5, scope: !1029)
!1660 = !DILocation(line: 1064, column: 39, scope: !1661)
!1661 = distinct !DILexicalBlock(scope: !1110, file: !589, line: 1064, column: 17)
!1662 = !DILocation(line: 1064, column: 25, scope: !1661)
!1663 = !DILocation(line: 1068, column: 17, scope: !1664)
!1664 = distinct !DILexicalBlock(scope: !1110, file: !589, line: 1068, column: 17)
!1665 = !DILocation(line: 1072, column: 21, scope: !1666)
!1666 = distinct !DILexicalBlock(scope: !1664, file: !589, line: 1071, column: 45)
!1667 = !DILocation(line: 1080, column: 20, scope: !1109)
!1668 = !DILocation(line: 1083, column: 37, scope: !1108)
!1669 = !DILocation(line: 1084, column: 21, scope: !1108)
!1670 = !DILocation(line: 1113, column: 27, scope: !1671)
!1671 = distinct !DILexicalBlock(scope: !1672, file: !589, line: 1113, column: 21)
!1672 = distinct !DILexicalBlock(scope: !1673, file: !589, line: 1112, column: 49)
!1673 = distinct !DILexicalBlock(scope: !1110, file: !589, line: 1110, column: 17)
!1674 = !DILocation(line: 1113, column: 35, scope: !1671)
!1675 = !DILocation(line: 1120, column: 32, scope: !1676)
!1676 = distinct !DILexicalBlock(scope: !1112, file: !589, line: 1119, column: 13)
!1677 = !DILocation(line: 1120, column: 24, scope: !1676)
!1678 = !DILocation(line: 131, column: 30, scope: !1679, inlinedAt: !1691)
!1679 = distinct !DISubprogram(name: "history_pre_cut", linkageName: "_ZL15history_pre_cutP7state_tii", scope: !589, file: !589, line: 124, type: !1680, scopeLine: 124, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !588, retainedNodes: !1683)
!1680 = !DISubroutineType(types: !1681)
!1681 = !{!113, !391, !1682, !112}
!1682 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !369)
!1683 = !{!1684, !1685, !1686, !1687, !1688, !1689, !1690}
!1684 = !DILocalVariable(name: "s", arg: 1, scope: !1679, file: !589, line: 124, type: !391)
!1685 = !DILocalVariable(name: "move", arg: 2, scope: !1679, file: !589, line: 124, type: !1682)
!1686 = !DILocalVariable(name: "depth", arg: 3, scope: !1679, file: !589, line: 124, type: !112)
!1687 = !DILocalVariable(name: "piece", scope: !1679, file: !589, line: 125, type: !113)
!1688 = !DILocalVariable(name: "bto", scope: !1679, file: !589, line: 125, type: !113)
!1689 = !DILocalVariable(name: "failure", scope: !1679, file: !589, line: 126, type: !113)
!1690 = !DILocalVariable(name: "success", scope: !1679, file: !589, line: 126, type: !113)
!1691 = distinct !DILocation(line: 1121, column: 16, scope: !1676)
!1692 = !DILocation(line: 1122, column: 22, scope: !1676)
!1693 = !DILocation(line: 1134, column: 19, scope: !1694)
!1694 = distinct !DILexicalBlock(scope: !1112, file: !589, line: 1134, column: 13)
!1695 = !DILocation(line: 1141, column: 26, scope: !1696)
!1696 = distinct !DILexicalBlock(scope: !1694, file: !589, line: 1141, column: 20)
!1697 = !DILocation(line: 1141, column: 20, scope: !1694)
!1698 = !DILocation(line: 1135, column: 24, scope: !1699)
!1699 = distinct !DILexicalBlock(scope: !1700, file: !589, line: 1135, column: 17)
!1700 = distinct !DILexicalBlock(scope: !1694, file: !589, line: 1134, column: 31)
!1701 = !DILocation(line: 1138, column: 24, scope: !1702)
!1702 = distinct !DILexicalBlock(scope: !1700, file: !589, line: 1138, column: 17)
!1703 = !DILocation(line: 1167, column: 17, scope: !1117)
!1704 = !DILocation(line: 1195, column: 21, scope: !1117)
!1705 = !DILocation(line: 1198, column: 70, scope: !1117)
!1706 = !DILocation(line: 1198, column: 16, scope: !1117)
!1707 = !DILocation(line: 1204, column: 23, scope: !1708)
!1708 = distinct !DILexicalBlock(scope: !1117, file: !589, line: 1204, column: 17)
!1709 = !DILocation(line: 1214, column: 34, scope: !1710)
!1710 = distinct !DILexicalBlock(scope: !1708, file: !589, line: 1211, column: 19)
!1711 = !DILocation(line: 1221, column: 68, scope: !1712)
!1712 = distinct !DILexicalBlock(scope: !1713, file: !589, line: 1220, column: 24)
!1713 = distinct !DILexicalBlock(scope: !1714, file: !589, line: 1218, column: 21)
!1714 = distinct !DILexicalBlock(scope: !1715, file: !589, line: 1217, column: 32)
!1715 = distinct !DILexicalBlock(scope: !1117, file: !589, line: 1217, column: 17)
!1716 = !DILocation(line: 1221, column: 67, scope: !1712)
!1717 = !DILocation(line: 1093, column: 44, scope: !1718)
!1718 = distinct !DILexicalBlock(scope: !1110, file: !589, line: 1093, column: 17)
!1719 = !DILocation(line: 1120, column: 20, scope: !1676)
!1720 = !DILocation(line: 1124, column: 31, scope: !1676)
!1721 = !DILocation(line: 1124, column: 22, scope: !1676)
!1722 = !DILocation(line: 1063, column: 16, scope: !1111)
!1723 = !DILocation(line: 1063, column: 20, scope: !1111)
!1724 = !DILocation(line: 0, scope: !1112)
!1725 = !DILocation(line: 1063, column: 13, scope: !1112)
!1726 = !DILocation(line: 1068, column: 43, scope: !1664)
!1727 = !DILocation(line: 1069, column: 17, scope: !1664)
!1728 = !DILocation(line: 1069, column: 21, scope: !1664)
!1729 = !DILocation(line: 1070, column: 21, scope: !1664)
!1730 = !DILocation(line: 1071, column: 24, scope: !1664)
!1731 = !DILocation(line: 1068, column: 17, scope: !1110)
!1732 = !DILocation(line: 1079, column: 17, scope: !1109)
!1733 = !DILocation(line: 1079, column: 36, scope: !1109)
!1734 = !DILocation(line: 1080, column: 17, scope: !1109)
!1735 = !DILocation(line: 1080, column: 50, scope: !1109)
!1736 = !DILocation(line: 1081, column: 17, scope: !1109)
!1737 = !DILocation(line: 1081, column: 20, scope: !1109)
!1738 = !DILocation(line: 1081, column: 52, scope: !1109)
!1739 = !DILocation(line: 1081, column: 49, scope: !1109)
!1740 = !DILocation(line: 1082, column: 17, scope: !1109)
!1741 = !DILocation(line: 1082, column: 20, scope: !1109)
!1742 = !DILocation(line: 1082, column: 40, scope: !1109)
!1743 = !DILocation(line: 1082, column: 37, scope: !1109)
!1744 = !DILocation(line: 1079, column: 17, scope: !1110)
!1745 = !DILocation(line: 1083, column: 82, scope: !1108)
!1746 = !DILocation(line: 1083, column: 30, scope: !1108)
!1747 = !DILocation(line: 0, scope: !1108)
!1748 = !DILocation(line: 1084, column: 28, scope: !1749)
!1749 = distinct !DILexicalBlock(scope: !1108, file: !589, line: 1084, column: 21)
!1750 = !DILocation(line: 1093, column: 17, scope: !1718)
!1751 = !DILocation(line: 1093, column: 26, scope: !1718)
!1752 = !DILocation(line: 1093, column: 34, scope: !1718)
!1753 = !DILocation(line: 1093, column: 31, scope: !1718)
!1754 = !DILocation(line: 1095, column: 28, scope: !1755)
!1755 = distinct !DILexicalBlock(scope: !1718, file: !589, line: 1093, column: 51)
!1756 = !DILocation(line: 1096, column: 13, scope: !1755)
!1757 = !DILocation(line: 1096, column: 25, scope: !1758)
!1758 = distinct !DILexicalBlock(scope: !1718, file: !589, line: 1096, column: 24)
!1759 = !DILocation(line: 1096, column: 32, scope: !1758)
!1760 = !DILocation(line: 1097, column: 28, scope: !1761)
!1761 = distinct !DILexicalBlock(scope: !1758, file: !589, line: 1096, column: 63)
!1762 = !DILocation(line: 1098, column: 21, scope: !1761)
!1763 = !DILocation(line: 1105, column: 17, scope: !1110)
!1764 = !DILocation(line: 1110, column: 17, scope: !1673)
!1765 = !DILocation(line: 1111, column: 17, scope: !1673)
!1766 = !DILocation(line: 1114, column: 28, scope: !1767)
!1767 = distinct !DILexicalBlock(scope: !1671, file: !589, line: 1113, column: 41)
!1768 = !DILocation(line: 1113, column: 21, scope: !1672)
!1769 = !DILocation(line: 1119, column: 13, scope: !1676)
!1770 = !DILocation(line: 1119, column: 32, scope: !1676)
!1771 = !DILocation(line: 1120, column: 13, scope: !1676)
!1772 = !DILocation(line: 0, scope: !1679, inlinedAt: !1691)
!1773 = !DILocation(line: 128, column: 23, scope: !1679, inlinedAt: !1691)
!1774 = !DILocation(line: 128, column: 13, scope: !1679, inlinedAt: !1691)
!1775 = !DILocation(line: 128, column: 35, scope: !1679, inlinedAt: !1691)
!1776 = !DILocation(line: 129, column: 11, scope: !1679, inlinedAt: !1691)
!1777 = !{!872, !867, i64 0}
!1778 = !DILocation(line: 131, column: 15, scope: !1679, inlinedAt: !1691)
!1779 = !{!1780, !1781, i64 0}
!1780 = !{!"array@_ZTSA8_A12_A64_i", !1781, i64 0}
!1781 = !{!"array@_ZTSA12_A64_i", !873, i64 0}
!1782 = !{!1781, !873, i64 0}
!1783 = !{!1780, !867, i64 0}
!1784 = !DILocation(line: 132, column: 15, scope: !1679, inlinedAt: !1691)
!1785 = !DILocation(line: 132, column: 52, scope: !1679, inlinedAt: !1691)
!1786 = !DILocation(line: 134, column: 19, scope: !1679, inlinedAt: !1691)
!1787 = !DILocation(line: 134, column: 30, scope: !1679, inlinedAt: !1691)
!1788 = !DILocation(line: 1122, column: 13, scope: !1676)
!1789 = !DILocation(line: 1123, column: 17, scope: !1676)
!1790 = !DILocation(line: 1125, column: 17, scope: !1676)
!1791 = !DILocation(line: 1126, column: 26, scope: !1792)
!1792 = distinct !DILexicalBlock(scope: !1676, file: !589, line: 1125, column: 37)
!1793 = distinct !{!1793, !1659, !1794}
!1794 = !DILocation(line: 1281, column: 5, scope: !1029)
!1795 = !DILocation(line: 1283, column: 34, scope: !1029)
!1796 = !DILocation(line: 1283, column: 24, scope: !1029)
!1797 = !DILocation(line: 1286, column: 18, scope: !1798)
!1798 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 1286, column: 9)
!1799 = !DILocation(line: 1134, column: 13, scope: !1112)
!1800 = !DILocation(line: 1135, column: 29, scope: !1699)
!1801 = !DILocation(line: 1138, column: 30, scope: !1702)
!1802 = !DILocation(line: 1138, column: 17, scope: !1700)
!1803 = !DILocation(line: 1151, column: 13, scope: !1112)
!1804 = !DILocation(line: 1152, column: 29, scope: !1805)
!1805 = distinct !DILexicalBlock(scope: !1806, file: !589, line: 1151, column: 43)
!1806 = distinct !DILexicalBlock(scope: !1112, file: !589, line: 1151, column: 13)
!1807 = !DILocation(line: 1152, column: 40, scope: !1805)
!1808 = !DILocation(line: 1152, column: 56, scope: !1805)
!1809 = !DILocation(line: 1152, column: 74, scope: !1805)
!1810 = !DILocation(line: 1152, column: 22, scope: !1805)
!1811 = !DILocation(line: 1157, column: 23, scope: !1112)
!1812 = !DILocation(line: 1157, column: 17, scope: !1112)
!1813 = !DILocation(line: 1153, column: 9, scope: !1805)
!1814 = !DILocation(line: 1157, column: 9, scope: !1112)
!1815 = !DILocation(line: 1161, column: 34, scope: !1118)
!1816 = !DILocation(line: 1161, column: 28, scope: !1118)
!1817 = !DILocation(line: 1161, column: 13, scope: !1118)
!1818 = !DILocation(line: 1161, column: 13, scope: !1112)
!1819 = !DILocation(line: 1165, column: 28, scope: !1117)
!1820 = !DILocation(line: 1167, column: 17, scope: !1821)
!1821 = distinct !DILexicalBlock(scope: !1117, file: !589, line: 1167, column: 17)
!1822 = !DILocation(line: 1175, column: 26, scope: !1823)
!1823 = distinct !DILexicalBlock(scope: !1117, file: !589, line: 1175, column: 17)
!1824 = !DILocation(line: 1177, column: 32, scope: !1825)
!1825 = distinct !DILexicalBlock(scope: !1826, file: !589, line: 1177, column: 25)
!1826 = distinct !DILexicalBlock(scope: !1827, file: !589, line: 1176, column: 31)
!1827 = distinct !DILexicalBlock(scope: !1828, file: !589, line: 1176, column: 21)
!1828 = distinct !DILexicalBlock(scope: !1823, file: !589, line: 1175, column: 63)
!1829 = !DILocation(line: 1176, column: 21, scope: !1828)
!1830 = !DILocation(line: 1177, column: 48, scope: !1825)
!1831 = !DILocation(line: 1177, column: 25, scope: !1826)
!1832 = !DILocation(line: 1178, column: 25, scope: !1833)
!1833 = distinct !DILexicalBlock(scope: !1825, file: !589, line: 1177, column: 68)
!1834 = !DILocation(line: 1179, column: 38, scope: !1833)
!1835 = !DILocation(line: 1181, column: 25, scope: !1833)
!1836 = !DILocation(line: 1185, column: 32, scope: !1837)
!1837 = distinct !DILexicalBlock(scope: !1838, file: !589, line: 1185, column: 25)
!1838 = distinct !DILexicalBlock(scope: !1839, file: !589, line: 1184, column: 31)
!1839 = distinct !DILexicalBlock(scope: !1828, file: !589, line: 1184, column: 21)
!1840 = !DILocation(line: 1184, column: 21, scope: !1828)
!1841 = !DILocation(line: 1185, column: 42, scope: !1837)
!1842 = !DILocation(line: 1185, column: 25, scope: !1838)
!1843 = !DILocation(line: 1186, column: 25, scope: !1844)
!1844 = distinct !DILexicalBlock(scope: !1837, file: !589, line: 1185, column: 62)
!1845 = !DILocation(line: 1187, column: 38, scope: !1844)
!1846 = !DILocation(line: 1189, column: 25, scope: !1844)
!1847 = !DILocation(line: 1194, column: 34, scope: !1117)
!1848 = !DILocation(line: 1194, column: 43, scope: !1117)
!1849 = !DILocation(line: 0, scope: !1117)
!1850 = !DILocation(line: 1195, column: 28, scope: !1117)
!1851 = !DILocation(line: 1195, column: 34, scope: !1117)
!1852 = !DILocation(line: 1195, column: 49, scope: !1117)
!1853 = !DILocation(line: 1195, column: 53, scope: !1117)
!1854 = !DILocation(line: 1195, column: 40, scope: !1117)
!1855 = !DILocation(line: 1195, column: 13, scope: !1117)
!1856 = !DILocation(line: 1197, column: 26, scope: !1117)
!1857 = !DILocation(line: 1197, column: 13, scope: !1117)
!1858 = !DILocation(line: 1197, column: 31, scope: !1117)
!1859 = !DILocation(line: 1198, column: 39, scope: !1117)
!1860 = !DILocation(line: 1198, column: 51, scope: !1117)
!1861 = !DILocation(line: 1198, column: 60, scope: !1117)
!1862 = !DILocation(line: 1198, column: 13, scope: !1117)
!1863 = !DILocation(line: 1198, column: 65, scope: !1117)
!1864 = !DILocation(line: 1199, column: 41, scope: !1117)
!1865 = !DILocation(line: 1199, column: 35, scope: !1117)
!1866 = !DILocation(line: 1199, column: 13, scope: !1117)
!1867 = !DILocation(line: 1199, column: 33, scope: !1117)
!1868 = !DILocation(line: 1205, column: 23, scope: !1708)
!1869 = !DILocation(line: 1205, column: 17, scope: !1708)
!1870 = !DILocation(line: 1207, column: 17, scope: !1708)
!1871 = !DILocation(line: 1208, column: 27, scope: !1708)
!1872 = !DILocalVariable(name: "s", arg: 1, scope: !1873, file: !589, line: 111, type: !391)
!1873 = distinct !DISubprogram(name: "history_score", linkageName: "_ZL13history_scoreP7state_ti", scope: !589, file: !589, line: 111, type: !1874, scopeLine: 111, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !588, retainedNodes: !1876)
!1874 = !DISubroutineType(types: !1875)
!1875 = !{!113, !391, !1682}
!1876 = !{!1872, !1877, !1878, !1879, !1880}
!1877 = !DILocalVariable(name: "move", arg: 2, scope: !1873, file: !589, line: 111, type: !1682)
!1878 = !DILocalVariable(name: "piece", scope: !1873, file: !589, line: 112, type: !113)
!1879 = !DILocalVariable(name: "bto", scope: !1873, file: !589, line: 112, type: !113)
!1880 = !DILocalVariable(name: "score", scope: !1873, file: !589, line: 113, type: !113)
!1881 = !DILocation(line: 0, scope: !1873, inlinedAt: !1882)
!1882 = distinct !DILocation(line: 1209, column: 20, scope: !1708)
!1883 = !DILocation(line: 115, column: 23, scope: !1873, inlinedAt: !1882)
!1884 = !DILocation(line: 115, column: 13, scope: !1873, inlinedAt: !1882)
!1885 = !DILocation(line: 115, column: 37, scope: !1873, inlinedAt: !1882)
!1886 = !DILocation(line: 118, column: 30, scope: !1873, inlinedAt: !1882)
!1887 = !DILocation(line: 118, column: 15, scope: !1873, inlinedAt: !1882)
!1888 = !DILocation(line: 118, column: 57, scope: !1873, inlinedAt: !1882)
!1889 = !DILocation(line: 119, column: 17, scope: !1873, inlinedAt: !1882)
!1890 = !DILocation(line: 119, column: 54, scope: !1873, inlinedAt: !1882)
!1891 = !DILocation(line: 119, column: 14, scope: !1873, inlinedAt: !1882)
!1892 = !DILocation(line: 1209, column: 47, scope: !1708)
!1893 = !DILocation(line: 1210, column: 21, scope: !1708)
!1894 = !DILocation(line: 1210, column: 17, scope: !1708)
!1895 = !DILocation(line: 1212, column: 24, scope: !1710)
!1896 = !DILocation(line: 1214, column: 43, scope: !1710)
!1897 = !DILocation(line: 1215, column: 13, scope: !1710)
!1898 = !DILocation(line: 1217, column: 23, scope: !1715)
!1899 = !DILocation(line: 0, scope: !1715)
!1900 = !DILocation(line: 1217, column: 17, scope: !1117)
!1901 = !DILocation(line: 1218, column: 21, scope: !1714)
!1902 = !DILocation(line: 1219, column: 30, scope: !1903)
!1903 = distinct !DILexicalBlock(scope: !1713, file: !589, line: 1218, column: 36)
!1904 = !DILocation(line: 1219, column: 29, scope: !1903)
!1905 = !DILocation(line: 1220, column: 17, scope: !1903)
!1906 = !DILocation(line: 1221, column: 30, scope: !1712)
!1907 = !DILocation(line: 1221, column: 29, scope: !1712)
!1908 = !DILocation(line: 0, scope: !1909)
!1909 = distinct !DILexicalBlock(scope: !1910, file: !589, line: 1224, column: 21)
!1910 = distinct !DILexicalBlock(scope: !1715, file: !589, line: 1223, column: 20)
!1911 = !DILocation(line: 1224, column: 21, scope: !1910)
!1912 = !DILocation(line: 1225, column: 30, scope: !1913)
!1913 = distinct !DILexicalBlock(scope: !1909, file: !589, line: 1224, column: 36)
!1914 = !DILocation(line: 1226, column: 17, scope: !1913)
!1915 = !DILocation(line: 1227, column: 30, scope: !1916)
!1916 = distinct !DILexicalBlock(scope: !1909, file: !589, line: 1226, column: 24)
!1917 = !DILocation(line: 1229, column: 27, scope: !1918)
!1918 = distinct !DILexicalBlock(scope: !1910, file: !589, line: 1229, column: 21)
!1919 = !DILocation(line: 1230, column: 35, scope: !1918)
!1920 = !DILocation(line: 1230, column: 25, scope: !1918)
!1921 = !DILocation(line: 1230, column: 21, scope: !1918)
!1922 = !DILocation(line: 1231, column: 31, scope: !1923)
!1923 = distinct !DILexicalBlock(scope: !1924, file: !589, line: 1231, column: 25)
!1924 = distinct !DILexicalBlock(scope: !1918, file: !589, line: 1230, column: 46)
!1925 = !DILocation(line: 1232, column: 29, scope: !1926)
!1926 = distinct !DILexicalBlock(scope: !1927, file: !589, line: 1232, column: 29)
!1927 = distinct !DILexicalBlock(scope: !1923, file: !589, line: 1231, column: 40)
!1928 = !DILocation(line: 1232, column: 29, scope: !1927)
!1929 = !DILocation(line: 1233, column: 36, scope: !1930)
!1930 = distinct !DILexicalBlock(scope: !1926, file: !589, line: 1232, column: 36)
!1931 = !DILocation(line: 1235, column: 44, scope: !1932)
!1932 = distinct !DILexicalBlock(scope: !1927, file: !589, line: 1235, column: 29)
!1933 = !DILocation(line: 1235, column: 36, scope: !1932)
!1934 = !DILocation(line: 1236, column: 46, scope: !1935)
!1935 = distinct !DILexicalBlock(scope: !1932, file: !589, line: 1235, column: 54)
!1936 = !DILocation(line: 1237, column: 42, scope: !1937)
!1937 = distinct !DILexicalBlock(scope: !1935, file: !589, line: 1237, column: 33)
!1938 = !DILocation(line: 1237, column: 33, scope: !1935)
!1939 = !DILocation(line: 1238, column: 42, scope: !1940)
!1940 = distinct !DILexicalBlock(scope: !1937, file: !589, line: 1237, column: 48)
!1941 = !DILocation(line: 1238, column: 41, scope: !1940)
!1942 = !DILocation(line: 1239, column: 29, scope: !1940)
!1943 = !DILocation(line: 1236, column: 55, scope: !1935)
!1944 = !DILocation(line: 1240, column: 80, scope: !1945)
!1945 = distinct !DILexicalBlock(scope: !1937, file: !589, line: 1239, column: 36)
!1946 = !DILocation(line: 1240, column: 42, scope: !1945)
!1947 = !DILocation(line: 1240, column: 41, scope: !1945)
!1948 = !DILocation(line: 1247, column: 23, scope: !1949)
!1949 = distinct !DILexicalBlock(scope: !1117, file: !589, line: 1247, column: 17)
!1950 = !DILocation(line: 1247, column: 17, scope: !1117)
!1951 = !DILocation(line: 1250, column: 9, scope: !1117)
!1952 = !DILocation(line: 1252, column: 25, scope: !1112)
!1953 = !DILocation(line: 1252, column: 19, scope: !1112)
!1954 = !DILocation(line: 1252, column: 9, scope: !1112)
!1955 = !DILocation(line: 1254, column: 23, scope: !1956)
!1956 = distinct !DILexicalBlock(scope: !1112, file: !589, line: 1254, column: 13)
!1957 = !DILocation(line: 1254, column: 13, scope: !1956)
!1958 = !DILocation(line: 1254, column: 13, scope: !1112)
!1959 = !DILocation(line: 1258, column: 13, scope: !1960)
!1960 = distinct !DILexicalBlock(scope: !1112, file: !589, line: 1258, column: 13)
!1961 = !DILocation(line: 1258, column: 13, scope: !1112)
!1962 = !DILocation(line: 1259, column: 23, scope: !1963)
!1963 = distinct !DILexicalBlock(scope: !1964, file: !589, line: 1259, column: 17)
!1964 = distinct !DILexicalBlock(scope: !1960, file: !589, line: 1258, column: 25)
!1965 = !DILocation(line: 1259, column: 17, scope: !1964)
!1966 = !DILocation(line: 1260, column: 27, scope: !1967)
!1967 = distinct !DILexicalBlock(scope: !1968, file: !589, line: 1260, column: 21)
!1968 = distinct !DILexicalBlock(scope: !1963, file: !589, line: 1259, column: 32)
!1969 = !DILocation(line: 0, scope: !1968)
!1970 = !DILocation(line: 1260, column: 21, scope: !1968)
!1971 = !DILocation(line: 1261, column: 21, scope: !1972)
!1972 = distinct !DILexicalBlock(scope: !1967, file: !589, line: 1260, column: 36)
!1973 = !DILocation(line: 1263, column: 35, scope: !1974)
!1974 = distinct !DILexicalBlock(scope: !1975, file: !589, line: 1263, column: 21)
!1975 = distinct !DILexicalBlock(scope: !1972, file: !589, line: 1263, column: 21)
!1976 = !DILocation(line: 1263, column: 21, scope: !1975)
!1977 = !DILocation(line: 98, column: 56, scope: !1978, inlinedAt: !1991)
!1978 = distinct !DILexicalBlock(scope: !1979, file: !589, line: 94, column: 54)
!1979 = distinct !DILexicalBlock(scope: !1980, file: !589, line: 94, column: 9)
!1980 = distinct !DISubprogram(name: "history_bad", linkageName: "_ZL11history_badP7state_tii", scope: !589, file: !589, line: 90, type: !1981, scopeLine: 90, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !588, retainedNodes: !1983)
!1981 = !DISubroutineType(types: !1982)
!1982 = !{null, !391, !1682, !112}
!1983 = !{!1984, !1985, !1986, !1987, !1988, !1989, !1990}
!1984 = !DILocalVariable(name: "s", arg: 1, scope: !1980, file: !589, line: 90, type: !391)
!1985 = !DILocalVariable(name: "move", arg: 2, scope: !1980, file: !589, line: 90, type: !1682)
!1986 = !DILocalVariable(name: "depth", arg: 3, scope: !1980, file: !589, line: 90, type: !112)
!1987 = !DILocalVariable(name: "i", scope: !1980, file: !589, line: 91, type: !113)
!1988 = !DILocalVariable(name: "j", scope: !1980, file: !589, line: 91, type: !113)
!1989 = !DILocalVariable(name: "piece", scope: !1980, file: !589, line: 92, type: !113)
!1990 = !DILocalVariable(name: "bto", scope: !1980, file: !589, line: 92, type: !113)
!1991 = distinct !DILocation(line: 1264, column: 25, scope: !1992)
!1992 = distinct !DILexicalBlock(scope: !1974, file: !589, line: 1263, column: 50)
!1993 = !DILocation(line: 98, column: 61, scope: !1978, inlinedAt: !1991)
!1994 = !DILocation(line: 1264, column: 40, scope: !1992)
!1995 = !DILocation(line: 0, scope: !1980, inlinedAt: !1991)
!1996 = !DILocation(line: 94, column: 34, scope: !1979, inlinedAt: !1991)
!1997 = !DILocation(line: 95, column: 27, scope: !1978, inlinedAt: !1991)
!1998 = !DILocation(line: 95, column: 17, scope: !1978, inlinedAt: !1991)
!1999 = !DILocation(line: 95, column: 39, scope: !1978, inlinedAt: !1991)
!2000 = !DILocation(line: 96, column: 15, scope: !1978, inlinedAt: !1991)
!2001 = !DILocation(line: 98, column: 24, scope: !1978, inlinedAt: !1991)
!2002 = !DILocation(line: 98, column: 9, scope: !1978, inlinedAt: !1991)
!2003 = !DILocation(line: 98, column: 46, scope: !1978, inlinedAt: !1991)
!2004 = !DILocation(line: 100, column: 50, scope: !2005, inlinedAt: !1991)
!2005 = distinct !DILexicalBlock(scope: !1978, file: !589, line: 100, column: 13)
!2006 = !DILocation(line: 100, column: 13, scope: !1978, inlinedAt: !1991)
!2007 = !DILocation(line: 103, column: 55, scope: !2008, inlinedAt: !1991)
!2008 = distinct !DILexicalBlock(scope: !2009, file: !589, line: 102, column: 42)
!2009 = distinct !DILexicalBlock(scope: !2010, file: !589, line: 102, column: 17)
!2010 = distinct !DILexicalBlock(scope: !2011, file: !589, line: 102, column: 17)
!2011 = distinct !DILexicalBlock(scope: !2012, file: !589, line: 101, column: 38)
!2012 = distinct !DILexicalBlock(scope: !2013, file: !589, line: 101, column: 13)
!2013 = distinct !DILexicalBlock(scope: !2014, file: !589, line: 101, column: 13)
!2014 = distinct !DILexicalBlock(scope: !2005, file: !589, line: 100, column: 59)
!2015 = !DILocation(line: 101, column: 13, scope: !2013, inlinedAt: !1991)
!2016 = !DILocation(line: 104, column: 55, scope: !2008, inlinedAt: !1991)
!2017 = !DILocation(line: 102, column: 17, scope: !2010, inlinedAt: !1991)
!2018 = !DILocation(line: 103, column: 86, scope: !2008, inlinedAt: !1991)
!2019 = !DILocation(line: 103, column: 91, scope: !2008, inlinedAt: !1991)
!2020 = !DILocation(line: 103, column: 52, scope: !2008, inlinedAt: !1991)
!2021 = !DILocation(line: 104, column: 86, scope: !2008, inlinedAt: !1991)
!2022 = !DILocation(line: 104, column: 91, scope: !2008, inlinedAt: !1991)
!2023 = !DILocation(line: 104, column: 52, scope: !2008, inlinedAt: !1991)
!2024 = !DILocation(line: 102, column: 38, scope: !2009, inlinedAt: !1991)
!2025 = !DILocation(line: 102, column: 31, scope: !2009, inlinedAt: !1991)
!2026 = distinct !{!2026, !2017, !2027}
!2027 = !DILocation(line: 105, column: 17, scope: !2010, inlinedAt: !1991)
!2028 = !DILocation(line: 101, column: 34, scope: !2012, inlinedAt: !1991)
!2029 = !DILocation(line: 101, column: 27, scope: !2012, inlinedAt: !1991)
!2030 = distinct !{!2030, !2015, !2031}
!2031 = !DILocation(line: 106, column: 13, scope: !2013, inlinedAt: !1991)
!2032 = !DILocation(line: 1263, column: 46, scope: !1974)
!2033 = distinct !{!2033, !1976, !2034}
!2034 = !DILocation(line: 1265, column: 21, scope: !1975)
!2035 = !DILocation(line: 1266, column: 75, scope: !1972)
!2036 = !DILocation(line: 1266, column: 69, scope: !1972)
!2037 = !DILocation(line: 1266, column: 56, scope: !1972)
!2038 = !DILocation(line: 1266, column: 79, scope: !1972)
!2039 = !DILocation(line: 1266, column: 86, scope: !1972)
!2040 = !DILocation(line: 1267, column: 29, scope: !1972)
!2041 = !DILocation(line: 1266, column: 21, scope: !1972)
!2042 = !DILocation(line: 1268, column: 21, scope: !1972)
!2043 = !DILocation(line: 1272, column: 24, scope: !1968)
!2044 = !DILocation(line: 1272, column: 22, scope: !1968)
!2045 = !DILocation(line: 1273, column: 13, scope: !1968)
!2046 = !DILocation(line: 1276, column: 44, scope: !1964)
!2047 = !DILocation(line: 1276, column: 38, scope: !1964)
!2048 = !DILocation(line: 1276, column: 31, scope: !1964)
!2049 = !DILocation(line: 1276, column: 13, scope: !1964)
!2050 = !DILocation(line: 1276, column: 36, scope: !1964)
!2051 = !DILocation(line: 1277, column: 15, scope: !1964)
!2052 = !DILocation(line: 1278, column: 9, scope: !1964)
!2053 = !DILocation(line: 1280, column: 22, scope: !1112)
!2054 = !DILocation(line: 1286, column: 9, scope: !1798)
!2055 = !DILocation(line: 1287, column: 13, scope: !2056)
!2056 = distinct !DILexicalBlock(scope: !2057, file: !589, line: 1287, column: 13)
!2057 = distinct !DILexicalBlock(scope: !1798, file: !589, line: 1286, column: 34)
!2058 = !DILocation(line: 1291, column: 46, scope: !2059)
!2059 = distinct !DILexicalBlock(scope: !2056, file: !589, line: 1290, column: 16)
!2060 = !DILocation(line: 1291, column: 53, scope: !2059)
!2061 = !DILocation(line: 1291, column: 62, scope: !2059)
!2062 = !DILocation(line: 1287, column: 13, scope: !2057)
!2063 = !DILocation(line: 1288, column: 34, scope: !2064)
!2064 = distinct !DILexicalBlock(scope: !2056, file: !589, line: 1287, column: 26)
!2065 = !DILocation(line: 1288, column: 29, scope: !2064)
!2066 = !DILocation(line: 1288, column: 13, scope: !2064)
!2067 = !DILocation(line: 1289, column: 32, scope: !2064)
!2068 = !DILocation(line: 1289, column: 27, scope: !2064)
!2069 = !DILocation(line: 1289, column: 13, scope: !2064)
!2070 = !DILocation(line: 1291, column: 13, scope: !2059)
!2071 = !DILocation(line: 1292, column: 13, scope: !2059)
!2072 = !DILocation(line: 1295, column: 16, scope: !2073)
!2073 = distinct !DILexicalBlock(scope: !2074, file: !589, line: 1295, column: 13)
!2074 = distinct !DILexicalBlock(scope: !1798, file: !589, line: 1294, column: 12)
!2075 = !DILocation(line: 1295, column: 22, scope: !2073)
!2076 = !DILocation(line: 1295, column: 13, scope: !2074)
!2077 = !DILocation(line: 1301, column: 49, scope: !2078)
!2078 = distinct !DILexicalBlock(scope: !2079, file: !589, line: 1300, column: 22)
!2079 = distinct !DILexicalBlock(scope: !1029, file: !589, line: 1300, column: 9)
!2080 = !DILocation(line: 1301, column: 54, scope: !2078)
!2081 = !DILocation(line: 1301, column: 61, scope: !2078)
!2082 = !DILocation(line: 1301, column: 70, scope: !2078)
!2083 = !DILocation(line: 1301, column: 9, scope: !2078)
!2084 = !DILocation(line: 1302, column: 5, scope: !2078)
!2085 = !DILocation(line: 1305, column: 1, scope: !1029)
!2086 = distinct !DISubprogram(name: "qsearch", linkageName: "_Z7qsearchP7state_tiiii", scope: !589, file: !589, line: 412, type: !2087, scopeLine: 412, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !588, retainedNodes: !2089)
!2087 = !DISubroutineType(types: !2088)
!2088 = !{!113, !391, !113, !113, !113, !113}
!2089 = !{!2090, !2091, !2092, !2093, !2094, !2095, !2096, !2097, !2098, !2099, !2100, !2101, !2102, !2103, !2104, !2105, !2106, !2107, !2108, !2109, !2110, !2111, !2112, !2113, !2114, !2121, !2125, !2128, !2129}
!2090 = !DILocalVariable(name: "s", arg: 1, scope: !2086, file: !589, line: 412, type: !391)
!2091 = !DILocalVariable(name: "alpha", arg: 2, scope: !2086, file: !589, line: 412, type: !113)
!2092 = !DILocalVariable(name: "beta", arg: 3, scope: !2086, file: !589, line: 412, type: !113)
!2093 = !DILocalVariable(name: "depth", arg: 4, scope: !2086, file: !589, line: 412, type: !113)
!2094 = !DILocalVariable(name: "qply", arg: 5, scope: !2086, file: !589, line: 412, type: !113)
!2095 = !DILocalVariable(name: "num_moves", scope: !2086, file: !589, line: 413, type: !113)
!2096 = !DILocalVariable(name: "i", scope: !2086, file: !589, line: 413, type: !113)
!2097 = !DILocalVariable(name: "score", scope: !2086, file: !589, line: 414, type: !113)
!2098 = !DILocalVariable(name: "standpat", scope: !2086, file: !589, line: 414, type: !113)
!2099 = !DILocalVariable(name: "legal_move", scope: !2086, file: !589, line: 415, type: !113)
!2100 = !DILocalVariable(name: "no_moves", scope: !2086, file: !589, line: 415, type: !113)
!2101 = !DILocalVariable(name: "delta", scope: !2086, file: !589, line: 416, type: !113)
!2102 = !DILocalVariable(name: "best", scope: !2086, file: !589, line: 417, type: !10)
!2103 = !DILocalVariable(name: "sbest", scope: !2086, file: !589, line: 417, type: !10)
!2104 = !DILocalVariable(name: "originalalpha", scope: !2086, file: !589, line: 418, type: !113)
!2105 = !DILocalVariable(name: "bound", scope: !2086, file: !589, line: 419, type: !113)
!2106 = !DILocalVariable(name: "xdummy", scope: !2086, file: !589, line: 419, type: !113)
!2107 = !DILocalVariable(name: "incheck", scope: !2086, file: !589, line: 420, type: !113)
!2108 = !DILocalVariable(name: "afterincheck", scope: !2086, file: !589, line: 420, type: !113)
!2109 = !DILocalVariable(name: "pass", scope: !2086, file: !589, line: 421, type: !113)
!2110 = !DILocalVariable(name: "multipass", scope: !2086, file: !589, line: 421, type: !113)
!2111 = !DILocalVariable(name: "standpatmargin", scope: !2086, file: !589, line: 422, type: !113)
!2112 = !DILocalVariable(name: "moves", scope: !2086, file: !589, line: 423, type: !609)
!2113 = !DILocalVariable(name: "move_ordering", scope: !2086, file: !589, line: 424, type: !617)
!2114 = !DILocalVariable(name: "npieces", scope: !2115, file: !589, line: 483, type: !481)
!2115 = distinct !DILexicalBlock(scope: !2116, file: !589, line: 482, column: 16)
!2116 = distinct !DILexicalBlock(scope: !2117, file: !589, line: 479, column: 20)
!2117 = distinct !DILexicalBlock(scope: !2118, file: !589, line: 477, column: 20)
!2118 = distinct !DILexicalBlock(scope: !2119, file: !589, line: 474, column: 13)
!2119 = distinct !DILexicalBlock(scope: !2120, file: !589, line: 473, column: 19)
!2120 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 473, column: 9)
!2121 = !DILocalVariable(name: "posteval", scope: !2122, file: !589, line: 599, type: !113)
!2122 = distinct !DILexicalBlock(scope: !2123, file: !589, line: 593, column: 39)
!2123 = distinct !DILexicalBlock(scope: !2124, file: !589, line: 593, column: 13)
!2124 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 567, column: 66)
!2125 = !DILocalVariable(name: "newdepth", scope: !2126, file: !589, line: 602, type: !113)
!2126 = distinct !DILexicalBlock(scope: !2127, file: !589, line: 601, column: 48)
!2127 = distinct !DILexicalBlock(scope: !2122, file: !589, line: 601, column: 17)
!2128 = !DILabel(scope: !2086, name: "mpass", file: !589, line: 555)
!2129 = !DILabel(scope: !2086, name: "endpass", file: !589, line: 636)
!2130 = !DILocation(line: 0, scope: !2086)
!2131 = !DILocation(line: 413, column: 5, scope: !2086)
!2132 = !DILocation(line: 417, column: 5, scope: !2086)
!2133 = !DILocation(line: 419, column: 5, scope: !2086)
!2134 = !DILocation(line: 423, column: 5, scope: !2086)
!2135 = !DILocation(line: 423, column: 12, scope: !2086)
!2136 = !DILocation(line: 424, column: 5, scope: !2086)
!2137 = !DILocation(line: 424, column: 9, scope: !2086)
!2138 = !DILocation(line: 426, column: 8, scope: !2086)
!2139 = !DILocation(line: 426, column: 13, scope: !2086)
!2140 = !DILocation(line: 427, column: 8, scope: !2086)
!2141 = !{!872, !874, i64 4088}
!2142 = !DILocation(line: 427, column: 14, scope: !2086)
!2143 = !DILocation(line: 429, column: 12, scope: !2144)
!2144 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 429, column: 9)
!2145 = !DILocation(line: 429, column: 21, scope: !2144)
!2146 = !{!872, !867, i64 4096}
!2147 = !DILocation(line: 429, column: 16, scope: !2144)
!2148 = !DILocation(line: 429, column: 9, scope: !2086)
!2149 = !DILocation(line: 430, column: 19, scope: !2150)
!2150 = distinct !DILexicalBlock(scope: !2144, file: !589, line: 429, column: 29)
!2151 = !DILocation(line: 431, column: 5, scope: !2150)
!2152 = !DILocation(line: 433, column: 9, scope: !2153)
!2153 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 433, column: 9)
!2154 = !DILocation(line: 433, column: 9, scope: !2086)
!2155 = !DILocation(line: 437, column: 9, scope: !2156)
!2156 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 437, column: 9)
!2157 = !DILocation(line: 437, column: 32, scope: !2156)
!2158 = !DILocation(line: 437, column: 38, scope: !2156)
!2159 = !DILocation(line: 437, column: 44, scope: !2156)
!2160 = !DILocation(line: 437, column: 9, scope: !2086)
!2161 = !DILocation(line: 438, column: 27, scope: !2162)
!2162 = distinct !DILexicalBlock(scope: !2156, file: !589, line: 437, column: 50)
!2163 = !DILocation(line: 438, column: 44, scope: !2162)
!2164 = !DILocation(line: 438, column: 38, scope: !2162)
!2165 = !DILocation(line: 0, scope: !2162)
!2166 = !DILocation(line: 438, column: 17, scope: !2162)
!2167 = !DILocation(line: 438, column: 9, scope: !2162)
!2168 = !DILocation(line: 441, column: 13, scope: !2086)
!2169 = !DILocation(line: 441, column: 5, scope: !2086)
!2170 = !DILocation(line: 443, column: 20, scope: !2171)
!2171 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 441, column: 92)
!2172 = !DILocation(line: 443, column: 13, scope: !2171)
!2173 = !DILocation(line: 446, column: 17, scope: !2174)
!2174 = distinct !DILexicalBlock(scope: !2171, file: !589, line: 446, column: 17)
!2175 = !DILocation(line: 446, column: 23, scope: !2174)
!2176 = !DILocation(line: 446, column: 17, scope: !2171)
!2177 = !DILocation(line: 451, column: 17, scope: !2178)
!2178 = distinct !DILexicalBlock(scope: !2171, file: !589, line: 451, column: 17)
!2179 = !DILocation(line: 451, column: 23, scope: !2178)
!2180 = !DILocation(line: 451, column: 17, scope: !2171)
!2181 = !DILocation(line: 458, column: 18, scope: !2171)
!2182 = !DILocation(line: 459, column: 13, scope: !2171)
!2183 = !DILocation(line: 462, column: 29, scope: !2184)
!2184 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 462, column: 9)
!2185 = !DILocation(line: 462, column: 18, scope: !2184)
!2186 = !DILocation(line: 462, column: 15, scope: !2184)
!2187 = !DILocation(line: 462, column: 38, scope: !2184)
!2188 = !DILocation(line: 462, column: 44, scope: !2184)
!2189 = !DILocation(line: 462, column: 48, scope: !2184)
!2190 = !DILocation(line: 462, column: 9, scope: !2086)
!2191 = !DILocation(line: 463, column: 16, scope: !2192)
!2192 = distinct !DILexicalBlock(scope: !2184, file: !589, line: 462, column: 60)
!2193 = !DILocation(line: 463, column: 9, scope: !2192)
!2194 = !DILocation(line: 466, column: 18, scope: !2086)
!2195 = !DILocation(line: 466, column: 15, scope: !2086)
!2196 = !DILocation(line: 470, column: 16, scope: !2086)
!2197 = !DILocation(line: 471, column: 31, scope: !2086)
!2198 = !DILocation(line: 473, column: 10, scope: !2120)
!2199 = !DILocation(line: 473, column: 9, scope: !2086)
!2200 = !DILocation(line: 474, column: 22, scope: !2118)
!2201 = !DILocation(line: 474, column: 13, scope: !2119)
!2202 = !DILocation(line: 475, column: 55, scope: !2203)
!2203 = distinct !DILexicalBlock(scope: !2118, file: !589, line: 474, column: 31)
!2204 = !DILocation(line: 475, column: 13, scope: !2203)
!2205 = !DILocation(line: 476, column: 13, scope: !2203)
!2206 = !DILocation(line: 477, column: 29, scope: !2117)
!2207 = !DILocation(line: 477, column: 20, scope: !2118)
!2208 = !DILocation(line: 479, column: 35, scope: !2116)
!2209 = !DILocation(line: 479, column: 47, scope: !2116)
!2210 = !DILocation(line: 479, column: 20, scope: !2117)
!2211 = !DILocation(line: 480, column: 73, scope: !2212)
!2212 = distinct !DILexicalBlock(scope: !2116, file: !589, line: 479, column: 57)
!2213 = !DILocation(line: 480, column: 13, scope: !2212)
!2214 = !DILocation(line: 481, column: 13, scope: !2212)
!2215 = !DILocation(line: 483, column: 31, scope: !2115)
!2216 = !DILocation(line: 483, column: 28, scope: !2115)
!2217 = !DILocation(line: 0, scope: !2115)
!2218 = !DILocation(line: 484, column: 20, scope: !2219)
!2219 = distinct !DILexicalBlock(scope: !2115, file: !589, line: 484, column: 17)
!2220 = !DILocation(line: 484, column: 17, scope: !2219)
!2221 = !DILocation(line: 484, column: 17, scope: !2115)
!2222 = !DILocation(line: 485, column: 22, scope: !2223)
!2223 = distinct !DILexicalBlock(scope: !2224, file: !589, line: 485, column: 21)
!2224 = distinct !DILexicalBlock(scope: !2219, file: !589, line: 484, column: 35)
!2225 = !DILocation(line: 485, column: 21, scope: !2224)
!2226 = !DILocation(line: 486, column: 26, scope: !2227)
!2227 = distinct !DILexicalBlock(scope: !2228, file: !589, line: 486, column: 25)
!2228 = distinct !DILexicalBlock(scope: !2223, file: !589, line: 485, column: 39)
!2229 = !DILocation(line: 486, column: 25, scope: !2228)
!2230 = !DILocation(line: 487, column: 30, scope: !2231)
!2231 = distinct !DILexicalBlock(scope: !2232, file: !589, line: 487, column: 29)
!2232 = distinct !DILexicalBlock(scope: !2227, file: !589, line: 486, column: 42)
!2233 = !DILocation(line: 487, column: 47, scope: !2231)
!2234 = !DILocation(line: 487, column: 51, scope: !2231)
!2235 = !DILocation(line: 487, column: 29, scope: !2232)
!2236 = !DILocation(line: 488, column: 48, scope: !2237)
!2237 = distinct !DILexicalBlock(scope: !2238, file: !589, line: 488, column: 33)
!2238 = distinct !DILexicalBlock(scope: !2231, file: !589, line: 487, column: 69)
!2239 = !DILocation(line: 488, column: 59, scope: !2237)
!2240 = !DILocation(line: 488, column: 33, scope: !2238)
!2241 = !DILocation(line: 492, column: 48, scope: !2242)
!2242 = distinct !DILexicalBlock(scope: !2243, file: !589, line: 492, column: 33)
!2243 = distinct !DILexicalBlock(scope: !2231, file: !589, line: 491, column: 32)
!2244 = !DILocation(line: 492, column: 61, scope: !2242)
!2245 = !DILocation(line: 492, column: 33, scope: !2243)
!2246 = !DILocation(line: 497, column: 44, scope: !2247)
!2247 = distinct !DILexicalBlock(scope: !2248, file: !589, line: 497, column: 29)
!2248 = distinct !DILexicalBlock(scope: !2227, file: !589, line: 496, column: 28)
!2249 = !DILocation(line: 497, column: 55, scope: !2247)
!2250 = !DILocation(line: 497, column: 29, scope: !2248)
!2251 = !DILocation(line: 498, column: 88, scope: !2252)
!2252 = distinct !DILexicalBlock(scope: !2247, file: !589, line: 497, column: 65)
!2253 = !DILocation(line: 498, column: 29, scope: !2252)
!2254 = !DILocation(line: 499, column: 29, scope: !2252)
!2255 = !DILocation(line: 504, column: 22, scope: !2256)
!2256 = distinct !DILexicalBlock(scope: !2257, file: !589, line: 504, column: 21)
!2257 = distinct !DILexicalBlock(scope: !2219, file: !589, line: 503, column: 20)
!2258 = !DILocation(line: 504, column: 21, scope: !2257)
!2259 = !DILocation(line: 505, column: 26, scope: !2260)
!2260 = distinct !DILexicalBlock(scope: !2261, file: !589, line: 505, column: 25)
!2261 = distinct !DILexicalBlock(scope: !2256, file: !589, line: 504, column: 39)
!2262 = !DILocation(line: 505, column: 25, scope: !2261)
!2263 = !DILocation(line: 506, column: 30, scope: !2264)
!2264 = distinct !DILexicalBlock(scope: !2265, file: !589, line: 506, column: 29)
!2265 = distinct !DILexicalBlock(scope: !2260, file: !589, line: 505, column: 42)
!2266 = !DILocation(line: 506, column: 47, scope: !2264)
!2267 = !DILocation(line: 506, column: 51, scope: !2264)
!2268 = !DILocation(line: 506, column: 29, scope: !2265)
!2269 = !DILocation(line: 507, column: 48, scope: !2270)
!2270 = distinct !DILexicalBlock(scope: !2271, file: !589, line: 507, column: 33)
!2271 = distinct !DILexicalBlock(scope: !2264, file: !589, line: 506, column: 69)
!2272 = !DILocation(line: 507, column: 59, scope: !2270)
!2273 = !DILocation(line: 507, column: 33, scope: !2271)
!2274 = !DILocation(line: 511, column: 48, scope: !2275)
!2275 = distinct !DILexicalBlock(scope: !2276, file: !589, line: 511, column: 33)
!2276 = distinct !DILexicalBlock(scope: !2264, file: !589, line: 510, column: 32)
!2277 = !DILocation(line: 511, column: 61, scope: !2275)
!2278 = !DILocation(line: 511, column: 33, scope: !2276)
!2279 = !DILocation(line: 516, column: 44, scope: !2280)
!2280 = distinct !DILexicalBlock(scope: !2281, file: !589, line: 516, column: 29)
!2281 = distinct !DILexicalBlock(scope: !2260, file: !589, line: 515, column: 28)
!2282 = !DILocation(line: 516, column: 55, scope: !2280)
!2283 = !DILocation(line: 516, column: 29, scope: !2281)
!2284 = !DILocation(line: 517, column: 88, scope: !2285)
!2285 = distinct !DILexicalBlock(scope: !2280, file: !589, line: 516, column: 65)
!2286 = !DILocation(line: 517, column: 29, scope: !2285)
!2287 = !DILocation(line: 518, column: 29, scope: !2285)
!2288 = !DILocation(line: 529, column: 23, scope: !2289)
!2289 = distinct !DILexicalBlock(scope: !2290, file: !589, line: 528, column: 19)
!2290 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 528, column: 9)
!2291 = !DILocation(line: 530, column: 5, scope: !2289)
!2292 = !DILocation(line: 538, column: 13, scope: !2086)
!2293 = !DILocation(line: 540, column: 15, scope: !2294)
!2294 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 540, column: 9)
!2295 = !DILocation(line: 0, scope: !2294)
!2296 = !DILocation(line: 540, column: 9, scope: !2086)
!2297 = !DILocation(line: 541, column: 13, scope: !2298)
!2298 = distinct !DILexicalBlock(scope: !2294, file: !589, line: 540, column: 21)
!2299 = !DILocation(line: 542, column: 25, scope: !2300)
!2300 = distinct !DILexicalBlock(scope: !2301, file: !589, line: 541, column: 22)
!2301 = distinct !DILexicalBlock(scope: !2298, file: !589, line: 541, column: 13)
!2302 = !DILocation(line: 543, column: 9, scope: !2300)
!2303 = !DILocation(line: 544, column: 25, scope: !2304)
!2304 = distinct !DILexicalBlock(scope: !2301, file: !589, line: 543, column: 16)
!2305 = !DILocation(line: 548, column: 13, scope: !2306)
!2306 = distinct !DILexicalBlock(scope: !2294, file: !589, line: 547, column: 12)
!2307 = !DILocation(line: 549, column: 25, scope: !2308)
!2308 = distinct !DILexicalBlock(scope: !2309, file: !589, line: 548, column: 23)
!2309 = distinct !DILexicalBlock(scope: !2306, file: !589, line: 548, column: 13)
!2310 = !DILocation(line: 550, column: 9, scope: !2308)
!2311 = !DILocation(line: 551, column: 25, scope: !2312)
!2312 = distinct !DILexicalBlock(scope: !2309, file: !589, line: 550, column: 16)
!2313 = !DILocation(line: 562, column: 32, scope: !2086)
!2314 = !DILocation(line: 128, column: 16, scope: !1679, inlinedAt: !2315)
!2315 = distinct !DILocation(line: 578, column: 21, scope: !2316)
!2316 = distinct !DILexicalBlock(scope: !2317, file: !589, line: 578, column: 21)
!2317 = distinct !DILexicalBlock(scope: !2318, file: !589, line: 577, column: 28)
!2318 = distinct !DILexicalBlock(scope: !2319, file: !589, line: 577, column: 17)
!2319 = distinct !DILexicalBlock(scope: !2320, file: !589, line: 568, column: 23)
!2320 = distinct !DILexicalBlock(scope: !2124, file: !589, line: 568, column: 13)
!2321 = !DILocation(line: 131, column: 30, scope: !1679, inlinedAt: !2315)
!2322 = !DILocation(line: 583, column: 28, scope: !2323)
!2323 = distinct !DILexicalBlock(scope: !2324, file: !589, line: 583, column: 21)
!2324 = distinct !DILexicalBlock(scope: !2325, file: !589, line: 582, column: 122)
!2325 = distinct !DILexicalBlock(scope: !2319, file: !589, line: 582, column: 17)
!2326 = !DILocation(line: 594, column: 70, scope: !2122)
!2327 = !DILocation(line: 594, column: 16, scope: !2122)
!2328 = !DILocation(line: 595, column: 16, scope: !2122)
!2329 = !DILocation(line: 599, column: 37, scope: !2122)
!2330 = !DILocation(line: 611, column: 67, scope: !2126)
!2331 = !DILocation(line: 641, column: 19, scope: !2332)
!2332 = distinct !DILexicalBlock(scope: !2333, file: !589, line: 641, column: 13)
!2333 = distinct !DILexicalBlock(scope: !2334, file: !589, line: 640, column: 40)
!2334 = distinct !DILexicalBlock(scope: !2335, file: !589, line: 640, column: 16)
!2335 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 637, column: 9)
!2336 = !DILocation(line: 540, column: 18, scope: !2294)
!2337 = !DILocation(line: 555, column: 1, scope: !2086)
!2338 = !DILocation(line: 556, column: 14, scope: !2339)
!2339 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 556, column: 9)
!2340 = !DILocation(line: 556, column: 9, scope: !2086)
!2341 = !DILocation(line: 557, column: 21, scope: !2342)
!2342 = distinct !DILexicalBlock(scope: !2339, file: !589, line: 556, column: 20)
!2343 = !DILocation(line: 558, column: 5, scope: !2342)
!2344 = !DILocation(line: 558, column: 21, scope: !2345)
!2345 = distinct !DILexicalBlock(scope: !2339, file: !589, line: 558, column: 16)
!2346 = !DILocation(line: 558, column: 16, scope: !2339)
!2347 = !DILocation(line: 559, column: 21, scope: !2348)
!2348 = distinct !DILexicalBlock(scope: !2345, file: !589, line: 558, column: 27)
!2349 = !DILocation(line: 560, column: 5, scope: !2348)
!2350 = !DILocation(line: 562, column: 58, scope: !2086)
!2351 = !DILocation(line: 562, column: 5, scope: !2086)
!2352 = !DILocation(line: 564, column: 7, scope: !2086)
!2353 = !DILocation(line: 567, column: 12, scope: !2086)
!2354 = !DILocation(line: 567, column: 5, scope: !2086)
!2355 = !DILocation(line: 569, column: 22, scope: !2356)
!2356 = distinct !DILexicalBlock(scope: !2319, file: !589, line: 569, column: 17)
!2357 = !DILocation(line: 574, column: 36, scope: !2358)
!2358 = distinct !DILexicalBlock(scope: !2319, file: !589, line: 574, column: 17)
!2359 = !DILocation(line: 574, column: 28, scope: !2358)
!2360 = !DILocation(line: 591, column: 23, scope: !2124)
!2361 = !DILocation(line: 570, column: 34, scope: !2362)
!2362 = distinct !DILexicalBlock(scope: !2363, file: !589, line: 570, column: 21)
!2363 = distinct !DILexicalBlock(scope: !2356, file: !589, line: 569, column: 28)
!2364 = !DILocation(line: 570, column: 25, scope: !2362)
!2365 = !DILocation(line: 570, column: 21, scope: !2362)
!2366 = !DILocation(line: 570, column: 55, scope: !2362)
!2367 = !DILocation(line: 570, column: 68, scope: !2362)
!2368 = !DILocation(line: 570, column: 64, scope: !2362)
!2369 = !DILocation(line: 582, column: 102, scope: !2325)
!2370 = !DILocation(line: 582, column: 92, scope: !2325)
!2371 = !DILocation(line: 582, column: 83, scope: !2325)
!2372 = !DILocation(line: 582, column: 79, scope: !2325)
!2373 = !DILocation(line: 582, column: 77, scope: !2325)
!2374 = !DILocation(line: 582, column: 17, scope: !2319)
!2375 = !DILocation(line: 583, column: 55, scope: !2323)
!2376 = !DILocation(line: 583, column: 21, scope: !2323)
!2377 = !DILocation(line: 583, column: 80, scope: !2323)
!2378 = !DILocation(line: 583, column: 21, scope: !2324)
!2379 = distinct !{!2379, !2354, !2380}
!2380 = !DILocation(line: 634, column: 5, scope: !2086)
!2381 = !DILocation(line: 574, column: 45, scope: !2358)
!2382 = !DILocation(line: 574, column: 64, scope: !2358)
!2383 = !DILocation(line: 574, column: 74, scope: !2358)
!2384 = !DILocation(line: 574, column: 81, scope: !2358)
!2385 = !DILocation(line: 574, column: 77, scope: !2358)
!2386 = !DILocation(line: 574, column: 111, scope: !2358)
!2387 = !DILocation(line: 574, column: 17, scope: !2319)
!2388 = !DILocation(line: 577, column: 17, scope: !2319)
!2389 = !DILocation(line: 578, column: 46, scope: !2316)
!2390 = !DILocation(line: 578, column: 40, scope: !2316)
!2391 = !DILocation(line: 0, scope: !1679, inlinedAt: !2315)
!2392 = !DILocation(line: 128, column: 23, scope: !1679, inlinedAt: !2315)
!2393 = !DILocation(line: 128, column: 13, scope: !1679, inlinedAt: !2315)
!2394 = !DILocation(line: 128, column: 35, scope: !1679, inlinedAt: !2315)
!2395 = !DILocation(line: 129, column: 11, scope: !1679, inlinedAt: !2315)
!2396 = !DILocation(line: 131, column: 15, scope: !1679, inlinedAt: !2315)
!2397 = !DILocation(line: 132, column: 15, scope: !1679, inlinedAt: !2315)
!2398 = !DILocation(line: 132, column: 52, scope: !1679, inlinedAt: !2315)
!2399 = !DILocation(line: 134, column: 30, scope: !1679, inlinedAt: !2315)
!2400 = !DILocation(line: 578, column: 21, scope: !2317)
!2401 = !DILocation(line: 0, scope: !2325)
!2402 = !DILocation(line: 582, column: 27, scope: !2325)
!2403 = !DILocation(line: 582, column: 56, scope: !2325)
!2404 = !DILocation(line: 582, column: 47, scope: !2325)
!2405 = !DILocation(line: 582, column: 43, scope: !2325)
!2406 = !DILocation(line: 583, column: 39, scope: !2323)
!2407 = !DILocation(line: 591, column: 17, scope: !2124)
!2408 = !DILocation(line: 591, column: 9, scope: !2124)
!2409 = !DILocation(line: 593, column: 28, scope: !2123)
!2410 = !DILocation(line: 593, column: 13, scope: !2123)
!2411 = !DILocation(line: 593, column: 13, scope: !2124)
!2412 = !DILocation(line: 594, column: 39, scope: !2122)
!2413 = !DILocation(line: 594, column: 56, scope: !2122)
!2414 = !DILocation(line: 594, column: 51, scope: !2122)
!2415 = !DILocation(line: 594, column: 60, scope: !2122)
!2416 = !DILocation(line: 594, column: 13, scope: !2122)
!2417 = !DILocation(line: 594, column: 65, scope: !2122)
!2418 = !DILocation(line: 595, column: 35, scope: !2122)
!2419 = !DILocation(line: 595, column: 13, scope: !2122)
!2420 = !DILocation(line: 595, column: 33, scope: !2122)
!2421 = !DILocation(line: 597, column: 28, scope: !2122)
!2422 = !DILocation(line: 598, column: 26, scope: !2122)
!2423 = !DILocation(line: 598, column: 13, scope: !2122)
!2424 = !DILocation(line: 598, column: 31, scope: !2122)
!2425 = !DILocation(line: 599, column: 44, scope: !2122)
!2426 = !DILocation(line: 599, column: 50, scope: !2122)
!2427 = !DILocation(line: 599, column: 57, scope: !2122)
!2428 = !DILocation(line: 599, column: 55, scope: !2122)
!2429 = !DILocation(line: 599, column: 29, scope: !2122)
!2430 = !DILocation(line: 0, scope: !2122)
!2431 = !DILocation(line: 601, column: 27, scope: !2127)
!2432 = !DILocation(line: 599, column: 28, scope: !2122)
!2433 = !DILocation(line: 601, column: 39, scope: !2127)
!2434 = !DILocation(line: 601, column: 17, scope: !2122)
!2435 = !DILocation(line: 606, column: 41, scope: !2436)
!2436 = distinct !DILexicalBlock(scope: !2437, file: !589, line: 606, column: 28)
!2437 = distinct !DILexicalBlock(scope: !2126, file: !589, line: 604, column: 21)
!2438 = !DILocation(line: 0, scope: !2126)
!2439 = !DILocation(line: 611, column: 26, scope: !2126)
!2440 = !DILocation(line: 611, column: 25, scope: !2126)
!2441 = !DILocation(line: 612, column: 13, scope: !2126)
!2442 = !DILocation(line: 618, column: 19, scope: !2124)
!2443 = !DILocation(line: 618, column: 9, scope: !2124)
!2444 = !DILocation(line: 620, column: 23, scope: !2445)
!2445 = distinct !DILexicalBlock(scope: !2124, file: !589, line: 620, column: 13)
!2446 = !DILocation(line: 620, column: 13, scope: !2445)
!2447 = !DILocation(line: 620, column: 13, scope: !2124)
!2448 = !DILocation(line: 624, column: 19, scope: !2449)
!2449 = distinct !DILexicalBlock(scope: !2124, file: !589, line: 624, column: 13)
!2450 = !DILocation(line: 624, column: 30, scope: !2449)
!2451 = !DILocation(line: 624, column: 27, scope: !2449)
!2452 = !DILocation(line: 625, column: 34, scope: !2453)
!2453 = distinct !DILexicalBlock(scope: !2449, file: !589, line: 624, column: 42)
!2454 = !DILocation(line: 625, column: 21, scope: !2453)
!2455 = !DILocation(line: 627, column: 23, scope: !2456)
!2456 = distinct !DILexicalBlock(scope: !2453, file: !589, line: 627, column: 17)
!2457 = !DILocation(line: 627, column: 17, scope: !2453)
!2458 = !DILocation(line: 628, column: 17, scope: !2459)
!2459 = distinct !DILexicalBlock(scope: !2456, file: !589, line: 627, column: 32)
!2460 = !DILocation(line: 629, column: 17, scope: !2459)
!2461 = !DILocation(line: 636, column: 1, scope: !2086)
!2462 = !DILocation(line: 637, column: 27, scope: !2335)
!2463 = !DILocation(line: 637, column: 19, scope: !2335)
!2464 = !DILocation(line: 640, column: 26, scope: !2334)
!2465 = !DILocation(line: 642, column: 31, scope: !2466)
!2466 = distinct !DILexicalBlock(scope: !2467, file: !589, line: 642, column: 17)
!2467 = distinct !DILexicalBlock(scope: !2332, file: !589, line: 641, column: 25)
!2468 = !DILocation(line: 654, column: 9, scope: !2469)
!2469 = distinct !DILexicalBlock(scope: !2086, file: !589, line: 654, column: 9)
!2470 = !DILocation(line: 654, column: 18, scope: !2469)
!2471 = !DILocation(line: 655, column: 28, scope: !2472)
!2472 = distinct !DILexicalBlock(scope: !2469, file: !589, line: 654, column: 30)
!2473 = !DILocation(line: 655, column: 23, scope: !2472)
!2474 = !DILocation(line: 656, column: 5, scope: !2472)
!2475 = !DILocation(line: 658, column: 5, scope: !2086)
!2476 = !DILocation(line: 660, column: 5, scope: !2086)
!2477 = !DILocation(line: 661, column: 1, scope: !2086)
