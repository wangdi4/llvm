; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts
; RUN: opt  -passes=tight-loop-emitter -tight-loop-emitter-run=remark -tight-loop-emitter-relax -debug-only=tight-loop-emitter  -disable-output < %s 2>&1 | FileCheck %s

; This test is for checking sanity of the cycle-finding algorithm. All cylces are found even if the cycles are not satisfying
; various tightness conditions.

; CHECK: Intel Tight Loop Emitter : bar
; CHECK:     Tight cycle found for Loop: while.body.i.i.i
; CHECK-NEXT:   %xr.0205.i.i.i = phi float [ %astype9.i.i.i, %while.body.i.preheader.new.i.i.new ], [ %add.i.3.i.i.1, %while.body.i.i.i ]
; CHECK-NEXT:   %sub15.i.i.i = fsub float %xr.0205.i.i.i, %9
; CHECK-NEXT:   %add.i.i.i = fadd float %sub15.i.i.i, %sub15.i.i.i
; CHECK-NEXT:   %sub15.i.1.i.i = fsub float %add.i.i.i, %12
; CHECK-NEXT:   %add.i.1.i.i = fadd float %sub15.i.1.i.i, %sub15.i.1.i.i
; CHECK-NEXT:   %sub15.i.2.i.i = fsub float %add.i.1.i.i, %13
; CHECK-NEXT:   %add.i.2.i.i = fadd float %sub15.i.2.i.i, %sub15.i.2.i.i
; CHECK-NEXT:   %sub15.i.3.i.i = fsub float %add.i.2.i.i, %16
; CHECK-NEXT:   %add.i.3.i.i = fadd float %sub15.i.3.i.i, %sub15.i.3.i.i
; CHECK-NEXT:   %sub15.i.i.i.1 = fsub float %add.i.3.i.i, %17
; CHECK-NEXT:   %add.i.i.i.1 = fadd float %sub15.i.i.i.1, %sub15.i.i.i.1
; CHECK-NEXT:   %sub15.i.1.i.i.1 = fsub float %add.i.i.i.1, %20
; CHECK-NEXT:   %add.i.1.i.i.1 = fadd float %sub15.i.1.i.i.1, %sub15.i.1.i.i.1
; CHECK-NEXT:   %sub15.i.2.i.i.1 = fsub float %add.i.1.i.i.1, %21
; CHECK-NEXT:   %add.i.2.i.i.1 = fadd float %sub15.i.2.i.i.1, %sub15.i.2.i.i.1
; CHECK-NEXT:   %sub15.i.3.i.i.1 = fsub float %add.i.2.i.i.1, %24
; CHECK-NEXT:   %add.i.3.i.i.1 = fadd float %sub15.i.3.i.i.1, %sub15.i.3.i.i.1
; CHECK:     Tight cycle found for Loop: while.body.i.i.i
; CHECK-NEXT:   %q.0203.i.i.i = phi i32 [ 0, %while.body.i.preheader.new.i.i.new ], [ %or14.i.3.i.i.1, %while.body.i.i.i ]
; CHECK-NEXT:   %10 = shl i32 %q.0203.i.i.i, 2
; CHECK-NEXT:   %shl.i.1.i.i = or i32 %10, %11
; CHECK-NEXT:   %or14.i.1.i.i = or i32 %shl.i.1.i.i, %conv.i.1.i.i
; CHECK-NEXT:   %14 = shl i32 %or14.i.1.i.i, 2
; CHECK-NEXT:   %shl.i.3.i.i = or i32 %15, %14
; CHECK-NEXT:   %or14.i.3.i.i = or i32 %shl.i.3.i.i, %conv.i.3.i.i
; CHECK-NEXT:   %18 = shl i32 %or14.i.3.i.i, 2
; CHECK-NEXT:   %shl.i.1.i.i.1 = or i32 %18, %19
; CHECK-NEXT:   %or14.i.1.i.i.1 = or i32 %shl.i.1.i.i.1, %conv.i.1.i.i.1
; CHECK-NEXT:   %22 = shl i32 %or14.i.1.i.i.1, 2
; CHECK-NEXT:   %shl.i.3.i.i.1 = or i32 %23, %22
; CHECK-NEXT:   %or14.i.3.i.i.1 = or i32 %shl.i.3.i.i.1, %conv.i.3.i.i.1
; CHECK:     Tight cycle found for Loop: while.body.i.i.i
; CHECK-NEXT:   %niter = phi i32 [ 0, %while.body.i.preheader.new.i.i.new ], [ %niter.next.1, %while.body.i.i.i ]
; CHECK-NEXT:   %niter.next.1 = add i32 %niter, 2
; CHECK:     Tight cycle found for Loop: while.body.i.epil.i.i
; CHECK-NEXT:   %xr.0205.i.epil.i.i = phi float [ %add.i.epil.i.i, %while.body.i.epil.i.i ], [ %xr.0205.i.unr.i.i, %while.end.loopexit.i.unr-lcssa.i.i ]
; CHECK-NEXT:   %sub15.i.epil.i.i = fsub float %xr.0205.i.epil.i.i, %32
; CHECK-NEXT:   %add.i.epil.i.i = fadd float %sub15.i.epil.i.i, %sub15.i.epil.i.i
; CHECK:     Tight cycle found for Loop: while.body.i.epil.i.i
; CHECK-NEXT:   %q.0203.i.epil.i.i = phi i32 [ %or14.i.epil.i.i, %while.body.i.epil.i.i ], [ %q.0203.i.unr.i.i, %while.end.loopexit.i.unr-lcssa.i.i ]
; CHECK-NEXT:   %shl.i.epil.i.i = shl i32 %q.0203.i.epil.i.i, 1
; CHECK-NEXT:   %or14.i.epil.i.i = or i32 %shl.i.epil.i.i, %conv.i.epil.i.i
; CHECK:     Tight cycle found for Loop: while.body.i.epil.i.i
; CHECK-NEXT:   %epil.iter.i.i = phi i32 [ %epil.iter.next.i.i, %while.body.i.epil.i.i ], [ 0, %while.end.loopexit.i.unr-lcssa.i.i ]
; CHECK-NEXT:   %epil.iter.next.i.i = add nuw nsw i32 %epil.iter.i.i, 1

; Function Attrs: alwaysinline argmemonly nofree norecurse nosync nounwind writeonly
define dso_local half @bar(half noundef %args_0, half noundef %args_1, i32* nocapture noundef writeonly %args_2) local_unnamed_addr #80 {
entry:
  %conv.i.i = fpext half %args_0 to float
  %conv1.i.i = fpext half %args_1 to float
  %astype.i.i.i.i = bitcast float %conv.i.i to i32
  %and.i.i.i.i = and i32 %astype.i.i.i.i, 2139095040
  %cmp.i.i.i.i = icmp ne i32 %and.i.i.i.i, 0
  %and2.i.i.i.i = and i32 %astype.i.i.i.i, 8388607
  %cmp3.not.i.i.i.i = icmp eq i32 %and2.i.i.i.i, 0
  %or.cond11.i.i.i.i = or i1 %cmp.i.i.i.i, %cmp3.not.i.i.i.i
  %and4.i.i.i.i = and i32 %astype.i.i.i.i, -2147483648
  %astype.i193.i.i.i = bitcast float %conv1.i.i to i32
  %and.i195.i.i.i = and i32 %astype.i193.i.i.i, 2139095040
  %cmp.i196.i.i.i = icmp ne i32 %and.i195.i.i.i, 0
  %and2.i197.i.i.i = and i32 %astype.i193.i.i.i, 8388607
  %cmp3.not.i198.i.i.i = icmp eq i32 %and2.i197.i.i.i, 0
  %or.cond11.i199.i.i.i = or i1 %cmp.i196.i.i.i, %cmp3.not.i198.i.i.i
  %and4.i200.i.i.i = and i32 %astype.i193.i.i.i, -2147483648
  %astype.i.i.i = select i1 %or.cond11.i.i.i.i, i32 %astype.i.i.i.i, i32 %and4.i.i.i.i
  %and.i.i.i = and i32 %astype.i.i.i, 2147483647
  %0 = lshr i32 %and.i.i.i, 23
  %astype3.i.i.i = select i1 %or.cond11.i199.i.i.i, i32 %astype.i193.i.i.i, i32 %and4.i200.i.i.i
  %and4.i.i.i = and i32 %astype3.i.i.i, 2147483647
  %1 = lshr i32 %and4.i.i.i, 23
  %and8.i.i.i = and i32 %astype.i.i.i, 8388607
  %or.i.i.i = or i32 %and8.i.i.i, 1065353216
  %astype9.i.i.i = bitcast i32 %or.i.i.i to float
  %and10.i.i.i = and i32 %astype3.i.i.i, 8388607
  %or11.i.i.i = or i32 %and10.i.i.i, 1065353216
  %astype12.i.i.i = bitcast i32 %or11.i.i.i to float
  %sub.i.i.i = sub nsw i32 %0, %1
  %cmp202.i.i.i = icmp sgt i32 %sub.i.i.i, 0
  br i1 %cmp202.i.i.i, label %while.body.i.preheader.i.i, label %_Z18__spirv_ocl_remquoDhDhPi.exit

while.body.i.preheader.i.i:                       ; preds = %entry
  %2 = xor i32 %1, -1
  %3 = add nsw i32 %0, %2
  %xtraiter.i.i = and i32 %sub.i.i.i, 3
  %4 = icmp ult i32 %3, 3
  br i1 %4, label %while.end.loopexit.i.unr-lcssa.i.i, label %while.body.i.preheader.new.i.i

while.body.i.preheader.new.i.i:                   ; preds = %while.body.i.preheader.i.i
  %5 = add nsw i32 %sub.i.i.i, -4
  %6 = lshr i32 %5, 2
  %7 = add nuw nsw i32 %6, 1
  %xtraiter = and i32 %7, 1
  %8 = icmp ult i32 %5, 4
  br i1 %8, label %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa, label %while.body.i.preheader.new.i.i.new

while.body.i.preheader.new.i.i.new:               ; preds = %while.body.i.preheader.new.i.i
  %unroll_iter = and i32 %7, 2147483646
  br label %while.body.i.i.i

while.body.i.i.i:                                 ; preds = %while.body.i.i.i, %while.body.i.preheader.new.i.i.new
  %xr.0205.i.i.i = phi float [ %astype9.i.i.i, %while.body.i.preheader.new.i.i.new ], [ %add.i.3.i.i.1, %while.body.i.i.i ]
  %q.0203.i.i.i = phi i32 [ 0, %while.body.i.preheader.new.i.i.new ], [ %or14.i.3.i.i.1, %while.body.i.i.i ]
  %niter = phi i32 [ 0, %while.body.i.preheader.new.i.i.new ], [ %niter.next.1, %while.body.i.i.i ]
  %cmp13.i.i.i = fcmp oge float %xr.0205.i.i.i, %astype12.i.i.i
  %9 = select i1 %cmp13.i.i.i, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.i.i = fsub float %xr.0205.i.i.i, %9
  %add.i.i.i = fadd float %sub15.i.i.i, %sub15.i.i.i
  %cmp13.i.1.i.i = fcmp oge float %add.i.i.i, %astype12.i.i.i
  %conv.i.1.i.i = zext i1 %cmp13.i.1.i.i to i32
  %10 = shl i32 %q.0203.i.i.i, 2
  %11 = select i1 %cmp13.i.i.i, i32 2, i32 0
  %shl.i.1.i.i = or i32 %10, %11
  %or14.i.1.i.i = or i32 %shl.i.1.i.i, %conv.i.1.i.i
  %12 = select i1 %cmp13.i.1.i.i, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.1.i.i = fsub float %add.i.i.i, %12
  %add.i.1.i.i = fadd float %sub15.i.1.i.i, %sub15.i.1.i.i
  %cmp13.i.2.i.i = fcmp oge float %add.i.1.i.i, %astype12.i.i.i
  %13 = select i1 %cmp13.i.2.i.i, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.2.i.i = fsub float %add.i.1.i.i, %13
  %add.i.2.i.i = fadd float %sub15.i.2.i.i, %sub15.i.2.i.i
  %cmp13.i.3.i.i = fcmp oge float %add.i.2.i.i, %astype12.i.i.i
  %conv.i.3.i.i = zext i1 %cmp13.i.3.i.i to i32
  %14 = shl i32 %or14.i.1.i.i, 2
  %15 = select i1 %cmp13.i.2.i.i, i32 2, i32 0
  %shl.i.3.i.i = or i32 %15, %14
  %or14.i.3.i.i = or i32 %shl.i.3.i.i, %conv.i.3.i.i
  %16 = select i1 %cmp13.i.3.i.i, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.3.i.i = fsub float %add.i.2.i.i, %16
  %add.i.3.i.i = fadd float %sub15.i.3.i.i, %sub15.i.3.i.i
  %cmp13.i.i.i.1 = fcmp oge float %add.i.3.i.i, %astype12.i.i.i
  %17 = select i1 %cmp13.i.i.i.1, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.i.i.1 = fsub float %add.i.3.i.i, %17
  %add.i.i.i.1 = fadd float %sub15.i.i.i.1, %sub15.i.i.i.1
  %cmp13.i.1.i.i.1 = fcmp oge float %add.i.i.i.1, %astype12.i.i.i
  %conv.i.1.i.i.1 = zext i1 %cmp13.i.1.i.i.1 to i32
  %18 = shl i32 %or14.i.3.i.i, 2
  %19 = select i1 %cmp13.i.i.i.1, i32 2, i32 0
  %shl.i.1.i.i.1 = or i32 %18, %19
  %or14.i.1.i.i.1 = or i32 %shl.i.1.i.i.1, %conv.i.1.i.i.1
  %20 = select i1 %cmp13.i.1.i.i.1, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.1.i.i.1 = fsub float %add.i.i.i.1, %20
  %add.i.1.i.i.1 = fadd float %sub15.i.1.i.i.1, %sub15.i.1.i.i.1
  %cmp13.i.2.i.i.1 = fcmp oge float %add.i.1.i.i.1, %astype12.i.i.i
  %21 = select i1 %cmp13.i.2.i.i.1, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.2.i.i.1 = fsub float %add.i.1.i.i.1, %21
  %add.i.2.i.i.1 = fadd float %sub15.i.2.i.i.1, %sub15.i.2.i.i.1
  %cmp13.i.3.i.i.1 = fcmp oge float %add.i.2.i.i.1, %astype12.i.i.i
  %conv.i.3.i.i.1 = zext i1 %cmp13.i.3.i.i.1 to i32
  %22 = shl i32 %or14.i.1.i.i.1, 2
  %23 = select i1 %cmp13.i.2.i.i.1, i32 2, i32 0
  %shl.i.3.i.i.1 = or i32 %23, %22
  %or14.i.3.i.i.1 = or i32 %shl.i.3.i.i.1, %conv.i.3.i.i.1
  %24 = select i1 %cmp13.i.3.i.i.1, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.3.i.i.1 = fsub float %add.i.2.i.i.1, %24
  %add.i.3.i.i.1 = fadd float %sub15.i.3.i.i.1, %sub15.i.3.i.i.1
  %niter.next.1 = add i32 %niter, 2
  %niter.ncmp.1 = icmp eq i32 %niter.next.1, %unroll_iter
  br i1 %niter.ncmp.1, label %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa.loopexit, label %while.body.i.i.i

while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa.loopexit: ; preds = %while.body.i.i.i
  %phi.bo = shl i32 %or14.i.3.i.i.1, 2
  br label %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa

while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa: ; preds = %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa.loopexit, %while.body.i.preheader.new.i.i
  %or14.i.3.i.i.lcssa.ph = phi i32 [ undef, %while.body.i.preheader.new.i.i ], [ %or14.i.3.i.i.1, %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa.loopexit ]
  %add.i.3.i.i.lcssa.ph = phi float [ undef, %while.body.i.preheader.new.i.i ], [ %add.i.3.i.i.1, %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa.loopexit ]
  %xr.0205.i.i.i.unr = phi float [ %astype9.i.i.i, %while.body.i.preheader.new.i.i ], [ %add.i.3.i.i.1, %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa.loopexit ]
  %q.0203.i.i.i.unr = phi i32 [ 0, %while.body.i.preheader.new.i.i ], [ %phi.bo, %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa.loopexit ]
  %lcmp.mod.not = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod.not, label %while.end.loopexit.i.unr-lcssa.i.i, label %while.body.i.i.i.epil

while.body.i.i.i.epil:                            ; preds = %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa
  %cmp13.i.i.i.epil = fcmp oge float %xr.0205.i.i.i.unr, %astype12.i.i.i
  %25 = select i1 %cmp13.i.i.i.epil, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.i.i.epil = fsub float %xr.0205.i.i.i.unr, %25
  %add.i.i.i.epil = fadd float %sub15.i.i.i.epil, %sub15.i.i.i.epil
  %cmp13.i.1.i.i.epil = fcmp oge float %add.i.i.i.epil, %astype12.i.i.i
  %conv.i.1.i.i.epil = zext i1 %cmp13.i.1.i.i.epil to i32
  %26 = select i1 %cmp13.i.i.i.epil, i32 2, i32 0
  %shl.i.1.i.i.epil = or i32 %q.0203.i.i.i.unr, %26
  %or14.i.1.i.i.epil = or i32 %shl.i.1.i.i.epil, %conv.i.1.i.i.epil
  %27 = select i1 %cmp13.i.1.i.i.epil, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.1.i.i.epil = fsub float %add.i.i.i.epil, %27
  %add.i.1.i.i.epil = fadd float %sub15.i.1.i.i.epil, %sub15.i.1.i.i.epil
  %cmp13.i.2.i.i.epil = fcmp oge float %add.i.1.i.i.epil, %astype12.i.i.i
  %28 = select i1 %cmp13.i.2.i.i.epil, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.2.i.i.epil = fsub float %add.i.1.i.i.epil, %28
  %add.i.2.i.i.epil = fadd float %sub15.i.2.i.i.epil, %sub15.i.2.i.i.epil
  %cmp13.i.3.i.i.epil = fcmp oge float %add.i.2.i.i.epil, %astype12.i.i.i
  %conv.i.3.i.i.epil = zext i1 %cmp13.i.3.i.i.epil to i32
  %29 = shl i32 %or14.i.1.i.i.epil, 2
  %30 = select i1 %cmp13.i.2.i.i.epil, i32 2, i32 0
  %shl.i.3.i.i.epil = or i32 %30, %29
  %or14.i.3.i.i.epil = or i32 %shl.i.3.i.i.epil, %conv.i.3.i.i.epil
  %31 = select i1 %cmp13.i.3.i.i.epil, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.3.i.i.epil = fsub float %add.i.2.i.i.epil, %31
  %add.i.3.i.i.epil = fadd float %sub15.i.3.i.i.epil, %sub15.i.3.i.i.epil
  br label %while.end.loopexit.i.unr-lcssa.i.i

while.end.loopexit.i.unr-lcssa.i.i:               ; preds = %while.body.i.i.i.epil, %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa, %while.body.i.preheader.i.i
  %or14.i.lcssa.ph.i.i = phi i32 [ undef, %while.body.i.preheader.i.i ], [ %or14.i.3.i.i.lcssa.ph, %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa ], [ %or14.i.3.i.i.epil, %while.body.i.i.i.epil ]
  %add.i.lcssa.ph.i.i = phi float [ undef, %while.body.i.preheader.i.i ], [ %add.i.3.i.i.lcssa.ph, %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa ], [ %add.i.3.i.i.epil, %while.body.i.i.i.epil ]
  %xr.0205.i.unr.i.i = phi float [ %astype9.i.i.i, %while.body.i.preheader.i.i ], [ %add.i.3.i.i.lcssa.ph, %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa ], [ %add.i.3.i.i.epil, %while.body.i.i.i.epil ]
  %q.0203.i.unr.i.i = phi i32 [ 0, %while.body.i.preheader.i.i ], [ %or14.i.3.i.i.lcssa.ph, %while.end.loopexit.i.unr-lcssa.i.i.loopexit.unr-lcssa ], [ %or14.i.3.i.i.epil, %while.body.i.i.i.epil ]
  %lcmp.mod.not.i.i = icmp eq i32 %xtraiter.i.i, 0
  br i1 %lcmp.mod.not.i.i, label %while.end.loopexit.i.i.i, label %while.body.i.epil.i.i

while.body.i.epil.i.i:                            ; preds = %while.end.loopexit.i.unr-lcssa.i.i, %while.body.i.epil.i.i
  %xr.0205.i.epil.i.i = phi float [ %add.i.epil.i.i, %while.body.i.epil.i.i ], [ %xr.0205.i.unr.i.i, %while.end.loopexit.i.unr-lcssa.i.i ]
  %q.0203.i.epil.i.i = phi i32 [ %or14.i.epil.i.i, %while.body.i.epil.i.i ], [ %q.0203.i.unr.i.i, %while.end.loopexit.i.unr-lcssa.i.i ]
  %epil.iter.i.i = phi i32 [ %epil.iter.next.i.i, %while.body.i.epil.i.i ], [ 0, %while.end.loopexit.i.unr-lcssa.i.i ]
  %cmp13.i.epil.i.i = fcmp oge float %xr.0205.i.epil.i.i, %astype12.i.i.i
  %conv.i.epil.i.i = zext i1 %cmp13.i.epil.i.i to i32
  %shl.i.epil.i.i = shl i32 %q.0203.i.epil.i.i, 1
  %or14.i.epil.i.i = or i32 %shl.i.epil.i.i, %conv.i.epil.i.i
  %32 = select i1 %cmp13.i.epil.i.i, float %astype12.i.i.i, float 0.000000e+00
  %sub15.i.epil.i.i = fsub float %xr.0205.i.epil.i.i, %32
  %add.i.epil.i.i = fadd float %sub15.i.epil.i.i, %sub15.i.epil.i.i
  %epil.iter.next.i.i = add nuw nsw i32 %epil.iter.i.i, 1
  %epil.iter.cmp.not.i.i = icmp eq i32 %epil.iter.next.i.i, %xtraiter.i.i
  br i1 %epil.iter.cmp.not.i.i, label %while.end.loopexit.i.i.i, label %while.body.i.epil.i.i

while.end.loopexit.i.i.i:                         ; preds = %while.body.i.epil.i.i, %while.end.loopexit.i.unr-lcssa.i.i
  %or14.i.lcssa.i.i = phi i32 [ %or14.i.lcssa.ph.i.i, %while.end.loopexit.i.unr-lcssa.i.i ], [ %or14.i.epil.i.i, %while.body.i.epil.i.i ]
  %add.i.lcssa.i.i = phi float [ %add.i.lcssa.ph.i.i, %while.end.loopexit.i.unr-lcssa.i.i ], [ %add.i.epil.i.i, %while.body.i.epil.i.i ]
  %phi.bo.i.i.i = shl i32 %or14.i.lcssa.i.i, 1
  br label %_Z18__spirv_ocl_remquoDhDhPi.exit

_Z18__spirv_ocl_remquoDhDhPi.exit:                ; preds = %entry, %while.end.loopexit.i.i.i
  %q.0.lcssa.i.i.i = phi i32 [ 0, %entry ], [ %phi.bo.i.i.i, %while.end.loopexit.i.i.i ]
  %xr.0.lcssa.i.i.i = phi float [ %astype9.i.i.i, %entry ], [ %add.i.lcssa.i.i, %while.end.loopexit.i.i.i ]
  %astype5.i.i.i = bitcast i32 %and4.i.i.i to float
  %astype2.i.i.i = bitcast i32 %and.i.i.i to float
  %cmp16.i.i.i = fcmp ogt float %xr.0.lcssa.i.i.i, %astype12.i.i.i
  %conv17.i.i.i = zext i1 %cmp16.i.i.i to i32
  %or19.i.i.i = or i32 %q.0.lcssa.i.i.i, %conv17.i.i.i
  %33 = select i1 %cmp16.i.i.i, float %astype12.i.i.i, float 0.000000e+00
  %sub25.i.i.i = fsub float %xr.0.lcssa.i.i.i, %33
  %cmp26.i.i.i = icmp ult i32 %0, %1
  %34 = select i1 %cmp26.i.i.i, i32 0, i32 %or19.i.i.i
  %35 = select i1 %cmp26.i.i.i, float %astype2.i.i.i, float %sub25.i.i.i
  %36 = select i1 %cmp26.i.i.i, float %astype5.i.i.i, float %astype12.i.i.i
  %mul.i.i.i = fmul float %35, 2.000000e+00
  %cmp43.i.i.i = fcmp olt float %36, %mul.i.i.i
  %conv44.i.i.i = zext i1 %cmp43.i.i.i to i32
  %cmp46.i.i.i = fcmp oeq float %36, %mul.i.i.i
  %conv47.i.i.i = zext i1 %cmp46.i.i.i to i32
  %and51.i.i.i = and i32 %34, %conv47.i.i.i
  %or52.i.i.i = or i32 %and51.i.i.i, %conv44.i.i.i
  %tobool53.not.i.i.i = icmp eq i32 %or52.i.i.i, 0
  %37 = select i1 %tobool53.not.i.i.i, float 0.000000e+00, float %36
  %sub58.i.i.i = fsub float %35, %37
  %add59.i.i.i = add i32 %or52.i.i.i, %34
  %shl60.i.i.i = and i32 %astype3.i.i.i, 2139095040
  %astype61.i.i.i = bitcast i32 %shl60.i.i.i to float
  %38 = select i1 %cmp26.i.i.i, float 1.000000e+00, float %astype61.i.i.i
  %mul67.i.i.i = fmul float %38, %sub58.i.i.i
  %cmp68.i.i.i = icmp eq i32 %and4.i.i.i.i, %and4.i200.i.i.i
  %and71.i.i.i = and i32 %add59.i.i.i, 127
  %cmp73.i.i.i = icmp eq i32 %and.i.i.i, %and4.i.i.i
  %mul72.i.i.i = select i1 %cmp73.i.i.i, i32 1, i32 %and71.i.i.i
  %39 = sub nsw i32 0, %mul72.i.i.i
  %40 = select i1 %cmp68.i.i.i, i32 %mul72.i.i.i, i32 %39
  %41 = bitcast float %mul67.i.i.i to i32
  %astype85.i.i.i = select i1 %cmp73.i.i.i, i32 0, i32 %41
  %xor86.i.i.i = xor i32 %astype85.i.i.i, %and4.i.i.i.i
  %astype87.i.i.i = bitcast i32 %xor86.i.i.i to float
  %42 = icmp ugt i32 %and.i.i.i, 2139095039
  %43 = add nsw i32 %and4.i.i.i, -2139095041
  %44 = icmp ult i32 %43, -2139095040
  %45 = or i1 %42, %44
  %46 = select i1 %45, i32 0, i32 %40
  %47 = fptrunc float %astype87.i.i.i to half
  %conv2.i.i = select i1 %45, half 0xH7E00, half %47
  store i32 %46, i32* %args_2, align 4
  ret half %conv2.i.i
}
; end INTEL_FEATURE_SW_ADVANCED
