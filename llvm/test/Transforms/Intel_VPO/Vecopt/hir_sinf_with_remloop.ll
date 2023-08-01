; Check math library calls in remainder loops use scalar variant provided by
; SVML if available, otherwise fall back to the same function as in the vector
; loop body.

; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=4 -vplan-enable-non-masked-vectorized-remainder=0 -vplan-enable-masked-vectorized-remainder=0 < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-SVML %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=LIBMVEC-X86 -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=4 -vplan-enable-non-masked-vectorized-remainder=0 -vplan-enable-masked-vectorized-remainder=0 < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-LIBMVEC %s


; Checks for generated HIR code
; CHECK-LABEL:         Function: test_sinf
; CHECK:               + DO i1 = 0, 127, 4   <DO_LOOP>
; CHECK-NEXT:          |   [[SRC:%.*]] = (<4 x float>*)(%b)[i1];
; CHECK-SVML-NEXT:     |   [[RESULT:%.*]] = @__svml_sinf4([[SRC]]);
; CHECK-LIBMVEC-NEXT:  |   [[RESULT:%.*]] = @_ZGVbN4v_sinf([[SRC]]);
; CHECK-NEXT:          |   (<4 x float>*)(%a)[i1] = [[RESULT]];
; CHECK-NEXT:          + END LOOP

; CHECK:               + DO i1 = {{.*}}, 130, 1   <DO_LOOP>
; CHECK-NEXT:          |   [[SRC_REM:%.*]] = (%b)[i1];
; CHECK-NEXT:          |   [[SRC_REM_VEC:%.*]] = [[SRC_REM]];
; CHECK-SVML-NEXT:     |   [[RESULT_REM_VEC:%.*]] = @__svml_sinf1([[SRC_REM_VEC]]);
; CHECK-LIBMVEC-NEXT:  |   [[RESULT_REM_VEC:%.*]] = @_ZGVbN4v_sinf([[SRC_REM_VEC]]);
; CHECK-NEXT:          |   [[RESULT_REM:%.*]] = extractelement [[RESULT_REM_VEC]],  0;
; CHECK-NEXT:          |   (%a)[i1] = [[RESULT_REM]]
; CHECK-NEXT:          + END LOOP

; CHECK-LABEL:         Function: test_sin
; CHECK:               + DO i1 = 0, 127, 4   <DO_LOOP>
; CHECK-NEXT:          |   [[SRC:%.*]] = (<4 x double>*)(%b)[i1];
; CHECK-SVML-NEXT:     |   [[RESULT:%.*]] = @__svml_sin4([[SRC]]);
; CHECK-LIBMVEC-NEXT:  |   [[RESULT:%.*]] = @_ZGVdN4v_sin([[SRC]]);
; CHECK-NEXT:          |   (<4 x double>*)(%a)[i1] = [[RESULT]];
; CHECK-NEXT:          + END LOOP

; CHECK:               + DO i1 = {{.*}}, 130, 1   <DO_LOOP>
; CHECK-NEXT:          |   [[SRC_REM:%.*]] = (%b)[i1];
; CHECK-NEXT:          |   [[SRC_REM_VEC:%.*]] = [[SRC_REM]];
; CHECK-SVML-NEXT:     |   [[RESULT_REM_VEC:%.*]] = @__svml_sin1([[SRC_REM_VEC]]);
; CHECK-LIBMVEC-NEXT:  |   [[RESULT_REM_VEC:%.*]] = @_ZGVdN4v_sin([[SRC_REM_VEC]]);
; CHECK-NEXT:          |   [[RESULT_REM:%.*]] = extractelement [[RESULT_REM_VEC]],  0;
; CHECK-NEXT:          |   (%a)[i1] = [[RESULT_REM]]
; CHECK-NEXT:          + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test_sinf(ptr noalias nocapture %a, ptr noalias nocapture readonly %b) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %b, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4, !tbaa !1
  %call = tail call fast float @sinf(float %0) #3
  %arrayidx5 = getelementptr inbounds float, ptr %a, i64 %indvars.iv
  store float %call, ptr %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 131
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: nounwind uwtable
define void @test_sin(ptr noalias nocapture %a, ptr noalias nocapture readonly %b) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds double, ptr %b, i64 %indvars.iv
  %0 = load double, ptr %arrayidx, align 8, !tbaa !1
  %call = tail call fast double @sin(double %0) #3
  %arrayidx5 = getelementptr inbounds double, ptr %a, i64 %indvars.iv
  store double %call, ptr %arrayidx5, align 8, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 131
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: nounwind readnone
declare float @sinf(float) local_unnamed_addr #2

declare double @sin(double) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }

!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
