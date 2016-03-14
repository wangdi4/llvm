; Test for General unrolling with single level loop
; with non-constant trip count loops.

; Source code:
; #define N 500
; int A[N], B[N];
; int foo(int64_t M) {
;    for(int64_t i = 0; i < M; i++) 
;        A[i] = A[i-1] + B[i];
;    return A[2];
; }

; REQUIRES: asserts
; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -S < %s 2>&1 | FileCheck %s

; Check the main unrolled loop.
; CHECK: REGION { modified }
; CHECK-NEXT: %[[TMP:[a-zA-Z0-9.]+]] = {{.*}}/u8 
; CHECK: DO i1 = 0, %[[TMP]]
; CHECK-NEXT: %2 = (@B)[0][8 * i1];
; CHECK-NEXT: %1 = %2  +  %1;
; CHECK-NEXT: (@A)[0][8 * i1] = %1;
; CHECK: END LOOP

; Check the remainder loop.
; CHECK-NEXT: DO i1 = 8 * %[[TMP]]
; CHECK: (@A)[0][i1] = %1;
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [500 x i32] zeroinitializer, align 16
@B = global [500 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @_Z3fool(i64 %M) #0 {
entry:
  %cmp.8 = icmp sgt i64 %M, 0
  br i1 %cmp.8, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %.pre = load i32, i32* getelementptr ([500 x i32], [500 x i32]* @A, i64 9223372036854775, i64 403), align 4, !tbaa !1
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %0 = load i32, i32* getelementptr inbounds ([500 x i32], [500 x i32]* @A, i64 0, i64 2), align 8, !tbaa !1
  ret i32 %0

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %1 = phi i32 [ %.pre, %for.body.lr.ph ], [ %add, %for.body ]
  %i.09 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %arrayidx1 = getelementptr inbounds [500 x i32], [500 x i32]* @B, i64 0, i64 %i.09
  %2 = load i32, i32* %arrayidx1, align 4, !tbaa !1
  %add = add nsw i32 %2, %1
  %arrayidx2 = getelementptr inbounds [500 x i32], [500 x i32]* @A, i64 0, i64 %i.09
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i.09, 1
  %exitcond = icmp eq i64 %inc, %M
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1250) (llvm/branches/loopopt 1271)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
