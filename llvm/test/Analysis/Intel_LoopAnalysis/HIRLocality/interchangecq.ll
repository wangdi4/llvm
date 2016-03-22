; This test represents locality cost model checking for a simple loop.
; The cost model in icc, would allow interchange to happen, whereas this
; should not happen. An icc cq already exist.
; The below example has correct loop ordering.

; Original C/C++ source
; #define N 10
; int64_t B[N][N], A[N][N], C[N];
; int64_t foo(int64_t M) {
;     for(int64_t i = 1; i < M; i++) {
;     for(int64_t j = 1; j < M; j++) {
;        C[i] = C[i]*A[i][j] + B[i][j] - C[i];
;     }}
;    return C[2];}


; TODO: Only runs in debug mode
; REQUIRES: asserts 
; RUN: opt < %s -basicaa -mem2reg -loop-rotate -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-locality-analysis -hir-debug-locality | FileCheck %s
;
; Verify loops in sorted order i(L1)-j(L2)
; CHECK: Loop level: 1
; CHECK: Loop level: 2



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = global [10 x [10 x i64]] zeroinitializer, align 16
@A = global [10 x [10 x i64]] zeroinitializer, align 16
@C = global [10 x i64] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i64 @_Z3fool(i64 %M) #0 {
entry:
  %cmp.29 = icmp sgt i64 %M, 1
  br i1 %cmp.29, label %for.body.4.lr.ph, label %for.cond.cleanup

for.body.4.lr.ph:                                 ; preds = %entry, %for.cond.cleanup.3
  %i.030 = phi i64 [ %inc12, %for.cond.cleanup.3 ], [ 1, %entry ]
  %arrayidx = getelementptr inbounds [10 x i64], [10 x i64]* @C, i64 0, i64 %i.030
  %arrayidx.promoted = load i64, i64* %arrayidx, align 8, !tbaa !1
  br label %for.body.4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.3, %entry
  %0 = load i64, i64* getelementptr inbounds ([10 x i64], [10 x i64]* @C, i64 0, i64 2), align 16, !tbaa !1
  ret i64 %0

for.cond.cleanup.3:                               ; preds = %for.body.4
  store i64 %sub, i64* %arrayidx, align 8, !tbaa !1
  %inc12 = add nuw nsw i64 %i.030, 1
  %exitcond32 = icmp eq i64 %inc12, %M
  br i1 %exitcond32, label %for.cond.cleanup, label %for.body.4.lr.ph

for.body.4:                                       ; preds = %for.body.4, %for.body.4.lr.ph
  %sub28 = phi i64 [ %arrayidx.promoted, %for.body.4.lr.ph ], [ %sub, %for.body.4 ]
  %j.027 = phi i64 [ 1, %for.body.4.lr.ph ], [ %inc, %for.body.4 ]
  %arrayidx6 = getelementptr inbounds [10 x [10 x i64]], [10 x [10 x i64]]* @A, i64 0, i64 %i.030, i64 %j.027
  %1 = load i64, i64* %arrayidx6, align 8, !tbaa !1
  %mul = mul nsw i64 %1, %sub28
  %arrayidx8 = getelementptr inbounds [10 x [10 x i64]], [10 x [10 x i64]]* @B, i64 0, i64 %i.030, i64 %j.027
  %2 = load i64, i64* %arrayidx8, align 8, !tbaa !1
  %add = sub i64 %2, %sub28
  %sub = add i64 %add, %mul
  %inc = add nuw nsw i64 %j.027, 1
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
