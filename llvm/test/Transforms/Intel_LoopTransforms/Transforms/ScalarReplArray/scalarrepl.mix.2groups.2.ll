; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: mix, 2 groups, working on group A[] only
;
; [REASONS]  
; - Applicable: YES  
; - Profitable: YES
; - Legal:      YES
;
; *** Source Code ***
;
;int A[1000];
;int B[1000];
;int C[1000];
;
;int foo(void) {
;
;  for (int i = 1; i <= 100; ++i) {
;    A[i + 1] = A[i - 1] + B[i - 1];
;    A[i] = A[i] + B[i] + B[i + 1];
;  }
;  return A[0] + B[1] + C[2] + 1;
;}
;
;
;[MemRef Group Data]
;((@A)[0][i1] (R), 0, %scalarepl ) 
;((@A)[0][i1 + 1] (R), 1, %scalarepl1 ) 
;((@A)[0][i1 + 1] (W), 1, %scalarepl1 ) min_idx 
;((@A)[0][i1 + 2] (W), 2, %scalarepl2 ) 
;%scalarepl, %scalarepl1, %scalarepl2, 
;
;<4> { ((@A)[0][i1] (R), 0, %scalarepl ) , ((@A)[0][i1 + 1] (R), 1, %scalarepl1 ) , ((@A)[0][i1 + 1] (W), 1, %scalarepl1 ) min_idx , ((@A)[0][i1 + 2] (W), 2, %scalarepl2 ) ,  } 2W : 2R , profitable , legal , MaxDD: 2, Symbase: 23, TmpV:<3> [ %scalarepl, %scalarepl1, %scalarepl2] 
; 
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:    BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   %4 = (@A)[0][i1];
; CHECK:         |   %5 = (@B)[0][i1];
; CHECK:         |   (@A)[0][i1 + 2] = %4 + %5;
; CHECK:         |   %6 = (@A)[0][i1 + 1];
; CHECK:         |   %7 = (@B)[0][i1 + 1];
; CHECK:         |   %8 = (@B)[0][i1 + 2];
; CHECK:         |   (@A)[0][i1 + 1] = %6 + %7 + %8;
; CHECK:         + END LOOP
; CHECK:   END REGION
;  
;
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
;
; [loads in loop's preheader]
; CHECK:          %scalarepl = (@A)[0][0];
; CHECK:          %scalarepl1 = (@A)[0][1];
;        
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   %4 = %scalarepl;
; CHECK:        |   %5 = (@B)[0][i1];
; CHECK:        |   %scalarepl2 = %4 + %5;
; CHECK:        |   %6 = %scalarepl1;
; CHECK:        |   %7 = (@B)[0][i1 + 1];
; CHECK:        |   %8 = (@B)[0][i1 + 2];
; CHECK:        |   %scalarepl1 = %6 + %7 + %8;
; CHECK:        |   (@A)[0][i1 + 1] = %scalarepl1;
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        |   %scalarepl1 = %scalarepl2;
; CHECK:        + END LOOP
;        
; [stores in loop's postexit]:
; CHECK:           (@A)[0][101] = %scalarepl1;
;
; CHECK:  END REGION
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

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %1 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %2 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @C, i64 0, i64 2), align 8, !tbaa !1
  %add18 = add i32 %0, 1
  %add19 = add i32 %add18, %1
  %add20 = add i32 %add19, %2
  ret i32 %add20

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %for.body ]
  %3 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %3
  %4 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx3 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %3
  %5 = load i32, i32* %arrayidx3, align 4, !tbaa !1
  %add = add nsw i32 %5, %4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx6 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv.next
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !1
  %arrayidx8 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  %6 = load i32, i32* %arrayidx8, align 4, !tbaa !1
  %arrayidx10 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv
  %7 = load i32, i32* %arrayidx10, align 4, !tbaa !1
  %add11 = add nsw i32 %7, %6
  %arrayidx14 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv.next
  %8 = load i32, i32* %arrayidx14, align 4, !tbaa !1
  %add15 = add nsw i32 %add11, %8
  store i32 %add15, i32* %arrayidx8, align 4, !tbaa !1
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
