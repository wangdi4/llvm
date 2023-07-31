; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
; Skip non-linear memrefs case
;
;*** IR Dump Before HIR Dead Store Elimination ***
;
; CHECK:   BEGIN REGION { }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   (@A)[0][i1 + sext.i32.i64(%t.019)] = i1;
; CHECK:        |   %1 = (@B)[0][i1];
; CHECK:        |   %t.019 = %1  +  %t.019;
; CHECK:        |   (@A)[0][i1 + sext.i32.i64(%t.019)] = i1 + 1;
; CHECK:        + END LOOP
; CHECK:   END REGION
;
;*** IR Dump After HIR Dead Store Elimination ***
;
; CHECK:   BEGIN REGION { }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   (@A)[0][i1 + sext.i32.i64(%t.019)] = i1;
; CHECK:        |   %1 = (@B)[0][i1];
; CHECK:        |   %t.019 = %1  +  %t.019;
; CHECK:        |   (@A)[0][i1 + sext.i32.i64(%t.019)] = i1 + 1;
; CHECK:        + END LOOP
; CHECK:   END REGION

;Module Before HIR; ModuleID = 'test2.c'
source_filename = "test2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %t.019 = phi i32 [ 0, %entry ], [ %add3, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %t.019, %0
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom
  store i32 %0, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx2, align 4, !tbaa !2
  %add3 = add nsw i32 %1, %t.019
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %add5 = add nsw i32 %add3, %0
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom6
  %2 = trunc i64 %indvars.iv.next to i32
  store i32 %2, ptr %arrayidx7, align 4, !tbaa !2
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4428ef1d878d7b57b7172aafd6ef156358ec55b2)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
