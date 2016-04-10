; Test for General unrolling for triangular loop

; Source code:
; #define N 500
; int A[N], B[N];
; int foo(int64_t M) {
;     for(int64_t j=0; j < M; j++) {
;     for(int64_t i = 0; i < j; i++) {
;         A[i] = A[i+1] + B[i];
;     }}
;    return A[2];
; }

; REQUIRES: asserts
; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -S < %s 2>&1 | FileCheck %s

; Check the main unrolled loop.
; CHECK: REGION { modified }
; CHECK: %[[TMP:[a-zA-Z0-9.]+]] = {{.*}}/u8
; CHECK-NEXT: DO i2 = 0, %[[TMP]]
; CHECK: %1 = {al:4}(@A)[0][8 * i2 + 8];
; CHECK: %2 = {al:4}(@B)[0][8 * i2 + 7];
; CHECK: {al:4}(@A)[0][8 * i2 + 7] = %1 + %2;
; CHECK: END LOOP

; Check the remainder loop.
; CHECK-NEXT: DO i2 = 8 * %[[TMP]]
; CHECK-NEXT: %1 = {al:4}(@A)[0][i2 + 1];
; CHECK: END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [500 x i32] zeroinitializer, align 16
@B = global [500 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @_Z3fool(i64 %M) #0 {
entry:
  %cmp.21 = icmp sgt i64 %M, 0
  br i1 %cmp.21, label %for.cond.1.preheader, label %for.cond.cleanup

for.cond.1.preheader:                             ; preds = %entry, %for.cond.cleanup.3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.cond.cleanup.3 ], [ 0, %entry ]
  %cmp2.19 = icmp sgt i64 %indvars.iv, 0
  br i1 %cmp2.19, label %for.body.4, label %for.cond.cleanup.3

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.3, %entry
  %0 = load i32, i32* getelementptr inbounds ([500 x i32], [500 x i32]* @A, i64 0, i64 2), align 8, !tbaa !1
  ret i32 %0

for.cond.cleanup.3:                               ; preds = %for.body.4, %for.cond.1.preheader
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next, %M
  br i1 %exitcond23, label %for.cond.cleanup, label %for.cond.1.preheader

for.body.4:                                       ; preds = %for.cond.1.preheader, %for.body.4
  %i.020 = phi i64 [ %add, %for.body.4 ], [ 0, %for.cond.1.preheader ]
  %add = add nuw nsw i64 %i.020, 1
  %arrayidx = getelementptr inbounds [500 x i32], [500 x i32]* @A, i64 0, i64 %add
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx5 = getelementptr inbounds [500 x i32], [500 x i32]* @B, i64 0, i64 %i.020
  %2 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %add6 = add nsw i32 %2, %1
  %arrayidx7 = getelementptr inbounds [500 x i32], [500 x i32]* @A, i64 0, i64 %i.020
  store i32 %add6, i32* %arrayidx7, align 4, !tbaa !1
  %exitcond = icmp eq i64 %add, %indvars.iv
  br i1 %exitcond, label %for.cond.cleanup.3, label %for.body.4
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
