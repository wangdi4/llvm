; Tests if the DA information of the bottom test (NewLoopLatch15) is updated
; correctly in merge loop exits transformation.
; The exit condition of loop2 (BB7 in checks) is uniform and the exit condition
; of bb1 (BB8 in checks) is divergent. Thus, the bottom test should be divergent.

; REQUIRES: asserts
; RUN: opt -S < %s -VPlanDriver -disable-vplan-codegen -vplan-print-after-hcfg -debug-only=VPlanPredicator 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK-LABEL:  Print after building H-CFG:
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
; CHECK-NEXT:     i32 [[VP_I:%.*]] = phi  [ i32 0, [[BB1]] ],  [ i32 [[VP_I_INC:%.*]], [[BB3:BB[0-9]+]] ]
; CHECK-NEXT:    SUCCESSORS(1):[[LOOP1:loop[0-9]+]]
; CHECK-NEXT:    PREDECESSORS(2): [[BB3]] [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    REGION: [[LOOP1]] (BP: NULL)
; CHECK-NEXT:    [[BB4:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[BB5:BB[0-9]+]]
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]] (BP: NULL) :
; CHECK-NEXT:     i32 [[VP_J:%.*]] = phi  [ i32 0, [[BB4]] ],  [ i32 [[VP_K_INC:%.*]], [[BB6:BB[0-9]+]] ]
; CHECK-NEXT:     i32 [[VP0:%.*]] = phi  [ i32 [[VP1:%.*]], [[BB6]] ],  [ i32 0, [[BB4]] ]
; CHECK-NEXT:     i32 [[VP_J_INC:%.*]] = add i32 [[VP_J]] i32 1
; CHECK-NEXT:     i1 [[VP_CMP2:%.*]] = icmp i32 [[VP_J_INC]] i32 16
; CHECK-NEXT:    SUCCESSORS(1):[[REGION1:region[0-9]+]]
; CHECK-NEXT:    PREDECESSORS(2): [[BB6]] [[BB4]]
; CHECK-EMPTY:
; CHECK-NEXT:    REGION: [[REGION1]] (BP: NULL)
; CHECK-NEXT:    [[BB7:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:     Condition([[BB5]]): i1 [[VP_CMP2]] = icmp i32 [[VP_J_INC]] i32 16
; CHECK-NEXT:    SUCCESSORS(2):[[BB8:BB[0-9]+]](i1 [[VP_CMP2]]), Intermediate[[BB9:BB[0-9]+]](!i1 [[VP_CMP2]])
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:      Intermediate[[BB9]] (BP: NULL) :
; CHECK-NEXT:       <Empty Block>
; CHECK-NEXT:      SUCCESSORS(1):NewLoopLatch15
; CHECK-NEXT:      PREDECESSORS(1): [[BB7]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB8]] (BP: NULL) :
; CHECK-NEXT:       i32 [[VP_L:%.*]] = add i32 [[VP_J]] i32 2
; CHECK-NEXT:       i32 [[VP_L_INC:%.*]] = add i32 [[VP_L]] i32 [[VP_I]]
; CHECK-NEXT:       i1 [[VP_CMP3:%.*]] = icmp i32 [[VP_L_INC]] i32 32
; CHECK-NEXT:      SUCCESSORS(2):[[BB10:BB[0-9]+]](i1 [[VP_CMP3]]), Intermediate[[BB11:BB[0-9]+]](!i1 [[VP_CMP3]])
; CHECK-NEXT:      PREDECESSORS(1): [[BB7]]
; CHECK-EMPTY:
; CHECK-NEXT:        Intermediate[[BB11]] (BP: NULL) :
; CHECK-NEXT:         <Empty Block>
; CHECK-NEXT:        SUCCESSORS(1):NewLoopLatch15
; CHECK-NEXT:        PREDECESSORS(1): [[BB8]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB10]] (BP: NULL) :
; CHECK-NEXT:       i32 [[VP_K_INC]] = add i32 [[VP_L]] i32 1
; CHECK-NEXT:      SUCCESSORS(1):NewLoopLatch15
; CHECK-NEXT:      PREDECESSORS(1): [[BB8]]
; CHECK-EMPTY:
; CHECK-NEXT:    NewLoopLatch15 (BP: NULL) :
; CHECK-NEXT:     i32 [[VP1]] = phi  [ i32 [[VP0]], [[BB10]] ],  [ i32 1, Intermediate[[BB9]] ],  [ i32 2, Intermediate[[BB11]] ]
; CHECK-NEXT:     i1 [[VP2:%.*]] = phi  [ i1 true, [[BB10]] ],  [ i1 false, Intermediate[[BB9]] ],  [ i1 false, Intermediate[[BB11]] ]
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(3): [[BB10]] Intermediate[[BB9]] Intermediate[[BB11]]
; CHECK-EMPTY:
; CHECK-NEXT:    SUCCESSORS(1):[[BB6]]
; CHECK-NEXT:    END Region([[REGION1]])
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:     Condition(NewLoopLatch15): i1 [[VP2]] = phi  [ i1 true, [[BB10]] ],  [ i1 false, Intermediate[[BB9]] ],  [ i1 false, Intermediate[[BB11]] ]
; CHECK-NEXT:    SUCCESSORS(2):[[BB5]](i1 [[VP2]]), IfBlock18(!i1 [[VP2]])
; CHECK-NEXT:    PREDECESSORS(1): [[REGION1]]
; CHECK-EMPTY:
; CHECK-NEXT:    IfBlock18 (BP: NULL) :
; CHECK-NEXT:     i1 [[VP3:%.*]] = icmp i32 [[VP1]] i32 2
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(1): [[BB6]]
; CHECK-EMPTY:
; CHECK-NEXT:    SUCCESSORS(1):[[REGION2:region[0-9]+]]
; CHECK-NEXT:    END Region([[LOOP1]])
; CHECK-EMPTY:
; CHECK-NEXT:    REGION: [[REGION2]] (BP: NULL)
; CHECK-NEXT:    [[BB12:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:     Condition(IfBlock18): i1 [[VP3]] = icmp i32 [[VP1]] i32 2
; CHECK-NEXT:    SUCCESSORS(2):[[BB13:BB[0-9]+]](i1 [[VP3]]), [[BB14:BB[0-9]+]](!i1 [[VP3]])
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB14]] (BP: NULL) :
; CHECK-NEXT:       <Empty Block>
; CHECK-NEXT:      SUCCESSORS(1):[[BB15:BB[0-9]+]]
; CHECK-NEXT:      PREDECESSORS(1): [[BB12]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB13]] (BP: NULL) :
; CHECK-NEXT:       <Empty Block>
; CHECK-NEXT:      SUCCESSORS(1):[[BB15]]
; CHECK-NEXT:      PREDECESSORS(1): [[BB12]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB15]] (BP: NULL) :
; CHECK-NEXT:     i32 [[VP_M3:%.*]] = phi  [ i32 1, [[BB14]] ],  [ i32 2, [[BB13]] ]
; CHECK-NEXT:     i32 [[VP_M4:%.*]] = add i32 [[VP_M3]] i32 4
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(2): [[BB13]] [[BB14]]
; CHECK-EMPTY:
; CHECK-NEXT:    SUCCESSORS(1):[[BB3]]
; CHECK-NEXT:    END Region([[REGION2]])
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB3]] (BP: NULL) :
; CHECK-NEXT:     i32 [[VP_I_INC]] = add i32 [[VP_I]] i32 [[VP_M4]]
; CHECK-NEXT:     i1 [[VP_CMP4:%.*]] = icmp i32 [[VP_I_INC]] i32 1024
; CHECK-NEXT:    SUCCESSORS(2):[[BB2]](i1 [[VP_CMP4]]), [[BB16:BB[0-9]+]](!i1 [[VP_CMP4]])
; CHECK-NEXT:    PREDECESSORS(1): [[REGION2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB16]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(1): [[BB3]]
; CHECK-EMPTY:
; CHECK-NEXT:    SUCCESSORS(1):[[BB17:BB[0-9]+]]
; CHECK-NEXT:    END Region([[LOOP0]])
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB17]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(1): [[LOOP0]]
; CHECK-EMPTY:
; CHECK-NEXT:    END Region([[REGION0]])
; CHECK-EMPTY:
; CHECK:  Before inner loop control flow transformation
; CHECK:  Checking inner loop control flow uniformity for:
; CHECK-NEXT:  SubLoopRegion: [[LOOP1]]
; CHECK-NEXT:  SubLoopPreHeader: [[BB4]]
; CHECK-NEXT:  SubLoopHeader: [[BB5]]
; CHECK-NEXT:  SubLoopLatch: [[BB6]]
; CHECK-NEXT:  SubLoopExitBlock: IfBlock18
; CHECK-NEXT:  BottomTest: i1 [[VP2]] = phi  [ i1 true, [[BB10]] ],  [ i1 false, Intermediate[[BB9]] ],  [ i1 false, Intermediate[[BB11]] ]
; CHECK-EMPTY:
; CHECK-NEXT:  BottomTest is divergent
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop1

loop1:
  %i = phi i32 [ 0, %entry ], [ %i_inc, %loop1_latch ]
  br label %loop2

loop2:
  %j = phi i32 [ 0, %loop1 ], [ %k_inc, %loop2_latch ]
  %j_inc = add nsw i32 %j, 1
  %cmp2 = icmp eq i32 %j_inc, 16
  br i1 %cmp2, label %bb1, label %bb2

bb1:
  %l = add nsw i32 %j, 2
  %l_inc = add nsw i32 %l, %i
  %cmp3 = icmp eq i32 %l_inc, 32
  br i1 %cmp3, label %loop2_latch, label %bb3

loop2_latch:
  %k_inc = add nsw i32 %l, 1
  br label %loop2

bb2:
  br label %bb4

bb3:
  br label %bb4

bb4:
  %m3 = phi i32 [ 1, %bb2 ], [ 2, %bb3 ]
  %m4 = add nsw i32 %m3, 4
  br label %loop1_latch

loop1_latch:
  %i_inc = add nsw i32 %i, %m4
  %cmp4 = icmp eq i32 %i_inc, 1024
  br i1 %cmp4, label %loop1, label %loop1_exit

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
