; REQUIRES: asserts
; RUN: opt -S < %s -VPlanDriver -debug-only=VPlanPredicator -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

@A = common local_unnamed_addr global [100 x [100 x i64]] zeroinitializer, align 16

define dso_local void @foo(i64 %N, i64 *%a, i64 %mask_out_inner_loop) local_unnamed_addr #0 {
entry:
  %cmp18 = icmp sgt i64 %N, 0
  br i1 %cmp18, label %for.cond1.preheader.preheader, label %for.end7

for.cond1.preheader.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.cond1.preheader

for.cond1.preheader:
  %outer.iv = phi i64 [ %outer.iv.next, %for.inc5 ], [ 0, %for.cond1.preheader.preheader ]
  %skip_loop = icmp eq i64 %outer.iv, %mask_out_inner_loop
  br i1 %skip_loop, label %for.inc5, label %top_test

top_test:
  %cmp216 = icmp eq i64 %outer.iv, 0
  br i1 %cmp216, label %for.inc5, label %for.body3.preheader

for.body3.preheader:
  br label %for.body3

; CHECK-LABEL: Subloop after inner loop control flow transformation
; CHECK-NEXT:    REGION: [[INNER_LOOP:loop[0-9]+]] (BP: NULL)
; CHECK-NEXT:    [[PREHEADER:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[HEADER:BB[0-9]+]]
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:    [[HEADER]] (BP: NULL) :
; CHECK-NEXT:     i64 [[PREV_IV_UPDATE_BLEND:%vp[0-9]*]] = phi  [ i64 [[IV_UPDATE_BLEND:%vp[0-9]*]], BB20 ],  [ i64 undef, [[PREHEADER]] ]
; CHECK-NEXT:     i1 [[PREV_SOME_CMP_BLEND:%vp[0-9]*]] = phi  [ i1 [[SOME_CMP_BLEND:%vp[0-9]*]], BB20 ],  [ i1 undef, BB6 ]
; CHECK-NEXT:     i64 [[PREV_INNER_IV_BLEND:%vp[0-9]*]] = phi [ i64 [[INNER_IV_BLEND:%vp[0-9]*]], BB20 ],  [ i64 undef, [[PREHEADER]] ]
; CHECK-NEXT:     i64 [[INNER_IV:%vp[0-9]*]] = phi  [ i64 [[INNER_IV_NEXT:%vp[0-9]*]], BB20 ],  [ i64 0, [[PREHEADER]] ]
; CHECK-NEXT:     i1 [[INNER_MASK:%vp[0-9]*]] = phi  [ i1 [[INNER_MASK_INPUT:%vp[0-9]*]], BB6 ],  [ i1 [[INNER_MASK_NEXT:%vp[0-9]*]], BB20 ]
; CHECK-NEXT:    SUCCESSORS(1):[[MASK_REGION:.*]]
; CHECK-NEXT:    PREDECESSORS(2): BB20 [[PREHEADER]]
; CHECK-EMPTY:
; CHECK-NEXT:    REGION: [[MASK_REGION]] (BP: NULL)
; CHECK-NEXT:    [[MASK_BB:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:     Condition(BB7): i1 [[INNER_MASK]] = phi  [ i1 [[INNER_MASK_INPUT]], BB6 ],  [ i1 [[INNER_MASK_NEXT]], BB20 ]
; CHECK-NEXT:    SUCCESSORS(2):BB19(i1 [[INNER_MASK]]), BB13(!i1 [[INNER_MASK]])
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:      BB19 (BP: NULL) :
; CHECK-NEXT:       i64* [[GEP:%vp[0-9]*]] = getelementptr inbounds i64* %a i64 [[INNER_IV]]
; CHECK-NEXT:       i64 [[LD:%vp[0-9]*]] = load i64* [[GEP]]
; CHECK-NEXT:       i1 [[SOME_CMP:%vp[0-9]*]] = icmp i64 [[LD]] i64 42
; CHECK-NEXT:       i64 [[INNER_IV_NEXT:%vp[0-9]*]] = add i64 [[INNER_IV]] i64 1
; CHECK-NEXT:      SUCCESSORS(1):BB13
; CHECK-NEXT:      PREDECESSORS(1): [[MASK_BB]]
; CHECK-EMPTY:
; CHECK-NEXT:    BB13 (BP: NULL) :
; CHECK-NEXT:     i1 [[INNER_EXIT_COND:%vp[0-9]*]] = icmp i64 [[INNER_IV_NEXT]] i64 [[OUTER_IV:%vp[0-9]*]]
; CHECK-NEXT:     i1 [[INNER_EXIT_COND_NEG:%vp[0-9]*]] = not i1 [[INNER_EXIT_COND]]
; CHECK-NEXT:     i1 [[INNER_MASK_NEXT]] = and i1 [[INNER_EXIT_COND_NEG]] i1 [[INNER_MASK]]
; CHECK-NEXT:     i64 [[INNER_IV_BLEND]] = select i1 [[INNER_MASK]] i64 [[INNER_IV]] i64 [[PREV_INNER_IV_BLEND]]
; CHECK-NEXT:     i1 [[SOME_CMP_BLEND]] = select i1 [[INNER_MASK]] i1 [[SOME_CMP]] i1 [[PREV_SOME_CMP_BLEND]]
; CHECK-NEXT:     i64 [[IV_UPDATE_BLEND]] = select i1 [[INNER_MASK]] i64 [[INNER_IV_NEXT]] i64 [[PREV_IV_UPDATE_BLEND]]
; CHECK-NEXT:     i1 [[ALL_ZERO_CHECK:%vp[0-9]*]] = all-zero-check i1 [[INNER_MASK_NEXT]]
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(2): BB19 [[MASK_BB]]
; CHECK-EMPTY:
; CHECK-NEXT:    SUCCESSORS(1):BB20
; CHECK-NEXT:    END Region([[MASK_REGION]])
; CHECK-EMPTY:
; CHECK-NEXT:    BB20 (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:     Condition(BB13): i1 [[ALL_ZERO_CHECK]] = all-zero-check i1 [[INNER_MASK_NEXT]]
; CHECK-NEXT:    SUCCESSORS(2):[[EXIT_BB:BB[0-9]+]](i1 [[ALL_ZERO_CHECK]]), [[HEADER]](!i1 [[ALL_ZERO_CHECK]])
; CHECK-NEXT:    PREDECESSORS(1): [[MASK_REGION]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[EXIT_BB]] (BP: NULL) :
; CHECK-NEXT:     i64 {{%vp[0-9]*}} = phi  [ i64 [[INNER_IV_BLEND]], BB20 ]
; CHECK-NEXT:     i64 {{%vp[0-9]*}}  = phi  [ i64 [[IV_UPDATE_BLEND]], BB20 ]
; CHECK-NEXT:     i1 {{%vp[0-9]*}}  = phi  [ i1 [[SOME_CMP_BLEND]], BB20 ]
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(1): BB20
for.body3:
  %inner.iv = phi i64 [ %inner.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %a, i64 %inner.iv
  %ld = load i64, i64* %arrayidx
  %some_cmp = icmp eq i64 %ld, 42
  %inner.iv.next = add nuw nsw i64 %inner.iv, 1
  %exitcond = icmp eq i64 %inner.iv.next, %outer.iv
  br i1 %exitcond, label %for.inc5.loopexit, label %for.body3

for.inc5.loopexit:
  %phi_use = phi i64 [ %inner.iv, %for.body3 ]
  %phi_update_use = phi i64 [ %inner.iv.next, %for.body3 ]
  %no_phi_inst_use = phi i1 [%some_cmp, %for.body3 ]
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
