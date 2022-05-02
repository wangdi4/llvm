; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec" -aa-pipeline="basic-aa" -print-after=hir-loop-distribute-memrec -disable-output %s 2>&1 | FileCheck %s


; Verify that distribution happens for PiBlock that has vector-inhibiting code. There is a flow edge
; for %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7" in the select instruction <124>.

; PiBlock of interest:

; "2": <2>          %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7.out" = %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7";
; "121": <121>        %rel.507 = trunc.i32.i1(%"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7") != 0;
; "124": <124>        %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7" = (umin(%rel.507, %rel.508) != 0) ? 0 : %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7";
; <end>

; Verify we distribute the piblock at the end of the loop

; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, (sext.i32.i64(%KTE_fetch.4725) + -1 * sext.i32.i64(%KTS_fetch.4724))/u64, 1
;               |   %min = (-64 * i1 + sext.i32.i64(%KTE_fetch.4725) + -1 * sext.i32.i64(%KTS_fetch.4724) <= 63) ? -64 * i1 + sext.i32.i64(%KTE_fetch.4725) + -1 * sext.i32.i64(%KTS_fetch.4724) : 63;
; CHECK:        |   + DO i2 = 0, %min, 1
;               |   |   %"module_mp_thompson_mp_mp_thompson_$TEMP450[]_fetch.4730" = (%"module_mp_thompson_mp_mp_thompson_$TEMP450")[64 * i1 + i2 + sext.i32.i64(%KTS_fetch.4724)];
;...

; CHECK:        |   + END LOOP
; CHECK:        |   + DO i2 = 0, %min, 1
;               |   |   %rel.508 = (%.TempArray)[0][i2];
;               |   |   %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7.out" = %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7";
;               |   |   %rel.507 = trunc.i32.i1(%"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7") != 0;
; CHECK:        |   |   %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7" = (umin(%rel.507, %rel.508) != 0) ? 0 : %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7";
;               |   + END LOOP
;               + END LOOP
;         END REGION


; Function Attrs: nounwind uwtable
define void @module_mp_thompson_mp_mp_thompson_(i32 %KTS_fetch.4724, i32 %KTE_fetch.4725, float* %"module_mp_thompson_mp_mp_thompson_$TEMP450", float* %"module_mp_thompson_mp_mp_thompson_$RHO1034", float* %"module_mp_thompson_mp_mp_thompson_$RHOF394", float* %"module_mp_thompson_mp_mp_thompson_$RHOF2386", float* %"module_mp_thompson_mp_mp_thompson_$PRES442", float* %"module_mp_thompson_mp_mp_thompson_$QVS378", float* %"module_mp_thompson_mp_mp_thompson_$QV1066", float* %"module_mp_thompson_mp_mp_thompson_$DELQVS362", float* %"module_mp_thompson_mp_mp_thompson_$QVSI370", float* %"module_mp_thompson_mp_mp_thompson_$SATW354", float* %"module_mp_thompson_mp_mp_thompson_$SATI346", float* %"module_mp_thompson_mp_mp_thompson_$SSATW338", float* %"module_mp_thompson_mp_mp_thompson_$SSATI330", float* %"module_mp_thompson_mp_mp_thompson_$DIFFU322", float* %"module_mp_thompson_mp_mp_thompson_$VISCO314", float* %"module_mp_thompson_mp_mp_thompson_$OCP282", float* %"module_mp_thompson_mp_mp_thompson_$VSC2306", float* %"module_mp_thompson_mp_mp_thompson_$LVAP290", float* %"module_mp_thompson_mp_mp_thompson_$TCOND298") #0 {
bb432_endif:
  %0 = tail call i32 @llvm.smax.i32(i32 0, i32 0)
  %1 = add nsw i32 0, 0
  %2 = zext i32 0 to i64
  %3 = tail call i32 @llvm.smax.i32(i32 0, i32 0)
  %4 = add nsw i32 0, 0
  %5 = zext i32 0 to i64
  br i1 false, label %bb442, label %bb433.preheader

bb433.preheader:                                  ; preds = %bb432_endif
  %6 = add nsw i32 0, 0
  ret void

bb452_endif:                                      ; No predecessors!
  %7 = fneg reassoc ninf nsz arcp contract afn float 0.000000e+00
  %8 = tail call reassoc ninf nsz arcp contract afn float @llvm.pow.f32(float 0.000000e+00, float 0.000000e+00)
  ret void

bb458_endif:                                      ; No predecessors!
  %9 = fneg reassoc ninf nsz arcp contract afn float 0.000000e+00
  %10 = tail call reassoc ninf nsz arcp contract afn float @llvm.pow.f32(float 0.000000e+00, float 0.000000e+00)
  ret void

bb442:                                            ; preds = %bb432_endif
  br label %bb465.preheader

bb465.preheader:                                  ; preds = %bb442
  %11 = sext i32 %KTS_fetch.4724 to i64
  %12 = add nsw i32 %KTE_fetch.4725, 1
  %wide.trip.count15562 = sext i32 %12 to i64
  br label %bb465

bb465:                                            ; preds = %bb476_endif, %bb465.preheader
  %indvars.iv15560 = phi i64 [ %11, %bb465.preheader ], [ %indvars.iv.next15561, %bb476_endif ]
  %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7" = phi i32 [ 0, %bb465.preheader ], [ %spec.select, %bb476_endif ]
  %"module_mp_thompson_mp_mp_thompson_$TEMP450[]1781" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$TEMP450", i64 %indvars.iv15560)
  %"module_mp_thompson_mp_mp_thompson_$TEMP450[]_fetch.4730" = load float, float* %"module_mp_thompson_mp_mp_thompson_$TEMP450[]1781", align 4
  %sub.347 = fadd reassoc ninf nsz arcp contract afn float %"module_mp_thompson_mp_mp_thompson_$TEMP450[]_fetch.4730", 0xC071126660000000
  %"module_mp_thompson_mp_mp_thompson_$RHO1034[]1791" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$RHO1034", i64 %indvars.iv15560)
  %"module_mp_thompson_mp_mp_thompson_$RHO1034[]_fetch.4734" = load float, float* %"module_mp_thompson_mp_mp_thompson_$RHO1034[]1791", align 4
  %div.235 = fdiv reassoc ninf nsz arcp contract afn float 0x3FF2F3CC60000000, %"module_mp_thompson_mp_mp_thompson_$RHO1034[]_fetch.4734"
  %func_result1783 = tail call reassoc ninf nsz arcp contract afn float @llvm.sqrt.f32(float %div.235)
  %"module_mp_thompson_mp_mp_thompson_$RHOF394[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$RHOF394", i64 %indvars.iv15560)
  store float %func_result1783, float* %"module_mp_thompson_mp_mp_thompson_$RHOF394[]", align 4
  %func_result1793 = tail call reassoc ninf nsz arcp contract afn float @llvm.sqrt.f32(float %func_result1783)
  %"module_mp_thompson_mp_mp_thompson_$RHOF2386[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$RHOF2386", i64 %indvars.iv15560)
  store float %func_result1793, float* %"module_mp_thompson_mp_mp_thompson_$RHOF2386[]", align 4
  %"module_mp_thompson_mp_mp_thompson_$PRES442[]1811" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$PRES442", i64 %indvars.iv15560)
  %sub.955.i = fadd reassoc ninf nsz arcp contract afn float %"module_mp_thompson_mp_mp_thompson_$TEMP450[]_fetch.4730", 0xC071128F60000000
  %rel.1298.i = fcmp reassoc ninf nsz arcp contract afn ole float %sub.955.i, -8.000000e+01
  %slct.473.i = select reassoc ninf nsz arcp contract afn i1 %rel.1298.i, float -8.000000e+01, float %sub.955.i
  %mul.3461.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, 0x3D221A7DA0000000
  %13 = fsub reassoc ninf nsz arcp contract afn float 0x3D90B12D20000000, %mul.3461.i
  %mul.3462.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %13
  %add.2163.i = fadd reassoc ninf nsz arcp contract afn float %mul.3462.i, 0x3E3E2D6580000000
  %mul.3463.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2163.i
  %add.2164.i = fadd reassoc ninf nsz arcp contract afn float %mul.3463.i, 0x3EC10AB3E0000000
  %mul.3464.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2164.i
  %add.2165.i = fadd reassoc ninf nsz arcp contract afn float %mul.3464.i, 0x3F339D4560000000
  %mul.3465.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2165.i
  %add.2166.i = fadd reassoc ninf nsz arcp contract afn float %mul.3465.i, 0x3F9B0E7B60000000
  %mul.3466.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2166.i
  %add.2167.i = fadd reassoc ninf nsz arcp contract afn float %mul.3466.i, 0x3FF6E88940000000
  %mul.3467.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2167.i
  %add.2168.i = fadd reassoc ninf nsz arcp contract afn float %mul.3467.i, 0x40463AF7E0000000
  %mul.3468.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2168.i
  %add.2169.i = fadd reassoc ninf nsz arcp contract afn float %mul.3468.i, 0x40831CAB60000000
  %P_fetch.16969.i = load float, float* %"module_mp_thompson_mp_mp_thompson_$PRES442[]1811", align 1
  %mul.3469.i = fmul reassoc ninf nsz arcp contract afn float %P_fetch.16969.i, 0x3FC3333340000000
  %rel.1299.i = fcmp reassoc ninf nsz arcp contract afn ole float %add.2169.i, %mul.3469.i
  %slct.474.i = select reassoc ninf nsz arcp contract afn i1 %rel.1299.i, float %add.2169.i, float %mul.3469.i
  %mul.3470.i = fmul reassoc ninf nsz arcp contract afn float %slct.474.i, 0x3FE3E76C80000000
  %sub.956.i = fsub reassoc ninf nsz arcp contract afn float %P_fetch.16969.i, %slct.474.i
  %div.542.i = fdiv reassoc ninf nsz arcp contract afn float %mul.3470.i, %sub.956.i
  %"module_mp_thompson_mp_mp_thompson_$QVS378[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$QVS378", i64 %indvars.iv15560)
  store float %div.542.i, float* %"module_mp_thompson_mp_mp_thompson_$QVS378[]", align 4
  %rel.1299.i15303 = fcmp reassoc ninf nsz arcp contract afn oge float %mul.3469.i, 0x4083191C40000000
  %slct.474.i15304 = select reassoc ninf nsz arcp contract afn i1 %rel.1299.i15303, float 0x4083191C40000000, float %mul.3469.i
  %mul.3470.i15305 = fmul reassoc ninf nsz arcp contract afn float %slct.474.i15304, 0x3FE3E76C80000000
  %sub.956.i15306 = fsub reassoc ninf nsz arcp contract afn float %P_fetch.16969.i, %slct.474.i15304
  %div.542.i15307 = fdiv reassoc ninf nsz arcp contract afn float %mul.3470.i15305, %sub.956.i15306
  %"module_mp_thompson_mp_mp_thompson_$QV1066[]1821" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$QV1066", i64 %indvars.iv15560)
  %"module_mp_thompson_mp_mp_thompson_$QV1066[]1821_fetch.4763" = load float, float* %"module_mp_thompson_mp_mp_thompson_$QV1066[]1821", align 4
  %sub.348 = fsub reassoc ninf nsz arcp contract afn float %div.542.i15307, %"module_mp_thompson_mp_mp_thompson_$QV1066[]1821_fetch.4763"
  %rel.503 = fcmp reassoc ninf nsz arcp contract afn ole float %sub.348, 0.000000e+00
  %slct.257 = select reassoc ninf nsz arcp contract afn i1 %rel.503, float 0.000000e+00, float %sub.348
  %"module_mp_thompson_mp_mp_thompson_$DELQVS362[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$DELQVS362", i64 %indvars.iv15560)
  store float %slct.257, float* %"module_mp_thompson_mp_mp_thompson_$DELQVS362[]", align 4
  %rel.504 = fcmp reassoc ninf nsz arcp contract afn ugt float %sub.347, 0.000000e+00
  %"module_mp_thompson_mp_mp_thompson_$QVSI370[]1866" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$QVSI370", i64 %indvars.iv15560)
  br i1 %rel.504, label %bb469_endif, label %call.pre.list1834_then

call.pre.list1834_then:                           ; preds = %bb465
  %mul.3471.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, 0x3D46B8A620000000
  %add.2170.i = fadd reassoc ninf nsz arcp contract afn float %mul.3471.i, 0x3DDD13F800000000
  %mul.3472.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2170.i
  %add.2171.i = fadd reassoc ninf nsz arcp contract afn float %mul.3472.i, 0x3E6086EA40000000
  %mul.3473.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2171.i
  %add.2172.i = fadd reassoc ninf nsz arcp contract afn float %mul.3473.i, 0x3ED5E1A560000000
  %mul.3474.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2172.i
  %add.2173.i = fadd reassoc ninf nsz arcp contract afn float %mul.3474.i, 0x3F4286DC40000000
  %mul.3475.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2173.i
  %add.2174.i = fadd reassoc ninf nsz arcp contract afn float %mul.3475.i, 0x3FA49EC1C0000000
  %mul.3476.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2174.i
  %add.2175.i = fadd reassoc ninf nsz arcp contract afn float %mul.3476.i, 0x3FFD8C30E0000000
  %mul.3477.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2175.i
  %add.2176.i = fadd reassoc ninf nsz arcp contract afn float %mul.3477.i, 0x4048F74C80000000
  %mul.3478.i = fmul reassoc ninf nsz arcp contract afn float %slct.473.i, %add.2176.i
  %add.2177.i = fadd reassoc ninf nsz arcp contract afn float %mul.3478.i, 0x40830EF3C0000000
  %rel.1301.i = fcmp reassoc ninf nsz arcp contract afn ole float %add.2177.i, %mul.3469.i
  %slct.476.i = select reassoc ninf nsz arcp contract afn i1 %rel.1301.i, float %add.2177.i, float %mul.3469.i
  %mul.3480.i = fmul reassoc ninf nsz arcp contract afn float %slct.476.i, 0x3FE3E76C80000000
  %sub.958.i = fsub reassoc ninf nsz arcp contract afn float %P_fetch.16969.i, %slct.476.i
  %div.543.i = fdiv reassoc ninf nsz arcp contract afn float %mul.3480.i, %sub.958.i
  br label %bb469_endif

bb469_endif:                                      ; preds = %call.pre.list1834_then, %bb465
  %storemerge15612 = phi float [ %div.543.i, %call.pre.list1834_then ], [ %div.542.i, %bb465 ]
  store float %storemerge15612, float* %"module_mp_thompson_mp_mp_thompson_$QVSI370[]1866", align 4
  %div.236 = fdiv reassoc ninf nsz arcp contract afn float %"module_mp_thompson_mp_mp_thompson_$QV1066[]1821_fetch.4763", %div.542.i
  %"module_mp_thompson_mp_mp_thompson_$SATW354[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$SATW354", i64 %indvars.iv15560)
  store float %div.236, float* %"module_mp_thompson_mp_mp_thompson_$SATW354[]", align 4
  %div.237 = fdiv reassoc ninf nsz arcp contract afn float %"module_mp_thompson_mp_mp_thompson_$QV1066[]1821_fetch.4763", %storemerge15612
  %"module_mp_thompson_mp_mp_thompson_$SATI346[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$SATI346", i64 %indvars.iv15560)
  store float %div.237, float* %"module_mp_thompson_mp_mp_thompson_$SATI346[]", align 4
  %sub.349 = fadd reassoc ninf nsz arcp contract afn float %div.236, -1.000000e+00
  %"module_mp_thompson_mp_mp_thompson_$SSATW338[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$SSATW338", i64 %indvars.iv15560)
  store float %sub.349, float* %"module_mp_thompson_mp_mp_thompson_$SSATW338[]", align 4
  %sub.350 = fadd reassoc ninf nsz arcp contract afn float %div.237, -1.000000e+00
  %"module_mp_thompson_mp_mp_thompson_$SSATI330[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$SSATI330", i64 %indvars.iv15560)
  store float %sub.350, float* %"module_mp_thompson_mp_mp_thompson_$SSATI330[]", align 4
  %abs.4830 = tail call reassoc ninf nsz arcp contract afn float @llvm.fabs.f32(float %sub.349)
  %rel.505 = fcmp reassoc ninf nsz arcp contract afn olt float %abs.4830, 0x3CD203AFA0000000
  br i1 %rel.505, label %bb_new1853_then, label %bb471_endif

bb_new1853_then:                                  ; preds = %bb469_endif
  store float 0.000000e+00, float* %"module_mp_thompson_mp_mp_thompson_$SSATW338[]", align 4
  br label %bb471_endif

bb471_endif:                                      ; preds = %bb_new1853_then, %bb469_endif
  %abs.4838 = tail call reassoc ninf nsz arcp contract afn float @llvm.fabs.f32(float %sub.350)
  %rel.506 = fcmp reassoc ninf nsz arcp contract afn olt float %abs.4838, 0x3CD203AFA0000000
  br i1 %rel.506, label %bb_new1857_then, label %bb473_endif

bb_new1857_then:                                  ; preds = %bb471_endif
  store float 0.000000e+00, float* %"module_mp_thompson_mp_mp_thompson_$SSATI330[]", align 4
  br label %bb473_endif

bb473_endif:                                      ; preds = %bb_new1857_then, %bb471_endif
  %"module_mp_thompson_mp_mp_thompson_$SSATI330[]_fetch.4846" = phi float [ %sub.350, %bb471_endif ], [ 0.000000e+00, %bb_new1857_then ]
  %and.632 = and i32 %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7", 1
  %rel.507 = icmp ne i32 %and.632, 0
  %rel.508 = fcmp reassoc ninf nsz arcp contract afn ogt float %"module_mp_thompson_mp_mp_thompson_$SSATI330[]_fetch.4846", 0.000000e+00
  %and.633 = and i1 %rel.507, %rel.508
  %spec.select = select i1 %and.633, i32 0, i32 %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7"
  %div.238 = fmul reassoc ninf nsz arcp contract afn float %"module_mp_thompson_mp_mp_thompson_$TEMP450[]_fetch.4730", 0x3F6DFDA840000000
  %func_result1919 = tail call reassoc ninf nsz arcp contract afn float @llvm.pow.f32(float %div.238, float 0x3FFF0A3D80000000)
  %14 = fmul reassoc ninf nsz arcp contract afn float %func_result1919, 0x40011A8980000000
  %mul.983 = fdiv reassoc ninf nsz arcp contract afn float %14, %P_fetch.16969.i
  %"module_mp_thompson_mp_mp_thompson_$DIFFU322[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$DIFFU322", i64 %indvars.iv15560)
  store float %mul.983, float* %"module_mp_thompson_mp_mp_thompson_$DIFFU322[]", align 4
  %rel.509 = fcmp reassoc ninf nsz arcp contract afn ult float %sub.347, 0.000000e+00
  %"module_mp_thompson_mp_mp_thompson_$VISCO314[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$VISCO314", i64 %indvars.iv15560)
  br i1 %rel.509, label %bb_new1868_else, label %bb_new1866_then

bb_new1866_then:                                  ; preds = %bb473_endif
  %15 = fmul reassoc ninf nsz arcp contract afn float %sub.347, 0x3E6A4E8240000000
  %mul.986 = fadd reassoc ninf nsz arcp contract afn float %15, 0x3EF203B8A0000000
  br label %bb476_endif

bb_new1868_else:                                  ; preds = %bb473_endif
  %mul.988 = fmul reassoc ninf nsz arcp contract afn float %sub.347, 0x3F741205C0000000
  %add.826 = fadd reassoc ninf nsz arcp contract afn float %mul.988, 0x3FFB7CEDA0000000
  %16 = fmul reassoc ninf nsz arcp contract afn float %sub.347, %sub.347
  %mul.990 = fmul reassoc ninf nsz arcp contract afn float %16, 0x3EE92A7380000000
  %sub.351 = fsub reassoc ninf nsz arcp contract afn float %add.826, %mul.990
  %mul.991 = fmul reassoc ninf nsz arcp contract afn float %sub.351, 0x3EE4F8B580000000
  br label %bb476_endif

bb476_endif:                                      ; preds = %bb_new1868_else, %bb_new1866_then
  %storemerge15362 = phi float [ %mul.986, %bb_new1866_then ], [ %mul.991, %bb_new1868_else ]
  store float %storemerge15362, float* %"module_mp_thompson_mp_mp_thompson_$VISCO314[]", align 4
  %17 = fmul reassoc ninf nsz arcp contract afn float %"module_mp_thompson_mp_mp_thompson_$QV1066[]1821_fetch.4763", 0x408BD46260000000
  %mul.993 = fadd reassoc ninf nsz arcp contract afn float %17, 1.004000e+03
  %div.240 = fdiv reassoc ninf nsz arcp contract afn float 1.000000e+00, %mul.993
  %"module_mp_thompson_mp_mp_thompson_$OCP282[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$OCP282", i64 %indvars.iv15560)
  store float %div.240, float* %"module_mp_thompson_mp_mp_thompson_$OCP282[]", align 4
  %div.241 = fdiv reassoc ninf nsz arcp contract afn float %"module_mp_thompson_mp_mp_thompson_$RHO1034[]_fetch.4734", %storemerge15362
  %func_result1948 = tail call reassoc ninf nsz arcp contract afn float @llvm.sqrt.f32(float %div.241)
  %"module_mp_thompson_mp_mp_thompson_$VSC2306[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$VSC2306", i64 %indvars.iv15560)
  store float %func_result1948, float* %"module_mp_thompson_mp_mp_thompson_$VSC2306[]", align 4
  %mul.996 = fmul reassoc ninf nsz arcp contract afn float %sub.347, 2.112000e+03
  %18 = fsub reassoc ninf nsz arcp contract afn float 2.500000e+06, %mul.996
  %"module_mp_thompson_mp_mp_thompson_$LVAP290[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$LVAP290", i64 %indvars.iv15560)
  store float %18, float* %"module_mp_thompson_mp_mp_thompson_$LVAP290[]", align 4
  %19 = fmul reassoc ninf nsz arcp contract afn float %sub.347, 0x3F12733480000000
  %mul.1000 = fadd reassoc ninf nsz arcp contract afn float %19, 0x3F9868D8C0000000
  %"module_mp_thompson_mp_mp_thompson_$TCOND298[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %"module_mp_thompson_mp_mp_thompson_$TCOND298", i64 %indvars.iv15560)
  store float %mul.1000, float* %"module_mp_thompson_mp_mp_thompson_$TCOND298[]", align 4
  %indvars.iv.next15561 = add nsw i64 %indvars.iv15560, 1
  %exitcond15563 = icmp eq i64 %indvars.iv.next15561, %wide.trip.count15562
  br i1 %exitcond15563, label %bb466, label %bb465

bb466:                                            ; preds = %bb476_endif
  %spec.select.lcssa = phi i32 [ %"module_mp_thompson_mp_mp_thompson_$NO_MICRO.7", %bb476_endif ]
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.pow.f32(float, float) #2

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.sqrt.f32(float) #2

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.fabs.f32(float) #2

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.smax.i32(i32, i32) #2

; uselistorder directives
uselistorder float* (i8, i64, i64, float*, i64)* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64, { 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 }
uselistorder float (float, float)* @llvm.pow.f32, { 2, 1, 0 }
uselistorder float (float)* @llvm.sqrt.f32, { 2, 1, 0 }
uselistorder float (float)* @llvm.fabs.f32, { 1, 0 }
uselistorder i32 (i32, i32)* @llvm.smax.i32, { 1, 0 }

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
