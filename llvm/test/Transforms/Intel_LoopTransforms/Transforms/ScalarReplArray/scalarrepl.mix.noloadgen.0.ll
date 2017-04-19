; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: mix of load(s) and store(s) on group (A[])
; Check: not to generate a load inside the loop (MemRef[HI] exists, but not ReadFirst)
;
; [REASONS]
; - Applicalbe:  
; - Profitable: 
; - Legal:      
;
; *** Source Code ***
; int A[1000];
;
;int foo(void) {
;  int i;
;  for (i = 0; i <= 100; ++i) {
;    A[i + 2] = A[i] + 1;
;    A[i] = A[i + 2] + 1;
;  }
;  return A[0] + 1;
;}
;
;
; CHECK: IR Dump Before HIR Scalar Repl
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:         |   %0 = (@A)[0][i1];
; CHECK:         |   (@A)[0][i1 + 2] = %0 + 1;
; CHECK:         |   %2 = (@A)[0][i1 + 2];
; CHECK:         |   (@A)[0][i1] = %2 + 1;
; CHECK:         + END LOOP
; CHECK:   END REGION
;  
;
; CHECK: IR Dump After HIR Scalar Repl
;
; CHECK:  BEGIN REGION { modified }
; Check Prehdr:
; CHECK:           %scalarepl = (@A)[0][0];
; CHECK:           %scalarepl1 = (@A)[0][1];
;
; Check loop's body:
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   %0 = %scalarepl;
; CHECK:        |   %scalarepl2 = %0 + 1;
; CHECK:        |   %2 = %scalarepl2;
; CHECK:        |   %scalarepl = %2 + 1;
; CHECK:        |   (@A)[0][i1] = %scalarepl;
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        |   %scalarepl1 = %scalarepl2;
; CHECK:        + END LOOP
;
; Check Postexit:
; CHECK:           (@A)[0][101] = %scalarepl;
; CHECK:           (@A)[0][102] = %scalarepl1;
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

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load  i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, 1
  %1 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx3 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %1
  store  i32 %add, i32* %arrayidx3, align 4, !tbaa !1
  %2 = load  i32, i32* %arrayidx3, align 4, !tbaa !1
  %add7 = add nsw i32 %2, 1
  store  i32 %add7, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %3 = load  i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %add10 = add nsw i32 %3, 1
  ret i32 %add10
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20470) (llvm/branches/loopopt 20638)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
