; C Source
; int arr1[1024], arr2[1024], arr3[1024], arr4[1024];
;
; void foo()
; {
;   int i;
;
; #pragma vector always
;   for (i = 0; i < 1024; i++)
;     arr1[arr2[i]] = arr3[arr4[i]];
; }
;
; Check that loop is vectorized
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -S < %s 2>&1 | FileCheck %s

; CHECK-NOT: DO i1 = 0, 1023, 1{{[[:space:]]}}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr3 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr4 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr1 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr2 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr4, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !0
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr3, i64 0, i64 %idxprom1
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !0
  %arrayidx4 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx4, align 4, !tbaa !0
  %idxprom5 = sext i32 %2 to i64
  %arrayidx6 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %idxprom5
  store i32 %1, i32* %arrayidx6, align 4, !tbaa !0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !5

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!0 = !{!1, !2, i64 0}
!1 = !{!"array@_ZTSA1024_i", !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.vectorize.ignore_profitability"}
!7 = !{!"llvm.loop.vectorize.enable", i1 true}
