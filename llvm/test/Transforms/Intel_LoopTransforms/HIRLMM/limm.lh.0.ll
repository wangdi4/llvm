; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -hir-lmm  -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
; (This test is based on CompleteUnroll/triinner.ll)
;
;// have LoopInv loads and stores
;int A[1000];
;int B[1000];
;int foo(void) {
;  int i, j;
;
;  for (j = 0; j < 1000; ++j) {
;    for (i = 0; i < 4; ++i) {
;      B[i] = A[j] + B[i] + 1;
;      A[j] = A[i] + B[j] + 2;
;    }
;  }
;
;  return A[0] + B[1] + 1;
;}
;
; INPUT HIR:
;
;          BEGIN REGION { modified }
;<34>            + DO i1 = 0, 999, 1   <DO_LOOP>
;<5>             |   %0 = (@A)[0][i1];
;<36>            |   %1 = (@B)[0][0];
;<37>            |   (@B)[0][0] = %1 + %0 + 1;
;<38>            |   %2 = (@A)[0][0];
;<39>            |   %3 = (@B)[0][i1];
;<40>            |   (@A)[0][i1] = %2 + %3 + 2;
;<41>            |   %0 = %2 + %3 + 2;
;<42>            |   %1 = (@B)[0][1];
;<43>            |   (@B)[0][1] = %1 + %0 + 1;
;<44>            |   %2 = (@A)[0][1];
;<45>            |   %3 = (@B)[0][i1];
;<46>            |   (@A)[0][i1] = %2 + %3 + 2;
;<47>            |   %0 = %2 + %3 + 2;
;<48>            |   %1 = (@B)[0][2];
;<49>            |   (@B)[0][2] = %1 + %0 + 1;
;<50>            |   %2 = (@A)[0][2];
;<51>            |   %3 = (@B)[0][i1];
;<52>            |   (@A)[0][i1] = %2 + %3 + 2;
;<53>            |   %0 = %2 + %3 + 2;
;<54>            |   %1 = (@B)[0][3];
;<55>            |   (@B)[0][3] = %1 + %0 + 1;
;<56>            |   %2 = (@A)[0][3];
;<57>            |   %3 = (@B)[0][i1];
;<58>            |   (@A)[0][i1] = %2 + %3 + 2;
;<59>            |   %0 = %2 + %3 + 2;
;<34>            + END LOOP
;          END REGION
;
;
;[LIMM Analysis]
;MemRefCollection, entries: 8
;  (@B)[0][0] {  R  W  } 1W : 1R  illegal 
;  (@A)[0][0] {  R  } 0W : 1R  illegal 
;  (@B)[0][1] {  R  W  } 1W : 1R  illegal 
;  (@A)[0][1] {  R  } 0W : 1R  illegal 
;  (@B)[0][2] {  R  W  } 1W : 1R  illegal 
;  (@A)[0][2] {  R  } 0W : 1R  illegal 
;  (@B)[0][3] {  R  W  } 1W : 1R  illegal 
;  (@A)[0][3] {  R  } 0W : 1R  illegal 
;
; LIMM's Opportunities:
; - LILH:  (0)
; - LISS:  (0)
; - LILHSS:(0)
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %0 = (@A)[0][i1];
; CHECK:        |   %1 = (@B)[0][0];
; CHECK:        |   (@B)[0][0] = %1 + %0 + 1;
; CHECK:        |   %2 = (@A)[0][0];
; CHECK:        |   %3 = (@B)[0][i1];
; CHECK:        |   (@A)[0][i1] = %2 + %3 + 2;
; CHECK:        |   %0 = %2 + %3 + 2;
; CHECK:        |   %1 = (@B)[0][1];
; CHECK:        |   (@B)[0][1] = %1 + %0 + 1;
; CHECK:        |   %2 = (@A)[0][1];
; CHECK:        |   %3 = (@B)[0][i1];
; CHECK:        |   (@A)[0][i1] = %2 + %3 + 2;
; CHECK:        |   %0 = %2 + %3 + 2;
; CHECK:        |   %1 = (@B)[0][2];
; CHECK:        |   (@B)[0][2] = %1 + %0 + 1;
; CHECK:        |   %2 = (@A)[0][2];
; CHECK:        |   %3 = (@B)[0][i1];
; CHECK:        |   (@A)[0][i1] = %2 + %3 + 2;
; CHECK:        |   %0 = %2 + %3 + 2;
; CHECK:        |   %1 = (@B)[0][3];
; CHECK:        |   (@B)[0][3] = %1 + %0 + 1;
; CHECK:        |   %2 = (@A)[0][3];
; CHECK:        |   %3 = (@B)[0][i1];
; CHECK:        |   (@A)[0][i1] = %2 + %3 + 2;
; CHECK:        |   %0 = %2 + %3 + 2;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; *** *** 
;          
; CHECK: IR Dump After HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %0 = (@A)[0][i1];
; CHECK:        |   %1 = (@B)[0][0];
; CHECK:        |   (@B)[0][0] = %1 + %0 + 1;
; CHECK:        |   %2 = (@A)[0][0];
; CHECK:        |   %3 = (@B)[0][i1];
; CHECK:        |   (@A)[0][i1] = %2 + %3 + 2;
; CHECK:        |   %0 = %2 + %3 + 2;
; CHECK:        |   %1 = (@B)[0][1];
; CHECK:        |   (@B)[0][1] = %1 + %0 + 1;
; CHECK:        |   %2 = (@A)[0][1];
; CHECK:        |   %3 = (@B)[0][i1];
; CHECK:        |   (@A)[0][i1] = %2 + %3 + 2;
; CHECK:        |   %0 = %2 + %3 + 2;
; CHECK:        |   %1 = (@B)[0][2];
; CHECK:        |   (@B)[0][2] = %1 + %0 + 1;
; CHECK:        |   %2 = (@A)[0][2];
; CHECK:        |   %3 = (@B)[0][i1];
; CHECK:        |   (@A)[0][i1] = %2 + %3 + 2;
; CHECK:        |   %0 = %2 + %3 + 2;
; CHECK:        |   %1 = (@B)[0][3];
; CHECK:        |   (@B)[0][3] = %1 + %0 + 1;
; CHECK:        |   %2 = (@A)[0][3];
; CHECK:        |   %3 = (@B)[0][i1];
; CHECK:        |   (@A)[0][i1] = %2 + %3 + 2;
; CHECK:        |   %0 = %2 + %3 + 2;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc17, %entry
  %indvars.iv33 = phi i64 [ 0, %entry ], [ %indvars.iv.next34, %for.inc17 ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %indvars.iv33
  %arrayidx12 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %indvars.iv33
  %.pre = load i32, i32* %arrayidx, align 4, !tbaa !1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %0 = phi i32 [ %.pre, %for.cond1.preheader ], [ %add14, %for.body3 ]
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %add = add i32 %0, 1
  %add6 = add i32 %add, %1
  store i32 %add6, i32* %arrayidx5, align 4, !tbaa !1
  %arrayidx10 = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx10, align 4, !tbaa !1
  %3 = load i32, i32* %arrayidx12, align 4, !tbaa !1
  %add13 = add i32 %2, 2
  %add14 = add i32 %add13, %3
  store i32 %add14, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc17, label %for.body3

for.inc17:                                        ; preds = %for.body3
  %indvars.iv.next34 = add nuw nsw i64 %indvars.iv33, 1
  %exitcond35 = icmp eq i64 %indvars.iv.next34, 1000
  br i1 %exitcond35, label %for.end19, label %for.cond1.preheader

for.end19:                                        ; preds = %for.inc17
  %4 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %5 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add20 = add i32 %4, 1
  %add21 = add i32 %add20, %5
  ret i32 %add21
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17952)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
