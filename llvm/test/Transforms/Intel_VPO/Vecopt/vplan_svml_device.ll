; This test checks that SPIR compilation uses device versions
; of SVML functions properly.

; RUN: opt -vplan-vec -vplan-target-vf=16 -vector-library=SVML -S < %s | FileCheck %s

; CHECK: call spir_func <16 x float> @_ZGVxM16v___svml_device_sinf(<16 x float>
; CHECK: call spir_func <16 x float> @_ZGVxM16v___svml_device_cosf(<16 x float>
; CHECK: call spir_func <16 x double> @_ZGVxN16vv___svml_device_pow(<16 x double>
; CHECK: call spir_func <16 x double> @_ZGVxN16v___svml_device_log2(<16 x double>

target datalayout = "p:32:32-p1:32:32-p2:16:16"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: convergent mustprogress nofree nounwind willreturn writeonly
declare spir_func float @sinf(float noundef) local_unnamed_addr

; Function Attrs: convergent mustprogress nofree nounwind willreturn writeonly
declare spir_func float @cosf(float noundef) local_unnamed_addr

; Function Attrs: convergent mustprogress nofree nounwind willreturn writeonly
declare spir_func double @llvm.pow.f64(double noundef, double noundef) local_unnamed_addr

; Function Attrs: convergent mustprogress nofree nounwind willreturn writeonly
declare spir_func double @log2(double noundef) local_unnamed_addr

define spir_kernel void @foo() {
start:
  br label %DIR.OMP.SIMD

DIR.OMP.SIMD:                                   ; preds = %start
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD, %if.end
  %indvars.iv27 = phi i32 [ 0, %DIR.OMP.SIMD ], [ %indvars.iv.next28, %if.end ]
  %conv = uitofp i32 %indvars.iv27  to float
  %1 = fmul float %conv, 0x3FF921FB60000000
  %rem20 = urem i32 %indvars.iv27, 3
  %test.not = icmp eq i32 %rem20, 0
  br i1 %test.not, label %if.else, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %call = call float @sinf(float noundef %1)
  br label %if.end

if.else:                                          ; preds = %omp.inner.for.body
  %call8 = call float @cosf(float noundef %1)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %indvars.iv.next28 = add nuw nsw i32 %indvars.iv27, 1
  %conv14 = uitofp i32 %indvars.iv.next28 to double
  %call15 = call double @llvm.pow.f64(double noundef %conv14, double noundef 6.000000e+00)
  %call22 = call double @log2(double noundef %call15)
  %exitcond29.not = icmp eq i32 %indvars.iv.next28, 16
  br i1 %exitcond29.not, label %DIR.OMP.END.SIMD, label %omp.inner.for.body

DIR.OMP.END.SIMD:                             ; preds = %if.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}
