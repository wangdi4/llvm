; RUN: opt < %s -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-general-unroll -debug-only=hir-general-unroll 2>&1 | FileCheck %s
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


%xalan.node = type { %xalan.block*, %xalan.node*, %xalan.node* }
%xalan.block = type <{ %xalan.block.base, i16, i16, [4 x i8] }>
%xalan.block.base = type { i64, i16, i16, i64* }

define void @foo(%xalan.node* %t317, %xalan.node* %t318, %xalan.block** %t319, i64* %t1, i64 %n) {
entry:
  br label %outer.loop

outer.loop:
  %outer.iv = phi i64 [ %n , %entry ], [ %outer.iv.inc, %outer.latch ]
  br label %loop

loop:                                              ; preds = %latch, %outer.loop
  %t322 = phi %xalan.node* [ %t326, %latch ], [ %t317, %outer.loop ]
  %t323 = icmp eq %xalan.node* %t322, %t318
  br i1 %t323, label %exit1, label %if1

if1:                                              ; preds = %loop
  %t325 = getelementptr inbounds %xalan.node, %xalan.node* %t322, i64 0, i32 1
  %t326 = load %xalan.node*, %xalan.node** %t325, align 8
  %t327 = bitcast %xalan.node* %t326 to %xalan.block.base**
  %t328 = load %xalan.block.base*, %xalan.block.base** %t327, align 8
  %t329 = getelementptr inbounds %xalan.block.base, %xalan.block.base* %t328, i64 0, i32 3
  %t330 = load i64*, i64** %t329, align 8
  %t331 = icmp ugt i64* %t330, %t1
  %t332 = bitcast %xalan.block.base* %t328 to %xalan.block*
  br i1 %t331, label %latch, label %if2

if2:                                              ; preds = %if1
  %t334 = getelementptr inbounds %xalan.block.base, %xalan.block.base* %t328, i64 0, i32 2
  %t335 = load i16, i16* %t334, align 2
  %t336 = zext i16 %t335 to i64
  %t337 = getelementptr inbounds i64, i64* %t330, i64 %t336
  %t338 = icmp ugt i64* %t337, %t1
  br i1 %t338, label %exit2, label %latch

latch:                                              ; preds = %if2, %if1
  %t340 = load %xalan.block*, %xalan.block** %t319, align 8
  %t341 = icmp eq %xalan.block* %t340, %t332
  br i1 %t341, label %exit1, label %loop

exit2:                                              ; preds = %if2
  %t343 = phi %xalan.node* [ %t322, %if2 ]
  %t344 = phi %xalan.block.base* [ %t328, %if2 ]
  %t345 = phi i64** [ %t329, %if2 ]
  %t346 = phi i64* [ %t330, %if2 ]
  %t347 = phi %xalan.block* [ %t332, %if2 ]
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

