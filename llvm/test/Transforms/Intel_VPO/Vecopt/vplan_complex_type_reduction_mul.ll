; Test to verify that VPlan framework bailsout for complex type mul reduction.
; Complex intrinsics from FE need to be enabled by default to support this
; opcode.

; C code:
; #include <complex.h>
;
; float complex foo(float complex *A) {
;   float complex prod = 1.0 + 0.0 * I;
;   int i = 0;
; #pragma omp simd reduction(+:prod)
;   for (i = 0; i < N; i++)
;     prod *= A[i];
;
;   return prod;
; }

; REQUIRES: asserts
; RUN: opt -passes=vplan-vec -vplan-force-vf=2 -S -debug-only=VPlanLegality < %s 2>&1 | FileCheck %s --check-prefix=LLVM-IR
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=2 -debug-only=VPlanLegality -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIRVEC
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=medium < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTMED
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI-HIR

; LLVM-IR: Complex mul/div type reductions are not supported
; LLVM-IR: %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MUL:CMPLX.TYPED"(ptr %prod, %complex_64bit zeroinitializer, i32 1) ]

; HIRVEC: Complex mul/div type reductions are not supported
; HIRVEC: BEGIN REGION { }
; HIRVEC:      %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(), QUAL.OMP.REDUCTION.MUL:CMPLX.TYPED(&((%prod)[0]), zeroinitializer, 1) ]
; HIRVEC:      DO i1 = 0, %N + -1, 1   <DO_LOOP> <simd> <vectorize>

; OPTRPTMED: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized: Complex mul/div type reductions are not supported.
; OPTRPTHI-HIR: remark #15436: loop was not vectorized: HIR: Complex mul/div type reductions are not supported.

%complex_64bit = type { float, float }

define %complex_64bit @foo(ptr nocapture readonly %A, i64 %N) {
entry:
  %prod = alloca %complex_64bit, align 8
  %insertval = insertvalue %complex_64bit zeroinitializer, float 1.000000e+00, 0
  %insertval1 = insertvalue %complex_64bit %insertval, float 0.000000e+00, 1
  store %complex_64bit %insertval1, ptr %prod, align 4
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MUL:CMPLX.TYPED"(ptr %prod, %complex_64bit zeroinitializer, i32 1) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %loop.latch ], [ 0, %begin.simd ]
  br label %begin.guard

begin.guard:
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %prod) ]
  br label %loop.body

loop.body:
  %arrayidx.realp = getelementptr inbounds %complex_64bit, ptr %A, i64 %indvars.iv, i32 0
  %A.i.real = load float, ptr %arrayidx.realp, align 4
  %arrayidx.imagp = getelementptr inbounds %complex_64bit, ptr %A, i64 %indvars.iv, i32 1
  %A.i.imag = load float, ptr %arrayidx.imagp, align 4
  %prod.real = load float, ptr %prod, align 4
  %prod.imagp = getelementptr inbounds %complex_64bit, ptr %prod, i64 0, i32 1
  %prod.imag = load float, ptr %prod.imagp, align 4
  %mul_ad = fmul fast float %A.i.imag, %prod.real
  %mul_bc = fmul fast float %A.i.real, %prod.imag
  %mul_i = fadd fast float %mul_bc, %mul_ad
  %mul_ac = fmul fast float %A.i.real, %prod.real
  %mul_bd = fmul fast float %prod.imag, %A.i.imag
  %mul_r = fsub fast float %mul_ac, %mul_bd
  store float %mul_r, ptr %prod, align 4
  store float %mul_i, ptr %prod.imagp, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %end.guard

end.guard:
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %loop.latch

loop.latch:
  %exitcond = icmp eq i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                             ; preds = %for.body
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  %fin = load %complex_64bit, ptr %prod, align 4
  ret %complex_64bit %fin

}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

