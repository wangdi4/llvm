; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: mix need cleanup 
;
; [REASONS]
; - Applicable: YES  
; - Legal:      NO (Unknown dependence distance for A[J] accesss)
; - Profitable: N/A
;
; *** Source Code ***
;
;int A[1000];
;int B[1000];
;
;int foo(void) {
;  for (int J = 0; J < 100; ++J) {
;    for (int I = 1; I <= 100; ++I) {
;      A[I] = A[I - 1] + B[I];
;      A[J] = A[J] + A[I];
;    }
;  }
;  return A[0] + B[1] + 1;
;}
;
; 
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   |   %3 = (@A)[0][i2];
; CHECK:         |   |   %4 = (@B)[0][i2 + 1];
; CHECK:         |   |   (@A)[0][i2 + 1] = %3 + %4;
; CHECK:         |   |   %5 = (@A)[0][i1];
; CHECK:         |   |   (@A)[0][i1] = %3 + %4 + %5;
; CHECK:         |   + END LOOP
; CHECK:         + END LOOP
; CHECK:   END REGION
;  
;
; MemRefGroup: { A[i], A[i+1], }
;                MinST    
; GapTracker:  { RW     W      }

;
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   |   %3 = (@A)[0][i2];
; CHECK:         |   |   %4 = (@B)[0][i2 + 1];
; CHECK:         |   |   (@A)[0][i2 + 1] = %3 + %4;
; CHECK:         |   |   %5 = (@A)[0][i1];
; CHECK:         |   |   (@A)[0][i1] = %3 + %4 + %5;
; CHECK:         |   + END LOOP
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

@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv34 = phi i64 [ 0, %entry ], [ %indvars.iv.next35, %for.cond.cleanup3 ]
  %arrayidx10 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv34
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %0 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %1 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add19 = add i32 %0, 1
  %add20 = add i32 %add19, %1
  ret i32 %add20

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond36 = icmp eq i64 %indvars.iv.next35, 100
  br i1 %exitcond36, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %2 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %2
  %3 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx6 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %add = add nsw i32 %4, %3
  %arrayidx8 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx8, align 4, !tbaa !1
  %5 = load i32, i32* %arrayidx10, align 4, !tbaa !1
  %add13 = add nsw i32 %5, %add
  store i32 %add13, i32* %arrayidx10, align 4, !tbaa !1
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
