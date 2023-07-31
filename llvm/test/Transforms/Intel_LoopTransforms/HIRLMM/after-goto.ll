; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; C Source Code:
;
;int A[1000];
;int C[1000];
;int foo(int N) {
;  int i;
;  for (i = 0; i < N; ++i) {
;      if(A[i] > 0){
;        break;
;      }
;      C[5] = A[i] +1;
;  }
;  return 1;
;}
;
;*** IR Dump Before HIR Loop Memory Motion ***
;
;<0>       BEGIN REGION { }
;<17>            + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1000>
;<3>             |   %1 = (@A)[0][i1];
;<5>             |   if (%1 > 0)
;<5>             |   {
;<6>             |      goto for.end.loopexit;
;<5>             |   }
;<10>            |   (@C)[0][5] = %1 + 1;
;<17>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;
; CHECK:    BEGIN REGION { modified }
; CHECK:           %limm = 0;
; CHECK:        + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1000>
; CHECK:        |   %1 = (@A)[0][i1];
; CHECK:        |   if (%1 > 0)
; CHECK:        |   {
; CHECK:        |      if (i1 != 0)
; CHECK:        |      {
; CHECK:        |         (@C)[0][5] = %limm;
; CHECK:        |      }
; CHECK:        |      goto for.end.loopexit;
; CHECK:        |   }
; CHECK:        |   %limm = %1 + 1;
; CHECK:        + END LOOP
; CHECK:           (@C)[0][5] = %limm;
; CHECK:    END REGION
;
;Module Before HIR; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %N) local_unnamed_addr #0 {
entry:
  %cmp8 = icmp sgt i32 %N, 0
  br i1 %cmp8, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %1, 0
  br i1 %cmp1, label %for.end.loopexit, label %if.end

if.end:                                           ; preds = %for.body
  %add = add nsw i32 %1, 1
  store i32 %add, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 5), align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body, %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 1
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang fc25755a7dd8cc64339b342d0cba7a81391fbd6e) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d501475078e65b9fddb732ee8c0291f04fef5546)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
