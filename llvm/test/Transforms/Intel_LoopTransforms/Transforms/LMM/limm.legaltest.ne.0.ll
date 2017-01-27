; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
; C Source Code:
;int A[1000];
;int B[1000];
;int N, M;
;int foo(void) {
;  int i, j;
;  for (j = 0; j < 100; ++j) {
;    for (i = 0; i < 50; ++i) {
;      A[j + 2] = A[j] + 1 + B[i + j];
;    }
;  }
;  return 1;
;}
;
; INPUT HIR:
;
;          BEGIN REGION { }
;<26>            + DO i1 = 0, 99, 1   <DO_LOOP>
;<25>            |   + DO i2 = 0, 49, 1   <DO_LOOP>
;<9>             |   |   %1 = (@A)[0][i1];
;<13>            |   |   %3 = (@B)[0][i1 + i2];
;<15>            |   |   (@A)[0][i1 + 2] = %1 + %3 + 1;
;<25>            |   + END LOOP
;<26>            + END LOOP
;          END REGION
;
;[LIMM Analysis]
;MemRefCollection, entries: 2
;  (@A)[0][i1] {  R  } 0W : 1R  profitable  legal  analyzed  :load-only: 
;  (@A)[0][i1 + 2] {  W  } 1W : 0R  profitable  legal  analyzed  :store-only: 
;
;
; [LIMM's Opportunities]
; - LILH:  (1)
; - LISS:  (1)
; - LILHSS:(0)
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK:         |   |   %1 = (@A)[0][i1];
; CHECK:         |   |   %3 = (@B)[0][i1 + i2];
; CHECK:         |   |   (@A)[0][i1 + 2] = %1 + %3 + 1;
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
; CHECK:        |      %limm = (@A)[0][i1];
; CHECK:        |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK:        |   |   %1 = %limm;
; CHECK:        |   |   %3 = (@B)[0][i1 + i2];
; CHECK:        |   |   %limm2 = %1 + %3 + 1;
; CHECK:        |   + END LOOP
; CHECK:        |      (@A)[0][i1 + 2] = %limm2;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@N = common local_unnamed_addr global i32 0, align 4
@M = common local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %entry
  %indvars.iv24 = phi i64 [ 0, %entry ], [ %indvars.iv.next25, %for.inc11 ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv24
  %0 = add nuw nsw i64 %indvars.iv24, 2
  %arrayidx10 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %0
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %1, 1
  %2 = add nuw nsw i64 %indvars.iv, %indvars.iv24
  %arrayidx6 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %2
  %3 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %add7 = add nsw i32 %add, %3
  store i32 %add7, i32* %arrayidx10, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.inc11, label %for.body3

for.inc11:                                        ; preds = %for.body3
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond27 = icmp eq i64 %indvars.iv.next25, 100
  br i1 %exitcond27, label %for.end13, label %for.cond1.preheader

for.end13:                                        ; preds = %for.inc11
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
