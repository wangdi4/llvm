
; This test represents locality cost model checking for matrix multiply.
; The test takes the input of matmul with a different loop ordering
; and outputs the loop cost of each loop inorder of their locality benefit,
; where higher cost is better. The best ordering would be i-k-j where j is
; in the innermost loop.

; Original C/C++ source
; #define N 10
; int64_t A[N][N], B[N][N], C[N][N];
; int64_t foo(int64_t M) {
;     for(int64_t k = 0; k < M; k++) {
;     for(int64_t j = 0; j < M; j++) {
;     for(int64_t i = 0; i < M; i++) {
;         C[i][j] += A[i][k]*B[k][j];
;     }}}
;     return C[2][3];
; }

; REQUIRES: asserts 
; RUN: opt < %s -basicaa -mem2reg -loop-rotate -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-locality-analysis -hir-debug-locality | FileCheck %s
;
; Verify loops in sorted order i(L3)-k(L1)-j(L2) 
; CHECK: Loop level: 3
; CHECK: Loop level: 1
; CHECK: Loop level: 2 

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [10 x [10 x i64]] zeroinitializer, align 16
@B = global [10 x [10 x i64]] zeroinitializer, align 16
@C = global [10 x [10 x i64]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i64 @_Z3fool(i64 %M) #0 {
entry:
  %cmp.39 = icmp sgt i64 %M, 0
  br i1 %cmp.39, label %for.cond.5.preheader.lr.ph, label %for.cond.cleanup

for.cond.5.preheader.lr.ph:                       ; preds = %entry, %for.cond.cleanup.3
  %k.040 = phi i64 [ %inc18, %for.cond.cleanup.3 ], [ 0, %entry ]
  br label %for.body.8.lr.ph

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.3, %entry
  %0 = load i64, i64* getelementptr inbounds ([10 x [10 x i64]], [10 x [10 x i64]]* @C, i64 0, i64 2, i64 3), align 8, !tbaa !1
  ret i64 %0

for.body.8.lr.ph:                                 ; preds = %for.cond.5.preheader.lr.ph, %for.cond.cleanup.7
  %j.037 = phi i64 [ 0, %for.cond.5.preheader.lr.ph ], [ %inc15, %for.cond.cleanup.7 ]
  %arrayidx11 = getelementptr inbounds [10 x [10 x i64]], [10 x [10 x i64]]* @B, i64 0, i64 %k.040, i64 %j.037
  %1 = load i64, i64* %arrayidx11, align 8, !tbaa !1
  br label %for.body.8

for.cond.cleanup.3:                               ; preds = %for.cond.cleanup.7
  %inc18 = add nuw nsw i64 %k.040, 1
  %exitcond43 = icmp eq i64 %inc18, %M
  br i1 %exitcond43, label %for.cond.cleanup, label %for.cond.5.preheader.lr.ph

for.cond.cleanup.7:                               ; preds = %for.body.8
  %inc15 = add nuw nsw i64 %j.037, 1
  %exitcond42 = icmp eq i64 %inc15, %M
  br i1 %exitcond42, label %for.cond.cleanup.3, label %for.body.8.lr.ph

for.body.8:                                       ; preds = %for.body.8, %for.body.8.lr.ph
  %i.035 = phi i64 [ 0, %for.body.8.lr.ph ], [ %inc, %for.body.8 ]
  %arrayidx9 = getelementptr inbounds [10 x [10 x i64]], [10 x [10 x i64]]* @A, i64 0, i64 %i.035, i64 %k.040
  %2 = load i64, i64* %arrayidx9, align 8, !tbaa !1
  %mul = mul nsw i64 %1, %2
  %arrayidx13 = getelementptr inbounds [10 x [10 x i64]], [10 x [10 x i64]]* @C, i64 0, i64 %i.035, i64 %j.037
  %3 = load i64, i64* %arrayidx13, align 8, !tbaa !1
  %add = add nsw i64 %3, %mul
  store i64 %add, i64* %arrayidx13, align 8, !tbaa !1
  %inc = add nuw nsw i64 %i.035, 1
  %exitcond = icmp eq i64 %inc, %M
  br i1 %exitcond, label %for.cond.cleanup.7, label %for.body.8
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
