; Test merge loop exits transformation when the loop has one side exit and one regular exit.

; REQUIRES: asserts
; RUN: opt -S %s -VPlanDriver -vplan-force-vf=8 -disable-vplan-codegen -debug 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: After merge loop exits transformation.
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
; CHECK-NEXT: i32 [[PHI_OUT:%vp.*]] = phi
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): {{BB[0-9]+}} {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: [[LoopPreHeader:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: [[LoopHeader:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: i32 [[PHI_OUT0:%vp.*]] = phi  [ i32 0, [[LoopPreHeader]] ], [ i32 {{%vp.*}}, {{NewLoopLatch[0-9]+}} ]
; CHECK-NEXT: i32 [[PHI_OUT1:%vp.*]] = phi  [ i32 {{%vp.*}}, {{NewLoopLatch[0-9]+}} ],  [ i32 0, [[LoopPreHeader]] ]
; CHECK-NEXT: i32 [[ADD_OUT1:%vp.*]] = add
; CHECK-NEXT: i1 [[CMP_OUT1:%vp.*]] = icmp
; CHECK-NEXT: SUCCESSORS(2):{{BB[0-9]+}}(i1 [[CMP_OUT1]]), {{IntermediateBB[0-9]+}}(!i1 [[CMP_OUT1]])
; CHECK-NEXT: PREDECESSORS(2): {{NewLoopLatch[0-9]+}} [[LoopPreHeader]]
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i32 [[ADD_OUT2:%vp.*]] = add
; CHECK-NEXT: i1 [[CMP_OUT2:%vp.*]] = icmp
; CHECK-NEXT: SUCCESSORS(2):{{BB[0-9]+}}(i1 [[CMP_OUT2]]), {{IntermediateBB[0-9]+}}(!i1 [[CMP_OUT2]])
; CHECK-NEXT: PREDECESSORS(1): [[LoopHeader]]
; CHECK-EMPTY:

; CHECK-NEXT: {{IntermediateBB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i32 [[PHI_OUT10:%vp.*]] = phi  [ i32 0, {{BB[0-9]+}} ],  [ i32 1, {{BB[0-9]+}} ]
; CHECK-NEXT: SUCCESSORS(1):{{NewLoopLatch[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): [[LoopHeader]] {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: [[OrigLoopLatch:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: i1 [[CMP_OUT3:%vp.*]] = icmp
; CHECK-NEXT: SUCCESSORS(1):{{NewLoopLatch[0-9]+}}
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{NewLoopLatch[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i32 [[PHI_OUT2:%vp.*]] = phi  [ i32 [[PHI_OUT1]], [[OrigLoopLatch]] ],  [ i32 1, {{IntermediateBB[0-9]+}} ]
; CHECK-NEXT: i1 [[PHI_OUT3:%vp.*]] = phi  [ i1 [[CMP_OUT3]], [[OrigLoopLatch]] ],  [ i1 false, {{IntermediateBB[0-9]+}} ]
; CHECK-NEXT: SUCCESSORS(2):[[LoopHeader]](i1 [[PHI_OUT3]]), {{IfBlock[0-9]+}}(!i1 [[PHI_OUT3]])
; CHECK-NEXT: PREDECESSORS(2): [[OrigLoopLatch]] {{IntermediateBB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{IfBlock[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i1 [[CMP_OUT5:%vp.*]] = icmp
; CHECK-NEXT: SUCCESSORS(2):{{BB[0-9]+}}(i1 [[CMP_OUT5]]), {{BB[0-9]+}}(!i1 [[CMP_OUT5]])
; CHECK-NEXT: PREDECESSORS(1): {{NewLoopLatch[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i32 [[ADD_OUT6:%vp.*]] = add
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(1): {{IfBlock[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i32 [[ADD_OUT7:%vp.*]] = add
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(1): {{IfBlock[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i32 [[PHI_OUT8:%vp.*]] = phi
; CHECK-NEXT: i32 [[ADD_OUT8:%vp.*]] = add
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): {{BB[0-9]+}} {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i32 [[ADD_OUT9:%vp.*]] = add
; CHECK-NEXT: i1 [[CMP_OUT9:%vp.*]] = icmp
; CHECK-NEXT: SUCCESSORS(2):{{BB[0-9]+}}(i1 [[CMP_OUT9]]), {{BB[0-9]+}}(!i1 [[CMP_OUT9]])
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: no SUCCESSORS
; CHECK-NEXT: PREDECESSORS(1): {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK-NEXT: END Region({{region[0-9]+}})

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() #0 {
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
  br i1 %cmp1, label %bb1, label %bb2

bb1:
  %k_inc = add nsw i32 %j, 1
  %cmp2 = icmp eq i32 %k_inc, 16
  br i1 %cmp2, label %loop2_latch, label %bb2

loop2_latch:
  %cmp3 = icmp eq i32 %j, 128
  br i1 %cmp3, label %loop2, label %bb4

bb2:
  %m1 = phi i32 [ 0, %loop2 ], [ 1, %bb1 ]
  %m2 = add nsw i32 %m1, 1
  br label %bb3

bb3:
  br label %bb5

bb4:
  %m3 = add nsw i32 %i, 2
  br label %bb5

bb5:
  %m4 = phi i32 [ 0, %bb4 ], [ 1, %bb3 ]
  %m5 = add nsw i32 %i, %m4
  br label %loop1_latch

loop1_latch:
  %i_inc = add nsw i32 %i, %m5
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
