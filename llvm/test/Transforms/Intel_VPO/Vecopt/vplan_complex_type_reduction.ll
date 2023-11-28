; Test to verify that VPlan framework imports and handles complex type reductions.

; C code:
; #include <complex.h>
;
; float complex foo(float complex *A) {
;   float complex sum = 0.0 + 0.0 * I;
;   int i = 0;
; #pragma omp simd reduction(+:sum)
;   for (i = 0; i < N; i++)
;     sum += A[i];
;
;   return sum;
; }

; RUN: opt -S -passes="vplan-vec" -vplan-entities-dump -vplan-print-legality -vplan-print-after-vpentity-instrs -vplan-force-vf=4 < %s 2>&1 | FileCheck %s -check-prefixes=IR,CHECK
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec,print<hir>" -vplan-entities-dump -vplan-print-legality -vplan-print-after-vpentity-instrs -vplan-force-vf=4 < %s 2>&1 | FileCheck %s -check-prefixes=HIR,CHECK
; REQUIRES: asserts

; ------------------------------------------------------------------------------

; Check that complex type reductions are captured in legality lists
; HIR-LABEL:   HIRLegality ReductionList:
; HIR:         Ref: &((%sum)[0])
; HIR:           UpdateInstructions:
; HIR-NEXT:      <20>         (%sum)[0].0 = %add.real;
; HIR-NEXT:      <21>         (%sum)[0].1 = %add.imag;
; HIR-NEXT:      InitValue: %sum
; HIR-NEXT:      RedDescr: {Kind: fadd, IsSigned: 0, IsComplex: 1}

; ------------------------------------------------------------------------------

; Check that complex type reductions are imported as VPEntities and lowered to VPInstructions.
; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:       Reduction list
; CHECK-NEXT:    complex (+) Start: ptr %sum
; CHECK-NEXT:    Linked values: ptr [[PVT:%vp.*]], <2 x float> [[INIT:%vp.*]],
; CHECK-NEXT:   Memory: ptr %sum

; Initialization
; CHECK:        ptr [[PVT]] = allocate-priv %complex_64bit = type { float, float }, OrigAlign = 8
; CHECK:        <2 x float> [[INIT]] = reduction-init-cmplx <2 x float> zeroinitializer
; CHECK:        store <2 x float> [[INIT]] ptr [[PVT]]

; In-loop instructions
; IR:           float [[SUM_REALP_V:%.*]] = load ptr [[PVT]]
; IR:           ptr [[SUM_IMAGP:%.*]] = getelementptr inbounds %complex_64bit, ptr [[PVT]] i64 0 i32 1
; IR:           float [[SUM_IMAGP_V:%.*]] = load ptr [[SUM_IMAGP]]
; HIR:          ptr [[SUM_REALP:%.*]] = subscript inbounds ptr [[PVT]]
; HIR:          float [[SUM_REALP_V:%.*]] = load ptr [[SUM_REALP]]
; HIR:          ptr [[SUM_IMAGP:%.*]] = subscript inbounds ptr [[PVT]] i64 0 (1 )
; HIR:          float [[SUM_IMAGP_V:%.*]] = load ptr [[SUM_IMAGP]]

; Finalization
; CHECK:        %complex_64bit = type { float, float } [[FIN:%.*]] = reduction-final-cmplx ptr [[PVT]] ptr %sum

; ------------------------------------------------------------------------------

; Check LLVM-IR CG of complex type reductions.
; IR-LABEL: @foo(
; IR:       entry:
; IR:         [[SUM_VEC:%.*]] = alloca [4 x %complex_64bit], align 8
; IR-NEXT:    [[SUM_VEC_BASE_ADDR:%.*]] = getelementptr %complex_64bit, ptr [[SUM_VEC]], <4 x i32> <i32 0, i32 1, i32 2, i32 3>

; IR:       VPlannedBB2:
; IR:    store <8 x float> zeroinitializer, ptr [[SUM_VEC]], align 1

; IR:       VPlannedBB13:
; IR-NEXT:    [[CMPLX_FIN_VEC:%.*]] = load <8 x float>, ptr [[SUM_VEC]], align 4
; IR-NEXT:    [[CMPLX_FIN_SECOND_HALF:%.*]] = shufflevector <8 x float> [[CMPLX_FIN_VEC]], <8 x float> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 poison, i32 poison, i32 poison, i32 poison>
; IR-NEXT:    [[CMPLX_FIN_REDOP:%.*]] = fadd <8 x float> [[CMPLX_FIN_VEC]], [[CMPLX_FIN_SECOND_HALF]]
; IR-NEXT:    [[CMPLX_FIN_SECOND_HALF15:%.*]] = shufflevector <8 x float> [[CMPLX_FIN_REDOP]], <8 x float> undef, <8 x i32> <i32 2, i32 3, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison>
; IR-NEXT:    [[CMPLX_FIN_REDOP16:%.*]] = fadd <8 x float> [[CMPLX_FIN_REDOP]], [[CMPLX_FIN_SECOND_HALF15]]
; IR-NEXT:    [[CMPLX_FIN_VEC_REDUCED:%.*]] = shufflevector <8 x float> [[CMPLX_FIN_REDOP16]], <8 x float> undef, <2 x i32> <i32 0, i32 1>
; IR-NEXT:    [[ORIG_CMPLX:%.*]] = load <2 x float>, ptr [[SUM:%.*]], align 4
; IR-NEXT:    [[CMPLX_FINAL:%.*]] = fadd <2 x float> [[ORIG_CMPLX]], [[CMPLX_FIN_VEC_REDUCED]]
; IR-NEXT:    store <2 x float> [[CMPLX_FINAL]], ptr [[SUM]], align 4

; ------------------------------------------------------------------------------

; HIR before VPlan
; BEGIN REGION { }
;       %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD:CMPLX.TYPED(&((%sum)[0])zeroinitializer1) ]
;
;       + DO i1 = 0, %N + -1, 1   <DO_LOOP> <simd>
;       |   %guard.start = @llvm.directive.region.entry(); [ DIR.VPO.GUARD.MEM.MOTION(),  QUAL.OMP.LIVEIN(&((%sum)[0])) ]
;       |   %add.real = (%A)[i1].0  +  (%sum)[0].0;
;       |   %add.imag = (%A)[i1].1  +  (%sum)[0].1;
;       |   (%sum)[0].0 = %add.real;
;       |   (%sum)[0].1 = %add.imag;
;       |   @llvm.directive.region.exit(%guard.start); [ DIR.VPO.END.GUARD.MEM.MOTION() ]
;       + END LOOP
;
;       @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
; END REGION

; Check HIR-CG of complex type reductions.
; HIR-LABEL: Function: foo
; HIR:              %priv.mem.bc = &((%complex_64bit*)(%priv.mem)[0]);
; HIR:              %red.init = zeroinitializer;
; HIR:              (<8 x float>*)(%priv.mem)[0] = %red.init;

; HIR:              + DO i1 = 0
; HIR:              + END LOOP

; HIR:              %cmplx.fin.vec = (<8 x float>*)(%priv.mem)[0];
; HIR-NEXT:         %cmplx.fin.second.half = shufflevector %cmplx.fin.vec,  undef,  <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef>;
; HIR-NEXT:         %cmplx.fin.vec = %cmplx.fin.vec  +  %cmplx.fin.second.half;
; HIR-NEXT:         %cmplx.fin.second.half20 = shufflevector %cmplx.fin.vec,  undef,  <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>;
; HIR-NEXT:         %cmplx.fin.vec = %cmplx.fin.vec  +  %cmplx.fin.second.half20;
; HIR-NEXT:         %cmplx.fin.vec.reduced = shufflevector %cmplx.fin.vec,  undef,  <i32 0, i32 1>;
; HIR-NEXT:         %orig.cmplx = (<2 x float>*)(%sum)[0];
; HIR-NEXT:         %cmplx.final = %orig.cmplx  +  %cmplx.fin.vec.reduced;
; HIR-NEXT:         (<2 x float>*)(%sum)[0] = %cmplx.final;

; ------------------------------------------------------------------------------

%complex_64bit = type { float, float }

define %complex_64bit @foo(ptr nocapture readonly %A, i64 %N) {
entry:
  %sum = alloca %complex_64bit, align 8
  %insertval = insertvalue %complex_64bit zeroinitializer, float 0.000000e+00, 0
  %insertval1 = insertvalue %complex_64bit %insertval, float 0.000000e+00, 1
  store %complex_64bit %insertval1, ptr %sum, align 4
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:CMPLX.TYPED"(ptr %sum, %complex_64bit zeroinitializer, i32 1) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %loop.latch ], [ 0, %begin.simd ]
  br label %begin.guard

begin.guard:
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %sum) ]
  br label %loop.body

loop.body:
  %arrayidx.realp = getelementptr inbounds %complex_64bit, ptr %A, i64 %indvars.iv, i32 0
  %A.i.real = load float, ptr %arrayidx.realp, align 4
  %arrayidx.imagp = getelementptr inbounds %complex_64bit, ptr %A, i64 %indvars.iv, i32 1
  %A.i.imag = load float, ptr %arrayidx.imagp, align 4
  %sum.real = load float, ptr %sum, align 4
  %sum.imagp = getelementptr inbounds %complex_64bit, ptr %sum, i64 0, i32 1
  %sum.imag = load float, ptr %sum.imagp, align 4
  %add.real = fadd float %A.i.real, %sum.real
  %add.imag = fadd float %A.i.imag, %sum.imag
  store float %add.real, ptr %sum, align 4
  store float %add.imag, ptr %sum.imagp, align 4
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
  %fin = load %complex_64bit, ptr %sum, align 4
  ret %complex_64bit %fin

}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

