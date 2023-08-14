; RUN: opt < %s -hir-cost-model-throttling=0 -passes="hir-ssa-deconstruction,print<hir>" -disable-output 2>&1 | FileCheck %s

; Verify that bblock %for.jump is linked in i1 loop by HIRCreation phase.
; The latch of i3 loop is jumping directly to i1 loop using %for.jump.
; Previously, we were linking %for.jump in the i2 loop. This was resulting in
; incorrect def level for %or9.lcssa1319. The temp is defined at level 1 in LLVM
; IR but is moved to level 2 in HIR. Due to this mismatch, parser was marking it
; as def@1 even when the definition was at level 2 resulting in verifier
; assertion.

; HIR before fix-

; + DO i1 = 0, 0, 1   <DO_MULTI_EXIT_LOOP>
; |   %or9.lcssa1319.out = %or9.lcssa1319;
; |
; |   + DO i2 = 0, 0, 1   <DO_MULTI_EXIT_LOOP>
; |   |   + DO i3 = 0, 0, 1   <DO_MULTI_EXIT_LOOP>
; |   |   |   if (undef false undef)
; |   |   |   {
; |   |   |      goto for.mid.latch;
; |   |   |   }
; |   |   + END LOOP
; |   |
; |   |   %or9.lcssa1319 = 0  |  %or9.lcssa1319;
; |   |   goto for.outer.latch;
; |   |   for.mid.latch:
; |   + END LOOP
; |
; |   goto for.mid.exit;
; |   for.outer.latch:
; |   (null)[0] = %or9.lcssa1319.out;
; + END LOOP

; Note that %or9.lcssa1319 was defined at i2 loop before the fix as the bblock
; for.jump was linked in i2 loop.

; CHECK: + DO i1 = 0, 0, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %or9.lcssa1319.out = %or9.lcssa1319;
; CHECK: |
; CHECK: |   + DO i2 = 0, 0, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   |   + DO i3 = 0, 0, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   |   |   if (undef false undef)
; CHECK: |   |   |   {
; CHECK: |   |   |      goto for.mid.latch;
; CHECK: |   |   |   }
; CHECK: |   |   + END LOOP
; CHECK: |   |
; CHECK: |   |   goto for.jump;
; CHECK: |   |   for.mid.latch:
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   goto for.mid.exit;
; CHECK: |   for.jump:
; CHECK: |   %or9.lcssa1319 = 0  |  %or9.lcssa1319;
; CHECK: |   (null)[0] = %or9.lcssa1319.out;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z1rv() {
entry:
  br label %for.outer

for.outer:                  ; preds = %for.outer.latch, %entry
  %or9.lcssa1319 = phi i32 [ 0, %entry ], [ %or.lcssa, %for.outer.latch ]
  br label %for.mid

for.mid:                            ; preds = %for.mid.latch, %for.outer
  br label %for.inner

for.inner:                                      ; preds = %for.inner.latch, %for.mid
  br i1 false, label %for.mid.latch, label %for.inner.latch

for.inner.latch:                                      ; preds = %for.inner
  %exitcond = icmp eq i64 0, 0
  br i1 %exitcond, label %for.jump, label %for.inner

for.mid.latch:                                      ; preds = %for.inner
  %tobool.not.i = icmp eq i8 0, 0
  br i1 %tobool.not.i, label %for.mid.exit, label %for.mid

for.jump:                                      ; preds = %for.inner.latch
  %or = or i32 0, %or9.lcssa1319
  br label %for.outer.latch

for.outer.latch:                              ; preds = %for.jump
  %or.lcssa = phi i32 [ %or, %for.jump ]
  store i32 %or9.lcssa1319, ptr null, align 4
  %tobool.not45.i = icmp eq i8 0, 0
  br i1 %tobool.not45.i, label %exit, label %for.outer

for.mid.exit:                               ; preds = %for.mid.latch
  ret void

exit:                             ; preds = %for.outer.latch
  ret void
}
