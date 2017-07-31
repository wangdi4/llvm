; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that the profitable inner i2 loop gets unrolled.

; CHECK: Dump Before HIR PostVec Complete Unroll
; CHECK: + DO i1 = 0, %h + -1, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK: |   |   %0 = (%p1.2469)[i2];
; CHECK: |   |   %1 = (%add.ptr220)[%lx * i1 + i2];
; CHECK: |   |   %2 = (%blk2)[%lx * i1 + i2];
; CHECK: |   |   %.p456 = (-1 * zext.i8.i32(%2) + ((1 + zext.i8.i32(%1) + zext.i8.i32(%0)) /u 2) > -1) ? -1 * zext.i8.i32(%2) + ((1 + zext.i8.i32(%1) + zext.i8.i32(%0)) /u 2) : zext.i8.i32(%2) + -1 * ((1 + zext.i8.i32(%1) + zext.i8.i32(%0)) /u 2);
; CHECK: |   |   %s.4473 = %.p456  +  %s.4473;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %p1.2469 = &((%add.ptr220)[%lx * i1]);
; CHECK: + END LOOP


; CHECK: Dump After HIR PostVec Complete Unroll
; CHECK-NOT: DO i2

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"

define i32 @foo(i8* %blk1, i8* %blk2, i8 *%add.ptr220, i32 %h, i32 %lx) {
entry:
  br label %for.cond225.preheader

for.cond225.preheader:                            ; preds = %entry, %for.end248
  %s.4473 = phi i32 [ %.lcssa, %for.end248 ], [ 0, %entry ]
  %j.2472 = phi i32 [ %inc252, %for.end248 ], [ 0, %entry ]
  %p2.2471 = phi i8* [ %add.ptr250, %for.end248 ], [ %blk2, %entry ]
  %p1a.0470 = phi i8* [ %add.ptr249, %for.end248 ], [ %add.ptr220, %entry ]
  %p1.2469 = phi i8* [ %p1a.0470, %for.end248 ], [ %blk1, %entry ]
  br label %for.body228

for.body228:                                      ; preds = %for.body228, %for.cond225.preheader
  %s.5467 = phi i32 [ %s.4473, %for.cond225.preheader ], [ %4, %for.body228 ]
  %i.1466 = phi i32 [ 0, %for.cond225.preheader ], [ %inc247, %for.body228 ]
  %arrayidx229 = getelementptr inbounds i8, i8* %p1.2469, i32 %i.1466
  %0 = load i8, i8* %arrayidx229, align 1
  %conv230 = zext i8 %0 to i32
  %arrayidx231 = getelementptr inbounds i8, i8* %p1a.0470, i32 %i.1466
  %1 = load i8, i8* %arrayidx231, align 1
  %conv232 = zext i8 %1 to i32
  %add233 = add nuw nsw i32 %conv230, 1
  %add234 = add nuw nsw i32 %add233, %conv232
  %shr235 = lshr i32 %add234, 1
  %arrayidx236 = getelementptr inbounds i8, i8* %p2.2471, i32 %i.1466
  %2 = load i8, i8* %arrayidx236, align 1
  %conv237 = zext i8 %2 to i32
  %sub238 = sub nsw i32 %shr235, %conv237
  %cmp239 = icmp sgt i32 %sub238, -1
  %3 = sub nsw i32 0, %sub238
  %.p456 = select i1 %cmp239, i32 %sub238, i32 %3
  %4 = add i32 %.p456, %s.5467
  %inc247 = add nuw nsw i32 %i.1466, 1
  %exitcond = icmp eq i32 %inc247, 16
  br i1 %exitcond, label %for.end248, label %for.body228

for.end248:                                       ; preds = %for.body228
  %.lcssa = phi i32 [ %4, %for.body228 ]
  %add.ptr249 = getelementptr inbounds i8, i8* %p1a.0470, i32 %lx
  %add.ptr250 = getelementptr inbounds i8, i8* %p2.2471, i32 %lx
  %inc252 = add nuw nsw i32 %j.2472, 1
  %exitcond497 = icmp eq i32 %inc252, %h
  br i1 %exitcond497, label %if.end299.loopexit510, label %for.cond225.preheader

if.end299.loopexit510:
  ret i32 %4
}
