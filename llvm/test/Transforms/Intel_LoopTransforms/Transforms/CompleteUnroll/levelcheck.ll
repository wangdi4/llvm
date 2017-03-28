; Test for Complete Unrolling which unrolls the innermost loop
; and updates the CanonExpr level to non-linear.

; Source Code
; int foo(int*A, int* B, int64_t M, int64_t K)
; {
;     int64_t i, j, t = 0, N;
;     N = M + K;
;     for(i=0; i<N; i++) {
;       t = B[i];
;       for(j=0; j<5; j++) {
;         A[j+1] = t*j;
;       }}
;   return t;
; }



; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll -hir-details 2>&1 < %s | FileCheck %s

; Traverse to first level loop.
; CHECK: DO i64 i1 = 0

; %0 is defined inside this loop (i1)
; CHECK: (%A)[2] = sext.i32.i64(%0)

; Check presence of non-linear marking.
; CHECK: <RVAL-REG> NON-LINEAR trunc.i64.i32(sext.i32.i64(%0))
; CHECK: <BLOB> NON-LINEAR i32 %0 

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32* nocapture %A, i32* nocapture readonly %B, i64 %M, i64 %K) #0 {
entry:
  %add = add i64 %K, %M
  %cmp.23 = icmp sgt i64 %add, 0
  br i1 %cmp.23, label %for.body, label %for.end.10

for.body:                                         ; preds = %entry, %for.inc.8
  %i.024 = phi i64 [ %inc9, %for.inc.8 ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %i.024
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %conv = sext i32 %0 to i64
  br label %for.body.4

for.body.4:                                       ; preds = %for.body.4, %for.body
  %j.022 = phi i64 [ 0, %for.body ], [ %add6, %for.body.4 ]
  %mul = mul nsw i64 %j.022, %conv
  %conv5 = trunc i64 %mul to i32
  %add6 = add nuw nsw i64 %j.022, 1
  %arrayidx7 = getelementptr inbounds i32, i32* %A, i64 %add6
  store i32 %conv5, i32* %arrayidx7, align 4, !tbaa !1
  %exitcond = icmp eq i64 %add6, 5
  br i1 %exitcond, label %for.inc.8, label %for.body.4

for.inc.8:                                        ; preds = %for.body.4
  %inc9 = add nuw nsw i64 %i.024, 1
  %exitcond25 = icmp eq i64 %inc9, %add
  br i1 %exitcond25, label %for.end.10, label %for.body

for.end.10:                                       ; preds = %for.inc.8, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %0, %for.inc.8 ]
  ret i32 %t.0.lcssa
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1456) (llvm/branches/loopopt 1548)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
