; Test merge loop exits transformation for a while loop with two side exits.

; REQUIRES: asserts
; RUN: opt -S %s -VPlanDriver -vplan-force-vf=8 -disable-vplan-codegen -debug 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK-LABEL: Before single exit while loop transformation.
; CHECK: [[Header:BB[0-9]+]] (BP: NULL) :
; CHECK: i32 [[PHI0:%vp[0-9]+]] = phi  [ i32 0, {{BB[0-9]+}} ], [ i32 {{%vp[0-9]+}}, [[Latch:BB[0-9]+]] ]
; CHECK: i32 [[ADD0:%vp[0-9]+]] = add
; CHECK-NEXT: SUCCESSORS(1):[[Exiting:BB[0-9]+]]
; CHECK-NEXT: PREDECESSORS(2): [[Latch:BB[0-9]+]] {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: [[Exiting:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: i32 [[ADD1:%vp[0-9]+]] = add
; CHECK-NEXT: i1 [[CMP1:%vp[0-9]+]] = icmp
; CHECK-NEXT: SUCCESSORS(2):{{BB[0-9]+}}(i1 [[CMP1]]), [[EXIT:BB[0-9]+]](!i1 [[CMP1]])
; CHECK-NEXT: PREDECESSORS(1): [[Header:BB[0-9]+]]
; CHECK-EMPTY:

; CHECK-NEXT: [[Exit:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: i32 [[PHI2:%vp[0-9]+]] = phi  [ i32 0, [[Exiting]] ]
; CHECK-NEXT: i32 [[ADD2:%vp[0-9]+]] = add
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(1): [[Latch:BB[0-9]+]]
; CHECK-EMPTY:

; CHECK: {{BB[0-9]+}} (BP: NULL) :
; CHECK: <Empty Block>
; CHECK: SUCCESSORS(1):[[Latch:BB[0-9]+]]
; CHECK: PREDECESSORS(1): [[Exiting]]
; CHECK-EMPTY:

; CHECK-NEXT: [[Latch:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):[[Header:BB[0-9]+]]
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-LABEL: After single exit while loop transformation.
; CHECK: REGION: {{region[0-9]+}} (BP: NULL)
; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i32 [[PHI_OUT:%vp[0-9]+]] = phi
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): {{BB[0-9]+}} {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: [[LoopPreHeader:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):[[LoopHeader:BB[0-9]+]]
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: [[LoopHeader:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: i32 [[PHI_OUT0:%vp[0-9]+]] = phi  [ i32 0, [[LoopPreHeader]] ], [ i32 [[ADD_OUT1:%vp[0-9]+]], [[NewLoopLatch:BB[0-9]+]] ]
; CHECK-NEXT: i32 [[ADD_OUT1]] = add
; CHECK-NEXT: SUCCESSORS(1):[[ExitingBlock:BB[0-9]+]]
; CHECK-NEXT: PREDECESSORS(2): [[NewLoopLatch:BB[0-9]+]] [[LoopPreHeader]]
; CHECK-EMPTY:

; The exiting block is redirected to the new loop latch.
; CHECK-NEXT: [[ExitingBlock:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: i32 [[ADD_OUT2:%vp[0-9]+]] = add
; CHECK-NEXT: i1 [[CMP_OUT2:%vp[0-9]+]] = icmp
; CHECK-NEXT: SUCCESSORS(2):{{BB[0-9]+}}(i1 [[CMP_OUT2]]), [[NewLoopLatch:BB[0-9]+]](!i1 [[CMP_OUT2]])
; CHECK-NEXT: PREDECESSORS(1): [[LoopHeader]]
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):[[OrigLoopLatch:BB[0-9]+]]
; CHECK-NEXT: PREDECESSORS(1): [[ExitingBlock]]
; CHECK-EMPTY:

; CHECK-NEXT: [[OrigLoopLatch:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):[[NewLoopLatch:BB[0-9]+]]
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: [[NewLoopLatch:BB[0-9]+]] (BP: NULL) :
; The phi node is also the condition bit.
; CHECK-NEXT: i1 [[PHI_OUT3:%vp[0-9]+]] = phi  [ i1 true, [[OrigLoopLatch]] ],  [ i1 false, [[ExitingBlock]] ]
; The back-edge is taken when phi is true. If phi is false, then we move to the exit block.
; CHECK-NEXT: SUCCESSORS(2):[[LoopHeader]](i1 [[PHI_OUT3]]), [[ExitBlock:BB[0-9]+]](!i1 [[PHI_OUT3]])
; CHECK-NEXT: PREDECESSORS(2): [[OrigLoopLatch]] [[ExitingBlock]]
; CHECK-EMPTY:

; CHECK-NEXT: [[ExitBlock:BB[0-9]+]] (BP: NULL) :
; The phi node of exit block is now updated to point to new loop latch.
; CHECK-NEXT: i32 [[PHI_OUT4:%vp[0-9]+]] = phi  [ i32 0, [[NewLoopLatch]] ]
; CHECK-NEXT: i32 [[ADD_OUT4:%vp[0-9]+]] = add
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(1): [[NewLoopLatch]]
; CHECK-EMPTY:
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop1

loop1:
  %i = phi i32 [ 0, %entry ], [ %i_inc, %loop1_latch ]
  br label %loop2

loop2:
  %j = phi i32 [ 0, %loop1 ], [ %j_inc, %loop2_latch ]
  %j_inc = add nsw i32 %j, 1
  br label %bb1

bb1:
  %k_inc = add nsw i32 %j, 1
  %cmp2 = icmp eq i32 %k_inc, 16
  br i1 %cmp2, label %bb2, label %bb3

bb2:
  br label %loop2_latch

loop2_latch:
  br label %loop2

bb3:
  %m1 = phi i32 [ 0, %bb1 ]
  %m2 = add nsw i32 %i, %m1
  br label %bb4

bb4:
  %m3 = add nsw i32 %i, 4
  br label %loop1_latch

loop1_latch:
  %i_inc = add nsw i32 %i, %m3
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
