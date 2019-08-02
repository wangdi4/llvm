; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: stores only (2 continue stores)
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
;    B[i] = A[i] + 1;
;    B[i + 1] = A[i] - 1;
;  }
;  return A[0] + B[1] + 1;
;}
;
;
; MemRefGroup: {B[i1], B[i1+1]  }
;              MinST
; GapTracker:  {W      W        }
;
;
; CHECK: Function
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:         |   %2 = (@A)[0][i1];
; CHECK:         |   (@B)[0][i1] = %2 + 1;
; CHECK:         |   (@B)[0][i1 + 1] = %2 + -1;
; CHECK:         + END LOOP
; CHECK:   END REGION
;
;
; =====================================================
; testcase not ready yet!
; Note: ScalarRepl transformation result missing!!
; =====================================================
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:        |   %2 = (@A)[0][i1];
; CHECK:        |   %scalarepl = %2 + 1;
; CHECK:        |   (@B)[0][i1] = %scalarepl;
; CHECK:        |   %scalarepl1 = %2 + -1;
; CHECK:        + END LOOP
; CHECK:           (@B)[0][101] = %scalarepl1;
; CHECK:  END REGION
;
; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-scalarrepl-array -disable-output -hir-cg -intel-loop-optreport=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-scalarrepl-array,hir-cg,simplify-cfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -disable-output -intel-loop-optreport=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
;
;OPTREPORT: LOOP BEGIN
;OPTREPORT:     Remark: Number of Array Refs Scalar Replaced In Loop: 2
;OPTREPORT: LOOP END

; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @A, i64 0, i64 0), align 16, !tbaa !1
  %1 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 1), align 4, !tbaa !1
  %add8 = add i32 %0, 1
  %add9 = add i32 %add8, %1
  ret i32 %add9

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %2, 1
  %arrayidx2 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !1
  %sub = add nsw i32 %2, -1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx7 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv.next
  store i32 %sub, i32* %arrayidx7, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20398) (llvm/branches/loopopt 20421)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
