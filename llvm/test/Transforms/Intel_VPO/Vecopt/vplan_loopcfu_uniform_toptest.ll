; REQUIRES: asserts
; RUN: opt -S < %s -VPlanDriver -disable-output -vplan-print-after-loop-cfu | FileCheck %s

; Ensure uniform top test doesn't cause crashes in LoopCFU transformation.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

@A = common local_unnamed_addr global [100 x [100 x i64]] zeroinitializer, align 16

define dso_local void @foo(i64 %N, i64 *%a, i64 %mask_out_inner_loop) local_unnamed_addr #0 {
; CHECK-LABEL:  After inner loop control flow transformation
; CHECK-NEXT:    REGION: [[REGION0:region[0-9]+]] (BP: NULL)
; CHECK-NEXT:    [[BB0:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[LOOP0:loop[0-9]+]]
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:    REGION: [[LOOP0]] (BP: NULL)
; CHECK-NEXT:    [[BB1:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[BB2:BB[0-9]+]]
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]] (BP: NULL) :
; CHECK-NEXT:     [DA: Divergent] i64 [[VP_OUTER_IV:%.*]] = phi  [ i64 [[VP_OUTER_IV_NEXT:%.*]], [[BB3:BB[0-9]+]] ],  [ i64 0, [[BB1]] ]
; CHECK-NEXT:     [DA: Uniform]   i1 [[VP_UNIFORM_TOP_TEST:%.*]] = icmp i64 [[N0:%.*]] i64 0
; CHECK-NEXT:    SUCCESSORS(1):[[REGION1:region[0-9]+]]
; CHECK-NEXT:    PREDECESSORS(2): [[BB3]] [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    REGION: [[REGION1]] (BP: NULL)
; CHECK-NEXT:    [[BB4:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     [DA: Uniform]   i1 [[VP_UNIFORM_TOP_TEST_NOT:%.*]] = not i1 [[VP_UNIFORM_TOP_TEST]]
; CHECK-NEXT:     Condition([[BB2]]): [DA: Uniform]   i1 [[VP_UNIFORM_TOP_TEST]] = icmp i64 [[N0]] i64 0
; CHECK-NEXT:    SUCCESSORS(2):[[BB5:BB[0-9]+]](i1 [[VP_UNIFORM_TOP_TEST]]), [[LOOP1:loop[0-9]+]](!i1 [[VP_UNIFORM_TOP_TEST]])
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:      REGION: [[LOOP1]] (BP: NULL)
; CHECK-NEXT:      [[BB6:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:       <Empty Block>
; CHECK-NEXT:      SUCCESSORS(1):[[BB7:BB[0-9]+]]
; CHECK-NEXT:      no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB7]] (BP: NULL) :
; CHECK-NEXT:       [DA: Uniform]   i64 [[VP_INNER_IV:%.*]] = phi  [ i64 [[VP_INNER_IV_NEXT:%.*]], [[BB8:BB[0-9]+]] ],  [ i64 0, [[BB6]] ]
; CHECK-NEXT:       [DA: Divergent] i1 [[VP_LOOP_MASK:%.*]] = phi  [ i1 [[VP_UNIFORM_TOP_TEST_NOT]], [[BB6]] ],  [ i1 [[VP_LOOP_MASK_NEXT:%.*]], [[BB8]] ]
; CHECK-NEXT:      SUCCESSORS(1):mask_[[REGION2:region[0-9]+]]
; CHECK-NEXT:      PREDECESSORS(2): [[BB8]] [[BB6]]
; CHECK-EMPTY:
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.cond1.preheader

for.cond1.preheader:
  %outer.iv = phi i64 [ %outer.iv.next, %for.inc5 ], [ 0, %entry ]
  %uniform_top_test = icmp eq i64 %N, 0
  br i1 %uniform_top_test, label %for.inc5, label %for.body3.preheader

for.body3.preheader:
  br label %for.body3

for.body3:
  %inner.iv = phi i64 [ %inner.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %a, i64 %inner.iv
  %inner.iv.next = add nuw nsw i64 %inner.iv, 1
  %exitcond = icmp eq i64 %inner.iv.next, %outer.iv
  br i1 %exitcond, label %for.inc5.loopexit, label %for.body3

for.inc5.loopexit:
  br label %for.inc5

for.inc5:
  %outer.iv.next = add nuw nsw i64 %outer.iv, 1
  %outer_exit_cond = icmp eq i64 %outer.iv.next, %N
  br i1 %outer_exit_cond, label %for.end7.loopexit, label %for.cond1.preheader

for.end7.loopexit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.end7

for.end7:
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20869)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
