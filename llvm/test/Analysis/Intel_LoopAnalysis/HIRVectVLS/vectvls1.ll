; This test represents checking of stride per nesting level.

; Original C/C++ source
; int foo(int *restrict a, int *b, int *c, int N){
;   int i,j;
;   int s=0;
;   for (i=0;i<4;i++) {
;     int x1 = b[4 * i + 4];
;     b[4 * i] = N+i;
;     int x2 = b[4 * i + 1];
;     int x3 = b[4 * i + 2];
;     int x4 = b[4 * i + 3];
;     s += (x1+x2+x3+x4);
;   }
;   return s;
; }


; TODO: Only runs in debug mode
; REQUIRES: asserts
; RUN: opt < %s -S -analyze -debug  -hir-vect-vls-analysis -hir-debug-vect-vls 2>&1 | FileCheck %s
;
; CHECK: Printing Groups- Total Groups 3
; CHECK-DAG: AccessMask(per byte, R to L): 1111
; CHECK-DAG: AccessMask(per byte, R to L): 111111111111
; CHECK-DAG: AccessMask(per byte, R to L): 1111


; ModuleID = 'vectvls1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32* noalias nocapture readnone %a, i32* nocapture %b, i32* nocapture readnone %c, i32 %N) #0 {
entry:
  %0 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %s.040 = phi i32 [ 0, %entry ], [ %add20, %for.body ]
  %1 = shl nsw i64 %indvars.iv, 2
  %2 = add nuw nsw i64 %1, 4
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %2
  %3 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %4 = add i64 %indvars.iv, %0
  %arrayidx4 = getelementptr inbounds i32, i32* %b, i64 %1
  %5 = trunc i64 %4 to i32
  store i32 %5, i32* %arrayidx4, align 4, !tbaa !1
  %6 = or i64 %1, 1
  %arrayidx8 = getelementptr inbounds i32, i32* %b, i64 %6
  %7 = load i32, i32* %arrayidx8, align 4, !tbaa !1
  %8 = or i64 %1, 2
  %arrayidx12 = getelementptr inbounds i32, i32* %b, i64 %8
  %9 = load i32, i32* %arrayidx12, align 4, !tbaa !1
  %10 = or i64 %1, 3
  %arrayidx16 = getelementptr inbounds i32, i32* %b, i64 %10
  %11 = load i32, i32* %arrayidx16, align 4, !tbaa !1
  %add17 = add i32 %3, %s.040
  %add18 = add i32 %add17, %7
  %add19 = add i32 %add18, %9
  %add20 = add i32 %add19, %11
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 %add20
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1686) (llvm/branches/loopopt 2023)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
