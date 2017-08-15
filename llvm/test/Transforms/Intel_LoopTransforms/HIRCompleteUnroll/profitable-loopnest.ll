; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that this 2-level loopnest is completely unrolled.

; HIR -
; + DO i1 = 0, 2, 1   <DO_LOOP>
; |   %r.1192.us.i.us = %r.0199.us.i.us;
; |   %b.1190.us.i.us = %b.0197.us.i.us;
; |   %g.1189.us.i.us = %g.0196.us.i.us;
; |   + DO i2 = 0, 2, 1   <DO_LOOP>
; |   |   %0 = (%base)[3 * %conv.i * i1 + 3 * i2 + 3 * ((%add.i.us * %conv.i) + %add36.i.us)];
; |   |   %conv41.us.i.us = uitofp.i8.float(%0);
; |   |   %1 = (@_ZZ13sharpen_imagePK5ImagePS_E6kernel)[0][3 * i1 + i2];
; |   |   %mul45.us.i.us = %conv41.us.i.us  *  %1;
; |   |   %r.1192.us.i.us = %mul45.us.i.us  +  %r.1192.us.i.us;
; |   |   %2 = (%base)[3 * %conv.i * i1 + 3 * i2 + 3 * ((%add.i.us * %conv.i) + %add36.i.us) + 1];
; |   |   %conv48.us.i.us = uitofp.i8.float(%2);
; |   |   %mul52.us.i.us = %conv48.us.i.us  *  %1;
; |   |   %g.1189.us.i.us = %mul52.us.i.us  +  %g.1189.us.i.us;
; |   |   %3 = (%base)[3 * %conv.i * i1 + 3 * i2 + 3 * ((%add.i.us * %conv.i) + %add36.i.us) + 2];
; |   |   %conv55.us.i.us = uitofp.i8.float(%3);
; |   |   %mul59.us.i.us = %conv55.us.i.us  *  %1;
; |   |   %b.1190.us.i.us = %mul59.us.i.us  +  %b.1190.us.i.us;
; |   + END LOOP
; |   %r.0199.us.i.us = %r.1192.us.i.us;
; |   %b.0197.us.i.us = %b.1190.us.i.us;
; |   %g.0196.us.i.us = %g.1189.us.i.us;
; + END LOOP

; CHECK: Dump Before HIR PostVec Complete Unroll
; CHECK: DO i1
; CHECK: DO i2


; CHECK: Dump After HIR PostVec Complete Unroll
; CHECK-NOT: DO i1


@_ZZ13sharpen_imagePK5ImagePS_E6kernel = internal constant [9 x float] [float 0.000000e+00, float -1.000000e+00, float 0.000000e+00, float -1.000000e+00, float 5.000000e+00, float -1.000000e+00, float 0.000000e+00, float -1.000000e+00, float 0.000000e+00], align 16

define void @_Z13sharpen_imagePK5ImagePS_(i64 %add.i.us, i64 %add36.i.us, i64 %conv.i, i8* %base) #0 {
entry:
  br label %for.cond30.preheader.us.i.us

for.cond30.preheader.us.i.us:                     ; preds = %for.cond30.for.cond.cleanup32_crit_edge.us.i.us, %entry
  %r.0199.us.i.us = phi float [ 0.000000e+00, %entry ], [ %add46.us.i.us.lcssa, %for.cond30.for.cond.cleanup32_crit_edge.us.i.us ]
  %u.0198.us.i.us = phi i64 [ 0, %entry ], [ %inc62.us.i.us, %for.cond30.for.cond.cleanup32_crit_edge.us.i.us ]
  %b.0197.us.i.us = phi float [ 0.000000e+00, %entry ], [ %add60.us.i.us.lcssa, %for.cond30.for.cond.cleanup32_crit_edge.us.i.us ]
  %g.0196.us.i.us = phi float [ 0.000000e+00, %entry ], [ %add53.us.i.us.lcssa, %for.cond30.for.cond.cleanup32_crit_edge.us.i.us ]
  %sub34.us.i.us = add nuw i64 %u.0198.us.i.us, %add.i.us
  %mul35.us.i.us = mul i64 %sub34.us.i.us, %conv.i
  %sub37.us.i.us = add i64 %add36.i.us, %mul35.us.i.us
  %mul42.us.i.us = mul nuw nsw i64 %u.0198.us.i.us, 3
  br label %for.body33.us.i.us

for.body33.us.i.us:                               ; preds = %for.body33.us.i.us, %for.cond30.preheader.us.i.us
  %r.1192.us.i.us = phi float [ %r.0199.us.i.us, %for.cond30.preheader.us.i.us ], [ %add46.us.i.us, %for.body33.us.i.us ]
  %v.0191.us.i.us = phi i64 [ 0, %for.cond30.preheader.us.i.us ], [ %inc.us.i.us, %for.body33.us.i.us ]
  %b.1190.us.i.us = phi float [ %b.0197.us.i.us, %for.cond30.preheader.us.i.us ], [ %add60.us.i.us, %for.body33.us.i.us ]
  %g.1189.us.i.us = phi float [ %g.0196.us.i.us, %for.cond30.preheader.us.i.us ], [ %add53.us.i.us, %for.body33.us.i.us ]
  %add38.us.i.us = add i64 %sub37.us.i.us, %v.0191.us.i.us
  %mul39.us.i.us = mul i64 %add38.us.i.us, 3
  %arrayidx.us.i.us = getelementptr inbounds i8, i8* %base, i64 %mul39.us.i.us
  %0 = load i8, i8* %arrayidx.us.i.us, align 1
  %conv41.us.i.us = uitofp i8 %0 to float
  %add43.us.i.us = add nuw nsw i64 %v.0191.us.i.us, %mul42.us.i.us
  %arrayidx44.us.i.us = getelementptr inbounds [9 x float], [9 x float]* @_ZZ13sharpen_imagePK5ImagePS_E6kernel, i64 0, i64 %add43.us.i.us
  %1 = load float, float* %arrayidx44.us.i.us, align 4
  %mul45.us.i.us = fmul fast float %conv41.us.i.us, %1
  %add46.us.i.us = fadd fast float %mul45.us.i.us, %r.1192.us.i.us
  %arrayidx47.us.i.us = getelementptr inbounds i8, i8* %arrayidx.us.i.us, i64 1
  %2 = load i8, i8* %arrayidx47.us.i.us, align 1
  %conv48.us.i.us = uitofp i8 %2 to float
  %mul52.us.i.us = fmul fast float %conv48.us.i.us, %1
  %add53.us.i.us = fadd fast float %mul52.us.i.us, %g.1189.us.i.us
  %arrayidx54.us.i.us = getelementptr inbounds i8, i8* %arrayidx.us.i.us, i64 2
  %3 = load i8, i8* %arrayidx54.us.i.us, align 1
  %conv55.us.i.us = uitofp i8 %3 to float
  %mul59.us.i.us = fmul fast float %conv55.us.i.us, %1
  %add60.us.i.us = fadd fast float %mul59.us.i.us, %b.1190.us.i.us
  %inc.us.i.us = add nuw nsw i64 %v.0191.us.i.us, 1
  %exitcond.i.us = icmp eq i64 %inc.us.i.us, 3
  br i1 %exitcond.i.us, label %for.cond30.for.cond.cleanup32_crit_edge.us.i.us, label %for.body33.us.i.us

for.cond30.for.cond.cleanup32_crit_edge.us.i.us:  ; preds = %for.body33.us.i.us
  %add60.us.i.us.lcssa = phi float [ %add60.us.i.us, %for.body33.us.i.us ]
  %add53.us.i.us.lcssa = phi float [ %add53.us.i.us, %for.body33.us.i.us ]
  %add46.us.i.us.lcssa = phi float [ %add46.us.i.us, %for.body33.us.i.us ]
  %inc62.us.i.us = add nuw nsw i64 %u.0198.us.i.us, 1
  %exitcond207.i.us = icmp eq i64 %inc62.us.i.us, 3
  br i1 %exitcond207.i.us, label %for.cond.cleanup28.i.us, label %for.cond30.preheader.us.i.us

for.cond.cleanup28.i.us:
  ret void
}

