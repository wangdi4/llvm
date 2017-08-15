; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: relevant memref inside a If block
;
; [REASONS]
; - Applicable: No  (relevant memref inside a If block)
; - Profitable: N/A
; - Legal:      N/A
;
; *** Source Code ***
;int foo(void) {
;  int i;
;  for (i = 0; i < N; ++i) {
;    A[i + 2] = A[i + 1] + A[i];
;    if (i % 2 == 0) {
;      B[i] = B[i + 2];
;    }
;  }
;  return A[0] + B[1] + C[2] + 1;
;}
;
;
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   %0 = (@A)[0][i1 + 1];
; CHECK:        |   %1 = (@A)[0][i1];
; CHECK:        |   (@A)[0][i1 + 2] = %0 + %1;
; CHECK:        |   if (-1 * i1 == 0)
; CHECK:        |   {
; CHECK:        |      %3 = (@B)[0][i1 + 2];
; CHECK:        |      (@B)[0][i1] = %3;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; MemRefGroup: { A[i], A[i+1], A[i+2] }
;                              MinST
; GapTracker:  { R     R       W      }
;
;  
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           %scalarepl = (@A)[0][0];
; CHECK:           %scalarepl1 = (@A)[0][1];
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   %0 = %scalarepl1;
; CHECK:        |   %1 = %scalarepl;
; CHECK:        |   %scalarepl2 = %0 + %1;
; CHECK:        |   (@A)[0][i1 + 2] = %scalarepl2;
; CHECK:        |   if (-1 * i1 == 0)
; CHECK:        |   {
; CHECK:        |      %3 = (@B)[0][i1 + 2];
; CHECK:        |      (@B)[0][i1] = %3;
; CHECK:        |   }
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        |   %scalarepl1 = %scalarepl2;
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

@A = common global [100 x i32] zeroinitializer, align 16
@B = common global [100 x i32] zeroinitializer, align 16
@C = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.cond.backedge, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.cond.backedge ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv.next
  %0 = load  i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %1 = load  i32, i32* %arrayidx2, align 4, !tbaa !1
  %add3 = add nsw i32 %1, %0
  %2 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %2
  store  i32 %add3, i32* %arrayidx6, align 4, !tbaa !1
  %rem24 = and i64 %indvars.iv, 1
  %cmp7 = icmp eq i64 %rem24, 0
  br i1 %cmp7, label %if.then, label %for.cond.backedge

for.cond.backedge:                                ; preds = %for.body, %if.then
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

if.then:                                          ; preds = %for.body
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %2
  %3 = load  i32, i32* %arrayidx10, align 4, !tbaa !1
  %arrayidx12 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  store  i32 %3, i32* %arrayidx12, align 4, !tbaa !1
  br label %for.cond.backedge

for.end:                                          ; preds = %for.cond.backedge
  %4 = load  i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %5 = load  i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %6 = load  i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @C, i64 0, i64 2), align 8, !tbaa !1
  %add13 = add i32 %4, 1
  %add14 = add i32 %add13, %5
  %add15 = add i32 %add14, %6
  ret i32 %add15
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20731) (llvm/branches/loopopt 20744)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
