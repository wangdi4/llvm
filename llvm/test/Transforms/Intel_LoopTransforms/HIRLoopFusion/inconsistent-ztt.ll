; RUN: opt -hir-details -hir-ssa-deconstruction -disable-output -hir-loop-fusion -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s
; RUN: opt -hir-details -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that loops i2 will not be fused because their ztts are inconsistent.

; BEGIN REGION { }
;       + DO i1 = 0, 0, 1   <DO_LOOP>
;       |   + DO i2 = 0, sext.i32.i64(%slct1192) + -1 * sext.i32.i64(%KTS_fetch), 1   <DO_LOOP>
;       |   |   + DO i3 = 0, sext.i32.i64(%slct71) + -1 * sext.i32.i64(%ITS_fetch), 1   <DO_LOOP>
;       |   |   |   %mul1825 = (undef)[0]  *  undef;
;       |   |   + END LOOP
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, sext.i32.i64(%slct1192) + -1 * sext.i32.i64(%KTS_fetch), 1   <DO_LOOP>
;       |   |   + DO i3 = 0, sext.i32.i64(%slct71) + -1 * sext.i32.i64(%ITS_fetch), 1   <DO_LOOP>
;       |   |   |   %mul2054 = (undef)[0]  *  undef;
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK-NOT: modified
; CHECK: i1

; CHECK: Ztt: No
; CHECK: i2
; CHECK: i3

; CHECK: Ztt: if (%brmerge == 0)
; CHECK: i2
; CHECK: i3

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @module_em_mp_rk_update_scalar_(i32* %ITS, i32* %KTS) local_unnamed_addr #0 {
alloca_5:
  %ITS_fetch = load i32, i32* %ITS, align 1
  %KTS_fetch = load i32, i32* %KTS, align 1
  %int_sext691 = sext i32 %ITS_fetch to i64
  %slct71 = select i1 undef, i32 undef, i32 undef
  %slct1192 = select i1 undef, i32 undef, i32 undef
  %rel1194 = icmp slt i32 %slct1192, %KTS_fetch
  %int_sext1222 = sext i32 %KTS_fetch to i64
  %brmerge = or i1 %rel1194, undef
  %0 = add nsw i32 %slct71, 1
  %1 = add nsw i32 %slct1192, 1
  %wide.trip.count3048 = sext i32 %1 to i64
  %wide.trip.count3044 = sext i32 %0 to i64
  %brmerge2988 = or i1 undef, %brmerge
  br label %bb692

bb692:                                            ; preds = %bb737_endif, %alloca_5
  br label %bb702

bb702:                                            ; preds = %bb707, %bb692
  %indvars.iv3084 = phi i64 [ %int_sext1222, %bb692 ], [ %indvars.iv.next3085, %bb707 ]
  br label %bb706

bb706:                                            ; preds = %bb706, %bb702
  %indvars.iv3080 = phi i64 [ %int_sext691, %bb702 ], [ %indvars.iv.next3081, %bb706 ]
  %"module_em_mp_rk_update_scalar_$TENDENCY66[]1819[][]_fetch" = load float, float* undef, align 1
  %mul1825 = fmul fast float %"module_em_mp_rk_update_scalar_$TENDENCY66[]1819[][]_fetch", undef
  %indvars.iv.next3081 = add nsw i64 %indvars.iv3080, 1
  %exitcond3083 = icmp eq i64 %indvars.iv.next3081, %wide.trip.count3044
  br i1 %exitcond3083, label %bb707, label %bb706

bb707:                                            ; preds = %bb706
  %indvars.iv.next3085 = add nsw i64 %indvars.iv3084, 1
  %exitcond3087 = icmp eq i64 %indvars.iv.next3085, %wide.trip.count3048
  br i1 %exitcond3087, label %bb703, label %bb702

bb703:                                            ; preds = %bb707
  br i1 %brmerge2988, label %bb737_endif, label %bb716.preheader

bb716.preheader:                                  ; preds = %bb703
  br label %bb716

bb716:                                            ; preds = %bb716.preheader, %bb721
  %indvars.iv3092 = phi i64 [ %indvars.iv.next3093, %bb721 ], [ %int_sext1222, %bb716.preheader ]
  br label %bb720

bb720:                                            ; preds = %bb720, %bb716
  %indvars.iv3088 = phi i64 [ %int_sext691, %bb716 ], [ %indvars.iv.next3089, %bb720 ]
  %"MSFTY[]2077[]_fetch" = load float, float* undef, align 1
  %mul2054 = fmul fast float %"MSFTY[]2077[]_fetch", undef
  %indvars.iv.next3089 = add nsw i64 %indvars.iv3088, 1
  %exitcond3091 = icmp eq i64 %indvars.iv.next3089, %wide.trip.count3044
  br i1 %exitcond3091, label %bb721, label %bb720

bb721:                                            ; preds = %bb720
  %indvars.iv.next3093 = add nsw i64 %indvars.iv3092, 1
  %exitcond3095 = icmp eq i64 %indvars.iv.next3093, %wide.trip.count3048
  br i1 %exitcond3095, label %bb737_endif.loopexit, label %bb716

bb737_endif.loopexit:                             ; preds = %bb721
  br label %bb737_endif

bb737_endif:                                      ; preds = %bb737_endif.loopexit, %bb703
  %exitcond3099 = icmp eq i64 0, 0
  br i1 %exitcond3099, label %bb693.loopexit, label %bb692

bb693.loopexit:                                   ; preds = %bb737_endif
  ret void
}

attributes #0 = { "intel-lang"="fortran" }

!omp_offload.info = !{}
