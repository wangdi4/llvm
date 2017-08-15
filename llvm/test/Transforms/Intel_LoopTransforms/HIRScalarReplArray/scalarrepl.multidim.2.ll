; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: multi-dimensional array
;
; [REASONS]
; - Applicable: YES
; - Profitable: YES
; - Legal:      YES
;
; *** Source Code ***
;#define N 10
;int A[N][N];
;int B[N][N];
;int C[N][N];
;
;int foo(void) {
;  int i, j, k;
;  for (i = 0; i < N; ++i) {
;    for (j = 0; j < N; ++j) {
;      A[i][j] = A[i][j] * 2 + B[i][j];
;      B[i][j] = B[i][j] * 3 + C[i][j];
;      C[i][j] = C[i][j] * 4 + A[i][j];
;    }
;  }
;  return A[0][0] + B[1][1] + C[2][2] + 1;
;}
;
;
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:         |   |   %0 = (@A)[0][i1][i2];
; CHECK:         |   |   %1 = (@B)[0][i1][i2];
; CHECK:         |   |   (@A)[0][i1][i2] = 2 * %0 + %1;
; CHECK:         |   |   %2 = (@B)[0][i1][i2];
; CHECK:         |   |   %3 = (@C)[0][i1][i2];
; CHECK:         |   |   (@B)[0][i1][i2] = 3 * %2 + %3;
; CHECK:         |   |   %4 = (@C)[0][i1][i2];
; CHECK:         |   |   %5 = (@A)[0][i1][i2];
; CHECK:         |   |   (@C)[0][i1][i2] = 4 * %4 + %5;
; CHECK:         |   + END LOOP
; CHECK:         + END LOOP
; CHECK:   END REGION
;
;  
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   %scalarepl = (@A)[0][i1][i2];
; CHECK:        |   |   %0 = %scalarepl;
; CHECK:        |   |   %scalarepl4 = (@B)[0][i1][i2];
; CHECK:        |   |   %1 = %scalarepl4;
; CHECK:        |   |   %scalarepl = 2 * %0 + %1;
; CHECK:        |   |   (@A)[0][i1][i2] = %scalarepl;
; CHECK:        |   |   %2 = %scalarepl4;
; CHECK:        |   |   %scalarepl9 = (@C)[0][i1][i2];
; CHECK:        |   |   %3 = %scalarepl9;
; CHECK:        |   |   %scalarepl4 = 3 * %2 + %3;
; CHECK:        |   |   (@B)[0][i1][i2] = %scalarepl4;
; CHECK:        |   |   %4 = %scalarepl9;
; CHECK:        |   |   %5 = %scalarepl;
; CHECK:        |   |   %scalarepl9 = 4 * %4 + %5;
; CHECK:        |   |   (@C)[0][i1][i2] = %scalarepl9;
; CHECK:        |   + END LOOP
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
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc42, %entry
  %indvars.iv73 = phi i64 [ 0, %entry ], [ %indvars.iv.next74, %for.inc42 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @A, i64 0, i64 %indvars.iv73, i64 %indvars.iv
  %0 = load  i32, i32* %arrayidx5, align 4, !tbaa !1
  %mul = shl i32 %0, 1
  %arrayidx9 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @B, i64 0, i64 %indvars.iv73, i64 %indvars.iv
  %1 = load  i32, i32* %arrayidx9, align 4, !tbaa !1
  %add = add nsw i32 %mul, %1
  store  i32 %add, i32* %arrayidx5, align 4, !tbaa !1
  %2 = load  i32, i32* %arrayidx9, align 4, !tbaa !1
  %mul18 = mul nsw i32 %2, 3
  %arrayidx22 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @C, i64 0, i64 %indvars.iv73, i64 %indvars.iv
  %3 = load  i32, i32* %arrayidx22, align 4, !tbaa !1
  %add23 = add nsw i32 %mul18, %3
  store  i32 %add23, i32* %arrayidx9, align 4, !tbaa !1
  %4 = load  i32, i32* %arrayidx22, align 4, !tbaa !1
  %mul32 = shl i32 %4, 2
  %5 = load  i32, i32* %arrayidx5, align 4, !tbaa !1
  %add37 = add nsw i32 %mul32, %5
  store  i32 %add37, i32* %arrayidx22, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc42, label %for.body3

for.inc42:                                        ; preds = %for.body3
  %indvars.iv.next74 = add nuw nsw i64 %indvars.iv73, 1
  %exitcond75 = icmp eq i64 %indvars.iv.next74, 10
  br i1 %exitcond75, label %for.end44, label %for.cond1.preheader

for.end44:                                        ; preds = %for.inc42
  %6 = load  i32, i32* getelementptr inbounds ([10 x [10 x i32]], [10 x [10 x i32]]* @A, i64 0, i64 0, i64 0), align 16, !tbaa !1
  %7 = load  i32, i32* getelementptr inbounds ([10 x [10 x i32]], [10 x [10 x i32]]* @B, i64 0, i64 1, i64 1), align 4, !tbaa !1
  %8 = load  i32, i32* getelementptr inbounds ([10 x [10 x i32]], [10 x [10 x i32]]* @C, i64 0, i64 2, i64 2), align 8, !tbaa !1
  %add45 = add i32 %6, 1
  %add46 = add i32 %add45, %7
  %add47 = add i32 %add46, %8
  ret i32 %add47
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
