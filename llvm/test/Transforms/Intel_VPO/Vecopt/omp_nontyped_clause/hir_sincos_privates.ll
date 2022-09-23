; Test to verify that VPlan HIR vectorizer correctly handles a loop with a call
; to 'sincos' with SIMD private arguments.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vector-library=SVML -disable-output < %s 2>&1 | FileCheck %s

; CHECK:             BEGIN REGION { modified }
; CHECK:                   + DO i1 = 0, 127, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:              |   %.vec = (<4 x float>*)(%a)[i1 + sext.i32.i64(%i)];
; CHECK-NEXT:              |   %__svml_sincosf4 = @__svml_sincosf4(%.vec);
; CHECK-NEXT:              |   %vp.sincos.sin = extractvalue %__svml_sincosf4, 0;
; CHECK-NEXT:              |   %vp.sincos.cos = extractvalue %__svml_sincosf4, 1;
; CHECK-NEXT:              |   (<4 x float>*)(%priv.mem2)[0] = %vp.sincos.sin;
; CHECK-NEXT:              |   (<4 x float>*)(%priv.mem)[0] = %vp.sincos.cos;
; CHECK-NEXT:              |   %.vec4 = (<4 x float>*)(%priv.mem2)[0];
; CHECK-NEXT:              |   %.vec5 = (<4 x float>*)(%priv.mem)[0];
; CHECK-NEXT:              |   %.vec6 = %.vec4  +  %.vec5;
; CHECK-NEXT:              |   (<4 x float>*)(%b)[i1 + sext.i32.i64(%i)] = %.vec6;
; CHECK-NEXT:              + END LOOP

; CHECK:                   + DO i1 = 128, 129, 1   <DO_LOOP> <vector-remainder> <novectorize>
; CHECK-NEXT:              |   %0 = (%a)[i1 + sext.i32.i64(%i)];
; CHECK-NEXT:              |   %copy = %0;
; CHECK-NEXT:              |   %__svml_sincosf1 = @__svml_sincosf1(%copy);
; CHECK-NEXT:              |   %sincos.sin = extractvalue %__svml_sincosf1, 0;
; CHECK-NEXT:              |   %sincos.cos = extractvalue %__svml_sincosf1, 1;
; CHECK-NEXT:              |   %elem = extractelement %sincos.sin,  0;
; CHECK-NEXT:              |   (%vsin)[0] = %elem;
; CHECK-NEXT:              |   %elem7 = extractelement %sincos.cos,  0;
; CHECK-NEXT:              |   (%vcos)[0] = %elem7;
; CHECK-NEXT:              |   %sin.load = (%vsin)[0];
; CHECK-NEXT:              |   %cos.load = (%vcos)[0];
; CHECK-NEXT:              |   %res = %sin.load  +  %cos.load;
; CHECK-NEXT:              |   (%b)[i1 + sext.i32.i64(%i)] = %res;
; CHECK-NEXT:              + END LOOP
; CHECK-NEXT:        END REGION

define void @test_sincosf(float* noalias nocapture readonly %a, float* noalias nocapture readonly %b, i32 %i) {
entry:
  %vsin = alloca float, align 4
  %vcos = alloca float, align 4
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.PRIVATE"(float* %vsin), "QUAL.OMP.PRIVATE"(float* %vcos) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %idxprom = sext i32 %i to i64
  %stride.mul = mul i32 1, %index
  %stride.cast = sext i32 %stride.mul to i64
  %stride.add = add i64 %idxprom, %stride.cast
  %arrayidx = getelementptr inbounds float, float* %a, i64 %stride.add
  %0 = load float, float* %arrayidx, align 4
  tail call void @sincosf(float inreg %0, float* nonnull %vsin, float* nonnull %vcos) #3
  %sin.load = load float, float* %vsin, align 4
  %cos.load = load float, float* %vcos, align 4
  %res = fadd fast float %sin.load, %cos.load
  %arrayidx2 = getelementptr inbounds float, float* %b, i64 %stride.add
  store float %res, float* %arrayidx2, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 130
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

declare dso_local void @sincosf(float, float*, float*) local_unnamed_addr #1

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #1 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
