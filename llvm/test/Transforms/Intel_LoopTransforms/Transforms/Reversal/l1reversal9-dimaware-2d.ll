; Sanity Test(s) on HIR Loop Reversal: l1 that is dimension aware (2Dimensional case)
; 
; l1reversal9-dimaware-2d.ll: 
; 1-level loop, sanity testcase, that is dimension-aware on 2D array
; 
; Loop is NOT GOOD for reversal,
; 
; [REASONS]
; - Prelimary:   _
; - Applicable:  _
; - Profitable:  _
; - Legal:       _
; 
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;// stride-aware 2D
;volatile int A[100][100];
;volatile int B[100][100];
;
;int foo(void) {
;  int i = 0;
;
;  //loop i
;  for (i = 0; i <= 4; i++) {
;    A[10 - i][2] = B[10 - i][i] -i;
;    A[1][20 - i] = B[20 - i][2] + 2*i;
;  }
;
;  return A[0][0] + B[1][1] + 1;
;}
;
; [AFTER LOOP REVERSAL: NOT reversed]
; 
;// stride-aware 2D
;volatile int A[100][100];
;volatile int B[100][100];
;
;int foo(void) {
;  int i = 0;
;
;  //loop i
;  for (i = 0; i <= 4; i++) {
;    A[10 - i][2] = B[10 - i][i] -i;
;    A[1][20 - i] = B[20 - i][2] + 2*i;
;  }
;
;  return A[0][0] + B[1][1] + 1;
;}
;
; 
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-cost-model-throttling=0 -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal, DOESN'T REVERSE anything ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-cost-model-throttling=0 -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=AFTER 
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
; 
; BEFORE:  BEGIN REGION { }
; BEFORE:        + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:        |   %1 = {vol}(@B)[0][-1 * i1 + 10][i1];
; BEFORE:        |   {vol}(@A)[0][-1 * i1 + 10][2] = -1 * i1 + %1;
; BEFORE:        |   %4 = {vol}(@B)[0][-1 * i1 + 20][2];
; BEFORE:        |   {vol}(@A)[0][1][-1 * i1 + 20] = 2 * i1 + %4;
; BEFORE:         + END LOOP
; BEFORE:  END REGION
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
; *** NEED TO TUNE THE COST MODEL !!!        ***
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal 
;
; AFTER:  BEGIN REGION { }
; AFTER:        + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:        |   %1 = {vol}(@B)[0][-1 * i1 + 10][i1];
; AFTER:        |   {vol}(@A)[0][-1 * i1 + 10][2] = -1 * i1 + %1;
; AFTER:        |   %4 = {vol}(@B)[0][-1 * i1 + 20][2];
; AFTER:        |   {vol}(@A)[0][1][-1 * i1 + 20] = 2 * i1 + %4;
; AFTER:         + END LOOP
; AFTER:  END REGION
; 
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [100 x [100 x i32]] zeroinitializer, align 16
@A = common global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = sub nuw nsw i64 10, %indvars.iv
  %arrayidx2 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %0, i64 %indvars.iv
  %1 = load volatile i32, i32* %arrayidx2, align 4, !tbaa !1
  %2 = trunc i64 %indvars.iv to i32
  %sub3 = sub nsw i32 %1, %2
  %arrayidx7 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %0, i64 2
  store volatile i32 %sub3, i32* %arrayidx7, align 8, !tbaa !1
  %3 = sub nuw nsw i64 20, %indvars.iv
  %arrayidx11 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %3, i64 2
  %4 = load volatile i32, i32* %arrayidx11, align 8, !tbaa !1
  %5 = shl i64 %indvars.iv, 1
  %6 = trunc i64 %5 to i32
  %add = add nsw i32 %4, %6
  %arrayidx14 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 1, i64 %3
  store volatile i32 %add, i32* %arrayidx14, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %7 = load volatile i32, i32* getelementptr inbounds ([100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 0, i64 0), align 16, !tbaa !1
  %8 = load volatile i32, i32* getelementptr inbounds ([100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 1, i64 1), align 4, !tbaa !1
  %add15 = add i32 %7, 1
  %add16 = add i32 %add15, %8
  ret i32 %add16
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12266)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
