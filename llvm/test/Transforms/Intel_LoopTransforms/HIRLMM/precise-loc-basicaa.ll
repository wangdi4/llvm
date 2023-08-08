; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,hir-lmm,print<hir>,print,hir-cg" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check that by providing precise location info to basic-aa for region invariant
; refs, we are able to disambiguate between (%bc2)[1] (which is equivalent to
; (@A)[0][1][1]) and unrolled references of the form (@A)[0][0][c] so that LMM
; is able to hoist them outside the i1 loop and get rid of the loop altogether.

; This is not possible with DD analysis due of conservative parsing of (%bc2)[1].

; Incoming HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   %0 = (%bc2)[1];
; |
; |   + DO i2 = 0, 2, 1   <DO_LOOP>
; |   |   (@A)[0][0][i2] = %0;
; |   + END LOOP
; + END LOOP


; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:     %0 = (%bc2)[1];
; CHECK-NEXT:     (@A)[0][0][2] = %0;
; CHECK-NEXT:     (@A)[0][0][1] = %0;
; CHECK-NEXT:     (@A)[0][0][0] = %0;
; CHECK-NEXT:  END REGION


; Verify that before CG we had created exactly 4 dummy geps for alias analysis corresponding to each ref.

; CHECK-LABEL: for.body:
; CHECK-DAG: %dummygep{{.*}} = getelementptr inbounds i32, ptr %bc2, i64 1
; CHECK-DAG: %dummygep{{.*}} = getelementptr inbounds [100 x [4 x i32]], ptr @A, i64 0, i64 0, i64 2
; CHECK-DAG: %dummygep{{.*}} = getelementptr inbounds [100 x [4 x i32]], ptr @A, i64 0, i64 0, i64 1
; CHECK-DAG: %dummygep{{.*}} = getelementptr inbounds [100 x [4 x i32]], ptr @A, i64 0, i64 0, i64 0
; CHECK-NOT: dummygep

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x [4 x i32]] zeroinitializer, align 16
; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %gep = getelementptr inbounds [100 x [4 x i32]], ptr @A, i64 0, i64 1, i64 0
  %bc1 = bitcast ptr %gep to ptr
  %bc2 = bitcast ptr %bc1 to ptr
  %gep2 = getelementptr inbounds i32, ptr %bc2, i64 1
  br label %for.body

for.body:                                         ; preds = %for.inc4, %entry
  %i.014 = phi i32 [ 0, %entry ], [ %inc5, %for.inc4 ]
  %0 = load i32, ptr %gep2, align 16
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [100 x [4 x i32]], ptr @A, i64 0, i64 0, i64 %indvars.iv
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.inc4, label %for.body3

for.inc4:                                         ; preds = %for.body3
  %inc5 = add nuw nsw i32 %i.014, 1
  %exitcond15 = icmp eq i32 %inc5, 100
  br i1 %exitcond15, label %for.end6, label %for.body

for.end6:                                         ; preds = %for.inc4
  ret void
}

