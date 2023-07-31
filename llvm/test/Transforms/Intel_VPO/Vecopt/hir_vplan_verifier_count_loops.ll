; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -debug -S < %s
; TODO | FileCheck %s
; REQUIRES: asserts

; Check that VPlanVerifier is not crashing when counting the number of loops
; in the underlying HIR and compare them against the number of VPLoops in VPlan.

; TODO: This loop is currently not vectorized because it has nested blobs.
; Please, enable this check when the support for nested blobs is committed.
; TODO-CHECK: vector.body:

define i32 @quant_4x4(ptr noalias nocapture %dct, ptr nocapture readonly %mf, ptr nocapture readonly %bias) {
entry:
  br label %for.body

for.cond.cleanup:
  %or.lcssa = phi i32 [ %or, %if.end ]
  %tobool = icmp ne i32 %or.lcssa, 0
  %lnot.ext = zext i1 %tobool to i32
  ret i32 %lnot.ext

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %if.end ]
  %nz.039 = phi i32 [ 0, %entry ], [ %or, %if.end ]
  %idx = getelementptr inbounds i16, ptr %dct, i64 %iv
  %0 = load i16, ptr %idx
  %idx2 = getelementptr inbounds i16, ptr %mf, i64 %iv
  %1 = load i16, ptr %idx2
  %idx4 = getelementptr inbounds i16, ptr %bias, i64 %iv
  %2 = load i16, ptr %idx4
  %conv = sext i16 %0 to i32
  %cmp5 = icmp sgt i16 %0, 0
  %conv7 = zext i16 %2 to i32
  br i1 %cmp5, label %if.then, label %if.else

if.then:
  %add = add nsw i32 %conv7, %conv
  %conv9 = zext i16 %1 to i32
  %mul = mul nsw i32 %add, %conv9
  %3 = lshr i32 %mul, 16
  %conv10 = trunc i32 %3 to i16
  br label %if.end

if.else:
  %sub = sub nsw i32 %conv7, %conv
  %conv13 = zext i16 %1 to i32
  %mul14 = mul nsw i32 %sub, %conv13
  %4 = lshr i32 %mul14, 16
  %5 = trunc i32 %4 to i16
  %conv17 = sub i16 0, %5
  br label %if.end

if.end:
  %coef.0 = phi i16 [ %conv10, %if.then ], [ %conv17, %if.else ]
  store i16 %coef.0, ptr %idx
  %conv20 = sext i16 %coef.0 to i32
  %or = or i32 %nz.039, %conv20
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 16
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}
