; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; The following source code is not currently rerolled, because of typecasts.

; #define SIZE 10
; int A[SIZE];
; int B[SIZE];
; int C[SIZE];
;
; void foo(int n) {
;   int D = n*n;
;   int q = 0;
;   for (int i=0;  i<n; i=i+4) {
;
;     B[i] = n + i*i;
;
;     B[i+1] = n + (i+1)*(i+1);
;
;     B[i+2] = n + (i+2)*(i+2);
;
;     B[i+3] = n + (i+3)*(i+3);
;   }
;
; }

; <31>            + DO i64 i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; <31>            | <RVAL-REG> LINEAR i64 (sext.i32.i64(%n) + -1)/u4 {sb:2}
; <31>            |    <BLOB> LINEAR i32 %n {sb:4}
; <31>            |
; <3>             |   %mul1 = 4 * i1  *  4 * i1;
; <3>             |   <LVAL-REG> NON-LINEAR i32 %mul1 {sb:6}
; <3>             |   <RVAL-REG> LINEAR trunc.i64.i32(4 * i1) {sb:2}
; <3>             |   <RVAL-REG> LINEAR trunc.i64.i32(4 * i1) {sb:2}
; <3>             |
; <4>             |   %add = %mul1  +  %n;
; <4>             |   <LVAL-REG> NON-LINEAR i32 %add {sb:7}
; <4>             |   <RVAL-REG> NON-LINEAR i32 %mul1 {sb:6} // %mul is a SelfBlob
; <4>             |   <RVAL-REG> LINEAR i32 %n {sb:4}
; <4>             |
; <6>             |   (@B)[0][4 * i1] = %add;
; <6>             |   <LVAL-REG> {al:16}(LINEAR [10 x i32]* @B)[i64 0][LINEAR i64 4 * i1] inbounds  !tbaa !3 {sb:27}
; <6>             |      <BLOB> LINEAR [10 x i32]* @B {sb:9}
; <6>             |   <RVAL-REG> NON-LINEAR i32 %add {sb:7}
; <6>             |
; <8>             |   %3 = 4 * i1 + 1  *  4 * i1 + 1;
; <8>             |   <LVAL-REG> NON-LINEAR i64 %3 {sb:11}
; <8>             |   <RVAL-REG> LINEAR i64 4 * i1 + 1 {sb:2}
; <8>             |   <RVAL-REG> LINEAR i64 4 * i1 + 1 {sb:2}
; <8>             |
; <11>            |   %5 = %3  +  %n;
; <11>            |   <LVAL-REG> NON-LINEAR i32 %5 {sb:14}
; <11>            |   <RVAL-REG> NON-LINEAR trunc.i64.i32(%3) {sb:2} // %3 is not a SelfBlob
; <11>            |      <BLOB> NON-LINEAR i64 %3 {sb:11}
; <11>            |   <RVAL-REG> LINEAR i32 %n {sb:4}
; <11>            |
; <12>            |   (@B)[0][4 * i1 + 1] = %5;
; <12>            |   <LVAL-REG> {al:4}(LINEAR [10 x i32]* @B)[i64 0][LINEAR i64 4 * i1 + 1] inbounds  !tbaa !3 {sb:27}
; <12>            |      <BLOB> LINEAR [10 x i32]* @B {sb:9}
; <12>            |   <RVAL-REG> NON-LINEAR i32 %5 {sb:14}

; List of Refs collected from store <6>:
; (@B)[0][4 * i1],     %n,     4 * i1,     4 * i1      (total 4)
;
; List of Refs collected from store <12>:
; (@B)[0][4 * i1 + 1], %n, 4 * i1 + 1, 4 * i1 + 1, %3  (total 5)

; This example might be rerolled if typecast on %3 can be conveyed backward
; to (4 * i1 + 1)s in <8>. (Can it be done only changing src/dst types of CEs?
; src/dst types of a DDRef cannot be changed by a client.)

;CHECK:  Function: foo
;
;CHECK:        BEGIN REGION { }
;CHECK:              + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
;CHECK:              |   %mul1 = 4 * i1  *  4 * i1;
;CHECK:              |   %add = %mul1  +  %n;
;CHECK:              |   (@B)[0][4 * i1] = %add;
;CHECK:              |   %3 = 4 * i1 + 1  *  4 * i1 + 1;
;CHECK:              |   %5 = %3  +  %n;
;CHECK:              |   (@B)[0][4 * i1 + 1] = %5;
;CHECK:              |   %7 = 4 * i1 + 2  *  4 * i1 + 2;
;CHECK:              |   %9 = %7  +  %n;
;CHECK:              |   (@B)[0][4 * i1 + 2] = %9;
;CHECK:              |   %11 = 4 * i1 + 3  *  4 * i1 + 3;
;CHECK:              |   %13 = %11  +  %n;
;CHECK:              |   (@B)[0][4 * i1 + 3] = %13;
;CHECK:              + END LOOP
;CHECK:        END REGION
;
;CHECK:  Function: foo
;
;CHECK:        BEGIN REGION { }
;CHECK:              + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
;CHECK:              |   %mul1 = 4 * i1  *  4 * i1;
;CHECK:              |   %add = %mul1  +  %n;
;CHECK:              |   (@B)[0][4 * i1] = %add;
;CHECK:              |   %3 = 4 * i1 + 1  *  4 * i1 + 1;
;CHECK:              |   %5 = %3  +  %n;
;CHECK:              |   (@B)[0][4 * i1 + 1] = %5;
;CHECK:              |   %7 = 4 * i1 + 2  *  4 * i1 + 2;
;CHECK:              |   %9 = %7  +  %n;
;CHECK:              |   (@B)[0][4 * i1 + 2] = %9;
;CHECK:              |   %11 = 4 * i1 + 3  *  4 * i1 + 3;
;CHECK:              |   %13 = %11  +  %n;
;CHECK:              |   (@B)[0][4 * i1 + 3] = %13;
;CHECK:              + END LOOP
;CHECK:        END REGION

;Module Before HIR; ModuleID = 'iv-pattern-with-complex-blobs.c'
source_filename = "iv-pattern-with-complex-blobs.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp48 = icmp sgt i32 %n, 0
  br i1 %cmp48, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %mul1 = mul nsw i32 %1, %1
  %add = add nsw i32 %mul1, %n
  %arrayidx = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %add, ptr %arrayidx, align 16, !tbaa !2
  %2 = or i64 %indvars.iv, 1
  %3 = mul nsw i64 %2, %2
  %arrayidx8 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %2
  %4 = trunc i64 %3 to i32
  %5 = add i32 %4, %n
  store i32 %5, ptr %arrayidx8, align 4, !tbaa !2
  %6 = or i64 %indvars.iv, 2
  %7 = mul nsw i64 %6, %6
  %arrayidx15 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %6
  %8 = trunc i64 %7 to i32
  %9 = add i32 %8, %n
  store i32 %9, ptr %arrayidx15, align 8, !tbaa !2
  %10 = or i64 %indvars.iv, 3
  %11 = mul nsw i64 %10, %10
  %arrayidx22 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %10
  %12 = trunc i64 %11 to i32
  %13 = add i32 %12, %n
  store i32 %13, ptr %arrayidx22, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e6de10bf60ed5be7555542cd7b35318c8f7cb851) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 498c08595872ebc415ef62b88f071225808af23b)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
