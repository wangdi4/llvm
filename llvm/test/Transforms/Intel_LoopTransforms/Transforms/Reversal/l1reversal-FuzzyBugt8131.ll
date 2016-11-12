; Fix for a fuzzy-test bug on HIR Loop Reversal: original testcase provided by John. 
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-reversal -print-before=hir-loop-reversal -print-after=hir-loop-reversal < %s 2>&1 | FileCheck %s
; 
; [REASONS]
; - Applicable: YES
; - Profitable: YES
; - Legal:      NO
; - Decision:   Not reversed
;
; Issue with the Fuzzy-t8131
; - with -hir-temp-cleanup, a load can now appear on a HLIf's condition.
;   Existing HIRLoopReversal's legal test will recursively inspect HLInst* type, 
;   which by default will inogre HLIf's condition and any load that may reside.
; - Fix:
;   Change's HIRLoopReversal's legal test to inspect on HLDDNode*, so that it can 
;   catch all cases in HLInst*, HLIf*, HLSwitch*.
; 
; *** Source Code ***
;int U[100];
;int V[100];
;int foo(void) {
;  int i = 0;
;  int tmp = 0;
;
;  for (i = 0; i <= 64; ++i) {
;    tmp = V[67 - i];
;    if (U[67 - i] < tmp) {
;      U[68 - i] = tmp;
;    }
;  }
;
;  return U[0] + V[1] + 1;
;}
;
;
;
; CHECK: IR Dump Before HIR Loop Reversal
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 64, 1   <DO_LOOP>
; CHECK:        |   %1 = (@V)[0][-1 * i1 + 67];
; CHECK:        |   if ((@U)[0][-1 * i1 + 67] < %1)
; CHECK:        |   {
; CHECK:        |      (@U)[0][-1 * i1 + 68] = %1;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;  
;
; *** --- ***
; 
; 
; CHECK: IR Dump After HIR Loop Reversal
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 64, 1   <DO_LOOP>
; CHECK:        |   %1 = (@V)[0][-1 * i1 + 67];
; CHECK:        |   if ((@U)[0][-1 * i1 + 67] < %1)
; CHECK:        |   {
; CHECK:        |      (@U)[0][-1 * i1 + 68] = %1;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@V = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@U = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %0 = sub nuw nsw i64 67, %indvars.iv
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @V, i64 0, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @U, i64 0, i64 %0
  %2 = load i32, i32* %arrayidx3, align 4, !tbaa !1
  %cmp4 = icmp slt i32 %2, %1
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %3 = sub nuw nsw i64 68, %indvars.iv
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* @U, i64 0, i64 %3
  store i32 %1, i32* %arrayidx7, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 65
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  %4 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @U, i64 0, i64 0), align 16, !tbaa !1
  %5 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @V, i64 0, i64 1), align 4, !tbaa !1
  %add = add i32 %4, 1
  %add8 = add i32 %add, %5
  ret i32 %add8
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20338)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
