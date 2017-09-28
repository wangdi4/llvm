; RUN: llvm-as %s -o %t.bc
; RUN: %oclopt  -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Crash1.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @__Vectorized_.ocl_Kernel1
; CHECK: for.cond5.preheader.i20.i:    ; preds =
; CHECK-NEXT: %indvars.iv14.i15.i = phi i64
; CHECK: ret

declare float @_Z4fabsf(float) nounwind readnone
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) nounwind
declare i64 @get_global_size(i32) nounwind readnone
declare i64 @_Z13get_global_idj(i32) nounwind readnone

define void @__Vectorized_.ocl_Kernel1(i32 %iEvalN, i32 %iVarN, i32 addrspace(1)* nocapture %gpiReduceN, float addrspace(1)* nocapture %gpfX, float addrspace(1)* nocapture %gpfV, float addrspace(1)* nocapture %gpfResults) nounwind {
entry:
  %c.i = alloca [4 x float], align 16
  %d.i = alloca [4 x float], align 16
  %afAbsc.i = alloca [4 x float], align 16
  %call = tail call i64 @get_global_size(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %conv2 = trunc i64 %call1 to i32
  %cmp51 = icmp sgt i32 %iVarN, 0
  br i1 %cmp51, label %for.cond4.preheader.lr.ph.split.us, label %for.inc13.preheader

for.inc13.preheader:                              ; preds = %entry
  br label %for.inc13

for.cond4.preheader.lr.ph.split.us:               ; preds = %entry
  %sext = shl i64 %call1, 32
  %0 = ashr exact i64 %sext, 32
  %sext9 = shl i64 %call, 32
  %1 = ashr exact i64 %sext9, 32
  br label %for.body7.lr.ph.us

for.inc13.us:                                     ; preds = %ocl_Func2.exit
  %indvars.iv.next8 = add i64 %indvars.iv7, %1
  %2 = trunc i64 %indvars.iv.next8 to i32
  %cmp.us = icmp slt i32 %2, %iEvalN
  br i1 %cmp.us, label %for.body7.lr.ph.us, label %for.end15.loopexit

for.body7.us:                                     ; preds = %for.body7.lr.ph.us, %ocl_Func2.exit
  %indvars.iv = phi i64 [ 0, %for.body7.lr.ph.us ], [ %indvars.iv.next, %ocl_Func2.exit ]
  %3 = trunc i64 %indvars.iv to i32
  %mul.us = mul nsw i32 %3, %iEvalN
  %add.us = add nsw i32 %mul.us, %19
  %idxprom.us = sext i32 %add.us to i64
  %arrayidx.us = getelementptr inbounds i32, i32 addrspace(1)* %gpiReduceN, i64 %idxprom.us
  %4 = load i32, i32 addrspace(1)* %arrayidx.us, align 4
  %5 = bitcast [4 x float]* %c.i to i8*
  %6 = bitcast [4 x float]* %d.i to i8*
  %7 = bitcast [4 x float]* %afAbsc.i to i8*
  call void @llvm.memset.p0i8.i64(i8* %5, i8 0, i64 16, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %6, i8 0, i64 16, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %7, i8 0, i64 16, i32 16, i1 false) nounwind
  %iNX.off.i = add i32 %4, -2
  %8 = icmp ult i32 %iNX.off.i, 2
  br i1 %8, label %if.then.i, label %ocl_Func2.exit

if.then.i:                                        ; preds = %for.body7.us
  %cmp2.i = icmp eq i32 %4, 2
  br i1 %cmp2.i, label %for.body.i.i.preheader, label %for.body.i14.i.preheader

for.body.i.i.preheader:                           ; preds = %if.then.i
  br label %for.body.i.i

for.body.i14.i.preheader:                         ; preds = %if.then.i
  br label %for.body.i14.i

for.body.i.i:                                     ; preds = %for.body.i.i.preheader, %for.body.i.i.for.body.i.i_crit_edge
  %9 = phi float [ %.pre, %for.body.i.i.for.body.i.i_crit_edge ], [ 0.000000e+00, %for.body.i.i.preheader ]
  %indvars.iv16.i.i = phi i64 [ %indvars.iv.next17.i.i, %for.body.i.i.for.body.i.i_crit_edge ], [ 0, %for.body.i.i.preheader ]
  %dif.011.i.i = phi float [ %dif.1.i.i, %for.body.i.i.for.body.i.i_crit_edge ], [ 0.000000e+00, %for.body.i.i.preheader ]
  %ns.010.i.i = phi i32 [ %ns.1.i.i, %for.body.i.i.for.body.i.i_crit_edge ], [ 0, %for.body.i.i.preheader ]
  %sub.i.i = fsub float 1.000000e+00, %9
  %call.i.i = call float @_Z4fabsf(float %sub.i.i) nounwind readnone
  %cmp1.i.i = fcmp olt float %call.i.i, %dif.011.i.i
  %10 = trunc i64 %indvars.iv16.i.i to i32
  %ns.1.i.i = select i1 %cmp1.i.i, i32 %10, i32 %ns.010.i.i
  %indvars.iv.next17.i.i = add i64 %indvars.iv16.i.i, 1
  %lftr.wideiv18.i.i = trunc i64 %indvars.iv.next17.i.i to i32
  %exitcond19.i.i = icmp eq i32 %lftr.wideiv18.i.i, 2
  br i1 %exitcond19.i.i, label %for.end29.i.i, label %for.body.i.i.for.body.i.i_crit_edge

for.body.i.i.for.body.i.i_crit_edge:              ; preds = %for.body.i.i
  %dif.1.i.i = select i1 %cmp1.i.i, float %call.i.i, float %dif.011.i.i
  %arrayidx.i.i.phi.trans.insert = getelementptr inbounds [4 x float], [4 x float]* %afAbsc.i, i64 0, i64 %indvars.iv.next17.i.i
  %.pre = load float, float* %arrayidx.i.i.phi.trans.insert, align 4
  br label %for.body.i.i

for.end29.i.i:                                    ; preds = %for.body.i.i
  %mul.i.i = shl nsw i32 %ns.1.i.i, 1
  %cmp31.i.i = icmp slt i32 %mul.i.i, 1
  br i1 %cmp31.i.i, label %cond.true.i.i, label %cond.false.i.i

cond.true.i.i:                                    ; preds = %for.end29.i.i
  %idxprom32.i.i = sext i32 %ns.1.i.i to i64
  %arrayidx33.i.i = getelementptr inbounds [4 x float], [4 x float]* %c.i, i64 0, i64 %idxprom32.i.i
  br label %ocl_Func1.exit.i

cond.false.i.i:                                   ; preds = %for.end29.i.i
  %dec.i.i = add nsw i32 %ns.1.i.i, -1
  %idxprom35.i.i = sext i32 %dec.i.i to i64
  %arrayidx36.i.i = getelementptr inbounds [4 x float], [4 x float]* %d.i, i64 0, i64 %idxprom35.i.i
  br label %ocl_Func1.exit.i

ocl_Func1.exit.i:                                 ; preds = %cond.false.i.i, %cond.true.i.i
  %cond.in.i.i = phi float* [ %arrayidx33.i.i, %cond.true.i.i ], [ %arrayidx36.i.i, %cond.false.i.i ]
  %cond.i.i = load float, float* %cond.in.i.i, align 4
  %add37.i.i = fadd float %cond.i.i, 0.000000e+00
  br label %phi-split-bb1

for.body.i14.i:                                   ; preds = %for.body.i14.i.preheader, %for.body.i14.i.for.body.i14.i_crit_edge
  %11 = phi float [ %.pre10, %for.body.i14.i.for.body.i14.i_crit_edge ], [ 0.000000e+00, %for.body.i14.i.preheader ]
  %indvars.iv16.i2.i = phi i64 [ %indvars.iv.next17.i11.i, %for.body.i14.i.for.body.i14.i_crit_edge ], [ 0, %for.body.i14.i.preheader ]
  %dif.011.i3.i = phi float [ %dif.1.i10.i, %for.body.i14.i.for.body.i14.i_crit_edge ], [ 0.000000e+00, %for.body.i14.i.preheader ]
  %ns.010.i4.i = phi i32 [ %ns.1.i9.i, %for.body.i14.i.for.body.i14.i_crit_edge ], [ 0, %for.body.i14.i.preheader ]
  %sub.i6.i = fsub float 1.000000e+00, %11
  %call.i7.i = call float @_Z4fabsf(float %sub.i6.i) nounwind readnone
  %cmp1.i8.i = fcmp olt float %call.i7.i, %dif.011.i3.i
  %12 = trunc i64 %indvars.iv16.i2.i to i32
  %ns.1.i9.i = select i1 %cmp1.i8.i, i32 %12, i32 %ns.010.i4.i
  %indvars.iv.next17.i11.i = add i64 %indvars.iv16.i2.i, 1
  %lftr.wideiv18.i12.i = trunc i64 %indvars.iv.next17.i11.i to i32
  %exitcond19.i13.i = icmp eq i32 %lftr.wideiv18.i12.i, 3
  br i1 %exitcond19.i13.i, label %for.cond5.preheader.i20.i.loopexit, label %for.body.i14.i.for.body.i14.i_crit_edge

for.body.i14.i.for.body.i14.i_crit_edge:          ; preds = %for.body.i14.i
  %dif.1.i10.i = select i1 %cmp1.i8.i, float %call.i7.i, float %dif.011.i3.i
  %arrayidx.i5.i.phi.trans.insert = getelementptr inbounds [4 x float], [4 x float]* %afAbsc.i, i64 0, i64 %indvars.iv.next17.i11.i
  %.pre10 = load float, float* %arrayidx.i5.i.phi.trans.insert, align 4
  br label %for.body.i14.i

for.cond5.preheader.i20.i.loopexit:               ; preds = %for.body.i14.i
  br label %for.cond5.preheader.i20.i

for.cond5.preheader.i20.i:                        ; preds = %for.cond5.preheader.i20.i.loopexit, %cond.end.i49.i
  %indvars.iv14.i15.i = phi i64 [ %indvars.iv.next15.i46.i, %cond.end.i49.i ], [ 1, %for.cond5.preheader.i20.i.loopexit ]
  %y.06.i16.i = phi float [ %add37.i45.i, %cond.end.i49.i ], [ 0.000000e+00, %for.cond5.preheader.i20.i.loopexit ]
  %ns.25.i17.i = phi i32 [ %ns.3.i42.i, %cond.end.i49.i ], [ %ns.1.i9.i, %for.cond5.preheader.i20.i.loopexit ]
  %13 = trunc i64 %indvars.iv14.i15.i to i32
  %sub6.i18.i = sub nsw i32 3, %13
  %cmp71.i19.i = icmp sgt i32 %sub6.i18.i, 0
  br i1 %cmp71.i19.i, label %for.body8.i26.i.preheader, label %for.end29.i34.i

for.body8.i26.i.preheader:                        ; preds = %for.cond5.preheader.i20.i
  br label %for.body8.i26.i

for.body8.i26.i:                                  ; preds = %for.body8.i26.i.preheader, %if.end22.i31.i
  %indvars.iv.i21.i = phi i64 [ %indvars.iv.next.i29.i, %if.end22.i31.i ], [ 0, %for.body8.i26.i.preheader ]
  %arrayidx10.i22.i = getelementptr inbounds [4 x float], [4 x float]* %afAbsc.i, i64 0, i64 %indvars.iv.i21.i
  %14 = load float, float* %arrayidx10.i22.i, align 4
  %15 = add nsw i64 %indvars.iv.i21.i, %indvars.iv14.i15.i
  %arrayidx12.i23.i = getelementptr inbounds [4 x float], [4 x float]* %afAbsc.i, i64 0, i64 %15
  %16 = load float, float* %arrayidx12.i23.i, align 4
  %sub19.i24.i = fsub float %14, %16
  %cmp20.i25.i = fcmp oeq float %sub19.i24.i, 0.000000e+00
  br i1 %cmp20.i25.i, label %phi-split-bb, label %if.end22.i31.i

if.end22.i31.i:                                   ; preds = %for.body8.i26.i
  %arrayidx24.i27.i = getelementptr inbounds [4 x float], [4 x float]* %d.i, i64 0, i64 %indvars.iv.i21.i
  store float %16, float* %arrayidx24.i27.i, align 4
  %arrayidx26.i28.i = getelementptr inbounds [4 x float], [4 x float]* %c.i, i64 0, i64 %indvars.iv.i21.i
  store float %14, float* %arrayidx26.i28.i, align 4
  %indvars.iv.next.i29.i = add i64 %indvars.iv.i21.i, 1
  %17 = trunc i64 %indvars.iv.next.i29.i to i32
  %cmp7.i30.i = icmp slt i32 %17, %sub6.i18.i
  br i1 %cmp7.i30.i, label %for.body8.i26.i, label %phi-split-bb

phi-split-bb:                                     ; preds = %for.body8.i26.i, %if.end22.i31.i
  br label %for.end29.i34.i

for.end29.i34.i:                                  ; preds = %phi-split-bb, %for.cond5.preheader.i20.i
  %mul.i32.i = shl nsw i32 %ns.25.i17.i, 1
  %cmp31.i33.i = icmp slt i32 %mul.i32.i, %sub6.i18.i
  br i1 %cmp31.i33.i, label %cond.true.i37.i, label %cond.false.i41.i

cond.true.i37.i:                                  ; preds = %for.end29.i34.i
  %idxprom32.i35.i = sext i32 %ns.25.i17.i to i64
  %arrayidx33.i36.i = getelementptr inbounds [4 x float], [4 x float]* %c.i, i64 0, i64 %idxprom32.i35.i
  br label %cond.end.i49.i

cond.false.i41.i:                                 ; preds = %for.end29.i34.i
  %dec.i38.i = add nsw i32 %ns.25.i17.i, -1
  %idxprom35.i39.i = sext i32 %dec.i38.i to i64
  %arrayidx36.i40.i = getelementptr inbounds [4 x float], [4 x float]* %d.i, i64 0, i64 %idxprom35.i39.i
  br label %cond.end.i49.i

cond.end.i49.i:                                   ; preds = %cond.false.i41.i, %cond.true.i37.i
  %ns.3.i42.i = phi i32 [ %ns.25.i17.i, %cond.true.i37.i ], [ %dec.i38.i, %cond.false.i41.i ]
  %cond.in.i43.i = phi float* [ %arrayidx33.i36.i, %cond.true.i37.i ], [ %arrayidx36.i40.i, %cond.false.i41.i ]
  %cond.i44.i = load float, float* %cond.in.i43.i, align 4
  %add37.i45.i = fadd float %y.06.i16.i, %cond.i44.i
  %indvars.iv.next15.i46.i = add i64 %indvars.iv14.i15.i, 1
  %lftr.wideiv.i47.i = trunc i64 %indvars.iv.next15.i46.i to i32
  %exitcond.i48.i = icmp eq i32 %lftr.wideiv.i47.i, 3
  br i1 %exitcond.i48.i, label %phi-split-bb1.loopexit, label %for.cond5.preheader.i20.i

phi-split-bb1.loopexit:                           ; preds = %cond.end.i49.i
  br label %phi-split-bb1

phi-split-bb1:                                    ; preds = %phi-split-bb1.loopexit, %ocl_Func1.exit.i
  %new_phi = phi float [ %add37.i.i, %ocl_Func1.exit.i ], [ %add37.i45.i, %phi-split-bb1.loopexit ]
  br label %ocl_Func2.exit

ocl_Func2.exit:                                   ; preds = %phi-split-bb1, %for.body7.us
  %fValue.0.i = phi float [ 0.000000e+00, %for.body7.us ], [ %new_phi, %phi-split-bb1 ]
  %18 = add nsw i64 %indvars.iv, %20
  %arrayidx12.us = getelementptr inbounds float, float addrspace(1)* %gpfResults, i64 %18
  store float %fValue.0.i, float addrspace(1)* %arrayidx12.us, align 4
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %iVarN
  br i1 %exitcond, label %for.inc13.us, label %for.body7.us

for.body7.lr.ph.us:                               ; preds = %for.inc13.us, %for.cond4.preheader.lr.ph.split.us
  %indvars.iv7 = phi i64 [ %indvars.iv.next8, %for.inc13.us ], [ %0, %for.cond4.preheader.lr.ph.split.us ]
  %19 = trunc i64 %indvars.iv7 to i32
  %mul9.us = mul nsw i32 %19, %iVarN
  %20 = sext i32 %mul9.us to i64
  br label %for.body7.us

for.inc13:                                        ; preds = %for.inc13.preheader, %for.inc13
  %iEval.04 = phi i32 [ %add14, %for.inc13 ], [ %conv2, %for.inc13.preheader ]
  %add14 = add nsw i32 %iEval.04, %conv
  %cmp = icmp slt i32 %add14, %iEvalN
  br i1 %cmp, label %for.inc13, label %for.end15.loopexit2

for.end15.loopexit:                               ; preds = %for.inc13.us
  br label %for.end15

for.end15.loopexit2:                              ; preds = %for.inc13
  br label %for.end15

for.end15:                                        ; preds = %for.end15.loopexit2, %for.end15.loopexit
  ret void
}
