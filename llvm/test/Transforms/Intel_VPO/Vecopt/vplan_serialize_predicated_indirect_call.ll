; Test for predicated indirect call serialization.

; RUN: opt %s -VPlanDriver -vplan-force-vf=2 -S | FileCheck %s
; RUN: opt %s -VPlanDriver -enable-vp-value-codegen -vplan-force-vf=2 -S | FileCheck %s

; CHECK-LABEL: vector.body:
; CHECK:         [[F1:%.*]] = extractelement <2 x i32 (i32)*> [[WIDE_MASKED_LOAD:%.*]], i32 1
; CHECK-NEXT:    [[F0:%.*]] = extractelement <2 x i32 (i32)*> [[WIDE_MASKED_LOAD]], i32 0
; CHECK:         [[ARG1:%.*]] = extractelement <2 x i32> [[WIDE_ARGS:%.*]], i32 1
; CHECK-NEXT:    [[ARG0:%.*]] = extractelement <2 x i32> [[WIDE_ARGS]], i32 0
; CHECK-NEXT:    [[PREDICATE0:%.*]] = extractelement <2 x i1> [[COND:%.*]], i64 0
; CHECK-NEXT:    [[TMP10:%.*]] = icmp eq i1 [[PREDICATE0]], true
; CHECK-NEXT:    br i1 [[TMP10]], label %[[PRED_CALL_IF:.*]], label %[[TMP12:.*]]
; CHECK:       [[PRED_CALL_IF]]:
; CHECK-NEXT:    [[TMP11:%.*]] = call i32 [[F0]](i32 [[ARG0]])
; CHECK-NEXT:    br label %[[TMP12]]
; CHECK:       [[TMP12]]:
; CHECK-NEXT:    [[TMP13:%.*]] = phi i32 [ undef, [[VECTOR_BODY:%.*]] ], [ [[TMP11]], %[[PRED_CALL_IF]] ]
; CHECK-NEXT:    br label %[[PRED_CALL_CONTINUE:.*]]
; CHECK:       [[PRED_CALL_CONTINUE]]:
; CHECK-NEXT:    [[PREDICATE1:%.*]] = extractelement <2 x i1> [[COND]], i64 1
; CHECK-NEXT:    [[TMP14:%.*]] = icmp eq i1 [[PREDICATE1]], true
; CHECK-NEXT:    br i1 [[TMP14]], label %[[PRED_CALL_IF4:.*]], label %[[TMP16:.*]]
; CHECK:       [[PRED_CALL_IF4]]:
; CHECK-NEXT:    [[TMP15:%.*]] = call i32 [[F1]](i32 [[ARG1]])
; CHECK-NEXT:    br label %[[TMP16]]
; CHECK:       [[TMP16]]:
; CHECK-NEXT:    [[TMP17:%.*]] = phi i32 [ undef, %[[PRED_CALL_CONTINUE]] ], [ [[TMP15]], %[[PRED_CALL_IF4]] ]
; CHECK-NEXT:    br label [[PRED_CALL_CONTINUE5:%.*]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @bar(i32* nocapture readonly %p1, i32 (i32)** nocapture readonly %p2, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.119

DIR.OMP.SIMD.119:                                 ; preds = %DIR.OMP.SIMD.1
  %wide.trip.count = sext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.119
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.119 ], [ %indvars.iv.next, %omp.body.continue ]
  %arrayidx = getelementptr inbounds i32, i32* %p1, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp6 = icmp sgt i32 %1, %n
  br i1 %cmp6, label %if.then, label %omp.body.continue

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx8 = getelementptr inbounds i32 (i32)*, i32 (i32)** %p2, i64 %indvars.iv
  %2 = load i32 (i32)*, i32 (i32)** %arrayidx8, align 8, !tbaa !6
  %3 = trunc i64 %indvars.iv to i32
  %call = call i32 %2(i32 %3) #1
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.body.continue
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSPFiiE", !4, i64 0}
