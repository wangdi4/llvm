; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check the following source code is rerolled

; #define SIZE 1000
; int A[SIZE];
; int B[SIZE];
;
; void foo(int n) {
;   int D = n*n;
;   for (int i=0;  i<n; i=i+4) {
;     A[i] = D * B[i];
;     A[i+1] = D * B[i+1];
;     A[i+2] = D * B[i+2];
;     A[i+3] = D * B[i+3];
;   }
; }

; CHECK: Function: foo

; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 250>
; CHECK:        |   %1 = (@B)[0][4 * i1];
; CHECK:        |   (@A)[0][4 * i1] = (%n * %n * %1);
; CHECK:        |   %3 = (@B)[0][4 * i1 + 1];
; CHECK:        |   (@A)[0][4 * i1 + 1] = (%n * %n * %3);
; CHECK:        |   %5 = (@B)[0][4 * i1 + 2];
; CHECK:        |   (@A)[0][4 * i1 + 2] = (%n * %n * %5);
; CHECK:        |   %7 = (@B)[0][4 * i1 + 3];
; CHECK:        |   (@A)[0][4 * i1 + 3] = (%n * %n * %7);
; CHECK:        + END LOOP
; CHECK:  END REGION

; CHECK: Function: foo

; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 4 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK:        |   %1 = (@B)[0][i1];
; CHECK:        |   (@A)[0][i1] = (%n * %n * %1);
; CHECK:        + END LOOP
; CHECK:  END REGION

;Module Before HIR; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n, %n
  %cmp41 = icmp sgt i32 %n, 0
  br i1 %cmp41, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 16, !tbaa !2
  %mul1 = mul nsw i32 %1, %mul
  %arrayidx3 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 %mul1, ptr %arrayidx3, align 16, !tbaa !2
  %2 = or i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %2
  %3 = load i32, ptr %arrayidx5, align 4, !tbaa !2
  %mul6 = mul nsw i32 %3, %mul
  %arrayidx9 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %2
  store i32 %mul6, ptr %arrayidx9, align 4, !tbaa !2
  %4 = or i64 %indvars.iv, 2
  %arrayidx12 = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %4
  %5 = load i32, ptr %arrayidx12, align 8, !tbaa !2
  %mul13 = mul nsw i32 %5, %mul
  %arrayidx16 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %4
  store i32 %mul13, ptr %arrayidx16, align 8, !tbaa !2
  %6 = or i64 %indvars.iv, 3
  %arrayidx19 = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %6
  %7 = load i32, ptr %arrayidx19, align 4, !tbaa !2
  %mul20 = mul nsw i32 %7, %mul
  %arrayidx23 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %6
  store i32 %mul20, ptr %arrayidx23, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6d93f34e605c44d05e5c49346cf267f862c04f87) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bd658c1b76dc5d9cdd488ed945e7eff0a1f708c0)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
