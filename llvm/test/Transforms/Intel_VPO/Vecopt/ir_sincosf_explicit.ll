; Test to check sincos functions are vectorized to SVML function in LLVM-IR vector CG.

; RUN: opt -vector-library=SVML -vplan-vec -verify -S -vplan-force-vf=4 %s | FileCheck -DVL=4 --check-prefixes=CHECK %s
; RUN: opt -vector-library=SVML -vplan-vec -verify -S -vplan-force-vf=16 %s | FileCheck -DVL=16 --check-prefixes=CHECK %s

; CHECK: [[RESULT:%.*]] = call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf[[VL]](<[[VL]] x float> {{.*}})
; CHECK: [[RESULT_SIN:%.*]] = extractvalue { <[[VL]] x float>, <[[VL]] x float> } [[RESULT]], 0
; CHECK: [[RESULT_COS:%.*]] = extractvalue { <[[VL]] x float>, <[[VL]] x float> } [[RESULT]], 1
; CHECK: store <[[VL]] x float> [[RESULT_SIN]], <[[VL]] x float>* {{.*}}, align 4
; CHECK: store <[[VL]] x float> [[RESULT_COS]], <[[VL]] x float>* {{.*}}, align 4

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(float* %a, float* %vsin, float* %vcos, i32 %i)  {
simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM:TYPED"(float* %vsin, float zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(float* %vcos, float zeroinitializer, i32 1) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %idxprom = sext i32 %i to i64
  %stride.mul = mul i32 1, %index
  %stride.cast = sext i32 %stride.mul to i64
  %stride.add = add i64 %idxprom, %stride.cast
  %arrayidx = getelementptr inbounds float, float* %a, i64 %stride.add
  %0 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float, float* %vsin, i64 %stride.add
  %arrayidx4 = getelementptr inbounds float, float* %vcos, i64 %stride.add
  tail call void @sincosf(float %0, float* %arrayidx2, float* %arrayidx4) #3
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare void @sincosf(float, float*, float*)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
