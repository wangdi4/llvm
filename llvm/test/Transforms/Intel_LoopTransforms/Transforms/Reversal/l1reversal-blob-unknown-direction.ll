; l1reversal-posorneg-blobindex.ll
; (1-level loop, memory access on a blob with unknown direction (don't know if the BlobIndex is positive or negative)
; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -print-after=hir-loop-reversal < %s 2>&1 | FileCheck %s
;  
; Loop is reverisble
;
; Reasons:
; - Applicalbe: YES (IVBlobIndex's sign is known at compile time)  
; - Profitable: YES
; - Legal:      YES
; Decision:     Reverse the loop!

; *** Source Code ***
;
;//Reversal Testcase for the sign direction for IV's BlobCoeff.
;long foo(long *A, long n) {
;  long i;
;
;  for (i = 0; i < n; i++) {
;    A[100 - 2 * n * i] = i;
;    // expect n is positive
;  }
;
;  return A[5];
;}
;
; CHECK: IR Dump Before HIR Loop Reversal
;
; CHECK:      BEGIN REGION { }
; CHECK:           + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK-NEXT:      |   (%A)[-2 * %n * i1 + 100] = i1;
; CHECK-NEXT:      + END LOOP
; CHECK-NEXT: END REGION
; 
; CHECK: IR Dump After HIR Loop Reversal
;
; CHECK:      BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK-NEXT:      |   (%A)[2 * %n * i1 + 2 * %n + -2 * (%n * %n) + 100] = -1 * i1 + %n + -1;
; CHECK-NEXT:      + END LOOP
; CHECK-NEXT:  END REGION
;
; 
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i64 @foo(i64* nocapture %A, i64 %n) local_unnamed_addr #0 {
entry:
  %cmp9 = icmp sgt i64 %n, 0
  br i1 %cmp9, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %mul = shl i64 %n, 1
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i.010 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %mul1 = mul nsw i64 %mul, %i.010
  %sub = sub nsw i64 100, %mul1
  %arrayidx = getelementptr inbounds i64, i64* %A, i64 %sub
  store i64 %i.010, i64* %arrayidx, align 8, !tbaa !1
  %inc = add nuw nsw i64 %i.010, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %arrayidx2 = getelementptr inbounds i64, i64* %A, i64 5
  %0 = load i64, i64* %arrayidx2, align 8, !tbaa !1
  ret i64 %0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17968)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
