; HIR-
; + DO i1 = 0, 47, 1   <DO_LOOP>
; |   %0 = (@A)[0][i1 + 1];
; |   %1 = (@B)[0][i1];
; |   %add3 = %0  +  %1;
; |   (@A)[0][i1] = %add3;
; + END LOOP

; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-locality-analysis -hir-spatial-locality | FileCheck %s --check-prefix=SCALAR
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-locality-analysis>" -hir-spatial-locality -disable-output 2>&1 | FileCheck %s --check-prefix=SCALAR

; Verify the number of cache lines accessed is 7.
; (@A)[0][i1] accesses 48 * 4 = 192 bytes. Extra 4 bytes accessed due to
; (@A)[0][i1 + 1] for a total of 196 bytes. Number of cache lines accessed =
; ceil(196/64) = 4.
; Similarly, (@B)[0][i1] accesses 3 cache lines.

; SCALAR: Locality Info for Loop level: 1     NumCacheLines: 7        SpatialCacheLines: 7     TempInvCacheLines: 0     AvgLvalStride: 4         AvgStride: 4


; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=8 -analyze -enable-new-pm=0 -hir-locality-analysis -hir-spatial-locality | FileCheck %s --check-prefix=VECTOR

; Verify that the number of cache lines accessed by vectorized loop is the same as the scalar loop but the average stride gets multiplied by vector factor of 8.

; VECTOR: Locality Info for Loop level: 1     NumCacheLines: 7        SpatialCacheLines: 7     TempInvCacheLines: 0     AvgLvalStride: 32        AvgStride: 32


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %indvars.iv.next
  %0 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [100 x float], [100 x float]* @B, i64 0, i64 %indvars.iv
  %1 = load float, float* %arrayidx2, align 4
  %add3 = fadd float %0, %1
  %arrayidx5 = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %indvars.iv
  store float %add3, float* %arrayidx5, align 4
  %exitcond = icmp eq i64 %indvars.iv.next, 48
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

