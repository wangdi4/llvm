; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework | FileCheck %s

; Verify that this case compiles successfully. We can have 'flowthrough' region liveouts which do not have any definition inside the region.
; In this loop, %t44 which is liveout of the loop is traced back to %t41 which is coming from outside the region.

; CHECK: LiveIns:
; CHECK-SAME: t41

; CHECK: LiveOuts:
; CHECK-SAME: %t44

; CHECK:      + DO i1 = 0
; CHECK-NEXT: |   %i.0.i437771.out = %i.0.i437771;
; CHECK-NEXT: |   %t46 = (%t41)[i1 + zext.i32.i64((2 + trunc.i64.i32(%indvars.iv)))];
; CHECK-NEXT: |   %t47 = (%t41)[%i.0.i437771];
; CHECK-NEXT: |   if (%t46 != %t47)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      goto for.body.i447.land.lhs.true.i.i.i428_crit_edge;
; CHECK-NEXT: |   }
; CHECK-NEXT: |   %i.0.i437771 = i1 + trunc.i64.i32(%indvars.iv) + 2;
; CHECK-NEXT: + END LOOP


define void @foo(i32 %i.0.i437768, i64 %indvars.iv, i32* %in, i32 %t36) {
entry:
  br label %for.body.i447.pre

for.body.i447.pre:
  %t41 = phi i32* [ %in, %entry ]
  br label %for.body.i447

for.body.i447:                                    ; preds = %for.cond.i439, %for.body.i447.pre
  %i.0.i437771 = phi i32 [ %i.0.i437768, %for.body.i447.pre ], [ %i.0.i437, %for.cond.i439 ]
  %indvars.iv647770 = phi i64 [ %indvars.iv, %for.body.i447.pre ], [ %indvars.iv647, %for.cond.i439 ]
  %add.i.i441 = add nuw i64 %indvars.iv647770, 2
  %idxprom.i.i442 = and i64 %add.i.i441, 4294967295
  %arrayidx.i.i443 = getelementptr inbounds i32, i32* %t41, i64 %idxprom.i.i442
  %t46 = load i32, i32* %arrayidx.i.i443, align 4
  %idxprom3.i.i444 = zext i32 %i.0.i437771 to i64
  %arrayidx4.i.i445 = getelementptr inbounds i32, i32* %t41, i64 %idxprom3.i.i444
  %t47 = load i32, i32* %arrayidx4.i.i445, align 4
  %cmp3.i446 = icmp eq i32 %t46, %t47
  %indvars.iv.next648 = add nuw nsw i64 %indvars.iv647770, 1
  br i1 %cmp3.i446, label %for.cond.i439, label %for.body.i447.land.lhs.true.i.i.i428_crit_edge

for.cond.i439:                                    ; preds = %for.body.i447
  %t44 = phi i32* [ %t41, %for.body.i447 ]
  %indvars.iv647 = phi i64 [ %indvars.iv.next648, %for.body.i447 ]
  %t45 = trunc i64 %indvars.iv647 to i32
  %i.0.i437 = add i32 %t45, 1
  %cmp.i438 = icmp ult i32 %i.0.i437, %t36
  br i1 %cmp.i438, label %for.body.i447, label %for.cond.i439.land.lhs.true.i.i.i428_crit_edge

for.body.i447.land.lhs.true.i.i.i428_crit_edge:   ; preds = %for.body.i447
  %split772 = phi i32* [ %t41, %for.body.i447 ]
  %split773 = phi i32 [ %i.0.i437771, %for.body.i447 ]
  ret void

for.cond.i439.land.lhs.true.i.i.i428_crit_edge:   ; preds = %for.cond.i439
  %split774 = phi i32* [ %t44, %for.cond.i439 ]
  ret void
}
