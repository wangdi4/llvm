; REQUIRES: asserts
; RUN: opt -S < %s -VPlanDriver -vplan-print-after-loop-cfu -disable-output -enable-vp-value-codegen | FileCheck %s
; RUN: opt -S < %s -VPlanDriver -vplan-print-after-loop-cfu -disable-output | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

;; Test for non-LCSSA form when the LCSSA phi is merged into the loop body by
;; merge loop exits transformation. That is happens because Loop Merge Exit
;; transformation wasn't preserving LCSSA form and the test would become useless
;; once this is fixed. In fact, we should fix this and add an assert in the
;; LoopCFU transformation for any non-LCSSA uses.
define dso_local void @foo_non_lcssa(i64 %N, i64 *%a, i64 %mask_out_inner_loop) local_unnamed_addr #0 {
; CHECK:         REGION:
; CHECK:         REGION:
; CHECK:         REGION:
; CHECK:          REGION: loop20 (BP: NULL)
; CHECK-NEXT:     [[PREHEADER:BB[0-9]*]] (BP: NULL) :
; CHECK-NEXT:      <Empty Block>
; CHECK-NEXT:     SUCCESSORS(1):BB7
; CHECK-NEXT:     no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:     BB7 (BP: NULL) :
; CHECK-NEXT:      i64 [[PREV_PHI_USE_BLEND:%vp.*]] = phi  [ i64 [[PHI_USE_BLEND:%vp.*]], BB25 ],  [ i64 undef, [[PREHEADER]] ]
; CHECK-NEXT:      i64 [[PREV_PHI_UPDATE_USE_BLEND:%vp.*]] = phi  [ i64 [[PHI_UPDATE_USE_BLEND:%vp.*]], BB25 ],  [ i64 undef, [[PREHEADER]] ]
; CHECK-NEXT:      i1 [[PREV_NO_PHI_INST_USE_BLEND:%vp.*]] = phi  [ i1 [[NO_PHI_INST_USE_BLEND:%vp.*]], BB25 ],  [ i1 undef, [[PREHEADER]] ]
; CHECK-NEXT:      i64 [[INNER_IV:%vp.*]] = phi  [ i64 [[INNER_IV_NEXT:%vp.*]], BB25 ],  [ i64 0, [[PREHEADER]] ]
; CHECK-NEXT:      i32 [[MERGE_LOOP_EXIT_ARTIFACT:%vp.*]] = phi  [ i32 [[MERGE_LOOP_EXIT_ARTIFACT_NEXT:%vp.*]], BB25 ],  [ i32 0, [[PREHEADER]] ]
; CHECK-NEXT:      i1 [[INNER_MASK:%vp.*]] = phi  [ i1 [[INNER_MASK_INPUT:%vp.*]], [[PREHEADER]] ],  [ i1 [[INNER_MASK_NEXT:%vp.*]], BB25 ]
; CHECK-NEXT:     SUCCESSORS(1):mask_region26
; CHECK-NEXT:     PREDECESSORS(2): BB25 [[PREHEADER]]
; CHECK-EMPTY:
; CHECK-NEXT:     REGION: mask_region26 (BP: NULL)
; CHECK-NEXT:     BB23 (BP: NULL) :
; CHECK-NEXT:      <Empty Block>
; CHECK-NEXT:      Condition(BB7): i1 [[INNER_MASK]] = phi  [ i1 [[INNER_MASK_INPUT]], [[PREHEADER]] ],  [ i1 [[INNER_MASK_NEXT]], BB25 ]
; CHECK-NEXT:     SUCCESSORS(2):BB24(i1 [[INNER_MASK]]), BB17(!i1 [[INNER_MASK]])
; CHECK-NEXT:     no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:       BB24 (BP: NULL) :
; CHECK-NEXT:        i64* [[GEP:%vp.*]] = getelementptr inbounds i64* %a i64 [[INNER_IV]]
; CHECK-NEXT:        i64 [[LD:%vp.*]] = load i64* [[GEP]]
; CHECK-NEXT:        i1 [[SOME_CMP:%vp.*]] = icmp i64 [[LD]] i64 42
; CHECK-NEXT:        i64 [[INNER_IV_NEXT]] = add i64 [[INNER_IV]] i64 1
; CHECK-NEXT:       SUCCESSORS(1):region22
; CHECK-NEXT:       PREDECESSORS(1): BB23
; CHECK-EMPTY:
; CHECK-NEXT:       REGION: region22 (BP: NULL)
; CHECK-NEXT:       BB16 (BP: NULL) :
; CHECK-NEXT:        <Empty Block>
; CHECK-NEXT:        Condition(BB24): i1 [[SOME_CMP]] = icmp i64 [[LD]] i64 42
; CHECK-NEXT:       SUCCESSORS(2):IntermediateBB14(i1 [[SOME_CMP]]), BB9(!i1 [[SOME_CMP]])
; CHECK-NEXT:       no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:         BB9 (BP: NULL) :
; CHECK-NEXT:          i1 [[EXITCOND:%vp.*]] = icmp i64 [[INNER_IV_NEXT]] i64 [[OUTER_IV:%vp.*]]
; CHECK-NEXT:         SUCCESSORS(1):NewLoopLatch13
; CHECK-NEXT:         PREDECESSORS(1): BB16
; CHECK-EMPTY:
; CHECK-NEXT:         IntermediateBB14 (BP: NULL) :
; CHECK-NEXT:          <Empty Block>
; CHECK-NEXT:         SUCCESSORS(1):NewLoopLatch13
; CHECK-NEXT:         PREDECESSORS(1): BB16
; CHECK-EMPTY:
; CHECK-NEXT:       NewLoopLatch13 (BP: NULL) :
; CHECK-NEXT:        i32 [[MERGE_LOOP_EXIT_ARTIFACT_NEXT]] = phi  [ i32 [[MERGE_LOOP_EXIT_ARTIFACT]], BB9 ],  [ i32 1, IntermediateBB14 ]
; CHECK-NEXT:        i1 [[NO_PHI_INST_USE:%vp.*]] = phi  [ i1 [[SOME_CMP]], IntermediateBB14 ],  [ i1 false, BB9 ]
; CHECK-NEXT:        i64 [[PHI_UPDATE_USE:%vp.*]] = phi  [ i64 [[INNER_IV_NEXT]], IntermediateBB14 ],  [ i64 100, BB9 ]
; CHECK-NEXT:        i64 [[PHI_USE:%vp.*]] = phi  [ i64 [[INNER_IV]], IntermediateBB14 ],  [ i64 100, BB9 ]
; CHECK-NEXT:        i1 [[INNER_MERGED_EXIT_COND:%vp.*]] = phi  [ i1 [[EXITCOND]], BB9 ],  [ i1 true, IntermediateBB14 ]
; CHECK-NEXT:       no SUCCESSORS
; CHECK-NEXT:       PREDECESSORS(2): BB9 IntermediateBB14
; CHECK-EMPTY:
; CHECK-NEXT:       SUCCESSORS(1):BB17
; CHECK-NEXT:       END Region(region22)
; CHECK-EMPTY:
; CHECK-NEXT:     BB17 (BP: NULL) :
; CHECK-NEXT:      i1 [[INNER_MERGED_EXIT_COND_NEG:%vp.*]] = not i1 [[INNER_MERGED_EXIT_COND]]
; CHECK-NEXT:      i1 [[INNER_MASK_NEXT]] = and i1 [[INNER_MERGED_EXIT_COND_NEG]] i1 [[INNER_MASK]]
; CHECK-NEXT:      i1 [[NO_PHI_INST_USE_BLEND]] = select i1 [[INNER_MASK]] i1 [[NO_PHI_INST_USE]] i1 [[PREV_NO_PHI_INST_USE_BLEND]]
; CHECK-NEXT:      i64 [[PHI_UPDATE_USE_BLEND]] = select i1 [[INNER_MASK]] i64 [[PHI_UPDATE_USE]] i64 [[PREV_PHI_UPDATE_USE_BLEND]]
; CHECK-NEXT:      i64 [[PHI_USE_BLEND]] = select i1 [[INNER_MASK]] i64 [[PHI_USE]] i64 [[PREV_PHI_USE_BLEND]]
; CHECK-NEXT:      i1 [[ALL_ZERO_CHECK:%vp.*]] = all-zero-check i1 [[INNER_MASK_NEXT]]
; CHECK-NEXT:     no SUCCESSORS
; CHECK-NEXT:     PREDECESSORS(2): region22 BB23
; CHECK-EMPTY:
; CHECK-NEXT:     SUCCESSORS(1):BB25
; CHECK-NEXT:     END Region(mask_region26)
; CHECK-EMPTY:
; CHECK-NEXT:     BB25 (BP: NULL) :
; CHECK-NEXT:      <Empty Block>
; CHECK-NEXT:      Condition(BB17): i1 [[ALL_ZERO_CHECK]] = all-zero-check i1 [[INNER_MASK_NEXT]]
; CHECK-NEXT:     SUCCESSORS(2):BB8(i1 [[ALL_ZERO_CHECK]]), BB7(!i1 [[ALL_ZERO_CHECK]])
; CHECK-NEXT:     PREDECESSORS(1): mask_region26
; CHECK-EMPTY:
; CHECK-NEXT:     BB8 (BP: NULL) :
; CHECK-NEXT:      i64 [[USE_A_LCSSA:%vp.*]] = phi  [ i64 [[PHI_USE_BLEND]], BB25 ]
; CHECK-NEXT:      i64 [[USE_B_LCSSA:%vp.*]] = phi  [ i64 [[PHI_UPDATE_USE_BLEND]], BB25 ]
; CHECK-NEXT:      i1 [[USE_C_LCSSA:%vp.*]] = phi  [ i1 [[NO_PHI_INST_USE_BLEND]], BB25 ]
; CHECK-NEXT:      i64 [[USE_A:%vp.*]] = add i64 [[USE_A_LCSSA]] i64 1
; CHECK-NEXT:      i64 [[USE_B:%vp.*]] = add i64 [[USE_B_LCSSA]] i64 1
; CHECK-NEXT:      i1 [[USE_C:%vp.*]] = xor i1 [[USE_C_LCSSA]] i1 true
; CHECK-NEXT:     no SUCCESSORS
; CHECK-NEXT:     PREDECESSORS(1): BB25
; CHECK-EMPTY:
; CHECK-NEXT:     SUCCESSORS(1):BB4
; CHECK-NEXT:     END Region(loop20)
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

for.body3:
  %inner.iv = phi i64 [ %inner.iv.next, %no_early_exit ], [ 0, %for.body3.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %a, i64 %inner.iv
  %ld = load i64, i64* %arrayidx
  %some_cmp = icmp eq i64 %ld, 42
  %inner.iv.next = add nuw nsw i64 %inner.iv, 1
  br i1 %some_cmp, label %for.inc5.loopexit, label %no_early_exit

no_early_exit:
  %exitcond = icmp eq i64 %inner.iv.next, %outer.iv
  br i1 %exitcond, label %for.inc5.loopexit, label %for.body3

for.inc5.loopexit:
  %phi_use = phi i64 [ %inner.iv, %for.body3 ], [ 100, %no_early_exit ]
  %phi_update_use = phi i64 [ %inner.iv.next, %for.body3 ], [ 100, %no_early_exit ]
  %no_phi_inst_use = phi i1 [%some_cmp, %for.body3 ], [ 100, %no_early_exit ]
  %use_a = add i64 %phi_use, 1
  %use_b = add i64 %phi_update_use, 1
  %use_c = xor i1 %no_phi_inst_use, -1
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
