; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser -hir-details | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser -hir-details 2>&1 | FileCheck %s

; Verify that %mul228 is conservatively parsed as a self-blob rather than as (%0 % i2).

; Since the rval operands are all parsed as linear, reverse engineering 4 * i1 into %0 causes a new blob to be created and it is not clear whether this is preferrable.

; The verifier asserted on this test case because %0 wasn't getting marked as livein. This is because parser was assuming that all blobs are discovered during parsing of rvals hence causing inconsistency.


; CHECK: + DO i64 i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   + DO i64 i2 = 0, undef + -1, 1   <DO_LOOP>
; CHECK: |   |
; CHECK: |   |   %mul228 = 4 * i1  *  i2;
; CHECK: |   |   <LVAL-REG> NON-LINEAR i64 %mul228 {sb:6}
; CHECK: |   |
; CHECK: |   |   (undef)[0] = &((undef)[4 * zext.i30.i64(trunc.i64.i30((%mul228 /u 4)))]);
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: uwtable
define dso_local void @_Z16allocateLoopDatav() {
entry:
  br label %for.body.i.i455

for.body.i.i455:                                  ; preds = %for.body.i.i455, %entry
  br i1 undef, label %_ZN12_GLOBAL__N_116allocAndInitDataERN8LoopData9RealArrayEi.exit456.loopexit, label %for.body.i.i455

_ZN12_GLOBAL__N_116allocAndInitDataERN8LoopData9RealArrayEi.exit456.loopexit: ; preds = %for.body.i.i455
  br label %for.cond223.preheader

for.cond184.loopexit:                             ; preds = %for.cond.cleanup225
  ret void

for.cond223.preheader:                            ; preds = %for.cond.cleanup225, %_ZN12_GLOBAL__N_116allocAndInitDataERN8LoopData9RealArrayEi.exit456.loopexit
  %indvars.iv520 = phi i64 [ 0, %_ZN12_GLOBAL__N_116allocAndInitDataERN8LoopData9RealArrayEi.exit456.loopexit ], [ %indvars.iv.next521, %for.cond.cleanup225 ]
  %0 = shl i64 %indvars.iv520, 2
  br label %for.body226

for.cond.cleanup225:                              ; preds = %for.body226
  %indvars.iv.next521 = add nuw nsw i64 %indvars.iv520, 1
  %exitcond523 = icmp eq i64 %indvars.iv.next521, 2
  br i1 %exitcond523, label %for.cond184.loopexit, label %for.cond223.preheader

for.body226:                                      ; preds = %for.body226, %for.cond223.preheader
  %indvars.iv517 = phi i64 [ 0, %for.cond223.preheader ], [ %indvars.iv.next518, %for.body226 ]
  %mul228 = mul i64 %0, %indvars.iv517
  %1 = and i64 %mul228, 4294967292
  %arrayidx230 = getelementptr inbounds double, double* undef, i64 %1
  store double* %arrayidx230, double** undef, align 8
  %indvars.iv.next518 = add nuw nsw i64 %indvars.iv517, 1
  %exitcond519 = icmp eq i64 %indvars.iv.next518, undef
  br i1 %exitcond519, label %for.cond.cleanup225, label %for.body226
}

