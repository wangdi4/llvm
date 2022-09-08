; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=2 -hir-general-unroll -print-before=hir-general-unroll -print-after=hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-general-unroll,print<hir>" -vplan-force-vf=2 -disable-output 2>&1 | FileCheck %s

; Verify that we are able to generate a switch remainder by successfully
; normalizing the loop containing vector CEs with IV.
; The test case was choking while normalizing this vector ref-
; (<2 x float>*)(%t143)[<i64 0, i64 1> + 7][%t920][i1 + %t913]

; Dump Before -

; CHECK: + DO i1 = 0, -1 * %t913 + %t918 + -1, 1   <DO_LOOP> <unroll = 4>
; CHECK: |   %t928 = 1.000000e+00  /  (%t922)[i1 + %t913];
; CHECK: |   %t931 = 0.000000e+00;
; CHECK: |   %t932 = 0.000000e+00;
; CHECK: |   %red.init = 0.000000e+00;
; CHECK: |   %red.init2 = 0.000000e+00;
; CHECK: |   %phi.temp = %red.init2;
; CHECK: |   %phi.temp3 = %red.init;
; CHECK: |   %.vec = (<2 x float>*)(@"module_ra_aerosol_mp_calc_aerosol_goddard_sw_$UPPER_WVL")[0][7];
; CHECK: |   %.vec5 = (<2 x float>*)(@"module_ra_aerosol_mp_calc_aerosol_goddard_sw_$LOWER_WVL")[0][7];
; CHECK: |   %.vec6 = %.vec  +  %.vec5;
; CHECK: |   %.vec7 = %.vec6  *  0x3FED1745C0000000;
; CHECK: |   %llvm.log.v2f32 = @llvm.log.v2f32(%.vec7);
; CHECK: |   %.vec8 = (<2 x float>*)(%t143)[<i64 0, i64 1> + 7][%t920][i1 + %t913];
; CHECK: |   %.vec9 = %.vec8  *  %t928;
; CHECK: |   %llvm.log.v2f3210 = @llvm.log.v2f32(%.vec9);
; CHECK: |   %.vec11 = %llvm.log.v2f3210  *  %llvm.log.v2f32;
; CHECK: |   %.vec12 = %.vec11  +  %phi.temp3;
; CHECK: |   %.vec13 = %llvm.log.v2f32  *  %llvm.log.v2f32;
; CHECK: |   %.vec14 = %.vec13  +  %phi.temp;
; CHECK: |   %phi.temp = %.vec14;
; CHECK: |   %phi.temp3 = %.vec12;
; CHECK: |   %t931 = @llvm.vector.reduce.fadd.v2f32(%t931,  %.vec12);
; CHECK: |   %t932 = @llvm.vector.reduce.fadd.v2f32(%t932,  %.vec14);
; CHECK: |   %t955 =  - %t931;
; CHECK: |   %t956 = %t955  /  %t932;
; CHECK: |   (%t923)[i1 + %t913] = %t956;
; CHECK: + END LOOP

; Dump After -

; CHECK: DO i1 = 0, %tgu + -1, 1
; CHECK: switch

target triple = "x86_64-unknown-linux-gnu"

@"module_ra_aerosol_mp_calc_aerosol_goddard_sw_$UPPER_WVL" = external hidden unnamed_addr constant [11 x float], align 32
@"module_ra_aerosol_mp_calc_aerosol_goddard_sw_$LOWER_WVL" = external hidden unnamed_addr constant [11 x float], align 32

define void @foo(float* noalias %t922, float* noalias %t143, float* noalias %t923, i64 %t144, i64 %t145, i64 %t911, i64 %t912, i64 %t913, i64 %t918, i64 %t920) {
entry:
  br label %outer.loop

outer.loop:                                              ; preds = %latch, %t921
  %t925 = phi i64 [ %t913, %entry ], [ %t958, %latch ]
  %t926 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %t911, i64 4, float* elementtype(float) nonnull %t922, i64 %t925)
  %t927 = load float, float* %t926, align 1
  %t928 = fdiv fast float 1.000000e+00, %t927
  br label %loop

loop:                                              ; preds = %loop, %outer.loop
  %t930 = phi i64 [ %t950, %loop ], [ 8, %outer.loop ]
  %t931 = phi float [ %t947, %loop ], [ 0.000000e+00, %outer.loop ]
  %t932 = phi float [ %t949, %loop ], [ 0.000000e+00, %outer.loop ]
  %t933 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) getelementptr inbounds ([11 x float], [11 x float]* @"module_ra_aerosol_mp_calc_aerosol_goddard_sw_$LOWER_WVL", i64 0, i64 0), i64 %t930)
  %t934 = load float, float* %t933, align 1
  %t935 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) getelementptr inbounds ([11 x float], [11 x float]* @"module_ra_aerosol_mp_calc_aerosol_goddard_sw_$UPPER_WVL", i64 0, i64 0), i64 %t930)
  %t936 = load float, float* %t935, align 1
  %t937 = fadd fast float %t936, %t934
  %t938 = fmul fast float %t937, 0x3FED1745C0000000
  %t939 = call fast float @llvm.log.f32(float %t938)
  %t940 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %t145, float* elementtype(float) nonnull %t143, i64 %t930)
  %t941 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %t912, i64 %t144, float* elementtype(float) nonnull %t940, i64 %t920)
  %t942 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %t911, i64 4, float* elementtype(float) nonnull %t941, i64 %t925)
  %t943 = load float, float* %t942, align 1
  %t944 = fmul fast float %t943, %t928
  %t945 = call fast float @llvm.log.f32(float %t944)
  %t946 = fmul fast float %t945, %t939
  %t947 = fadd fast float %t946, %t931
  %t948 = fmul fast float %t939, %t939
  %t949 = fadd fast float %t948, %t932
  %t950 = add nuw nsw i64 %t930, 1
  %t951 = icmp eq i64 %t950, 10
  br i1 %t951, label %latch, label %loop

latch:                                              ; preds = %loop
  %t953 = phi float [ %t947, %loop ]
  %t954 = phi float [ %t949, %loop ]
  %t955 = fneg fast float %t953
  %t956 = fdiv fast float %t955, %t954
  %t957 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %t911, i64 4, float* elementtype(float) %t923, i64 %t925)
  store float %t956, float* %t957, align 1
  %t958 = add nsw i64 %t925, 1
  %t959 = icmp eq i64 %t958, %t918
  br i1 %t959, label %exit, label %outer.loop, !llvm.loop !0

exit:
  ret void
}

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #2

declare float @llvm.log.f32(float) #3

attributes #2 = { nounwind readnone speculatable }

attributes #3 = { nofree nosync nounwind readnone speculatable willreturn }

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 4}
