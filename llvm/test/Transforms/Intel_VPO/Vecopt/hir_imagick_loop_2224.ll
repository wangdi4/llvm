; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; XFAIL:*
; TODO: CMPLRLLVM-34082 fixed logic that moved the SCEX load inside the same if level as the original def. This caused the Vplan CM to not vectorize the loop. Needs investigation.
; RUN: opt -xmain-opt-level=3 -disable-output -S -hir-loop-distribute-scex-cost=12 -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-distribute-memrec -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s

; The imagick loop on line 2224. Check that we are able to distribute and vectorize it.

; Incoming HIR of interest.
;    |   |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;    |   |
;    |   |   + DO i3 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>   <LEGAL_MAX_TC = 64>
;    |   |   |   %tmp52 = 64 * i2 + i3 + %arg5  *  64 * i2 + i3 + %arg5;
;    |   |   |   %tmp53 = %tmp52  +  %tmp38;
;    |   |   |   %tmp81 = (%.TempArray)[0][i3];
;    |   |   |   %tmp82 = (%.TempArray10)[0][i3];
;    |   |   |   %tmp83 = (%.TempArray12)[0][i3];
;    |   |   |   %tmp84 = (%.TempArray14)[0][i3];
;    |   |   |   %tmp85 = (%.TempArray16)[0][i3];
;    |   |   |   if (%tmp53 <= %arg6)
;    |   |   |   {
;    |   |   |      %tmp86 = uitofp.i16.float(%tmp82);
;    |   |   |      %tmp87 = %arg12  -  %tmp86;
;    |   |   |      %tmp88 = %tmp87  *  %tmp87;
;    |   |   |      %tmp89 = uitofp.i16.float(%tmp83);
;    |   |   |      %tmp90 = %arg13  -  %tmp89;
;    |   |   |      %tmp91 = %tmp90  *  %tmp90;
;    |   |   |      %tmp92 = %tmp91  +  %tmp88;
;    |   |   |      %tmp93 = uitofp.i16.float(%tmp84);
;    |   |   |      %tmp94 = %arg14  -  %tmp93;
;    |   |   |      %tmp95 = %tmp94  *  %tmp94;
;    |   |   |      %tmp96 = %tmp92  +  %tmp95;
;    |   |   |      %tmp97 = fpext.float.double(%tmp96);
;    |   |   |      if (%arg15 <u %tmp97)
;    |   |   |      {
;    |   |   |         %tmp37 = %tmp85;
;    |   |   |      }
;    |   |   |      else
;    |   |   |      {
;    |   |   |         %tmp100 = sitofp.i64.double(64 * i2 + i3 + %arg5);
;    |   |   |         %tmp101 = %tmp35  +  %arg16; <Safe Reduction>
;    |   |   |         %tmp35 = %tmp101  +  %tmp100; <Safe Reduction>
;    |   |   |         %tmp36 = %tmp41  +  %tmp36; <Safe Reduction>
;    |   |   |         %tmp32 = %tmp32  +  %tmp86; <Safe Reduction>
;    |   |   |         %tmp31 = %tmp31  +  %tmp89; <Safe Reduction>
;    |   |   |         %tmp30 = %tmp30  +  %tmp93; <Safe Reduction>
;    |   |   |         %tmp107 = uitofp.i16.float(%tmp81);
;    |   |   |         %tmp = %tmp  +  %tmp107; <Safe Reduction>
;    |   |   |         %tmp34 = %tmp34  +  1; <Safe Reduction>
;    |   |   |         %tmp37 = %tmp85;
;    |   |   |      }
;    |   |   |   }
;    |   |   + END LOOP
;    |   |
;    |   |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;    |   + END LOOP
;    |
;    |   (%arg26)[0] = %tmp34;
;    |   (%arg25)[0] = %tmp35;
;    |   (%arg24)[0] = %tmp36;
;    |   (%arg23)[0] = %tmp37;
;    |   (%arg22)[0] = %tmp32;
;    |   (%arg21)[0] = %tmp31;
;    |   (%arg20)[0] = %tmp30;
;    |   (%arg19)[0] = %tmp;


; CHECK:            |   |   %tgu = (%min + 1)/u4;
; CHECK-NEXT:       |   |   if (0 <u 4 * %tgu)
; CHECK-NEXT:       |   |   {
; CHECK-NEXT:       |   |      %red.init = 0.000000e+00;
; CHECK-NEXT:       |   |      %red.init18 = 0.000000e+00;
; CHECK-NEXT:       |   |      %red.init19 = 0.000000e+00;
; CHECK-NEXT:       |   |      %red.init20 = 0.000000e+00;
; CHECK-NEXT:       |   |      %red.init21 = 0.000000e+00;
; CHECK-NEXT:       |   |      %red.init22 = 0.000000e+00;
; CHECK-NEXT:       |   |      %red.init23 = 0;
; CHECK-NEXT:       |   |      %red.init.insert = insertelement %red.init23,  %tmp34,  0;
; CHECK-NEXT:       |   |      %phi.temp = %tmp37;
; CHECK-NEXT:       |   |      %phi.temp24 = %red.init.insert;
; CHECK-NEXT:       |   |      %phi.temp26 = %red.init22;
; CHECK-NEXT:       |   |      %phi.temp28 = %red.init21;
; CHECK-NEXT:       |   |      %phi.temp30 = %red.init20;
; CHECK-NEXT:       |   |      %phi.temp32 = %red.init19;
; CHECK-NEXT:       |   |      %phi.temp34 = %red.init18;
; CHECK-NEXT:       |   |      %phi.temp36 = %red.init;
; CHECK-NEXT:       |   |      %phi.temp38 = -1;
; CHECK-NEXT:       |   |
; CHECK-NEXT:       |   |      + DO i3 = 0, 4 * %tgu + -1, 4   <DO_LOOP>  <MAX_TC_EST = 16>   <LEGAL_MAX_TC = 16> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:       |   |      |   %.vec = 64 * i2 + i3 + %arg5 + <i64 0, i64 1, i64 2, i64 3>  *  64 * i2 + i3 + %arg5 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:       |   |      |   %.vec40 = (<4 x i16>*)(%.TempArray)[0][i3];
; CHECK-NEXT:       |   |      |   %.vec41 = (<4 x i16>*)(%.TempArray10)[0][i3];
; CHECK-NEXT:       |   |      |   %.vec42 = (<4 x i16>*)(%.TempArray12)[0][i3];
; CHECK-NEXT:       |   |      |   %.vec43 = (<4 x i16>*)(%.TempArray14)[0][i3];
; CHECK-NEXT:       |   |      |   %.vec44 = (<4 x i32>*)(%.TempArray16)[0][i3];
; CHECK-NEXT:       |   |      |   %.vec45 = %tmp38 + %.vec <= %arg6;
; CHECK-NEXT:       |   |      |   %.vec46 = uitofp.<4 x i16>.<4 x float>(%.vec41);
; CHECK-NEXT:       |   |      |   %.vec47 = %arg12  -  %.vec46;
; CHECK-NEXT:       |   |      |   %.vec48 = %.vec47  *  %.vec47;
; CHECK-NEXT:       |   |      |   %.vec49 = uitofp.<4 x i16>.<4 x float>(%.vec42);
; CHECK-NEXT:       |   |      |   %.vec50 = %arg13  -  %.vec49;
; CHECK-NEXT:       |   |      |   %.vec51 = %.vec50  *  %.vec50;
; CHECK-NEXT:       |   |      |   %.vec52 = %.vec51  +  %.vec48;
; CHECK-NEXT:       |   |      |   %.vec53 = uitofp.<4 x i16>.<4 x float>(%.vec43);
; CHECK-NEXT:       |   |      |   %.vec54 = %arg14  -  %.vec53;
; CHECK-NEXT:       |   |      |   %.vec55 = %.vec54  *  %.vec54;
; CHECK-NEXT:       |   |      |   %.vec56 = %.vec52  +  %.vec55;
; CHECK-NEXT:       |   |      |   %.vec57 = fpext.<4 x float>.<4 x double>(%.vec56);
; CHECK-NEXT:       |   |      |   %.vec58 = %arg15 <u %.vec57;
; CHECK-NEXT:       |   |      |   %.vec59 = %.vec58  ^  -1;
; CHECK-NEXT:       |   |      |   %.vec60 = %.vec45  &  %.vec59;
; CHECK-NEXT:       |   |      |   %.vec61 = %.vec45  &  %.vec58;
; CHECK-NEXT:       |   |      |   %.vec62 = sitofp.<4 x i64>.<4 x double>(64 * i2 + i3 + %arg5 + <i64 0, i64 1, i64 2, i64 3>);
; CHECK-NEXT:       |   |      |   %.vec63 = %phi.temp36  +  %arg16;
; CHECK-NEXT:       |   |      |   %.vec64 = %.vec63  +  %.vec62;
; CHECK-NEXT:       |   |      |   %.vec65 = %tmp41  +  %phi.temp34;
; CHECK-NEXT:       |   |      |   %.vec66 = %phi.temp32  +  %.vec46;
; CHECK-NEXT:       |   |      |   %.vec67 = %phi.temp30  +  %.vec49;
; CHECK-NEXT:       |   |      |   %.vec68 = %phi.temp28  +  %.vec53;
; CHECK-NEXT:       |   |      |   %.vec69 = uitofp.<4 x i16>.<4 x float>(%.vec40);
; CHECK-NEXT:       |   |      |   %.vec70 = %phi.temp26  +  %.vec69;
; CHECK-NEXT:       |   |      |   %.vec71 = %phi.temp24  +  1;
; CHECK-NEXT:       |   |      |   %.copy72 = %.vec44;
; CHECK-NEXT:       |   |      |   %.copy73 = %.vec44;
; CHECK-NEXT:       |   |      |   %select = (%.vec60 == <i1 true, i1 true, i1 true, i1 true>) ? i3 + <i64 0, i64 1, i64 2, i64 3> : %phi.temp38;
; CHECK-NEXT:       |   |      |   %select74 = (%.vec61 == <i1 true, i1 true, i1 true, i1 true>) ? i3 + <i64 0, i64 1, i64 2, i64 3> : %select;
; CHECK-NEXT:       |   |      |   %select75 = (%.vec60 == <i1 true, i1 true, i1 true, i1 true>) ? %.copy72 : %phi.temp;
; CHECK-NEXT:       |   |      |   %select76 = (%.vec61 == <i1 true, i1 true, i1 true, i1 true>) ? %.copy73 : %select75;
; CHECK-NEXT:       |   |      |   %select77 = (%.vec60 == <i1 true, i1 true, i1 true, i1 true>) ? %.vec71 : %phi.temp24;
; CHECK-NEXT:       |   |      |   %select78 = (%.vec61 == <i1 true, i1 true, i1 true, i1 true>) ? %phi.temp24 : %select77;
; CHECK-NEXT:       |   |      |   %select79 = (%.vec60 == <i1 true, i1 true, i1 true, i1 true>) ? %.vec70 : %phi.temp26;
; CHECK-NEXT:       |   |      |   %select80 = (%.vec61 == <i1 true, i1 true, i1 true, i1 true>) ? %phi.temp26 : %select79;
; CHECK-NEXT:       |   |      |   %select81 = (%.vec60 == <i1 true, i1 true, i1 true, i1 true>) ? %.vec68 : %phi.temp28;
; CHECK-NEXT:       |   |      |   %select82 = (%.vec61 == <i1 true, i1 true, i1 true, i1 true>) ? %phi.temp28 : %select81;
; CHECK-NEXT:       |   |      |   %select83 = (%.vec60 == <i1 true, i1 true, i1 true, i1 true>) ? %.vec67 : %phi.temp30;
; CHECK-NEXT:       |   |      |   %select84 = (%.vec61 == <i1 true, i1 true, i1 true, i1 true>) ? %phi.temp30 : %select83;
; CHECK-NEXT:       |   |      |   %select85 = (%.vec60 == <i1 true, i1 true, i1 true, i1 true>) ? %.vec66 : %phi.temp32;
; CHECK-NEXT:       |   |      |   %select86 = (%.vec61 == <i1 true, i1 true, i1 true, i1 true>) ? %phi.temp32 : %select85;
; CHECK-NEXT:       |   |      |   %select87 = (%.vec60 == <i1 true, i1 true, i1 true, i1 true>) ? %.vec65 : %phi.temp34;
; CHECK-NEXT:       |   |      |   %select88 = (%.vec61 == <i1 true, i1 true, i1 true, i1 true>) ? %phi.temp34 : %select87;
; CHECK-NEXT:       |   |      |   %select89 = (%.vec60 == <i1 true, i1 true, i1 true, i1 true>) ? %.vec64 : %phi.temp36;
; CHECK-NEXT:       |   |      |   %select90 = (%.vec61 == <i1 true, i1 true, i1 true, i1 true>) ? %phi.temp36 : %select89;
; CHECK-NEXT:       |   |      |   %phi.temp = %select76;
; CHECK-NEXT:       |   |      |   %phi.temp24 = %select78;
; CHECK-NEXT:       |   |      |   %phi.temp26 = %select80;
; CHECK-NEXT:       |   |      |   %phi.temp28 = %select82;
; CHECK-NEXT:       |   |      |   %phi.temp30 = %select84;
; CHECK-NEXT:       |   |      |   %phi.temp32 = %select86;
; CHECK-NEXT:       |   |      |   %phi.temp34 = %select88;
; CHECK-NEXT:       |   |      |   %phi.temp36 = %select90;
; CHECK-NEXT:       |   |      |   %phi.temp38 = %select74;
; CHECK-NEXT:       |   |      + END LOOP
; CHECK-NEXT:       |   |
; CHECK-NEXT:       |   |      %tmp35 = @llvm.vector.reduce.fadd.v4f64(%tmp35,  %select90);
; CHECK-NEXT:       |   |      %tmp36 = @llvm.vector.reduce.fadd.v4f64(%tmp36,  %select88);
; CHECK-NEXT:       |   |      %tmp32 = @llvm.vector.reduce.fadd.v4f32(%tmp32,  %select86);
; CHECK-NEXT:       |   |      %tmp31 = @llvm.vector.reduce.fadd.v4f32(%tmp31,  %select84);
; CHECK-NEXT:       |   |      %tmp30 = @llvm.vector.reduce.fadd.v4f32(%tmp30,  %select82);
; CHECK-NEXT:       |   |      %tmp = @llvm.vector.reduce.fadd.v4f32(%tmp,  %select80);
; CHECK-NEXT:       |   |      %tmp34 = @llvm.vector.reduce.add.v4i64(%select78);
; CHECK-NEXT:       |   |      %.vec106 = %select74 != -1;
; CHECK-NEXT:       |   |      %0 = bitcast.<4 x i1>.i4(%.vec106);
; CHECK-NEXT:       |   |      %cmp = %0 == 0;
; CHECK-NEXT:       |   |      %all.zero.check = %cmp;
; CHECK-NEXT:       |   |      %phi.temp107 = %tmp37;
; CHECK-NEXT:       |   |      %unifcond = extractelement %all.zero.check,  0;
; CHECK-NEXT:       |   |      if (%unifcond == 1)
; CHECK-NEXT:       |   |      {
; CHECK-NEXT:       |   |         goto BB12.[[BB:[0-9]+]];
; CHECK-NEXT:       |   |      }
; CHECK-NEXT:       |   |      %priv.idx.max = @llvm.vector.reduce.smax.v4i64(%select74);
; CHECK-NEXT:       |   |      %priv.idx.cmp = %select74 == %priv.idx.max;
; CHECK-NEXT:       |   |      %bsfintmask = bitcast.<4 x i1>.i4(%priv.idx.cmp);
; CHECK-NEXT:       |   |      %bsf = @llvm.cttz.i4(%bsfintmask,  1);
; CHECK-NEXT:       |   |      %tmp37 = extractelement %select76,  %bsf;
; CHECK-NEXT:       |   |      %phi.temp107 = %tmp37;
; CHECK-NEXT:       |   |      BB12.[[BB]]:
; CHECK-NEXT:       |   |   }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.eggs.0 = type { i16, i16, i16, i16 }
%struct.zot = type { i32, i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, %struct.eggs.0*, %struct.eggs.0, %struct.eggs.0, %struct.eggs.0, double, %struct.spam.1, i32, i8*, i32, i8*, i8*, i8*, i64, double, double, %struct.widget, %struct.widget, %struct.widget, double, double, double, i32, i32, i32, i32, i32, i32, %struct.zot*, i64, i64, i64, i64, i64, i64, %struct.blam, %struct.widget.2, i32 (i8*, i64, i64, i8*)*, i8*, i8*, i8*, %struct.hoge.3*, %struct.bar*, [4096 x i8], [4096 x i8], [4096 x i8], i64, i64, %struct.hoge, i32, i64, %struct.bar.6*, %struct.snork, %struct.snork, %struct.snork*, i64, i64, %struct.zot*, %struct.zot*, %struct.zot*, i32, i32, %struct.eggs.0, %struct.zot*, %struct.widget, i8*, i8*, i32, i32, i64, i32, i64, i64, i32, i64 }
%struct.spam.1 = type { %struct.foo, %struct.foo, %struct.foo, %struct.foo }
%struct.foo = type { double, double, double }
%struct.blam = type { double, double, double }
%struct.widget.2 = type { %struct.ham, %struct.ham, i32, i64 }
%struct.ham = type { double, double, double }
%struct.hoge.3 = type { i64, i64, [10 x i8] }
%struct.bar = type { i64, i64, i64, i32, i32, i64, i64, i32, i32, i32, i32, i32, %struct.ham.4, %struct.wombat, i64 (%struct.zot*, i8*, i64)*, i8*, i32, %struct.bar.6*, i64, i64 }
%struct.ham.4 = type { %struct.barney* }
%struct.barney = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct.wobble*, %struct.barney*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct.wobble = type { %struct.wobble*, %struct.barney*, i32 }
%struct.wombat = type { i64, i64, i64, i32, i32, i32, i32, i64, i64, i64, i64, %struct.foo.5, %struct.foo.5, %struct.foo.5, [3 x i64] }
%struct.foo.5 = type { i64, i64 }
%struct.hoge = type { i32, i32, i8*, i8*, i8*, i32, %struct.bar.6*, i64 }
%struct.bar.6 = type { i64, i32, i64, i64 }
%struct.snork = type { i8*, i64, i8*, i64 }
%struct.widget = type { i64, i64, i64, i64 }
%struct.wombat.7 = type { i32, %struct.widget, i64, %struct.eggs.0*, %struct.eggs.0*, i32, i16*, i64 }

declare hidden fastcc %struct.eggs.0* @wobble(%struct.zot*, i32, i64, i64, i64, i64, %struct.wombat.7* nocapture, %struct.hoge*) unnamed_addr

define dso_local void @zot(i64 %arg, i32 %arg1, i64 %arg2, double %arg3, i64 %arg5, i64 %arg6, i64 %arg7, %struct.zot** %arg8, i32* %arg9, %struct.wombat.7*** %arg10, %struct.hoge* %arg11, float %arg12, float %arg13, float %arg14, double %arg15, double %arg16, i64 %arg17, i64 %arg18, float* %arg19, float* %arg20, float* %arg21, float* %arg22, i32* %arg23, double* %arg24, double* %arg25, i64* %arg26) {
bb:
  br label %bb29

bb27:                                             ; preds = %bb129
  ret void

bb29:                                             ; preds = %bb129, %bb
  %tmp = phi float [ %tmp130, %bb129 ], [ 0.000000e+00, %bb ]
  %tmp30 = phi float [ %tmp131, %bb129 ], [ 0.000000e+00, %bb ]
  %tmp31 = phi float [ %tmp132, %bb129 ], [ 0.000000e+00, %bb ]
  %tmp32 = phi float [ %tmp133, %bb129 ], [ 0.000000e+00, %bb ]
  %tmp33 = phi i64 [ %tmp138, %bb129 ], [ %arg, %bb ]
  %tmp34 = phi i64 [ %tmp137, %bb129 ], [ 0, %bb ]
  %tmp35 = phi double [ %tmp136, %bb129 ], [ 0.000000e+00, %bb ]
  %tmp36 = phi double [ %tmp135, %bb129 ], [ 0.000000e+00, %bb ]
  %tmp37 = phi i32 [ %tmp134, %bb129 ], [ %arg1, %bb ]
  %tmp38 = mul nsw i64 %tmp33, %tmp33
  %tmp39 = add i64 %tmp33, %arg2
  %tmp40 = sitofp i64 %tmp33 to double
  %tmp41 = fadd fast double %arg3, %tmp40
  br label %bb42

bb42:                                             ; preds = %bb118, %bb29
  %tmp43 = phi float [ %tmp, %bb29 ], [ %tmp119, %bb118 ]
  %tmp44 = phi float [ %tmp30, %bb29 ], [ %tmp120, %bb118 ]
  %tmp45 = phi float [ %tmp31, %bb29 ], [ %tmp121, %bb118 ]
  %tmp46 = phi float [ %tmp32, %bb29 ], [ %tmp122, %bb118 ]
  %tmp47 = phi i64 [ %arg5, %bb29 ], [ %tmp127, %bb118 ]
  %tmp48 = phi i64 [ %tmp34, %bb29 ], [ %tmp126, %bb118 ]
  %tmp49 = phi double [ %tmp35, %bb29 ], [ %tmp125, %bb118 ]
  %tmp50 = phi double [ %tmp36, %bb29 ], [ %tmp124, %bb118 ]
  %tmp51 = phi i32 [ %tmp37, %bb29 ], [ %tmp123, %bb118 ]
  %tmp52 = mul nsw i64 %tmp47, %tmp47
  %tmp53 = add nuw nsw i64 %tmp52, %tmp38
  %tmp54 = icmp sgt i64 %tmp53, %arg6
  br i1 %tmp54, label %bb118, label %bb55

bb55:                                             ; preds = %bb42
  %tmp56 = add i64 %tmp47, %arg7
  %tmp57 = load %struct.zot*, %struct.zot** %arg8, align 8
  %tmp58 = getelementptr inbounds %struct.zot, %struct.zot* %tmp57, i64 0, i32 12, i32 0
  %tmp59 = load i16, i16* %tmp58, align 2
  %tmp60 = getelementptr inbounds %struct.zot, %struct.zot* %tmp57, i64 0, i32 12, i32 1
  %tmp61 = load i16, i16* %tmp60, align 2
  %tmp62 = getelementptr inbounds %struct.zot, %struct.zot* %tmp57, i64 0, i32 12, i32 2
  %tmp63 = load i16, i16* %tmp62, align 2
  %tmp64 = getelementptr inbounds %struct.zot, %struct.zot* %tmp57, i64 0, i32 12, i32 3
  %tmp65 = load i16, i16* %tmp64, align 2
  %tmp66 = load i32, i32* %arg9, align 8
  %tmp67 = load %struct.wombat.7**, %struct.wombat.7*** %arg10, align 8
  %tmp68 = load %struct.wombat.7*, %struct.wombat.7** %tmp67, align 8
  %tmp69 = call fastcc %struct.eggs.0* @wobble(%struct.zot* %tmp57, i32 %tmp66, i64 %tmp56, i64 %tmp39, i64 1, i64 1, %struct.wombat.7* %tmp68, %struct.hoge* %arg11)
  %tmp70 = icmp eq %struct.eggs.0* %tmp69, null
  br i1 %tmp70, label %bb80, label %bb71

bb71:                                             ; preds = %bb55
  %tmp72 = getelementptr inbounds %struct.eggs.0, %struct.eggs.0* %tmp69, i64 0, i32 0
  %tmp73 = load i16, i16* %tmp72, align 2
  %tmp74 = getelementptr inbounds %struct.eggs.0, %struct.eggs.0* %tmp69, i64 0, i32 1
  %tmp75 = load i16, i16* %tmp74, align 2
  %tmp76 = getelementptr inbounds %struct.eggs.0, %struct.eggs.0* %tmp69, i64 0, i32 2
  %tmp77 = load i16, i16* %tmp76, align 2
  %tmp78 = getelementptr inbounds %struct.eggs.0, %struct.eggs.0* %tmp69, i64 0, i32 3
  %tmp79 = load i16, i16* %tmp78, align 2
  br label %bb80

bb80:                                             ; preds = %bb71, %bb55
  %tmp81 = phi i16 [ %tmp65, %bb55 ], [ %tmp79, %bb71 ]
  %tmp82 = phi i16 [ %tmp63, %bb55 ], [ %tmp77, %bb71 ]
  %tmp83 = phi i16 [ %tmp61, %bb55 ], [ %tmp75, %bb71 ]
  %tmp84 = phi i16 [ %tmp59, %bb55 ], [ %tmp73, %bb71 ]
  %tmp85 = phi i32 [ 1, %bb71 ], [ 0, %bb55 ]
  %tmp86 = uitofp i16 %tmp82 to float
  %tmp87 = fsub fast float %arg12, %tmp86
  %tmp88 = fmul fast float %tmp87, %tmp87
  %tmp89 = uitofp i16 %tmp83 to float
  %tmp90 = fsub fast float %arg13, %tmp89
  %tmp91 = fmul fast float %tmp90, %tmp90
  %tmp92 = fadd fast float %tmp91, %tmp88
  %tmp93 = uitofp i16 %tmp84 to float
  %tmp94 = fsub fast float %arg14, %tmp93
  %tmp95 = fmul fast float %tmp94, %tmp94
  %tmp96 = fadd fast float %tmp92, %tmp95
  %tmp97 = fpext float %tmp96 to double
  %tmp98 = fcmp fast ult double %arg15, %tmp97
  br i1 %tmp98, label %bb118, label %bb99

bb99:                                             ; preds = %bb80
  %tmp100 = sitofp i64 %tmp47 to double
  %tmp101 = fadd fast double %tmp49, %arg16
  %tmp102 = fadd fast double %tmp101, %tmp100
  %tmp103 = fadd fast double %tmp41, %tmp50
  %tmp104 = fadd fast float %tmp46, %tmp86
  %tmp105 = fadd fast float %tmp45, %tmp89
  %tmp106 = fadd fast float %tmp44, %tmp93
  %tmp107 = uitofp i16 %tmp81 to float
  %tmp108 = fadd fast float %tmp43, %tmp107
  %tmp109 = add nsw i64 %tmp48, 1
  br label %bb118

bb118:                                            ; preds = %bb80, %bb99, %bb42
  %tmp119 = phi float [ %tmp43, %bb42 ], [ %tmp43, %bb80 ], [ %tmp108, %bb99 ]
  %tmp120 = phi float [ %tmp44, %bb42 ], [ %tmp44, %bb80 ], [ %tmp106, %bb99 ]
  %tmp121 = phi float [ %tmp45, %bb42 ], [ %tmp45, %bb80 ], [ %tmp105, %bb99 ]
  %tmp122 = phi float [ %tmp46, %bb42 ], [ %tmp46, %bb80 ], [ %tmp104, %bb99 ]
  %tmp123 = phi i32 [ %tmp51, %bb42 ], [ %tmp85, %bb99 ], [ %tmp85, %bb80 ]
  %tmp124 = phi double [ %tmp50, %bb42 ], [ %tmp103, %bb99 ], [ %tmp50, %bb80 ]
  %tmp125 = phi double [ %tmp49, %bb42 ], [ %tmp102, %bb99 ], [ %tmp49, %bb80 ]
  %tmp126 = phi i64 [ %tmp48, %bb42 ], [ %tmp109, %bb99 ], [ %tmp48, %bb80 ]
  %tmp127 = add nsw i64 %tmp47, 1
  %tmp128 = icmp slt i64 %tmp47, %arg17
  br i1 %tmp128, label %bb42, label %bb129

bb129:                                            ; preds = %bb118
  %tmp130 = phi float [ %tmp119, %bb118 ]
  %tmp131 = phi float [ %tmp120, %bb118 ]
  %tmp132 = phi float [ %tmp121, %bb118 ]
  %tmp133 = phi float [ %tmp122, %bb118 ]
  %tmp134 = phi i32 [ %tmp123, %bb118 ]
  %tmp135 = phi double [ %tmp124, %bb118 ]
  %tmp136 = phi double [ %tmp125, %bb118 ]
  %tmp137 = phi i64 [ %tmp126, %bb118 ]
  store i64 %tmp137, i64* %arg26
  store double %tmp136, double* %arg25
  store double %tmp135, double* %arg24
  store i32 %tmp134, i32* %arg23
  store float %tmp133, float* %arg22
  store float %tmp132, float* %arg21
  store float %tmp131, float* %arg20
  store float %tmp130, float* %arg19
  %tmp138 = add nsw i64 %tmp33, 1
  %tmp139 = icmp slt i64 %tmp33, %arg18
  br i1 %tmp139, label %bb29, label %bb27
}
; end INTEL_FEATURE_SW_ADVANCED
