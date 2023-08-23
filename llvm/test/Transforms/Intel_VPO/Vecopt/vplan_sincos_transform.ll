; RUN: opt -passes=vplan-vec -vplan-force-vf=2 -vector-library=SVML \
; RUN: -vplan-print-after-transformed-library-calls -S < %s 2>&1 | FileCheck %s

; CHECK-LABEL: VPlan after transforming library calls
; CHECK:       [[BODY:BB[0-9]+]]:
; CHECK:         ptr [[SIN_ADDR:%.*]] = getelementptr inbounds [256 x double], ptr [[SIN_ARR:%.*]] i64 0 i64 {{.*}}
; CHECK:         ptr [[COS_ADDR:%.*]] = getelementptr inbounds [256 x double], ptr [[COS_ARR:%.*]] i64 0 i64 {{.*}}
; CHECK:         %.vplan.sincos = type { double, double } [[VP_RES:%.*]] = transform-lib-call double [[VAL:.*]] __svml_sincos2
; CHECK-NEXT:    double [[VP_SIN:%.*]] = soa-extract-value %.vplan.sincos = type { double, double } [[VP_RES]] i64 0
; CHECK-NEXT:    double [[VP_COS:%.*]] = soa-extract-value %.vplan.sincos = type { double, double } [[VP_RES]] i64 1
; CHECK-NEXT:    store double [[VP_SIN]] ptr [[SIN_ADDR]]
; CHECK-NEXT:    store double [[VP_COS]] ptr [[COS_ADDR]]

; CHECK-LABEL: define dso_local void @test_sincos_transform() {
; CHECK-NEXT:    [[SIN_ARR]] = alloca [256 x double], align 16
; CHECK-NEXT:    [[COS_ARR]] = alloca [256 x double], align 16
; CHECK:       vector.body:
; CHECK:         [[SIN_GEP:%.*]] = getelementptr inbounds [256 x double], ptr [[SIN_ARR]], i64 0, i64 {{.*}}
; CHECK:         [[COS_GEP:%.*]] = getelementptr inbounds [256 x double], ptr [[COS_ARR]], i64 0, i64 {{.*}}
; CHECK:         [[RES:%.*]] = call svml_cc { <2 x double>, <2 x double> } @__svml_sincos2(<2 x double> <double [[VAL]], double [[VAL]]>)
; CHECK-NEXT:    [[SIN:%.*]] = extractvalue { <2 x double>, <2 x double> } [[RES]], 0
; CHECK-NEXT:    [[COS:%.*]] = extractvalue { <2 x double>, <2 x double> } [[RES]], 1
; CHECK-NEXT:    store <2 x double> [[SIN]], ptr [[SIN_GEP]], align 8
; CHECK-NEXT:    store <2 x double> [[COS]], ptr [[COS_GEP]], align 8

; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=2 -vector-library=SVML -vplan-print-after-transformed-library-calls -disable-output -S < %s 2>&1 | FileCheck %s --check-prefix=HIR

; HIR-LABEL: VPlan after transforming library calls
; HIR:       [[BODY:BB[0-9]+]]:
; HIR:         ptr [[SIN_ADDR:%.*]] = subscript inbounds ptr [[SIN_ARR:%.*]] i64 0 i64 {{.*}}
; HIR:         ptr [[COS_ADDR:%.*]] = subscript inbounds ptr [[COS_ARR:%.*]] i64 0 i64 {{.*}}
; HIR:         %.vplan.sincos = type { double, double } [[VP_RES:%.*]] = transform-lib-call double [[VAL:.*]] __svml_sincos2
; HIR-NEXT:    double [[VP_SIN:%.*]] = soa-extract-value %.vplan.sincos = type { double, double } [[VP_RES]] i64 0
; HIR-NEXT:    double [[VP_COS:%.*]] = soa-extract-value %.vplan.sincos = type { double, double } [[VP_RES]] i64 1
; HIR-NEXT:    store double [[VP_SIN]] ptr [[SIN_ADDR]]
; HIR-NEXT:    store double [[VP_COS]] ptr [[COS_ADDR]]

; HIR-LABEL: BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 255, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:        |   [[RES:%.*]] = @__svml_sincos2({{.*}});
; HIR-NEXT:        |   [[SIN:%.*]] = extractvalue [[RES]], 0;
; HIR-NEXT:        |   [[COS:%.*]] = extractvalue [[RES]], 1;
; HIR-NEXT:        |   (<2 x double>*)(%sin.arr)[0][i1] = [[SIN]];
; HIR-NEXT:        |   (<2 x double>*)(%cos.arr)[0][i1] = [[COS]];
; HIR-NEXT:        + END LOOP
; HIR:       END REGION

define dso_local void @test_sincos_transform() {
  %sin.arr = alloca [256 x double], align 16
  %cos.arr = alloca [256 x double], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.preheader

for.preheader:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.preheader ], [ %iv.next, %for.body ]

  %sin = getelementptr inbounds [256 x double], ptr %sin.arr, i64 0, i64 %iv
  %cos = getelementptr inbounds [256 x double], ptr %cos.arr, i64 0, i64 %iv

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

; Function Attrs: nounwind
declare dso_local void @sincos(double noundef, double* noundef, double* noundef) local_unnamed_addr #1
attributes #1 = { nounwind }
