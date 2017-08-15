; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: loads only (2 continue loads)
;
; [REASONS]
; - Applicable: YES
; - Legal:      YES
; - Profitable: NO (ANTIDep only)
;
; *** Source Code ***
;#define N 10
;int A[N][N];
;int B[N][N];
;int C[N][N];
;
;int foo(void) {
;  int i, j, k;
;  for (k = 0; k < N; ++k) {
;    C[2][k] += A[k][0] + B[k][2] + C[2][k];
;  }
;  return A[0][0] + B[1][1] + C[2][2] + 1;
;}
;
;
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:         |   %0 = (@A)[0][i1][0];
; CHECK:         |   %1 = (@B)[0][i1][2];
; CHECK:         |   %2 = (@C)[0][2][i1];
; CHECK:         |   %3 = (@C)[0][2][i1];
; CHECK:         |   (@C)[0][2][i1] = %0 + %1 + %2 + %3;
; CHECK:         + END LOOP
; CHECK:   END REGION
;

  
; MemRefGroup: {  C[2][i1]       } 
;                 MaxLD, MinST
; GapTracker:  {  RW             }


;  
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   %0 = (@A)[0][i1][0];
; CHECK:        |   %1 = (@B)[0][i1][2];
; CHECK:        |   %scalarepl = (@C)[0][2][i1];
; CHECK:        |   %2 = %scalarepl;
; CHECK:        |   %3 = %scalarepl;
; CHECK:        |   %scalarepl = %0 + %1 + %2 + %3;
; CHECK:        |   (@C)[0][2][i1] = %scalarepl;
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

@A = common global [10 x [10 x i32]] zeroinitializer, align 16
@B = common global [10 x [10 x i32]] zeroinitializer, align 16
@C = common global [10 x [10 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @A, i64 0, i64 %indvars.iv, i64 0
  %0 = load  i32, i32* %arrayidx1, align 8, !tbaa !1
  %arrayidx4 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @B, i64 0, i64 %indvars.iv, i64 2
  %1 = load  i32, i32* %arrayidx4, align 8, !tbaa !1
  %add = add nsw i32 %1, %0
  %arrayidx6 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @C, i64 0, i64 2, i64 %indvars.iv
  %2 = load  i32, i32* %arrayidx6, align 4, !tbaa !1
  %add7 = add nsw i32 %add, %2
  %3 = load  i32, i32* %arrayidx6, align 4, !tbaa !1
  %add10 = add nsw i32 %add7, %3
  store  i32 %add10, i32* %arrayidx6, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %4 = load  i32, i32* getelementptr inbounds ([10 x [10 x i32]], [10 x [10 x i32]]* @A, i64 0, i64 0, i64 0), align 16, !tbaa !1
  %5 = load  i32, i32* getelementptr inbounds ([10 x [10 x i32]], [10 x [10 x i32]]* @B, i64 0, i64 1, i64 1), align 4, !tbaa !1
  %6 = load  i32, i32* getelementptr inbounds ([10 x [10 x i32]], [10 x [10 x i32]]* @C, i64 0, i64 2, i64 2), align 8, !tbaa !1
  %add11 = add i32 %4, 1
  %add12 = add i32 %add11, %5
  %add13 = add i32 %add12, %6
  ret i32 %add13
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
