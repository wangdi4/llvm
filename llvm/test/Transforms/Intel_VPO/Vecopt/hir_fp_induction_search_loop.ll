; Check that VPlan SearchLoop idiom recognizer doesn't choke for a
; multi-exit loop containing FP IV idiom. Such loops are unsafe to
; vectorize in current state.

; HIR before VPlan -
; BEGIN REGION { }
;       %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;       + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <vectorize>
;       |   %fpiv.phi = %fpiv.phi  +  1.000000e+00;
;       |   if ((%a)[i1] !=u %fpiv.phi)
;       |   {
;       |      %indvars.iv.out = i1;
;       |      goto cleanup.loopexit.split.loop.exit;
;       |   }
;       + END LOOP
;
;       @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; END REGION

; RUN: opt < %s -S -passes=hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec -debug-only=vplan-idioms 2>&1 | FileCheck %s

; CHECK: Search loop idiom was not recognized.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @_Z3fooiPKaPaa(i32 %n, ptr nocapture readonly %a, float %start) local_unnamed_addr #0 {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %fpiv.phi = phi float [ %start, %for.body.preheader ], [ %fpiv.next, %for.inc ]
  %fpiv.next = fadd reassoc float %fpiv.phi, 1.000000e+00
  %arrayidx = getelementptr inbounds float, ptr %a, i64 %indvars.iv
  %1 = load float, ptr %arrayidx, align 1
  %cmp2 = fcmp oeq float %1, %fpiv.next
  br i1 %cmp2, label %for.inc, label %cleanup.loopexit.split.loop.exit

for.inc:                                          ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %cleanup.loopexit, !llvm.loop !5

cleanup.loopexit.split.loop.exit:                 ; preds = %for.body
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %for.body ]
  %2 = trunc i64 %indvars.iv.lcssa to i32
  br label %cleanup

cleanup.loopexit:                                 ; preds = %for.inc
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %cleanup.loopexit.split.loop.exit, %entry
  %index.0 = phi i32 [ -1, %entry ], [ %2, %cleanup.loopexit.split.loop.exit ], [ -1, %cleanup.loopexit ]
  ret i32 %index.0
}

!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.vectorize.ignore_profitability"}
!7 = !{!"llvm.loop.vectorize.enable", i1 true}
