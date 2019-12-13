; Test to check that we make it obvious that vector loop backedge will never
; be taken when the scalar loop trip count is equal to VF.
; RUN: opt -S -VPlanDriver -vplan-build-vect-candidates=1 -vplan-force-vf=8 %s | FileCheck %s
; RUN: opt -S -passes="vplan-driver" -vplan-build-vect-candidates=1 -vplan-force-vf=8 %s | FileCheck %s
; CHECK-LABEL: vector.body
; CHECK: br i1 true, label {{.*}}, label %vector.body
@arr = common dso_local local_unnamed_addr global i32* null, align 8

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %0 = load i32*, i32** @arr, align 8
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 8
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
