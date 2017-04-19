; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: mix of load(s) and store(s) (1 load, 1 store) in 2 groups. Work on Group (B[]) only
;
; [REASONS]
; - Applicable: YES  
; - Profitable: YES
; - Legal:      YES
;
; *** Source Code ***
;int A[1000];
;int B[1000];
;int C[1000];
;
;int foo(void) {
;
;  for (int i = 1; i <= 100; ++i) {
;    A[i] = B[i - 1];
;    B[i] = A[i] + C[i];
;  }
;  return A[0] + B[1] + C[2] + 1;
;}
;
; 
; MemRefGroup: { B[i+1]        }
;                MaxLoad/MinST
; GapTracker:  { RW            }
;
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   %4 = (@B)[0][i1];
; CHECK:         |   (@A)[0][i1 + 1] = %4;
; CHECK:         |   %5 = (@C)[0][i1 + 1];
; CHECK:         |   (@B)[0][i1 + 1] = %4 + %5;
; CHECK:         + END LOOP
; CHECK:   END REGION
;  
;
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
;
; [load(s) in loop's preheader]
; CHECK:          %scalarepl = (@B)[0][0];
;
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   %4 = %scalarepl;
; CHECK:        |   (@A)[0][i1 + 1] = %4;
; CHECK:        |   %5 = (@C)[0][i1 + 1];
; CHECK:        |   %scalarepl1 = %4 + %5;
; CHECK:        |   (@B)[0][i1 + 1] = %scalarepl1;
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        + END LOOP
;        
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

@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %1 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %2 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @C, i64 0, i64 2), align 8, !tbaa !1
  %add9 = add i32 %0, 1
  %add10 = add i32 %add9, %1
  %add11 = add i32 %add10, %2
  ret i32 %add11

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %for.body ]
  %3 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %3
  %4 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 %4, i32* %arrayidx2, align 4, !tbaa !1
  %arrayidx6 = getelementptr inbounds [1000 x i32], [1000 x i32]* @C, i64 0, i64 %indvars.iv
  %5 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %add = add nsw i32 %5, %4
  %arrayidx8 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx8, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
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
