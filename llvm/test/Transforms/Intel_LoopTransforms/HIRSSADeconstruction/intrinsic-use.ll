; RUN: opt < %s -passes=hir-ssa-deconstruction -S | FileCheck %s

; Verify that the use of %length.016.i in the intrinsic @llvm.smax.i32() is
; replaced by the liveout copy. %length.016.i is part of this SCC:
; %length.016.i -> %t18

; The intrinsic is parsed as SCEVSMaxExpr and propagated to its use in %t19. 
; %t19 is part of this SCC:
; %max.017.i -> %t19

; %length.016.i is getting redefined by %t18 since they are the same values in
; HIR as they form an SCC so propagating its use in %t19 (%max.017.i) causes
; live range violation.

; HIR after fix-
; + DO i1 = 0, (-1 * ptrtoint.i32*.i64(%t14) + umax((4 + ptrtoint.i32*.i64(%t14)), ptrtoint.i32*.i64(%add.ptr.i)) + -1)/u4, 1   <DO_LOOP>
; |   %length.016.i.out = %length.016.i;
; |   %max.017.i.out = %max.017.i;
; |   %t16 = (%t14)[i1];
; |   %length.016.i = (%t16 > -1) ? %length.016.i.out + 1 : 0;
; |   %max.017.i = (%t16 > -1) ? %max.017.i : smax(%length.016.i.out, %max.017.i.out);
; + END LOOP

; smax expr in the last HLInst is now using %length.016.i.out instead of
; %length.016.i before the fix.
 
; CHECK: @llvm.smax.i32(i32 %length.016.i.out

define void @foo(i32* %add.ptr.i, i32* %t14) {
entry:
  br label %for.body.i34

for.body.i34:                                     ; preds = %for.body.i34, %entry
  %max.017.i = phi i32 [ %t19, %for.body.i34 ], [ 0, %entry ]
  %length.016.i = phi i32 [ %t18, %for.body.i34 ], [ 0, %entry ]
  %itemp.015.i = phi i32* [ %incdec.ptr.i, %for.body.i34 ], [ %t14, %entry ]
  %t16 = load i32, i32* %itemp.015.i, align 4
  %cmp1.i = icmp sgt i32 %t16, -1
  %inc.i = add nsw i32 %length.016.i, 1
  %t17 = tail call i32 @llvm.smax.i32(i32 %length.016.i, i32 %max.017.i)
  %t18 = select i1 %cmp1.i, i32 %inc.i, i32 0
  %t19 = select i1 %cmp1.i, i32 %max.017.i, i32 %t17
  %incdec.ptr.i = getelementptr inbounds i32, i32* %itemp.015.i, i64 1
  %cmp.i = icmp ult i32* %incdec.ptr.i, %add.ptr.i
  br i1 %cmp.i, label %for.body.i34, label %set_maxrhs.exit.loopexit

set_maxrhs.exit.loopexit:                         ; preds = %for.body.i34
  %.lcssa = phi i32 [ %t19, %for.body.i34 ]
  ret void
}

declare i32 @llvm.smax.i32(i32, i32) #2

attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

