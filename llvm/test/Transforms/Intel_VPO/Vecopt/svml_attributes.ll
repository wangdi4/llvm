; Check that parameter attributes are preserved during vectorization.
; Note all the "inreg" attributes specified in this test don't make any
; practical sense. LLVM doesn't has an attribute for parameters with
; floating-point type. So "inreg" is used instead for the purpose of this test.

; RUN: opt -vector-library=SVML -vplan-vec -verify -S -vplan-force-vf=4 %s | FileCheck -DVL=4 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-LT-512 %s
; RUN: opt -vector-library=SVML -vplan-vec -verify -S -vplan-force-vf=8 %s | FileCheck -DVL=8 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-512 %s
; RUN: opt -vector-library=SVML -vplan-vec -verify -S -vplan-force-vf=16 %s | FileCheck -DVL=16 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s
; RUN: opt -vector-library=SVML -vplan-vec -verify -S -vplan-force-vf=32 %s | FileCheck -DVL=32 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s
; RUN: opt -vector-library=SVML -vplan-vec -verify -S -vplan-force-vf=64 %s | FileCheck -DVL=64 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s
; RUN: opt -vector-library=SVML -vplan-vec -verify -S -vplan-force-vf=128 %s | FileCheck -DVL=64 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s

; RUN: opt -vector-library=SVML -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -hir-cg -verify -S -vplan-force-vf=4 < %s 2>&1 | FileCheck -DVL=4 --check-prefixes=CHECK,CHECK-HIR,FLOAT-LT-512,DOUBLE-LT-512 %s
; RUN: opt -vector-library=SVML -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -hir-cg -verify -S -vplan-force-vf=8 < %s 2>&1 | FileCheck -DVL=8 --check-prefixes=CHECK,CHECK-HIR,FLOAT-LT-512,DOUBLE-512 %s
; RUN: opt -vector-library=SVML -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -hir-cg -verify -S -vplan-force-vf=16 < %s 2>&1 | FileCheck -DVL=16 --check-prefixes=CHECK,CHECK-HIR,FLOAT-512,DOUBLE-512 %s
; RUN: opt -vector-library=SVML -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -hir-cg -verify -S -vplan-force-vf=32 < %s 2>&1 | FileCheck -DVL=32 --check-prefixes=CHECK,CHECK-HIR,FLOAT-512,DOUBLE-512 %s

; CHECK-LABEL: @test_sinf(
; CHECK: call fast svml_cc <[[VL]] x float> @__svml_sinf[[VL]](<[[VL]] x float> inreg {{%.*}})
; CHECK-HIR: call fast svml_cc <[[VL]] x float> @__svml_sinf[[VL]](<[[VL]] x float> inreg {{%.*}})

; CHECK-LABEL: @test_sin(
; CHECK: call fast svml_cc <[[VL]] x double> @__svml_sin[[VL]](<[[VL]] x double> inreg {{%.*}})
; CHECK-HIR: call fast svml_cc <[[VL]] x double> @__svml_sin[[VL]](<[[VL]] x double> inreg {{%.*}})

; CHECK-LABEL: @test_masked_sinf(
; FLOAT-LT-512: call fast svml_cc <[[VL]] x float> @__svml_sinf[[VL]]_mask(<[[VL]] x float> inreg {{%.*}}, <[[VL]] x i32> {{%.*}})
; FLOAT-512: call fast svml_cc <[[VL]] x float> @__svml_sinf[[VL]]_mask(<[[VL]] x float> undef, <[[VL]] x i1> {{%.*}}, <[[VL]] x float> inreg {{%.*}})
; CHECK-HIR: call fast svml_cc <[[VL]] x float> @__svml_sinf[[VL]](<[[VL]] x float> inreg {{%.*}})

; CHECK-LABEL: @test_masked_sin(
; DOUBLE-LT-512: call fast svml_cc <[[VL]] x double> @__svml_sin[[VL]]_mask(<[[VL]] x double> inreg {{%.*}}, <[[VL]] x i64> {{%.*}})
; DOUBLE-512: call fast svml_cc <[[VL]] x double> @__svml_sin[[VL]]_mask(<[[VL]] x double> undef, <[[VL]] x i1> {{%.*}}, <[[VL]] x double> inreg {{%.*}})
; CHECK-HIR: call fast svml_cc <[[VL]] x double> @__svml_sin[[VL]](<[[VL]] x double> inreg {{%.*}})

; CHECK-LABEL: @test_sincosf(
; CHECK: call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf[[VL]](<[[VL]] x float> inreg {{%.*}})
; CHECK-HIR: call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf[[VL]](<[[VL]] x float> inreg {{%.*}})

; CHECK-LABEL: @test_masked_sincosf(
; FLOAT-LT-512: call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf[[VL]]_mask(<[[VL]] x float> inreg {{%.*}}, <[[VL]] x i32> {{%.*}})
; FLOAT-512: call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf[[VL]]_mask({ <[[VL]] x float>, <[[VL]] x float> } undef, <[[VL]] x i1> {{%.*}}, <[[VL]] x float> inreg {{%.*}})
; CHECK-HIR: call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf[[VL]](<[[VL]] x float> inreg {{%.*}})

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@N = common dso_local local_unnamed_addr global i32 0, align 4

define void @test_sinf(float* nocapture %a, float* nocapture readonly %b, i32 %i) #0 {
entry:
  br label %simd.begin.region

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
  %call = tail call fast float @sinf(float inreg %0)
  %arrayidx2 = getelementptr inbounds float, float* %a, i64 %stride.add
  store float %call, float* %arrayidx2, align 4
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

define void @test_sin(double* nocapture %a, double* nocapture readonly %b, i32 %i) #0 {
entry:
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM:TYPED"(double* %a, double zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(double* %b, double zeroinitializer, i32 1) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %idxprom = sext i32 %i to i64
  %stride.mul = mul i32 1, %index
  %stride.cast = sext i32 %stride.mul to i64
  %stride.add = add i64 %idxprom, %stride.cast
  %arrayidx = getelementptr inbounds double, double* %b, i64 %stride.add
  %0 = load double, double* %arrayidx, align 8
  %call = tail call fast double @sin(double inreg %0)
  %arrayidx2 = getelementptr inbounds double, double* %a, i64 %stride.add
  store double %call, double* %arrayidx2, align 8
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

define void @test_masked_sinf(float* nocapture readonly %input, float* nocapture readonly %b, float* %a) #0 {
entry:
  %0 = load i32, i32* @N, align 4
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count = sext i32 %0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.body.continue ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %2 = load float, float* %arrayidx, align 4
  %cmp6 = fcmp ogt float %2, 3.000000e+00
  br i1 %cmp6, label %if.then, label %omp.body.continue

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx8 = getelementptr inbounds float, float* %input, i64 %indvars.iv
  %3 = load float, float* %arrayidx8, align 4
  %arrayidx10 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %call = tail call fast float @sinf(float inreg %3)
  store float %call, float* %arrayidx10, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.body.continue
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

define void @test_masked_sin(double* nocapture readonly %input, double* nocapture readonly %b, double* %a) #0 {
entry:
  %0 = load i32, i32* @N, align 4
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count = sext i32 %0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.body.continue ]
  %arrayidx = getelementptr inbounds double, double* %b, i64 %indvars.iv
  %2 = load double, double* %arrayidx, align 8
  %cmp6 = fcmp ogt double %2, 3.000000e+00
  br i1 %cmp6, label %if.then, label %omp.body.continue

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx8 = getelementptr inbounds double, double* %input, i64 %indvars.iv
  %3 = load double, double* %arrayidx8, align 8
  %arrayidx10 = getelementptr inbounds double, double* %a, i64 %indvars.iv
  %call = tail call fast double @sin(double inreg %3)
  store double %call, double* %arrayidx10, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.body.continue
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

define void @test_sincosf(float* noalias nocapture readonly %a, float* noalias %vsin, float* noalias %vcos, i32 %i) #0 {
entry:
  br label %simd.begin.region

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
  tail call void @sincosf(float inreg %0, float* nonnull %arrayidx2, float* nonnull %arrayidx4) #3
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

define void @test_masked_sincosf(float* nocapture readonly %input, float* nocapture readonly %b, float* %vsin, float* %vcos) #0 {
entry:
  %0 = load i32, i32* @N, align 4
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count = sext i32 %0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.body.continue ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %2 = load float, float* %arrayidx, align 4
  %cmp6 = fcmp ogt float %2, 3.000000e+00
  br i1 %cmp6, label %if.then, label %omp.body.continue

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx8 = getelementptr inbounds float, float* %input, i64 %indvars.iv
  %3 = load float, float* %arrayidx8, align 4
  %arrayidx10 = getelementptr inbounds float, float* %vsin, i64 %indvars.iv
  %arrayidx12 = getelementptr inbounds float, float* %vcos, i64 %indvars.iv
  tail call void @sincosf(float inreg %3, float* nonnull %arrayidx10, float* nonnull %arrayidx12)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.body.continue
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

declare dso_local float @sinf(float) local_unnamed_addr #1

declare dso_local double @sin(double) local_unnamed_addr #1

declare dso_local void @sincosf(float, float*, float*) local_unnamed_addr #1

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
