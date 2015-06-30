; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../../Full/runtime.bc -std-compile-opts  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s
; ModuleID = 'infLoop.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: ocl_Kernel_Func1
; CHECK: ret
define void @ocl_Kernel_Func1(i32 %iCallN, i32 %iInit, i32 %iUpOrDown, i32 %iWithCap, float %fVRacT, float %fBarrier, i32 %iSide, float %fEpsilon, float addrspace(1)* nocapture %gpfAsset, float addrspace(1)* nocapture %gpfAbove) nounwind {
entry:
  %call = tail call i64 @get_local_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %cmp102 = icmp slt i32 %conv, %iCallN
  br i1 %cmp102, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %call1 = tail call i64 @get_local_size(i32 0) nounwind readnone
  %tobool = icmp eq i32 %iWithCap, 0
  %tobool21 = icmp ne i32 %iInit, 0
  %cmp22 = icmp eq i32 %iUpOrDown, 1
  %or.cond = and i1 %tobool21, %cmp22
  %cmp31 = icmp eq i32 %iUpOrDown, 0
  %or.cond1 = and i1 %tobool21, %cmp31
  %sext61 = shl i64 %call, 32
  %0 = ashr exact i64 %sext61, 32
  %sext62 = shl i64 %call1, 32
  %1 = ashr exact i64 %sext62, 32
  br i1 %tobool, label %for.body.lr.ph.split.us, label %for.body.lr.ph.split

for.body.lr.ph.split.us:                          ; preds = %for.body.lr.ph
  br i1 %or.cond, label %if.end40.us.us, label %if.else28.us

if.end40.us.us:                                   ; preds = %for.body.lr.ph.split.us, %if.end40.us.us
  %indvars.iv52 = phi i64 [ %indvars.iv.next53, %if.end40.us.us ], [ %0, %for.body.lr.ph.split.us ]
  %call27.us.us = tail call float @_Z3maxff(float 0.000000e+00, float fdiv (float fsub (float 5.000000e-01, float undef), float fsub (float 1.000000e+00, float undef))) nounwind readnone
  %arrayidx42.us.us = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv52
  store float %call27.us.us, float addrspace(1)* %arrayidx42.us.us, align 4
  %indvars.iv.next53 = add i64 %indvars.iv52, %1
  %2 = trunc i64 %indvars.iv.next53 to i32
  %cmp10.us.us = icmp slt i32 %2, %iCallN
  br i1 %cmp10.us.us, label %if.end40.us.us, label %for.end

if.else28.us:                                     ; preds = %for.body.lr.ph.split.us, %if.end40.us
  %indvars.iv46 = phi i64 [ %indvars.iv.next47, %if.end40.us ], [ %0, %for.body.lr.ph.split.us ]
  br i1 %or.cond1, label %if.then33.us, label %if.end40.us

if.then33.us:                                     ; preds = %if.else28.us
  %call36.us = tail call float @_Z3maxff(float 0.000000e+00, float fdiv (float fsub (float undef, float 5.000000e-01), float undef)) nounwind readnone
  %sub37.us = fsub float 1.000000e+00, %call36.us
  br label %if.end40.us

if.end40.us:                                      ; preds = %if.then33.us, %if.else28.us
  %fAbove.0.us = phi float [ %sub37.us, %if.then33.us ], [ 5.000000e-01, %if.else28.us ]
  %arrayidx42.us = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv46
  store float %fAbove.0.us, float addrspace(1)* %arrayidx42.us, align 4
  %indvars.iv.next47 = add i64 %indvars.iv46, %1
  %3 = trunc i64 %indvars.iv.next47 to i32
  %cmp10.us = icmp slt i32 %3, %iCallN
  br i1 %cmp10.us, label %if.else28.us, label %for.end

for.body.lr.ph.split:                             ; preds = %for.body.lr.ph
  br i1 %or.cond, label %if.then.us8, label %for.body.lr.ph.split.split

if.then.us8:                                      ; preds = %for.body.lr.ph.split, %if.end40.us20
  %indvars.iv48 = phi i64 [ %indvars.iv.next49, %if.end40.us20 ], [ %0, %for.body.lr.ph.split ]
  %arrayidx.us7 = getelementptr inbounds float addrspace(1)* %gpfAsset, i64 %indvars.iv48
  %4 = load float addrspace(1)* %arrayidx.us7, align 4
  %cmp12.us9 = fcmp ogt float %4, 0.000000e+00
  br i1 %cmp12.us9, label %if.end40.us20, label %if.else.us10

if.else.us10:                                     ; preds = %if.then.us8
  %cmp15.us11 = fcmp olt float %4, 0.000000e+00
  %..us12 = select i1 %cmp15.us11, float 0.000000e+00, float 5.000000e-01
  br label %if.end40.us20

if.end40.us20:                                    ; preds = %if.else.us10, %if.then.us8
  %fAbove.0.us21 = phi float [ 1.000000e+00, %if.then.us8 ], [ %..us12, %if.else.us10 ]
  %arrayidx42.us22 = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv48
  store float %fAbove.0.us21, float addrspace(1)* %arrayidx42.us22, align 4
  %indvars.iv.next49 = add i64 %indvars.iv48, %1
  %5 = trunc i64 %indvars.iv.next49 to i32
  %cmp10.us24 = icmp slt i32 %5, %iCallN
  br i1 %cmp10.us24, label %if.then.us8, label %for.end

for.body.lr.ph.split.split:                       ; preds = %for.body.lr.ph.split
  br i1 %or.cond1, label %if.then.us29, label %if.then

if.then.us29:                                     ; preds = %for.body.lr.ph.split.split, %if.end40.us41
  %indvars.iv50 = phi i64 [ %indvars.iv.next51, %if.end40.us41 ], [ %0, %for.body.lr.ph.split.split ]
  %arrayidx.us28 = getelementptr inbounds float addrspace(1)* %gpfAsset, i64 %indvars.iv50
  %6 = load float addrspace(1)* %arrayidx.us28, align 4
  %cmp12.us30 = fcmp ogt float %6, 0.000000e+00
  br i1 %cmp12.us30, label %if.end40.us41, label %if.else.us31

if.else.us31:                                     ; preds = %if.then.us29
  %cmp15.us32 = fcmp olt float %6, 0.000000e+00
  %..us33 = select i1 %cmp15.us32, float 0.000000e+00, float 5.000000e-01
  br label %if.end40.us41

if.end40.us41:                                    ; preds = %if.else.us31, %if.then.us29
  %fAbove.0.us42 = phi float [ 1.000000e+00, %if.then.us29 ], [ %..us33, %if.else.us31 ]
  %arrayidx42.us43 = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv50
  store float %fAbove.0.us42, float addrspace(1)* %arrayidx42.us43, align 4
  %indvars.iv.next51 = add i64 %indvars.iv50, %1
  %7 = trunc i64 %indvars.iv.next51 to i32
  %cmp10.us45 = icmp slt i32 %7, %iCallN
  br i1 %cmp10.us45, label %if.then.us29, label %for.end

if.then:                                          ; preds = %for.body.lr.ph.split.split, %if.end40
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end40 ], [ %0, %for.body.lr.ph.split.split ]
  %arrayidx = getelementptr inbounds float addrspace(1)* %gpfAsset, i64 %indvars.iv
  %8 = load float addrspace(1)* %arrayidx, align 4
  %cmp12 = fcmp ogt float %8, 0.000000e+00
  br i1 %cmp12, label %if.end40, label %if.else

if.else:                                          ; preds = %if.then
  %cmp15 = fcmp olt float %8, 0.000000e+00
  %. = select i1 %cmp15, float 0.000000e+00, float 5.000000e-01
  br label %if.end40

if.end40:                                         ; preds = %if.else, %if.then
  %fAbove.0 = phi float [ 1.000000e+00, %if.then ], [ %., %if.else ]
  %arrayidx42 = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv
  store float %fAbove.0, float addrspace(1)* %arrayidx42, align 4
  %indvars.iv.next = add i64 %indvars.iv, %1
  %9 = trunc i64 %indvars.iv.next to i32
  %cmp10 = icmp slt i32 %9, %iCallN
  br i1 %cmp10, label %if.then, label %for.end

for.end:                                          ; preds = %if.end40.us, %if.end40.us.us, %if.end40.us41, %if.end40, %if.end40.us20, %entry
  ret void
}

declare i64 @get_local_id(i32) nounwind readnone

declare i64 @get_local_size(i32) nounwind readnone

declare float @_Z3maxff(float, float) nounwind readnone

define void @ocl_Kernel_Func2(i32 %iCallN, i32 %iWithCap, float %fVRacT, float addrspace(1)* nocapture %gpfBarrier, i32 %iSide, float %fEpsilon, float addrspace(1)* nocapture %gpfAsset, float addrspace(1)* nocapture %gpfAbove) nounwind {
entry:
  %call = tail call i64 @get_local_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %call1 = tail call i64 @get_local_size(i32 0) nounwind readnone
  %tobool = icmp eq i32 %iWithCap, 0
  br i1 %tobool, label %cond.end, label %cond.true

cond.true:                                        ; preds = %entry
  %mul = fmul float %fVRacT, 5.000000e+00
  %call3 = tail call float @_Z3expf(float %mul) nounwind readnone
  br label %cond.end

cond.end:                                         ; preds = %entry, %cond.true
  %cond = phi float [ %call3, %cond.true ], [ 0.000000e+00, %entry ]
  %cmp10 = icmp slt i32 %conv, %iCallN
  br i1 %cmp10, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %cond.end
  %cmp9 = icmp eq i32 %iSide, 1
  %cond14 = select i1 %cmp9, float %fEpsilon, float 0.000000e+00
  %cmp15 = icmp eq i32 %iSide, -1
  %cond20 = select i1 %cmp15, float %fEpsilon, float 0.000000e+00
  %tobool39 = icmp ne i32 %iWithCap, 0
  %sext = shl i64 %call, 32
  %0 = ashr exact i64 %sext, 32
  %sext14 = shl i64 %call1, 32
  %1 = ashr exact i64 %sext14, 32
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %indvars.iv = phi i64 [ %0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds float addrspace(1)* %gpfAsset, i64 %indvars.iv
  %2 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx8 = getelementptr inbounds float addrspace(1)* %gpfBarrier, i64 %indvars.iv
  %3 = load float addrspace(1)* %arrayidx8, align 4
  %add = fadd float %cond14, %3
  %sub = fsub float %3, %cond20
  br i1 %tobool, label %if.else.thread, label %if.then

if.else.thread:                                   ; preds = %for.body
  %arrayidx5117 = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv
  br label %if.else52

if.then:                                          ; preds = %for.body
  %cmp23 = fcmp ogt float %sub, 0.000000e+00
  br i1 %cmp23, label %cond.true25, label %cond.false26

cond.true25:                                      ; preds = %if.then
  %div = fdiv float %sub, %cond, !fpmath !14
  br label %cond.end28

cond.false26:                                     ; preds = %if.then
  %mul27 = fmul float %cond, %sub
  br label %cond.end28

cond.end28:                                       ; preds = %cond.false26, %cond.true25
  %cond29 = phi float [ %div, %cond.true25 ], [ %mul27, %cond.false26 ]
  %cmp31 = fcmp ogt float %add, 0.000000e+00
  br i1 %cmp31, label %cond.true33, label %cond.false35

cond.true33:                                      ; preds = %cond.end28
  %mul34 = fmul float %cond, %add
  br label %if.end

cond.false35:                                     ; preds = %cond.end28
  %div36 = fdiv float %add, %cond, !fpmath !14
  br label %if.end

if.end:                                           ; preds = %cond.true33, %cond.false35
  %fSmoothMax.1 = phi float [ %div36, %cond.false35 ], [ %mul34, %cond.true33 ]
  %cmp40 = fcmp ogt float %2, %fSmoothMax.1
  %or.cond = and i1 %tobool39, %cmp40
  br i1 %or.cond, label %if.then42, label %if.else

if.then42:                                        ; preds = %if.end
  %arrayidx44 = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv
  store float 1.000000e+00, float addrspace(1)* %arrayidx44, align 4
  br label %for.inc

if.else:                                          ; preds = %if.end
  %cmp47 = fcmp olt float %2, %cond29
  %or.cond1 = and i1 %tobool39, %cmp47
  %arrayidx51 = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv
  br i1 %or.cond1, label %if.then49, label %if.else52

if.then49:                                        ; preds = %if.else
  store float 0.000000e+00, float addrspace(1)* %arrayidx51, align 4
  br label %for.inc

if.else52:                                        ; preds = %if.else.thread, %if.else
  %arrayidx5119 = phi float addrspace(1)* [ %arrayidx5117, %if.else.thread ], [ %arrayidx51, %if.else ]
  store float 5.000000e-01, float addrspace(1)* %arrayidx5119, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then42, %if.else52, %if.then49
  %indvars.iv.next = add i64 %indvars.iv, %1
  %4 = trunc i64 %indvars.iv.next to i32
  %cmp = icmp slt i32 %4, %iCallN
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc, %cond.end
  ret void
}

declare float @_Z3expf(float) nounwind readnone

define void @ocl_Kernel_5(i32 %iCallN, i32 %iWithCap, float %fVRacT, float addrspace(1)* nocapture %gpfBarrier, i32 %iSide, float %fEpsilon, float addrspace(1)* nocapture %gpfAsset, float addrspace(1)* nocapture %gpfAbove) nounwind {
entry:
  %call.i = tail call i64 @get_local_id(i32 0) nounwind readnone
  %conv.i = trunc i64 %call.i to i32
  %call1.i = tail call i64 @get_local_size(i32 0) nounwind readnone
  %tobool.i = icmp eq i32 %iWithCap, 0
  br i1 %tobool.i, label %cond.end.i, label %cond.true.i

cond.true.i:                                      ; preds = %entry
  %mul.i = fmul float %fVRacT, 5.000000e+00
  %call3.i = tail call float @_Z3expf(float %mul.i) nounwind readnone
  br label %cond.end.i

cond.end.i:                                       ; preds = %cond.true.i, %entry
  %cond.i = phi float [ %call3.i, %cond.true.i ], [ 0.000000e+00, %entry ]
  %cmp10.i = icmp slt i32 %conv.i, %iCallN
  br i1 %cmp10.i, label %for.body.lr.ph.i, label %ocl_Kernel_Func2.exit

for.body.lr.ph.i:                                 ; preds = %cond.end.i
  %cmp9.i = icmp eq i32 %iSide, 1
  %cond14.i = select i1 %cmp9.i, float %fEpsilon, float 0.000000e+00
  %cmp15.i = icmp eq i32 %iSide, -1
  %cond20.i = select i1 %cmp15.i, float %fEpsilon, float 0.000000e+00
  %tobool39.i = icmp ne i32 %iWithCap, 0
  %sext.i = shl i64 %call.i, 32
  %0 = ashr exact i64 %sext.i, 32
  %sext14.i = shl i64 %call1.i, 32
  %1 = ashr exact i64 %sext14.i, 32
  br label %for.body.i

for.body.i:                                       ; preds = %for.inc.i, %for.body.lr.ph.i
  %indvars.iv.i = phi i64 [ %0, %for.body.lr.ph.i ], [ %indvars.iv.next.i, %for.inc.i ]
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %gpfAsset, i64 %indvars.iv.i
  %2 = load float addrspace(1)* %arrayidx.i, align 4
  %arrayidx8.i = getelementptr inbounds float addrspace(1)* %gpfBarrier, i64 %indvars.iv.i
  %3 = load float addrspace(1)* %arrayidx8.i, align 4
  %add.i = fadd float %cond14.i, %3
  %sub.i = fsub float %3, %cond20.i
  br i1 %tobool.i, label %if.else.thread.i, label %if.then.i

if.else.thread.i:                                 ; preds = %for.body.i
  %arrayidx5117.i = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv.i
  br label %if.else52.i

if.then.i:                                        ; preds = %for.body.i
  %cmp23.i = fcmp ogt float %sub.i, 0.000000e+00
  br i1 %cmp23.i, label %cond.true25.i, label %cond.false26.i

cond.true25.i:                                    ; preds = %if.then.i
  %div.i = fdiv float %sub.i, %cond.i, !fpmath !14
  br label %cond.end28.i

cond.false26.i:                                   ; preds = %if.then.i
  %mul27.i = fmul float %cond.i, %sub.i
  br label %cond.end28.i

cond.end28.i:                                     ; preds = %cond.false26.i, %cond.true25.i
  %cond29.i = phi float [ %div.i, %cond.true25.i ], [ %mul27.i, %cond.false26.i ]
  %cmp31.i = fcmp ogt float %add.i, 0.000000e+00
  br i1 %cmp31.i, label %cond.true33.i, label %cond.false35.i

cond.true33.i:                                    ; preds = %cond.end28.i
  %mul34.i = fmul float %cond.i, %add.i
  br label %if.end.i

cond.false35.i:                                   ; preds = %cond.end28.i
  %div36.i = fdiv float %add.i, %cond.i, !fpmath !14
  br label %if.end.i

if.end.i:                                         ; preds = %cond.false35.i, %cond.true33.i
  %fSmoothMax.1.i = phi float [ %div36.i, %cond.false35.i ], [ %mul34.i, %cond.true33.i ]
  %cmp40.i = fcmp ogt float %2, %fSmoothMax.1.i
  %or.cond.i = and i1 %tobool39.i, %cmp40.i
  br i1 %or.cond.i, label %if.then42.i, label %if.else.i

if.then42.i:                                      ; preds = %if.end.i
  %arrayidx44.i = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv.i
  store float 1.000000e+00, float addrspace(1)* %arrayidx44.i, align 4
  br label %for.inc.i

if.else.i:                                        ; preds = %if.end.i
  %cmp47.i = fcmp olt float %2, %cond29.i
  %or.cond1.i = and i1 %tobool39.i, %cmp47.i
  %arrayidx51.i = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv.i
  br i1 %or.cond1.i, label %if.then49.i, label %if.else52.i

if.then49.i:                                      ; preds = %if.else.i
  store float 0.000000e+00, float addrspace(1)* %arrayidx51.i, align 4
  br label %for.inc.i

if.else52.i:                                      ; preds = %if.else.i, %if.else.thread.i
  %arrayidx5119.i = phi float addrspace(1)* [ %arrayidx5117.i, %if.else.thread.i ], [ %arrayidx51.i, %if.else.i ]
  store float 5.000000e-01, float addrspace(1)* %arrayidx5119.i, align 4
  br label %for.inc.i

for.inc.i:                                        ; preds = %if.else52.i, %if.then49.i, %if.then42.i
  %indvars.iv.next.i = add i64 %indvars.iv.i, %1
  %4 = trunc i64 %indvars.iv.next.i to i32
  %cmp.i = icmp slt i32 %4, %iCallN
  br i1 %cmp.i, label %for.body.i, label %ocl_Kernel_Func2.exit

ocl_Kernel_Func2.exit:                            ; preds = %cond.end.i, %for.inc.i
  ret void
}

define [7 x i64] @WG.boundaries.ocl_Kernel_5(i32, i32, float, float addrspace(1)*, i32, float, float addrspace(1)*, float addrspace(1)*) {
entry:
  %8 = call i64 @get_local_size(i32 0)
  %9 = call i64 @get_base_global_id.(i32 0)
  %10 = call i64 @get_local_size(i32 1)
  %11 = call i64 @get_base_global_id.(i32 1)
  %12 = call i64 @get_local_size(i32 2)
  %13 = call i64 @get_base_global_id.(i32 2)
  %14 = insertvalue [7 x i64] undef, i64 %8, 2
  %15 = insertvalue [7 x i64] %14, i64 %9, 1
  %16 = insertvalue [7 x i64] %15, i64 %10, 4
  %17 = insertvalue [7 x i64] %16, i64 %11, 3
  %18 = insertvalue [7 x i64] %17, i64 %12, 6
  %19 = insertvalue [7 x i64] %18, i64 %13, 5
  %20 = insertvalue [7 x i64] %19, i64 1, 0
  ret [7 x i64] %20
}

declare i64 @get_base_global_id.(i32)

define void @__Vectorized_.ocl_Kernel_5(i32 %iCallN, i32 %iWithCap, float %fVRacT, float addrspace(1)* nocapture %gpfBarrier, i32 %iSide, float %fEpsilon, float addrspace(1)* nocapture %gpfAsset, float addrspace(1)* nocapture %gpfAbove) nounwind {
entry:
  %call.i = tail call i64 @get_local_id(i32 0) nounwind readnone
  %conv.i = trunc i64 %call.i to i32
  %call1.i = tail call i64 @get_local_size(i32 0) nounwind readnone
  %tobool.i = icmp eq i32 %iWithCap, 0
  br i1 %tobool.i, label %cond.end.i, label %cond.true.i

cond.true.i:                                      ; preds = %entry
  %mul.i = fmul float %fVRacT, 5.000000e+00
  %call3.i = tail call float @_Z3expf(float %mul.i) nounwind readnone
  br label %cond.end.i

cond.end.i:                                       ; preds = %cond.true.i, %entry
  %cond.i = phi float [ %call3.i, %cond.true.i ], [ 0.000000e+00, %entry ]
  %cmp10.i = icmp slt i32 %conv.i, %iCallN
  br i1 %cmp10.i, label %for.body.lr.ph.i, label %ocl_Kernel_Func2.exit

for.body.lr.ph.i:                                 ; preds = %cond.end.i
  %cmp9.i = icmp eq i32 %iSide, 1
  %cond14.i = select i1 %cmp9.i, float %fEpsilon, float 0.000000e+00
  %cmp15.i = icmp eq i32 %iSide, -1
  %cond20.i = select i1 %cmp15.i, float %fEpsilon, float 0.000000e+00
  %tobool39.i = icmp ne i32 %iWithCap, 0
  %sext.i = shl i64 %call.i, 32
  %0 = ashr exact i64 %sext.i, 32
  %sext14.i = shl i64 %call1.i, 32
  %1 = ashr exact i64 %sext14.i, 32
  br label %for.body.i

for.body.i:                                       ; preds = %for.inc.i, %for.body.lr.ph.i
  %indvars.iv.i = phi i64 [ %0, %for.body.lr.ph.i ], [ %indvars.iv.next.i, %for.inc.i ]
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %gpfAsset, i64 %indvars.iv.i
  %2 = load float addrspace(1)* %arrayidx.i, align 4
  %arrayidx8.i = getelementptr inbounds float addrspace(1)* %gpfBarrier, i64 %indvars.iv.i
  %3 = load float addrspace(1)* %arrayidx8.i, align 4
  %add.i = fadd float %cond14.i, %3
  %sub.i = fsub float %3, %cond20.i
  br i1 %tobool.i, label %if.else.thread.i, label %if.then.i

if.else.thread.i:                                 ; preds = %for.body.i
  %arrayidx5117.i = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv.i
  br label %if.else52.i

if.then.i:                                        ; preds = %for.body.i
  %cmp23.i = fcmp ogt float %sub.i, 0.000000e+00
  br i1 %cmp23.i, label %cond.true25.i, label %cond.false26.i

cond.true25.i:                                    ; preds = %if.then.i
  %div.i = fdiv float %sub.i, %cond.i, !fpmath !14
  br label %cond.end28.i

cond.false26.i:                                   ; preds = %if.then.i
  %mul27.i = fmul float %cond.i, %sub.i
  br label %cond.end28.i

cond.end28.i:                                     ; preds = %cond.false26.i, %cond.true25.i
  %cond29.i = phi float [ %div.i, %cond.true25.i ], [ %mul27.i, %cond.false26.i ]
  %cmp31.i = fcmp ogt float %add.i, 0.000000e+00
  br i1 %cmp31.i, label %cond.true33.i, label %cond.false35.i

cond.true33.i:                                    ; preds = %cond.end28.i
  %mul34.i = fmul float %cond.i, %add.i
  br label %if.end.i

cond.false35.i:                                   ; preds = %cond.end28.i
  %div36.i = fdiv float %add.i, %cond.i, !fpmath !14
  br label %if.end.i

if.end.i:                                         ; preds = %cond.false35.i, %cond.true33.i
  %fSmoothMax.1.i = phi float [ %div36.i, %cond.false35.i ], [ %mul34.i, %cond.true33.i ]
  %cmp40.i = fcmp ogt float %2, %fSmoothMax.1.i
  %or.cond.i = and i1 %tobool39.i, %cmp40.i
  br i1 %or.cond.i, label %if.then42.i, label %if.else.i

if.then42.i:                                      ; preds = %if.end.i
  %arrayidx44.i = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv.i
  store float 1.000000e+00, float addrspace(1)* %arrayidx44.i, align 4
  br label %phi-split-bb

if.else.i:                                        ; preds = %if.end.i
  %cmp47.i = fcmp olt float %2, %cond29.i
  %or.cond1.i = and i1 %tobool39.i, %cmp47.i
  %arrayidx51.i = getelementptr inbounds float addrspace(1)* %gpfAbove, i64 %indvars.iv.i
  br i1 %or.cond1.i, label %if.then49.i, label %if.else52.i

if.then49.i:                                      ; preds = %if.else.i
  store float 0.000000e+00, float addrspace(1)* %arrayidx51.i, align 4
  br label %phi-split-bb

if.else52.i:                                      ; preds = %if.else.i, %if.else.thread.i
  %arrayidx5119.i = phi float addrspace(1)* [ %arrayidx5117.i, %if.else.thread.i ], [ %arrayidx51.i, %if.else.i ]
  store float 5.000000e-01, float addrspace(1)* %arrayidx5119.i, align 4
  br label %for.inc.i

phi-split-bb:                                     ; preds = %if.then42.i, %if.then49.i
  br label %for.inc.i

for.inc.i:                                        ; preds = %phi-split-bb, %if.else52.i
  %indvars.iv.next.i = add i64 %indvars.iv.i, %1
  %4 = trunc i64 %indvars.iv.next.i to i32
  %cmp.i = icmp slt i32 %4, %iCallN
  br i1 %cmp.i, label %for.body.i, label %ocl_Kernel_Func2.exit.loopexit

ocl_Kernel_Func2.exit.loopexit:                   ; preds = %for.inc.i
  br label %ocl_Kernel_Func2.exit

ocl_Kernel_Func2.exit:                            ; preds = %ocl_Kernel_Func2.exit.loopexit, %cond.end.i
  ret void
}

declare i1 @__ocl_allOne(i1)

declare i1 @__ocl_allZero(i1)

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!2}
!opencl.enable.FP_CONTRACT = !{}
!opencl.kernel_info = !{!3}

!0 = !{void (i32, i32, float, float addrspace(1)*, i32, float, float addrspace(1)*, float addrspace(1)*)* @ocl_Kernel_5, !1}
!1 = !{!"argument_attribute", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = !{!"-cl-std=CL1.2"}
!3 = !{void (i32, i32, float, float addrspace(1)*, i32, float, float addrspace(1)*, float addrspace(1)*)* @ocl_Kernel_5, !4}
!4 = !{!5, !6, !7, !8, !9, !10, !11, !12, !13}
!5 = !{!"local_buffer_size", null}
!6 = !{!"barrier_buffer_size", null}
!7 = !{!"kernel_execution_length", null}
!8 = !{!"kernel_has_barrier", null}
!9 = !{!"no_barrier_path", i1 true}
!10 = !{!"vectorized_kernel", null}
!11 = !{!"vectorized_width", null}
!12 = !{!"kernel_wrapper", null}
!13 = !{!"scalarized_kernel", null}
!14 = !{float 2.500000e+00}
