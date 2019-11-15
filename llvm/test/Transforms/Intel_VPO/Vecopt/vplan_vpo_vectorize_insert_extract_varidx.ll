; This test checks if we correctly widen the insertelement/extractelement
; instruction that have a non-const index


; Run the following command and intercept the function and print module
; before VPlanDriver is invoked on a function
;
; icx -Xclang -fintel-openmp-region -c -fopenmp -mllvm --vplan-driver
; -mllvm -vplan-force-vf=2 -mllvm --loopopt=0 tt2.cpp
;
; =====================tt2.cpp============================================
;typedef float float4 __attribute__ ((vector_size (16)));
;void setElement(float4* vec, float val, int i) {
;  int v_i = val, retVal=0;
;  #pragma omp simd
;  for (int i = 1; i < 4; i++) {
;    float4 t = vec[i];
;    t[v_i%i] = val;
;    vec[i] = t;
;  }
;}
;
;void setElement2(float4* vec, float val, int i) {
;  int v_i = val, retVal=0;
;  #pragma omp simd
;  for (int i = 1; i < 1024; i++) {
;    float4 t = vec[i];
;    t[v_i%4] = i;
;    vec[i] = t;
;  }
;}
;
;float getElement(float4* vec, float val, int i) {
;  int v_i = val;
;  float retVal = 0;
;  #pragma omp simd
;  for (int i = 1; i < 4; i++) {
;    float4 t = vec[i];
;    retVal += t[v_i%i];
;  }
;  return retVal;
;}
; =======================================================================


; RUN: opt %s -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -VPlanDriver \
; RUN: -vplan-force-vf=2 | FileCheck %s --check-prefix=CHECK-VF2

; Check the correct sequence for 'insertelement' with non-const index
; CHECK-LABEL:@_Z10setElementPU8__vectorffi
; CHECK-VF2:       [[VARIDX1:%.*]] = extractelement <2 x i32> [[VARIDXVEC:%.*]], i64 0
; CHECK-VF2-NEXT:  [[OFF1:%.*]] = add i32 0, [[VARIDX1]]
; CHECK-VF2-NEXT:  [[RES1:%.*]] = insertelement <8 x float> [[VEC:%.*]], float [[E1:%.*]], i32 [[OFF1]]
; CHECK-VF2-NEXT:  [[VARIDX2:%.*]] = extractelement <2 x i32> [[VARIDXVEC]], i64 1
; CHECK-VF2-NEXT:  [[OFF2:%.*]] = add i32 4, [[VARIDX2]]
; CHECK-VF2-NEXT:  [[RES2:%.*]] = insertelement <8 x float> [[RES1]], float [[E2:%.*]], i32 [[OFF2]]

; Check the correct sequence for 'insertelement' with non-const index and loop variant scalar value to be inserted
; CHECK-LABEL:@setElement2
; CHECK-VF2:       [[INSERTVAL:%.*]] = sitofp <2 x i32> [[IV:%.*]] to <2 x float>
; CHECK-VF2-NEXT:  [[E1:%.*]] = extractelement <2 x float> [[INSERTVAL]], i32 1
; CHECK-VF2-NEXT:  [[E2:%.*]] = extractelement <2 x float> [[INSERTVAL]], i32 0
; CHECK-VF2:       [[VARIDX1:%.*]] = extractelement <2 x i32> [[VARIDXVEC:%.*]], i64 0
; CHECK-VF2-NEXT:  [[OFF1:%.*]] = add i32 0, [[VARIDX1]]
; CHECK-VF2-NEXT:  [[RES1:%.*]] = insertelement <8 x float> [[VEC:%.*]], float [[E2]], i32 [[OFF1]]
; CHECK-VF2-NEXT:  [[VARIDX2:%.*]] = extractelement <2 x i32> [[VARIDXVEC]], i64 1
; CHECK-VF2-NEXT:  [[OFF2:%.*]] = add i32 4, [[VARIDX2]]
; CHECK-VF2-NEXT:  [[RES2:%.*]] = insertelement <8 x float> [[RES1]], float [[E1]], i32 [[OFF2]]

; Check the correct sequence for 'extractelement' with non-const index
; CHECK-LABEL:@_Z10getElementPDv4_ffi
; CHECK-VF2:       [[VARIDX1:%.*]] = extractelement <2 x i32> [[VARIDXVEC:%.*]], i64 0
; CHECK-VF2-NEXT:  [[OFF1:%.*]] = add i32 0, [[VARIDX1]]
; CHECK-VF2-NEXT:  [[RES1:%.*]] = extractelement <8 x float> [[VEC:%.*]], i32 [[OFF1]]
; CHECK-VF2-NEXT:  [[WIDE_EXTRACT1:%.*]] = insertelement <2 x float> {{.*}}, float [[RES1]], i64 0
; CHECK-VF2-NEXT:  [[VARIDX2:%.*]] = extractelement <2 x i32> [[VARIDXVEC]], i64 1
; CHECK-VF2-NEXT:  [[OFF2:%.*]] = add i32 4, [[VARIDX2]]
; CHECK-VF2-NEXT:  [[RES2:%.*]] = extractelement <8 x float> [[VEC]], i32 [[OFF2]]
; CHECK-VF2-NEXT:  [[WIDE_EXTRACT2:%.*]] = insertelement <2 x float> [[WIDE_EXTRACT1]], float [[RES2]], i64 1


source_filename = "tt2.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z10setElementPU8__vectorffi(<4 x float>* nocapture %vec, float %val, i32 %i) local_unnamed_addr #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %t = alloca <4 x float>, align 16
  %conv = fptosi float %val to i32
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  store i32 2, i32* %.omp.ub, align 4, !tbaa !2
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(<4 x float>* %t) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, i32* %.omp.iv, align 4, !tbaa !2
  %3 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp11 = icmp slt i32 %3, 0
  br i1 %cmp11, label %omp.loop.exit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.2
  %4 = bitcast <4 x float>* %t to i8*
  %5 = sext i32 %3 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %4) #2
  %arrayidx = getelementptr inbounds <4 x float>, <4 x float>* %vec, i64 %indvars.iv.next
  %6 = load <4 x float>, <4 x float>* %arrayidx, align 16, !tbaa !6
  %7 = trunc i64 %indvars.iv.next to i32
  %rem = srem i32 %conv, %7
  %vecins = insertelement <4 x float> %6, float %val, i32 %rem
  store <4 x float> %vecins, <4 x float>* %arrayidx, align 16, !tbaa !6
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #2
  %cmp = icmp slt i64 %indvars.iv, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %omp.inner.for.body
  %indvars.iv.next.lcssa = phi i64 [ %indvars.iv.next, %omp.inner.for.body ]
  %8 = trunc i64 %indvars.iv.next.lcssa to i32
  store i32 %8, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %DIR.OMP.SIMD.2
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local void @setElement2(<4 x float>* nocapture %vec, float %val, i32 %i) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %conv = fptosi float %val to i32
  %t.priv = alloca <4 x float>, align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(<4 x float>* %t.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast <4 x float>* %t.priv to i8*
  %rem = srem i32 %conv, 4
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.2 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %1) #2
  %arrayidx = getelementptr inbounds <4 x float>, <4 x float>* %vec, i64 %indvars.iv.next
  %2 = load <4 x float>, <4 x float>* %arrayidx, align 16, !tbaa !2
  %3 = trunc i64 %indvars.iv.next to i32
  %conv3 = sitofp i32 %3 to float
  %vecins = insertelement <4 x float> %2, float %conv3, i32 %rem
  store <4 x float> %vecins, <4 x float>* %arrayidx, align 16, !tbaa !2
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %1) #2
  %exitcond = icmp eq i64 %indvars.iv.next, 1023
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local float @_Z10getElementPDv4_ffi(<4 x float>* nocapture readonly %vec, float %val, i32 %i) local_unnamed_addr #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %t = alloca <4 x float>, align 16
  %conv = fptosi float %val to i32
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  store i32 2, i32* %.omp.ub, align 4, !tbaa !2
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(<4 x float>* %t) ]
  store i32 0, i32* %.omp.iv, align 4, !tbaa !2
  %3 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp9 = icmp slt i32 %3, 0
  br i1 %cmp9, label %omp.loop.exit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %entry
  %4 = bitcast <4 x float>* %t to i8*
  %5 = sext i32 %3 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %retVal.011 = phi float [ %add2, %omp.inner.for.body ], [ 0.000000e+00, %omp.inner.for.body.lr.ph ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %4) #2
  %arrayidx = getelementptr inbounds <4 x float>, <4 x float>* %vec, i64 %indvars.iv.next
  %6 = load <4 x float>, <4 x float>* %arrayidx, align 16, !tbaa !6
  %7 = trunc i64 %indvars.iv.next to i32
  %rem = srem i32 %conv, %7
  %vecext = extractelement <4 x float> %6, i32 %rem
  %add2 = fadd float %retVal.011, %vecext
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %4) #2
  %cmp = icmp slt i64 %indvars.iv, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %omp.inner.for.body
  %8 = trunc i64 %indvars.iv.next to i32
  store i32 %8, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %entry
  %retVal.0.lcssa = phi float [ %add2, %omp.inner.for.cond.omp.loop.exit_crit_edge ], [ 0.000000e+00, %entry ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret float %retVal.0.lcssa
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!4, !4, i64 0}
