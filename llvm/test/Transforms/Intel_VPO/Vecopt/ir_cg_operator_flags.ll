; Check if vector codegen copies operator-related attributes to generated
; vector code.

; RUN: opt < %s -S -passes=vplan-vec -vplan-force-vf=2 | FileCheck %s

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind

define void @test1(i64 %n, ptr %arr1, ptr %arr2) {
; CHECK-LABEL: @test1(
; CHECK:       vector.body:
; CHECK:         [[TMP0:%.*]] = ashr exact <2 x i64> [[VEC_PHI:%.*]], <i64 32, i64 32>
; CHECK-NEXT:    [[TMP1:%.*]] = mul nuw nsw <2 x i64> [[TMP0]], <i64 42, i64 42>
; CHECK:         [[TMP2:%.*]] = fadd fast <2 x float> [[WIDE_MASKED_GATHER:%.*]], [[WIDE_MASKED_GATHER5:%.*]]
; CHECK-NEXT:    [[TMP3:%.*]] = add nuw nsw <2 x i64> [[VEC_PHI]], <i64 2, i64 2>
; CHECK-NEXT:    [[TMP4:%.*]] = add nuw nsw i64 [[UNI_PHI1:%.*]], 2
;
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.body.lr.ph ], [ %iv.next, %for.body ]
  %exact = ashr exact i64 %iv, 32
  %overflow = mul nsw nuw i64 %exact, 42
  %gep1 = getelementptr inbounds float, ptr %arr1, i64 %overflow
  %load1 = load float, ptr %gep1
  %gep2 = getelementptr inbounds float, ptr %arr2, i64 %overflow
  %load2 = load float, ptr %gep2
  %fmf = fadd fast float %load1, %load2
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %exit

exit:
  ret void
}
