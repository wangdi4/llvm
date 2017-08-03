; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
;// have LoopInv loads and stores
;int A[1000];
;int B[10];
;int foo(void) {
;  int i, j;
;
;  for (j = 0; j < 1000; ++j) {
;    for (i = 0; i < 4; ++i) {
;      B[i] = i;
;    }
;  }
;
;  return A[0] + B[1] + 1;
;}
;
;[LIMM Analysis]
;MemRefCollection, entries: 8
;  (@B)[0][0] {  W  } 1W : 0R  legal 
;  (@A)[0][0] {  R  } 0W : 1R  legal 
;  (@B)[0][1] {  W  } 1W : 0R  legal 
;  (@A)[0][1] {  R  } 0W : 1R  legal 
;  (@B)[0][2] {  W  } 1W : 0R  legal 
;  (@A)[0][2] {  R  } 0W : 1R  legal 
;  (@B)[0][3] {  W  } 1W : 0R  legal 
;  (@A)[0][3] {  R  } 0W : 1R  legal 
;
; [LIMM Opportunity]
; - LILH:  (4)
; - LISS:  (4)
; - LILHSS:(0)
;
; INPUT HIR:
;          BEGIN REGION { modified }
;<26>            + DO i1 = 0, 999, 1   <DO_LOOP>
;<28>            |   (@B)[0][0] = 0;
;<29>            |   %1 = (@A)[0][0];
;<30>            |   (@A)[0][i1] = 2 * %1;
;<31>            |   (@B)[0][1] = 1;
;<32>            |   %1 = (@A)[0][1];
;<33>            |   (@A)[0][i1] = 2 * %1;
;<34>            |   (@B)[0][2] = 2;
;<35>            |   %1 = (@A)[0][2];
;<36>            |   (@A)[0][i1] = 2 * %1;
;<37>            |   (@B)[0][3] = 3;
;<38>            |   %1 = (@A)[0][3];
;<39>            |   (@A)[0][i1] = 2 * %1;
;<26>            + END LOOP
;          END REGION
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   (@B)[0][0] = 0;
; CHECK:        |   %1 = (@A)[0][0];
; CHECK:        |   (@A)[0][i1] = 2 * %1;
; CHECK:        |   (@B)[0][1] = 1;
; CHECK:        |   %1 = (@A)[0][1];
; CHECK:        |   (@A)[0][i1] = 2 * %1;
; CHECK:        |   (@B)[0][2] = 2;
; CHECK:        |   %1 = (@A)[0][2];
; CHECK:        |   (@A)[0][i1] = 2 * %1;
; CHECK:        |   (@B)[0][3] = 3;
; CHECK:        |   %1 = (@A)[0][3];
; CHECK:        |   (@A)[0][i1] = 2 * %1;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; *** *** 
;  
; CHECK: IR Dump After HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %limm = 0;
; CHECK:        |   %1 = (@A)[0][0];
; CHECK:        |   (@A)[0][i1] = 2 * %1;
; CHECK:        |   %limm2 = 1;
; CHECK:        |   %1 = (@A)[0][1];
; CHECK:        |   (@A)[0][i1] = 2 * %1;
; CHECK:        |   %limm4 = 2;
; CHECK:        |   %1 = (@A)[0][2];
; CHECK:        |   (@A)[0][i1] = 2 * %1;
; CHECK:        |   %limm6 = 3;
; CHECK:        |   %1 = (@A)[0][3];
; CHECK:        |   (@A)[0][i1] = 2 * %1;
; CHECK:        + END LOOP
; CHECK:           (@B)[0][3] = %limm6;
; CHECK:           (@B)[0][2] = %limm4;
; CHECK:           (@B)[0][1] = %limm2;
; CHECK:           (@B)[0][0] = %limm;
; CHECK:  END REGION
;

source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc8, %entry
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.inc8 ]
  %arrayidx7 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv22
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx5 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %mul = shl nsw i32 %1, 1
  store i32 %mul, i32* %arrayidx7, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc8, label %for.body3

for.inc8:                                         ; preds = %for.body3
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next23, 1000
  br i1 %exitcond24, label %for.end10, label %for.cond1.preheader

for.end10:                                        ; preds = %for.inc8
  %2 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %3 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add = add i32 %2, 1
  %add11 = add i32 %add, %3
  ret i32 %add11
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17966)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
