; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
;// have LoopInv stores after unrolling innermost loop
;int A[1000];
;int B[1000];
;int foo(void) {
;  int i, j;
;
;  for (j = 0; j < 1000; ++j) {
;    for (i = 0; i < 4; ++i) {
;      B[i] = A[j] + 1;
;    }
;  }
;
;  return A[0] + B[1] + 1;
;}
;
;INPUT HIR:
;
;          BEGIN REGION { modified }
;<23>            + DO i1 = 0, 999, 1   <DO_LOOP>
;<3>             |   %0 = (@A)[0][i1];
;<25>            |   (@B)[0][0] = %0 + 1;
;<26>            |   (@B)[0][1] = %0 + 1;
;<27>            |   (@B)[0][2] = %0 + 1;
;<28>            |   (@B)[0][3] = %0 + 1;
;<23>            + END LOOP
;          END REGION
;
;[LIMM Analysis]
;MemRefCollection, entries: 4
;  (@B)[0][0] {  W  } 1W : 0R  legal 
;  (@B)[0][1] {  W  } 1W : 0R  legal 
;  (@B)[0][2] {  W  } 1W : 0R  legal 
;  (@B)[0][3] {  W  } 1W : 0R  legal 
;
; LIMM's Opportunities:
; - LILH:  (0)
; - LISS:  (4)
; - LILHSS:(0)
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %0 = (@A)[0][i1];
; CHECK:        |   (@B)[0][0] = %0 + 1;
; CHECK:        |   (@B)[0][1] = %0 + 1;
; CHECK:        |   (@B)[0][2] = %0 + 1;
; CHECK:        |   (@B)[0][3] = %0 + 1;
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
; CHECK:        |   %limm = %0 + 1;
; CHECK:        |   %limm2 = %0 + 1;
; CHECK:        |   %limm4 = %0 + 1;
; CHECK:        |   %limm6 = %0 + 1;
; CHECK:        + END LOOP
; CHECK:           (@B)[0][3] = %limm6;
; CHECK:           (@B)[0][2] = %limm4;
; CHECK:           (@B)[0][1] = %limm2;
; CHECK:           (@B)[0][0] = %limm;
; CHECK:  END REGION
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc6, %entry
  %indvars.iv18 = phi i64 [ 0, %entry ], [ %indvars.iv.next19, %for.inc6 ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv18
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, 1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %indvars.iv.next19 = add nuw nsw i64 %indvars.iv18, 1
  %exitcond20 = icmp eq i64 %indvars.iv.next19, 1000
  br i1 %exitcond20, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.inc6
  %1 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %2 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add9 = add i32 %1, 1
  %add10 = add i32 %add9, %2
  ret i32 %add10
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20318)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
