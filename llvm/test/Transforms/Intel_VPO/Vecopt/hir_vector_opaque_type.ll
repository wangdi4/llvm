; The test verifies that HIR vec codegen can handle memrefs with opaque types.

; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -hir-details -vplan-force-vf=2 -enable-vp-value-codegen-hir=0 < %s 2>&1 | FileCheck %s --check-prefixes=MIXED,CHECK
; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -hir-details -vplan-force-vf=2 -enable-vp-value-codegen-hir < %s 2>&1 | FileCheck %s --check-prefixes=VPVAL,CHECK

; HIR:
; BEGIN REGION { }
;   + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;   |   (%a)[i1] = &((%b)[0]);
;   + END LOOP
; END REGION

; Verify that memref with opaque type is vectorized.
; MIXED: <RVAL-REG> &((<2 x %struct.OType*>)(LINEAR %struct.OType* %b)[<2 x i64> 0])
; VPVAL: <RVAL-REG> LINEAR <2 x %struct.OType*> %b

; Check broadcast pattern is generated for the opaque type memref after HIR-CG.
; CHECK:       [[SPLATINSERT:%.*]] = insertelement <2 x %struct.OType*> undef, %struct.OType* %b, i32 0
; CHECK-NEXT:  [[SPLAT:%.*]] = shufflevector <2 x %struct.OType*> [[SPLATINSERT]], <2 x %struct.OType*> undef, <2 x i32> zeroinitializer

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.OType = type opaque

; Function Attrs: norecurse nounwind uwtable
define void @foo(%struct.OType** nocapture %a, %struct.OType* %b, i32 %n) local_unnamed_addr {
entry:
  %cmp4 = icmp sgt i32 %n, 0
  br i1 %cmp4, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds %struct.OType*, %struct.OType** %a, i64 %indvars.iv
  store %struct.OType* %b, %struct.OType** %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}



