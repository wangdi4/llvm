; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
; Source code
;struct ab {
;   int a;
;   int b;
;};
;
;struct ab A[50];
;struct ab B[50];
;struct ab T[50];
;int foo(int N){
;  int i, j;
;  for(i = 0; i < N; i++){
;    A[i].a = 0;
;    for(j = 0; j < N; j++){
;      T[j].a = T[j].a + B[j].b;
;    }
;    A[i] = T[i];
;  }
;  return A[0].a;
;}
;
;*** IR Dump Before HIR Dead Store Elimination ***
;
;<0>       BEGIN REGION { }
;<33>            + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
;<3>             |   (@A)[0][i1].0 = 0;
;<34>            |
;<34>            |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
;<8>             |   |   %0 = (@T)[0][i2].0;
;<10>            |   |   %1 = (@B)[0][i2].1;
;<12>            |   |   (@T)[0][i2].0 = %0 + %1;
;<34>            |   + END LOOP
;<34>            |
;<21>            |   %3 = (@T)[0][i1].0;
;<22>            |   (@A)[0][i1].0 = %3;
;<24>            |   %5 = (@T)[0][i1].1;
;<26>            |   (@A)[0][i1].1 = %5;
;<33>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Dead Store Elimination ***
;
; CHECK:   BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
; CHECK-NEXT:   |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
; CHECK:        |   |   %0 = (@T)[0][i2].0;
; CHECK:        |   |   %1 = (@B)[0][i2].1;
; CHECK:        |   |   (@T)[0][i2].0 = %0 + %1;
; CHECK:        |   + END LOOP
; CHECK:        |
; CHECK:        |   %3 = (@T)[0][i1].0;
; CHECK:        |   (@A)[0][i1].0 = %3;
; CHECK:        |   %5 = (@T)[0][i1].1;
; CHECK:        |   (@A)[0][i1].1 = %5;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;Module Before HIR; ModuleID = 'foo1.c'
source_filename = "foo1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ab = type { i32, i32 }

@A = common dso_local local_unnamed_addr global [50 x %struct.ab] zeroinitializer, align 16
@T = common dso_local local_unnamed_addr global [50 x %struct.ab] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [50 x %struct.ab] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %N) local_unnamed_addr #0 {
entry:
  %cmp32 = icmp sgt i32 %N, 0
  br i1 %cmp32, label %for.body.lr.ph, label %for.end18

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.body.lr.ph, %for.end
  %indvars.iv34 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next35, %for.end ]
  %a = getelementptr inbounds [50 x %struct.ab], ptr @A, i64 0, i64 %indvars.iv34, i32 0
  store i32 0, ptr %a, align 8, !tbaa !2
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %a6 = getelementptr inbounds [50 x %struct.ab], ptr @T, i64 0, i64 %indvars.iv, i32 0
  %0 = load i32, ptr %a6, align 8, !tbaa !2
  %b = getelementptr inbounds [50 x %struct.ab], ptr @B, i64 0, i64 %indvars.iv, i32 1
  %1 = load i32, ptr %b, align 4, !tbaa !7
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %a6, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %2 = getelementptr inbounds [50 x %struct.ab], ptr @T, i64 0, i64 %indvars.iv34, i32 0
  %3 = load i32, ptr %2, align 8, !tbaa !8
  store i32 %3, ptr %a, align 8, !tbaa !8
  %4 = getelementptr inbounds [50 x %struct.ab], ptr @T, i64 0, i64 %indvars.iv34, i32 1
  %5 = load i32, ptr %4, align 4, !tbaa !8
  %6 = getelementptr inbounds [50 x %struct.ab], ptr @A, i64 0, i64 %indvars.iv34, i32 1
  store i32 %5, ptr %6, align 4, !tbaa !8
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next35, %wide.trip.count
  br i1 %exitcond37, label %for.end18.loopexit, label %for.body3.preheader

for.end18.loopexit:                               ; preds = %for.end
  br label %for.end18

for.end18:                                        ; preds = %for.end18.loopexit, %entry
  %7 = load i32, ptr @A, align 16, !tbaa !2
  ret i32 %7
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 84727657f42ef4ecbc5a320a2375649ba63c8ffa)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@ab", !4, i64 0, !4, i64 4}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!3, !4, i64 4}
!8 = !{!4, !4, i64 0}
