; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -analyze -enable-new-pm=0 -hir-locality-analysis -hir-spatial-locality | FileCheck %s

; This test was compfailing during wrap analysis of (%A)[zext.i3.i64(i1)].

; Incoming HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; |   (%A)[i1] = i1;
; + END LOOP

; TODO: Make locality info of scalar/vector loops more precise in the presence of wraparound.
; The loop access 400 bytes of memory, thus the cache lines accessed is ceiling(400/64) = 7

; CHECK: Locality Info for Loop level: 1     NumCacheLines: 7        SpatialCacheLines: 7     TempInvCacheLines: 0     AvgLvalStride: 16         AvgStride: 16

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %A) {
for.body.preheader:
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %and = and i32 %i.06, 7
  %0 = zext i32 %and to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %0
  store i32 %i.06, i32* %arrayidx, align 4
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, 100
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

