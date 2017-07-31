; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: loads only (2 continue loads, with negative IVCoeff)
;
; [REASONS]
; - Applicable: YES
; - Legal:      YES (treat valid groups with neg-IVCoeff specially) 
; - Profitable: YES
;
; *** Source Code ***
;int A[1000];
;int B[1000];
;int foo(void) {
;  int i;
;  for (i = 0; i <= 100; ++i) {
;    B[i] = A[1000 - i] + A[999 - i];
;  }
;  return A[0] + B[0] + 1;
;}
;
; MemRefGroup: { A[999-i],  A[1000-i]}
;                           MaxLD
; GapTracker:  { R          R        }

; 
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:         |   %1 = (@A)[0][-1 * i1 + 1000];
; CHECK:         |   %3 = (@A)[0][-1 * i1 + 999];
; CHECK:         |   (@B)[0][i1] = %1 + %3;
; CHECK:         + END LOOP
; CHECK:   END REGION
;
;  
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           %scalarepl = (@A)[0][1000];
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   %1 = %scalarepl;
; CHECK:        |   %scalarepl1 = (@A)[0][-1 * i1 + 999];
; CHECK:        |   %3 = %scalarepl1;
; CHECK:        |   (@B)[0][i1] = %1 + %3;
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        + END LOOP
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

@A = common global [1002 x i32] zeroinitializer, align 16
@B = common global [1002 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = sub nuw nsw i64 1000, %indvars.iv
  %arrayidx = getelementptr inbounds [1002 x i32], [1002 x i32]* @A, i64 0, i64 %0
  %1 = load  i32, i32* %arrayidx, align 4, !tbaa !1
  %2 = sub nuw nsw i64 999, %indvars.iv
  %arrayidx3 = getelementptr inbounds [1002 x i32], [1002 x i32]* @A, i64 0, i64 %2
  %3 = load  i32, i32* %arrayidx3, align 4, !tbaa !1
  %add = add nsw i32 %3, %1
  %arrayidx5 = getelementptr inbounds [1002 x i32], [1002 x i32]* @B, i64 0, i64 %indvars.iv
  store  i32 %add, i32* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %4 = load  i32, i32* getelementptr inbounds ([1002 x i32], [1002 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %5 = load  i32, i32* getelementptr inbounds ([1002 x i32], [1002 x i32]* @B, i64 0, i64 0), align 16, !tbaa !1
  %add6 = add i32 %4, 1
  %add7 = add i32 %add6, %5
  ret i32 %add7
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
