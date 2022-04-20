; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -disable-output -print-after=hir-vplan-vec -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
;
; LIT test to demonstrate incorrect code generated for AllZeroCheck which is subsequently
; used as a branch condition. AllZeroCheck is a uniform instruction. When generating the
; vector instruction, all lanes should get the same value. However, while doing the broadcast,
; we were marking the generated instruction as masked, which had the effect of this being
; a no-op for inactive lanes. This will cause a problem later when we try to extract lane 0
; value and lane 0 happens to be inactive causing wrong branch to be taken.
;
; Incoming HIR:
;      + DO i1 = 0, 7, 1   <DO_LOOP> <simd>
;      |   %ubval = (%ub)[i1];
;      |
;      |   + DO i2 = 0, %ubval + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;      |   |   (@A)[0][i2][i1] = i1;
;      |   + END LOOP
;      + END LOOP
;
; CHECK:           + DO i1 = 0, 7, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:      |   %all.zero.check10 = undef;
; CHECK-NEXT:      |   %.vec = (<4 x i64>*)(%ub)[i1];
; CHECK-NEXT:      |   %.vec3 = %.vec > 0;
; CHECK-NEXT:      |   %0 = bitcast.<4 x i1>.i4(%.vec3);
; CHECK-NEXT:      |   %cmp = %0 == 0;
; CHECK-NEXT:      |   %all.zero.check = %cmp;
; CHECK-NEXT:      |   %unifcond = extractelement %all.zero.check,  0;
; CHECK-NEXT:      |   if (%unifcond != 1)
; CHECK-NEXT:      |   {
; CHECK-NEXT:      |      %phi.temp = 0;
; CHECK-NEXT:      |      %phi.temp4 = %.vec3;
; CHECK-NEXT:      |
; CHECK-NEXT:      |      + UNKNOWN LOOP i2 <novectorize>
; CHECK-NEXT:      |      |   <i2 = 0>
; CHECK-NEXT:      |      |   BB6.59:
; CHECK-NEXT:      |      |   %.vec6 = %.vec3  &  %phi.temp4;
; CHECK-NEXT:      |      |   (<4 x i64>*)(@A)[0][i2][i1] = i1 + <i64 0, i64 1, i64 2, i64 3>, Mask = @{%.vec6};
; CHECK-NEXT:      |      |   %.vec7 = i2 + 1 < %.vec;
; CHECK-NEXT:      |      |   %.vec8 = %.vec7  &  %phi.temp4;
; CHECK-NEXT:      |      |   %allz.and. = %.vec3  &  %.vec8;
; CHECK-NEXT:      |      |   %1 = bitcast.<4 x i1>.i4(%allz.and.);
; CHECK-NEXT:      |      |   %cmp9 = %1 == 0;
; FIXME: The next instruction needs to be generated unmasked so that the subsequent
; extractelement instruction gets the right value. Otherwise, the if condition is
; using a potentially undefined value leading to undesirable behavior.
; CHECK-NEXT:      |      |   %all.zero.check10 = %cmp9, Mask = @{%.vec3};
; CHECK-NEXT:      |      |   %phi.temp = i2 + 1;
; CHECK-NEXT:      |      |   %phi.temp4 = %.vec8;
; CHECK-NEXT:      |      |   %unifcond13 = extractelement %all.zero.check10,  0;
; CHECK-NEXT:      |      |   if (%unifcond13 != 1)
; CHECK-NEXT:      |      |   {
; CHECK-NEXT:      |      |      <i2 = i2 + 1>
; CHECK-NEXT:      |      |      goto BB6.59;
; CHECK-NEXT:      |      |   }
; CHECK-NEXT:      |      + END LOOP
; CHECK-NEXT:      |   }
; CHECK-NEXT:      + END LOOP
;
@A = external dso_local local_unnamed_addr global [100 x [100 x i64]], align 16

define void @foo(i64* %ub) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc5
  %i.016 = phi i64 [ 0, %entry ], [ %inc6, %for.inc5 ]
  %arrayidx = getelementptr inbounds i64, i64* %ub, i64 %i.016
  %ubval = load i64, i64* %arrayidx, align 8
  %cmp120 = icmp sgt i64 %ubval, 0
  br i1 %cmp120, label %inner.ph, label %for.inc5

inner.ph:
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %j.015 = phi i64 [ 0, %inner.ph ], [ %inc, %for.body3 ]
  %arrayidx4 = getelementptr inbounds [100 x [100 x i64]], [100 x [100 x i64]]* @A, i64 0, i64 %j.015, i64 %i.016
  store i64 %i.016, i64* %arrayidx4, align 8
  %inc = add nuw nsw i64 %j.015, 1
  %exitcond.not = icmp eq i64 %inc, %ubval
  br i1 %exitcond.not, label %inner.exit, label %for.body3

inner.exit:
  br label %for.inc5

for.inc5:                                         ; preds = %for.body3
  %inc6 = add nuw nsw i64 %i.016, 1
  %exitcond17.not = icmp eq i64 %inc6, 8
  br i1 %exitcond17.not, label %for.end7, label %for.cond1.preheader

for.end7:                                         ; preds = %for.inc5
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() #1
declare void @llvm.directive.region.exit(token) #1
