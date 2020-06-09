; Test to check HIR vector codegen support for vectorizable intrinsic calls with always scalar operands.
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -enable-vp-value-codegen-hir=0 -disable-output  < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -enable-vp-value-codegen-hir -disable-output  < %s 2>&1 | FileCheck %s

; Check that intrinsic calls are vectorized.
; CHECK-LABEL: ctlz_f64
; CHECK: @llvm.ctlz.v4i64(%.vec,  {{-1|true}})

; CHECK-LABEL: cttz_f64
; CHECK: @llvm.cttz.v4i64(%.vec,  {{-1|true}})

; CHECK-LABEL: powi_f64
; CHECK: @llvm.powi.v4f64(%.vec,  %P)

; Check that loop is not vectorized, when intrinsic with always scalar operand is loop variant.
; CHECK-LABEL: powi_f64_variant
; CHECK: DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: %call = @llvm.powi.f64(%0,  i1)


declare i64  @llvm.ctlz.i64 (i64, i1) nounwind readnone

define void @ctlz_f64(i32 %n, i64* noalias nocapture readonly %y, i64* noalias nocapture %x) local_unnamed_addr #1 {
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %y, i64 %indvars.iv
  %0 = load i64, i64* %arrayidx, align 8
  %call = tail call i64 @llvm.ctlz.i64(i64 %0, i1 true) nounwind readnone
  %arrayidx4 = getelementptr inbounds i64, i64* %x, i64 %indvars.iv
  store i64 %call, i64* %arrayidx4, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare i64  @llvm.cttz.i64 (i64, i1) nounwind readnone

define void @cttz_f64(i32 %n, i64* noalias nocapture readonly %y, i64* noalias nocapture %x) local_unnamed_addr #1 {
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %y, i64 %indvars.iv
  %0 = load i64, i64* %arrayidx, align 8
  %call = tail call i64 @llvm.cttz.i64(i64 %0, i1 true) #2 nounwind readnone
  %arrayidx4 = getelementptr inbounds i64, i64* %x, i64 %indvars.iv
  store i64 %call, i64* %arrayidx4, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare double @llvm.powi.f64(double %Val, i32 %power) nounwind readnone

define void @powi_f64(i32 %n, double* noalias nocapture readonly %y, double* noalias nocapture %x, i32 %P) local_unnamed_addr #2 {
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, double* %y, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8
  %call = tail call double @llvm.powi.f64(double %0, i32 %P) #4
  %arrayidx4 = getelementptr inbounds double, double* %x, i64 %indvars.iv
  store double %call, double* %arrayidx4, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Test for scenario where powi's exponent operand (should be always scalar) is loop variant.
define void @powi_f64_variant(i32 %n, double* noalias nocapture readonly %y, double* noalias nocapture %x) local_unnamed_addr #2 {
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, double* %y, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8
  %exponent = trunc i64 %indvars.iv to i32
  %call = tail call double @llvm.powi.f64(double %0, i32 %exponent) #4
  %arrayidx4 = getelementptr inbounds double, double* %x, i64 %indvars.iv
  store double %call, double* %arrayidx4, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
