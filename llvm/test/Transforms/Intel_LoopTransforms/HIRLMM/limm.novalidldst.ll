; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
;// have LoopInv loads and stores, but none on dominating path.
;int A[1000];
;int B[1000];
;int foo(void) {
;  int i, j;
;  for (j = 0; j < 1000; ++j) {
;    for (i = 0; i < 4; ++i) {
;      if ((i*3 + j) % 2 == 0) {
;        B[i] = B[i] + A[j];
;      }
;    }
;  }
;  return A[0] + B[1] + 1;
;}
;
; INPUT HIR:
;
;          BEGIN REGION { modified }
;<34>            + DO i1 = 0, 999, 1   <DO_LOOP>
;<36>            |   if (-1 * i1 == 0)
;<36>            |   {
;<37>            |      %2 = (@B)[0][0];
;<38>            |      %3 = (@A)[0][i1];
;<39>            |      (@B)[0][0] = %2 + %3;
;<36>            |   }
;<40>            |   if (-1 * i1 + -1 == 0)
;<40>            |   {
;<41>            |      %2 = (@B)[0][1];
;<42>            |      %3 = (@A)[0][i1];
;<43>            |      (@B)[0][1] = %2 + %3;
;<40>            |   }
;<44>            |   if (-1 * i1 + -2 == 0)
;<44>            |   {
;<45>            |      %2 = (@B)[0][2];
;<46>            |      %3 = (@A)[0][i1];
;<47>            |      (@B)[0][2] = %2 + %3;
;<44>            |   }
;<48>            |   if (-1 * i1 + -3 == 0)
;<48>            |   {
;<49>            |      %2 = (@B)[0][3];
;<50>            |      %3 = (@A)[0][i1];
;<51>            |      (@B)[0][3] = %2 + %3;
;<48>            |   }
;<34>            + END LOOP
;          END REGION
;
;
;[LIMM Analysis]
;MemRefCollection, entries: 4
;  (@B)[0][0] {  R  W  } 1W : 1R  illegal 
;  (@B)[0][1] {  R  W  } 1W : 1R  illegal 
;  (@B)[0][2] {  R  W  } 1W : 1R  illegal 
;  (@B)[0][3] {  R  W  } 1W : 1R  illegal 
;
; LIMM's Opportunities:
; - LILH:  (0)
; - LISS:  (0)
; - LILHSS:(0)
;
; Note:
; However, none of the loop-inv load/store is on dominating path.
;
; CHECK: IR Dump Before HIR Loop Memory Motion
;
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   if (-1 * i1 == 0)
; CHECK:        |   {
; CHECK:        |      %2 = (@B)[0][0];
; CHECK:        |      %3 = (@A)[0][i1];
; CHECK:        |      (@B)[0][0] = %2 + %3;
; CHECK:        |   }
; CHECK:        |   if (-1 * i1 + -1 == 0)
; CHECK:        |   {
; CHECK:        |      %2 = (@B)[0][1];
; CHECK:        |      %3 = (@A)[0][i1];
; CHECK:        |      (@B)[0][1] = %2 + %3;
; CHECK:        |   }
; CHECK:        |   if (-1 * i1 + -2 == 0)
; CHECK:        |   {
; CHECK:        |      %2 = (@B)[0][2];
; CHECK:        |      %3 = (@A)[0][i1];
; CHECK:        |      (@B)[0][2] = %2 + %3;
; CHECK:        |   }
; CHECK:        |   if (-1 * i1 + -3 == 0)
; CHECK:        |   {
; CHECK:        |      %2 = (@B)[0][3];
; CHECK:        |      %3 = (@A)[0][i1];
; CHECK:        |      (@B)[0][3] = %2 + %3;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; *** *** 
;          
; CHECK: IR Dump After HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   if (-1 * i1 == 0)
; CHECK:        |   {
; CHECK:        |      %2 = (@B)[0][0];
; CHECK:        |      %3 = (@A)[0][i1];
; CHECK:        |      (@B)[0][0] = %2 + %3;
; CHECK:        |   }
; CHECK:        |   if (-1 * i1 + -1 == 0)
; CHECK:        |   {
; CHECK:        |      %2 = (@B)[0][1];
; CHECK:        |      %3 = (@A)[0][i1];
; CHECK:        |      (@B)[0][1] = %2 + %3;
; CHECK:        |   }
; CHECK:        |   if (-1 * i1 + -2 == 0)
; CHECK:        |   {
; CHECK:        |      %2 = (@B)[0][2];
; CHECK:        |      %3 = (@A)[0][i1];
; CHECK:        |      (@B)[0][2] = %2 + %3;
; CHECK:        |   }
; CHECK:        |   if (-1 * i1 + -3 == 0)
; CHECK:        |   {
; CHECK:        |      %2 = (@B)[0][3];
; CHECK:        |      %3 = (@A)[0][i1];
; CHECK:        |      (@B)[0][3] = %2 + %3;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc10, %entry
  %indvars.iv29 = phi i64 [ 0, %entry ], [ %indvars.iv.next30, %for.inc10 ]
  %arrayidx6 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv29
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  %0 = mul nuw nsw i64 %indvars.iv, 3
  %1 = add nuw nsw i64 %0, %indvars.iv29
  %rem24 = and i64 %1, 1
  %cmp4 = icmp eq i64 %rem24, 0
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %3 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %add7 = add nsw i32 %3, %2
  store i32 %add7, i32* %arrayidx, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc10, label %for.body3

for.inc10:                                        ; preds = %for.inc
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %exitcond31 = icmp eq i64 %indvars.iv.next30, 1000
  br i1 %exitcond31, label %for.end12, label %for.cond1.preheader

for.end12:                                        ; preds = %for.inc10
  %4 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %5 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add13 = add i32 %4, 1
  %add14 = add i32 %add13, %5
  ret i32 %add14
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20374)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
