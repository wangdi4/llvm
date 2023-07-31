; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: stores only (with store gap(s)).
; (In this case, we have B[i+1], B[i+3] as store gaps.)
;
; [REASONS]
; - Applicable: YES
; - Profitable: YES
; - Legal:      YES
;
; *** Source Code ***
;int A[1000];
;int B[1000];
;int foo(void) {
;  for (int i = 0; i <= 100; ++i) {
;    B[i] = i + 1;
;    B[i + 2] = i - 1;
;    B[i + 4] = i + 2;
;  }
;  return A[0] + B[1] + 1;
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   (@B)[0][i1] = i1 + 1;
; CHECK:        |   (@B)[0][i1 + 2] = i1 + 4294967295;
; CHECK:        |   (@B)[0][i1 + 4] = i1 + 2;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; MemRefGroup: { B[i1](W), B[i1+1](G), B[i1+2](W), B[i1+3](G), B[i1+4](W)}
;                MinSt
; GapTracker:  { W         G           W            G           W         }
;
; =====================================================
;
; CHECK: Function
;
;
; CHECK:  BEGIN REGION { modified }
;
; Code in prehdr:
;
; CHECK:           %scalarepl = (@B)[0][0];
; CHECK:           %scalarepl1 = (@B)[0][1];
; CHECK:           %scalarepl2 = (@B)[0][2];
; CHECK:           %scalarepl3 = (@B)[0][3];
;
; Code in loop body:
;
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   %scalarepl = i1 + 1;
; CHECK:        |   (@B)[0][i1] = %scalarepl;
; CHECK:        |   %scalarepl2 = i1 + 4294967295;
; CHECK:        |   %scalarepl4 = i1 + 2;
; CHECK:        |   %scalarepl = %scalarepl1;
; CHECK:        |   %scalarepl1 = %scalarepl2;
; CHECK:        |   %scalarepl2 = %scalarepl3;
; CHECK:        |   %scalarepl3 = %scalarepl4;
; CHECK:        + END LOOP
;
; Code in postexit:
;
; CHECK:           (@B)[0][101] = %scalarepl;
; CHECK:           (@B)[0][102] = %scalarepl1;
; CHECK:           (@B)[0][103] = %scalarepl2;
; CHECK:           (@B)[0][104] = %scalarepl3;
;
; CHECK:  END REGION

; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load i32, ptr @A, align 16, !tbaa !1
  %1 = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @B, i64 0, i64 1), align 4, !tbaa !1
  %add8 = add i32 %0, 1
  %add9 = add i32 %add8, %1
  ret i32 %add9

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv.next to i32
  store i32 %2, ptr %arrayidx, align 4, !tbaa !1
  %3 = add i64 %indvars.iv, 4294967295
  %4 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx3 = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %4
  %5 = trunc i64 %3 to i32
  store i32 %5, ptr %arrayidx3, align 4, !tbaa !1
  %6 = add nuw nsw i64 %indvars.iv, 4
  %arrayidx7 = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %6
  %7 = trunc i64 %4 to i32
  store i32 %7, ptr %arrayidx7, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 21010)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
