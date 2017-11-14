; Verify that we form the loop at O3 but not at O2 due to different if-nesting thresholds.

; RUN: opt < %s -hir-ssa-deconstruction -xmain-opt-level=2 | opt -analyze -hir-parser -xmain-opt-level=2 | FileCheck %s --check-prefix=O2

; O2-NOT: DO i1

; RUN: opt < %s -hir-ssa-deconstruction -xmain-opt-level=3 | opt -analyze -hir-parser -xmain-opt-level=3 | FileCheck %s --check-prefix=O3

; O3: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; O3: |   %t.023.out1 = %t.023;
; O3: |   %0 = (%A)[i1];
; O3: |   if (%0 != 0)
; O3: |   {
; O3: |      %1 = (%B)[i1];
; O3: |      if (%1 != 0)
; O3: |      {
; O3: |         %2 = (%C)[i1];
; O3: |         if (%2 != 0)
; O3: |         {
; O3: |            %3 = (%D)[i1];
; O3: |            %t.023 = (%3 == 0) ? %t.023 : %t.023.out1 + 3;
; O3: |         }
; O3: |      }
; O3: |   }
; O3: |   %t.023.out = %t.023;
; O3: + END LOOP


; ModuleID = 'if-nest.c'
source_filename = "if-nest.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo(i32* nocapture readonly %A, i32* nocapture readonly %B, i32* nocapture readonly %C, i32* nocapture readonly %D, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i32 %n, 0
  br i1 %cmp21, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %t.023 = phi i32 [ %t.1, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %cmp1 = icmp eq i32 %0, 0
  br i1 %cmp1, label %for.inc, label %land.lhs.true

land.lhs.true:                                    ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4, !tbaa !1
  %cmp4 = icmp eq i32 %1, 0
  br i1 %cmp4, label %for.inc, label %land.lhs.true5

land.lhs.true5:                                   ; preds = %land.lhs.true
  %arrayidx7 = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx7, align 4, !tbaa !1
  %cmp8 = icmp eq i32 %2, 0
  br i1 %cmp8, label %for.inc, label %land.lhs.true9

land.lhs.true9:                                   ; preds = %land.lhs.true5
  %arrayidx11 = getelementptr inbounds i32, i32* %D, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx11, align 4, !tbaa !1
  %cmp12 = icmp eq i32 %3, 0
  %add = add nsw i32 %t.023, 3
  %t.0.add = select i1 %cmp12, i32 %t.023, i32 %add
  br label %for.inc

for.inc:                                          ; preds = %land.lhs.true9, %land.lhs.true5, %land.lhs.true, %for.body
  %t.1 = phi i32 [ %t.023, %land.lhs.true5 ], [ %t.023, %land.lhs.true ], [ %t.023, %for.body ], [ %t.0.add, %land.lhs.true9 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  %t.1.lcssa = phi i32 [ %t.1, %for.inc ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %t.1.lcssa, %for.end.loopexit ]
  ret i32 %t.0.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
