; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
;int A[100];
;int B[100];
;// have an LoopInv Load in Switch
;int foo(void) {
;  int i, j;
;
;  for (j = 0; j < 10; ++j) {
;    for (i = 0; i < 20; ++i) {
;      switch (B[i]) {
;      case 0: A[j] = A[j] + B[i] + 1; break;
;      case 1: A[j] = A[j] + B[i] + 2; break;
;      default:A[j] = A[j] + B[i] + 3; break;
;      }
;      //end_switch
;    }
;  }
;
;  return A[0] + B[1] + 1;
;}
;
;[LIMM Analysis]
;MemRefCollection, entries: 1
;  (@B)[0][i1] {  R  } 0W : 1R  legal 
;
;
; [LIMM's Opportunities]
; - LILH:  (1)
; - LISS:  (0)
; - LILHSS:(0)
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   %0 = (@B)[0][i1];
; CHECK:        |   |   switch(%0)
; CHECK:        |   |   {
; CHECK:        |   |   case 0:
; CHECK:        |   |      %1 = (@A)[0][i2];
; CHECK:        |   |      (@A)[0][i2] = %1 + 1;
; CHECK:        |   |      break;
; CHECK:        |   |   case 1:
; CHECK:        |   |      %2 = (@A)[0][i2];
; CHECK:        |   |      (@A)[0][i2] = %2 + 3;
; CHECK:        |   |      break;
; CHECK:        |   |   default:
; CHECK:        |   |      %3 = (@A)[0][i2];
; CHECK:        |   |      (@A)[0][i2] = %0 + %3 + 3;
; CHECK:        |   |      break;
; CHECK:        |   |   }
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; *** *** 
;          
; CHECK: IR Dump After HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:        |      %limm = (@B)[0][i1];
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   %0 = %limm;
; CHECK:        |   |   switch(%0)
; CHECK:        |   |   {
; CHECK:        |   |   case 0:
; CHECK:        |   |      %1 = (@A)[0][i2];
; CHECK:        |   |      (@A)[0][i2] = %1 + 1;
; CHECK:        |   |      break;
; CHECK:        |   |   case 1:
; CHECK:        |   |      %2 = (@A)[0][i2];
; CHECK:        |   |      (@A)[0][i2] = %2 + 3;
; CHECK:        |   |      break;
; CHECK:        |   |   default:
; CHECK:        |   |      %3 = (@A)[0][i2];
; CHECK:        |   |      (@A)[0][i2] = %0 + %3 + 3;
; CHECK:        |   |      break;
; CHECK:        |   |   }
; CHECK:        |   + END LOOP
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

for.cond1.preheader:                              ; preds = %for.inc28, %entry
  %indvars.iv49 = phi i64 [ 0, %entry ], [ %indvars.iv.next50, %for.inc28 ]
  %arrayidx21 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv49
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  switch i32 %0, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb11
  ]

sw.bb:                                            ; preds = %for.body3
  %1 = load i32, i32* %arrayidx21, align 4, !tbaa !1
  %add8 = add i32 %1, 1
  store i32 %add8, i32* %arrayidx21, align 4, !tbaa !1
  br label %for.inc

sw.bb11:                                          ; preds = %for.body3
  %2 = load i32, i32* %arrayidx21, align 4, !tbaa !1
  %add17 = add i32 %2, 3
  store i32 %add17, i32* %arrayidx21, align 4, !tbaa !1
  br label %for.inc

sw.default:                                       ; preds = %for.body3
  %3 = load i32, i32* %arrayidx21, align 4, !tbaa !1
  %add24 = add i32 %0, 3
  %add25 = add i32 %add24, %3
  store i32 %add25, i32* %arrayidx21, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %sw.bb, %sw.bb11, %sw.default
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %for.inc28, label %for.body3

for.inc28:                                        ; preds = %for.inc
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond51 = icmp eq i64 %indvars.iv.next50, 10
  br i1 %exitcond51, label %for.end30, label %for.cond1.preheader

for.end30:                                        ; preds = %for.inc28
  %4 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %5 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add31 = add i32 %4, 1
  %add32 = add i32 %add31, %5
  ret i32 %add32
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
