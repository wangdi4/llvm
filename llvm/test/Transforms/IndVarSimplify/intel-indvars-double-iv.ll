; RUN: opt -passes="loop(indvars)" -S < %s | FileCheck %s
; The double-type phi is being passed to SCEV for analysis, which is not
; a valid type. indvars can still remove it without using SCEV.
; CHECK-NOT: phi double

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @dftd3_debug_core_mp_pbcgdisp_() local_unnamed_addr #0 {
alloca_2:
  br i1 undef, label %bb571, label %bb341.preheader

bb341.preheader:                                  ; preds = %alloca_2
  br label %bb345

bb345:                                            ; preds = %bb350.loopexit, %bb341.preheader
  br label %bb349

bb349:                                            ; preds = %bb417.loopexit, %bb345
  br label %bb416

bb416:                                            ; preds = %bb437.loopexit, %bb349
  br label %bb436

bb436:                                            ; preds = %bb457.loopexit, %bb416
  br label %bb456

bb456:                                            ; preds = %bb477, %bb436
  %"dftd3_debug_core_mp_pbcgdisp_$EABC.5" = phi double [ %"dftd3_debug_core_mp_pbcgdisp_$EABC.10.lcssa", %bb477 ], [ 0.0, %bb436 ]
  %"dftd3_debug_core_mp_pbcgdisp_$JTAUZ.0" = phi i32 [ %add1604, %bb477 ], [ 0, %bb436 ]
  %rel1272.inv = icmp sgt i32 %"dftd3_debug_core_mp_pbcgdisp_$JTAUZ.0", 0
  %slct1274.v = select i1 %rel1272.inv, i32 %"dftd3_debug_core_mp_pbcgdisp_$JTAUZ.0", i32 0
  %slct1274 = sub nsw i32 %slct1274.v, 1 
  br label %bb476

bb476:                                            ; preds = %bb485, %bb456
  br label %bb484

bb484:                                            ; preds = %bb493, %bb476
  br label %bb492

bb492:                                            ; preds = %bb492, %bb484
  %"dftd3_debug_core_mp_pbcgdisp_$KTAUZ.0" = phi i32 [ %add1574, %bb492 ], [ %slct1274, %bb484 ]
  %int_sext1329 = sext i32 %"dftd3_debug_core_mp_pbcgdisp_$KTAUZ.0" to i64
  %0 = getelementptr inbounds double, double* null, i64 %int_sext1329
  %add1574 = add nsw i32 %"dftd3_debug_core_mp_pbcgdisp_$KTAUZ.0", 1
  %cmp1 = icmp slt i32 %add1574, 100
  br i1 false, label %bb492, label %bb493

bb493:                                            ; preds = %bb492
  br i1 false, label %bb484, label %bb485

bb485:                                            ; preds = %bb493
  br i1 false, label %bb476, label %bb477

bb477:                                            ; preds = %bb485
  %"dftd3_debug_core_mp_pbcgdisp_$EABC.10.lcssa" = phi double [ 0.0, %bb485 ]
  %add1604 = add nsw i32 %"dftd3_debug_core_mp_pbcgdisp_$JTAUZ.0", 1
  %cmp2 = icmp slt i32 %add1574, 100
  br i1 %cmp2, label %bb456, label %bb457.loopexit

bb457.loopexit:                                   ; preds = %bb477
  br i1 undef, label %bb436, label %bb437.loopexit

bb437.loopexit:                                   ; preds = %bb457.loopexit
  br i1 undef, label %bb416, label %bb417.loopexit

bb417.loopexit:                                   ; preds = %bb437.loopexit
  br i1 undef, label %bb349, label %bb350.loopexit

bb350.loopexit:                                   ; preds = %bb417.loopexit
  br label %bb345

bb571:                                            ; preds = %alloca_2
  ret void
}

attributes #0 = { "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
