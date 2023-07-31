; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll" -debug-only=hir-general-unroll < %s 2>&1 | FileCheck %s

; Verify that penalty is applied to the inner multi-exit loop with liveouts to
; account for register pressure. HIR is not formed for the outer loop but it is
; still accounted for when calculating loop depth.

; Input HIR-
; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   loop:
; |   %t322.out = &((%t322)[0]);
; |   if (&((%t322)[0]) == &((%t318)[0]))
; |   {
; |      goto exit1;
; |   }
; |   %t326 = (%t322)[0].1;
; |   %t328 = (%xalan.block.base**)(%t326)[0];
; |   %t329 = &((%t328)[0].3);
; |   %t330 = (%t328)[0].3;
; |   %t332 = bitcast.%xalan.block.base*.%xalan.block*(&((%t328)[0]));
; |   if (&((%t330)[0]) <=u &((%t1)[0]))
; |   {
; |      %t335 = (%t328)[0].2;
; |      if (&((%t330)[%t335]) >u &((%t1)[0]))
; |      {
; |         goto exit2;
; |      }
; |   }
; |   %t322 = &((%t326)[0]);
; |   if ((%t319)[0] != &((%xalan.block*)(%t328)[0]))
; |   {
; |      <i1 = i1 + 1>
; |      goto loop;
; |   }
; + END LOOP

; Number of liveouts = 5
; Number of exits = 3
; Computed penalty = (5 + 3) = 8
; CHECK: Computed penalty for inner loop liveouts: 8


%xalan.node = type { ptr, ptr, ptr }
%xalan.block = type <{ %xalan.block.base, i16, i16, [4 x i8] }>
%xalan.block.base = type { i64, i16, i16, ptr }

define void @foo(ptr %t317, ptr %t318, ptr %t319, ptr %t1, i64 %n) {
entry:
  br label %outer.loop

outer.loop:
  %outer.iv = phi i64 [ %n , %entry ], [ %outer.iv.inc, %outer.latch ]
  br label %loop

loop:                                              ; preds = %latch, %outer.loop
  %t322 = phi ptr [ %t326, %latch ], [ %t317, %outer.loop ]
  %t323 = icmp eq ptr %t322, %t318
  br i1 %t323, label %exit1, label %if1

if1:                                              ; preds = %loop
  %t325 = getelementptr inbounds %xalan.node, ptr %t322, i64 0, i32 1
  %t326 = load ptr, ptr %t325, align 8
  %t327 = bitcast ptr %t326 to ptr
  %t328 = load ptr, ptr %t327, align 8
  %t329 = getelementptr inbounds %xalan.block.base, ptr %t328, i64 0, i32 3
  %t330 = load ptr, ptr %t329, align 8
  %t331 = icmp ugt ptr %t330, %t1
  %t332 = bitcast ptr %t328 to ptr
  br i1 %t331, label %latch, label %if2

if2:                                              ; preds = %if1
  %t334 = getelementptr inbounds %xalan.block.base, ptr %t328, i64 0, i32 2
  %t335 = load i16, ptr %t334, align 2
  %t336 = zext i16 %t335 to i64
  %t337 = getelementptr inbounds i64, ptr %t330, i64 %t336
  %t338 = icmp ugt ptr %t337, %t1
  br i1 %t338, label %exit2, label %latch

latch:                                              ; preds = %if2, %if1
  %t340 = load ptr, ptr %t319, align 8
  %t341 = icmp eq ptr %t340, %t332
  br i1 %t341, label %exit1, label %loop

exit2:                                              ; preds = %if2
  %t343 = phi ptr [ %t322, %if2 ]
  %t344 = phi ptr [ %t328, %if2 ]
  %t345 = phi ptr [ %t329, %if2 ]
  %t346 = phi ptr [ %t330, %if2 ]
  %t347 = phi ptr [ %t332, %if2 ]
  br label %outer.latch

exit1:
  br label %outer.latch

outer.latch:
  %outer.iv.inc = sdiv i64 %outer.iv, 2
  %cmp = icmp sgt i64 %outer.iv.inc, 1
  br i1 %cmp, label %outer.loop, label %exit

exit:
  ret void
}

