; This test represents locality cost model checking for transpose.
; Since more weight is given to write, the interchange should occur.
; Thus, for the below example, j should be the innermost loop.

; Original C/C++ source
; #define N 10
; int64_t C[N][N];
; int64_t D[N][N];
; int64_t foo(int64_t M) {
;   for(int64_t j = 1; j < M; j++) {
;    for(int64_t i = 1; i < M; i++) {
;         D[i][j] = C[j][i];
;     }}
;     return D[2][3];}


; Only run in debug mode.
; REQUIRES: asserts 
; RUN: opt < %s -basicaa -mem2reg -loop-rotate -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-locality-analysis -hir-debug-locality | FileCheck %s
;
; Verify loops in sorted order i(L2)-j(L1)
; CHECK: Loop level: 2
; CHECK: Loop level: 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = global [10 x [10 x i64]] zeroinitializer, align 16
@D = global [10 x [10 x i64]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i64 @_Z3fool(i64 %M) #0 {
entry:
  %cmp.22 = icmp sgt i64 %M, 1
  br i1 %cmp.22, label %for.body.4.lr.ph, label %for.cond.cleanup

for.body.4.lr.ph:                                 ; preds = %entry, %for.cond.cleanup.3
  %j.023 = phi i64 [ %inc9, %for.cond.cleanup.3 ], [ 1, %entry ]
  br label %for.body.4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.3, %entry
  %0 = load i64, i64* getelementptr inbounds ([10 x [10 x i64]], [10 x [10 x i64]]* @D, i64 0, i64 2, i64 3), align 8, !tbaa !1
  ret i64 %0

for.cond.cleanup.3:                               ; preds = %for.body.4
  %inc9 = add nuw nsw i64 %j.023, 1
  %exitcond25 = icmp eq i64 %inc9, %M
  br i1 %exitcond25, label %for.cond.cleanup, label %for.body.4.lr.ph

for.body.4:                                       ; preds = %for.body.4, %for.body.4.lr.ph
  %i.021 = phi i64 [ 1, %for.body.4.lr.ph ], [ %inc, %for.body.4 ]
  %arrayidx5 = getelementptr inbounds [10 x [10 x i64]], [10 x [10 x i64]]* @C, i64 0, i64 %j.023, i64 %i.021
  %1 = load i64, i64* %arrayidx5, align 8, !tbaa !1
  %arrayidx7 = getelementptr inbounds [10 x [10 x i64]], [10 x [10 x i64]]* @D, i64 0, i64 %i.021, i64 %j.023
  store i64 %1, i64* %arrayidx7, align 8, !tbaa !1
  %inc = add nuw nsw i64 %i.021, 1
  %exitcond = icmp eq i64 %inc, %M
  br i1 %exitcond, label %for.cond.cleanup.3, label %for.body.4
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 1049) (llvm/branches/loopopt 1060)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
