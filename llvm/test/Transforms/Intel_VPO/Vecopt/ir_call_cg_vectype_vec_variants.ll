; Test to check correctness of LLVM-IR vector CG for re-vectorization of calls with SIMD
; vector-variants.

; RUN: opt -VPlanDriver -S < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; Function Attrs: nounwind uwtable
define dso_local void @foo(<4 x float>* nocapture readonly %in, <4 x float>* nocapture %out) local_unnamed_addr {
; CHECK:       vector.body:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i64 [ 0, [[VECTOR_PH:%.*]] ], [ [[INDEX_NEXT:%.*]], [[VECTOR_BODY:%.*]] ]
; CHECK-NEXT:    [[UNI_PHI:%.*]] = phi i64 [ [[TMP4:%.*]], [[VECTOR_BODY]] ], [ 0, [[VECTOR_PH]] ]
; CHECK-NEXT:    [[VEC_PHI:%.*]] = phi <4 x i64> [ [[TMP3:%.*]], [[VECTOR_BODY]] ], [ <i64 0, i64 1, i64 2, i64 3>, [[VECTOR_PH]] ]
; CHECK-NEXT:    [[SCALAR_GEP:%.*]] = getelementptr inbounds <4 x float>, <4 x float>* [[IN:%.*]], i64 [[UNI_PHI]]
; CHECK-NEXT:    [[TMP0:%.*]] = bitcast <4 x float>* [[SCALAR_GEP]] to <16 x float>*
; CHECK-NEXT:    [[WIDE_LOAD:%.*]] = load <16 x float>, <16 x float>* [[TMP0]], align 16
; CHECK-NEXT:    [[TMP1:%.*]] = call <16 x float> @_ZGVbN4v_simdBar(<16 x float> [[WIDE_LOAD]])
; CHECK-NEXT:    [[SCALAR_GEP1:%.*]] = getelementptr inbounds <4 x float>, <4 x float>* [[OUT:%.*]], i64 [[UNI_PHI]]
; CHECK-NEXT:    [[TMP2:%.*]] = bitcast <4 x float>* [[SCALAR_GEP1]] to <16 x float>*
; CHECK-NEXT:    store <16 x float> [[TMP1]], <16 x float>* [[TMP2]], align 16
; CHECK-NEXT:    [[TMP3]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 4, i64 4, i64 4, i64 4>
; CHECK-NEXT:    [[TMP4]] = add nuw nsw i64 [[UNI_PHI]], 4
; CHECK-NEXT:    [[TMP5:%.*]] = icmp eq <4 x i64> [[TMP3]], <i64 1024, i64 1024, i64 1024, i64 1024>
; CHECK-NEXT:    [[DOTEXTRACT_0_:%.*]] = extractelement <4 x i1> [[TMP5]], i32 0
; CHECK-NEXT:    [[INDEX_NEXT]] = add i64 [[INDEX]], 4
; CHECK-NEXT:    [[TMP6:%.*]] = icmp eq i64 [[INDEX_NEXT]], 1024
; CHECK-NEXT:    br i1 [[TMP6]], label [[VPLANNEDBB:%.*]], label [[VECTOR_BODY]]
;
omp.inner.for.body.lr.ph:
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.1 ]
  %arrayidx = getelementptr inbounds <4 x float>, <4 x float>* %in, i64 %indvars.iv
  %1 = load <4 x float>, <4 x float>* %arrayidx, align 16
  %call = call <4 x float> @simdBar(<4 x float> %1)
  %arrayidx2 = getelementptr inbounds <4 x float>, <4 x float>* %out, i64 %indvars.iv
  store <4 x float> %call, <4 x float>* %arrayidx2, align 16
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  ret void
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry()
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
declare dso_local <4 x float> @simdBar(<4 x float>) local_unnamed_addr #2

attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN4v_simdBar,_ZGVcN4v_simdBar,_ZGVdN4v_simdBar,_ZGVeN4v_simdBar,_ZGVbM4v_simdBar,_ZGVcM4v_simdBar,_ZGVdM4v_simdBar,_ZGVeM4v_simdBar" }

