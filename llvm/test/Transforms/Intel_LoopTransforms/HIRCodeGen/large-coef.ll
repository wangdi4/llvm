
; RUN: opt -S -hir-ssa-deconstruction -hir-cg -force-hir-cg %s | FileCheck %s
; Crash in CG on following region 
;          BEGIN REGION { }
;<15>            + DO i1 = 0, %0 + smax(-2, (-1 + (-1 * %0))) + 1, 1   <DO_LOOP>
;<2>             |   %ret.011.out = %ret.011;
;<6>             |   %2 = (%1)[-1 * i1 + %0 + -1];
;<8>             |   %or = %2  ||  4294967296 * %ret.011.out;
;<9>             |   %ret.011 = %or  %  %w;
;<15>            + END LOOP
;          END REGION

; Framework considers %shl = shl i64 %ret.011, 32
; as 4294967296 * %ret.011 in HIR
; Ensure we cg such large coeffs correctly
; CHECK: region.0:
; CHECK: mul i64 4294967296
; ModuleID = 'encdec/crypto/bn/bn_word.c'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.bignum_st = type { i32*, i32, i32, i32 }

; Function Attrs: nounwind readonly
define i32 @BN_mod_word(%struct.bignum_st* nocapture readonly %a, i32 %w) {
entry:
  %top = getelementptr inbounds %struct.bignum_st, %struct.bignum_st* %a, i32 0, i32 1
  %0 = load i32, i32* %top, align 4
  %cmp.10 = icmp sgt i32 %0, 0
  br i1 %cmp.10, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %d = getelementptr inbounds %struct.bignum_st, %struct.bignum_st* %a, i32 0, i32 0
  %1 = load i32*, i32** %d, align 4
  %conv1 = zext i32 %w to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %i.012.in = phi i32 [ %0, %for.body.lr.ph ], [ %i.012, %for.body ]
  %ret.011 = phi i64 [ 0, %for.body.lr.ph ], [ %rem, %for.body ]
  %i.012 = add nsw i32 %i.012.in, -1
  %shl = shl i64 %ret.011, 32
  %arrayidx = getelementptr inbounds i32, i32* %1, i32 %i.012
  %2 = load i32, i32* %arrayidx, align 4
  %conv = zext i32 %2 to i64
  %or = or i64 %conv, %shl
  %rem = urem i64 %or, %conv1
  %cmp = icmp sgt i32 %i.012.in, 1
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  %extract.t = trunc i64 %rem to i32
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %ret.0.lcssa.off0 = phi i32 [ 0, %entry ], [ %extract.t, %for.end.loopexit ]
  ret i32 %ret.0.lcssa.off0
}

