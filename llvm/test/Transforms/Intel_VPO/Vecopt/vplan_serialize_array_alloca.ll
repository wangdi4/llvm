; This test checks that we have serialized allocation and serialized load/stores
; from private array-types.

; This test was for an experimental feature and is not meant to run for VPValue-based codegen.

; RUN: opt -S -lcssa -VPlanDriver -vplan-force-vf=4 -vplan-serialize-alloca=true -enable-vp-value-codegen=false %s | FileCheck %s

; CHECK: [[ORIG_PRIV1:%.*]] = alloca [624 x i32], align 4
; CHECK: [[PRIV1:%.*]] = alloca [624 x i32], align 4
; CHECK-NEXT: [[PRIV2:%.*]] = alloca [624 x i32], align 4
; CHECK-NEXT: [[PRIV3:%.*]] = alloca [624 x i32], align 4
; CHECK-NEXT: [[PRIV4:%.*]] = alloca [624 x i32], align 4

; CHECK-LABEL: vector.body
; CHECK: [[PRIV_BASE1:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV1]], i64 0, i64 0

; CHECK: [[PRIV_BASE2:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV2]], i64 0, i64 0

; CHECK: [[PRIV_BASE3:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV3]], i64 0, i64 0

; CHECK: [[PRIV_BASE4:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV4]], i64 0, i64 0

; CHECK-NEXT: [[SV1:%.*]] = extractelement <4 x i32> [[TRUNC:%.*]], i64 0
; CHECK-NEXT: store i32 {{.*}}, i32* [[PRIV_BASE1]], align 4
; CHECK-NEXT: [[SV2:%.*]] = extractelement <4 x i32> [[TRUNC:%.*]], i64 1
; CHECK-NEXT: store i32 {{.*}}, i32* [[PRIV_BASE2]], align 4
; CHECK-NEXT: [[SV3:%.*]] = extractelement <4 x i32> [[TRUNC:%.*]], i64 2
; CHECK-NEXT: store i32 {{.*}}, i32* [[PRIV_BASE3]], align 4
; CHECK-NEXT: [[SV4:%.*]] = extractelement <4 x i32> [[TRUNC:%.*]], i64 3
; CHECK-NEXT: store i32 {{.*}}, i32* [[PRIV_BASE4]], align 4

; CHECK: [[V1:%.*]] = load i32, i32* [[PRIV_BASE1]], align 4
; CHECK-NEXT: [[VEC1:%.*]] = insertelement <4 x i32> undef, i32 [[V1]], i64 0

; CHECK-NEXT: [[V2:%.*]] = load i32, i32* [[PRIV_BASE2]], align 4
; CHECK-NEXT: [[VEC2:%.*]] = insertelement <4 x i32> [[VEC1]], i32 [[V2]], i64 1

; CHECK-NEXT: [[V3:%.*]] = load i32, i32* [[PRIV_BASE3]], align 4
; CHECK-NEXT: [[VEC3:%.*]] = insertelement <4 x i32> [[VEC2]], i32 [[V3]], i64 2

; CHECK-NEXT: [[V4:%.*]] = load i32, i32* [[PRIV_BASE4]], align 4
; CHECK-NEXT: [[VEC4:%.*]] = insertelement <4 x i32> [[VEC3]], i32 [[V4]], i64 3

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"


; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32)

define void @"MonteCarloSimulation"(float %_arg_) {
  %cosPtr = alloca float
  %priv = alloca [624 x i32], align 4
  %gid = call i64 @_Z13get_global_idj(i32 0)
  br label %simd.begin.region

simd.begin.region:                                ; preds = %0
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(float %_arg_), "QUAL.OMP.PRIVATE"(float* %cosPtr, [624 x i32]* %priv), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %s = sext i32 %index to i64
  %bc1 = bitcast [624 x i32]* %priv to i8*
  %add = add nuw i64 %s, %gid
  %trunc = trunc i64 %add to i32
  %gep1 = getelementptr inbounds [624 x i32], [624 x i32]* %priv, i64 0, i64 0
  store i32 %trunc, i32* %gep1, align 4
  %load1 = load i32, i32* %gep1, align 4
  br label %simd.loop.exit
simd.loop.exit:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region
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
