; This test checks that masked svml call is serialized if the masked version of
; intirinsic is not available.

; RUN: opt -VPlanDriver -vplan-force-vf=2 -vector-library=SVML -S %s | FileCheck %s

target datalayout = "p:32:32-p1:32:32-p2:16:16"

@arr = dso_local global [1024 x float] zeroinitializer, align 16

@global = global i8 0

; Function Attrs: nounwind
declare double @_Z4sqrtd(double) local_unnamed_addr

; Function Attrs: nounwind
declare double @_Z3expd(double) local_unnamed_addr

; Function Attrs: nounwind
declare float @_Z3logf(float) local_unnamed_addr

; CHECK-LABEL: @test1(
; CHECK: vector.body:
; CHECK: {{.*}} = call <2 x double> @_Z4sqrtDv2_d({{.*}})
; CHECK: [[WL:%.*]] = call <2 x float> @llvm.masked.load.v2f32.p0v2f32
; CHECK-NEXT: [[E2:%.*]] = extractelement <2 x float> [[WL]], i32 1
; CHECK-NEXT: [[E1:%.*]] = extractelement <2 x float> [[WL]], i32 0

; CHECK: pred.call.if:
; CHECK-NEXT: {{.*}} = call float @_Z3logf(float [[E1]])
; CHECK-NEXT: br label %[[LBL3:.*]]

; CHECK: pred.call.if{{[1-9]+}}:
; CHECK-NEXT: {{.*}} = call float @_Z3logf(float [[E2]])
; CHECK-NEXT: br label %[[LBL4:.*]]

define void @test1(double %t, float %tf, i32 %idx) {
  br label %simd.begin.region
  simd.begin.region:                                ; preds = %19
    %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
    br label %simd.loop

  simd.loop:
    %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
    %1 = call double @_Z3expd(double %t)
    %2 = call double @_Z4sqrtd(double %t)
    %cmp1 = icmp sgt i32 %index, 2
    br i1 %cmp1, label %if.then, label %omp.body.continue
  if.then:
    ;; TODO: For uniform input arguments like 'tf', we should have a single
    ;; call to the '@_Z3logf()' function followed by a 'broadcast'. The predicated,
    ;; serialized call to @_Z3logf(), should only be generated when we have
    ;; non-uniform argument to the function-call. In the future when we have the correct
    ;; implementation, we should change the test accordingly.
    %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @arr, i32 0, i32 %index
    %load = load float, float* %arrayidx, align 4
    %pred = call float @_Z3logf(float %load)
    br label %omp.body.continue
  omp.body.continue:
     br label %simd.loop.exit
  simd.loop.exit:
    %indvar = add nuw i32 %index, 1
    %vl.cond = icmp ult i32 %indvar, 8
    br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !19

  simd.end.region:                                  ; preds = %simd.loop.exit
    call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
    br label %return

  return:                                           ; preds = %simd.end.region
    ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

!19 = distinct !{!19, !20}
!20 = !{!"llvm.loop.unroll.disable"}
