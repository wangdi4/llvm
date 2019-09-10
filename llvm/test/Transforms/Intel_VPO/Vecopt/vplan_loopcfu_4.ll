; Test inner control flow uniformity where the inner loop is a while loop without loop index.

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
; CHECK-NEXT: i1 [[TOPTEST:%vp.*]] = icmp
; CHECK-NEXT: SUCCESSORS(1):{{region[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): {{BB[0-9]+}} {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: REGION: {{region[0-9]+}} (BP: NULL)
; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: Condition({{BB[0-9]+}}): [DA: Divergent] i1 [[TOPTEST]] = icmp
; CHECK-NEXT: SUCCESSORS(2):[[INNERLOOPREGION:loop[0-9]+]](i1 [[TOPTEST]]), {{BB[0-9]+}}(!i1 [[TOPTEST]])
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: REGION: [[INNERLOOPREGION]] (BP: NULL)
; CHECK-NEXT: [[PREHEADER:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: getelementptr
; CHECK-NEXT: trunc
; CHECK-NEXT: SUCCESSORS(1):[[HEADER:BB[0-9]+]]
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: [[HEADER]] (BP: NULL) :
; CHECK-NEXT: phi
; CHECK-NEXT: i1 [[MASKPHI:%vp.*]] = phi  [ i1 [[TOPTEST]], [[PREHEADER]] ],  [ i1 [[BOTTOMTEST:%vp.*]], [[LATCH:BB[0-9]+]] ]
; CHECK-NEXT: SUCCESSORS(1):{{mask_region[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): [[LATCH]] [[PREHEADER]]
; CHECK-EMPTY:

; CHECK-NEXT: REGION: [[MASKREGION:mask_region[0-9]+]] (BP: NULL)
; CHECK-NEXT: [[MASKREGIONENTRY:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: Condition([[HEADER]]): [DA: Divergent] i1 [[MASKPHI]]
; CHECK-NEXT: SUCCESSORS(2):[[LOOPBODYHEADER:BB[0-9]+]](i1 [[MASKPHI]]), [[REGIONEXIT:BB[0-9]+]](!i1 [[MASKPHI]])
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: [[LOOPBODYHEADER]] (BP: NULL) :
; CHECK-NEXT: mul
; CHECK-NEXT: store
; CHECK-NEXT: load
; CHECK-NEXT: SUCCESSORS(1):[[REGIONEXIT]]
; CHECK-NEXT: PREDECESSORS(1): [[MASKREGIONENTRY]]
; CHECK-EMPTY:

; CHECK-NEXT: [[REGIONEXIT]] (BP: NULL) :
; CHECK-NEXT: i1 [[BOTTOMTEST_1:%vp.*]] = icmp
; CHECK-NEXT: i1 [[BOTTOMTEST]] = and i1 [[BOTTOMTEST_1]] i1 [[MASKPHI]]
; CHECK-NEXT: i1 [[ALLZEROCHECK:%vp.*]] = all-zero-check i1 [[BOTTOMTEST]]
; CHECK-NEXT: i1 [[NOTALLZEROCHECK:%vp.*]] = not i1 [[ALLZEROCHECK]]
; CHECK-NEXT: no SUCCESSORS
; CHECK-NEXT: PREDECESSORS(2): [[LOOPBODYHEADER]] [[MASKREGIONENTRY]]
; CHECK-EMPTY:
; CHECK-NEXT: SUCCESSORS(1):[[LATCH]]
; CHECK-NEXT: END Region([[MASKREGION]])
; CHECK-EMPTY:

; CHECK-NEXT: [[LATCH]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: Condition([[REGIONEXIT]]): [DA: Uniform]  i1 [[NOTALLZEROCHECK]] = not
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
define dso_local void @foo(i32* nocapture %a, i32 %m, i32* nocapture readonly %ub, i32 %k) local_unnamed_addr #0 {
entry:
  %cmp15 = icmp sgt i32 %m, 0
  br i1 %cmp15, label %simd.begin, label %for.end

simd.begin:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %while.cond.preheader.preheader

while.cond.preheader.preheader:                   ; preds = %entry
  %wide.trip.count = sext i32 %m to i64
  br label %while.cond.preheader

while.cond.preheader:                             ; preds = %for.inc, %while.cond.preheader.preheader
  %indvars.iv = phi i64 [ 0, %while.cond.preheader.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %ub, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp114 = icmp sgt i32 %0, 0
  br i1 %cmp114, label %while.body.lr.ph, label %for.inc

while.body.lr.ph:                                 ; preds = %while.cond.preheader
  %arrayidx5 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  br label %while.body

while.body:                                       ; preds = %while.body.lr.ph, %while.body
  %2 = phi i32 [ %0, %while.body.lr.ph ], [ %3, %while.body ]
  %mul = mul nsw i32 %2, %1
  store i32 %mul, i32* %arrayidx5, align 4
  %3 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %3, 0
  br i1 %cmp1, label %while.body, label %for.inc.loopexit

for.inc.loopexit:                                 ; preds = %while.body
  br label %for.inc

for.inc:                                          ; preds = %for.inc.loopexit, %while.cond.preheader
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %while.cond.preheader

for.end.loopexit:                                 ; preds = %for.inc
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20869)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
