; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Rerolls since blob and ivs are combined in the top level llvm-instruction.
; ICC does not.

;#define SIZE 10
;#include <stdint.h>
;int64_t A[SIZE];
;int64_t B[SIZE];
;int64_t C[SIZE];
;
;void foo(int n) {
;  int D = n*n;
;  int q = 0;
;  for (int i=0;  i<n; i=i+4) {
;    B[i]   =1 + i + i*i;
;
;    B[i+1] =1 + i+1 + (i+1)*(i+1);
;
;    B[i+2] =1 + i+2 + (i+2)*(i+2);
;
;    B[i+3] =1 + i+3 + (i+3)*(i+3);
;  }
;}

; CHECK:Function: foo

; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:             |   %mul1 = 4 * i1  *  4 * i1;
; CHECK:             |   %add2 = %mul1  +  4 * i1 + 1;
; CHECK:             |   (@B)[0][4 * i1] = trunc.i64.i32(%add2);
; CHECK:             |   %4 = 4 * i1 + 1  *  4 * i1 + 1;
; CHECK:             |   %5 = 4 * i1 + 2  +  %4;
; CHECK:             |   (@B)[0][4 * i1 + 1] = trunc.i64.i32(%5);
; CHECK:             |   %9 = 4 * i1 + 2  *  4 * i1 + 2;
; CHECK:             |   %10 = %9  +  4 * i1 + 3;
; CHECK:             |   (@B)[0][4 * i1 + 2] = trunc.i64.i32(%10);
; CHECK:             |   %12 = 4 * i1 + 3  *  4 * i1 + 3;
; CHECK:             |   %13 = 4 * i1 + 4  +  %12;
; CHECK:             |   (@B)[0][4 * i1 + 3] = trunc.i64.i32(%13);
; CHECK:             + END LOOP
; CHECK:       END REGION

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 4 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8>
; CHECK:              |   %mul1 = i1  *  i1;
; CHECK:              |   %add2 = %mul1  +  i1 + 1;
; CHECK:              |   (@B)[0][i1] = trunc.i64.i32(%add2);
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR; ModuleID = 'new-1.c'
source_filename = "new-1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp56 = icmp sgt i32 %n, 0
  br i1 %cmp56, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = or i64 %indvars.iv, 1
  %mul1 = mul i64 %indvars.iv, %indvars.iv
  %add2 = add i64 %mul1, %1
  %2 = and i64 %add2, 4294967293
  %arrayidx = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i64 %2, ptr %arrayidx, align 16, !tbaa !2
  %3 = add nuw nsw i64 %1, 1
  %4 = mul nsw i64 %1, %1
  %5 = add nuw nsw i64 %3, %4
  %6 = and i64 %5, 4294967295
  %arrayidx12 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %1, !intel-tbaa !2
  store i64 %6, ptr %arrayidx12, align 8, !tbaa !2
  %7 = or i64 %indvars.iv, 3
  %8 = or i64 %indvars.iv, 2
  %9 = mul nsw i64 %8, %8
  %10 = add nuw nsw i64 %9, %7
  %11 = and i64 %10, 4294967295
  %arrayidx22 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %8, !intel-tbaa !2
  store i64 %11, ptr %arrayidx22, align 16, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %12 = mul nsw i64 %7, %7
  %13 = add nuw nsw i64 %indvars.iv.next, %12
  %14 = and i64 %13, 4294967293
  %arrayidx32 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %7, !intel-tbaa !2
  store i64 %14, ptr %arrayidx32, align 8, !tbaa !2
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 81a88af0f8e5fbc44460e0e3d157b6ba6d246190) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 0cafc90f06328d1ee0589dc32b6c293f8f548162)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_l", !4, i64 0}
!4 = !{!"long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
