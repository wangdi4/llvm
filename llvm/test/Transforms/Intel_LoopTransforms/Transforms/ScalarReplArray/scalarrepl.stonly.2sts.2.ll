; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: loads only (2 continue loads)
;
; [REASONS]
; - Applicable: YES
; - Profitable: YES
; - Legal:      YES
;
; *** Source Code ***
;#define N 1000
;int A[N];
;int B[N];
;int C[N];
;//volatile int D[N];
;
;int foo(void) {
;  int i;
;  for (i = 1; i <= 100; ++i) {
;    B[i] = A[i] + 1;
;    A[i + 1] = i;
;    A[i + 2] = i + 1;
;  }
;  return A[0] + B[1] + C[2] + 1;
;}
;
;
; MemRefGroup:  
;            { r   ,   w   ,  w      }
;               0      1      2     
;
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   %0 = (@A)[0][i1];
; CHECK:        |   (@B)[0][i1] = %0 + 1;
; CHECK:        |   (@A)[0][i1 + 1] = i1;
; CHECK:        |   (@A)[0][i1 + 2] = i1 + 1;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;  
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           %scalarepl = (@A)[0][0];
; CHECK:           %scalarepl1 = (@A)[0][1];
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   %0 = %scalarepl;
; CHECK:        |   (@B)[0][i1] = %0 + 1;
; CHECK:        |   %scalarepl1 = i1;
; CHECK:        |   (@A)[0][i1 + 1] = %scalarepl1;
; CHECK:        |   %scalarepl2 = i1 + 1;
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        |   %scalarepl1 = %scalarepl2;
; CHECK:        + END LOOP
; CHECK:           (@A)[0][102] = %scalarepl1;
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

@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, 1
  %arrayidx2 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv.next
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx5, align 4, !tbaa !1
  %2 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx9 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %2
  %3 = trunc i64 %indvars.iv.next to i32
  store i32 %3, i32* %arrayidx9, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %4 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %5 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %6 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @C, i64 0, i64 2), align 8, !tbaa !1
  %add10 = add i32 %4, 1
  %add11 = add i32 %add10, %5
  %add12 = add i32 %add11, %6
  ret i32 %add12
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 21010) (llvm/branches/loopopt 21063)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
