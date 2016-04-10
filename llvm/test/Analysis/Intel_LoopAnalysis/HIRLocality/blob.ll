
; This test represents locality cost model checking for fft2radix example.
; Here, we use blob coeff and const IV coeff to check if the locality can
; output the correct interchange order.

; Original C/C++ Source Code
; #define N 1000
; int64_t C[N];
; int64_t  X[N];
;int64_t foo(int64_t M) {
;    int64_t blob = M/2;
;    int64_t   tmp1, tmp2;
;    for(int64_t i = 1; i < M; i++) {
;         tmp1  = X[2 *i ];
;         tmp2 =  X[2* i +1 ];
;    for(int64_t j = 1; j < M; j++) {
;        C[2* i+  2* blob * j ] = tmp1  +1;
;        C[2*i +   2 *blob*j +1] = tmp2 + 2;
;    }}
;    return C[2]; }


; REQUIRES: asserts
; RUN: opt < %s -basicaa -mem2reg -loop-rotate -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-locality-analysis -hir-debug-locality | FileCheck %s
;
; Verify loops in sorted order j(L2)-i(L1)
; CHECK: Loop level: 2
; CHECK: Loop level: 1


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = global [1000 x i64] zeroinitializer, align 16
@X = global [1000 x i64] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i64 @_Z3fool(i64 %M) #0 {
entry:
  %cmp.41 = icmp sgt i64 %M, 1
  br i1 %cmp.41, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %div = sdiv i64 %M, 2
  %mul9 = shl nsw i64 %div, 1
  br label %for.body.6.lr.ph

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.5, %entry
  %0 = load i64, i64* getelementptr inbounds ([1000 x i64], [1000 x i64]* @C, i64 0, i64 2), align 16, !tbaa !1
  ret i64 %0

for.body.6.lr.ph:                                 ; preds = %for.body.lr.ph, %for.cond.cleanup.5
  %i.042 = phi i64 [ 1, %for.body.lr.ph ], [ %inc21, %for.cond.cleanup.5 ]
  %mul = shl nsw i64 %i.042, 1
  %add = or i64 %mul, 1
  %arrayidx2 = getelementptr inbounds [1000 x i64], [1000 x i64]* @X, i64 0, i64 %add
  %1 = load i64, i64* %arrayidx2, align 8, !tbaa !1
  %arrayidx = getelementptr inbounds [1000 x i64], [1000 x i64]* @X, i64 0, i64 %mul
  %2 = load i64, i64* %arrayidx, align 16, !tbaa !1
  %add7 = add nsw i64 %2, 1
  %add13 = add nsw i64 %1, 2
  br label %for.body.6

for.cond.cleanup.5:                               ; preds = %for.body.6
  %inc21 = add nuw nsw i64 %i.042, 1
  %exitcond43 = icmp eq i64 %inc21, %M
  br i1 %exitcond43, label %for.cond.cleanup, label %for.body.6.lr.ph

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %j.040 = phi i64 [ 1, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %mul10 = mul nsw i64 %mul9, %j.040
  %add11 = add nsw i64 %mul10, %mul
  %arrayidx12 = getelementptr inbounds [1000 x i64], [1000 x i64]* @C, i64 0, i64 %add11
  store i64 %add7, i64* %arrayidx12, align 16, !tbaa !1
  %add18 = or i64 %add11, 1
  %arrayidx19 = getelementptr inbounds [1000 x i64], [1000 x i64]* @C, i64 0, i64 %add18
  store i64 %add13, i64* %arrayidx19, align 8, !tbaa !1
  %inc = add nuw nsw i64 %j.040, 1
  %exitcond = icmp eq i64 %inc, %M
  br i1 %exitcond, label %for.cond.cleanup.5, label %for.body.6
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
