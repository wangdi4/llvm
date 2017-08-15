; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: stores only (2 continue stores, with negative IVCoeff)
;
; [REASONS]  
; - Profitable: YES
; - Legal:      YES
;
; *** Source Code ***
;int A[1000];
;int B[1000];
;int foo(void) {
;  int i;
;  for (int i = 0; i <= 100; ++i) {
;    B[900 - i] = i + 1;
;    B[901 - i] = A[i] - 1;
;  }
;  return A[0] + B[1] + 1;
;}
;
;
; MemRef Group to transform: {B[900-i1], B[901-i1]}
;
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:         |   (@B)[0][-1 * i1 + 900] = i1 + 1;
; CHECK:         |   %4 = (@A)[0][i1];
; CHECK:         |   (@B)[0][-1 * i1 + 901] = %4 + -1;
; CHECK:         + END LOOP
; CHECK:   END REGION
;
;  
; =====================================================
; testcase not ready yet!
; Note: ScalarRepl transformation result missing!!
; =====================================================
;
;
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
; no explicit loads needed: store-only group
;
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; replace store with scalar1:
; CHECK:        |   %scalarepl1 = i1 + 1;
; CHECK:        |   %4 = (@A)[0][i1];
; replace store with scalar:
; CHECK:        |   %scalarepl = %4 + -1;
; issue a live store:
; CHECK:        |   (@B)[0][-1 * i1 + 901] = %scalarepl;
; CHECK:        + END LOOP
; fix any store(s) live-out loop
; CHECK:           (@B)[0][800] = %scalarepl1;
; CHECK:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [1002 x i32] zeroinitializer, align 16
@A = common global [1002 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load  i32, i32* getelementptr inbounds ([1002 x i32], [1002 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %1 = load  i32, i32* getelementptr inbounds ([1002 x i32], [1002 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add8 = add i32 %0, 1
  %add9 = add i32 %add8, %1
  ret i32 %add9

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %2 = sub nuw nsw i64 900, %indvars.iv
  %arrayidx = getelementptr inbounds [1002 x i32], [1002 x i32]* @B, i64 0, i64 %2
  %3 = trunc i64 %indvars.iv.next to i32
  store  i32 %3, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx3 = getelementptr inbounds [1002 x i32], [1002 x i32]* @A, i64 0, i64 %indvars.iv
  %4 = load  i32, i32* %arrayidx3, align 4, !tbaa !1
  %sub4 = add nsw i32 %4, -1
  %5 = sub nuw nsw i64 901, %indvars.iv
  %arrayidx7 = getelementptr inbounds [1002 x i32], [1002 x i32]* @B, i64 0, i64 %5
  store  i32 %sub4, i32* %arrayidx7, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20641) (llvm/branches/loopopt 20654)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
