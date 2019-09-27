; Test merge loop exits transformation for the case that the back-edge is taken
; if the trip count condition is false.

; REQUIRES: asserts
; RUN: opt -S < %s -VPlanDriver -vplan-force-vf=8 -disable-output -debug-only=VPlanHCFGBuilder 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK-LABEL: Before merge loop exits transformation.
; CHECK-NEXT: VPlan IR for: HCFGBuilder: Plain CFG
; CHECK-EMPTY:
; CHECK-NEXT:   REGION: [[region_1:region[0-9]+]] (BP: NULL)
; CHECK-NEXT:   [[BB_12:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:    <Empty Block>
; CHECK-NEXT:   SUCCESSORS(1):[[BB_2:BB[0-9]+]]
; CHECK-NEXT:   no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:   [[BB_2]] (BP: NULL) :
; CHECK-NEXT:    <Empty Block>
; CHECK-NEXT:   SUCCESSORS(1):[[BB_3:BB[0-9]+]]
; CHECK-NEXT:   PREDECESSORS(1): [[BB_12]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[BB_3]] (BP: NULL) :
; CHECK-NEXT:    i32 [[vp_40240:%vp.*]] = phi  [ i32 0, [[BB_2]] ],  [ i32 [[vp_49152:%vp.*]], [[BB_5:BB[0-9]+]] ]
; CHECK-NEXT:   SUCCESSORS(1):[[LoopPreHeader:BB[0-9]+]]
; CHECK-NEXT:   PREDECESSORS(2): [[BB_5]] [[BB_2]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[LoopPreHeader]] (BP: NULL) :
; CHECK-NEXT:    <Empty Block>
; CHECK-NEXT:   SUCCESSORS(1):[[LoopHeader:BB[0-9]+]]
; CHECK-NEXT:   PREDECESSORS(1): [[BB_3]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[LoopHeader]] (BP: NULL) :
; CHECK-NEXT:    i32 [[vp_42064:%vp.*]] = phi  [ i32 0, [[LoopPreHeader]] ],  [ i32 [[vp_43376:%vp.*]], [[BB_7:BB[0-9]+]] ]
; CHECK-NEXT:    i32 [[vp_43376]] = add i32 [[vp_42064]] i32 1
; CHECK-NEXT:    i1 [[vp_43600:%vp.*]] = icmp i32 [[vp_43376]] i32 16
; CHECK-NEXT:   SUCCESSORS(2):[[BB_6:BB[0-9]+]](i1 [[vp_43600]]), [[OrigLoopLatch:BB[0-9]+]](!i1 [[vp_43600]])
; CHECK-NEXT:   PREDECESSORS(2): [[OrigLoopLatch]] [[LoopPreHeader]]
; CHECK-EMPTY:
; CHECK-NEXT:     [[OrigLoopLatch]] (BP: NULL) :
; CHECK-NEXT:      i1 [[vp_48064:%vp.*]] = icmp i32 [[vp_42064]] i32 128
; CHECK-NEXT:     SUCCESSORS(2):[[BB_8:BB[0-9]+]](i1 [[vp_48064]]), [[LoopHeader]](!i1 [[vp_48064]])
; CHECK-NEXT:     PREDECESSORS(1): [[LoopHeader]]

; CHECK: After merge loop exits transformation.
; CHECK-NEXT: VPlan IR for: HCFGBuilder: Plain CFG
; CHECK-EMPTY:
; CHECK-NEXT:   REGION: [[region_1]] (BP: NULL)
; CHECK-NEXT:   [[BB_12]] (BP: NULL) :
; CHECK-NEXT:    <Empty Block>
; CHECK-NEXT:   SUCCESSORS(1):[[BB_2]]
; CHECK-NEXT:   no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:   [[BB_2]] (BP: NULL) :
; CHECK-NEXT:    <Empty Block>
; CHECK-NEXT:   SUCCESSORS(1):[[BB_3]]
; CHECK-NEXT:   PREDECESSORS(1): [[BB_12]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[BB_3]] (BP: NULL) :
; CHECK-NEXT:    i32 [[vp_40240]] = phi  [ i32 0, [[BB_2]] ],  [ i32 [[vp_49152]], [[BB_5]] ]
; CHECK-NEXT:   SUCCESSORS(1):[[LoopPreHeader]]
; CHECK-NEXT:   PREDECESSORS(2): [[BB_5]] [[BB_2]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[LoopPreHeader]] (BP: NULL) :
; CHECK-NEXT:    <Empty Block>
; CHECK-NEXT:   SUCCESSORS(1):[[LoopHeader]]
; CHECK-NEXT:   PREDECESSORS(1): [[BB_3]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[LoopHeader]] (BP: NULL) :
; CHECK-NEXT:    i32 [[vp_42064]] = phi  [ i32 0, [[LoopPreHeader]] ],  [ i32 [[vp_43376]], [[NewLoopLatch_15:NewLoopLatch[0-9]+]] ]
; CHECK-NEXT:    i32 [[vp_4208:%vp.*]] = phi  [ i32 [[vp_3792:%vp.*]], [[NewLoopLatch_15]] ],  [ i32 0, [[LoopPreHeader]] ]
; CHECK-NEXT:    i32 [[vp_43376]] = add i32 [[vp_42064]] i32 1
; CHECK-NEXT:    i1 [[vp_43600]] = icmp i32 [[vp_43376]] i32 16
; CHECK-NEXT:   SUCCESSORS(2):[[IntermediateBB16:IntermediateBB[0-9]+]](i1 [[vp_43600]]), [[OrigLoopLatch]](!i1 [[vp_43600]])
; CHECK-NEXT:   PREDECESSORS(2): [[NewLoopLatch_15]] [[LoopPreHeader]]
; CHECK-EMPTY:
; CHECK-NEXT:     [[OrigLoopLatch]] (BP: NULL) :
; CHECK-NEXT:      i1 [[vp_48064]] = icmp i32 [[vp_42064]] i32 128
; CHECK-NEXT:     SUCCESSORS(1):[[NewLoopLatch_15]]
; CHECK-NEXT:     PREDECESSORS(1): [[LoopHeader]]
; CHECK-EMPTY:
; One Intermediate basic block is generated since there is only one side-exit.
; CHECK-NEXT:     [[IntermediateBB16]] (BP: NULL) :
; CHECK-NEXT:      <Empty Block>
; CHECK-NEXT:     SUCCESSORS(1):[[NewLoopLatch_15]]
; CHECK-NEXT:     PREDECESSORS(1): [[LoopHeader]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[NewLoopLatch_15]] (BP: NULL) :
; CHECK-NEXT:    i32 [[vp_3792]] = phi  [ i32 [[vp_4208]], [[OrigLoopLatch]] ],  [ i32 1, [[IntermediateBB16]] ]
; The following phi is the new condbit. The backedge is taken when the condbit is false. Thus, vp_48064 is false.
; CHECK-NEXT:    i1 [[vp_4000:%vp.*]] = phi  [ i1 [[vp_48064]], [[OrigLoopLatch]] ],  [ i1 true, [[IntermediateBB16]] ]
; CHECK-NEXT:   SUCCESSORS(2):[[IfBlock_17:IfBlock[0-9]+]](i1 [[vp_4000]]), [[LoopHeader]](!i1 [[vp_4000]])
; CHECK-NEXT:   PREDECESSORS(2): [[OrigLoopLatch]] [[IntermediateBB16]]
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop1

loop1:
  %i = phi i32 [ 0, %entry ], [ %i_inc, %loop1_latch ]
  br label %loop2

loop2:
  %j = phi i32 [ 0, %loop1 ], [ %j_inc, %loop2_latch ]
  %j_inc = add nsw i32 %j, 1
  %cmp1 = icmp eq i32 %j_inc, 16
  br i1 %cmp1, label %bb1, label %loop2_latch

loop2_latch:
  %cmp2 = icmp eq i32 %j, 128
  br i1 %cmp2, label %bb3, label %loop2

bb1:
  br label %bb2

bb2:
  br label %bb4

bb3:
  br label %bb4

bb4:
  br label %loop1_latch

loop1_latch:
  %i_inc = add nsw i32 %i, 1
  %cmp3 = icmp eq i32 %i, 128
  br i1 %cmp3, label %loop1, label %loop1_exit

loop1_exit:
  br label %end

end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21280)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
