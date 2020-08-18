; RUN: opt -hir-ssa-deconstruction -hir-cost-model-throttling=0 -hir-loop-reversal -print-before=hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1 < %s  |	FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir>,hir-loop-reversal,print<hir>" -hir-cost-model-throttling=0 -S 2>&1 < %s  | FileCheck %s

; l1reversal9-dimaware-2d.ll:
; 1-level loop, sanity testcase, dimension-aware on a 2D array
;
; *** Source Code ***
;
;//stride-aware 2D
;int A[100][100];
;int B[100][100];
;
;int foo(void) {
;  int i = 0;
;
;  //loop i
;  for (i = 0; i <= 4; i++) {
;    A[10 - i][2] = B[10 - i][i] -i;
;    A[1][20 - i] = B[20 - i][2] + 2*i;
;  }
;
;  return A[0][0] + B[1][1] + 1;
;}
;

;    *** IR Dump Before HIR Loop Reversal ***

; CHECK:        BEGIN REGION { }
; CHECK:            + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:            |   %1 = (@B)[0][-1 * i1 + 10][i1];
; CHECK:            |   (@A)[0][-1 * i1 + 10][2] = -1 * i1 + %1;
; CHECK:            |   %4 = (@B)[0][-1 * i1 + 20][2];
; CHECK:            |   (@A)[0][1][-1 * i1 + 20] = 2 * i1 + %4;
; CHECK:            + END LOOP
; CHECK:      END REGION


;   *** IR Dump After HIR Loop Reversal ***

; CHECK:      BEGIN REGION { modified }
; CHECK:            + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:            |   %1 = (@B)[0][i1 + 6][-1 * i1 + 4];
; CHECK:            |   (@A)[0][i1 + 6][2] = i1 + %1 + -4;
; CHECK:            |   %4 = (@B)[0][i1 + 16][2];
; CHECK:            |   (@A)[0][1][i1 + 16] = -2 * i1 + %4 + 8;
; CHECK:            + END LOOP
; CHECK:      END REGION

;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [100 x [100 x i32]] zeroinitializer, align 16
@A = common global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = sub nuw nsw i64 10, %indvars.iv
  %arrayidx2 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %2 = trunc i64 %indvars.iv to i32
  %sub3 = sub nsw i32 %1, %2
  %arrayidx7 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %0, i64 2
  store i32 %sub3, i32* %arrayidx7, align 8, !tbaa !1
  %3 = sub nuw nsw i64 20, %indvars.iv
  %arrayidx11 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %3, i64 2
  %4 = load i32, i32* %arrayidx11, align 8, !tbaa !1
  %5 = shl i64 %indvars.iv, 1
  %6 = trunc i64 %5 to i32
  %add = add nsw i32 %4, %6
  %arrayidx14 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 1, i64 %3
  store i32 %add, i32* %arrayidx14, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %7 = load i32, i32* getelementptr inbounds ([100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 0, i64 0), align 16, !tbaa !1
  %8 = load i32, i32* getelementptr inbounds ([100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 1, i64 1), align 4, !tbaa !1
  %add15 = add i32 %7, 1
  %add16 = add i32 %add15, %8
  ret i32 %add16
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12266)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
