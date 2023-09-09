; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s

; Verify that the load temp %i10 which is liveout of the loop is not
; substituted into its use (%vGatePrev.074 = %i10) as we first encounter a goto
; through which %i10 is liveout.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %logIDSPrev.073.out = %logIDSPrev.073;
; CHECK: |   %vGatePrev.074.out = %vGatePrev.074;
; CHECK: |   %i8 = (%iDS)[0][%indvars.iv84][i1 + 1];
; CHECK: |   %i9 = @llvm.log10.f64(%i8);
; CHECK: |   %i10 = (%__const.main.vGate)[0][i1 + 1];
; CHECK: |   if (%i9 >= 0xC0220D493337BB0B)
; CHECK: |   {
; CHECK: |      goto for.inc24;
; CHECK: |   }
; CHECK: |   %vGatePrev.074 = %i10;
; CHECK: |   %logIDSPrev.073 = %i9;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: + DO i1 = 0, 1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %logIDSPrev.073.out = %logIDSPrev.073;
; CHECK: |   %vGatePrev.074.out = %vGatePrev.074;
; CHECK: |   %i9 = @llvm.log10.f64((%iDS)[0][%indvars.iv84][i1 + 1]);
; CHECK: |   %i10 = (%__const.main.vGate)[0][i1 + 1];
; CHECK: |   if (%i9 >= 0xC0220D493337BB0B)
; CHECK: |   {
; CHECK: |      goto for.inc24;
; CHECK: |   }
; CHECK: |   %vGatePrev.074 = %i10;
; CHECK: |   %logIDSPrev.073 = %i9;
; CHECK: + END LOOP

define void @foo(double %i7, i64 %indvars.iv84, ptr %iDS, ptr %__const.main.vGate) {
entry:
  br label %for.body6

for.body6:                                        ; preds = %for.cond3, %for.body
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %for.cond3 ]
  %vGatePrev.074 = phi double [ 1.000000e-01, %entry ], [ %i10, %for.cond3 ]
  %logIDSPrev.073 = phi double [ %i7, %entry ], [ %i9, %for.cond3 ]
  %arrayidx10 = getelementptr inbounds [2 x [3 x double]], ptr %iDS, i64 0, i64 %indvars.iv84, i64 %indvars.iv
  %i8 = load double, ptr %arrayidx10, align 8
  %i9 = tail call fast double @llvm.log10.f64(double %i8)
  %arrayidx12 = getelementptr inbounds [3 x double], ptr %__const.main.vGate, i64 0, i64 %indvars.iv
  %i10 = load double, ptr %arrayidx12, align 8
  %cmp13 = fcmp fast ult double %i9, 0xC0220D493337BB0B
  br i1 %cmp13, label %for.cond3, label %for.inc24

for.cond3:                                        ; preds = %for.body6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond.not, label %cleanup26, label %for.body6

for.inc24:                                        ; preds = %for.body6
  %vGatePrev.074.lcssa = phi double [ %vGatePrev.074, %for.body6 ]
  %logIDSPrev.073.lcssa = phi double [ %logIDSPrev.073, %for.body6 ]
  %.lcssa101 = phi double [ %i9, %for.body6 ]
  %.lcssa = phi double [ %i10, %for.body6 ]
  ret void

cleanup26:                                        ; preds = %for.cond3
  %indvars.iv84.lcssa105 = phi i64 [ %indvars.iv84, %for.cond3 ]
  ret void
}

declare double @llvm.log10.f64(double)
