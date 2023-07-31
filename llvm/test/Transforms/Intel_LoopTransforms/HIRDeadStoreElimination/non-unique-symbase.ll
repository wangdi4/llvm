; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
; Verify that we can perform dead store elimination when we have multiple groups
; with the same symbase (like A[i] and A[i+1]) which do not overlap.
;
; Source code:
;int A[50];
;int B[50];
;int T[50];
;int foo(int N){
;  int i, j;
;  for(i = 0; i < N; i++){
;    A[i] = 0;
;    A[i+1] = B[i];
;    for(j = 0; j < N; j++){
;      T[j] = T[j] + B[j];
;    }
;    A[i] = T[i];
;    A[i+1] = T[i+1];
;  }
;  return A[0];
;}
;
;*** IR Dump Before HIR Dead Store Elimination ***
;<0>       BEGIN REGION { }
;<36>            + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 49>
;<3>             |   (@A)[0][i1] = 0;
;<5>             |   %0 = (@B)[0][i1];
;<8>             |   (@A)[0][i1 + 1] = %0;
;<37>            |
;<37>            |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
;<13>            |   |   %1 = (@T)[0][i2];
;<15>            |   |   %2 = (@B)[0][i2];
;<17>            |   |   (@T)[0][i2] = %1 + %2;
;<37>            |   + END LOOP
;<37>            |
;<26>            |   %3 = (@T)[0][i1];
;<27>            |   (@A)[0][i1] = %3;
;<29>            |   %4 = (@T)[0][i1 + 1];
;<30>            |   (@A)[0][i1 + 1] = %4;
;<36>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Dead Store Elimination ***
;
; CHECK:   BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 49>
; CHECK-NEXT:   |   %0 = (@B)[0][i1];
; CHECK-NEXT:   |
; CHECK:        |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
; CHECK:        |   |   %1 = (@T)[0][i2];
; CHECK:        |   |   %2 = (@B)[0][i2];
; CHECK:        |   |   (@T)[0][i2] = %1 + %2;
; CHECK:        |   + END LOOP
; CHECK:        |
; CHECK:        |   %3 = (@T)[0][i1];
; CHECK:        |   (@A)[0][i1] = %3;
; CHECK:        |   %4 = (@T)[0][i1 + 1];
; CHECK:        |   (@A)[0][i1 + 1] = %4;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;Module Before HIR; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16
@T = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %N) local_unnamed_addr #0 {
entry:
  %cmp45 = icmp sgt i32 %N, 0
  br i1 %cmp45, label %for.body.lr.ph, label %for.end27

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body7.preheader

for.body7.preheader:                              ; preds = %for.body.lr.ph, %for.end
  %indvars.iv47 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next48, %for.end ]
  %arrayidx = getelementptr inbounds [50 x i32], ptr @A, i64 0, i64 %indvars.iv47
  store i32 0, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds [50 x i32], ptr @B, i64 0, i64 %indvars.iv47
  %0 = load i32, ptr %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %arrayidx4 = getelementptr inbounds [50 x i32], ptr @A, i64 0, i64 %indvars.iv.next48
  store i32 %0, ptr %arrayidx4, align 4, !tbaa !2
  br label %for.body7

for.body7:                                        ; preds = %for.body7, %for.body7.preheader
  %indvars.iv = phi i64 [ 0, %for.body7.preheader ], [ %indvars.iv.next, %for.body7 ]
  %arrayidx9 = getelementptr inbounds [50 x i32], ptr @T, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx9, align 4, !tbaa !2
  %arrayidx11 = getelementptr inbounds [50 x i32], ptr @B, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx11, align 4, !tbaa !2
  %add12 = add nsw i32 %2, %1
  store i32 %add12, ptr %arrayidx9, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body7

for.end:                                          ; preds = %for.body7
  %arrayidx16 = getelementptr inbounds [50 x i32], ptr @T, i64 0, i64 %indvars.iv47
  %3 = load i32, ptr %arrayidx16, align 4, !tbaa !2
  store i32 %3, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx21 = getelementptr inbounds [50 x i32], ptr @T, i64 0, i64 %indvars.iv.next48
  %4 = load i32, ptr %arrayidx21, align 4, !tbaa !2
  store i32 %4, ptr %arrayidx4, align 4, !tbaa !2
  %exitcond50 = icmp eq i64 %indvars.iv.next48, %wide.trip.count
  br i1 %exitcond50, label %for.end27.loopexit, label %for.body7.preheader

for.end27.loopexit:                               ; preds = %for.end
  br label %for.end27

for.end27:                                        ; preds = %for.end27.loopexit, %entry
  %5 = load i32, ptr @A, align 16, !tbaa !2
  ret i32 %5
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 84727657f42ef4ecbc5a320a2375649ba63c8ffa)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA50_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

