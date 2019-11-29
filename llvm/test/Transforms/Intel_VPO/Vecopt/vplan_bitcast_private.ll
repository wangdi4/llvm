; This test checks that bitcast instruction is correctly widened for pointers to private aggregate type.

; RUN: opt -S -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen=false %s | FileCheck %s --check-prefixes=CHECK,CHECK-LLVM
; RUN: opt -S -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen %s | FileCheck %s --check-prefixes=CHECK,CHECK-VPVALUE


; CHECK:[[VEC1:%.*]] = alloca [2 x [624 x i32]], align 4
; CHECK: [[PRIV:%.*]] = bitcast [2 x [624 x i32]]* [[VEC1]] to [624 x i32]*
; CHECK: [[GEP:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV]], <2 x i32> <i32 0, i32 1>
; CHECK: [[BC:%.*]] = bitcast <2 x [624 x i32]*>  [[GEP]] to <2 x i8*>
; CHECK: [[E2:%.*]] = extractelement <2 x i8*> [[BC]], i32 1
; CHECK: [[E1:%.*]] = extractelement <2 x i8*> [[BC]], i32 0
; CHECK-LLVM: call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull [[E1]])
; CHECK-VPVALUE: call void @llvm.lifetime.start.p0i8(i64 2496, i8* [[E1]])
; CHECK-LLVM-NEXT: call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull [[E2]])
; CHECK-VPVALUE-NEXT: call void @llvm.lifetime.start.p0i8(i64 2496, i8* [[E2]])

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)
declare float @_Z3cosf(float)
declare i64 @_Z13get_global_idj(i32)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; Function Attrs: nounwind
define void @"test_bitcast"(){
  %1 = alloca [624 x i32], align 4
  %2 = call i64 @_Z13get_global_idj(i32 0)
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([624 x i32]* %1) ]
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %3 = sext i32 %index to i64
  %add = add nuw i64 %3, %2
  %bc = bitcast [624 x i32]* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %bc)
  %add1 = trunc i64 %add to i32
  %gep = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 0
  store i32 %add1, i32* %gep, align 4
  br label %simd.loop.exit

simd.loop.exit:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}
