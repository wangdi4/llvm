
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Some operations in RHS are not conforming.

; #define SIZE 10
; #include <stdint.h>
; int64_t A[SIZE];
; int64_t B[SIZE];
; int64_t C[SIZE];
;
; void foo(int n) {
;   int D = n*n;
;   int q = 0;
;   for (int i=0;  i<n; i=i+4) {
;
;     B[i]   = n + i*i;
;
;     B[i+1] = n - (i+1)*(i+1);  // '-'
;
;     B[i+2] = n + (i+2)*(i+2);
;
;     B[i+3] = n + (i+3)*(i+3);
;   }
; }


; CHECK: Function: foo
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:        |   %1 = 4 * i1  *  4 * i1;
; CHECK:        |   %2 = %1  +  %n;
; CHECK:        |   (@B)[0][4 * i1] = %2;
; CHECK:        |   %4 = 4 * i1 + 1  *  4 * i1 + 1;
; CHECK:        |   %5 = %n  -  %4;
; CHECK:        |   (@B)[0][4 * i1 + 1] = %5;
; CHECK:        |   %7 = 4 * i1 + 2  *  4 * i1 + 2;
; CHECK:        |   %8 = %7  +  %n;
; CHECK:        |   (@B)[0][4 * i1 + 2] = %8;
; CHECK:        |   %10 = 4 * i1 + 3  *  4 * i1 + 3;
; CHECK:        |   %11 = %10  +  %n;
; CHECK:        |   (@B)[0][4 * i1 + 3] = %11;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; CHECK: Function: foo
;
; CHECK:    BEGIN REGION { }
; CHECK:          + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:          |   %1 = 4 * i1  *  4 * i1;
; CHECK:          |   %2 = %1  +  %n;
; CHECK:          |   (@B)[0][4 * i1] = %2;
; CHECK:          |   %4 = 4 * i1 + 1  *  4 * i1 + 1;
; CHECK:          |   %5 = %n  -  %4;
; CHECK:          |   (@B)[0][4 * i1 + 1] = %5;
; CHECK:          |   %7 = 4 * i1 + 2  *  4 * i1 + 2;
; CHECK:          |   %8 = %7  +  %n;
; CHECK:          |   (@B)[0][4 * i1 + 2] = %8;
; CHECK:          |   %10 = 4 * i1 + 3  *  4 * i1 + 3;
; CHECK:          |   %11 = %10  +  %n;
; CHECK:          |   (@B)[0][4 * i1 + 3] = %11;
; CHECK:          + END LOOP
; CHECK:    END REGION

;Module Before HIR; ModuleID = 'minus.c'
source_filename = "minus.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16

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
  %1 = mul nsw i64 %indvars.iv, %indvars.iv
  %2 = add nsw i64 %1, %0
  %arrayidx = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %indvars.iv
  store i64 %2, ptr %arrayidx, align 16, !tbaa !2
  %3 = or i64 %indvars.iv, 1
  %4 = mul nsw i64 %3, %3
  %5 = sub nsw i64 %0, %4
  %arrayidx8 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %3
  store i64 %5, ptr %arrayidx8, align 8, !tbaa !2
  %6 = or i64 %indvars.iv, 2
  %7 = mul nsw i64 %6, %6
  %8 = add nsw i64 %7, %0
  %arrayidx16 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %6
  store i64 %8, ptr %arrayidx16, align 16, !tbaa !2
  %9 = or i64 %indvars.iv, 3
  %10 = mul nsw i64 %9, %9
  %11 = add nsw i64 %10, %0
  %arrayidx24 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %9
  store i64 %11, ptr %arrayidx24, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e6de10bf60ed5be7555542cd7b35318c8f7cb851) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 1288f20472d2bee9e7b78f36105668969392d751)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_l", !4, i64 0}
!4 = !{!"long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
