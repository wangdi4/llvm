; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
; Source code Sketch: Original Loop
;
;int A[100];
;int B[100];
;int foo(void) {
;  int i, j;
;
;  for (j = 0; j < 10; ++j) {
;    for (i = 0; i < 20; ++i) {
;      if (B[i]=2*j) {
;        A[j] = A[j] + 1;
;      }
;    }
;  }
;
;  return A[0] + B[1] + 1;
;}
;
; INPUT HIR:
;
;          BEGIN REGION { modified }
;<33>            + DO i1 = 0, 19, 1   <DO_LOOP>
;<34>            |   + DO i2 = 0, 9, 1   <DO_LOOP>
;<10>            |   |   (@B)[0][i1] = 2 * i2;
;<11>            |   |   if (i2 != 0)
;<11>            |   |   {
;<15>            |   |      %2 = (@A)[0][i2];
;<17>            |   |      (@A)[0][i2] = %2 + 1;
;<11>            |   |   }
;<34>            |   + END LOOP
;<33>            + END LOOP
;          END REGION
;
;[LIMM Anlaysis]
;MemRefCollection, entries: 1
;  (@B)[0][i1] {  W  } 1W : 0R  legal 
;
; 
; LIMM's Opportunities:
; - LILH:  (0)
; - LISS:  (1)
; - LILHSS:(0)
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:   BEGIN REGION { modified }
; CHECK:         + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:         |   |   (@B)[0][i1] = 2 * i2;
; CHECK:         |   |   if (i2 != 0)
; CHECK:         |   |   {
; CHECK:         |   |      %2 = (@A)[0][i2];
; CHECK:         |   |      (@A)[0][i2] = %2 + 1;
; CHECK:         |   |   }
; CHECK:         |   + END LOOP
; CHECK:         + END LOOP
; CHECK:   END REGION
;
; *** *** 
;          
;  
; CHECK: IR Dump After HIR Loop Memory Motion
;
; CHECK:   BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   %limm = 2 * i2;
; CHECK:        |   |   if (i2 != 0)
; CHECK:        |   |   {
; CHECK:        |   |      %2 = (@A)[0][i2];
; CHECK:        |   |      (@A)[0][i2] = %2 + 1;
; CHECK:        |   |   }
; CHECK:        |   + END LOOP
; CHECK:        |      (@B)[0][i1] = %limm;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc8, %entry
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.inc8 ]
  %0 = shl nsw i64 %indvars.iv22, 1
  %tobool = icmp eq i64 %indvars.iv22, 0
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv22
  %1 = trunc i64 %0 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %1, i32* %arrayidx, align 4, !tbaa !1
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body3
  %2 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %add = add nsw i32 %2, 1
  store i32 %add, i32* %arrayidx5, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %for.inc8, label %for.body3

for.inc8:                                         ; preds = %for.inc
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond25 = icmp eq i64 %indvars.iv.next23, 10
  br i1 %exitcond25, label %for.end10, label %for.cond1.preheader

for.end10:                                        ; preds = %for.inc8
  %3 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %4 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add11 = add i32 %3, 1
  %add12 = add i32 %add11, %4
  ret i32 %add12
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
