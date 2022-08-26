; LLVM IR generated from testcase below using: icx -S -emit-llvm -Qoption,c,-fveclib=SVML -openmp -restrict -ffast-math -O2
;
;void foo(float* restrict a, float* restrict b) {
;  unsigned int i;
;  for (i = 0; i < N; i++) {
;    if (b[i] > 3)
;      a[i] = sinf(b[i]);
;  }
;}
;
; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S -vplan-force-vf=4 < %s 2>&1 | FileCheck -DVL=4 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-LT-512 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=4 < %s 2>&1 | FileCheck -DVL=4 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-LT-512 %s

; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S -vplan-force-vf=8 < %s 2>&1 | FileCheck -DVL=8 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-512 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=8 < %s 2>&1 | FileCheck -DVL=8 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-512 %s

; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S -vplan-force-vf=16 < %s 2>&1 | FileCheck -DVL=16 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=16 < %s 2>&1 | FileCheck -DVL=16 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s

; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S -vplan-force-vf=32 < %s 2>&1 | FileCheck -DVL=32 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=32 < %s 2>&1 | FileCheck -DVL=32 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s


; Test VPValue based code generation
; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S -vplan-force-vf=4 < %s 2>&1 | FileCheck -DVL=4 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-LT-512 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=4 < %s 2>&1 | FileCheck -DVL=4 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-LT-512 %s

; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S -vplan-force-vf=8 < %s 2>&1 | FileCheck -DVL=8 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-512 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=8 < %s 2>&1 | FileCheck -DVL=8 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-512 %s

; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S -vplan-force-vf=16 < %s 2>&1 | FileCheck -DVL=16 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=16 < %s 2>&1 | FileCheck -DVL=16 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s

; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -S -vplan-force-vf=32 < %s 2>&1 | FileCheck -DVL=32 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=32 < %s 2>&1 | FileCheck -DVL=32 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s


; Checks for generated HIR code
; CHECK-LABEL:         Function: test_sinf
; CHECK:               + DO i1 = 0, 127, [[VL]]   <DO_LOOP>
; CHECK-NEXT:          |   [[SRC:%.*]] = (<[[VL]] x float>*)(%b)[i1];
; CHECK-NEXT:          |   [[WIDECMP:%.*]] = [[SRC]] > 3.000000e+00;
; FLOAT-LT-512-NEXT:   |   [[MASKEXT:%.*]] = sext.<[[VL]] x i1>.<[[VL]] x i32>([[WIDECMP]]);
; FLOAT-LT-512-NEXT:   |   [[RESULT:%.*]] = @__svml_sinf[[VL]]_mask([[SRC]],  [[MASKEXT]]);
; FLOAT-512-NEXT:      |   [[RESULT:%.*]] = @__svml_sinf[[VL]]_mask(undef, [[WIDECMP]], [[SRC]]);
; CHECK-NEXT:          |   (<[[VL]] x float>*)(%a)[i1] = [[RESULT]], Mask = @{[[WIDECMP]]};
; CHECK-NEXT:          + END LOOP

; CHECK:               + DO i1 = {{.*}}, 130, 1   <DO_LOOP>
; CHECK-NEXT:          |   [[SRC_REM:%.*]] = (%b)[i1];
; CHECK-NEXT:          |   if ([[SRC_REM]] > 3.000000e+00)
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      [[SRC_REM_VEC:%.*]] = [[SRC_REM]];
; CHECK-NEXT:          |      [[RESULT_REM_VEC:%.*]] = @__svml_sinf[[VL]]([[SRC_REM_VEC]]);
; CHECK-NEXT:          |      [[RESULT_REM:%.*]] = extractelement [[RESULT_REM_VEC]],  0;
; CHECK-NEXT:          |      (%a)[i1] = [[RESULT_REM]]
; CHECK-NEXT:          |   }
; CHECK-NEXT:          + END LOOP

; CHECK-LABEL:         Function: test_sin
; CHECK:               + DO i1 = 0, 127, [[VL]]   <DO_LOOP>
; CHECK-NEXT:          |   [[SRC:%.*]] = (<[[VL]] x double>*)(%b)[i1];
; CHECK-NEXT:          |   [[WIDECMP:%.*]] = [[SRC]] > 3.000000e+00;
; DOUBLE-LT-512-NEXT:  |   [[MASKEXT:%.*]] = sext.<[[VL]] x i1>.<[[VL]] x i64>([[WIDECMP]]);
; DOUBLE-LT-512-NEXT:  |   [[RESULT:%.*]] = @__svml_sin[[VL]]_mask([[SRC]],  [[MASKEXT]]);
; DOUBLE-512-NEXT:     |   [[RESULT:%.*]] = @__svml_sin[[VL]]_mask(undef, [[WIDECMP]], [[SRC]]);
; CHECK-NEXT:          |   (<[[VL]] x double>*)(%a)[i1] = [[RESULT]], Mask = @{[[WIDECMP]]};
; CHECK-NEXT:          + END LOOP

; CHECK:               + DO i1 = {{.*}}, 130, 1   <DO_LOOP>
; CHECK-NEXT:          |   [[SRC_REM:%.*]] = (%b)[i1];
; CHECK-NEXT:          |   if ([[SRC_REM]] > 3.000000e+00)
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      [[SRC_REM_VEC:%.*]] = [[SRC_REM]];
; CHECK-NEXT:          |      [[RESULT_REM_VEC:%.*]] = @__svml_sin[[VL]]([[SRC_REM_VEC]]);
; CHECK-NEXT:          |      [[RESULT_REM:%.*]] = extractelement [[RESULT_REM_VEC]],  0;
; CHECK-NEXT:          |      (%a)[i1] = [[RESULT_REM]]
; CHECK-NEXT:          |   }
; CHECK-NEXT:          + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test_sinf(float* noalias nocapture %a, float* noalias nocapture readonly %b) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %cmp1 = fcmp fast ogt float %0, 3.000000e+00
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %call = tail call fast float @sinf(float %0) #3
  %arrayidx5 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  store float %call, float* %arrayidx5, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 131
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

; Function Attrs: nounwind uwtable
define void @test_sin(double* noalias nocapture %a, double* noalias nocapture readonly %b) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds double, double* %b, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8, !tbaa !1
  %cmp1 = fcmp fast ogt double %0, 3.000000e+00
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %call = tail call fast double @sin(double %0) #3
  %arrayidx5 = getelementptr inbounds double, double* %a, i64 %indvars.iv
  store double %call, double* %arrayidx5, align 8, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 131
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
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
