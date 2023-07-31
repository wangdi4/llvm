; RUN: opt -passes="mem2reg,loop(loop-rotate),hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;
; [MemRefGroup]: {
;   (@flags1)[0][i1 + 1](W), (@flags1)[0][i1](W), (@flags1)[0][i1 + 1](R)
; }
;
; Note:
; - There is no MaxIndexLoad in this group, since (@flags1)[0][i2 + 1](W) dominates all (@flags1)[0][i2 + 1](R).
; - This testcase is not generated directly from a C source code. Rather, it is produced by hacking LLVM IR.
;   Generating it directly from C source will need to disable various LLVM/HIR passes, which maybe
;   more work than hacking LLVM IR.
; - Line #65 (the load instruction) is the hack inserted, replacing the original use of temp.
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   (@A)[0][i1 + 1] = i1;
; CHECK:        |   (@A)[0][i1] = (@A)[0][i1 + 1];
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           %scalarepl = (@A)[0][0];
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   %scalarepl1 = i1;
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        |   (@A)[0][i1] = %scalarepl;
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        + END LOOP
; CHECK:           (@A)[0][101] = %scalarepl;
; CHECK:  END REGION
;
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i32 undef

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv.next
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx6 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv
  %ld = load i32, ptr %arrayidx, align 4, !tbaa !2
  store i32 %ld, ptr %arrayidx6, align 4, !tbaa !2
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
