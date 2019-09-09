  ; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

; Verify that we properly represent and external definition that is an IV.

;#define N 1600
;int a[N][N];
;int b[N][N];
;
;void foo() {
;
;  for (int i = 0; i < N; ++i) {
;    // Vectorizing here, i is an external definition.
;    for (int j = 0; j < N; ++j) {
;      a[i][j] = b[i][j] * i;
;    }
;  }
;}

; Loop body (context):
;
;  BB4 (BP: NULL) :
;    i64 %vp62560 = phi  [ i64 0, BB3 ],  [ i64 %vp1520, BB7 ]
;    i32* %vp64576 = getelementptr [1600 x [1600 x i32]]* @b i64 0 i64 %i1 i64 %vp62560
;    i32 %vp64784 = load i32* %vp64576
;    i32 %vp448 = trunc i64 %i1
;    i32 %vp640 = mul i32 %vp64784 i32 %vp448
;    i32* %vp1024 = getelementptr [1600 x [1600 x i32]]* @a i64 0 i64 %i1 i64 %vp62560
;    store i32 %vp640 i32* %vp1024
;    i64 %vp1520 = add i64 %vp62560 i64 1
;    i1 %vp1776 = icmp i64 %vp1520 i64 1599
;  SUCCESSORS(1):BB7

; CHECK: i32 [[Load:%vp.*]] = load i32* %vp{{[0-9]*}}
; CHECK: i32 [[Trunc:%vp.*]] = trunc i64 %i1
; CHECK: i32 %vp{{[0-9]*}} = mul i32 [[Load]] i32 [[Trunc]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common dso_local local_unnamed_addr global [1600 x [1600 x i32]] zeroinitializer
@a = common dso_local local_unnamed_addr global [1600 x [1600 x i32]] zeroinitializer

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:
  %iv25 = phi i64 [ 0, %entry ], [ %iv.next26, %for.cond.cleanup3 ]
  %0 = trunc i64 %iv25 to i32
  br label %for.body4

for.cond.cleanup:
  ret void

for.cond.cleanup3:
  %iv.next26 = add nuw nsw i64 %iv25, 1
  %exitcond27 = icmp eq i64 %iv.next26, 1600
  br i1 %exitcond27, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:
  %iv = phi i64 [ 0, %for.cond1.preheader ], [ %iv.next, %for.body4 ]
  %idx6 = getelementptr inbounds [1600 x [1600 x i32]], [1600 x [1600 x i32]]* @b, i64 0, i64 %iv25, i64 %iv, !intel-tbaa !2
  %1 = load i32, i32* %idx6
  %mul = mul nsw i32 %1, %0
  %idx10 = getelementptr inbounds [1600 x [1600 x i32]], [1600 x [1600 x i32]]* @a, i64 0, i64 %iv25, i64 %iv, !intel-tbaa !2
  store i32 %mul, i32* %idx10
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1600
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA1600_A1600_i", !4, i64 0}
!4 = !{!"array@_ZTSA1600_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
