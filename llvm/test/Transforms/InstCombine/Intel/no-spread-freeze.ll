; RUN: opt -enable-intel-advanced-opts -passes="instcombine" -S %s | FileCheck %s
; RUN: opt -passes="instcombine" -S %s | FileCheck %s

; InstCombine was "spreading" the freeze in storemerge.fr, to the other uses
; of storemerge92, especially the mul...200 instruction.
; This blocks IV analysis and makes loop analysis more difficult.
; It is OK to convert the add...-2 instruction, as it is already used in a
; freeze computation (%0,%1,%.fr)
; The mul instruction has no freeze in its def-use chain, and freeze should not
; be spread to its %storemerge92 operand.

; CHECK-NOT: mul{{.*}}.fr

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define hidden void @LBM_initializeSpecialCellsForLDC.DIR.OMP.PARALLEL.LOOP.2.split70(ptr %grid.addr.0.val) #0 {
DIR.OMP.PARALLEL.LOOP.3:
  br label %if.then.preheader

if.then.preheader:                                ; preds = %if.then.loopexit.split.split.us90, %DIR.OMP.PARALLEL.LOOP.3
  %storemerge92 = phi i32 [ 0, %DIR.OMP.PARALLEL.LOOP.3 ], [ %inc46, %if.then.loopexit.split.split.us90 ]
  %0 = add nsw i32 %storemerge92, -2
  %1 = icmp ult i32 %0, 196
  %mul34 = mul nuw nsw i32 %storemerge92, 200
  %add35 = add i32 %mul34, 0
  %storemerge.fr = freeze i32 %storemerge92
  %.fr = freeze i1 %1
  switch i32 %storemerge.fr, label %if.then.preheader.split [
    i32 0, label %if.then.preheader.split.us
    i32 199, label %if.then.preheader.split.us
  ]

if.then.preheader.split.us:                       ; preds = %if.then.preheader, %if.then.preheader
  unreachable

if.then.preheader.split:                          ; preds = %if.then.preheader
  br i1 %.fr, label %for.body4.lr.ph.us74, label %for.body4.lr.ph

for.body4.lr.ph.us74:                             ; preds = %if.then.preheader.split
  switch i32 undef, label %for.body4.us [
    i32 2, label %if.then.loopexit.split.split.us90
    i32 261, label %if.then.loopexit.split.split.us90
    i32 3, label %for.body4.lr.ph.split.split.split.us43.us
    i32 260, label %for.body4.lr.ph.split.split.split.us43.us
  ]

for.body4.us:                                     ; preds = %for.body4.lr.ph.us74
  switch i32 0, label %lor.lhs.false7.us [
    i32 -1, label %if.then.loopexit.split.split.us90
    i32 198, label %if.then.loopexit.split.split.us90
  ]

lor.lhs.false7.us:                                ; preds = %for.body4.us
  unreachable

if.then.loopexit.split.split.us90:                ; preds = %for.body4.us44.us, %for.body4.us44.us, %for.body4.us, %for.body4.us, %for.body4.lr.ph.us74, %for.body4.lr.ph.us74
  %inc46 = add nuw nsw i32 %storemerge92, 1
  br label %if.then.preheader

for.body4.lr.ph.split.split.split.us43.us:        ; preds = %for.body4.lr.ph.us74, %for.body4.lr.ph.us74
  br label %for.body4.us44.us

for.body4.us44.us:                                ; preds = %lor.lhs.false7.us47.us, %for.body4.lr.ph.split.split.split.us43.us
  %inc27.us45.us = phi i32 [ 1, %for.body4.lr.ph.split.split.split.us43.us ], [ undef, %lor.lhs.false7.us47.us ]
  switch i32 undef, label %lor.lhs.false7.us47.us [
    i32 -1, label %if.then.loopexit.split.split.us90
    i32 198, label %if.then.loopexit.split.split.us90
  ]

lor.lhs.false7.us47.us:                           ; preds = %for.body4.us44.us
  %add38.us.us = add i32 %add35, %inc27.us45.us
  %mul39.us.us = mul nuw nsw i32 %add38.us.us, 20
  %add40.us.us = add nuw nsw i32 %mul39.us.us, 19
  %idxprom41.us.us = zext i32 %add40.us.us to i64
  %arrayidx42.us.us = getelementptr inbounds double, ptr %grid.addr.0.val, i64 %idxprom41.us.us
  store i32 0, ptr %arrayidx42.us.us, align 4
  br label %for.body4.us44.us

for.body4.lr.ph:                                  ; preds = %if.then.preheader.split
  unreachable
}

attributes #0 = { "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }

!nvvm.annotations = !{}
