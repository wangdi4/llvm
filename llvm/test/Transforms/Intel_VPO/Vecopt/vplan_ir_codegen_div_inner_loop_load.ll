; Test for codegen of uniform loads inside divergent inner loops.

; RUN: opt < %s -VPlanDriver -vplan-force-vf=2 -S | FileCheck %s
; RUN: opt < %s -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen -S | FileCheck %s

; CHECK-LABEL: @test1
; CHECK:       vector.body:
; CHECK:         [[MASK:%.*]] = icmp eq <2 x i64> [[VEC_IND:%.*]], [[BROADCAST_SPLAT:%.*]]
; CHECK-NEXT:    br label %[[VPLANNEDBB:.*]]
; CHECK:       [[VPLANNEDBB]]:
; CHECK-NEXT:    [[INNER_VEC_PHI:%.*]] = phi <2 x i64> [ [[INNER_IV_ADD:%.*]], %[[PRED_LOAD_CONTINUE:.*]] ], [ zeroinitializer, [[VECTOR_BODY:%.*]] ]
; CHECK:         [[GEP:%.*]] = getelementptr inbounds i64, i64* [[ARR1:%.*]], i64 42
; CHECK-NEXT:    [[BC_MASK:%.*]] = bitcast <2 x i1> [[MASK]] to i2
; CHECK-NEXT:    [[NOT_AZ:%.*]] = icmp ne i2 [[BC_MASK]], 0
; CHECK-NEXT:    br i1 [[NOT_AZ]], label %[[PRED_LOAD_IF:.*]], label %[[MERGE:.*]]
; CHECK:       [[PRED_LOAD_IF]]:
; CHECK-NEXT:    [[LOAD:%.*]] = load i64, i64* [[GEP]]
; CHECK-NEXT:    [[BROADCAST_SPLATINSERT1:%.*]] = insertelement <2 x i64> undef, i64 [[LOAD]], i32 0
; CHECK-NEXT:    br label %[[MERGE]]
; CHECK:       [[MERGE]]:
; CHECK-NEXT:    [[MERGE_PHI:%.*]] = phi <2 x i64> [ undef, %[[VPLANNEDBB]] ], [ [[BROADCAST_SPLATINSERT1]], %[[PRED_LOAD_IF]] ]
; CHECK-NEXT:    br label %[[PRED_LOAD_CONTINUE:.*]]
; CHECK:       pred.load.continue:
; CHECK-NEXT:    [[BROADCAST_SPLAT2:%.*]] = shufflevector <2 x i64> [[MERGE_PHI]], <2 x i64> undef, <2 x i32> zeroinitializer
; CHECK-NEXT:    [[USER:%.*]] = add <2 x i64> [[BROADCAST_SPLAT2]], [[VEC_IND]]

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

define void @test1(float* nocapture %ptr, i64 %n, i64* %arr, i64* %arr1) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i64* %arr1, i64* %arr) ]
  br label %for.body

for.body:                                         ; preds = %for.latch, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  %outer.gep = getelementptr inbounds i64, i64* %arr, i64 42
  %outer.load = load i64, i64* %outer.gep
  %inner.cond = icmp eq i64 %indvars.iv, %outer.load
  br i1 %inner.cond, label %for.body2.preheader, label %for.latch

for.body2.preheader:                              ; preds = %for.body
  br label %for.body2

for.body2:                                        ; preds = %for.body2.preheader, %for.body2
  %indvars.iv2 = phi i64 [ %indvars.iv.next2, %for.body2 ], [ 0, %for.body2.preheader ]
  %uni.gep = getelementptr inbounds i64, i64* %arr1, i64 42
  %uni.load = load i64, i64* %uni.gep
  %use = add i64 %uni.load, %indvars.iv
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond2 = icmp eq i64 %indvars.iv.next2, %n
  br i1 %exitcond2, label %for.latch.loopexit, label %for.body2

for.latch.loopexit:                               ; preds = %for.body2
  br label %for.latch

for.latch:                                        ; preds = %for.latch.loopexit, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.latch
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:                                             ; preds = %for.end, %entry
  ret void
}

; Check for inner loop with live out uniform GEP. CHECK for GEP instruction is explicitly omitted
; because of bcast pattern differences between IR-based CG and VPValue-based CG.
; CHECK-LABEL: @test1_liveout
; CHECK:       VPlannedBB:
; CHECK-NEXT:    [[INNER_VEC_PHI:%.*]] = phi <2 x i64> [ [[INNER_IV_ADD:%.*]], [[VPLANNEDBB:%.*]] ], [ zeroinitializer, [[VECTOR_BODY:%.*]] ]
; CHECK:         [[EXTRACT_GEP:%.*]] = extractelement <2 x i64*> [[GEP:%.*]], i32 0
; CHECK-NEXT:    [[BC_MASK:%.*]] = bitcast <2 x i1> [[MASK:%.*]] to i2
; CHECK-NEXT:    [[NOT_AZ:%.*]] = icmp ne i2 [[BC_MASK]], 0
; CHECK-NEXT:    br i1 [[NOT_AZ]], label %[[PRED_LOAD_IF:.*]], label %[[MERGE:.*]]
; CHECK:       [[PRED_LOAD_IF]]:
; CHECK-NEXT:    [[LOAD:%.*]] = load i64, i64* [[EXTRACT_GEP]]
; CHECK-NEXT:    [[BCAST_INSERT:%.*]] = insertelement <2 x i64> undef, i64 [[LOAD]], i32 0
; CHECK-NEXT:    br label %[[MERGE]]
; CHECK:       [[MERGE]]:
; CHECK-NEXT:    [[MERGE_PHI:%.*]] = phi <2 x i64> [ undef, %VPlannedBB ], [ [[BCAST_INSERT]], %[[PRED_LOAD_IF]] ]
; CHECK-NEXT:    br label %[[PRED_LOAD_CONTINUE:.*]]
; CHECK:       [[PRED_LOAD_CONTINUE]]:
; CHECK-NEXT:    [[BCAST_SHUF:%.*]] = shufflevector <2 x i64> [[MERGE_PHI]], <2 x i64> undef, <2 x i32> zeroinitializer
; CHECK-NEXT:    [[USER:%.*]] = add <2 x i64> [[BCAST_SHUF]], [[VEC_IND:%.*]]

define void @test1_liveout(float* nocapture %ptr, i64 %n, i64* %arr, i64* %arr1) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i64* %arr1, i64* %arr) ]
  br label %for.body

for.body:                                         ; preds = %for.latch, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  %outer.gep = getelementptr inbounds i64, i64* %arr, i64 42
  %outer.load = load i64, i64* %outer.gep
  %inner.cond = icmp eq i64 %indvars.iv, %outer.load
  br i1 %inner.cond, label %for.body2.preheader, label %for.latch

for.body2.preheader:                              ; preds = %for.body
  br label %for.body2

for.body2:                                        ; preds = %for.body2.preheader, %for.body2
  %indvars.iv2 = phi i64 [ %indvars.iv.next2, %for.body2 ], [ 0, %for.body2.preheader ]
  %uni.gep = getelementptr inbounds i64, i64* %arr1, i64 42
  %uni.load = load i64, i64* %uni.gep
  %use = add i64 %uni.load, %indvars.iv
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond2 = icmp eq i64 %indvars.iv.next2, %n
  br i1 %exitcond2, label %for.latch.loopexit, label %for.body2

for.latch.loopexit:                               ; preds = %for.body2
  %lcssa.phi = phi i64* [ %uni.gep, %for.body2 ]
  br label %for.latch

for.latch:                                        ; preds = %for.latch.loopexit, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.latch
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:                                             ; preds = %for.end, %entry
  ret void
}
