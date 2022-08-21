; Test to verify that VPlan vectorizers bailout on encountering complex type
; reductions.

; REQUIRES: asserts
; RUN: opt -vplan-vec -vplan-force-vf=2 -S -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s --check-prefix=LLVM-IR
; RUN: opt -passes="vplan-vec" -vplan-force-vf=2 -S -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s --check-prefix=LLVM-IR
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -debug-only=HIRLegality -print-after=hir-vplan-vec -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s --check-prefix=HIR
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -debug-only=HIRLegality -print-after=hir-vplan-vec -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s --check-prefix=HIR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -debug-only=HIRLegality -print-after=hir-vplan-vec -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s --check-prefix=HIR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -debug-only=HIRLegality -print-after=hir-vplan-vec -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s --check-prefix=HIR

; LLVM-IR: Complex type reductions are not supported
; LLVM-IR: %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:CMPLX.TYPED"(%complex_64bit* %sum, %complex_64bit zeroinitializer, i32 1) ]

; HIR: Complex type reductions are not supported
; HIR: BEGIN REGION { }
; HIR:      %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(), QUAL.OMP.REDUCTION.ADD:CMPLX.TYPED(&((%sum)[0])zeroinitializer1) ]
; HIR:      DO i1 = 0, %N + -1, 1   <DO_LOOP> <simd> <vectorize>

%complex_64bit = type { float, float }

define %complex_64bit @foo(%complex_64bit* nocapture readonly %A, i64 %N) {
entry:
  %sum = alloca %complex_64bit, align 8
  %insertval = insertvalue %complex_64bit zeroinitializer, float 0.000000e+00, 0
  %insertval1 = insertvalue %complex_64bit %insertval, float 0.000000e+00, 1
  store %complex_64bit %insertval1, %complex_64bit* %sum, align 4
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:CMPLX.TYPED"(%complex_64bit* %sum, %complex_64bit zeroinitializer, i32 1) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %begin.simd ]
  %arrayidx = getelementptr inbounds %complex_64bit, %complex_64bit* %A, i64 %indvars.iv
  %A.i = load %complex_64bit, %complex_64bit* %arrayidx, align 4
  %sum.ld = load %complex_64bit, %complex_64bit* %sum, align 4
  %A.i.0 = extractvalue %complex_64bit %A.i, 0
  %sum.ld.0 = extractvalue %complex_64bit %sum.ld, 0
  %add.0 = fadd float %A.i.0, %sum.ld.0
  %A.i.1 = extractvalue %complex_64bit %A.i, 1
  %sum.ld.1 = extractvalue %complex_64bit %sum.ld, 1
  %add.1 = fadd float %A.i.1, %sum.ld.1
  %next.0 = insertvalue %complex_64bit zeroinitializer, float %add.0, 0
  %next.1 = insertvalue %complex_64bit %next.0, float %add.1, 0
  store %complex_64bit %next.1, %complex_64bit* %sum, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                             ; preds = %for.body
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  %fin = load %complex_64bit, %complex_64bit* %sum, align 4
  ret %complex_64bit %fin

}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

