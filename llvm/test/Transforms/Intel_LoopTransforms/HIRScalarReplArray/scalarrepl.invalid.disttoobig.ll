; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-scalarrepl-array-depdist-threshold=5 < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Sanity Test: skip doing scalar replacement because the dep-distance is too big.
; Note: use -O1 to compile the test.c file, because clang will have done the scalar replacement under -O2.
;
; [REASONS]
; - Applicable: NO (dependence is too big: bigger than 5)
; - Profitable: n/a
; - Legal:      n/a
;
; *** Source Code ***
;
;int A[1000];
;int B[1000];
;int foo(void) {
;  for (int i = 0; i <= 100; ++i) {
;    B[i] = A[i] + A[i + 6];
;  }
;  return A[0] + A[1] + 1;
;}
;
;
;
; CHECK: Function
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:         |   %2 = (@A)[0][i1];
; CHECK:         |   %4 = (@A)[0][i1 + 6];
; CHECK:         |   (@B)[0][i1] = %2 + %4;
; CHECK:         + END LOOP
; CHECK:   END REGION
;
;
; =====================================================
;
; CHECK: Function
;
; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK:         |   %2 = (@A)[0][i1];
; CHECK:         |   %4 = (@A)[0][i1 + 6];
; CHECK:         |   (@B)[0][i1] = %2 + %4;
; CHECK:         + END LOOP
; CHECK:   END REGION
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
@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load i32, ptr @A, align 16, !tbaa !1
  %1 = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @A, i64 0, i64 1), align 4, !tbaa !1
  %add6 = add i32 %0, 1
  %add7 = add i32 %add6, %1
  ret i32 %add7

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx, align 4, !tbaa !1
  %3 = add nuw nsw i64 %indvars.iv, 6
  %arrayidx2 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %3
  %4 = load i32, ptr %arrayidx2, align 4, !tbaa !1
  %add3 = add nsw i32 %4, %2
  %arrayidx5 = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %add3, ptr %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20398) (llvm/branches/loopopt 20421)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
