; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-complete-unroll -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
;int A[10];
;int B[10];
;// have an LoopInv Switch in the body
;int foo(void) {
;  int i, j;
;  for (j = 0; j < 4; ++j) {
;    for (i = 0; i < 4; ++i) {
;      switch (B[j]) {
;      case 0:
;        A[j] = A[j] + B[i] + 1;
;        break;
;      case 1:
;        A[j] = A[j] + B[i] + 2;
;        break;
;      default:
;        A[j] = A[j] + B[i] + 3;
;        break;
;      } //end_switch
;      B[i] = B[j] + 1;
;    }
;  }
;  return A[0] + B[1] + 1;
;}
;
;INPUT HIR:
;<56>      + DO i2 = 0, 3, 1   <DO_LOOP>
;<7>       |   %0 = (@B)[0][i1];
;<8>       |   switch(%0)
;<8>       |   {
;<8>       |   case 0:
;<22>      |      %1 = (@A)[0][i1];
;<24>      |      %2 = (@B)[0][i2];
;<27>      |      (@A)[0][i1] = %1 + %2 + 1;
;<28>      |      %arrayidx32.pre-phi = &((@B)[0][i2]);
;<8>       |      break;
;<8>       |   case 1:
;<13>      |      %3 = (@A)[0][i1];
;<15>      |      %4 = (@B)[0][i2];
;<18>      |      (@A)[0][i1] = %3 + %4 + 2;
;<19>      |      %arrayidx32.pre-phi = &((@B)[0][i2]);
;<8>       |      break;
;<8>       |   default:
;<31>      |      %5 = (@A)[0][i1];
;<33>      |      %6 = (@B)[0][i2];
;<36>      |      (@A)[0][i1] = %5 + %6 + 3;
;<37>      |      %arrayidx32.pre-phi = &((@B)[0][i2]);
;<8>       |      break;
;<8>       |   }
;<41>      |   (%arrayidx32.pre-phi)[0] = %0 + 1;
;<56>      + END LOOP
;
;
;
; [LIMM Analysis]
;MemRefCollection, entries: 2
;  (@B)[0][i1] {  R  } 0W : 1R  illegal 
;  (@A)[0][i1] {  R  W  R  W  R  W  } 3W : 3R  illegal 
;
;
; LIMM Opportunities:
; - LILH:  (0)
; - LISS:  (0)
; - LILHSS:(0)
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:    BEGIN REGION { }
; CHECK:        + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:        |   |   %0 = (@B)[0][i1];
; CHECK:        |   |   switch(%0)
; CHECK:        |   |   {
; CHECK:        |   |   case 0:
; CHECK:        |   |      %1 = (@A)[0][i1];
; CHECK:        |   |      %2 = (@B)[0][i2];
; CHECK:        |   |      (@A)[0][i1] = %1 + %2 + 1;
; CHECK:        |   |      %arrayidx32.pre-phi = &((@B)[0][i2]);
; CHECK:        |   |      break;
; CHECK:        |   |   case 1:
; CHECK:        |   |      %3 = (@A)[0][i1];
; CHECK:        |   |      %4 = (@B)[0][i2];
; CHECK:        |   |      (@A)[0][i1] = %3 + %4 + 2;
; CHECK:        |   |      %arrayidx32.pre-phi = &((@B)[0][i2]);
; CHECK:        |   |      break;
; CHECK:        |   |   default:
; CHECK:        |   |      %5 = (@A)[0][i1];
; CHECK:        |   |      %6 = (@B)[0][i2];
; CHECK:        |   |      (@A)[0][i1] = %5 + %6 + 3;
; CHECK:        |   |      %arrayidx32.pre-phi = &((@B)[0][i2]);
; CHECK:        |   |      break;
; CHECK:        |   |   }
; CHECK:        |   |   (%arrayidx32.pre-phi)[0] = %0 + 1;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; *** *** 
;
; CHECK: IR Dump After HIR Loop Memory Motion
;
; CHECK:   BEGIN REGION { }
; CHECK:        + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:        |   |   %0 = (@B)[0][i1];
; CHECK:        |   |   switch(%0)
; CHECK:        |   |   {
; CHECK:        |   |   case 0:
; CHECK:        |   |      %1 = (@A)[0][i1];
; CHECK:        |   |      %2 = (@B)[0][i2];
; CHECK:        |   |      (@A)[0][i1] = %1 + %2 + 1;
; CHECK:        |   |      %arrayidx32.pre-phi = &((@B)[0][i2]);
; CHECK:        |   |      break;
; CHECK:        |   |   case 1:
; CHECK:        |   |      %3 = (@A)[0][i1];
; CHECK:        |   |      %4 = (@B)[0][i2];
; CHECK:        |   |      (@A)[0][i1] = %3 + %4 + 2;
; CHECK:        |   |      %arrayidx32.pre-phi = &((@B)[0][i2]);
; CHECK:        |   |      break;
; CHECK:        |   |   default:
; CHECK:        |   |      %5 = (@A)[0][i1];
; CHECK:        |   |      %6 = (@B)[0][i2];
; CHECK:        |   |      (@A)[0][i1] = %5 + %6 + 3;
; CHECK:        |   |      %arrayidx32.pre-phi = &((@B)[0][i2]);
; CHECK:        |   |      break;
; CHECK:        |   |   }
; CHECK:        |   |   (%arrayidx32.pre-phi)[0] = %0 + 1;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc33, %entry
  %indvars.iv55 = phi i64 [ 0, %entry ], [ %indvars.iv.next56, %for.inc33 ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %indvars.iv55
  %arrayidx21 = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %indvars.iv55
  br label %for.body3

for.body3:                                        ; preds = %sw.epilog, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %sw.epilog ]
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  switch i32 %0, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb11
  ]

sw.bb:                                            ; preds = %for.body3
  %1 = load i32, i32* %arrayidx21, align 4, !tbaa !1
  %arrayidx7 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx7, align 4, !tbaa !1
  %add = add i32 %1, 1
  %add8 = add i32 %add, %2
  store i32 %add8, i32* %arrayidx21, align 4, !tbaa !1
  br label %sw.epilog

sw.bb11:                                          ; preds = %for.body3
  %3 = load i32, i32* %arrayidx21, align 4, !tbaa !1
  %arrayidx15 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx15, align 4, !tbaa !1
  %add16 = add i32 %3, 2
  %add17 = add i32 %add16, %4
  store i32 %add17, i32* %arrayidx21, align 4, !tbaa !1
  br label %sw.epilog

sw.default:                                       ; preds = %for.body3
  %5 = load i32, i32* %arrayidx21, align 4, !tbaa !1
  %arrayidx23 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %indvars.iv
  %6 = load i32, i32* %arrayidx23, align 4, !tbaa !1
  %add24 = add i32 %5, 3
  %add25 = add i32 %add24, %6
  store i32 %add25, i32* %arrayidx21, align 4, !tbaa !1
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb11, %sw.bb
  %arrayidx32.pre-phi = phi i32* [ %arrayidx23, %sw.default ], [ %arrayidx15, %sw.bb11 ], [ %arrayidx7, %sw.bb ]
  %add30 = add nsw i32 %0, 1
  store i32 %add30, i32* %arrayidx32.pre-phi, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc33, label %for.body3

for.inc33:                                        ; preds = %sw.epilog
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond57 = icmp eq i64 %indvars.iv.next56, 4
  br i1 %exitcond57, label %for.end35, label %for.cond1.preheader

for.end35:                                        ; preds = %for.inc33
  %7 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %8 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add36 = add i32 %7, 1
  %add37 = add i32 %add36, %8
  ret i32 %add37
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
