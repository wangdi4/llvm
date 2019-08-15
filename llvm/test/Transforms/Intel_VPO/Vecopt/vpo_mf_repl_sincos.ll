; RUN: opt < %s -S -replace-with-math-library-functions -mf-x86-target -iml-trans | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; TEST 1
;
; The llvm.sin and llvm.cos calls below should be combined to:

; %cos.ptr = alloca float
; ...
; ...OMP.SIMD "QUAL.OMP.PRIVATE"(float* %cos.ptr)
; ...
; %3 = call fast float @_Z6sincosfPf(float %div, float* %cos.ptr)
; %cos.val = load float, float* %cos.ptr
; %add3 = fadd fast float %3, %cos.val

; CHECK: %cos.ptr = alloca float
; CHECK: PRIVATE{{.*}}%cos.ptr
; CHECK: [[SINVAL:%[a-z0-9.]+]] = call fast float @_Z6sincosfPf
; CHECK: %cos.val = load float, float* %cos.ptr
; CHECK: fadd{{.*}}[[SINVAL]]{{.*}}%cos.val

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z3fooPfS_(float* noalias nocapture readonly %f, float* noalias nocapture %r) local_unnamed_addr #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
omp.inner.for.body.lr.ph:
  %val.priv = alloca float, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv), "QUAL.OMP.PRIVATE"(float* %val.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast float* %val.priv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.2 ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  %arrayidx = getelementptr inbounds float, float* %f, i64 %indvars.iv
  %2 = load float, float* %arrayidx, align 4
  %div = fmul fast float %2, 0x3FD5555560000000
  %3 = call fast float @llvm.sin.f32(float %div) #2
  %4 = call fast float @llvm.cos.f32(float %div) #2
  %add3 = fadd fast float %3, %4
  %arrayidx5 = getelementptr inbounds float, float* %r, i64 %indvars.iv
  store float %add3, float* %arrayidx5, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  store i32 99, i32* %i.lpriv, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.417

DIR.OMP.END.SIMD.417:                             ; preds = %DIR.OMP.END.SIMD.3
  ret void
}

; TEST 2
;
; The sincosf call below should be rewritten like this:

; call void @sincosf(float %div, float* nonnull %s.priv, float* nonnull %c.priv) #0
; =>
; %6 = call float @_Z6sincosfPf(float %div, float* %c.priv)
; store float %6, float* %s.priv

; CHECK-NOT: @sincosf
; CHECK: [[SINVAL:%[a-z0-9.]+]] = call float @_Z6sincosfPf
; CHECK: store float [[SINVAL]], float* %s.priv

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo2(float* noalias nocapture readonly %f, float* noalias nocapture %r) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %c.priv = alloca float, align 4
  %s.priv = alloca float, align 4
  %val.priv = alloca float, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv), "QUAL.OMP.PRIVATE"(float* %val.priv), "QUAL.OMP.PRIVATE"(float* %s.priv), "QUAL.OMP.PRIVATE"(float* %c.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast float* %val.priv to i8*
  %2 = bitcast float* %s.priv to i8*
  %3 = bitcast float* %c.priv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.2 ]
  %4 = trunc i64 %indvars.iv to i32
  store i32 %4, i32* %i.lpriv, align 4
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  %arrayidx = getelementptr inbounds float, float* %f, i64 %indvars.iv
  %5 = load float, float* %arrayidx, align 4
  %div = fmul fast float %5, 0x3FD5555560000000
  store float %div, float* %val.priv, align 4
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #2
  call void @sincosf(float %div, float* nonnull %s.priv, float* nonnull %c.priv) #0
  %6 = load float, float* %s.priv, align 4
  %mul1 = fmul fast float %6, 2.000000e+00
  %7 = load float, float* %c.priv, align 4
  %add2 = fadd fast float %mul1, %7
  %8 = load i32, i32* %i.lpriv, align 4
  %idxprom3 = sext i32 %8 to i64
  %arrayidx4 = getelementptr inbounds float, float* %r, i64 %idxprom3
  store float %add2, float* %arrayidx4, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  ret void
}

; TEST 3:
; The output of the vectorizer is a widened 2-pointer sincos like this:
;
;  sincos_ret2ptr(%10, %c.priv.vec, %sinPtr.vec);
;  %wide.sin.InitVal = *%sinPtr.vec;
;  %wide.load21 = *%c.priv.vec;
;  %17 = %wide.sin.InitVal * 2.0f + %wide.load21
;
;  The real IR has some temp loads and stores.
;
;  %15 = call <4 x float> @_Z14sincos_ret2ptrDv4_fPS_S1_(<4 x float> %10, <4 x float>* %c.priv.vec, <4 x float>* %sinPtr.vec)
;  %wide.sin.InitVal = load <4 x float>, <4 x float>* %sinPtr.vec, align 4
;  store <4 x float> %wide.sin.InitVal, <4 x float>* %s.priv.vec, align 4
;  %wide.load20 = load <4 x float>, <4 x float>* %s.priv.vec, align 4
;  %16 = fmul fast <4 x float> %wide.load20, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
;  %wide.load21 = load <4 x float>, <4 x float>* %c.priv.vec, align 4
;  %17 = fadd fast <4 x float> %16, %wide.load21
;
; We replace the 2-pointer sincos with __svml_sincosf4_ha. This takes a single
; float vector and returns a structure of 2 vectors.
;
; The -replace-with-math-library-functions pass will generate __svml_sincosf4
; as an intermediate step, then iml-trans replaces the generic SVML call with
; the variant __svml_sincosf4_ha.
;
;  The result looks like this.
;
;  %15 = call %sincosret @__svml_sincosf4_ha(<4 x float> %10) #5
;  %16 = extractvalue %sincosret %15, 0
;  %17 = extractvalue %sincosret %15, 1
;  store <4 x float> %16, <4 x float>* %s.priv.vec, align 4
;  %wide.load20 = load <4 x float>, <4 x float>* %s.priv.vec, align 4
;  %18 = fmul fast <4 x float> %wide.load20, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
;  %19 = fadd fast <4 x float> %18, %17
;
;
; We get rid of the loads of %sinPtr.vec and %c.priv.vec, and replace them with
; extracts of the SVML return values.
; We only are responsible for store-forwarding the former sincos pointer
; parameters, not any other stores (such as %s.priv.vec).

; CHECK-NOT: ret2ptr
; CHECK: [[SINCOSRET:%[a-z0-9.]+]] = call svml_cc %sincosret @__svml_sincosf4_ha(<4 x float> [[ANGLE:%[a-z0-9.]+]]
; CHECK: [[SINVAL:%[a-z0-9.]+]] = extractvalue %sincosret [[SINCOSRET]], 0
; CHECK: [[COSVAL:%[a-z0-9.]+]] = extractvalue %sincosret [[SINCOSRET]], 1
; CHECK: store {{.*}} [[SINVAL]], {{.*}} %s.priv.vec
; CHECK: [[SINVALCOPY:%[a-z0-9.]+]] = load {{.*}} %s.priv.vec
; CHECK: [[SIN2X:%[a-z0-9.]+]] = fmul {{.*}} [[SINVALCOPY]]
; CHECK: fadd {{.*}} [[SIN2X]], [[COSVAL]]

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo3(float* noalias nocapture readonly %f, float* noalias nocapture %r) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %c.priv.vec = alloca <4 x float>, align 4
  %sinPtr.vec = alloca <4 x float>, align 4
  %s.priv.vec = alloca <4 x float>, align 4
  %val.priv.vec = alloca <4 x float>, align 4
  %i.lpriv.vec = alloca <4 x i32>, align 4
  %c.priv = alloca float, align 4
  %s.priv = alloca float, align 4
  %val.priv = alloca float, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %0 = bitcast float* %val.priv to i8*
  %1 = bitcast float* %s.priv to i8*
  %2 = bitcast float* %c.priv to i8*
  br i1 false, label %scalar.ph, label %min.iters.checked

min.iters.checked:                                ; preds = %DIR.OMP.SIMD.2
  br i1 false, label %scalar.ph, label %vector.ph

vector.ph:                                        ; preds = %min.iters.checked
  %i.lprivInitVal = load i32, i32* %i.lpriv
  %i.lprivInitVal.splatinsert = insertelement <4 x i32> undef, i32 %i.lprivInitVal, i32 0
  %i.lprivInitVal.splat = shufflevector <4 x i32> %i.lprivInitVal.splatinsert, <4 x i32> undef, <4 x i32> zeroinitializer
  store <4 x i32> %i.lprivInitVal.splat, <4 x i32>* %i.lpriv.vec
  %val.privInitVal = load float, float* %val.priv
  %val.privInitVal.splatinsert = insertelement <4 x float> undef, float %val.privInitVal, i32 0
  %val.privInitVal.splat = shufflevector <4 x float> %val.privInitVal.splatinsert, <4 x float> undef, <4 x i32> zeroinitializer
  store <4 x float> %val.privInitVal.splat, <4 x float>* %val.priv.vec
  %3 = bitcast <4 x float>* %val.priv.vec to <4 x i8>*
  %s.privInitVal = load float, float* %s.priv
  %s.privInitVal.splatinsert = insertelement <4 x float> undef, float %s.privInitVal, i32 0
  %s.privInitVal.splat = shufflevector <4 x float> %s.privInitVal.splatinsert, <4 x float> undef, <4 x i32> zeroinitializer
  store <4 x float> %s.privInitVal.splat, <4 x float>* %s.priv.vec
  %4 = bitcast <4 x float>* %s.priv.vec to <4 x i8>*
  %c.privInitVal = load float, float* %c.priv
  %c.privInitVal.splatinsert = insertelement <4 x float> undef, float %c.privInitVal, i32 0
  %c.privInitVal.splat = shufflevector <4 x float> %c.privInitVal.splatinsert, <4 x float> undef, <4 x i32> zeroinitializer
  store <4 x float> %c.privInitVal.splat, <4 x float>* %c.priv.vec
  %5 = bitcast <4 x float>* %c.priv.vec to <4 x i8>*
  %broadcast.splatinsert = insertelement <4 x float*> undef, float* %r, i32 0
  %broadcast.splat = shufflevector <4 x float*> %broadcast.splatinsert, <4 x float*> undef, <4 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
  %vec.ind = phi <4 x i64> [ <i64 0, i64 1, i64 2, i64 3>, %vector.ph ], [ %vec.ind.next, %vector.body ]
  %6 = trunc <4 x i64> %vec.ind to <4 x i32>
  store <4 x i32> %6, <4 x i32>* %i.lpriv.vec, align 4
  %privaddr = bitcast <4 x i8>* %3 to i8*
  %7 = getelementptr i8, i8* %privaddr, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %8 = extractelement <4 x i8*> %7, i32 0
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %8) #2
  %scalar.gep. = getelementptr inbounds float, float* %f, i64 %index
  %9 = bitcast float* %scalar.gep. to <4 x float>*
  %wide.load = load <4 x float>, <4 x float>* %9, align 4
  %10 = fmul fast <4 x float> %wide.load, <float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000>
  store <4 x float> %10, <4 x float>* %val.priv.vec, align 4
  %privaddr18 = bitcast <4 x i8>* %4 to i8*
  %11 = getelementptr i8, i8* %privaddr18, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %12 = extractelement <4 x i8*> %11, i32 0
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %12) #2
  %privaddr19 = bitcast <4 x i8>* %5 to i8*
  %13 = getelementptr i8, i8* %privaddr19, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %14 = extractelement <4 x i8*> %13, i32 0
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %14) #2
  %15 = call <4 x float> @_Z14sincos_ret2ptrDv4_fPS_S1_(<4 x float> %10, <4 x float>* %c.priv.vec, <4 x float>* %sinPtr.vec)
  %wide.sin.InitVal = load <4 x float>, <4 x float>* %sinPtr.vec, align 4
  store <4 x float> %wide.sin.InitVal, <4 x float>* %s.priv.vec, align 4
  %wide.load20 = load <4 x float>, <4 x float>* %s.priv.vec, align 4
  %16 = fmul fast <4 x float> %wide.load20, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  %wide.load21 = load <4 x float>, <4 x float>* %c.priv.vec, align 4
  %17 = fadd fast <4 x float> %16, %wide.load21
  %wide.load22 = load <4 x i32>, <4 x i32>* %i.lpriv.vec, align 4
  %18 = sext <4 x i32> %wide.load22 to <4 x i64>
  %mm_vectorGEP = getelementptr inbounds float, <4 x float*> %broadcast.splat, <4 x i64> %18
  call void @llvm.masked.scatter.v4f32.v4p0f32(<4 x float> %17, <4 x float*> %mm_vectorGEP, i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %14) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %12) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %8) #2
  %19 = add nuw nsw <4 x i64> %vec.ind, <i64 1, i64 1, i64 1, i64 1>
  %20 = icmp eq <4 x i64> %19, <i64 256, i64 256, i64 256, i64 256>
  %21 = extractelement <4 x i1> %20, i32 0
  %index.next = add i64 %index, 4
  %22 = icmp eq i64 %index.next, 256
  %vec.ind.next = add <4 x i64> %vec.ind, <i64 4, i64 4, i64 4, i64 4>
  br i1 %22, label %VPlannedBB, label %vector.body

VPlannedBB:                                       ; preds = %vector.body
  br label %middle.block

middle.block:                                     ; preds = %VPlannedBB
  %cmp.n = icmp eq i64 256, 256
  %23 = bitcast <4 x i32>* %i.lpriv.vec to i32*
  %LastUpdatedLanePtr = getelementptr i32, i32* %23, i64 3
  %LastVal = load i32, i32* %LastUpdatedLanePtr
  store i32 %LastVal, i32* %i.lpriv
  br i1 %cmp.n, label %DIR.OMP.END.SIMD.4, label %scalar.ph

scalar.ph:                                        ; preds = %middle.block, %min.iters.checked, %DIR.OMP.SIMD.2
  %bc.resume.val = phi i64 [ 256, %middle.block ], [ 0, %DIR.OMP.SIMD.2 ], [ 0, %min.iters.checked ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %scalar.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ %bc.resume.val, %scalar.ph ]
  %24 = trunc i64 %indvars.iv to i32
  store i32 %24, i32* %i.lpriv, align 4
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %arrayidx = getelementptr inbounds float, float* %f, i64 %indvars.iv
  %25 = load float, float* %arrayidx, align 4
  %div = fmul fast float %25, 0x3FD5555560000000
  store float %div, float* %val.priv, align 4
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  %26 = call float @_Z6sincosfPf(float %div, float* %c.priv)
  store float %26, float* %s.priv
  %27 = load float, float* %s.priv, align 4
  %mul1 = fmul fast float %27, 2.000000e+00
  %28 = load float, float* %c.priv, align 4
  %add2 = fadd fast float %mul1, %28
  %29 = load i32, i32* %i.lpriv, align 4
  %idxprom3 = sext i32 %29 to i64
  %arrayidx4 = getelementptr inbounds float, float* %r, i64 %idxprom3
  store float %add2, float* %arrayidx4, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %middle.block, %omp.inner.for.body
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  ret void
}

; TEST 4
;
; Same as test 3, but with a 16 x float type which is illegal for this target,
; which only supports 8 x float. The MapIntrinToIml pass must break
; svml_sincos<16 x float> into two svml_sincos<8 x float>. Then, the struct
; results from the 8x sincos must be re-combined into 16x results.
; The code looks like this:
;
;  %shuffle = shufflevector <16 x float> %5, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
;  %vcall = call svml_cc %svml.ret.agg @__svml_sincosf8_ha(<8 x float> %shuffle)
;  %7 = extractvalue %svml.ret.agg %vcall, 1
;  %8 = extractvalue %svml.ret.agg %vcall, 0
;  %shuffle1 = shufflevector <16 x float> %5, <16 x float> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
;  %vcall2 = call svml_cc %svml.ret.agg @__svml_sincosf8_ha(<8 x float> %shuffle1)
;  %9 = extractvalue %svml.ret.agg %vcall2, 1
;  %shuffle.comb3 = shufflevector <8 x float> %7, <8 x float> %9, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
;  %10 = extractvalue %svml.ret.agg %vcall2, 0
;  %shuffle.comb = shufflevector <8 x float> %8, <8 x float> %10, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
;
; %shuffle.comb is the concatenation of the first fields of the two sincos calls.
; %shuffle.comb3 is the concatenation of the second fields of the two sincos calls.
; { %shuffle.comb, %shuffle.comb3 } is the same result as the 16xfloat SVML sincos call.

; CHECK-NOT: svml_sincosf16
; CHECK-NOT: ret2ptr
; CHECK: [[ANGLELO:%[a-z0-9.]+]] = shufflevector{{.*}}<i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[SINCOSLO:%[a-z0-9.]+]] = call{{.*}}svml_sincosf8_ha{{.*}}[[ANGLELO]]
; CHECK-DAG: [[SINCOS1:%[a-z0-9.]+]] = extractvalue{{.*}}[[SINCOSLO]], 1
; CHECK-DAG: [[SINCOS0:%[a-z0-9.]+]] = extractvalue{{.*}}[[SINCOSLO]], 0
; CHECK: [[ANGLEHI:%[a-z0-9.]+]] = shufflevector{{.*}}<i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[SINCOSHI:%[a-z0-9.]+]] = call{{.*}}svml_sincosf8_ha{{.*}}[[ANGLEHI]]
; CHECK: [[SINCOS3:%[a-z0-9.]+]] = extractvalue{{.*}}[[SINCOSHI]], 1
; CHECK-DAG: [[COMBHI:%[a-z0-9.]+]] = shufflevector{{.*}}[[SINCOS1]]{{.*}}[[SINCOS3]]{{.*}}<16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-DAG: [[SINCOS2:%[a-z0-9.]+]] = extractvalue{{.*}}[[SINCOSHI]], 0
; CHECK: [[COMBLO:%[a-z0-9.]+]] = shufflevector{{.*}}[[SINCOS0]]{{.*}}[[SINCOS2]]{{.*}}<16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: fmul{{.*}}[[COMBLO]]
; CHECK: fadd{{.*}}[[COMBHI]]

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo4(float* noalias nocapture readonly %f, float* noalias nocapture %r) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %cos.ptr.vec = alloca <16 x float>
  %sinPtr.vec = alloca <16 x float>
  %val.priv.vec = alloca <16 x float>, align 4
  %cos.ptr = alloca float
  %val.priv = alloca float, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %0 = bitcast float* %val.priv to i8*
  br i1 false, label %scalar.ph, label %min.iters.checked

min.iters.checked:                                ; preds = %DIR.OMP.SIMD.2
  br i1 false, label %scalar.ph, label %vector.ph

vector.ph:                                        ; preds = %min.iters.checked
  %val.privInitVal = load float, float* %val.priv
  %val.privInitVal.splatinsert = insertelement <16 x float> undef, float %val.privInitVal, i32 0
  %val.privInitVal.splat = shufflevector <16 x float> %val.privInitVal.splatinsert, <16 x float> undef, <16 x i32> zeroinitializer
  store <16 x float> %val.privInitVal.splat, <16 x float>* %val.priv.vec
  %1 = bitcast <16 x float>* %val.priv.vec to <16 x i8>*
  %cos.ptrInitVal = load float, float* %cos.ptr
  %cos.ptrInitVal.splatinsert = insertelement <16 x float> undef, float %cos.ptrInitVal, i32 0
  %cos.ptrInitVal.splat = shufflevector <16 x float> %cos.ptrInitVal.splatinsert, <16 x float> undef, <16 x i32> zeroinitializer
  store <16 x float> %cos.ptrInitVal.splat, <16 x float>* %cos.ptr.vec
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
  %vec.ind = phi <16 x i64> [ <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>, %vector.ph ], [ %vec.ind.next, %vector.body ]
  %privaddr = bitcast <16 x i8>* %1 to i8*
  %2 = getelementptr i8, i8* %privaddr, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %3 = extractelement <16 x i8*> %2, i32 0
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #2
  %scalar.gep. = getelementptr inbounds float, float* %f, i64 %index
  %4 = bitcast float* %scalar.gep. to <16 x float>*
  %wide.load = load <16 x float>, <16 x float>* %4, align 4
  %5 = fmul fast <16 x float> %wide.load, <float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000, float 0x3FD5555560000000>
  %privaddr17 = bitcast <16 x float>* %cos.ptr.vec to float*
  %6 = getelementptr float, float* %privaddr17, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %7 = call fast <16 x float> @_Z14sincos_ret2ptrDv16_fPS_S1_(<16 x float> %5, <16 x float>* %cos.ptr.vec, <16 x float>* %sinPtr.vec)
  %wide.sin.InitVal = load <16 x float>, <16 x float>* %sinPtr.vec
  %wide.load18 = load <16 x float>, <16 x float>* %cos.ptr.vec, align 4
  %8 = fmul fast <16 x float> %wide.sin.InitVal, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  %9 = fadd fast <16 x float> %8, %wide.load18
  %scalar.gep.19 = getelementptr inbounds float, float* %r, i64 %index
  %10 = bitcast float* %scalar.gep.19 to <16 x float>*
  store <16 x float> %9, <16 x float>* %10, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3) #2
  %11 = add nuw nsw i64 %index, 1
  %broadcast.splatinsert = insertelement <16 x i64> undef, i64 %11, i32 0
  %broadcast.splat = shufflevector <16 x i64> %broadcast.splatinsert, <16 x i64> undef, <16 x i32> zeroinitializer
  %12 = icmp eq <16 x i64> %broadcast.splat, <i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256, i64 256>
  %13 = extractelement <16 x i1> %12, i32 0
  %index.next = add i64 %index, 16
  %14 = icmp eq i64 %index.next, 256
  %vec.ind.next = add <16 x i64> %vec.ind, <i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16>
  br i1 %14, label %VPlannedBB, label %vector.body

VPlannedBB:                                       ; preds = %vector.body
  br label %middle.block

middle.block:                                     ; preds = %VPlannedBB
  %cmp.n = icmp eq i64 256, 256
  br i1 %cmp.n, label %DIR.OMP.END.SIMD.4, label %scalar.ph

scalar.ph:                                        ; preds = %middle.block, %min.iters.checked, %DIR.OMP.SIMD.2
  %bc.resume.val = phi i64 [ 256, %middle.block ], [ 0, %DIR.OMP.SIMD.2 ], [ 0, %min.iters.checked ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %scalar.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ %bc.resume.val, %scalar.ph ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %arrayidx = getelementptr inbounds float, float* %f, i64 %indvars.iv
  %15 = load float, float* %arrayidx, align 4
  %div = fmul fast float %15, 0x3FD5555560000000
  %16 = call fast float @_Z6sincosfPf(float %div, float* %cos.ptr)
  %cos.val = load float, float* %cos.ptr
  %mul1 = fmul fast float %16, 2.000000e+00
  %add2 = fadd fast float %mul1, %cos.val
  %arrayidx4 = getelementptr inbounds float, float* %r, i64 %indvars.iv
  store float %add2, float* %arrayidx4, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %middle.block, %omp.inner.for.body
  store i32 255, i32* %i.lpriv, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  br label %DIR.OMP.END.SIMD.416

DIR.OMP.END.SIMD.416:                             ; preds = %DIR.OMP.END.SIMD.3
  ret void
}



; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone speculatable
declare float @llvm.sin.f32(float) #3

; Function Attrs: nounwind readnone speculatable
declare float @llvm.cos.f32(float) #3

; Function Attrs: nofree nounwind
declare dso_local void @sincosf(float, float*, float*) local_unnamed_addr #0

; Function Attrs: nounwind
declare float @_Z6sincosfPf(float, float*) #2

; Function Attrs: nounwind
declare <4 x float> @_Z14sincos_ret2ptrDv4_fPS_S1_(<4 x float>, <4 x float>*, <4 x float>*) #2

; Function Attrs: nounwind
; Function Attrs: nounwind
declare <16 x float> @_Z14sincos_ret2ptrDv16_fPS_S1_(<16 x float>, <16 x float>*, <16 x float>*) #2

; Function Attrs: nounwind willreturn
declare void @llvm.masked.scatter.v4f32.v4p0f32(<4 x float>, <4 x float*>, i32 immarg, <4 x i1>) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone speculatable }
