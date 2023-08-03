; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; C Source Code:
;
;int A[100];
;int B[100];
;void foo() {
;  int n, i, j, k;
;  for(n = 0; n < 10; n++){
;    B[n] = A[n] + n;
;    for (i=0; i< 10; i++) {
;      for (j=0; j< 20; j++) {
;        #pragma unroll 5
;        for(k=0;k<5; k++) {
;          A[i+j+k] += B[k];
;        }
;      }
;    }
;  }
;}
;
;*** IR Dump Before HIR Loop Memory Motion ***
;Function: foo
;
;<0>          BEGIN REGION { modified }
;<52>               + DO i1 = 0, 9, 1   <DO_LOOP>
;<3>                |   %0 = (@A)[0][i1];
;<7>                |   (@B)[0][i1] = i1 + %0;
;<53>               |
;<53>               |   + DO i2 = 0, 9, 1   <DO_LOOP>
;<54>               |   |   + DO i3 = 0, 19, 1   <DO_LOOP>
;<57>               |   |   |   %3 = (@B)[0][0];
;<58>               |   |   |   %5 = (@A)[0][i2 + i3];
;<59>               |   |   |   (@A)[0][i2 + i3] = %3 + %5;
;<60>               |   |   |   %3 = (@B)[0][1];
;<61>               |   |   |   %5 = (@A)[0][i2 + i3 + 1];
;<62>               |   |   |   (@A)[0][i2 + i3 + 1] = %3 + %5;
;<63>               |   |   |   %3 = (@B)[0][2];
;<64>               |   |   |   %5 = (@A)[0][i2 + i3 + 2];
;<65>               |   |   |   (@A)[0][i2 + i3 + 2] = %3 + %5;
;<66>               |   |   |   %3 = (@B)[0][3];
;<67>               |   |   |   %5 = (@A)[0][i2 + i3 + 3];
;<68>               |   |   |   (@A)[0][i2 + i3 + 3] = %3 + %5;
;<19>               |   |   |   %3 = (@B)[0][4];
;<22>               |   |   |   %5 = (@A)[0][i2 + i3 + 4];
;<24>               |   |   |   (@A)[0][i2 + i3 + 4] = %3 + %5;
;<54>               |   |   + END LOOP
;<53>               |   + END LOOP
;<52>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;Function: foo
;
;CHECK:      BEGIN REGION { modified }
;CHECK:            + DO i1 = 0, 9, 1   <DO_LOOP>
;CHECK:            |   %0 = (@A)[0][i1];
;CHECK:            |   (@B)[0][i1] = i1 + %0;
;CHECK:            |
;CHECK:            |      %[[LIMM:limm[0-9]*]] = (@B)[0][0];
;CHECK:            |      %[[LIMM2:limm[0-9]*]] = (@B)[0][1];
;CHECK:            |      %[[LIMM3:limm[0-9]*]] = (@B)[0][2];
;CHECK:            |      %[[LIMM4:limm[0-9]*]] = (@B)[0][3];
;CHECK:            |      %[[LIMM5:limm[0-9]*]] = (@B)[0][4];
;CHECK:            |   + DO i2 = 0, 9, 1   <DO_LOOP>
;CHECK:            |   |   + DO i3 = 0, 19, 1   <DO_LOOP>
;CHECK:            |   |   |   %3 = %[[LIMM]];
;CHECK:            |   |   |   %5 = (@A)[0][i2 + i3];
;CHECK:            |   |   |   (@A)[0][i2 + i3] = %3 + %5;
;CHECK:            |   |   |   %3 = %[[LIMM2]];
;CHECK:            |   |   |   %5 = (@A)[0][i2 + i3 + 1];
;CHECK:            |   |   |   (@A)[0][i2 + i3 + 1] = %3 + %5;
;CHECK:            |   |   |   %3 = %[[LIMM3]];
;CHECK:            |   |   |   %5 = (@A)[0][i2 + i3 + 2];
;CHECK:            |   |   |   (@A)[0][i2 + i3 + 2] = %3 + %5;
;CHECK:            |   |   |   %3 = %[[LIMM4]];
;CHECK:            |   |   |   %5 = (@A)[0][i2 + i3 + 3];
;CHECK:            |   |   |   (@A)[0][i2 + i3 + 3] = %3 + %5;
;CHECK:            |   |   |   %3 = %[[LIMM5]];
;CHECK:            |   |   |   %5 = (@A)[0][i2 + i3 + 4];
;CHECK:            |   |   |   (@A)[0][i2 + i3 + 4] = %3 + %5;
;CHECK:            |   |   + END LOOP
;CHECK:            |   + END LOOP
;CHECK:            + END LOOP
;CHECK:      END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc25, %entry
  %indvars.iv55 = phi i64 [ 0, %entry ], [ %indvars.iv.next56, %for.inc25 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv55, !intel-tbaa !2
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %1 = trunc i64 %indvars.iv55 to i32
  %add = add nsw i32 %0, %1
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv55, !intel-tbaa !2
  store i32 %add, ptr %arrayidx2, align 4, !tbaa !2
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.inc22, %for.body
  %indvars.iv52 = phi i64 [ 0, %for.body ], [ %indvars.iv.next53, %for.inc22 ]
  br label %for.cond9.preheader

for.cond9.preheader:                              ; preds = %for.inc19, %for.cond6.preheader
  %indvars.iv48 = phi i64 [ 0, %for.cond6.preheader ], [ %indvars.iv.next49, %for.inc19 ]
  %2 = add nuw nsw i64 %indvars.iv48, %indvars.iv52
  br label %for.body11

for.body11:                                       ; preds = %for.body11, %for.cond9.preheader
  %indvars.iv = phi i64 [ 0, %for.cond9.preheader ], [ %indvars.iv.next, %for.body11 ]
  %arrayidx13 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %3 = load i32, ptr %arrayidx13, align 4, !tbaa !2
  %4 = add nuw nsw i64 %2, %indvars.iv
  %arrayidx17 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %4, !intel-tbaa !2
  %5 = load i32, ptr %arrayidx17, align 4, !tbaa !2
  %add18 = add nsw i32 %5, %3
  store i32 %add18, ptr %arrayidx17, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.inc19, label %for.body11, !llvm.loop !7

for.inc19:                                        ; preds = %for.body11
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond51 = icmp eq i64 %indvars.iv.next49, 20
  br i1 %exitcond51, label %for.inc22, label %for.cond9.preheader

for.inc22:                                        ; preds = %for.inc19
  %indvars.iv.next53 = add nuw nsw i64 %indvars.iv52, 1
  %exitcond54 = icmp eq i64 %indvars.iv.next53, 10
  br i1 %exitcond54, label %for.inc25, label %for.cond6.preheader

for.inc25:                                        ; preds = %for.inc22
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond57 = icmp eq i64 %indvars.iv.next56, 10
  br i1 %exitcond57, label %for.end27, label %for.body

for.end27:                                        ; preds = %for.inc25
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.unroll.count", i32 5}
