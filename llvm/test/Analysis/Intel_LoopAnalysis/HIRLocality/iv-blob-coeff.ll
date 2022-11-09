; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-locality-analysis>" -hir-spatial-locality -disable-output 2>&1 | FileCheck %s

; HIR-
; + DO i1 = 0, smax(0, %m), 1   <DO_LOOP>  <MAX_TC_EST = 4>  <LEGAL_MAX_TC = 4> <max_trip_count = 4>
; |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
; |   |   (%ptr)[(2 + %n) * i1 + i2 + 1] = 10;
; |   + END LOOP
; + END LOOP


; Verify that we compute larger number of cache lines for i1 loop than the i2
; loop. The analysis assumes a trip count of 100 for i2 loop in the absence of
; any other information. Based on that assumption we also cache the value of
; %n blob as 100. The number of cache lines for i1 loop are then calculated as
; follows-
; i1 blob coeff = (2 + %n) = 102
; Number of bytes accessed by store in i1 loop- 
; (IVBlobCoeff * TripCount * RefSize) = (102 * 4 * 2) => 816 bytes.
; Number of cachelines accessed = (816 / 64) => 13 cache lines.

; CHECK: Locality Info for Loop level: 1     NumCacheLines: 13       SpatialCacheLines: 13    TempInvCacheLines: 0     AvgLvalStride: 204       AvgStride: 204
; CHECK: Locality Info for Loop level: 2     NumCacheLines: 4        SpatialCacheLines: 4     TempInvCacheLines: 0     AvgLvalStride: 2         AvgStride: 2

define void @foo(i16* noalias %t4, i16* noalias %t6, i16* noalias %ptr, i64 %init, i64 %n, i64 %m) {
entry:
  br label %outer.loop

outer.loop:                                               ; preds = %latch, %t82
  %outer.iv = phi i64 [ %outer.iv.inc, %latch ], [ 0, %entry ]
  %t87 = shl nuw nsw i64 %outer.iv, 1
  %t88 = or i64 %t87, 1
  %t89 = mul i64 %outer.iv, %n
  %t90 = add nsw i64 %t88, %t89
  br label %inner.loop

inner.loop:                                               ; preds = %inner.loop, %outer.loop
  %inner.iv = phi i64 [ 0, %outer.loop ], [ %inner.iv.inc, %inner.loop ]
  %t95 = phi i64 [ %t90, %outer.loop ], [ %t107, %inner.loop ]
  %t106 = getelementptr inbounds i16, i16* %ptr, i64 %t95
  store i16 10, i16* %t106, align 2
  %t107 = add nsw i64 %t95, 1
  %inner.iv.inc = add nuw nsw i64 %inner.iv, 1
  %t109 = icmp eq i64 %inner.iv.inc, %n
  br i1 %t109, label %latch, label %inner.loop

latch:                                              ; preds = %inner.loop
  %outer.iv.inc = add nuw nsw i64 %outer.iv, 1
  %t114 = icmp sgt i64 %outer.iv.inc, %m
  br i1 %t114, label %exit, label %outer.loop, !llvm.loop !0

exit:
  ret void
}

!0 = !{!0, !1}
!1 = !{!"llvm.loop.intel.loopcount_maximum", i32 4}
