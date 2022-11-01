; This test verifies that library calls (e.g. to sincos) are consistently
; vectorized in the vector body and in any peel/remainder loops. This means, if
; we decide to not library vectorize in the main loop (e.g. because we
; vectorized with a variant), then we should not library vectorize in
; peel/remainder loops, even if it is possible (i.e. SVML is enabled).

; With VF = 2, a variant will be matched, so we won't use SVML
; RUN: opt -S < %s -hir-ssa-deconstruction -hir-vplan-vec \
; RUN:   -vplan-enable-peeling -vplan-vec-scenario='s1;v2;v2s1' --vector-library=SVML \
; RUN:   -disable-output -print-after=hir-vplan-vec -vplan-enable-peel-rem-strip=0 2>&1 | FileCheck %s --check-prefix=NOLIB

; NOLIB-LABEL: + DO i1 = {{.*}} <vector-peel>
; NOLIB-NEXT:  |   @sincos({{.*}}, &((@sin.arr)[0][i1 + %n]), &((@cos.arr)[0][i1 + %n]))
; NOLIB-NEXT:  + END LOOP

; NOLIB-LABEL: + DO i1 = {{.*}} <simd-vectorized>
; NOLIB-NEXT:  |   @_ZGVbN2vvv_sincos({{.*}}, &((<2 x double*>)(@sin.arr)[0][i1 + %n + <i64 0, i64 1>]), &((<2 x double*>)(@cos.arr)[0][i1 + %n + <i64 0, i64 1>]));
; NOLIB-NEXT:  + END LOOP

; NOLIB-LABEL: + DO i1 = {{.*}} <vector-remainder>
; NOLIB-NEXT:  |   @_ZGVbN2vvv_sincos({{.*}}, &((<2 x double*>)(@sin.arr)[0][i1 + %n + <i64 0, i64 1>]), &((<2 x double*>)(@cos.arr)[0][i1 + %n + <i64 0, i64 1>]));
; NOLIB-NEXT:  + END LOOP

; NOLIB-LABEL: + DO i1 = {{.*}} <vector-remainder>
; NOLIB-NEXT:  |   @sincos({{.*}}, &((@sin.arr)[0][i1 + %n]), &((@cos.arr)[0][i1 + %n]))
; NOLIB-NEXT:  + END LOOP

; With VF = 4, no variant will match (due to ISA), so we will vectorize with SVML
; RUN: opt -S < %s -hir-ssa-deconstruction -hir-vplan-vec \
; RUN:   -vplan-enable-peeling -vplan-vec-scenario='s1;v4;v4s1' --vector-library=SVML \
; RUN:   -disable-output -print-after=hir-vplan-vec -vplan-enable-peel-rem-strip=0 2>&1 | FileCheck %s --check-prefix=LIB

; LIB-LABEL: + DO i1 = {{.*}} <vector-peel>
; LIB:       |   [[RES:%.*]] = @__svml_sincos1({{.*}});
; LIB-NEXT:  |   [[SIN_V:%sincos.sin[0-9]*]] = extractvalue [[RES]], 0;
; LIB-NEXT:  |   [[COS_V:%sincos.cos[0-9]*]] = extractvalue [[RES]], 1;
; LIB-NEXT:  |   [[SIN:%.*]] = extractelement [[SIN_V]], 0;
; LIB-NEXT:  |   (@sin.arr)[0][i1 + %n] = [[SIN]];
; LIB-NEXT:  |   [[COS:%.*]] = extractelement [[COS_V]], 0;
; LIB-NEXT:  |   (@cos.arr)[0][i1 + %n] = [[COS]];
; LIB-NEXT:  + END LOOP

; LIB-LABEL: + DO i1 = {{.*}} <simd-vectorized>
; LIB:       |   [[RES:%.*]] = @__svml_sincos4({{.*}});
; LIB-NEXT:  |   [[SIN:%vp.sincos.sin[0-9]*]] = extractvalue [[RES]], 0;
; LIB-NEXT:  |   [[COS:%vp.sincos.cos[0-9]*]] = extractvalue [[RES]], 1;
; LIB-NEXT:  |   (<4 x double>*)(@sin.arr)[0][i1 + %n] = [[SIN]];
; LIB-NEXT:  |   (<4 x double>*)(@cos.arr)[0][i1 + %n] = [[COS]];
; LIB-NEXT:  + END LOOP

; LIB-LABEL: + DO i1 = {{.*}} <vector-remainder>
; LIB:       |   [[RES:%.*]] = @__svml_sincos4({{.*}});
; LIB-NEXT:  |   [[SIN:%vp.sincos.sin[0-9]*]] = extractvalue [[RES]], 0;
; LIB-NEXT:  |   [[COS:%vp.sincos.cos[0-9]*]] = extractvalue [[RES]], 1;
; LIB-NEXT:  |   (<4 x double>*)(@sin.arr)[0][i1 + %n] = [[SIN]];
; LIB-NEXT:  |   (<4 x double>*)(@cos.arr)[0][i1 + %n] = [[COS]];
; LIB-NEXT:  + END LOOP

; LIB-LABEL: + DO i1 = {{.*}} <vector-remainder>
; LIB:       |   [[RES:%.*]] = @__svml_sincos1({{.*}});
; LIB-NEXT:  |   [[SIN_V:%sincos.sin[0-9]*]] = extractvalue [[RES]], 0;
; LIB-NEXT:  |   [[COS_V:%sincos.cos[0-9]*]] = extractvalue [[RES]], 1;
; LIB-NEXT:  |   [[SIN:%.*]] = extractelement [[SIN_V]], 0;
; LIB-NEXT:  |   (@sin.arr)[0][i1 + %n] = [[SIN]];
; LIB-NEXT:  |   [[COS:%.*]] = extractelement [[COS_V]], 0;
; LIB-NEXT:  |   (@cos.arr)[0][i1 + %n] = [[COS]];
; LIB-NEXT:  + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sin.arr = external global [256 x double], align 16
@cos.arr = external global [256 x double], align 16

define dso_local void @test_sincos_peel_remainder(i64 %n) {
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.preheader

for.preheader:
  br label %for.body

for.body:
  %iv = phi i64 [ %n, %for.preheader ], [ %iv.next, %for.body ]

  %sin = getelementptr inbounds [256 x double], [256 x double]* @sin.arr, i64 0, i64 %iv
  %cos = getelementptr inbounds [256 x double], [256 x double]* @cos.arr, i64 0, i64 %iv

  call void @sincos(double 1.0, double* %sin, double* %cos)

  %iv.next = add nuw nsw i64 %iv, 1
  %cmp = icmp ult i64 %iv.next, 256
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)

; Function Attrs: nofree nounwind
declare dso_local void @sincos(double, double*, double*) local_unnamed_addr #1

attributes #1 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN2vvv_sincos,_ZGVcN4vvv_sincos,_ZGVdN4vvv_sincos,_ZGVeN8vvv_sincos" }
