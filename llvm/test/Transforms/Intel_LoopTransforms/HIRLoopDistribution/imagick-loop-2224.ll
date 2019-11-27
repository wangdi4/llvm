; RUN: opt -xmain-opt-level=3 -disable-output -S -hir-loop-distribute-scex-cost=12 -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec < %s 2>&1 | FileCheck %s
; RUN: opt -xmain-opt-level=3 -disable-output -S -hir-loop-distribute-scex-cost=12 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; The imagick loop on line 2224. Check that we are able to distribute it.

; BEGIN REGION { }
;       + DO i1 = 0, -1 * %arg + smax(%arg, %arg18), 1   <DO_LOOP>
;       |   %tmp38 = i1 + %arg  *  i1 + %arg;
;       |   %tmp40 = sitofp.i64.double(i1 + %arg);
;       |   %tmp41 = %arg3  +  %tmp40;
;       |
;       |   + DO i2 = 0, -1 * %arg5 + smax(%arg5, %arg17), 1   <DO_LOOP>
;       |   |   %tmp52 = i2 + %arg5  *  i2 + %arg5;
;       |   |   %tmp53 = %tmp52  +  %tmp38;
;       |   |   if (%tmp53 <= %arg6)
;       |   |   {
;       |   |      %tmp57 = (%arg8)[0];
;       |   |      %tmp59 = (%tmp57)[0].12.0;
;       |   |      %tmp61 = (%tmp57)[0].12.1;
;       |   |      %tmp63 = (%tmp57)[0].12.2;
;       |   |      %tmp65 = (%tmp57)[0].12.3;
;       |   |      %tmp67 = (%arg10)[0];
;       |   |      %tmp68 = (%tmp67)[0];
;       |   |      %tmp69 = @wobble(...)
;       |   |      %tmp81 = %tmp65;
;       |   |      %tmp82 = %tmp63;
;       |   |      %tmp83 = %tmp61;
;       |   |      %tmp84 = %tmp59;
;       |   |      %tmp85 = 0;
;       |   |      if (&((%tmp69)[0]) != null)
;       |   |      {
;       |   |         %tmp81 = (%tmp69)[0].3;
;       |   |         %tmp82 = (%tmp69)[0].2;
;       |   |         %tmp83 = (%tmp69)[0].1;
;       |   |         %tmp84 = (%tmp69)[0].0;
;       |   |         %tmp85 = 1;
;       |   |      }
;       |   |      %tmp86 = uitofp.i16.float(%tmp82);
;       |   |      %tmp87 = %arg12  -  %tmp86;
;       |   |      %tmp88 = %tmp87  *  %tmp87;
;       |   |      %tmp89 = uitofp.i16.float(%tmp83);
;       |   |      %tmp90 = %arg13  -  %tmp89;
;       |   |      %tmp91 = %tmp90  *  %tmp90;
;       |   |      %tmp92 = %tmp91  +  %tmp88;
;       |   |      %tmp93 = uitofp.i16.float(%tmp84);
;       |   |      %tmp94 = %arg14  -  %tmp93;
;       |   |      %tmp95 = %tmp94  *  %tmp94;
;       |   |      %tmp96 = %tmp92  +  %tmp95;
;       |   |      %tmp97 = fpext.float.double(%tmp96);
;       |   |      if (%arg15 <u %tmp97)
;       |   |      {
;       |   |         %tmp37 = %tmp85;
;       |   |      }
;       |   |      else
;       |   |      {
;       |   |         %tmp100 = sitofp.i64.double(i2 + %arg5);
;       |   |         %tmp101 = %tmp35  +  %arg16;
;       |   |         %tmp35 = %tmp101  +  %tmp100;
;       |   |         %tmp36 = %tmp41  +  %tmp36;
;       |   |         %tmp32 = %tmp32  +  %tmp86;
;       |   |         %tmp31 = %tmp31  +  %tmp89;
;       |   |         %tmp30 = %tmp30  +  %tmp93;
;       |   |         %tmp107 = uitofp.i16.float(%tmp81);
;       |   |         %tmp = %tmp  +  %tmp107;
;       |   |         %tmp34 = %tmp34  +  1;
;       |   |         %tmp37 = %tmp85;
;       |   |      }
;       |   |   }
;       |   + END LOOP
;       |
;       |   (%arg26)[0] = %tmp34;
;       |   (%arg25)[0] = %tmp35;
;       |   (%arg24)[0] = %tmp36;
;       |   (%arg23)[0] = %tmp37;
;       |   (%arg22)[0] = %tmp32;
;       |   (%arg21)[0] = %tmp31;
;       |   (%arg20)[0] = %tmp30;
;       |   (%arg19)[0] = %tmp;
;       + END LOOP
; END REGION

; CHECK: DO i1
; CHECK:   DO i2
; CHECK:     DO i3
; CHECK:     DO i3


; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-conditional-temp-sinking -print-before=hir-conditional-temp-sinking -print-after=hir-conditional-temp-sinking < %s 2>&1 | FileCheck %s --check-prefix=TEMP-SINKING
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-conditional-temp-sinking,print<hir>" < %s 2>&1 | FileCheck %s --check-prefix=TEMP-SINKING

; Dump Before-

; TEMP-SINKING: |   |      if (%arg15 <u %tmp97)
; TEMP-SINKING: |   |      {
; TEMP-SINKING: |   |         %tmp37 = %tmp85;
; TEMP-SINKING: |   |      }
; TEMP-SINKING: |   |      else
; TEMP-SINKING: |   |      {
; TEMP-SINKING: |   |         %tmp37 = %tmp85;
; TEMP-SINKING: |   |      }
     

; Dump After-

; TEMP-SINKING: |   |      if (%arg15 >= %tmp97)
; TEMP-SINKING: |   |      {
; TEMP-SINKING: |   |      }
; TEMP-SINKING: |   |      %tmp37 = %tmp85;
    
 
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
