; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
; C Source Code:
;int A[1000];
;int B[1000];
;int C[1000];
;int N, M;
;int foo(void) {
;  int i, j;
;  for (j = 0; j < 100; ++j) {
;    for (i = 0; i < 50; ++i) {
;      C[j + 2 + M] = A[j + M + N] + 1 + B[i + j];
;    }
;  }
;  return 1;
;}
;
;[LIMM Analysis]
;
;
; [LIMM's Opportunities]
; - LILH:  ()
; - LISS:  ()
; - LILHSS:()
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK:         |   |   %8 = (@A)[0][i1 + sext.i32.i64(%1) + sext.i32.i64(%0)];
; CHECK:         |   |   %10 = (@B)[0][i1 + i2];
; CHECK:         |   |   (@C)[0][i1 + sext.i32.i64(%0) + 2] = %8 + %10 + 1;
; CHECK:         |   + END LOOP
; CHECK:         + END LOOP
; CHECK:   END REGION
;
;
; *** *** 
;          
; CHECK: IR Dump After HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |      %limm = (@A)[0][i1 + sext.i32.i64(%1) + sext.i32.i64(%0)];
; CHECK:        |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK:        |   |   %8 = %limm;
; CHECK:        |   |   %10 = (@B)[0][i1 + i2];
; CHECK:        |   |   %limm2 = %8 + %10 + 1;
; CHECK:        |   + END LOOP
; CHECK:        |      (@C)[0][i1 + sext.i32.i64(%0) + 2] = %limm2;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@M = common local_unnamed_addr global i32 0, align 4
@N = common local_unnamed_addr global i32 0, align 4
@A = common global [1000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* @M, align 4
  %1 = load i32, i32* @N, align 4
  %2 = sext i32 %0 to i64
  %3 = sext i32 %1 to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc14, %entry
  %indvars.iv27 = phi i64 [ 0, %entry ], [ %indvars.iv.next28, %for.inc14 ]
  %4 = add nsw i64 %2, %indvars.iv27
  %5 = add nsw i64 %4, %3
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %5
  %6 = add nuw nsw i64 %indvars.iv27, 2
  %7 = add nsw i64 %6, %2
  %arrayidx13 = getelementptr inbounds [1000 x i32], [1000 x i32]* @C, i64 0, i64 %7
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %8 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add5 = add nsw i32 %8, 1
  %9 = add nuw nsw i64 %indvars.iv, %indvars.iv27
  %arrayidx8 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %9
  %10 = load i32, i32* %arrayidx8, align 4, !tbaa !1
  %add9 = add nsw i32 %add5, %10
  store i32 %add9, i32* %arrayidx13, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.inc14, label %for.body3

for.inc14:                                        ; preds = %for.body3
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond33 = icmp eq i64 %indvars.iv.next28, 100
  br i1 %exitcond33, label %for.end16, label %for.cond1.preheader

for.end16:                                        ; preds = %for.inc14
  ret i32 1
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20470) (llvm/branches/loopopt 20482)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
