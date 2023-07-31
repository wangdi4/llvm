; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,print<hir>,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
;int A[10000];
;int B[10000];
;int foo(void) {
;  int i, j;
;  for (j = 0; j < 1000; ++j) {
;    for (i = 0; i < 4; ++i) {
;      A[i] = B[i + 1] * 2;
;      B[i] = A[i] + 1;
;    }
;  }
;  return A[0] + B[1] + 1;
;}
;
; INPUT HIR to LIMM:
;
;          BEGIN REGION { modified }
;<26>            + DO i1 = 0, 999, 1   <DO_LOOP>
;<28>            |   %0 = (@B)[0][1];
;<29>            |   (@A)[0][0] = 2 * %0;
;<30>            |   (@B)[0][0] = 2 * %0 + 1;
;<31>            |   %0 = (@B)[0][2];
;<32>            |   (@A)[0][1] = 2 * %0;
;<33>            |   (@B)[0][1] = 2 * %0 + 1;
;<34>            |   %0 = (@B)[0][3];
;<35>            |   (@A)[0][2] = 2 * %0;
;<36>            |   (@B)[0][2] = 2 * %0 + 1;
;<37>            |   %0 = (@B)[0][4];
;<38>            |   (@A)[0][3] = 2 * %0;
;<39>            |   (@B)[0][3] = 2 * %0 + 1;
;<26>            + END LOOP
;          END REGION
;
;[LIMM Analysis]
;MemRefCollection, entries: 9
;  (@B)[0][1] {  R  W  } 1W : 1R  legal
;  (@A)[0][0] {  W  } 1W : 0R  legal
;  (@B)[0][0] {  W  } 1W : 0R  legal
;  (@B)[0][2] {  R  W  } 1W : 1R  legal
;  (@A)[0][1] {  W  } 1W : 0R  legal
;  (@B)[0][3] {  R  W  } 1W : 1R  legal
;  (@A)[0][2] {  W  } 1W : 0R  legal
;  (@B)[0][4] {  R  } 0W : 1R  legal
;  (@A)[0][3] {  W  } 1W : 0R  legal
;
; LIMM's Opportunities:
; - LILH:  (1)
; - LISS:  (5)
; - LILHSS:(3)
;
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %0 = (@B)[0][1];
; CHECK:        |   (@A)[0][0] = 2 * %0;
; CHECK:        |   (@B)[0][0] = 2 * %0 + 1;
; CHECK:        |   %0 = (@B)[0][2];
; CHECK:        |   (@A)[0][1] = 2 * %0;
; CHECK:        |   (@B)[0][1] = 2 * %0 + 1;
; CHECK:        |   %0 = (@B)[0][3];
; CHECK:        |   (@A)[0][2] = 2 * %0;
; CHECK:        |   (@B)[0][2] = 2 * %0 + 1;
; CHECK:        |   %0 = (@B)[0][4];
; CHECK:        |   (@A)[0][3] = 2 * %0;
; CHECK:        |   (@B)[0][3] = 2 * %0 + 1;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; *** ***
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           %limm = (@B)[0][1];
; CHECK:           %limm18 = (@B)[0][2];
; CHECK:           %limm23 = (@B)[0][3];
; CHECK:           %limm28 = (@B)[0][4];
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %0 = %limm;
; CHECK:        |   %limm14 = 2 * %0;
; CHECK:        |   %limm16 = 2 * %0 + 1;
; CHECK:        |   %0 = %limm18;
; CHECK:        |   %limm21 = 2 * %0;
; CHECK:        |   %limm = 2 * %0 + 1;
; CHECK:        |   %0 = %limm23;
; CHECK:        |   %limm26 = 2 * %0;
; CHECK:        |   %limm18 = 2 * %0 + 1;
; CHECK:        |   %0 = %limm28;
; CHECK:        |   %limm30 = 2 * %0;
; CHECK:        |   %limm23 = 2 * %0 + 1;
; CHECK:        + END LOOP
; CHECK:           (@A)[0][3] = %limm30;
; CHECK:           (@A)[0][2] = %limm26;
; CHECK:           (@B)[0][3] = %limm23;
; CHECK:           (@A)[0][1] = %limm21;
; CHECK:           (@B)[0][2] = %limm18;
; CHECK:           (@B)[0][0] = %limm16;
; CHECK:           (@A)[0][0] = %limm14;
; CHECK:           (@B)[0][1] = %limm;
; CHECK:  END REGION
;
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [10000 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [10000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %entry
  %j.025 = phi i32 [ 0, %entry ], [ %inc12, %for.inc11 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [10000 x i32], ptr @B, i64 0, i64 %indvars.iv.next
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !1
  %mul = shl nsw i32 %0, 1
  %arrayidx5 = getelementptr inbounds [10000 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 %mul, ptr %arrayidx5, align 4, !tbaa !1
  %add8 = or i32 %mul, 1
  %arrayidx10 = getelementptr inbounds [10000 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %add8, ptr %arrayidx10, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc11, label %for.body3

for.inc11:                                        ; preds = %for.body3
  %inc12 = add nuw nsw i32 %j.025, 1
  %exitcond26 = icmp eq i32 %inc12, 1000
  br i1 %exitcond26, label %for.end13, label %for.cond1.preheader

for.end13:                                        ; preds = %for.inc11
  %1 = load i32, ptr @A, align 16, !tbaa !1
  %2 = load i32, ptr getelementptr inbounds ([10000 x i32], ptr @B, i64 0, i64 1), align 4, !tbaa !1
  %add14 = add i32 %1, 1
  %add15 = add i32 %add14, %2
  ret i32 %add15
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17966)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
