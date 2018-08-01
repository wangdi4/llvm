; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -hir-cost-model-throttling=0 -S < %s 2>&1 | FileCheck %s

; Verify that we are successfully able to generate vector code for the function pointer setting loop.
 
; CHECK: + DO i1 = 0, 79, 2   <DO_LOOP>
; CHECK: |   (<2 x %struct.TypHeader* (%struct.TypHeader*)*>*)(@EvTab)[0][i1] = @CantEval;
; CHECK: + END LOOP

; CHECK: bitcast %struct.TypHeader* (%struct.TypHeader*)** %arrayIdx to <2 x %struct.TypHeader* (%struct.TypHeader*)*>*
; CHECK: store <2 x %struct.TypHeader* (%struct.TypHeader*)*> <%struct.TypHeader* (%struct.TypHeader*)* @CantEval, %struct.TypHeader* (%struct.TypHeader*)* @CantEval>


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


%struct.TypHeader = type { i64, %struct.TypHeader**, [3 x i8], i8 }

@EvTab = external hidden unnamed_addr global [81 x %struct.TypHeader* (%struct.TypHeader*)*], align 16

declare hidden %struct.TypHeader* @CantEval(%struct.TypHeader*)

define void @InitEval() {
  br label %loop

loop:                                      ; preds = %loop, %0
  %iv = phi i64 [ 0, %0 ], [ %inc, %loop ]
  %gep = getelementptr [81 x %struct.TypHeader* (%struct.TypHeader*)*], [81 x %struct.TypHeader* (%struct.TypHeader*)*]* @EvTab, i64 0, i64 %iv
  store %struct.TypHeader* (%struct.TypHeader*)* @CantEval, %struct.TypHeader* (%struct.TypHeader*)** %gep, align 8, !tbaa !1
  %inc = add nuw nsw i64 %iv, 1
  %cmp = icmp eq i64 %inc, 81
  br i1 %cmp, label %exit, label %loop

exit:                                      ; preds = %loop
  ret void
}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"unspecified pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

