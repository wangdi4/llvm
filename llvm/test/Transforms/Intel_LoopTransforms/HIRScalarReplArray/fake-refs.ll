; RUN: opt -hir-idiom-small-trip-count=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-idiom,print<hir>,hir-scalarrepl-array,hir-cg" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we successfully pass through scalar replacement for this test case.
; It was failing because locality analysis formed a locality group out of fake refs (%A)[i1 + 1][undef] and (%A)[i1][undef] attached to memcpy instruction.
; Scalar replacement then tried to replace them as if they were real memrefs leading to assertion.


; CHECK: Function

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -2, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK: |   |   (%A)[i1 + 1][i2] = (%A)[i1][i2];
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Function

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -2, 1   <DO_LOOP>
; CHECK: |   @llvm.memcpy.p0.p0.i64(&((i8*)(%A)[i1 + 1][0]),  &((i8*)(%A)[i1][0]),  20,  0);
; CHECK: + END LOOP


;Module Before HIR;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %A, i32 %n) local_unnamed_addr {
entry:
  %cmp23 = icmp sgt i32 %n, 1
  br i1 %cmp23, label %for.cond1.preheader.preheader, label %for.end12

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc10
  %indvars.iv26 = phi i64 [ %indvars.iv.next27, %for.inc10 ], [ 1, %for.cond1.preheader.preheader ]
  %0 = add nsw i64 %indvars.iv26, -1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [50 x i32], ptr %A, i64 %0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx5, align 4, !tbaa !1
  %arrayidx9 = getelementptr inbounds [50 x i32], ptr %A, i64 %indvars.iv26, i64 %indvars.iv
  store i32 %1, ptr %arrayidx9, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.inc10, label %for.body3

for.inc10:                                        ; preds = %for.body3
  %indvars.iv.next27 = add nuw nsw i64 %indvars.iv26, 1
  %exitcond29 = icmp eq i64 %indvars.iv.next27, %wide.trip.count
  br i1 %exitcond29, label %for.end12.loopexit, label %for.cond1.preheader

for.end12.loopexit:                               ; preds = %for.inc10
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %entry
  ret void
}

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA50_i", !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
