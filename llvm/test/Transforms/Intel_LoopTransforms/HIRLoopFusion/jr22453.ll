; RUN: opt -hir-ssa-deconstruction -disable-output -hir-loop-fusion -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that pre-fusion HLNode sorting doesn't fails with assertion.

; BEGIN REGION { }
;       + DO i1 = 0, 10, 1   <DO_LOOP>
;       |   + DO i2 = 0, sext.i32.i64(undef) + -1 * undef + -1, 1   <DO_LOOP>
;       |   |   %"NCLDS[]13600_fetch" = (undef)[0];
;       |   + END LOOP
;       |
;       |   (undef)[0] = undef;
;       |
;       |   + DO i2 = 0, sext.i32.i64(undef) + -1 * undef + -1, 1   <DO_LOOP>
;       |   |   %"NCLDS[]14228_fetch" = (undef)[0];
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, sext.i32.i64(undef) + -1 * undef + -1, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, undef + -3, 1   <DO_LOOP>
;       |   |   |   (%"module_ra_gfdleta_mp_swr93_$DFNTRN475")[i3 + 2][i2 + undef] = undef;
;       |   |   + END LOOP
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, sext.i32.i64(undef) + -1 * undef + -1, 1   <DO_LOOP>
;       |   |   %"NCLDS[]14857_fetch" = (undef)[0];
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, sext.i32.i64(undef) + -1 * undef + -1, 1   <DO_LOOP>
;       |   |   %"NCLDS[]15890_fetch" = (undef)[0];
;       |   |   (%"module_ra_gfdleta_mp_swr93_$DFNTRN475")[1][i2 + undef] = undef;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 10, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(undef) + -1 * undef + -1, 1   <DO_LOOP> 
;                  |   |   %"NCLDS[]13600_fetch" = (undef)[0];
;                  |   |   %"NCLDS[]14857_fetch" = (undef)[0];
;                  |   |   %"NCLDS[]15890_fetch" = (undef)[0];
;                  |   |   (%"module_ra_gfdleta_mp_swr93_$DFNTRN475")[1][i2 + undef] = undef;
;                  |   |   %"NCLDS[]14228_fetch" = (undef)[0];
; CHECK:           |   + END LOOP
;                  |
;                  |   (undef)[0] = undef;
;                  |
; CHECK:           |   + DO i2 = 0, sext.i32.i64(undef) + -1 * undef + -1, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, undef + -3, 1   <DO_LOOP>
;                  |   |   |   (%"module_ra_gfdleta_mp_swr93_$DFNTRN475")[i3 + 2][i2 + undef] = undef;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @module_ra_gfdleta_mp_swr93_() local_unnamed_addr #0 {
alloca_16:
  %"module_ra_gfdleta_mp_swr93_$DFNTRN475" = alloca float, i64 undef, align 1
  %wide.trip.count22225 = sext i32 undef to i64
  br label %bb18199

bb18199:                                          ; preds = %bb19017.preheader, %alloca_16
  %indvars.iv22350 = phi i64 [ %indvars.iv.next22351, %bb19017.preheader ], [ 2, %alloca_16 ]
  br label %bb18203

bb18203:                                          ; preds = %bb18203, %bb18199
  %indvars.iv22223 = phi i64 [ %indvars.iv.next22224, %bb18203 ], [ undef, %bb18199 ]
  %"NCLDS[]13600_fetch" = load i32, i32* undef, align 1
  %indvars.iv.next22224 = add nsw i64 %indvars.iv22223, 1
  %exitcond22226 = icmp eq i64 %indvars.iv.next22224, %wide.trip.count22225
  br i1 %exitcond22226, label %bb18204, label %bb18203

bb18204:                                          ; preds = %bb18203
  store float undef, float* undef, align 1
  br label %bb18350

bb18350:                                          ; preds = %bb18350, %bb18204
  %indvars.iv22251 = phi i64 [ undef, %bb18204 ], [ %indvars.iv.next22252, %bb18350 ]
  %"NCLDS[]14228_fetch" = load i32, i32* undef, align 1
  %indvars.iv.next22252 = add nsw i64 %indvars.iv22251, 1
  %exitcond22254 = icmp eq i64 %indvars.iv.next22252, %wide.trip.count22225
  br i1 %exitcond22254, label %bb18375.preheader, label %bb18350

bb18375.preheader:                                ; preds = %bb18350
  br label %bb18375

bb18375:                                          ; preds = %bb18375, %bb18375.preheader
  %indvars.iv22255 = phi i64 [ %indvars.iv.next22256, %bb18375 ], [ undef, %bb18375.preheader ]
  %indvars.iv.next22256 = add nsw i64 %indvars.iv22255, 1
  %exitcond22258 = icmp eq i64 %indvars.iv.next22256, %wide.trip.count22225
  br i1 %exitcond22258, label %bb18431.preheader, label %bb18375

bb18431.preheader:                                ; preds = %bb18375
  br label %bb18431

bb18431:                                          ; preds = %bb18431, %bb18431.preheader
  %indvars.iv22263 = phi i64 [ %indvars.iv.next22264, %bb18431 ], [ undef, %bb18431.preheader ]
  %indvars.iv.next22264 = add nsw i64 %indvars.iv22263, 1
  %exitcond22266 = icmp eq i64 %indvars.iv.next22264, %wide.trip.count22225
  br i1 %exitcond22266, label %bb18485.preheader, label %bb18431

bb18485.preheader:                                ; preds = %bb18431
  br label %bb18485

bb18485:                                          ; preds = %bb15677, %bb18485.preheader
  %indvars.iv22271 = phi i64 [ %indvars.iv.next22272, %bb15677 ], [ undef, %bb18485.preheader ]
  br label %bb18496

bb18496:                                          ; preds = %bb18496, %bb18485
  %indvars.iv22267 = phi i64 [ 2, %bb18485 ], [ %indvars.iv.next22268, %bb18496 ]
  %"module_ra_gfdleta_mp_swr93_$DFNTRN475[]14772" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 undef, i64 undef, float* elementtype(float) nonnull %"module_ra_gfdleta_mp_swr93_$DFNTRN475", i64 %indvars.iv22267)
  %"module_ra_gfdleta_mp_swr93_$DFNTRN475[]14772[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 undef, i64 4, float* elementtype(float) nonnull %"module_ra_gfdleta_mp_swr93_$DFNTRN475[]14772", i64 %indvars.iv22271)
  store float undef, float* %"module_ra_gfdleta_mp_swr93_$DFNTRN475[]14772[]", align 1
  %indvars.iv.next22268 = add nuw nsw i64 %indvars.iv22267, 1
  %exitcond22270 = icmp eq i64 %indvars.iv.next22268, undef
  br i1 %exitcond22270, label %bb15677, label %bb18496

bb15677:                                          ; preds = %bb18496
  %indvars.iv.next22272 = add nsw i64 %indvars.iv22271, 1
  %exitcond22274 = icmp eq i64 %indvars.iv.next22272, %wide.trip.count22225
  br i1 %exitcond22274, label %bb18518.preheader, label %bb18485

bb18518.preheader:                                ; preds = %bb15677
  br label %bb18518

bb18518:                                          ; preds = %bb18518, %bb18518.preheader
  %indvars.iv22279 = phi i64 [ %indvars.iv.next22280, %bb18518 ], [ undef, %bb18518.preheader ]
  %"NCLDS[]14857_fetch" = load i32, i32* undef, align 1
  %indvars.iv.next22280 = add nsw i64 %indvars.iv22279, 1
  %exitcond22282 = icmp eq i64 %indvars.iv.next22280, %wide.trip.count22225
  br i1 %exitcond22282, label %bb18563.preheader, label %bb18518

bb18563.preheader:                                ; preds = %bb18518
  br label %bb18588

bb18588:                                          ; preds = %bb18588, %bb18563.preheader
  %indvars.iv22292 = phi i64 [ %indvars.iv.next22293, %bb18588 ], [ undef, %bb18563.preheader ]
  %indvars.iv.next22293 = add nsw i64 %indvars.iv22292, 1
  %exitcond22295 = icmp eq i64 %indvars.iv.next22293, %wide.trip.count22225
  br i1 %exitcond22295, label %bb18649.preheader, label %bb18588

bb18649.preheader:                                ; preds = %bb18588
  br label %bb18750

bb18750:                                          ; preds = %bb18750, %bb18649.preheader
  %indvars.iv22312 = phi i64 [ %indvars.iv.next22313, %bb18750 ], [ undef, %bb18649.preheader ]
  %"NCLDS[]15890_fetch" = load i32, i32* undef, align 1
  %"module_ra_gfdleta_mp_swr93_$DFNTRN475[]16000" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 undef, i64 undef, float* elementtype(float) nonnull %"module_ra_gfdleta_mp_swr93_$DFNTRN475", i64 1)
  %"module_ra_gfdleta_mp_swr93_$DFNTRN475[]16000[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 undef, i64 4, float* elementtype(float) nonnull %"module_ra_gfdleta_mp_swr93_$DFNTRN475[]16000", i64 %indvars.iv22312)
  store float undef, float* %"module_ra_gfdleta_mp_swr93_$DFNTRN475[]16000[]", align 1
  %indvars.iv.next22313 = add nsw i64 %indvars.iv22312, 1
  %exitcond22315 = icmp eq i64 %indvars.iv.next22313, %wide.trip.count22225
  br i1 %exitcond22315, label %bb18791.preheader, label %bb18750

bb18791.preheader:                                ; preds = %bb18750
  br label %bb18968

bb18968:                                          ; preds = %bb18968, %bb18791.preheader
  %indvars.iv22342 = phi i64 [ undef, %bb18791.preheader ], [ %indvars.iv.next22343, %bb18968 ]
  %indvars.iv.next22343 = add nsw i64 %indvars.iv22342, 1
  %exitcond22345 = icmp eq i64 %indvars.iv.next22343, %wide.trip.count22225
  br i1 %exitcond22345, label %bb19017.preheader, label %bb18968

bb19017.preheader:                                ; preds = %bb18968
  %indvars.iv.next22351 = add nuw nsw i64 %indvars.iv22350, 1
  %exitcond22352 = icmp eq i64 %indvars.iv.next22351, 13
  br i1 %exitcond22352, label %bb18202, label %bb18199

bb18202:                                          ; preds = %bb19017.preheader
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }

