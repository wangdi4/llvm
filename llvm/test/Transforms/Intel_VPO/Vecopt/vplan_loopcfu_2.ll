; Test inner loop control flow uniformity where inner loop exit condition is divergent memory reference.

; REQUIRES: asserts
; RUN: opt -S %s -VPlanDriver -debug 2>&1 | FileCheck %s

; ModuleID = 'case2.c'
source_filename = "case2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: After inner loop control flow transformation
; CHECK: REGION: {{region[0-9]+}} (BP: NULL)
; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):[[OUTERLOOPREGION:loop[0-9]+]]
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: REGION: [[OUTERLOOPREGION]] (BP: NULL)
; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: phi
; CHECK-NEXT: getelementptr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr
; CHECK-NEXT: load
; CHECK-NEXT: i1 [[TOPTEST:%vp[0-9]+]] = icmp
; CHECK-NEXT: SUCCESSORS(1):{{region[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): {{BB[0-9]+}} {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: REGION: {{region[0-9]+}} (BP: NULL)
; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: Condition({{BB[0-9]+}}): i1 [[TOPTEST]] = icmp
; CHECK-NEXT: SUCCESSORS(2):[[INNERLOOPREGION:loop[0-9]+]](i1 [[TOPTEST]]), {{BB[0-9]+}}(!i1 [[TOPTEST]])
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: REGION: [[INNERLOOPREGION]] (BP: NULL)
; CHECK-NEXT: [[PREHEADER:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):[[HEADER:BB[0-9]+]]
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: [[HEADER]] (BP: NULL) :
; CHECK-NEXT: phi
; CHECK-NEXT: i1 [[MASKPHI:%vp[0-9]+]] = phi  [ i1 [[TOPTEST]], [[PREHEADER]] ],  [ i1 [[BOTTOMTEST:%vp[0-9]+]], [[LATCH:BB[0-9]+]] ]
; CHECK-NEXT: SUCCESSORS(1):{{mask_region[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): [[LATCH]] [[PREHEADER]]
; CHECK-EMPTY:

; CHECK-NEXT: REGION: [[MASKREGION:mask_region[0-9]+]] (BP: NULL)
; CHECK-NEXT: [[MASKREGIONENTRY:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: Condition([[HEADER]]): i1 [[MASKPHI]]
; CHECK-NEXT: SUCCESSORS(2):[[LOOPBODYHEADER:BB[0-9]+]](i1 [[MASKPHI]]), [[REGIONEXIT:BB[0-9]+]](!i1 [[MASKPHI]])
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: [[LOOPBODYHEADER]] (BP: NULL) :
; CHECK-NEXT: shl
; CHECK-NEXT: getelementptr
; CHECK-NEXT: store
; CHECK-NEXT: add
; CHECK-NEXT: load
; CHECK-NEXT: SUCCESSORS(1):[[REGIONEXIT]]
; CHECK-NEXT: PREDECESSORS(1): [[MASKREGIONENTRY]]
; CHECK-EMPTY:

; CHECK-NEXT: [[REGIONEXIT]] (BP: NULL) :
; CHECK-NEXT: i1 [[BOTTOMTEST_1:%vp[0-9]+]] = icmp
; CHECK-NEXT: i1 [[BOTTOMTEST]] = and i1 [[BOTTOMTEST_1]] i1 [[MASKPHI]]
; CHECK-NEXT: i1 [[ALLZEROCHECK:%vp[0-9]+]] = all-zero-check i1 [[BOTTOMTEST]]
; CHECK-NEXT: i1 [[NOTALLZEROCHECK:%vp[0-9]+]] = not i1 [[ALLZEROCHECK]]
; CHECK-NEXT: no SUCCESSORS
; CHECK-NEXT: PREDECESSORS(2): [[LOOPBODYHEADER]] [[MASKREGIONENTRY]]
; CHECK-EMPTY:
; CHECK-NEXT: SUCCESSORS(1):[[LATCH]]
; CHECK-NEXT: END Region([[MASKREGION]])
; CHECK-EMPTY:

; CHECK-NEXT: [[LATCH]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: Condition([[REGIONEXIT]]): i1 [[NOTALLZEROCHECK]] = not
; CHECK-NEXT: SUCCESSORS(2):[[HEADER]](i1 [[NOTALLZEROCHECK]]), [[EXIT:BB[0-9]+]](!i1 [[NOTALLZEROCHECK]])
; CHECK-NEXT: PREDECESSORS(1): [[MASKREGION]]
; CHECK-EMPTY:

; CHECK-NEXT: [[EXIT]] (BP: NULL) :

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

@A = common local_unnamed_addr global [100 x [100 x i64]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i64 %N, i64* nocapture readonly %lb, i64* nocapture readonly %ub) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i64 %N, 0
  br i1 %cmp21, label %for.body.preheader, label %for.end9

for.body.preheader:                               ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc7
  %i.022 = phi i64 [ %inc8, %for.inc7 ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %lb, i64 %i.022
  %0 = load i64, i64* %arrayidx, align 8
  %arrayidx2 = getelementptr inbounds i64, i64* %ub, i64 %i.022
  %1 = load i64, i64* %arrayidx2, align 8
  %cmp319 = icmp slt i64 %0, %1
  br i1 %cmp319, label %for.body4.preheader, label %for.inc7

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %j.020 = phi i64 [ %inc, %for.body4 ], [ %0, %for.body4.preheader ]
  %shl = shl i64 %j.020, 3
  %arrayidx6 = getelementptr inbounds [100 x [100 x i64]], [100 x [100 x i64]]* @A, i64 0, i64 %j.020, i64 %i.022
  store i64 %shl, i64* %arrayidx6, align 8
  %inc = add nsw i64 %j.020, 1
  %2 = load i64, i64* %arrayidx2, align 8
  %cmp3 = icmp slt i64 %inc, %2
  br i1 %cmp3, label %for.body4, label %for.inc7.loopexit

for.inc7.loopexit:                                ; preds = %for.body4
  br label %for.inc7

for.inc7:                                         ; preds = %for.inc7.loopexit, %for.body
  %inc8 = add nuw nsw i64 %i.022, 1
  %exitcond = icmp eq i64 %inc8, %N
  br i1 %exitcond, label %for.end9.loopexit, label %for.body

for.end9.loopexit:                                ; preds = %for.inc7
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20869)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
