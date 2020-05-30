; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -enable-vp-value-codegen-hir -vplan-force-vf=4 -hir-cg -simplifycfg -sroa -print-after=VPlanDriverHIR -print-after=sroa -disable-output < %s 2>&1 | FileCheck %s
;
; LIT test where the address corresponding to a unit stride ref is a blob. When
; generating a widened memory reference of the form blob[0], the type of 0 needs
; to remain scalar so that we generate a wide load/store. The test here checks
; that we do not generate a gather/scatter. The arrays arr1/arr2 are of type float.
; Due to LLVM canonicalization, the pointers are bitcast to i32 *. VPValue based
; code generation currently uses an extract instruction to get the scalar pointer
; for lane 0 to use in wide load/store. However, when creating the memref uni.idx[0]
; we were incorrectly setting the type of 0 to <VF x i64> causing a gather/scatter
; to lane 0 address.
;
; Scalar HIR:
;               DO i1 = 0, 99, 1   <DO_LOOP>
;                 %1 = (i32*)(@arr2)[0][i1];
;                 (i32*)(@arr1)[0][i1] = %1;
;               END LOOP
;
; CHECK-LABEL: *** IR Dump After VPlan Vectorization Driver HIR ***
; CHECK:        DO i1 = 0, 99, 4   <DO_LOOP> <novectorize>
; CHECK-NEXT:     %uni.idx = extractelement &((<4 x i32*>)(@arr2)[0][i1 + <i64 0, i64 1, i64 2, i64 3>]),  0;
; CHECK-NEXT:     %.vec = (<4 x i32>*)(%uni.idx)[0];
; CHECK-NEXT:     %uni.idx1 = extractelement &((<4 x i32*>)(@arr1)[0][i1 + <i64 0, i64 1, i64 2, i64 3>]),  0;
; CHECK-NEXT:     (<4 x i32>*)(%uni.idx1)[0] = %.vec;
; CHECK-NEXT    END LOOP
;
; Checks for wide load/store.
; CHECK-LABEL:  *** IR Dump After SROA ***
; CHECK:         [[ARRAYIDX0:%.*]] = getelementptr inbounds [100 x float], <4 x [100 x float]*> <[100 x float]* @arr2, [100 x float]* @arr2, [100 x float]* @arr2, [100 x float]* @arr2>, <4 x i64> zeroinitializer, <4 x i64> [[TMP0:.*]]
; CHECK-NEXT:    [[TMP1:%.*]] = bitcast <4 x float*> [[ARRAYIDX0]] to <4 x i32*>
; CHECK-NEXT:    [[UNI_IDX20:%.*]] = extractelement <4 x i32*> [[TMP1]], i64 0
; CHECK-NEXT:    [[TMP2:%.*]] = bitcast i32* [[UNI_IDX20]] to <4 x i32>*
; CHECK-NEXT:    [[GEPLOAD0:%.*]] = load <4 x i32>, <4 x i32>* [[TMP2]], align 4
; CHECK:         [[ARRAYIDX50:%.*]] = getelementptr inbounds [100 x float], <4 x [100 x float]*> <[100 x float]* @arr1, [100 x float]* @arr1, [100 x float]* @arr1, [100 x float]* @arr1>, <4 x i64> zeroinitializer, <4 x i64> [[TMP3:.*]]
; CHECK-NEXT:    [[TMP4:%.*]] = bitcast <4 x float*> [[ARRAYIDX50]] to <4 x i32*>
; CHECK-NEXT:    [[UNI_IDX160:%.*]] = extractelement <4 x i32*> [[TMP4]], i64 0
; CHECK-NEXT:    [[TMP5:%.*]] = bitcast i32* [[UNI_IDX160]] to <4 x i32>*
; CHECK-NEXT:    store <4 x i32> [[GEPLOAD0]], <4 x i32>* [[TMP5]], align 4

@arr2 = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@arr1 = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

define void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.06 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @arr2, i64 0, i64 %l1.06
  %0 = bitcast float* %arrayidx to i32*
  %1 = load i32, i32* %0, align 4
  %arrayidx1 = getelementptr inbounds [100 x float], [100 x float]* @arr1, i64 0, i64 %l1.06
  %2 = bitcast float* %arrayidx1 to i32*
  store i32 %1, i32* %2, align 4
  %inc = add nuw nsw i64 %l1.06, 1
  %cmp = icmp ult i64 %l1.06, 99
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}
