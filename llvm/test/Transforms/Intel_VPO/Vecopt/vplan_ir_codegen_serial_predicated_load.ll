; Test for predicated uniform loads in VPlan vectorizer codegen.

; RUN: opt %s -S -VPlanDriver -vplan-force-vf=2 | FileCheck %s
; RUN: opt %s -S -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen | FileCheck %s

; CHECK:       vector.body:
; CHECK:         [[MASK:%.*]] = icmp eq <2 x i64> [[VEC_IV:%.*]], <i64 42, i64 42>
; CHECK-NEXT:    [[GEP:%.*]] = getelementptr inbounds i64, i64* [[ARR:%.*]], i64 42
; CHECK-NEXT:    [[BC_MASK:%.*]] = bitcast <2 x i1> [[MASK]] to i2
; CHECK-NEXT:    [[NOT_AZ:%.*]] = icmp ne i2 [[BC_MASK]], 0
; CHECK-NEXT:    br i1 [[NOT_AZ]], label %[[PRED_LOAD_IF:.*]], label %[[MERGE:.*]]
; CHECK:       [[PRED_LOAD_IF]]:
; CHECK-NEXT:    [[LOAD:%.*]] = load i64, i64* [[GEP]]
; CHECK-NEXT:    [[BROADCAST_SPLATINSERT1:%.*]] = insertelement <2 x i64> undef, i64 [[LOAD]], i32 0
; CHECK-NEXT:    br label %[[MERGE]]
; CHECK:       [[MERGE]]:
; CHECK-NEXT:    [[MERGE_PHI:%.*]] = phi <2 x i64> [ undef, [[VECTOR_BODY:%.*]] ], [ [[BROADCAST_SPLATINSERT1]], %[[PRED_LOAD_IF]] ]
; CHECK-NEXT:    br label %[[PRED_LOAD_CONTINUE:.*]]
; CHECK:       [[PRED_LOAD_CONTINUE]]:
; CHECK-NEXT:    [[BROADCAST_SPLAT2:%.*]] = shufflevector <2 x i64> [[MERGE_PHI]], <2 x i64> undef, <2 x i32> zeroinitializer
; CHECK-NEXT:    [[USER:%.*]] = add <2 x i64> [[BROADCAST_SPLAT2]], [[VEC_IND:%.*]]

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

define void @test1(i64 %n, i64* %arr) {
  entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i64* %arr) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  %cond = icmp eq i64 %indvars.iv, 42
  br i1 %cond, label %if.then, label %for.latch

if.then:
  %gep = getelementptr inbounds i64, i64* %arr, i64 42
  %load = load i64, i64* %gep
  %use = add i64 %load, %indvars.iv
  br label %for.latch

for.latch:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %exit

exit:
  ret void
}
