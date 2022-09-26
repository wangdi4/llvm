; RUN: opt -vector-library=SVML -vplan-vec -S -vplan-force-vf=4  %s | FileCheck -DVL=4 %s
; RUN: opt -vector-library=SVML -vplan-vec -S -vplan-force-vf=8  %s | FileCheck -DVL=8 %s
; RUN: opt -vector-library=SVML -vplan-vec -S -vplan-force-vf=16 %s | FileCheck -DVL=16 %s
; RUN: opt -vector-library=SVML -vplan-vec -S -vplan-force-vf=32 %s | FileCheck -DVL=32 %s

; CHECK-LABEL: test_sinf
; CHECK:  [[RESULT:%.*]] = call fast svml_cc <[[VL]] x float> @__svml_sinf[[VL]](<[[VL]] x float> {{.*}})
; CHECK:  [[PTR:%.*]] = bitcast float* {{.*}} to <[[VL]] x float>*
; CHECK:  store <[[VL]] x float> [[RESULT]], <[[VL]] x float>* [[PTR]], align 4

; CHECK-LABEL: test_sin
; CHECK:  [[RESULT:%.*]] = call fast svml_cc <[[VL]] x double> @__svml_sin[[VL]](<[[VL]] x double> {{.*}})
; CHECK:  [[PTR:%.*]] = bitcast double* {{.*}} to <[[VL]] x double>*
; CHECK:  store <[[VL]] x double> [[RESULT]], <[[VL]] x double>* [[PTR]], align 8

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define x86_regcallcc void @test_sinf(float* nocapture %a, float* nocapture readonly %b, i32 %i) {
simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM:TYPED"(float* %a, float zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(float* %b, float zeroinitializer, i32 1) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %idxprom = sext i32 %i to i64
  %stride.mul = mul i32 1, %index
  %stride.cast = sext i32 %stride.mul to i64
  %stride.add = add i64 %idxprom, %stride.cast
  %arrayidx = getelementptr inbounds float, float* %b, i64 %stride.add
  %0 = load float, float* %arrayidx, align 4
  %call = tail call fast float @sinf(float %0) #3
  %arrayidx2 = getelementptr inbounds float, float* %a, i64 %stride.add
  store float %call, float* %arrayidx2, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 32
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define x86_regcallcc void @test_sin(double* nocapture %a, double* nocapture readonly %b, i32 %i) {
simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM:TYPED"(double* %a, float zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(double* %b, float zeroinitializer, i32 1) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %idxprom = sext i32 %i to i64
  %stride.mul = mul i32 1, %index
  %stride.cast = sext i32 %stride.mul to i64
  %stride.add = add i64 %idxprom, %stride.cast
  %arrayidx = getelementptr inbounds double, double* %b, i64 %stride.add
  %0 = load double, double* %arrayidx, align 8
  %call = tail call fast double @sin(double %0) #3
  %arrayidx2 = getelementptr inbounds double, double* %a, i64 %stride.add
  store double %call, double* %arrayidx2, align 8
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 32
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

declare float @sinf(float)
declare double @sin(double)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
