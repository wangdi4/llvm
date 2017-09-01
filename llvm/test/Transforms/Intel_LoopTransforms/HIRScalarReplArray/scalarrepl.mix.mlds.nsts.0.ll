; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: mix of multiple loads and stores for group (A[]), full group (A[])
;
; [REASONS]
; - Applicable: YES  
; - Profitable: YES
; - Legal:      YES
;
; *** Source Code ***
;//multiple loads and stores
;int A[1000];
;int B[1000];
;
;int foo(void) {
;  for (int I = 1; I <= 100; ++I) {
;    A[I] = A[I] + A[I + 1] + A[I + 2];
;    A[I + 1] = A[I - 1] + A[I + 1];
;    A[I + 2] = A[I] - A[I + 1];
;    A[I] = A[I] + A[I - 1] + A[I + 1];
;  }
;  return A[0] + B[1] + 1;
;}
;
;
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   %2 = (@A)[0][i1 + 1];
; CHECK:         |   %3 = (@A)[0][i1 + 2];
; CHECK:         |   %5 = (@A)[0][i1 + 3];
; CHECK:         |   (@A)[0][i1 + 1] = %2 + %3 + %5;
; CHECK:         |   %7 = (@A)[0][i1];
; CHECK:         |   %8 = (@A)[0][i1 + 2];
; CHECK:         |   (@A)[0][i1 + 2] = %7 + %8;
; CHECK:         |   %9 = (@A)[0][i1 + 1];
; CHECK:         |   %10 = (@A)[0][i1 + 2];
; CHECK:         |   (@A)[0][i1 + 3] = %9 + -1 * %10;
; CHECK:         |   %11 = (@A)[0][i1 + 1];
; CHECK:         |   %12 = (@A)[0][i1];
; CHECK:         |   %13 = (@A)[0][i1 + 2];
; CHECK:         |   (@A)[0][i1 + 1] = %11 + %12 + %13;
; CHECK:         + END LOOP
; CHECK:   END REGION
;
;
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
;[loads in loop's preheader]
; CHECK:            %scalarepl = (@A)[0][0];
; CHECK:            %scalarepl1 = (@A)[0][1];
; CHECK:            %scalarepl2 = (@A)[0][2];
;            
;[in-loop proc]
; CHECK:            + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   %2 = %scalarepl1;
; CHECK:           |   %3 = %scalarepl2;
; CHECK:           |   %scalarepl3 = (@A)[0][i1 + 3];
; CHECK:           |   %5 = %scalarepl3;
; CHECK:           |   %scalarepl1 = %2 + %3 + %5;
; CHECK:           |   %7 = %scalarepl;
; CHECK:           |   %8 = %scalarepl2;
; CHECK:           |   %scalarepl2 = %7 + %8;
; CHECK:           |   %9 = %scalarepl1;
; CHECK:           |   %10 = %scalarepl2;
; CHECK:           |   %scalarepl3 = %9 + -1 * %10;
; CHECK:           |   %11 = %scalarepl1;
; CHECK:           |   %12 = %scalarepl;
; CHECK:           |   %13 = %scalarepl2;
; CHECK:           |   %scalarepl1 = %11 + %12 + %13;
; CHECK:           |   (@A)[0][i1 + 1] = %scalarepl1;
; CHECK:           |   %scalarepl = %scalarepl1;
; CHECK:           |   %scalarepl1 = %scalarepl2;
; CHECK:           |   %scalarepl2 = %scalarepl3;
; CHECK:            + END LOOP
;            
;[stores in loop's postexit]:
; CHECK:           (@A)[0][101] = %scalarepl1;
; CHECK:           (@A)[0][102] = %scalarepl2;
;
; CHECK:  END REGION

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

@A = common global [1000 x i32] zeroinitializer, align 16
@B = common global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load  i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %1 = load  i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add40 = add i32 %0, 1
  %add41 = add i32 %add40, %1
  ret i32 %add41

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  %2 = load  i32, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv.next
  %3 = load  i32, i32* %arrayidx2, align 4, !tbaa !1
  %add3 = add nsw i32 %3, %2
  %4 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx6 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %4
  %5 = load  i32, i32* %arrayidx6, align 4, !tbaa !1
  %add7 = add nsw i32 %add3, %5
  store  i32 %add7, i32* %arrayidx, align 4, !tbaa !1
  %6 = add nsw i64 %indvars.iv, -1
  %arrayidx11 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %6
  %7 = load  i32, i32* %arrayidx11, align 4, !tbaa !1
  %8 = load  i32, i32* %arrayidx2, align 4, !tbaa !1
  %add15 = add nsw i32 %8, %7
  store  i32 %add15, i32* %arrayidx2, align 4, !tbaa !1
  %9 = load  i32, i32* %arrayidx, align 4, !tbaa !1
  %10 = load  i32, i32* %arrayidx2, align 4, !tbaa !1
  %sub24 = sub nsw i32 %9, %10
  store  i32 %sub24, i32* %arrayidx6, align 4, !tbaa !1
  %11 = load  i32, i32* %arrayidx, align 4, !tbaa !1
  %12 = load  i32, i32* %arrayidx11, align 4, !tbaa !1
  %add33 = add nsw i32 %12, %11
  %13 = load  i32, i32* %arrayidx2, align 4, !tbaa !1
  %add37 = add nsw i32 %add33, %13
  store  i32 %add37, i32* %arrayidx, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20470) (llvm/branches/loopopt 20506)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
