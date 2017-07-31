; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: mix, need to cleanup before proceed to ScalarRepl
;
; [REASONS]
; - Legal: YES   
; - Profitable: NO (ANTI-dep only) 
;       
;
; Note:
; Since this is a case of anti-dependence with dep-dist 0, there is no savings after ScalarRepl.
; ScalarRepl skips such cases.
;
; *** Source Code ***
;int A[1000];
;int B[1000];
;int C[1000][1000];
;int D[1000];
;
;int foo(void) {
;  for (int J = 0; J < 100; ++J) {
;    for (int I = 1; I <= 100; ++I) {
;      A[J] = B[I] + C[I][J];
;      C[I][J] = A[J] + D[I];
;    }
;  }
;  return A[0] + B[1] + C[2][2] + D[3] + 1;
;}
;
; 
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   |   %4 = (@B)[0][i2 + 1];
; CHECK:         |   |   %5 = (@C)[0][i2 + 1][i1];
; CHECK:         |   |   %6 = (@D)[0][i2 + 1];
; CHECK:         |   |   (@C)[0][i2 + 1][i1] = %4 + %5 + %6;
; CHECK:         |   + END LOOP
; CHECK:         |   
; CHECK:         |   (@A)[0][i1] = %4 + %5;
; CHECK:         + END LOOP
; CHECK:   END REGION
;
;  
; MemRefGroup: { C[i2], C[i2+1]       }
;                MinST  MaxLD
; GapTracker:  { W      R             }
;
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   |   %4 = (@B)[0][i2 + 1];
; CHECK:         |   |   %5 = (@C)[0][i2 + 1][i1];
; CHECK:         |   |   %6 = (@D)[0][i2 + 1];
; CHECK:         |   |   (@C)[0][i2 + 1][i1] = %4 + %5 + %6;
; CHECK:         |   + END LOOP
; CHECK:         |   
; CHECK:         |   (@A)[0][i1] = %4 + %5;
; CHECK:         + END LOOP
; CHECK:   END REGION
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

@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16
@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@D = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv41 = phi i64 [ 0, %entry ], [ %indvars.iv.next42, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %0 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %1 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %2 = load i32, i32* getelementptr inbounds ([1000 x [1000 x i32]], [1000 x [1000 x i32]]* @C, i64 0, i64 2, i64 2), align 8, !tbaa !1
  %3 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @D, i64 0, i64 3), align 4, !tbaa !1
  %add23 = add i32 %0, 1
  %add24 = add i32 %add23, %1
  %add25 = add i32 %add24, %2
  %add26 = add i32 %add25, %3
  ret i32 %add26

for.cond.cleanup3:                                ; preds = %for.body4
  %arrayidx10 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv41
  store i32 %add, i32* %arrayidx10, align 4, !tbaa !1
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43 = icmp eq i64 %indvars.iv.next42, 100
  br i1 %exitcond43, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx8 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @C, i64 0, i64 %indvars.iv, i64 %indvars.iv41
  %5 = load i32, i32* %arrayidx8, align 4, !tbaa !1
  %add = add nsw i32 %5, %4
  %arrayidx14 = getelementptr inbounds [1000 x i32], [1000 x i32]* @D, i64 0, i64 %indvars.iv
  %6 = load i32, i32* %arrayidx14, align 4, !tbaa !1
  %add15 = add nsw i32 %6, %add
  store i32 %add15, i32* %arrayidx8, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20398) (llvm/branches/loopopt 20421)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
