; LLVM IR generated from testcase below using: icx -S -emit-llvm -Qoption,c,-fveclib=SVML -restrict -ffast-math -O3
;
;void sincos_calc(float * restrict sinA, float * restrict cosA, int N) {
;  for (int i = 0; i < N; ++i) {
;    sincosf(i, sinA + i, cosA + i);
;  }
;}

;void acos_calc(double * resA, int N) {
;  for (int i = 0; i < N; ++i) {
;    resA[i] = sinf(i);
;  }
;}
;;
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vector-library=SVML -vplan-enable-masked-vectorized-remainder=0 -vplan-enable-non-masked-vectorized-remainder=0 -vplan-cost-model-print-analysis-for-vf=2,4,8,16 < %s 2>&1 | FileCheck %s

; Check to see that the cost model correctly determines pump factor for svml call

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @sincos_calc(ptr noalias %sinA, ptr noalias %cosA, ptr noalias %sinB, ptr noalias %cosB, i32 %N) local_unnamed_addr #0 {
; CHECK:  Cost 102 for call float [[VAL:%.*]] ptr [[SINP:%.*]] ptr [[COSP:%.*]] __svml_sincosf2 [x 1]
; CHECK:  Cost 208 for call float [[VAL]] ptr [[SINP]] ptr [[COSP]] __svml_sincosf4 [x 1]
; CHECK:  Cost 432 for call float [[VAL]] ptr [[SINP]] ptr [[COSP]] __svml_sincosf8 [x 1]
; CHECK:  Cost 928 for call float [[VAL]] ptr [[SINP]] ptr [[COSP]] __svml_sincosf16 [x 1]
entry:
  %cmp17 = icmp sgt i32 %N, 0
  br i1 %cmp17, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count19 = zext i32 %N to i64
  br label %simd.loop

simd.loop:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body


for.cond.cleanup.loopexit:                        ; preds = %for.inc
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %simd.loop ], [ %indvars.iv.next, %for.inc ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to float
  %add.ptr = getelementptr inbounds float, ptr %sinA, i64 %indvars.iv
  %add.ptr2 = getelementptr inbounds float, ptr %cosA, i64 %indvars.iv
  tail call void @sincosf(float %conv, ptr %add.ptr, ptr %add.ptr2) #3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count19
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

define void @acos_calc(ptr noalias %resA, i32 %N) local_unnamed_addr #0 {
; CHECK:  Cost 102 for double [[VAL:%.*]] = call double [[VAL2:%.*]] __svml_acos2 [x 1]
; CHECK:  Cost 208 for double [[VAL]] = call double [[VAL2]] __svml_acos4 [x 1]
; CHECK:  Cost 432 for double [[VAL]] = call double [[VAL2]] __svml_acos8 [x 1]
; CHECK:  Cost 928 for double [[VAL]] = call double [[VAL2]] __svml_acos16 [x 1]
entry:
  %cmp17 = icmp sgt i32 %N, 0
  br i1 %cmp17, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count19 = zext i32 %N to i64
  br label %simd.loop

simd.loop:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body


for.cond.cleanup.loopexit:                        ; preds = %for.inc
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %simd.loop ], [ %indvars.iv.next, %for.inc ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to double
  %res = tail call fast nofpclass(nan inf) double @acos(double noundef nofpclass(nan inf) %conv) #1
  %add.ptr = getelementptr inbounds double, ptr %resA, i64 %indvars.iv
  store double %res, ptr %add.ptr
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count19
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: nounwind
declare void @sincosf(float, ptr, ptr) local_unnamed_addr

; Function Attrs: nounwind
declare dso_local nofpclass(nan inf) double @acos(double noundef nofpclass(nan inf))

