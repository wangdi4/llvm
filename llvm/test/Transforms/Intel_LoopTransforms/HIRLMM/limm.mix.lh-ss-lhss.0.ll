; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
;int A[10000];
;int B[10000];
;int foo(void) {
;  int i, j;
;  for (j = 0; j < 1000; ++j) {
;    for (i = 0; i < 4; ++i) {
;      B[i] = A[i] + 1;
;      A[i] = B[i + 1] * 2;
;    }
;  }
;  return A[0] + B[1] + 1;
;}
;
;[LIMM Analysis]
;MemRefCollection, entries: 9
;  (@A)[0][0] {  R  W  } 1W : 1R  legal 
;  (@B)[0][0] {  W  } 1W : 0R  legal 
;  (@B)[0][1] {  R  W  } 1W : 1R  legal 
;  (@A)[0][1] {  R  W  } 1W : 1R  legal 
;  (@B)[0][2] {  R  W  } 1W : 1R  legal 
;  (@A)[0][2] {  R  W  } 1W : 1R  legal 
;  (@B)[0][3] {  R  W  } 1W : 1R  legal 
;  (@A)[0][3] {  R  W  } 1W : 1R  legal 
;  (@B)[0][4] {  R  } 0W : 1R  legal 
;
; LIMM's Opportunities:
; - LILH:  (1)
; - LISS:  (1)
; - LILHSS:(7)
;
; INPUT HIR to LIMM:
;
;          BEGIN REGION { modified }
;<27>            + DO i1 = 0, 999, 1   <DO_LOOP>
;<29>            |   %0 = (@A)[0][0];
;<30>            |   (@B)[0][0] = %0 + 1;
;<31>            |   %1 = (@B)[0][1];
;<32>            |   (@A)[0][0] = 2 * %1;
;<33>            |   %0 = (@A)[0][1];
;<34>            |   (@B)[0][1] = %0 + 1;
;<35>            |   %1 = (@B)[0][2];
;<36>            |   (@A)[0][1] = 2 * %1;
;<37>            |   %0 = (@A)[0][2];
;<38>            |   (@B)[0][2] = %0 + 1;
;<39>            |   %1 = (@B)[0][3];
;<40>            |   (@A)[0][2] = 2 * %1;
;<41>            |   %0 = (@A)[0][3];
;<42>            |   (@B)[0][3] = %0 + 1;
;<43>            |   %1 = (@B)[0][4];
;<44>            |   (@A)[0][3] = 2 * %1;
;<27>            + END LOOP
;          END REGION
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %0 = (@A)[0][0];
; CHECK:        |   (@B)[0][0] = %0 + 1;
; CHECK:        |   %1 = (@B)[0][1];
; CHECK:        |   (@A)[0][0] = 2 * %1;
; CHECK:        |   %0 = (@A)[0][1];
; CHECK:        |   (@B)[0][1] = %0 + 1;
; CHECK:        |   %1 = (@B)[0][2];
; CHECK:        |   (@A)[0][1] = 2 * %1;
; CHECK:        |   %0 = (@A)[0][2];
; CHECK:        |   (@B)[0][2] = %0 + 1;
; CHECK:        |   %1 = (@B)[0][3];
; CHECK:        |   (@A)[0][2] = 2 * %1;
; CHECK:        |   %0 = (@A)[0][3];
; CHECK:        |   (@B)[0][3] = %0 + 1;
; CHECK:        |   %1 = (@B)[0][4];
; CHECK:        |   (@A)[0][3] = 2 * %1;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; *** *** 
;          
; CHECK: IR Dump After HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           %limm = (@A)[0][0];
; CHECK:           %limm5 = (@B)[0][1];
; CHECK:           %limm8 = (@A)[0][1];
; CHECK:           %limm11 = (@B)[0][2];
; CHECK:           %limm14 = (@A)[0][2];
; CHECK:           %limm17 = (@B)[0][3];
; CHECK:           %limm20 = (@A)[0][3];
; CHECK:           %limm23 = (@B)[0][4];
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %0 = %limm;
; CHECK:        |   %limm3 = %0 + 1;
; CHECK:        |   %1 = %limm5;
; CHECK:        |   %limm = 2 * %1;
; CHECK:        |   %0 = %limm8;
; CHECK:        |   %limm5 = %0 + 1;
; CHECK:        |   %1 = %limm11;
; CHECK:        |   %limm8 = 2 * %1;
; CHECK:        |   %0 = %limm14;
; CHECK:        |   %limm11 = %0 + 1;
; CHECK:        |   %1 = %limm17;
; CHECK:        |   %limm14 = 2 * %1;
; CHECK:        |   %0 = %limm20;
; CHECK:        |   %limm17 = %0 + 1;
; CHECK:        |   %1 = %limm23;
; CHECK:        |   %limm20 = 2 * %1;
; CHECK:        + END LOOP
; CHECK:           (@A)[0][3] = %limm20;
; CHECK:           (@B)[0][3] = %limm17;
; CHECK:           (@A)[0][2] = %limm14;
; CHECK:           (@B)[0][2] = %limm11;
; CHECK:           (@A)[0][1] = %limm8;
; CHECK:           (@B)[0][1] = %limm5;
; CHECK:            (@B)[0][0] = %limm3;
; CHECK:           (@A)[0][0] = %limm;
; CHECK:  END REGION
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [10000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [10000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %entry
  %j.025 = phi i32 [ 0, %entry ], [ %inc12, %for.inc11 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [10000 x i32], [10000 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, 1
  %arrayidx5 = getelementptr inbounds [10000 x i32], [10000 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx8 = getelementptr inbounds [10000 x i32], [10000 x i32]* @B, i64 0, i64 %indvars.iv.next
  %1 = load i32, i32* %arrayidx8, align 4, !tbaa !1
  %mul = shl nsw i32 %1, 1
  store i32 %mul, i32* %arrayidx, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc11, label %for.body3

for.inc11:                                        ; preds = %for.body3
  %inc12 = add nuw nsw i32 %j.025, 1
  %exitcond26 = icmp eq i32 %inc12, 1000
  br i1 %exitcond26, label %for.end13, label %for.cond1.preheader

for.end13:                                        ; preds = %for.inc11
  %2 = load i32, i32* getelementptr inbounds ([10000 x i32], [10000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %3 = load i32, i32* getelementptr inbounds ([10000 x i32], [10000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add14 = add i32 %2, 1
  %add15 = add i32 %add14, %3
  ret i32 %add15
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
