; RUN: opt -enable-new-pm=0 -hir-framework -hir-vplan-vec -vplan-vec -vplan-force-vf=2 -vplan-print-after-plain-cfg -print-after=hir-vplan-vec -S -vplan-enable-new-cfg-merge-hir=false < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-vplan-vec,print<hir>,vplan-vec" -vplan-force-vf=2 -vplan-print-after-plain-cfg -S -vplan-enable-new-cfg-merge-hir=false < %s 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=0 -hir-framework -hir-vplan-vec -vplan-vec -vplan-force-vf=2 -vplan-print-after-plain-cfg -print-after=hir-vplan-vec -S -vplan-enable-new-cfg-merge-hir < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-vplan-vec,print<hir>,vplan-vec" -vplan-force-vf=2 -vplan-print-after-plain-cfg -S -vplan-enable-new-cfg-merge-hir < %s 2>&1 | FileCheck %s

;
; Test checks that we do not crash during HIR decomposition. The test also
; checks the generated vector HIR and that VPlan LLVM IR path successfully
; vectorizes the loop.
;

; Checks for HIR vectorizer
; CHECK-LABEL:    VPlan after importing plain CFG
; CHECK:          <4 x float> %vp{{.*}} = insertelement <4 x float> <float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00> float %vp{{.*}} i32 0
; CHECK:          + DO i1 = 0, {{.*}}, 2   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <simd-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:     |   [[VEC:%.*]] = (<2 x float>*)(%f)[i1];
; CHECK-NEXT:     |   [[EXTENDED:%.*]] = shufflevector [[VEC]],  undef,  <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>;
; CHECK-NEXT:     |   [[WIDE_INSERT:%.*]] = shufflevector <float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>,  [[EXTENDED]],  <i32 8, i32 1, i32 2, i32 3, i32 9, i32 5, i32 6, i32 7>
; CHECK-NEXT:     |   [[SERIAL_TEMP:%.*]] = undef;
; CHECK-NEXT:     |   [[EXTRACTSUBVEC:%.*]] = shufflevector [[WIDE_INSERT]],  undef,  <i32 0, i32 1, i32 2, i32 3>;
; CHECK-NEXT:     |   [[CALL:%.*]] = @llvm.x86.avx512bf16.mask.cvtneps2bf16.128([[EXTRACTSUBVEC]],  zeroinitializer,  <i1 true, i1 true, i1 true, i1 true>);
; CHECK-NEXT:     |   [[SERIAL_TEMP]] = @llvm.vector.insert.v16i16.v8i16([[SERIAL_TEMP]],  [[CALL]],  0);
; CHECK-NEXT:     |   [[EXTRACTSUBVEC1:%.*]] = shufflevector [[WIDE_INSERT]],  undef,  <i32 4, i32 5, i32 6, i32 7>;
; CHECK-NEXT:     |   [[CALL1:%.*]] = @llvm.x86.avx512bf16.mask.cvtneps2bf16.128([[EXTRACTSUBVEC1]],  zeroinitializer,  <i1 true, i1 true, i1 true, i1 true>);
; CHECK-NEXT:     |   [[SERIAL_TEMP]] = @llvm.vector.insert.v16i16.v8i16([[SERIAL_TEMP]],  [[CALL1]],  8);
; CHECK-NEXT:     |   [[WIDE_EXTRACT:%.*]] = shufflevector [[SERIAL_TEMP]],  undef,  <i32 0, i32 8>;
; CHECK-NEXT:     |   (<2 x i16>*)(%bf)[i1] = [[WIDE_EXTRACT]];
; CHECK-NEXT:     + END LOOP


; Checks for LLVM-IR vectorizer
; CHECK-LABEL:    VPlan after importing plain CFG
; CHECK:          <4 x float> %vp{{.*}} = insertelement <4 x float> <float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00> float %vp{{.*}} i32 0
; CHECK-LABEL:    vector.body:
; CHECK-COUNT-2:    {{%.*}} = call <8 x i16> @llvm.x86.avx512bf16.mask.cvtneps2bf16.128

define dso_local void @vec_bf(float* nocapture readonly %f, i16* nocapture %bf, i32 %len) local_unnamed_addr #0 {
entry:
  %i.linear.iv = alloca i32, align 4
  %cmp = icmp eq i32 %len, 0
  br i1 %cmp, label %omp.precond.end, label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.015 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add7, %omp.inner.for.body ]
  %idxprom = sext i32 %.omp.iv.local.015 to i64
  %ptridx = getelementptr inbounds float, float* %f, i64 %idxprom
  %1 = load float, float* %ptridx, align 4
  %vecinit3.i = insertelement <4 x float> <float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, float %1, i32 0
  %2 = call <8 x i16> @llvm.x86.avx512bf16.mask.cvtneps2bf16.128(<4 x float> %vecinit3.i, <8 x i16> zeroinitializer, <4 x i1> <i1 true, i1 true, i1 true, i1 true>) #1
  %vecext.i = extractelement <8 x i16> %2, i32 0
  %ptridx6 = getelementptr inbounds i16, i16* %bf, i64 %idxprom
  store i16 %vecext.i, i16* %ptridx6, align 2
  %add7 = add nuw i32 %.omp.iv.local.015, 1
  %exitcond = icmp eq i32 %add7, %len
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body
  store i32 %len, i32* %i.linear.iv, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind readnone
declare <8 x i16> @llvm.x86.avx512bf16.mask.cvtneps2bf16.128(<4 x float>, <8 x i16>, <4 x i1>) #2
