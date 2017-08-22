; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
; Source code:
;int A[1000];
;int B[1000];
;int C[1000];
;
;int foo(void) {
;  int i, j;
;
;  for (j = 0; j < 1000; ++j) {
;    for (i = 0; i < 4; ++i) {
;      B[A[i]] = j;
;    }
;  }
;  return A[0] + B[1] + C[2];
;}
;
;
; INPUT HIR:
;
;          BEGIN REGION { modified }
;<23>            + DO i1 = 0, 999, 1   <DO_LOOP>
;<25>            |   %0 = (@A)[0][0];
;<26>            |   (@B)[0][%0] = i1;
;<27>            |   %0 = (@A)[0][1];
;<28>            |   (@B)[0][%0] = i1;
;<29>            |   %0 = (@A)[0][2];
;<30>            |   (@B)[0][%0] = i1;
;<31>            |   %0 = (@A)[0][3];
;<32>            |   (@B)[0][%0] = i1;
;<23>            + END LOOP
;          END REGION
;
;
;[LIMM Analysis]
;MemRefCollection, entries: _
;
; LIMM's Opportunities:
; - LILH:  (0)
; - LISS:  (0)
; - LILHSS:(0)
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:   BEGIN REGION { modified }
; CHECK:         + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:         |   %0 = (@A)[0][0];
; CHECK:         |   (@B)[0][%0] = i1;
; CHECK:         |   %0 = (@A)[0][1];
; CHECK:         |   (@B)[0][%0] = i1;
; CHECK:         |   %0 = (@A)[0][2];
; CHECK:         |   (@B)[0][%0] = i1;
; CHECK:         |   %0 = (@A)[0][3];
; CHECK:         |   (@B)[0][%0] = i1;
; CHECK:         + END LOOP
; CHECK:   END REGION
;
; *** *** 
;          
; CHECK: IR Dump After HIR Loop Memory Motion
; *** Note: need to revise the IR below, once the logic is implemented *** 
;
; CHECK:   BEGIN REGION { modified }
; CHECK:           %limm = (@A)[0][0];
; CHECK:           %limm2 = (@A)[0][1];
; CHECK:           %limm4 = (@A)[0][2];
; CHECK:           %limm6 = (@A)[0][3];
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %0 = %limm;
; CHECK:        |   (@B)[0][%0] = i1;
; CHECK:        |   %0 = %limm2;
; CHECK:        |   (@B)[0][%0] = i1;
; CHECK:        |   %0 = %limm4;
; CHECK:        |   (@B)[0][%0] = i1;
; CHECK:        |   %0 = %limm6;
; CHECK:        |   (@B)[0][%0] = i1;
; CHECK:        + END LOOP
; CHECK:   END REGION
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc6, %entry
  %j.016 = phi i32 [ 0, %entry ], [ %inc7, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %idxprom4 = sext i32 %0 to i64
  %arrayidx5 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %idxprom4
  store i32 %j.016, i32* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %inc7 = add nuw nsw i32 %j.016, 1
  %exitcond17 = icmp eq i32 %inc7, 1000
  br i1 %exitcond17, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.inc6
  %1 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %2 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add = add nsw i32 %2, %1
  %3 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @C, i64 0, i64 2), align 8, !tbaa !1
  %add9 = add nsw i32 %add, %3
  ret i32 %add9
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20398) (llvm/branches/loopopt 20434)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
