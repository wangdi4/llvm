; Test to check HIR vector codegen support for vectorizable intrinsic calls with always scalar operands.
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg" -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s

; Check that intrinsic calls are vectorized.
; CHECK-LABEL: ctlz_f64
; CHECK: @llvm.ctlz.v4i64([[VEC:%.*]],  {{-1|true}})

; CHECK-LABEL: cttz_f64
; CHECK: @llvm.cttz.v4i64([[VEC:%.*]],  {{-1|true}})

; CHECK-LABEL: powi_f64
; CHECK: @llvm.powi.v4f64.i32([[VEC:%.*]],  %P)

; Check that call is serialized, when intrinsic with always scalar operand is loop variant.
; CHECK-LABEL: powi_f64_variant
; CHECK-COUNT-4: [[SERIAL_POW:%.*]] = @llvm.powi.f64.i32([[SCALAR_OP_1:%.*]], [[SCALAR_OP_2:.*]])

; Check that call is vectorized, when intrinsic with always scalar operand is uniform despite
; being loop variant.
; CHECK-LABEL: powi_f64_uni_variant
; CHECK: @llvm.powi.v4f64.i32([[VEC:%.*]],  %.unifload)

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

declare double @llvm.powi.f64.i32(double %Val, i32 %power) nounwind readnone

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
  %call = tail call double @llvm.powi.f64.i32(double %0, i32 %P) #4
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
  %call = tail call double @llvm.powi.f64.i32(double %0, i32 %exponent) #4
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

; Test for scenario where powi's exponent operand (should be always scalar) is loop variant,
; but uniform in nature.
define void @powi_f64_uni_variant(double* noalias nocapture readonly %y, double* noalias nocapture %x, i32* %LP) {
entry:
  br label %for.body

for.body: ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds double, double* %y, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8
  %P = load i32, i32* %LP, align 4
  %call = tail call double @llvm.powi.f64.i32(double %0, i32 %P) #4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv, 99
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit: ; preds = %for.body
  ret void
}
