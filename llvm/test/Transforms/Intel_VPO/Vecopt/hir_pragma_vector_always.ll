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
; ModuleID = 'tva.c'
; Check that loop is vectorized
; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -S < %s 2>&1 | FileCheck %s

; CHECK-NOT: DO i1 = 0, 1023, 1{{[[:space:]]}}
source_filename = "tva.c"
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
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr3, i64 0, i64 %idxprom1
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %arrayidx4 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx4, align 4, !tbaa !2
  %idxprom5 = sext i32 %2 to i64
  %arrayidx6 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %idxprom5
  store i32 %1, i32* %arrayidx6, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !7

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 2a99f65e666939ed83d39310be8421f2dd32c6a3) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm c8989f04f7ccc7b7412ec77d41e2bebcb62e0c8f)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.vectorize.ignore_profitability"}
!9 = !{!"llvm.loop.vectorize.enable", i1 true}
