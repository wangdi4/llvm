; RUN: opt -vplan-force-vf=4 -S -VPlanDriver < %s | FileCheck %s

;void foo(int *A, int *B, int N, int c) {
;  for (int i=0: N) {
;    if (c != 0)
;      tmp = B[i];
;    else
;      tmp = 6;
;    A[i] += tmp;
;  }
;}
define void @foo(i32* noalias nocapture %A, i32* noalias nocapture readonly %B, i32 %N, i32 %c) local_unnamed_addr #0 {
; CHECK-LABEL: foo
; CHECK:   vector.body
; CHECK:      %index = phi i64 [ 0, %vector.ph ], [ %index.next, %[[VPBB1:.*]] ]
; CHECK-NEXT: [[SCAL_PHI:%.*]] = phi i64 [ 1, %vector.ph ], [ [[SCAL_NDX:%.*]], %[[VPBB1]] ]
; CHECK-NEXT: [[VEC_PHI:%.*]] = phi <4 x i64> [ <i64 1, i64 2, i64 3, i64 4>, %vector.ph ], [ [[VEC_NDX:%.*]], %[[VPBB1]] ]
; CHECK:      br i1 %cmp1, label %[[VPBB1]], label %[[VPBB:.*]]

; CHECK: [[VPBB]]:
; CHECK:   %wide.load{{.*}} = load <4 x i32>, <4 x i32>*
; CHECK:   br label %[[VPBB1]]

; CHECK: [[VPBB1]]:
; CHECK:      phi <4 x i32> [ %wide.load, %[[VPBB]] ], [ <i32 6, i32 6, i32 6, i32 6>, %vector.body ]
; CHECK:      %wide.load{{.*}} = load <4 x i32>,
; CHECK:      [[VEC_NDX]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 4, i64 4, i64 4, i64 4>
; CHECK-NEXT: [[SCAL_NDX]] = add nuw nsw i64 [[SCAL_PHI]], 4
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %L1

L1:
  %cmp16 = icmp sgt i32 %N, 1
  br i1 %cmp16, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp1 = icmp eq i32 %c, 0
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %if.end
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void

for.body:                                         ; preds = %if.end, %for.body.lr.ph
  %indvars.iv = phi i64 [ 1, %for.body.lr.ph ], [ %indvars.iv.next, %if.end ]
  br i1 %cmp1, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %for.body, %if.then
  %.sink7 = phi i32 [ %0, %if.then ], [ 6, %for.body ]
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4
  %add6 = add nsw i32 %1, %.sink7
  store i32 %add6, i32* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
