; LLVM IR generated from testcase below using: icx -S -emit-llvm -Qoption,c,-fveclib=SVML -restrict -ffast-math -O3
;
;void unit_strided(float * restrict sinA, float * restrict cosA,
;                 float * restrict sinB, float * restrict cosB, int N) {
;  for (int i = 0; i < N; ++i) {
;    sincosf(i, sinA + i, cosA + i);
;    if (!(i & 1)) {
;      sincosf(i, sinB + i, cosB + i);
;    }
;  }
;}
;
;void non_unit_strided(float * restrict sinA, float * restrict cosA,
;                 float * restrict sinB, float * restrict cosB, int N) {
;  for (int i = 0; i < N; ++i) {
;    sincosf(i, sinA + 2 * i, cosA + 2 * i);
;    if (!(i & 1)) {
;      sincosf(i, sinB + 2 * i, cosB + 2 * i);
;    }
;  }
;}
;
; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -verify -print-after=hir-vplan-vec -S -vplan-force-vf=4 < %s 2>&1 | FileCheck -D#VL=4 --check-prefixes=CHECK,CHECK-128 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg,verify" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=4 < %s 2>&1 | FileCheck -D#VL=4 --check-prefixes=CHECK,CHECK-128 %s

; RUN: opt -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -verify -print-after=hir-vplan-vec -S -vplan-force-vf=16 < %s 2>&1 | FileCheck -D#VL=16 --check-prefixes=CHECK,CHECK-512 %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg,verify" -vector-library=SVML -vplan-enable-all-zero-bypass-non-loops=false -S -vplan-force-vf=16 < %s 2>&1 | FileCheck -D#VL=16 --check-prefixes=CHECK,CHECK-512 %s

; Check to see that the main vector loop was vectorized with svml and
; remainder loop broadcasts the call arguments and uses svml to match the main
; vector loop.

; CHECK-LABEL: Function: unit_strided
; CHECK:         + DO i1 = 0, {{.*}}, [[#VL]]   <DO_LOOP>  <MAX_TC_EST = {{.*}}> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:    |   [[SRC_UNIT:%.*]] = sitofp.<[[#VL]] x i32>.<[[#VL]] x float>(i1 + <i64 0, i64 1, i64 2, i64 3
; CHECK-NEXT:    |   [[RET_UNIT:%.*]] = @__svml_sincosf[[#VL]]([[SRC_UNIT]]);
; CHECK-NEXT:    |   %vp.sincos.sin = extractvalue [[RET_UNIT]], 0;
; CHECK-NEXT:    |   %vp.sincos.cos = extractvalue [[RET_UNIT]], 1;
; CHECK-NEXT:    |   (<[[#VL]] x float>*)(%sinA)[i1] = %vp.sincos.sin;
; CHECK-NEXT:    |   (<[[#VL]] x float>*)(%cosA)[i1] = %vp.sincos.cos;
; CHECK-128:     |   [[MASK_EXT_UNIT:%.*]] = sext.<[[#VL]] x i1>.<[[#VL]] x i32>([[MASK_UNIT:%.*]]);
; CHECK-128:     |   [[RET_MASK_UNIT:%.*]] = @__svml_sincosf4_mask([[SRC_UNIT]], [[MASK_EXT_UNIT]]);
; CHECK-512:     |   [[RET_MASK_UNIT:%.*]] = @__svml_sincosf16_mask(undef, [[MASK_UNIT:%.*]], [[SRC_UNIT]]);
; CHECK-NEXT:    |   [[SIN_MASK_UNIT:%.*]] = extractvalue [[RET_MASK_UNIT]], 0;
; CHECK-NEXT:    |   [[COS_MASK_UNIT:%.*]] = extractvalue [[RET_MASK_UNIT]], 1;
; CHECK-NEXT:    |   (<[[#VL]] x float>*)(%sinB)[i1] = [[SIN_MASK_UNIT]], Mask = @{[[MASK_UNIT]]};
; CHECK-NEXT:    |   (<[[#VL]] x float>*)(%cosB)[i1] = [[COS_MASK_UNIT]], Mask = @{[[MASK_UNIT]]};
; CHECK-NEXT:    + END LOOP
; CHECK:      }

; CHECK:      + DO i1 = {{.*}}, zext.i32.i64(%N) + -1, 1   <DO_LOOP>
; CHECK-NEXT: |   %conv = sitofp.i32.float(i1);
; CHECK-NEXT: |   %copy = %conv;
; CHECK-NEXT: |   [[RET_REM_UNIT:%.*]] = @__svml_sincosf[[#VL]](%copy);
; CHECK-NEXT: |   [[SIN_REM_UNIT:%.*]] = extractvalue [[RET_REM_UNIT]], 0;
; CHECK-NEXT: |   [[COS_REM_UNIT:%.*]] = extractvalue [[RET_REM_UNIT]], 1;
; CHECK-NEXT: |   [[SIN_SCALAR_REM_UNIT:%.*]] = extractelement [[SIN_REM_UNIT]],  0;
; CHECK-NEXT: |   (%sinA)[i1] = [[SIN_SCALAR_REM_UNIT]];
; CHECK-NEXT: |   [[COS_SCALAR_REM_UNIT:%.*]] = extractelement [[COS_REM_UNIT]],  0;
; CHECK-NEXT: |   (%cosA)[i1] = [[COS_SCALAR_REM_UNIT]];
; CHECK-NEXT: |   if (-1 * i1 == 0)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      [[COPY_REM_MASK_UNIT:%.*]] = %conv;
; CHECK-NEXT: |      [[RET_REM_MASK_UNIT:%.*]] = @__svml_sincosf[[#VL]]([[COPY_REM_MASK_UNIT]]);
; CHECK-NEXT: |      [[SIN_REM_MASK_UNIT:%.*]] = extractvalue [[RET_REM_MASK_UNIT]], 0;
; CHECK-NEXT: |      [[COS_REM_MASK_UNIT:%.*]] = extractvalue [[RET_REM_MASK_UNIT]], 1;
; CHECK-NEXT: |      [[SIN_SCALAR_REM_MASK_UNIT:%.*]] = extractelement [[SIN_REM_MASK_UNIT]],  0;
; CHECK-NEXT: |      (%sinB)[i1] = [[SIN_SCALAR_REM_MASK_UNIT]];
; CHECK-NEXT: |      [[COS_SCALAR_REM_MASK_UNIT:%.*]] = extractelement [[COS_REM_MASK_UNIT]],  0;
; CHECK-NEXT: |      (%cosB)[i1] = [[COS_SCALAR_REM_MASK_UNIT]];
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP

; Check to see that non-unit-strided destinations of sincos are vectorized to scatters
; CHECK-LABEL: Function: non_unit_strided
; CHECK:         + DO i1 = 0, {{.*}}, [[#VL]]   <DO_LOOP>
; CHECK-NEXT:    |   [[SRC_NONUNIT:%.*]] = sitofp.<[[#VL]] x i32>.<[[#VL]] x float>(i1 + <i64 0, i64 1, i64 2, i64 3
; CHECK:         |   [[RET_NONUNIT:%.*]] = @__svml_sincosf[[#VL]]([[SRC_NONUNIT]]);
; CHECK-NEXT:    |   %vp.sincos.sin = extractvalue [[RET_NONUNIT]], 0;
; CHECK-NEXT:    |   %vp.sincos.cos = extractvalue [[RET_NONUNIT]], 1;
; CHECK-NEXT:    |   (<[[#VL]] x float>*)(%sinA)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3{{.*}}>] = %vp.sincos.sin;
; CHECK-NEXT:    |   (<[[#VL]] x float>*)(%cosA)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3{{.*}}>] = %vp.sincos.cos;
; CHECK-128:     |   [[MASK_EXT_NONUNIT:%.*]] = sext.<[[#VL]] x i1>.<[[#VL]] x i32>([[MASK_NONUNIT:%.*]]);
; CHECK-128:     |   [[RET_MASK_NONUNIT:%.*]] = @__svml_sincosf4_mask([[SRC_NONUNIT]], [[MASK_EXT_NONUNIT]]);
; CHECK-512:     |   [[RET_MASK_NONUNIT:%.*]] = @__svml_sincosf16_mask(undef, [[MASK_NONUNIT:%.*]], [[SRC_NONUNIT]]);
; CHECK-NEXT:    |   [[SIN_MASK_NONUNIT:%.*]] = extractvalue [[RET_MASK_NONUNIT]], 0;
; CHECK-NEXT:    |   [[COS_MASK_NONUNIT:%.*]] = extractvalue [[RET_MASK_NONUNIT]], 1;
; CHECK-NEXT:    |   (<[[#VL]] x float>*)(%sinB)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3{{.*}}>] = [[SIN_MASK_NONUNIT]], Mask = @{[[MASK_NONUNIT]]};
; CHECK-NEXT:    |   (<[[#VL]] x float>*)(%cosB)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3{{.*}}>] = [[COS_MASK_NONUNIT]], Mask = @{[[MASK_NONUNIT]]};
; CHECK-NEXT:    + END LOOP
; CHECK:      }

; CHECK:      + DO i1 = {{.*}}, zext.i32.i64(%N) + -1, 1   <DO_LOOP>
; CHECK-NEXT: |   %conv = sitofp.i32.float(i1);
; CHECK-NEXT: |   %copy = %conv;
; CHECK-NEXT: |   [[RET_REM_NONUNIT:%.*]] = @__svml_sincosf[[#VL]](%copy);
; CHECK-NEXT: |   [[SIN_REM_NONUNIT:%.*]] = extractvalue [[RET_REM_NONUNIT]], 0;
; CHECK-NEXT: |   [[COS_REM_NONUNIT:%.*]] = extractvalue [[RET_REM_NONUNIT]], 1;
; CHECK-NEXT: |   [[SIN_SCALAR_REM_NONUNIT:%.*]] = extractelement [[SIN_REM_NONUNIT]],  0;
; CHECK-NEXT: |   (%sinA)[2 * i1] = [[SIN_SCALAR_REM_NONUNIT]];
; CHECK-NEXT: |   [[COS_SCALAR_REM_NONUNIT:%.*]] = extractelement [[COS_REM_NONUNIT]],  0;
; CHECK-NEXT: |   (%cosA)[2 * i1] = [[COS_SCALAR_REM_NONUNIT]];
; CHECK-NEXT: |   if (-1 * i1 == 0)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      [[COPY_REM_MASK_NONUNIT:%.*]] = %conv;
; CHECK-NEXT: |      [[RET_REM_MASK_NONUNIT:%.*]] = @__svml_sincosf[[#VL]]([[COPY_REM_MASK_NONUNIT]]);
; CHECK-NEXT: |      [[SIN_REM_MASK_NONUNIT:%.*]] = extractvalue [[RET_REM_MASK_NONUNIT]], 0;
; CHECK-NEXT: |      [[COS_REM_MASK_NONUNIT:%.*]] = extractvalue [[RET_REM_MASK_NONUNIT]], 1;
; CHECK-NEXT: |      [[SIN_SCALAR_REM_MASK_NONUNIT:%.*]] = extractelement [[SIN_REM_MASK_NONUNIT]],  0;
; CHECK-NEXT: |      (%sinB)[2 * i1] = [[SIN_SCALAR_REM_MASK_NONUNIT]];
; CHECK-NEXT: |      [[COS_SCALAR_REM_MASK_NONUNIT:%.*]] = extractelement [[COS_REM_MASK_NONUNIT]],  0;
; CHECK-NEXT: |      (%cosB)[2 * i1] = [[COS_SCALAR_REM_MASK_NONUNIT]];
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @unit_strided(float* noalias %sinA, float* noalias %cosA, float* noalias %sinB, float* noalias %cosB, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp17 = icmp sgt i32 %N, 0
  br i1 %cmp17, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count19 = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to float
  %add.ptr = getelementptr inbounds float, float* %sinA, i64 %indvars.iv
  %add.ptr2 = getelementptr inbounds float, float* %cosA, i64 %indvars.iv
  tail call void @sincosf(float %conv, float* %add.ptr, float* %add.ptr2) #3
  %and = and i32 %0, 1
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %add.ptr5 = getelementptr inbounds float, float* %sinB, i64 %indvars.iv
  %add.ptr7 = getelementptr inbounds float, float* %cosB, i64 %indvars.iv
  tail call void @sincosf(float %conv, float* %add.ptr5, float* %add.ptr7) #3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count19
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nofree nounwind uwtable
define void @non_unit_strided(float* noalias %sinA, float* noalias %cosA, float* noalias %sinB, float* noalias %cosB, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp17 = icmp sgt i32 %N, 0
  br i1 %cmp17, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count23 = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to float
  %1 = shl nuw nsw i64 %indvars.iv, 1
  %add.ptr = getelementptr inbounds float, float* %sinA, i64 %1
  %add.ptr3 = getelementptr inbounds float, float* %cosA, i64 %1
  tail call void @sincosf(float %conv, float* nonnull %add.ptr, float* nonnull %add.ptr3) #3
  %and = and i32 %0, 1
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %add.ptr7 = getelementptr inbounds float, float* %sinB, i64 %1
  %add.ptr10 = getelementptr inbounds float, float* %cosB, i64 %1
  tail call void @sincosf(float %conv, float* nonnull %add.ptr7, float* nonnull %add.ptr10) #3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count23
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind
declare void @sincosf(float, float*, float*) local_unnamed_addr #2

attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }

!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
