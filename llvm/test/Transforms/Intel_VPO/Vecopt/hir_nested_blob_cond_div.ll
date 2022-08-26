; LLVM IR generated from the following testcase (atg_CMPLRS-47273.cpp) using icx -O1 -S -emit-llvm
; #include <stdio.h>
; unsigned long long time_ago = 0;
; unsigned int m = 0;
; unsigned int a [9] = {0};
; int main () {
;     unsigned char i = 0;
;     for (i = 8; i > 0; --i)
;     {
;         unsigned short v = time_ago;
;         m *= (a[i] ? (2) / a[i] : (v));
;     }
;     return 0;
; }
;
; Test that we suppress vectorization for divides in a nested blob for a masked statement
; until we start using SVML for masked divides.
; RUN: opt -enable-new-pm=0 -vplan-force-vf=4 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -enable-blob-coeff-vec -enable-nested-blob-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -enable-blob-coeff-vec -enable-nested-blob-vec -disable-output < %s 2>&1 | FileCheck %s

; CHECK:      DO i1 = 0, 7, 1   <DO_LOOP>
; CHECK:      END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@time_ago = local_unnamed_addr global i64 0, align 8
@m = local_unnamed_addr global i32 0, align 4
@a = local_unnamed_addr global [9 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
  %1 = load i64, i64* @time_ago, align 8
  %conv1 = trunc i64 %1 to i32
  %conv4 = and i32 %conv1, 65535
  %m.promoted = load i32, i32* @m, align 4, !tbaa !0
  br label %2

2:                                                ; preds = %5, %0
  %indvars.iv = phi i64 [ 8, %0 ], [ %indvars.iv.next, %5 ]
  %mul10 = phi i32 [ %m.promoted, %0 ], [ %mul, %5 ]
  %arrayidx = getelementptr inbounds [9 x i32], [9 x i32]* @a, i64 0, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx, align 4, !tbaa !4
  %tobool = icmp eq i32 %3, 0
  br i1 %tobool, label %5, label %4

4:                                                ; preds = %2
  %div = udiv i32 2, %3
  br label %5

5:                                                ; preds = %4, %2
  %cond = phi i32 [ %div, %4 ], [ %conv4, %2 ]
  %mul = mul i32 %mul10, %cond
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %cmp = icmp eq i64 %indvars.iv.next, 0
  br i1 %cmp, label %6, label %2

6:                                                ; preds = %5
  store i32 %mul, i32* @m, align 4, !tbaa !0
  ret i32 0
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C++ TBAA"}
!4 = !{!5, !1, i64 0}
!5 = !{!"array@_ZTSA9_j", !1, i64 0}
