; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: mix of loads and stores, having MemRef(s) with gap
;
; Note:
; generate load(s) from missing MemRefs (the gaps)
;
; [REASONS]
; - Applicable: YES
; - Legal:      YES      
; - Profitable: YES
;
; *** Source Code ***
;int A[1000];
;int B[1000];
;int D[1000];
;
;int foo(void) {
;  for (int i = 0; i <= 100; ++i) {
;    A[i] = D[i] + A[i + 1];
;    A[i + 4] = A[i] + B[i];
;  }
;
;  return A[0] + B[1] + 1;
;}
;
; 
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:         |   %2 = (@D)[0][i1];
; CHECK:         |   %3 = (@A)[0][i1 + 1];
; CHECK:         |   (@A)[0][i1] = %2 + %3;
; CHECK:         |   %4 = (@B)[0][i1];
; CHECK:         |   (@A)[0][i1 + 4] = %2 + %3 + %4;
; CHECK:         + END LOOP
; CHECK:   END REGION
;
; =====================================================
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
;[loads in loop's preheader]
; CHECK:        %scalarepl = (@A)[0][0];
; CHECK:        %scalarepl1 = (@A)[0][1];
; CHECK:        %scalarepl2 = (@A)[0][2];
; CHECK:        %scalarepl3 = (@A)[0][3];
;        
;[in-loop proc]
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   %2 = (@D)[0][i1];
; CHECK:        |   %3 = %scalarepl1;
; CHECK:        |   %scalarepl = %2 + %3;
; CHECK:        |   (@A)[0][i1] = %scalarepl;
; CHECK:        |   %4 = (@B)[0][i1];
; CHECK:        |   %scalarepl4 = %2 + %3 + %4;
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        |   %scalarepl1 = %scalarepl2;
; CHECK:        |   %scalarepl2 = %scalarepl3;
; CHECK:        |   %scalarepl3 = %scalarepl4;
; CHECK:        + END LOOP
;            
;[stores in loop's postexit]
; CHECK:           (@A)[0][101] = %scalarepl;
; CHECK:           (@A)[0][102] = %scalarepl1;
; CHECK:           (@A)[0][103] = %scalarepl2;
; CHECK:           (@A)[0][104] = %scalarepl3;
;
; CHECK:          END REGION

;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@D = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %1 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add14 = add i32 %0, 1
  %add15 = add i32 %add14, %1
  ret i32 %add15

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @D, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv.next
  %3 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %add3 = add nsw i32 %3, %2
  %arrayidx5 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 %add3, i32* %arrayidx5, align 4, !tbaa !1
  %arrayidx9 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx9, align 4, !tbaa !1
  %add10 = add nsw i32 %4, %add3
  %5 = add nuw nsw i64 %indvars.iv, 4
  %arrayidx13 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %5
  store i32 %add10, i32* %arrayidx13, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
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
