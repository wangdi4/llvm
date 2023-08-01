; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s
; RUN: opt < %s -passes="convert-to-subscript,hir-ssa-deconstruction" | opt -passes="print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; CHECK:      BEGIN REGION
; CHECK:            + DO i1 = 0, zext.i32.i64(%n) + -1, 1
; CHECK-NEXT:       |   %0 = (%A)[i1];
; CHECK-NEXT:       |   (%A)[i1] = smax((-1 * %0), %0);
; CHECK-NEXT:       + END LOOP
; CHECK:      END REGION

define void @_Z3fooPii(ptr nocapture %A, i32 %n) {
entry:
  %cmp17 = icmp sgt i32 %n, 0
  br i1 %cmp17, label %for.body.preheader, label %for.cond.cleanup

  for.body.preheader:                               ; preds = %entry
  %wide.trip.count19 = zext i32 %n to i64
  br label %for.body

  for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

  for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

  for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %ptridx, align 4
  %1 = tail call i32 @llvm.abs.i32(i32 %0, i1 true)
  store i32 %1, ptr %ptridx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count19
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

declare i32 @llvm.abs.i32(i32, i1 immarg) #0

attributes #0 = { nofree nosync nounwind readnone speculatable willreturn }

